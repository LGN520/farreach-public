#ifndef COMMON_IMPL_H
#define COMMON_IMPL_H

#include "packet_format_impl.h"

/*
 * Class and alias
 */

struct alignas(CACHELINE_SIZE) TcpServerParam {
	uint8_t thread_id;
};
typedef TcpServerParam tcpserver_param_t

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

/*
 * Parameters
 */

// loading phase
size_t split_n;
size_t load_n;
char output_dir[256];

// normal worker
size_t fg_n = 1;
short dst_port_start = -1;
const char *workload_name = nullptr;
uint32_t kv_bucket_num;
size_t per_server_range;
size_t bg_n = 1;

// backuper for processing in-switch snapshot
short backup_port;
// notified for processing explicit notification of server-side snapshot
short notified_port;

// switch os simulator for processing special cases and packet loss handling (and tcp servers for workers)
short pktloss_port_start = -1;

// cache update (population and eviction)
const char *controller_ip = nullptr;
short controller_popserver_port_start = -1;

/*
 * Get configuration
 */

inline void parse_ini(const char * config_file);
inline void parse_args(int, char **);

inline void parse_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	split_n = ini.get_split_num();
	INVARIANT(split_n >= 2);
	load_n = split_n - 1;

	fg_n = ini.get_server_num();
	INVARIANT(fg_n >= load_n);
	dst_port_start = ini.get_server_port();
	workload_name = ini.get_workload_name();
	kv_bucket_num = ini.get_bucket_num();
	val_t::MAX_VALLEN = ini.get_max_vallen();
	val_t::SWITCH_MAX_VALLEN = ini.get_switch_max_vallen();
	backup_port = ini.get_server_backup_port();
	pktloss_port_start = ini.get_server_pktloss_port();
	per_server_range = std::numeric_limits<size_t>::max() / fg_n;
	notified_port = ini.get_server_notified_port();

	LOAD_SPLIT_DIR(output_dir, workload_name, split_n); // get the split directory for loading phase
	struct stat dir_stat;
	if (!(stat(output_dir, &dir_stat) == 0 && S_ISDIR(dir_stat.st_mode))) {
		printf("Output directory does not exist: %s\n", output_dir);
		exit(-1);
	}

	controller_ip = ini.get_controller_ip();
	controller_popserver_port_start = ini.get_controller_popserver_port();

	COUT_VAR(split_n);
	COUT_VAR(load_n);
	COUT_VAR(fg_n);
	COUT_VAR(dst_port_start);
	printf("workload_name: %s\n", workload_name);
	COUT_VAR(kv_bucket_num);
	COUT_VAR(val_t::MAX_VALLEN);
	COUT_VAR(val_t::SWITCH_MAX_VALLEN);
	COUT_VAR(backup_port);
	COUT_VAR(pktloss_port_start);
	COUT_VAR(per_server_range);
	COUT_VAR(notified_port);
	COUT_VAR(controller_ip);
	COUT_VAR(controller_popserver_port_start);
}

inline void parse_args(int argc, char **argv) {
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
}

#endif
