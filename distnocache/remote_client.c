#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <atomic>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <fstream>
#include <fcntl.h>
#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // inetaddr conversion
#include <unistd.h>
#include <set>
//#include <sys/time.h> // struct timeval

#include "helper.h"
#include "key.h"
#include "val.h"
#include "iniparser/iniparser_wrapper.h"
#include "crc32.h"
#include "latency_helper.h"
#include "socket_helper.h"
#include "dynamic_rulemap.h"
#include "dynamic_array.h"
#include "io_helper.h"

#ifdef USE_YCSB
#include "workloadparser/ycsb_parser.h"
#elif defined USE_SYNTHETIC
#include "workloadparser/synthetic_parser.h"
#endif

#include "common_impl.h"

//#define DUMP_BUF

struct alignas(CACHELINE_SIZE) ClientWorkerParam {
	uint16_t local_client_logical_idx;
	std::map<uint16_t, int> nodeidx_pktcnt_map;
	std::vector<double> process_latency_list;
	std::vector<double> send_latency_list; // time of sending packet
	int unmatched_cnt;
	int timeout_cnt;
	std::vector<double> wait_latency_list; // time between receiving response and sending next request
};
typedef ClientWorkerParam client_worker_param_t;

// used for dynamic workload
dynamic_rulemap_t * dynamic_rulemap_ptr = NULL;

//bool volatile stop_for_dynamic_control = false; // not need this as collecting persec statistics does not affect result much (use limited time)

volatile bool can_sendpkt = false;
volatile bool running = false;
std::atomic<size_t> ready_threads(0);
std::atomic<size_t> finish_threads(0);

int *client_udpsock_list = NULL;
int client_physical_idx = -1;

#ifdef SERVER_ROTATION
std::vector<uint16_t> valid_global_server_logical_idxes;
#endif

const uint32_t range_gap = 1024; // add 2^10 to keylo of startkey
//const int range_num = 10; // max number of returned kv pairs
netreach_key_t generate_endkey(netreach_key_t &startkey) {
	netreach_key_t endkey = startkey;
	if (std::numeric_limits<uint32_t>::max() - endkey.keylolo > range_gap) {
		endkey.keylolo += range_gap;
	}
	else {
		endkey.keylolo = std::numeric_limits<uint32_t>::max();
	}
	return endkey;
}

void run_benchmark();
void * run_client_worker(void *param); // sender
void * run_client_sendpktserver(void *param); // sendpktserver (all non-first physical clients wait for the first physical client to start to send packets)
void * run_client_rulemapserver(void *param); // rulemapserver (all non-first physical clients switch rulemape to simulate key popularity change after receiving command from the first physical client under dynamic workload)

//cpu_set_t nonclientworker_cpuset; // TMPPERFDEBUG

int main(int argc, char **argv) {
	parse_ini("config.ini");

	// TMPPERFDEBUG
	/*CPU_ZERO(&nonclientworker_cpuset);
	for (int i = 32; i < 48; i++) {
		CPU_SET(i, &nonclientworker_cpuset);
	}
	//int ret = sched_setaffinity(0, sizeof(nonclientworker_cpuset), &nonclientworker_cpuset);
	pthread_t main_thread = pthread_self();
	int ret = pthread_setaffinity_np(main_thread, sizeof(nonclientworker_cpuset), &nonclientworker_cpuset);
	if (ret) {
		printf("[Error] fail to set affinity of client.main; errno: %d\n", errno);
		exit(-1);
	}*/

#ifdef SERVER_ROTATION
	INVARIANT(workload_mode == 0);
	for (int tmp_server_physical_idx = 0; tmp_server_physical_idx < server_physical_num; tmp_server_physical_idx++) {
		for (int i = 0; i < server_logical_idxes_list[tmp_server_physical_idx].size(); i++) {
			valid_global_server_logical_idxes.push_back(server_logical_idxes_list[tmp_server_physical_idx][i]);
		}
	}
	INVARIANT(valid_global_server_logical_idxes.size() == 1 || valid_global_server_logical_idxes.size() == 2);
#endif

	if (argc != 2) {
		printf("Usage: ./remote_client client_physical_idx\n");
		exit(-1);
	}
	client_physical_idx = atoi(argv[1]);
	INVARIANT(client_physical_idx >= 0);
	INVARIANT(client_physical_idx < client_physical_num);

	// prepare for clients
	if (workload_mode != 0) {
		dynamic_rulemap_ptr = new dynamic_rulemap_t(dynamic_periodnum, dynamic_ruleprefix);
		INVARIANT(dynamic_rulemap_ptr != NULL);
		dynamic_rulemap_ptr->nextperiod();
	}

	// prepare for client.worker
	uint32_t current_client_logical_num = client_logical_nums[client_physical_idx];
	client_udpsock_list = new int[current_client_logical_num];
	INVARIANT(client_udpsock_list != NULL);
	for (size_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
		// NOTE: udp recv buffer size cannot be too larger under > 1024 server threads in one phsyical server
		if (current_client_logical_num <= 1024) { // avoid linux kernel overflow
			create_udpsock(client_udpsock_list[local_client_logical_idx], true, "remote_client", CLIENT_SOCKET_TIMEOUT_SECS, 0, UDP_LARGE_RCVBUFSIZE); // enable timeout for client-side retry if pktloss
		}
		else {
			create_udpsock(client_udpsock_list[local_client_logical_idx], true, "remote_client", CLIENT_SOCKET_TIMEOUT_SECS, 0, UDP_DEFAULT_RCVBUFSIZE); // enable timeout for client-side retry if pktloss
		}
		// bind client.worker udp socket into a specific port to avoid from overlapping with reserved ports for server.workers
		sockaddr_in tmp_client_worker_addr;
		set_sockaddr(tmp_client_worker_addr, htonl(INADDR_ANY), client_worker_port_start + local_client_logical_idx);
		bind(client_udpsock_list[local_client_logical_idx], (struct sockaddr*)&tmp_client_worker_addr, sizeof(tmp_client_worker_addr));
	}

	run_benchmark();

	if (dynamic_rulemap_ptr != NULL) {
		delete dynamic_rulemap_ptr;
		dynamic_rulemap_ptr = NULL;
	}

	exit(0);
}

void run_benchmark() {
	int ret = 0;

	running = false;

	/* (1) launch all client worker threads and sendpktserver thread if any */

	// Prepare client worker params
	uint32_t current_client_logical_num = client_logical_nums[client_physical_idx];
	pthread_t threads[current_client_logical_num];
	client_worker_param_t client_worker_params[current_client_logical_num];
	// check if parameters are cacheline aligned
	for (size_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
		if ((uint64_t)(&(client_worker_params[local_client_logical_idx])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(client_worker_params[local_client_logical_idx]));
		}
	}

	// Launch workers
	//cpu_set_t clientworker_cpuset; // TMPPERFDEBUG
	for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
		client_worker_params[local_client_logical_idx].local_client_logical_idx = local_client_logical_idx;
		client_worker_params[local_client_logical_idx].nodeidx_pktcnt_map.clear();
		client_worker_params[local_client_logical_idx].process_latency_list.reserve(10 * 1024 * 1024);
		client_worker_params[local_client_logical_idx].send_latency_list.reserve(10 * 1024 * 1024);
		client_worker_params[local_client_logical_idx].unmatched_cnt = 0;
		client_worker_params[local_client_logical_idx].timeout_cnt = 0;
		client_worker_params[local_client_logical_idx].wait_latency_list.reserve(10 * 1024 * 1024);
		ret = pthread_create(&threads[local_client_logical_idx], nullptr, run_client_worker, (void *)&client_worker_params[local_client_logical_idx]);
		UNUSED(ret);
		//COUT_THIS("[client] Lanuch client " << local_client_logical_idx)
		
		// TMPPERFDEBUG
		/*CPU_ZERO(&clientworker_cpuset);
		CPU_SET(local_client_logical_idx % 32, &clientworker_cpuset);
		int ret = pthread_setaffinity_np(threads[local_client_logical_idx], sizeof(clientworker_cpuset), &clientworker_cpuset);
		if (ret) {
			printf("[Error] fail to set affinity of client.worker; errno: %d\n", errno);
			exit(-1);
		}*/
	}

	// launch sendpktserver for non-first physical client
	pthread_t sendpktserver_thread;
	if (client_physical_idx != 0) {
		ret = pthread_create(&sendpktserver_thread, nullptr, run_client_sendpktserver, nullptr);
		INVARIANT(ret == 0);

		// TMPPERFDEBUG
		/*int ret = pthread_setaffinity_np(sendpktserver_thread, sizeof(nonclientworker_cpuset), &nonclientworker_cpuset);
		if (ret) {
			printf("[Error] fail to set affinity of client.sendpktserver; errno: %d\n", errno);
			exit(-1);
		}*/
	}
	
	// launch rulemapserver for non-first physical client under dynamic workload
	pthread_t rulemapserver_thread;
	if (client_physical_idx != 0 && workload_mode != 0) {
		ret = pthread_create(&rulemapserver_thread, nullptr, run_client_rulemapserver, nullptr);
		INVARIANT(ret == 0);

		// TMPPERFDEBUG
		/*int ret = pthread_setaffinity_np(rulemapserver_thread, sizeof(nonclientworker_cpuset), &nonclientworker_cpuset);
		if (ret) {
			printf("[Error] fail to set affinity of client.rulemapserver; errno: %d\n", errno);
			exit(-1);
		}*/
	}

	COUT_THIS("[client] prepare clients ...");
	// NOTE: current_client_logical_num * client.worker + [client.sendpktserver] + [client.rulemapserver]
	if (client_physical_idx != 0) {
		if (workload_mode == 0) {
			while (ready_threads < current_client_logical_num + 1) sleep(1);
		}
		else {
			while (ready_threads < current_client_logical_num + 2) sleep(1);
		}
	}
	else {
		while (ready_threads < current_client_logical_num) sleep(1);
	}
	COUT_THIS("[client] all clients ready...");

	/* (2) the first physical client is responsible for starting packet send of all physical clients */

	// NOTE: by now we get aggregate pktcnts from the first sec until the ith sec
	std::vector<std::vector<int>> persec_perclient_aggpktcnts;
	std::vector<std::vector<int>> persec_perclient_aggcachehitcnts;
	std::vector<std::vector<std::vector<int>>> persec_perclient_perserver_aggcachemisscnts;
	const int sleep_usecs = 1000; // 1000us = 1ms
	const int onesec_usecs = 1 * 1000 * 1000; // 1s
	// used for dynamic workload (and static worklaod for debugging)
	std::vector<std::vector<std::map<uint16_t, int>>> persec_perclient_nodeidx_aggpktcnt_map;
	if (workload_mode != 0) {
		persec_perclient_nodeidx_aggpktcnt_map.resize(dynamic_periodinterval * dynamic_periodnum);
		for (size_t i = 0; i < dynamic_periodinterval * dynamic_periodnum; i++) {
			persec_perclient_nodeidx_aggpktcnt_map[i].resize(current_client_logical_num);
		}
	}

	if (client_physical_idx != 0) { // wait for the first physical client
		printf("wait to be able to send pkt if necessary\n");
		while (!can_sendpkt) {}
	}
	else { // sendpktclient of the first physical client notifies other physical clients to start to send packets
		if (client_physical_num > 1) { // notify other physical clients if any to send packets
			int client_sendpktclient_udpsock = -1;
			create_udpsock(client_sendpktclient_udpsock, true, "client.sendpktclient");
			char sendpktclientbuf[256];
			int recvsize = -1;

			// NOTE: use client_physical_idx-1 to index for the client of client_physical_idx
			struct sockaddr_in other_client_sendpktserver_addr[client_physical_num - 1];
			socklen_t other_client_sendpktserver_addrlen[client_physical_num - 1];
			for (int other_client_physical_idx = 1; other_client_physical_idx < client_physical_num; other_client_physical_idx++) {
				set_sockaddr(other_client_sendpktserver_addr[other_client_physical_idx - 1], inet_addr(client_ip_for_client0_list[other_client_physical_idx]), client_sendpktserver_port_start + other_client_physical_idx - 1);
				other_client_sendpktserver_addrlen[other_client_physical_idx - 1] = sizeof(struct sockaddr_in);
			}

			while (true) {
				// NOTE: we launch other physical clients first; after they are waiting for can_sendpkt, we launch the first physical client to tell them send pkt at the same time
				for (int other_client_physical_idx = 1; other_client_physical_idx < client_physical_num; other_client_physical_idx++) {
					*((int *)sendpktclientbuf) = client_sendpktserver_port_start + other_client_physical_idx - 1;
					udpsendto(client_sendpktclient_udpsock, sendpktclientbuf, sizeof(int), 0, &other_client_sendpktserver_addr[other_client_physical_idx - 1], other_client_sendpktserver_addrlen[other_client_physical_idx - 1], "client.sendpktclient");
				}

				bool is_timeout = false;
				for (int other_client_physical_idx = 1; other_client_physical_idx < client_physical_num; other_client_physical_idx++) {
					is_timeout = udprecvfrom(client_sendpktclient_udpsock, sendpktclientbuf, 256, 0, NULL, NULL, recvsize, "client.sendpktclient");
					if (is_timeout) {
						break;
					}
					INVARIANT(recvsize == sizeof(int));
				}

				if (is_timeout) {
					continue;
				}
				else {
					break;
				}
			}

			close(client_sendpktclient_udpsock);
		}
	}

	printf("start to send pkt\n");
	struct timespec total_t1, total_t2, total_t3;
	running = true;

	/* (3) collect persec statistics for static/dynamic workload */

	CUR_TIME(total_t1);

/*#ifdef SERVER_ROTATION
	if (client_physical_idx == 0) {
		usleep(1000); // wait for 1000us to ensure that client.workers start to send packets
		// trigger controller.snapshot to evaluate the limited influence on transaction phase performance
		system("./preparefinish_client &");
	}
#endif*/

	if (workload_mode == 0) { // send all workloads in static mode
		while (finish_threads < current_client_logical_num) {
			CUR_TIME(total_t2);
			DELTA_TIME(total_t2, total_t1, total_t3);
			if (GET_MICROSECOND(total_t3) >= (persec_perclient_aggpktcnts.size()+1) * onesec_usecs) { // per second
				// collect for normalized throughput (TMPDEBUG)
				std::vector<std::map<uint16_t, int>> cursec_perclient_nodeidx_aggpktcnt_map(current_client_logical_num);
				for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
					cursec_perclient_nodeidx_aggpktcnt_map[local_client_logical_idx] = client_worker_params[local_client_logical_idx].nodeidx_pktcnt_map;
				}
				persec_perclient_nodeidx_aggpktcnt_map.push_back(cursec_perclient_nodeidx_aggpktcnt_map);

				// collect for runtime throughput
				std::vector<int> cursec_perclient_aggpktcnts(current_client_logical_num);
				std::vector<int> cursec_perclient_aggcachehitcnts(current_client_logical_num);
				std::vector<std::vector<int>> cursec_perclient_perserver_aggcachemisscnts(current_client_logical_num);
				for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
					cursec_perclient_aggpktcnts[local_client_logical_idx] = client_worker_params[local_client_logical_idx].process_latency_list.size();
					cursec_perclient_aggcachehitcnts[local_client_logical_idx] = client_worker_params[local_client_logical_idx].nodeidx_pktcnt_map[0xFFFF];
					std::vector<int> cursec_curclient_perserver_aggcachemisscnts(max_server_total_logical_num);
					for (uint16_t global_server_logical_idx = 0; global_server_logical_idx < max_server_total_logical_num; global_server_logical_idx++) {
						cursec_curclient_perserver_aggcachemisscnts[global_server_logical_idx] = client_worker_params[local_client_logical_idx].nodeidx_pktcnt_map[global_server_logical_idx];
					}
					cursec_perclient_perserver_aggcachemisscnts[local_client_logical_idx] = cursec_curclient_perserver_aggcachemisscnts;
				}
				persec_perclient_aggpktcnts.push_back(cursec_perclient_aggpktcnts);
				persec_perclient_aggcachehitcnts.push_back(cursec_perclient_aggcachehitcnts);
				persec_perclient_perserver_aggcachemisscnts.push_back(cursec_perclient_perserver_aggcachemisscnts);
			}

			// time limitation for static workload (either w/ or w/o server rotation)
			if (persec_perclient_aggpktcnts.size() >= 10) { // 10s
				break;
			}

			usleep(sleep_usecs);
		}
	}
	else { // send enough periods in dynamic mode (workload_mode != 0)
		// prepare for client0.rulemapclient
		int client_rulemapclient_udpsock = -1;
		char rulemapclientbuf[256];
		int recvsize = -1;
		// NOTE: use client_physical_idx-1 to index for the client of client_physical_idx
		struct sockaddr_in other_client_rulemapserver_addr[client_physical_num - 1];
		socklen_t other_client_rulemapserver_addrlen[client_physical_num - 1];
		if (client_physical_idx == 0 && client_physical_num > 1) {
			create_udpsock(client_rulemapclient_udpsock, false, "client.rulemapclient");
			for (int other_client_physical_idx = 1; other_client_physical_idx < client_physical_num; other_client_physical_idx++) {
				set_sockaddr(other_client_rulemapserver_addr[other_client_physical_idx - 1], inet_addr(client_ip_for_client0_list[other_client_physical_idx]), client_rulemapserver_port_start + other_client_physical_idx - 1);
				other_client_rulemapserver_addrlen[other_client_physical_idx - 1] = sizeof(struct sockaddr_in);
			}
		}

		struct timespec dynamic_t1, dynamic_t2, dynamic_t3;
		for (int periodidx = 0; periodidx < dynamic_periodnum; periodidx++) {
			for (int secidx = 0; secidx < dynamic_periodinterval; secidx++) { // wait for every 10 secs
				CUR_TIME(dynamic_t1);
				while (true) { // wait for every 1 sec to update thpt
					CUR_TIME(dynamic_t2);
					DELTA_TIME(dynamic_t2, dynamic_t1, dynamic_t3);

					int t3_usecs = int(GET_MICROSECOND(dynamic_t3));
					if (t3_usecs >= onesec_usecs) {
						//stop_for_dynamic_control = true;

						// collect for normalized throughput
						int global_secidx = periodidx * dynamic_periodinterval + secidx;
						for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
							persec_perclient_nodeidx_aggpktcnt_map[global_secidx][local_client_logical_idx] = client_worker_params[local_client_logical_idx].nodeidx_pktcnt_map;
						}

						// collect for runtime throughput
						std::vector<int> cursec_perclient_aggpktcnts(current_client_logical_num);
						std::vector<int> cursec_perclient_aggcachehitcnts(current_client_logical_num);
						std::vector<std::vector<int>> cursec_perclient_perserver_aggcachemisscnts(current_client_logical_num);
						for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
							cursec_perclient_aggpktcnts[local_client_logical_idx] = client_worker_params[local_client_logical_idx].process_latency_list.size();
							cursec_perclient_aggcachehitcnts[local_client_logical_idx] = client_worker_params[local_client_logical_idx].nodeidx_pktcnt_map[0xFFFF];
							std::vector<int> cursec_curclient_perserver_aggcachemisscnts(max_server_total_logical_num);
							for (uint16_t global_server_logical_idx = 0; global_server_logical_idx < max_server_total_logical_num; global_server_logical_idx++) {
								cursec_curclient_perserver_aggcachemisscnts[global_server_logical_idx] = client_worker_params[local_client_logical_idx].nodeidx_pktcnt_map[global_server_logical_idx];
							}
							cursec_perclient_perserver_aggcachemisscnts[local_client_logical_idx] = cursec_curclient_perserver_aggcachemisscnts;
						}
						persec_perclient_aggpktcnts.push_back(cursec_perclient_aggpktcnts);
						persec_perclient_aggcachehitcnts.push_back(cursec_perclient_aggcachehitcnts);
						persec_perclient_perserver_aggcachemisscnts.push_back(cursec_perclient_perserver_aggcachemisscnts);

						break;
					}
					else {
						usleep(sleep_usecs);
					}
				}

				/*if (secidx != dynamic_periodinterval - 1) { // not the last second in current period
					stop_for_dynamic_control = false;
				}*/
			}

			//INVARIANT(stop_for_dynamic_control == true);
			if (periodidx != dynamic_periodnum - 1) { // not the last period
				// the first physical client notifies other physical clients if any to switch dynamic rulemap
				if (client_physical_idx == 0) {
					if (client_physical_num > 1) {
						for (int other_client_physical_idx = 1; other_client_physical_idx < client_physical_num; other_client_physical_idx++) {
							*((int *)rulemapclientbuf) = periodidx; // from 0 to dynamic_periodnum-2
							udpsendto(client_rulemapclient_udpsock, rulemapclientbuf, sizeof(int), 0, &other_client_rulemapserver_addr[other_client_physical_idx - 1], other_client_rulemapserver_addrlen[other_client_physical_idx - 1], "client.rulemapclient");
						}
					}

					// switch dynamic rulemap to simulate key popularity change
					printf("switch dynamic rulemap from period %d to %d\n", periodidx, periodidx+1);
					dynamic_rulemap_ptr->nextperiod();
					//stop_for_dynamic_control = false;
				}
			}
		} // while loop for dynamic periods

		if (client_physical_idx == 0 && client_physical_num > 1) {
			close(client_rulemapclient_udpsock);
		}
	} // workload_mode != 0
	CUR_TIME(total_t2);
	DELTA_TIME(total_t2, total_t1, total_t3);
	double total_secs = GET_MICROSECOND(total_t3) / 1000.0 / 1000.0;

	running = false;
	//INVARIANT(workload_mode == 0 || stop_for_dynamic_control == true);
	//stop_for_dynamic_control = false;
	COUT_THIS("[client] all clients finish!");

	/* (4) process and dump statistics */

	COUT_THIS("[client] processing statistics");

	// Dump persec statistics for debugging
	// TODO: count perinterval statistics here if necessary
	// NOTE: we need to use the ith aggregate pktcnt - the {i-1}th aggregate pktcnt to get the ith sec pktcnt
	int seccnt = persec_perclient_aggpktcnts.size();
	printf("\nthpt interval: %d s, seccnt: %d\n", onesec_usecs/1000/1000, seccnt);
	std::vector<int> cursec_perclient_pktcnts;
	std::vector<int> cursec_perclient_cachehitcnts;
	std::vector<std::vector<int>> cursec_perclient_perserver_cachemisscnts;
	if (seccnt > 0) {
		cursec_perclient_pktcnts = persec_perclient_aggpktcnts[0];
		cursec_perclient_cachehitcnts = persec_perclient_aggcachehitcnts[0];
		cursec_perclient_perserver_cachemisscnts = persec_perclient_perserver_aggcachemisscnts[0];
	}
	bool first_stopped = true;
	for (size_t i = 0; i < seccnt; i++) {
		if (i != 0) {
			for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
				cursec_perclient_pktcnts[local_client_logical_idx] = persec_perclient_aggpktcnts[i][local_client_logical_idx] - persec_perclient_aggpktcnts[i-1][local_client_logical_idx];
				cursec_perclient_cachehitcnts[local_client_logical_idx] = persec_perclient_aggcachehitcnts[i][local_client_logical_idx] - persec_perclient_aggcachehitcnts[i-1][local_client_logical_idx];
				for (uint16_t global_server_logical_idx = 0; global_server_logical_idx < max_server_total_logical_num; global_server_logical_idx++) {
					cursec_perclient_perserver_cachemisscnts[local_client_logical_idx][global_server_logical_idx] = persec_perclient_perserver_aggcachemisscnts[i][local_client_logical_idx][global_server_logical_idx] - persec_perclient_perserver_aggcachemisscnts[i-1][local_client_logical_idx][global_server_logical_idx];
				}
			}
		}

		printf("\n[sec %d]\n", i);

#ifdef DEBUG_PERSEC
		// Dump pre-stopped threads
		printf("per-stopped-client agg pktcnt: ");
		for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
			if (cursec_perclient_pktcnts[local_client_logical_idx] == 0) {
				if (first_stopped) {
					printf("client%d-", local_client_logical_idx);
					for (size_t prev_sec_idx = 0; prev_sec_idx < i; prev_sec_idx++) {
						printf("%d-", persec_perclient_aggpktcnts[prev_sec_idx][local_client_logical_idx]);
					}
					printf("%d ", persec_perclient_aggpktcnts[i][local_client_logical_idx]);
				}
				else {
					printf("client%d-%d ", local_client_logical_idx, persec_perclient_aggpktcnts[i][local_client_logical_idx]);
				}
			}
		}
		printf("\n");

		/*printf("per-client throughput (MOPS): ");
		for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
			printf("%f ", double(cursec_perclient_pktcnts[local_client_logical_idx])/(onesec_usecs/1000/1000)/1024.0/1024.0);
		}*/
#endif
		int cursec_total_pktcnt = 0;
		for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
			cursec_total_pktcnt += cursec_perclient_pktcnts[local_client_logical_idx];
		}
		printf("\noverall totalpktcnt: %d, throughput (MOPS): %f\n", cursec_total_pktcnt, double(cursec_total_pktcnt)/(onesec_usecs/1000/1000)/1024.0/1024.0);

#ifdef DEBUG_PERSEC
		int cursec_total_cachehitcnt = 0;
		//printf("per-client cachehit rate: ");
		for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
			cursec_total_cachehitcnt += cursec_perclient_cachehitcnts[local_client_logical_idx];
			//printf("%f ", double(cursec_perclient_cachehitcnts[local_client_logical_idx])/double(cursec_perclient_pktcnts[local_client_logical_idx]));
		}
		printf("\noverall cachehit cnt: %d, cachehit rate: %f\n", cursec_total_cachehitcnt, double(cursec_total_cachehitcnt)/double(cursec_total_pktcnt));

		// per-server load ratio seems not important
		/*printf("per-client servers' load ratio: ");
		int cursec_total_cachemisscnt = 0;
		int cursec_perserver_cachemisscnts[max_server_total_logical_num];
		memset(cursec_perserver_cachemisscnts, 0, sizeof(int)*max_server_total_logical_num);
		for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
			int cursec_curclient_total_cachemisscnt = 0;
			for (size_t serveridx = 0; serveridx < max_server_total_logical_num; serveridx++) {
				cursec_curclient_total_cachemisscnt += cursec_perclient_perserver_cachemisscnts[local_client_logical_idx][serveridx];
				cursec_perserver_cachemisscnts[serveridx] += cursec_perclient_perserver_cachemisscnts[local_client_logical_idx][serveridx];
			}
			cursec_total_cachemisscnt += cursec_curclient_total_cachemisscnt;

			printf("%d-", cursec_curclient_total_cachemisscnt);
			for (uint16_t global_server_logical_idx = 0; global_server_logical_idx < max_server_total_logical_num; global_server_logical_idx++) {
				if (global_server_logical_idx != max_server_total_logical_num - 1) printf("%f-", cursec_perclient_perserver_cachemisscnts[local_client_logical_idx][global_server_logical_idx]/double(cursec_curclient_total_cachemisscnt));
				else printf("%f ", cursec_perclient_perserver_cachemisscnts[local_client_logical_idx][global_server_logical_idx]/double(cursec_curclient_total_cachemisscnt));
			}
		}
		printf("\noverall load ratio: %d-", cursec_total_cachemisscnt);
		for (size_t serveridx = 0; serveridx < max_server_total_logical_num; serveridx++) {
			if (serveridx != max_server_total_logical_num - 1) printf("%f-", cursec_perserver_cachemisscnts[serveridx]/double(cursec_total_cachemisscnt));
			else printf("%f\n", cursec_perserver_cachemisscnts[serveridx]/double(cursec_total_cachemisscnt));
		}*/

		std::vector<double> cursec_send_latency_list;
		std::vector<double> cursec_process_latency_list;
		std::vector<double> cursec_wait_latency_list;
		for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
			int startidx = 0;
			if (i != 0) {
				startidx = persec_perclient_aggpktcnts[i-1][local_client_logical_idx];
			}
			int endidx = persec_perclient_aggpktcnts[i][local_client_logical_idx];
			std::vector<double> cursec_curclient_send_latency_list(client_worker_params[local_client_logical_idx].send_latency_list.begin() + startidx, client_worker_params[local_client_logical_idx].send_latency_list.begin() + endidx);
			std::vector<double> cursec_curclient_process_latency_list(client_worker_params[local_client_logical_idx].process_latency_list.begin() + startidx, client_worker_params[local_client_logical_idx].process_latency_list.begin() + endidx);
			std::vector<double> cursec_curclient_wait_latency_list(client_worker_params[local_client_logical_idx].wait_latency_list.begin() + startidx, client_worker_params[local_client_logical_idx].wait_latency_list.begin() + endidx);
			cursec_send_latency_list.insert(cursec_send_latency_list.end(), cursec_curclient_send_latency_list.begin(), cursec_curclient_send_latency_list.end());
			cursec_process_latency_list.insert(cursec_process_latency_list.end(), cursec_curclient_process_latency_list.begin(), cursec_curclient_process_latency_list.end());
			cursec_wait_latency_list.insert(cursec_wait_latency_list.end(), cursec_curclient_wait_latency_list.begin(), cursec_curclient_wait_latency_list.end());

			/*std::string tmplabel;
			GET_STRING(tmplabel, "send_latency_list client "<<clientidx);
			dump_latency(cursec_curclient_send_latency_list, tmplabel.c_str());
			GET_STRING(tmplabel, "wait_latency_list client "<<clientidx);
			dump_latency(cursec_curclient_wait_latency_list, tmplabel.c_str());*/
		}
		dump_latency(cursec_send_latency_list, "send_latency_list overall");
		dump_latency(cursec_process_latency_list, "process_latency_list overall");
		dump_latency(cursec_wait_latency_list, "wait_latency_list overall");
#endif
	}
	printf("\n");

	// Process latency statistics
	std::map<uint16_t, int> nodeidx_pktcnt_map;
	std::vector<double> total_latency_list;
	for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
		for (std::map<uint16_t, int>::iterator iter = client_worker_params[local_client_logical_idx].nodeidx_pktcnt_map.begin(); \
				iter != client_worker_params[local_client_logical_idx].nodeidx_pktcnt_map.end(); iter++) {
			if (nodeidx_pktcnt_map.find(iter->first) == nodeidx_pktcnt_map.end()) {
				nodeidx_pktcnt_map.insert(*iter);
			}
			else {
				nodeidx_pktcnt_map[iter->first] += iter->second;
			}
		}
		total_latency_list.insert(total_latency_list.end(), client_worker_params[local_client_logical_idx].process_latency_list.begin(), client_worker_params[local_client_logical_idx].process_latency_list.end());
	}
	std::vector<double> perclient_avgsend_latency_list(current_client_logical_num);
	std::vector<double> perclient_avgwait_latency_list(current_client_logical_num);
	for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
		perclient_avgsend_latency_list[local_client_logical_idx] = 0.0;
		perclient_avgwait_latency_list[local_client_logical_idx] = 0.0;
	}
	for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
		for (size_t j = 0; j < client_worker_params[local_client_logical_idx].send_latency_list.size(); j++) {
			perclient_avgsend_latency_list[local_client_logical_idx] += client_worker_params[local_client_logical_idx].send_latency_list[j];
		}
		for (size_t j = 0; j < client_worker_params[local_client_logical_idx].wait_latency_list.size(); j++) {
			perclient_avgwait_latency_list[local_client_logical_idx] += client_worker_params[local_client_logical_idx].wait_latency_list[j];
		}
	}
	for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
		perclient_avgsend_latency_list[local_client_logical_idx] /= client_worker_params[local_client_logical_idx].send_latency_list.size();
		perclient_avgwait_latency_list[local_client_logical_idx] /= client_worker_params[local_client_logical_idx].wait_latency_list.size();
	}

	// Dump latency statistics
	dump_latency(total_latency_list, "total_latency_list");
	dump_latency(perclient_avgsend_latency_list, "perclient_avgsend_latency_list");
	dump_latency(perclient_avgwait_latency_list, "perclient_avgwait_latency_list");

	int total_unmatched_cnt = 0;
	int total_timeout_cnt = 0;
	for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
		total_unmatched_cnt += client_worker_params[local_client_logical_idx].unmatched_cnt;
		total_timeout_cnt += client_worker_params[local_client_logical_idx].timeout_cnt;
	}
	COUT_VAR(total_unmatched_cnt);
	COUT_VAR(total_timeout_cnt);

	// Dump throughput statistics
	int total_pktcnt = total_latency_list.size();
	double total_thpt = double(total_pktcnt) / total_secs / 1024.0 / 1024.0;
	printf("client-side total pktcnt: %d, total time: %f s, total thpt: %f MOPS\n", total_pktcnt, total_secs, total_thpt);
	
	// WRONG way of calculating system average throughput (not including 0ops if client finishes before system running time)
	/*double total_thpt = 0.0;
	for (size_t i = 0; i < client_num; i++) {
		double curclient_thpt = 0.0;
		int curclient_pktcnt = client_worker_params[i].req_latency_list.size();
		double curclient_totalusecs = 0.0;
		for (size_t j = 0; j < curclient_pktcnt; j++) {
			curclient_totalusecs += (client_worker_params[i].req_latency_list[j] + client_worker_params[i].rsp_latency_list[j] + client_worker_params[i].wait_latency_list[j]);
		}
		double curclient_totalsecs = curclient_totalusecs / 1000.0 / 1000.0;
		curclient_thpt = double(curclient_pktcnt) / curclient_totalsecs / 1024.0 / 1024.0;
		total_thpt += curclient_thpt;
	}
	printf("client-side total pktcnt: %d, total thpt: %f MOPS\n", total_pktcnt, total_thpt);*/

	COUT_THIS("cache hit pktcnt: " << nodeidx_pktcnt_map[0xFFFF]);
	printf("per-server pktcnt: ");
	for (uint16_t global_server_logical_idx = 0; global_server_logical_idx < max_server_total_logical_num; global_server_logical_idx++) {
		if (global_server_logical_idx != max_server_total_logical_num - 1) {
			printf("%d ", nodeidx_pktcnt_map[global_server_logical_idx]);
		}
		else {
			printf("%d\n", nodeidx_pktcnt_map[global_server_logical_idx]);
		}
	}

	/* (5) the first physical client aggregates results of server rotation from other physical clients */

#ifdef SERVER_ROTATION
	double avg_total_latency = 0.0;
	for (size_t i = 0; i < total_latency_list.size(); i++) {
		avg_total_latency += total_latency_list[i];
	}
	avg_total_latency /= total_latency_list.size();
	if (valid_global_server_logical_idxes.size() == 2) {
		printf("result for server rotation of client %d: %d %d %d %f %f\n", client_physical_idx, nodeidx_pktcnt_map[valid_global_server_logical_idxes[0]], \
				nodeidx_pktcnt_map[valid_global_server_logical_idxes[1]], total_pktcnt, total_thpt, avg_total_latency);
	}
	else if (valid_global_server_logical_idxes.size() == 1) {
		printf("result for server rotation of client %d: %d %d %f %f\n", client_physical_idx, nodeidx_pktcnt_map[valid_global_server_logical_idxes[0]], \
				total_pktcnt, total_thpt, avg_total_latency);
	}

	if (client_physical_idx != 0) {
		struct sockaddr_in client0_rotationdataserver_addr;
		set_sockaddr(client0_rotationdataserver_addr, inet_addr(client_ip_for_client0_list[0]), client_rotationdataserver_port);
		socklen_t client0_rotationdataserver_addrlen = sizeof(struct sockaddr_in);

		int client_rotationdataclient_udpsock = -1;
		create_udpsock(client_rotationdataclient_udpsock, true, "client.rotationdataclient");

		char rotationdataclient_buf[256];
		int recvsize = -1;

		while (true) {
			int tmpoff = 0;
			*((int *)(rotationdataclient_buf + tmpoff)) = nodeidx_pktcnt_map[valid_global_server_logical_idxes[0]];
			tmpoff += sizeof(int);
			if (valid_global_server_logical_idxes.size() == 2) {
				*((int *)(rotationdataclient_buf + tmpoff)) = nodeidx_pktcnt_map[valid_global_server_logical_idxes[1]];
			}
			else {
				*((int *)(rotationdataclient_buf + tmpoff)) = 0;
			}
			tmpoff += sizeof(int);
			*((int *)(rotationdataclient_buf + tmpoff)) = total_pktcnt;
			tmpoff += sizeof(int);
			*((double *)(rotationdataclient_buf + tmpoff)) = total_thpt;
			tmpoff += sizeof(double);
			*((double *)(rotationdataclient_buf + tmpoff)) = avg_total_latency;
			tmpoff += sizeof(double);

			udpsendto(client_rotationdataclient_udpsock, rotationdataclient_buf, tmpoff, 0, &client0_rotationdataserver_addr, client0_rotationdataserver_addrlen, "client.rotationdataclient");

			bool is_timeout = udprecvfrom(client_rotationdataclient_udpsock, rotationdataclient_buf, 256, 0, NULL, NULL, recvsize, "client.rotationdataclient");
			if (is_timeout) {
				continue;
			}
			else {
				break;
			}
		}

		close(client_rotationdataclient_udpsock);
	}
	else {
		if (client_physical_num > 1) {
			char rotationdataserver_buf[256];
			int recvsize = -1;

			int client_rotationdataserver_udpsock = -1;
			prepare_udpserver(client_rotationdataserver_udpsock, false, client_rotationdataserver_port, "client.rotationdataserver");

			int agg_bottleneckserver_pktcnt = nodeidx_pktcnt_map[valid_global_server_logical_idxes[0]];
			int agg_rotateserver_pktcnt = nodeidx_pktcnt_map[valid_global_server_logical_idxes[1]];
			int agg_total_pktcnt = total_pktcnt;
			double agg_total_thpt = total_thpt;
			double agg_avg_total_latency = avg_total_latency;
			for (uint16_t other_client_physical_idx = 1; other_client_physical_idx < client_physical_num; other_client_physical_idx++) {
				struct sockaddr_in other_client_rotationdataclient_addr;
				socklen_t other_client_rotationdataclient_addrlen = sizeof(struct sockaddr_in);
				udprecvfrom(client_rotationdataserver_udpsock, rotationdataserver_buf, 256, 0, &other_client_rotationdataclient_addr, &other_client_rotationdataclient_addrlen, recvsize, "client.rotationdataserver");

				int tmpoff = 0;
				agg_bottleneckserver_pktcnt += *((int *)(rotationdataserver_buf + tmpoff));
				tmpoff += sizeof(int);
				agg_rotateserver_pktcnt += *((int *)(rotationdataserver_buf + tmpoff));
				tmpoff += sizeof(int);
				agg_total_pktcnt += *((int *)(rotationdataserver_buf + tmpoff));
				tmpoff += sizeof(int);
				agg_total_thpt += *((double *)(rotationdataserver_buf + tmpoff));
				tmpoff += sizeof(double);
				agg_avg_total_latency += *((double *)(rotationdataserver_buf + tmpoff));
				tmpoff += sizeof(double);

				udpsendto(client_rotationdataserver_udpsock, rotationdataserver_buf, recvsize, 0, &other_client_rotationdataclient_addr, other_client_rotationdataclient_addrlen, "client.rotationdataserver");
			}
			agg_avg_total_latency /= client_physical_num;

			std::string rotationdata_filepath = "tmp_rotation.out";
			bool rotationdata_isexist = isexist(rotationdata_filepath);
			FILE *fd = fopen(rotationdata_filepath.c_str(), "a+");
			if (!rotationdata_isexist) {
				fprintf(fd, "%d\n", max_server_total_logical_num);
			}
			if (valid_global_server_logical_idxes.size() == 2) {
				printf("result for server rotation of all clients: %d %d %d %f %f %d %d\n", agg_bottleneckserver_pktcnt, \
						agg_rotateserver_pktcnt, agg_total_pktcnt, agg_total_thpt, agg_avg_total_latency, server_logical_idxes_list[0][0], server_logical_idxes_list[1][0]);
				fprintf(fd, "%d %d %d %f %f %d %d\n", agg_bottleneckserver_pktcnt, \
						agg_rotateserver_pktcnt, agg_total_pktcnt, agg_total_thpt, agg_avg_total_latency, server_logical_idxes_list[0][0], server_logical_idxes_list[1][0]);
			}
			else if (valid_global_server_logical_idxes.size() == 1) {
				printf("result for server rotation of all clients: %d %d %f %f %d\n", agg_bottleneckserver_pktcnt, \
						agg_total_pktcnt, agg_total_thpt, agg_avg_total_latency, server_logical_idxes_list[0][0]);
				fprintf(fd, "%d %d %f %f %d\n", agg_bottleneckserver_pktcnt, \
						agg_total_pktcnt, agg_total_thpt, agg_avg_total_latency, server_logical_idxes_list[0][0]);
			}
			fflush(fd);
			fclose(fd);

			close(client_rotationdataserver_udpsock);
		}
	}
#endif

	/* (6) process and dump dynamic workload statisics */

	if (workload_mode != 0) {
		// get persec nodeidx-pktcnt mapping data
		INVARIANT(seccnt == persec_perclient_nodeidx_aggpktcnt_map.size());
		std::map<uint16_t, int> persec_nodeidx_pktcnt_map[seccnt];
		// aggregate perclient pktcnt for each previous i seconds -> accumulative pktcnt for each previous i seconds
		for (int i = 0; i < seccnt; i++) {
			for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
				for (std::map<uint16_t, int>::iterator iter = persec_perclient_nodeidx_aggpktcnt_map[i][local_client_logical_idx].begin(); \
						iter != persec_perclient_nodeidx_aggpktcnt_map[i][local_client_logical_idx].end(); iter++) {
					if (persec_nodeidx_pktcnt_map[i].find(iter->first) == persec_nodeidx_pktcnt_map[i].end()) {
						persec_nodeidx_pktcnt_map[i].insert(*iter);
					}
					else {
						persec_nodeidx_pktcnt_map[i][iter->first] += iter->second;
					}
				}
			}
		}
		// accumulative pktcnt for previous i seconds - that for previous i-1 seconds -> pktcnt for the ith second
		for (int i = seccnt - 1; i > 0; i--) {
			int tmptotalpktcnt = 0;
			for (std::map<uint16_t, int>::iterator iter = persec_nodeidx_pktcnt_map[i].begin(); \
					iter != persec_nodeidx_pktcnt_map[i].end(); iter++) {
				tmptotalpktcnt += iter->second;
				if (persec_nodeidx_pktcnt_map[i-1].find(iter->first) != persec_nodeidx_pktcnt_map[i-1].end()) {
					INVARIANT(iter->second >= persec_nodeidx_pktcnt_map[i-1][iter->first]);
					iter->second -= persec_nodeidx_pktcnt_map[i-1][iter->first];
				}
			}
		}
		// get persec total pktcnt
		int persec_totalpktcnt_list[seccnt];
		memset(persec_totalpktcnt_list, 0, seccnt*sizeof(int));
		for (int i = 0; i < seccnt; i++) {
			for (std::map<uint16_t, int>::iterator iter = persec_nodeidx_pktcnt_map[i].begin(); \
					iter != persec_nodeidx_pktcnt_map[i].end(); iter++) {
				persec_totalpktcnt_list[i] += iter->second;
			}
		}
		printf("\nper-sec total throughput:\n");
		for (int i = 0; i < seccnt; i++) {
			if (i != seccnt - 1) {
				printf("%d ", persec_totalpktcnt_list[i]);
			}
			else {
				printf("%d\n", persec_totalpktcnt_list[i]);
			}
		}
		printf("\nper-sec per-server throughput:\n");
		for (int i = 0; i < seccnt; i++) {
			for (uint16_t global_server_logical_idx = 0; global_server_logical_idx < max_server_total_logical_num; global_server_logical_idx++) {
				int tmppktcnt = 0;
				if (persec_nodeidx_pktcnt_map[i].find(global_server_logical_idx) != persec_nodeidx_pktcnt_map[i].end()) {
					tmppktcnt = persec_nodeidx_pktcnt_map[i][global_server_logical_idx];
				}
				if (global_server_logical_idx != max_server_total_logical_num - 1) {
					printf("%d ", tmppktcnt);
				}
				else {
					printf("%d\n", tmppktcnt);
				}
			}
		}
	}

	free_common();
	COUT_THIS("Finish dumping statistics!")

	void *status;
	for (uint16_t local_client_logical_idx = 0; local_client_logical_idx < current_client_logical_num; local_client_logical_idx++) {
		int rc = pthread_join(threads[local_client_logical_idx], &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join worker " << rc);
		}
	}
	if (client_physical_idx != 0) {
		int rc = pthread_join(sendpktserver_thread, &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join sendpktserver " << rc);
		}
	}
	if (client_physical_idx != 0 && workload_mode != 0) {
		int rc = pthread_join(rulemapserver_thread, &status);
		if (rc) {
			COUT_N_EXIT("Error: unable to join rulemapserver " << rc);
		}
	}
}

void *run_client_worker(void *param) {
	client_worker_param_t &thread_param = *(client_worker_param_t *)param;
	uint16_t local_client_logical_idx = thread_param.local_client_logical_idx;

	int res = 0;

	// ycsb parser
	char load_filename[256];
	memset(load_filename, '\0', 256);
	uint16_t global_client_logical_idx = local_client_logical_idx;
	if (client_physical_idx > 0) {
		for (uint32_t i = 0; i < client_physical_idx; i++) {
			global_client_logical_idx += client_logical_nums[i];
		}
	}
	RUN_SPLIT_WORKLOAD(load_filename, client_workload_dir, global_client_logical_idx);

	ParserIterator *iter = NULL;
#ifdef USE_YCSB
	iter = new YcsbParserIterator(load_filename);
#elif defined USE_SYNTHETIC
	iter = new SyntheticParserIterator(load_filename);
#endif
	INVARIANT(iter != NULL);

	netreach_key_t tmpkey;
	//char tmpval_bytes[128];
	//memset(tmpval_bytes, 0x11, 128 * sizeof(char));
	char tmpval_bytes[10240];
	memset(tmpval_bytes, 0x11, 10240 * sizeof(char));
	val_t *tmpval_ptr = NULL;
	// small value
	if (local_client_logical_idx >= 0) { // all small packets
	//if (local_client_logical_idx >= 256) { // [0, x] small packets; [x+1, 512] all large packets
	//if (local_client_logical_idx >= 512) { // all large packets
		tmpval_ptr = new val_t(tmpval_bytes, 128);
	}
	// large value
	else {
		tmpval_ptr = new val_t(tmpval_bytes, 10240);
	}
	val_t &tmpval = *tmpval_ptr;
	optype_t tmptype;

	// for network communication
	char buf[MAX_BUFSIZE];
	dynamic_array_t dynamicbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
	int req_size = 0;
	int recv_size = 0;
	struct sockaddr_in server_addr;
	// all client threads use the ip of the first physical server and the port of the first server.worker as destination address (switch covers server-side partition)
	set_sockaddr(server_addr, inet_addr(server_ips[0]), server_worker_port_start);
	socklen_t server_addrlen = sizeof(struct sockaddr_in);

	// DEPRECATED: we only support small range scan -> client_num * server_num * MAX_BUFSIZE memory overhead
	// Now we dynamically assign memory to receive scan response in udprecvlarge_multisrc in socket_helper.c
	// We also determine number of srcs in runtime instead of using server_num or max_server_num in client side
	//char *scanbufs = new char[server_num * MAX_BUFSIZE];
	//int scan_recvsizes[server_num];

#if !defined(NDEBUGGING_LOG)
	std::string logname;
	GET_STRING(logname, "tmp_client"<< uint32_t(local_client_logical_idx)<<".out");
	std::ofstream ofs(logname, std::ofstream::out);
#endif

	// NOTE: we pre-load key-value requests from disk into memory to alleviate effect of client-side overhead on system runtime throughput
	// For server rotation, we only use the requests destinated to the one/two server partitions under in order to saturate servers
	std::vector<optype_t> preload_types;
	std::vector<netreach_key_t> preload_keys;
	//std::vector<val_t> preload_vals; // comment preload_vals to save memory
	int preload_idx = 0;
	while (true) {
		if (!iter->next()) {
			break;
		}

#ifdef SERVER_ROTATION
#ifdef USE_HASH
		uint32_t expected_serveridx = iter->key().get_hashpartition_idx(switch_partition_count, max_server_total_logical_num);
#elif defined(USE_RANGE)
		uint32_t expected_serveridx = iter->key().get_rangepartition_idx(max_server_total_logical_num);
#endif
		if ((valid_global_server_logical_idxes.size() == 1 && expected_serveridx != valid_global_server_logical_idxes[0]) || \
			(valid_global_server_logical_idxes.size() == 2 && expected_serveridx != valid_global_server_logical_idxes[0] && expected_serveridx != valid_global_server_logical_idxes[1])) {
			continue;
		}
#endif

		preload_types.push_back(iter->type());
		preload_keys.push_back(iter->key());
		//preload_vals.push_back(iter->val());
	}


	COUT_THIS("[client " << uint32_t(local_client_logical_idx) << "] Ready.");
	ready_threads++;

	// For rate limit
	//double cur_sending_time = 0.0; // Set to 0 periodically
	//size_t cur_sending_rate = 0;

	while (!running)
		;

	bool is_timeout = false;
	bool isfirst_pkt = true;
	uint32_t fragseq = 0; // for packet loss of large value to server
	//srand(0); // for read-write mixed workload
	while (running) {
/*#ifndef SERVER_ROTATION
		if (!iter->next()) {
			if (workload_mode != 0) { // dynamic mode
				iter->reset();
				iter->next();
			}
			else { // static mode
				iter->closeiter();
				delete iter;
				iter = NULL;
				break;
			}
		}
		tmptype = iter->type();
		tmpkey = iter->key();
		tmpval = iter->val();
#else*/
		if (preload_idx >= preload_types.size()) {
			if (workload_mode != 0) { // dynamic mode
				preload_idx = 0;
			}
			else { // static mode
				break;
			}
		}
		tmptype = preload_types[preload_idx];
		//tmptype = optype_t(packet_type_t::GETREQ); // read-only
		//tmptype = (rand()%2==0)?optype_t(packet_type_t::GETREQ):optype_t(packet_type_t::PUTREQ); // 50% write
		tmpkey = preload_keys[preload_idx];
		//tmpval = preload_vals[preload_idx];
		preload_idx += 1;
//#endif

		struct timespec process_t1, process_t2, process_t3, send_t1, send_t2, send_t3, wait_t1, wait_t2, wait_t3;
		uint16_t tmp_nodeidx_foreval = 0;
		double wait_time = 0.0;
		while (true) { // timeout-and-retry mechanism
			CUR_TIME(process_t1);

			//printf("expected server of key %x: %d\n", tmpkey.keyhihi, tmpkey.get_hashpartition_idx(server_num));
			//printf("expected server of key %x: %d\n", tmpkey.keyhihi, tmpkey.get_rangepartition_idx(server_num));
			if (workload_mode != 0) { // change key popularity if necessary
				/*while (stop_for_dynamic_control) {} // stop for dynamic control between client.main and server.main
				if (unlikely(!running)) {
					break;
				}*/
				dynamic_rulemap_ptr->trymap(tmpkey);
			}

			if (tmptype == optype_t(packet_type_t::GETREQ)) { // get
				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);
				get_request_t req(tmpkey);
				FDEBUG_THIS(ofs, "[client " << uint32_t(local_client_logical_idx) << "] key = " << tmpkey.to_string());
				req_size = req.serialize(buf, MAX_BUFSIZE);
#ifdef DUMP_BUF
				dump_buf(buf, req_size);
#endif
				if (!isfirst_pkt) {
					CUR_TIME(wait_t2);
					DELTA_TIME(wait_t2, wait_t1, wait_t3);
					wait_time = GET_MICROSECOND(wait_t3);
				}
				CUR_TIME(send_t1);
				udpsendto(client_udpsock_list[local_client_logical_idx], buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(send_t2);

				// filter unmatched responses to fix duplicate responses of previous request due to false positive timeout-and-retry
				while (true) {
					//is_timeout = udprecvfrom(client_udpsock_list[local_client_logical_idx], buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
					dynamicbuf.clear();
					is_timeout = udprecvlarge_ipfrag(client_udpsock_list[local_client_logical_idx], dynamicbuf, 0, NULL, NULL, "ycsb_remote_client");
					recv_size = dynamicbuf.size();
					CUR_TIME(wait_t1);
#ifdef DUMP_BUF
					dump_buf(dynamicbuf.array(), recv_size);
#endif
					if (!is_timeout) {
						INVARIANT(recv_size > 0);
						packet_type_t tmp_pkttype_for_getreq = get_packet_type(dynamicbuf.array(), recv_size);
						if (tmp_pkttype_for_getreq != packet_type_t::GETRES && tmp_pkttype_for_getreq != packet_type_t::GETRES_LARGEVALUE) {
							thread_param.unmatched_cnt++;
							continue; // continue to receive next packet
						}
						else {
							netreach_key_t tmp_key_for_getreq;
							uint16_t tmp_nodeidx_foreval_for_getreq = 0;
							if (tmp_pkttype_for_getreq == packet_type_t::GETRES) {
								get_response_t rsp(dynamicbuf.array(), recv_size);
								tmp_key_for_getreq = rsp.key();
								tmp_nodeidx_foreval_for_getreq = rsp.nodeidx_foreval();
							}
							else {
								get_response_largevalue_t rsp(dynamicbuf.array(), recv_size);
								tmp_key_for_getreq = rsp.key();
								tmp_nodeidx_foreval_for_getreq = rsp.nodeidx_foreval();
							}
							if (tmp_key_for_getreq != tmpkey) {
								thread_param.unmatched_cnt++;
								continue; // continue to receive next packet
							}
							else {
								tmp_nodeidx_foreval = tmp_nodeidx_foreval_for_getreq;
								//FDEBUG_THIS(ofs, "[client " << uint32_t(local_client_logical_idx) << "] key = " << rsp.key().to_string() << " val = " << rsp.val().to_string());
								break; // break to update statistics and send next packet
							}
						}
					}
					else {
						break; // break to resend
					}
				}
				if (is_timeout) {
					thread_param.timeout_cnt += 1;
					continue; // continue to resend
				}
			}
			else if (tmptype == optype_t(packet_type_t::PUTREQ)) { // update or insert
				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);

				FDEBUG_THIS(ofs, "[client " << uint32_t(local_client_logical_idx) << "] key = " << tmpkey.to_string() << " val = " << req.val().to_string());
				if (tmpval.val_length <= val_t::SWITCH_MAX_VALLEN) {
					put_request_t req(tmpkey, tmpval);
					req_size = req.serialize(buf, MAX_BUFSIZE);
#ifdef DUMP_BUF
					dump_buf(buf, req_size);
#endif
					if (!isfirst_pkt) {
						CUR_TIME(wait_t2);
						DELTA_TIME(wait_t2, wait_t1, wait_t3);
						wait_time = GET_MICROSECOND(wait_t3);
					}
					CUR_TIME(send_t1);
					udpsendto(client_udpsock_list[local_client_logical_idx], buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");
					CUR_TIME(send_t2);
				}
				else { // for large value
					put_request_largevalue_t req(tmpkey, tmpval, global_client_logical_idx, fragseq);
					fragseq += 1;
					dynamicbuf.clear();
					req_size = req.dynamic_serialize(dynamicbuf);
#ifdef DUMP_BUF
					dump_buf(dynamicbuf.array(), req_size);
#endif
					if (!isfirst_pkt) {
						CUR_TIME(wait_t2);
						DELTA_TIME(wait_t2, wait_t1, wait_t3);
						wait_time = GET_MICROSECOND(wait_t3);
					}
					CUR_TIME(send_t1);
					udpsendlarge_ipfrag(client_udpsock_list[local_client_logical_idx], dynamicbuf.array(), req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client", put_request_largevalue_t::get_frag_hdrsize());
					CUR_TIME(send_t2);
				}

				// filter unmatched responses to fix duplicate responses of previous request due to false positive timeout-and-retry
				while (true) {
					// TMPDEBUG
					/*bool prevtimeout = is_timeout;
					if (is_timeout && local_client_logical_idx == 0) {
						dump_buf(buf, req_size);
					}*/

					is_timeout = udprecvfrom(client_udpsock_list[local_client_logical_idx], buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
					CUR_TIME(wait_t1);

					/*if (is_timeout && local_client_logical_idx != 0) { // TMPDEBUG
						printf("client %d exit\n", local_client_logical_idx);
						close(client_udpsock_list[local_client_logical_idx]);
						pthread_exit(nullptr);
					}

					if (prevtimeout && local_client_logical_idx == 0) { // TMPDEBUG
						if (is_timeout) {
							printf("still timeout; errno: %d!\n", errno);
						}
						else {
							COUT_VAR(recv_size);
							dump_buf(buf, recv_size);
						}
					}*/
#ifdef DUMP_BUF
					dump_buf(buf, recv_size);
#endif
					if (!is_timeout) {
						INVARIANT(recv_size > 0);
						if (get_packet_type(buf, recv_size) != packet_type_t::PUTRES) {
							thread_param.unmatched_cnt++;
							continue; // continue to receive next packet
						}
						else {
							put_response_t rsp(buf, recv_size);
							if (rsp.key() != tmpkey) {
								thread_param.unmatched_cnt++;
								continue; // continue to receive next packet
							}
							else {
								tmp_nodeidx_foreval = rsp.nodeidx_foreval();
								FDEBUG_THIS(ofs, "[client " << uint32_t(local_client_logical_idx) << "] stat = " << rsp.stat());
								break; // break to update statistics and send next packet
							}
						}
					}
					else {
						break; // break to resend
					}
				}
				if (is_timeout) {
#ifdef USE_HASH
					printf("timeout key %x from client %d to server %d\n", tmpkey.keyhihi, local_client_logical_idx, tmpkey.get_hashpartition_idx(switch_partition_count, max_server_total_logical_num));
#else
					printf("timeout key %x from client %d to server %d\n", tmpkey.keyhihi, local_client_logical_idx, tmpkey.get_rangepartition_idx(max_server_total_logical_num));
#endif
					thread_param.timeout_cnt += 1;
					continue; // continue to resend
				}
			}
			else if (tmptype == optype_t(packet_type_t::LOADREQ)) { // for loading phase
				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);

				FDEBUG_THIS(ofs, "[client " << uint32_t(local_client_logical_idx) << "] key = " << tmpkey.to_string() << " val = " << req.val().to_string());
				// NOTE: LOADREQ allows both small/large value
				load_request_t req(tmpkey, tmpval, global_client_logical_idx, fragseq);
				fragseq += 1;
				dynamicbuf.clear();
				req_size = req.dynamic_serialize(dynamicbuf);
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), req_size);
#endif
				if (!isfirst_pkt) {
					CUR_TIME(wait_t2);
					DELTA_TIME(wait_t2, wait_t1, wait_t3);
					wait_time = GET_MICROSECOND(wait_t3);
				}
				CUR_TIME(send_t1);
				udpsendlarge_ipfrag(client_udpsock_list[local_client_logical_idx], dynamicbuf.array(), req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client", load_request_t::get_frag_hdrsize());
				CUR_TIME(send_t2);

				// filter unmatched responses to fix duplicate responses of previous request due to false positive timeout-and-retry
				while (true) {
					is_timeout = udprecvfrom(client_udpsock_list[local_client_logical_idx], buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
					CUR_TIME(wait_t1);
#ifdef DUMP_BUF
					dump_buf(buf, recv_size);
#endif
					if (!is_timeout) {
						INVARIANT(recv_size > 0);
						if (get_packet_type(buf, recv_size) != packet_type_t::LOADACK) {
							thread_param.unmatched_cnt++;
							continue; // continue to receive next packet
						}
						else {
							load_ack_t rsp(buf, recv_size);
							if (rsp.key() != tmpkey) {
								thread_param.unmatched_cnt++;
								continue; // continue to receive next packet
							}
							else {
								break; // break to update statistics and send next packet
							}
						}
					}
					else {
						break; // break to resend
					}
				}
				if (is_timeout) {
					printf("timeout key %x\n", tmpkey.keyhihi);
					thread_param.timeout_cnt += 1;
					continue; // continue to resend
				}
			}
			else if (tmptype == optype_t(packet_type_t::DELREQ)) {
				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);
				del_request_t req(tmpkey);
				FDEBUG_THIS(ofs, "[client " << uint32_t(local_client_logical_idx) << "] key = " << tmpkey.to_string());
				req_size = req.serialize(buf, MAX_BUFSIZE);
#ifdef DUMP_BUF
				dump_buf(buf, req_size);
#endif

				if (!isfirst_pkt) {
					CUR_TIME(wait_t2);
					DELTA_TIME(wait_t2, wait_t1, wait_t3);
					wait_time = GET_MICROSECOND(wait_t3);
				}
				CUR_TIME(send_t1);
				udpsendto(client_udpsock_list[local_client_logical_idx], buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(send_t2);

				// filter unmatched responses to fix duplicate responses of previous request due to false positive timeout-and-retry
				while (true) {
					is_timeout = udprecvfrom(client_udpsock_list[local_client_logical_idx], buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
					CUR_TIME(wait_t1);
#ifdef DUMP_BUF
					dump_buf(buf, recv_size);
#endif
					if (!is_timeout) {
						INVARIANT(recv_size > 0);
						if (get_packet_type(buf, recv_size) != packet_type_t::DELRES) {
							thread_param.unmatched_cnt++;
							continue; // continue to receive next packet
						}
						else {
							del_response_t rsp(buf, recv_size);
							if (rsp.key() != tmpkey) {
								thread_param.unmatched_cnt++;
								continue; // continue to receive next packet
							}
							else {
								tmp_nodeidx_foreval = rsp.nodeidx_foreval();
								FDEBUG_THIS(ofs, "[client " << uint32_t(local_client_logical_idx) << "] stat = " << rsp.stat());
								break; // break to update statistics and send next packet
							}
						}
					}
					else {
						break; // break to resend
					}
				}
				if (is_timeout) {
					thread_param.timeout_cnt += 1;
					continue; // continue to resend
				}
			}
			else if (tmptype == optype_t(packet_type_t::SCANREQ)) {
				//netreach_key_t endkey = generate_endkey(tmpkey);
				netreach_key_t endkey = netreach_key_t(tmpkey.keylolo, tmpkey.keylohi, tmpkey.keyhilo, (((tmpkey.keyhihi>>16)&0xFFFF)+513)<<16); // TMPDEBUG
				/*size_t first_server_idx = get_server_idx(tmpkey);
				size_t last_server_idx = get_server_idx(endkey);
				size_t split_num = last_server_idx - first_server_idx + 1;*/

				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);
				//scan_request_t req(tmpkey, endkey, range_num);
				scan_request_t req(tmpkey, endkey);
				FDEBUG_THIS(ofs, "[client " << uint32_t(local_client_logical_idx) << "] startkey = " << tmpkey.to_string() 
						<< "endkey = " << endkey.to_string());
				req_size = req.serialize(buf, MAX_BUFSIZE);
#ifdef DUMP_BUF
				dump_buf(buf, req_size);
#endif

				if (!isfirst_pkt) {
					CUR_TIME(wait_t2);
					DELTA_TIME(wait_t2, wait_t1, wait_t3);
					wait_time = GET_MICROSECOND(wait_t3);
				}
				CUR_TIME(send_t1);
				udpsendto(client_udpsock_list[local_client_logical_idx], buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(send_t2);

				std::vector<std::vector<dynamic_array_t>> perswitch_perserver_scanbufs;
				std::vector<std::vector<struct sockaddr_in>> perswitch_perserver_addrs;
				std::vector<std::vector<socklen_t>> perswitch_perserver_addrlens;
				set_recvtimeout(client_udpsock_list[local_client_logical_idx], CLIENT_SCAN_SOCKET_TIMEOUT_SECS, 0); // 10s for SCAN
				is_timeout = udprecvlarge_multisrc_ipfrag(client_udpsock_list[local_client_logical_idx], perswitch_perserver_scanbufs, 0, perswitch_perserver_addrs, perswitch_perserver_addrlens, "remote_client.worker", scan_response_split_t::get_frag_hdrsize(), scan_response_split_t::get_srcnum_off(), scan_response_split_t::get_srcnum_len(), scan_response_split_t::get_srcnum_conversion(), scan_response_split_t::get_srcid_off(), scan_response_split_t::get_srcid_len(), scan_response_split_t::get_srcid_conversion(), scan_response_split_t::get_srcswitchnum_off(), scan_response_split_t::get_srcswitchnum_len(), scan_response_split_t::get_srcswitchnum_conversion(), scan_response_split_t::get_srcswitchid_off(), scan_response_split_t::get_srcswitchid_len(), scan_response_split_t::get_srcswitchid_conversion(), true, optype_t(packet_type_t::SCANRES_SPLIT), tmpkey);
				CUR_TIME(wait_t1);
				set_recvtimeout(client_udpsock_list[local_client_logical_idx], CLIENT_SOCKET_TIMEOUT_SECS, 0); // 100ms for other reqs
				if (is_timeout) {
					thread_param.timeout_cnt += 1;
					continue;
				}

				int snapshotid = -1;
				int totalnum = 0;
				int tmp_srcswitchnum = perswitch_perserver_scanbufs.size();
				COUT_VAR(tmp_srcswitchnum);
				for (int i = 0; i < tmp_srcswitchnum; i++) {
					printf("[leaf switch %d] tmp_srcservernum %d\n", i, perswitch_perserver_scanbufs[i].size());
					for (int j = 0; j < perswitch_perserver_scanbufs[i].size(); j++) {
						scan_response_split_t rsp(perswitch_perserver_scanbufs[i][j].array(), perswitch_perserver_scanbufs[i][j].size());
						FDEBUG_THIS(ofs, "[client " << uint32_t(local_client_logical_idx) << "] startkey = " << rsp.key().to_string()
								<< "endkey = " << rsp.endkey().to_string() << " pairnum = " << rsp.pairnum());
						totalnum += rsp.pairnum();
						// check scan response consistency
						if (snapshotid == -1) {
							snapshotid = rsp.snapshotid();
							tmp_nodeidx_foreval = rsp.nodeidx_foreval();
						}
						else if (snapshotid != rsp.snapshotid()) {
							printf("Inconsistent scan response!\n"); // TMPDEBUG
							is_timeout = true; // retry
							break;
						}
					}

					if (is_timeout) {
						break;
					}
				}
				COUT_VAR(totalnum);

				if (is_timeout) {
					thread_param.unmatched_cnt++;
					continue;
				}
			}
			else {
				printf("Invalid request type: %u\n", uint32_t(tmptype));
				exit(-1);
			}
			break;
		} // end of while(true)
		is_timeout = false;
		CUR_TIME(process_t2);

		if (unlikely(!running)) {
			break;
		}

		if (tmptype != optype_t(packet_type_t::LOADREQ)) {
			INVARIANT(tmp_nodeidx_foreval == 0xFFFF || tmp_nodeidx_foreval < max_server_total_logical_num);
			if (thread_param.nodeidx_pktcnt_map.find(tmp_nodeidx_foreval) == thread_param.nodeidx_pktcnt_map.end()) {
				thread_param.nodeidx_pktcnt_map.insert(std::pair<uint16_t, int>(tmp_nodeidx_foreval, 1));
			}
			else {
				thread_param.nodeidx_pktcnt_map[tmp_nodeidx_foreval] += 1;
			}

			DELTA_TIME(process_t2, process_t1, process_t3);
			DELTA_TIME(send_t2, send_t1, send_t3);
			double process_time = GET_MICROSECOND(process_t3);
			double send_time = GET_MICROSECOND(send_t3);
			thread_param.process_latency_list.push_back(process_time);
			thread_param.send_latency_list.push_back(send_time);
			if (!isfirst_pkt) {
				thread_param.wait_latency_list.push_back(wait_time);
			}

			isfirst_pkt = false;
		}

		//SUM_TIME(req_t3, rsp_t3, final_t3); // time of sending req and receiving rsp
		//double final_time = GET_MICROSECOND(final_t3);
		/*double final_time = req_time + rsp_time;
		if (wait_time > dpdk_polling_time) {
			// wait_time: in-switch queuing + server-side latency + dpdk overhead + client-side packet dispatching (receiver) + schedule cost for PMD (unnecessary)
			final_time += (wait_time - dpdk_polling_time); // time of in-switch queuing and server-side latency
		}*/

		//thread_param.sum_latency += final_time;

		// Rate limit (within each rate_limit_period, we can send at most per_client_per_period_max_sending_rate reqs)
		//cur_sending_rate++;
		//cur_sending_time += final_time;
		/*if (cur_sending_rate >= per_client_per_period_max_sending_rate) {
			if (cur_sending_time < rate_limit_period) {
				usleep(rate_limit_period - cur_sending_time);
			}
			cur_sending_rate = 0;
			cur_sending_time = 0.0;
		}*/
	}

	close(client_udpsock_list[local_client_logical_idx]);
#if !defined(NDEBUGGING_LOG)
	ofs.close();
#endif
	finish_threads++;
	pthread_exit(nullptr);
	return 0;
}

void *run_client_sendpktserver(void *param) {
	INVARIANT(running == false && client_physical_idx != 0);

	// prepare for client.sendpktserver
	int client_sendpktserver_udpsock = -1;
	prepare_udpserver(client_sendpktserver_udpsock, false, client_sendpktserver_port_start + client_physical_idx - 1, "client.sendpktserver");

	printf("[client.sendpktserver] ready\n");
	ready_threads++;

	char sendpktserverbuf[256];
	int recvsize = -1;
	struct sockaddr_in client0_addr;
	memset(&client0_addr, 0, sizeof(struct sockaddr_in));
	socklen_t client0_addrlen = sizeof(struct sockaddr_in);

	udprecvfrom(client_sendpktserver_udpsock, sendpktserverbuf, 256, 0, &client0_addr, &client0_addrlen, recvsize, "client.sendpktserver");
	INVARIANT(recvsize == sizeof(int) && *((int *)sendpktserverbuf) == client_sendpktserver_port_start + client_physical_idx - 1);

	udpsendto(client_sendpktserver_udpsock, sendpktserverbuf, recvsize, 0, &client0_addr, client0_addrlen, "client.sendpktserver");
	can_sendpkt = true;
	close(client_sendpktserver_udpsock);
	pthread_exit(nullptr);
}

void *run_client_rulemapserver(void *param) {
	INVARIANT(running == false && client_physical_idx != 0 && workload_mode != 0);

	// prepare for client.rulemapserver
	int client_rulemapserver_udpsock = -1;
	prepare_udpserver(client_rulemapserver_udpsock, false, client_rulemapserver_port_start + client_physical_idx - 1, "client.rulemapserver");

	printf("[client.rulemapserver] ready\n");
	ready_threads++;

	while (!running) {}

	char rulemapserverbuf[256];
	int recvsize = -1;
	struct sockaddr_in client0_addr;
	memset(&client0_addr, 0, sizeof(struct sockaddr_in));
	socklen_t client0_addrlen = sizeof(struct sockaddr_in);

	int dynamic_periodidx = 0; // expected dynamic period idx (from 0 to dynamic_periodnum - 2)
	while (running) {
		udprecvfrom(client_rulemapserver_udpsock, rulemapserverbuf, 256, 0, &client0_addr, &client0_addrlen, recvsize, "client.rulemapserver");
		INVARIANT(recvsize == sizeof(int) && *((int *)rulemapserverbuf) == dynamic_periodidx);


		// We do not sendback ACK for client0.rulemapclient, as we assume that the control message will not suffer from pktlos
		// NOTE: if with pktloss (nearly impossible), physical clients do not change rulemap at similar time and we should re-test dynamic workload
		//udpsendto(client_rulemapserver_udpsock, rulemapserverbuf, recvsize, 0, &client0_addr, client0_addrlen, "client.rulemapserver");
		
		// switch dynamic rulemap to simulate key popularity change
		dynamic_rulemap_ptr->nextperiod();
		printf("switch dynamic rulemap from period %d to %d\n", dynamic_periodidx, dynamic_periodidx+1);
		
		if (dynamic_periodidx == dynamic_periodnum - 2) { // NOTE: the last dynamic period does not need to switch rulemap -> rulemapserver can exit after dynamic_periodnum-1 indexes
			break;
		}
		dynamic_periodidx += 1; // update expected dynamic period idx for next command if any
	}
	close(client_rulemapserver_udpsock);
	pthread_exit(nullptr);
}
