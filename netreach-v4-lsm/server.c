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
	// server: server_num workers + server_num popclients + evictserver + consnapshotserver + reflector.worker + reflector.popserver
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

	// used under dynamic workload
	int totalsecs = dynamic_periodnum * dynamic_periodinterval;
	size_t persec_perserver_thpts[totalsecs][server_num];
	int dynamicserver_udpsock = -1;
	prepare_udpserver(dynamicserver_udpsock, false, server_dynamicserver_port, "server.dynamicserver");
	struct sockaddr_in dynamicclient_addr;
	socklen_t dynamicclient_addrlen = sizeof(struct sockaddr_in);
	bool with_dynamicclient_addr = false;

	transaction_running = true;
	COUT_THIS("[transaction.main] all threads ready");

	signal(SIGTERM, kill); // Set for main thread (kill -15)

	if (workload_mode == 0) { // wait for manual close in static mode
		while (!killed) {
			sleep(1);
		}
	}
	else { // wait for all periods in dynamic mode
		char buf[MAX_BUFSIZE];
		int recvsize = 0;
		size_t thpt_history[server_num];
		memset(thpt_history, 0, sizeof(size_t) * server_num);
		for (int secidx = 0; secidx < totalsecs; secidx++) {
			if (!with_dynamicclient_addr) {
				udprecvfrom(dynamicserver_udpsock, buf, MAX_BUFSIZE, 0, (struct sockaddr*)&dynamicclient_addr, &dynamicclient_addrlen, recvsize, "server.dynamicserver");
				with_dynamicclient_addr = true;
			}
			else {
				udprecvfrom(dynamicserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "server.dynamicserver");
			}
			INVARIANT(recvsize == sizeof(int));
			INVARIANT(*((int *)buf) == secidx);
			for (size_t i = 0; i < server_num; i++) {
				size_t tmp_thpt = server_worker_params[i].throughput; // accumulated thpt
				persec_perserver_thpts[secidx][i] = tmp_thpt - thpt_history[i]; // thpt of current second
				thpt_history[i] = tmp_thpt;
			}
			udpsendto(dynamicserver_udpsock, buf, recvsize, 0, (struct sockaddr*)&dynamicclient_addr, dynamicclient_addrlen, "server.dynamicserver");
		}
	}
	transaction_running = false;

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
	std::vector<int> worker_pktcnt_list;
	for (size_t i = 0; i < server_num; i++) {
		worker_latency_list.insert(worker_latency_list.end(), server_worker_params[i].latency_list.begin(), server_worker_params[i].latency_list.end());
		worker_pktcnt_list.push_back(server_worker_params[i].latency_list.size());
	}
	dump_latency(worker_latency_list, "worker_latency_list");
	printf("Per-server pktcnt: ");
	for (size_t i = 0; i < server_num; i++) {
		if (i != server_num - 1) {
			printf("%d ", worker_pktcnt_list[i]);
		}
		else {
			printf("%d\n", worker_pktcnt_list[i]);
		}
	}

	if (workload_mode != 0) {
		printf("\nper-sec per-server pktcnts:\n");
		for (int secidx = 0; secidx < totalsecs; secidx++) {
			for (size_t i = 0; i < server_num; i++) {
				if (i != server_num - 1) {
					printf("%d ", persec_perserver_thpts[secidx][i]);
				}
				else {
					printf("%d\n", persec_perserver_thpts[secidx][i]);
				}
			}
		}
	}

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
	printf("wait for server.consnapshotserver\n");
	rc = pthread_join(consnapshotserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join server.consnapshotserver " << rc);
	}
	printf("server.consnapshotserver finish\n");
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
