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
#include <errno.h>
#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // inetaddr conversion
//#include <sys/time.h> // struct timeval

#include "helper.h"
#include "raw_socket.h"
#include "packet_format_impl.h"

struct alignas(CACHELINE_SIZE) FGParam;
class Key;

typedef FGParam fg_param_t;
typedef Key index_key_t;
typedef uint64_t val_t;
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
void *run_fg(void *param);

// parameters
double read_ratio = 1;
double insert_ratio = 0;
double update_ratio = 0;
double delete_ratio = 0;
double scan_ratio = 0;
size_t runtime = 10;
size_t fg_n = 1;
//std::string server_addr = "10.0.0.32";
//int server_port = 1111;

// Raw socket
std::string src_ifname = "ens3f0";
//std::string dst_ifname = "ens3f1";
uint8_t src_macaddr[6] = {0x9c, 0x69, 0xb4, 0x60, 0xef, 0xa4};
uint8_t dst_macaddr[6] = {0x9c, 0x69, 0xb4, 0x60, 0xef, 0x8d};
std::string src_ipaddr = "10.0.0.31";
std::string dst_ipaddr = "10.0.0.32";
short src_port_start = 8888;
short dst_port_start = 1111;

volatile bool running = false;
std::atomic<size_t> ready_threads(0);
std::vector<index_key_t> exist_keys;
std::vector<index_key_t> non_exist_keys;

struct alignas(CACHELINE_SIZE) FGParam {
  uint64_t throughput;
  uint32_t thread_id;
};

class Key {
  typedef std::array<double, 1> model_key_t;

 public:
  static constexpr size_t model_key_size() { return 1; }
  static Key max() {
    static Key max_key(std::numeric_limits<uint64_t>::max());
    return max_key;
  }
  static Key min() {
    static Key min_key(std::numeric_limits<uint64_t>::min());
    return min_key;
  }

  Key() : key(0) {}
  Key(uint64_t key) : key(key) {}
  Key(const Key &other) { key = other.key; }
  Key &operator=(const Key &other) {
    key = other.key;
    return *this;
  }

  model_key_t to_model_key() const {
    model_key_t model_key;
    model_key[0] = key;
    return model_key;
  }

  friend bool operator<(const Key &l, const Key &r) { return l.key < r.key; }
  friend bool operator>(const Key &l, const Key &r) { return l.key > r.key; }
  friend bool operator>=(const Key &l, const Key &r) { return l.key >= r.key; }
  friend bool operator<=(const Key &l, const Key &r) { return l.key <= r.key; }
  friend bool operator==(const Key &l, const Key &r) { return l.key == r.key; }
  friend bool operator!=(const Key &l, const Key &r) { return l.key != r.key; }

  uint64_t key;
} PACKED;

int main(int argc, char **argv) {
  parse_args(argc, argv);
  load();
  run_benchmark(runtime);
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
	  //{"server-addr", required_argument, 0, 'i'},
	  //{"server-port", required_argument, 0, 'j'},
      {0, 0, 0, 0}};
  std::string ops = "a:b:c:d:e:g:h:";
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
	  /*case 'i':
		server_addr = std::string(optarg);
		INVARIANT(server_addr.length() > 0);
		break;
	  case 'j':
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
  pthread_t threads[fg_n];
  fg_param_t fg_params[fg_n];
  // check if parameters are cacheline aligned
  for (size_t i = 0; i < fg_n; i++) {
    if ((uint64_t)(&(fg_params[i])) % CACHELINE_SIZE != 0) {
      COUT_N_EXIT("wrong parameter address: " << &(fg_params[i]));
    }
  }

  running = false;
  for (size_t worker_i = 0; worker_i < fg_n; worker_i++) {
    fg_params[worker_i].thread_id = worker_i;
    fg_params[worker_i].throughput = 0;
    int ret = pthread_create(&threads[worker_i], nullptr, run_fg,
                             (void *)&fg_params[worker_i]);
    if (ret) {
      COUT_N_EXIT("Error:" << ret);
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
  void *status;
  for (size_t i = 0; i < fg_n; i++) {
    int rc = pthread_join(threads[i], &status);
    if (rc) {
      COUT_N_EXIT("Error:unable to join," << rc);
    }
  }

  size_t throughput = 0;
  for (auto &p : fg_params) {
    throughput += p.throughput;
  }
  COUT_THIS("[client] Throughput(op/s): " << throughput / sec);
}

void *run_fg(void *param) {
  fg_param_t &thread_param = *(fg_param_t *)param;
  uint32_t thread_id = thread_param.thread_id;

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

  // Prepare socket (UDP socket)
  /*int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  INVARIANT(sockfd >= 0);
  struct sockaddr_in remote_sockaddr;
  uint32_t sockaddr_len = sizeof(struct sockaddr);
  memset(&remote_sockaddr, 0, sizeof(struct sockaddr_in));
  remote_sockaddr.sin_family = AF_INET;
  INVARIANT(inet_pton(AF_INET, server_addr.c_str(), &remote_sockaddr.sin_addr) > 0);
  remote_sockaddr.sin_port = htons(server_port);*/
  // Set timeout
  /*struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec =  0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));*/

  // Prepare socket (raw socket)
  //int raw_sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
  int raw_sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  INVARIANT(raw_sockfd != -1);
  int optval = 7; // valid values are in the range [1,7]  // 1- low priority, 7 - high priority  
  if (setsockopt(raw_sockfd, SOL_SOCKET, SO_PRIORITY, &optval, sizeof(optval)) < 0) {
	perror("setsockopt");
  }
  int ifidx = lookup_if(raw_sockfd, src_ifname, NULL);
  struct sockaddr_ll raw_socket_address;
  init_raw_sockaddr(&raw_socket_address, ifidx, src_macaddr); // set target interface for sendto

  int res = bind(raw_sockfd, (struct sockaddr *)&raw_socket_address, sizeof(struct sockaddr_ll)); // bind target interface for recvfrom
  INVARIANT(res != -1);
  char totalbuf[MAX_BUFSIZE]; // headers + payload
  short src_port = src_port_start + thread_id;
  short dst_port = dst_port_start + thread_id;

  // exsiting keys fall within range [delete_i, insert_i)
  char buf[MAX_BUFSIZE]; // payload
  ready_threads++;
  int req_size = 0;
  int recv_size = 0;
  val_t dummy_value = 1234;
  COUT_THIS("[client " << thread_id << "] Ready.");
  size_t query_i = 0, insert_i = op_keys.size() / 2, delete_i = 0, update_i = 0;

  // DEBUG
  uint32_t curidx = 0;
  uint32_t maxidx = 5;
  int tmpruns[maxidx] = {0, 3, 0, 2, 0};
  size_t idxes[maxidx] = {0, 0, 0, 0, 0};
  val_t vals[maxidx] = {1, 1, 1, 33, 1};

  while (!running)
    ;

  while (running) {
	// DEBUG
	if (curidx >= maxidx) break;
	int tmprun = tmpruns[curidx];
	if (vals[curidx] != 0) dummy_value = vals[curidx];
	query_i = idxes[curidx];
	delete_i = idxes[curidx];
	insert_i = idxes[curidx];
	update_i = idxes[curidx];
	curidx++;

    double d = ratio_dis(gen);
    //if (d <= read_ratio) {  // get
    if (tmprun == 0) {  // get
	  get_request_t req(thread_id, op_keys[(query_i + delete_i) % op_keys.size()]);
	  DEBUG_THIS("[client " << thread_id << "] key = " << op_keys[(query_i + delete_i) % op_keys.size()].key);
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // UDP socket
	  //res = sendto(sockfd, buf, req_size, 0, (struct sockaddr *)&remote_sockaddr, sizeof(struct sockaddr));
	  //INVARIANT(res != -1);
	  //recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, NULL, NULL);
	  //INVARIANT(recv_size != -1);
	  
	  // Raw socket
	  size_t totalsize = init_buf(totalbuf, MAX_BUFSIZE, src_macaddr, dst_macaddr, src_ipaddr, dst_ipaddr, src_port, dst_port, buf, req_size);
	  res = sendto(raw_sockfd, totalbuf, totalsize, 0, (struct sockaddr *) &raw_socket_address, sizeof(struct sockaddr_ll));
	  INVARIANT(res != -1);
	  while (true) {
		recv_size = recvfrom(raw_sockfd, totalbuf, MAX_BUFSIZE, 0, NULL, NULL);
	  	INVARIANT(recv_size != -1);
		recv_size = client_recv_payload(buf, totalbuf, recv_size, src_port, dst_port);
		if (recv_size != -1) break;
	  }

	  packet_type_t pkt_type = get_packet_type(buf, recv_size);
	  INVARIANT(pkt_type == packet_type_t::GET_RES);
	  get_response_t rsp(buf, recv_size);
	  DEBUG_THIS("[client " << thread_id << "] val = " << rsp.val());
      query_i++;
      if (unlikely(query_i == op_keys.size() / 2)) {
        query_i = 0;
      }
    //} else if (d <= read_ratio + update_ratio) {  // update
    } else if (tmprun == 1) {  // update
	  put_request_t req(thread_id, op_keys[(update_i + delete_i) % op_keys.size()], dummy_value);
	  DEBUG_THIS("[client " << thread_id << "] key = " << op_keys[(update_i + delete_i) % op_keys.size()].key << " val = " << req.val());
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // UDP socket
	  //res = sendto(sockfd, buf, req_size, 0, (struct sockaddr *)&remote_sockaddr, sizeof(struct sockaddr));
	  //INVARIANT(res != -1);
	  //recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, NULL, NULL);
	  //INVARIANT(recv_size != -1);
	  
	  // Raw socket
	  size_t totalsize = init_buf(totalbuf, MAX_BUFSIZE, src_macaddr, dst_macaddr, src_ipaddr, dst_ipaddr, src_port, dst_port, buf, req_size);
	  res = sendto(raw_sockfd, totalbuf, totalsize, 0, (struct sockaddr *) &raw_socket_address, sizeof(struct sockaddr_ll));
	  INVARIANT(res != -1);
	  while (true) {
		recv_size = recvfrom(raw_sockfd, totalbuf, MAX_BUFSIZE, 0, NULL, NULL);
	  	INVARIANT(recv_size != -1);
		recv_size = client_recv_payload(buf, totalbuf, recv_size, src_port, dst_port);
		if (recv_size != -1) break;
	  }

	  packet_type_t pkt_type = get_packet_type(buf, recv_size);
	  INVARIANT(pkt_type == packet_type_t::PUT_RES);
	  put_response_t rsp(buf, recv_size);
	  DEBUG_THIS("[client " << thread_id << "] stat = " << rsp.stat());
      update_i++;
      if (unlikely(update_i == op_keys.size() / 2)) {
        update_i = 0;
      }
    //} else if (d <= read_ratio + update_ratio + insert_ratio) {  // insert
    } else if (tmprun == 2) {  // insert
	  put_request_t req(thread_id, op_keys[insert_i], dummy_value);
	  DEBUG_THIS("[client " << thread_id << "] key = " << op_keys[insert_i].key << " val = " << req.val());
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // UDP socket
	  //res = sendto(sockfd, buf, req_size, 0, (struct sockaddr *)&remote_sockaddr, sizeof(struct sockaddr));
	  //INVARIANT(res != -1);
	  //recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, NULL, NULL);
	  //INVARIANT(recv_size != -1);
	  
	  // Raw socket
	  size_t totalsize = init_buf(totalbuf, MAX_BUFSIZE, src_macaddr, dst_macaddr, src_ipaddr, dst_ipaddr, src_port, dst_port, buf, req_size);
	  res = sendto(raw_sockfd, totalbuf, totalsize, 0, (struct sockaddr *) &raw_socket_address, sizeof(struct sockaddr_ll));
	  INVARIANT(res != -1);
	  while (true) {
		recv_size = recvfrom(raw_sockfd, totalbuf, MAX_BUFSIZE, 0, NULL, NULL);
	  	INVARIANT(recv_size != -1);
		recv_size = client_recv_payload(buf, totalbuf, recv_size, src_port, dst_port);
		if (recv_size != -1) break;
	  }

	  packet_type_t pkt_type = get_packet_type(buf, recv_size);
	  INVARIANT(pkt_type == packet_type_t::PUT_RES);
	  put_response_t rsp(buf, recv_size);
	  DEBUG_THIS("[client " << thread_id << "] stat = " << rsp.stat());
      insert_i++;
      if (unlikely(insert_i == op_keys.size())) {
        insert_i = 0;
      }
    //} else if (d <= read_ratio + update_ratio + insert_ratio + delete_ratio) {  // remove
    } else if (tmprun == 3) {  // remove
	  del_request_t req(thread_id, op_keys[delete_i]);
	  DEBUG_THIS("[client " << thread_id << "] key = " << op_keys[delete_i].key);
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // UDP socket
	  //res = sendto(sockfd, buf, req_size, 0, (struct sockaddr *)&remote_sockaddr, sizeof(struct sockaddr));
	  //INVARIANT(res != -1);
	  //recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, NULL, NULL);
	  //INVARIANT(recv_size != -1);
	  
	  // Raw socket
	  size_t totalsize = init_buf(totalbuf, MAX_BUFSIZE, src_macaddr, dst_macaddr, src_ipaddr, dst_ipaddr, src_port, dst_port, buf, req_size);
	  res = sendto(raw_sockfd, totalbuf, totalsize, 0, (struct sockaddr *) &raw_socket_address, sizeof(struct sockaddr_ll));
	  INVARIANT(res != -1);
	  while (true) {
		recv_size = recvfrom(raw_sockfd, totalbuf, MAX_BUFSIZE, 0, NULL, NULL);
	  	INVARIANT(recv_size != -1);
		recv_size = client_recv_payload(buf, totalbuf, recv_size, src_port, dst_port);
		if (recv_size != -1) break;
	  }

	  packet_type_t pkt_type = get_packet_type(buf, recv_size);
	  INVARIANT(pkt_type == packet_type_t::DEL_RES);
	  del_response_t rsp(buf, recv_size);
	  DEBUG_THIS("[client " << thread_id << "] stat = " << rsp.stat());
      delete_i++;
      if (unlikely(delete_i == op_keys.size())) {
        delete_i = 0;
      }
    } else {  // scan
	  scan_request_t req(thread_id, op_keys[(query_i + delete_i) % op_keys.size()], 10);
	  DEBUG_THIS("[client " << thread_id << "] key = " << req.key().key);
	  req_size = req.serialize(buf, MAX_BUFSIZE);

	  // UDP socket
	  //res = sendto(sockfd, buf, req_size, 0, (struct sockaddr *)&remote_sockaddr, sizeof(struct sockaddr));
	  //INVARIANT(res != -1);
	  //recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, NULL, NULL);
	  //INVARIANT(recv_size != -1);
	  
	  // Raw socket
	  size_t totalsize = init_buf(totalbuf, MAX_BUFSIZE, src_macaddr, dst_macaddr, src_ipaddr, dst_ipaddr, src_port, dst_port, buf, req_size);
	  res = sendto(raw_sockfd, totalbuf, totalsize, 0, (struct sockaddr *) &raw_socket_address, sizeof(struct sockaddr_ll));
	  INVARIANT(res != -1);
	  while (true) {
		recv_size = recvfrom(raw_sockfd, totalbuf, MAX_BUFSIZE, 0, NULL, NULL);
	  	INVARIANT(recv_size != -1);
		recv_size = client_recv_payload(buf, totalbuf, recv_size, src_port, dst_port);
		if (recv_size != -1) break;
	  }

	  packet_type_t pkt_type = get_packet_type(buf, recv_size);
	  INVARIANT(pkt_type == packet_type_t::SCAN_RES);
	  scan_response_t rsp(buf, recv_size);
	  DEBUG_THIS("[client " << thread_id << "] num = " << rsp.num());
	  /*for (uint32_t val_i = 0; val_i < rsp.num(); val_i++) {
		  COUT_VAR(rsp.pairs()[val_i].first.key)
		  COUT_VAR(rsp.pairs()[val_i].second)
	  }*/
      query_i++;
      if (unlikely(query_i == op_keys.size() / 2)) {
        query_i = 0;
      }
    }
    thread_param.throughput++;
  }

  //close(sockfd);
  close(raw_sockfd);
  pthread_exit(nullptr);
}
