#include <string.h>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ycsb/parser.h"
#include "helper.h"

#include "common_impl.h"

int main(int argc, char **argv) {

	if (argc != 2) {
		printf("Usage: ./split_workload load/run\n");
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

	char *output_names[splitnum] = {nullptr};
	FILE *fps[splitnum] = {nullptr};
	for (uint32_t i = 0; i < splitnum; i++) {
		output_names[i] = new char[256];
		GET_SPLIT_WORKLOAD(output_names[i], output_dir, i);
		fps[i] = fopen(output_names[i], "w+");
		if (fps[i] == nullptr) {
			printf("Fail to open %s!\n", output_names[i]);
			exit(-1);
		}
	}

	Parser parser(filename);
	ParserIterator iter = parser.begin();
	uint32_t line_idx = 0;
	while (true) {
		fwrite(iter.line().c_str(), iter.line().length(), 1, fps[line_idx]);
		line_idx++;
		if (line_idx >= splitnum) {
			line_idx = 0;
		}
		if (!iter.next()) {
			break;
		}
	}

	for (uint32_t i = 0; i < splitnum; i++) {
		delete [] output_names[i];
		output_names[i] = nullptr;
		fclose(fps[i]);
		fps[i] = nullptr;
	}
}
