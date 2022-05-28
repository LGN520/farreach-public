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
#include "latency_helper.h"
#include "socket_helper.h"

#define MAX_VERSION 0xFFFFFFFFFFFFFFFF

#include "common_impl.h"

/* variables */

// transaction phase
bool volatile transaction_running = false;
std::atomic<size_t> transaction_ready_threads(0);
size_t transaction_expected_ready_threads = 0;
bool volatile killed = false;

/* functions */

// transaction phase
#include "reflector_impl.h"
#include "server_impl.h"
void transaction_main(); // transaction phase
void kill(int signum);

int main(int argc, char **argv) {
  parse_ini("config.ini");
  parse_control_ini("control_type.ini");
  
  /* (1) prepare phase */

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

  // close_load_server();
  close_reflector();
  close_server();

  COUT_THIS("[ycsb_server.main] Exit successfully")
  exit(0);
}

/*
 * Transaction phase
 */

void transaction_main() {
	// reflector: popserver + worker
	//// server: server_num workers + receiver + evictserver + consnapshotserver
	// server: server_num workers + server_num popclients + evictserver + consnapshotserver
	transaction_expected_ready_threads = 2*server_num + 4;

	int ret = 0;

	transaction_running = false;

	// launch reflector.popserver
	pthread_t reflector_popserver_thread;
	ret = pthread_create(&reflector_popserver_thread, nullptr, run_reflector_popserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error of launching reflector.popserver: " << ret);
	}

	// launch reflector.worker
	pthread_t reflector_worker_thread;
	ret = pthread_create(&reflector_worker_thread, nullptr, run_reflector_worker, nullptr);
	if (ret) {
		COUT_N_EXIT("Error of launching reflector.worker: " << ret);
	}

	// launch popclients
	pthread_t popclient_threads[server_num];
	uint16_t popclient_params[server_num];
	for (uint16_t popclient_i = 0; popclient_i < server_num; popclient_i++) {
		popclient_params[popclient_i] = popclient_i;
		int ret = pthread_create(&popclient_threads[popclient_i], nullptr, run_server_popclient, &popclient_params[popclient_i]);
		if (ret) {
		  COUT_N_EXIT("Error of launching some server.popclient:" << ret);
		}
	}

	// launch workers (processing normal packets)
	pthread_t worker_threads[server_num];
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
		int ret = pthread_create(&worker_threads[worker_i], nullptr, run_server_worker, (void *)&server_worker_params[worker_i]);
		if (ret) {
		  COUT_N_EXIT("Error of launching some server.worker:" << ret);
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
	for (size_t i = 0; i < server_num; i++) {
		int rc = pthread_join(worker_threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error: unable to join " << rc);
		}
		rc = pthread_join(popclient_threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error: unable to join " << rc);
		}
	}
	int rc = pthread_join(evictserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join evictserver " << rc);
	}
	rc = pthread_join(consnapshotserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join consnapshotserver " << rc);
	}
	printf("[transaction.main] all threads end");
}

void kill(int signum) {
	COUT_THIS("[transaction phase] receive SIGKILL!")
	killed = true;
}
