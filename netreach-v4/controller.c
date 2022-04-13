#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <fstream>
#include <errno.h>
#include <set>
#include <signal.h> // for signal and raise
#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // inetaddr conversion
#include <sys/time.h> // struct timeval
#include <string.h>
#include <map>

#include "helper.h"
#include "key.h"
#include "val.h"
#include "socket_helper.h"

#include "common_impl.h"

typedef Key index_key_t;
typedef Val val_t;

bool volatile controller_running = false;
std::atomic<size_t> controller_ready_threads(0);
const size_t controller_expected_ready_threads = 3;

struct alignas(CACHELINE_SIZE) ControllerPopserverSubthreadParam {
	int connfd;
	uint32_t subthreadidx;
};
typedef ControllerPopserverSubthreadParam controller_popserver_subthread_param_t;

// Per-server popclient <-> one popserver.subthread in controller
// NOTE: subthreadidx != serveridx
int volatile controller_popserver_tcpsock = -1;
pthread_t *volatile controller_popserver_subthreads = NULL;
std::atomic<size_t> controller_finish_subthreads(0);
size_t controller_expected_finish_subthreads = -1;

// Keep atomicity for the following variables
std::mutex mutex_for_pop;
//std::set<index_key_t> * volatile controller_cached_keyset_list = NULL; // TODO: Comment it after checking server.cached_keyset_list
//std::map<index_key_t, int16_t> volatile controller_cachedkey_serveridx_map; // TODO: Evict removes the corresponding kv pair
//std::map<int16_t, uint32_t> volatile controller_serveridx_subthreadidx_map; // Not used
// Message queue between controller.popservers with controller.popclient (connected with switchos.popserver)
cache_pop_t ** volatile controller_cache_pop_ptrs = NULL;
uint32_t volatile controller_head_for_pop = 0;
uint32_t volatile controller_tail_for_pop = 0;

// controller.popclient <-> switchos.popserver
bool volatile is_controller_popclient_connected = false;
int volatile controller_popclient_tcpsock = -1;

// switchos.popworker <-> controller.evictserver
int volatile controller_evictserver_tcpsock = -1;
// controller.evictclients <-> servers.evictserver
// NOTE: evictclient.index = serveridx
bool volatile is_controller_evictserver_evictclients_connected = false;
int * volatile controller_evictserver_evictclient_tcpsock_list = NULL;

void prepare_controller();
void *run_controller_popserver(void *param); // Accept connections from servers
void *run_controller_popserver_subthread(void *param); // Receive CACHE_POPs from one server
void *run_controller_popclient(void *param); // Send CACHE_POPs to switch os
void *run_controller_evictserver(void *param); // Forward CACHE_EVICT to server and CACHE_EVICT_ACK to switchos in cache eviction
void close_controller();

int main(int argc, char **argv) {
	parse_ini("config.ini");

	prepare_controller();

	pthread_t popserver_thread;
	int ret = pthread_create(&popserver_thread, nullptr, run_controller_popserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t popclient_thread;
	int ret = pthread_create(&popclient_thread, nullptr, run_controller_popclient, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	while (controller_ready_threads < controller_expected_ready_threads) sleep(1);

	controller_running = true;

	while (controller_finish_subthreads < controller_expected_finishs_subthreads) sleep(1);

	controller_running = false;

	int rc = pthread_join(popsever_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(popclient_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}

	close_controller();
}

void prepare_controller() {
	controller_running =false;

	// Prepare for cache population
	controller_popserver_subthreads = new pthread_t[server_num];
	controller_expected_finish_subthreads = server_num;

	// prepare popserver socket
	prepare_tcpserver(controller_popserver_tcpsock, true, controller_popserver_port, server_num, "controller.popserver"); // MAX_PENDING_CONNECTION = server num

	/*controller_cached_keyset_list = new std::set<index_key_t>[server_num];
	for (size_t i = 0; i < server_num; i++) {
		controller_cached_keyset_list[i].clear();
	}*/

	controller_cache_pop_ptrs = new cache_pop_t*[MQ_SIZE];
	for (size_t i = 0; i < MQ_SIZE; i++) {
		controller_cache_pop_ptrs[i] = NULL;
	}

	//controller_cachedkey_serveridx_map.clear();
	//controller_serveridx_subthreadidx_map.clear();
	controller_head_for_pop = 0;
	controller_tail_for_pop = 0;
	create_tcpsock(controller_popclient_tcpsock, "controller.popclient");

	// prepare evictserver
	prepare_tcpserver(controller_evictserver_tcpsock, false, controller_evictserver_port, 1, "controller.evictserver"); // MAX_PENDING_CONNECTION = 1 (switchos.popworker.evictclient)

	// prepare evictclients
	controller_evictserver_evictclient_tcpsock_list = new int[server_num];
	for (size_t i = 0; i < server_num; i++) {
		create_tcpsock(controller_evictserver_evictclient_tcpsock_list[i], "controller.evictserver.evictclient");
	}
}

void *run_controller_popserver(void *param) {
	uint32_t subthreadidx = 0;
	controller_ready_threads++;

	while (!controller_running) {}

	while (controller_running) {
		// Not used
		struct sockaddr_in server_addr;
		unsigned int server_addr_len = sizeof(struct sockaddr);

		int connfd = -1;
		bool is_timeout = tcpaccept(controller_popserver_tcpsock, NULL, NULL, connfd, "controller.popserver");
		if (is_timeout) {
			continue; // timeout or interrupted system call
		}

		// NOTE: subthreadidx != serveridx
		controller_popserver_subthread_param_t param;
		param.connfd = connfd;
		param.subthreadidx = subthreadidx;
		int ret = pthread_create(&controller_popserver_subthreads[subthreadidx], nullptr, run_controller_popserver_subthread, (void *)&param);
		if (ret) {
			COUT_N_EXIT("Error: unable to create controller.popserver.subthread " << ret);
		}
		subthreadidx += 1;
		INVARIANT(subthreadidx <= server_num);
	}
	uint32_t real_servernum = subthreadidx;
	INVARIANT(real_servernum == server_num);

	for (size_t i = 0; i < real_servernum; i++) {
		void * status;
		int rc = pthread_join(controller_popserver_subthreads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join " << rc);
		}
	}

	close(controller_popserver_tcpsock);
	pthread_exit(nullptr);
}

void run_controller_popserver_subthread(void *param) {
	controller_popserver_subthread_param_t *curparam = (controller_popserver_subthread_param_t *)param;
	int connfd = curparam->connfd;
	uint32_t subthreadidx = curparam->subthreadidx;

	// Process CACHE_POP packet <optype, key, vallen, value, seq, serveridx>
	char buf[MAX_BUFSIZE];
	int cur_recv_bytes = -1;
	int8_t optype = -1;
	int32_t vallen = -1;
	int arrive_serveridx_bytes = -1;
	bool is_cached_before = false;
	//index_key_t tmpkey(0, 0, 0, 0);
	const int arrive_optype_bytes = sizeof(int8_t);
	const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(key_t) + sizeof(int32_t);
	while (true) {
		int recvsize = 0;
		bool is_broken = tcprecv(connfd, buf + cur_recv_bytes, MAX_BUFSIZE - cur_recv_bytes, 0, recvsize, "controller.popserver.subthread");
		if (is_broken) {
			break;
		}

		cur_recv_bytes += recvsize;
		if (cur_recv_bytes >= MAX_BUFSIZE) {
			printf("[controller.popserver] Overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_BUFSIZE);
            exit(-1);
		}

		// Get optype
		if (optype == -1 && cur_recv_bytes >= arrive_optype_bytes) {
			optype = *((int8_t *)buf);
			INVARIANT(packet_type_t(optype) == packet_type_t::CACHE_POP);
		}

		// Get vallen
		if (optype != -1 && vallen == -1 && cur_recv_bytes >= arrive_vallen_bytes) {
			//tmpkey.deserialize(buf + arrive_optype_bytes, cur_recv_bytes - arrive_optype_bytes);
			vallen = *((int32_t *)(buf + arrive_vallen_bytes - sizeof(int32_t)));
			vallen = int32_t(ntohl(uint32_t(vallen)));
			INVARIANT(vallen >= 0);
			int padding_size = int(val_t::get_padding_size(vallen)); // padding for value <= 128B
			arrive_serveridx_bytes = arrive_vallen_bytes + vallen + padding_size + sizeof(int32_t) + sizeof(int16_t);
		}

		// Get one complete CACHE_POP
		if (optype != -1 && vallen != -1 && cur_recv_bytes >= arrive_serveridx_bytes) {
			cache_pop_t *tmp_cache_pop_ptr = new cache_pop_t(buf, arrive_serveridx_bytes); // freed by controller.popclient

			//is_cached_before = (controller_cached_keyset_list[tmp_cache_pop_ptr->serveridx()].find(tmp_cache_pop_ptr->key()) != controller_cached_keyset_list[tmp_cache_pop_ptr->serveridx()].end());
			if (!is_cached_before) {
				// Add key into cached keyset (TODO: need to be protected by mutex lock)
				//controller_cached_keyset_list[tmp_cache_pop_ptr->serveridx()].insert(tmp_cache_pop_ptr->key());
				/*if (controller_cachedkey_serveridx_map.find(tmp_cache_pop->key()) == controller_cachedkey_serveridx_map.end()) {
					controller_cachedkey_serveridx_map.insert(std::pair<index_key_t, uint32_t>(tmp_cache_pop_ptr->key(), tmp_cache_pop_ptr->serveridx()));
				}
				else {
					printf("[controller] Receive duplicate key from server %ld!", tmp_cache_pop_ptr->serveridx());
					exit(-1);
				}*/

				// Serialize CACHE_POPs
				mutex_for_pop.lock();
				/*if (controller_serveridx_subthreadidx_map.find(tmp_cache_pop_ptr->serveridx()) == controller_serveridx_subthreadidx_map.end()) {
					controller_serveridx_subthreadidx_map.insert(std::pair<int16_t, uint32_t>(tmp_cache_pop_ptr->serveridx(), subthreadidx));
				}
				else {
					INVARIANT(controller_serveridx_subthreadidx_map[tmp_cache_pop_ptr->serveridx()] == subthreadidx);
				}*/
				if ((controller_head_for_pop+1)%MQ_SIZE != controller_tail_for_pop) {
					controller_cache_pop_ptrs[controller_head_for_pop] = tmp_cache_pop_ptr;
					controller_head_for_pop = (controller_head_for_pop + 1) % MQ_SIZE;
				}
				else {
					printf("[controller] message queue overflow of controller.controller_cache_pop_ptrs!");
				}
				mutex_for_pop.unlock();
			}

			// Move remaining bytes and reset metadata
			if (cur_recv_bytes > arrive_seq_bytes) {
				memcpy(buf, buf + arrive_serveridx_bytes, cur_recv_bytes - arrive_serveridx_bytes);
				cur_recv_bytes = cur_recv_bytes - arrive_serveridx_byets;
				optype = -1;
				vallen = -1;
				arrive_serveridx_bytes = -1;
				is_cached_before = false;
			}
			else {
				cur_recv_bytes = 0;
				optype = -1;
				vallen = -1;
				arrive_serveridx_bytes = -1;
				is_cached_before = false;
			}
		}
	}

	controller_finish_subthreads++;
	close(connfd);
	pthread_exit(nullptr);
}

void run_controller_popclient(void *param) {
	controller_ready_threads++;

	while (!controller_running) {}

	while (controller_running) {
		char buf[MAX_BUFSIZE];
		if (controller_tail_for_pop != controller_head_for_pop) {
			if (!is_controller_popclient_connected) {
				tcpconnect(controller_popclient_tcpsock, switchos_ip, switchos_popserver_port, "controller.popoclient", "switchos.popserver");
				is_controller_popclient_connected = true;
			}

			cache_pop_t *tmp_cache_pop_ptr = controller_cache_pop_ptrs[controller_tail_for_pop];
			// send CACHE_POP to switch os
			uint32_t popsize = tmp_cache_pop_ptr->serialize(buf, MAX_BUFSIZE);
			tcpsend(controller_popclient_tcpsock, buf, popsize, "controller.popclient");
			// free CACHE_POP
			delete tmp_cache_pop_ptr;
			tmp_cache_pop_ptr = NULL;
			controller_cache_pop_ptrs[controller_tail_for_pop] = NULL;
			controller_tail_for_pop = (controller_tail_for_pop + 1) % MQ_SIZE;
		}
	}

	close(controller_popclient_tcpsock);
	pthread_exit(nullptr);
}

void run_controller_evictserver(void *param) {
	// Not used
	struct sockaddr_in switchos_addr;
	unsigned int switchos_addr_len = sizeof(struct switchos_addr);

	controller_ready_threads++;

	while (!controller_running) {}

	// accept connection from switchos.popworker.evictclient
	int connfd = -1;
	tcpaccept(controller_evictserver_tcpsock, NULL, NULL, connfd, "controller.evictserver");

	// process CACHE_EVICT packet <optype, key, vallen, value, result, seq, serveridx>
	char buf[MAX_BUFSIZE];
	int cur_recv_bytes = -1;
	int8_t optype = -1;
	int32_t vallen = -1;
	int16_t serveridx = -1;
	const int arrive_optype_bytes = sizeof(int8_t);
	const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(key_t) + sizeof(int32_t);
	while (controller_running) {
		int recvsize = 0;
		bool is_broken = tcprecv(connfd, buf + cur_recv_bytes, MAX_BUFSIZE - cur_recv_bytes, 0, recvsize, "controller.evictserver");
		if (is_broken) {
			break;
		}
		
		if (!is_controller_evictserver_evictclients_connected) {
			for (size_t i = 0; i < server_num; i++) {
				tcpconnect(controller_evictserver_evictclient_tcpsock_list[i], server_ip, server_evictserver_port_start+i, "controller.evictserver.evictclient", "server.evictserver");
			}
			is_controller_evictserver_evictclients_connected = true;
		}

		cur_recv_bytes += recvsize;
		if (cur_recv_bytes >= MAX_BUFSIZE) {
			printf("[controller.evictserver] Overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_BUFSIZE);
            exit(-1);
		}

		// Get optype
		if (optype == -1 && cur_recv_bytes >= arrive_optype_bytes) {
			optype = *((int8_t *)buf);
			INVARIANT(packet_type_t(optype) == packet_type_t::CACHE_EVICT);
		}

		// Get vallen
		if (optype != -1 && vallen == -1 && cur_recv_bytes >= arrive_vallen_bytes) {
			//tmpkey.deserialize(buf + arrive_optype_bytes, cur_recv_bytes - arrive_optype_bytes);
			vallen = *((int32_t *)(buf + arrive_vallen_bytes - sizeof(int32_t)));
			vallen = int32_t(ntohl(uint32_t(vallen)));
			INVARIANT(vallen >= 0);
			int padding_size = int(val_t::get_padding_size(vallen)); // padding for value <= 128B
			arrive_serveridx_bytes = arrive_vallen_bytes + vallen + padding_size + sizeof(bool) + sizeof(int32_t) + sizeof(int16_t);
		}

		// Get one complete CACHE_EVICT (only need serveridx here)
		if (optype != -1 && vallen != -1 && cur_recv_bytes >= arrive_serveridx_bytes) {
			//cache_evict_t *tmp_cache_evict_ptr = new cache_evict_t(buf, arrive_serveridx_bytes);

			// send CACHE_EVICT to corresponding server
			serveridx = *((int16_t *)(buf + arrive_serveridx_bytes - sizeof(int16_t)));
			serveridx = int16_t(ntohs(uint16_t(serveridx)));
			INVARIANT(serveridx >= 0 && serveridx < server_num);
			tcpsend(controller_evictserver_evictclient_tcpsock_list[serveridx], buf, arrive_serveridx_bytes, "controller.evictserver.evictclient");

			//delete tmp_cache_evict_ptr;
			//tmp_cache_evict_ptr = NULL;
		}

		// TODO: wait for ACK

		// TODO: reset metadata
	}

}

void close_controller() {
	/*if (controller_cached_keyset_list != NULL) {
		delete [] controller_cached_keyset_list;
		controller_cached_keyset_list = NULL;
	}*/
	if (controller_cache_pop_ptrs != NULL) {
		for (size_t i = 0; i < MQ_SIZE; i++) {
			if (controller_cache_pop_ptrs[i] != NULL) {
				delete controller_cache_pop_ptrs[i];
				controller_cache_pop_ptrs[i] = NULL;
			}
		}
		delete [] controller_cache_pop_ptrs;
		controller_cache_pop_ptrs = NULL;
	}
	if (controller_evictserver_evictclient_tcpsock_list != NULL) {
		delete [] controller_evictserver_evictclient_tcpsock_list;
		controller_evictserver_evictclient_tcpsock_list = NULL;
	}
}
