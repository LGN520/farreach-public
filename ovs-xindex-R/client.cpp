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
#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // inetaddr conversion
//#include <sys/time.h> // struct timeval

#include "helper.h"
#include "packet_format_impl.h"

#define MAX_BUFSIZE 256

struct alignas(CACHELINE_SIZE) FGParam;
class Key;

typedef FGParam fg_param_t;
typedef Key index_key_t;
typedef Request<index_key_t> request_t;
typedef GetRequest<index_key_t> get_request_t;

inline void prepare();
void load();
void run_benchmark(size_t sec);

void *run_fg(void *param);

inline void parse_args(int, char **);

// parameters
double read_ratio = 1;
double insert_ratio = 0;
double update_ratio = 0;
double delete_ratio = 0;
double scan_ratio = 0;
size_t table_size = 1000000;
size_t runtime = 10;
size_t fg_n = 1;
std::string server_addr = "10.0.0.1";
int server_port = 1111;

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
}

void load() {
	std::ifstream fd("exist_keys.out", std::ios::in | std::ios::binary);
	INVARIANT(fd);
	while (true) {
		double tmp = 0;
		fd.read((char *)&tmp, sizeof(double));
		if (likely(!fd.eof())) {
			exist_keys.push_back(tmp);
		}
		else {
			break;
		}
	}
	fd.close();
	COUT_THIS("# of exist keys = " << exist_keys.size());

	if (insert_ratio > 0) {
		fd.open("non_exist_keys.out", std::ios::in | std::ios::binary);
		INVARIANT(fd);
		while (true) {
			double tmp = 0;
			fd.read((char *)&tmp, sizeof(double));
			if (likely(!fd.eof())) {
				non_exist_keys.push_back(tmp);
			}
			else {
				break;
			}
		}
		fd.close();
	}
	COUT_THIS("# of nonexist keys = " << non_exist_keys.size());
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

  // Prepare socket
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  INVARIANT(sockfd >= 0);
  struct sockaddr_in server_sockaddr;
  memset(&server_sockaddr, 0, sizeof(struct sockaddr_in));
  server_sockaddr.sin_family = AF_INET;
  INVARIANT(inet_pton(AF_INET, server_addr.c_str(), &server_sockaddr.sin_addr) > 0);
  server_sockaddr.sin_port = htons(server_port);
  // Set timeout
  /*struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec =  0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));*/

  COUT_THIS("[client] Worker" << thread_id << " Ready.");
  size_t query_i = 0, insert_i = op_keys.size() / 2, delete_i = 0, update_i = 0;
  // exsiting keys fall within range [delete_i, insert_i)
  ready_threads++;
  //volatile bool res = false;
  //uint64_t dummy_value = 1234;
  int req_size = 0;
  int res = 0;
  int recv_size = 0;
  uint32_t sockaddr_len = 0;
  char buf[MAX_BUFSIZE];
  UNUSED(res);

  while (!running)
    ;

  while (running) {
    double d = ratio_dis(gen);
    if (d <= read_ratio) {  // get
      //res = table->get(op_keys[(query_i + delete_i) % op_keys.size()],
      //                 dummy_value, thread_id);
	  get_request_t req(thread_id, op_keys[(query_i + delete_i) % op_keys.size()]);
	  COUT_THIS("[client] key:thread_id = " << op_keys[(query_i + delete_i) % op_keys.size()].key << ":" << thread_id)
	  req_size = req.serialize(buf, MAX_BUFSIZE);
	  COUT_THIS("[client] req_key:req_thread_id:req_size = " << req.key().key << ":" << req.thread_id() << ":" << req_size)
	  res = sendto(sockfd, buf, req_size, 0, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr));
	  INVARIANT(res != -1);
	  recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, (struct sockaddr *)&server_sockaddr, &sockaddr_len);
	  INVARIANT(recv_size != -1);
	  exit(-1);
      query_i++;
      if (unlikely(query_i == op_keys.size() / 2)) {
        query_i = 0;
      }
    } else if (d <= read_ratio + update_ratio) {  // update
      //res = table->put(op_keys[(update_i + delete_i) % op_keys.size()],
      //                 dummy_value, thread_id);
      /*update_i++;
      if (unlikely(update_i == op_keys.size() / 2)) {
        update_i = 0;
      }*/
	  continue;
    } else if (d <= read_ratio + update_ratio + insert_ratio) {  // insert
      //res = table->put(op_keys[insert_i], dummy_value, thread_id);
      /*insert_i++;
      if (unlikely(insert_i == op_keys.size())) {
        insert_i = 0;
      }*/
	  continue;
    } else if (d <= read_ratio + update_ratio + insert_ratio +
                        delete_ratio) {  // remove
      //res = table->remove(op_keys[delete_i], thread_id);
      /*delete_i++;
      if (unlikely(delete_i == op_keys.size())) {
        delete_i = 0;
      }*/
	  continue;
    } else {  // scan
      //std::vector<std::pair<index_key_t, uint64_t>> results;
      //table->scan(op_keys[(query_i + delete_i) % op_keys.size()], 10, results,
      //            thread_id);
      /*query_i++;
      if (unlikely(query_i == op_keys.size() / 2)) {
        query_i = 0;
      }*/
	  continue;
    }
    thread_param.throughput++;
  }

  pthread_exit(nullptr);
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

inline void parse_args(int argc, char **argv) {
  struct option long_options[] = {
      {"read", required_argument, 0, 'a'},
      {"insert", required_argument, 0, 'b'},
      {"remove", required_argument, 0, 'c'},
      {"update", required_argument, 0, 'd'},
      {"scan", required_argument, 0, 'e'},
      {"table-size", required_argument, 0, 'f'},
      {"runtime", required_argument, 0, 'g'},
      {"fg", required_argument, 0, 'h'},
	  {"server-addr", required_argument, 0, 'i'},
	  {"server-port", required_argument, 0, 'j'},
      {0, 0, 0, 0}};
  std::string ops = "a:b:c:d:e:f:g:h:i:j:";
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
      case 'f':
        table_size = strtoul(optarg, NULL, 10);
        INVARIANT(table_size > 0);
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
		server_addr = std::string(optarg);
		INVARIANT(server_addr.length() > 0);
		break;
	  case 'j':
		server_port = atoi(optarg);
		INVARIANT(server_port > 0);
		break;
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
