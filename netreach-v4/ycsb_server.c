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
#include "deleted_set_impl.h"
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
#include "server_impl.h"
#include "reflector_impl.h"

struct alignas(CACHELINE_SIZE) TcpServerParam {
	uint8_t thread_id;
};
typedef TcpServerParam tcpserver_param_t;

struct alignas(CACHELINE_SIZE) LoadSFGParam {
	xindex_t *table;
	uint8_t thread_id;
};
typedef LoadSFGParam load_sfg_param_t;

typedef DeletedSet<index_key_t> deleted_set_t;
//typedef ReceiverParam receiver_param_t;

// Prepare data structures
void prepare_dpdk();
void prepare_receiver();

// loading phase
volatile bool load_running = false;
std::atomic<size_t> load_ready_threads(0);
void run_load_server(xindex_t *table); // loading phase
void *run_load_sfg(void *param); // workers in loading phase
void load(std::vector<index_key_t> &keys, std::vector<val_t> &vals);

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
/*struct alignas(CACHELINE_SIZE) ReceiverParam {
	size_t overall_thpt;
};*/

// (2-1) Notified for processing explicit notification of server-side snapshot
void *run_notified(void *param); // the notified


// (3) Switch os simulator for processing special cases and packet loss handling (and tcp servers for workers)
deleted_set_t *deleted_sets = NULL;

// server.workers for processing pkts
void kill(int signum);


size_t get_server_idx(index_key_t key) {
	size_t server_idx = key.keylo / per_server_range;
	if (server_idx >= server_num) {
		server_idx = server_num - 1;
	}
	return server_idx;
}

bool killed = false;
volatile bool running = false;
// TODO: consider evictserver, consnapshotserver, and reflector
std::atomic<size_t> ready_threads(0);


int main(int argc, char **argv) {
  parse_ini("config.ini");
  parse_control_ini("control_type.ini");
  //parse_args(argc, argv);
  //xindex::init_options(); // init options of rocksdb

  // prepare xindex
  std::vector<index_key_t> keys;
  std::vector<val_t> vals;
  load(keys, vals); // Use the last split workload to initialize the XIndex
  xindex_t *tab_xi = new xindex_t(keys, vals, server_num, bg_n); // server_num to create array of RCU status; bg_n background threads have been launched

  run_load_server(tab_xi); // loading phase

  // Prepare DPDK EAL param
  prepare_dpdk();

  // Prepare pkts and stats for receiver (based on ring buffer)
  prepare_receiver();

  // register signal handler
  signal(SIGTERM, SIG_IGN); // Ignore SIGTERM for subthreads

  run_server(tab_xi); // running phase
  if (tab_xi != nullptr) delete tab_xi; // terminate_bg -> bg_master joins bg_threads

  close_controller();

  // Free DPDK mbufs
  for (size_t i = 0; i < server_num; i++) {
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

void prepare_receiver() {
  // From receiver to each server
  //pkts = new volatile struct rte_mbuf*[server_num];
  //stats = new volatile bool[server_num];
  //memset((void *)pkts, 0, sizeof(struct rte_mbuf *)*server_num);
  //memset((void *)stats, 0, sizeof(bool)*server_num);
  //for (size_t i = 0; i < server_num; i++) {
  //  pkts[i] = rte_pktmbuf_alloc(mbuf_pool);
  //}
  pkts_list = new struct rte_mbuf**[server_num];
  heads = new uint32_t[server_num];
  tails = new uint32_t[server_num];
  memset((void*)heads, 0, sizeof(uint32_t)*server_num);
  memset((void*)tails, 0, sizeof(uint32_t)*server_num);
  //int res = 0;
  for (size_t i = 0; i < server_num; i++) {
	  pkts_list[i] = new struct rte_mbuf*[MQ_SIZE];
	  for (size_t j = 0; j < MQ_SIZE; j++) {
		  pkts_list[i][j] = nullptr;
	  }
	  //res = rte_pktmbuf_alloc_bulk(mbuf_pool, pkts_list[i], MQ_SIZE);
  }
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

	// Launch notified (processing explicit notification for server-side snapshot)
	pthread_t notified_thread;
	ret = pthread_create(&notified_thread, nullptr, run_notified, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: unable to create notified " << ret);
	}

	// Launch workers (processing normal packets)
	//pthread_t threads[server_num];
	server_worker_param_t server_worker_params[server_num];
	// check if parameters are cacheline aligned
	for (size_t i = 0; i < server_num; i++) {
		if ((uint64_t)(&(server_worker_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(server_worker_params[i]));
		}
	}
	for (uint8_t worker_i = 0; worker_i < server_num; worker_i++) {
		server_worker_params[worker_i].table = table;
		server_worker_params[worker_i].thread_id = worker_i;
		server_worker_params[worker_i].throughput = 0;
#ifdef TEST_AGG_THPT
		server_worker_params[worker_i].sum_latency = 0.0;
#endif
		//int ret = pthread_create(&threads[worker_i], nullptr, run_sfg, (void *)&server_worker_params[worker_i]);
		ret = rte_eal_remote_launch(run_server_worker, (void *)&server_worker_params[worker_i], lcoreid);
		if (ret) {
		  COUT_N_EXIT("Error:" << ret);
		}
		lcoreid++;
		if (lcoreid >= MAX_LCORE_NUM) {
			lcoreid = 1;
		}
	}

	COUT_THIS("[server] prepare server foreground threads...")
	while (ready_threads < server_num) sleep(1);

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
#ifdef TEST_AGG_THPT
	double max_agg_thpt = 0.0;
	for (size_t i = 0; i < server_num; i++) {
		max_agg_thpt += (double(server_worker_params[i].throughput) / server_worker_params[i].sum_latency * 1000 * 1000);
	}
	max_agg_thpt /= double(1024 * 1024);
	COUT_THIS("Max server-side aggregate throughput: " << max_agg_thpt << " MQPS");
#endif

	running = false;
	/*void *status;
	for (size_t i = 0; i < server_num; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error:unable to join," << rc);
		}
	}*/
	void * status;
	for (size_t i = 0; i < server_num; i++) {
		int rc = pthread_join(tcpserver_threads[i], &status);
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
				packet_type_t optype = packet_type_t(get_optype(received_pkts[i]));
				if (optype == packet_type_t::CACHE_POP_INSWITCH_ACK
						|| optype == packet_type_t::GETRES_LATEST_SEQ_INSWITCH_CASE1
						|| optype == packet_type_t::GETRES_DELETED_SEQ_INSWITCH_CASE1
						|| optype == packet_type_t::PUTREQ_SEQ_INSWITCH_CASE1
						|| optype == packet_type_t::DELREQ_SEQ_INSWITCH_CASE1) {
					if (((reflector_head_for_popack_snapshot + 1) % MQ_SIZE) != reflector_tail_for_popack_snapshot) {
						reflector_pkts_for_popack_snapshot[reflector_head_for_popack_snapshot] = received_pkts[i];
						reflector_head_for_popack_snapshot = (reflector_head_for_popack_snapshot + 1) % MQ_SIZE;
					}
				}
				else { // normal packets
					int idx = received_port - server_port_start;
					if (idx < 0 || unsigned(idx) >= server_num) {
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
	return 0;
}

void kill(int signum) {
	COUT_THIS("[server] Receive SIGKILL!")
	killed = true;
}
