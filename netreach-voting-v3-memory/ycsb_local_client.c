#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
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
#include "ycsb/parser.h"
#include "iniparser/iniparser_wrapper.h"

#if 0
#define CORRECTNESS_TEST
#endif

struct alignas(CACHELINE_SIZE) SFGParam;

typedef Key index_key_t;
typedef Val val_t;
typedef xindex::XIndex<index_key_t, val_t> xindex_t;
typedef SFGParam sfg_param_t;

inline void parse_ini(const char * config_file);
void load(std::vector<index_key_t> &keys, std::vector<val_t> &vals);
void run_server(xindex_t *table);
void *run_sfg(void *param);

// parameters
size_t split_n;
size_t fg_n = 1;
const char *workload_name;
char output_dir[256];

volatile bool running = false;
std::atomic<size_t> ready_threads(0);

struct alignas(CACHELINE_SIZE) SFGParam {
	xindex_t *table;
	uint8_t thread_id;
};

int main(int argc, char **argv) {
	parse_ini("config.ini");
	xindex::init_options(); // init options of rocksdb

	// prepare xindex

#ifndef CORRECTNESS_TEST
	std::vector<index_key_t> keys;
	std::vector<val_t> vals;
	load(keys, vals); // Use the last split workload to initialize the XIndex
	xindex_t *tab_xi = new xindex_t(keys, vals, fg_n, 0, workload_name); // fg_n to create array of RCU status; bg_n background threads have been launched
#else
	xindex_t *tab_xi = new xindex_t(fg_n, 0, workload_name);
#endif

	if (fg_n > 0) {
		run_server(tab_xi);
	}
	if (tab_xi != nullptr) delete tab_xi; // terminate_bg -> bg_master joins bg_threads

	COUT_THIS("[localtest] Exit successfully")
	exit(0);
}

inline void parse_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	split_n = ini.get_split_num();
	INVARIANT(split_n >= 2);
#ifndef CORRECTNESS_TEST
	fg_n = split_n - 1;
#else
	fg_n = split_n;
#endif
	workload_name = ini.get_workload_name();

	COUT_VAR(split_n);
	COUT_VAR(fg_n);
	COUT_VAR(std::string(workload_name, strlen(workload_name)));

	LOAD_SPLIT_DIR(output_dir, workload_name, split_n); // get the split directory for loading phase
	struct stat dir_stat;
	if (!(stat(output_dir, &dir_stat) == 0 && S_ISDIR(dir_stat.st_mode))) {
		printf("Output directory does not exist: %s\n", output_dir);
		exit(-1);
	}
}

void load(std::vector<index_key_t> &keys, std::vector<val_t> &vals) {
	char load_filename[256];
	memset(load_filename, '\0', 256);
	GET_SPLIT_WORKLOAD(load_filename, output_dir, split_n-1);

	Parser parser(load_filename);
	ParserIterator iter = parser.begin();
	index_key_t tmpkey;
	val_t tmpval;

	std::map<index_key_t, val_t> loadmap;
	while (true) {
		tmpkey = iter.key();
		tmpval = iter.val();	
		if (iter.type() == uint8_t(packet_type_t::PUT_REQ)) {	// INESRT
			loadmap.insert(std::pair<index_key_t, val_t>(tmpkey, tmpval));
		}
		else {
			COUT_N_EXIT("Invalid type: !" << int(iter.type()));
		}
		if (!iter.next()) {
			break;
		}
	}
	iter.close();

	keys.resize(loadmap.size());
	vals.resize(loadmap.size());
	size_t i = 0;
	for (std::map<index_key_t, val_t>::iterator iter = loadmap.begin(); iter != loadmap.end(); iter++) {
		keys[i] = iter->first;
		vals[i] = iter->second;
		i++;
	}
}

void run_server(xindex_t *table) {
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

	COUT_THIS("[local client] prepare server foreground threads...")
	while (ready_threads < fg_n) sleep(1);

	running = true;
	COUT_THIS("[local client] start running...")

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

	char load_filename[256];
	memset(load_filename, '\0', 256);
	GET_SPLIT_WORKLOAD(load_filename, output_dir, thread_id);

	Parser parser(load_filename);
	ParserIterator iter = parser.begin();
	index_key_t tmpkey;
	val_t tmpval;

	int res = 0;
	COUT_THIS("[local client " << uint32_t(thread_id) << "] Ready.");

	ready_threads++;

#if !defined(NDEBUGGING_LOG)
  std::string logname;
  GET_STRING(logname, "tmp_localtest"<< uint32_t(thread_id)<<".out");
  std::ofstream ofs(logname, std::ofstream::out);
#endif

	while (!running) {
	}

	while (running) {
		tmpkey = iter.key();
		tmpval = iter.val();	
		if (iter.type() == uint8_t(packet_type_t::PUT_REQ)) {	// INESRT
#ifndef CORRECTNESS_TEST
			bool tmp_stat = table->data_put(tmpkey, tmpval, thread_id);
			if (!tmp_stat) {
				COUT_N_EXIT("Loading phase: fail to put <" << tmpkey.to_string() << ", " << tmpval.to_string() << ">");
			}
#else
			val_t getval;
			bool tmp_stat = table->get(tmpkey, getval, thread_id);
			FDEBUG_THIS(ofs, "[local client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string() 
					<< " getval = " << getval.to_string()
					<< " val = " << tmpval.to_string());
#endif
		}
		else {
			COUT_N_EXIT("Invalid type: !" << int(iter.type()));
		}
		if (!iter.next()) {
			break;
		}
	}

	pthread_exit(nullptr);
	iter.close();
#if !defined(NDEBUGGING_LOG)
	ofs.close();
#endif
	return 0;
}
