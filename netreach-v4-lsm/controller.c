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

struct SnapshotclientSubthreadParam {
	int udpsock;
	struct sockaddr_in dstaddr;
	socklen_t dstaddrlen;
	uint16_t serveridx;
};
typedef SnapshotclientSubthreadParam snapshotclient_subthread_param_t;

bool volatile controller_running = false;
std::atomic<size_t> controller_ready_threads(0);
size_t controller_expected_ready_threads = -1;

// cache population/eviction

// server.popclient <-> controller.popserver
int *controller_popserver_udpsock_list = NULL;
// controller.popclient <-> switchos.popserver
int *controller_popserver_popclient_udpsock_list = NULL;
// TODO: replace with SIGTERM
std::atomic<size_t> controller_popserver_finish_threads(0);
size_t controller_expected_popserver_finish_threads = -1;

// switchos.popworker <-> controller.evictserver
int controller_evictserver_udpsock = -1;
// controller.evictclients <-> servers.evictservers
// NOTE: evictclient.index = serveridx
//bool volatile is_controller_evictserver_evictclients_connected = false;
//int * controller_evictserver_evictclient_tcpsock_list = NULL;
// controller.evictclient <-> server.evictserver
int controller_evictserver_evictclient_udpsock = -1;

// snapshot

int controller_snapshotid = 1; // server uses snapshot id 0 after loading phase

// controller.snapshotclient <-> switchos/per-server.snapshotserver
int controller_snapshotclient_for_switchos_udpsock = -1;
int *controller_snapshotclient_for_server_udpsock_list = NULL;
// written by controller.snapshotclient; read by controller.snapshotclient.senddata_subthread to server.snapshotdataserver
dynamic_array_t *controller_snapshotclient_for_server_databuf_list = NULL;

void prepare_controller();
void *run_controller_popserver(void *param); // Receive CACHE_POPs from each server
void *run_controller_evictserver(void *param); // Forward CACHE_EVICT to server and CACHE_EVICT_ACK to switchos in cache eviction
void controller_load_snapshotid(); // retrieve latest snapshot id
void controller_update_snapshotid(); // store latest snapshotid and inswitch snapshot data
void *run_controller_snapshotclient(void *param); // Periodically notify switch os to launch snapshot
void *run_controller_snapshotclient_cleanup_subthread(void *param);
void *run_controller_snapshotclient_start_subthread(void *param);
void *run_controller_snapshotclient_senddata_subthread(void *param);
void close_controller();

cpu_set_t nonserverworker_cpuset; // [server_cores, total_cores-1] for all other threads

int main(int argc, char **argv) {
	parse_ini("config.ini");
	parse_control_ini("control_type.ini");

	int ret = 0;

	// NOTE: now we deploy controller in the same physical machine with servers
	// If we use an individual physical machine, we can comment all the setaffinity statements
	CPU_ZERO(&nonserverworker_cpuset);
	for (int i = server_cores; i < total_cores; i++) {
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

	pthread_t popserver_threads[server_num];
	uint16_t popserver_params[server_num];
	for (uint16_t i = 0; i < server_num; i++) {
		popserver_params[i] = i;
		ret = pthread_create(&popserver_threads[i], nullptr, run_controller_popserver, &popserver_params[i]);
		if (ret) {
			COUT_N_EXIT("Error: " << ret);
		}
		ret = pthread_setaffinity_np(popserver_threads[i], sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
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

	pthread_t snapshotclient_thread;
	ret = pthread_create(&snapshotclient_thread, nullptr, run_controller_snapshotclient, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}
	// NOTE: subthreads created by controller.snapshotclient via pthread_create will inherit the CPU affinity mask of snapshotclient_thread
	ret = pthread_setaffinity_np(snapshotclient_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
	if (ret) {
		printf("Error of setaffinity for controller.snapshotclient; errno: %d\n", errno);
		exit(-1);
	}

	while (controller_ready_threads < controller_expected_ready_threads) sleep(1);
	printf("[controller] all threads ready\n");

	controller_running = true;

	// connections from servers
	while (controller_popserver_finish_threads < controller_expected_popserver_finish_threads) sleep(1);
	printf("[controller] all popservers finish\n");

	controller_running = false;

	void * status;
	for (size_t i = 0; i < server_num; i++) {
		int rc = pthread_join(popserver_threads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error:unable to join popserver " << rc);
		}
	}
	int rc = pthread_join(evictserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(snapshotclient_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}

	printf("[controller] all threads end\n");
	close_controller();
}

void prepare_controller() {
	printf("[controller] prepare start\n");

	controller_running =false;

	controller_expected_ready_threads = server_num + 2;
	controller_expected_popserver_finish_threads = server_num;

	// prepare popserver sockets
	controller_popserver_udpsock_list = new int[server_num];
	controller_popserver_popclient_udpsock_list = new int[server_num];
	for (uint16_t i = 0; i < server_num; i++) {
		prepare_udpserver(controller_popserver_udpsock_list[i], false, controller_popserver_port_start + i, "controller.popserver");
		create_udpsock(controller_popserver_popclient_udpsock_list[i], true, "controller.popserver.popclient");
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

	// load latest snapshotid
	controller_load_snapshotid();

	// prepare snapshotclient
	create_udpsock(controller_snapshotclient_for_switchos_udpsock, true, "controller.snapshotclient_for_switchos", SOCKET_TIMEOUT, 0, UDP_LARGE_RCVBUFSIZE);
	controller_snapshotclient_for_server_udpsock_list = new int[server_num];
	controller_snapshotclient_for_server_databuf_list = new dynamic_array_t[server_num];
	for (size_t i = 0; i < server_num; i++) {
		create_udpsock(controller_snapshotclient_for_server_udpsock_list[i], true, "controller.snapshotclient_for_server");
		controller_snapshotclient_for_server_databuf_list[i].init(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
	}

	memory_fence();

	printf("[controller] prepare end\n");
}

void controller_load_snapshotid() {
	std::string snapshotid_path;
	get_controller_snapshotid_path(snapshotid_path);
	if (isexist(snapshotid_path)) {
		load_snapshotid(controller_snapshotid, snapshotid_path);
		controller_snapshotid += 1;
	}
	else {
		controller_snapshotid = 1;
	}
	return;
}

void controller_update_snapshotid(char *buf, int bufsize) {
	// TODO: store inswitch snapshot data for switch failure
	std::string snapshotdata_path;
	get_controller_snapshotdata_path(snapshotdata_path, controller_snapshotid);
	store_buf(buf, bufsize, snapshotdata_path);
	// store latest snapshot id for controller failure 
	std::string snapshotid_path;
	get_controller_snapshotid_path(snapshotid_path);
	store_snapshotid(controller_snapshotid, snapshotid_path);
	// remove old-enough snapshot data
	int old_snapshotid = controller_snapshotid - 1;
	if (old_snapshotid > 0) {
		std::string old_snapshotdata_path;
		get_controller_snapshotdata_path(old_snapshotdata_path, old_snapshotid);
		rmfiles(old_snapshotdata_path.c_str());
	}

	controller_snapshotid += 1;
}

void *run_controller_popserver(void *param) {
	// controlelr.popserver i <-> server.popclient i
	uint16_t idx = *((uint16_t *)param);
	struct sockaddr_in server_popclient_addr;
	socklen_t server_popclient_addrlen = sizeof(struct sockaddr);
	//bool with_server_popclient_addr = false;

	// controller.popserver.popclient i <-> switchos.popserver
	struct sockaddr_in switchos_popserver_addr;
	set_sockaddr(switchos_popserver_addr, inet_addr(switchos_ip), switchos_popserver_port);
	socklen_t switchos_popserver_addrlen = sizeof(struct sockaddr);

	printf("[controller.popserver %d] ready\n", idx);
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
		udprecvfrom(controller_popserver_udpsock_list[idx], buf, MAX_BUFSIZE, 0, &server_popclient_addr, &server_popclient_addrlen, recvsize, "controller.popserver");

		//printf("receive CACHE_POP from server and send it to switchos\n");
		//dump_buf(buf, recvsize);
		cache_pop_t tmp_cache_pop(buf, recvsize);

		// send CACHE_POP to switch os
		udpsendto(controller_popserver_popclient_udpsock_list[idx], buf, recvsize, 0, &switchos_popserver_addr, switchos_popserver_addrlen, "controller.popserver.popclient");
		
		// receive CACHE_POP_ACK from switch os
		bool is_timeout = udprecvfrom(controller_popserver_popclient_udpsock_list[idx], buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.popserver.popclient");
		if (!is_timeout) {
			// send CACHE_POP_ACK to server.popclient immediately to avoid timeout
			cache_pop_ack_t tmp_cache_pop_ack(buf, recvsize);
			INVARIANT(tmp_cache_pop_ack.key() == tmp_cache_pop.key());
			udpsendto(controller_popserver_udpsock_list[idx], buf, recvsize, 0, &server_popclient_addr, server_popclient_addrlen, "controller.popserver");
		}

		/*if (controller_cachedkey_serveridx_map.find(tmp_cache_pop->key()) == controller_cachedkey_serveridx_map.end()) {
			controller_cachedkey_serveridx_map.insert(std::pair<netreach_key_t, uint32_t>(tmp_cache_pop_ptr->key(), tmp_cache_pop_ptr->serveridx()));
		}
		else {
			printf("[controller] Receive duplicate key from server %ld!", tmp_cache_pop_ptr->serveridx());
			exit(-1);
		}*/

	}

	controller_popserver_finish_threads++;
	close(controller_popserver_udpsock_list[idx]);
	close(controller_popserver_popclient_udpsock_list[idx]);
	pthread_exit(nullptr);
}

void *run_controller_evictserver(void *param) {
	struct sockaddr_in switchos_evictclient_addr;
	unsigned int switchos_evictclient_addrlen = sizeof(struct sockaddr);
	//bool with_switchos_evictclient_addr = false;

	struct sockaddr_in server_evictserver_addr;
	set_sockaddr(server_evictserver_addr, inet_addr(server_ip_for_controller), server_evictserver_port_start);
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

		// send CACHE_EVICT to corresponding server
		//tmpserveridx = *((uint16_t *)(buf + arrive_serveridx_bytes - sizeof(uint16_t)));
		//tmpserveridx = uint16_t(ntohs(uint16_t(tmpserveridx)));
		//INVARIANT(tmpserveridx >= 0 && tmpserveridx < server_num);
		//tcpsend(controller_evictserver_evictclient_tcpsock_list[tmpserveridx], buf, arrive_serveridx_bytes, "controller.evictserver.evictclient");
		
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

void *run_controller_snapshotclient(void *param) {
	struct sockaddr_in switchos_snapshotserver_addr;
	set_sockaddr(switchos_snapshotserver_addr, inet_addr(switchos_ip), switchos_snapshotserver_port);
	socklen_t switchos_snapshotserver_addrlen = sizeof(struct sockaddr_in);

	struct sockaddr_in server_snapshotserver_addr_list[server_num];
	socklen_t server_snapshotserver_addrlen_list[server_num];
	for (size_t i = 0; i < server_num; i++) {
		set_sockaddr(server_snapshotserver_addr_list[i], inet_addr(server_ip_for_controller), server_snapshotserver_port_start + i);
		server_snapshotserver_addrlen_list[i] = sizeof(struct sockaddr_in);
	}

	struct sockaddr_in server_snapshotdataserver_addr_list[server_num];
	socklen_t server_snapshotdataserver_addrlen_list[server_num];
	for (size_t i = 0; i < server_num; i++) {
		set_sockaddr(server_snapshotdataserver_addr_list[i], inet_addr(server_ip_for_controller), server_snapshotdataserver_port_start + i);
		server_snapshotdataserver_addrlen_list[i] = sizeof(struct sockaddr_in);
	}

	// prepare for concurrent SNAPSHOT_CLEANUP (param.serveridx not used)
	pthread_t cleanup_subthread_for_switchos;
	snapshotclient_subthread_param_t cleanup_subthread_param_for_switchos;
	cleanup_subthread_param_for_switchos.udpsock = controller_snapshotclient_for_switchos_udpsock;
	cleanup_subthread_param_for_switchos.dstaddr = switchos_snapshotserver_addr;
	cleanup_subthread_param_for_switchos.dstaddrlen = switchos_snapshotserver_addrlen;
	pthread_t cleanup_subthread_for_server_list[server_num];
	snapshotclient_subthread_param_t cleanup_subthread_param_for_server_list[server_num];
	for (size_t i = 0; i < server_num; i++) {
		cleanup_subthread_param_for_server_list[i].udpsock = controller_snapshotclient_for_server_udpsock_list[i];
		cleanup_subthread_param_for_server_list[i].dstaddr = server_snapshotserver_addr_list[i];
		cleanup_subthread_param_for_server_list[i].dstaddrlen = server_snapshotserver_addrlen_list[i];
		cleanup_subthread_param_for_server_list[i].serveridx = i;
	}

	// prepare for concurrent SNAPSHOT_START (param.serveridx not used)
	pthread_t start_subthread_for_switchos;
	snapshotclient_subthread_param_t start_subthread_param_for_switchos;
	start_subthread_param_for_switchos.udpsock = controller_snapshotclient_for_switchos_udpsock;
	start_subthread_param_for_switchos.dstaddr = switchos_snapshotserver_addr;
	start_subthread_param_for_switchos.dstaddrlen = switchos_snapshotserver_addrlen;
	pthread_t start_subthread_for_server_list[server_num];
	snapshotclient_subthread_param_t start_subthread_param_for_server_list[server_num];
	for (size_t i = 0; i < server_num; i++) {
		start_subthread_param_for_server_list[i].udpsock = controller_snapshotclient_for_server_udpsock_list[i];
		start_subthread_param_for_server_list[i].dstaddr = server_snapshotserver_addr_list[i];
		start_subthread_param_for_server_list[i].dstaddrlen = server_snapshotserver_addrlen_list[i];
		start_subthread_param_for_server_list[i].serveridx = i;
	}

	// prepare for concurrent SNAPSHOT_SENDDATA
	pthread_t senddata_subthread_for_server_list[server_num];
	snapshotclient_subthread_param_t senddata_subthread_param_for_server_list[server_num];
	for (size_t i = 0; i < server_num; i++) {
		senddata_subthread_param_for_server_list[i].udpsock = controller_snapshotclient_for_server_udpsock_list[i];
		senddata_subthread_param_for_server_list[i].dstaddr = server_snapshotdataserver_addr_list[i];
		senddata_subthread_param_for_server_list[i].dstaddrlen = server_snapshotdataserver_addrlen_list[i];
		senddata_subthread_param_for_server_list[i].serveridx = i;
	}

	// prepare for SNAPSHOT_GETDATA_ACK
	dynamic_array_t databuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);

	printf("[controller.snapshotclient] ready\n");
	controller_ready_threads++;

	while (!controller_running) {}

	char sendbuf[MAX_BUFSIZE];
	char recvbuf[MAX_BUFSIZE];
	int recvsize = 0;
	bool is_timeout = false;
	struct timespec snapshot_t1, snapshot_t2, snapshot_t3;
	while (controller_running) {
		usleep(controller_snapshot_period * 1000); // ms -> us

		// TMPDEBUG
		printf("Type to send SNAPSHOT_START...\n");
		getchar();

		CUR_TIME(snapshot_t1);

		// (1) send SNAPSHOT_CLEANUP to each switchos and server concurrently
		printf("[controller.snapshotclient] send SNAPSHOT_CLEANUPs to each switchos and server\n");
		pthread_create(&cleanup_subthread_for_switchos, nullptr, run_controller_snapshotclient_cleanup_subthread, &cleanup_subthread_param_for_switchos);
		for (size_t i = 0; i < server_num; i++) {
			pthread_create(&cleanup_subthread_for_server_list[i], nullptr, run_controller_snapshotclient_cleanup_subthread, &cleanup_subthread_param_for_server_list[i]);
		}
		pthread_join(cleanup_subthread_for_switchos, NULL);
		for (size_t i = 0; i < server_num; i++) {
			pthread_join(cleanup_subthread_for_server_list[i], NULL);
		}

		// (2) send SNAPSHOT_PREPARE to each switch os to enable single path for snapshot atomicity
		printf("[controller.snapshotclient] send SNAPSHOT_PREPARE to each switchos\n");
		memcpy(sendbuf, &SNAPSHOT_PREPARE, sizeof(int));
		memcpy(sendbuf + sizeof(int), &controller_snapshotid, sizeof(int));
		while (true) {
			udpsendto(controller_snapshotclient_for_switchos_udpsock, sendbuf, 2*sizeof(int), 0, &switchos_snapshotserver_addr, switchos_snapshotserver_addrlen, "controller.snapshotclient");

			is_timeout = udprecvfrom(controller_snapshotclient_for_switchos_udpsock, recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotclient");
			if (is_timeout) {
				continue;
			}
			else {
				INVARIANT(recvsize == sizeof(int) && *((int *)recvbuf) == SNAPSHOT_PREPARE_ACK);
				break;
			}
		}

#ifdef DEBUG_SNAPSHOT
		// TMPDEBUG
		printf("Type to set snapshot flag...\n");
		getchar();
#endif

		// (3) send SNAPSHOT_SETFLAG to each switch os to set snapshot flag
		printf("[controller.snapshotclient] send SNAPSHOT_SETFLAG to each switchos\n");
		memcpy(sendbuf, &SNAPSHOT_SETFLAG, sizeof(int));
		memcpy(sendbuf + sizeof(int), &controller_snapshotid, sizeof(int));
		while (true) {
			udpsendto(controller_snapshotclient_for_switchos_udpsock, sendbuf, 2*sizeof(int), 0, &switchos_snapshotserver_addr, switchos_snapshotserver_addrlen, "controller.snapshotclient");

			is_timeout = udprecvfrom(controller_snapshotclient_for_switchos_udpsock, recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotclient");
			if (is_timeout) {
				continue;
			}
			else {
				INVARIANT(recvsize == sizeof(int) && *((int *)recvbuf) == SNAPSHOT_SETFLAG_ACK);
				break;
			}
		}

		// (4) send SNAPSHOT_START to each switchos and server concurrently
		printf("[controller.snapshotclient] send SNAPSHOT_STARTs to each switchos and server\n");
		pthread_create(&start_subthread_for_switchos, nullptr, run_controller_snapshotclient_start_subthread, &start_subthread_param_for_switchos);
		for (size_t i = 0; i < server_num; i++) {
			pthread_create(&start_subthread_for_server_list[i], nullptr, run_controller_snapshotclient_start_subthread, &start_subthread_param_for_server_list[i]);
		}
		pthread_join(start_subthread_for_switchos, NULL);
		for (size_t i = 0; i < server_num; i++) {
			pthread_join(start_subthread_for_server_list[i], NULL);
		}

		// prepare to process per-server snapshot data
		// per-server snapshot data: <int SNAPSHOT_SENDDATA, int snapshotid, int32_t perserver_bytes, uint16_t serveridx, int32_t record_cnt, per-record data>
		// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>
		for (uint16_t tmpi = 0; tmpi < server_num; tmpi++) {
			controller_snapshotclient_for_server_databuf_list[tmpi].clear();
		}
		int32_t snapshot_senddata_default_perserverbytes = sizeof(int) + sizeof(int) + sizeof(int32_t) + sizeof(uint16_t) + sizeof(int32_t);
		int32_t snapshot_senddata_default_recordcnt = 0;
		for (uint16_t tmpi = 0; tmpi < server_num; tmpi++) {
			// prepare header for SNAPSHOT_SENDDATA
			controller_snapshotclient_for_server_databuf_list[tmpi].dynamic_memcpy(0, (char *)&SNAPSHOT_SENDDATA, sizeof(int));
			controller_snapshotclient_for_server_databuf_list[tmpi].dynamic_memcpy(0 + sizeof(int), (char *)&controller_snapshotid, sizeof(int));
			controller_snapshotclient_for_server_databuf_list[tmpi].dynamic_memcpy(0 + sizeof(int) + sizeof(int), (char *)&snapshot_senddata_default_perserverbytes, sizeof(int32_t));
			controller_snapshotclient_for_server_databuf_list[tmpi].dynamic_memcpy(0 + sizeof(int) + sizeof(int) + sizeof(int32_t), (char *)&tmpi, sizeof(uint16_t));
			controller_snapshotclient_for_server_databuf_list[tmpi].dynamic_memcpy(0 + sizeof(int) + sizeof(int) + sizeof(int32_t) + sizeof(uint16_t), (char *)&snapshot_senddata_default_recordcnt, sizeof(int32_t));
			INVARIANT(controller_snapshotclient_for_server_databuf_list[tmpi].size() == snapshot_senddata_default_perserverbytes);
		}

		// (5) send SNAPSHOT_GETDATA to each switch os to get consistent snapshot data
		printf("[controller.snapshotclient] send SNAPSHOT_GETDATA to each switchos\n");
		memcpy(sendbuf, &SNAPSHOT_GETDATA, sizeof(int));
		memcpy(sendbuf + sizeof(int), &controller_snapshotid, sizeof(int));
		while (true) {
			udpsendto(controller_snapshotclient_for_switchos_udpsock, sendbuf, 2*sizeof(int), 0, &switchos_snapshotserver_addr, switchos_snapshotserver_addrlen, "controller.snapshotclient");

			databuf.clear();
			is_timeout = udprecvlarge_udpfrag(controller_snapshotclient_for_switchos_udpsock, databuf, 0, NULL, NULL, "controller.snapshotclient");
			if (is_timeout) {
				continue;
			}
			else {
				// snapshot data: <int SNAPSHOT_GETDATA_ACK, int32_t total_bytes (including SNAPSHOT_GETDATA_ACK), per-server data>
				// per-server data: <int32_t perserver_bytes, uint16_t serveridx, int32_t recordcnt, per-record data>
				// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>
				
				INVARIANT(*((int *)databuf.array()) == SNAPSHOT_GETDATA_ACK);
				int32_t total_bytes = *((int32_t *)(databuf.array() + sizeof(int)));
				INVARIANT(databuf.size() == total_bytes);
				
				// per-server snapshot data: <int SNAPSHOT_SENDDATA, int snapshotid, int32_t perserver_bytes (including SNAPSHOT_SENDDATA), uint16_t serveridx, int32_t record_cnt, per-record data>
				// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>
				int tmp_offset = sizeof(int) + sizeof(int32_t); // SNAPSHOT_GETDATA_ACK + total_bytes
				while (tmp_offset < total_bytes) {
					int32_t tmp_serverbytes = *((int32_t *)(databuf.array() + tmp_offset));
					int32_t effective_serverbytes = tmp_serverbytes - sizeof(int32_t) - sizeof(uint16_t) - sizeof(int32_t);
					uint16_t tmp_serveridx = *((uint16_t *)(databuf.array() + tmp_offset + sizeof(int32_t)));
					int32_t tmp_recordcnt = *((int32_t *)(databuf.array() + tmp_offset + sizeof(int32_t) + sizeof(uint16_t)));

					// increase perserver_bytes and record_cnt in SNAPSHOT_SENDDATA header
					int32_t old_serverbytes = *((int *)(controller_snapshotclient_for_server_databuf_list[tmp_serveridx].array() + sizeof(int) + sizeof(int)));
					int32_t old_recordcnt = *((int *)(controller_snapshotclient_for_server_databuf_list[tmp_serveridx].array() + sizeof(int) + sizeof(int) + sizeof(int32_t) + sizeof(uint16_t)));
					int32_t new_serverbytes = old_serverbytes + effective_serverbytes;
					int32_t new_recordcnt = old_recordcnt + tmp_recordcnt;
					controller_snapshotclient_for_server_databuf_list[tmp_serveridx].dynamic_memcpy(0 + sizeof(int) + sizeof(int), (char *)&new_serverbytes, sizeof(int32_t));
					controller_snapshotclient_for_server_databuf_list[tmp_serveridx].dynamic_memcpy(0 + sizeof(int) + sizeof(int) + sizeof(int32_t) + sizeof(uint16_t), (char *)&new_recordcnt, sizeof(int32_t));
					
					// copy per-record data in SNAPSHOT_SENDDATA body
					int tmp_databuflen = controller_snapshotclient_for_server_databuf_list[tmp_serveridx].size();
					controller_snapshotclient_for_server_databuf_list[tmp_serveridx].dynamic_memcpy(0 + tmp_databuflen, databuf.array() + tmp_offset + sizeof(int32_t) + sizeof(uint16_t) + sizeof(int32_t), effective_serverbytes);

					tmp_offset += tmp_serverbytes;
				}
				break;
			}
		}

		// (6) send SNAPSHOT_SENDDATA to each server concurrently
		printf("[controller.snapshotclient] send SNAPSHOT_SENDDATAs to each server\n");
		for (size_t i = 0; i < server_num; i++) {
			// NOTE: even if databuf_list[i].size() == default_perserverbytes, we still need to notify the server that snapshot is end by SNAPSHOT_SENDDATA
			pthread_create(&senddata_subthread_for_server_list[i], nullptr, run_controller_snapshotclient_senddata_subthread, &senddata_subthread_param_for_server_list[i]);
		}
		for (size_t i = 0; i < server_num; i++) {
			pthread_join(senddata_subthread_for_server_list[i], NULL);
		}

		CUR_TIME(snapshot_t2);
		DELTA_TIME(snapshot_t2, snapshot_t1, snapshot_t3);
		printf("Time of making consistent system snapshot: %f s\n", GET_MICROSECOND(snapshot_t3) / 1000.0 / 1000.0);
		
		// (7) save per-switch SNAPSHOT_GETDATA_ACK (databuf) for controller failure recovery
		controller_update_snapshotid(databuf.array(), databuf.size());
	}

	close(controller_snapshotclient_for_switchos_udpsock);
	for (size_t i = 0; i < server_num; i++) {
		close(controller_snapshotclient_for_server_udpsock_list[i]);
	}
	pthread_exit(nullptr);
}

void *run_controller_snapshotclient_cleanup_subthread(void *param) {
	snapshotclient_subthread_param_t &subthread_param = *((snapshotclient_subthread_param_t *)param);

	char sendbuf[MAX_BUFSIZE];
	memcpy(sendbuf, &SNAPSHOT_CLEANUP, sizeof(int));
	memcpy(sendbuf + sizeof(int), &controller_snapshotid, sizeof(int));

	char recvbuf[MAX_BUFSIZE];
	int recvsize = 0;
	bool is_timeout = false;
	while (true) {
		udpsendto(subthread_param.udpsock, sendbuf, 2*sizeof(int), 0, &subthread_param.dstaddr, subthread_param.dstaddrlen, "controller.snapshotclient.cleanup_subthread");

		// wait for SNAPSHOT_CLEANUP_ACK
		is_timeout = udprecvfrom(subthread_param.udpsock, recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotclient.cleanup_subthread");
		if (is_timeout) {
			continue;
		}
		else {
			INVARIANT(recvsize == sizeof(int) && *((int *)recvbuf) == SNAPSHOT_CLEANUP_ACK);
			break;
		}
	}

	pthread_exit(nullptr);
}

void *run_controller_snapshotclient_start_subthread(void *param) {
	snapshotclient_subthread_param_t &subthread_param = *((snapshotclient_subthread_param_t *)param);

	char sendbuf[MAX_BUFSIZE];
	memcpy(sendbuf, &SNAPSHOT_START, sizeof(int));
	memcpy(sendbuf + sizeof(int), &controller_snapshotid, sizeof(int));

	char recvbuf[MAX_BUFSIZE];
	int recvsize = 0;
	bool is_timeout = false;
	while (true) {
		udpsendto(subthread_param.udpsock, sendbuf, 2*sizeof(int), 0, &subthread_param.dstaddr, subthread_param.dstaddrlen, "controller.snapshotclient.start_subthread");

		// wait for SNAPSHOT_start_ACK
		is_timeout = udprecvfrom(subthread_param.udpsock, recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotclient.start_subthread");
		if (is_timeout) {
			continue;
		}
		else {
			INVARIANT(recvsize == sizeof(int) && *((int *)recvbuf) == SNAPSHOT_START_ACK);
			break;
		}
	}

	pthread_exit(nullptr);
}

void *run_controller_snapshotclient_senddata_subthread(void *param) {
	snapshotclient_subthread_param_t &subthread_param = *((snapshotclient_subthread_param_t *)param);

	char recvbuf[MAX_BUFSIZE];
	int recvsize = 0;
	bool is_timeout = false;
	while (true) {
		udpsendlarge_udpfrag(subthread_param.udpsock, controller_snapshotclient_for_server_databuf_list[subthread_param.serveridx].array(), controller_snapshotclient_for_server_databuf_list[subthread_param.serveridx].size(), 0, &subthread_param.dstaddr, subthread_param.dstaddrlen, "controller.snapshotclient.senddata_subthread");

		// wait for SNAPSHOT_SENDDATA_ACK
		is_timeout = udprecvfrom(subthread_param.udpsock, recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotclient.senddata_subthread");
		if (is_timeout) {
			continue;
		}
		else {
			INVARIANT(recvsize == sizeof(int) && *((int *)recvbuf) == SNAPSHOT_SENDDATA_ACK);
			break;
		}
	}

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
	if (controller_snapshotclient_for_server_udpsock_list != NULL) {
		delete [] controller_snapshotclient_for_server_udpsock_list;
		controller_snapshotclient_for_server_udpsock_list = NULL;
	}
	if (controller_snapshotclient_for_server_databuf_list != NULL) {
		delete [] controller_snapshotclient_for_server_databuf_list;
		controller_snapshotclient_for_server_databuf_list = NULL;
	}
}
