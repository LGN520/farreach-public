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
#include <signal.h> // for signal and raise
#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // inetaddr conversion
#include <sys/time.h> // struct timeval

#include "helper.h"
#include "xindex.h"
#include "xindex_impl.h"
#include "xindex_util.h"
#include "packet_format_impl.h"
#include "dpdk_helper.h"
#include "key.h"
#include "val.h"

#define MQ_SIZE 256

struct alignas(CACHELINE_SIZE) SFGParam;
class Key;

typedef Key index_key_t;
typedef Val val_t;
typedef SFGParam sfg_param_t;
typedef xindex::XIndex<index_key_t, val_t> xindex_t;
typedef GetRequest<index_key_t> get_request_t;
typedef PutRequest<index_key_t, val_t> put_request_t;
typedef DelRequest<index_key_t> del_request_t;
typedef ScanRequest<index_key_t> scan_request_t;
typedef GetResponse<index_key_t, val_t> get_response_t;
typedef PutResponse<index_key_t> put_response_t;
typedef DelResponse<index_key_t> del_response_t;
typedef ScanResponse<index_key_t, val_t> scan_response_t;

inline void parse_args(int, char **);
void load();
void run_server(xindex_t *table);
//void *run_sfg(void *param);
void kill(int signum);

// DPDK
static int run_sfg(void *param); // workers
static int run_receiver(__attribute__((unused)) void *param); // receiver
struct rte_mempool *mbuf_pool = NULL;
//volatile struct rte_mbuf **pkts;
//volatile bool *stats;
volatile struct rte_mbuf ***pkts_list;
volatile uint32_t *heads;
volatile uint32_t *tails;

// parameters
size_t fg_n = 1;
size_t bg_n = 1;
short dst_port_start = 1111;

std::vector<index_key_t> exist_keys;

bool killed = false;
volatile bool running = false;
std::atomic<size_t> ready_threads(0);

struct alignas(CACHELINE_SIZE) SFGParam {
  xindex_t *table;
  uint8_t thread_id;
};

int main(int argc, char **argv) {

  parse_args(argc, argv);
  xindex::init_options(); // init options of rocksdb
  load();

  // Prepare DPDK EAL param
  int dpdk_argc = 3;
  char **dpdk_argv;
  dpdk_argv = new char *[dpdk_argc];
  for (int i = 0; i < dpdk_argc; i++) {
	dpdk_argv[i] = new char[20];
	memset(dpdk_argv[i], '\0', 20);
  }
  std::string arg_proc = "./client";
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
  //pkts = new volatile struct rte_mbuf*[fg_n];
  //stats = new volatile bool[fg_n];
  //memset((void *)pkts, 0, sizeof(struct rte_mbuf *)*fg_n);
  //memset((void *)stats, 0, sizeof(bool)*fg_n);
  //for (size_t i = 0; i < fg_n; i++) {
  //  pkts[i] = rte_pktmbuf_alloc(mbuf_pool);
  //}
  pkts_list = new volatile struct rte_mbuf**[fg_n];
  heads = new volatile uint32_t[fg_n];
  tails = new volatile uint32_t[fg_n];
  memset((void*)heads, 0, sizeof(uint32_t)*fg_n);
  memset((void*)tails, 0, sizeof(uint32_t)*fg_n);
  //int res = 0;
  for (size_t i = 0; i < fg_n; i++) {
	  pkts_list[i] = new volatile struct rte_mbuf*[MQ_SIZE];
	  for (size_t j = 0; j < MQ_SIZE; j++) {
		  pkts_list[i][j] = NULL;
	  }
	  //res = rte_pktmbuf_alloc_bulk(mbuf_pool, pkts_list[i], MQ_SIZE);
  }

  // prepare xindex
  uint64_t init_val_data[1] = {1};
  std::vector<val_t> vals(exist_keys.size(), val_t(init_val_data, 1));
  xindex_t *tab_xi = new xindex_t(exist_keys, vals, fg_n, bg_n); // fg_n to create array of RCU status; bg_n background threads have been launched

  // register signal handler
  signal(SIGTERM, SIG_IGN); // Ignore SIGTERM for subthreads

  run_server(tab_xi);
  if (tab_xi != nullptr) delete tab_xi; // terminate_bg -> bg_master joins bg_threads


  // Free DPDK mbufs
  for (size_t i = 0; i < fg_n; i++) {
	//rte_pktmbuf_free((struct rte_mbuf *)pkts[i]);
	  while (heads[i] != tails[i]) {
		  rte_pktmbuf_free((struct rte_mbuf*)pkts_list[i][tails[i]]);
		  tails[i] += 1;
	  }
  }
  dpdk_free();

  COUT_THIS("[server] Exit successfully")
  exit(0);
}

inline void parse_args(int argc, char **argv) {
  struct option long_options[] = {
      {"fg", required_argument, 0, 'h'},
      {"bg", required_argument, 0, 'i'},
      {"xindex-root-err-bound", required_argument, 0, 'j'},
      {"xindex-root-memory", required_argument, 0, 'k'},
      {"xindex-group-err-bound", required_argument, 0, 'l'},
      {"xindex-group-err-tolerance", required_argument, 0, 'm'},
      {"xindex-buf-size-bound", required_argument, 0, 'n'},
      {"xindex-buf-compact-threshold", required_argument, 0, 'o'},
      {0, 0, 0, 0}};
  std::string ops = "h:i:j:k:l:m:n:o:";
  int option_index = 0;

  while (1) {
    int c = getopt_long(argc, argv, ops.c_str(), long_options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 0:
        if (long_options[option_index].flag != 0) break;
        abort();
        break;
      case 'h':
        fg_n = strtoul(optarg, NULL, 10);
        INVARIANT(fg_n > 0);
        break;
      case 'i':
        bg_n = strtoul(optarg, NULL, 10);
        break;
      case 'j':
        xindex::config.root_error_bound = strtol(optarg, NULL, 10);
        INVARIANT(xindex::config.root_error_bound > 0);
        break;
      case 'k':
        xindex::config.root_memory_constraint =
            strtol(optarg, NULL, 10) * 1024 * 1024;
        INVARIANT(xindex::config.root_memory_constraint > 0);
        break;
      case 'l':
        xindex::config.group_error_bound = strtol(optarg, NULL, 10);
        INVARIANT(xindex::config.group_error_bound > 0);
        break;
      case 'm':
        xindex::config.group_error_tolerance = strtol(optarg, NULL, 10);
        INVARIANT(xindex::config.group_error_tolerance > 0);
        break;
      case 'n':
        xindex::config.buffer_size_bound = strtol(optarg, NULL, 10);
        INVARIANT(xindex::config.buffer_size_bound > 0);
        break;
      case 'o':
        xindex::config.buffer_compact_threshold = strtol(optarg, NULL, 10);
        INVARIANT(xindex::config.buffer_compact_threshold > 0);
        break;
      default:
        abort();
    }
  }

  COUT_VAR(fg_n);
  COUT_VAR(bg_n);
  COUT_VAR(xindex::config.root_error_bound);
  COUT_VAR(xindex::config.root_memory_constraint);
  COUT_VAR(xindex::config.group_error_bound);
  COUT_VAR(xindex::config.group_error_tolerance);
  COUT_VAR(xindex::config.buffer_size_bound);
  COUT_VAR(xindex::config.buffer_size_tolerance);
  COUT_VAR(xindex::config.buffer_compact_threshold);
}

void load() {
	std::ifstream fd("exist_keys.out", std::ios::in | std::ios::binary);
	INVARIANT(fd);
	while (true) {
		index_key_t tmp;
		fd.read((char *)&tmp, sizeof(index_key_t));
		if (likely(!fd.eof())) {
			exist_keys.push_back(tmp);
		}
		else {
			break;
		}
	}
	fd.close();
	COUT_THIS("[server] # of exist keys = " << exist_keys.size());

	/*fd.open("nonexist_keys.out", std::ios::in | std::ios::binary);
	INVARIANT(fd);
	while (true) {
		index_key_t tmp;
		fd.read((char *)&tmp, sizeof(index_key_t));
		if (likely(!fd.eof())) {
			non_exist_keys.push_back(tmp);
		}
		else {
			break;
		}
	}
	fd.close();
	COUT_THIS("[server] # of nonexist keys = " << non_exist_keys.size());*/
}

void run_server(xindex_t *table) {
	int ret = 0;
	unsigned lcoreid = 1;

	running = false;

	// Launch receiver
	ret = rte_eal_remote_launch(run_receiver, NULL, lcoreid);
	if (ret) {
		COUT_N_EXIT("Error:" << ret);
	}
	COUT_THIS("[server] Launch receiver with ret code " << ret)
	lcoreid++;

	// Prepare fg params
	//pthread_t threads[fg_n];
	sfg_param_t sfg_params[fg_n];
	// check if parameters are cacheline aligned
	for (size_t i = 0; i < fg_n; i++) {
		if ((uint64_t)(&(sfg_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(sfg_params[i]));
		}
	}

	// Launch workers
	for (uint8_t worker_i = 0; worker_i < fg_n; worker_i++) {
		sfg_params[worker_i].table = table;
		sfg_params[worker_i].thread_id = worker_i;
		//int ret = pthread_create(&threads[worker_i], nullptr, run_sfg, (void *)&sfg_params[worker_i]);
		ret = rte_eal_remote_launch(run_sfg, (void *)&sfg_params[worker_i], lcoreid);
		if (ret) {
		  COUT_N_EXIT("Error:" << ret);
		}
		lcoreid++;
		if (lcoreid >= MAX_LCORE_NUM) {
			lcoreid = 1;
		}
	}

	COUT_THIS("[server] prepare server foreground threads...")
	while (ready_threads < fg_n) sleep(1);

	signal(SIGTERM, kill); // Set for main thread

	running = true;
	COUT_THIS("[server] start running...")

	while (!killed) {
		sleep(1);
	}

	running = false;
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
		uint16_t n_rx;
		n_rx = rte_eth_rx_burst(0, 0, received_pkts, 32);
		//struct rte_eth_stats ethstats;
		//rte_eth_stats_get(0, &ethstats);

		if (n_rx == 0) continue;
		for (size_t i = 0; i < n_rx; i++) {
			int ret = get_dstport(received_pkts[i]);
			if (ret == -1) {
				continue;
			}
			else {
				uint16_t received_port = (uint16_t)ret;
				int idx = received_port - dst_port_start;
				if (idx < 0 || unsigned(idx) >= fg_n) {
					COUT_THIS("Invalid dst port received by server: %u" << received_port)
					continue;
				}
				else {
					/*if (stats[idx]) {
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

static int run_sfg(void * param) {
//void *run_sfg(void * param) {
  // Parse param
  sfg_param_t &thread_param = *(sfg_param_t *)param;
  uint8_t thread_id = thread_param.thread_id;
  xindex_t *table = thread_param.table;

  int res = 0;

  // DPDK
  //struct rte_mbuf *sent_pkt = rte_pktmbuf_alloc(mbuf_pool); // Send to DPDK port
  //struct rte_mbuf *sent_pkt_wrapper[1] = {sent_pkt};
  // Optimize mbuf allocation
  uint16_t burst_size = 256;
  struct rte_mbuf *sent_pkts[burst_size];
  uint16_t sent_pkt_idx = 0;
  struct rte_mbuf *sent_pkt = NULL;
  res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
  INVARIANT(res == 0);

  // prepare socket
  /*int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  INVARIANT(sockfd >= 0);
  int disable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
	perror("Disable udp checksum failed");
  }
  short dst_port = dst_port_start + thread_id;
  struct sockaddr_in server_sockaddr;
  memset(&server_sockaddr, 0, sizeof(struct sockaddr_in));
  server_sockaddr.sin_family = AF_INET;
  server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_sockaddr.sin_port = htons(dst_port);
  res = bind(sockfd, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr));
  INVARIANT(res != -1);
  uint32_t sockaddr_len = sizeof(struct sockaddr);

  // Set timeout
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec =  0;
  res = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  INVARIANT(res >= 0);*/

  char buf[MAX_BUFSIZE];
  int recv_size = 0;
  int rsp_size = 0;

  // DPDK
  uint8_t srcmac[6];
  uint8_t dstmac[6];
  char srcip[16];
  char dstip[16];
  uint16_t srcport;
  uint16_t dstport;

  ready_threads++;

  while (!running) {
  }

	// DEBUG
	//double prevt0 = 0;
	//double t0 = CUR_TIME();
	//uint32_t debug_idx = 0;

  while (running) {
	/*recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, (struct sockaddr *)&server_sockaddr, &sockaddr_len);
	if (recv_size == -1) {
		if (errno == EWOULDBLOCK || errno == EINTR) {
			continue; // timeout or interrupted system call
		}
		else {
			COUT_THIS("[server] Error of recvfrom: errno = " << errno)
			exit(-1);
		}
	}*/

	//if (stats[thread_id]) {
	if (heads[thread_id] != tails[thread_id]) {
		//COUT_THIS("[server] Receive packet!")

		sent_pkt = sent_pkts[sent_pkt_idx];

		// DPDK
		//stats[thread_id] = false;
		//recv_size = decode_mbuf(pkts[thread_id], srcmac, dstmac, srcip, dstip, &srcport, &dstport, buf);
		//rte_pktmbuf_free((struct rte_mbuf*)pkts[thread_id]);
		INVARIANT(pkts_list[thread_id][tails[thread_id]] != NULL);
		memset(srcip, '\0', 16);
		memset(dstip, '\0', 16);
		memset(srcmac, 0, 6);
		memset(dstmac, 0, 6);
		recv_size = decode_mbuf(pkts_list[thread_id][tails[thread_id]], srcmac, dstmac, srcip, dstip, &srcport, &dstport, buf);
		rte_pktmbuf_free((struct rte_mbuf*)pkts_list[thread_id][tails[thread_id]]);
		tails[thread_id] = (tails[thread_id] + 1) % MQ_SIZE;

		/*if ((debug_idx + 1) % 10001 == 0) {
			COUT_VAR((t0 - prevt0) / 10000.0);
			prevt0 = t0;
		}*/

		packet_type_t pkt_type = get_packet_type(buf, recv_size);
		double tmpt0 = CUR_TIME();
		switch (pkt_type) {
			case packet_type_t::GET_REQ: 
				{
					get_request_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					val_t tmp_val;
					bool tmp_stat = table->get(req.key(), tmp_val, req.thread_id());
					//COUT_THIS("[server] val = " << tmp_val.to_string())
					get_response_t rsp(req.thread_id(), req.key(), tmp_val);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);

					//res = sendto(sockfd, buf, rsp_size, 0, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr)); // UDP socket
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dstport, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::PUT_REQ:
				{
					put_request_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool tmp_stat = table->put(req.key(), req.val(), req.thread_id());
					//COUT_THIS("[server] stat = " << tmp_stat)
					put_response_t rsp(req.thread_id(), req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					//res = sendto(sockfd, buf, rsp_size, 0, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr));
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dstport, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::DEL_REQ:
				{
					del_request_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					bool tmp_stat = table->remove(req.key(), req.thread_id());
					//COUT_THIS("[server] stat = " << tmp_stat)
					del_response_t rsp(req.thread_id(), req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					//res = sendto(sockfd, buf, rsp_size, 0, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr));
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dstport, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::SCAN_REQ:
				{
					scan_request_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " num = " << req.num())
					std::vector<std::pair<index_key_t, val_t>> results;
					size_t tmp_num = table->scan(req.key(), req.num(), results, req.thread_id());
					/*COUT_THIS("[server] num = " << tmp_num)
					for (uint32_t val_i = 0; val_i < tmp_num; val_i++) {
						COUT_VAR(results[val_i].first.to_string())
						COUT_VAR(results[val_i].second.to_string())
					}*/
					scan_response_t rsp(req.thread_id(), req.key(), tmp_num, results);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					//res = sendto(sockfd, buf, rsp_size, 0, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr));
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dstport, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			default:
				{
					COUT_THIS("[server] Invalid packet type: " << int(pkt_type))
					std::cout << std::flush;
					exit(-1);
				}
		}

		if (sent_pkt_idx >= burst_size) {
			sent_pkt_idx = 0;
			res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
			INVARIANT(res == 0);
		}

		//debug_idx++;
		//double tmpt1 = CUR_TIME();
		//t0 += (tmpt1 - tmpt0);
	}
  }
  
  // DPDK
  //rte_pktmbuf_free((struct rte_mbuf*)sent_pkt);
  if (sent_pkt_idx < burst_size) {
	for (uint16_t free_idx = sent_pkt_idx; free_idx != burst_size; free_idx++) {
		rte_pktmbuf_free(sent_pkts[free_idx]);
	}
  }

  //close(sockfd);
  COUT_THIS("[thread" << uint32_t(thread_id) << "] exits")
  //pthread_exit(nullptr);
  return 0;
}

void kill(int signum) {
	COUT_THIS("[server] Receive SIGKILL!")
	killed = true;
}
