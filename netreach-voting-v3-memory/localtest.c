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
#include <sys/time.h> // struct timeval

#include "helper.h"
#include "xindex.h"
#include "xindex_impl.h"
#include "xindex_util.h"
#include "packet_format_impl.h"
#include "key.h"
#include "val.h"
#include "iniparser/iniparser_wrapper.h"

struct alignas(CACHELINE_SIZE) SFGParam;

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

inline void parse_ini(const char * config_file);
inline void parse_args(int, char **);
void load();
void run_server(xindex_t *table, size_t sec);
void *run_sfg(void *param);

// parameters
double read_ratio = 1;
double insert_ratio = 0;
double update_ratio = 0;
double delete_ratio = 0;
double scan_ratio = 0;
size_t runtime = 10;
size_t fg_n = 1;
size_t bg_n = 1;
const char *workload_name;
uint32_t kv_bucket_num;

std::vector<index_key_t> exist_keys;
std::vector<index_key_t> non_exist_keys;

volatile bool running = false;
std::atomic<size_t> ready_threads(0);

std::map<index_key_t, val_t>* volatile backup_data = nullptr;

void test_merge_latency() {
	backup_data = new std::map<index_key_t, val_t>;
	for (size_t i = 0; i < kv_bucket_num; i++) {
		uint64_t init_val_data[1] = {1};
		backup_data->insert(std::pair<index_key_t, val_t>(exist_keys[i], val_t(init_val_data, 1)));
	}
}

struct alignas(CACHELINE_SIZE) SFGParam {
  xindex_t *table;
  uint64_t throughput;
  uint8_t thread_id;
};

int main(int argc, char **argv) {

  parse_ini("config.ini");
  parse_args(argc, argv);
  xindex::init_options(); // init options of rocksdb
  load();

  test_merge_latency(); // DEBUG test

  // prepare xindex
  uint64_t init_val_data[1] = {1};
  std::vector<val_t> vals(exist_keys.size(), val_t(init_val_data, 1));
  xindex_t *tab_xi = new xindex_t(exist_keys, vals, fg_n, bg_n, std::string(workload_name)); // fg_n to create array of RCU status; bg_n background threads have been launched

  run_server(tab_xi, runtime);
  if (tab_xi != nullptr) delete tab_xi; // terminate_bg -> bg_master joins bg_threads

  COUT_THIS("[localtest] Exit successfully")
  exit(0);
}

inline void parse_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	fg_n = ini.get_server_num();
	workload_name = ini.get_workload_name();
	kv_bucket_num = ini.get_bucket_num();
}

inline void parse_args(int argc, char **argv) {
  struct option long_options[] = {
      {"read", required_argument, 0, 'a'},
      {"insert", required_argument, 0, 'b'},
      {"remove", required_argument, 0, 'c'},
      {"update", required_argument, 0, 'd'},
      {"scan", required_argument, 0, 'e'},
      {"runtime", required_argument, 0, 'g'},
      //{"fg", required_argument, 0, 'h'},
      {"bg", required_argument, 0, 'i'},
      {"xindex-root-err-bound", required_argument, 0, 'j'},
      {"xindex-root-memory", required_argument, 0, 'k'},
      {"xindex-group-err-bound", required_argument, 0, 'l'},
      {"xindex-group-err-tolerance", required_argument, 0, 'm'},
      {"xindex-buf-size-bound", required_argument, 0, 'n'},
      {"xindex-buf-compact-threshold", required_argument, 0, 'o'},
      {0, 0, 0, 0}};
  std::string ops = "a:b:c:d:e:f:g:i:j:k:l:m:n:o:";
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
	COUT_THIS("[localtest] # of exist keys = " << exist_keys.size());

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
	COUT_THIS("[localtest] # of nonexist keys = " << non_exist_keys.size());
}

void run_server(xindex_t *table, size_t sec) {
	int ret = 0;
	unsigned lcoreid = 1;

	running = false;

	// Prepare fg params
	pthread_t threads[fg_n];
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
    	sfg_params[worker_i].throughput = 0;
		sfg_params[worker_i].thread_id = static_cast<uint8_t>(worker_i);
		int ret = pthread_create(&threads[worker_i], nullptr, run_sfg, (void *)&sfg_params[worker_i]);
		if (ret) {
		  COUT_N_EXIT("Error:" << ret);
		}
		lcoreid++;
		if (lcoreid >= MAX_LCORE_NUM) {
			lcoreid = 1;
		}
	}

	COUT_THIS("[localtest] prepare server foreground threads...")
	while (ready_threads < fg_n) sleep(1);

	running = true;
	COUT_THIS("[localtest] start running...")
	std::vector<size_t> tput_history(fg_n, 0);
	size_t current_sec = 0;
	while (current_sec < sec) {
		sleep(1);
		uint64_t tput = 0;
		for (size_t i = 0; i < fg_n; i++) {
		  tput += sfg_params[i].throughput - tput_history[i];
		  tput_history[i] = sfg_params[i].throughput;
		}
		COUT_THIS("[localtest] >>> sec " << current_sec << " throughput: " << tput);
		++current_sec;
	}

	running = false;

	size_t throughput = 0;
	for (auto &p : sfg_params) {
		throughput += p.throughput;
	}
	COUT_THIS("[localtest] Throughput(op/s): " << throughput / sec);

	void *status;
	for (size_t i = 0; i < fg_n; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error:unable to join," << rc);
		}
	}
}

void *run_sfg(void * param) {
  // Parse param
  sfg_param_t &thread_param = *(sfg_param_t *)param;
  uint8_t thread_id = thread_param.thread_id;
  xindex_t *table = thread_param.table;

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
  uint64_t dummy_value_data[2] = {1234, 5678};
  val_t dummy_value = val_t(dummy_value_data, 2);
  size_t query_i = 0, insert_i = op_keys.size() / 2, delete_i = 0, update_i = 0;
  COUT_THIS("[localtest " << uint32_t(thread_id) << "] Ready.");

  ready_threads++;

#if !defined(NDEBUGGING_LOG)
  std::string logname;
  GET_STRING(logname, "tmp_localtest"<< uint32_t(thread_id)<<".out");
  std::ofstream ofs(logname, std::ofstream::out);
#endif

  // DEBUG TEST
  //uint32_t debugtest_idx = insert_i + 20;
  //uint32_t debugtest_i = 0;

  while (!running) {
  }

  while (running) {
	// DEBUG TEST
	/*int tmprun = 0;
	query_i = debugtest_idx;
	update_i = debugtest_idx;
	insert_i = debugtest_idx;
	delete_i = 0;
	if (debugtest_i == 0) tmprun = 2;
	debugtest_i++;*/

    double d = ratio_dis(gen);

	int tmprun = 4;
    //if (d <= read_ratio) {  // get
    if (tmprun == 0) {  // get
	  /*val_t tmp_val;
	  Key tmp_key;
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << tmp_key.to_string());
	  bool tmp_stat = table->get(tmp_key, tmp_val, thread_id);
	  if (!tmp_stat) {
		tmp_val = 0;
	  }
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << tmp_key.to_string() << " val = " << tmp_val);*/

	  val_t tmp_val;
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << op_keys[(query_i + delete_i) % op_keys.size()].to_string());
	  bool tmp_stat = table->get(op_keys[(query_i + delete_i) % op_keys.size()], tmp_val, thread_id);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << op_keys[(query_i + delete_i) % op_keys.size()].to_string() << " val = " << tmp_val.to_string());
      query_i++;
      if (unlikely(query_i == op_keys.size() / 2)) {
        query_i = 0;
      }
    //} else if (d <= read_ratio + update_ratio) {  // update
    } else if (tmprun == 1) {  // update
	  bool tmp_stat = table->put(op_keys[(update_i + delete_i) % op_keys.size()], dummy_value, thread_id);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << op_keys[(update_i + delete_i) % op_keys.size()].to_string() << " val = " << dummy_value.to_string()
			  << " stat = " << tmp_stat);
      update_i++;
      if (unlikely(update_i == op_keys.size() / 2)) {
        update_i = 0;
      }
    //} else if (d <= read_ratio + update_ratio + insert_ratio) {  // insert
    } else if (tmprun == 2) {  // insert
	  bool tmp_stat = table->put(op_keys[insert_i], dummy_value, thread_id);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << op_keys[insert_i].to_string() << " val = " << dummy_value.to_string()
			  << " stat = " << tmp_stat);
      insert_i++;
      if (unlikely(insert_i == op_keys.size())) {
        insert_i = 0;
      }
    //} else if (d <= read_ratio + update_ratio + insert_ratio + delete_ratio) {  // remove
    } else if (tmprun == 3) {  // remove
	  bool tmp_stat = table->remove(op_keys[delete_i], thread_id);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << op_keys[delete_i].to_string() << " stat = " << tmp_stat);
      delete_i++;
      if (unlikely(delete_i == op_keys.size())) {
        delete_i = 0;
      }
    } else {  // scan
	  std::vector<std::pair<index_key_t, val_t>> results;
	  size_t targetnum = 10;
	  size_t tmp_num = table->scan(op_keys[(query_i + delete_i) % op_keys.size()], targetnum, results, thread_id);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << op_keys[(query_i + delete_i) % op_keys.size()].to_string() << " num = " << tmp_num);
	  for (uint32_t val_i = 0; val_i < tmp_num; val_i++) {
		  FDEBUG_THIS(ofs, results[val_i].first.to_string());
		  FDEBUG_THIS(ofs, results[val_i].second.to_string());
	  }

		std::vector<std::pair<index_key_t, val_t>> merge_results;
		//std::map<index_key_t, val_t> *kvdata = listener_data;
		std::map<index_key_t, val_t> *kvdata = backup_data; // uncomment it for thpt
		if (kvdata != nullptr) {
			std::map<index_key_t, val_t>::iterator kviter = kvdata->lower_bound(
					op_keys[(query_i + delete_i) % op_keys.size()]);
			uint32_t result_idx = 0;
			if (kvdata->size() != 0 && results.size() != 0) {
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
					if (merge_results.size() == targetnum) {
						break;
					}
				}
			}
			if (merge_results.size() < targetnum) {
				// Only enter one loop
				for (; result_idx < results.size(); result_idx++) {
					merge_results.push_back(results[result_idx]);
					if (merge_results.size() == targetnum) {
						break;
					}
				}
				for (; kviter != kvdata->end(); kviter++) {
					merge_results.push_back(*kviter);
					if (merge_results.size() == targetnum) {
						break;
					}
				}
			}
		}
    	query_i++;
    	if (unlikely(query_i == op_keys.size() / 2)) {
    		query_i = 0;
    	}
    } // END of SCAN
    thread_param.throughput++;
  } // END of loop

  pthread_exit(nullptr);
#if !defined(NDEBUGGING_LOG)
  ofs.close();
#endif
  return 0;
}
