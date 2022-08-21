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

// Per-server popclient <-> one popserver in controller
int *controller_popserver_udpsock_list = NULL;
std::atomic<size_t> controller_popserver_finish_threads(0);
size_t controller_expected_popserver_finish_threads = -1;

// Keep atomicity for the following variables
std::mutex mutex_for_pop;
//std::map<netreach_key_t, uint16_t> volatile controller_cachedkey_serveridx_map; // TODO: Evict removes the corresponding kv pair
// Message queue between controller.popservers with controller.popclient (connected with switchos.popserver)
MessagePtrQueue<cache_pop_t> controller_cache_pop_ptr_queue(MQ_SIZE);
/*cache_pop_t ** volatile controller_cache_pop_ptrs = NULL;
uint32_t volatile controller_head_for_pop = 0;
uint32_t volatile controller_tail_for_pop = 0;*/

// controller.popclient <-> switchos.popserver
int controller_popclient_udpsock = -1;

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
bool volatile is_controller_snapshotclient_connected = false;
int controller_snapshotclient_tcpsock = -1;

// controller.snapshotclient.consnapshotclient <-> server.consnapshotserver
bool volatile is_controller_snapshotclient_consnapshotclient_connected = false;
int controller_snapshotclient_consnapshotclient_tcpsock = -1;

void prepare_controller();
void *run_controller_popserver(void *param); // Receive CACHE_POPs from each server
void *run_controller_popclient(void *param); // Send CACHE_POPs to switch os
void *run_controller_evictserver(void *param); // Forward CACHE_EVICT to server and CACHE_EVICT_ACK to switchos in cache eviction
void *run_controller_snapshotclient(void *param); // Periodically notify switch os to launch snapshot
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

	pthread_t popclient_thread;
	int ret = pthread_create(&popclient_thread, nullptr, run_controller_popclient, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
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

	controller_expected_ready_threads = server_num + 3;
	controller_expected_popserver_finish_threads = server_num;

	// prepare popserver sockets
	controller_popserver_udpsock_list = new int[server_num];
	for (uint16_t i = 0; i < server_num; i++) {
		prepare_udpserver(controller_popserver_udpsock_list[i], false, controller_popserver_port_start + i, "controller.popserver");
	}

	//controller_cachedkey_serveridx_map.clear();
	/*controller_cache_pop_ptrs = new cache_pop_t*[MQ_SIZE];
	for (size_t i = 0; i < MQ_SIZE; i++) {
		controller_cache_pop_ptrs[i] = NULL;
	}
	controller_head_for_pop = 0;
	controller_tail_for_pop = 0;*/

	create_udpsock(controller_popclient_udpsock, true, "controller.popclient");

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

	// prepare snapshotclient
	create_tcpsock(controller_snapshotclient_tcpsock, false, "controller.snapshotclient");

	// prepare consnapshotclient
	create_tcpsock(controller_snapshotclient_consnapshotclient_tcpsock, false, "controller.snapshotclient.consnapshotclient");

	memory_fence();

	printf("[controller] prepare end\n");
}

void *run_controller_popserver(void *param) {
	uint16_t idx = *((uint16_t *)param);
	struct sockaddr_in server_popclient_addr;
	socklen_t server_popclient_addrlen = sizeof(struct sockaddr);
	bool with_server_popclient_addr = false;

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

		//printf("receive CACHE_POP from server\n");
		//dump_buf(buf, recvsize);
		cache_pop_t *tmp_cache_pop_ptr = new cache_pop_t(buf, recvsize); // freed by controller.popclient

		// send CACHE_POP_ACK to server.popclient immediately to avoid timeout
		cache_pop_ack_t tmp_cache_pop_ack(tmp_cache_pop_ptr->key());
		uint32_t acksize = tmp_cache_pop_ack.serialize(buf, MAX_BUFSIZE);
		udpsendto(controller_popserver_udpsock_list[idx], buf, acksize, 0, (struct sockaddr*)&server_popclient_addr, server_popclient_addrlen, "controller.popserver");

		/*if (controller_cachedkey_serveridx_map.find(tmp_cache_pop->key()) == controller_cachedkey_serveridx_map.end()) {
			controller_cachedkey_serveridx_map.insert(std::pair<netreach_key_t, uint32_t>(tmp_cache_pop_ptr->key(), tmp_cache_pop_ptr->serveridx()));
		}
		else {
			printf("[controller] Receive duplicate key from server %ld!", tmp_cache_pop_ptr->serveridx());
			exit(-1);
		}*/

		// Serialize CACHE_POPs
		mutex_for_pop.lock();
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

	controller_popserver_finish_threads++;
	close(controller_popserver_udpsock_list[idx]);
	pthread_exit(nullptr);
}

void *run_controller_popclient(void *param) {
	struct sockaddr_in switchos_popserver_addr;
	set_sockaddr(switchos_popserver_addr, inet_addr(switchos_ip), switchos_popserver_port);
	socklen_t switchos_popserver_addrlen = sizeof(struct sockaddr);

	printf("[controller.popclient] ready\n");
	controller_ready_threads++;

	while (!controller_running) {}

	char buf[MAX_BUFSIZE];
	int recvsize = 0;
	while (controller_running) {
		cache_pop_t *tmp_cache_pop_ptr = controller_cache_pop_ptr_queue.read();
		if (tmp_cache_pop_ptr != NULL) {
		//if (controller_tail_for_pop != controller_head_for_pop) {

			//cache_pop_t *tmp_cache_pop_ptr = controller_cache_pop_ptrs[controller_tail_for_pop];

			while (true) {
				// send CACHE_POP to switch os
				uint32_t popsize = tmp_cache_pop_ptr->serialize(buf, MAX_BUFSIZE);
				//printf("send CACHE_POP to switchos\n");
				//dump_buf(buf, popsize);
				udpsendto(controller_popclient_udpsock, buf, popsize, 0, (struct sockaddr*)&switchos_popserver_addr, switchos_popserver_addrlen, "controller.popclient");
				//printf("[controller.popclient] popsize: %d\n", int(popsize)); // TMPDEBUG
				
				bool is_timeout = udprecvfrom(controller_popclient_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.popclient");
				if (unlikely(is_timeout)) {
					continue;
				}
				else {
					cache_pop_ack_t tmp_cache_pop_ack(buf, recvsize);
					INVARIANT(tmp_cache_pop_ack.key() == tmp_cache_pop_ptr->key());
					break;
				}
			}

			// free CACHE_POP
			delete tmp_cache_pop_ptr;
			tmp_cache_pop_ptr = NULL;
			//controller_cache_pop_ptrs[controller_tail_for_pop] = NULL;
			//controller_tail_for_pop = (controller_tail_for_pop + 1) % MQ_SIZE;
		}
	}

	close(controller_popclient_udpsock);
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
	bool direct_parse = false;
	bool is_broken = false;
	int phase = 0; // 0: wait for SNAPSHOT_SERVERSIDE; 1: wait for crash-consistent snapshot data
	int control_type_phase0 = -1;
	char ack_recvbuf[MAX_BUFSIZE]; // SNAPSHOT_SERVERSIDE_ACK from server
	int ack_cur_recv_bytes = 0;
	bool ack_direct_parse = false;
	bool ack_is_broken = false;
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

		// TMPDEBUG
		printf("Type to send SNAPSHOT_START...\n");
		getchar();

		// send SNAPSHOT_START (little-endian) to switchos
		printf("[controller.snapshotclient] send SNAPSHOT_START\n");
		tcpsend(controller_snapshotclient_tcpsock, (char *)&SNAPSHOT_START, sizeof(int), "controlelr.snapshotclient");

		// wait for SNAPSHOT_SERVERSIDE from switchos
		while (true) {
			if (!direct_parse) {
				int recvsize = 0;
				is_broken = tcprecv(controller_snapshotclient_tcpsock, recvbuf + cur_recv_bytes, MAX_LARGE_BUFSIZE - cur_recv_bytes, 0, recvsize, "controller.snapshotclient");
				if (is_broken) {
					break;
				}

				cur_recv_bytes += recvsize;
				if (cur_recv_bytes >= MAX_LARGE_BUFSIZE) {
					printf("[controller.snapshotclient] overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_LARGE_BUFSIZE);
					exit(-1);
				}
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
					printf("[controller.snapshotclient] receive SNAPSHOT_SERVERSIDE from switchos and send to server\n"); // TMPDEBUG
					tcpsend(controller_snapshotclient_consnapshotclient_tcpsock, (char *)&SNAPSHOT_SERVERSIDE, sizeof(int), "controller.snapshotclient.consnapshotclient");

					// wait for SNAPSHOT_SERVERSIDE_ACK from server, and send it to switchos
					while (true) {
						if (!ack_direct_parse) {
							int tmp_recvsize = 0;
							ack_is_broken = tcprecv(controller_snapshotclient_consnapshotclient_tcpsock, ack_recvbuf + ack_cur_recv_bytes, MAX_BUFSIZE - ack_cur_recv_bytes, 0, tmp_recvsize, "controller.snapshotclient.consnapshotclient");
							if (ack_is_broken) {
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
							printf("[controller.snapshotclient] receive SNAPSHOT_SERVERSIDE_ACK from server and send to switchos\n"); // TMPDEBUG
							tcpsend(controller_snapshotclient_tcpsock, (char *)&SNAPSHOT_SERVERSIDE_ACK, sizeof(int), "controller.snapshotclient");

							// Move remaining bytes and reset metadata
							if (ack_cur_recv_bytes > int(sizeof(int))) {
								memcpy(ack_recvbuf, ack_recvbuf + sizeof(int), ack_cur_recv_bytes - sizeof(int));
								ack_cur_recv_bytes = ack_cur_recv_bytes - sizeof(int);
							}
							else {
								ack_cur_recv_bytes = 0;
							}
							if (ack_cur_recv_bytes >= int(sizeof(int))) {
								ack_direct_parse = true;
							}
							else {
								ack_direct_parse = false;
							}
							ack_is_broken = false;
							ack_control_type_phase0 = -1;
							break;
						} // receive a SNAPSHOT_SERVERSIDE_ACK
					} // while (true)

					phase = 1; // wait for crash-consistent snapshot data
					direct_parse = false;
				} // receive a SNAPSHOT_SERVERSIDE
			} // phase == 0
			
			if (phase == 1) { // wait for crash-consistent snapshot data
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
					printf("[controller.snapshotclient] receive snapshot data from switchos and send to server\n"); // TMPDEBUG
					tcpsend(controller_snapshotclient_consnapshotclient_tcpsock, recvbuf + sizeof(int), total_bytes, "controller.snapshotclient.consnapshotclient");

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

					// Move remaining bytes and reset metadata
					if (cur_recv_bytes > sizeof(int) + total_bytes) {
						memcpy(recvbuf, recvbuf + sizeof(int) + total_bytes, cur_recv_bytes - sizeof(int) - total_bytes);
						cur_recv_bytes = cur_recv_bytes - sizeof(int) - total_bytes;
					}
					else {
						cur_recv_bytes = 0;
					}
					if (cur_recv_bytes >= sizeof(int)) {
						direct_parse = true;
					}
					else {
						direct_parse = false;
					}
					is_broken = false;
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
	if (controller_popserver_udpsock_list != NULL) {
		delete [] controller_popserver_udpsock_list;
		controller_popserver_udpsock_list = NULL;
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
