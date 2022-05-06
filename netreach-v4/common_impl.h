#ifndef COMMON_IMPL_H
#define COMMON_IMPL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "packet_format_impl.h"
#include "socket_helper.h"
#include "message_queue_impl.h"

/*
 * Class and alias
 */

typedef Key index_key_t;
#ifdef ORIGINAL_XINDEX
typedef uint64_t val_t;
#else
#include "val.h"
typedef Val val_t;
#endif

typedef GetRequest<index_key_t> get_request_t;
typedef PutRequest<index_key_t, val_t> put_request_t;
typedef DelRequest<index_key_t> del_request_t;
typedef ScanRequest<index_key_t> scan_request_t;
typedef GetResponse<index_key_t, val_t> get_response_t;
typedef PutResponse<index_key_t> put_response_t;
typedef DelResponse<index_key_t> del_response_t;
typedef ScanResponseSplit<index_key_t, val_t> scan_response_split_t;
typedef GetRequestPOP<index_key_t> get_request_pop_t;
typedef GetRequestNLatest<index_key_t> get_request_nlatest_t;
typedef GetResponseLatestSeq<index_key_t, val_t> get_response_latest_seq_t;
typedef GetResponseLatestSeqInswitchCase1<index_key_t, val_t> get_response_latest_seq_inswitch_case1_t;
typedef GetResponseDeletedSeq<index_key_t, val_t> get_response_deleted_seq_t;
typedef GetResponseDeletedSeqInswitchCase1<index_key_t, val_t> get_response_deleted_seq_inswitch_case1_t;
typedef PutRequestSeq<index_key_t, val_t> put_request_seq_t;
typedef PutRequestPopSeq<index_key_t, val_t> put_request_pop_seq_t;
typedef PutRequestSeqInswitchCase1<index_key_t, val_t> put_request_seq_inswitch_case1_t;
typedef PutRequestSeqCase3<index_key_t, val_t> put_request_seq_case3_t;
typedef PutRequestPopSeqCase3<index_key_t, val_t> put_request_pop_seq_case3_t;
typedef DelRequestSeq<index_key_t> del_request_seq_t;
typedef DelRequestSeqInswitchCase1<index_key_t, val_t> del_request_seq_inswitch_case1_t;
typedef DelRequestSeqCase3<index_key_t> del_request_seq_case3_t;
typedef ScanRequestSplit<index_key_t> scan_request_split_t;
typedef CachePop<index_key_t, val_t> cache_pop_t;
typedef CachePopInSwitch<index_key_t, val_t> cache_pop_inswitch_t;
typedef CachePopInSwitchAck<index_key_t> cache_pop_inswitch_ack_t;
typedef CacheEvict<index_key_t, val_t> cache_evict_t;
typedef CacheEvictAck<index_key_t> cache_evict_ack_t;
typedef CacheEvictCase2<index_key_t, val_t> cache_evict_case2_t;

/*
 * Constants
 */

const double dpdk_polling_time = 21.82; // Test by enabling TEST_DPDK_POLLING, and only transmit packets between client and switch
const size_t max_sending_rate = size_t(1.2 * 1024 * 1024); // 1.2 MQPS; limit sending rate to x (e.g., the aggregate rate of servers)
const size_t rate_limit_period = 10 * 1000; // 10 * 1000us

/*
 * Parameters
 */

// global
const char *workload_name = nullptr;

// client
size_t client_num;
short client_port_start;
const char *client_ip;
uint8_t client_macaddr[6];
char client_workload_dir[256];
size_t per_client_per_period_max_sending_rate;

// server: loading phase
uint32_t split_n;
uint32_t load_n;
char server_load_workload_dir[256];

// server: transaction phase
uint32_t server_num = 1;
short server_port_start = -1;
// switch os simulator for processing special cases and packet loss handling (and tcp servers for workers)
const char* server_ip = nullptr;
uint8_t server_macaddr[6];
// backuper for processing in-switch snapshot
const char* backup_ip = nullptr;
short backup_port;
// notified for processing explicit notification of server-side snapshot
short notified_port;
short server_evictserver_port_start = -1;
short server_consnapshotserver_port = -1;
int64_t perserver_keyrange = -1; // use size_t to avoid int overflow

// controller
const char *controller_ip_for_server = nullptr;
const char *controller_ip_for_switchos = nullptr;
//short controller_popserver_port_start = -1;
short controller_popserver_port = -1;
short controller_evictserver_port = -1;
uint32_t controller_snapshot_period = -1; // ms

// switch
uint32_t switch_kv_bucket_num;
short switchos_popserver_port = -1;
short switchos_paramserver_port = -1;
const char *switchos_ip = nullptr;
uint32_t switchos_sample_cnt = 0;
short switchos_snapshotserver_port = -1;
short switchos_specialcaseserver_port = -1;
short switchos_snapshotdataserver_port = -1;

// reflector
const char *reflector_ip_for_switchos = nullptr;
short reflector_popserver_port = -1;

// others (xindex)
size_t bg_n = 1;

// Packet types used by switchos.paramserver and ptf framework
int SWITCHOS_GET_FREEIDX = -1; // ptf get freeidx from paramserver
int SWITCHOS_GET_KEY_FREEIDX = -1; // ptf get key and freeidx from paramserver
int SWITCHOS_SET_EVICTDATA = -1; // ptf set evictidx, evictvallen, evictval, evictstat, and evictseq to paramserver
int SWITCHOS_GET_EVICTKEY = -1; // ptf get evictkey
int SWITCHOS_GET_CACHEDEMPTYINDEX = -1; // ptf get cached_empty_index

// Packet types used by switchos/controller/server for snapshot
int SNAPSHOT_START = -1;
int SNAPSHOT_SERVERSIDE = -1;
int SNAPSHOT_SERVERSIDE_ACK = -1;
int SNAPSHOT_DATA = -1;

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
	printf("workload_name: %s\n", workload_name);
	val_t::MAX_VALLEN = ini.get_max_vallen();
	COUT_VAR(val_t::MAX_VALLEN);
#ifndef ORIGINAL_XINDEX
	val_t::MAX_VALLEN = ini.get_max_vallen();
	COUT_VAR(val_t::MAX_VALLEN);
#endif

	// client
	client_num = ini.get_client_num();
	client_port_start = ini.get_client_port();
	client_ip = ini.get_client_ip();
	ini.get_client_mac(client_macaddr);
	RUN_SPLIT_DIR(client_workload_dir, workload_name, int(client_num));
	per_client_per_period_max_sending_rate = max_sending_rate / client_num / (1 * 1000 * 1000 / rate_limit_period);
	COUT_VAR(client_num);
	COUT_VAR(client_port_start);
	printf("client_ip: %s\n", client_ip);
	printf("client_macaddr: ");
	for (size_t i = 0; i < 6; i++) {
		printf("%02x", client_macaddr[i]);
		if (i != 5) printf(":");
		else printf("\n");
	}
	printf("client_workload_dir: %s\n", client_workload_dir);

	// server: loading phase
	split_n = ini.get_split_num();
	INVARIANT(split_n >= 2);
	load_n = split_n - 1;
	LOAD_SPLIT_DIR(server_load_workload_dir, workload_name, int(split_n)); // get the split directory for loading phase
	struct stat dir_stat;
	if (!(stat(server_load_workload_dir, &dir_stat) == 0 && S_ISDIR(dir_stat.st_mode))) {
		printf("Output directory does not exist: %s\n", server_load_workload_dir);
		exit(-1);
	}
	COUT_VAR(split_n);
	COUT_VAR(load_n);
	printf("server_load_workload_dir for loading phase: %s\n", server_load_workload_dir);

	// server: transaction phase
	server_num = ini.get_server_num();
	INVARIANT(server_num >= load_n);
	server_port_start = ini.get_server_port();
	server_ip = ini.get_server_ip();
	ini.get_server_mac(server_macaddr);
	backup_ip = ini.get_server_backup_ip();
	backup_port = ini.get_server_backup_port();
	notified_port = ini.get_server_notified_port();
	server_evictserver_port_start = ini.get_server_evictserver_port();
	server_consnapshotserver_port = ini.get_server_consnapshotserver_port();
	perserver_keyrange = 4ll*1024ll*1024ll*1024ll / int64_t(server_num); // 2^32 / server_num
	COUT_VAR(server_num);
	COUT_VAR(server_port_start);
	printf("server_ip: %s\n", server_ip);
	printf("server_macaddr: ");
	for (size_t i = 0; i < 6; i++) {
		printf("%02x", server_macaddr[i]);
		if (i != 5) printf(":");
		else printf("\n");
	}
	printf("backup_ip: %s\n", backup_ip);
	COUT_VAR(backup_port);
	COUT_VAR(notified_port);
	COUT_VAR(server_evictserver_port_start);
	COUT_VAR(server_consnapshotserver_port);
	COUT_VAR(perserver_keyrange);

	// controller
	controller_ip_for_server = ini.get_controller_ip_for_server();
	controller_ip_for_switchos = ini.get_controller_ip_for_switchos();
	//controller_popserver_port_start = ini.get_controller_popserver_port();
	controller_popserver_port = ini.get_controller_popserver_port();
	controller_evictserver_port = ini.get_controller_evictserver_port();
	controller_snapshot_period = ini.get_controller_snapshot_period();
	printf("controller ip for server: %s\n", controller_ip_for_server);
	printf("controller ip for switchos: %s\n", controller_ip_for_switchos);
	//COUT_VAR(controller_popserver_port_start);
	COUT_VAR(controller_popserver_port);
	COUT_VAR(controller_evictserver_port);
	COUT_VAR(controller_snapshot_period);
	
	// switch
	switch_kv_bucket_num = ini.get_switch_kv_bucket_num();
	switchos_popserver_port = ini.get_switchos_popserver_port();
	switchos_paramserver_port = ini.get_switchos_paramserver_port();
	switchos_ip = ini.get_switchos_ip();
	switchos_sample_cnt = ini.get_switchos_sample_cnt();
	switchos_snapshotserver_port = ini.get_switchos_snapshotserver_port();
	switchos_specialcaseserver_port = ini.get_switchos_specialcaseserver_port();
	switchos_snapshotdataserver_port = ini.get_switchos_snapshotdataserver_port();
	COUT_VAR(switch_kv_bucket_num);
#ifndef ORIGINAL_XINDEX
	val_t::SWITCH_MAX_VALLEN = ini.get_switch_max_vallen();
	COUT_VAR(val_t::SWITCH_MAX_VALLEN);
#endif
	COUT_VAR(switchos_popserver_port);
	COUT_VAR(switchos_paramserver_port);
	printf("switchos ip: %s\n", switchos_ip);
	COUT_VAR(switchos_sample_cnt);
	COUT_VAR(switchos_snapshotserver_port);
	COUT_VAR(switchos_specialcaseserver_port);
	COUT_VAR(switchos_snapshotdataserver_port);

	// reflector
	reflector_ip_for_switchos = ini.get_reflector_ip_for_switchos();
	reflector_popserver_port = ini.get_reflector_popserver_port();
	printf("reflector ip for switchos: %s\n", reflector_ip_for_switchos);
	COUT_VAR(reflector_popserver_port);
}

inline void parse_control_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	SWITCHOS_GET_FREEIDX = ini.get_switchos_get_freeidx();
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
	COUT_VAR(SNAPSHOT_DATA);
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
