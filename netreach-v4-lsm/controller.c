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

#include "helper.h"
#include "io_helper.h"

#include "common_impl.h"

bool volatile controller_running = false;
std::atomic<size_t> controller_ready_threads(0);
size_t controller_expected_ready_threads = -1;

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

// controller.snapshotclient <-> switchos.snapshotserver
int controller_snapshotid = 0;
int controller_snapshotclient_udpsock = -1;
int controller_snapshotserver_udpsock = -1;
int controller_snapshotserver_snapshotclient_for_server_snapshotserver_udpsock = -1;
int controller_consnapshotserver_udpsock = -1;
int controller_consnapshotserver_snapshotclient_for_server_consnapshotserver_udpsock = -1;

// controller.snapshotclient.consnapshotclient <-> server.consnapshotserver
bool volatile is_controller_snapshotclient_consnapshotclient_connected = false;
int controller_snapshotclient_consnapshotclient_tcpsock = -1;

void prepare_controller();
void *run_controller_popserver(void *param); // Receive CACHE_POPs from each server
void *run_controller_evictserver(void *param); // Forward CACHE_EVICT to server and CACHE_EVICT_ACK to switchos in cache eviction
void *run_controller_snapshotclient(void *param); // Periodically notify switch os to launch snapshot
void *run_controller_snapshotserver(void *param); // Forward SNAPSHOT_SERERSIDE to server
void *run_controller_consnapshotserver(void *param); // Forward SNAPSHOT_DATA to server
void close_controller();

int main(int argc, char **argv) {
	parse_ini("config.ini");
	parse_control_ini("control_type.ini");

	prepare_controller();

	pthread_t popserver_threads[server_num];
	uint16_t popserver_params[server_num];
	for (uint16_t i = 0; i < server_num; i++) {
		popserver_params[i] = i;
		int ret = pthread_create(&popserver_threads[i], nullptr, run_controller_popserver, &popserver_params[i]);
		if (ret) {
			COUT_N_EXIT("Error: " << ret);
		}
	}

	pthread_t evictserver_thread;
	ret = pthread_create(&evictserver_thread, nullptr, run_controller_evictserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t snapshotclient_thread;
	ret = pthread_create(&snapshotclient_thread, nullptr, run_controller_snapshotclient, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t snapshotserver_thread;
	ret = pthread_create(&snapshotserver_thread, nullptr, run_controller_snapshotserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t consnapshotserver_thread;
	ret = pthread_create(&consnapshotserver_thread, nullptr, run_controller_consnapshotserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
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
	int rc = pthread_join(popclient_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(evictserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(snapshotclient_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(snapshotserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(consnapshotserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}

	printf("[controller] all threads end\n");
	close_controller();
}

void prepare_controller() {
	printf("[controller] prepare start\n");

	controller_running =false;

	controller_expected_ready_threads = server_num + 4;
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
	std::string snapshotid_path;
	get_controller_snapshotid_path(snapshotid_path);
	if (isexist(snapshotid_path)) {
		load_snapshotid(controller_snapshotid, snapshotid_path);
	}

	// prepare snapshotclient, snapshotserver, consnapshotserver
	create_udpsock(controller_snapshotclient_udpsock, true, "controller.snapshotclient");
	prepare_udpserver(controller_snapshotserver_udpsock, false, controller_snapshotserver_port, "controller.snapshotserver");
	create_udpsock(controller_snapshotserver_snapshotclient_for_server_snapshotserver_udpsock, true, "controller.snapshotserver.snapshotclient_for_server_snapshotserver");
	prepare_udpserver(controller_consnapshotserver_udpsock, false, controller_consnapshotserver_port, "controller.consnapshotserver");
	create_udpsock(controller_consnapshotserver_snapshotclient_for_server_consnapshotserver_udpsock, true, "controller.consnapshotserver.snapshotclient_for_server_consnapshotserver");

	// prepare consnapshotclient
	create_tcpsock(controller_snapshotclient_consnapshotclient_tcpsock, false, "controller.snapshotclient.consnapshotclient");

	memory_fence();

	printf("[controller] prepare end\n");
}

void *run_controller_popserver(void *param) {
	// controlelr.popserver i <-> server.popclient i
	uint16_t idx = *((uint16_t *)param);
	struct sockaddr_in server_popclient_addr;
	socklen_t server_popclient_addrlen = sizeof(struct sockaddr);
	bool with_server_popclient_addr = false;

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
		if (!with_server_popclient_addr) {
			udprecvfrom(controller_popserver_udpsock_list[idx], buf, MAX_BUFSIZE, 0, (struct sockaddr *)&server_popclient_addr, &server_popclient_addrlen, recvsize, "controller.popserver");
			with_server_popclient_addr = true;
		}
		else {
			udprecvfrom(controller_popserver_udpsock_list[idx], buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.popserver");
		}

		//printf("receive CACHE_POP from server and send it to switchos\n");
		//dump_buf(buf, recvsize);
		cache_pop_t tmp_cache_pop(buf, recvsize);

		// send CACHE_POP to switch os
		udpsendto(controller_popserver_popclient_udpsock_list[idx], buf, recvsize, 0, (struct sockaddr*)&switchos_popserver_addr, switchos_popserver_addrlen, "controller.popserver.popclient");
		
		// receive CACHE_POP_ACK from switch os
		bool is_timeout = udprecvfrom(controller_popserver_popclient_udpsock_list[idx], buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.popserver.popclient");
		if (!is_timeout) {
			// send CACHE_POP_ACK to server.popclient immediately to avoid timeout
			cache_pop_ack_t tmp_cache_pop_ack(buf, recvsize);
			INVARIANT(tmp_cache_pop_ack.key() == tmp_cache_pop.key());
			udpsendto(controller_popserver_udpsock_list[idx], buf, recvsize, 0, (struct sockaddr*)&server_popclient_addr, server_popclient_addrlen, "controller.popserver");
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
	bool with_switchos_evictclient_addr = false;

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
		if (!with_switchos_evictclient_addr) {
			udprecvfrom(controller_evictserver_udpsock, buf, MAX_BUFSIZE, 0, (struct sockaddr*)&switchos_evictclient_addr, &switchos_evictclient_addrlen, recvsize, "controller.evictserver");
			with_switchos_evictclient_addr = true;
		}
		else {
			udprecvfrom(controller_evictserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.evictserver");
		}

		// send CACHE_EVICT to corresponding server
		//tmpserveridx = *((uint16_t *)(buf + arrive_serveridx_bytes - sizeof(uint16_t)));
		//tmpserveridx = uint16_t(ntohs(uint16_t(tmpserveridx)));
		//INVARIANT(tmpserveridx >= 0 && tmpserveridx < server_num);
		//tcpsend(controller_evictserver_evictclient_tcpsock_list[tmpserveridx], buf, arrive_serveridx_bytes, "controller.evictserver.evictclient");
		
		//printf("receive CACHE_EVICT from switchos and send to server\n");
		//dump_buf(buf, recvsize);
		udpsendto(controller_evictserver_evictclient_udpsock, buf, recvsize, 0, (struct sockaddr*)&server_evictserver_addr, server_evictserver_addrlen, "controller.evictserver.evictclient");

		// NOTE: timeout-and-retry of CACHE_EVICT is handled by switchos.popworker.evictclient (cover entire eviction workflow)
		bool is_timeout = udprecvfrom(controller_evictserver_evictclient_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.evictserver.evictclient");
		if (!is_timeout) {
			// send CACHE_EVICT_ACK to switchos.popworker.evictclient
			//printf("receive CACHE_EVICT_ACK from server and send to switchos\n");
			//dump_buf(buf, recvsize);
			udpsendto(controller_evictserver_udpsock, buf, recvsize, 0, (struct sockaddr*)&switchos_evictclient_addr, switchos_evictclient_addrlen, "controller.evictserver");
		}
	}

	close(controller_evictserver_udpsock);
	close(controller_evictserver_evictclient_udpsock);
	pthread_exit(nullptr);
}

// TODO: controller.snapshotclient should guide snapshot in switchos
void *run_controller_snapshotclient(void *param) {
	struct sockaddr_in switchos_snapshotserver_addr;
	set_sockaddr(switchos_snapshotserver_addr, inet_addr(switchos_ip), switchos_snapshotserver_port);
	socklen_t switchos_snapshotserver_addrlen = sizeof(struct sockaddr_in);
	printf("[controller.snapshotclient] ready\n");
	controller_ready_threads++;

	while (!controller_running) {}

	char buf[MAX_BUFSIZE];
	int recvsize = 0;
	while (controller_running) {
		usleep(controller_snapshot_period * 1000); // ms -> us

		// TMPDEBUG
		printf("Type to send SNAPSHOT_START...\n");
		getchar();

		// send SNAPSHOT_START (little-endian) to switchos
		printf("[controller.snapshotclient] send SNAPSHOT_START\n");
		while (true) {
			udpsendto(controller_snapshotclient_udpsock, (char *)&SNAPSHOT_START, sizeof(int), 0, (struct sockaddr*)&switchos_snapshotserver_addr, switchos_snapshotserver_addrlen, "controlelr.snapshotclient");

			bool is_timeout = udprecvfrom(controller_snapshotclient_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotclient");
			if (is_timeout) {
				continue;
			}
			else {
				INVARIANT(recvsize == sizeof(int) && *((int *)buf) == SNAPSHOT_START_ACK);
				break;
			}
		}
	}

	close(controller_snapshotclient_udpsock);
	pthread_exit(nullptr);
}

void *run_controller_snapshotserver(void *param) {
	struct sockaddr_in switchos_snapshotclient_addr;
	socklen_t switchos_snapshotclient_addrlen = sizeof(struct sockaddr_in);
	bool with_switchos_snapshotclient_addr = false;

	struct sockaddr_in server_snapshotserver_addr;
	// TODO: server_snapshotserver_port
	set_sockaddr(server_snapshotserver_addr, inet_addr(server_ip_for_controller), server_consnapshotserver_port);
	socklen_t server_snapshotserver_addrlen = sizeof(struct sockaddr_in);

	printf("[controller.snapshotserver] ready\n");
	controller_ready_threads++;

	while (!controller_running) {}

	char buf[MAX_BUFSIZE];
	int recvsize = 0;
	while (controller_running) {
		if (!with_switchos_snapshotclient_addr) {
			udprecvfrom(controller_snapshotserver_udpsock, buf, MAX_BUFSIZE, 0, (struct sockaddr*)&switchos_snapshotclient_addr, &switchos_snapshotclient_addrlen, recvsize, "controller.snapshotserver");
			with_switchos_snapshotclient_addr = true;
		}
		else {
			udprecvfrom(controller_snapshotserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotserver");
		}
		INVARIANT(recvsize == sizoef(int) && *((int *)buf) == SNAPSHOT_SERVERSIDE);

		// send SNAPSHOT_SERVERSIDE to server.snapshotserver
		udpsendto(controller_snapshotserver_snapshotclient_for_server_snapshotserver_udpsock, &SNAPSHOT_SERVERSIDE_ACK, sizeof(int), 0, (struct sockaddr*)&server_snapshotserver_addr, server_snapshotserver_addrlen, "controller.snapshotserver.snapshotclient_for_server_snapshotserver");

		// wait for SNAPSHOT_SERVERSIDE_ACK
		bool is_timeout = udprecvfrom(controller_snapshotserver_snapshotclient_for_server_snapshotserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotserver.snapshotclient_for_server_snapshotserver");
		if (!is_timeout) {
			// send SNAPSHOT_SERVERSIDE_ACK to switchos.snapshotclient
			INVARIANT(recvsize == sizeof(int) && *((int *)buf) == SNAPSHOT_SERVERSIDE_ACK);
			udpsendto(controller_snapshotserver_udpsock, buf, recvsize, 0, (struct sockaddr*)&switchos_snapshotclient_addr, switchos_snapshotclient_addrlen, "controller.snapshotserver");
		}
	}

	close(controller_snapshotserver_udpsock);
	close(controller_snapshotserver_snapshotclient_for_server_snapshotserver_udpsock);
	pthread_exit(nullptr);
}

void *run_controller_consnapshotserver(void *param) {
	char *recvbuf = new char[MAX_LARGE_BUFSIZE]; // SNAPSHOT_DATA from switchos
	INVARIANT(recvbuf != NULL);
	memset(recvbuf, 0, MAX_LARGE_BUFSIZE);
	int recvsize = 0;

	struct sockaddr_in switchos_consnapshotclient_addr;
	socklen_t switchos_consnapshotclient_addrlen;
	bool with_switchos_consnapshotclient_addr = false;

	struct sockaddr_in server_consnapshotserver_addr;
	set_sockaddr(server_consnapshotserver_addr, inet_addr(server_ip_for_controller), server_consnapshotserver_port);
	socklen_t server_consnapshotserver_addrlen = sizeof(struct sockaddr_in);

	printf("[controller.consnapshotserver] ready\n");
	controller_ready_threads++;

	while (!controller_running) {}

	// NOTE: as messages are sent among end-hosts (controller, switchos, and server), we do not perform endian conversion
	char ackbuf[MAX_BUFSIZE];
	int ack_recvsize = 0;
	while (controller_running) {
		if (!with_switchos_consnapshotclient_addr) {
			udprecvlarge_udpfrag(controller_consnapshotserver_udpsock, recvbuf, MAX_LARGE_BUFSIZE, 0, (struct sockaddr*)&switchos_consnapshotclient_addr, switchos_consnapshotclient_addrlen, recvsize, "controller.consnapshotserver");
			with_switchos_consnapshotclient_addr = true;
		}
		else {
			udprecvlarge_udpfrag(controller_consnapshotserver_udpsock, recvbuf, MAX_LARGE_BUFSIZE, 0, NULL, NULL, recvsize, "controller.consnapshotserver");
		}
			
		// snapshot data: <int SNAPSHOT_DATA, int32_t total_bytes, per-server data>
		// per-server data: <int32_t perserver_bytes, uint16_t serveridx, int32_t recordcnt, per-record data>
		//// per-record data: <16B key, uint32_t vallen, value (w/ padding), uint32_t seq, bool stat>
		// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>
		
		// send snapshot data to server.consnapshotserver
		printf("[controller.consnapshotserver] receive snapshot data from switchos and send to server\n"); // TMPDEBUG
		udpsendlarge_udpfrag(controller_consnapshotserver_snapshotclient_for_server_consnapshotserver_udpsock, recvbuf, recvsize, 0, (struct sockaddr*)&server_consnapshotserver_addr, server_consnapshotserver_addrlen, "controller.snapshotclient.consnapshotclient");
		
		bool is_timeout = udprecvfrom(controller_consnapshotserver_snapshotclient_for_server_consnapshotserver_udpsock, ackbuf, ack_recvsize, 0, NULL, NULL, "controller.snapshotclient.consnapshotclient");
		if (!is_timeout) {
			INVARIANT(ack_recvsize == sizeof(int) && *((int *)ackbuf) == SNAPSHOT_DATA_ACK);
			// send SNAPSHOT_DATA_ACK to switchos
			udpsendto(controller_consnapshotserver_udpsock, ackbuf, ack_recvsize, 0, (struct sockaddr*)&switchos_consnapshotclient_addr, switchos_consnapshotclient_addrlen, "controller.consnapshotserver");

			// for switch failure recovery
			controller_snapshotid += 1;
			// store inswitch snapshot data
			std::string snapshotdata_path;
			get_controller_snapshotdata_path(snapshotdata_path, controller_snapshotid);
			store_buf(recvbuf + sizeof(int), total_bytes, snapshotdata_path);
			// store latest snapshot id at last 
			std::string snapshotid_path;
			get_controller_snapshotid_path(snapshotid_path);
			store_snapshotid(controller_snapshotid, snapshotid_path);
			// remove old-enough snapshot data
			int old_snapshotid = controller_snapshotid - 1;
			std::string old_snapshotdata_path;
			get_controller_snapshotdata_path(old_snapshotdata_path, old_snapshotid);
			rmfiles(old_snapshotdata_path.c_str());
		}
	} // while (controller_running)

	delete [] recvbuf;
	recvbuf = NULL;
	close(controller_consnapshotserver_udpsock);
	close(controller_consnapshotserver_snapshotclient_for_server_consnapshotserver_udpsock);
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
