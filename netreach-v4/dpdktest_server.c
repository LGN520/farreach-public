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
#include "dpdk_helper.h"

#include "common_impl.h"
#include "latency_helper.h"

//#define KEYVALUE

size_t pktcnt = 10000;
std::vector<double> wait_latency_list;
std::vector<double> process_latency_list;

void prepare_dpdk();
void run_server();

int main(int argc, char **argv) {
  parse_ini("config.ini");
  
  prepare_dpdk();

  run_server();

  dump_latency(wait_latency_list, "wait_latency_list");
  dump_latency(process_latency_list, "process_latency_list");

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
  
  dpdk_eal_init(&dpdk_argc, &dpdk_argv); // Init DPDK
  dpdk_port_init(0, 1, 1);
}

void run_server() {
  index_key_t testkey(1, 1, 1, 1);
  char valbytes[Val::SWITCH_MAX_VALLEN];
  memset(valbytes, 0xff, Val::SWITCH_MAX_VALLEN);
  val_t testval(valbytes, Val::SWITCH_MAX_VALLEN);

  int res = 0;

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

  char buf[MAX_BUFSIZE];
  int recv_size = 0;
  int rsp_size = 0;

  // packet headers
  uint8_t srcmac[6];
  uint8_t dstmac[6];
  char srcip[16];
  char dstip[16];
  uint16_t srcport;
  uint16_t unused_dstport; // we use server_port_start instead of received dstport to hide server-side partition for client

  size_t pktidx = 0;
  while (true) {
	struct timespec t1, t2, t3, wait_t1, wait_t2, wait_t3;

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

	CUR_TIME(t1);
	sent_pkt = sent_pkts[sent_pkt_idx];
	INVARIANT(received_pkts[0] != NULL);
	memset(srcip, '\0', 16);
	memset(dstip, '\0', 16);
	memset(srcmac, 0, 6);
	memset(dstmac, 0, 6);
	recv_size = decode_mbuf(received_pkts[0], srcmac, dstmac, srcip, dstip, &srcport, &unused_dstport, buf);
	rte_pktmbuf_free((struct rte_mbuf*)received_pkts[0]);
	received_pkts[0] = NULL;

#ifdef KEYVALUE
	packet_type_t pkt_type = get_packet_type(buf, recv_size);
	switch (pkt_type) {
		case packet_type_t::GETREQ: 
			{
				get_request_t req(buf, recv_size);
				get_response_t rsp(req.key(), testval, true);
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				
				// DPDK
				encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
				res = rte_eth_tx_burst(0, 0, &sent_pkt, 1);
				sent_pkt_idx++;
				break;
			}
		case packet_type_t::PUTREQ_SEQ:
			{
				put_request_seq_t req(buf, recv_size);
				put_response_t rsp(req.key(), true);
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				
				// DPDK
				encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
				res = rte_eth_tx_burst(0, 0, &sent_pkt, 1);
				sent_pkt_idx++;
				break;
			}
	}
#else
	// DPDK
	encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, recv_size);
	res = rte_eth_tx_burst(0, 0, &sent_pkt, 1);
	sent_pkt_idx++;
#endif
	CUR_TIME(t2);

	DELTA_TIME(wait_t2, wait_t1, wait_t3);
	DELTA_TIME(t2, t1, t3);
	wait_latency_list.push_back(GET_MICROSECOND(wait_t3));
	process_latency_list.push_back(GET_MICROSECOND(t3));

	if (sent_pkt_idx >= burst_size) {
		sent_pkt_idx = 0;
		res = rte_pktmbuf_alloc_bulk(tx_mbufpool, sent_pkts, burst_size);
		INVARIANT(res == 0);
	}

	pktidx += 1;
	if (pktidx >= pktcnt) {
		break;
	}
  }
  
  // DPDK
  if (sent_pkt_idx < burst_size) {
	for (uint16_t free_idx = sent_pkt_idx; free_idx != burst_size; free_idx++) {
		rte_pktmbuf_free(sent_pkts[free_idx]);
	}
  }
}
