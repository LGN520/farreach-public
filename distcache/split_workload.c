#include <string.h>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "helper.h"

#include "common_impl.h"

int op = -1;
int linenum = -1;

uint32_t splitnum;
char output_dir[256];
char filename[256];

void *run_split_worker(void *param);

int main(int argc, char **argv) {

	if (argc != 3) {
		printf("Usage: ./split_workload load/run linenum\n");
		exit(-1);
	}

	if (strcmp(argv[1], "load") == 0) {
		op = 0; // load
	}
	else if (strcmp(argv[1], "run") == 0) {
		op = 1; // run
	}
	else {
		printf("Usage: ./split_workload load/run linenum\n");
		exit(-1);
	}
	linenum = atoi(argv[2]);
	printf("op: %d, linenum: %d\n", op, linenum);

	parse_ini("config.ini");

	/*char prefix[256];
	memset(prefix, '\0', 256);
	char *delimiter = strrchr(filename, '.');
	if (delimiter == nullptr) {
		memcpy(prefix, filename, 256);
	}
	else {
		strncpy(prefix, filename, delimiter - filename);
	}*/

	if (op == 0) { // load
		splitnum = max_server_total_logical_num;
		memcpy(output_dir, server_load_workload_dir, 256);
		memcpy(filename, raw_load_workload_filename, 256);
	}
	else if (op == 1) { // transaction / run
		splitnum = client_total_logical_num;
		memcpy(output_dir, client_workload_dir, 256);
		memcpy(filename, raw_run_workload_filename, 256);
	}

	struct stat dir_stat;
	if (stat(output_dir, &dir_stat) == 0 && S_ISDIR(dir_stat.st_mode)) {
		printf("Output directory exists: %s\n", output_dir);
		exit(-1);
	}
	else {
		int status = mkdir(output_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (status != 0) {
			printf("Fail to create directory: %s\n", output_dir);
			exit(-1);
		}
	}

	pthread_t split_worker_threads[client_total_logical_num];
	int split_worker_params[client_total_logical_num];
	for (size_t i = 0; i < client_total_logical_num; i++) {
		split_worker_params[i] = i;
		pthread_create(&split_worker_threads[i], nullptr, run_split_worker, (void *)&split_worker_params[i]);
	}
	COUT_THIS("start to split...\n");

	for (size_t i = 0; i < client_total_logical_num; i++) {
		pthread_join(split_worker_threads[i], NULL);
	}
	COUT_THIS("finish split...\n");
}

void *run_split_worker(void *param) {
	int splitidx = *((int *)param);

	char command[256];
	if (op == 0) { // load
		int per_loader_linenum = linenum / splitnum / server_load_factor;
		int line_startidx = splitidx * server_load_factor * per_loader_linenum + 1;
		int line_endidx = 0;

		char *output_names[server_load_factor] = {nullptr};
		//FILE *fps[splitnum * load_factor] = {nullptr};
		for (uint32_t i = 0; i < server_load_factor; i++) {
			int globalidx = splitidx * server_load_factor + i;
			if (globalidx == splitnum * server_load_factor - 1) {
				line_endidx = linenum;
			}
			else {
				line_endidx = line_startidx + per_loader_linenum - 1;
			}

			output_names[i] = new char[256];
			LOAD_SPLIT_WORKLOAD(output_names[i], output_dir, splitidx, i);

			sprintf(command, "sed -n \'%d,%dp\' %s > %s", line_startidx, line_endidx, filename, output_names[i]);
			system(command);
			/*fps[i] = fopen(output_names[i], "w+");
			if (fps[i] == nullptr) {
				printf("Fail to open %s!\n", output_names[i]);
				exit(-1);
			}*/

			line_startidx = line_endidx + 1;
		}

		for (uint32_t i = 0; i < server_load_factor; i++) {
			delete [] output_names[i];
			output_names[i] = nullptr;
			//close(fps[i]);
			//fps[i] = nullptr;
		}
	}
	else if (op == 1) {
		int per_client_linenum = linenum / splitnum;
		int line_startidx = splitidx * per_client_linenum + 1;
		int line_endidx = 0;

		char *output_name = nullptr;
		if (splitidx == splitnum - 1) {
			line_endidx = linenum;
		}
		else {
			line_endidx = line_startidx + per_client_linenum - 1;
		}

		output_name = new char[256];
		RUN_SPLIT_WORKLOAD(output_name, output_dir, splitidx);

		sprintf(command, "sed -n \'%d,%dp\' %s > %s", line_startidx, line_endidx, filename, output_name);
		system(command);
		/*fps[i] = fopen(output_names[i], "w+");
		if (fps[i] == nullptr) {
			printf("Fail to open %s!\n", output_names[i]);
			exit(-1);
		}*/

		delete [] output_name;
		output_name = nullptr;
	}
	pthread_exit(nullptr);
}
