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
#include "latency_helper.h"

#define MAX_VERSION 0xFFFFFFFFFFFFFFFF

#include "common_impl.h"

/* variables */

// transaction phase
bool volatile transaction_running = false;
std::atomic<size_t> transaction_ready_threads(0);
size_t transaction_expected_ready_threads = 0;
bool volatile killed = false;

/* functions */

// prepare phase
void prepare_dpdk();

// transaction phase
#include "reflector_impl.h"
#include "server_impl.h"
void transaction_main(); // transaction phase
void kill(int signum);

int main(int argc, char **argv) {
  parse_ini("config.ini");
  parse_control_ini("control_type.ini");
  
  /* (1) prepare phase */

  // Prepare DPDK EAL param
  prepare_dpdk();

  // register signal handler
  signal(SIGTERM, SIG_IGN); // Ignore SIGTERM for subthreads

  /* (2) transaction phase */
  printf("[main] transaction phase start\n");

  // update transaction_expected_ready_threads
  transaction_expected_ready_threads = server_num + 3 + 2;

  prepare_reflector();
  prepare_server();
  transaction_main();

  /* (3) free phase */

  delete [] db_wrappers;
  db_wrappers = NULL;

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
  
  dpdk_eal_init(&dpdk_argc, &dpdk_argv); // Init DPDK
  dpdk_port_init(0, server_num+1, server_num+1); // tx:rx server_num server.workers + one reflector
}

/*
 * Transaction phase
 */

void transaction_main() {
	// reflector: popserver + dpdkserver
	//// server: server_num workers + receiver + evictserver + consnapshotserver
	// server: server_num workers + evictserver + consnapshotserver
	transaction_expected_ready_threads = server_num + 4;

	int ret = 0;
	//// 0: master lcore; 1: slave lcore for receiver; 2~: slave lcores for workers
	// 0: master lcore; 1~: slave lcores for workers
	unsigned lcoreid = 1;

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
	/*ret = rte_eal_remote_launch(run_receiver, NULL, lcoreid);
	if (ret) {
		COUT_N_EXIT("Error of launching server.receiver:" << ret);
	}
	//COUT_THIS("[transaction phase] Launch receiver with ret code " << ret);
	lcoreid++;*/

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
	ret = pthread_create(&consnapshotserver_thread, nullptr, run_server_consnapshotserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error of launching server.consnapshotserver: " << ret);
	}

	while (transaction_ready_threads < transaction_expected_ready_threads) sleep(1);

	for (uint16_t idx = 0; idx < server_num; idx++) {
		generate_udp_fdir_rule(0, idx, server_port_start+idx);
	}
	generate_udp_fdir_rule(0, server_num, reflector_port);
	dpdk_port_start(0);

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
	//dump_latency(receiver_latency_list, "receiver_latency_list");
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
