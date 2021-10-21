#ifndef INIPARSER_WRAPPER_H
#define INIPARSER_WRAPPER_H

#include <stdint.h>
#include "iniparser.h"

class IniparserWrapper {
	public:
		IniparserWrapper();
		
		void load(const char* filename);

		size_t get_server_num();
		size_t get_client_num();
		short get_server_port();
		const char *get_workload_name();
		uint32_t get_bucket_num();

	private:
		dictionary *ini = nullptr;
};

#endif
