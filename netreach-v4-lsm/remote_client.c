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

#ifdef USE_YCSB
#include "workloadparser/ycsb_parser.h"
#elif defined USE_SYNTHETIC
#include "workloadparser/synthetic_parser.h"
#endif

#include "common_impl.h"

//#define DUMP_BUF

struct alignas(CACHELINE_SIZE) FGParam {
	uint16_t thread_id;
	std::map<uint16_t, int> nodeidx_pktcnt_map;
	std::vector<double> process_latency_list;
	std::vector<double> send_latency_list;
};
typedef FGParam fg_param_t;

void run_benchmark();

void * run_fg(void *param); // sender

// SCAN split
/*size_t get_server_idx(netreach_key_t key) {
#ifdef LARGE_KEY
	size_t server_idx = key.keyhihi / perserver_keyrange;
#else
	size_t server_idx = key.keyhi / perserver_keyrange;
#endif
	if (server_idx >= server_num) {
		server_idx = server_num - 1;
	}
	return server_idx;
}*/

const uint32_t range_gap = 1024; // add 2^10 to keylo of startkey
//const uint32_t range_gap = 0x80000000; // add 2^31 to keylo of startkey
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

// used for dynamic workload
dynamic_rulemap_t * dynamic_rulemap_ptr = NULL;
bool volatile stop_for_dynamic_control = false;

volatile bool running = false;
std::atomic<size_t> ready_threads(0);
std::atomic<size_t> finish_threads(0);

int main(int argc, char **argv) {
	parse_ini("config.ini");

	// prepare for clients
	if (workload_mode != 0) {
		dynamic_rulemap_ptr = new dynamic_rulemap_t(dynamic_periodnum, dynamic_ruleprefix);
		INVARIANT(dynamic_rulemap_ptr != NULL);
		dynamic_rulemap_ptr->nextperiod();
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

	// Prepare fg params
	pthread_t threads[client_num];
	fg_param_t fg_params[client_num];
	// check if parameters are cacheline aligned
	for (size_t i = 0; i < client_num; i++) {
		if ((uint64_t)(&(fg_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(fg_params[i]));
		}
	}

	// Launch workers
	for (uint16_t worker_i = 0; worker_i < client_num; worker_i++) {
		fg_params[worker_i].thread_id = worker_i;
		fg_params[worker_i].nodeidx_pktcnt_map.clear();
		fg_params[worker_i].process_latency_list.clear();
		fg_params[worker_i].send_latency_list.clear();
		int ret = pthread_create(&threads[worker_i], nullptr, run_fg, (void *)&fg_params[worker_i]);
		COUT_THIS("[client] Lanuch client " << worker_i)
	}

	COUT_THIS("[client] prepare clients ...");
	while (ready_threads < client_num) sleep(1);

	// used for dynamic workload
	std::map<uint16_t, int> persec_perclient_nodeidx_pktcnt_map[dynamic_periodnum * dynamic_periodinterval][client_num];

	struct timespec total_t1, total_t2, total_t3;
	running = true;

	std::vector<int> perinterval_totalpktcnts;
	int interval_usecs = 5 * 1000 * 1000; // 5s
	CUR_TIME(total_t1);
	if (workload_mode == 0) { // send all workloads in static mode
		while (finish_threads < client_num) {
			CUR_TIME(total_t2);
			DELTA_TIME(total_t2, total_t1, total_t3);
			if (GET_MICROSECOND(total_t3) >= (perinterval_totalpktcnts.size()+1) * interval_usecs) {
				int curinterval_totalpktcnt = 0;
				for (size_t i = 0; i < client_num; i++) {
					curinterval_totalpktcnt += fg_params[i].process_latency_list.size();
				}
				perinterval_totalpktcnts.push_back(curinterval_totalpktcnt);
			}
			usleep(10 * 1000); // 10ms
		}
	}
	else { // send enough periods in dynamic mode
		const int sleep_usecs = 1000; // 1000us = 1ms
		const int onesec_usecs = 1 * 1000 * 1000; // 1s
		struct timespec dynamic_t1, dynamic_t2, dynamic_t3;
		for (int periodidx = 0; periodidx < dynamic_periodnum; periodidx++) {
			for (int secidx = 0; secidx < dynamic_periodinterval; secidx++) { // wait for every 10 secs
				CUR_TIME(dynamic_t1);
				while (true) { // wait for every 1 sec to update thpt
					CUR_TIME(dynamic_t2);
					DELTA_TIME(dynamic_t2, dynamic_t1, dynamic_t3);

					int t3_usecs = int(GET_MICROSECOND(dynamic_t3));
					if (t3_usecs >= onesec_usecs) {
						stop_for_dynamic_control = true;

						int global_secidx = periodidx * dynamic_periodinterval + secidx;
						for (size_t i = 0; i < client_num; i++) {
							persec_perclient_nodeidx_pktcnt_map[global_secidx][i] = fg_params[i].nodeidx_pktcnt_map;
						}

						break;
					}
					else {
						usleep(sleep_usecs);
					}
				}

				if (secidx != dynamic_periodinterval - 1) { // not the last second in current period
					stop_for_dynamic_control = false;
				}
			}

			COUT_VAR(periodidx);
			INVARIANT(stop_for_dynamic_control == true);
			if (periodidx != dynamic_periodnum - 1) { // not the last period
				dynamic_rulemap_ptr->nextperiod(); // change key popularity
				stop_for_dynamic_control = false;
			}
		}
	}
	CUR_TIME(total_t2);
	DELTA_TIME(total_t2, total_t1, total_t3);
	double total_secs = GET_MICROSECOND(total_t3) / 1000.0 / 1000.0;

	running = false;
	INVARIANT(workload_mode == 0 || stop_for_dynamic_control == true);
	stop_for_dynamic_control = false;
	COUT_THIS("[client] all clients finish!");

	/* Process statistics */

	COUT_THIS("[client] processing statistics");

	// Process latency statistics
	std::map<uint16_t, int> nodeidx_pktcnt_map;
	std::vector<double> total_latency_list;
	for (size_t i = 0; i < client_num; i++) {
		for (std::map<uint16_t, int>::iterator iter = fg_params[i].nodeidx_pktcnt_map.begin(); \
				iter != fg_params[i].nodeidx_pktcnt_map.end(); iter++) {
			if (nodeidx_pktcnt_map.find(iter->first) == nodeidx_pktcnt_map.end()) {
				nodeidx_pktcnt_map.insert(*iter);
			}
			else {
				nodeidx_pktcnt_map[iter->first] += iter->second;
			}
		}
		total_latency_list.insert(total_latency_list.end(), fg_params[i].process_latency_list.begin(), fg_params[i].process_latency_list.end());
	}
	double avg_send_latency_list[client_num];
	for (size_t i = 0; i < client_num; i++) {
		avg_send_latency_list[i] = 0.0;
	}
	for (size_t i = 0; i < client_num; i++) {
		for (size_t j = 0; j < fg_params[i].send_latency_list.size(); j++) {
			avg_send_latency_list[i] += fg_params[i].send_latency_list[j];
		}
	}
	for (size_t i = 0; i < client_num; i++) {
		avg_send_latency_list[i] /= fg_params[i].send_latency_list.size();
	}

	// Dump latency statistics
	dump_latency(total_latency_list, "total_latency_list");
	printf("average send latency list: ");
	for (size_t i = 0; i < client_num; i++) {
		if (i != client_num - 1) {
			printf("%f ", avg_send_latency_list[i]);
		}
		else {
			printf("%f\n", avg_send_latency_list[i]);
		}
	}

	// Dump throughput statistics
	printf("thpt interval: %d s\n", interval_usecs/1000/1000);
	int previnterval_pktcnt = 0;
	for (size_t i = 0; i < perinterval_totalpktcnts.size(); i++) {
		printf("interval[%d]: throughput = %f MOPS\n", i, double(perinterval_totalpktcnts[i] - previnterval_pktcnt)/(interval_usecs/1000/1000)/1024.0/1024.0);
		previnterval_pktcnt = perinterval_totalpktcnts[i];
	}
	int total_pktcnt = total_latency_list.size();
	printf("client-side total pktcnt: %d, total time: %f s, total thpt: %f MOPS\n", total_pktcnt, total_secs, double(total_pktcnt) / total_secs / 1024.0 / 1024.0);
	
	// WRONG way of calculating system average throughput (not including 0ops if client finishes before system running time)
	/*double total_thpt = 0.0;
	for (size_t i = 0; i < client_num; i++) {
		double curclient_thpt = 0.0;
		int curclient_pktcnt = fg_params[i].req_latency_list.size();
		double curclient_totalusecs = 0.0;
		for (size_t j = 0; j < curclient_pktcnt; j++) {
			curclient_totalusecs += (fg_params[i].req_latency_list[j] + fg_params[i].rsp_latency_list[j] + fg_params[i].wait_latency_list[j]);
		}
		double curclient_totalsecs = curclient_totalusecs / 1000.0 / 1000.0;
		curclient_thpt = double(curclient_pktcnt) / curclient_totalsecs / 1024.0 / 1024.0;
		total_thpt += curclient_thpt;
	}
	printf("client-side total pktcnt: %d, total thpt: %f MOPS\n", total_pktcnt, total_thpt);*/

	COUT_THIS("cache hit pktcnt: " << nodeidx_pktcnt_map[0xFFFF]);
	printf("per-server pktcnt: ");
	for (uint16_t i = 0; i < server_num; i++) {
		if (i != server_num - 1) {
			printf("%d ", nodeidx_pktcnt_map[i]);
		}
		else {
			printf("%d\n", nodeidx_pktcnt_map[i]);
		}
	}

	if (workload_mode != 0) {
		// get persec nodeidx-pktcnt mapping data
		std::map<uint16_t, int> persec_nodeidx_pktcnt_map[dynamic_periodnum*dynamic_periodinterval];
		// aggregate perclient pktcnt for each previous i seconds -> accumulative pktcnt for each previous i seconds
		for (size_t i = 0; i < dynamic_periodnum*dynamic_periodinterval; i++) {
			for (size_t j = 0; j < client_num; j++) {
				for (std::map<uint16_t, int>::iterator iter = persec_perclient_nodeidx_pktcnt_map[i][j].begin(); \
						iter != persec_perclient_nodeidx_pktcnt_map[i][j].end(); iter++) {
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
		for (size_t i = dynamic_periodnum*dynamic_periodinterval-1; i > 0; i--) {
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
		int persec_totalpktcnt_list[dynamic_periodnum*dynamic_periodinterval];
		memset(persec_totalpktcnt_list, 0, dynamic_periodnum*dynamic_periodinterval*sizeof(int));
		for (size_t i = 0; i < dynamic_periodnum*dynamic_periodinterval; i++) {
			for (std::map<uint16_t, int>::iterator iter = persec_nodeidx_pktcnt_map[i].begin(); \
					iter != persec_nodeidx_pktcnt_map[i].end(); iter++) {
				persec_totalpktcnt_list[i] += iter->second;
			}
		}
		printf("\nper-sec total throughput:\n");
		for (size_t i = 0; i < dynamic_periodnum*dynamic_periodinterval; i++) {
			if (i != dynamic_periodnum * dynamic_periodinterval - 1) {
				printf("%d ", persec_totalpktcnt_list[i]);
			}
			else {
				printf("%d\n", persec_totalpktcnt_list[i]);
			}
		}
		printf("\nper-sec per-server throughput:\n");
		for (size_t i = 0; i < dynamic_periodnum*dynamic_periodinterval; i++) {
			for (uint16_t j = 0; j < server_num; j++) {
				int tmppktcnt = 0;
				if (persec_nodeidx_pktcnt_map[i].find(j) != persec_nodeidx_pktcnt_map[i].end()) {
					tmppktcnt = persec_nodeidx_pktcnt_map[i][j];
				}
				if (j != server_num - 1) {
					printf("%d ", tmppktcnt);
				}
				else {
					printf("%d\n", tmppktcnt);
				}
			}
		}
	}

	COUT_THIS("Finish dumping statistics!")
	void *status;
	for (size_t i = 0; i < client_num; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error:unable to join " << rc);
		}
	}
}

void *run_fg(void *param) {
	fg_param_t &thread_param = *(fg_param_t *)param;
	uint16_t thread_id = thread_param.thread_id;

	int res = 0;
	short src_port = client_port_start + thread_id;
	//short server_port = server_port_start + thread_id;

	// ycsb parser
	char load_filename[256];
	memset(load_filename, '\0', 256);
	RUN_SPLIT_WORKLOAD(load_filename, client_workload_dir, thread_id);

	ParserIterator *iter = NULL;
#ifdef USE_YCSB
	iter = new YcsbParserIterator(load_filename);
#elif defined USE_SYNTHETIC
	iter = new SyntheticParserIterator(load_filename);
#endif
	INVARIANT(iter != NULL);

	netreach_key_t tmpkey;
	val_t tmpval;

	// for network communication
	char buf[MAX_BUFSIZE];
	int req_size = 0;
	int recv_size = 0;
	int clientsock = -1;
	//create_udpsock(clientsock, true, "ycsb_remote_client", 0, 1 * 1000, UDP_LARGE_RCVBUFSIZE); // enable timeout for client-side retry if pktloss
	create_udpsock(clientsock, true, "ycsb_remote_client", 0, CLIENT_SOCKET_TIMEOUT_USECS, UDP_LARGE_RCVBUFSIZE); // enable timeout for client-side retry if pktloss
	struct sockaddr_in server_addr;
	set_sockaddr(server_addr, inet_addr(server_ip), server_port_start);
	socklen_t server_addrlen = sizeof(struct sockaddr_in);

	// DEPRECATED: we only support small range scan -> client_num * server_num * MAX_BUFSIZE memory overhead
	// Now we dynamically assign memory to receive scan response in udprecvlarge_multisrc in socket_helper.c
	// We also determine number of srcs in runtime instead of using server_num or max_server_num in client side
	//char *scanbufs = new char[server_num * MAX_BUFSIZE];
	//int scan_recvsizes[server_num];

#if !defined(NDEBUGGING_LOG)
	std::string logname;
	GET_STRING(logname, "tmp_client"<< uint32_t(thread_id)<<".out");
	std::ofstream ofs(logname, std::ofstream::out);
#endif

	COUT_THIS("[client " << uint32_t(thread_id) << "] Ready.");
	ready_threads++;

	// For rate limit
	double cur_sending_time = 0.0; // Set to 0 periodically
	size_t cur_sending_rate = 0;

	while (!running)
		;

	bool is_timeout = false;
	while (running) {
		if (!iter->next()) {
			break;
		}

		struct timespec process_t1, process_t2, process_t3, send_t1, send_t2, send_t3;
		uint16_t tmp_nodeidx_foreval = 0;
		while (true) { // timeout-and-retry mechanism
			CUR_TIME(process_t1);

			tmpkey = iter->key();
			//printf("expected server of key %x: %d\n", tmpkey.keyhihi, tmpkey.get_hashpartition_idx(server_num));
			//printf("expected server of key %x: %d\n", tmpkey.keyhihi, tmpkey.get_rangepartition_idx(server_num));
			if (workload_mode != 0) { // change key popularity if necessary
				while (stop_for_dynamic_control) {} // stop for dynamic control between client.main and server.main
				if (unlikely(!running)) {
					break;
				}
				dynamic_rulemap_ptr->trymap(tmpkey);
			}

			if (iter->type() == uint8_t(packet_type_t::GETREQ)) { // get
				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);
				get_request_t req(tmpkey);
				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string());
				req_size = req.serialize(buf, MAX_BUFSIZE);
#ifdef DUMP_BUF
				dump_buf(buf, req_size);
#endif
				CUR_TIME(send_t1);
				udpsendto(clientsock, buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(send_t2);

				// filter unmatched responses to fix duplicate responses of previous request due to false positive timeout-and-retry
				while (true) {
					is_timeout = udprecvfrom(clientsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
#ifdef DUMP_BUF
					dump_buf(buf, recv_size);
#endif
					if (!is_timeout) {
						INVARIANT(recv_size > 0);
						if (*((uint8_t *)buf) != uint8_t(packet_type_t::GETRES)) {
							continue; // continue to receive next packet
						}
						else {
							get_response_t rsp(buf, recv_size);
							if (rsp.key() != tmpkey) {
								continue; // continue to receive next packet
							}
							else {
								tmp_nodeidx_foreval = rsp.nodeidx_foreval();
								FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << rsp.key().to_string() << " val = " << rsp.val().to_string());
								break; // break to update statistics and send next packet
							}
						}
					}
					else {
						break; // break to resend
					}
				}
				if (is_timeout) {
					continue; // continue to resend
				}
			}
			else if (iter->type() == uint8_t(packet_type_t::PUTREQ)) { // update or insert
				tmpval = iter->val();

				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);

				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string() << " val = " << req.val().to_string());
				INVARIANT(tmpval.val_length <= val_t::SWITCH_MAX_VALLEN);
				put_request_t req(tmpkey, tmpval);
				req_size = req.serialize(buf, MAX_BUFSIZE);
				/*if (tmpval.val_length <= val_t::SWITCH_MAX_VALLEN) {
					put_request_t req(hashidx, tmpkey, tmpval);
					req_size = req.serialize(buf, MAX_BUFSIZE);
				}
				else {
					put_request_large_t req(hashidx, tmpkey, tmpval);
					req_size = req.serialize(buf, MAX_BUFSIZE);
				}*/

#ifdef DUMP_BUF
				dump_buf(buf, req_size);
#endif
				CUR_TIME(send_t1);
				udpsendto(clientsock, buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(send_t2);

				// filter unmatched responses to fix duplicate responses of previous request due to false positive timeout-and-retry
				while (true) {
					is_timeout = udprecvfrom(clientsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
#ifdef DUMP_BUF
					dump_buf(buf, recv_size);
#endif
					if (!is_timeout) {
						INVARIANT(recv_size > 0);
						if (*((uint8_t *)buf) != uint8_t(packet_type_t::PUTRES)) {
							continue; // continue to receive next packet
						}
						else {
							put_response_t rsp(buf, recv_size);
							if (rsp.key() != tmpkey) {
								continue; // continue to receive next packet
							}
							else {
								tmp_nodeidx_foreval = rsp.nodeidx_foreval();
								FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] stat = " << rsp.stat());
								break; // break to update statistics and send next packet
							}
						}
					}
					else {
						break; // break to resend
					}
				}
				if (is_timeout) {
					continue; // continue to resend
				}
			}
			else if (iter->type() == uint8_t(packet_type_t::DELREQ)) {
				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);
				del_request_t req(tmpkey);
				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string());
				req_size = req.serialize(buf, MAX_BUFSIZE);
#ifdef DUMP_BUF
				dump_buf(buf, req_size);
#endif
				CUR_TIME(send_t1);
				udpsendto(clientsock, buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(send_t2);

				// filter unmatched responses to fix duplicate responses of previous request due to false positive timeout-and-retry
				while (true) {
					is_timeout = udprecvfrom(clientsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
#ifdef DUMP_BUF
					dump_buf(buf, recv_size);
#endif
					if (!is_timeout) {
						INVARIANT(recv_size > 0);
						if (*((uint8_t *)buf) != uint8_t(packet_type_t::DELRES)) {
							continue; // continue to receive next packet
						}
						else {
							del_response_t rsp(buf, recv_size);
							if (rsp.key() != tmpkey) {
								continue; // continue to receive next packet
							}
							else {
								tmp_nodeidx_foreval = rsp.nodeidx_foreval();
								FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] stat = " << rsp.stat());
								break; // break to update statistics and send next packet
							}
						}
					}
					else {
						break; // break to resend
					}
				}
				if (is_timeout) {
					continue; // continue to resend
				}
			}
			else if (iter->type() == uint8_t(packet_type_t::SCANREQ)) {
				//netreach_key_t endkey = generate_endkey(tmpkey);
				netreach_key_t endkey = netreach_key_t(tmpkey.keylolo, tmpkey.keylohi, tmpkey.keyhilo, (((tmpkey.keyhihi>>16)&0xFFFF)+513)<<16); // TMPDEBUG
				/*size_t first_server_idx = get_server_idx(tmpkey);
				size_t last_server_idx = get_server_idx(endkey);
				size_t split_num = last_server_idx - first_server_idx + 1;*/

				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);
				//scan_request_t req(tmpkey, endkey, range_num);
				scan_request_t req(tmpkey, endkey);
				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] startkey = " << tmpkey.to_string() 
						<< "endkey = " << endkey.to_string());
				req_size = req.serialize(buf, MAX_BUFSIZE);
#ifdef DUMP_BUF
				dump_buf(buf, req_size);
#endif
				CUR_TIME(send_t1);
				udpsendto(clientsock, buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(send_t2);

				size_t received_scannum = 0;
				dynamic_array_t *scanbufs = NULL;
				set_recvtimeout(clientsock, CLIENT_SCAN_SOCKET_TIMEOUT_SECS, 0); // 10s for SCAN
				//is_timeout = udprecvlarge_multisrc_ipfrag(clientsock, scanbufs, server_num, MAX_BUFSIZE, 0, NULL, NULL, scan_recvsizes, received_scannum, "ycsb_remote_client", scan_response_split_t::get_frag_hdrsize(), scan_response_split_t::get_srcnum_off(), scan_response_split_t::get_srcnum_len(), scan_response_split_t::get_srcnum_conversion(), scan_response_split_t::get_srcid_off(), scan_response_split_t::get_srcid_len(), scan_response_split_t::get_srcid_conversion());
				is_timeout = udprecvlarge_multisrc_ipfrag(clientsock, &scanbufs, received_scannum, 0, NULL, NULL, "ycsb_remote_client", scan_response_split_t::get_frag_hdrsize(), scan_response_split_t::get_srcnum_off(), scan_response_split_t::get_srcnum_len(), scan_response_split_t::get_srcnum_conversion(), scan_response_split_t::get_srcid_off(), scan_response_split_t::get_srcid_len(), scan_response_split_t::get_srcid_conversion(), true, uint8_t(packet_type_t::SCANRES_SPLIT), tmpkey);
				set_recvtimeout(clientsock, 0, CLIENT_SOCKET_TIMEOUT_USECS); // 100ms for other reqs
				if (is_timeout) {
					continue;
				}

				int snapshotid = -1;
				int totalnum = 0;
				for (int tmpscanidx = 0; tmpscanidx < received_scannum; tmpscanidx++) {
					//scan_response_split_t rsp(scanbufs + tmpscanidx * MAX_BUFSIZE, scan_recvsizes[tmpscanidx]);
					scan_response_split_t rsp(scanbufs[tmpscanidx].array(), scanbufs[tmpscanidx].size());
					FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] startkey = " << rsp.key().to_string()
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
				COUT_VAR(received_scannum);
				COUT_VAR(totalnum);

				if (scanbufs != NULL) {
					delete [] scanbufs;
					scanbufs = NULL;
				}
			}
			else {
				printf("Invalid request type: %u\n", uint32_t(iter->type()));
				exit(-1);
			}
			break;
		} // end of while(true)
		is_timeout = false;
		CUR_TIME(process_t2);

		if (unlikely(!running)) {
			break;
		}

		INVARIANT(tmp_nodeidx_foreval == 0xFFFF || tmp_nodeidx_foreval < server_num);
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

	iter->closeiter();
	delete iter;
	iter = NULL;

	close(clientsock);
#if !defined(NDEBUGGING_LOG)
	ofs.close();
#endif
	finish_threads++;
	pthread_exit(nullptr);
	return 0;
}
