#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
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
#include "packet_format_impl.h"
#include "dpdk_helper.h"
#include "key.h"
#include "val.h"
#include "ycsb/parser.h"
#include "iniparser/iniparser_wrapper.h"
#include "crc32.h"

#define MQ_SIZE 256

const double dpdk_polling_time = 21.82; // Test by enabling TEST_DPDK_POLLING, and only transmit packets between client and switch
const size_t max_sending_rate = size_t(1.2 * 1024 * 1024); // 1.2 MQPS; limit sending rate to x (e.g., the aggregate rate of servers)
const size_t rate_limit_period = 10 * 1000; // 10 * 1000us

struct alignas(CACHELINE_SIZE) FGParam;

typedef FGParam fg_param_t;
typedef Key index_key_t;
typedef Val val_t;
typedef GetRequest<index_key_t> get_request_t;
typedef PutRequest<index_key_t, val_t> put_request_t;
typedef PutRequestLarge<index_key_t, val_t> put_request_large_t;
typedef DelRequest<index_key_t> del_request_t;
typedef ScanRequest<index_key_t> scan_request_t;
typedef GetResponse<index_key_t, val_t> get_response_t;
typedef PutResponse<index_key_t> put_response_t;
typedef DelResponse<index_key_t> del_response_t;
typedef ScanResponse<index_key_t, val_t> scan_response_t;

inline void parse_ini(const char * config_file);
void load();
void run_benchmark();
//void *run_fg(void *param); // sender
void prepare_dpdk();
void prepare_receiver();

// DPDK
static int run_fg(void *param); // sender
static int run_receiver(__attribute__((unused)) void *param); // receiver
struct rte_mempool *mbuf_pool = NULL;
//volatile struct rte_mbuf **pkts;
//volatile bool *stats;
struct rte_mbuf*** volatile pkts_list;
uint32_t* volatile heads;
uint32_t* volatile tails;

// parameters
uint8_t src_macaddr[6];
uint8_t dst_macaddr[6];
const char *src_ipaddr;
const char *server_addr;
short src_port_start;
size_t fg_n; // client num
//short dst_port_start = 1111;
short dst_port;
const char *workload_name;
char output_dir[256];
size_t per_client_per_period_max_sending_rate;
uint32_t kv_bucket_num;

// SCAN split
size_t server_num;
size_t per_server_range;
size_t get_server_idx(index_key_t key) {
	size_t server_idx = key.keylo / per_server_range;
	if (server_idx >= server_num) {
		server_idx = server_num - 1;
	}
	return server_idx;
}

const uint32_t range_gap = 1024; // add 2^10 to keylo of startkey
//const uint32_t range_gap = 0x80000000; // add 2^31 to keylo of startkey
const int range_num = 10; // max number of returned kv pairs
index_key_t generate_endkey(index_key_t &startkey) {
	index_key_t endkey = startkey;
	if (std::numeric_limits<uint64_t>::max() - endkey.keylo > range_gap) {
		endkey.keylo += range_gap;
	}
	else {
		endkey.keylo = std::numeric_limits<uint64_t>::max();
	}
	return endkey;
}

volatile bool running = false;
std::atomic<size_t> ready_threads(0);
std::atomic<size_t> finish_threads(0);

struct alignas(CACHELINE_SIZE) FGParam {
	uint8_t thread_id;
	std::vector<double> latency_list;
#ifdef TEST_DPDK_POLLING
	std::vector<double> wait_list; // TMPTMP: To count dpdk polling time
#endif
	double sum_latency;
};
#ifdef TEST_DPDK_POLLING
double receiver_sum_latency = 0.0; // TMPTMP
#endif

int main(int argc, char **argv) {
	parse_ini("config.ini");

	// Prepare DPDK EAL param and init DPDK
	prepare_dpdk();

	// Prepare pkts and stats for receiver (based on ring buffer)
	prepare_receiver();

	run_benchmark();

	// Free DPDK mbufs
	for (size_t i = 0; i < fg_n; i++) {
		while (heads[i] != tails[i]) {
			rte_pktmbuf_free((struct rte_mbuf*)pkts_list[i][tails[i]]);
			tails[i] += 1;
		}
	}
	dpdk_free();

	exit(0);
}

inline void parse_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	ini.get_client_mac(src_macaddr);
	ini.get_server_mac(dst_macaddr);
	src_ipaddr = ini.get_client_ip();
	server_addr = ini.get_server_ip();
	src_port_start = ini.get_client_port();
	fg_n = ini.get_client_num();
	dst_port = ini.get_server_port();
	workload_name = ini.get_workload_name();
	RUN_SPLIT_DIR(output_dir, workload_name, fg_n);
	server_num = ini.get_server_num();
	per_server_range = std::numeric_limits<size_t>::max() / server_num;
	per_client_per_period_max_sending_rate = max_sending_rate / fg_n / (1 * 1000 * 1000 / rate_limit_period);
	kv_bucket_num = ini.get_bucket_num();
	val_t::MAX_VALLEN = ini.get_max_vallen();
	val_t::SWITCH_MAX_VALLEN = ini.get_switch_max_vallen();

	printf("src_macaddr: ");
	for (size_t i = 0; i < 6; i++) {
		printf("%02x", src_macaddr[i]);
		if (i != 5) printf(":");
		else printf("\n");
	}
	printf("dst_macaddr: ");
	for (size_t i = 0; i < 6; i++) {
		printf("%02x", dst_macaddr[i]);
		if (i != 5) printf(":");
		else printf("\n");
	}
	printf("src_ipaddr: %s\n", src_ipaddr);
	printf("server_addr: %s\n", server_addr);
	COUT_VAR(src_port_start);
	COUT_VAR(fg_n);
	COUT_VAR(dst_port);
	COUT_VAR(workload_name);
	printf("output_dir: %s\n", output_dir);
	COUT_VAR(server_num);
	COUT_VAR(per_server_range);
	COUT_VAR(kv_bucket_num);
	COUT_VAR(val_t::MAX_VALLEN);
	COUT_VAR(val_t::SWITCH_MAX_VALLEN);
}

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
	dpdk_init(&mbuf_pool, fg_n, 1);
}

void prepare_receiver() {
	//pkts = new volatile struct rte_mbuf*[fg_n];
	//stats = new volatile bool[fg_n];
	//memset((void *)pkts, 0, sizeof(struct rte_mbuf *)*fg_n);
	//memset((void *)stats, 0, sizeof(bool)*fg_n);
	//for (size_t i = 0; i < fg_n; i++) {
	//	pkts[i] = rte_pktmbuf_alloc(mbuf_pool);
	//}
	pkts_list = new struct rte_mbuf**[fg_n];
	heads = new uint32_t[fg_n];
	tails = new uint32_t[fg_n];
	memset((void*)heads, 0, sizeof(uint32_t)*fg_n);
	memset((void*)tails, 0, sizeof(uint32_t)*fg_n);
	//int res = 0;
	for (size_t i = 0; i < fg_n; i++) {
		pkts_list[i] = new struct rte_mbuf*[MQ_SIZE];
		for (size_t j = 0; j < MQ_SIZE; j++) {
			pkts_list[i][j] = nullptr;
		}
		//res = rte_pktmbuf_alloc_bulk(mbuf_pool, pkts_list[i], MQ_SIZE);
	}
}

void run_benchmark() {
	int ret = 0;
	unsigned lcoreid = 1;

	running = false;

	// Launch receiver
	ret = rte_eal_remote_launch(run_receiver, NULL, lcoreid);
	if (ret) {
		COUT_N_EXIT("Error:" << ret);
	}
	COUT_THIS("[client] Launch receiver at lcore " << lcoreid)
	lcoreid++;

	// Prepare fg params
	//pthread_t threads[fg_n];
	fg_param_t fg_params[fg_n];
	// check if parameters are cacheline aligned
	for (size_t i = 0; i < fg_n; i++) {
		if ((uint64_t)(&(fg_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(fg_params[i]));
		}
	}

	// Launch workers
	for (uint8_t worker_i = 0; worker_i < fg_n; worker_i++) {
		fg_params[worker_i].thread_id = worker_i;
		fg_params[worker_i].latency_list.clear();
#ifdef TEST_DPDK_POLLING
		fg_params[worker_i].wait_list.clear();
#endif
		fg_params[worker_i].sum_latency = 0.0;
		/*int ret = pthread_create(&threads[worker_i], nullptr, run_fg,
														 (void *)&fg_params[worker_i]);*/
	ret = rte_eal_remote_launch(run_fg, (void *)&fg_params[worker_i], lcoreid);
		if (ret) {
			COUT_N_EXIT("Error:" << ret);
	}
	COUT_THIS("[client] Lanuch worker [" << worker_i << "] at lcore " << lcoreid)
	lcoreid++;
	if (lcoreid >= MAX_LCORE_NUM) {
		lcoreid = 1;
	}
	}

	COUT_THIS("[client] prepare workers ...");
	while (ready_threads < fg_n) sleep(1);

	running = true;
	while (finish_threads < fg_n) sleep(1);
	COUT_THIS("[client] all workers finish!");

	/* Process statistics */
	COUT_THIS("[client] processing statistics");

	// Dump latency statistics
	std::vector<double> latency_list;
#ifdef TEST_DPDK_POLLING
	std::vector<double> wait_list; // TMPTMP
#endif
	double sum_latency;
	for (size_t i = 0; i < fg_n; i++) {
		latency_list.insert(latency_list.end(), fg_params[i].latency_list.begin(), fg_params[i].latency_list.end());
#ifdef TEST_DPDK_POLLING
		wait_list.insert(wait_list.end(), fg_params[i].wait_list.begin(), fg_params[i].wait_list.end()); // TMPTMP
#endif
		sum_latency += fg_params[i].sum_latency;
	}
	double min_latency, max_latency, avg_latency, tail90_latency, tail99_latency, median_latency;
	std::sort(latency_list.begin(), latency_list.end());
	min_latency = latency_list[0];
	max_latency = latency_list[latency_list.size()-1];
	tail90_latency = latency_list[latency_list.size()*0.9];
	tail99_latency = latency_list[latency_list.size()*0.99];
	median_latency = latency_list[latency_list.size()/2];
	avg_latency = sum_latency / double(latency_list.size());
	COUT_THIS("[Latency Statistics]")
	COUT_THIS("| min | max | avg | 90th | 99th | median | sum |");
	COUT_THIS("| " << min_latency << " | " << max_latency << " | " << avg_latency << " | " << tail90_latency \
			<< " | " << tail99_latency << " | " << median_latency << " | " << sum_latency << " |");
#ifdef TEST_DPDK_POLLING
	// TMPTMP
	double avg_receiver_latency = receiver_sum_latency / double(wait_list.size());
	double median_wait, min_wait; // min_wait can be treated as the cost of dpdk itself without scheduling for PMD
	std::sort(wait_list.begin(), wait_list.end());
	min_wait = wait_list[0];
	median_wait = wait_list[wait_list.size()/2];
	// wait_time: in-switch queuing + server-side latency + dpdk overhead + client-side packet dispatching (receiver) + schedule cost for PMD (unnecessary)
	// When testing DPDK PMD scheduling cost, we only transmit packets between client and switch by 1 thread -> no in-switch ququing and server-side latency
	// median_wait: dpdk overhead + client-side packet dispatching + schedule cost for PMD
	// min_watt: dpdk overhead
	// avg_receiver_latency: client-side packet dispatching
	COUT_THIS("Ddpdk polling time: " << (median_wait - min_wait - avg_receiver_latency)); // schedule cost for PMD
#endif
	COUT_THIS("Client-side throughput: " << latency_list.size());



	running = false; // After processing statistics
	COUT_THIS("Finish dumping statistics!")
	/*void *status;
	for (size_t i = 0; i < fg_n; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error:unable to join," << rc);
		}
	}*/
	rte_eal_mp_wait_lcore();
}

static int run_receiver(void *param) {
	while (!running)
		;

	struct rte_mbuf *received_pkts[32];
#ifdef TEST_DPDK_POLLING
	struct timespec receiver_t1, receiver_t2, receiver_t3;
#endif
	while (running) {
#ifdef TEST_DPDK_POLLING
		CUR_TIME(receiver_t1);
#endif
		uint16_t n_rx = rte_eth_rx_burst(0, 0, received_pkts, 32);
		if (n_rx == 0) {
			continue;
		}
		for (size_t i = 0; i < n_rx; i++) {
			int ret = get_dstport(received_pkts[i]);
			if (unlikely(ret == -1)) {
				continue;
			}
			else {
				uint16_t received_port = (uint16_t)ret;
				int idx = received_port - src_port_start;
				if (unlikely(idx < 0 || unsigned(idx) >= fg_n)) {
					COUT_THIS("Invalid dst port received by client: %u" << received_port)
					continue;
				}
				else {
					/*if (unlikely(stats[idx])) {
						COUT_THIS("Invalid stas[" << idx << "] which is true!")
						continue;
					}
					else {
						pkts[idx] = received_pkts[i];
						stats[idx] = true;
					}*/
					if (((heads[idx] + 1) % MQ_SIZE) != tails[idx]) {
						pkts_list[idx][heads[idx]] = received_pkts[i];
						heads[idx] = (heads[idx] + 1) % MQ_SIZE;
#ifdef TEST_DPDK_POLLING
						CUR_TIME(receiver_t2);
						DELTA_TIME(receiver_t2, receiver_t1, receiver_t3);
						receiver_sum_latency += GET_MICROSECOND(receiver_t3);
#endif
					}
					else {
						COUT_THIS("Drop pkt since pkts_list["<<idx<<"] is full!")
						rte_pktmbuf_free(received_pkts[i]);
					}
				}
			}
		}
	}
	return 0;
}

static int run_fg(void *param) {
	fg_param_t &thread_param = *(fg_param_t *)param;
	uint8_t thread_id = thread_param.thread_id;

	char load_filename[256];
	memset(load_filename, '\0', 256);
	GET_SPLIT_WORKLOAD(load_filename, output_dir, thread_id);

	Parser parser(load_filename);
	ParserIterator iter = parser.begin();
	index_key_t tmpkey;
	val_t tmpval;

	int res = 0;

	// DPDK
	short src_port = src_port_start + thread_id;
	//short dst_port = dst_port_start + thread_id;
	// Optimize mbuf allocation
	uint16_t burst_size = 256;
	struct rte_mbuf *sent_pkts[burst_size];
	uint16_t sent_pkt_idx = 0;
	struct rte_mbuf *sent_pkt = NULL;
	res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
	INVARIANT(res == 0);

	// exsiting keys fall within range [delete_i, insert_i)
	char buf[MAX_BUFSIZE];
	int req_size = 0;
	int recv_size = 0;

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

	while (running) {
		sent_pkt = sent_pkts[sent_pkt_idx];

		tmpkey = iter.key();
		struct timespec req_t1, req_t2, req_t3, rsp_t1, rsp_t2, rsp_t3, final_t3;
		struct timespec wait_t1, wait_t2, wait_t3;
		if (iter.type() == uint8_t(packet_type_t::GET_REQ)) { // get
			CUR_TIME(req_t1);
			uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), index_key_t::model_key_size() * 8) % kv_bucket_num);
			get_request_t req(hashidx, tmpkey);
			FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string());
			req_size = req.serialize(buf, MAX_BUFSIZE);

			// DPDK
			encode_mbuf(sent_pkt, src_macaddr, dst_macaddr, src_ipaddr, server_addr, src_port, dst_port, buf, req_size);
			res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
			INVARIANT(res == 1);
			CUR_TIME(req_t2);

			/*while (!stats[thread_id])
				;
			stats[thread_id] = false;
			recv_size = get_payload(pkts[thread_id], buf);
			rte_pktmbuf_free((struct rte_mbuf*)pkts[thread_id]);*/
			CUR_TIME(wait_t1);
			while (heads[thread_id] == tails[thread_id])
				;
			INVARIANT(pkts_list[thread_id][tails[thread_id]] != nullptr);
			CUR_TIME(wait_t2);

			CUR_TIME(rsp_t1);
			recv_size = get_payload(pkts_list[thread_id][tails[thread_id]], buf);
			rte_pktmbuf_free((struct rte_mbuf*)pkts_list[thread_id][tails[thread_id]]);
			tails[thread_id] = (tails[thread_id] + 1) % MQ_SIZE;
			INVARIANT(recv_size != -1);

			packet_type_t pkt_type = get_packet_type(buf, recv_size);
			INVARIANT(pkt_type == packet_type_t::GET_RES);
			get_response_t rsp(buf, recv_size);
			FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << rsp.key().to_string() << " val = " << rsp.val().to_string());
			CUR_TIME(rsp_t2);
		}
		else if (iter.type() == uint8_t(packet_type_t::PUT_REQ)) { // update or insert
			tmpval = iter.val();

			CUR_TIME(req_t1);
			uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), index_key_t::model_key_size() * 8) % kv_bucket_num);

			FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string() << " val = " << req.val().to_string());
			if (tmpval.val_length <= val_t::SWITCH_MAX_VALLEN) {
				put_request_t req(hashidx, tmpkey, tmpval);
				req_size = req.serialize(buf, MAX_BUFSIZE);
			}
			else {
				put_request_large_t req(hashidx, tmpkey, tmpval);
				req_size = req.serialize(buf, MAX_BUFSIZE);
			}

			// DPDK
			encode_mbuf(sent_pkt, src_macaddr, dst_macaddr, src_ipaddr, server_addr, src_port, dst_port, buf, req_size);
			res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
			INVARIANT(res == 1);
			CUR_TIME(req_t2);

			CUR_TIME(wait_t1);
			while (heads[thread_id] == tails[thread_id])
				;
			INVARIANT(pkts_list[thread_id][tails[thread_id]] != nullptr);
			CUR_TIME(wait_t2);

			CUR_TIME(rsp_t1);
			recv_size = get_payload(pkts_list[thread_id][tails[thread_id]], buf);
			rte_pktmbuf_free((struct rte_mbuf*)pkts_list[thread_id][tails[thread_id]]);
			tails[thread_id] = (tails[thread_id] + 1) % MQ_SIZE;
			INVARIANT(recv_size != -1);

			packet_type_t pkt_type = get_packet_type(buf, recv_size);
			INVARIANT(pkt_type == packet_type_t::PUT_RES);
			put_response_t rsp(buf, recv_size);
			FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] stat = " << rsp.stat());
			CUR_TIME(rsp_t2);
		}
		else if (iter.type() == uint8_t(packet_type_t::DEL_REQ)) {
			CUR_TIME(req_t1);
			uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), index_key_t::model_key_size() * 8) % kv_bucket_num);
			del_request_t req(hashidx, tmpkey);
			FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string());
			req_size = req.serialize(buf, MAX_BUFSIZE);

			// DPDK
			encode_mbuf(sent_pkt, src_macaddr, dst_macaddr, src_ipaddr, server_addr, src_port, dst_port, buf, req_size);
			res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
			INVARIANT(res == 1);
			CUR_TIME(req_t2);

			CUR_TIME(wait_t1);
			while (heads[thread_id] == tails[thread_id])
				;
			INVARIANT(pkts_list[thread_id][tails[thread_id]] != nullptr);
			CUR_TIME(wait_t2);

			CUR_TIME(rsp_t1);
			recv_size = get_payload(pkts_list[thread_id][tails[thread_id]], buf);
			rte_pktmbuf_free((struct rte_mbuf*)pkts_list[thread_id][tails[thread_id]]);
			tails[thread_id] = (tails[thread_id] + 1) % MQ_SIZE;
			INVARIANT(recv_size != -1);

			packet_type_t pkt_type = get_packet_type(buf, recv_size);
			INVARIANT(pkt_type == packet_type_t::DEL_RES);
			del_response_t rsp(buf, recv_size);
			FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] stat = " << rsp.stat());
			CUR_TIME(rsp_t2);
		}
		else if (iter.type() == uint8_t(packet_type_t::SCAN_REQ)) {
			index_key_t endkey = generate_endkey(tmpkey);
			size_t first_server_idx = get_server_idx(tmpkey);
			size_t last_server_idx = get_server_idx(endkey);
			size_t split_num = last_server_idx - first_server_idx + 1;

			CUR_TIME(req_t1);
			uint16_t hashidx = uint16_t(crc32((unsigned char *)(&tmpkey), index_key_t::model_key_size() * 8) % kv_bucket_num);
			scan_request_t req(hashidx, tmpkey, endkey, range_num);
			FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] startkey = " << tmpkey.to_string() 
					<< "endkey = " << endkey.to_string() << " num = " << range_num);
			req_size = req.serialize(buf, MAX_BUFSIZE);
			encode_mbuf(sent_pkt, src_macaddr, dst_macaddr, src_ipaddr, server_addr, src_port, dst_port, buf, req_size);
			res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
			INVARIANT(res == 1);
			CUR_TIME(req_t2);

			struct timespec scan_rsp_t1, scan_rsp_t2, scan_rsp_t3;
			struct timespec scan_wait_t1, scan_wait_t2, scan_wait_t3;
			double scan_rsp_latency = 0.0, scan_wait_latency = 0.0;
			for (size_t tmpsplit = 0; tmpsplit < split_num; tmpsplit++) {
				CUR_TIME(scan_wait_t1);
				while (heads[thread_id] == tails[thread_id])
					;
				INVARIANT(pkts_list[thread_id][tails[thread_id]] != nullptr);
				CUR_TIME(scan_wait_t2);

				CUR_TIME(scan_rsp_t1);
				recv_size = get_payload(pkts_list[thread_id][tails[thread_id]], buf);
				rte_pktmbuf_free((struct rte_mbuf*)pkts_list[thread_id][tails[thread_id]]);
				tails[thread_id] = (tails[thread_id] + 1) % MQ_SIZE;
				INVARIANT(recv_size != -1);

				packet_type_t pkt_type = get_packet_type(buf, recv_size);
				INVARIANT(pkt_type == packet_type_t::SCAN_RES);
				scan_response_t rsp(buf, recv_size);
				FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] startkey = " << rsp.key().to_string()
						<< "endkey = " << rsp.endkey().to_string() << " num = " << rsp.num());
				CUR_TIME(scan_rsp_t2);

				DELTA_TIME(scan_rsp_t2, scan_rsp_t1, scan_rsp_t3);
				double tmp_scan_rsp_latency = GET_MICROSECOND(scan_rsp_t3);
				if (tmpsplit == 0 || tmp_scan_rsp_latency < scan_rsp_latency) {
					scan_rsp_latency = tmp_scan_rsp_latency;
					rsp_t1 = scan_rsp_t1;
					rsp_t2 = scan_rsp_t2;
				}
				DELTA_TIME(scan_wait_t2, scan_wait_t1, scan_wait_t3);
				double tmp_scan_wait_latency = GET_MICROSECOND(scan_wait_t3);
				if (tmpsplit == 0 || tmp_scan_wait_latency < scan_wait_latency) {
					scan_wait_latency = tmp_scan_wait_latency;
					wait_t1 = scan_wait_t1;
					wait_t2 = scan_wait_t2;
				}
			}
		}
		else {
			printf("Invalid request type: %u\n", uint32_t(iter.type()));
			exit(-1);
		}
		DELTA_TIME(req_t2, req_t1, req_t3);
		DELTA_TIME(rsp_t2, rsp_t1, rsp_t3);
		DELTA_TIME(wait_t2, wait_t1, wait_t3);
		SUM_TIME(req_t3, rsp_t3, final_t3); // time of sending req and receiving rsp
		double wait_time = GET_MICROSECOND(wait_t3);
		double final_time = GET_MICROSECOND(final_t3);
		if (wait_time > dpdk_polling_time) {
			// wait_time: in-switch queuing + server-side latency + dpdk overhead + client-side packet dispatching (receiver) + schedule cost for PMD (unnecessary)
			final_time += (wait_time - dpdk_polling_time); // time of in-switch queuing and server-side latency
		}

		thread_param.sum_latency += final_time;
		thread_param.latency_list.push_back(final_time);
#ifdef TEST_DPDK_POLLING
		thread_param.wait_list.push_back(wait_time); // TMPTMP
#endif

		// Rate limit (within each rate_limit_period, we can send at most per_client_per_period_max_sending_rate reqs)
		cur_sending_rate++;
		cur_sending_time += final_time;
		if (cur_sending_rate >= per_client_per_period_max_sending_rate) {
			if (cur_sending_time < rate_limit_period) {
				usleep(rate_limit_period - cur_sending_time);
			}
			cur_sending_rate = 0;
			cur_sending_time = 0.0;
		}

		sent_pkt_idx++;
		if (sent_pkt_idx >= burst_size) {
			sent_pkt_idx = 0;
			res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
			if (res < 0) {
				COUT_N_EXIT("rte_pktmbuf_alloc_bulk fails: " << res);
			}
		}

		if (!iter.next()) {
			break;
		}
	}

	if (sent_pkt_idx < burst_size) {
		for (uint16_t free_idx = sent_pkt_idx; free_idx != burst_size; free_idx++) {
			rte_pktmbuf_free(sent_pkts[free_idx]);
		}
	}

#if !defined(NDEBUGGING_LOG)
	ofs.close();
#endif
	finish_threads++;
	return 0;
}
