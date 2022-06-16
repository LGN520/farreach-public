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

// CPU affinity
#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>

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

cpu_set_t nonserverworker_cpuset; // [server_cores, total_cores-1] for all other threads

/* functions */

// transaction phase
#include "reflector_impl.h"
#include "server_impl.h"
void transaction_main(); // transaction phase
void kill(int signum);

int main(int argc, char **argv) {
  parse_ini("config.ini");
  parse_control_ini("control_type.ini");

  CPU_ZERO(&nonserverworker_cpuset);
  for (int i = server_cores; i < total_cores; i++) {
	CPU_SET(i, &nonserverworker_cpuset);
  }
  int result = sched_setaffinity(0, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
  if (result) {
	  printf("[Error] fail to set affinity of server.main; errno: %d\n", errno);
	  exit(-1);
  }
  
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
	// server: server_num * (workers + popclients + snapshotserver + snapshotdataserver) + evictserver
	transaction_expected_ready_threads = 2 + 4*server_num + 1;

	int ret = 0;

	transaction_running = false;

	cpu_set_t serverworker_cpuset; // [0, server_cores-1] for each server.worker

	// launch reflector.popserver
	pthread_t reflector_popserver_thread;
	ret = pthread_create(&reflector_popserver_thread, nullptr, run_reflector_popserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error of launching reflector.popserver: " << ret);
	}
	ret = pthread_setaffinity_np(reflector_popserver_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
	if (ret) {
		printf("Error of setaffinity for reflector.popserver; errno: %d\n", errno);
		exit(-1);
	}

	// launch reflector.worker
	pthread_t reflector_worker_thread;
	ret = pthread_create(&reflector_worker_thread, nullptr, run_reflector_worker, nullptr);
	if (ret) {
		COUT_N_EXIT("Error of launching reflector.worker: " << ret);
	}
	ret = pthread_setaffinity_np(reflector_worker_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
	if (ret) {
		printf("Error of setaffinity for reflector.worker; errno: %d\n", errno);
		exit(-1);
	}

	// launch popclients
	pthread_t popclient_threads[server_num];
	uint16_t popclient_params[server_num];
	for (uint16_t popclient_i = 0; popclient_i < server_num; popclient_i++) {
		popclient_params[popclient_i] = popclient_i;
		ret = pthread_create(&popclient_threads[popclient_i], nullptr, run_server_popclient, &popclient_params[popclient_i]);
		if (ret) {
		  COUT_N_EXIT("Error of launching some server.popclient:" << ret);
		}
		ret = pthread_setaffinity_np(popclient_threads[popclient_i], sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
		if (ret) {
			printf("Error of setaffinity for server.popclient; errno: %d\n", errno);
			exit(-1);
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
		server_worker_params[worker_i].process_latency_list.clear();
		server_worker_params[worker_i].wait_latency_list.clear();
		ret = pthread_create(&worker_threads[worker_i], nullptr, run_server_worker, (void *)&server_worker_params[worker_i]);
		if (ret) {
		  COUT_N_EXIT("Error of launching some server.worker:" << ret);
		}

		CPU_ZERO(&serverworker_cpuset);
		CPU_SET(worker_i % server_cores, &serverworker_cpuset);
		ret = pthread_setaffinity_np(worker_threads[worker_i], sizeof(serverworker_cpuset), &serverworker_cpuset);
		if (ret) {
			printf("Error of setaffinity for server.worker; errno: %d\n", errno);
			exit(-1);
		}
	}
	//COUT_THIS("[tranasaction phase] prepare server worker threads...")

	// launch evictserver
	pthread_t evictserver_thread;
	ret = pthread_create(&evictserver_thread, nullptr, run_server_evictserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error of launching server.evictserver: " << ret);
	}
	ret = pthread_setaffinity_np(evictserver_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
	if (ret) {
		printf("Error of setaffinity for server.evictserver; errno: %d\n", errno);
		exit(-1);
	}

	// launch snapshotservers
	uint16_t snapshotserver_params[server_num];
	pthread_t snapshotserver_threads[server_num];
	for (uint16_t snapshotserver_i = 0; snapshotserver_i < server_num; snapshotserver_i++) {
		snapshotserver_params[snapshotserver_i] = snapshotserver_i;
		ret = pthread_create(&snapshotserver_threads[snapshotserver_i], nullptr, run_server_snapshotserver, &snapshotserver_params[snapshotserver_i]);
		if (ret) {
		  COUT_N_EXIT("Error of launching some server.snapshotserver:" << ret);
		}
		ret = pthread_setaffinity_np(snapshotserver_threads[snapshotserver_i], sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
		if (ret) {
			printf("Error of setaffinity for server.snapshotserver; errno: %d\n", errno);
			exit(-1);
		}
	}

	// launch snapshotdataservers
	uint16_t snapshotdataserver_params[server_num];
	pthread_t snapshotdataserver_threads[server_num];
	for (uint16_t snapshotdataserver_i = 0; snapshotdataserver_i < server_num; snapshotdataserver_i++) {
		snapshotdataserver_params[snapshotdataserver_i] = snapshotdataserver_i;
		ret = pthread_create(&snapshotdataserver_threads[snapshotdataserver_i], nullptr, run_server_snapshotdataserver, &snapshotdataserver_params[snapshotdataserver_i]);
		if (ret) {
		  COUT_N_EXIT("Error of launching some server.snapshotdataserver:" << ret);
		}
		ret = pthread_setaffinity_np(snapshotdataserver_threads[snapshotdataserver_i], sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
		if (ret) {
			printf("Error of setaffinity for server.snapshotdataserver; errno: %d\n", errno);
			exit(-1);
		}
	}

	while (transaction_ready_threads < transaction_expected_ready_threads) sleep(1);

	transaction_running = true;
	COUT_THIS("[transaction.main] all threads ready");

	signal(SIGTERM, kill); // Set for main thread (kill -15)

	while (!killed) {
		sleep(1);
	}
	transaction_running = false;

	/* Processing Statistics */

	// dump per-server load ratio
	size_t overall_pktcnt = 0;
	for (size_t i = 0; i < server_num; i++) {
		overall_pktcnt += server_worker_params[i].process_latency_list.size();
	}
	COUT_THIS("Server-side overall pktcnt: " << overall_pktcnt);
	double avg_per_server_thpt = double(overall_pktcnt) / double(server_num);
	for (size_t i = 0; i < server_num; i++) {
		double tmp_load_balance_ratio = double(server_worker_params[i].process_latency_list.size()) / avg_per_server_thpt;
		COUT_THIS("Load balance ratio of server " << i << ": " << tmp_load_balance_ratio);
	}

	// dump process latency
	printf("\nprocess latency:\n");
	/*std::vector<double> worker_process_latency_list;
	for (size_t i = 0; i < server_num; i++) {
		printf("[server %d]\n", i);
		std::string tmp_label;
		GET_STRING(tmp_label, "worker_process_latency_list " << i);
		dump_latency(server_worker_params[i].process_latency_list, tmp_label);

		worker_process_latency_list.insert(worker_process_latency_list.end(), server_worker_params[i].process_latency_list.begin(), server_worker_params[i].process_latency_list.end());
	}
	printf("[overall]\n");
	dump_latency(worker_process_latency_list, "worker_process_latency_list overall");*/
	std::vector<double> worker_avg_process_latency_list(server_num);
	for (size_t i = 0; i < server_num; i++) {
		double tmp_avg_process_latency = 0.0;
		for (size_t j = 0; j < server_worker_params[i].process_latency_list.size(); j++) {
			tmp_avg_process_latency += server_worker_params[i].process_latency_list[j];
		}
		tmp_avg_process_latency /= server_worker_params[i].process_latency_list.size();
		worker_avg_process_latency_list[i] = tmp_avg_process_latency;
	}
	dump_latency(worker_avg_process_latency_list, "worker_avg_process_latency_list");

	// dump wait latency
	printf("\nwait latency:\n");
	/*std::vector<double> worker_wait_latency_list;
	for (size_t i = 0; i < server_num; i++) {
		printf("[server %d]\n", i);
		std::string tmp_label;
		GET_STRING(tmp_label, "worker_wait_latency_list " << i);
		dump_latency(server_worker_params[i].wait_latency_list, tmp_label);

		worker_wait_latency_list.insert(worker_wait_latency_list.end(), server_worker_params[i].wait_latency_list.begin(), server_worker_params[i].wait_latency_list.end());
	}
	printf("[overall]\n");
	dump_latency(worker_wait_latency_list, "worker_wait_latency_list overall");*/
	std::vector<double> worker_avg_wait_latency_list(server_num);
	for (size_t i = 0; i < server_num; i++) {
		double tmp_avg_wait_latency = 0.0;
		for (size_t j = 0; j < server_worker_params[i].wait_latency_list.size(); j++) {
			tmp_avg_wait_latency += server_worker_params[i].wait_latency_list[j];
		}
		tmp_avg_wait_latency /= server_worker_params[i].wait_latency_list.size();
		worker_avg_wait_latency_list[i] = tmp_avg_wait_latency;
	}
	dump_latency(worker_avg_wait_latency_list, "worker_avg_wait_latency_list");

	void *status;
	printf("wait for server.workers\n");
	for (size_t i = 0; i < server_num; i++) {
		int rc = pthread_join(worker_threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error: unable to join server.worker " << rc);
		}
	}
	printf("server.workers finish\n");
	printf("wait for server.popclients\n");
	for (size_t i = 0; i < server_num; i++) {
		int rc = pthread_join(popclient_threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error: unable to join server.popclient " << rc);
		}
	}
	printf("server.popclients finish\n");
	printf("wait for sever.evictserver\n");
	int rc = pthread_join(evictserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join server.evictserver " << rc);
	}
	printf("server.evictserver finish\n");
	printf("wait for server.snapshotservers\n");
	for (size_t i = 0; i < server_num; i++) {
		rc = pthread_join(snapshotserver_threads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join server.snapshotserver " << rc);
		}
	}
	printf("server.snapshotservers finish\n");
	printf("wait for server.snapshotdataservers\n");
	for (size_t i = 0; i < server_num; i++) {
		rc = pthread_join(snapshotdataserver_threads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join server.snapshotdataserver " << rc);
		}
	}
	printf("server.snapshotdataservers finish\n");
	printf("wait for reflector.popserver\n");
	rc = pthread_join(reflector_popserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join reflector.popserver " << rc);
	}
	printf("reflector.popserver finish\n");
	printf("wait for reflector.worker\n");
	rc = pthread_join(reflector_worker_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join reflector.worker " << rc);
	}
	printf("reflector.worker finish\n");

	printf("[transaction.main] all threads end\n");
}

void kill(int signum) {
	COUT_THIS("[transaction phase] receive SIGKILL!")
	killed = true;
}
