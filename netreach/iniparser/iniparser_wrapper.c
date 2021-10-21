#include "iniparser_wrapper.h"

IniparserWrapper::IniparserWrapper() {
	ini = nullptr;
}
		
void IniparserWrapper::load(const char* filename) {
	ini = iniparser_load(filename);
	if (ini == nullptr) {
		printf("Cannot parse ini file: %s\n", filename);
		exit(-1);
	}
}

size_t IniparserWrapper::get_client_num() {
	int tmp = iniparser_getint(ini, "client:client_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [client:client_num]: %d\n", tmp);
		exit(-1);
	}
	return size_t(tmp);
}

size_t IniparserWrapper::get_server_num() {
	int tmp = iniparser_getint(ini, "server:server_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_num]: %d\n", tmp);
		exit(-1);
	}
	return size_t(tmp);
}

short IniparserWrapper::get_server_port() {
	int tmp = iniparser_getint(ini, "server:server_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

const char *IniparserWrapper::get_workload_name() {
	const char *workload_name = iniparser_getstring(ini, "global:workload_name", nullptr);
	if (workload_name == nullptr) {
		printf("Invalid entry of [global:workload_name]\n");
		exit(-1);
	}
	return workload_name;
}

uint32_t IniparserWrapper::get_bucket_num() {
	int tmp = iniparser_getint(ini, "switch:bucket_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:bucket_num]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}
