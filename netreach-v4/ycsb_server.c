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
#include "ycsb/parser.h"

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
#include "latency_helper.h"

/* class and alias */

typedef xindex::XIndex<index_key_t, val_t> xindex_t;

struct alignas(CACHELINE_SIZE) LoadSFGParam {
	xindex_t *table;
	uint16_t thread_id;
};
typedef LoadSFGParam load_sfg_param_t;

/* variables */

// prepare phase
struct rte_mempool *mbuf_pool = NULL;
// loading phase
volatile bool load_running = false;
std::atomic<size_t> load_ready_threads(0);
// transaction phase
bool volatile transaction_running = false;
std::atomic<size_t> transaction_ready_threads(0);
size_t transaction_expected_ready_threads = 0;
bool volatile killed = false;

/* functions */

// prepare phase
void prepare_dpdk();
// loading phase
void loading_main(xindex_t *table); // loading phase
void *run_load_sfg(void *param); // workers in loading phase
void load(std::vector<index_key_t> &keys, std::vector<val_t> &vals);
// transaction phase
#include "reflector_impl.h"
#include "server_impl.h"
void transaction_main(xindex_t *table); // transaction phase
void kill(int signum);

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
  printf("[main] loading phase start\n");

  //prepare_load_server();
  loading_main(tab_xi);

  /* (3) transaction phase */
  printf("[main] transaction phase start\n");

  // update transaction_expected_ready_threads
  transaction_expected_ready_threads = server_num + 3 + 2;

  prepare_reflector();
  prepare_server();
  transaction_main(tab_xi);

  /* (4) free phase */

  if (tab_xi != nullptr) delete tab_xi; // terminate_bg -> bg_master joins bg_threads
  dpdk_free();
  // close_load_server();
  close_reflector();
  close_server();

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
 * Prepare phase
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
		if (iter.type() == uint8_t(packet_type_t::PUTREQ)) {	// INESRT
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

/*
 * Loading phase
 */

void loading_main(xindex_t *table) {
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
		load_sfg_params[worker_i].thread_id = static_cast<uint16_t>(worker_i);
		int ret = pthread_create(&threads[worker_i], nullptr, run_load_sfg, (void *)&load_sfg_params[worker_i]);
		if (ret) {
			COUT_N_EXIT("Error:" << ret);
		}
		lcoreid++;
		if (lcoreid >= MAX_LCORE_NUM) {
			lcoreid = 1;
		}
	}

	while (load_ready_threads < load_n) sleep(1);
	COUT_THIS("[loading] all loading threads ready");

	load_running = true;

	void *status;
	for (size_t i = 0; i < server_num; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error:unable to join," << rc);
		}
	}
	COUT_THIS("[loading] loading phase finish");
}

void *run_load_sfg(void * param) {
	// Parse param
	load_sfg_param_t &thread_param = *(load_sfg_param_t *)param;
	uint16_t thread_id = thread_param.thread_id;
	xindex_t *table = thread_param.table;

	char load_filename[256];
	memset(load_filename, '\0', 256);
	GET_SPLIT_WORKLOAD(load_filename, server_load_workload_dir, thread_id);

	Parser parser(load_filename);
	ParserIterator iter = parser.begin();
	index_key_t tmpkey;
	val_t tmpval;

	COUT_THIS("[loading.worker " << uint32_t(thread_id) << "] Ready.");

	load_ready_threads++;

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
		if (iter.type() == uint8_t(packet_type_t::PUTREQ)) {	// INESRT
			bool tmp_stat = table->force_put(tmpkey, tmpval, thread_id); // Use put instead of data_put as data structure (i.e., vector) cannot handle dyanmic insert
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

void transaction_main(xindex_t *table) {
	// reflector: popserver + dpdkserver
	// server: server_num workers + receiver + evictserver + consnapshotserver
	transaction_expected_ready_threads = server_num + 5;

	int ret = 0;
	unsigned lcoreid = 1; // 0: master lcore; 1: slave lcore for receiver; 2~: slave lcores for workers

	transaction_running = false;

	// launch popserver
	pthread_t popserver_thread;
	ret = pthread_create(&popserver_thread, nullptr, run_reflector_popserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error of launching reflector.popserver: " << ret);
	}

	// launch dpdkserver
	pthread_t dpdkserver_thread;
	ret = pthread_create(&dpdkserver_thread, nullptr, run_reflector_dpdkserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error of launching reflector.dpdkserver: " << ret);
	}

	// launch receiver
	//receiver_param_t receiver_param;
	//receiver_param.overall_thpt = 0;
	//ret = rte_eal_remote_launch(run_receiver, (void*)&receiver_param, lcoreid);
	ret = rte_eal_remote_launch(run_receiver, NULL, lcoreid);
	if (ret) {
		COUT_N_EXIT("Error of launching server.receiver:" << ret);
	}
	//COUT_THIS("[transaction phase] Launch receiver with ret code " << ret);
	lcoreid++;

	// launch workers (processing normal packets)
	//pthread_t threads[server_num];
	server_worker_param_t server_worker_params[server_num];
	// check if parameters are cacheline aligned
	for (size_t i = 0; i < server_num; i++) {
		if ((uint64_t)(&(server_worker_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(server_worker_params[i]));
		}
	}
	for (uint16_t worker_i = 0; worker_i < server_num; worker_i++) {
		server_worker_params[worker_i].table = table;
		server_worker_params[worker_i].serveridx = worker_i;
		server_worker_params[worker_i].throughput = 0;
		server_worker_params[worker_i].latency_list.clear();
#ifdef TEST_AGG_THPT
		server_worker_params[worker_i].sum_latency = 0.0;
#endif
		//int ret = pthread_create(&threads[worker_i], nullptr, run_sfg, (void *)&server_worker_params[worker_i]);
		ret = rte_eal_remote_launch(run_server_worker, (void *)&server_worker_params[worker_i], lcoreid);
		if (ret) {
		  COUT_N_EXIT("Error of launching some server.worker:" << ret);
		}
		lcoreid++;
		if (lcoreid >= MAX_LCORE_NUM) {
			lcoreid = 1;
		}
	}
	//COUT_THIS("[tranasaction phase] prepare server worker threads...")

	// launch evictserver
	pthread_t evictserver_thread;
	ret = pthread_create(&evictserver_thread, nullptr, run_server_evictserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error of launching server.evictserver: " << ret);
	}

	// launch consnapshotserver
	pthread_t consnapshotserver_thread;
	ret = pthread_create(&consnapshotserver_thread, nullptr, run_server_consnapshotserver, (void *)table);
	if (ret) {
		COUT_N_EXIT("Error of launching server.consnapshotserver: " << ret);
	}

	while (transaction_ready_threads < transaction_expected_ready_threads) sleep(1);

	transaction_running = true;
	COUT_THIS("[transaction.main] all threads ready");

	signal(SIGTERM, kill); // Set for main thread (kill -15)

	while (!killed) {
		sleep(1);
	}

	/* Processing Statistics */
	//COUT_THIS("Server-side aggregate throughput: " << receiver_param.overall_thpt);
	size_t overall_thpt = 0;
	std::vector<double> load_balance_ratio_list;
	for (size_t i = 0; i < server_num; i++) {
		overall_thpt += server_worker_params[i].throughput;
	}
	COUT_THIS("Server-side overall throughput: " << overall_thpt);
	double avg_per_server_thpt = double(overall_thpt) / double(server_num);
	for (size_t i = 0; i < server_num; i++) {
		load_balance_ratio_list.push_back(double(server_worker_params[i].throughput) / avg_per_server_thpt);
	}
	for (size_t i = 0; i < load_balance_ratio_list.size(); i++) {
		COUT_THIS("Load balance ratio of server " << i << ": " << load_balance_ratio_list[i]);
	}
	std::vector<double> worker_latency_list;
	for (size_t i = 0; i < server_num; i++) {
		worker_latency_list.insert(worker_latency_list.end(), server_worker_params[i].latency_list.begin(), server_worker_params[i].latency_list.end());
	}
	dump_latency(worker_latency_list, "worker_latency_list");
	dump_latency(receiver_latency_list, "receiver_latency_list");
#ifdef TEST_AGG_THPT
	double max_agg_thpt = 0.0;
	double avg_latency = 0.0;
	for (size_t i = 0; i < server_num; i++) {
		max_agg_thpt += (double(server_worker_params[i].throughput) / server_worker_params[i].sum_latency * 1000 * 1000);
		avg_latency += server_worker_params[i].sum_latency;
	}
	max_agg_thpt /= double(1024 * 1024);
	avg_latency /= overall_thpt;
	COUT_THIS("Max server-side aggregate throughput: " << max_agg_thpt << " MQPS");
	COUT_THIS("Average latency: " << avg_latency << "us");
#endif

	transaction_running = false;
	void *status;
	/*for (size_t i = 0; i < server_num; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error: unable to join," << rc);
		}
	}*/
	int rc = pthread_join(evictserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join evictserver " << rc);
	}
	rc = pthread_join(consnapshotserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join consnapshotserver " << rc);
	}
	rte_eal_mp_wait_lcore();
	printf("[transaction.main] all threads end");
}

void kill(int signum) {
	COUT_THIS("[transaction phase] receive SIGKILL!")
	killed = true;
}
