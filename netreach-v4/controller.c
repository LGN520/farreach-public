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

#include "common_impl.h"

typedef Key index_key_t;
typedef Val val_t;

bool volatile controller_running = false;
std::atomic<size_t> controller_ready_threads(0);
const size_t controller_expected_ready_threads = 2;

struct alignas(CACHELINE_SIZE) ControllerPopserverSubthreadParam {
	int connfd;
	uint32_t subthreadidx;
};
typedef ControllerPopserverSubthreadParam controller_popserver_subthread_param_t;

// Per-server popclient <-> one popserver.subthread in controller
int volatile controller_popserver_tcpsock = -1;
pthread_t *volatile controller_popserver_subthreads = NULL;
std::atomic<size_t> controller_finish_subthreads(0);
size_t controller_expected_finish_subthreads = -1;

// Keep atomicity for the following variables
std::mutex mutex_for_pop;
//std::set<index_key_t> * volatile controller_cached_keyset_list = NULL; // TODO: Comment it after checking server.cached_keyset_list
//std::map<index_key_t, uint32_t> volatile controller_cachedkey_serveridx_map; // TODO: Evict removes the corresponding kv pair
std::map<int32_t, uint32_t> volatile controller_serveridx_subthreadidx_map;
// Message queue between controller.popservers with controller.popclient (connected with switchos.popserver)
cache_pop_t ** volatile controller_cache_pop_ptrs = NULL;
uint32_t volatile controller_head_for_pop = 0;
uint32_t volatile controller_tail_for_pop = 0;

// controller.popclient <-> switchos.popserver
int volatile controller_popclient_tcpsock = -1;

// Per-server evictserver <-> one evictclient in controller
int * volatile controller_evictclient_tcpsock_list = NULL;

void prepare_controller();
void *run_controller_popserver(void *param); // Accept connections from servers
void *run_controller_popserver_subthread(void *param); // Receive CACHE_POPs from one server
void *run_controller_popclient(void *param); // Send CACHE_POPs to switch os
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

	// Set popserver socket
	controller_popserver_tcpsock = socket(AF_INET, SOCK_STREAM, 0);
	if (controller_popserver_tcpsock == -1) {
		printf("[controller] Fail to create tcp socket of popserver: errno: %d!\n", errno);
		exit(-1);
	}
	// reuse the occupied port for the last created socket instead of being crashed
	const int trueFlag = 1;
	if (setsockopt(controller_popserver_tcpsock, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
		printf("[controller] Fail to setsockopt of of popserver: errno: %d!\n", errno);
		exit(-1);
	}
	// Disable udp/tcp check
	int disable = 1;
	if (setsockopt(controller_popserver_tcpsock, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		COUT_N_EXIT("[controller] Disable tcp checksum failed");
	}
	// Set timeout for recvfrom/accept of udp/tcp
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec =  0;
	int res = setsockopt(controller_popserver_tcpsock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	INVARIANT(res >= 0);
	// Set listen address
	sockaddr_in listen_addr;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(controller_popserver_port);
	if ((bind(controller_popserver_tcpsock, (struct sockaddr*)&listen_addr, sizeof(listen_addr))) != 0) {
		printf("[controller] Fail to bind socket on port %hu for popserver, errno: %d!\n", controller_popserver_port, errno);
		exit(-1);
	}
	if ((listen(controller_popserver_tcpsock, server_num)) != 0) { // MAX_PENDING_CONNECTION = server num
		printf("[controller] Fail to listen on port %hu for popserver, errno: %d!\n", controller_popserver_port, errno);
		exit(-1);
	}

	/*controller_cached_keyset_list = new std::set<index_key_t>[server_num];
	for (size_t i = 0; i < server_num; i++) {
		controller_cached_keyset_list[i].clear();
	}*/

	controller_cache_pop_ptrs = new cache_pop_t*[MQ_SIZE];
	for (size_t i = 0; i < MQ_SIZE; i++) {
		controller_cache_pop_ptrs[i] = NULL;
	}

	//controller_cachedkey_serveridx_map.clear();
	controller_serveridx_subthreadidx_map.clear();
	controller_head_for_pop = 0;
	controller_tail_for_pop = 0;
	controller_popclient_tcpsock = socket(AF_INET, SOCK_STREAM, 0);
}

void *run_controller_popserver(void *param) {
	uint32_t subthreadidx = 0;
	controller_ready_threads++;

	while (!controller_running) {}

	while (controller_running) {
		// Not used
		struct sockaddr_in server_addr;
		unsigned int server_addr_len = sizeof(struct sockaddr);

		int connfd = accept(controller_popserver_tcpsock, NULL, NULL);
		if (connfd == -1) {
			if (errno == EWOULDBLOCK || errno == EINTR) {
				continue; // timeout or interrupted system call
			}
			else {
				COUT_N_EXIT("[controller] Error of accept: errno = " << std::string(strerror(errno)));
			}
			COUT_N_EXIT("[controller] Error of accept: errno = " << std::string(strerror(errno)));
		}

		// NOTE: subthreadidx != serveridx
		controller_popserver_subthread_param_t param;
		param.connfd = connfd;
		param.subthreadidx = subthreadidx;
		int ret = pthread_create(&controller_popserver_subthreads[subthreadidx], nullptr, run_controller_popserver_subthread, (void *)&param);
		if (ret) {
			COUT_N_EXIT("Error: unable to create controller.popserver.subthread " << ret);
		}
		controller_popserver_subthreads_runnings[subthreadidx] = true;
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
		int tmpsize = recv(connfd, buf + cur_recv_bytes, MAX_BUFSIZE - cur_recv_bytes, 0);
		if (tmpsize < 0) {
			// Errno 32 means broken pipe, i.e., server.popclient is closed
			if (errno == 32) {
				printf("[controller] remote server.popclient %ld is closed", thread_id);
				break;
			}
			else {
				COUT_N_EXIT("[controller] popserver recv fails: " << tmpsize << " errno = " << std::string(strerror(errno)));
			}
		}
		cur_recv_bytes += tmpsize;
		if (cur_recv_bytes >= MAX_BUFSIZE) {
			printf("[controller] Overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_BUFSIZE);
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
			INVARIANT(vallen >= 0);
			int padding_size = int(val_t::get_padding_size(vallen)); // padding for value <= 128B
			arrive_serveridx_bytes = arrive_vallen_bytes + vallen + padding_size + sizeof(bool) + sizeof(int32_t) + sizeof(int16_t);
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
				if (controller_serveridx_subthreadidx_map.find(tmp_cache_pop_ptr->serveridx()) == controller_serveridx_subthreadidx_map.end()) {
					controller_serveridx_subthreadidx_map.insert(std::pair<int32_t, uint32_t>(tmp_cache_pop_ptr->serveridx(), subthreadidx));
				}
				else {
					INVARIANT(controller_serveridx_subthreadidx_map[tmp_cache_pop_ptr->serveridx()] == subthreadidx);
				}
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
	sockaddr_in switchos_addr;
	memset(&switchos_addr, 0, sizeof(switchos_addr));
	switchos_addr.sin_family = AF_INET;
	switchos_addr.sin_addr.s_addr = inet_addr(switchos_ip);
	switchos_addr.sin_port = htons(switchos_popserver_port);
	if (connect(controller_popclient_tcpsock, (struct sockaddr*)&switchos_addr, sizeof(switchos_addr)) != 0) {
		error("Fail to connect switchos.popserver at %s:%hu, errno: %d!\n", switchos_ip, switchos_popserver_port, errno);
	}
	controller_ready_threads++;

	while (!controller_running) {}

	while (controller_running) {
		char buf[MAX_BUFSIZE];
		if (controller_tail_for_pop != controller_head_for_pop) {
			cache_pop_t *tmp_cache_pop_ptr = controller_cache_pop_ptrs[controller_tail_for_pop];
			// send CACHE_POP to switch os
			uint32_t popsize = tmp_cache_pop_ptr->serialize(buf, MAX_BUFSIZE);
			int send_size = popsize;
			const char *ptr = buf;
			while (send_size > 0) {
				int tmpsize = send(controller_popclient_tcpsock, ptr, send_size, 0);
				if (tmpsize < 0) {
					// Errno 32 means broken pipe, i.e., remote TCP connection is closed
					printf("TCP send returns %d, errno: %d\n", tmpsize, errno);
					break;
				}
				ptr += tmpsize;
				send_size -= tmpsize;
			}
			// free CACHE_POP
			delete tmp_cache_pop_ptr;
			tmp_cache_pop_ptr = NULL;
			controller_cache_pop_ptrs[controller_tail_for_pop] = NULL;
			controller_tail_for_pop = (controller_tail_for_pop + 1) % MQ_SIZE;
		}
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
	if (controller_evictclient_tcpsock_list != NULL) {
		delete [] controller_evictclient_tcpsock_list;
		controller_evictclient_tcpsock_list = NULL;
	}
}
