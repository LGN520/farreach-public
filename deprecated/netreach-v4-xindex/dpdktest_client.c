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

#include "dpdk_helper.h"
#include "key.h"
#include "val.h"

#include "common_impl.h"
#include "latency_helper.h"

//#define KEYVALUE

size_t pktcnt = 10000;
std::vector<double> req_latency_list;
std::vector<double> rsp_latency_list;
std::vector<double> wait_latency_list;

void prepare_dpdk();
void run_benchmark();

int main(int argc, char **argv) {
	parse_ini("config.ini");

	// Prepare DPDK EAL param and init DPDK
	prepare_dpdk();

	run_benchmark();

	dump_latency(req_latency_list, "req_latency_list");
	dump_latency(rsp_latency_list, "rsp_latency_list");
	dump_latency(wait_latency_list, "wait_latency_list");

	dpdk_free();

	exit(0);
}

void prepare_dpdk() {
	int dpdk_argc = 5;
	char **dpdk_argv;
	dpdk_argv = new char *[dpdk_argc];
	for (int i = 0; i < dpdk_argc; i++) {
		dpdk_argv[i] = new char[20];
		memset(dpdk_argv[i], '\0', 20);
	}
	std::string arg_proc = "./ycsb_remote_client";
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
	dpdk_port_init(0, 1, 1);
}

void run_benchmark() {
#ifdef KEYVALUE
	index_key_t testkey(1, 1, 1, 1);
	char valbytes[Val::SWITCH_MAX_VALLEN];
	memset(valbytes, 0xff, Val::SWITCH_MAX_VALLEN);
	val_t testval(valbytes, Val::SWITCH_MAX_VALLEN);
#else
	int nohit_payload = 0;
	int hit_payload = 1;
	//int payloadvalue = nohit_payload;
	int payloadvalue = hit_payload;
#endif

	int res = 0;
	short src_port = client_port_start;

	// DPDK
	struct rte_mempool *tx_mbufpool = NULL;
	dpdk_queue_setup(0, 0, &tx_mbufpool);
	INVARIANT(tx_mbufpool != NULL);
	// Optimize mbuf allocation
	uint16_t burst_size = 256;
	struct rte_mbuf *sent_pkts[burst_size];
	uint16_t sent_pkt_idx = 0;
	struct rte_mbuf *sent_pkt = NULL;
	res = rte_pktmbuf_alloc_bulk(tx_mbufpool, sent_pkts, burst_size);
	INVARIANT(res == 0);
	struct rte_mbuf *received_pkts[1];
	dpdk_port_start(0);

	// exsiting keys fall within range [delete_i, insert_i)
	char buf[MAX_BUFSIZE];
	int req_size = 0;
	int recv_size = 0;
	int pkttype = 0; // 0: GETREQ; 1: PUTREQ
	for (size_t i = 0; i < pktcnt; i++) {
		sent_pkt = sent_pkts[sent_pkt_idx];

		struct timespec req_t1, req_t2, req_t3, rsp_t1, rsp_t2, rsp_t3, final_t3;
		struct timespec wait_t1, wait_t2, wait_t3;

		CUR_TIME(req_t1);
#ifdef KEYVALUE
		if (pkttype == 0) { // GETREQ
			get_request_t req(testkey);
			req_size = req.serialize(buf, MAX_BUFSIZE);
		}
#else
		int bigendian_payloadvalue = int(ntohl(uint32_t(payloadvalue)));
		memcpy(buf, (char *)&bigendian_payloadvalue, sizeof(int));
		req_size = sizeof(int);
#endif

		// DPDK
		encode_mbuf(sent_pkt, client_macaddr, server_macaddr, client_ip, server_ip, src_port, server_port_start, buf, req_size);
		res = rte_eth_tx_burst(0, 0, &sent_pkt, 1);
		INVARIANT(res == 1);
		CUR_TIME(req_t2);

		CUR_TIME(wait_t1);
		uint16_t n_rx = 0;
		while (true) {
			n_rx = rte_eth_rx_burst(0, 0, received_pkts, 1);
			if (n_rx == 0) {
				continue;
			}
			else {
				break;
			}
		}
		INVARIANT(n_rx == 1);
		CUR_TIME(wait_t2);

		CUR_TIME(rsp_t1);
		recv_size = get_payload(received_pkts[0], buf);
		rte_pktmbuf_free((struct rte_mbuf*)received_pkts[0]);
		received_pkts[0] = NULL;
		INVARIANT(recv_size != -1);
#ifdef KEYVALUE
		if (pkttype == 0) { // GETREQ
			get_response_t rsp(buf, recv_size);
		}
#else
		INVARIANT(recv_size == sizeof(int));
		INVARIANT(*((int *)buf) == bigendian_payloadvalue);
#endif
		CUR_TIME(rsp_t2);

		DELTA_TIME(req_t2, req_t1, req_t3);
		DELTA_TIME(rsp_t2, rsp_t1, rsp_t3);
		DELTA_TIME(wait_t2, wait_t1, wait_t3);
		double req_time = GET_MICROSECOND(req_t3);
		double rsp_time = GET_MICROSECOND(rsp_t3);
		double wait_time = GET_MICROSECOND(wait_t3);
		req_latency_list.push_back(req_time);
		rsp_latency_list.push_back(rsp_time);
		wait_latency_list.push_back(wait_time);

		sent_pkt_idx++;
		if (sent_pkt_idx >= burst_size) {
			sent_pkt_idx = 0;
			res = rte_pktmbuf_alloc_bulk(tx_mbufpool, sent_pkts, burst_size);
			if (res < 0) {
				COUT_N_EXIT("rte_pktmbuf_alloc_bulk fails: " << res);
			}
		}
	}

	if (sent_pkt_idx < burst_size) {
		for (uint16_t free_idx = sent_pkt_idx; free_idx != burst_size; free_idx++) {
			rte_pktmbuf_free(sent_pkts[free_idx]);
		}
	}
}
