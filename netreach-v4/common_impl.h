#ifndef COMMON_IMPL_H
#define COMMON_IMPL_H

#include "packet_format_impl.h"

/*
 * Class and alias
 */

struct alignas(CACHELINE_SIZE) TcpServerParam {
	uint8_t thread_id;
};
typedef TcpServerParam tcpserver_param_t;

typedef Key index_key_t;
typedef Val val_t;
typedef xindex::XIndex<index_key_t, val_t> xindex_t;
typedef GetRequest<index_key_t> get_request_t;
typedef PutRequest<index_key_t, val_t> put_request_t;
typedef DelRequest<index_key_t> del_request_t;
typedef ScanRequest<index_key_t> scan_request_t;
typedef GetResponse<index_key_t, val_t> get_response_t;
typedef PutResponse<index_key_t> put_response_t;
typedef DelResponse<index_key_t> del_response_t;
typedef ScanResponse<index_key_t, val_t> scan_response_t;
typedef GetRequestPOP<index_key_t> get_request_pop_t;
typedef GetRequestNLatest<index_key_t> get_request_nlatest_t;
typedef GetResponseLatestSeq<index_key_t, val_t> get_response_latest_seq_t;
typedef GetResponseDeletedSeq<index_key_t, val_t> get_response_deleted_seq_t;
typedef CachePop<index_key_t, val_t> cache_pop_t;
typedef CachePopInSwitch<index_key_t, val_t> cache_pop_inswitch_t;
typedef CachePopInSwitchAck<index_key_t, val_t> cache_pop_inswitch_ack_t;

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
val_t::MAX_VALLEN = 128;

// client
size_t client_num;
short client_port_start;
const char *client_ip;
uint8_t client_macaddr[6];
char client_workload_dir[256];
size_t per_client_per_period_max_sending_rate;

// server: loading phase
size_t split_n;
size_t load_n;
char server_load_workload_dir[256];

// server: transaction phase
size_t server_num = 1;
short server_port_start = -1;
// switch os simulator for processing special cases and packet loss handling (and tcp servers for workers)
short pktloss_port_start = -1;
const char* server_ip = nullptr;
uint8_t server_macaddr[6];
// backuper for processing in-switch snapshot
const char* backup_ip = nullptr;
short backup_port;
// notified for processing explicit notification of server-side snapshot
short notified_port;
size_t per_server_range;

// controller
const char *controller_ip = nullptr;
//short controller_popserver_port_start = -1;
short controller_popserver_port = -1;

// switch
uint32_t kv_bucket_num;
val_t::SWITCH_MAX_VALLEN = 128;
short switchos_popserver_port = -1;
short switchos_paramserver_port = -1;
const char *switchos_ip = nullptr;
uint32_t switchos_sample_cnt = 0;

// reflector
const char *reflector_ip = nullptr;
short reflector_port = -1;

// others
size_t bg_n = 1;


/*
 * Get configuration
 */

inline void parse_ini(const char * config_file);
//inline void parse_args(int, char **);

inline void parse_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	// global
	workload_name = ini.get_workload_name();
	val_t::MAX_VALLEN = ini.get_max_vallen();
	printf("workload_name: %s\n", workload_name);
	COUT_VAR(val_t::MAX_VALLEN);

	// client
	client_num = ini.get_client_num();
	client_port_start = ini.get_client_port();
	client_ip = ini.get_client_ip();
	ini.get_client_mac(client_macaddr);
	RUN_SPLIT_DIR(client_workload_dir, workload_name, client_num);
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
	LOAD_SPLIT_DIR(server_load_workload_dir, workload_name, split_n); // get the split directory for loading phase
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
	pktloss_port_start = ini.get_server_pktloss_port();
	server_ip = ini.get_server_ip();
	ini.get_server_mac(server_macaddr);
	backup_ip = ini.get_server_backup_ip();
	backup_port = ini.get_server_backup_port();
	notified_port = ini.get_server_notified_port();
	per_server_range = std::numeric_limits<size_t>::max() / server_num;
	COUT_VAR(server_num);
	COUT_VAR(server_port_start);
	COUT_VAR(pktloss_port_start);
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
	COUT_VAR(per_server_range);

	// controller
	controller_ip = ini.get_controller_ip();
	//controller_popserver_port_start = ini.get_controller_popserver_port();
	controller_popserver_port = ini.get_controller_popserver_port();
	printf("controller ip: %s\n", controller_ip);
	//COUT_VAR(controller_popserver_port_start);
	COUT_VAR(controller_popserver_port);
	
	// switch
	kv_bucket_num = ini.get_bucket_num();
	val_t::SWITCH_MAX_VALLEN = ini.get_switch_max_vallen();
	switchos_popserver_port = ini.get_switchos_popserver_port();
	switchos_paramserver_port = ini.get_switchos_paramserver_port();
	switchos_ip = ini.get_switchos_ip();
	switchos_sample_cnt = ini.get_switchos_sample_cnt();
	COUT_VAR(kv_bucket_num);
	COUT_VAR(val_t::SWITCH_MAX_VALLEN);
	COUR_VAR(switchos_popserver_port);
	COUT_VAR(switchos_paramserver_port);
	printf("switchos ip: %s\n", switchos_ip);
	COUT_VAR(switchos_sample_cnt);

	// reflector
	reflector_ip = ini.get_reflector_ip();
	reflector_port = ini.get_reflector_port();
	printf("reflector ip: %s\n", reflector_ip);
	COUT_VAR(reflector_port);
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
