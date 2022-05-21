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
#include <map>
#include <sys/time.h> // struct timeval

#include "helper.h"
#include "key.h"
#include "iniparser/iniparser_wrapper.h"

#ifdef ORIGINAL_XINDEX
#include "original_xindex/xindex.h"
#include "original_xindex/xindex_impl.h"
#else
#include "val.h"
#ifdef DYNAMIC_MEMORY
#ifdef SEQ_MECHANISM
#include "extended_xindex_dynamic_seq/xindex.h"
#include "extended_xindex_dynamic_seq/xindex_impl.h"
#else
#include "extended_xindex_dynamic/xindex.h"
#include "extended_xindex_dynamic/xindex_impl.h"
#endif
#else
#include "extended_xindex/xindex.h"
#include "extended_xindex/xindex_impl.h"
#endif
#endif

#include "common_impl.h"

typedef xindex::XIndex<index_key_t, val_t> xindex_t;

struct alignas(CACHELINE_SIZE) SFGParam {
  xindex_t *table;
  uint64_t throughput;
  uint16_t thread_id;
};
typedef SFGParam sfg_param_t;

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

std::vector<index_key_t> exist_keys;
std::vector<index_key_t> non_exist_keys;

volatile bool running = false;
std::atomic<size_t> ready_threads(0);

std::map<index_key_t, val_t>* volatile backup_data = nullptr;

#ifndef ORIGINAL_XINDEX
char init_val_data[128] = {1};
val_t init_val(init_val_data, 128);
#else
val_t init_val = 1;
#endif

void test_merge_latency() {
	backup_data = new std::map<index_key_t, val_t>;
	for (size_t i = 0; i < switch_kv_bucket_num; i++) {
		backup_data->insert(std::pair<index_key_t, val_t>(exist_keys[i], init_val));
	}
}

int main(int argc, char **argv) {

  parse_ini("config.ini");
  parse_args(argc, argv);
  //xindex::init_options(); // init options of rocksdb
  load();

  //test_merge_latency(); // DEBUG test

  // prepare xindex
  printf("prepare xindex: loading phase\n");
  size_t training_cnt = 10 * 1000 * 1000;
  if (exist_keys.size() < training_cnt) {
	  training_cnt = exist_keys.size();
  }
  std::vector<index_key_t> training_keys = std::vector<index_key_t>(exist_keys.begin(), exist_keys.begin() + training_cnt);
#ifdef ORIGINAL_XINDEX
  xindex_t *tab_xi = new xindex_t(training_keys, std::vector<val_t>(training_cnt, 1), server_num, bg_n); // server_num to create array of RCU status; bg_n background threads have been launched
#else
  xindex_t *tab_xi = new xindex_t(training_keys, std::vector<val_t>(training_cnt, init_val), server_num, bg_n); // server_num to create array of RCU status; bg_n background threads have been launched
#endif

  printf("after training\n");
  if (exist_keys.size() > training_cnt) {
	  for (size_t i = training_cnt; i < exist_keys.size(); i++) {
#ifndef ORIGINAL_XINDEX
		  tab_xi->force_put(exist_keys[i], init_val, 0);
#else
		  tab_xi->put(exist_keys[i], init_val, 0);
#endif
	  }
  }

  printf("transaction phase\n");
  run_server(tab_xi, runtime);
  if (tab_xi != nullptr) delete tab_xi; // terminate_bg -> bg_master joins bg_threads

  COUT_THIS("[localtest] Exit successfully")
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

  COUT_VAR(server_num);
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
	pthread_t threads[server_num];
	sfg_param_t sfg_params[server_num];
	// check if parameters are cacheline aligned
	for (size_t i = 0; i < server_num; i++) {
		if ((uint64_t)(&(sfg_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(sfg_params[i]));
		}
	}

	// Launch workers
	for (size_t worker_i = 0; worker_i < server_num; worker_i++) {
		sfg_params[worker_i].table = table;
    	sfg_params[worker_i].throughput = 0;
		sfg_params[worker_i].thread_id = static_cast<uint16_t>(worker_i);
		ret = pthread_create(&threads[worker_i], nullptr, run_sfg, (void *)&sfg_params[worker_i]);
		if (ret) {
		  COUT_N_EXIT("Error:" << ret);
		}
		lcoreid++;
		if (lcoreid >= MAX_LCORE_NUM) {
			lcoreid = 1;
		}
	}

	COUT_THIS("[localtest] prepare server foreground threads...")
	while (ready_threads < server_num) sleep(1);

	running = true;
	COUT_THIS("[localtest] start running...")
	std::vector<size_t> tput_history(server_num, 0);
	size_t current_sec = 0;
	while (current_sec < sec) {
		sleep(1);
		uint64_t tput = 0;
		for (size_t i = 0; i < server_num; i++) {
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
	for (size_t i = 0; i < server_num; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error:unable to join," << rc);
		}
	}
}

void *run_sfg(void * param) {
  // Parse param
  sfg_param_t &thread_param = *(sfg_param_t *)param;
  uint16_t thread_id = thread_param.thread_id;
  xindex_t *table = thread_param.table;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> ratio_dis(0, 1);

  size_t exist_key_n_per_thread = exist_keys.size() / server_num;
  size_t exist_key_start = thread_id * exist_key_n_per_thread;
  size_t exist_key_end = (thread_id + 1) * exist_key_n_per_thread;
  //std::vector<index_key_t> op_keys(exist_keys.begin() + exist_key_start,
  //                                 exist_keys.begin() + exist_key_end);
  size_t op_exist_keys_size = exist_key_end - exist_key_start;
  size_t op_nonexist_keys_size = 0;
  size_t op_keys_size = op_exist_keys_size;

  if (non_exist_keys.size() > 0) {
    size_t non_exist_key_n_per_thread = non_exist_keys.size() / server_num;
    size_t non_exist_key_start = thread_id * non_exist_key_n_per_thread,
           non_exist_key_end = (thread_id + 1) * non_exist_key_n_per_thread;
    //op_keys.insert(op_keys.end(), non_exist_keys.begin() + non_exist_key_start,
    //               non_exist_keys.begin() + non_exist_key_end);
	op_nonexist_keys_size = (non_exist_key_end - non_exist_key_start);
	op_keys_size += op_nonexist_keys_size;
  }

#ifdef ORIGINAL_XINDEX
  val_t dummy_value = 1234;
#else
  char dummy_value_data[128] = {12, 34};
  val_t dummy_value = val_t(dummy_value_data, 128);
#endif
  size_t query_i = 0, insert_i = op_keys_size / 2, delete_i = 0, update_i = 0, scan_i = 0;
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

  uint32_t seqnum = 1;
  while (running) {
	// DEBUG TEST
	/*int tmprun = 0;
	query_i = debugtest_idx;
	update_i = debugtest_idx;
	insert_i = debugtest_idx;
	delete_i = 0;
	if (debugtest_i == 0) tmprun = 2;
	debugtest_i++;*/

	/*if (thread_id == 0) {
		table->make_snapshot(thread_id);
		FDEBUG_THIS(ofs, "make snapshot");
		continue;
	}*/

    double d = ratio_dis(gen);
	UNUSED(d);

	int tmprun = 1;
	UNUSED(tmprun);
	index_key_t curkey;
	size_t curkeyidx;
    size_t targetnum = 10;
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
	  uint32_t tmp_seq = 0;
	  curkeyidx = (query_i + delete_i) % op_keys_size;
	  if (curkeyidx < op_exist_keys_size) {
		  curkey = exist_keys[curkeyidx];
	  }
	  else {
		  curkey = non_exist_keys[curkeyidx - op_exist_keys_size];
	  }
#ifndef ORIGINAL_XINDEX
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << curkey.to_string());
	  bool tmp_stat = table->get(curkey, tmp_val, thread_id, tmp_seq);
#else
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << curkey.to_string() << " val = " << tmp_val.to_string());
	  bool tmp_stat = table->get(curkey, tmp_val, thread_id);
#endif
	  UNUSED(tmp_stat);
      query_i++;
      if (unlikely(query_i == op_keys_size / 2)) {
        query_i = 0;
      }
    //} else if (d <= read_ratio + update_ratio) {  // update
    } else if (tmprun == 1) {  // update
	  curkeyidx = (update_i + delete_i) % op_keys_size;
	  if (curkeyidx < op_exist_keys_size) {
		  curkey = exist_keys[curkeyidx];
	  }
	  else {
		  curkey = non_exist_keys[curkeyidx - op_exist_keys_size];
	  }
#ifndef ORIGINAL_XINDEX
	  bool tmp_stat = table->put(curkey, dummy_value, thread_id, seqnum);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << curkey.to_string() << " val = " << dummy_value.to_string()
			  << " stat = " << tmp_stat);
#else
	  bool tmp_stat = table->put(curkey, dummy_value, thread_id);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << curkey.to_string() << " val = " << dummy_value
			  << " stat = " << tmp_stat);
#endif
	  UNUSED(tmp_stat);
      update_i++;
      if (unlikely(update_i == op_keys_size / 2)) {
        update_i = 0;
      }
    //} else if (d <= read_ratio + update_ratio + insert_ratio) {  // insert
    } else if (tmprun == 2) {  // insert
	  curkeyidx = insert_i;
	  if (curkeyidx < op_exist_keys_size) {
		  curkey = exist_keys[curkeyidx];
	  }
	  else {
		  curkey = non_exist_keys[curkeyidx - op_exist_keys_size];
	  }
#ifndef ORIGINAL_XINDEX
	  bool tmp_stat = table->put(curkey, dummy_value, thread_id, seqnum);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << curkey.to_string() << " val = " << dummy_value.to_string()
			  << " stat = " << tmp_stat);
#else
	  bool tmp_stat = table->put(curkey, dummy_value, thread_id);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << curkey.to_string() << " val = " << dummy_value
			  << " stat = " << tmp_stat);
#endif
	  UNUSED(tmp_stat);
      insert_i++;
      if (unlikely(insert_i == op_keys_size)) {
        insert_i = 0;
      }
    //} else if (d <= read_ratio + update_ratio + insert_ratio + delete_ratio) {  // remove
    } else if (tmprun == 3) {  // remove
	  curkeyidx = delete_i;
	  if (curkeyidx < op_exist_keys_size) {
		  curkey = exist_keys[curkeyidx];
	  }
	  else {
		  curkey = non_exist_keys[curkeyidx - op_exist_keys_size];
	  }
#ifndef ORIGINAL_XINDEX
	  bool tmp_stat = table->remove(curkey, thread_id, seqnum);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << curkey.to_string() << " stat = " << tmp_stat);
#else
	  bool tmp_stat = table->remove(curkey, thread_id);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << curkey.to_string() << " stat = " << tmp_stat);
#endif
	  UNUSED(tmp_stat);
      delete_i++;
      if (unlikely(delete_i == op_keys_size)) {
        delete_i = 0;
      }
    } else {  // scan
	  size_t startkeyidx = scan_i % op_keys_size, endkeyidx = (scan_i + targetnum) % op_keys_size;
	  index_key_t startkey, endkey;
	  if (startkeyidx < op_exist_keys_size) {
		  startkey = exist_keys[startkeyidx];
	  }
	  else {
		  startkey = non_exist_keys[startkeyidx - op_exist_keys_size];
	  }
	  if (endkeyidx < op_exist_keys_size) {
		  endkey = exist_keys[endkeyidx];
	  }
	  else {
		  endkey = non_exist_keys[endkeyidx - op_exist_keys_size];
	  }
	  std::vector<std::pair<index_key_t, val_t>> results;
	  size_t tmp_num = table->range_scan(startkey, endkey, results, thread_id);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << startkey << " num = " << tmp_num);
	  for (uint32_t val_i = 0; val_i < tmp_num; val_i++) {
		  FDEBUG_THIS(ofs, results[val_i].first.to_string());
#ifndef ORIGINAL_XINDEX
		  FDEBUG_THIS(ofs, results[val_i].second.to_string());
#else
		  FDEBUG_THIS(ofs, results[val_i].second);
#endif
	  }

		// Add kv pairs of backup data into results
		//std::vector<std::pair<index_key_t, val_t>> merge_results;
		std::map<index_key_t, val_t> *kvdata = backup_data;
		if (kvdata != nullptr) {
			std::map<index_key_t, val_t>::iterator kviter = kvdata->lower_bound(startkey);
			for (; kviter != kvdata->end() && kviter->first < endkey; kviter++) {
				results.push_back(*kviter);
			}

			/*uint32_t result_idx = 0;
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
			}*/
		}
      scan_i++;
      if (unlikely(scan_i == op_keys_size / 2)) {
        scan_i = 0;
      }
    } // END of SCAN
    thread_param.throughput++;
	seqnum++;
  } // END of loop

  pthread_exit(nullptr);
#if !defined(NDEBUGGING_LOG)
  ofs.close();
#endif
  return 0;
}
