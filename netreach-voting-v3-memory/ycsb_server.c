#include <getopt.h>
#include <stdlib.h>
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

#include "helper.h"
#include "packet_format_impl.h"
#include "dpdk_helper.h"
#include "key.h"
#include "iniparser/iniparser_wrapper.h"
#include "ycsb/parser.h"
#include "backup_data.h"
#include "special_case.h"
#include "deleted_set_impl.h"

#ifdef ORIGINAL_XINDEX
#include "original_xindex/xindex.h"
#include "original_xindex/xindex_impl.h"
#else
#include "val.h"
#ifdef DYNAMIC_MEMORY
#ifdef SEQ_MECHANISM
#include "extended_xindex_dynamic_seq/xindex.h"
#include "extended_xindex_dynamic_seq/xindex_impl.h"
#else
#include "extended_xindex_dynamic/xindex.h"
#include "extended_xindex_dynamic/xindex_impl.h"
#endif
#else
#include "extended_xindex/xindex.h"
#include "extended_xindex/xindex_impl.h"
#endif

#define MQ_SIZE 256
#define MAX_VERSION 0xFFFFFFFFFFFFFFFF

struct alignas(CACHELINE_SIZE) LoadSFGParam;
struct alignas(CACHELINE_SIZE) SFGParam;
//struct alignas((CACHELINE_SIZE)) ReceiverParam;
struct alignas((CACHELINE_SIZE)) TcpServerParam;

typedef BackupData backup_data_t;
typedef SpecialCase special_case_t;
typedef DeletedSet deleted_set_t;
typedef Key index_key_t;
typedef Val val_t;
typedef LoadSFGParam load_sfg_param_t;
typedef SFGParam sfg_param_t;
//typedef ReceiverParam receiver_param_t;
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
typedef GetResponsePOP<index_key_t, val_t> get_response_pop_t;
typedef GetResponseNPOP<index_key_t, val_t> get_response_npop_t;
typedef GetResponsePOPLarge<index_key_t, val_t> get_response_pop_lareg_t;
typedef GetResponsePOPEvict<index_key_t, val_t> get_response_pop_evict_t;
typedef PutRequestSeq<index_key_t, val_t> put_request_seq_t;
typedef PutRequestPOPEvict<index_key_t, val_t> put_request_pop_evict_t;
typedef PutRequestLarge<index_key_t, val_t> put_request_large_t;
typedef PutRequestLargeSeq<index_key_t, val_t> put_request_large_seq_t;
typedef PutRequestLargeEvict<index_key_t, val_t> put_rquest_large_evict_t;
typedef PutRequestCase1<index_key_t, val_t> put_request_case1_t;
typedef DelRequestCase1<index_key_t, val_t> del_request_case1_t;
typedef GetResponsePOPEvictCase2<index_key_t, val_t> get_response_pop_evict_case2_t;
typedef PutRequestLargeEvictCase2<index_key_t, val_t> put_request_large_evict_case2_t;
typedef PutRequestPOPEvictCase2<index_key_t, val_t> put_request_pop_evict_case2_t;
typedef PutRequestCase3<index_key_t, val_t> put_request_case3_t;
typedef DelRequestCase3<index_key_t> del_request_case3_t;
typedef PutRequestLargeCase3<index_key_t, val_t> put_request_large_case3_t;
typedef PutResponseCase3<index_key_t> put_response_case3_t;
typedef DelResponseCase3<index_key_t> del_response_case3_t;
typedef GetResponsePOPEvictSwitch<index_key_t, val_t> get_response_pop_evict_switch_t;
typedef PutRequestPOPEvictSwitch<index_key_t, val_t> put_request_pop_evict_switch_t;
typedef PutRequestLargeEvictSwitch<index_key_t, val_t> put_request_large_evict_switch_t;
typedef GetResponsePOPEvictCase2Switch<index_key_t, val_t> get_response_pop_evict_case2_switch_t;
typedef PutRequestPOPEvictCase2Switch<index_key_t, val_t> put_request_pop_evict_case2_switch_t;
typedef PutRequestLargeEvictCase2Switch<index_key_t, val_t> put_request_large_evict_case2_switch_t;

// Get configuration
inline void parse_ini(const char * config_file);
inline void parse_args(int, char **);

// Prepare data structures
void preprae_for_pktloss();
void prepare_dpdk();
void prepare_receiver();

// loading phase
size_t split_n;
size_t load_n;
char output_dir[256];
volatile bool load_running = false;
std::atomic<size_t> load_ready_threads(0);
void run_load_server(xindex_t *table); // loading phase
void *run_load_sfg(void *param); // workers in loading phase
void load(std::vector<index_key_t> &keys, std::vector<val_t> &vals);
struct alignas(CACHELINE_SIZE) LoadSFGParam {
	xindex_t *table;
	uint8_t thread_id;
};

// Transaction phase
void run_server(xindex_t *table); // running phase

// (1) Receiver for receiving pkts
static int run_receiver(__attribute__((unused)) void *param); // receiver
//static int run_receiver(void *param); // receiver
struct rte_mempool *mbuf_pool = NULL;
//volatile struct rte_mbuf **pkts;
//volatile bool *stats;
struct rte_mbuf*** volatile pkts_list; // pkts from receiver to each server
uint32_t* volatile heads;
uint32_t* volatile tails;
struct rte_mbuf** volatile pkts_for_simulator; // pkts from receiver to switch os simulator
volatile uint32_t head_for_simulator;
volatile uint32_t tail_for_simulator;
struct rte_mbuf*** volatile pkts_list_for_pktloss; // pkts from switch os to each server 
uint32_t* volatile heads_for_pktloss;
uint32_t* volatile tails_for_pktloss;
/*struct alignas(CACHELINE_SIZE) ReceiverParam {
	size_t overall_thpt;
};*/

// (2) Backuper for processing in-switch snapshot
short backup_port;
void *run_backuper(void *param); // backuper
void *run_kvparser(void *param); // KV parser 
void parse_kv(const char* data_buf, unsigned int data_size, unsigned int expected_count, backup_data_t *new_backup_data); // Helper to process backup data
void rollback(backup_data_t *new_backup_data);
backup_data_t * volatile backup_data = nullptr;
uint32_t* volatile backup_rcu;
//std::map<unsigned short, special_case_t> **special_cases_list = nullptr; // per-thread special cases for each worker
std::map<unsigned short, special_case_t> *special_cases = nullptr; // one-thread special cases for switch os simulator
bool volatile isbackup = false; // TODO: it should be atomic
std::atomic_flag is_kvsnapshot = ATOMIC_FLAG_INIT;
void try_kvsnapshot(xindex_t *table);

// (2-1) Notified for processing explicit notification of server-side snapshot
short notified_port;
void *run_notified(void *param); // the notified

// (3) Switch os simulator for processing special cases and packet loss handling (and tcp servers for workers)
deleted_set_t *deleted_sets = NULL;
//static int run_switchos_simulator(void *param);
void *run_switchos_simulator(void *param);
short pktloss_port_start = 2222;
int * volatile server_tcpsocks = NULL; // tcp socket in server to receive mirrored pkt from switch os
int * volatile switchos_tcpsocks = NULL; // tcp socket in switch os to send mirrored pkt to server
std::atomic<size_t> ready_tcpserver_threads(0);
void *run_tcpserver_for_pktloss(void *param);
struct alignas(CACHELINE_SIZE) TcpServerParam {
	uint8_t thread_id;
};

// (4) Worker for processing pkts
static int run_sfg(void *param); // workers in runing phase
//void *run_sfg(void *param);
void kill(int signum);
size_t fg_n = 1;
size_t bg_n = 1;
short dst_port_start = 1111;
const char *workload_name = nullptr;
uint32_t kv_bucket_num;
struct alignas(CACHELINE_SIZE) SFGParam {
  xindex_t *table;
  uint8_t thread_id;
  size_t throughput;
#ifdef TEST_AGG_THPT
  double sum_latency;
#endif
};


size_t per_server_range;
size_t get_server_idx(index_key_t key) {
	size_t server_idx = key.keylo / per_server_range;
	if (server_idx >= fg_n) {
		server_idx = fg_n - 1;
	}
	return server_idx;
}

bool killed = false;
volatile bool running = false;
std::atomic<size_t> ready_threads(0);


int main(int argc, char **argv) {
  parse_ini("config.ini");
  parse_args(argc, argv);
  //xindex::init_options(); // init options of rocksdb

  // Prepare per-server special cases
  /*special_cases_list = new std::map<unsigned short, special_case_t> *[fg_n];
  for (size_t i = 0; i < fg_n; i++) {
	special_cases_list[i] = new std::map<unsigned short, special_case_t>;
  }*/
  // Prepare one-thread special cases
  special_cases = new std::map<unsigned short, special_case_t>; 

  // Prepare switch os simulator for packet loss
  prepare_for_pktloss();

  // prepare xindex
  std::vector<index_key_t> keys;
  std::vector<val_t> vals;
  load(keys, vals); // Use the last split workload to initialize the XIndex
  xindex_t *tab_xi = new xindex_t(keys, vals, fg_n, bg_n); // fg_n to create array of RCU status; bg_n background threads have been launched

  run_load_server(tab_xi); // loading phase

  // Prepare DPDK EAL param
  prepare_dpdk();

  // Prepare pkts and stats for receiver (based on ring buffer)
  prepare_receiver();

  // RCU for backup data (NOTE: necessary only if SCAN needs to access backup data)
  backup_rcu = new uint32_t[fg_n];
  memset((void *)backup_rcu, 0, fg_n * sizeof(uint32_t));

  // register signal handler
  signal(SIGTERM, SIG_IGN); // Ignore SIGTERM for subthreads

  run_server(tab_xi); // running phase
  if (tab_xi != nullptr) delete tab_xi; // terminate_bg -> bg_master joins bg_threads

  // Free DPDK mbufs
  for (size_t i = 0; i < fg_n; i++) {
	//rte_pktmbuf_free((struct rte_mbuf *)pkts[i]);
	  while (heads[i] != tails[i]) {
		  rte_pktmbuf_free((struct rte_mbuf*)pkts_list[i][tails[i]]);
		  tails[i] += 1;
	  }
  }
  dpdk_free();

  COUT_THIS("[server] Exit successfully")
  exit(0);
}

/*
 * Get configuration
 */

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

/*
 * Prepare data structures 
 */

void prepare_for_pktloss() {
	// From switch os to each server
	pkts_list_for_pktloss = new struct rte_mbuf**[fg_n];
	heads_for_pktloss = new uint32_t[fg_n];
	tails_for_pktloss = new uint32_t[fg_n];
	memset((void*)heads_for_pktloss, 0, sizeof(uint32_t)*fg_n);
	memset((void*)tails_for_pktloss, 0, sizeof(uint32_t)*fg_n);
	for (size_t i = 0; i < fg_n; i++) {
		pkts_list_for_pktloss[i] = new struct rte_mbuf*[MQ_SIZE];
		for (size_t j = 0; j < MQ_SIZE; j++) {
			pkts_list_for_pktloss[i][j] = nullptr;
		}
	}

	// For linearizability under packet loss
	for (size_t i = 0; i < fg_n; i++) { // per-server deleted set
		deleted_sets[i] = new deleted_set_t();
	}

	// Tcp channels between switch os and servers
	server_tcpsocks = new int[fg_n];
	switchos_tcpsocks = new int[fg_n];
	for (size_t i = 0; i < fg_n; i++) {
		// Set server sockets
		server_tcpsocks[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (server_tcpsocks[i] == -1) {
			printf("Fail to create tcp socket of server %ld: errno: %d!\n", i, errno);
			exit(-1);
		}
		// reuse the occupied port for the last created socket instead of being crashed
		const int trueFlag = 1;
		if (setsockopt(server_tcpsocks[i], SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
			printf("Fail to setsockopt of of server %ld: errno: %d!\n", i, errno);
			exit(-1);
		}
		sockaddr_in listen_addr;
		memset(&listen_addr, 0, sizeof(listen_addr));
		listen_addr.sin_family = AF_INET;
		listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		listen_addr.sin_port = htons(pktloss_start_port+i); // start from 2222
		if ((bind(server_tcpsocks[i], (struct sockaddr*)&listen_addr, sizeof(listen_addr))) != 0) {
			printf("Fail to bind socket on 0.0.0.0:%hu of server %ld, errno: %d!\n", pktloss_start_port+i, i, errno);
			exit(-1);
		}
		if ((listen(server_tcpsocks[i], 1)) != 0) { // MAX_PENDING_CONNECTINO = 1
			printf("Fail to listen on 0.0.0.0:%hu, errno: %d!\n", listen_port, errno);
			exit(-1);
		}
		// Leave accept in each worker_tcpserver

		// Set switch os sockets
		switchos_tcpsocks[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (switchos_tcpsocks[i] == -1) {
			printf("Fail to create tcp socket in switch os for server %ld: errno: %d!\n", i, errno);
			exit(-1);
		}
		// Leave connect in switchos_simulator
	}
}

void prepare_dpdk() {
  int dpdk_argc = 5;
  char **dpdk_argv;
  dpdk_argv = new char *[dpdk_argc];
  for (int i = 0; i < dpdk_argc; i++) {
	dpdk_argv[i] = new char[20];
	memset(dpdk_argv[i], '\0', 20);
  }
  std::string arg_proc = "./client";
  std::string arg_iovamode = "--iova-mode";
  std::string arg_iovamode_val = "pa";
  std::string arg_file_prefix = "--file-prefix";
  std::string arg_file_prefix_val = "netbuffer";
  //std::string arg_whitelist = "-w";
  //std::string arg_whitelist_val = "0000:5e:00.1";
  memcpy(dpdk_argv[0], arg_proc.c_str(), arg_proc.size());
  memcpy(dpdk_argv[1], arg_iovamode.c_str(), arg_iovamode.size());
  memcpy(dpdk_argv[2], arg_iovamode_val.c_str(), arg_iovamode_val.size());
  memcpy(dpdk_argv[3], arg_file_prefix.c_str(), arg_file_prefix.size());
  memcpy(dpdk_argv[4], arg_file_prefix_val.c_str(), arg_file_prefix_val.size());
  //memcpy(dpdk_argv[3], arg_whitelist.c_str(), arg_whitelist.size());
  //memcpy(dpdk_argv[4], arg_whitelist_val.c_str(), arg_whitelist_val.size());
  rte_eal_init_helper(&dpdk_argc, &dpdk_argv); // Init DPDK
  dpdk_init(&mbuf_pool, fg_n, 1);
}

void prepare_receiver() {
  // From receiver to each server
  //pkts = new volatile struct rte_mbuf*[fg_n];
  //stats = new volatile bool[fg_n];
  //memset((void *)pkts, 0, sizeof(struct rte_mbuf *)*fg_n);
  //memset((void *)stats, 0, sizeof(bool)*fg_n);
  //for (size_t i = 0; i < fg_n; i++) {
  //  pkts[i] = rte_pktmbuf_alloc(mbuf_pool);
  //}
  pkts_list = new struct rte_mbuf**[fg_n];
  heads = new uint32_t[fg_n];
  tails = new uint32_t[fg_n];
  memset((void*)heads, 0, sizeof(uint32_t)*fg_n);
  memset((void*)tails, 0, sizeof(uint32_t)*fg_n);
  //int res = 0;
  for (size_t i = 0; i < fg_n; i++) {
	  pkts_list[i] = new struct rte_mbuf*[MQ_SIZE];
	  for (size_t j = 0; j < MQ_SIZE; j++) {
		  pkts_list[i][j] = nullptr;
	  }
	  //res = rte_pktmbuf_alloc_bulk(mbuf_pool, pkts_list[i], MQ_SIZE);
  }

  // From receiver to switch os
  pkts_for_simulator = new struct rte_mbuf*[MQ_SIZE];
  for (size_t j = 0; j < MQ_SIZE; j++) {
	  pkts_for_simulator[i][j] = nullptr;
  }
  head_for_simulator = 0;
  tail_for_simulator = 0;
}

/*
 * Loading phase
 */

void load(std::vector<index_key_t> &keys, std::vector<val_t> &vals) {
	char load_filename[256];
	memset(load_filename, '\0', 256);
	GET_SPLIT_WORKLOAD(load_filename, output_dir, split_n-1);

	Parser parser(load_filename);
	ParserIterator iter = parser.begin();
	index_key_t tmpkey;
	val_t tmpval;

	std::map<index_key_t, val_t> loadmap;
	while (true) {
		tmpkey = iter.key();
		tmpval = iter.val();	
		if (iter.type() == uint8_t(packet_type_t::PUT_REQ)) {	// INESRT
			loadmap.insert(std::pair<index_key_t, val_t>(tmpkey, tmpval));
		}
		else {
			COUT_N_EXIT("Invalid type: !" << int(iter.type()));
		}
		if (!iter.next()) {
			break;
		}
	}
	iter.close();

	keys.resize(loadmap.size());
	vals.resize(loadmap.size());
	size_t i = 0;
	for (std::map<index_key_t, val_t>::iterator iter = loadmap.begin(); iter != loadmap.end(); iter++) {
		keys[i] = iter->first;
		vals[i] = iter->second;
		i++;
	}

	// Check whether keys are sorted (xindex requires sorted data for initialization) -> sorted
	/*for (size_t check_i = 0; check_i < (keys.size()-1); check_i++) {
		if (keys[check_i] >= keys[check_i+1]) {
			COUT_N_EXIT("keys["<<check_i<<"] "<<keys[check_i].to_string()<<" >= keys["<<check_i+1<<"] "<<keys[check_i+1].to_string());
		}
	}
	COUT_N_EXIT("Keys are sorted!");*/
}

void run_load_server(xindex_t *table) {
	int ret = 0;
	unsigned lcoreid = 1;

	load_running = false;

	// Prepare load_n params
	pthread_t threads[load_n];
	load_sfg_param_t load_sfg_params[load_n];
	// check if parameters are cacheline aligned
	for (size_t i = 0; i < load_n; i++) {
		if ((uint64_t)(&(load_sfg_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(load_sfg_params[i]));
		}
	}

	// Launch workers
	for (size_t worker_i = 0; worker_i < load_n; worker_i++) {
		load_sfg_params[worker_i].table = table;
		load_sfg_params[worker_i].thread_id = static_cast<uint8_t>(worker_i);
		int ret = pthread_create(&threads[worker_i], nullptr, run_load_sfg, (void *)&load_sfg_params[worker_i]);
		if (ret) {
			COUT_N_EXIT("Error:" << ret);
		}
		lcoreid++;
		if (lcoreid >= MAX_LCORE_NUM) {
			lcoreid = 1;
		}
	}

	COUT_THIS("[local client] prepare server foreground threads...")
	while (load_ready_threads < load_n) sleep(1);

	load_running = true;
	COUT_THIS("[local client] start running...")

	void *status;
	for (size_t i = 0; i < fg_n; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error:unable to join," << rc);
		}
	}
	COUT_THIS("[local client] finish loading phase...")
}

void *run_load_sfg(void * param) {
	// Parse param
	load_sfg_param_t &thread_param = *(load_sfg_param_t *)param;
	uint8_t thread_id = thread_param.thread_id;
	xindex_t *table = thread_param.table;

	char load_filename[256];
	memset(load_filename, '\0', 256);
	GET_SPLIT_WORKLOAD(load_filename, output_dir, thread_id);

	Parser parser(load_filename);
	ParserIterator iter = parser.begin();
	index_key_t tmpkey;
	val_t tmpval;

	int res = 0;
	COUT_THIS("[local client " << uint32_t(thread_id) << "] Ready.");

	ready_threads++;

#if !defined(NDEBUGGING_LOG)
  std::string logname;
  GET_STRING(logname, "tmp_localtest"<< uint32_t(thread_id)<<".out");
  std::ofstream ofs(logname, std::ofstream::out);
#endif

	while (!load_running) {
	}

	while (load_running) {
		tmpkey = iter.key();
		tmpval = iter.val();	
		if (iter.type() == uint8_t(packet_type_t::PUT_REQ)) {	// INESRT
			bool tmp_stat = table->put(tmpkey, tmpval, thread_id); // Use put instead of data_put as data structure (i.e., vector) cannot handle dyanmic insert
			if (!tmp_stat) {
				COUT_N_EXIT("Loading phase: fail to put <" << tmpkey.to_string() << ", " << tmpval.to_string() << ">");
			}
		}
		else {
			COUT_N_EXIT("Invalid type: !" << int(iter.type()));
		}
		if (!iter.next()) {
			break;
		}
	}

	pthread_exit(nullptr);
	iter.close();
#if !defined(NDEBUGGING_LOG)
	ofs.close();
#endif
	return 0;
}


/*
 * Transaction phase 
 */

void run_server(xindex_t *table) {
	int ret = 0;
	unsigned lcoreid = 1;

	running = false;

	// TODO: add ready count for each thread

	// Launch receiver (receiving normal packets)
	//receiver_param_t receiver_param;
	//receiver_param.overall_thpt = 0;
	//ret = rte_eal_remote_launch(run_receiver, (void*)&receiver_param, lcoreid);
	ret = rte_eal_remote_launch(run_receiver, NULL, lcoreid);
	if (ret) {
		COUT_N_EXIT("Error:" << ret);
	}
	COUT_THIS("[server] Launch receiver with ret code " << ret)
	lcoreid++;

	// Launch backuper (processing in-switch snapshot from switch OS)
	pthread_t backuper_thread;
	ret = pthread_create(&backuper_thread, nullptr, run_backuper, (void *)table);
	if (ret) {
		COUT_N_EXIT("Error: unable to create backuper " << ret);
	}

	// Launch notified (processing explicit notification for server-side snapshot)
	pthread_t notified_thread;
	ret = pthread_create(&notified_thread, nullptr, run_notified, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: unable to create notified " << ret);
	}

	// Launch a tcp server for each worker (i.e., server) to receive mirrored packets
	tcpserver_param_t tcpserver_params[fg_n];
	for (size_t i = 0; i < fg_n; i++) {
		if ((uint64_t)(&(tcpserver_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(tcpserver_params[i]));
		}
	}
	pthread_t tcpserver_threads[fg_n];
	for (size_t i = 0; i < fg_n; i++) {
		tcpserver_params[i].thread_id = uint8_t(i);
		ret = pthread_create(&tcpserver_threads[i], nullptr, run_tcpserver_for_pktloss, (void *)&tcpserver_params[i]);
		if (ret) {
			COUT_N_EXIT("Error: unable to create tcp server for worker " << i);
		}
	}

	// Wait for tcpservers, as switch os simulator needs to connect each tcpserver to build reliable channel for mirroring
	while (ready_tcpserver_threads < fg_n) sleep(1);
	sleep(1); // Ensure that all tcpservers enter accept status

	// Launch switch OS simulator (processing rollback messages (case1 and case2) and mirrored evicted data)
	pthread_t switchos_simulator_thread;
	ret = pthread_create(&switchos_simulator_thread, nullptr, run_switchos_simulator, (void *)nullptr);
	if (ret) {
		COUT_N_EXIT("Error: unable to create switch os simulator " << ret);
	}

	// Launch workers (processing normal packets)
	//pthread_t threads[fg_n];
	sfg_param_t sfg_params[fg_n];
	// check if parameters are cacheline aligned
	for (size_t i = 0; i < fg_n; i++) {
		if ((uint64_t)(&(sfg_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(sfg_params[i]));
		}
	}
	for (uint8_t worker_i = 0; worker_i < fg_n; worker_i++) {
		sfg_params[worker_i].table = table;
		sfg_params[worker_i].thread_id = worker_i;
		sfg_params[worker_i].throughput = 0;
#ifdef TEST_AGG_THPT
		sfg_params[worker_i].sum_latency = 0.0;
#endif
		//int ret = pthread_create(&threads[worker_i], nullptr, run_sfg, (void *)&sfg_params[worker_i]);
		ret = rte_eal_remote_launch(run_sfg, (void *)&sfg_params[worker_i], lcoreid);
		if (ret) {
		  COUT_N_EXIT("Error:" << ret);
		}
		lcoreid++;
		if (lcoreid >= MAX_LCORE_NUM) {
			lcoreid = 1;
		}
	}

	COUT_THIS("[server] prepare server foreground threads...")
	while (ready_threads < fg_n) sleep(1);

	signal(SIGTERM, kill); // Set for main thread (kill -15)

	running = true;
	COUT_THIS("[server] start running...")

	while (!killed) {
		sleep(1);
	}

	/* Processing Statistics */
	//COUT_THIS("Server-side aggregate throughput: " << receiver_param.overall_thpt);
	size_t overall_thpt = 0;
	std::vector<double> load_balance_ratio_list;
	for (size_t i = 0; i < fg_n; i++) {
		overall_thpt += sfg_params[i].throughput;
	}
	COUT_THIS("Server-side overall throughput: " << overall_thpt);
	double avg_per_server_thpt = double(overall_thpt) / double(fg_n);
	for (size_t i = 0; i < fg_n; i++) {
		load_balance_ratio_list.push_back(double(sfg_params[i].throughput) / avg_per_server_thpt);
	}
	for (size_t i = 0; i < load_balance_ratio_list.size(); i++) {
		COUT_THIS("Load balance ratio of server " << i << ": " << load_balance_ratio_list[i]);
	}
#ifdef TEST_AGG_THPT
	double max_agg_thpt = 0.0;
	for (size_t i = 0; i < fg_n; i++) {
		max_agg_thpt += (double(sfg_params[i].throughput) / sfg_params[i].sum_latency * 1000 * 1000);
	}
	max_agg_thpt /= double(1024 * 1024);
	COUT_THIS("Max server-side aggregate throughput: " << max_agg_thpt << " MQPS");
#endif

	running = false;
	/*void *status;
	for (size_t i = 0; i < fg_n; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error:unable to join," << rc);
		}
	}*/
	void * status;
	int rc = pthread_join(backuper_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join backuper: " << rc);
	}
	rc = pthread_join(switchos_simulator_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join simulator: " << rc);
	}
	for (size_t i = 0; i < fg_n; i++) {
		rc = pthread_join(tcpserver_threads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join tcpserver " << i << ": " << rc);
		}
	}
	rte_eal_mp_wait_lcore();
}


/*
 * Receiver for normal packets
 */

static int run_receiver(void *param) {
	//receiver_param_t &receiver_param = *((receiver_param_t *)param);
	while (!running)
		;

	struct rte_mbuf *received_pkts[32];
	index_key_t startkey, endkey;
	uint32_t num;
	while (running) {
		uint16_t n_rx;
		n_rx = rte_eth_rx_burst(0, 0, received_pkts, 32);
		//struct rte_eth_stats ethstats;
		//rte_eth_stats_get(0, &ethstats);

		if (n_rx == 0) continue;
		for (size_t i = 0; i < n_rx; i++) {
			int received_port = get_dstport(received_pkts[i]);
			if (received_port == -1) {
				continue;
			}
			else {
				bool isscan = get_scan_keys(received_pkts[i], &startkey, &endkey, &num);
				if (isscan) {
					//receiver_param.overall_thpt++;
					size_t first_server_idx = get_server_idx(startkey);
					size_t last_server_idx = get_server_idx(endkey);
					size_t split_num = last_server_idx - first_server_idx + 1;
					struct rte_mbuf *sub_scan_reqs[split_num];
					sub_scan_reqs[0] = received_pkts[i];
					if (split_num > 1) {
						int res = rte_pktmbuf_alloc_bulk(mbuf_pool, sub_scan_reqs + 1, split_num - 1);
						INVARIANT(res == 0);
						for (size_t mbufidx = 1; mbufidx < split_num; mbufidx ++) {
							memcpy(sub_scan_reqs[mbufidx], received_pkts[i], sizeof(struct rte_mbuf));
						}
					}

					index_key_t tmpkey = startkey;
					uint64_t avg_range = (endkey.keylo - startkey.keylo) / split_num;
					//uint32_t avg_num = num / split_num;
					uint32_t avg_num = num;
					tmpkey.keylo += avg_range;
					for (size_t idx = first_server_idx; idx <= last_server_idx; idx++) {
						struct rte_mbuf *cur_mbuf = sub_scan_reqs[idx - first_server_idx];
						if (idx == last_server_idx) {
							tmpkey = endkey;
							//avg_num = num - avg_num * (split_num - 1);
						}
						set_scan_keys(cur_mbuf, &startkey, &tmpkey, &avg_num);
						startkey = tmpkey;
						tmpkey.keylo += avg_range;

						if (((heads[idx] + 1) % MQ_SIZE) != tails[idx]) {
							pkts_list[idx][heads[idx]] = cur_mbuf; // cur_mbuf will be freed in run_server
							heads[idx] = (heads[idx] + 1) % MQ_SIZE;
						}
						else {
							COUT_THIS("Drop pkt since pkts_list["<<idx<<"] is full!")
							rte_pktmbuf_free(cur_mbuf);
						}
					}
				}
				else {
					packet_type_t optype = packet_type_t(get_optype(received_pkts[i]));
					if (optype == packet_type_t::PUT_REQ_CASE1 || optype == packet_type_t::DEL_REQ_CASE1 || \
							optype == packet_type_t::PUT_REQ_POP_EVICT_SWITCH || optype == packet_type_t::PUT_REQ_POP_EVICT_CASE2_SWITCH || \
							optype == packet_type_t::PUT_REQ_LARGE_EVICT_SWITCH || optype == packet_type_t::PUT_REQ_LARGE_EVICT_CASE2_SWITCH || \
							optype == packet_type_t::GET_RES_POP_EVICT_SWITCH || optype == packet_type_t::GET_RES_POP_EVICT_CASE2_SWITCH) { // Forward to switch os simulator to simulator the process of copy to cpu
						if (((head_for_simulator+1) % MQ_SIZE) != tail_for_simulator) {
							pkts_for_simulator[head_for_simulator] = received_pkts[i];
							head_for_simulator = (head_for_simulator + 1) % MQ_SIZE;
						}
						else {
							COUT_THIS("Drop pkt for switch os simulator since pkts_for_simluator is full!")
							rte_pktmbuf_free(received_pkts[i]);
						}
					}
					else { // normal packets
						int idx = received_port - dst_port_start;
						if (idx < 0 || unsigned(idx) >= fg_n) {
							COUT_THIS("Invalid dst port received by server: " << received_port)
							continue;
						}
						else {
							/*if (stats[idx]) {
								COUT_THIS("Invalid stas[" << idx << "] which is true!")
								continue;
							}
							else {
								pkts[idx] = received_pkts[i];
								stats[idx] = true;
							}*/
							if (((heads[idx] + 1) % MQ_SIZE) != tails[idx]) {
								//receiver_param.overall_thpt++;
								pkts_list[idx][heads[idx]] = received_pkts[i];
								heads[idx] = (heads[idx] + 1) % MQ_SIZE;
							}
							else {
								COUT_THIS("Drop pkt since pkts_list["<<idx<<"] is full!")
								rte_pktmbuf_free(received_pkts[i]);
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

/*
 * Backuper for in-switch snapshot 
 */

void *run_backuper(void *param) {
	xindex_t *table = (xindex_t*)param;

	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	INVARIANT(sock_fd >= 0);
	// Disable udp/tcp check
	int disable = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		COUT_N_EXIT("Disable tcp checksum failed");
	}
	// Set timeout for recvfrom/accept of udp/tcp
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec =  0;
	int res = setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	INVARIANT(res >= 0);
	// Set listen addr
	struct sockaddr_in backup_server_addr;
	memset(&backup_server_addr, 0, sizeof(sockaddr_in));
	backup_server_addr.sin_family = AF_INET;
	backup_server_addr.sin_port = htons(backup_port);
	backup_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	res = bind(sock_fd, (struct sockaddr *)&backup_server_addr, sizeof(backup_server_addr));
	INVARIANT(res >= 0);
	// Mark tcp socket as passive to process SYN packets
	res = listen(sock_fd, 5); // 5 is the max number of pending connections
	INVARIANT(res >= 0);

	while (!running)
		;

	bool iscreate = false;
	pthread_t latest_thread;
	while (running) {
		struct sockaddr_in controller_addr;
		unsigned int controller_addr_len;
		int connfd = accept(sock_fd, (struct sockaddr *)&controller_addr, &controller_addr_len);
		if (connfd == -1) {
			if (errno == EWOULDBLOCK || errno == EINTR) {
				continue; // timeout or interrupted system call
			}
			else {
				COUT_N_EXIT("[server] Error of accept: errno = " << std::string(strerror(errno)));
			}
		}

		if (!isbackup) {
			isbackup = true; // Ensure that all other worker threads do not touch special_cases_list
			// TODO: test periodic backup with background traffic
			/*for (uint32_t i = 0; i < fg_n; i++) {
				uint32_t prev_val = backup_rcu[i];
				while (running && backup_rcu[i] == prev_val) {}
			}*/

			try_kvsnapshot(table);

			// TODO: we do not need multi-threading here?
			int ret = pthread_create(&latest_thread, nullptr, run_kvparser, (void *)&connfd); // It finally sets backup as false
			iscreate = true;
			if (ret) {
				COUT_N_EXIT("Error: unable to create kv parser " << ret);
			}
		}
		else {
			close(connfd); // close(connfd) -> bad file descriptor in sub-thread if without "else" block?
		}
	}

	if (iscreate) {
		void * status;
		int rc = pthread_join(latest_thread, &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join " << rc);
		}
	}

	close(sock_fd);
	pthread_exit(nullptr);
}

void *run_kvparser(void *param) {
	uint32_t max_recv_buf_size = kv_bucket_num * (34 + val_t::max_bytesnum()); // n * (16+16+1+Val::max_bytesnum) + 8 < n * (34 + Val::max_bytesnum)
	char recv_buf[max_recv_buf_size];
	memset(recv_buf, 0, sizeof(recv_buf));

	int connfd = *((int *)param);

	// Metadata
	bool isfirst = true;
	unsigned int count = 0;
	unsigned int expected_size = 0;
	// Data
	uint32_t start_idx = 0;
	while (true) {
		int recv_size = recv(connfd, recv_buf + start_idx, max_recv_buf_size - start_idx, 0);
		if (recv_size < 0) {
			COUT_N_EXIT("[server] kvparser recv fails: " << recv_size << " errno = " << std::string(strerror(errno)));
		}
		if (isfirst) {
			isfirst = false;
			expected_size = *((unsigned int *)recv_buf);
			if (expected_size >= max_recv_buf_size) {
				COUT_N_EXIT("[server] kvparser expected bytes (" << expected_size << ") exceeds max bytes (" << max_recv_buf_size << ")");
			}
			count = *((unsigned int *)(recv_buf + 4));
		}
		start_idx += recv_size;
		if (start_idx >= expected_size) {
			break;
		}
		else if (start_idx >= max_recv_buf_size) {
			COUT_N_EXIT("[server] kvparser received bytes (" << start_idx << ") exceeds max bytes (" << max_recv_buf_size << ")");
		}
	}
	close(connfd);

	backup_data_t *new_backup_data = new backup_data_t();
	parse_kv(recv_buf + 8, expected_size - 8, count, new_backup_data); // databuf pointer, data size, expected count, new backup data
	rollback(new_backup_data);

	volatile backup_data_t *old_backup_data = backup_data;
	backup_data = new_backup_data;

	COUT_THIS("After rollback")
	for (std::map<index_key_t, val_t>::iterator iter = backup_data->_kvmap.begin(); iter != backup_data->_kvmap.end(); iter++) {
		COUT_VAR(iter->first.to_string());
		COUT_VAR(iter->second.to_string());
	}

	// peace period of RCU (may not need if we do not use them in SCAN)
	// TODO: test periodic backup with background traffic
	/*for (uint32_t i = 0; i < fg_n; i++) {
		uint32_t prev_val = backup_rcu[i];
		while (running && backup_rcu[i] == prev_val) {}
	}*/

	if (old_backup_data != nullptr) {
		delete old_backup_data;
		old_backup_data = nullptr;
	}
	for (size_t i = 0; i < fg_n; i++) {
		//special_cases_list[i]->clear();
		special_cases->clear();
	}

	is_kvsnapshot.clear(std::memory_order_release);
	isbackup = false;

	pthread_exit(nullptr);
}

void parse_kv(const char* data_buf, unsigned int data_size, unsigned int expected_count, backup_data_t *new_backup_data) {
	INVARIANT(data_buf != nullptr && new_backup_data != nullptr);
	const char *cur = data_buf;
	COUT_VAR(expected_count);
	for (uint32_t i = 0; i < expected_count; i++) {
		// Parse data
		unsigned short curhashidx = *((unsigned short *)cur);
		cur += 2;
#ifdef LARGE_KEY
		index_key_t curkey(*(uint64_t*)cur, *(uint64_t*)(cur+8));
		cur += 16;
#else
		index_key_t curkey(*(uint64_t*)cur);
		cur += 8;
#endif
		uint8_t curvallen = *(uint8_t*)cur;
		cur += 1;
		INVARIANT(curvallen >= 0);
		uint64_t curval_data[curvallen];
		for (uint8_t val_idx = 0; val_idx < curvallen; val_idx++) {
			curval_data[val_idx] = *(uint64_t*)cur;
			cur += 8;
		}
		val_t curval(curval_data, curvallen);
		COUT_VAR(curhashidx);
		COUT_VAR(curkey.to_string());
		COUT_VAR(curvallen);
		COUT_VAR(curval.to_string());

		// Update new backup data
		new_backup_data->_kvmap.insert(std::pair<index_key_t, val_t>(curkey, curval));
		new_backup_data->_idxmap.insert(std::pair<unsigned short, index_key_t>(curhashidx, curkey));
	}
}

void rollback(backup_data_t *new_backup_data) {
	//for (size_t i = 0; i < fg_n; i++) {
		//std::map<unsigned short, special_case_t> *cur_special_cases = special_cases_list[i];
		//for (std::map<unsigned short, special_case_t>::iterator iter = cur_special_cases->begin(); 
		//		iter != cur_special_cases->end(); iter++) {
		for (std::map<unsigned short, special_case_t>::iterator iter = special_cases->begin(); 
				iter != special_cases->end(); iter++) {
			if (!iter->second._valid) { // Invalid case
				std::map<unsigned short, index_key_t>::iterator tmpiter = new_backup_data->_idxmap.find(iter->first);
				if (tmpiter != new_backup_data->_idxmap.end()) { // Remove the idx and kv
					new_backup_data->_kvmap.erase(tmpiter->second);
					new_backup_data->_idxmap.erase(tmpiter->first);
				}
			}
			else { // Valid case (vallen could be zero)
				std::map<unsigned short, index_key_t>::iterator tmpiter = new_backup_data->_idxmap.find(iter->first);
				if (tmpiter != new_backup_data->_idxmap.end()) { // Has the idx
					if (tmpiter->second == iter->second._key) { // Same key
						if (new_backup_data->_kvmap.find(tmpiter->second) != new_backup_data->_kvmap.end()) { // Has the key
							new_backup_data->_kvmap[tmpiter->second] = iter->second._val; // Update val
						}
						else { // Without the key
							new_backup_data->_kvmap.insert(std::pair<index_key_t, val_t>(iter->second._key, iter->second._val)); // Insert kv
						}
					}
					else { // Different keys
						new_backup_data->_kvmap.erase(tmpiter->second); // Remove original kv
						new_backup_data->_idxmap[tmpiter->first] = iter->second._key; // Replace the key in idxmap
						new_backup_data->_kvmap.insert(std::pair<index_key_t, val_t>(iter->second._key, iter->second._val)); // Insert new kv in kvmap
					}
				}
				else { // Without the idx
					new_backup_data->_idxmap.insert(std::pair<unsigned short, index_key_t>(iter->first, iter->second._key)); // Insert idx
					new_backup_data->_kvmap.insert(std::pair<index_key_t, val_t>(iter->second._key, iter->second._val)); // Insert new kv
				}
			}
		} // End of current speical_cases
	//} // End of special_cases_list
}

void try_kvsnapshot(xindex_t *table) {
	if (!is_kvsnapshot.test_and_set(std::memory_order_acquire)) {
		// TODO: test periodic backup with background traffic
		//table->make_snapshot();
		table->make_snapshot(true);
	}
}

/*
 * The notified thread to receive explicit notification for server-side snapshot from practical switch
 */

void *run_notified(void *param) {
	// prepare socket as udp server
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	INVARIANT(sockfd >= 0);
	/*int disable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		perror("Disable udp checksum failed");
	}*/
	struct sockaddr_in server_sockaddr;
	memset(&server_sockaddr, 0, sizeof(struct sockaddr_in));
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sockaddr.sin_port = htons(notified_port);
	res = bind(sockfd, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr));
	INVARIANT(res != -1);
	uint32_t sockaddr_len = sizeof(struct sockaddr);

	// Set timeout
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec =  0;
	res = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	INVARIANT(res >= 0);

	char buf[128]; // payload size should <= 1 byte
	while(running) {
		recv_size = recvfrom(sockfd, buf, 128, 0, (struct sockaddr *)&server_sockaddr, &sockaddr_len);
		if (recv_size == -1) {
			if (errno == EWOULDBLOCK || errno == EINTR) {
				continue; // timeout or interrupted system call
			}
			else {
				COUT_THIS("[server] Error of recvfrom: errno = " << errno)
				exit(-1);
			}
		}
		INVARIANT(recv_size == 1);

		// Make snapshot for the notification
		if (!isbackup) {
			try_kvsnapshot(table);
		}
	}

	close(sockfd);
	pthread_exit(nullptr);
}

/*
 * Tcp server for each worker to receive mirrored packet for packet loss
 */

void *run_tcpserver_for_pktloss(void *param) {
	tcpserver_param_t param = *((tcpserver_param_t *)param);
	uint8_t thread_id = param.thread_id;

	//sockaddr_in switchos_addr;
    //memset(&switchos_addr, 0, sizeof(switchos_addr)); 
    //unsigned int len = sizeof(switchos_addr);
	//int connfd = accept(server_tcpsocks[thread_id], (struct sockaddr*)&switchos_addr, &len);
	
	// For tcp channel
	char buf[MAX_BUFSIZE];
	int cur_recv_bytes = 0;
	int cur_vallen_bytes = -1;
	const int min_recv_bytes = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(key_t) + sizeof(uint32_t); // sizeof(op_hdr) + sizeof(vallen)
	// For mbuf from tcpserver to worker
	uint16_t burst_size = 256;
	struct rte_mbuf *sent_pkts[burst_size]; // Worker will free each sent_pkt after processing
	uint16_t sent_pkt_idx = 0;
	int res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
	INVARIANT(res == 0);
	// Used as placeholder in mbuf; worker does not care about them as EVICT/CASE2 do not need response
	uint8_t emptymac[6];
	memset((char *)emptymac, 0, 6);
	char emptyip[16] = "0.0.0.0";
	uint16_t emptyport = 0;
	
	ready_tcpserver_threads++;
	
	// We only accept one connection from switch os to each tcpserver in worker i
	int connfd = accept(server_tcpsocks[thread_id], NULL, NULL);
	if (connfd < 0) {
		printf("Tcp server of server %ld fails to accept, errno: %d!\n", thread_id, errno);
		exit(-1);
	}
	//printf("Tcp server of server %ld accepts a connection successfully: %d\n", thread_id, connfd);
	
	while (true) {
		int tmpsize = recv(connfd, buf + cur_recv_bytes, MAX_BUFSIZE - cur_recv_bytes, 0);
		if (tmpsize < 0) {
            printf("Fail to recv at tcp server of server %ld: %d\n", thread_id, tmpsize);
			break;
        }
		cur_recv_bytes += tmpsize;
		if (cur_recv_bytes >= MAX_BUFSIZE) {
			printf("Overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_BUFSIZE);
            exit(-1);
		}

		// Get value length
		if (cur_vallen_bytes == -1 && cur_recv_bytes >= min_recv_bytes) {
			cur_vallen_bytes = *((uint32_t*)(buf+min_recv_bytes-sizeof(uint32_t)));
			// padding if necessary
			if (cur_vallen_bytes <= val_t::SWITCH_MAX_VALLEN) {
				cur_vallen_bytes = (cur_vallen_bytes+7)/8*8;
			}
		}

		// Get one complete packet
		if (cur_vallen_bytes != -1 && cur_recv_bytes >= (min_recv_bytes + cur_vallen_bytes)) {
			// Notify mbuf to corresponding worker
			encode_mbuf(sent_pkts[sent_pkt_idx], emptymac, emptymac, emptyip, emptyip, emptyport, emptyport, buf, min_recv_bytes+cur_vallen_bytes);
			if (((heads_for_pktloss[thread_id] + 1) % MQ_SIZE) != tails_for_pktloss[thread_id]) {
				pkts_list_for_pktloss[thread_id][heads_for_pktloss[thread_id]] = sent_pkts[sent_pkt_idx]; // it will be freed by worker
				heads_for_pktloss[thread_id] = (heads_for_pktloss[thread_id] + 1) % MQ_SIZE;
			}
			else {
				COUT_THIS("Drop pkt since pkts_list_for_pktloss["<<thread_id<<"] is full!")
				rte_pktmbuf_free(sent_pkts[sent_pkt_idx]);
			}

			// Move remaining bytes and reset metadata
			if (cur_recv_bytes > (min_recv_bytes + cur_vallen_bytes)) {
				memcpy(buf, buf + min_recv_bytes + cur_vallen_bytes, cur_recv_bytes - min_recv_bytes - cur_vallen_bytes);
				cur_recv_bytes = cur_recv_bytes - min_recv_bytes - cur_vallen_bytes;
				cur_vallen_bytes = -1;
			}
			else {
				cur_recv_bytes = 0;
				cur_vallen_bytes = -1;
			}

			sent_pkt_idx++;
			if (sent_pkt_idx >= burst_size) {
				sent_pkt_idx = 0;
				res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
				INVARIANT(res == 0);
			}
		}
	}

	if (sent_pkt_idx < burst_size) {
		for (uint16_t free_idx = sent_pkt_idx; free_idx != burst_size; free_idx++) {
			rte_pktmbuf_free(sent_pkts[free_idx]);
		}
	}
	close(connfd);
	close(server_tcpsocks[thread_id]);
	pthread_exit(NULL);
}

/*
 * Switch OS simulator for special case processing and packet loss handling
 */

void *run_switchos_simulator(void *param) {
	// (1) Switch OS simulator does not need mbufs to send DPDK packets; instead, it uses tcp packets to communicate with servers if necessary; (2) it also does not need to decode metadata of client; instead, it only needs to get the payload; (3) we simulate practical latency (pcie latency of copy to switch os + transmission latency from switch os to server -> transmission latency of copy to switch os + pcie latency from switch os to server); what's more, it is irrelevant of normal pkt latency and very rare on skewed workload;
	
	// Create fg_n tcp channels
	for (size_t i = 0; i < fg_n; i++) {
		sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // enforce the packet to go through NIC 
		server_addr.sin_port = htons(pktloss_start_port+i);
		if (connect(switchos_tcpsocks[i], (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
			error("Fail to connect tcp server at %hu, errno: %d!\n", pktloss_start_port+i, errno);
		}
	}

	char buf[MAX_BUFSIZE];
	int recv_size = 0;
	int received_port = -1;

	while (!running) {
	}

	while (running) {
		if (head_for_simulator != tail_for_simulator) {
			INVARIANT(pkts_for_simulator[tail_for_simulator] != nullptr);
			received_port = get_dstport(pkts_for_simulator[tail_for_simulator]);
			recv_size = get_payload(pkts_for_simulator[tail_for_simulator], buf);
			rte_pktmbuf_free((struct rte_mbuf*)pkts_for_simulator[tail_for_simulator]);
			tail_for_simulator = (tail_for_simulator + 1) % MQ_SIZE;

			packet_type_t pkt_type = get_packet_type(buf, recv_size);
			switch (pkt_type) {
				// NOTE: switch os add special cases for rollback (case1 does not trigger server-side snapshot)
				case packet_type_t::PUT_REQ_CASE1:
					{
						COUT_THIS("PUT_REQ_CASE1")
						if (!isbackup) {
							put_request_case1_t req(buf, recv_size);
							//if (special_cases_list[thread_id]->find((unsigned short)req.hashidx()) 
							//		== special_cases_list[thread_id]->end()) { // No such hashidx
							if (special_cases->find((unsigned short)req.hashidx()) == special_cases->end()) { // No such hashidx
								SpecialCase tmpcase;
								tmpcase._key = req.key();
								tmpcase._val = req.val();
								tmpcase._valid = (req.val().val_length > 0); // vallen = 0 means deleted
								//special_cases_list[thread_id]->insert(std::pair<unsigned short, SpecialCase>(\
								//			(unsigned short)req.hashidx(), tmpcase));
								special_cases->insert(std::pair<unsigned short, SpecialCase>((unsigned short)req.hashidx(), tmpcase));
							}
						}
						break;
					}
				case packet_type_t::DEL_REQ_CASE1:
					{
						COUT_THIS("DEL_REQ_CASE1")
						del_request_case1_t req(buf, recv_size);
						if (!isbackup) {
							//if (special_cases_list[thread_id]->find((unsigned short)req.hashidx()) 
							//		== special_cases_list[thread_id]->end()) { // No such hashidx
							if (special_cases->find((unsigned short)req.hashidx()) == special_cases->end()) { // No such hashidx
								SpecialCase tmpcase;
								tmpcase._key = req.key();
								tmpcase._val = req.val();
								tmpcase._valid = (req.val().val_length > 0); // vallen = 0 means deleted
								//special_cases_list[thread_id]->insert(std::pair<unsigned short, SpecialCase>(\
								//			(unsigned short)req.hashidx(), tmpcase));
								special_cases->insert(std::pair<unsigned short, SpecialCase>((unsigned short)req.hashidx(), tmpcase));
							}
						}
						break;
					}
				case packet_type_t::GET_RES_POP_EVICT_SWITCH:
				case packet_type_t::PUT_REQ_POP_EVICT_SWITCH:
				case packet_type_t::PUT_REQ_LARGE_EVICT_SWITCH:
				case packet_type_t::GET_RES_POP_EVICT_CASE2_SWITCH:
				case packet_type_t::PUT_REQ_POP_EVICT_CASE2_SWITCH:
				case packet_type_t::PUT_REQ_LARGE_EVICT_CASE2_SWITCH:
					{
						// NOTE: not break -> proceed to forward to server by tcp channel
						if (pkt_type == packet_type_t::GET_RES_POP_EVICT_CASE2_SWITCH) {
							COUT_THIS("GET_RES_POP_EVICT_CASE2_SWITCH")
							get_response_pop_evict_case2_switch_t req(buf, recv_size);
							if (!isbackup) {
								//if (special_cases_list[thread_id]->find((unsigned short)req.hashidx()) 
								//		== special_cases_list[thread_id]->end()) { // No such hashidx
								if (special_cases->find((unsigned short)req.hashidx()) == special_cases->end()) { // No such hashidx
									SpecialCase tmpcase;
									tmpcase._key = req.key();
									tmpcase._val = req.val();
									//tmpcase._valid = (req.val().val_length > 0); // vallen = 0 means deleted
									tmpcase._valid = req.valid();
									//special_cases_list[thread_id]->insert(std::pair<unsigned short, SpecialCase>(\
									//			(unsigned short)req.hashidx(), tmpcase));
									special_cases->insert(std::pair<unsigned short, SpecialCase>((unsigned short)req.hashidx(), tmpcase));
								}
							}
						}
						else if (pkt_type == packet_type_t::PUT_REQ_POP_EVICT_CASE2_SWITCH) {
							COUT_THIS("PUT_REQ_POP_EVICT_CASE2_SWITCH")
							put_request_pop_evict_case2_switch_t req(buf, recv_size);
							if (!isbackup) {
								//if (special_cases_list[thread_id]->find((unsigned short)req.hashidx()) 
								//		== special_cases_list[thread_id]->end()) { // No such hashidx
								if (special_cases->find((unsigned short)req.hashidx()) == special_cases->end()) { // No such hashidx
									SpecialCase tmpcase;
									tmpcase._key = req.key();
									tmpcase._val = req.val();
									//tmpcase._valid = (req.val().val_length > 0); // vallen = 0 means deleted
									tmpcase._valid = req.valid();
									//special_cases_list[thread_id]->insert(std::pair<unsigned short, SpecialCase>(\
									//			(unsigned short)req.hashidx(), tmpcase));
									special_cases->insert(std::pair<unsigned short, SpecialCase>((unsigned short)req.hashidx(), tmpcase));
								}
							}
						}
						else if (pkt_type == packet_type_t::PUT_REQ_LARGE_EVICT_CASE2_SWITCH) {
							COUT_THIS("PUT_REQ_LARGE_EVICT_CASE2_SWITCH")
							put_request_large_evict_case2_switch_t req(buf, recv_size);
							INVARIANT(req.val().val_length <= val_t.SWITCH_MAX_VALLEN);
							if (!isbackup) { // Not receive backup data yet
								//if (special_cases_list[thread_id]->find((unsigned short)req.hashidx()) 
								//		== special_cases_list[thread_id]->end()) { // No such hashidx
								if (special_cases->find((unsigned short)req.hashidx()) == special_cases->end()) { // No such hashidx
									SpecialCase tmpcase;
									tmpcase._key = req.key();
									tmpcase._val = req.val();
									//tmpcase._valid = (req.val().val_length > 0); // vallen = 0 means deleted
									tmpcase._valid = 1; // valid must be 1 for PUTREQ_LARGE_EVICT_CASE2
									//special_cases_list[thread_id]->insert(std::pair<unsigned short, SpecialCase>(\
									//			(unsigned short)req.hashidx(), tmpcase));
									special_cases->insert(std::pair<unsigned short, SpecialCase>((unsigned short)req.hashidx(), tmpcase));
								}
							}
						}

						// Forward to corresponding server by tcp channel
						int idx = received_port - dst_port_start;
						if (idx < 0 || unsigned(idx) >= fg_n) {
							COUT_THIS("Invalid dst port received by switch os: " << received_port)
							continue;
						}
						int send_size = recv_size;
						const char *ptr = buf;
						while (send_size > 0) {
							int tmpsize = send(switchos_socks[idx], ptr, send_size, 0);
							f (tmpsize < 0) {
								// Errno 32 means broken pipe, i.e., remote TCP connection is closed
								error("TCP send returns %d, errno: %d\n", tmpsize, errno);
								break;
							}
							ptr += tmpsize;
							send_size -= tmpsize;
						}
						break;
					}
				default:
					{
						COUT_THIS("[switch os simulator] Invalid packet type: " << int(pkt_type))
						std::cout << std::flush;
						exit(-1);
					}
			}

		}
	}

	for (size_t i = 0; i < fg_n; i++) {
		close(switchos_tcpsocks[i]);
	}
	COUT_THIS("switch os simulator exits")
	pthread_exit(nullptr);
}

/*
 * Worker for server-side processing 
 */

static int run_sfg(void * param) {
//void *run_sfg(void * param) {
  // Parse param
  sfg_param_t &thread_param = *(sfg_param_t *)param;
  uint8_t thread_id = thread_param.thread_id;
  xindex_t *table = thread_param.table;

  int res = 0;

  // DPDK
  //struct rte_mbuf *sent_pkt = rte_pktmbuf_alloc(mbuf_pool); // Send to DPDK port
  //struct rte_mbuf *sent_pkt_wrapper[1] = {sent_pkt};
  // Optimize mbuf allocation
  uint16_t burst_size = 256;
  struct rte_mbuf *sent_pkts[burst_size];
  uint16_t sent_pkt_idx = 0;
  struct rte_mbuf *sent_pkt = NULL;
  res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
  INVARIANT(res == 0);

  // prepare socket
  /*int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  INVARIANT(sockfd >= 0);
  int disable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
	perror("Disable udp checksum failed");
  }
  short dst_port = dst_port_start + thread_id;
  struct sockaddr_in server_sockaddr;
  memset(&server_sockaddr, 0, sizeof(struct sockaddr_in));
  server_sockaddr.sin_family = AF_INET;
  server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_sockaddr.sin_port = htons(dst_port);
  res = bind(sockfd, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr));
  INVARIANT(res != -1);
  uint32_t sockaddr_len = sizeof(struct sockaddr);

  // Set timeout
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec =  0;
  res = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  INVARIANT(res >= 0);*/

  char buf[MAX_BUFSIZE];
  int recv_size = 0;
  int rsp_size = 0;

  // DPDK
  uint8_t srcmac[6];
  uint8_t dstmac[6];
  char srcip[16];
  char dstip[16];
  uint16_t srcport;
  uint16_t unused_dstport; // we use dst_port_start instead of received dstport to hide server-side partition for client

  ready_threads++;

  while (!running) {
  }

  while (running) {
	/*recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, (struct sockaddr *)&server_sockaddr, &sockaddr_len);
	if (recv_size == -1) {
		if (errno == EWOULDBLOCK || errno == EINTR) {
			continue; // timeout or interrupted system call
		}
		else {
			COUT_THIS("[server] Error of recvfrom: errno = " << errno)
			exit(-1);
		}
	}*/

#ifdef TEST_AGG_THPT
	struct timespec t1, t2, t3;
	CUR_TIME(t1);
#endif
	//if (stats[thread_id]) {
	if (heads[thread_id] != tails[thread_id]) {
		//COUT_THIS("[server] Receive packet!")

		// DPDK
		//stats[thread_id] = false;
		//recv_size = decode_mbuf(pkts[thread_id], srcmac, dstmac, srcip, dstip, &srcport, &dstport, buf);
		//rte_pktmbuf_free((struct rte_mbuf*)pkts[thread_id]);
		sent_pkt = sent_pkts[sent_pkt_idx];

		INVARIANT(pkts_list[thread_id][tails[thread_id]] != nullptr);
		memset(srcip, '\0', 16);
		memset(dstip, '\0', 16);
		memset(srcmac, 0, 6);
		memset(dstmac, 0, 6);
		recv_size = decode_mbuf(pkts_list[thread_id][tails[thread_id]], srcmac, dstmac, srcip, dstip, &srcport, &unused_dstport, buf);
		rte_pktmbuf_free((struct rte_mbuf*)pkts_list[thread_id][tails[thread_id]]);
		tails[thread_id] = (tails[thread_id] + 1) % MQ_SIZE;

		/*if ((debug_idx + 1) % 10001 == 0) {
			COUT_VAR((t0 - prevt0) / 10000.0);
			prevt0 = t0;
		}*/

		packet_type_t pkt_type = get_packet_type(buf, recv_size);
		switch (pkt_type) {
			case packet_type_t::GET_REQ: 
				{
					get_request_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					val_t tmp_val;
					bool tmp_stat = table->get(req.key(), tmp_val, thread_id);
					//COUT_THIS("[server] val = " << tmp_val.to_string())
					get_response_t rsp(req.hashidx(), req.key(), tmp_val);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dst_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::PUT_REQ_SEQ:
				{
					put_request_seq_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool tmp_stat = table->put(req.key(), req.val(), thread_id, req.seq());
					//COUT_THIS("[server] stat = " << tmp_stat)
					put_response_t rsp(req.hashidx(), req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dst_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::DEL_REQ_SEQ:
				{
					del_request_seq_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					bool tmp_stat = table->remove(req.key(), thread_id, req.seq());
					//COUT_THIS("[server] stat = " << tmp_stat)
					del_response_t rsp(req.hashidx(), req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dst_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;

					// NOTE: no matter tmp_stat is true (key is deleted) or false (no such key or key has been deleted before), we should always treat the key does not exist (i.e., being deleted), so a reordered eviction will never overwrite this result for linearizability -> we should always update the corresponding deleted set
					deleted_sets[thread_id].add(req.key(), req.seq());
					break;
				}
			case packet_type_t::SCAN_REQ:
				{
					scan_request_t req(buf, recv_size);
					//COUT_THIS("[server] startkey = " << req.key().to_string() << 
					//		<< "endkey = " << req.endkey().to_string() << " num = " << req.num())
					std::vector<std::pair<index_key_t, val_t>> results;
					//size_t tmp_num = table->scan(req.key(), req.num(), results, thread_id);
					size_t tmp_num = table->range_scan(req.key(), req.endkey(), results, thread_id);

					// Add kv pairs of backup data into results
					std::map<index_key_t, val_t> *kvdata = &backup_data->_kvmap;
					if (kvdata != nullptr) {
						std::map<index_key_t, val_t>::iterator kviter = kvdata->lower_bound(req.key());
						for (; kviter != kvdata->end() && kviter->first < req.endkey(); kviter++) {
							results.push_back(*kviter);
						}
					}
					//COUT_THIS("SCAN results size: " << results.size())

					scan_response_t rsp(req.hashidx(), req.key(), req.endkey(), results.size(), results);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dst_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::GET_REQ_POP: 
				{
					get_request_pop_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					val_t tmp_val;
					bool tmp_stat = table->get(req.key(), tmp_val, thread_id);
					//COUT_THIS("[server] val = " << tmp_val.to_string())
					
					if (tmp_stat && tmp_val.val_length > 0) {
						get_response_pop_t rsp(req.hashidx(), req.key(), tmp_val);
						rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					}
					else {
						if (tmp_val.val_length > val_t::SWITCH_MAX_VALLEN) {
							get_response_pop_large_t rsp(req.hashidx(), req.key(), tmp_val);
							rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
						}
						else {
							get_response_npop_t rsp(req.hashidx(), req.key(), tmp_val);
							rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
						}
					}
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dst_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::GET_RES_POP_EVICT:
				{
					// Put evicted data into key-value store
					get_response_pop_evict_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_POP_EVICT:
				{
					// Put evicted data into key-value store
					put_request_pop_evict_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_SEQ:
				{
					put_request_large_seq_t req(buf, recv_size);
					INVARIANT(req.val().val_length > val_t::SWITCH_MAX_VALLEN);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool tmp_stat = table->put(req.key(), req.val(), thread_id, req.seq());
					//COUT_THIS("[server] stat = " << tmp_stat)
					put_response_t rsp(req.hashidx(), req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dst_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_EVICT:
				{
					// Put evicted data into key-value store
					put_request_large_evict_t req(buf, recv_size);
					INVARIANT(req.val().val_length <= val_t::SWITCH_MAX_VALLEN);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::GET_RES_POP_EVICT_CASE2:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					get_response_pop_evict_case2_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_POP_EVICT_CASE2:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					put_request_pop_evict_case2_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_EVICT_CASE2:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					put_request_large_evict_case2_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					INVARIANT(req.val().val_length <= val_t.SWITCH_MAX_VALLEN);
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_CASE3:
				{
					COUT_THIS("PUT_REQ_CASE3")
					put_request_case3_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool tmp_stat = table->put(req.key(), req.val(), thread_id, req.seq());
					put_response_case3_t rsp(req.hashidx(), req.key(), thread_id, tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dst_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::DEL_REQ_CASE3:
				{
					COUT_THIS("DEL_REQ_CASE3")
					del_request_case3_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool tmp_stat = table->remove(req.key(), thread_id, req.seq());
					del_response_case3_t rsp(req.hashidx(), req.key(), thread_id, tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dst_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;

					// NOTE: no matter tmp_stat is true (key is deleted) or false (no such key or key has been deleted before), we should always treat the key does not exist (i.e., being deleted), so a reordered eviction will never overwrite this result for linearizability -> we should always update the corresponding deleted set
					deleted_sets[thread_id].add(req.key(), req.seq());
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_CASE3:
				{
					COUT_THIS("PUT_REQ_LARGE_CASE3")
					put_request_large_case3_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool tmp_stat = table->put(req.key(), req.val(), thread_id, req.seq());
					put_response_case3_t rsp(req.hashidx(), req.key(), thread_id, tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dst_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			default:
				{
					COUT_THIS("[server] Invalid packet type from receiver: " << int(pkt_type))
					std::cout << std::flush;
					exit(-1);
				}
		}
#ifdef TEST_AGG_THPT
		CUR_TIME(t2);
		DELTA_TIME(t2, t1, t3);
		thread_param.sum_latency += GET_MICROSECOND(t3);
#endif
		backup_rcu[thread_id]++;
		thread_param.throughput++;

		if (sent_pkt_idx >= burst_size) {
			sent_pkt_idx = 0;
			res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
			INVARIANT(res == 0);
		}
	} // End for mbuf from receiver to server
	if (heads_for_pktloss[thread_id] != tails_for_pktloss[thread_id]) {
		INVARIANT(pkts_list_for_pktloss[thread_id][tails_for_pktloss[thread_id]] != nullptr);
		recv_size = get_payload(pkts_list_for_pktloss[thread_id][tails_for_pktloss[thread_id]], buf);
		rte_pktmbuf_free((struct rte_mbuf*)pkts_list_for_pktloss[thread_id][tails_for_pktloss[thread_id]]);
		tails_for_pktloss[thread_id] = (tails_for_pktloss[thread_id] + 1) % MQ_SIZE;

		packet_type_t pkt_type = get_packet_type(buf, recv_size);
		switch (pkt_type) {
			// NOTE: we use seq mechanism to avoid incorrect overwrite of packet reordering
			case packet_type_t::GET_RES_POP_EVICT_SWITCH:
				{
					// Put evicted data into key-value store
					get_response_pop_evict_switch_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_POP_EVICT_SWITCH:
				{
					// Put evicted data into key-value store
					put_request_pop_evict_switch_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_EVICT_SWITCH:
				{
					// Put evicted data into key-value store
					put_request_large_evict_switch_t req(buf, recv_size);
					INVARIANT(req.val().val_length <= val_t::SWITCH_MAX_VALLEN);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::GET_RES_POP_EVICT_CASE2_SWITCH:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					get_response_pop_evict_case2_switch_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_POP_EVICT_CASE2_SWITCH:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					put_request_pop_evict_case2_switch_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_EVICT_CASE2_SWITCH:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					put_request_large_evict_case2_switch_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					INVARIANT(req.val().val_length <= val_t.SWITCH_MAX_VALLEN);
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			default:
				{
					COUT_THIS("[server] Invalid packet type from tcp channel: " << int(pkt_type))
					std::cout << std::flush;
					exit(-1);
				}
		}
	} // End for mbuf from switch os (and tcp server) to server 
  }
  
  // DPDK
  //rte_pktmbuf_free((struct rte_mbuf*)sent_pkt);
  if (sent_pkt_idx < burst_size) {
	for (uint16_t free_idx = sent_pkt_idx; free_idx != burst_size; free_idx++) {
		rte_pktmbuf_free(sent_pkts[free_idx]);
	}
  }

  //close(sockfd);
  COUT_THIS("[thread" << uint32_t(thread_id) << "] exits")
  //pthread_exit(nullptr);
  return 0;
}

void kill(int signum) {
	COUT_THIS("[server] Receive SIGKILL!")
	killed = true;
}
