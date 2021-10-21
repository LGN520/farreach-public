#include <string.h>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ycsb/parser.h"
#include "helper.h"
#include "iniparser/iniparser_wrapper.h"

int main(int argc, char **argv) {

	if (argc != 4) {
		printf("Usage: ./split_workload load/run\n");
		exit(-1);
	}

	int op = 0; // load
	if (strcmp(argv[1], "run") == 0) {
		op = 1; // run
	}

	IniparserWrapper ini;
	ini.load("config.ini");
	char *workload_name = ini.get_workload_name();
	uint32_t splitnum = 0;
	if (op == 0) {
		splitnum = ini.get_split_num();
	}
	else {
		splitnum = ini.get_client_num();
	}

	/*char prefix[256];
	memset(prefix, '\0', 256);
	char *delimiter = strrchr(filename, '.');
	if (delimiter == nullptr) {
		memcpy(prefix, filename, 256);
	}
	else {
		strncpy(prefix, filename, delimiter - filename);
	}*/

	char output_dir[256];
	char filename[256];
	if (op == 0) {
		LOAD_SPLIT_DIR(output_dir, workload_name, splitnum);
		LOAD_RAW_WORKLOAD(filename, workload_name);
	}
	else {
		RUN_SPLIT_DIR(output_dir, workload_name, splitnum);
		RUN_RAW_WORKLOAD(filename, workload_name);
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
		delete output_names[i];
		output_names[i] = nullptr;
		fclose(fps[i]);
		fps[i] = nullptr;
	}
}
