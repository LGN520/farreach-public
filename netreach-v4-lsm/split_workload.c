#include <string.h>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "helper.h"

#include "common_impl.h"

int main(int argc, char **argv) {

	if (argc != 3) {
		printf("Usage: ./split_workload load/run linenum\n");
		exit(-1);
	}

	int op = -1;
	if (strcmp(argv[1], "load") == 0) {
		op = 0; // load
	}
	else if (strcmp(argv[1], "run") == 0) {
		op = 1; // run
	}
	else {
		printf("Usage: ./split_workload load/run\n");
		exit(-1);
	}
	int linenum = atoi(argv[2]);
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

	uint32_t splitnum;
	char output_dir[256];
	char filename[256];
	if (op == 0) { // load
		splitnum = server_num;
		memcpy(output_dir, server_load_workload_dir, 256);
		memcpy(filename, raw_load_workload_filename, 256);
	}
	else if (op == 1) { // transaction / run
		splitnum = client_num;
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

	int line_startidx = 1;
	int line_endidx = 0;
	char command[256];
	if (op == 0) {
		int per_loader_linenum = linenum / splitnum / load_factor;
		char *output_names[splitnum * load_factor] = {nullptr};
		//FILE *fps[splitnum * load_factor] = {nullptr};
		for (uint32_t i = 0; i < splitnum * load_factor; i++) {
			if (i == splitnum * load_factor - 1) {
				line_endidx = linenum;
			}
			else {
				line_endidx = line_startidx + per_loader_linenum - 1;
			}

			output_names[i] = new char[256];
			LOAD_SPLIT_WORKLOAD(output_names[i], output_dir, i/load_factor, i);

			sprintf(command, "sed -n \'%d,%dp\' %s > %s", line_startidx, line_endidx, filename, output_names[i]);
			system(command);
			/*fps[i] = fopen(output_names[i], "w+");
			if (fps[i] == nullptr) {
				printf("Fail to open %s!\n", output_names[i]);
				exit(-1);
			}*/

			line_startidx = line_endidx + 1;
		}

		for (uint32_t i = 0; i < splitnum * load_factor; i++) {
			delete [] output_names[i];
			output_names[i] = nullptr;
			//close(fps[i]);
			//fps[i] = nullptr;
		}
	}
	else if (op == 1) {
		int per_client_linenum = linenum / splitnum;

		char *output_names[splitnum] = {nullptr};
		//FILE *fps[splitnum] = {nullptr};
		for (uint32_t i = 0; i < splitnum; i++) {
			if (i == splitnum - 1) {
				line_endidx = linenum;
			}
			else {
				line_endidx = line_startidx + per_client_linenum - 1;
			}

			output_names[i] = new char[256];
			RUN_SPLIT_WORKLOAD(output_names[i], output_dir, i);

			sprintf(command, "sed -n \'%d,%dp\' %s > %s", line_startidx, line_endidx, filename, output_names[i]);
			system(command);
			/*fps[i] = fopen(output_names[i], "w+");
			if (fps[i] == nullptr) {
				printf("Fail to open %s!\n", output_names[i]);
				exit(-1);
			}*/

			line_startidx = line_endidx + 1;
		}

		for (uint32_t i = 0; i < splitnum; i++) {
			delete [] output_names[i];
			output_names[i] = nullptr;
			//close(fps[i]);
			//fps[i] = nullptr;
		}
	}
}
