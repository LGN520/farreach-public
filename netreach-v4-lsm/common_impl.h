#ifndef COMMON_IMPL_H
#define COMMON_IMPL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "key.h"
#include "val.h"
#include "packet_format_impl.h"
#include "socket_helper.h"
#include "message_queue_impl.h"
#include "iniparser/iniparser_wrapper.h"
#include "workloadparser/parser.h"

/*
 * Class and alias
 */

typedef GetRequest<netreach_key_t> get_request_t;
typedef PutRequest<netreach_key_t, val_t> put_request_t;
typedef DelRequest<netreach_key_t> del_request_t;
typedef ScanRequest<netreach_key_t> scan_request_t;
typedef GetResponse<netreach_key_t, val_t> get_response_t;
typedef PutResponse<netreach_key_t> put_response_t;
typedef DelResponse<netreach_key_t> del_response_t;
typedef ScanResponseSplit<netreach_key_t, val_t> scan_response_split_t;
typedef GetRequestPOP<netreach_key_t> get_request_pop_t;
typedef GetRequestNLatest<netreach_key_t> get_request_nlatest_t;
typedef GetResponseLatestSeq<netreach_key_t, val_t> get_response_latest_seq_t;
typedef GetResponseLatestSeqInswitchCase1<netreach_key_t, val_t> get_response_latest_seq_inswitch_case1_t;
typedef GetResponseDeletedSeq<netreach_key_t, val_t> get_response_deleted_seq_t;
typedef GetResponseDeletedSeqInswitchCase1<netreach_key_t, val_t> get_response_deleted_seq_inswitch_case1_t;
typedef PutRequestSeq<netreach_key_t, val_t> put_request_seq_t;
typedef PutRequestPopSeq<netreach_key_t, val_t> put_request_pop_seq_t;
typedef PutRequestSeqInswitchCase1<netreach_key_t, val_t> put_request_seq_inswitch_case1_t;
typedef PutRequestSeqCase3<netreach_key_t, val_t> put_request_seq_case3_t;
typedef PutRequestPopSeqCase3<netreach_key_t, val_t> put_request_pop_seq_case3_t;
typedef DelRequestSeq<netreach_key_t> del_request_seq_t;
typedef DelRequestSeqInswitchCase1<netreach_key_t, val_t> del_request_seq_inswitch_case1_t;
typedef DelRequestSeqCase3<netreach_key_t> del_request_seq_case3_t;
typedef ScanRequestSplit<netreach_key_t> scan_request_split_t;
typedef CachePop<netreach_key_t, val_t> cache_pop_t;
typedef CachePopInswitch<netreach_key_t, val_t> cache_pop_inswitch_t;
typedef CachePopInswitchAck<netreach_key_t> cache_pop_inswitch_ack_t;
typedef CacheEvict<netreach_key_t, val_t> cache_evict_t;
typedef CacheEvictAck<netreach_key_t> cache_evict_ack_t;
typedef CacheEvictCase2<netreach_key_t, val_t> cache_evict_case2_t;
typedef WarmupRequest<netreach_key_t, val_t> warmup_request_t;
typedef WarmupAck<netreach_key_t> warmup_ack_t;
typedef LoadRequest<netreach_key_t, val_t> load_request_t;
typedef LoadAck<netreach_key_t> load_ack_t;
typedef CachePopAck<netreach_key_t> cache_pop_ack_t;

/*
 * Constants
 */

//const double dpdk_polling_time = 21.82; // Test by enabling TEST_DPDK_POLLING, and only transmit packets between client and switch
const double dpdk_polling_time = 0.0;
size_t max_sending_rate = size_t(20 * 1024); // limit sending rate to x (e.g., the aggregate rate of servers)
const size_t rate_limit_period = 1000 * 1000; // 1s

/*
 * Parameters
 */

// global
const char *workload_name = nullptr;
int workload_mode = 0;
int dynamic_periodnum = 0;
int dynamic_periodinterval = 0;
const char *dynamic_ruleprefix = nullptr;

// client
size_t client_num;
short client_port_start;
const char *client_ip;
uint8_t client_macaddr[6];
//const char *client_ip_for_server;

// server: loading phase
uint32_t load_factor = 1;
//uint32_t split_n;
//uint32_t load_n;

// server: transaction phase
uint32_t server_num = 1;
short server_port_start = -1;
// switch os simulator for processing special cases and packet loss handling (and tcp servers for workers)
const char* server_ip = nullptr;
uint8_t server_macaddr[6];
const char* server_ip_for_controller = nullptr;
short server_evictserver_port_start = -1;
short server_snapshotserver_port_start = -1;
short server_snapshotdataserver_port_start = -1;
//short server_dynamicserver_port = -1;
//const char *server_ip_for_client;

// controller
const char *controller_ip_for_server = nullptr;
const char *controller_ip_for_switchos = nullptr;
short controller_popserver_port_start = -1;
short controller_evictserver_port = -1;
uint32_t controller_snapshot_period = 0; // ms

// switch
uint32_t partition_count;
uint32_t switch_kv_bucket_num;
short switchos_popserver_port = -1;
//short switchos_paramserver_port = -1;
const char *switchos_ip = nullptr;
uint32_t switchos_sample_cnt = 0;
short switchos_snapshotserver_port = -1;
short switchos_specialcaseserver_port = -1;
//short switchos_snapshotdataserver_port = -1;
short switchos_ptf_popserver_port = -1;
short switchos_ptf_snapshotserver_port = -1;

// reflector
const char *reflector_ip_for_switchos = nullptr;
short reflector_port = -1;
short reflector_popserver_port = -1;

// calculated metadata
char raw_load_workload_filename[256]; // used by split_workload for loading phase
char server_load_workload_dir[256];
char raw_warmup_workload_filename[256];
char raw_run_workload_filename[256]; // used by split_workload for transaction phase
char client_workload_dir[256];
size_t per_client_per_period_max_sending_rate;
uint64_t perserver_keyrange = 0; // use size_t to avoid int overflow

// others (xindex)
size_t bg_n = 1;

// Packet types used by switchos.paramserver and ptf framework
/*int SWITCHOS_GET_FREEIDX = -1; // ptf get freeidx from paramserver
int SWITCHOS_GET_KEY_FREEIDX = -1; // ptf get key and freeidx from paramserver
int SWITCHOS_SET_EVICTDATA = -1; // ptf set evictidx, evictvallen, evictval, evictstat, and evictseq to paramserver
int SWITCHOS_GET_EVICTKEY = -1; // ptf get evictkey
int SWITCHOS_GET_CACHEDEMPTYINDEX = -1; // ptf get cached_empty_index*/
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

/*
 * Get configuration
 */

inline void parse_ini(const char * config_file);
inline void parse_control_ini(const char * config_file);
//inline void parse_args(int, char **);

inline void parse_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	// global
	workload_name = ini.get_workload_name();
	workload_mode = ini.get_workload_mode();
	printf("workload_name: %s\n", workload_name);
	COUT_VAR(workload_mode);
	dynamic_periodnum = ini.get_dynamic_periodnum();
	dynamic_periodinterval = ini.get_dynamic_periodinterval();
	dynamic_ruleprefix = ini.get_dynamic_ruleprefix();
	COUT_VAR(dynamic_periodnum);
	COUT_VAR(dynamic_periodinterval);
	printf("dynamic_ruleprefix: %s\n", dynamic_ruleprefix);
	val_t::MAX_VALLEN = ini.get_max_vallen();
	ParserIterator::load_batch_size = ini.get_load_batch_size();
	COUT_VAR(val_t::MAX_VALLEN);
	val_t::MAX_VALLEN = ini.get_max_vallen();
	COUT_VAR(val_t::MAX_VALLEN);
	COUT_VAR(ParserIterator::load_batch_size);

	// client
	client_num = ini.get_client_num();
	client_port_start = ini.get_client_port();
	client_ip = ini.get_client_ip();
	ini.get_client_mac(client_macaddr);
	//client_ip_for_server = ini.get_client_ip_for_server();
	COUT_VAR(client_num);
	COUT_VAR(client_port_start);
	printf("client_ip: %s\n", client_ip);
	printf("client_macaddr: ");
	for (size_t i = 0; i < 6; i++) {
		printf("%02x", client_macaddr[i]);
		if (i != 5) printf(":");
		else printf("\n");
	}
	//printf("client_ip_for_server: %s\n", client_ip_for_server);

	// server: loading phase
	load_factor = ini.get_load_factor();
	//split_n = ini.get_split_num();
	//INVARIANT(split_n >= 2);
	//load_n = split_n - 1;
	COUT_VAR(load_factor);
	//COUT_VAR(split_n);
	//COUT_VAR(load_n);

	// server: transaction phase
	server_num = ini.get_server_num();
	//INVARIANT(server_num >= load_n);
	server_port_start = ini.get_server_port();
	server_ip = ini.get_server_ip();
	ini.get_server_mac(server_macaddr);
	server_ip_for_controller = ini.get_server_ip_for_controller();
	server_evictserver_port_start = ini.get_server_evictserver_port();
	server_snapshotserver_port_start = ini.get_server_snapshotserver_port();
	server_snapshotdataserver_port_start = ini.get_server_snapshotdataserver_port();
	//server_dynamicserver_port = ini.get_server_dynamicserver_port();
	//server_ip_for_client = ini.get_server_ip_for_client();
	COUT_VAR(server_num);
	COUT_VAR(server_port_start);
	printf("server_ip: %s\n", server_ip);
	printf("server_macaddr: ");
	for (size_t i = 0; i < 6; i++) {
		printf("%02x", server_macaddr[i]);
		if (i != 5) printf(":");
		else printf("\n");
	}
	COUT_VAR(server_evictserver_port_start);
	COUT_VAR(server_snapshotserver_port_start);
	COUT_VAR(server_snapshotdataserver_port_start);
	//COUT_VAR(server_dynamicserver_port);
	//printf("server_ip_for_client: %s\n", server_ip_for_client);

	// controller
	controller_ip_for_server = ini.get_controller_ip_for_server();
	controller_ip_for_switchos = ini.get_controller_ip_for_switchos();
	controller_popserver_port_start = ini.get_controller_popserver_port();
	controller_evictserver_port = ini.get_controller_evictserver_port();
	controller_snapshot_period = ini.get_controller_snapshot_period();
	printf("controller ip for server: %s\n", controller_ip_for_server);
	printf("controller ip for switchos: %s\n", controller_ip_for_switchos);
	COUT_VAR(controller_popserver_port_start);
	COUT_VAR(controller_evictserver_port);
	COUT_VAR(controller_snapshot_period);
	
	// switch
	partition_count = ini.get_partition_count();
	switch_kv_bucket_num = ini.get_switch_kv_bucket_num();
	switchos_popserver_port = ini.get_switchos_popserver_port();
	//switchos_paramserver_port = ini.get_switchos_paramserver_port();
	switchos_ip = ini.get_switchos_ip();
	switchos_sample_cnt = ini.get_switchos_sample_cnt();
	switchos_snapshotserver_port = ini.get_switchos_snapshotserver_port();
	switchos_specialcaseserver_port = ini.get_switchos_specialcaseserver_port();
	switchos_ptf_popserver_port = ini.get_switchos_popserver_port();
	switchos_ptf_snapshotserver_port = ini.get_switchos_snapshotserver_port();
	//switchos_snapshotdataserver_port = ini.get_switchos_snapshotdataserver_port();
	COUT_VAR(partition_count);
	COUT_VAR(switch_kv_bucket_num);
	val_t::SWITCH_MAX_VALLEN = ini.get_switch_max_vallen();
	COUT_VAR(val_t::SWITCH_MAX_VALLEN);
	COUT_VAR(switchos_popserver_port);
	//COUT_VAR(switchos_paramserver_port);
	printf("switchos ip: %s\n", switchos_ip);
	COUT_VAR(switchos_sample_cnt);
	COUT_VAR(switchos_snapshotserver_port);
	COUT_VAR(switchos_specialcaseserver_port);
	//COUT_VAR(switchos_snapshotdataserver_port);
	COUT_VAR(switchos_popserver_port);
	COUT_VAR(switchos_snapshotserver_port);

	// reflector
	reflector_ip_for_switchos = ini.get_reflector_ip_for_switchos();
	reflector_port = ini.get_reflector_port();
	reflector_popserver_port = ini.get_reflector_popserver_port();
	printf("reflector ip for switchos: %s\n", reflector_ip_for_switchos);
	COUT_VAR(reflector_port);
	COUT_VAR(reflector_popserver_port);

	// calculated metadata

	LOAD_RAW_WORKLOAD(raw_load_workload_filename, workload_name);
	//LOAD_SPLIT_DIR(server_load_workload_dir, workload_name, int(split_n)); // get the split directory for loading phase
	LOAD_SPLIT_DIR(server_load_workload_dir, workload_name, int(server_num)); // get the split directory for loading phase
	WARMUP_RAW_WORKLOAD(raw_warmup_workload_filename, workload_name);
	RUN_RAW_WORKLOAD(raw_run_workload_filename, workload_name);
	RUN_SPLIT_DIR(client_workload_dir, workload_name, int(client_num));
	max_sending_rate *= server_num;
	per_client_per_period_max_sending_rate = max_sending_rate / client_num / (1 * 1000 * 1000 / rate_limit_period);
	//perserver_keyrange = 4ll*1024ll*1024ll*1024ll / int64_t(server_num); // 2^32 / server_num
	perserver_keyrange = 64*1024 / server_num; // 2^16 / server_num
	printf("raw_load_workload_filename for loading phase: %s\n", raw_load_workload_filename);
	printf("server_load_workload_dir for loading phase: %s\n", server_load_workload_dir);
	printf("raw_warmup_workload_filename for warmup phase: %s\n", raw_warmup_workload_filename);
	printf("raw_run_workload_filename for transaction phase: %s\n", raw_run_workload_filename);
	printf("client_workload_dir for transaction phase: %s\n", client_workload_dir);
	COUT_VAR(perserver_keyrange);
}

inline void parse_control_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	/*SWITCHOS_GET_FREEIDX = ini.get_switchos_get_freeidx();
	SWITCHOS_GET_KEY_FREEIDX = ini.get_switchos_get_key_freeidx();
	SWITCHOS_SET_EVICTDATA = ini.get_switchos_set_evictdata();
	SWITCHOS_GET_EVICTKEY = ini.get_switchos_get_evictkey();
	SWITCHOS_GET_CACHEDEMPTYINDEX = ini.get_switchos_get_cachedemptyindex();
	SNAPSHOT_START = ini.get_snapshot_start();
	SNAPSHOT_SERVERSIDE = ini.get_snapshot_serverside();
	SNAPSHOT_SERVERSIDE_ACK = ini.get_snapshot_serverside_ack();
	SNAPSHOT_DATA = ini.get_snapshot_data();
	COUT_VAR(SWITCHOS_GET_FREEIDX);
	COUT_VAR(SWITCHOS_GET_KEY_FREEIDX);
	COUT_VAR(SWITCHOS_SET_EVICTDATA);
	COUT_VAR(SWITCHOS_GET_EVICTKEY);
	COUT_VAR(SWITCHOS_GET_CACHEDEMPTYINDEX);
	COUT_VAR(SNAPSHOT_START);
	COUT_VAR(SNAPSHOT_SERVERSIDE);
	COUT_VAR(SNAPSHOT_SERVERSIDE_ACK);
	COUT_VAR(SNAPSHOT_DATA);*/

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

/*inline void parse_args(int argc, char **argv) {
  struct option long_options[] = {
      {"bg", required_argument, 0, 'i'},
      {"xindex-root-err-bound", required_argument, 0, 'j'},
      {"xindex-root-memory", required_argument, 0, 'k'},
      {"xindex-group-err-bound", required_argument, 0, 'l'},
      {"xindex-group-err-tolerance", required_argument, 0, 'm'},
      {"xindex-buf-size-bound", required_argument, 0, 'n'},
      {"xindex-buf-compact-threshold", required_argument, 0, 'o'},
      {0, 0, 0, 0}};
  std::string ops = "i:j:k:l:m:n:o:";
  int option_index = 0;

  while (1) {
    int c = getopt_long(argc, argv, ops.c_str(), long_options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 0:
        if (long_options[option_index].flag != 0) break;
        abort();
        break;
      case 'i':
        bg_n = strtoul(optarg, NULL, 10);
        break;
      case 'j':
        xindex::config.root_error_bound = strtol(optarg, NULL, 10);
        INVARIANT(xindex::config.root_error_bound > 0);
        break;
      case 'k':
        xindex::config.root_memory_constraint =
            strtol(optarg, NULL, 10) * 1024 * 1024;
        INVARIANT(xindex::config.root_memory_constraint > 0);
        break;
      case 'l':
        xindex::config.group_error_bound = strtol(optarg, NULL, 10);
        INVARIANT(xindex::config.group_error_bound > 0);
        break;
      case 'm':
        xindex::config.group_error_tolerance = strtol(optarg, NULL, 10);
        INVARIANT(xindex::config.group_error_tolerance > 0);
        break;
      case 'n':
        xindex::config.buffer_size_bound = strtol(optarg, NULL, 10);
        INVARIANT(xindex::config.buffer_size_bound > 0);
        break;
      case 'o':
        xindex::config.buffer_compact_threshold = strtol(optarg, NULL, 10);
        INVARIANT(xindex::config.buffer_compact_threshold > 0);
        break;
      default:
        abort();
    }
  }

  COUT_VAR(bg_n);
  COUT_VAR(xindex::config.root_error_bound);
  COUT_VAR(xindex::config.root_memory_constraint);
  COUT_VAR(xindex::config.group_error_bound);
  COUT_VAR(xindex::config.group_error_tolerance);
  COUT_VAR(xindex::config.buffer_size_bound);
  COUT_VAR(xindex::config.buffer_size_tolerance);
  COUT_VAR(xindex::config.buffer_compact_threshold);
}*/

#endif
