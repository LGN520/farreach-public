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
#include "ycsb/parser.h"
#include "rocksdb_wrapper.h"

#include "common_impl.h"

#if 0
#define CORRECTNESS_TEST
#endif

struct alignas(CACHELINE_SIZE) SFGParam {
	uint16_t thread_id;
};
typedef SFGParam sfg_param_t;

inline void parse_ini(const char * config_file);
void load(std::vector<netreach_key_t> &keys, std::vector<val_t> &vals);
void run_server();
void *run_sfg(void *param);

volatile bool running = false;
std::atomic<size_t> ready_threads(0);

RocksdbWrapper *db_wrappers = NULL;

int main(int argc, char **argv) {
	parse_ini("config.ini");

	RocksdbWrapper::prepare_rocksdb();

	db_wrappers = new RocksdbWrapper[server_num];
	INVARIANT(db_wrappers != NULL);

	run_server();

	COUT_THIS("[ycsb_loader] Exit successfully")
	exit(0);
}

void run_server() {
	int ret = 0;

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

	// Launch workers for loading phase
	for (uint16_t worker_i = 0; worker_i < server_num; worker_i++) {
		sfg_params[worker_i].thread_id = worker_i;
		int ret = pthread_create(&threads[worker_i], nullptr, run_sfg, (void *)&sfg_params[worker_i]);
		if (ret) {
			COUT_N_EXIT("Error:" << ret);
		}
	}

	COUT_THIS("[ycsb_loader] prepare workers...")
	while (ready_threads < server_num) sleep(1);

	running = true;
	COUT_THIS("[ycsb_loader] start running...")

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

	bool is_existing = db_wrappers[thread_id].open(thread_id);

	if (is_existing) { // not need loading phase
		ready_threads++;
		pthread_exit(nullptr);
		return 0;
	}

	char load_filename[256];
	memset(load_filename, '\0', 256);
	GET_SPLIT_WORKLOAD(load_filename, server_load_workload_dir, thread_id);

	Parser parser(load_filename);
	ParserIterator iter = parser.begin();
	netreach_key_t tmpkey;
	val_t tmpval;

	int res = 0;

	COUT_THIS("[ycsb_loader.worker " << uint32_t(thread_id) << "] Ready.");
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
		if (iter.type() == uint8_t(packet_type_t::PUTREQ)) {	// INESRT
#ifndef CORRECTNESS_TEST
			bool tmp_stat = db_wrappers[thread_id].force_put(tmpkey, tmpval);
			if (!tmp_stat) {
				COUT_N_EXIT("Loading phase: fail to put <" << tmpkey.to_string_for_print() << ", " << tmpval.to_string_for_print() << ">");
			}
#else
			val_t getval;
			uint32_t getseq;
			bool tmp_stat = db_wrappers[thread_id].get(tmpkey, getval, getseq);
			FDEBUG_THIS(ofs, "[local client " << uint32_t(thread_id) << "] key = " << tmpkey.to_string_for_print() 
					<< " getval = " << getval.to_string_for_print()
					<< " val = " << tmpval.to_string_for_print());
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
