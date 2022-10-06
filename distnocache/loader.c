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
#include "rocksdb_wrapper.h"

#ifdef USE_YCSB
#include "workloadparser/ycsb_parser.h"
#elif defined USE_SYNTHETIC
#include "workloadparser/synthetic_parser.h"
#endif

#include "common_impl.h"

#if 0
#define CORRECTNESS_TEST
#endif

inline void parse_ini(const char * config_file);
void load(std::vector<netreach_key_t> &keys, std::vector<val_t> &vals);
void run_server();
void *run_loader(void *param);

volatile bool running = false;
std::atomic<size_t> ready_threads(0);

RocksdbWrapper *db_wrappers = NULL;

int main(int argc, char **argv) {
	parse_ini("config.ini");

	RocksdbWrapper::prepare_rocksdb();

	db_wrappers = new RocksdbWrapper[max_server_total_logical_num];
	INVARIANT(db_wrappers != NULL);
	for (int i = 0; i < max_server_total_logical_num; i++) {
		db_wrappers[i].init(CURMETHOD_ID);
	}

	run_server();

	delete [] db_wrappers;
	db_wrappers = NULL;

	COUT_THIS("[ycsb_loader] Exit successfully")
	exit(0);
}

void run_server() {
	int ret = 0;

	running = false;

	bool is_existing = false;
	for (size_t i = 0; i < max_server_total_logical_num; i++) {
  		is_existing = db_wrappers[i].open(i);
	}
	if (is_existing) {
		printf("Database has been created before ycsb_loader\n");
		exit(-1);
	}

	// Launch loaders
	pthread_t loader_threads[max_server_total_logical_num * server_load_factor];
	size_t loader_ids[max_server_total_logical_num * server_load_factor];
	for (size_t loader_i = 0; loader_i < max_server_total_logical_num * server_load_factor; loader_i++) {
		loader_ids[loader_i] = loader_i;
		ret = pthread_create(&loader_threads[loader_i], nullptr, run_loader, (void *)&loader_ids[loader_i]);
		if (ret) {
		  COUT_N_EXIT("Error:" << ret);
		}
	}

	COUT_THIS("[ycsb_loader] prepare loaders...")
	while (ready_threads < max_server_total_logical_num * server_load_factor) sleep(1);

	running = true;
	COUT_THIS("[ycsb_loader] start running...")

	void *status;
	for (size_t i = 0; i < max_server_total_logical_num * server_load_factor; i++) {
		int rc = pthread_join(loader_threads[i], &status);
		if (rc) {
			COUT_N_EXIT("Error:unable to join," << rc);
		}
	}

	COUT_THIS("[ycsb_loader] all loaders finish")
	/*COUT_THIS("[ycsb_loader] all loaders finish -> make snapshot...")
	for (size_t i = 0; i < max_server_total_logical_num; i++) {
  		db_wrappers[i].make_snapshot();
		db_wrappers[i].stop_snapshot();
	}*/
}

void *run_loader(void * param) {
	// Parse param
	size_t loader_id = *((size_t *)param);
	uint16_t worker_id = static_cast<uint16_t>(loader_id/server_load_factor);
	printf("loader_id: %d, worker_id: %d\n", int(loader_id), int(worker_id));

	/*bool is_existing = db_wrappers[thread_id].open(thread_id);
	if (is_existing) { // not need loading phase
		ready_threads++;
		pthread_exit(nullptr);
		return 0;
	}*/

	char load_filename[256];
	memset(load_filename, '\0', 256);
	LOAD_SPLIT_WORKLOAD(load_filename, server_load_workload_dir, worker_id, loader_id);

	ParserIterator *iter = NULL;
#ifdef USE_YCSB
	iter = new YcsbParserIterator (load_filename);
#elif defined USE_SYNTHETIC
	iter = new SyntheticParserIterator(load_filename);
#endif
	INVARIANT(iter != NULL);

	int res = 0;

	COUT_THIS("[ycsb_loader.loader " << uint32_t(loader_id) << "] Ready.");
	ready_threads++;

#if !defined(NDEBUGGING_LOG)
  std::string logname;
  GET_STRING(logname, "tmp_localtest"<< uint32_t(thread_id)<<".out");
  std::ofstream ofs(logname, std::ofstream::out);
#endif

	while (!running) {
	}

	//netreach_key_t tmpkey;
	//val_t tmpval;
	while (running) {
		/*if (!iter->next()) {
			break;
		}

		tmpkey = iter->key();
		uint32_t tmp_worker_id = tmpkey.get_hashpartition_idx(partition_num, max_server_total_logical_num);
		tmpval = iter->val();	
		if (iter->type() == optype_t(packet_type_t::PUTREQ)) {	// INESRT
#ifndef CORRECTNESS_TEST
			bool tmp_stat = db_wrappers[tmp_worker_id].force_put(tmpkey, tmpval);
			if (!tmp_stat) {
				COUT_N_EXIT("Loading phase: fail to put <" << tmpkey.to_string_for_print() << ", " << tmpval.to_string_for_print() << ">");
			}
#else
			val_t getval;
			uint32_t getseq;
			bool tmp_stat = db_wrappers[tmp_worker_id].get(tmpkey, getval, getseq);
			FDEBUG_THIS(ofs, "[local client " << uint32_t(tmp_worker_id) << "] key = " << tmpkey.to_string_for_print() 
					<< " getval = " << getval.to_string_for_print()
					<< " val = " << tmpval.to_string_for_print());
#endif
		}
		else {
			COUT_N_EXIT("Invalid type: !" << int(iter->type()));
		}*/

		if (!iter->next_batch()) {
			break;
		}

		optype_t *tmptypes = iter->types();
		int tmpmaxidx = iter->maxidx();
		for (int i = 0; i < tmpmaxidx; i++) {
			if (tmptypes[i] != optype_t(packet_type_t::PUTREQ)) {
				COUT_N_EXIT("Invalid type: !" << int(tmptypes[i]));
			}
		}

		// split current batch into different databases by hash partition
		std::map<uint16_t, std::pair<std::vector<netreach_key_t>, std::vector<val_t>>> tmpmap;
		for (int i = 0; i < tmpmaxidx; i++) {
#ifdef USE_HASH
			uint16_t tmp_worker_id = iter->keys()[i].get_hashpartition_idx(switch_partition_count, max_server_total_logical_num);
#elif defined USE_RANGE
			uint16_t tmp_worker_id = iter->keys()[i].get_rangepartition_idx(max_server_total_logical_num);
#endif
			if (tmpmap.find(tmp_worker_id) == tmpmap.end()) {
				std::vector<netreach_key_t> tmpkeyvec;
				std::vector<val_t> tmpvalvec;
				tmpkeyvec.push_back(iter->keys()[i]);
				tmpvalvec.push_back(iter->vals()[i]);
				tmpmap.insert(std::pair<uint16_t, std::pair<std::vector<netreach_key_t>, std::vector<val_t>>>(\
							tmp_worker_id, std::pair<std::vector<netreach_key_t>, std::vector<val_t>>(tmpkeyvec, tmpvalvec)));
			}
			else {
				tmpmap[tmp_worker_id].first.push_back(iter->keys()[i]);
				tmpmap[tmp_worker_id].second.push_back(iter->vals()[i]);
			}
		}

		for (std::map<uint16_t, std::pair<std::vector<netreach_key_t>, std::vector<val_t>>>::iterator tmpiter = tmpmap.begin(); \
				tmpiter != tmpmap.end(); tmpiter++) {
			uint16_t tmp_worker_id = tmpiter->first;
			bool tmp_stat = db_wrappers[tmp_worker_id].force_multiput(tmpiter->second.first.data(), tmpiter->second.second.data(), tmpiter->second.first.size());
			if (!tmp_stat) {
				printf("Loading phase: fail to multiput %d records\n", tmpmaxidx);
				exit(-1);
			}
		}
	}

	pthread_exit(nullptr);
	iter->closeiter();
	delete iter;
	iter = NULL;
#if !defined(NDEBUGGING_LOG)
	ofs.close();
#endif
	return 0;
}
