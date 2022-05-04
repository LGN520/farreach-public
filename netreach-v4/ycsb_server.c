#include <getopt.h>
#include <stdlib.h>
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
#include "dpdk_helper.h"
#include "key.h"
#include "iniparser/iniparser_wrapper.h"
#include "ycsb/parser.h"
#include "socket_helper.h"

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
#endif

#define MAX_VERSION 0xFFFFFFFFFFFFFFFF

#include "common_impl.h"
#include "reflector_impl.h"
#include "server_impl.h"

struct alignas(CACHELINE_SIZE) LoadSFGParam {
	xindex_t *table;
	uint8_t thread_id;
};
typedef LoadSFGParam load_sfg_param_t;

// Prepare data structures
void prepare_dpdk();

// loading phase
volatile bool load_running = false;
std::atomic<size_t> load_ready_threads(0);
void run_load_server(xindex_t *table); // loading phase
void *run_load_sfg(void *param); // workers in loading phase
void load(std::vector<index_key_t> &keys, std::vector<val_t> &vals);

size_t get_server_idx(index_key_t key) {
	size_t server_idx = key.keylo / per_server_range;
	if (server_idx >= server_num) {
		server_idx = server_num - 1;
	}
	return server_idx;
}

int main(int argc, char **argv) {
  parse_ini("config.ini");
  parse_control_ini("control_type.ini");
  //parse_args(argc, argv);
  //xindex::init_options(); // init options of rocksdb
  
  /* (1) prepare phase */

  // prepare xindex
  std::vector<index_key_t> keys;
  std::vector<val_t> vals;
  load(keys, vals); // Use the last split workload to initialize the XIndex
  xindex_t *tab_xi = new xindex_t(keys, vals, server_num, bg_n); // server_num to create array of RCU status; bg_n background threads have been launched

  // Prepare DPDK EAL param
  prepare_dpdk();

  // register signal handler
  signal(SIGTERM, SIG_IGN); // Ignore SIGTERM for subthreads

  /* (2) loading phase */

  run_load_server(tab_xi);

  /* (3) transaction phase */

  run_server(tab_xi); // loop until killed

  if (tab_xi != nullptr) delete tab_xi; // terminate_bg -> bg_master joins bg_threads

  close_server();
  dpdk_free();

  COUT_THIS("[ycsb_server.main] Exit successfully")
  exit(0);
}

/*
 * Prepare data structures 
 */

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
  dpdk_init(&mbuf_pool, server_num+1, 1); // tx: server_num server.workers + one reflector; rx: one receiver
}

/*
 * Loading phase
 */

void load(std::vector<index_key_t> &keys, std::vector<val_t> &vals) {
	char load_filename[256];
	memset(load_filename, '\0', 256);
	GET_SPLIT_WORKLOAD(load_filename, server_load_workload_dir, split_n-1);

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
	for (size_t i = 0; i < server_num; i++) {
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
	GET_SPLIT_WORKLOAD(load_filename, server_load_workload_dir, thread_id);

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
