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

// cache population/eviction

// switchos.popworker.popclient_for_controller <-> controller.popserver
int controller_popserver_udpsock = -1;
// controller.popclient <-> server.popserver
int controller_popserver_popclient_for_server_udpsock = -1;
// controller.popclient <-> spine/leaf switchos.cppopserver
int controller_popserver_popclient_for_spine_udpsock = -1;
int controller_popserver_popclient_for_leaf_udpsock = -1;
// TODO: replace with SIGTERM
bool volatile is_controller_popserver_finish = false;

// spineswitchos.popworker.victimclient_for_controller <-> controller.victimserver
int controller_victimserver_udpsock = -1;
// controller.victimserver <-> leafswitchos.popworker.victimserver
int controller_victimserver_victimclient_for_leaf_udpsock = -1;

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
void *run_controller_victimserver(void *param); // receive DISTCACHE_CACHE_EVICT_VICTIM from spine switchos
void validate_switchidx(netreach_key_t key); // validate spine/leaf switchidx for the give key
void *run_controller_evictserver(void *param); // Forward CACHE_EVICT to server and CACHE_EVICT_ACK to switchos in cache eviction
void close_controller();

cpu_set_t nonserverworker_cpuset; // [server_cores, total_cores-1] for all other threads

int main(int argc, char **argv) {
	parse_ini("config.ini");
	parse_control_ini("control_type.ini");

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

	pthread_t popserver_thread;
	ret = pthread_create(&popserver_thread, nullptr, run_controller_popserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}
	ret = pthread_setaffinity_np(popserver_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
	if (ret) {
		printf("Error of setaffinity for controller.popserver; errno: %d\n", errno);
		exit(-1);
	}

	pthread_t victimserver_thread;
	ret = pthread_create(&victimserver_thread, nullptr, run_controller_victimserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}
	ret = pthread_setaffinity_np(victimserver_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
	if (ret) {
		printf("Error of setaffinity for controller.victimserver; errno: %d\n", errno);
		exit(-1);
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
	while (!is_controller_popserver_finish) sleep(1);
	printf("[controller] controller.popserver finishes\n");

	controller_running = false;

	void * status;
	int rc = pthread_join(popserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join popserver " << rc);
	}
	rc = pthread_join(victimserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join victimserver " << rc);
	}
	rc = pthread_join(evictserver_thread, &status);
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

	controller_expected_ready_threads = 3;

	// prepare popserver sockets
	prepare_udpserver(controller_popserver_udpsock, false, controller_popserver_port_start, "controller.popserver");
	create_udpsock(controller_popserver_popclient_for_server_udpsock, true, "controller.popserver.popclient_for_server");
	create_udpsock(controller_popserver_popclient_for_spine_udpsock, true, "controller.popserver.popclient_for_spine");
	create_udpsock(controller_popserver_popclient_for_leaf_udpsock, true, "controller.popserver.popclient_for_leaf");

	// prepare victimserver
	prepare_udpserver(controller_victimserver_udpsock, false, controller_victimserver_port, "controller.victimserver");
	create_udpsock(controller_victimserver_victimclient_for_leaf_udpsock, true, "controller.victimserver.victimclient_for_leaf");

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

	struct sockaddr_in switchos_dppopserver_popworker_addr;
	socklen_t switchos_dppopserver_popworker_addrlen = sizeof(struct sockaddr);

	struct sockaddr_in spineswitchos_cppopserver_addr;
	set_sockaddr(spineswitchos_cppopserver_addr, inet_addr(spineswitchos_ip), switchos_cppopserver_port);
	socklen_t spineswitchos_cppopserver_addrlen = sizeof(struct sockaddr_in);

	struct sockaddr_in leafswitchos_cppopserver_addr;
	set_sockaddr(leafswitchos_cppopserver_addr, inet_addr(leafswitchos_ip), switchos_cppopserver_port);
	socklen_t leafswitchos_cppopserver_addrlen = sizeof(struct sockaddr_in);

	printf("[controller.popserver] ready\n");
	controller_ready_threads++;

	while (!controller_running) {}

	// Process NETCACHE_CACHE_POP packet <optype, key, serveridx>
	char buf[MAX_BUFSIZE];
	int recvsize = 0;
	bool is_timeout = false;
	while (controller_running) {
		udprecvfrom(controller_popserver_udpsock, buf, MAX_BUFSIZE, 0, &switchos_dppopserver_popworker_addr, &switchos_dppopserver_popworker_addrlen, recvsize, "controller.popserver");

		//printf("receive NETCACHE_CACHE_POP or NETCACHE_CACHE_POP_FINISH from switchos and send it to server\n");
		//dump_buf(buf, recvsize);
		packet_type_t tmp_optype = get_packet_type(buf, recvsize);
		netreach_key_t tmp_key;
		uint16_t tmp_serveridx = 0;
		if (tmp_optype == packet_type_t::NETCACHE_CACHE_POP) {
			netcache_cache_pop_t tmp_netcache_cache_pop(buf, recvsize);
			tmp_key = tmp_netcache_cache_pop.key();
			tmp_serveridx = tmp_netcache_cache_pop.serveridx();
		}
		else if (tmp_optype == packet_type_t::NETCACHE_CACHE_POP_FINISH) {
			netcache_cache_pop_finish_t tmp_netcache_cache_pop_finish(buf, recvsize);
			tmp_key = tmp_netcache_cache_pop_finish.key();
			tmp_serveridx = tmp_netcache_cache_pop_finish.serveridx();
		}
		else {
			printf("[controller.popserver] invalid packet type: %x\n", optype_t(tmp_optype));
			exit(-1);
		}
		INVARIANT(tmp_serveridx >= 0 && tmp_serveridx < max_server_total_logical_num);

		// find corresponding server X
		int tmp_server_physical_idx = -1;
		for (int i = 0; i < server_physical_num; i++) {
			for (int j = 0; j < server_logical_idxes_list[i].size(); j++) {
				if (tmp_serveridx == server_logical_idxes_list[i][j]) {
					tmp_server_physical_idx = i;
					break;
				}
			}
		}
		INVARIANT(tmp_server_physical_idx != -1);

		// controller.popserver.popclient <-> server.popserver X
		struct sockaddr_in tmp_server_popserver_addr;
		set_sockaddr(tmp_server_popserver_addr, inet_addr(server_ip_for_controller_list[tmp_server_physical_idx]), server_popserver_port_start + tmp_serveridx);
		socklen_t tmp_server_popserver_addrlen = sizeof(struct sockaddr);

		// send NETCACHE_CACHE_POP/_FINISH to corresponding server
		udpsendto(controller_popserver_popclient_for_server_udpsock, buf, recvsize, 0, &tmp_server_popserver_addr, tmp_server_popserver_addrlen, "controller.popserver.popclient_for_server");
		
		// receive NETCACHE_CACHE_POP_ACK/_FINISH_ACK from corresponding server
		bool is_timeout = udprecvfrom(controller_popserver_popclient_for_server_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.popserver.popclient_for_server");
		if (!is_timeout) { // NOTE: if timeout, it will be covered by switcho.popworker
			// send NETCACHE_CACHE_POP_ACK/_FINISH_ACK to switchos.popworker immediately to avoid timeout
			packet_type_t tmp_acktype = get_packet_type(buf, recvsize);
			if (tmp_acktype == packet_type_t::NETCACHE_CACHE_POP_ACK) {
				netcache_cache_pop_ack_t tmp_netcache_cache_pop_ack(buf, recvsize);
				INVARIANT(tmp_netcache_cache_pop_ack.key() == tmp_key);

				// validate spine/leaf switchidx
				validate_switchidx(tmp_netcache_cache_pop_ack.key());

				// send two CACHE_POPs to spine/leaf switchos.cppopserver
				cache_pop_t tmp_cache_pop(tmp_netcache_cache_pop_ack.key(), tmp_netcache_cache_pop_ack.val(), tmp_netcache_cache_pop_ack.seq(), tmp_netcache_cache_pop_ack.stat(), tmp_netcache_cache_pop_ack.serveridx());
				int pktsize = tmp_cache_pop.serialize(buf, MAX_BUFSIZE);
				udpsendto(controller_popserver_popclient_for_spine, buf, pktsize, 0, &spineswitchos_cppopserver_addr, spineswitchos_cppopserver_addrlen, "controller.popserver.popclient_for_spine");
				udpsendto(controller_popserver_popclient_for_leaf, buf, pktsize, 0, &leafswitchos_cppopserver_addr, leafswitchos_cppopserver_addrlen, "controller.popserver.popclient_for_leaf");

				// wait for two CACHE_POP_ACKs
				is_timeout = udprecvfrom(controller_popserver_popclient_for_spine, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.popserver.popclient_for_spine");
				if (!is_timeout) {
					cache_pop_ack_t tmp_cache_pop_ack_from_spine(buf, recvsize);
					INVARIANT(tmp_cache_pop_ack_from_spine.key() == tmp_cache_pop.key());
					is_timeout = udprecvfrom(controller_popserver_popclient_for_leaf, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.popserver.popclient_for_leaf");
					if (!is_timeout) {
						cache_pop_ack_t tmp_cache_pop_ack_from_leaf(buf, recvsize);
						INVARIANT(tmp_cache_pop_ack_from_leaf.key() == tmp_cache_pop.key());

						// send NETCACHE_CACHE_POP_ACK to original switchos.dppopserver
						pktsize = tmp_netcache_cache_pop_ack.serialize(buf, MAX_BUFSIZE);
						udpsendto(controller_popserver_udpsock, buf, pktsize, 0, &switchos_dppopserver_popworker_addr, switchos_dppopserver_popworker_addrlen, "controller.popserver");
					}
				}
			}
			else if (tmp_acktype == packet_type_t::NETCACHE_CACHE_POP_FINISH_ACK) {
				netcache_cache_pop_finish_ack_t tmp_netcache_cache_pop_finish_ack(buf, recvsize);
				INVARIANT(tmp_netcache_cache_pop_finish_ack.key() == tmp_key);
				// send NETCACHE_CACHE_POP_FINISH_ACK to original switchos.popworker
				udpsendto(controller_popserver_udpsock, buf, recvsize, 0, &switchos_dppopserver_popworker_addr, switchos_dppopserver_popworker_addrlen, "controller.popserver");
			}
			else {
				printf("[controller.popserver] invalid acktype: %x\n", optype_t(tmp_acktype));
				exit(-1);
			}
		}
	}

	is_controller_popserver_finish = true;
	close(controller_popserver_udpsock);
	close(controller_popserver_popclient_for_server_udpsock);
	close(controller_popserver_popclient_for_spine_udpsock);
	close(controller_popserver_popclient_for_leaf_udpsock);
	pthread_exit(nullptr);
}

void *run_controller_victimserver(void *param) {
	struct sockaddr_in spineswitchos_victimclient_addr;
	socklen_t soineswitchos_victimclient_addrlen = sizeof(struct sockaddr_in);

	struct sockaddr_in leafswitchos_victimserver_addr;
	set_sockaddr(leafswitchos_victimserver_addr, inet_addr(leafswitchos_ip_for_controller), leafswitchos_victimserver_port);
	socklen_t leafswitchos_victimserver_addrlen = sizeof(struct sockaddr_in);

	printf("[controller.victimserver] ready\n");
	controller_ready_threads++;

	while (!controller_running) {}

	// Process NETCACHE_CACHE_POP packet <optype, key, serveridx>
	char buf[MAX_BUFSIZE];
	int recvsize = 0;
	bool is_timeout = false;
	while (controller_running) {
		udprecvfrom(controller_victimserver_udpsock, buf, MAX_BUFSIZE, 0, &spineswitchos_victimclient_addr, &spineswitchos_victimclient_addrlen, recvsize, "controller.victimserver");

		//printf("receive DISTCACHE_CACHE_EVICT_VICTIM from spineswitchos\n");
		//dump_buf(buf, recvsize);

		distcache_cache_evict_victim_t tmp_distcache_cache_evict_victim(buf, recvsize);

		// validate spine/leaf switchidx
		validate_switchidx(tmp_distcache_cache_evict_victim.key());

		// send DISTCACHE_CACHE_EVICT_VICTIM to leaf switchos
		udpsendto(controller_victimserver_victimclient_for_leaf_udpsock, buf, recvsize, 0, &leafswitchos_victimserver_addr, leafswitchos_victimserver_addrlen, "controller.victimserver.victimclient_for_leaf");

		// wait for DISTCACHE_CACHE_EVICT_VICTIM_ACK from leaf switchos
		is_timeout = udprecvfrom(controller_victimserver_victimclient_for_leaf_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controlelr.victimserver.victimclient_for_leaf");
		if (!is_timeout) {
			distcache_cache_evict_victim_ack_t tmp_distcache_cache_evict_victim_ack(buf, recvsize);
			INVARIANT(tmp_distcache_cache_evict_victim_ack.key() == tmp_distcache_cache_evict_victim.key());
			// send DISTCACHE_CACHE_EVICT_VICTIM_ACK to spine switchos
			udpsendto(controller_victimserver_udpsock, buf, recvsize, 0, &spineswitchos_victimclient_addr, spineswitchos_victimclient_addrlen, "controller.victimserver");
		}
	}

	close(controller_victimserver_udpsock);
	close(controlelr_victimserver_victimclient_for_leaf_udpsock);
	pthread_exit(nullptr);
}

void validate_switchidx(netreach_key_t key) {
	uint32_t tmp_spineswitchidx = key.get_spineswitch_idx(switch_partition_count, spineswitch_total_logical_num);
	bool tmp_valid = false;
	for (size_t i = 0; i < spineswitch_total_logical_num; i++) {
		if (tmp_spineswitchidx == spineswitch_logical_idxes[i]) {
			tmp_valid = true;
			break;
		}
	}
	if (tmp_valid == false) {
		printf("Invalid spintswitchidx %d for key %x\n", tmp_spineswitchidx, key.keyhihi);
		exit(-1);
	}
	uint32_t tmp_leafswitchidx = key.get_leafswitch_idx(switch_partition_count, max_server_total_logical_num, leafswitch_total_logical_num, spineswitch_total_logical_num);
	tmp_valid = false;
	for (size_t i = 0; i < leafswitch_total_logical_num; i++) {
		if (tmp_leafswitchidx == leafswitch_logical_idxes[i]) {
			tmp_valid = true;
			break;
		}
	}
	if (tmp_valid == false) {
		printf("Invalid leafswitchidx %d for key %x\n", tmp_leafswitchidx, key.keyhihi);
		exit(-1);
	}
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

	// process NETCACHE_CACHE_EVICT packet <optype, key, serveridx>
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
		netcache_cache_evict_t tmp_netcache_cache_evict(buf, recvsize);
		uint16_t tmp_global_server_logical_idx = tmp_netcache_cache_evict.serveridx();
		INVARIANT(tmp_global_server_logical_idx >= 0 && tmp_global_server_logical_idx < max_server_total_logical_num);
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
		
		//printf("receive NETCACHE_CACHE_EVICT from switchos and send to server\n");
		//dump_buf(buf, recvsize);
		udpsendto(controller_evictserver_evictclient_udpsock, buf, recvsize, 0, &server_evictserver_addr, server_evictserver_addrlen, "controller.evictserver.evictclient");

		// NOTE: timeout-and-retry of NETCACHE_CACHE_EVICT is handled by switchos.popworker.evictclient (cover entire eviction workflow)
		bool is_timeout = udprecvfrom(controller_evictserver_evictclient_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.evictserver.evictclient");
		if (!is_timeout) {
			// send NETCACHE_CACHE_EVICT_ACK to switchos.popworker.evictclient
			//printf("receive NETCACHE_CACHE_EVICT_ACK from server and send to switchos\n");
			//dump_buf(buf, recvsize);
			netcache_cache_evict_ack_t tmp_netcache_cache_evict_ack(buf, recvsize);
			INVARIANT(tmp_netcache_cache_evict_ack.key() == tmp_netcache_cache_evict.key());
			udpsendto(controller_evictserver_udpsock, buf, recvsize, 0, &switchos_evictclient_addr, switchos_evictclient_addrlen, "controller.evictserver");
		}
	}

	close(controller_evictserver_udpsock);
	close(controller_evictserver_evictclient_udpsock);
	pthread_exit(nullptr);
}

void close_controller() {
	return;
}
