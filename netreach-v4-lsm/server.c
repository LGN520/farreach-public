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

int transaction_loadfinishserver_udpsock = -1;
int server_physical_idx = -1;

cpu_set_t nonserverworker_cpuset; // [server_cores, total_cores-1] for all other threads

/* functions */

// transaction phase
#include "reflector_impl.h"
#include "server_impl.h"
void transaction_main(); // transaction phase
void *run_transaction_loadfinishserver(void *param);
void kill(int signum);

int main(int argc, char **argv) {
  parse_ini("config.ini");
  parse_control_ini("control_type.ini");

  if (argc != 2) {
	printf("Usage: ./server server_physical_idx\n");
	exit(-1);
  }
  server_physical_idx = atoi(argv[1]);
  INVARIANT(server_physical_idx >= 0);
  INVARIANT(server_physical_idx < server_physical_num);

  CPU_ZERO(&nonserverworker_cpuset);
  for (int i = server_worker_corenums[server_physical_idx]; i < server_total_corenums[server_physical_idx]; i++) {
	CPU_SET(i, &nonserverworker_cpuset);
  }
  //int ret = sched_setaffinity(0, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
  pthread_t main_thread = pthread_self();
  int ret = pthread_setaffinity_np(main_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
  if (ret) {
	  printf("[Error] fail to set affinity of server.main; errno: %d\n", errno);
	  exit(-1);
  }
  
  /* (1) prepare phase */

  // register signal handler
  signal(SIGTERM, SIG_IGN); // Ignore SIGTERM for subthreads

  /* (2) transaction phase */
  printf("[main] transaction phase start\n");

  prepare_reflector();
  prepare_server();
  transaction_main();

  /* (3) free phase */

  free_common();
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
	uint32_t current_server_logical_num = server_logical_idxes_list[server_physical_idx].size();

	// update transaction_expected_ready_threads
	// reflector: popserver + worker
	// server: server_num * (worker + evictserver + snapshotserver + snapshotdataserver)
	// transaction.main: loadfinishserver
	if (server_physical_idx == 0) { // deploy reflector in the first physical server
		transaction_expected_ready_threads = 2 + 4*current_server_logical_num + 1;
	}
	else {
		transaction_expected_ready_threads = 4*current_server_logical_num + 1;
	}

	int ret = 0;

	transaction_running = false;

	cpu_set_t serverworker_cpuset; // [0, server_cores-1] for each server.worker

	pthread_t reflector_cp2dpserver_thread;
	pthread_t reflector_dp2cpserver_thread;
	if (server_physical_idx == 0) {
		// launch reflector.cp2dpserver
		ret = pthread_create(&reflector_cp2dpserver_thread, nullptr, run_reflector_cp2dpserver, nullptr);
		if (ret) {
			COUT_N_EXIT("Error of launching reflector.cp2dpserver: " << ret);
		}
		ret = pthread_setaffinity_np(reflector_cp2dpserver_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
		if (ret) {
			printf("Error of setaffinity for reflector.cp2dpserver; errno: %d\n", errno);
			exit(-1);
		}

		// launch reflector.dp2cpserver
		ret = pthread_create(&reflector_dp2cpserver_thread, nullptr, run_reflector_dp2cpserver, nullptr);
		if (ret) {
			COUT_N_EXIT("Error of launching reflector.dp2cpserver: " << ret);
		}
		ret = pthread_setaffinity_np(reflector_dp2cpserver_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
		if (ret) {
			printf("Error of setaffinity for reflector.dp2cpserver; errno: %d\n", errno);
			exit(-1);
		}
	}

	// launch popclients
	/*pthread_t popclient_threads[server_num];
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
	}*/

	// launch workers (processing normal packets)
	pthread_t worker_threads[current_server_logical_num];
	server_worker_param_t server_worker_params[current_server_logical_num];
	// check if parameters are cacheline aligned
	for (size_t i = 0; i < current_server_logical_num; i++) {
		if ((uint64_t)(&(server_worker_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(server_worker_params[i]));
		}
	}
	for (uint16_t worker_i = 0; worker_i < current_server_logical_num; worker_i++) {
		server_worker_params[worker_i].local_server_logical_idx = worker_i;
		server_worker_params[worker_i].process_latency_list.clear();
		server_worker_params[worker_i].wait_latency_list.clear();
		server_worker_params[worker_i].rocksdb_latency_list.clear();
		ret = pthread_create(&worker_threads[worker_i], nullptr, run_server_worker, (void *)&server_worker_params[worker_i]);
		if (ret) {
		  COUT_N_EXIT("Error of launching some server.worker:" << ret);
		}

		CPU_ZERO(&serverworker_cpuset);
		CPU_SET(worker_i % server_worker_corenums[server_physical_idx], &serverworker_cpuset);
		ret = pthread_setaffinity_np(worker_threads[worker_i], sizeof(serverworker_cpuset), &serverworker_cpuset);
		if (ret) {
			printf("Error of setaffinity for server.worker; errno: %d\n", errno);
			exit(-1);
		}
	}
	//COUT_THIS("[tranasaction phase] prepare server worker threads...")

	// launch evictserver
	uint16_t evictserver_params[current_server_logical_num];
	pthread_t evictserver_threads[current_server_logical_num];
	for (uint16_t evictserver_i = 0; evictserver_i < current_server_logical_num; evictserver_i++) {
		evictserver_params[evictserver_i] = evictserver_i;
		ret = pthread_create(&evictserver_threads[evictserver_i], nullptr, run_server_evictserver, &evictserver_params[evictserver_i]);
		if (ret) {
			COUT_N_EXIT("Error of launching server.evictserver: " << ret);
		}
		ret = pthread_setaffinity_np(evictserver_threads[evictserver_i], sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
		if (ret) {
			printf("Error of setaffinity for server.evictserver; errno: %d\n", errno);
			exit(-1);
		}
	}

	// launch snapshotservers
	uint16_t snapshotserver_params[current_server_logical_num];
	pthread_t snapshotserver_threads[current_server_logical_num];
	for (uint16_t snapshotserver_i = 0; snapshotserver_i < current_server_logical_num; snapshotserver_i++) {
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
	uint16_t snapshotdataserver_params[current_server_logical_num];
	pthread_t snapshotdataserver_threads[current_server_logical_num];
	for (uint16_t snapshotdataserver_i = 0; snapshotdataserver_i < current_server_logical_num; snapshotdataserver_i++) {
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

	// launch loadfinishserver
	prepare_udpserver(transaction_loadfinishserver_udpsock, true, transaction_loadfinishserver_port, "transaction.main.loadfinishserver");
	pthread_t loadfinishserver_thread;
	ret = pthread_create(&loadfinishserver_thread, nullptr, run_transaction_loadfinishserver, nullptr);
	if (ret) {
	  COUT_N_EXIT("Error of launching transaction.main.loadfinishserver: " << ret);
	}

	while (transaction_ready_threads < transaction_expected_ready_threads) sleep(1);

	// IMPORTANT: avoid CPU contention between server.workers and rocksdb's background threads
	printf("Reset CPU affinity of rocksdb's background threads\n");
	char command[256];
	sprintf(command, "./reset_rocksdb_affinity.sh %d %d %d ", server_worker_corenums[server_physical_idx], server_total_corenums[server_physical_idx], current_server_logical_num);
	for (size_t i = 0; i < current_server_logical_num; i++) {
		if (i != current_server_logical_num - 1) {
			sprintf(command + strlen(command), "%d ", server_worker_lwpid_list[i]);
		}
		else {
			sprintf(command + strlen(command), "%d", server_worker_lwpid_list[i]);
		}
	}
	printf("Execute %s\n", command);
	system(command);

	transaction_running = true;
	COUT_THIS("[transaction.main] all threads ready");

	signal(SIGTERM, kill); // Set for main thread (kill -15)

	std::vector<std::vector<int>> persec_perserver_aggpktcnt;
	while (!killed) {
		sleep(1);
		std::vector<int> cursec_perserver_aggpktcnt(current_server_logical_num);
		for (size_t i = 0; i < current_server_logical_num; i++) {
			cursec_perserver_aggpktcnt[i] = server_worker_params[i].process_latency_list.size();
		}
		persec_perserver_aggpktcnt.push_back(cursec_perserver_aggpktcnt);
	}
	transaction_running = false;

	/* Processing Statistics */

#ifdef DEBUG_PERSEC
	// dump per-sec statistics
	int seccnt = persec_perserver_aggpktcnt.size();
	std::vector<int> cursec_perserver_pktcnt = persec_perserver_aggpktcnt[0];
	for (size_t i = 0; i < seccnt; i++) {
		if (i != 0) {
			for (size_t j = 0; j < current_server_logical_num; j++) {
				cursec_perserver_pktcnt[j] = persec_perserver_aggpktcnt[i][j] - persec_perserver_aggpktcnt[i-1][j];
			}
		}

		printf("[sec %d]\n", i);

		int cursec_total_pktcnt = 0;
		for (size_t j = 0; j < current_server_logical_num; j++) {
			cursec_total_pktcnt += cursec_perserver_pktcnt[j];
		}
		printf("per-server load ratio: %d-", cursec_total_pktcnt);
		for (size_t j = 0; j < current_server_logical_num; j++) {
			printf("%f ", cursec_perserver_pktcnt[j]/double(cursec_total_pktcnt));
		}
		printf("\n");

		std::vector<double> cursec_wait_latency_list;
		std::vector<double> cursec_process_latency_list;
		std::vector<double> cursec_rocksdb_latency_list;
		for (size_t j = 0; j < current_server_logical_num; j++) {
			int startidx = 0;
			if (i != 0) startidx = persec_perserver_aggpktcnt[i-1][j];
			int endidx = persec_perserver_aggpktcnt[i][j];
			std::vector<double> cursec_curserver_wait_latency_list(server_worker_params[j].wait_latency_list.begin() + startidx, server_worker_params[j].wait_latency_list.begin() + endidx);
			cursec_wait_latency_list.insert(cursec_wait_latency_list.end(), cursec_curserver_wait_latency_list.begin(), cursec_curserver_wait_latency_list.end());
			std::vector<double> cursec_curserver_process_latency_list(server_worker_params[j].process_latency_list.begin() + startidx, server_worker_params[j].process_latency_list.begin() + endidx);
			cursec_process_latency_list.insert(cursec_process_latency_list.end(), cursec_curserver_process_latency_list.begin(), cursec_curserver_process_latency_list.end());
			std::vector<double> cursec_curserver_rocksdb_latency_list(server_worker_params[j].rocksdb_latency_list.begin() + startidx, server_worker_params[j].rocksdb_latency_list.begin() + endidx);
			cursec_rocksdb_latency_list.insert(cursec_rocksdb_latency_list.end(), cursec_curserver_rocksdb_latency_list.begin(), cursec_curserver_rocksdb_latency_list.end());

			std::string tmplabel;
			GET_STRING(tmplabel, "wait_latency_list server "<<j);
			dump_latency(cursec_curserver_wait_latency_list, tmplabel);
			GET_STRING(tmplabel, "process_latency_list server "<<j);
			dump_latency(cursec_curserver_process_latency_list, tmplabel);
			GET_STRING(tmplabel, "rocksdb_latency_list server "<<j);
			dump_latency(cursec_curserver_rocksdb_latency_list, tmplabel);
		}
		dump_latency(cursec_wait_latency_list, "wait_latency_list overall");
		dump_latency(cursec_process_latency_list, "process_latency_list overall");
		dump_latency(cursec_rocksdb_latency_list, "rocksdb_latency_list overall");
		printf("\n");
	}
	printf("\n");
#endif

	// dump per-server load ratio
	size_t overall_pktcnt = 0;
	for (size_t i = 0; i < current_server_logical_num; i++) {
		overall_pktcnt += server_worker_params[i].process_latency_list.size();
	}
	COUT_THIS("Server-side overall pktcnt: " << overall_pktcnt);
	double avg_per_server_thpt = double(overall_pktcnt) / double(current_server_logical_num);
	for (size_t i = 0; i < current_server_logical_num; i++) {
		double tmp_load_balance_ratio = double(server_worker_params[i].process_latency_list.size()) / avg_per_server_thpt;
		COUT_THIS("Load balance ratio of server " << i << ": " << tmp_load_balance_ratio);
	}

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
	std::vector<double> worker_avg_wait_latency_list(current_server_logical_num);
	for (size_t i = 0; i < current_server_logical_num; i++) {
		double tmp_avg_wait_latency = 0.0;
		for (size_t j = 0; j < server_worker_params[i].wait_latency_list.size(); j++) {
			tmp_avg_wait_latency += server_worker_params[i].wait_latency_list[j];
		}
		tmp_avg_wait_latency /= server_worker_params[i].wait_latency_list.size();
		worker_avg_wait_latency_list[i] = tmp_avg_wait_latency;
	}
	dump_latency(worker_avg_wait_latency_list, "worker_avg_wait_latency_list");

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
	std::vector<double> worker_avg_process_latency_list(current_server_logical_num);
	for (size_t i = 0; i < current_server_logical_num; i++) {
		double tmp_avg_process_latency = 0.0;
		for (size_t j = 0; j < server_worker_params[i].process_latency_list.size(); j++) {
			tmp_avg_process_latency += server_worker_params[i].process_latency_list[j];
		}
		tmp_avg_process_latency /= server_worker_params[i].process_latency_list.size();
		worker_avg_process_latency_list[i] = tmp_avg_process_latency;
	}
	dump_latency(worker_avg_process_latency_list, "worker_avg_process_latency_list");

	// dump rocksdb latency
	printf("\nrocksdb latency:\n");
	std::vector<double> worker_avg_rocksdb_latency_list(current_server_logical_num);
	for (size_t i = 0; i < current_server_logical_num; i++) {
		double tmp_avg_rocksdb_latency = 0.0;
		for (size_t j = 0; j < server_worker_params[i].rocksdb_latency_list.size(); j++) {
			tmp_avg_rocksdb_latency += server_worker_params[i].rocksdb_latency_list[j];
		}
		tmp_avg_rocksdb_latency /= server_worker_params[i].rocksdb_latency_list.size();
		worker_avg_rocksdb_latency_list[i] = tmp_avg_rocksdb_latency;
	}
	dump_latency(worker_avg_rocksdb_latency_list, "worker_avg_rocksdb_latency_list");

	void *status;
	printf("wait for server.workers\n");
	for (size_t i = 0; i < current_server_logical_num; i++) {
		int rc = pthread_join(worker_threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error: unable to join server.worker " << rc);
		}
	}
	printf("server.workers finish\n");

	/*printf("wait for server.popclients\n");
	for (size_t i = 0; i < server_num; i++) {
		int rc = pthread_join(popclient_threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error: unable to join server.popclient " << rc);
		}
	}
	printf("server.popclients finish\n");*/

	printf("wait for server.evictserver\n");
	for (size_t i = 0; i < current_server_logical_num; i++) {
		int rc = pthread_join(evictserver_threads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join server.evictserver " << rc);
		}
	}
	printf("server.evictserver finish\n");

	printf("wait for server.snapshotservers\n");
	for (size_t i = 0; i < current_server_logical_num; i++) {
		int rc = pthread_join(snapshotserver_threads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join server.snapshotserver " << rc);
		}
	}
	printf("server.snapshotservers finish\n");

	printf("wait for server.snapshotdataservers\n");
	for (size_t i = 0; i < current_server_logical_num; i++) {
		int rc = pthread_join(snapshotdataserver_threads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join server.snapshotdataserver " << rc);
		}
	}
	printf("server.snapshotdataservers finish\n");

	if (server_physical_idx == 0) {
		printf("wait for reflector.cp2dpserver\n");
		int rc = pthread_join(reflector_cp2dpserver_thread, &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join reflector.cp2dpserver " << rc);
		}
		printf("reflector.cp2dpserver finish\n");

		printf("wait for reflector.dp2cpserver\n");
		rc = pthread_join(reflector_dp2cpserver_thread, &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join reflector.dp2cpserver " << rc);
		}
		printf("reflector.dp2cpserver finish\n");
	}

	printf("wait for transaction.main.loadfinishserver\n");
	int rc = pthread_join(loadfinishserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join transaction.main.loadfinishserver " << rc);
	}
	printf("transaction.main.loadfinishserver finish\n");

	printf("[transaction.main] all threads end\n");
}

void *run_transaction_loadfinishserver(void *param) {
	uint32_t current_server_logical_num = server_logical_idxes_list[server_physical_idx].size();

	printf("[transaction.main.loadfinishserver] ready\n");
	transaction_ready_threads++;

	while(!transaction_running) {}

	char buf[256];
	int recvsize = 0;
	while (transaction_running) {
		bool is_timeout = udprecvfrom(transaction_loadfinishserver_udpsock, buf, 256, 0, NULL, NULL, recvsize, "transaction.main.loadfinishserver");
		if (is_timeout) {
			continue;
		}

		printf("Receive loadfinish notification!\n");
		for (size_t tmp_local_server_logical_idx = 0; tmp_local_server_logical_idx < current_server_logical_num; tmp_local_server_logical_idx++) {
			// make server-side snapshot of snapshot id 0 if necessary
			db_wrappers[tmp_local_server_logical_idx].init_snapshot();
		}
		break;
	}

	printf("[transaction.main.loadfinishserver] exit\n");
	close(transaction_loadfinishserver_udpsock);
	pthread_exit(nullptr);
}

void kill(int signum) {
	COUT_THIS("[transaction phase] receive SIGKILL!")
	killed = true;
}
