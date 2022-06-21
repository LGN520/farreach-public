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

#include "helper.h"
#include "io_helper.h"
#include "key.h"
#include "val.h"
//#include "common_impl.h"
//
#include "socket_helper.h"
#include "special_case.h"
#include "iniparser/iniparser_wrapper.h"
#include "message_queue_impl.h"
#include "packet_format_impl.h"
#include "concurrent_map_impl.h"
#include "dynamic_array.h"

// ptf scripts used by switchos
// cache eviction: get_evictdata_setvalid3.sh, remove_cache_lookup.sh
// cache population: setvalid0.sh, add_cache_lookup_setvalid1.sh
// snapshot: set_snapshot_flag.sh, load_snapshot_data.sh, reset_snapshot_flag_and_reg.sh

typedef Key netreach_key_t;
typedef Val val_t;
typedef SpecialCase special_case_t;
typedef GetResponseLatestSeqInswitchCase1<netreach_key_t, val_t> getres_latest_seq_inswitch_case1_t;
typedef GetResponseDeletedSeqInswitchCase1<netreach_key_t, val_t> getres_deleted_seq_inswitch_case1_t;
typedef PutRequestSeqInswitchCase1<netreach_key_t, val_t> putreq_seq_inswitch_case1_t;
typedef DelRequestSeqInswitchCase1<netreach_key_t, val_t> delreq_seq_inswitch_case1_t;
typedef CachePop<netreach_key_t, val_t> cache_pop_t;
typedef CachePopInswitch<netreach_key_t, val_t> cache_pop_inswitch_t;
typedef CachePopInswitchAck<netreach_key_t> cache_pop_inswitch_ack_t;
typedef CacheEvict<netreach_key_t, val_t> cache_evict_t;
typedef CacheEvictAck<netreach_key_t> cache_evict_ack_t;
typedef CacheEvictCase2<netreach_key_t, val_t> cache_evict_case2_t;
typedef ConcurrentMap<uint16_t, special_case_t> concurrent_specicalcase_map_t;
typedef CachePopAck<netreach_key_t> cache_pop_ack_t;

bool recover_mode = false;

bool volatile switchos_running = false;
std::atomic<size_t> switchos_ready_threads(0);
const size_t switchos_expected_ready_threads = 4;
bool volatile switchos_popserver_finish = false;

// Parameters
size_t server_num = 1;
short switchos_popserver_port = 0;
uint32_t switch_kv_bucket_num = 0;
uint32_t switchos_sample_cnt = 0; // sample_cnt is used by switchos instead of ptf
const char *reflector_ip_for_switchos = nullptr;
short reflector_popserver_port = -1;
const char *controller_ip_for_switchos = nullptr;
short controller_evictserver_port = -1;
short switchos_snapshotserver_port = -1;
short switchos_specialcaseserver_port = -1;
short switchos_ptf_popserver_port = -1;
short switchos_ptf_snapshotserver_port = -1;

// Packet types used by switchos and ptf framework
int SWITCHOS_SETVALID0 = -1;
int SWITCHOS_SETVALID0_ACK = -1;
int SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1 = -1;
int SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK = -1;
int SWITCHOS_GET_EVICTDATA_SETVALID3 = -1;
int SWITCHOS_GET_EVICTDATA_SETVALID3_ACK = -1;
int SWITCHOS_REMOVE_CACHE_LOOKUP = -1;
int SWITCHOS_REMOVE_CACHE_LOOKUP_ACK = -1;
int SWITCHOS_CLEANUP = -1;
int SWITCHOS_CLEANUP_ACK = -1;
int SWITCHOS_ENABLE_SINGLEPATH = -1;
int SWITCHOS_ENABLE_SINGLEPATH_ACK = -1;
int SWITCHOS_SET_SNAPSHOT_FLAG = -1;
int SWITCHOS_SET_SNAPSHOT_FLAG_ACK = -1;
int SWITCHOS_DISABLE_SINGLEPATH = -1;
int SWITCHOS_DISABLE_SINGLEPATH_ACK = -1;
int SWITCHOS_LOAD_SNAPSHOT_DATA = -1;
int SWITCHOS_LOAD_SNAPSHOT_DATA_ACK = -1;
int SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG = -1;
int SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG_ACK = -1;
int SWITCHOS_PTF_POPSERVER_END = -1;
int SWITCHOS_PTF_SNAPSHOTSERVER_END = -1;

// Packet types used by switchos/controller/server for snapshot
int SNAPSHOT_CLEANUP = -1;
int SNAPSHOT_CLEANUP_ACK = -1;
int SNAPSHOT_PREPARE = -1;
int SNAPSHOT_PREPARE_ACK = -1;
int SNAPSHOT_SETFLAG = -1;
int SNAPSHOT_SETFLAG_ACK = -1;
int SNAPSHOT_START = -1;
int SNAPSHOT_START_ACK = -1;
int SNAPSHOT_GETDATA = -1;
int SNAPSHOT_GETDATA_ACK = -1;
int SNAPSHOT_SENDDATA = -1;
int SNAPSHOT_SENDDATA_ACK = -1;

// Cache population

// controller.popclient <-> switchos.popserver
int switchos_popserver_udpsock = -1;
// message queue between switchos.popserver and switchos.popworker
MessagePtrQueue<cache_pop_t> switchos_cache_pop_ptr_queue(MQ_SIZE);
/*cache_pop_t * volatile * switchos_cache_pop_ptrs = NULL;
uint32_t volatile switchos_head_for_pop;
uint32_t volatile switchos_tail_for_pop;*/

// switchos.popworker
std::map<netreach_key_t, uint32_t> switchos_cached_keyidx_map; // TODO: Comment it after checking server.cached_keyset_list
netreach_key_t volatile * switchos_cached_keyarray = NULL; // idx (of inswitch KV) -> key (TODO: different switches for distributed case)
uint16_t volatile * switchos_cached_serveridxarray = NULL; // idx (of inswitch KV) -> serveridx of the key
uint32_t volatile switchos_cached_empty_index = 0; // [empty index, kv_bucket_num-1] is empty
// std::map<netreach_key_t, uint16_t> volatile switchos_cached_key_idx_map; // key -> idx (of inswitch KV)
int switchos_popworker_popclient_for_reflector_udpsock = -1;

// Cache eviction

int switchos_popworker_evictclient_for_controller_udpsock = -1;

// Snapshot

// snapshotserver socket to lead snapshot workflow
int switchos_snapshotserver_udpsock = -1;

// prepare to backup cache metadata for snapshot case2 with atomicity
bool volatile is_stop_cachepop = false;
bool volatile popworker_know_stop_cachepop = false; // ensure current cache population/eviction is finished before metadata backup
// NOTE: not need specialcaseserver_know_stop_cachepop as switchos_specialcases has been cleared in the previous snapshot period

// backuped cache metadata (set/reset by snapshotserver, read by popoworker/specialcaseserver)
netreach_key_t * volatile switchos_cached_keyarray_backup = NULL; // idx (of inswitch KV) -> key (TODO: different switches for distributed case)
uint16_t * volatile switchos_cached_serveridxarray_backup = NULL; // idx (of inswitch KV) -> serveridx of the key
uint32_t volatile switchos_cached_empty_index_backup = 0; // [empty index, kv_bucket_num-1] is empty (not used)
// std::map<netreach_key_t, uint16_t> volatile switchos_cached_key_idx_map_backup; // key -> idx (of inswitch KV)

// collect special cases during snapshot
bool volatile is_snapshot = false;
// specialcaseserver socket
int switchos_specialcaseserver_udpsock = -1;
// special cases updated by specialcaseserver and popworker
std::mutex switchos_mutex_for_specialcases;
//std::map<uint16_t, special_case_t> volatile switchos_specialcases;
concurrent_specicalcase_map_t * volatile switchos_specialcases_ptr;

// rollback after collecting all special cases
bool volatile is_snapshot_end = false;
bool volatile popworker_know_snapshot_end = false; // ensure current case2 is reported before rollback
bool volatile specialcaseserver_know_snapshot_end = false; // ensure current case1s are reported before rollback

// switchos <-> ptf

// switchos.popworker <-> ptf.popserver
int switchos_popworker_popclient_for_ptf_udpsock = -1; 
// switchos.snapshotserver <-> ptf.snapshotserver
int switchos_snapshotserver_snapshotclient_for_ptf_udpsock = -1; 

inline void parse_ini(const char *config_file);
inline void parse_control_ini(const char *config_file);
void prepare_switchos();
void recover();
void *run_switchos_popserver(void *param);
void *run_switchos_popworker(void *param);
void *run_switchos_snapshotserver(void *param);
void *run_switchos_specialcaseserver(void *param);
void process_specialcase(const uint16_t &tmpidx, const netreach_key_t &tmpkey, const val_t &tmpval, const uint32_t &tmpseq, const bool &tmpstat);
void close_switchos();

// switchos <-> ptf.popserver
inline uint32_t serialize_setvalid0(char *buf, uint16_t freeidx);
inline uint32_t serialize_add_cache_lookup_setvalid1(char *buf, netreach_key_t key, uint16_t freeidx);
inline uint32_t serialize_get_evictdata_setvalid3(char *buf);
inline void parse_evictdata(char *buf, int recvsize, uint16_t &switchos_evictidx, val_t &switchos_evictvalue, uint32_t &switchos_evictseq, bool &switchos_evictstat);
inline uint32_t serialize_remove_cache_lookup(char *buf, netreach_key_t key);
inline uint32_t serialize_load_snapshot_data(char *buf, uint32_t emptyidx);
void parse_snapshotdata_fromptf(char *buf, uint32_t buflen, val_t *values, uint32_t *seqs, bool *stats, uint32_t record_cnt);

int main(int argc, char **argv) {
	if ((argc == 2) && (strcmp(argv[1], "recover") == 0)) {
		recover_mode = true;
	}

	parse_ini("config.ini");
	parse_control_ini("control_type.ini");

	prepare_switchos();

	pthread_t popserver_thread;
	int ret = pthread_create(&popserver_thread, nullptr, run_switchos_popserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t popworker_thread;
	ret = pthread_create(&popworker_thread, nullptr, run_switchos_popworker, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t snapshotserver_thread;
	ret = pthread_create(&snapshotserver_thread, nullptr, run_switchos_snapshotserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t specialcaseserver_thread;
	ret = pthread_create(&specialcaseserver_thread, nullptr, run_switchos_specialcaseserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	while (switchos_ready_threads < switchos_expected_ready_threads) sleep(1);
	printf("[switchos] all threads ready\n");

	switchos_running = true;

	// connection from controller
	while (!switchos_popserver_finish) {}

	switchos_running = false;

	void *status;
	int rc = pthread_join(popserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(popworker_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(snapshotserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(specialcaseserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}

	printf("[switchos] all threads end\n");
	close_switchos();
}

// TODO: try common_impl.h
inline void parse_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	server_num = ini.get_server_num();
	switchos_popserver_port = ini.get_switchos_popserver_port();
	switch_kv_bucket_num = ini.get_switch_kv_bucket_num();
	switchos_sample_cnt = ini.get_switchos_sample_cnt();
	val_t::SWITCH_MAX_VALLEN = ini.get_switch_max_vallen();
	val_t::MAX_VALLEN = ini.get_max_vallen();
	reflector_ip_for_switchos = ini.get_reflector_ip_for_switchos();
	reflector_popserver_port = ini.get_reflector_popserver_port();
	controller_ip_for_switchos = ini.get_controller_ip_for_switchos();
	controller_evictserver_port = ini.get_controller_evictserver_port();
	switchos_snapshotserver_port = ini.get_switchos_snapshotserver_port();
	switchos_specialcaseserver_port = ini.get_switchos_specialcaseserver_port();
	switchos_ptf_popserver_port = ini.get_switchos_ptf_popserver_port();
	switchos_ptf_snapshotserver_port = ini.get_switchos_ptf_snapshotserver_port();
	
	COUT_VAR(server_num);
	COUT_VAR(switchos_popserver_port);
	COUT_VAR(switch_kv_bucket_num);
	COUT_VAR(switchos_sample_cnt);
	COUT_VAR(val_t::SWITCH_MAX_VALLEN);
	COUT_VAR(val_t::MAX_VALLEN);
	printf("reflector ip for switchos: %s\n", reflector_ip_for_switchos);
	COUT_VAR(reflector_popserver_port);
	printf("controller ip for switchos: %s\n", controller_ip_for_switchos);
	COUT_VAR(controller_evictserver_port);
	COUT_VAR(switchos_snapshotserver_port);
	COUT_VAR(switchos_specialcaseserver_port);
	COUT_VAR(switchos_ptf_popserver_port);
	COUT_VAR(switchos_ptf_snapshotserver_port);
}

inline void parse_control_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	SWITCHOS_SETVALID0 = ini.get_switchos_setvalid0();
	SWITCHOS_SETVALID0_ACK = ini.get_switchos_setvalid0_ack();
	SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1 = ini.get_switchos_add_cache_lookup_setvalid1();
	SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK = ini.get_switchos_add_cache_lookup_setvalid1_ack();
	SWITCHOS_GET_EVICTDATA_SETVALID3 = ini.get_switchos_get_evictdata_setvalid3();
	SWITCHOS_GET_EVICTDATA_SETVALID3_ACK = ini.get_switchos_get_evictdata_setvalid3_ack();
	SWITCHOS_REMOVE_CACHE_LOOKUP = ini.get_switchos_remove_cache_lookup();
	SWITCHOS_REMOVE_CACHE_LOOKUP_ACK = ini.get_switchos_remove_cache_lookup_ack();
	SWITCHOS_CLEANUP = ini.get_switchos_cleanup();
	SWITCHOS_CLEANUP_ACK = ini.get_switchos_cleanup_ack();
	SWITCHOS_ENABLE_SINGLEPATH = ini.get_switchos_enable_singlepath();
	SWITCHOS_ENABLE_SINGLEPATH_ACK = ini.get_switchos_enable_singlepath_ack();
	SWITCHOS_SET_SNAPSHOT_FLAG = ini.get_switchos_set_snapshot_flag();
	SWITCHOS_SET_SNAPSHOT_FLAG_ACK = ini.get_switchos_set_snapshot_flag_ack();
	SWITCHOS_DISABLE_SINGLEPATH = ini.get_switchos_disable_singlepath();
	SWITCHOS_DISABLE_SINGLEPATH_ACK = ini.get_switchos_disable_singlepath_ack();
	SWITCHOS_LOAD_SNAPSHOT_DATA = ini.get_switchos_load_snapshot_data();
	SWITCHOS_LOAD_SNAPSHOT_DATA_ACK = ini.get_switchos_load_snapshot_data_ack();
	SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG = ini.get_switchos_reset_snapshot_flag_and_reg();
	SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG_ACK = ini.get_switchos_reset_snapshot_flag_and_reg_ack();
	SWITCHOS_PTF_POPSERVER_END = ini.get_switchos_ptf_popserver_end();
	SWITCHOS_PTF_SNAPSHOTSERVER_END = ini.get_switchos_ptf_snapshotserver_end();

	SNAPSHOT_CLEANUP = ini.get_snapshot_cleanup();
	SNAPSHOT_CLEANUP_ACK = ini.get_snapshot_cleanup_ack();
	SNAPSHOT_PREPARE = ini.get_snapshot_prepare();
	SNAPSHOT_PREPARE_ACK = ini.get_snapshot_prepare_ack();
	SNAPSHOT_SETFLAG = ini.get_snapshot_setflag();
	SNAPSHOT_SETFLAG_ACK = ini.get_snapshot_setflag_ack();
	SNAPSHOT_START = ini.get_snapshot_start();
	SNAPSHOT_START_ACK = ini.get_snapshot_start_ack();
	SNAPSHOT_GETDATA = ini.get_snapshot_getdata();
	SNAPSHOT_GETDATA_ACK = ini.get_snapshot_getdata_ack();
	SNAPSHOT_SENDDATA = ini.get_snapshot_senddata();
	SNAPSHOT_SENDDATA_ACK = ini.get_snapshot_senddata_ack();
}

void prepare_switchos() {
	printf("[switchos] prepare start\n");

	srand(0); // set random seed as 0 for cache eviction

	// prepare popserver socket
	prepare_udpserver(switchos_popserver_udpsock, false, switchos_popserver_port, "switchos.popserver");

	//switchos_cache_pop_ptrs = new cache_pop_t*[MQ_SIZE];
	//switchos_head_for_pop = 0;
	//switchos_tail_for_pop = 0;

	switchos_cached_keyidx_map.clear();
	create_udpsock(switchos_popworker_popclient_for_reflector_udpsock, true, "switchos.popworker");
	switchos_cached_keyarray = new netreach_key_t[switch_kv_bucket_num]();
	switchos_cached_serveridxarray = new uint16_t[switch_kv_bucket_num];
	for (size_t i = 0; i < switch_kv_bucket_num; i++) {
		switchos_cached_serveridxarray[i] = -1;
	}
	switchos_cached_empty_index = 0;
	//switchos_cached_key_idx_map.clear();

	create_udpsock(switchos_popworker_evictclient_for_controller_udpsock, true, "switchos.popworker.evictclient");
	//switchos_evictvalbytes = new char[val_t::MAX_VALLEN];
	//INVARIANT(switchos_evictvalbytes != NULL);
	//memset(switchos_evictvalbytes, 0, val_t::MAX_VALLEN);

	// prepare snapshotserver socket
	prepare_udpserver(switchos_snapshotserver_udpsock, false, switchos_snapshotserver_port, "switchos.snapshotserver");

	// prepare for switchos <-> ptf
	create_udpsock(switchos_popworker_popclient_for_ptf_udpsock, false, "switchos.popworker.popclient_for_ptf");
	create_udpsock(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, false, "switchos.snapshotserver.snapshotclient_for_ptf", SOCKET_TIMEOUT, 0, UDP_LARGE_RCVBUFSIZE);

	// prepare specialcaseserver socket
	prepare_udpserver(switchos_specialcaseserver_udpsock, true, switchos_specialcaseserver_port, "switchos.specialcaseserver", 0, 1000); // timeout interval: 1000us to avoid long wait time when making snapshot

	//switchos_specialcases->clear();

	if (recover_mode) {
		recover();
	}

	memory_fence();

	printf("[switchos] prepare end\n");
}

void recover() {
	std::string snapshotid_path;
	get_controller_snapshotid_path(snapshotid_path);
	if (!isexist(snapshotid_path)) {
		printf("You need to copy latest snapshotid from controller to switchos before running with recover mode\n");
		exit(-1);
	}

	int controller_snapshotid = 0;
	load_snapshotid(controller_snapshotid, snapshotid_path);
	std::string snapshotdata_path;
	get_controller_snapshotdata_path(snapshotdata_path, controller_snapshotid);
	if (!isexist(snapshotdata_path)) {
		printf("You need to copy inswitch snapshot data from controller to switchos before running with recover mode\n");
		exit(-1);
	}

	uint32_t filesize = get_filesize(snapshotdata_path);
	INVARIANT(filesize > 0);
	char *content = readonly_mmap(snapshotdata_path, 0, filesize);
	INVARIANT(content != NULL);

	// set inswitch stateful memory with inswitch snapshot data
	system("sudo bash tofino/recover_switch.sh");

	// extract snapshot data
	// snapshot data: <int SNAPSHOT_GETDATA_ACK, int32_t total_bytes (including SNAPSHOT_GETDATA_ACK), per-server data>
	// per-server data: <int32_t perserver_bytes, uint16_t serveridx, int32_t recordcnt, per-record data>
	// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>
	int control_type = *((int *)content);
	INVARIANT(control_type == SNAPSHOT_GETDATA_ACK);
	int total_bytes = *((int32_t *)(content + sizeof(int)));
	int tmp_offset = sizeof(int) + sizeof(int32_t); // SNAPSHOT_DATA + total_bytes
	while (true) {
		tmp_offset += sizeof(int32_t); // skip perserver_bytes
		uint16_t tmp_serveridx = *((uint16_t *)(content + tmp_offset));
		tmp_offset += sizeof(uint16_t);
		int32_t tmp_recordcnt = *((int32_t *)(content + tmp_offset));
		tmp_offset += sizeof(int32_t);
		for (int32_t tmp_recordidx = 0; tmp_recordidx < tmp_recordcnt; tmp_recordidx++) {
			netreach_key_t tmp_key;
			uint32_t tmp_keysize = tmp_key.deserialize(content + tmp_offset, total_bytes - tmp_offset);
			tmp_offset += tmp_keysize;
			val_t tmp_val;
			uint32_t tmp_valsize = tmp_val.deserialize(content + tmp_offset, total_bytes - tmp_offset);
			tmp_offset += tmp_valsize;
			uint32_t seq = *((uint32_t *)(content + tmp_offset));
			tmp_offset += sizeof(uint32_t);
			bool stat = *((bool *)(content + tmp_offset));
			tmp_offset += sizeof(bool);

			// Update switchos inswitch cache metadata
			switchos_cached_keyarray[switchos_cached_empty_index] = tmp_key;
			switchos_cached_serveridxarray[switchos_cached_empty_index] = tmp_serveridx;
			switchos_cached_empty_index += 1;
		}
		if (tmp_offset >= total_bytes) {
			break;
		}
	}

	munmap(content, filesize);
}

void *run_switchos_popserver(void *param) {
	// NOTE: controller.popclient address continues to change
	struct sockaddr_in controller_popclient_addr;
	socklen_t controller_popclient_addrlen = sizeof(struct sockaddr_in);
	
	printf("[switchos.popserver] ready\n");
	switchos_ready_threads++;

	while (!switchos_running) {}

	// Process CACHE_POP packet <optype, key, vallen, value, seq, serveridx>
	char buf[MAX_BUFSIZE];
	int recvsize = 0;
	while (switchos_running) {
		udprecvfrom(switchos_popserver_udpsock, buf, MAX_BUFSIZE, 0, &controller_popclient_addr, &controller_popclient_addrlen, recvsize, "switchos.popserver");

		//printf("receive CACHE_POP from controller\n");
		//dump_buf(buf, recvsize);
		cache_pop_t *tmp_cache_pop_ptr = new cache_pop_t(buf, recvsize); // freed by switchos.popworker

		// send CACHE_POP_ACK to controller.popclient immediately to avoid timeout
		cache_pop_ack_t tmp_cache_pop_ack(tmp_cache_pop_ptr->key());
		uint32_t acksize = tmp_cache_pop_ack.serialize(buf, MAX_BUFSIZE);
		udpsendto(switchos_popserver_udpsock, buf, acksize, 0, &controller_popclient_addr, controller_popclient_addrlen, "switchos.popserver");

		bool res = switchos_cache_pop_ptr_queue.write(tmp_cache_pop_ptr);
		if (!res) {
			printf("[switch os] message queue overflow of switchos.switchos_cache_pop_ptr_queue!");
		}
		/*if ((switchos_head_for_pop+1)%MQ_SIZE != switchos_tail_for_pop) {
			switchos_cache_pop_ptrs[switchos_head_for_pop] = tmp_cache_pop_ptr;
			switchos_head_for_pop = (switchos_head_for_pop + 1) % MQ_SIZE;
		}
		else {
			printf("[switch os] message queue overflow of switchos.switchos_cache_pop_ptrs!");
		}*/
	}

	switchos_popserver_finish = true;
	close(switchos_popserver_udpsock);
	pthread_exit(nullptr);
}

void *run_switchos_popworker(void *param) {
	// used by udp socket for cache population
	sockaddr_in reflector_popserver_addr;
	set_sockaddr(reflector_popserver_addr, inet_addr(reflector_ip_for_switchos), reflector_popserver_port);
	int reflector_popserver_addr_len = sizeof(struct sockaddr);

	// used by udpsocket to communicate with ptf.popserver
	sockaddr_in ptf_popserver_addr;
	set_sockaddr(ptf_popserver_addr, inet_addr("127.0.0.1"), switchos_ptf_popserver_port);
	int ptf_popserver_addr_len = sizeof(struct sockaddr);

	// used by popworker.evictclient
	sockaddr_in controller_evictserver_addr;
	set_sockaddr(controller_evictserver_addr, inet_addr(controller_ip_for_switchos), controller_evictserver_port);
	socklen_t controller_evictserver_addrlen = sizeof(struct sockaddr_in);

	printf("[switchos.popworker] ready\n");
	switchos_ready_threads++;

	while (!switchos_running) {}

	// (1) communicate with controller.evictserver or reflector.popserer
	// send CACHE_POP_INSWITCH and CACHE_EVICT to controller
	char pktbuf[MAX_BUFSIZE];
	uint32_t pktsize = 0;
	// recv CACHE_POP_INSWITCH_ACK and CACHE_EVICT_ACK from controlelr
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
	//std::vector<double> evict_load_time_list, evict_sendrecv_time_list, evict_remove_time_list, evict_total_time_list;
	//struct timespec evict_load_t1, evict_load_t2, evict_load_t3, evict_sendrecv_t1, evict_sendrecv_t2, evict_sendrecv_t3, evict_remove_t1, evict_remove_t2, evict_remove_t3, evict_total_t1, evict_total_t2, evict_total_t3;

	while (switchos_running) {
		if (is_stop_cachepop && !popworker_know_stop_cachepop) {
			popworker_know_stop_cachepop = true;
		}
		else if (!is_stop_cachepop) {
		//if (switchos_tail_for_pop != switchos_head_for_pop) {
			cache_pop_t *tmp_cache_pop_ptr = switchos_cache_pop_ptr_queue.read();
			if (tmp_cache_pop_ptr != NULL) {
				//cache_pop_t *tmp_cache_pop_ptr = switchos_cache_pop_ptrs[switchos_tail_for_pop];
				//INVARIANT(tmp_cache_pop_ptr != NULL);
				
				if (switchos_cached_keyidx_map.find(tmp_cache_pop_ptr->key()) != switchos_cached_keyidx_map.end()) {
					printf("Error: populating a key %x cached at %u from server %d\n", tmp_cache_pop_ptr->key().keyhihi, switchos_cached_keyidx_map[tmp_cache_pop_ptr->key()], int(tmp_cache_pop_ptr->serveridx()));
					exit(-1);
				}

				// assign switchos_freeidx for new record 
				if (switchos_cached_empty_index < switch_kv_bucket_num) { // With free idx
					switchos_freeidx = switchos_cached_empty_index;
					switchos_cached_empty_index += 1;
					// NOTE: as freeidx of new record must > switchos_cached_empty_index_backup, no case2 for cache population
				}
				else { // Without free idx
					//CUR_TIME(evict_total_t1);
					bool is_case2 = is_snapshot || (is_snapshot_end && !popworker_know_snapshot_end);

					//CUR_TIME(evict_load_t1);
					
					// get evictdata from ptf framework 
					////system("bash tofino/get_evictdata_setvalid3.sh");
					/*ptf_sendsize = serialize_get_evictdata_setvalid3(ptfbuf);
					udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
					udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
					INVARIANT(*((int *)ptfbuf) == SWITCHOS_GET_EVICTDATA_SETVALID3_ACK); // wait for SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK
					parse_evictdata(ptfbuf, ptf_recvsize, switchos_evictidx, switchos_evictvalue, switchos_evictseq, switchos_evictstat);*/

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
						//printf("send CACHE_EVICT_LOADFREQ_INSWITCHs to reflector\n");
						for (size_t i = 0; i < switchos_sample_cnt; i++) {
							cache_evict_loadfreq_inswitch_t tmp_cache_evict_loadfreq_inswitch_req(switchos_cached_keyarray[sampled_idxes[i]], sampled_idxes[i]);
							pktsize = tmp_cache_evict_loadfreq_inswitch_req.serialize(pktbuf, MAX_BUFSIZE);
							udpsendto(switchos_popworker_popclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_popserver_addr, reflector_popserver_addr_len, "switchos.popworker.evictclient");
						}

						// loop until receiving corresponding ACK (ignore unmatched ACKs which are duplicate ACKs of previous cache population)
						bool is_timeout = false;
						int tmp_acknum = 0
						while (true) {
							is_timeout = udprecvfrom(switchos_popworker_popclient_for_reflector_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.evictclient");
							if (unlikely(is_timeout)) {
								break;
							}

							cache_evict_loadfreq_inswitch_ack_t tmp_cache_evict_loadfreq_inswitch_ack(ackbuf, ack_recvsize);
							for (size_t i = 0; i < switchos_sample_cnt; i++) {
								if (switchos_cached_keyarray[sampled_idxes[i]] == tmp_cache_evict_loadfreq_inswitch_ack.key()) {
									frequency_counters[i] = tmp_cache_evict_loadfreq_inswitch_ack.frequency();
									tmp_acknum += 1;
								}
								else if (i == switchos_sample_cnt - 1) {
									printf("Not match any sampled key from %d CACHE_EVICT_LOADFREQ_INSWITCH_ACKs, continue to receive!\n", switchos_sample_cnt);
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

					// TMPDEBUG
					for (size_t i = 0; i < switchos_sample_cnt; i++) {
						COUT_VAR(sampled_idxes[i]);
					}
					printf("Type after checking frequency counters...\n");
					getchar();
					
					// TODO: load evicted data of victim from data plane and set valid=3 at the same time for availability of latest value

					//CUR_TIME(evict_load_t2);

					// switchos.popworker.evictclient sends CACHE_EVICT to controller.evictserver
					INVARIANT(switchos_evictidx >= 0 && switchos_evictidx < switch_kv_bucket_num);
					netreach_key_t cur_evictkey = switchos_cached_keyarray[switchos_evictidx];

					if (switchos_cached_keyidx_map.find(cur_evictkey) == switchos_cached_keyidx_map.end()) {
						printf("Evicted key %x at %d is not cached\n", switchos_cached_keyarray[switchos_evictidx].keyhihi, switchos_evictidx);
						exit(-1);
					}

					//CUR_TIME(evict_sendrecv_t1);
					if (!is_case2) {
						cache_evict_t tmp_cache_evict(cur_evictkey, \
								//val_t(switchos_evictvalbytes, switchos_evictvallen),
								switchos_evictvalue, \
								switchos_evictseq, switchos_evictstat, \
								switchos_cached_serveridxarray[switchos_evictidx]);
						pktsize = tmp_cache_evict.serialize(pktbuf, MAX_BUFSIZE);
					}
					else { // send CACHE_EVICT_CASE2 for server-side snapshot
						cache_evict_case2_t tmp_cache_evict_case2(cur_evictkey, \
								//val_t(switchos_evictvalbytes, switchos_evictvallen),
								switchos_evictvalue, \
								switchos_evictseq, switchos_evictstat, \
								switchos_cached_serveridxarray[switchos_evictidx]);
						pktsize = tmp_cache_evict_case2.serialize(pktbuf, MAX_BUFSIZE);
					}

					while (true) {
						//printf("send CACHE_EVICT to controller\n");
						//dump_buf(pktbuf, pktsize);
						udpsendto(switchos_popworker_evictclient_for_controller_udpsock, pktbuf, pktsize, 0, &controller_evictserver_addr, controller_evictserver_addrlen, "switchos.popworker.evictclient");

						// wait for CACHE_EVICT_ACK from controller.evictserver
						// NOTE: no concurrent CACHE_EVICTs -> use request-and-reply manner to wait for entire eviction workflow
						bool is_timeout = udprecvfrom(switchos_popworker_evictclient_for_controller_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.evictclient");
						if (unlikely(is_timeout)) {
							continue;
						}
						else {
							cache_evict_ack_t tmp_cache_evict_ack(ackbuf, ack_recvsize);
							INVARIANT(tmp_cache_evict_ack.key() == cur_evictkey);
							break;
						}
					}
					//CUR_TIME(evict_sendrecv_t2);

					// store case2
					if (is_case2) {
						process_specialcase(uint16_t(switchos_evictidx), cur_evictkey, \
								//val_t(switchos_evictvalbytes, switchos_evictvallen), 
								val_t(switchos_evictvalue), \
								uint32_t(switchos_evictseq), bool(switchos_evictstat));
					}

					//CUR_TIME(evict_remove_t1);
					// remove evicted data from cache_lookup_tbl
					//system("bash tofino/remove_cache_lookup.sh");
					////switchos_cached_key_idx_map.erase(cur_evictkey);
					ptf_sendsize = serialize_remove_cache_lookup(ptfbuf, switchos_cached_keyarray[switchos_evictidx]);
					udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
					udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
					INVARIANT(*((int *)ptfbuf) == SWITCHOS_REMOVE_CACHE_LOOKUP_ACK); // wait for SWITCHOS_REMOVE_CACHE_LOOKUP_ACK
					//CUR_TIME(evict_remove_t2);

					//printf("Evict %x to %x\n", switchos_cached_keyarray[switchos_evictidx].keyhihi, tmp_cache_pop_ptr->key().keyhihi);

					// set freeidx as evictidx for cache popluation later
					switchos_freeidx = switchos_evictidx;

					// reset keyarray and serveridxarray at evictidx
					switchos_cached_keyidx_map.erase(cur_evictkey);
					switchos_cached_keyarray[switchos_evictidx] = netreach_key_t();
					switchos_cached_serveridxarray[switchos_evictidx] = -1;

					//CUR_TIME(evict_total_t2);

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
				//printf("[switchos.popworker] switchos_cached_empty_index: %d, switchos_freeidx: %d\n", int(switchos_cached_empty_index), int(switchos_freeidx)); // TMPDEBUG

				// set valid=0 for atomicity
				//system("bash tofino/setvalid0.sh");
				ptf_sendsize = serialize_setvalid0(ptfbuf, switchos_freeidx);
				udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
				udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
				INVARIANT(*((int *)ptfbuf) == SWITCHOS_SETVALID0_ACK); // wait for SWITCHOS_SETVALID0_ACK

				// send CACHE_POP_INSWITCH to reflector (TODO: try internal pcie port)
				cache_pop_inswitch_t tmp_cache_pop_inswitch(tmp_cache_pop_ptr->key(), tmp_cache_pop_ptr->val(), tmp_cache_pop_ptr->seq(), switchos_freeidx);
				pktsize = tmp_cache_pop_inswitch.serialize(pktbuf, MAX_BUFSIZE);

				while (true) {
					udpsendto(switchos_popworker_popclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_popserver_addr, reflector_popserver_addr_len, "switchos.popworker.popclient");

					// loop until receiving corresponding ACK (ignore unmatched ACKs which are duplicate ACKs of previous cache population)
					bool is_timeout = false;
					bool with_correctack = false;
					while (true) {
						is_timeout = udprecvfrom(switchos_popworker_popclient_for_reflector_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.popclient");
						if (unlikely(is_timeout)) {
							break;
						}

						cache_pop_inswitch_ack_t tmp_cache_pop_inswitch_ack(ackbuf, ack_recvsize);
						if (tmp_cache_pop_inswitch_ack.key() == tmp_cache_pop_ptr->key()) {
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

				// (1) add new <key, value> pair into cache_lookup_tbl; (2) and set valid=1 to enable the entry
				//system("bash tofino/add_cache_lookup_setvalid1.sh");
				ptf_sendsize = serialize_add_cache_lookup_setvalid1(ptfbuf, tmp_cache_pop_ptr->key(), switchos_freeidx);
				udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
				udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
				INVARIANT(*((int *)ptfbuf) == SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK); // wait for SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK

				switchos_cached_keyidx_map.insert(std::pair<netreach_key_t, uint32_t>(tmp_cache_pop_ptr->key(), switchos_freeidx));
				switchos_cached_keyarray[switchos_freeidx] = tmp_cache_pop_ptr->key();
				switchos_cached_serveridxarray[switchos_freeidx] = tmp_cache_pop_ptr->serveridx();
				//switchos_cached_key_idx_map.insert(std::pair<netreach_key_t, uint16_t>(tmp_cache_pop_ptr->key(), switchos_freeidx));

				// free CACHE_POP
				delete tmp_cache_pop_ptr;
				tmp_cache_pop_ptr = NULL;
				//switchos_cache_pop_ptrs[switchos_tail_for_pop] = NULL;
				//switchos_tail_for_pop = (switchos_tail_for_pop + 1) % MQ_SIZE;

				// reset metadata for popclient_for_ptf
				ptf_sendsize = 0;
				ptf_recvsize = 0;
				switchos_freeidx = 0;
				switchos_evictidx = 0;
				switchos_evictvalue = val_t();
				switchos_evictseq = 0;
				switchos_evictstat = false;
			} // tmp_cache_pop_ptr != NULL
		} // !is_stop_cachepop
		if (is_snapshot_end && !popworker_know_snapshot_end) {
			popworker_know_snapshot_end = true;
		}
	}

	// send SWITCHOS_PTF_POPSERVER_END to ptf.popserver
	memcpy(ptfbuf, &SWITCHOS_PTF_POPSERVER_END, sizeof(int));
	udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");

	close(switchos_popworker_popclient_for_reflector_udpsock);
	close(switchos_popworker_evictclient_for_controller_udpsock);
	close(switchos_popworker_popclient_for_ptf_udpsock);
	pthread_exit(nullptr);
}

void *run_switchos_snapshotserver(void *param) {
	// for SNAPSHOT_START/_ACK
	struct sockaddr_in controller_snapshotclient_addr;
	socklen_t controller_snapshotclient_addrlen = sizeof(struct sockaddr);
	//bool with_controller_snapshotclient_addr = false;

	struct sockaddr_in ptf_addr;
	set_sockaddr(ptf_addr, inet_addr("127.0.0.1"), switchos_ptf_snapshotserver_port);
	socklen_t ptf_addrlen = sizeof(struct sockaddr_in);

	// for snapshot data from ptf
	val_t * switchos_snapshot_values = NULL;
	uint32_t * switchos_snapshot_seqs = NULL;
	bool * switchos_snapshot_stats = NULL;
	dynamic_array_t ptf_largebuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
	
	// for consistent snapshot data to controller
	dynamic_array_t tmp_sendbuf_list[server_num];
	for (uint16_t i = 0; i < server_num; i++) {
		tmp_sendbuf_list[i].init(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
	}
	int tmp_send_bytes[server_num];
	memset((void *)tmp_send_bytes, 0, server_num*sizeof(int));
	int tmp_record_cnts[server_num];
	memset((void *)tmp_record_cnts, 0, server_num*sizeof(int));
	dynamic_array_t sendbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);

	printf("[switchos.snapshotserver] ready\n");
	switchos_ready_threads++;

	while (!switchos_running) {}

	// communicate with controller.snapshotclient
	char recvbuf[MAX_BUFSIZE];
	int recvsize = 0;
	int control_type = -1;
	int snapshotid = -1;
	// communicate with ptf.snapshotserver
	char ptfbuf[MAX_BUFSIZE];
	int ptf_recvsize = 0;
	struct timespec stop_cachepop_t1, stop_cachepop_t2, stop_cachepop_t3, enable_singlepath_t1, enable_singlepath_t2, enable_singlepath_t3, load_snapshotdata_t1, load_snapshotdata_t2, load_snapshotdata_t3;
	while (switchos_running) {
		// wait for control instruction from controller.snapshotclient
		/*if (!with_controller_snapshotclient_addr) {
			udprecvfrom(switchos_snapshotserver_udpsock, recvbuf, MAX_BUFSIZE, 0, &controller_snapshotclient_addr, &controller_snapshotclient_addrlen, recvsize, "switchos.snapshotserver");
			with_controller_snapshotclient_addr = true;
		}
		else {
			udprecvfrom(switchos_snapshotserver_udpsock, recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "switchos.snapshotserver");
		}*/
		udprecvfrom(switchos_snapshotserver_udpsock, recvbuf, MAX_BUFSIZE, 0, &controller_snapshotclient_addr, &controller_snapshotclient_addrlen, recvsize, "switchos.snapshotserver");

		INVARIANT(recvsize == 2*sizeof(int));
		// Fix duplicate packet
		if (control_type == *((int *)recvbuf) && snapshotid == *((int *)(recvbuf + sizeof(int)))) {
			printf("[switchos.snapshotserver] receive duplicate control type %d for snapshot id %d\n", control_type, snapshotid); // TMPDEBUG
			continue;
		}
		else {
			control_type = *((int *)recvbuf);
			snapshotid = *((int *)(recvbuf + sizeof(int)));
			printf("[switchos.snapshotserver] receive control type %d for snapshot id %d\n", control_type, snapshotid); // TMPDEBUG
		}

		if (control_type == SNAPSHOT_CLEANUP) {
			// (1) cleanup stale snapshot
			
			// cleanup dataplane: disable single path, reset flag and reg
			memcpy(ptfbuf, &SWITCHOS_CLEANUP, sizeof(int));
			udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
			udprecvfrom(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.snapshotserver.snapshotclient_for_ptf");
			INVARIANT(ptf_recvsize == sizeof(int) && *((int *)ptfbuf) == SWITCHOS_CLEANUP_ACK);
			
			// enable cache popuation/eviction
			is_stop_cachepop = false;
			popworker_know_stop_cachepop = false;

			if (likely(switchos_specialcases_ptr != NULL)) { // not the first snapshot
				INVARIANT(switchos_cached_keyarray_backup != NULL && switchos_cached_serveridxarray_backup != NULL);

				// notify to end stale snapshot
				is_snapshot_end = true;
				is_snapshot = false;
				popworker_know_snapshot_end = false;
				specialcaseserver_know_snapshot_end = false;

				// wait for case2 from popworker and case1 from specialcaseserver (finish using switchos_specialcases_ptr)
				while (!popworker_know_snapshot_end || !specialcaseserver_know_snapshot_end) {}

				// end stale snapshot (by now popworker/specialcaseserver will not access switchos_specialcases_ptr and backuped metadata)
				is_snapshot_end = false;
				popworker_know_snapshot_end = false;
				specialcaseserver_know_snapshot_end = false;

				// free stale special cases
				// NOTE: popserver/specialcaseserver will not touch speicalcases_ptr now, as both is_snapshot/is_snapshot_end are false
				delete switchos_specialcases_ptr;
				switchos_specialcases_ptr = NULL;

				// free backuped metadata
				// NOTE: popserver/specialcaseserver will not touch backuped metadata now, as both is_snapshot/is_snapshot_end are false
				delete [] switchos_cached_keyarray_backup;
				switchos_cached_keyarray_backup = NULL;
				delete [] switchos_cached_serveridxarray_backup;
				switchos_cached_serveridxarray_backup = NULL;
				switchos_cached_empty_index_backup = 0;
			}
			else { // the first snapshot
				INVARIANT(switchos_cached_keyarray_backup == NULL && switchos_cached_serveridxarray_backup == NULL);
				INVARIANT(is_snapshot == false && is_snapshot_end == false && popworker_know_snapshot_end == false && specialcaseserver_know_snapshot_end == false);
			}

			// free snapshot data
			if (switchos_snapshot_values != NULL) {
				delete [] switchos_snapshot_values;
				switchos_snapshot_values = NULL;
				delete [] switchos_snapshot_seqs;
				switchos_snapshot_seqs = NULL;
				delete [] switchos_snapshot_stats;
				switchos_snapshot_stats = NULL;
			}

			// (2) init for new snapshot
			
			// create new speical cases (freed by next SNAPSHOT_CLEANUP)
			switchos_specialcases_ptr = new concurrent_specicalcase_map_t();

			CUR_TIME(stop_cachepop_t1);
			// stop cache population/eviction (stop until we can ensure that snapshot has started aka SNAPSHOT_START)
			is_stop_cachepop = true;

			// wait until popworker_know_stop_cachepop = true
			while (!popworker_know_stop_cachepop) {}
			// by now, cache population/eviction is temporarily stopped -> snapshotserver can backup cache metadata atomically

			// backup cache metadata (freed by next SNAPSHOT_CLEANUP)
			// NOTE: is_snapshot/_end = false -> we can write backuped metadata
			// NOTE: is_stop_cache/popworker_know_stop_cachepop = true -> we can read cache metadata
			memory_fence();
			switchos_cached_keyarray_backup = new netreach_key_t[switch_kv_bucket_num];
			INVARIANT(switchos_cached_keyarray_backup != NULL);
			memcpy(switchos_cached_keyarray_backup, (void *)switchos_cached_keyarray, switch_kv_bucket_num * sizeof(netreach_key_t));
			switchos_cached_serveridxarray_backup = new uint16_t[switch_kv_bucket_num];
			INVARIANT(switchos_cached_serveridxarray_backup != NULL);
			memcpy(switchos_cached_serveridxarray_backup, (void *)switchos_cached_serveridxarray, switch_kv_bucket_num * sizeof(uint16_t));
			switchos_cached_empty_index_backup = switchos_cached_empty_index;
			//switchos_cached_key_idx_map_backup = switchos_cached_key_idx_map; // by copy assignment
			memory_fence();
			
			// sendback SNAPSHOT_CLEANUP_ACK
			udpsendto(switchos_snapshotserver_udpsock, &SNAPSHOT_CLEANUP_ACK, sizeof(int), 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "switchos.snapshotserver");
		}
		else if (control_type == SNAPSHOT_PREPARE) {
			CUR_TIME(enable_singlepath_t1);
			// enable a single path to prepare for setting snapshot with atomicity
			memcpy(ptfbuf, &SWITCHOS_ENABLE_SINGLEPATH, sizeof(int));
			udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
			udprecvfrom(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.snapshotserver.snapshotclient_for_ptf");
			INVARIANT(ptf_recvsize == sizeof(int) && *((int *)ptfbuf) == SWITCHOS_ENABLE_SINGLEPATH_ACK);

			// sendback SNAPSHOT_PREPARE_ACK
			udpsendto(switchos_snapshotserver_udpsock, &SNAPSHOT_PREPARE_ACK, sizeof(int), 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "switchos.snapshotserver");
		}
		else if (control_type == SNAPSHOT_SETFLAG) {
			// ptf sets snapshot flag as true atomically
			//system("bash tofino/set_snapshot_flag.sh");
			memcpy(ptfbuf, &SWITCHOS_SET_SNAPSHOT_FLAG, sizeof(int));
			udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
			udprecvfrom(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.snapshotserver.snapshotclient_for_ptf");
			INVARIANT(ptf_recvsize == sizeof(int) && *((int *)ptfbuf) == SWITCHOS_SET_SNAPSHOT_FLAG_ACK);

			// NOTE: by now data plane starts to report case1 to controller, although switchos wlll not process case1 after SNAPSHOT_START, the special cases will be stored in UDP socket receive buffer

			// sendback SNAPSHOT_SETFLAG_ACK
			udpsendto(switchos_snapshotserver_udpsock, &SNAPSHOT_SETFLAG_ACK, sizeof(int), 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "switchos.snapshotserver");
		}
		else if (control_type == SNAPSHOT_START) {
			// disable single path
			memcpy(ptfbuf, &SWITCHOS_DISABLE_SINGLEPATH, sizeof(int));
			udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
			udprecvfrom(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.snapshotserver.snapshotclient_for_ptf");
			INVARIANT(ptf_recvsize == sizeof(int) && *((int *)ptfbuf) == SWITCHOS_DISABLE_SINGLEPATH_ACK);
			CUR_TIME(enable_singlepath_t2);

			// enable special case processing and resume cache population/eviction
			INVARIANT(is_snapshot == false && is_stop_cachepop == true && popworker_know_stop_cachepop == true);
			is_snapshot = true; // notify popworker and specialcaseserver to collect special cases
			is_stop_cachepop = false; // resume cache population/eviction
			popworker_know_stop_cachepop = false;
			CUR_TIME(stop_cachepop_t2);

			DELTA_TIME(stop_cachepop_t2, stop_cachepop_t1, stop_cachepop_t3);
			DELTA_TIME(enable_singlepath_t2, enable_singlepath_t1, enable_singlepath_t3);
			printf("Time of stopping cache population: %f ms\n", GET_MICROSECOND(stop_cachepop_t3) / 1000.0);
			printf("Time of enabling single path: %f ms\n", GET_MICROSECOND(enable_singlepath_t3) / 1000.0);
			

#ifdef DEBUG_SNAPSHOT
			// TMPDEBUG (NOTE: temporarily dislabe timeout-and-retry of SNAPSHOT_START in controller to debug snapshot here)
			printf("Type to load snapshot data...\n");
			getchar();
#endif

			// load vallen, value, deleted, and savedseq in [0, switchos_cached_empty_index_backup-1]
			CUR_TIME(load_snapshotdata_t1);
			if (switchos_cached_empty_index_backup > 0) {
				// NOTE: freed by next SNAPSHOT_CLEANUP
				switchos_snapshot_values = new val_t[switchos_cached_empty_index_backup];
				switchos_snapshot_seqs = new uint32_t[switchos_cached_empty_index_backup];
				switchos_snapshot_stats = new bool[switchos_cached_empty_index_backup];

				//system("bash tofino/load_snapshot_data.sh"); // load snapshot (maybe inconsistent -> need rollback later)
				uint32_t ptf_sendsize = serialize_load_snapshot_data(ptfbuf, switchos_cached_empty_index_backup);
				udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
				ptf_largebuf.clear();
				udprecvlarge_udpfrag(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptf_largebuf, 0, NULL, NULL, "switchos.snapshotserver.snapshotclient_for_ptf");
				parse_snapshotdata_fromptf(ptf_largebuf.array(), ptf_largebuf.size(), \
						switchos_snapshot_values, switchos_snapshot_seqs, switchos_snapshot_stats, switchos_cached_empty_index_backup);
			}
			CUR_TIME(load_snapshotdata_t2);
			DELTA_TIME(load_snapshotdata_t2, load_snapshotdata_t1, load_snapshotdata_t3);
			printf("Time of loading snapshot data by ptf: %f s\n", GET_MICROSECOND(load_snapshotdata_t3) / 1000.0 / 1000.0);

			// sendback SNAPSHOT_START_ACK
			udpsendto(switchos_snapshotserver_udpsock, &SNAPSHOT_START_ACK, sizeof(int), 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "switchos.snapshotserver");
		}
		else if (control_type == SNAPSHOT_GETDATA) {
			// reset snapshot flag as false in data plane
			//system("bash tofino/reset_snapshot_flag_and_reg.sh");
			memcpy(ptfbuf, &SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG, sizeof(int));
			udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
			udprecvfrom(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.snapshotserver.snapshotclient_for_ptf");
			INVARIANT(ptf_recvsize == sizeof(int) && *((int *)ptfbuf) == SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG_ACK);

			// finish snapshot
			INVARIANT(is_snapshot == true && is_snapshot_end == false && popworker_know_snapshot_end == false && specialcaseserver_know_snapshot_end == false);
			is_snapshot_end = true;
			is_snapshot = false;

			// wait for case2 from popworker and case1 from specialcaseserver
			while (!popworker_know_snapshot_end || !specialcaseserver_know_snapshot_end) {}

			// end snapshot (by now popworker/specialcaseserver will not access switchos_specialcases_ptr and backuped metadata)
			is_snapshot_end = false;
			popworker_know_snapshot_end = false;
			specialcaseserver_know_snapshot_end = false;

#ifdef DEBUG_SNAPSHOT
			// TMPDEBUG
			printf("[before rollback] snapshot size: %d\n", switchos_cached_empty_index_backup);
			/*for (size_t debugi = 0; debugi < switchos_cached_empty_index_backup; debugi++) {
				char debugbuf[MAX_BUFSIZE];
				uint32_t debugkeysize = switchos_cached_keyarray_backup[debugi].serialize(debugbuf, MAX_BUFSIZE);
				uint32_t debugvalsize = switchos_snapshot_values[debugi].serialize(debugbuf+debugkeysize, MAX_BUFSIZE-debugkeysize);
				printf("serialized debug key-value[%d]:\n", int(debugi));
				dump_buf(debugbuf, debugkeysize+debugvalsize);
				printf("seq: %d, stat %d\n", switchos_snapshot_seqs[debugi], switchos_snapshot_stats[debugi]?1:0);
			}*/
#endif

			// perform rollback (now both popserver/specicalserver will not touch specialcases)
			/*for (std::map<uint16_t, special_case_t>::iterator iter = switchos_specialcases.begin(); iter != switchos_specialcases.end(); iter++) {
				INVARIANT(switchos_cached_keyarray_backup[iter->first] == iter->second._key);
				switchos_snapshot_values[iter->first] = iter->second._val;
				switchos_snapshot_seqs[iter->first] = iter->second._seq;
				switchos_snapshot_stats[iter->first] = iter->second._valid;
			}*/
			INVARIANT(switchos_specialcases_ptr != NULL);
			concurrent_specicalcase_map_t::DataSource source(0, (concurrent_specicalcase_map_t *)switchos_specialcases_ptr);
			source.advance_to_next_valid();
			while (source.has_next) {
				special_case_t tmpcase = source.get_val();
				INVARIANT(switchos_cached_keyarray_backup[source.get_key()] == tmpcase._key);
				switchos_snapshot_values[source.get_key()] = tmpcase._val;
				switchos_snapshot_seqs[source.get_key()] = tmpcase._seq;
				switchos_snapshot_stats[source.get_key()] = tmpcase._valid;
				source.advance_to_next_valid();
			}

#ifdef DEBUG_SNAPSHOT
			// TMPDEBUG
			printf("[after rollback] snapshot size: %d\n", switchos_cached_empty_index_backup);
			/*for (size_t debugi = 0; debugi < switchos_cached_empty_index_backup; debugi++) {
				char debugbuf[MAX_BUFSIZE];
				uint32_t debugkeysize = switchos_cached_keyarray_backup[debugi].serialize(debugbuf, MAX_BUFSIZE);
				uint32_t debugvalsize = switchos_snapshot_values[debugi].serialize(debugbuf+debugkeysize, MAX_BUFSIZE-debugkeysize);
				printf("serialized debug key-value[%d]:\n", int(debugi));
				dump_buf(debugbuf, debugkeysize+debugvalsize);
				printf("seq: %d, stat %d\n", switchos_snapshot_seqs[debugi], switchos_snapshot_stats[debugi]?1:0);
			}*/
#endif

			// snapshot data: <int SNAPSHOT_GETDATA_ACK, int32_t total_bytes, per-server data>
			// per-server data: <int32_t perserver_bytes, uint16_t serveridx, int32_t recordcnt, per-record data>
			// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>
			memset((void *)tmp_send_bytes, 0, server_num*sizeof(int));
			memset((void *)tmp_record_cnts, 0, server_num*sizeof(int));
			for (size_t i = 0; i < server_num; i++) {
				tmp_sendbuf_list[i].clear();
			}
			sendbuf.clear();
			for (uint32_t tmpidx = 0; tmpidx < switchos_cached_empty_index_backup; tmpidx++) { // prepare per-server per-record data
				uint16_t tmp_serveridx = switchos_cached_serveridxarray_backup[tmpidx];
				dynamic_array_t &tmp_sendbuf = tmp_sendbuf_list[tmp_serveridx];
				uint32_t tmp_keysize = switchos_cached_keyarray_backup[tmpidx].dynamic_serialize(tmp_sendbuf, tmp_send_bytes[tmp_serveridx]);
				tmp_send_bytes[tmp_serveridx] += tmp_keysize;
				uint32_t tmp_valsize = switchos_snapshot_values[tmpidx].dynamic_serialize(tmp_sendbuf, tmp_send_bytes[tmp_serveridx]);
				tmp_send_bytes[tmp_serveridx] += tmp_valsize;
				tmp_sendbuf.dynamic_memcpy(tmp_send_bytes[tmp_serveridx], (char *)&switchos_snapshot_seqs[tmpidx], sizeof(uint32_t));
				tmp_send_bytes[tmp_serveridx] += sizeof(uint32_t);
				tmp_sendbuf.dynamic_memcpy(tmp_send_bytes[tmp_serveridx], (char *)&switchos_snapshot_stats[tmpidx], sizeof(bool));
				tmp_send_bytes[tmp_serveridx] += sizeof(bool);
				tmp_record_cnts[tmp_serveridx] += 1;
			}
			int total_bytes = sizeof(int) + sizeof(int32_t); // leave 4B for SNAPSHOT_DATA and total_bytes
			for (uint16_t tmp_serveridx = 0; tmp_serveridx < server_num; tmp_serveridx++) {
				if (tmp_record_cnts[tmp_serveridx] > 0) {
					int32_t tmp_perserver_bytes = tmp_send_bytes[tmp_serveridx] + sizeof(int32_t) + sizeof(uint16_t) + sizeof(int);
					sendbuf.dynamic_memcpy(total_bytes, (char *)&tmp_perserver_bytes, sizeof(int32_t));
					total_bytes += sizeof(int32_t);
					sendbuf.dynamic_memcpy(total_bytes, (char *)&tmp_serveridx, sizeof(uint16_t));
					total_bytes += sizeof(uint16_t);
					sendbuf.dynamic_memcpy(total_bytes, (char *)&tmp_record_cnts[tmp_serveridx], sizeof(int));
					total_bytes += sizeof(int);
					sendbuf.dynamic_memcpy(total_bytes, tmp_sendbuf_list[tmp_serveridx].array(), tmp_send_bytes[tmp_serveridx]);
					total_bytes += tmp_send_bytes[tmp_serveridx];
				}
			}
			sendbuf.dynamic_memcpy(0, (char *)&SNAPSHOT_GETDATA_ACK, sizeof(int)); // set 1st 4B as SNAPSHOT_GETDATA_ACK
			sendbuf.dynamic_memcpy(0 + sizeof(int), (char *)&total_bytes, sizeof(int32_t)); // set 2nd 4B as total_bytes
			INVARIANT(total_bytes <= MAX_LARGE_BUFSIZE);

			// send rollbacked snapshot data to controller.snapshotserver
			printf("[switchos.snapshotserver] send snapshot data to controller\n"); // TMPDEBUG
			udpsendlarge_udpfrag(switchos_snapshotserver_udpsock, sendbuf.array(), total_bytes, 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "switchos.snapshotserver");
		}
		else {
			printf("[switchos.snapshotserver] invalid control type: %d\n", control_type);
			exit(-1);
		}
	} // while (switchos_running)

	// send SWITCHOS_PTF_SNAPSHOTSERVER_END to ptf.snapshotserver
	memcpy(ptfbuf, &SWITCHOS_PTF_SNAPSHOTSERVER_END, sizeof(int));
	udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");

	close(switchos_snapshotserver_udpsock);
	close(switchos_snapshotserver_snapshotclient_for_ptf_udpsock);
	pthread_exit(nullptr);
}

void *run_switchos_specialcaseserver(void *param) {
	// reflector addr (not used) 
	//struct sockaddr_in reflector_addr;
	//unsigned int reflector_addr_len = sizeof(struct sockaddr);

	char buf[MAX_BUFSIZE];
	printf("[switchos.specialcaseserver] ready\n");
	switchos_ready_threads++;
	while (!switchos_running) {}

	while (switchos_running) {
		memset(buf, 0, MAX_BUFSIZE);

		if (is_snapshot || (is_snapshot_end && !specialcaseserver_know_snapshot_end)) {
			int recv_size = 0;
			bool is_timeout = udprecvfrom(switchos_specialcaseserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "switchos.specialcaseserver");
			if (is_timeout) {
				if (is_snapshot_end && !specialcaseserver_know_snapshot_end) {
					specialcaseserver_know_snapshot_end = true;
				}
				continue;
			}

			packet_type_t pkt_type = get_packet_type(buf, recv_size);
			switch (pkt_type) {
				case packet_type_t::GETRES_LATEST_SEQ_INSWITCH_CASE1:
					{
						getres_latest_seq_inswitch_case1_t req(buf, recv_size);
						process_specialcase(req.idx(), req.key(), req.val(), req.seq(), req.stat());
						break;
					}
				case packet_type_t::GETRES_DELETED_SEQ_INSWITCH_CASE1:
					{
						getres_deleted_seq_inswitch_case1_t req(buf, recv_size);
						process_specialcase(req.idx(), req.key(), req.val(), req.seq(), req.stat());
						break;
					}
				case packet_type_t::PUTREQ_SEQ_INSWITCH_CASE1:
					{
						putreq_seq_inswitch_case1_t req(buf, recv_size);
						process_specialcase(req.idx(), req.key(), req.val(), req.seq(), req.stat());
						break;
					}
				case packet_type_t::DELREQ_SEQ_INSWITCH_CASE1:
					{
						delreq_seq_inswitch_case1_t req(buf, recv_size);
						process_specialcase(req.idx(), req.key(), req.val(), req.seq(), req.stat());
						break;
					}
				default:
					{
						printf("[switchos.specialcaseserver] Invalid packet type from reflector.worker.specialcaseclient!\n");
						exit(-1);
					}
			} // end of switch
		} // is_snapshot || (is_snapshot_end && !specialcaseserver_know_snapshot_end)
	}

	close(switchos_specialcaseserver_udpsock);
	pthread_exit(nullptr);
}

void process_specialcase(const uint16_t &tmpidx, const netreach_key_t &tmpkey, const val_t &tmpval, const uint32_t &tmpseq, const bool &tmpstat) {
	// verify key (NOTE: use switchos_cached_key_idx_map_backup) 
	/*std::map<netreach_key_t, uint16_t>::iterator key_idx_map_iter = switchos_cached_key_idx_map_backup.find(req.key()); 
	if (key_idx_map_iter == switchos_cached_key_idx_map_backup.end()) { // unmatched key 
		break; 
	} 
	uint16_t tmpidx = key_idx_map_iter->second;*/

	INVARIANT(tmpidx >= 0 && tmpidx < switch_kv_bucket_num);
	INVARIANT(switchos_specialcases_ptr != NULL && switchos_cached_keyarray_backup != NULL);
	INVARIANT(switchos_cached_empty_index_backup >=0 && switchos_cached_empty_index_backup <= switch_kv_bucket_num);
	if (tmpidx < switchos_cached_empty_index_backup && switchos_cached_keyarray_backup[tmpidx] == tmpkey) { 
		//switchos_mutex_for_specialcases.lock(); 
		//std::map<uint16_t, special_case_t>::iterator specialcase_iter = switchos_specialcases.find(tmpidx); 
		special_case_t tmpcase;
		bool res = switchos_specialcases_ptr->get(tmpidx, tmpcase);
		//if (specialcase_iter == switchos_specialcases.end()) { // no special case for the idx/key
		if (!res) { // no special case for the idx/key
			//special_case_t tmpcase; 
			tmpcase._key = tmpkey;
			tmpcase._val = tmpval;
			tmpcase._seq = tmpseq;
			tmpcase._valid = tmpstat; // stat=1 means not deleted
			//switchos_specialcases.insert(std::pair<uint16_t, special_case_t>(tmpidx, tmpcase)); 
			switchos_specialcases_ptr->insert(tmpidx, tmpcase);
		} 
		//else if (req.seq() < specialcase_iter->second.seq()) { // current special case is more close to snapshot timepoint
		else if (tmpseq < tmpcase._seq) { // current special case is more close to snapshot timepoint
			//INVARIANT(specialcase_iter->second._key == tmpkey); 
			//specialcase_iter->second._val = tmpval;
			//specialcase_iter->second._seq = tmpseq; 
			//specialcase_iter->second._valid = tmpstat; 
			INVARIANT(tmpcase._key == tmpkey);
			tmpcase._val = tmpval;
			tmpcase._seq = tmpseq;
			tmpcase._valid = tmpstat; // stat=1 means not deleted
			res = switchos_specialcases_ptr->update(tmpidx, tmpcase);
			INVARIANT(res == true);
		} 
		//switchos_mutex_for_specialcases.unlock(); 
	} 
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
	if (switchos_cached_keyarray != NULL) {
		delete [] switchos_cached_keyarray;
		switchos_cached_keyarray = NULL;
	}
	if (switchos_cached_serveridxarray != NULL) {
		delete [] switchos_cached_serveridxarray;
		switchos_cached_serveridxarray = NULL;
	}
	/*if (switchos_evictvalbytes != NULL) {
		delete [] switchos_evictvalbytes;
		switchos_evictvalbytes = NULL;
	}*/
	if (switchos_cached_keyarray_backup != NULL) {
		delete [] switchos_cached_keyarray_backup;
		switchos_cached_keyarray_backup = NULL;
	}
	if (switchos_cached_serveridxarray_backup != NULL) {
		delete [] switchos_cached_serveridxarray_backup;
		switchos_cached_serveridxarray_backup = NULL;
	}
	if (switchos_specialcases_ptr != NULL) {
		delete switchos_specialcases_ptr;
		switchos_specialcases_ptr = NULL;
	}
}

// switchos <-> ptf.popserver

inline uint32_t serialize_setvalid0(char *buf, uint16_t freeidx) {
	memcpy(buf, &SWITCHOS_SETVALID0, sizeof(int));
	memcpy(buf + sizeof(int), &freeidx, sizeof(uint16_t));
	return sizeof(int) + sizeof(uint16_t);
}

inline uint32_t serialize_add_cache_lookup_setvalid1(char *buf, netreach_key_t key, uint16_t freeidx) {
	memcpy(buf, &SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1, sizeof(int));
	uint32_t tmp_keysize = key.serialize(buf + sizeof(int), MAX_BUFSIZE - sizeof(int));
	memcpy(buf + sizeof(int) + tmp_keysize, &freeidx, sizeof(uint16_t));
	return sizeof(int) + tmp_keysize + sizeof(uint16_t);
}

inline uint32_t serialize_get_evictdata_setvalid3(char *buf) {
	memcpy(buf, &SWITCHOS_GET_EVICTDATA_SETVALID3, sizeof(int));
	return sizeof(int);
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
}

inline uint32_t serialize_remove_cache_lookup(char *buf, netreach_key_t key) {
	memcpy(buf, &SWITCHOS_REMOVE_CACHE_LOOKUP, sizeof(int));
	uint32_t tmp_keysize = key.serialize(buf + sizeof(int), MAX_BUFSIZE - sizeof(int));
	return sizeof(int) + tmp_keysize;
}

inline uint32_t serialize_load_snapshot_data(char *buf, uint32_t emptyidx) {
	memcpy(buf, &SWITCHOS_LOAD_SNAPSHOT_DATA, sizeof(int));
	memcpy(buf + sizeof(int), &emptyidx, sizeof(uint32_t));
	return sizeof(int) + sizeof(uint32_t);
}

void parse_snapshotdata_fromptf(char *buf, uint32_t buflen, \
		val_t *values, uint32_t *seqs, bool *stats, uint32_t record_cnt) {
	int control_type = *((int *)buf);
	INVARIANT(control_type == SWITCHOS_LOAD_SNAPSHOT_DATA_ACK);
	int total_bytesnum = *((int32_t *)(buf + sizeof(int)));
	INVARIANT(total_bytesnum <= int(buflen));

	// save snapshot data: <SWITCHOS_LOAD_SNAPSHOT_DATA_ACK, total_bytesnum, records>
	// for each record: <vallen (big-endian), valbytes (same order), seq, result>
	uint32_t tmp_offset = sizeof(int) + sizeof(int32_t);
	for (uint32_t tmp_recordidx = 0; tmp_recordidx < record_cnt; tmp_recordidx++) {
		uint32_t tmp_valsize = values[tmp_recordidx].deserialize(buf + tmp_offset, buflen - tmp_offset);
		tmp_offset += tmp_valsize;
		seqs[tmp_recordidx] = *((uint32_t *)(buf + tmp_offset));
		tmp_offset += sizeof(uint32_t);
		stats[tmp_recordidx] = *((bool *)(buf + tmp_offset));
		tmp_offset += sizeof(bool);
		INVARIANT(int32_t(tmp_offset) <= total_bytesnum);
	}
	memory_fence();
	return;
}
