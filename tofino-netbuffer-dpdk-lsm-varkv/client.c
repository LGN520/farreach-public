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

inline void parse_args(int, char **);
void load();
void run_benchmark(size_t sec);
//void *run_fg(void *param); // sender

// DPDK
static int run_fg(void *param); // sender
static int run_receiver(__attribute__((unused)) void *param); // receiver
struct rte_mempool *mbuf_pool = NULL;
volatile struct rte_mbuf **pkts;
volatile bool *stats;

// parameters
double read_ratio = 1;
double insert_ratio = 0;
double update_ratio = 0;
double delete_ratio = 0;
double scan_ratio = 0;
size_t runtime = 10;
size_t fg_n = 1;
uint8_t src_macaddr[6] = {0x9c, 0x69, 0xb4, 0x60, 0xef, 0xa5};
uint8_t dst_macaddr[6] = {0x9c, 0x69, 0xb4, 0x60, 0xef, 0x8d};
char src_ipaddr[16] = "10.0.0.31";
char server_addr[16] = "10.0.0.32";
short src_port_start = 8888;
short dst_port_start = 1111;

volatile bool running = false;
std::atomic<size_t> ready_threads(0);
std::vector<index_key_t> exist_keys;
std::vector<index_key_t> non_exist_keys;

struct alignas(CACHELINE_SIZE) FGParam {
  uint64_t throughput;
  uint8_t thread_id;
};

int main(int argc, char **argv) {
  parse_args(argc, argv);
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
  COUT_THIS("Prepare " << fg_n << "pkts and stats")
  pkts = new volatile struct rte_mbuf*[fg_n];
  stats = new volatile bool[fg_n];
  memset((void *)pkts, 0, sizeof(struct rte_mbuf *)*fg_n);
  memset((void *)stats, 0, sizeof(bool)*fg_n);
  for (size_t i = 0; i < fg_n; i++) {
	pkts[i] = rte_pktmbuf_alloc(mbuf_pool);
  }

  run_benchmark(runtime);

  // Free DPDK mbufs
  for (size_t i = 0; i < fg_n; i++) {
	rte_pktmbuf_free((struct rte_mbuf *)pkts[i]);
  }
  dpdk_free();

  exit(0);
}

inline void parse_args(int argc, char **argv) {
  struct option long_options[] = {
      {"read", required_argument, 0, 'a'},
      {"insert", required_argument, 0, 'b'},
      {"remove", required_argument, 0, 'c'},
      {"update", required_argument, 0, 'd'},
      {"scan", required_argument, 0, 'e'},
      {"runtime", required_argument, 0, 'g'},
      {"fg", required_argument, 0, 'h'},
	  {"server-addr", required_argument, 0, 'i'},
	  //{"server-port", required_argument, 0, 'j'},
      {0, 0, 0, 0}};
  std::string ops = "a:b:c:d:e:g:h:i:";
  int option_index = 0;

  while (1) {
    int c = getopt_long(argc, argv, ops.c_str(), long_options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 0:
        if (long_options[option_index].flag != 0) break;
        abort();
        break;
      case 'a':
        read_ratio = strtod(optarg, NULL);
        INVARIANT(read_ratio >= 0 && read_ratio <= 1);
        break;
      case 'b':
        insert_ratio = strtod(optarg, NULL);
        INVARIANT(insert_ratio >= 0 && insert_ratio <= 1);
        break;
      case 'c':
        delete_ratio = strtod(optarg, NULL);
        INVARIANT(delete_ratio >= 0 && delete_ratio <= 1);
        break;
      case 'd':
        update_ratio = strtod(optarg, NULL);
        INVARIANT(update_ratio >= 0 && update_ratio <= 1);
        break;
      case 'e':
        scan_ratio = strtod(optarg, NULL);
        INVARIANT(scan_ratio >= 0 && scan_ratio <= 1);
        break;
      case 'g':
        runtime = strtoul(optarg, NULL, 10);
        INVARIANT(runtime > 0);
        break;
      case 'h':
        fg_n = strtoul(optarg, NULL, 10);
        INVARIANT(fg_n > 0);
        break;
	  case 'i':
		INVARIANT(strlen(optarg) > 0);
		memcpy(server_addr, optarg, strlen(optarg));
		break;
	  /*case 'j':
		server_port = atoi(optarg);
		INVARIANT(server_port > 0);
		break;*/
      default:
        abort();
    }
  }

  COUT_THIS("[micro] Read:Insert:Update:Delete:Scan = "
            << read_ratio << ":" << insert_ratio << ":" << update_ratio << ":"
            << delete_ratio << ":" << scan_ratio)
  double ratio_sum =
      read_ratio + insert_ratio + delete_ratio + scan_ratio + update_ratio;
  INVARIANT(ratio_sum > 0.9999 && ratio_sum < 1.0001);  // avoid precision lost
  COUT_VAR(runtime);
  COUT_VAR(fg_n);
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
	COUT_THIS("[client] # of exist keys = " << exist_keys.size());

	if (insert_ratio > 0) {
		fd.open("nonexist_keys.out", std::ios::in | std::ios::binary);
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
	}
	COUT_THIS("[client] # of nonexist keys = " << non_exist_keys.size());
}

void run_benchmark(size_t sec) {
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
  std::vector<size_t> tput_history(fg_n, 0);
  size_t current_sec = 0;
  while (current_sec < sec) {
    sleep(1);
    uint64_t tput = 0;
    for (size_t i = 0; i < fg_n; i++) {
      tput += fg_params[i].throughput - tput_history[i];
      tput_history[i] = fg_params[i].throughput;
    }
    COUT_THIS("[client] >>> sec " << current_sec << " throughput: " << tput);
    ++current_sec;
  }

  running = false;

  size_t throughput = 0;
  for (auto &p : fg_params) {
    throughput += p.throughput;
  }
  COUT_THIS("[client] Throughput(op/s): " << throughput / sec);

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

  //DEBUG
  //double prevt0 = 0;
  //double t0 = CUR_TIME();
  //uint32_t debug_idx = 0;

	struct rte_mbuf *received_pkts[32];
	while (running) {
		//double tmpt0 = CUR_TIME();
		uint16_t n_rx = rte_eth_rx_burst(0, 0, received_pkts, 32);
		if (n_rx == 0) {
			//double tmpt1 = CUR_TIME();
			//t0 += (tmpt1 - tmpt0);
			continue;
		}
		/*if ((debug_idx + 1) % 10001 == 0) {
			COUT_VAR((t0 - prevt0) / 10000.0);
			prevt0 = t0;
			COUT_VAR(n_rx);
		}*/
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
		//debug_idx++;
		//double tmpt1 = CUR_TIME();
		//t0 += (tmpt1 - tmpt0);
	}
	return 0;
}

static int run_fg(void *param) {
  fg_param_t &thread_param = *(fg_param_t *)param;
  uint8_t thread_id = thread_param.thread_id;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> ratio_dis(0, 1);

  size_t exist_key_n_per_thread = exist_keys.size() / fg_n;
  size_t exist_key_start = thread_id * exist_key_n_per_thread;
  size_t exist_key_end = (thread_id + 1) * exist_key_n_per_thread;
  std::vector<index_key_t> op_keys(exist_keys.begin() + exist_key_start,
                                   exist_keys.begin() + exist_key_end);

  if (non_exist_keys.size() > 0) {
    size_t non_exist_key_n_per_thread = non_exist_keys.size() / fg_n;
    size_t non_exist_key_start = thread_id * non_exist_key_n_per_thread,
           non_exist_key_end = (thread_id + 1) * non_exist_key_n_per_thread;
    op_keys.insert(op_keys.end(), non_exist_keys.begin() + non_exist_key_start,
                   non_exist_keys.begin() + non_exist_key_end);
  }

  int res = 0;

  // Prepare socket
  /*int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  INVARIANT(sockfd >= 0);
  int disable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
	perror("Disable udp checksum failed");
  }
  short src_port = src_port_start + thread_id;
  short dst_port = dst_port_start + thread_id;
  // Client address
  struct sockaddr_in client_sockaddr;
  memset(&client_sockaddr, 0, sizeof(struct sockaddr_in));
  client_sockaddr.sin_family = AF_INET;
  client_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  client_sockaddr.sin_port = htons(src_port);
  res = bind(sockfd, (struct sockaddr *)&client_sockaddr, sizeof(struct sockaddr_in));
  INVARIANT(res != -1)
  // Server address
  struct sockaddr_in remote_sockaddr;
  memset(&remote_sockaddr, 0, sizeof(struct sockaddr_in));
  remote_sockaddr.sin_family = AF_INET;
  INVARIANT(inet_pton(AF_INET, server_addr, &remote_sockaddr.sin_addr) > 0);
  remote_sockaddr.sin_port = htons(dst_port);*/
  // Set timeout
  /*struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec =  0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));*/

  // DPDK
  short src_port = src_port_start + thread_id;
  short dst_port = dst_port_start + thread_id;
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
  uint64_t dummy_value_data[1] = {1234};
  val_t dummy_value = Val(dummy_value_data, 1);
  size_t query_i = 0, insert_i = op_keys.size() / 2, delete_i = 0, update_i = 0;
  COUT_THIS("[client " << uint32_t(thread_id) << "] Ready.");
  ready_threads++;

  // DEBUG TEST
  //uint32_t debugtest_idx = 0;
  //uint32_t debugtest_i = 0;

#if !defined(NDEBUGGING_LOG)
  std::string logname;
  GET_STRING(logname, "tmp_client"<< uint32_t(thread_id)<<".out");
  std::ofstream ofs(logname, std::ofstream::out);
#endif

  while (!running)
    ;

  //DEBUG
  //double t0 = CUR_TIME();
  //uint32_t debug_idx = 0;
  //uint32_t tmploop = 0;

  while (running) {
	// DEBUG TEST
	/*int tmprun = 0;
	query_i = debugtest_idx;
	update_i = debugtest_idx;
	if (debugtest_i == 0) tmprun = 1;
	debugtest_i++;*/

	sent_pkt = sent_pkts[sent_pkt_idx];

    double d = ratio_dis(gen);

	/*if ((debug_idx + 1) % 10001 == 0) {
		COUT_VAR(t0 / 10000.0);
		t0 = 0;
		COUT_VAR(tmploop / 10000.0);
		tmploop = 0;
	}*/

	int tmprun = 0;
    //if (d <= read_ratio) {  // get
    if (tmprun == 0) {  // get
	  get_request_t req(thread_id, op_keys[(query_i + delete_i) % op_keys.size()]);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << op_keys[(query_i + delete_i) % op_keys.size()].to_string());
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // UDP socket
	  //res = sendto(sockfd, buf, req_size, 0, (struct sockaddr *)&remote_sockaddr, sizeof(struct sockaddr));
	  //INVARIANT(res != -1);
	  //recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, NULL, NULL);
	  //INVARIANT(recv_size != -1);
	  
	  // DPDK
	  encode_mbuf(sent_pkt, src_macaddr, dst_macaddr, src_ipaddr, server_addr, src_port, dst_port, buf, req_size);
	  //double tmpt0 = CUR_TIME();
	  res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
	  INVARIANT(res == 1);
	  while (!stats[thread_id])
		  //tmploop++;
		  ;
      //debug_idx++;
	  //double tmpt1 = CUR_TIME();
	  //t0 += (tmpt1 - tmpt0);
	  stats[thread_id] = false;
	  recv_size = get_payload(pkts[thread_id], buf);
	  rte_pktmbuf_free((struct rte_mbuf*)pkts[thread_id]);
	  INVARIANT(recv_size != -1);

	  packet_type_t pkt_type = get_packet_type(buf, recv_size);
	  INVARIANT(pkt_type == packet_type_t::GET_RES);
	  get_response_t rsp(buf, recv_size);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << rsp.key().to_string() << " val = " << rsp.val().to_string());
      query_i++;
      if (unlikely(query_i == op_keys.size() / 2)) {
        query_i = 0;
      }
    //} else if (d <= read_ratio + update_ratio) {  // update
    } else if (tmprun == 1) {  // update
	  put_request_t req(thread_id, op_keys[(update_i + delete_i) % op_keys.size()], dummy_value);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << op_keys[(update_i + delete_i) % op_keys.size()].to_string() << " val = " << req.val().to_string());
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // UDP socket
	  //res = sendto(sockfd, buf, req_size, 0, (struct sockaddr *)&remote_sockaddr, sizeof(struct sockaddr));
	  //INVARIANT(res != -1);
	  //recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, NULL, NULL);
	  //INVARIANT(recv_size != -1);

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
      update_i++;
      if (unlikely(update_i == op_keys.size() / 2)) {
        update_i = 0;
      }
    //} else if (d <= read_ratio + update_ratio + insert_ratio) {  // insert
    } else if (tmprun == 2) {  // insert
	  put_request_t req(thread_id, op_keys[insert_i], dummy_value);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << op_keys[insert_i].to_string() << " val = " << req.val().to_string());
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // UDP socket
	  //res = sendto(sockfd, buf, req_size, 0, (struct sockaddr *)&remote_sockaddr, sizeof(struct sockaddr));
	  //INVARIANT(res != -1);
	  //recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, NULL, NULL);
	  //INVARIANT(recv_size != -1);
	  
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
      insert_i++;
      if (unlikely(insert_i == op_keys.size())) {
        insert_i = 0;
      }
    //} else if (d <= read_ratio + update_ratio + insert_ratio + delete_ratio) {  // remove
    } else if (tmprun == 3) {  // remove
	  del_request_t req(thread_id, op_keys[delete_i]);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << op_keys[delete_i].to_string());
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // UDP socket
	  //res = sendto(sockfd, buf, req_size, 0, (struct sockaddr *)&remote_sockaddr, sizeof(struct sockaddr));
	  //INVARIANT(res != -1);
	  //recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, NULL, NULL);
	  //INVARIANT(recv_size != -1);

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
      delete_i++;
      if (unlikely(delete_i == op_keys.size())) {
        delete_i = 0;
      }
    } else {  // scan
	  scan_request_t req(thread_id, op_keys[(query_i + delete_i) % op_keys.size()], 10);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] key = " << req.key().to_string());
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // UDP socket
	  //res = sendto(sockfd, buf, req_size, 0, (struct sockaddr *)&remote_sockaddr, sizeof(struct sockaddr));
	  //INVARIANT(res != -1);
	  //recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, NULL, NULL);
	  //INVARIANT(recv_size != -1);

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
	  INVARIANT(pkt_type == packet_type_t::SCAN_RES);
	  scan_response_t rsp(buf, recv_size);
	  FDEBUG_THIS(ofs, "[client " << uint32_t(thread_id) << "] num = " << rsp.num());
	  for (uint32_t val_i = 0; val_i < rsp.num(); val_i++) {
		  FDEBUG_THIS(ofs, rsp.pairs()[val_i].first.to_string());
		  FDEBUG_THIS(ofs, rsp.pairs()[val_i].second.to_string());
	  }
      query_i++;
      if (unlikely(query_i == op_keys.size() / 2)) {
        query_i = 0;
      }
    }
    thread_param.throughput++;

	sent_pkt_idx++;
	if (sent_pkt_idx >= burst_size) {
		sent_pkt_idx = 0;
		res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
		INVARIANT(res == 0);
	}
  }

  if (sent_pkt_idx < burst_size) {
	for (uint16_t free_idx = sent_pkt_idx; free_idx != burst_size; free_idx++) {
		rte_pktmbuf_free(sent_pkts[free_idx]);
	}
  }

  //close(sockfd);
  //pthread_exit(nullptr); // UDP socket
#if !defined(NDEBUGGING_LOG)
  ofs.close();
#endif
  return 0;
}
