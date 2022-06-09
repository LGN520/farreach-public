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

#ifdef USE_YCSB
#include "workloadparser/ycsb_parser.h"
#elif defined USE_SYNTHETIC
#include "workloadparser/synthetic_parser.h"
#endif

#include "common_impl.h"

#define DUMP_BUF

struct alignas(CACHELINE_SIZE) FGParam {
	uint16_t thread_id;
	std::vector<double> req_latency_list;
	std::vector<double> rsp_latency_list;
	std::vector<double> wait_latency_list;
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
		fg_params[worker_i].req_latency_list.clear();
		fg_params[worker_i].rsp_latency_list.clear();
		fg_params[worker_i].wait_latency_list.clear();
		int ret = pthread_create(&threads[worker_i], nullptr, run_fg, (void *)&fg_params[worker_i]);
		COUT_THIS("[client] Lanuch client " << worker_i)
	}

	COUT_THIS("[client] prepare clients ...");
	while (ready_threads < client_num) sleep(1);

	// used for dynamic workload
	int persec_totalthpt[dynamic_periodnum * dynamic_periodinterval];
	memset(persec_totalthpt, 0, sizeof(int) * dynamic_periodnum * dynamic_periodinterval);

	running = true;
	if (workload_mode == 0) { // send all workloads in static mode
		while (finish_threads < client_num) sleep(1);
		COUT_THIS("[client] all clients finish!");
	}
	else { // send enough periods in dynamic mode
		int dynamicclient_udpsock = -1;
		create_udpsock(dynamicclient_udpsock, false, "client.dynamicclient");
		struct sockaddr_in dynamicserver_addr;
		set_sockaddr(dynamicserver_addr, inet_addr(server_ip_for_client), server_dynamicserver_port);
		socklen_t dynamicserver_addrlen = sizeof(struct sockaddr_in);
		char buf[MAX_BUFSIZE];
		int recvsize = 0;

		const int sleep_usecs = 1000; // 1000us = 1ms
		const int onesec_usecs = 1 * 1000 * 1000; // 1s
		int history_thpt = 0;
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
						udpsendto(dynamicclient_udpsock, &global_secidx, sizeof(int), 0, &dynamicserver_addr, dynamicserver_addrlen, "client.dynamicclient"); // notify server to count statistics of current second
						udprecvfrom(dynamicclient_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "client.dynamicclient"); // wait for ack from server.main
						INVARIANT(recvsize == sizeof(int));
						INVARIANT(*((int *)buf) == global_secidx);

						int tmp_totalthpt = 0;
						for (size_t i = 0; i < client_num; i++) {
							tmp_totalthpt += fg_params[i].req_latency_list.size();
						}
						persec_totalthpt[global_secidx] = tmp_totalthpt - history_thpt;
						history_thpt = tmp_totalthpt;
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
	running = false;
	INVARIANT(workload_mode == 0 || stop_for_dynamic_control == true);
	stop_for_dynamic_control = false;

	/* Process statistics */
	COUT_THIS("[client] processing statistics");

	// Dump latency statistics
	std::vector<double> req_latency_list, rsp_latency_list, wait_latency_list;
	for (size_t i = 0; i < client_num; i++) {
		req_latency_list.insert(req_latency_list.end(), fg_params[i].req_latency_list.begin(), fg_params[i].req_latency_list.end());
		rsp_latency_list.insert(rsp_latency_list.end(), fg_params[i].rsp_latency_list.begin(), fg_params[i].rsp_latency_list.end());
		wait_latency_list.insert(wait_latency_list.end(), fg_params[i].wait_latency_list.begin(), fg_params[i].wait_latency_list.end());
	}
	std::vector<double> total_latency_list(req_latency_list.size());
	for (size_t i = 0; i < req_latency_list.size(); i++) {
		total_latency_list[i] = req_latency_list[i] + rsp_latency_list[i] + wait_latency_list[i];
	}
	dump_latency(req_latency_list, "req_latency_list");
	dump_latency(rsp_latency_list, "rsp_latency_list");
	dump_latency(wait_latency_list, "wait_latency_list");
	COUT_THIS("Client-side total pktcnt: " << req_latency_list.size());

	if (workload_mode != 0) {
		printf("\nper-sec client total thpt:\n");
		for (size_t i = 0; i < dynamic_periodnum*dynamic_periodinterval; i++) {
			if (i != dynamic_periodnum*dynamic_periodinterval-1) {
				printf("%d ", int(persec_totalthpt[i]));
			}
			else {
				printf("%d\n", int(persec_totalthpt[i]));
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
	create_udpsock(clientsock, true, "ycsb_remote_client"); // enable timeout for client-side retry if pktloss
	struct sockaddr_in server_addr;
	set_sockaddr(server_addr, inet_addr(server_ip), server_port_start);
	socklen_t server_addrlen = sizeof(struct sockaddr_in);

	// NOTE: now we only support small range scan -> client_num * server_num * MAX_BUFSIZE memory overhead
	// TODO: we can dynamically assign memory to receive scan response in udprecvlarge_multisrc in socket_helper.c
	// TODO: if so, we can also determine number of srcs in runtime instead of using server_num or max_server_num in client side
	char *scanbufs = new char[server_num * MAX_BUFSIZE];
	int scan_recvsizes[server_num];

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

		struct timespec req_t1, req_t2, req_t3, rsp_t1, rsp_t2, rsp_t3, final_t3, wait_t1, wait_t2, wait_t3;
		while (true) {
			tmpkey = iter->key();
			printf("expected server of key %x: %d\n", tmpkey.keyhihi, tmpkey.get_rangepartition_idx(server_num));
			if (workload_mode != 0) { // change key popularity if necessary
				while (stop_for_dynamic_control) {} // stop for dynamic control between client.main and server.main
				if (unlikely(!running)) {
					break;
				}
				dynamic_rulemap_ptr->trymap(tmpkey);
			}

			if (iter->type() == uint8_t(packet_type_t::GETREQ)) { // get
				CUR_TIME(req_t1);
				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);
				get_request_t req(tmpkey);
				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string());
				req_size = req.serialize(buf, MAX_BUFSIZE);
#ifdef DUMP_BUF
				dump_buf(buf, req_size);
#endif
				udpsendto(clientsock, buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(req_t2);

				CUR_TIME(wait_t1);
				is_timeout = udprecvfrom(clientsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				if (is_timeout) {
					continue;
				}
				INVARIANT(recv_size > 0);
				CUR_TIME(wait_t2);

				CUR_TIME(rsp_t1);
				get_response_t rsp(buf, recv_size);
				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << rsp.key().to_string() << " val = " << rsp.val().to_string());
				CUR_TIME(rsp_t2);
			}
			else if (iter->type() == uint8_t(packet_type_t::PUTREQ)) { // update or insert
				tmpval = iter->val();

				CUR_TIME(req_t1);
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
				udpsendto(clientsock, buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(req_t2);

				CUR_TIME(wait_t1);
				is_timeout = udprecvfrom(clientsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				if (is_timeout) {
					continue;
				}
				INVARIANT(recv_size > 0);
				CUR_TIME(wait_t2);

				CUR_TIME(rsp_t1);
				put_response_t rsp(buf, recv_size);
				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] stat = " << rsp.stat());
				CUR_TIME(rsp_t2);
			}
			else if (iter->type() == uint8_t(packet_type_t::DELREQ)) {
				CUR_TIME(req_t1);
				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);
				del_request_t req(tmpkey);
				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string());
				req_size = req.serialize(buf, MAX_BUFSIZE);
#ifdef DUMP_BUF
				dump_buf(buf, req_size);
#endif
				udpsendto(clientsock, buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(req_t2);

				CUR_TIME(wait_t1);
				is_timeout = udprecvfrom(clientsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				if (is_timeout) {
					continue;
				}
				INVARIANT(recv_size > 0);
				CUR_TIME(wait_t2);

				CUR_TIME(rsp_t1);
				del_response_t rsp(buf, recv_size);
				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] stat = " << rsp.stat());
				CUR_TIME(rsp_t2);
			}
			else if (iter->type() == uint8_t(packet_type_t::SCANREQ)) {
				//netreach_key_t endkey = generate_endkey(tmpkey);
				netreach_key_t endkey = netreach_key_t(tmpkey.keylolo, tmpkey.keylohi, tmpkey.keyhilo, (((tmpkey.keyhihi>>16)&0xFFFF)+513)<<16); // TMPDEBUG
				/*size_t first_server_idx = get_server_idx(tmpkey);
				size_t last_server_idx = get_server_idx(endkey);
				size_t split_num = last_server_idx - first_server_idx + 1;*/

				CUR_TIME(req_t1);
				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);
				//scan_request_t req(tmpkey, endkey, range_num);
				scan_request_t req(tmpkey, endkey);
				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] startkey = " << tmpkey.to_string() 
						<< "endkey = " << endkey.to_string());
				req_size = req.serialize(buf, MAX_BUFSIZE);
#ifdef DUMP_BUF
				dump_buf(buf, req_size);
#endif
				udpsendto(clientsock, buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(req_t2);

				int received_scannum = 0;
				CUR_TIME(wait_t1);
				is_timeout = udprecvlarge_multisrc_ipfrag(clientsock, scanbufs, server_num, MAX_BUFSIZE, 0, NULL, NULL, scan_recvsizes, received_scannum, "ycsb_remote_client", scan_response_split_t::get_frag_hdrsize(), scan_response_split_t::get_srcnum_off(), scan_response_split_t::get_srcnum_len(), scan_response_split_t::get_srcnum_conversion(), scan_response_split_t::get_srcid_off(), scan_response_split_t::get_srcid_len(), scan_response_split_t::get_srcid_conversion());
				CUR_TIME(wait_t2);

				CUR_TIME(rsp_t1);
				int snapshotid = -1;
				for (int tmpscanidx = 0; tmpscanidx < received_scannum; tmpscanidx++) {
					scan_response_split_t rsp(scanbufs + tmpscanidx * MAX_BUFSIZE, scan_recvsizes[tmpscanidx]);
					FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] startkey = " << rsp.key().to_string()
							<< "endkey = " << rsp.endkey().to_string() << " pairnum = " << rsp.pairnum());
					// check scan response consistency
					if (snapshotid == -1) {
						snapshotid = rsp.snapshotid();
					}
					else if (snapshotid != rsp.snapshotid()) {
						printf("Inconsistent scan response!\n"); // TMPDEBUG
						is_timeout = true; // retry
						break;
					}
				}
				CUR_TIME(rsp_t2);

				if (is_timeout) {
					continue;
				}
			}
			else {
				printf("Invalid request type: %u\n", uint32_t(iter->type()));
				exit(-1);
			}
			break;
		} // end of while(true)
		is_timeout = false;

		if (unlikely(!running)) {
			break;
		}

		DELTA_TIME(req_t2, req_t1, req_t3);
		DELTA_TIME(rsp_t2, rsp_t1, rsp_t3);
		DELTA_TIME(wait_t2, wait_t1, wait_t3);
		double req_time = GET_MICROSECOND(req_t3);
		double rsp_time = GET_MICROSECOND(rsp_t3);
		double wait_time = GET_MICROSECOND(wait_t3);

		//SUM_TIME(req_t3, rsp_t3, final_t3); // time of sending req and receiving rsp
		//double final_time = GET_MICROSECOND(final_t3);
		double final_time = req_time + rsp_time;
		if (wait_time > dpdk_polling_time) {
			// wait_time: in-switch queuing + server-side latency + dpdk overhead + client-side packet dispatching (receiver) + schedule cost for PMD (unnecessary)
			final_time += (wait_time - dpdk_polling_time); // time of in-switch queuing and server-side latency
		}

		//thread_param.sum_latency += final_time;
		thread_param.req_latency_list.push_back(req_time);
		thread_param.rsp_latency_list.push_back(rsp_time);
		thread_param.wait_latency_list.push_back(wait_time);

		// Rate limit (within each rate_limit_period, we can send at most per_client_per_period_max_sending_rate reqs)
		cur_sending_rate++;
		cur_sending_time += final_time;
		/*if (cur_sending_rate >= per_client_per_period_max_sending_rate) {
			if (cur_sending_time < rate_limit_period) {
				usleep(rate_limit_period - cur_sending_time);
			}
			cur_sending_rate = 0;
			cur_sending_time = 0.0;
		}*/
	}

	delete [] scanbufs;

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
