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
#include "rocksdb_wrapper.h"
#include "snapshot_record.h"

#include "common_impl.h"

struct alignas(CACHELINE_SIZE) SFGParam {
  uint64_t throughput;
  uint16_t thread_id;
};
typedef SFGParam sfg_param_t;

inline void parse_ini(const char * config_file);
inline void parse_args(int, char **);
void load();
void run_server(size_t sec);
void *run_loader(void *param);
void *run_sfg(void *param);

// parameters
double read_ratio = 1;
double insert_ratio = 0;
double update_ratio = 0;
double delete_ratio = 0;
double scan_ratio = 0;
size_t runtime = 10;

std::vector<netreach_key_t> exist_keys;
std::vector<netreach_key_t> non_exist_keys;

volatile bool running = false;
std::atomic<size_t> ready_threads(0);

RocksdbWrapper *db_wrappers = NULL;

char init_val_data[128] = {1};
val_t init_val(init_val_data, 128);

int main(int argc, char **argv) {

  parse_ini("config.ini");
  parse_args(argc, argv);

  RocksdbWrapper::prepare_rocksdb();

  db_wrappers = new RocksdbWrapper[server_total_logical_num];
  INVARIANT(db_wrappers != NULL);
	for (int i = 0; i < server_total_logical_num; i++) {
		db_wrappers[i].init(CURMETHOD_ID);
	}

  load();

  //test_merge_latency(); // DEBUG test

  run_server(runtime);

  delete [] db_wrappers;
  db_wrappers = NULL;

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
      {0, 0, 0, 0}};
  std::string ops = "a:b:c:d:e:f:g:";
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
      default:
        abort();
    }
  }
}

void load() {
	std::ifstream fd("exist_keys.out", std::ios::in | std::ios::binary);
	INVARIANT(fd);
	while (true) {
		netreach_key_t tmp;
		fd.read((char *)&tmp, sizeof(netreach_key_t));
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
		netreach_key_t tmp;
		fd.read((char *)&tmp, sizeof(netreach_key_t));
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

void run_server(size_t sec) {
	int ret = 0;

	running = false;

	bool is_existing = false;
	for (size_t i = 0; i < server_total_logical_num; i++) {
  		is_existing = db_wrappers[i].open(i);
	}

	// Launch loaders
	if (!is_existing) {
		pthread_t loader_threads[server_total_logical_num * server_load_factor];
		size_t loader_ids[server_total_logical_num * server_load_factor];
		for (size_t loader_i = 0; loader_i < server_total_logical_num * server_load_factor; loader_i++) {
			loader_ids[loader_i] = loader_i;
			ret = pthread_create(&loader_threads[loader_i], nullptr, run_loader, (void *)&loader_ids[loader_i]);
			if (ret) {
			  COUT_N_EXIT("Error:" << ret);
			}
		}

		// wait for loaders
		void *loader_status;
		for (size_t i = 0; i < server_total_logical_num * server_load_factor; i++) {
			int loader_rc = pthread_join(loader_threads[i], &loader_status);
			if (loader_rc) {
			  COUT_N_EXIT("Error:unable to join," << loader_rc);
			}
		}
	}

	// Prepare fg params
	pthread_t threads[server_total_logical_num];
	sfg_param_t sfg_params[server_total_logical_num];
	// check if parameters are cacheline aligned
	for (size_t i = 0; i < server_total_logical_num; i++) {
		if ((uint64_t)(&(sfg_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(sfg_params[i]));
		}
	}

	// Launch workers
	for (size_t worker_i = 0; worker_i < server_total_logical_num; worker_i++) {
    	sfg_params[worker_i].throughput = 0;
		sfg_params[worker_i].thread_id = static_cast<uint16_t>(worker_i);
		ret = pthread_create(&threads[worker_i], nullptr, run_sfg, (void *)&sfg_params[worker_i]);
		if (ret) {
		  COUT_N_EXIT("Error:" << ret);
		}
	}

	COUT_THIS("[localtest] prepare server foreground threads...")
	while (ready_threads < server_total_logical_num) sleep(1);

	running = true;
	COUT_THIS("[localtest] start running...")
	std::vector<size_t> tput_history(server_total_logical_num, 0);
	size_t current_sec = 0;
	while (current_sec < sec) {
		sleep(1);
		uint64_t tput = 0;
		for (size_t i = 0; i < server_total_logical_num; i++) {
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
	for (size_t i = 0; i < server_total_logical_num; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error:unable to join," << rc);
		}
	}
}

void *run_loader(void * param) {
  // Parse param
  size_t loader_id = *((size_t *)param);
  uint16_t worker_id = static_cast<uint16_t>(loader_id/server_load_factor);
  printf("loader_id: %d, worker_id: %d\n", int(loader_id), int(worker_id));

  size_t exist_key_n_per_thread = exist_keys.size() / server_total_logical_num;
  size_t exist_key_n_per_loader = exist_keys.size() / server_total_logical_num / server_load_factor;
  size_t exist_key_start = worker_id * exist_key_n_per_thread + loader_id * exist_key_n_per_loader;
  //size_t exist_key_end = exist_key_start + exist_key_n_per_loader;

  //bool is_existing = db_wrappers[worker_id].open(worker_id);
  std::vector<val_t> init_vals(exist_key_n_per_loader, init_val);
  db_wrappers[worker_id].force_multiput(exist_keys.data() + exist_key_start, init_vals.data(), exist_key_n_per_loader);

  pthread_exit(nullptr);
  return 0;
}

void *run_sfg(void * param) {
  // Parse param
  sfg_param_t &thread_param = *(sfg_param_t *)param;
  uint16_t thread_id = thread_param.thread_id;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> ratio_dis(0, 1);

  size_t exist_key_n_per_thread = exist_keys.size() / server_total_logical_num;
  size_t exist_key_start = thread_id * exist_key_n_per_thread;
  size_t exist_key_end = (thread_id + 1) * exist_key_n_per_thread;
  //std::vector<netreach_key_t> op_keys(exist_keys.begin() + exist_key_start,
  //                                 exist_keys.begin() + exist_key_end);
  size_t op_exist_keys_size = exist_key_end - exist_key_start;
  size_t op_nonexist_keys_size = 0;
  size_t op_keys_size = op_exist_keys_size;

  if (non_exist_keys.size() > 0) {
    size_t non_exist_key_n_per_thread = non_exist_keys.size() / server_total_logical_num;
    size_t non_exist_key_start = thread_id * non_exist_key_n_per_thread,
           non_exist_key_end = (thread_id + 1) * non_exist_key_n_per_thread;
    //op_keys.insert(op_keys.end(), non_exist_keys.begin() + non_exist_key_start,
    //               non_exist_keys.begin() + non_exist_key_end);
	op_nonexist_keys_size = (non_exist_key_end - non_exist_key_start);
	op_keys_size += op_nonexist_keys_size;
  }

  char dummy_value_data[128] = {12, 34};
  val_t dummy_value = val_t(dummy_value_data, 128);
  size_t query_i = 0, insert_i = op_keys_size / 2, delete_i = 0, update_i = 0, scan_i = 0;

  //bool is_existing = db_wrappers[thread_id].open(thread_id);
  //UNUSED(is_existing);
  /*if (!is_existing) {
	  //for (size_t i = exist_key_start; i < exist_key_end; i++) {
	  //db_wrappers[thread_id].force_put(exist_keys[i], init_val);
	  //}
	  std::vector<val_t> init_vals(exist_keys.size(), init_val);
	  db_wrappers[thread_id].force_multiput(exist_keys.data(), init_vals.data(), exist_keys.size());
  }*/

  COUT_THIS("[localtest " << uint32_t(thread_id) << "] Ready.");
  ready_threads++;

#if !defined(NDEBUGGING_LOG)
  std::string logname;
  GET_STRING(logname, "tmp_localtest"<< uint32_t(thread_id)<<".out");
  std::ofstream ofs(logname, std::ofstream::out);
#endif

  while (!running) {
  }

  uint32_t seqnum = 1;
  while (running) {

    double d = ratio_dis(gen);
	UNUSED(d);
	int tmprun = 1;
	UNUSED(tmprun);

	netreach_key_t curkey;
	size_t curkeyidx;
    size_t targetnum = 10;

    //if (d <= read_ratio) {  // get
    if (tmprun == 0) {  // get
	  val_t tmp_val;
	  uint32_t tmp_seq = 0;
	  curkeyidx = (query_i + delete_i) % op_keys_size;
	  if (curkeyidx < op_exist_keys_size) {
		  curkey = exist_keys[curkeyidx];
	  }
	  else {
		  curkey = non_exist_keys[curkeyidx - op_exist_keys_size];
	  }
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << curkey.to_string_for_print());
	  bool tmp_stat = db_wrappers[thread_id].get(curkey, tmp_val, tmp_seq);
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
	  bool tmp_stat = db_wrappers[thread_id].put(curkey, dummy_value, seqnum);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << curkey.to_string_for_print() << " val = " << dummy_value.to_string_for_print()
			  << " stat = " << tmp_stat);
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
	  bool tmp_stat = db_wrappers[thread_id].put(curkey, dummy_value, seqnum);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << curkey.to_string_for_print() << " val = " << dummy_value.to_string_for_print()
			  << " stat = " << tmp_stat);
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
	  bool tmp_stat = db_wrappers[thread_id].remove(curkey, seqnum);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << curkey.to_string_for_print() << " stat = " << tmp_stat);
	  UNUSED(tmp_stat);
      delete_i++;
      if (unlikely(delete_i == op_keys_size)) {
        delete_i = 0;
      }
    } else {  // scan
	  size_t startkeyidx = scan_i % op_keys_size, endkeyidx = (scan_i + targetnum) % op_keys_size;
	  netreach_key_t startkey, endkey;
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
	  std::vector<std::pair<netreach_key_t, snapshot_record_t>> results;
	  size_t tmp_num = db_wrappers[thread_id].range_scan(startkey, endkey, results);
	  FDEBUG_THIS(ofs, "[localtest " << uint32_t(thread_id) << "] key = " << startkey << " num = " << tmp_num);
	  for (uint32_t val_i = 0; val_i < tmp_num; val_i++) {
		  FDEBUG_THIS(ofs, results[val_i].first.to_string_for_print());
		  FDEBUG_THIS(ofs, results[val_i].second.val.to_string_for_print());
		  FDEBUG_THIS(ofs, results[val_i].second.seq);
		  FDEBUG_THIS(ofs, results[val_i].second.stat);
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
