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
#include <mutex>

#include "helper.h"
#include "snapshot_helper.h"

#include "common_impl.h"

struct alignas(CACHELINE_SIZE) ControllerPopserverSubthreadParam {
	int connfd;
	uint32_t subthreadidx;
};
typedef ControllerPopserverSubthreadParam controller_popserver_subthread_param_t;

bool volatile controller_running = false;
std::atomic<size_t> controller_ready_threads(0);
//const size_t controller_expected_ready_threads = 4;
const size_t controller_expected_ready_threads = 3;

// Per-server popclient <-> one popserver.subthread in controller
// NOTE: subthreadidx != serveridx
int controller_popserver_tcpsock = -1;
pthread_t *volatile controller_popserver_subthreads = NULL;
std::atomic<size_t> controller_finish_subthreads(0);
size_t controller_expected_finish_subthreads = 0;

// Keep atomicity for the following variables
std::mutex mutex_for_pop;
//std::set<index_key_t> * volatile controller_cached_keyset_list = NULL; // TODO: Comment it after checking server.cached_keyset_list
//std::map<index_key_t, uint16_t> volatile controller_cachedkey_serveridx_map; // TODO: Evict removes the corresponding kv pair
//std::map<uint16_t, uint32_t> volatile controller_serveridx_subthreadidx_map; // Not used
// Message queue between controller.popservers with controller.popclient (connected with switchos.popserver)
MessagePtrQueue<cache_pop_t> controller_cache_pop_ptr_queue(MQ_SIZE);
/*cache_pop_t ** volatile controller_cache_pop_ptrs = NULL;
uint32_t volatile controller_head_for_pop = 0;
uint32_t volatile controller_tail_for_pop = 0;*/

// controller.popclient <-> switchos.popserver
bool volatile is_controller_popclient_connected = false;
int controller_popclient_tcpsock = -1;

// switchos.popworker <-> controller.evictserver
int controller_evictserver_tcpsock = -1;
// controller.evictclients <-> servers.evictservers
// NOTE: evictclient.index = serveridx
//bool volatile is_controller_evictserver_evictclients_connected = false;
//int * controller_evictserver_evictclient_tcpsock_list = NULL;
// controller.evictclient <-> server.evictserver
bool volatile is_controller_evictserver_evictclient_connected = false;
int controller_evictserver_evictclient_tcpsock = -1;

// controller.snapshotclient <-> switchos.snapshotserver
bool volatile is_controller_snapshotclient_connected = false;
int controller_snapshotclient_tcpsock = -1;

// controller.snapshotclient.consnapshotclient <-> server.consnapshotserver
bool volatile is_controller_snapshotclient_consnapshotclient_connected = false;
int controller_snapshotclient_consnapshotclient_tcpsock = -1;

void prepare_controller();
void *run_controller_popserver(void *param); // Accept connections from servers
void *run_controller_popserver_subthread(void *param); // Receive CACHE_POPs from one server
void *run_controller_popclient(void *param); // Send CACHE_POPs to switch os
void *run_controller_evictserver(void *param); // Forward CACHE_EVICT to server and CACHE_EVICT_ACK to switchos in cache eviction
void *run_controller_snapshotclient(void *param); // Periodically notify switch os to launch snapshot
void close_controller();

int main(int argc, char **argv) {
	parse_ini("config.ini");
	parse_control_ini("control_type.ini");

	prepare_controller();

	pthread_t popserver_thread;
	int ret = pthread_create(&popserver_thread, nullptr, run_controller_popserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t popclient_thread;
	ret = pthread_create(&popclient_thread, nullptr, run_controller_popclient, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t evictserver_thread;
	ret = pthread_create(&evictserver_thread, nullptr, run_controller_evictserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	/*pthread_t snapshotclient_thread;
	ret = pthread_create(&snapshotclient_thread, nullptr, run_controller_snapshotclient, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}*/

	while (controller_ready_threads < controller_expected_ready_threads) sleep(1);
	printf("[controller] all threads ready\n");

	controller_running = true;

	// connections from servers
	while (controller_finish_subthreads < controller_expected_finish_subthreads) sleep(1);
	printf("[controlelr] all popserver.subthreads finish\n");

	controller_running = false;

	void * status;
	int rc = pthread_join(popserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(popclient_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(evictserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	/*rc = pthread_join(snapshotclient_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}*/

	printf("[controller] all threads end\n");
	close_controller();
}

void prepare_controller() {
	printf("[controller] prepare start\n");

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

	//controller_cachedkey_serveridx_map.clear();
	//controller_serveridx_subthreadidx_map.clear();
	/*controller_cache_pop_ptrs = new cache_pop_t*[MQ_SIZE];
	for (size_t i = 0; i < MQ_SIZE; i++) {
		controller_cache_pop_ptrs[i] = NULL;
	}
	controller_head_for_pop = 0;
	controller_tail_for_pop = 0;*/

	create_tcpsock(controller_popclient_tcpsock, "controller.popclient");

	// prepare evictserver
	prepare_tcpserver(controller_evictserver_tcpsock, false, controller_evictserver_port, 1, "controller.evictserver"); // MAX_PENDING_CONNECTION = 1 (switchos.popworker.evictclient)

	// prepare evictclients
	/*controller_evictserver_evictclient_tcpsock_list = new int[server_num];
	for (size_t i = 0; i < server_num; i++) {
		create_tcpsock(controller_evictserver_evictclient_tcpsock_list[i], "controller.evictserver.evictclient");
	}*/

	// prepare evictclient
	create_tcpsock(controller_evictserver_evictclient_tcpsock, "controller.evictserver.evictclient");

	// prepare snapshotclient
	create_tcpsock(controller_snapshotclient_tcpsock, "controller.snapshotclient");

	// prepare consnapshotclient
	create_tcpsock(controller_snapshotclient_consnapshotclient_tcpsock, "controller.snapshotclient.consnapshotclient");

	memory_fence();

	printf("[controller] prepare end\n");
}

void *run_controller_popserver(void *param) {
	uint32_t subthreadidx = 0;
	printf("[controller.popserver] ready\n");
	controller_ready_threads++;

	while (!controller_running) {}

	while (controller_running) {
		// Not used
		//struct sockaddr_in server_addr;
		//unsigned int server_addr_len = sizeof(struct sockaddr);

		int connfd = -1;
		bool is_timeout = tcpaccept(controller_popserver_tcpsock, NULL, NULL, connfd, "controller.popserver");
		if (is_timeout) {
			continue; // timeout or interrupted system call
		}
		INVARIANT(connfd != -1);

		// disable timeout for popserver.subthread.connfd
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		int res = setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		INVARIANT(res >= 0);

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

void *run_controller_popserver_subthread(void *param) {
	controller_popserver_subthread_param_t *curparam = (controller_popserver_subthread_param_t *)param;
	int connfd = curparam->connfd;
	//uint32_t subthreadidx = curparam->subthreadidx;

	// Process CACHE_POP packet <optype, key, vallen, value, seq, serveridx>
	char buf[MAX_BUFSIZE];
	int cur_recv_bytes = 0;
	uint8_t optype = 0;
	bool with_optype = false;
	//uint32_t vallen = 0;
	uint16_t vallen = 0;
	bool with_vallen = false;
	bool is_cached_before = false; // TODO: remove
	//index_key_t tmpkey(0, 0, 0, 0);
	const int arrive_optype_bytes = sizeof(uint8_t);
	//const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(index_key_t) + sizeof(uint32_t);
	const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(index_key_t) + sizeof(uint16_t);
	int arrive_serveridx_bytes = -1;
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
		if (!with_optype && cur_recv_bytes >= arrive_optype_bytes) {
			optype = *((uint8_t *)buf);
			INVARIANT(packet_type_t(optype) == packet_type_t::CACHE_POP);
			with_optype = true;
		}

		// Get vallen
		if (with_optype && !with_vallen && cur_recv_bytes >= arrive_vallen_bytes) {
			////tmpkey.deserialize(buf + arrive_optype_bytes, cur_recv_bytes - arrive_optype_bytes);
			//vallen = *((uint32_t *)(buf + arrive_vallen_bytes - sizeof(uint32_t)));
			//vallen = ntohl(vallen);
			vallen = *((uint16_t *)(buf + arrive_vallen_bytes - sizeof(uint16_t)));
			vallen = ntohs(vallen);
			INVARIANT(vallen >= 0);
			int padding_size = int(val_t::get_padding_size(vallen)); // padding for value <= 128B
			arrive_serveridx_bytes = arrive_vallen_bytes + vallen + padding_size + sizeof(uint32_t) + sizeof(uint16_t);
			with_vallen = true;
		}

		// Get one complete CACHE_POP
		if (with_optype && with_vallen && cur_recv_bytes >= arrive_serveridx_bytes) {
			printf("[controller.popserver.subthread] cur_recv_bytes: %d, arrive_serveridx_bytes: %d\n", cur_recv_bytes, arrive_serveridx_bytes); // TMPDEBUG
			printf("receive CACHE_POP from server\n");
			dump_buf(buf, arrive_serveridx_bytes);
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
					controller_serveridx_subthreadidx_map.insert(std::pair<uint16_t, uint32_t>(tmp_cache_pop_ptr->serveridx(), subthreadidx));
				}
				else {
					INVARIANT(controller_serveridx_subthreadidx_map[tmp_cache_pop_ptr->serveridx()] == subthreadidx);
				}*/
				bool res = controller_cache_pop_ptr_queue.write(tmp_cache_pop_ptr);
				if (!res) {
					printf("[controller.popserver] message queue overflow of controller.controller_cache_pop_ptr_queue!");
				}
				/*if ((controller_head_for_pop+1)%MQ_SIZE != controller_tail_for_pop) {
					controller_cache_pop_ptrs[controller_head_for_pop] = tmp_cache_pop_ptr;
					controller_head_for_pop = (controller_head_for_pop + 1) % MQ_SIZE;
				}
				else {
					printf("[controller] message queue overflow of controller.controller_cache_pop_ptrs!");
				}*/
				mutex_for_pop.unlock();
			}

			// Move remaining bytes and reset metadata
			if (cur_recv_bytes > arrive_serveridx_bytes) {
				memcpy(buf, buf + arrive_serveridx_bytes, cur_recv_bytes - arrive_serveridx_bytes);
				cur_recv_bytes = cur_recv_bytes - arrive_serveridx_bytes;
			}
			else {
				cur_recv_bytes = 0;
			}
			optype = 0;
			with_optype = false;
			vallen = 0;
			with_vallen = false;
			arrive_serveridx_bytes = -1;
			is_cached_before = false;
		}
	}

	controller_finish_subthreads++;
	close(connfd);
	pthread_exit(nullptr);
}

void *run_controller_popclient(void *param) {
	printf("[controller.popclient] ready\n");
	controller_ready_threads++;

	while (!controller_running) {}

	while (controller_running) {
		char buf[MAX_BUFSIZE];
		cache_pop_t *tmp_cache_pop_ptr = controller_cache_pop_ptr_queue.read();
		if (tmp_cache_pop_ptr != NULL) {
		//if (controller_tail_for_pop != controller_head_for_pop) {
			if (!is_controller_popclient_connected) {
				tcpconnect(controller_popclient_tcpsock, switchos_ip, switchos_popserver_port, "controller.popoclient", "switchos.popserver");
				is_controller_popclient_connected = true;
			}

			//cache_pop_t *tmp_cache_pop_ptr = controller_cache_pop_ptrs[controller_tail_for_pop];
			// send CACHE_POP to switch os
			uint32_t popsize = tmp_cache_pop_ptr->serialize(buf, MAX_BUFSIZE);
			printf("send CACHE_POP to switchos\n");
			dump_buf(buf, popsize);
			tcpsend(controller_popclient_tcpsock, buf, popsize, "controller.popclient");
			printf("[controller.popclient] popsize: %d\n", int(popsize)); // TMPDEBUG
			// free CACHE_POP
			delete tmp_cache_pop_ptr;
			tmp_cache_pop_ptr = NULL;
			//controller_cache_pop_ptrs[controller_tail_for_pop] = NULL;
			//controller_tail_for_pop = (controller_tail_for_pop + 1) % MQ_SIZE;
		}
	}

	close(controller_popclient_tcpsock);
	pthread_exit(nullptr);
}

void *run_controller_evictserver(void *param) {
	// Not used
	//struct sockaddr_in switchos_addr;
	//unsigned int switchos_addr_len = sizeof(struct sockaddr);

	printf("[controller.evictserver] ready\n");
	controller_ready_threads++;

	while (!controller_running) {}

	// accept connection from switchos.popworker.evictclient
	int connfd = -1;
	tcpaccept(controller_evictserver_tcpsock, NULL, NULL, connfd, "controller.evictserver");

	// process CACHE_EVICT/_CASE2 packet <optype, key, vallen, value, result, seq, serveridx>
	char buf[MAX_BUFSIZE];
	int cur_recv_bytes = 0;
	uint8_t optype = 0;
	bool with_optype = false;
	////index_key_t tmpkey = index_key_t();
	//uint32_t vallen = 0;
	uint16_t vallen = 0;
	bool with_vallen = false;
	//uint16_t tmpserveridx = 0;
	bool is_waitack = false;
	const int arrive_optype_bytes = sizeof(uint8_t);
	//const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(index_key_t) + sizeof(uint32_t);
	const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(index_key_t) + sizeof(uint16_t);
	int arrive_serveridx_bytes = -1;
	char evictclient_buf[MAX_BUFSIZE];
	int evictclient_cur_recv_bytes = 0;
	const int evictclient_arrive_key_bytes = arrive_optype_bytes + sizeof(index_key_t);
	while (controller_running) {
		int recvsize = 0;
		bool is_broken = tcprecv(connfd, buf + cur_recv_bytes, MAX_BUFSIZE - cur_recv_bytes, 0, recvsize, "controller.evictserver");
		if (is_broken) {
			break;
		}
		
		/*if (!is_controller_evictserver_evictclients_connected) {
			for (size_t i = 0; i < server_num; i++) {
				tcpconnect(controller_evictserver_evictclient_tcpsock_list[i], server_ip_for_controlelr, server_evictserver_port_start+i, "controller.evictserver.evictclient", "server.evictserver");
			}
			is_controller_evictserver_evictclients_connected = true;
		}*/

		if (!is_controller_evictserver_evictclient_connected) {
			tcpconnect(controller_evictserver_evictclient_tcpsock, server_ip_for_controller, server_evictserver_port_start, "controller.evictserver.evictclient", "server.evictserver");
			is_controller_evictserver_evictclient_connected = true;
		}

		cur_recv_bytes += recvsize;
		if (cur_recv_bytes >= MAX_BUFSIZE) {
			printf("[controller.evictserver] Overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_BUFSIZE);
            exit(-1);
		}

		// Get optype
		if (!with_optype && cur_recv_bytes >= arrive_optype_bytes) {
			optype = *((uint8_t *)buf);
			INVARIANT(packet_type_t(optype) == packet_type_t::CACHE_EVICT || packet_type_t(optype) == packet_type_t::CACHE_EVICT_CASE2);
			with_optype = true;
		}

		// Get vallen
		if (with_optype && !with_vallen && cur_recv_bytes >= arrive_vallen_bytes) {
			////tmpkey.deserialize(buf + arrive_optype_bytes, cur_recv_bytes - arrive_optype_bytes);
			//vallen = *((uint32_t *)(buf + arrive_vallen_bytes - sizeof(uint32_t)));
			//vallen = ntohl(vallen);
			vallen = *((uint16_t *)(buf + arrive_vallen_bytes - sizeof(uint16_t)));
			vallen = ntohs(vallen);
			INVARIANT(vallen >= 0);
			int padding_size = int(val_t::get_padding_size(vallen)); // padding for value <= 128B
			arrive_serveridx_bytes = arrive_vallen_bytes + vallen + padding_size + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
			with_vallen = true;
		}

		// Get one complete CACHE_EVICT/_CASE2 (only need serveridx here)
		if (with_optype && with_vallen && cur_recv_bytes >= arrive_serveridx_bytes && !is_waitack) {
			//cache_evict_t *tmp_cache_evict_ptr = new cache_evict_t(buf, arrive_serveridx_bytes);

			// send CACHE_EVICT to corresponding server
			//tmpserveridx = *((uint16_t *)(buf + arrive_serveridx_bytes - sizeof(uint16_t)));
			//tmpserveridx = uint16_t(ntohs(uint16_t(tmpserveridx)));
			//INVARIANT(tmpserveridx >= 0 && tmpserveridx < server_num);
			//tcpsend(controller_evictserver_evictclient_tcpsock_list[tmpserveridx], buf, arrive_serveridx_bytes, "controller.evictserver.evictclient");
			
			printf("receive CACHE_EVICT from switchos and send to server\n");
			dump_buf(buf, arrive_serveridx_bytes);
			tcpsend(controller_evictserver_evictclient_tcpsock, buf, arrive_serveridx_bytes, "controller.evictserver.evictclient");

			is_waitack = true;

			//delete tmp_cache_evict_ptr;
			//tmp_cache_evict_ptr = NULL;
		}

		// wait for CACHE_EVICT_ACK from server.evictserver
		if (is_waitack) {
			int evictclient_recvsize = 0;
			//bool evictclient_is_broken = tcprecv(controller_evictserver_evictclient_tcpsock_list[tmpserveridx], evictclient_buf + evictclient_cur_recv_bytes, MAX_BUFSIZE - evictclient_cur_recv_bytes, 0, evictclient_recvsize, "controller.evictserver.evictclient");
			bool evictclient_is_broken = tcprecv(controller_evictserver_evictclient_tcpsock, evictclient_buf + evictclient_cur_recv_bytes, MAX_BUFSIZE - evictclient_cur_recv_bytes, 0, evictclient_recvsize, "controller.evictserver.evictclient");
			if (evictclient_is_broken) {
				break;
			}

			evictclient_cur_recv_bytes += evictclient_recvsize;
			if (evictclient_cur_recv_bytes >= MAX_BUFSIZE) {
				printf("[controller.evictserver.evictclient] Overflow: cur received bytes (%d), maxbufsize (%d)\n", evictclient_cur_recv_bytes, MAX_BUFSIZE);
				exit(-1);
			}

			// get CACHE_EVICT_ACK from server.evictserver
			if (evictclient_cur_recv_bytes >= evictclient_arrive_key_bytes) {
				uint8_t evictclient_optype = *((uint8_t *)evictclient_buf);
				INVARIANT(packet_type_t(evictclient_optype) == packet_type_t::CACHE_EVICT_ACK);

				// TODO: update metadata if any (no metadata now)

				// send CACHE_EVICT_ACK to switchos.popworker.evictclient
				printf("receive CACHE_EVICT_ACK from server and send to switchos\n");
				dump_buf(evictclient_buf, evictclient_arrive_key_bytes);
				tcpsend(connfd, evictclient_buf, evictclient_arrive_key_bytes, "controller.evictserver");

				// move remaining bytes and reset metadata
				if (cur_recv_bytes > arrive_serveridx_bytes) {
					memcpy(buf, buf + arrive_serveridx_bytes, cur_recv_bytes - arrive_serveridx_bytes);
					cur_recv_bytes = cur_recv_bytes - arrive_serveridx_bytes;
				}
				else {
					cur_recv_bytes = 0;
				}
				optype = 0;
				with_optype = false;
				//tmpkey = index_key_t();
				vallen = 0;
				with_vallen = false;
				//tmpserveridx = -1;
				is_waitack = false;
				arrive_serveridx_bytes = -1;
				if (evictclient_cur_recv_bytes > evictclient_arrive_key_bytes) {
					memcpy(evictclient_buf, evictclient_buf + evictclient_arrive_key_bytes, evictclient_cur_recv_bytes - evictclient_arrive_key_bytes);
					evictclient_cur_recv_bytes = evictclient_cur_recv_bytes - evictclient_arrive_key_bytes;
				}
				else {
					evictclient_cur_recv_bytes = 0;
				}
			}
		}
	}

	close(controller_evictserver_tcpsock);
	pthread_exit(nullptr);
}

void *run_controller_snapshotclient(void *param) {
	char *recvbuf = new char[MAX_LARGE_BUFSIZE]; // SNAPSHOT_SERVERSIDE/snapshotdata from switchos
	INVARIANT(recvbuf != NULL);
	memset(recvbuf, 0, MAX_LARGE_BUFSIZE);
	printf("[controller.snapshotclient] ready\n");
	controller_ready_threads++;

	while (!controller_running) {}

	// NOTE: as messages are sent among end-hosts (controller, switchos, and server), we do not perform endian conversion
	uint32_t last_duration = 0; // ms
	uint32_t cur_recv_bytes = 0;
	int phase = 0; // 0: wait for SNAPSHOT_SERVERSIDE; 1: wait for crash-consistent snapshot data
	int control_type_phase0 = -1;
	char ack_recvbuf[MAX_BUFSIZE]; // SNAPSHOT_SERVERSIDE_ACK from server
	int ack_cur_recv_bytes = 0;
	int ack_control_type_phase0 = -1;
	int control_type_phase1 = -1;
	int total_bytes = -1;
	while (controller_running) {
		INVARIANT(controller_snapshot_period > last_duration);
		usleep((controller_snapshot_period - last_duration) * 1000); // ms -> us

		// count time for last_duration
		struct timespec t1, t2, t3;
		CUR_TIME(t1);

		// connect switchos.snapshotserver
		if (!is_controller_snapshotclient_connected) {
			tcpconnect(controller_snapshotclient_tcpsock, switchos_ip, switchos_snapshotserver_port, "controller.snapshotclient", "switchos.snapshotserver");
			is_controller_snapshotclient_connected = true;
		}

		// send SNAPSHOT_START (little-endian) to switchos
		printf("[controller.snapshotclient] send SNAPSHOT_START\n");
		tcpsend(controller_snapshotclient_tcpsock, (char *)&SNAPSHOT_START, sizeof(int), "controlelr.snapshotclient");

		// wait for SNAPSHOT_SERVERSIDE from switchos
		while (true) {
			int recvsize = 0;
			bool is_broken = tcprecv(controller_snapshotclient_tcpsock, recvbuf + cur_recv_bytes, MAX_LARGE_BUFSIZE - cur_recv_bytes, 0, recvsize, "controller.snapshotclient");
			if (is_broken) {
				break;
			}

			cur_recv_bytes += recvsize;
			if (cur_recv_bytes >= MAX_LARGE_BUFSIZE) {
				printf("[controller.snapshotclient] overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_LARGE_BUFSIZE);
				exit(-1);
			}

			// wait for SNAPSHOT_SERVERSIDE from switchos, and send it to server
			if (phase == 0) {
				if (control_type_phase0 == -1 && cur_recv_bytes >= sizeof(int)) {
					control_type_phase0 = *((int *)recvbuf);
					INVARIANT(control_type_phase0 == SNAPSHOT_SERVERSIDE);

					// connect server.consnapshotserver
					if (!is_controller_snapshotclient_consnapshotclient_connected) {
						tcpconnect(controller_snapshotclient_consnapshotclient_tcpsock, server_ip_for_controller, server_consnapshotserver_port, "controller.snapshotclient.consnapshotclient", "server.consnapshotserver");
						is_controller_snapshotclient_consnapshotclient_connected = true;
					}

					// send SNAPSHOT_SERVERSIDE to server for server-side snapshot
					tcpsend(controller_snapshotclient_consnapshotclient_tcpsock, (char *)&SNAPSHOT_SERVERSIDE, sizeof(int), "controller.snapshotclient.consnapshotclient");

					// wait for SNAPSHOT_SERVERSIDE_ACK from server, and send it to switchos
					while (true) {
						if (ack_control_type_phase0 == -1 && ack_cur_recv_bytes < int(sizeof(int))) {
							int tmp_recvsize = 0;
							bool tmp_is_broken = tcprecv(controller_snapshotclient_consnapshotclient_tcpsock, ack_recvbuf + ack_cur_recv_bytes, MAX_BUFSIZE - ack_cur_recv_bytes, 0, tmp_recvsize, "controller.snapshotclient.consnapshotclient");
							if (tmp_is_broken) {
								break;
							}

							ack_cur_recv_bytes += tmp_recvsize;
							if (ack_cur_recv_bytes >= MAX_BUFSIZE) {
								printf("[controller.snapshotclient.consnapshotclient] overflow: cur received bytes (%d), maxbufsize (%d)\n", ack_cur_recv_bytes, MAX_BUFSIZE);
								exit(-1);
							}
						}

						if (ack_control_type_phase0 == -1 && ack_cur_recv_bytes >= int(sizeof(int))) {
							ack_control_type_phase0 = *((int *)ack_recvbuf);
							INVARIANT(ack_control_type_phase0 == SNAPSHOT_SERVERSIDE_ACK);

							// send SNAPSHOT_SERVERSIDE_ACK to switchos
							tcpsend(controller_snapshotclient_tcpsock, (char *)&SNAPSHOT_SERVERSIDE_ACK, sizeof(int), "controller.snapshotclient");
						}

						if (ack_control_type_phase0 != -1) {
							// Move remaining bytes and reset metadata
							if (ack_cur_recv_bytes > int(sizeof(int))) {
								memcpy(ack_recvbuf, ack_recvbuf + sizeof(int), ack_cur_recv_bytes - sizeof(int));
								ack_cur_recv_bytes = ack_cur_recv_bytes - sizeof(int);
							}
							else {
								ack_cur_recv_bytes = 0;
							}
							ack_control_type_phase0 = -1;
							break;
						} // receive a SNAPSHOT_SERVERSIDE_ACK
					} // while (true)

					phase = 1; // wait for crash-consistent snapshot data
				} // receive a SNAPSHOT_SERVERSIDE
			} // phase == 0
			else if (phase == 1) { // wait for crash-consistent snapshot data
				// NOTE: skip sizeof(int) for SNAPSHOT_SERVERSIDE
				if (control_type_phase1 == -1 && cur_recv_bytes >= sizeof(int) + sizeof(int) + sizeof(int32_t)) { // SNAPSHOT_SERVERSIDE + SNAPSHOT_DATA + total_bytes
					control_type_phase1 = *((int *)(recvbuf + sizeof(int)));
					INVARIANT(control_type_phase1 == SNAPSHOT_DATA);
					total_bytes = *((int32_t *)(recvbuf + sizeof(int) + sizeof(int)));
					INVARIANT(total_bytes > 0);
				}

				// snapshot data: <int SNAPSHOT_DATA, int32_t total_bytes, per-server data>
				// per-server data: <int32_t perserver_bytes, uint16_t serveridx, int32_t recordcnt, per-record data>
				//// per-record data: <16B key, uint32_t vallen, value (w/ padding), uint32_t seq, bool stat>
				// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>
				if (control_type_phase1 != -1 && cur_recv_bytes >= sizeof(int) + total_bytes) { // SNAPSHOT_SERVERSIDE + snapshot data of total_bytes
					// NOTE: per-server_bytes is used for sending snapshot data to different server.consnapshotservers (not used now)
					
					// send snapshot data to server.consnapshotserver
					tcpsend(controller_snapshotclient_consnapshotclient_tcpsock, recvbuf + sizeof(int), total_bytes, "controller.snapshotclient.consnapshotclient");

					// Move remaining bytes and reset metadata
					if (cur_recv_bytes > sizeof(int) + total_bytes) {
						memcpy(recvbuf, recvbuf + sizeof(int) + total_bytes, cur_recv_bytes - sizeof(int) - total_bytes);
						cur_recv_bytes = cur_recv_bytes - sizeof(int) - total_bytes;
					}
					else {
						cur_recv_bytes = 0;
					}
					phase = 0;
					control_type_phase0 = -1;
					control_type_phase1 = -1;
					total_bytes = -1;
				}
			}
		} // while (true)
		CUR_TIME(t2);
		DELTA_TIME(t2, t1, t3);
		last_duration = GET_MICROSECOND(t3) / 1000; // us -> ms
	} // while (controller_running)

	delete [] recvbuf;
	recvbuf = NULL;
	close(controller_snapshotclient_tcpsock);
	pthread_exit(nullptr);
}

void close_controller() {
	/*if (controller_cached_keyset_list != NULL) {
		delete [] controller_cached_keyset_list;
		controller_cached_keyset_list = NULL;
	}*/
	/*if (controller_cache_pop_ptrs != NULL) {
		for (size_t i = 0; i < MQ_SIZE; i++) {
			if (controller_cache_pop_ptrs[i] != NULL) {
				delete controller_cache_pop_ptrs[i];
				controller_cache_pop_ptrs[i] = NULL;
			}
		}
		delete [] controller_cache_pop_ptrs;
		controller_cache_pop_ptrs = NULL;
	}*/
	/*if (controller_evictserver_evictclient_tcpsock_list != NULL) {
		delete [] controller_evictserver_evictclient_tcpsock_list;
		controller_evictserver_evictclient_tcpsock_list = NULL;
	}*/
}
