#include "iniparser_wrapper.h"
#include <string.h>

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

// Global

const char *IniparserWrapper::get_workload_name() {
	const char *workload_name = iniparser_getstring(ini, "global:workload_name", nullptr);
	if (workload_name == nullptr) {
		printf("Invalid entry of [global:workload_name]\n");
		exit(-1);
	}
	return workload_name;
}

uint32_t IniparserWrapper::get_max_vallen() {
	int tmp = iniparser_getint(ini, "global:max_vallen", -1);
	if (tmp == -1) {
		printf("Invalid entry of [global:max_vallen]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

// Client

size_t IniparserWrapper::get_client_num() {
	int tmp = iniparser_getint(ini, "client:client_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [client:client_num]: %d\n", tmp);
		exit(-1);
	}
	return size_t(tmp);
}

short IniparserWrapper::get_client_port() {
	int tmp = iniparser_getint(ini, "client:client_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [client:client_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

const char *IniparserWrapper::get_client_ip() {
	const char *client_ip = iniparser_getstring(ini, "client:client_ip", nullptr);
	if (client_ip == nullptr) {
		printf("Invalid entry of [client:client_ip]\n");
		exit(-1);
	}
	return client_ip;
}

void IniparserWrapper::get_client_mac(uint8_t *macaddr) {
	const char *client_mac = iniparser_getstring(ini, "client:client_mac", nullptr);
	if (client_mac == nullptr) {
		printf("Invalid entry of [client:client_mac]\n");
		exit(-1);
	}
	parse_mac(macaddr, client_mac);
}

// Server

size_t IniparserWrapper::get_split_num() {
	int tmp = iniparser_getint(ini, "other:split_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [other:split_num]: %d\n", tmp);
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

const char *IniparserWrapper::get_server_ip() {
	const char *server_ip = iniparser_getstring(ini, "server:server_ip", nullptr);
	if (server_ip == nullptr) {
		printf("Invalid entry of [server:server_ip]\n");
		exit(-1);
	}
	return server_ip;
}

void IniparserWrapper::get_server_mac(uint8_t *macaddr) {
	const char *server_mac = iniparser_getstring(ini, "server:server_mac", nullptr);
	if (server_mac == nullptr) {
		printf("Invalid entry of [server:server_mac]\n");
		exit(-1);
	}
	parse_mac(macaddr, server_mac);
}

const char* IniparserWrapper::get_server_backup_ip() {
	const char *server_backup_ip = iniparser_getstring(ini, "server:server_backup_ip", nullptr);
	if (server_backup_ip == nullptr) {
		printf("Invalid entry of [server:server_backup_ip]\n");
		exit(-1);
	}
	return server_backup_ip;
}

short IniparserWrapper::get_server_backup_port() {
	int tmp = iniparser_getint(ini, "server:server_backup_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_backup_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_server_pktloss_port() {
	int tmp = iniparser_getint(ini, "server:server_pktloss_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_pktloss_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_server_notified_port() {
	int tmp = iniparser_getint(ini, "server:server_notified_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_notified_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

// Switch

uint32_t IniparserWrapper::get_kv_bucket_num() {
	int tmp = iniparser_getint(ini, "switch:kv_bucket_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:kv_bucket_num]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

uint32_t IniparserWrapper::get_switch_max_vallen() {
	int tmp = iniparser_getint(ini, "switch:switch_max_vallen", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switch_max_vallen]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

short get_switchos_popserver_port() {
	int tmp = iniparser_getint(ini, "switch:switchos_popserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switchos_popserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short get_switchos_paramserver_port() {
	int tmp = iniparser_getint(ini, "switch:switchos_paramserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switchos_paramserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

const char* IniparserWrapper::get_switchos_ip() {
	const char *switchos_ip = iniparser_getstring(ini, "switchos:switchos_ip", nullptr);
	if (controller_ip == nullptr) {
		printf("Invalid entry of [switchos:switchos_ip]\n");
		exit(-1);
	}
	return switchos_ip;
}

// Controller

const char* IniparserWrapper::get_controller_ip() {
	const char *controller_ip = iniparser_getstring(ini, "controller:controller_ip", nullptr);
	if (controller_ip == nullptr) {
		printf("Invalid entry of [controller:controller_ip]\n");
		exit(-1);
	}
	return controller_ip;
}

short IniparserWrapper::get_controller_popserver_port() {
	int tmp = iniparser_getint(ini, "controller:controller_popserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [controller:controller_popserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

// Reflector

const char* IniparserWrapper::get_reflector_ip() {
	const char *reflector_ip = iniparser_getstring(ini, "reflector:reflector_ip", nullptr);
	if (reflector_ip == nullptr) {
		printf("Invalid entry of [reflector:reflector_ip]\n");
		exit(-1);
	}
	return reflector_ip;
}

short IniparserWrapper::get_reflector_port() {
	int tmp = iniparser_getint(ini, "reflector:reflector_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [reflector:reflector_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

// Helper func

void IniparserWrapper::parse_mac(uint8_t *macaddr, const char* macstr) {
	const char *curpos = macstr;
	for (size_t i = 0; i < 6; i++) {
		if (curpos == nullptr) {
			printf("Invalid mac string: %s\n", macstr);
			exit(-1);
		}
		else {
			macaddr[i] = uint8_t(strtol(curpos, nullptr, 16));
		}
		if (i != 5) {
			curpos = strchr(curpos, ':') + 1;
		}
		else {
			curpos = macstr + strlen(macstr);
		}
	}
}
