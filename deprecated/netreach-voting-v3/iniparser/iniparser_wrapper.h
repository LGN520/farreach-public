#ifndef INIPARSER_WRAPPER_H
#define INIPARSER_WRAPPER_H

#include <stdint.h>
#include "iniparser.h"

class IniparserWrapper {
	public:
		IniparserWrapper();
		
		void load(const char* filename);

		const char *get_workload_name();
		uint32_t get_max_val_length();

		size_t get_client_num();
		short get_client_port();
		const char *get_client_ip();
		void get_client_mac(uint8_t* macaddr);
		
		size_t get_server_num();
		short get_server_port();
		const char *get_server_ip();
		void get_server_mac(uint8_t* macaddr);
		const char *get_server_backup_ip();
		short get_server_backup_port();

		uint32_t get_bucket_num();

		size_t get_split_num();

	private:
		dictionary *ini = nullptr;

		void parse_mac(uint8_t* macaddr, const char* macstr);
};

#endif
