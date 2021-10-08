#include <string.h>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ycsb/parser.h"

int main(int argc, char **argv) {

	if (argc != 3) {
		printf("Usage: ./split_workload filename threadnum\n");
		exit(-1);
	}


	if (strlen(argv[1]) >= 256) {
		printf("Filename is too long: %s\n", argv[1]);
		exit(-1);
	}

	char filename[256];
	memset(filename, '\0', 256);
	memcpy(filename, argv[1], strlen(argv[1]));

	uint32_t threadnum = 0;
	threadnum = std::stoi(argv[2]);

	char prefix[256];
	memset(prefix, '\0', 256);
	char *delimiter = strrchr(filename, '.');
	if (delimiter == nullptr) {
		memcpy(prefix, filename, 256);
	}
	else {
		strncpy(prefix, filename, delimiter - filename);
	}

	char output_dir[512];
	sprintf(output_dir, "%s-%s", prefix, argv[2]);
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


	char *output_names[threadnum] = {nullptr};
	FILE *fps[threadnum] = {nullptr};
	for (uint32_t i = 0; i < threadnum; i++) {
		output_names[i] = new char[256];
		sprintf(output_names[i], "%s/%s%u.out", output_dir, prefix, i);
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
		if (line_idx >= threadnum) {
			line_idx = 0;
		}
		if (!iter.next()) {
			break;
		}
	}

	for (uint32_t i = 0; i < threadnum; i++) {
		delete output_names[i];
		output_names[i] = nullptr;
		fclose(fps[i]);
		fps[i] = nullptr;
	}
}
