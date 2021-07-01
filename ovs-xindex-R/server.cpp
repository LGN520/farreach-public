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
#include "packet_format_impl.h"

class Key;

typedef Key index_key_t;
typedef uint64_t val_t;
typedef xindex::XIndex<index_key_t, val_t> xindex_t;
typedef GetRequest<index_key_t> get_request_t;
typedef GetResponse<index_key_t, val_t> get_response_t;

inline void parse_args(int, char **);
void load();
void run_server(xindex_t *table);
void kill(int signum);

// parameters
size_t fg_n = 1;
size_t bg_n = 1;
int server_port = 1111;

volatile bool running = false;
std::atomic<size_t> ready_threads(0);
std::vector<index_key_t> exist_keys;
std::vector<index_key_t> non_exist_keys;

bool killed = false;

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

  // prepare xindex
  std::vector<val_t> vals(exist_keys.size(), 1);
  xindex_t *tab_xi = new xindex_t(exist_keys, vals, fg_n, bg_n); // fg_n to create array of RCU status; bg_n background threads have been launched

  // register signal handler
  signal(SIGTERM, kill);

  run_server(tab_xi);
  if (tab_xi != nullptr) delete tab_xi; // terminate_bg -> bg_master joins bg_threads
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
  std::string ops = "i:j:k:l:m:n:o:";
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
	COUT_THIS("[server] # of nonexist keys = " << non_exist_keys.size());
}

void run_server(xindex_t *table) {
  // prepare socket
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  INVARIANT(sockfd >= 0);
  struct sockaddr_in server_sockaddr;
  memset(&server_sockaddr, 0, sizeof(struct sockaddr_in));
  server_sockaddr.sin_family = AF_INET;
  server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_sockaddr.sin_port = htons(server_port);
  int res = bind(sockfd, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr));
  INVARIANT(res != -1);

  // Set timeout
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec =  0;
  res = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  INVARIANT(res >= 0);

  running = true;

  char buf[MAX_BUFSIZE];
  int recv_size = 0;
  int rsp_size = 0;
  uint32_t sockaddr_len = sizeof(struct sockaddr);
  while (!killed) {
	COUT_VAR(sockaddr_len)
	recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, (struct sockaddr *)&server_sockaddr, &sockaddr_len);
	COUT_VAR(sockaddr_len)
	if (recv_size == -1) {
		if (errno == EWOULDBLOCK || errno == EINTR) {
			continue; // timeout or interrupted system call
		}
		else {
			COUT_THIS("[server] Error of recvfrom: errno = " << errno)
			exit(-1);
		}
	}
	else {
		COUT_THIS("[server] Receive packet!")
		packet_type_t pkt_type = get_packet_type(buf, recv_size);
		switch (pkt_type) {
			case packet_type_t::GET_REQ: 
				{
					get_request_t req(buf, recv_size);
					COUT_THIS("[server] key = " << req.key().key)
					val_t tmp_val;
					table->get(req.key(), tmp_val, req.thread_id());
					COUT_THIS("[server] val = " << tmp_val)
					get_response_t rsp(req.thread_id(), req.key(), tmp_val);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					res = sendto(sockfd, buf, rsp_size, 0, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr));

					char tmpip[20];
					memset(tmpip, '\0', 20);
					inet_ntop(AF_INET, (void *)&server_sockaddr.sin_addr, tmpip, 16);
					COUT_THIS("[server] received ip: " << std::string(tmpip) << " port: " << server_sockaddr.sin_port);
					COUT_THIS("[server] res of sendto: " << res << " rsp val: " << rsp.val())
					INVARIANT(res != -1);
					break;
				}
			default:
				{
					COUT_THIS("[server] Invalid packet type: " << int(pkt_type))
					std::cout << std::flush;
					exit(-1);
				}
		}
	}
  }

  running = false; // bg_threads will not retrain (compact, model/group merge/split)
  close(sockfd);
}

void kill(int signum) {
	COUT_THIS("[server] Receive SIGKILL!")
	killed = true;
}
