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
//#include <sys/time.h> // struct timeval

#include "helper.h"
#include "packet_format_impl.h"
#include "dpdk_helper.h"
#include "key.h"
#include "val.h"
#include "ycsb/parser.h"
#include "iniparser/iniparser_wrapper.h"

struct alignas(CACHELINE_SIZE) FGParam;

typedef FGParam fg_param_t;
typedef Key index_key_t;
typedef Val val_t;
typedef GetRequest<index_key_t> get_request_t;
typedef PutRequest<index_key_t, val_t> put_request_t;
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

// DPDK
static int run_fg(void *param); // sender
static int run_receiver(__attribute__((unused)) void *param); // receiver
struct rte_mempool *mbuf_pool = NULL;
volatile struct rte_mbuf **pkts;
volatile bool *stats;

// parameters
uint8_t src_macaddr[6];
uint8_t dst_macaddr[6];
const char *src_ipaddr;
const char *server_addr;
short src_port_start;
size_t fg_n;
//short dst_port_start = 1111;
short dst_port;
const char *workload_name;
char output_dir[256];

const uint32_t range_gap = 0x80000000; // add 2^31 to keylo of startkey
const int range_num = 10; // max number of returned kv pairs
key_t generate_endkey(key_t &startkey) {
	key_t endkey = startkey;
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
  uint64_t throughput;
  uint8_t thread_id;
};

int main(int argc, char **argv) {
  parse_ini("config.ini");

  // Prepare DPDK EAL param
  int dpdk_argc = 3;
  char **dpdk_argv;
  dpdk_argv = new char *[dpdk_argc];
  for (int i = 0; i < dpdk_argc; i++) {
	dpdk_argv[i] = new char[20];
	memset(dpdk_argv[i], '\0', 20);
  }
  std::string arg_proc = "./ycsb_remote_client";
  std::string arg_iovamode = "--iova-mode";
  std::string arg_iovamode_val = "pa";
  //std::string arg_whitelist = "-w";
  //std::string arg_whitelist_val = "0000:5e:00.1";
  memcpy(dpdk_argv[0], arg_proc.c_str(), arg_proc.size());
  memcpy(dpdk_argv[1], arg_iovamode.c_str(), arg_iovamode.size());
  memcpy(dpdk_argv[2], arg_iovamode_val.c_str(), arg_iovamode_val.size());
  //memcpy(dpdk_argv[3], arg_whitelist.c_str(), arg_whitelist.size());
  //memcpy(dpdk_argv[4], arg_whitelist_val.c_str(), arg_whitelist_val.size());

  // Init DPDK
  rte_eal_init_helper(&dpdk_argc, &dpdk_argv);
  dpdk_init(&mbuf_pool, fg_n, 1);

  // Prepare pkts and stats for receiver
  COUT_THIS("Prepare " << fg_n << "pkts and stats")
  pkts = new volatile struct rte_mbuf*[fg_n];
  stats = new volatile bool[fg_n];
  memset((void *)pkts, 0, sizeof(struct rte_mbuf *)*fg_n);
  memset((void *)stats, 0, sizeof(bool)*fg_n);
  for (size_t i = 0; i < fg_n; i++) {
	pkts[i] = rte_pktmbuf_alloc(mbuf_pool);
  }

  run_benchmark();

  // Free DPDK mbufs
  for (size_t i = 0; i < fg_n; i++) {
	rte_pktmbuf_free((struct rte_mbuf *)pkts[i]);
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
    fg_params[worker_i].throughput = 0;
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

  // TODO: process statistics
  
  // TODO: uncomment it after processing statistics
  // running = false;

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
	while (running) {
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
					if (unlikely(stats[idx])) {
						COUT_THIS("Invalid stas[" << idx << "] which is true!")
						continue;
					}
					else {
						pkts[idx] = received_pkts[i];
						stats[idx] = true;
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

  while (!running)
    ;

  while (running) {
	sent_pkt = sent_pkts[sent_pkt_idx];

	tmpkey = iter.key();
	if (iter.type() == uint8_t(packet_type_t::GET_REQ)) { // get
	  get_request_t req(thread_id, tmpkey);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string());
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // DPDK
	  encode_mbuf(sent_pkt, src_macaddr, dst_macaddr, src_ipaddr, server_addr, src_port, dst_port, buf, req_size);
	  res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
	  INVARIANT(res == 1);
	  while (!stats[thread_id])
		  ;
	  stats[thread_id] = false;
	  recv_size = get_payload(pkts[thread_id], buf);
	  rte_pktmbuf_free((struct rte_mbuf*)pkts[thread_id]);
	  INVARIANT(recv_size != -1);

	  packet_type_t pkt_type = get_packet_type(buf, recv_size);
	  INVARIANT(pkt_type == packet_type_t::GET_RES);
	  get_response_t rsp(buf, recv_size);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << rsp.key().to_string() << " val = " << rsp.val().to_string());
	}
	else if (iter.type() == uint8_t(packet_type_t::PUT_REQ)) { // update or insert
	  tmpval = iter.val();

	  put_request_t req(thread_id, tmpkey, tmpval);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string() << " val = " << req.val().to_string());
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // DPDK
	  encode_mbuf(sent_pkt, src_macaddr, dst_macaddr, src_ipaddr, server_addr, src_port, dst_port, buf, req_size);
	  res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
	  INVARIANT(res == 1);
	  while (!stats[thread_id])
		  ;
	  stats[thread_id] = false;
	  recv_size = get_payload(pkts[thread_id], buf);
	  rte_pktmbuf_free((struct rte_mbuf*)pkts[thread_id]);
	  INVARIANT(recv_size != -1);

	  packet_type_t pkt_type = get_packet_type(buf, recv_size);
	  INVARIANT(pkt_type == packet_type_t::PUT_RES);
	  put_response_t rsp(buf, recv_size);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] stat = " << rsp.stat());
	}
	else if (iter.type() == uint8_t(packet_type_t::DEL_REQ)) {
	  del_request_t req(thread_id, tmpkey);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string());
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // DPDK
	  encode_mbuf(sent_pkt, src_macaddr, dst_macaddr, src_ipaddr, server_addr, src_port, dst_port, buf, req_size);
	  res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
	  INVARIANT(res == 1);
	  while (!stats[thread_id])
		  ;
	  stats[thread_id] = false;
	  recv_size = get_payload(pkts[thread_id], buf);
	  rte_pktmbuf_free((struct rte_mbuf*)pkts[thread_id]);
	  INVARIANT(recv_size != -1);

	  packet_type_t pkt_type = get_packet_type(buf, recv_size);
	  INVARIANT(pkt_type == packet_type_t::DEL_RES);
	  del_response_t rsp(buf, recv_size);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] stat = " << rsp.stat());
	}
	else if (iter.type() == uint8_t(packet_type_t::SCAN_REQ)) {
	  index_key_t endkey = generate_endkey(tmpkey);
	  scan_request_t req(thread_id, tmpkey, endkey, range_num);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] startkey = " << tmpkey.to_string() << "endkey = " << endkey.to_string());
	  req_size = req.serialize(buf, MAX_BUFSIZE);
	}
	else {
	  printf("Invalid request type: %u\n", uint32_t(iter.type()));
	  exit(-1);
	}
    thread_param.throughput++;

	sent_pkt_idx++;
	if (sent_pkt_idx >= burst_size) {
		sent_pkt_idx = 0;
		res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
		INVARIANT(res == 0);
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
