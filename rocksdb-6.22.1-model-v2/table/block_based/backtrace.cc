#include "table/block_based/backtrace.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <cstdarg>

void backtrace() {
	int nptrs;
	void *buffer[128];
	char **strings;
	nptrs = backtrace(buffer, 128);
	strings = backtrace_symbols(buffer, nptrs);
	if (strings != NULL) {
		for (int j = 0; j < nptrs; j++)
			print_msg("%s\n", strings[j]);
		free(strings);
	}
}

void print_msg(const char * fmt, ...) {
	//FILE *fd = fopen("debug.out", "a+");
	std::va_list args;
	va_start(args, fmt);
	//fprintf(fd, fmt, args);
	printf(fmt, args);
	va_end(args);
	//fclose(fd);
}
