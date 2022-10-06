#include <getopt.h>
#include <map>
#include <vector>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
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
#include <unistd.h>
//#include <sys/time.h> // struct timeval

#include "../common/helper.h"
#include "../common/key.h"
#include "../common/val.h"
#include "../common/iniparser/iniparser_wrapper.h"

#ifdef USE_YCSB
#include "../common/workloadparser/ycsb_parser.h"
#elif defined USE_SYNTHETIC
#include "../common/workloadparser/synthetic_parser.h"
#endif

#include "common_impl.h"

int hot_threshold = 30;

struct alignas(CACHELINE_SIZE) FGParam {
	uint16_t thread_id;
	std::map<netreach_key_t, int> frequency_map;
};
typedef FGParam fg_param_t;

void run_benchmark();
void * run_checker(void *param);

std::atomic<size_t> finish_threads(0);

int main(int argc, char **argv) {

	if (argc != 2) {
		printf("Usage: ./skewness_check hot_threshold\n");
		exit(-1);
	}

	hot_threshold = atoi(argv[1]);

	parse_ini("config.ini");

	run_benchmark();

	exit(0);
}

void run_benchmark() {
	int ret = 0;

	// Prepare fg params
	pthread_t threads[client_total_logical_num];
	fg_param_t fg_params[client_total_logical_num];
	// check if parameters are cacheline aligned
	for (size_t i = 0; i < client_total_logical_num; i++) {
		if ((uint64_t)(&(fg_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(fg_params[i]));
		}
	}

	// Launch checkers
	for (uint16_t checker_i = 0; checker_i < client_total_logical_num; checker_i++) {
		fg_params[checker_i].thread_id = checker_i;
		fg_params[checker_i].frequency_map.clear();
		int ret = pthread_create(&threads[checker_i], nullptr, run_checker,
														 (void *)&fg_params[checker_i]);
	}

	while (finish_threads < client_total_logical_num) sleep(1);
	COUT_THIS("[client] all checkers finish!");

	/* Process statistics */
	COUT_THIS("[client] processing statistics");

	std::map<netreach_key_t, int> total_frequency_map;
	std::map<netreach_key_t, int> union_hot_frequency_map;
	for (uint16_t checker_i = 0; checker_i < client_total_logical_num; checker_i++) {
		//std::vector<std::pair<netreach_key_t, int>> hotfrequency_list;
		int hot_keynum = 0;
		int hot_sumfrequency = 0;
		for (std::map<netreach_key_t, int>::iterator iter = fg_params[checker_i].frequency_map.begin();\
				iter != fg_params[checker_i].frequency_map.end(); iter++) {
			if (iter->second > hot_threshold) {
				hot_keynum += 1;
				hot_sumfrequency += iter->second;

				if (union_hot_frequency_map.find(iter->first) == union_hot_frequency_map.end()) {
					union_hot_frequency_map.insert(*iter);
				}
				else {
					union_hot_frequency_map[iter->first] += iter->second;
				}
			}

			if (total_frequency_map.find(iter->first) == total_frequency_map.end()) {
				total_frequency_map.insert(*iter);
			}
			else {
				total_frequency_map[iter->first] += iter->second;
			}
		}
		printf("[checker %d] hot_keynum: %d, hot_sumfrequency: %d\n", checker_i, hot_keynum, hot_sumfrequency);
	}

	int union_hot_keynum = 0;
	int union_hot_sumfrequency = 0;
	for (std::map<netreach_key_t, int>::iterator iter = union_hot_frequency_map.begin(); \
			iter != union_hot_frequency_map.end(); iter++) {
		union_hot_keynum += 1;
		union_hot_sumfrequency += iter->second;
	}
	printf("union_hot_keynum: %d, union_hot_sumfrequency: %d\n", union_hot_keynum, union_hot_sumfrequency);

	int total_hot_keynum = 0;
	int total_hot_sumfrequency = 0;
	int total_keynum = 0;
	int total_sumfrequency = 0;
	for (std::map<netreach_key_t, int>::iterator iter = total_frequency_map.begin(); \
			iter != total_frequency_map.end(); iter++) {
		if (iter->second > hot_threshold) {
			total_hot_keynum += 1;
			total_hot_sumfrequency += iter->second;
		}
		total_keynum += 1;
		total_sumfrequency += iter->second;
	}
	printf("total_hot_keynum: %d, total_hot_sumfrequency: %d, total_keynum: %d, total_sumfrequency: %d\n", \
				total_hot_keynum, total_hot_sumfrequency, total_keynum, total_sumfrequency);

	COUT_THIS("Finish dumping statistics!")
}

void * run_checker(void *param) {
	fg_param_t &thread_param = *(fg_param_t *)param;
	uint16_t thread_id = thread_param.thread_id;
	std::map<netreach_key_t, int> &frequency_map = thread_param.frequency_map;

	// ycsb parser
	char load_filename[256];
	memset(load_filename, '\0', 256);
	RUN_SPLIT_WORKLOAD(load_filename, client_workload_dir, thread_id);

	ParserIterator *iter = NULL;
#ifdef USE_YCSB
	iter = new YcsbParserIterator (load_filename);
#elif defined USE_SYNTHETIC
	iter = new SyntheticParserIterator(load_filename);
#endif
	INVARIANT(iter != NULL);

	netreach_key_t tmpkey;
	val_t tmpval;

	while (true) {
		if (!iter->next()) {
			break;
		}

		std::map<netreach_key_t, int>::iterator mapiter = frequency_map.find(iter->key());
		if (mapiter == frequency_map.end()) {
			frequency_map.insert(std::pair<netreach_key_t, int>(iter->key(), 1));
		}
		else {
			mapiter->second += 1;
		}
	}

	iter->closeiter();
	delete iter;
	iter = NULL;

	finish_threads++;
	pthread_exit(nullptr);
	return 0;
}
