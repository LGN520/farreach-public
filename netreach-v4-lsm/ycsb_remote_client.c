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
//#include <sys/time.h> // struct timeval

#include "helper.h"
#include "key.h"
#include "val.h"
#include "ycsb/parser.h"
#include "iniparser/iniparser_wrapper.h"
#include "crc32.h"
#include "latency_helper.h"
#include "socket_helper.h"

#include "common_impl.h"

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

volatile bool running = false;
std::atomic<size_t> ready_threads(0);
std::atomic<size_t> finish_threads(0);

int main(int argc, char **argv) {
	parse_ini("config.ini");

	run_benchmark();

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

	running = true;
	while (finish_threads < client_num) sleep(1);
	COUT_THIS("[client] all clients finish!");

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
	COUT_THIS("Client-side throughput: " << req_latency_list.size());

	running = false; // After processing statistics
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
	ParserIterator iter(load_filename);
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
		if (!iter.next()) {
			break;
		}

		struct timespec req_t1, req_t2, req_t3, rsp_t1, rsp_t2, rsp_t3, final_t3, wait_t1, wait_t2, wait_t3;
		while (true) {
			tmpkey = iter.key();
			if (iter.type() == uint8_t(packet_type_t::GETREQ)) { // get
				CUR_TIME(req_t1);
				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);
				get_request_t req(tmpkey);
				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string());
				req_size = req.serialize(buf, MAX_BUFSIZE);
				udpsendto(clientsock, buf, req_size, 0, (struct sockaddr *)&server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(req_t2);

				CUR_TIME(wait_t1);
				is_timeout = udprecvfrom(clientsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
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
			else if (iter.type() == uint8_t(packet_type_t::PUTREQ)) { // update or insert
				tmpval = iter.val();

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

				udpsendto(clientsock, buf, req_size, 0, (struct sockaddr *)&server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(req_t2);

				CUR_TIME(wait_t1);
				is_timeout = udprecvfrom(clientsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
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
			else if (iter.type() == uint8_t(packet_type_t::DELREQ)) {
				CUR_TIME(req_t1);
				//uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), netreach_key_t::model_key_size() * 8) % kv_bucket_num);
				del_request_t req(tmpkey);
				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string());
				req_size = req.serialize(buf, MAX_BUFSIZE);
				udpsendto(clientsock, buf, req_size, 0, (struct sockaddr *)&server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(req_t2);

				CUR_TIME(wait_t1);
				is_timeout = udprecvfrom(clientsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
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
			else if (iter.type() == uint8_t(packet_type_t::SCANREQ)) {
				netreach_key_t endkey = generate_endkey(tmpkey);
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
				udpsendto(clientsock, buf, req_size, 0, (struct sockaddr *)&server_addr, server_addrlen, "ycsb_remove_client");
				CUR_TIME(req_t2);

				struct timespec scan_rsp_t1, scan_rsp_t2, scan_rsp_t3;
				struct timespec scan_wait_t1, scan_wait_t2, scan_wait_t3;
				double scan_rsp_latency = 0.0, scan_wait_latency = 0.0;
				//for (size_t tmpsplit = 0; tmpsplit < split_num; tmpsplit++) {
				uint16_t received_scannum = 0;
				uint16_t max_scannum = 0;
				bool with_max_scannum = false;
				bool split_is_timeout = false;
				while (true) {
					CUR_TIME(scan_wait_t1);
					split_is_timeout = udprecvfrom(clientsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
					if (split_is_timeout) {
						is_timeout = true;
						break;
					}
					INVARIANT(recv_size > 0);
					CUR_TIME(scan_wait_t2);

					DELTA_TIME(scan_wait_t2, scan_wait_t1, scan_wait_t3);
					double tmp_scan_wait_latency = GET_MICROSECOND(scan_wait_t3);
					scan_wait_latency += tmp_scan_wait_latency;
					/*if (scan_wait_latency == 0.0 || tmp_scan_wait_latency < scan_wait_latency) {
						scan_wait_latency = tmp_scan_wait_latency;
						wait_t1 = scan_wait_t1;
						wait_t2 = scan_wait_t2;
					}*/

					CUR_TIME(scan_rsp_t1);
					scan_response_split_t rsp(buf, recv_size);
					FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] startkey = " << rsp.key().to_string()
							<< "endkey = " << rsp.endkey().to_string() << " pairnum = " << rsp.pairnum());
					received_scannum += 1;
					if (!with_max_scannum) {
						max_scannum = rsp.max_scannum();
						INVARIANT(max_scannum >= 1 && max_scannum <= server_num);
						with_max_scannum = true;
					}
					CUR_TIME(scan_rsp_t2);

					DELTA_TIME(scan_rsp_t2, scan_rsp_t1, scan_rsp_t3);
					double tmp_scan_rsp_latency = GET_MICROSECOND(scan_rsp_t3);
					scan_rsp_latency += tmp_scan_rsp_latency;
					/*if (scan_rsp_latency == 0.0 || tmp_scan_rsp_latency < scan_rsp_latency) {
						scan_rsp_latency = tmp_scan_rsp_latency;
						rsp_t1 = scan_rsp_t1;
						rsp_t2 = scan_rsp_t2;
					}*/

					if (received_scannum >= max_scannum) {
						break;
					}
				}
				if (is_timeout) {
					continue;
				}
			}
			else {
				printf("Invalid request type: %u\n", uint32_t(iter.type()));
				exit(-1);
			}
		}
		is_timeout = false;

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

	close(clientsock);
#if !defined(NDEBUGGING_LOG)
	ofs.close();
#endif
	finish_threads++;
	pthread_exit(nullptr);
	return 0;
}
