#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
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

// CPU affinity
#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>

#include "helper.h"
#include "io_helper.h"

#include "common_impl.h"

bool volatile controller_running = false;
std::atomic<size_t> controller_ready_threads(0);
size_t controller_expected_ready_threads = -1;

uint32_t server_total_logical_num_for_controller;

// cache population/eviction

// switchos.popworker.popclient_for_controller <-> controller.popserver
int *controller_popserver_udpsock = NULL;
// controller.popclient <-> server.popserver
int *controller_popserver_popclient_udpsock = NULL;
// TODO: replace with SIGTERM
bool volatile is_controlelr_popserver_finish = false;

// switchos.popworker <-> controller.evictserver
int controller_evictserver_udpsock = -1;
// controller.evictclients <-> servers.evictservers
// NOTE: evictclient.index = serveridx
//bool volatile is_controller_evictserver_evictclients_connected = false;
//int * controller_evictserver_evictclient_tcpsock_list = NULL;
// controller.evictclient <-> server.evictserver
int controller_evictserver_evictclient_udpsock = -1;

void prepare_controller();
void *run_controller_popserver(void *param); // Receive NETCACHE_CACHE_POP from switchos
void *run_controller_evictserver(void *param); // Forward CACHE_EVICT to server and CACHE_EVICT_ACK to switchos in cache eviction
void close_controller();

cpu_set_t nonserverworker_cpuset; // [server_cores, total_cores-1] for all other threads

int main(int argc, char **argv) {
	parse_ini("config.ini");
	parse_control_ini("control_type.ini");

#ifdef SERVER_ROTATION
	server_total_logical_num_for_controller = server_total_logical_num_for_rotation;
#else
	server_total_logical_num_for_controller = server_total_logical_num;
#endif

	int ret = 0;

	// NOTE: now we deploy controller in the same physical machine with servers
	// If we use an individual physical machine, we can comment all the setaffinity statements
	CPU_ZERO(&nonserverworker_cpuset);
	for (int i = server_worker_corenums[0]; i < server_total_corenums[0]; i++) {
		CPU_SET(i, &nonserverworker_cpuset);
	}
	//ret = sched_setaffinity(0, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
	pthread_t main_thread = pthread_self();
	ret = pthread_setaffinity_np(main_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
	if (ret) {
		printf("[Error] fail to set affinity of controller.main; errno: %d\n", errno);
		exit(-1);
	}

	prepare_controller();

	pthread_t popserver_threads[server_total_logical_num_for_controller];
	uint16_t popserver_params[server_total_logical_num_for_controller];
	for (uint16_t tmp_global_server_logical_idx = 0; tmp_global_server_logical_idx < server_total_logical_num_for_controller; tmp_global_server_logical_idx++) {
		popserver_params[tmp_global_server_logical_idx] = tmp_global_server_logical_idx;
		ret = pthread_create(&popserver_threads[tmp_global_server_logical_idx], nullptr, run_controller_popserver, &popserver_params[tmp_global_server_logical_idx]);
		if (ret) {
			COUT_N_EXIT("Error: " << ret);
		}
		ret = pthread_setaffinity_np(popserver_threads[tmp_global_server_logical_idx], sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
		if (ret) {
			printf("Error of setaffinity for controller.popserver; errno: %d\n", errno);
			exit(-1);
		}
	}

	pthread_t evictserver_thread;
	ret = pthread_create(&evictserver_thread, nullptr, run_controller_evictserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}
	ret = pthread_setaffinity_np(evictserver_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
	if (ret) {
		printf("Error of setaffinity for controller.evictserver; errno: %d\n", errno);
		exit(-1);
	}

	while (controller_ready_threads < controller_expected_ready_threads) sleep(1);
	printf("[controller] all threads ready\n");

	controller_running = true;

	// connections from servers
	while (!is_controlelr_popserver_finish) sleep(1);
	printf("[controller] controller.popserver finishes\n");

	controller_running = false;

	void * status;
	for (uint16_t tmp_global_server_logical_idx = 0; tmp_global_server_logical_idx < server_total_logical_num_for_controller; tmp_global_server_logical_idx++) {
		int rc = pthread_join(popserver_threads[tmp_global_server_logical_idx], &status);
		if (rc) {
			COUT_N_EXIT("Error:unable to join popserver " << rc);
		}
	}
	int rc = pthread_join(evictserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}

	free_common();
	close_controller();
	printf("[controller] all threads end\n");
}

void prepare_controller() {
	printf("[controller] prepare start\n");

	controller_running =false;

	controller_expected_ready_threads = server_total_logical_num_for_controller + 2;

	// prepare popserver sockets
	controller_popserver_udpsock_list = new int[server_total_logical_num_for_controller];
	controller_popserver_popclient_udpsock_list = new int[server_total_logical_num_for_controller];
	for (uint16_t tmp_global_server_logical_idx = 0; tmp_global_server_logical_idx < server_total_logical_num_for_controller; tmp_global_server_logical_idx++) {
		prepare_udpserver(controller_popserver_udpsock_list[tmp_global_server_logical_idx], false, controller_popserver_port_start + tmp_global_server_logical_idx, "controller.popserver");
		create_udpsock(controller_popserver_popclient_udpsock_list[tmp_global_server_logical_idx], true, "controller.popserver.popclient");
	}

	//controller_cachedkey_serveridx_map.clear();
	/*controller_cache_pop_ptrs = new cache_pop_t*[MQ_SIZE];
	for (size_t i = 0; i < MQ_SIZE; i++) {
		controller_cache_pop_ptrs[i] = NULL;
	}
	controller_head_for_pop = 0;
	controller_tail_for_pop = 0;*/


	// prepare evictserver
	prepare_udpserver(controller_evictserver_udpsock, false, controller_evictserver_port, "controller.evictserver");

	// prepare evictclients
	/*controller_evictserver_evictclient_tcpsock_list = new int[server_num];
	for (size_t i = 0; i < server_num; i++) {
		create_tcpsock(controller_evictserver_evictclient_tcpsock_list[i], false, "controller.evictserver.evictclient");
	}*/

	// prepare evictclient
	create_udpsock(controller_evictserver_evictclient_udpsock, true, "controller.evictserver.evictclient");

	memory_fence();

	printf("[controller] prepare end\n");
}

void *run_controller_popserver(void *param) {
	// controlelr.popserver i <-> server.popclient i
	uint16_t global_server_logical_idx = *((uint16_t *)param);

	struct sockaddr_in server_popclient_addr;
	socklen_t server_popclient_addrlen = sizeof(struct sockaddr);
	//bool with_server_popclient_addr = false;

	// controller.popserver.popclient i <-> switchos.popserver
	struct sockaddr_in switchos_popserver_addr;
	set_sockaddr(switchos_popserver_addr, inet_addr(switchos_ip), switchos_popserver_port);
	socklen_t switchos_popserver_addrlen = sizeof(struct sockaddr);

	printf("[controller.popserver %d] ready\n", global_server_logical_idx);
	controller_ready_threads++;

	while (!controller_running) {}

	// Process CACHE_POP packet <optype, key, vallen, value, seq, serveridx>
	char buf[MAX_BUFSIZE];
	int recvsize = 0;
	bool is_timeout = false;
	while (controller_running) {
		/*if (!with_server_popclient_addr) {
			udprecvfrom(controller_popserver_udpsock_list[idx], buf, MAX_BUFSIZE, 0, &server_popclient_addr, &server_popclient_addrlen, recvsize, "controller.popserver");
			with_server_popclient_addr = true;
		}
		else {
			udprecvfrom(controller_popserver_udpsock_list[idx], buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.popserver");
		}*/
		udprecvfrom(controller_popserver_udpsock_list[global_server_logical_idx], buf, MAX_BUFSIZE, 0, &server_popclient_addr, &server_popclient_addrlen, recvsize, "controller.popserver");

		//printf("receive CACHE_POP from server and send it to switchos\n");
		//dump_buf(buf, recvsize);
		cache_pop_t tmp_cache_pop(buf, recvsize);

		// send CACHE_POP to switch os
		udpsendto(controller_popserver_popclient_udpsock_list[global_server_logical_idx], buf, recvsize, 0, &switchos_popserver_addr, switchos_popserver_addrlen, "controller.popserver.popclient");
		
		// receive CACHE_POP_ACK from switch os
		bool is_timeout = udprecvfrom(controller_popserver_popclient_udpsock_list[global_server_logical_idx], buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.popserver.popclient");
		if (!is_timeout) {
			// send CACHE_POP_ACK to server.popclient immediately to avoid timeout
			cache_pop_ack_t tmp_cache_pop_ack(buf, recvsize);
			INVARIANT(tmp_cache_pop_ack.key() == tmp_cache_pop.key());
			udpsendto(controller_popserver_udpsock_list[global_server_logical_idx], buf, recvsize, 0, &server_popclient_addr, server_popclient_addrlen, "controller.popserver");
		}

		/*if (controller_cachedkey_serveridx_map.find(tmp_cache_pop->key()) == controller_cachedkey_serveridx_map.end()) {
			controller_cachedkey_serveridx_map.insert(std::pair<netreach_key_t, uint32_t>(tmp_cache_pop_ptr->key(), tmp_cache_pop_ptr->serveridx()));
		}
		else {
			printf("[controller] Receive duplicate key from server %ld!", tmp_cache_pop_ptr->serveridx());
			exit(-1);
		}*/

	}

	is_controlelr_popserver_finish = true;
	close(controller_popserver_udpsock_list[global_server_logical_idx]);
	close(controller_popserver_popclient_udpsock_list[global_server_logical_idx]);
	pthread_exit(nullptr);
}

void *run_controller_evictserver(void *param) {
	struct sockaddr_in switchos_evictclient_addr;
	unsigned int switchos_evictclient_addrlen = sizeof(struct sockaddr);
	//bool with_switchos_evictclient_addr = false;

	struct sockaddr_in server_evictserver_addr;
	memset(&server_evictserver_addr, 0, sizeof(server_evictserver_addr));
	//set_sockaddr(server_evictserver_addr, inet_addr(server_ip_for_controller), server_evictserver_port_start);
	socklen_t server_evictserver_addrlen = sizeof(struct sockaddr_in);

	printf("[controller.evictserver] ready\n");
	controller_ready_threads++;

	while (!controller_running) {}

	// process CACHE_EVICT/_CASE2 packet <optype, key, vallen, value, result, seq, serveridx>
	char buf[MAX_BUFSIZE];
	int recvsize = 0;
	while (controller_running) {
		/*if (!with_switchos_evictclient_addr) {
			udprecvfrom(controller_evictserver_udpsock, buf, MAX_BUFSIZE, 0, &switchos_evictclient_addr, &switchos_evictclient_addrlen, recvsize, "controller.evictserver");
			with_switchos_evictclient_addr = true;
		}
		else {
			udprecvfrom(controller_evictserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.evictserver");
		}*/
		udprecvfrom(controller_evictserver_udpsock, buf, MAX_BUFSIZE, 0, &switchos_evictclient_addr, &switchos_evictclient_addrlen, recvsize, "controller.evictserver");

		// set dstaddr for the corresponding server
		cache_evict_t *tmp_cache_evict_ptr;
		packet_type_t optype = get_packet_type(buf, recvsize);
		if (optype == packet_type_t::CACHE_EVICT) {
			tmp_cache_evict_ptr = new cache_evict_t(buf, recvsize);
		}
		else if (optype == packet_type_t::CACHE_EVICT_CASE2) {
			tmp_cache_evict_ptr = new cache_evict_case2_t(buf, recvsize);
		}
		uint16_t tmp_global_server_logical_idx = tmp_cache_evict_ptr->serveridx();
		INVARIANT(tmp_global_server_logical_idx >= 0 && tmp_global_server_logical_idx < server_total_logical_num_for_controller);
		int tmp_server_physical_idx = -1;
		for (int i = 0; i < server_physical_num; i++) {
			for (int j = 0; j < server_logical_idxes_list[i].size(); j++) {
				if (tmp_global_server_logical_idx == server_logical_idxes_list[i][j]) {
					tmp_server_physical_idx = i;
					break;
				}
			}
		}
		INVARIANT(tmp_server_physical_idx != -1);
		memset(&server_evictserver_addr, 0, sizeof(server_evictserver_addr));
		set_sockaddr(server_evictserver_addr, inet_addr(server_ip_for_controller_list[tmp_server_physical_idx]), server_evictserver_port_start + tmp_global_server_logical_idx);
		delete tmp_cache_evict_ptr;
		tmp_cache_evict_ptr = NULL;
		
		//printf("receive CACHE_EVICT from switchos and send to server\n");
		//dump_buf(buf, recvsize);
		udpsendto(controller_evictserver_evictclient_udpsock, buf, recvsize, 0, &server_evictserver_addr, server_evictserver_addrlen, "controller.evictserver.evictclient");

		// NOTE: timeout-and-retry of CACHE_EVICT is handled by switchos.popworker.evictclient (cover entire eviction workflow)
		bool is_timeout = udprecvfrom(controller_evictserver_evictclient_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.evictserver.evictclient");
		if (!is_timeout) {
			// send CACHE_EVICT_ACK to switchos.popworker.evictclient
			//printf("receive CACHE_EVICT_ACK from server and send to switchos\n");
			//dump_buf(buf, recvsize);
			udpsendto(controller_evictserver_udpsock, buf, recvsize, 0, &switchos_evictclient_addr, switchos_evictclient_addrlen, "controller.evictserver");
		}
	}

	close(controller_evictserver_udpsock);
	close(controller_evictserver_evictclient_udpsock);
	pthread_exit(nullptr);
}

void close_controller() {
	if (controller_popserver_udpsock_list != NULL) {
		delete [] controller_popserver_udpsock_list;
		controller_popserver_udpsock_list = NULL;
	}
	if (controller_popserver_popclient_udpsock_list != NULL) {
		delete [] controller_popserver_popclient_udpsock_list;
		controller_popserver_popclient_udpsock_list = NULL;
	}
}
