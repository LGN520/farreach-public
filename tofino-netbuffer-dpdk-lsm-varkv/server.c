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

#define MQ_SIZE 256
#define KV_BUCKET_COUT 32768
//#define KV_BUCKET_COUT 65536
#define MAX_VERSION 0xFFFFFFFFFFFFFFFF

struct alignas(CACHELINE_SIZE) SFGParam;

typedef Key index_key_t;
typedef uint64_t val_t;
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
typedef PutRequestS<index_key_t, val_t> put_request_s_t;
typedef DelRequestS<index_key_t> del_request_s_t;

inline void parse_args(int, char **);
void load();
void run_server(xindex_t *table);
//void *run_sfg(void *param);
void kill(int signum);

// DPDK
static int run_sfg(void *param);
static int run_receiver(__attribute__((unused)) void *param); // receiver
struct rte_mempool *mbuf_pool = NULL;
//volatile struct rte_mbuf **pkts;
//volatile bool *stats;
volatile struct rte_mbuf ***pkts_list;
uint32_t* volatile heads;
uint32_t* volatile tails;

void *run_backuper(__attribute__((unused)) void *param); // backuper
std::map<index_key_t, val_t>* volatile backup_data = nullptr;
uint32_t* volatile backup_rcu;

void *run_listener(__attribute__((unused)) void *param); // listener to listen the responses of KV pull requests for SCAN
std::map<index_key_t, val_t>* volatile listener_data = nullptr;
volatile uint64_t listener_version = 0;

// parameters
size_t fg_n = 1;
size_t bg_n = 1;
short dst_port_start = 1111;
short backup_port = 3333;
short controller_port = 3334;
short listener_port = 3335;
char controller_ip[20] = "172.16.112.19";

std::vector<index_key_t> exist_keys;

bool killed = false;
volatile bool running = false;
std::atomic<size_t> ready_threads(0);

struct alignas(CACHELINE_SIZE) SFGParam {
  xindex_t *table;
  uint32_t thread_id;
};

void test_merge_latency() {
	backup_data = new std::map<index_key_t, val_t>;
	for (size_t i = 0; i < 64*1024; i++) {
		backup_data->insert(std::pair<index_key_t, val_t>(exist_keys[i], 1));
	}
}

int main(int argc, char **argv) {
  test_merge_latency(); // DEBUG test

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
  heads = new uint32_t[fg_n];
  tails = new uint32_t[fg_n];
  memset((void*)heads, 0, sizeof(uint32_t)*fg_n);
  memset((void*)tails, 0, sizeof(uint32_t)*fg_n);
  //int res = 0;
  for (size_t i = 0; i < fg_n; i++) {
	  pkts_list[i] = new volatile struct rte_mbuf*[MQ_SIZE];
	  for (size_t j = 0; j < MQ_SIZE; j++) {
		  pkts_list[i][j] = nullptr;
	  }
	  //res = rte_pktmbuf_alloc_bulk(mbuf_pool, pkts_list[i], MQ_SIZE);
  }

  backup_rcu = new uint32_t[fg_n];
  memset((void *)backup_rcu, 0, fg_n * sizeof(uint32_t));

  // prepare xindex
  std::vector<val_t> vals(exist_keys.size(), 1);
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

	// Launch backuper
	pthread_t backuper_thread;
	ret = pthread_create(&backuper_thread, nullptr, run_backuper, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: unable to create backuper " << ret);
	}

	// Launch listener
	pthread_t listener_thread;
	ret = pthread_create(&listener_thread, nullptr, run_listener, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: unable to create listener " << ret);
	}

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
	for (size_t worker_i = 0; worker_i < fg_n; worker_i++) {
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
	void * status;
	int rc = pthread_join(backuper_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join," << rc);
	}
	rc = pthread_join(listener_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join," << rc);
	}
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

void *run_backuper(void *param) {

	int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	INVARIANT(sock_fd >= 0);
	// Diskable udp check
	int disable = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		COUT_N_EXIT("Disable udp checksum failed");
	}
	// Set timeout
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec =  0;
	int res = setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	INVARIANT(res >= 0);
	// Set listen addr
	struct sockaddr_in backup_server_addr;
	memset(&backup_server_addr, 0, sizeof(sockaddr_in));
	backup_server_addr.sin_family = AF_INET;
	backup_server_addr.sin_port = htons(backup_port);
	backup_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	res = bind(sock_fd, (struct sockaddr *)&backup_server_addr, sizeof(backup_server_addr));
	INVARIANT(res >= 0);
	
	int recv_size = 0;
	char recv_buf[KV_BUCKET_COUT * 26]; // n * 25 + 14 + 20 + 8 < n * 26
	memset(recv_buf, 0, sizeof(recv_buf));

	while (!running)
		;

	while (running) {
		recv_size = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, nullptr, nullptr);
		if (recv_size == -1) {
			if (errno == EWOULDBLOCK || errno == EINTR) {
				continue; // timeout or interrupted system call
			}
			else {
				COUT_N_EXIT("[server] Error of recvfrom: errno = " << errno);
			}
		}
		COUT_THIS("backuper recvsize: "<<recv_size)
		//dump_buf(recv_buf, recv_size);

		char *cur = recv_buf;
		uint32_t kvnum = *(uint32_t *)cur;
		cur += 4;
		std::map<index_key_t, val_t> *new_backup_data = new std::map<index_key_t, val_t>;
		for (uint32_t i = 0; i < kvnum; i++) {
#ifdef LARGE_KEY
			index_key_t curkey(*(uint64_t*)cur, *(uint64_t*)(cur+8));
			cur += 16;
#else
			index_key_t curkey(*(uint64_t*)cur);
			cur += 8;
#endif
			val_t curval = *(uint64_t*)cur;
			cur += 8;
			uint8_t curvalid = *(uint8_t*)cur;
			cur += 1;
			if (curvalid == 1) {
				new_backup_data->insert(std::pair<index_key_t, val_t>(curkey, curval));
			}
		}

		volatile std::map<index_key_t, val_t> *old_backup_data = backup_data;
		backup_data = new_backup_data;

		// peace period of RCU (may not need if we do not use them in SCAN)
		for (uint32_t i = 0; i < fg_n; i++) {
			uint32_t prev_val = backup_rcu[i];
			while (running && backup_rcu[i] == prev_val) {}
		}
		if (old_backup_data != nullptr) {
			delete old_backup_data;
			old_backup_data = nullptr;
		}
		
	}
	pthread_exit(nullptr);
}

void *run_listener(void *param) {

	int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	INVARIANT(sock_fd >= 0);
	// Diskable udp check
	int disable = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		COUT_N_EXIT("Disable udp checksum failed");
	}
	// Set timeout
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec =  0;
	int res = setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	INVARIANT(res >= 0);
	// Set listen addr
	struct sockaddr_in listener_server_addr;
	memset(&listener_server_addr, 0, sizeof(sockaddr_in));
	listener_server_addr.sin_family = AF_INET;
	listener_server_addr.sin_port = htons(listener_port);
	listener_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	res = bind(sock_fd, (struct sockaddr *)&listener_server_addr, sizeof(listener_server_addr));
	INVARIANT(res >= 0);
	
	int recv_size = 0;
	char recv_buf[KV_BUCKET_COUT * 26]; // n * 25 + 14 + 20 + 8 < n * 26
	memset(recv_buf, 0, sizeof(recv_buf));

	while (!running)
		;

	while (running) {
		recv_size = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, nullptr, nullptr);
		double t0 = CUR_TIME();
		if (recv_size == -1) {
			if (errno == EWOULDBLOCK || errno == EINTR) {
				continue; // timeout or interrupted system call
			}
			else {
				COUT_N_EXIT("[server] Error of recvfrom: errno = " << errno);
			}
		}
		//COUT_THIS("listener recvsize: "<<recv_size)
		//dump_buf(recv_buf, recv_size);

		char *cur = recv_buf;
		uint32_t kvnum = *(uint32_t *)cur;
		cur += 4;
		std::map<index_key_t, val_t> *new_listener_data = new std::map<index_key_t, val_t>;
		for (uint32_t i = 0; i < kvnum; i++) {
#ifdef LARGE_KEY
			index_key_t curkey(*(uint64_t*)cur, *(uint64_t*)(cur+8));
			cur += 16;
#else
			index_key_t curkey(*(uint64_t*)cur);
			cur += 8;
#endif
			val_t curval = *(uint64_t*)cur;
			cur += 8;
			uint8_t curvalid = *(uint8_t*)cur;
			cur += 1;
			if (curvalid == 1) {
				new_listener_data->insert(std::pair<index_key_t, val_t>(curkey, curval));
			}
		}

		volatile std::map<index_key_t, val_t> *old_listener_data = listener_data;
		listener_data = new_listener_data;
		if (unlikely(listener_version == MAX_VERSION)) { // avoid overflow
			listener_version = 0;
		}
		else {
			listener_version += 1;
		}

		// Do no need peace period of RCU since SCAN will never use old listener data
		if (old_listener_data != nullptr) {
			delete old_listener_data;
			old_listener_data = nullptr;
		}

		double t1 = CUR_TIME();
		//COUT_THIS("Update KV: " << (t1 - t0) << "us")
	}
	pthread_exit(nullptr);
}

static int run_sfg(void * param) {
//void *run_sfg(void * param) {
  // Parse param
  sfg_param_t &thread_param = *(sfg_param_t *)param;
  uint32_t thread_id = thread_param.thread_id;
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

  // UDP socket for SCAN
  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  INVARIANT(sock_fd >= 0);
  struct sockaddr_in controller_addr;
  memset(&controller_addr, 0, sizeof(sockaddr_in));
  controller_addr.sin_family = AF_INET;
  controller_addr.sin_port = htons(controller_port);
  controller_addr.sin_addr.s_addr = inet_addr(controller_ip);

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
		
		uint64_t old_version = listener_version;

		// DPDK
		//stats[thread_id] = false;
		//recv_size = decode_mbuf(pkts[thread_id], srcmac, dstmac, srcip, dstip, &srcport, &dstport, buf);
		//rte_pktmbuf_free((struct rte_mbuf*)pkts[thread_id]);
		sent_pkt = sent_pkts[sent_pkt_idx];

		INVARIANT(pkts_list[thread_id][tails[thread_id]] != nullptr);
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
					if (!tmp_stat) {
						tmp_val = 0;
					}
					//COUT_THIS("[server] val = " << tmp_val)
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
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val())
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
					// Send KV pull request to switch (should not be counted into latency)
					sendto(sock_fd , "1", 1, 0, (struct sockaddr *)&controller_addr, sizeof(controller_addr));

					scan_request_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " num = " << req.num())
					std::vector<std::pair<index_key_t, val_t>> results;
					//double t00 = CUR_TIME();
					size_t tmp_num = table->scan(req.key(), req.num(), results, req.thread_id());
					//double t11 = CUR_TIME();
					//COUT_THIS("Index SCAN: " << (t11 - t00) << "us")
					/*COUT_THIS("[server] num = " << tmp_num)
					for (uint32_t val_i = 0; val_i < tmp_num; val_i++) {
						COUT_VAR(results[val_i].first.to_string())
						COUT_VAR(results[val_i].second)
					}*/

					// wait the new KV from switch (only for latency)
					// comment it for thpt to trade consistency for perf (directly use backup_data, uncomment the later sentence)
					//while (listener_version == old_version) {}

					// Merge results with backup data
					//double t0 = CUR_TIME();
					std::vector<std::pair<index_key_t, val_t>> merge_results;
					//std::map<index_key_t, val_t> *kvdata = listener_data;
					std::map<index_key_t, val_t> *kvdata = backup_data; // uncomment it for thpt
					if (kvdata != nullptr) {
						std::map<index_key_t, val_t>::iterator kviter = kvdata->lower_bound(req.key());
						uint32_t result_idx = 0;
						for (; kviter != kvdata->end(); kviter++) {
							if (kviter->first >= results[result_idx].first) {
								merge_results.push_back(results[result_idx]);
								result_idx++;
								if (result_idx >= results.size()) {
									break;
								}
							}
							else {
								merge_results.push_back(*kviter);
							}
						}
						if (merge_results.size() < req.num()) {
							// Only enter one loop
							for (; result_idx < results.size(); result_idx++) {
								merge_results.push_back(results[result_idx]);
								if (merge_results.size() == req.num()) {
									break;
								}
							}
							for (; kviter != kvdata->end(); kviter++) {
								merge_results.push_back(*kviter);
								if (merge_results.size() == req.num()) {
									break;
								}
							}
						}
					}
					//double t1 = CUR_TIME();
					//COUT_THIS("Merge: "<< (t1-t0) <<"us")

					scan_response_t *rsp = nullptr;
					if (kvdata != nullptr) {
						rsp = new scan_response_t(req.thread_id(), req.key(), merge_results.size(), merge_results);
					}
					else {
						rsp = new scan_response_t(req.thread_id(), req.key(), tmp_num, results);
					}
					rsp_size = rsp->serialize(buf, MAX_BUFSIZE);
					//res = sendto(sockfd, buf, rsp_size, 0, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr));
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, dstport, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::PUT_REQ_S:
				{
					put_request_s_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val())
					bool tmp_stat = table->put(req.key(), req.val(), req.thread_id());
					//COUT_THIS("[server] stat = " << tmp_stat)
					break;
				}
			case packet_type_t::DEL_REQ_S:
				{
					del_request_s_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					bool tmp_stat = table->remove(req.key(), req.thread_id());
					//COUT_THIS("[server] stat = " << tmp_stat)
					break;
				}
			default:
				{
					COUT_THIS("[server] Invalid packet type: " << int(pkt_type))
					std::cout << std::flush;
					exit(-1);
				}
		}
		backup_rcu[thread_id]++;

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
  COUT_THIS("[thread" << thread_id << "] exits")
  //pthread_exit(nullptr);
  return 0;
}

void kill(int signum) {
	COUT_THIS("[server] Receive SIGKILL!")
	killed = true;
}
