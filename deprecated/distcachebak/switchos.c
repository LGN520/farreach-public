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
#include <set>
#include <signal.h> // for signal and raise
#include <arpa/inet.h> // inetaddr conversion
#include <sys/time.h> // struct timeval
#include <string.h>
#include <map>
#include <mutex>

#include "../common/helper.h"
#include "../common/io_helper.h"
#include "../common/key.h"
#include "../common/val.h"

#include "../common/socket_helper.h"
#include "../common/iniparser/iniparser_wrapper.h"
#include "message_queue_impl.h"
#include "../common/packet_format_impl.h"
#include "concurrent_map_impl.h"
#include "../common/dynamic_array.h"

#include "common_impl.h"

// ptf scripts used by switchos
// cache eviction: get_evictdata_setvalid3.sh, remove_cache_lookup.sh
// cache population: setvalid0.sh, add_cache_lookup_setvalid1.sh

char switchos_role[256];
// set reflector configuration based on switchos role
char reflector_ip_for_switchos[256];
short reflector_dp2cpserver_port = -1;
short reflector_cp2dpserver_port = -1;

bool volatile switchos_running = false;
std::atomic<size_t> switchos_ready_threads(0);
const size_t switchos_expected_ready_threads = 3;
bool volatile switchos_dppopserver_finish = false;

// Cache population

// reflector.dp2cpserver -> switchos.dppopserver (for NETCACHE_WARMUPREQ_INSWITCH_POP / NETCACHE_GETREQ_POP)
int switchos_dppopserver_udpsock = -1;
// switchos.dppopserver.popclient_for_controller <-> controller.popserver (for NETCACHE_CACHE_POP)
int switchos_dppopserver_popclient_for_controller_udpsock = -1;
// used by switchos.dppopsever and switchos.popworker -> whether the key has been reported by data plane
std::mutex mutex_for_cached_keyset;
std::set<netreach_key_t> switchos_dppopserver_cached_keyset;
// switchos.cppopserver <-> controller.popserver.popclient_for_spine/leaf (for CACHE_POP)
int switchos_cppopserver_udpsock = -1;
// message queue between switchos.cppopserver and switchos.popworker
MessagePtrQueue<cache_pop_t> switchos_cache_pop_ptr_queue(MQ_SIZE);

// switchos.popworker

// switchos.popworker <-> controller.popserver (for NETCACHE_CACHE_POP_FINISH)
int switchos_popworker_popclient_for_controller_udpsock = -1;
// spineswitchos.popworker <-> controller.victimserver
int spineswitchos_popworker_victimclient_for_controller_udpsock = -1;
// leafswitchos.popworker <-> controller.victimserver.victimclient_for_leaf
int leafswitchos_popworker_victimserver_udpsock = -1;
// switchos.popworker <-> ptf.popserver
int switchos_popworker_popclient_for_ptf_udpsock = -1; 
// switchos.popworker <-> reflector.cp2dpserver
int switchos_popworker_popclient_for_reflector_udpsock = -1;
// switchos.popworker <-> controller.evictserver for cache eviction
int switchos_popworker_evictclient_for_controller_udpsock = -1;
// cached metadata
std::map<netreach_key_t, uint32_t> switchos_cached_keyidx_map; // TODO: Comment it after checking server.cached_keyset_list
netreach_key_t volatile ** switchos_perpipeline_cached_keyarray = NULL; // idx (of inswitch KV) -> key (TODO: different switches for distributed case)
uint16_t volatile ** switchos_perpipeline_cached_serveridxarray = NULL; // idx (of inswitch KV) -> serveridx of the key
uint32_t volatile * switchos_perpipeline_cached_empty_index = 0; // [empty index, kv_bucket_num-1] is empty

void prepare_switchos();
void *run_switchos_dppopserver(void *param);
void *run_switchos_cppopserver(void *param);
void *run_switchos_popworker(void *param);
void calculate_switchidx(netreach_key_t key, uint16_t &spineswitchidx, uint16_t &leafswitchidx);
uint16_t calculate_switchidx_by_role(netreach_key_t key);
void close_switchos();

// switchos <-> ptf.popserver
//inline uint32_t serialize_setvalid0(char *buf, uint16_t freeidx, uint32_t pipeidx);
//inline uint32_t serialize_add_cache_lookup_setvalid1(char *buf, netreach_key_t key, uint16_t freeidx, uint32_t pipeidx);
inline uint32_t serialize_add_cache_lookup(char *buf, netreach_key_t key, uint16_t switchidx, uint16_t freeidx);
//inline uint32_t serialize_setvalid3(char *buf, uint16_t evictidx, uint32_t pipeidx);
// NOTE: now we load evicted data directly from data plane instead of via ptf channel
//inline uint32_t serialize_get_evictdata_setvalid3(char *buf, uint32_t pipeidx);
//inline void parse_evictdata(char *buf, int recvsize, uint16_t &switchos_evictidx, val_t &switchos_evictvalue, uint32_t &switchos_evictseq, bool &switchos_evictstat);
inline uint32_t serialize_remove_cache_lookup(char *buf, netreach_key_t key, uint16_t switchidx);

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Usage: ./switchos spine/leaf\n");
		fflush(stdout);
		exit(-1);
	}

	parse_ini("config.ini");
	parse_control_ini("control_type.ini");

	memcpy(switchos_role, argv[1], strlen(argv[1]));
	// update reflector configuration based on switchos role
	if (strcmp(switchos_role, "spine") == 0) {
		memcpy(reflector_ip_for_switchos, spine_reflector_ip_for_switchos, strlen(spine_reflector_ip_for_switchos));
		reflector_dp2cpserver_port = spine_reflector_dp2cpserver_port;
		reflector_cp2dpserver_port = spine_reflector_cp2dpserver_port;
	}
	else if (strcmp(switchos_role, "leaf") == 0) {
		memcpy(reflector_ip_for_switchos, leaf_reflector_ip_for_switchos, strlen(leaf_reflector_ip_for_switchos));
		reflector_dp2cpserver_port = leaf_reflector_dp2cpserver_port;
		reflector_cp2dpserver_port = leaf_reflector_cp2dpserver_port;
	}
	else {
		printf("Invalid switchos role: %s which should be spine/leaf\n", switchos_role);
		fflush(stdout);
		exit(-1);
	}

	prepare_switchos();

	pthread_t dppopserver_thread;
	int ret = pthread_create(&dppopserver_thread, nullptr, run_switchos_dppopserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t cppopserver_thread;
	ret = pthread_create(&cppopserver_thread, nullptr, run_switchos_cppopserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t popworker_thread;
	ret = pthread_create(&popworker_thread, nullptr, run_switchos_popworker, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	while (switchos_ready_threads < switchos_expected_ready_threads) sleep(1);
	printf("[switchos] all threads ready\n");

	switchos_running = true;

	// connection from controller
	while (!switchos_dppopserver_finish) sleep(1);

	switchos_running = false;

	void *status;
	int rc = pthread_join(dppopserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(cppopserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(popworker_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}

	free_common();
	close_switchos();
	printf("[switchos] all threads end\n");
}

void prepare_switchos() {
	printf("[switchos] prepare start\n");

	srand(0); // set random seed as 0 for cache eviction

	// prepare for dppopserver

	// prepare dppopserver socket
	prepare_udpserver(switchos_dppopserver_udpsock, false, switchos_dppopserver_port, "switchos.dppopserver", SOCKET_TIMEOUT, 0, UDP_LARGE_RCVBUFSIZE * 2);

	// dppopserver <-> controller.popserver (for NETCAHCE_CACHE_POP)
	create_udpsock(switchos_dppopserver_popclient_for_controller_udpsock, true, "switchos.dppopserver.popclient_for_controller");

	// prepare for cppopserver
	
	// prepare cppopserver socket
	prepare_udpserver(switchos_cppopserver_udpsock, false, switchos_cppopserver_port, "switchos.cppopserver");

	// prepare for popworker
	
	// popworker <-> controller.popserver (for NETCACHE_CACHE_POP_FINISH)
	create_udpsock(switchos_popworker_popclient_for_controller_udpsock, true, "switchos.popworker.popclient_for_controller");

	if (strcmp(switchos_role, "spine") == 0) {
		// spineswitchos.popworker <-> controller.victimserver
		create_udpsock(spineswitchos_popworker_victimclient_for_controller_udpsock, true, "spineswitchos.popworker.victimclient_for_controller");
	}
	else if (strcmp(switchos_role, "leaf") == 0) {
		// leafswitchos.popworker <-> controller.victimserver.victimclient_for_leaf
		prepare_udpserver(leafswitchos_popworker_victimserver_udpsock, false, leafswitchos_victimserver_port, "leafswitchos.popworker.victimserver");
	}
	else {
		printf("[prepare switchos] invalid switchos role: %s\n", switchos_role);
		fflush(stdout);
		exit(-1);
	}
	
	// popworker <-> ptf.popserver
	create_udpsock(switchos_popworker_popclient_for_ptf_udpsock, false, "switchos.popworker.popclient_for_ptf");

	// popworker <-> controller.evictserver
	create_udpsock(switchos_popworker_evictclient_for_controller_udpsock, true, "switchos.popworker.evictclient");
	//switchos_evictvalbytes = new char[val_t::MAX_VALLEN];
	//INVARIANT(switchos_evictvalbytes != NULL);
	//memset(switchos_evictvalbytes, 0, val_t::MAX_VALLEN);
	
	// popworker <-> reflector.cp2dpserver
	create_udpsock(switchos_popworker_popclient_for_reflector_udpsock, true, "switchos.popworker.popclient_for_reflector", 0, SWITCHOS_POPCLIENT_FOR_REFLECTOR_TIMEOUT_USECS);

	// cached metadata
	switchos_cached_keyidx_map.clear();
	switchos_perpipeline_cached_keyarray = (netreach_key_t volatile **)(new netreach_key_t*[switch_pipeline_num]);
	switchos_perpipeline_cached_serveridxarray = (uint16_t volatile **)new uint16_t*[switch_pipeline_num];
	switchos_perpipeline_cached_empty_index = new uint32_t[switch_pipeline_num];
	for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
		switchos_perpipeline_cached_keyarray[tmp_pipeidx] = new netreach_key_t[switch_kv_bucket_num]();
		switchos_perpipeline_cached_serveridxarray[tmp_pipeidx] = new uint16_t[switch_kv_bucket_num];
		memset((void *)switchos_perpipeline_cached_serveridxarray[tmp_pipeidx], 0, sizeof(uint16_t) * switch_kv_bucket_num);
		switchos_perpipeline_cached_empty_index[tmp_pipeidx] = 0;
	}

	memory_fence();

	printf("[switchos] prepare end\n");
}

void *run_switchos_dppopserver(void *param) {
	// used to fetch value from server by controller
	struct sockaddr_in controller_popserver_addr;
	set_sockaddr(controller_popserver_addr, inet_addr(controller_ip_for_switchos), controller_popserver_port_start);
	socklen_t controller_popserver_addrlen = sizeof(struct sockaddr_in);

	printf("[switchos.dppopserver] ready\n");
	switchos_ready_threads++;

	while (!switchos_running) {}

	// Process NETCACHE_GETREQ_POP packet <optype, key, clone_hdr>
	char buf[MAX_BUFSIZE];
	int recvsize = 0;
	while (switchos_running) {
		udprecvfrom(switchos_dppopserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "switchos.dppopserver");

		//printf("receive NETCACHE_GETREQ_POP/NETCACHE_WARMUPREQ_INSWITCH_POP from reflector\n");
		//dump_buf(buf, recvsize);
		
		netcache_getreq_pop_t *tmp_netcache_getreq_pop_ptr = NULL; // freed by current switchos.dppopserver
		packet_type_t tmp_optype = get_packet_type(buf, recvsize);
		if (tmp_optype == packet_type_t::NETCACHE_WARMUPREQ_INSWITCH_POP) {
			netcache_warmupreq_inswitch_pop_t tmp_netcache_warmupreq_inswitch_pop(CURMETHOD_ID, buf, recvsize);
			tmp_netcache_getreq_pop_ptr = new netcache_getreq_pop_t(CURMETHOD_ID, tmp_netcache_warmupreq_inswitch_pop.key());
		}
		else if (tmp_optype == packet_type_t::NETCACHE_GETREQ_POP) {
			if (workload_mode == 0) { // static pattern does not need change cache for NETCACHE_GETREQ_POP
				continue;
			}

			tmp_netcache_getreq_pop_ptr = new netcache_getreq_pop_t(CURMETHOD_ID, buf, recvsize);
		}
		else {
			printf("[switchos.dppopserver] invalid pkttype: %x\n", optype_t(tmp_optype));
			fflush(stdout);
			exit(-1);
		}
		INVARIANT(tmp_netcache_getreq_pop_ptr != NULL);

		// fix duplicate cache population (NETCACHE_GETREQ_POP/NETCACHE_WARMUPREQ_INSWITCH_POP) from data plane
		bool is_cached = false;
		mutex_for_cached_keyset.lock();
		if (switchos_dppopserver_cached_keyset.find(tmp_netcache_getreq_pop_ptr->key()) == switchos_dppopserver_cached_keyset.end()) {
			switchos_dppopserver_cached_keyset.insert(tmp_netcache_getreq_pop_ptr->key());

			//bool tmpbool = switchos_dppopserver_cached_keyset.find(tmp_netcache_getreq_pop_ptr->key()) == switchos_dppopserver_cached_keyset.end();
			//COUT_VAR(tmpbool);
			//fflush(stdout);
		}
		else {
			is_cached = true;
		}
		mutex_for_cached_keyset.unlock();

		if (!is_cached) { // not cached
			// calculate global server logical index
#ifdef USE_HASH
			uint32_t tmp_global_server_logical_idx = tmp_netcache_getreq_pop_ptr->key().get_hashpartition_idx(switch_partition_count, max_server_total_logical_num);
#elif defined(USE_RANGE)
			uint32_t tmp_global_server_logical_idx = tmp_netcache_getreq_pop_ptr->key().get_rangepartition_idx(max_server_total_logical_num);
#endif

			//// DEPRECATED: send NETCACHE_CACHE_POP to fetch value from correpsonding server by controller.popserver
			// send NETCACHE_CACHE_POP to controller.popserver to populate new key in both spine and server-leaf switch for power-of-two-choices-based query routing
			netcache_cache_pop_t tmp_netcache_cache_pop(CURMETHOD_ID, tmp_netcache_getreq_pop_ptr->key(), uint16_t(tmp_global_server_logical_idx));
			while (true) {
				int pktsize = tmp_netcache_cache_pop.serialize(buf, MAX_BUFSIZE);
				udpsendto(switchos_dppopserver_popclient_for_controller_udpsock, buf, pktsize, 0, &controller_popserver_addr, controller_popserver_addrlen, "switchos.dppopserver.popclient_for_controller");

				bool is_timeout = udprecvfrom(switchos_dppopserver_popclient_for_controller_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "switchos.dppopserver.popclient_for_controller");
				if (unlikely(is_timeout)) {
					continue;
				}
				else {
					netcache_cache_pop_ack_t tmp_netcache_cache_pop_ack(CURMETHOD_ID, buf, recvsize);
					if (unlikely(tmp_netcache_cache_pop_ack.key() != tmp_netcache_cache_pop.key())) {
						printf("unmatched key of NETCACHE_CACHE_POP_ACK!\n");
						continue;
					}
					else {
						INVARIANT(tmp_netcache_cache_pop_ack.val() == val_t()); // NETCACHE_CACHE_POP_ACK must w/ default value in DistCache
						break;
					}
				}
			}
		}

		// free NETCACHE_GETREQ_POP
		delete tmp_netcache_getreq_pop_ptr;
		tmp_netcache_getreq_pop_ptr = NULL;
	}

	switchos_dppopserver_finish = true;
	close(switchos_dppopserver_udpsock);
	close(switchos_dppopserver_popclient_for_controller_udpsock);
	pthread_exit(nullptr);
}

void *run_switchos_cppopserver(void *param) {
	struct sockaddr_in controller_popserver_popclient_addr;
	socklen_t controller_popserver_popclient_addrlen = sizeof(struct sockaddr_in);

	printf("[switchos.cppopserver] ready\n");
	switchos_ready_threads++;

	while (!switchos_running) {}

	// Process CACHE_POP packet <optype, key, vallen, value, seq, serveridx>
	char buf[MAX_BUFSIZE];
	int recvsize = 0;
	while (switchos_running) {
		udprecvfrom(switchos_cppopserver_udpsock, buf, MAX_BUFSIZE, 0, &controller_popserver_popclient_addr, &controller_popserver_popclient_addrlen, recvsize, "switchos.dppopserver");

		//printf("receive CACHE_POP from controller\n");
		//dump_buf(buf, recvsize);

		cache_pop_t *tmp_cache_pop_ptr = new cache_pop_t(CURMETHOD_ID, buf, recvsize); // freed by switchos.popworker

		// send CACHE_POP_ACK to controller.popserver.popclient_for_spine/leaf immediately to avoid timeout
		cache_pop_ack_t tmp_cache_pop_ack(CURMETHOD_ID, tmp_cache_pop_ptr->key());
		uint32_t acksize = tmp_cache_pop_ack.serialize(buf, MAX_BUFSIZE);
		udpsendto(switchos_cppopserver_udpsock, buf, acksize, 0, &controller_popserver_popclient_addr, controller_popserver_popclient_addrlen, "switchos.cppopserver");

		// notify switchos.popworker for cache population/eviction
		bool res = switchos_cache_pop_ptr_queue.write(tmp_cache_pop_ptr);
		if (!res) {
			printf("[switch os] message queue overflow of switchos.switchos_cache_pop_ptr_queue!");
		}
	}

	close(switchos_cppopserver_udpsock);
	pthread_exit(nullptr);
}

void *run_switchos_popworker(void *param) {
	// used by server-leaf switchos for DISTCACHE_CACHE_EVICT_VICTIM/_ACK
	struct sockaddr_in controller_victimserver_victimclient_for_leaf_addr;
	socklen_t controller_victimserver_victimclient_for_leaf_addrlen = sizeof(struct sockaddr_in);

	// used by spine switchos for DISTCACHE_CACHE_EVICT_VICTIM/_ACK
	struct sockaddr_in controller_victimserver_addr;
	set_sockaddr(controller_victimserver_addr, inet_addr(controller_ip_for_switchos), controller_victimserver_port);
	socklen_t controller_victimserver_addrlen = sizeof(struct sockaddr_in);

	// used for NETCACHE_CACHE_POP_FINISH
	struct sockaddr_in controller_popserver_addr;
	set_sockaddr(controller_popserver_addr, inet_addr(controller_ip_for_switchos), controller_popserver_port_start);
	socklen_t controller_popserver_addrlen = sizeof(struct sockaddr_in);

	// used by udp socket for cache population
	sockaddr_in reflector_cp2dpserver_addr;
	set_sockaddr(reflector_cp2dpserver_addr, inet_addr(reflector_ip_for_switchos), reflector_cp2dpserver_port);
	int reflector_cp2dpserver_addr_len = sizeof(struct sockaddr);

	// used by udpsocket to communicate with ptf.popserver
	sockaddr_in ptf_popserver_addr;
	set_sockaddr(ptf_popserver_addr, inet_addr("127.0.0.1"), switchos_ptf_popserver_port);
	int ptf_popserver_addr_len = sizeof(struct sockaddr);

	// used by popworker.evictclient
	sockaddr_in controller_evictserver_addr;
	set_sockaddr(controller_evictserver_addr, inet_addr(controller_ip_for_switchos), controller_evictserver_port);
	socklen_t controller_evictserver_addrlen = sizeof(struct sockaddr_in);

	// get valid server logical idxes (TMPDEBUG)
	std::vector<uint16_t> valid_global_server_logical_idxes;
	for (int i = 0; i < server_physical_num; i++) {
		for (int j = 0; j < server_logical_idxes_list[i].size(); j++) {
			uint16_t tmp_global_server_logical_idx = server_logical_idxes_list[i][j];
			valid_global_server_logical_idxes.push_back(tmp_global_server_logical_idx);
		}
	}

	printf("[switchos.popworker] ready\n");
	switchos_ready_threads++;

	while (!switchos_running) {}

	// (1) communicate with controller.victimserver or controller.evictserver or reflector.cp2dpserer
	// send CACHE_POP_INSWITCH and CACHE_EVICT to controller
	// send/receive DISTCACHE_CACHE_EVICT_VICTIM/_ACK
	char pktbuf[MAX_BUFSIZE];
	int pktsize = 0;
	// recv CACHE_POP_INSWITCH_ACK and CACHE_EVICT_ACK from controller
	char ackbuf[MAX_BUFSIZE];
	int ack_recvsize = 0;
	// (2) communicate with ptf.popserver
	char ptfbuf[MAX_BUFSIZE];
	uint32_t ptf_sendsize = 0;
	int ptf_recvsize = 0;
	uint16_t switchos_freeidx = 0;
	// load evictdata from ptf
	uint16_t switchos_evictidx = 0;
	val_t switchos_evictvalue = val_t();
	uint32_t switchos_evictseq = 0;
	bool switchos_evictstat = false;

	// TMPDEBUG
	//std::vector<double> pop_setvalid0_time_list, pop_cachepop_time_list, pop_addkey_time_list, pop_setvalid1_time_list, pop_total_time_list;
	//struct timespec pop_setvalid0_t1, pop_setvalid0_t2, pop_setvalid0_t3, pop_cachepop_t1, pop_cachepop_t2, pop_cachepop_t3, pop_addkey_t1, pop_addkey_t2, pop_addkey_t3, pop_setvalid1_t1, pop_setvalid1_t2, pop_setvalid1_t3, pop_total_t1, pop_total_t2, pop_total_t3;
	//std::vector<double> evict_load_time_list, evict_sendrecv_time_list, evict_remove_time_list, evict_total_time_list;
	//struct timespec evict_load_t1, evict_load_t2, evict_load_t3, evict_sendrecv_t1, evict_sendrecv_t2, evict_sendrecv_t3, evict_remove_t1, evict_remove_t2, evict_remove_t3, evict_total_t1, evict_total_t2, evict_total_t3;

	while (switchos_running) {
		cache_pop_t *tmp_cache_pop_ptr = switchos_cache_pop_ptr_queue.read();
		if (tmp_cache_pop_ptr != NULL) {
				
			//CUR_TIME(pop_total_t1); // TMPDEBUG

			uint32_t tmp_global_server_logical_idx = tmp_cache_pop_ptr->serveridx();
			
			// TMPDEBUG
			bool is_valid = false;
			for (int i = 0; i < valid_global_server_logical_idxes.size(); i++) {
				if (tmp_global_server_logical_idx == valid_global_server_logical_idxes[i]) {
					is_valid = true;
					break;
				}
			}
			INVARIANT(is_valid == true);
			
			if (switchos_cached_keyidx_map.find(tmp_cache_pop_ptr->key()) != switchos_cached_keyidx_map.end()) {
				// NOTE: given the switch A reporting current hot key, if the key was not reported by switch A before, dppopserver of switch A cannot filter cout current hot key, which incurs duplicate CACHE_POP requests
				//printf("[switchos.popworker] Message: key %x already cached at kvidx %d\n", tmp_cache_pop_ptr->key().keyhihi, switchos_cached_keyidx_map[tmp_cache_pop_ptr->key()]);
				//fflush(stdout);
				continue; // skip the key already cached in switch
				
				//printf("Error: populating a key %x cached at kvidx %u from switch\n", tmp_cache_pop_ptr->key().keyhihi, switchos_cached_keyidx_map[tmp_cache_pop_ptr->key()]);
				//fflush(stdout);
				//exit(-1);
			}

			// find corresponding pipeline idx
			uint32_t tmp_pipeidx = 0;
			if (strcmp(switchos_role, "spine") == 0) {
				tmp_pipeidx = leafswitch_pipeidx;
			}
			else if (strcmp(switchos_role, "leaf") == 0) {
				int tmp_server_physical_idx = -1;
				for (int i = 0; i < server_physical_num; i++) {
					for (int j = 0; j < server_logical_idxes_list[i].size(); j++) {
						if (server_logical_idxes_list[i][j] == tmp_global_server_logical_idx) {
							tmp_server_physical_idx = i;
							break;
						}
					}
				}
				INVARIANT(tmp_server_physical_idx != -1);
				tmp_pipeidx = server_pipeidxes[tmp_server_physical_idx];
			}
			else {
				printf("[switchos.popworker] invalid switchos role: %s\n", switchos_role);
				fflush(stdout);
				exit(-1);
			}

			// assign switchos_freeidx for new record 
			if (switchos_perpipeline_cached_empty_index[tmp_pipeidx] < switch_kv_bucket_num) { // With free idx
				switchos_freeidx = switchos_perpipeline_cached_empty_index[tmp_pipeidx];
				switchos_perpipeline_cached_empty_index[tmp_pipeidx] += 1;
			}
			else { // Without free idx
				if (workload_mode == 1) { // dynamic pattern does not need cache eviction
					// delete tmp_netcache_getreq_pop_ptr;
					// tmp_netcache_getreq_pop_ptr = NULL;
					continue;
				}

				//CUR_TIME(evict_total_t1); // TMPDEBUG

				//CUR_TIME(evict_load_t1); // TMPDEBUG
				
				// get evictdata from ptf framework 
				////system("bash tofino/get_evictdata_setvalid3.sh");
				/*ptf_sendsize = serialize_get_evictdata_setvalid3(ptfbuf, tmp_pipeidx);
				udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
				udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
				INVARIANT(*((int *)ptfbuf) == SWITCHOS_GET_EVICTDATA_SETVALID3_ACK); // wait for SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK
				parse_evictdata(ptfbuf, ptf_recvsize, switchos_evictidx, switchos_evictvalue, switchos_evictseq, switchos_evictstat);*/

				if (strcmp(switchos_role, "spine") == 0) { // spine.popworker
					// generate sampled indexes
					uint32_t sampled_idxes[switchos_sample_cnt];
					memset(sampled_idxes, 0, sizeof(uint32_t) * switchos_sample_cnt);
					for (size_t i = 0; i < switchos_sample_cnt; i++) {
						sampled_idxes[i] = rand() % switch_kv_bucket_num; // [0, switch_kv_bucket_num - 1]
					}

					// load frequency counters from data plane
					uint32_t frequency_counters[switchos_sample_cnt];
					memset(frequency_counters, 0, sizeof(uint32_t) * switchos_sample_cnt);
					while (true) {
						//printf("send %d CACHE_EVICT_LOADFREQ_INSWITCHs to reflector\n", switchos_sample_cnt);
						for (size_t i = 0; i < switchos_sample_cnt; i++) {
							uint16_t spineswitchidx_for_sampled_key = 0, leafswitchidx_for_sampled_key = 0;
							calculate_switchidx(static_cast<netreach_key_t>(switchos_perpipeline_cached_keyarray[tmp_pipeidx][sampled_idxes[i]]), spineswitchidx_for_sampled_key, leafswitchidx_for_sampled_key);
							cache_evict_loadfreq_inswitch_t tmp_cache_evict_loadfreq_inswitch_req(spineswitchidx_for_sampled_key, leafswitchidx_for_sampled_key, switchos_perpipeline_cached_keyarray[tmp_pipeidx][sampled_idxes[i]], sampled_idxes[i]);
							pktsize = tmp_cache_evict_loadfreq_inswitch_req.serialize(pktbuf, MAX_BUFSIZE);
							udpsendto(switchos_popworker_popclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.popworker.popclient_for_reflector");
						}

						// loop until receiving corresponding ACK (ignore unmatched ACKs which are duplicate ACKs of previous cache population)
						bool is_timeout = false;
						int tmp_acknum = 0;
						while (true) {
							is_timeout = udprecvfrom(switchos_popworker_popclient_for_reflector_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.popclient_for_reflector");
							if (unlikely(is_timeout)) {
								break;
							}

							cache_evict_loadfreq_inswitch_ack_t tmp_cache_evict_loadfreq_inswitch_ack(CURMETHOD_ID, ackbuf, ack_recvsize);
							for (size_t i = 0; i < switchos_sample_cnt; i++) {
								if (static_cast<netreach_key_t>(switchos_perpipeline_cached_keyarray[tmp_pipeidx][sampled_idxes[i]]) == tmp_cache_evict_loadfreq_inswitch_ack.key()) {
									frequency_counters[i] = tmp_cache_evict_loadfreq_inswitch_ack.frequency();
									tmp_acknum += 1;
									break;
								}
								else if (i == switchos_sample_cnt - 1) {
									printf("Receive CACHE_EVICT_LOADFREQ_ACK of key %x, not match any sampled key from %d CACHE_EVICT_LOADFREQ_INSWITCH_ACKs!\n", tmp_cache_evict_loadfreq_inswitch_ack.key().keyhihi, switchos_sample_cnt);
									fflush(stdout);
									exit(-1);
								}
							}

							if (tmp_acknum >= switchos_sample_cnt) {
								break;
							}
						}

						if (unlikely(is_timeout)) {
							continue;
						}
						else {
							break;
						}
					}

					// choose the idx with minimum frequency counter as victim
					uint32_t min_frequency_counter = 0;
					for (size_t i = 0; i < switchos_sample_cnt; i++) {
						if ((i == 0) || (min_frequency_counter > frequency_counters[i])) {
							min_frequency_counter = frequency_counters[i];
							switchos_evictidx = sampled_idxes[i]; // NOTE we should use switchos_evictidx with the scope outside current if block
						}
					}
					
					// send DISTCACHE_CACHE_EVICT_VICTIM to controller.victimserver
					netreach_key_t tmp_evictkey = switchos_perpipeline_cached_keyarray[tmp_pipeidx][switchos_evictidx];
					distcache_cache_evict_victim_t tmp_distcache_cache_evict_victim(CURMETHOD_ID, tmp_cache_pop_ptr->key(), tmp_evictkey, switchos_evictidx);
					while (true) {
						pktsize = tmp_distcache_cache_evict_victim.serialize(pktbuf, MAX_BUFSIZE);
						udpsendto(spineswitchos_popworker_victimclient_for_controller_udpsock, pktbuf, pktsize, 0, &controller_victimserver_addr, controller_victimserver_addrlen, "spineswitchos.popworker.victimclient_for_controller");

						bool is_timeout = udprecvfrom(spineswitchos_popworker_victimclient_for_controller_udpsock, pktbuf, MAX_BUFSIZE, 0, NULL, NULL, pktsize, "spineswitchos.popworker.victimclient_for_controller");
						if (unlikely(is_timeout)) {
							continue;
						}
						else {
							distcache_cache_evict_victim_ack_t tmp_distcache_cache_evict_victim_ack(CURMETHOD_ID, pktbuf, pktsize);
							INVARIANT(tmp_distcache_cache_evict_victim_ack.key() == tmp_distcache_cache_evict_victim.key());
							break;
						}
					}
				}
				else if (strcmp(switchos_role, "leaf") == 0) { // server-leaf.popworker
					while (true) {
						// wait for DISTCACHE_CACHE_EVICT_VICTIM
						udprecvfrom(leafswitchos_popworker_victimserver_udpsock, pktbuf, MAX_BUFSIZE, 0, &controller_victimserver_victimclient_for_leaf_addr, &controller_victimserver_victimclient_for_leaf_addrlen, pktsize, "leafswitchos.popworker.victimserver");

						distcache_cache_evict_victim_t tmp_distcache_cache_evict_victim(CURMETHOD_ID, pktbuf, pktsize);

						// validate victim key and idx
						// TODO: launch leafswitchos.popworker.victimserver as an individual thread -> for duplicate DISTCACHE_CACHE_EVICT_VICTIM caused by timeout-and-retry in spineswitchos, individual victimserver can ignore it for robustness
						if (tmp_distcache_cache_evict_victim.key() == tmp_cache_pop_ptr->key()) {
							INVARIANT(tmp_distcache_cache_evict_victim.victimkey() == static_cast<netreach_key_t>(switchos_perpipeline_cached_keyarray[tmp_pipeidx][tmp_distcache_cache_evict_victim.victimidx()]));
							switchos_evictidx = tmp_distcache_cache_evict_victim.victimidx();

							// send DISTCACHE_CACHE_EVICT_VICTIM_ACK to controller.vicitmserver.victimclient_for_leaf
							distcache_cache_evict_victim_ack_t tmp_distcache_cache_evict_victim_ack(CURMETHOD_ID, tmp_distcache_cache_evict_victim.key());
							pktsize = tmp_distcache_cache_evict_victim_ack.serialize(pktbuf, MAX_BUFSIZE);

							udpsendto(leafswitchos_popworker_victimserver_udpsock, pktbuf, pktsize, 0, &controller_victimserver_victimclient_for_leaf_addr, controller_victimserver_victimclient_for_leaf_addrlen, "leafswitchos.popworker.victimserver");
							break;
						}
					}
				}
				else {
					printf("[switchos.popworker] invalid switchos role: %s\n", switchos_role);
					fflush(stdout);
					exit(-1);
				}

				// validate switchos_evictidx and cur_evictkey
				INVARIANT(switchos_evictidx >= 0 && switchos_evictidx < switch_kv_bucket_num);
				netreach_key_t cur_evictkey = switchos_perpipeline_cached_keyarray[tmp_pipeidx][switchos_evictidx];
				if (switchos_cached_keyidx_map.find(cur_evictkey) == switchos_cached_keyidx_map.end()) {
					printf("Evicted key %x at kvidx %d is not cached\n", switchos_perpipeline_cached_keyarray[tmp_pipeidx][switchos_evictidx].keyhihi, switchos_evictidx);
					fflush(stdout);
					exit(-1);
				}

				// TMPDEBUG
				//printf("Evict key %ld for new hot key %ld\n", ((uint64_t)cur_evictkey.keyhihi)<<32 | ((uint64_t)cur_evictkey.keyhilo), ((uint64_t)tmp_cache_pop_ptr->key().keyhihi)<<32 | ((uint64_t)tmp_cache_pop_ptr->key().keyhilo));
				//fflush(stdout);

				//printf("switchos_evictidx: %d, evictkey %x -> newkey %x\n", switchos_evictidx, cur_evictkey.keyhihi, tmp_cache_pop_ptr->key().keyhihi);
				//fflush(stdout);

				//CUR_TIME(evict_load_t2); // TMPDEBUG

				// calculate correpsonding switchidx
				uint16_t switchidx_for_cur_evictkey = calculate_switchidx_by_role(cur_evictkey);

				//CUR_TIME(evict_remove_t1); // TMPDEBUG
				// remove evicted data from cache_lookup_tbl
				//system("bash tofino/remove_cache_lookup.sh");
				ptf_sendsize = serialize_remove_cache_lookup(ptfbuf, cur_evictkey, switchidx_for_cur_evictkey);
				udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
				udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
				INVARIANT(*((int *)ptfbuf) == SWITCHOS_REMOVE_CACHE_LOOKUP_ACK); // wait for SWITCHOS_REMOVE_CACHE_LOOKUP_ACK
				//CUR_TIME(evict_remove_t2); // TMPDEBUG

				// switchos.popworker.evictclient sends CACHE_EVICT to controller.evictserver
				
				//CUR_TIME(evict_sendrecv_t1); // TMPDEBUG
				netcache_cache_evict_t tmp_netcache_cache_evict(CURMETHOD_ID, cur_evictkey, switchos_perpipeline_cached_serveridxarray[tmp_pipeidx][switchos_evictidx]);
				pktsize = tmp_netcache_cache_evict.serialize(pktbuf, MAX_BUFSIZE);
				while (true) {
					//printf("send NETCACHE_CACHE_EVICT to controller.evictserver\n");
					//dump_buf(pktbuf, pktsize);
					udpsendto(switchos_popworker_evictclient_for_controller_udpsock, pktbuf, pktsize, 0, &controller_evictserver_addr, controller_evictserver_addrlen, "switchos.popworker.evictclient_for_controller");

					// wait for NETCACHE_CACHE_EVICT_ACK from controller.evictserver
					// NOTE: no concurrent CACHE_EVICTs -> use request-and-reply manner to wait for entire eviction workflow
					bool is_timeout = udprecvfrom(switchos_popworker_evictclient_for_controller_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.evictclient_for_controller");
					if (unlikely(is_timeout)) {
						continue;
					}
					else {
						netcache_cache_evict_ack_t tmp_cache_evict_ack(CURMETHOD_ID, ackbuf, ack_recvsize);
						INVARIANT(tmp_cache_evict_ack.key() == cur_evictkey);
						break;
					}
				}
				//CUR_TIME(evict_sendrecv_t2); // TMPDEBUG

				//printf("Evict %x to %x\n", cur_evictkey.keyhihi, tmp_cache_pop_ptr->key().keyhihi);

				// set freeidx as evictidx for cache popluation later
				switchos_freeidx = switchos_evictidx;

				// reset keyarray and serveridxarray at evictidx
				switchos_cached_keyidx_map.erase(cur_evictkey);
				switchos_perpipeline_cached_keyarray[tmp_pipeidx][switchos_evictidx] = netreach_key_t();
				switchos_perpipeline_cached_serveridxarray[tmp_pipeidx][switchos_evictidx] = -1;

				// update popserver.cachedkeyset atomically
				mutex_for_cached_keyset.lock();
			//bool tmpbool2 = switchos_dppopserver_cached_keyset.find(cur_evictkey) == switchos_dppopserver_cached_keyset.end();
			//COUT_VAR(tmpbool2)
			//fflush(stdout);
				if (switchos_dppopserver_cached_keyset.find(cur_evictkey) != switchos_dppopserver_cached_keyset.end()) {
					// NOTE: the evicted key may NOT be reported by dppopserver of current switchos -> NOT exist in switchos_dppopserver_cached_keyset
					switchos_dppopserver_cached_keyset.erase(cur_evictkey);
				}
				mutex_for_cached_keyset.unlock();

				//CUR_TIME(evict_total_t2); // TMPDEBUG

				// TMPDEBUG
				/*DELTA_TIME(evict_load_t2, evict_load_t1, evict_load_t3);
				DELTA_TIME(evict_sendrecv_t2, evict_sendrecv_t1, evict_sendrecv_t3);
				DELTA_TIME(evict_remove_t2, evict_remove_t1, evict_remove_t3);
				DELTA_TIME(evict_total_t2, evict_total_t1, evict_total_t3);
				evict_load_time_list.push_back(GET_MICROSECOND(evict_load_t3));
				evict_sendrecv_time_list.push_back(GET_MICROSECOND(evict_sendrecv_t3));
				evict_remove_time_list.push_back(GET_MICROSECOND(evict_remove_t3));
				evict_total_time_list.push_back(GET_MICROSECOND(evict_total_t3));

				if ((evict_total_time_list.size() + 1) % 100 == 0) {
					double load_time = 0, sendrecv_time = 0, remove_time = 0, total_time = 0;
					for (size_t i = 0; i < evict_total_time_list.size(); i++) {
						load_time += evict_load_time_list[i];
						sendrecv_time += evict_sendrecv_time_list[i];
						remove_time += evict_remove_time_list[i];
						total_time += evict_total_time_list[i];
					}
					int tmpsize = evict_total_time_list.size();
					printf("average load time: %f, sendrecv time: %f, remove time: %f, total time: %f\n", load_time/tmpsize, sendrecv_time/tmpsize, remove_time/tmpsize, total_time/tmpsize);
				}*/
			}

			/* cache population for new record */

			INVARIANT(switchos_freeidx >= 0 && switchos_freeidx < switch_kv_bucket_num);
			//printf("[switchos.popworker] switchos_perpipeline_cached_empty_index[%d]: %d, switchos_freeidx: %d\n", tmp_pipeidx, int(switchos_perpipeline_cached_empty_index[tmp_pipeidx]), int(switchos_freeidx)); // TMPDEBUG
			
			//CUR_TIME(pop_cachepop_t1); // TMPDEBUG

			uint16_t spineswitchidx_for_tmp_cache_pop_key = 0, leafswitchidx_for_tmp_cache_pop_key = 0;
			calculate_switchidx(tmp_cache_pop_ptr->key(), spineswitchidx_for_tmp_cache_pop_key, leafswitchidx_for_tmp_cache_pop_key);

			// send CACHE_POP_INSWITCH to reflector (TODO: try internal pcie port)
			cache_pop_inswitch_t tmp_cache_pop_inswitch(CURMETHOD_ID, spineswitchidx_for_tmp_cache_pop_key, leafswitchidx_for_tmp_cache_pop_key, tmp_cache_pop_ptr->key(), tmp_cache_pop_ptr->val(), tmp_cache_pop_ptr->seq(), switchos_freeidx, tmp_cache_pop_ptr->stat());
			pktsize = tmp_cache_pop_inswitch.serialize(pktbuf, MAX_BUFSIZE);

			while (true) {
				udpsendto(switchos_popworker_popclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.popworker.popclient");

				// loop until receiving corresponding ACK (ignore unmatched ACKs which are duplicate ACKs of previous cache population)
				bool is_timeout = false;
				bool with_correctack = false;
				while (true) {
					is_timeout = udprecvfrom(switchos_popworker_popclient_for_reflector_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.popclient");
					if (unlikely(is_timeout)) {
						break;
					}

					cache_pop_inswitch_ack_t tmp_cache_pop_inswitch_ack(CURMETHOD_ID, ackbuf, ack_recvsize);
					if (tmp_cache_pop_inswitch_ack.key() == tmp_cache_pop_inswitch.key()) {
						with_correctack = true;
						break;
					}
				}

				if (unlikely(is_timeout)) {
					continue;
				}
				if (with_correctack) {
					break;
				}
			}

			//CUR_TIME(pop_cachepop_t2); // TMPDEBUG
			//CUR_TIME(pop_addkey_t1); // TMPDEBUG

			// calculate corresponding switchidx based on role
			uint16_t switchidx_for_tmp_cache_pop_key = calculate_switchidx_by_role(tmp_cache_pop_ptr->key());

			// (1) add new <key, value> pair into cache_lookup_tbl; DEPRECATED: (2) and set valid=1 to enable the entry
			////system("bash tofino/add_cache_lookup_setvalid1.sh");
			//ptf_sendsize = serialize_add_cache_lookup_setvalid1(ptfbuf, tmp_cache_pop_ptr->key(), switchos_freeidx, tmp_pipeidx);
			ptf_sendsize = serialize_add_cache_lookup(ptfbuf, tmp_cache_pop_ptr->key(), switchidx_for_tmp_cache_pop_key, switchos_freeidx);
			udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
			udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
			//INVARIANT(*((int *)ptfbuf) == SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK); // wait for SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK
			INVARIANT(*((int *)ptfbuf) == SWITCHOS_ADD_CACHE_LOOKUP_ACK); // wait for SWITCHOS_ADD_CACHE_LOOKUP_ACK

			//CUR_TIME(pop_addkey_t2); // TMPDEBUG

			// update inswitch cache metadata
			switchos_cached_keyidx_map.insert(std::pair<netreach_key_t, uint32_t>(tmp_cache_pop_ptr->key(), switchos_freeidx));
			switchos_perpipeline_cached_keyarray[tmp_pipeidx][switchos_freeidx] = tmp_cache_pop_ptr->key();
			switchos_perpipeline_cached_serveridxarray[tmp_pipeidx][switchos_freeidx] = tmp_global_server_logical_idx;

			// send NETCACHE_CACHE_POP_FINISH to server and wait for ACK
			netcache_cache_pop_finish_t tmp_netcache_cache_pop_finish(CURMETHOD_ID, tmp_cache_pop_ptr->key(), tmp_global_server_logical_idx, switchos_freeidx);
			pktsize = tmp_netcache_cache_pop_finish.serialize(pktbuf, MAX_BUFSIZE);
			while (true) {
				udpsendto(switchos_popworker_popclient_for_controller_udpsock, pktbuf, pktsize, 0, &controller_popserver_addr, controller_popserver_addrlen, "switchos.popworker.popclient_for_controller");

				bool is_timeout = udprecvfrom(switchos_popworker_popclient_for_controller_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.popclient_for_controller");
				if (unlikely(is_timeout)) {
					continue;
				}
				else {
					netcache_cache_pop_finish_ack_t tmp_netcache_cache_pop_finish_ack(CURMETHOD_ID, ackbuf, ack_recvsize);
					//INVARIANT(tmp_netcache_cache_pop_finish_ack.key() == tmp_netcache_cache_pop_finish.key());
					if (unlikely(!(tmp_netcache_cache_pop_finish_ack.key() == tmp_netcache_cache_pop_finish.key()))) {
						printf("Unmatched key of NETCACHE_CACHE_POP_FINISH_ACK\n");
						continue;
					}
					break;
				}
			}

			// TMPDEBUG
			/*CUR_TIME(pop_total_t2);
			DELTA_TIME(pop_cachepop_t2, pop_cachepop_t1, pop_cachepop_t3);
			pop_cachepop_time_list.push_back(GET_MICROSECOND(pop_cachepop_t3));
			DELTA_TIME(pop_addkey_t2, pop_addkey_t1, pop_addkey_t3);
			pop_addkey_time_list.push_back(GET_MICROSECOND(pop_addkey_t3));
			DELTA_TIME(pop_total_t2, pop_total_t1, pop_total_t3);
			pop_total_time_list.push_back(GET_MICROSECOND(pop_total_t3));
			if (((pop_total_time_list.size() + 1) % 100) == 0) {
				double pop_cachepop_time = 0.0, pop_addkey_time = 0.0, pop_total_time = 0.0;
				for (size_t i = 0; i < pop_total_time_list.size(); i++) {
					pop_cachepop_time += pop_cachepop_time_list[i];
					pop_addkey_time += pop_addkey_time_list[i];
					pop_total_time += pop_total_time_list[i];
				}
				pop_cachepop_time /= pop_total_time_list.size();
				pop_addkey_time /= pop_total_time_list.size();
				pop_total_time /= pop_total_time_list.size();
				printf("average pop cachepop time: %f, addkey time: %f, total time: %f\n", pop_cachepop_time, pop_addkey_time, pop_total_time);
				fflush(stdout);
			}*/

			// free NETCACHE_GETREQ_POP
			delete tmp_cache_pop_ptr;
			tmp_cache_pop_ptr = NULL;

			// reset metadata for popclient_for_ptf
			ptf_sendsize = 0;
			ptf_recvsize = 0;
			switchos_freeidx = 0;
			switchos_evictidx = 0;
			switchos_evictvalue = val_t();
			switchos_evictseq = 0;
			switchos_evictstat = false;
		} // tmp_cache_pop_ptr != NULL
	}

	// send SWITCHOS_PTF_POPSERVER_END to ptf.popserver
	memcpy(ptfbuf, &SWITCHOS_PTF_POPSERVER_END, sizeof(int));
	udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");

	if (strcmp(switchos_role, "spine") == 0) {
		close(spineswitchos_popworker_victimclient_for_controller_udpsock);
	}
	else if (strcmp(switchos_role, "leaf") == 0) {
		close(leafswitchos_popworker_victimserver_udpsock);
	}
	else {
		printf("[switchos.popworker] invalid switchos role %s\n", switchos_role);
		fflush(stdout);
		exit(-1);
	}
	close(switchos_popworker_popclient_for_controller_udpsock);
	close(switchos_popworker_popclient_for_reflector_udpsock);
	close(switchos_popworker_evictclient_for_controller_udpsock);
	close(switchos_popworker_popclient_for_ptf_udpsock);
	pthread_exit(nullptr);
}

void calculate_switchidx(netreach_key_t key, uint16_t &spineswitchidx, uint16_t &leafswitchidx) {
	spineswitchidx = uint16_t(key.get_spineswitch_idx(switch_partition_count, spineswitch_total_logical_num));
	leafswitchidx = uint16_t(key.get_leafswitch_idx(switch_partition_count, max_server_total_logical_num, leafswitch_total_logical_num, spineswitch_total_logical_num));
	return;
}

uint16_t calculate_switchidx_by_role(netreach_key_t key) {
	uint16_t tmp_switchidx = 0;
	if (strcmp(switchos_role, "spine") == 0) {
		tmp_switchidx = uint16_t(key.get_spineswitch_idx(switch_partition_count, spineswitch_total_logical_num));
	}
	else if (strcmp(switchos_role, "leaf") == 0) {
		tmp_switchidx = uint16_t(key.get_leafswitch_idx(switch_partition_count, max_server_total_logical_num, leafswitch_total_logical_num, spineswitch_total_logical_num));
	}
	else {
		printf("[switchos.popworker] invalid switchos role: %s\n", switchos_role);
		fflush(stdout);
		exit(-1);
	}
	return tmp_switchidx;
}

void close_switchos() {
	/*if (switchos_cache_pop_ptrs != NULL) {
		for (size_t i = 0; i < MQ_SIZE; i++) {
			if (switchos_cache_pop_ptrs[i] != NULL) {
				delete switchos_cache_pop_ptrs[i];
				switchos_cache_pop_ptrs[i] = NULL;
			}
		}
		delete [] switchos_cache_pop_ptrs;
		switchos_cache_pop_ptrs = NULL;
	}*/
	if (switchos_perpipeline_cached_keyarray != NULL) {
		for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
			if (switchos_perpipeline_cached_keyarray[tmp_pipeidx] != NULL) {
				delete [] switchos_perpipeline_cached_keyarray[tmp_pipeidx];
				switchos_perpipeline_cached_keyarray[tmp_pipeidx] = NULL;
			}
		}
		delete [] switchos_perpipeline_cached_keyarray;
		switchos_perpipeline_cached_keyarray = NULL;
	}
	if (switchos_perpipeline_cached_serveridxarray != NULL) {
		for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
			if (switchos_perpipeline_cached_serveridxarray[tmp_pipeidx] != NULL) {
				delete [] switchos_perpipeline_cached_serveridxarray[tmp_pipeidx];
				switchos_perpipeline_cached_serveridxarray[tmp_pipeidx] = NULL;
			}
		}
		delete [] switchos_perpipeline_cached_serveridxarray;
		switchos_perpipeline_cached_serveridxarray = NULL;
	}
}

// switchos <-> ptf.popserver

/*inline uint32_t serialize_setvalid0(char *buf, uint16_t freeidx, uint32_t pipeidx) {
	memcpy(buf, &SWITCHOS_SETVALID0, sizeof(int));
	memcpy(buf + sizeof(int), &freeidx, sizeof(uint16_t));
	memcpy(buf + sizeof(int) + sizeof(uint16_t), &pipeidx, sizeof(uint32_t));
	return sizeof(int) + sizeof(uint16_t) + sizeof(uint32_t);
}*/

//inline uint32_t serialize_add_cache_lookup_setvalid1(char *buf, netreach_key_t key, uint16_t freeidx, uint32_t pipeidx) {
inline uint32_t serialize_add_cache_lookup(char *buf, netreach_key_t key, uint16_t switchidx, uint16_t freeidx) {
	memcpy(buf, &SWITCHOS_ADD_CACHE_LOOKUP, sizeof(int));
	uint32_t tmp_keysize = key.serialize(buf + sizeof(int), MAX_BUFSIZE - sizeof(int));
	memcpy(buf + sizeof(int) + tmp_keysize, &switchidx, sizeof(uint16_t));
	memcpy(buf + sizeof(int) + tmp_keysize + sizeof(uint16_t), &freeidx, sizeof(uint16_t));
	return sizeof(int) + tmp_keysize + sizeof(uint16_t) + sizeof(uint16_t);
	//memcpy(buf + sizeof(int) + tmp_keysize + sizeof(uint16_t), &pipeidx, sizeof(uint32_t));
	//return sizeof(int) + tmp_keysize + sizeof(uint16_t) + sizeof(uint32_t);
}

/*inline uint32_t serialize_setvalid3(char *buf, uint16_t evictidx, uint32_t pipeidx) {
	memcpy(buf, &SWITCHOS_SETVALID3, sizeof(int));
	memcpy(buf + sizeof(int), &evictidx, sizeof(uint16_t));
	memcpy(buf + sizeof(int) + sizeof(uint16_t), &pipeidx, sizeof(uint32_t));
	return sizeof(int) + sizeof(uint16_t) + sizeof(uint32_t);
}*/

/*inline uint32_t serialize_get_evictdata_setvalid3(char *buf, uin32_t pipeidx) {
	memcpy(buf, &SWITCHOS_GET_EVICTDATA_SETVALID3, sizeof(int));
	memcpy(buf + sizeof(int), &pipeidx, sizeof(uint32_t));
	return sizeof(int) + sizeof(uint32_t);
}

inline void parse_evictdata(char *buf, int recvsize, uint16_t &switchos_evictidx, val_t &switchos_evictvalue, uint32_t &switchos_evictseq, bool &switchos_evictstat) {
	char * curptr = buf + sizeof(int);
	switchos_evictidx = *((uint16_t *)curptr);
	curptr += sizeof(uint16_t);
	uint32_t tmp_valsize = switchos_evictvalue.deserialize(curptr, recvsize - sizeof(int) - sizeof(uint16_t));
	curptr += tmp_valsize;
	switchos_evictseq = *((uint32_t *)curptr);
	curptr += sizeof(uint32_t);
	switchos_evictstat = *((bool *)curptr);
}*/

inline uint32_t serialize_remove_cache_lookup(char *buf, netreach_key_t key, uint16_t switchidx) {
	memcpy(buf, &SWITCHOS_REMOVE_CACHE_LOOKUP, sizeof(int));
	uint32_t tmp_keysize = key.serialize(buf + sizeof(int), MAX_BUFSIZE - sizeof(int));
	memcpy(buf + sizeof(int) + tmp_keysize, &switchidx, sizeof(uint16_t));
	return sizeof(int) + tmp_keysize + sizeof(uint16_t);
}
