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

/* config.ini */

// Global

const char *IniparserWrapper::get_workload_name() {
	const char *workload_name = iniparser_getstring(ini, "global:workload_name", nullptr);
	if (workload_name == nullptr) {
		printf("Invalid entry of [global:workload_name]\n");
		exit(-1);
	}
	return workload_name;
}

int IniparserWrapper::get_workload_mode() {
	int tmp = iniparser_getint(ini, "global:workload_mode", -1);
	if (tmp == -1) {
		printf("Invalid entry of [global:workload_mode]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_dynamic_periodnum() {
	int tmp = iniparser_getint(ini, "global:dynamic_periodnum", -1);
	if (tmp == -1) {
		printf("Invalid entry of [global:dynamic_periodnum]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_dynamic_periodinterval() {
	int tmp = iniparser_getint(ini, "global:dynamic_periodinterval", -1);
	if (tmp == -1) {
		printf("Invalid entry of [global:dynamic_periodinterval]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

const char *IniparserWrapper::get_dynamic_ruleprefix() {
	const char *dynamic_ruleprefix = iniparser_getstring(ini, "global:dynamic_ruleprefix", nullptr);
	if (dynamic_ruleprefix == nullptr) {
		printf("Invalid entry of [global:dynamic_ruleprefix]\n");
		exit(-1);
	}
	return dynamic_ruleprefix;
}

uint16_t IniparserWrapper::get_max_vallen() {
	int tmp = iniparser_getint(ini, "global:max_vallen", -1);
	if (tmp == -1) {
		printf("Invalid entry of [global:max_vallen]: %d\n", tmp);
		exit(-1);
	}
	return uint16_t(tmp);
}

uint32_t IniparserWrapper::get_load_batch_size() {
	int tmp = iniparser_getint(ini, "global:load_batch_size", -1);
	if (tmp == -1) {
		printf("Invalid entry of [global:load_batch_size]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

uint32_t IniparserWrapper::get_client_physical_num() {
	int tmp = iniparser_getint(ini, "global:client_physical_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [global:client_physical_num]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

uint32_t IniparserWrapper::get_server_physical_num() {
	int tmp = iniparser_getint(ini, "global:server_physical_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [global:server_physical_num]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

uint32_t IniparserWrapper::get_client_total_logical_num() {
	int tmp = iniparser_getint(ini, "global:client_total_logical_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [global:client_total_logical_num]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

uint32_t IniparserWrapper::get_server_total_logical_num() {
	int tmp = iniparser_getint(ini, "global:server_total_logical_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [global:server_total_logical_num]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

uint32_t IniparserWrapper::get_server_total_logical_num_for_rotation() {
	int tmp = iniparser_getint(ini, "global:server_total_logical_num_for_rotation", -1);
	if (tmp == -1) {
		printf("Invalid entry of [global:server_total_logical_num_for_rotation]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

// common client configuration

short IniparserWrapper::get_client_rotationdataserver_port() {
	int tmp = iniparser_getint(ini, "client:client_rotationdataserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [client:client_rotationdataserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_client_sendpktserver_port_start() {
	int tmp = iniparser_getint(ini, "client:client_sendpktserver_port_start", -1);
	if (tmp == -1) {
		printf("Invalid entry of [client:client_sendpktserver_port_start]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_client_rulemapserver_port_start() {
	int tmp = iniparser_getint(ini, "client:client_rulemapserver_port_start", -1);
	if (tmp == -1) {
		printf("Invalid entry of [client:client_rulemapserver_port_start]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_client_worker_port_start() {
	int tmp = iniparser_getint(ini, "client:client_worker_port_start", -1);
	if (tmp == -1) {
		printf("Invalid entry of [client:client_worker_port_start]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

// each physical client

uint32_t IniparserWrapper::get_client_logical_num(uint32_t client_physical_idx) {
	char key[256];
	sprintf(key, "client%u:client_logical_num", client_physical_idx);
	int tmp = iniparser_getint(ini, key, -1);
	if (tmp == -1) {
		printf("Invalid entry of [%s]: %d\n", key, tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

const char *IniparserWrapper::get_client_ip(uint32_t client_physical_idx) {
	char key[256];
	sprintf(key, "client%u:client_ip", client_physical_idx);
	const char *client_ip = iniparser_getstring(ini, key, nullptr);
	if (client_ip == nullptr) {
		printf("Invalid entry of [%s]\n", key);
		exit(-1);
	}
	return client_ip;
}

void IniparserWrapper::get_client_mac(uint8_t *macaddr, uint32_t client_physical_idx) {
	char key[256];
	sprintf(key, "client%u:client_mac", client_physical_idx);
	const char *client_mac = iniparser_getstring(ini, key, nullptr);
	if (client_mac == nullptr) {
		printf("Invalid entry of [%s]\n", key);
		exit(-1);
	}
	parse_mac(macaddr, client_mac);
}

const char *IniparserWrapper::get_client_fpport(uint32_t client_physical_idx) {
	char key[256];
	sprintf(key, "client%u:client_fpport", client_physical_idx);
	const char *client_fpport = iniparser_getstring(ini, key, nullptr);
	if (client_fpport == nullptr) {
		printf("Invalid entry of [%s]\n", key);
		exit(-1);
	}
	return client_fpport;
}

uint32_t IniparserWrapper::get_client_pipeidx(uint32_t client_physical_idx) {
	char key[256];
	sprintf(key, "client%u:client_pipeidx", client_physical_idx);
	int tmp = iniparser_getint(ini, key, -1);
	if (tmp == -1) {
		printf("Invalid entry of [%s]: %d\n", key, tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

const char *IniparserWrapper::get_client_ip_for_client0(uint32_t client_physical_idx) {
	char key[256];
	sprintf(key, "client%u:client_ip_for_client0", client_physical_idx);
	const char *client_ip_for_client0 = iniparser_getstring(ini, key, nullptr);
	if (client_ip_for_client0 == nullptr) {
		printf("Invalid entry of [%s]\n", key);
		exit(-1);
	}
	return client_ip_for_client0;
}


// server common configuration

uint32_t IniparserWrapper::get_server_load_factor() {
	int tmp = iniparser_getint(ini, "server:server_load_factor", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_load_factor]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

short IniparserWrapper::get_server_worker_port_start() {
	int tmp = iniparser_getint(ini, "server:server_worker_port_start", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_worker_port_start]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_server_evictserver_port_start() {
	int tmp = iniparser_getint(ini, "server:server_evictserver_port_start", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_evictserver_port_start]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_server_snapshotserver_port_start() {
	int tmp = iniparser_getint(ini, "server:server_snapshotserver_port_start", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_snapshotserver_port_start]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_server_snapshotdataserver_port_start() {
	int tmp = iniparser_getint(ini, "server:server_snapshotdataserver_port_start", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_snapshotdataserver_port_start]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_transaction_loadfinishserver_port() {
	int tmp = iniparser_getint(ini, "server:transaction_loadfinishserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:transaction_loadfinishserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

// each physical server

uint32_t IniparserWrapper::get_server_worker_corenum(uint32_t server_physical_idx) {
	char key[256];
	sprintf(key, "server%u:server_worker_corenum", server_physical_idx);
	int tmp = iniparser_getint(ini, key, -1);
	if (tmp == -1) {
		printf("Invalid entry of [%s]: %d\n", key, tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

uint32_t IniparserWrapper::get_server_total_corenum(uint32_t server_physical_idx) {
	char key[256];
	sprintf(key, "server%u:server_total_corenum", server_physical_idx);
	int tmp = iniparser_getint(ini, key, -1);
	if (tmp == -1) {
		printf("Invalid entry of [%s]: %d\n", key, tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

std::vector<uint16_t> IniparserWrapper::get_server_logical_idxes(uint32_t server_physical_idx) {
	char key[256];
	sprintf(key, "server%u:server_logical_idxes", server_physical_idx);
	const char *tmpstr = iniparser_getstring(ini, key, nullptr);
	if (tmpstr == nullptr) {
		printf("Invalid entry of [%s]\n", key);
		exit(-1);
	}

	std::vector<uint16_t> result;
	const char *begin = tmpstr;
	while (true) {
		char *end = strchr((char *)begin, ':');
		if (end == NULL) {
			end = (char *)tmpstr + strlen(tmpstr);
		}

		INVARIANT(end - begin > 0);
		std::string idxstr(begin, end - begin);
		uint16_t tmpidx = uint16_t(std::stoul(idxstr));
		result.push_back(tmpidx);

		begin = end + 1;
		if (begin - tmpstr >= strlen(tmpstr)) {
			break;
		}
	}
	return result;
}

const char *IniparserWrapper::get_server_ip(uint32_t server_physical_idx) {
	char key[256];
	sprintf(key, "server%u:server_ip", server_physical_idx);
	const char *server_ip = iniparser_getstring(ini, key, nullptr);
	if (server_ip == nullptr) {
		printf("Invalid entry of [%s]\n", key);
		exit(-1);
	}
	return server_ip;
}

void IniparserWrapper::get_server_mac(uint8_t *macaddr, uint32_t server_physical_idx) {
	char key[256];
	sprintf(key, "server%u:server_mac", server_physical_idx);
	const char *server_mac = iniparser_getstring(ini, key, nullptr);
	if (server_mac == nullptr) {
		printf("Invalid entry of [%s]\n", key);
		exit(-1);
	}
	parse_mac(macaddr, server_mac);
}

const char *IniparserWrapper::get_server_fpport(uint32_t server_physical_idx) {
	char key[256];
	sprintf(key, "server%u:server_fpport", server_physical_idx);
	const char *server_fpport = iniparser_getstring(ini, key, nullptr);
	if (server_fpport == nullptr) {
		printf("Invalid entry of [%s]\n", key);
		exit(-1);
	}
	return server_fpport;
}

uint32_t IniparserWrapper::get_server_pipeidx(uint32_t server_physical_idx) {
	char key[256];
	sprintf(key, "server%u:server_pipeidx", server_physical_idx);
	int tmp = iniparser_getint(ini, key, -1);
	if (tmp == -1) {
		printf("Invalid entry of [%s]: %d\n", key, tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

const char *IniparserWrapper::get_server_ip_for_controller(uint32_t server_physical_idx) {
	char key[256];
	sprintf(key, "server%u:server_ip_for_controller", server_physical_idx);
	const char *server_ip_for_controller = iniparser_getstring(ini, key, nullptr);
	if (server_ip_for_controller == nullptr) {
		printf("Invalid entry of [%s]\n", key);
		exit(-1);
	}
	return server_ip_for_controller;
}

// Controller

const char *IniparserWrapper::get_controller_ip_for_server() {
	const char *controller_ip_for_server = iniparser_getstring(ini, "controller:controller_ip_for_server", nullptr);
	if (controller_ip_for_server == nullptr) {
		printf("Invalid entry of [controller:controller_ip_for_server]\n");
		exit(-1);
	}
	return controller_ip_for_server;
}

const char* IniparserWrapper::get_controller_ip_for_switchos() {
	const char *controller_ip_for_switchos = iniparser_getstring(ini, "controller:controller_ip_for_switchos", nullptr);
	if (controller_ip_for_switchos == nullptr) {
		printf("Invalid entry of [controller:controller_ip_for_switchos]\n");
		exit(-1);
	}
	return controller_ip_for_switchos;
}

short IniparserWrapper::get_controller_popserver_port_start() {
	int tmp = iniparser_getint(ini, "controller:controller_popserver_port_start", -1);
	if (tmp == -1) {
		printf("Invalid entry of [controller:controller_popserver_port_start]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_controller_evictserver_port() {
	int tmp = iniparser_getint(ini, "controller:controller_evictserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [controller:controller_evictserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

uint32_t IniparserWrapper::get_controller_snapshot_period() {
	int tmp = iniparser_getint(ini, "controller:controller_snapshot_period", -1);
	if (tmp == -1) {
		printf("Invalid entry of [controller:controller_snapshot_period]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

short IniparserWrapper::get_controller_warmupfinishserver_port() {
	int tmp = iniparser_getint(ini, "controller:controller_warmupfinishserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [controller:controller_warmupfinishserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

// Switch

uint32_t IniparserWrapper::get_switch_partition_count() {
	int tmp = iniparser_getint(ini, "switch:switch_partition_count", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switch_partition_count]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

uint32_t IniparserWrapper::get_switch_kv_bucket_num() {
	int tmp = iniparser_getint(ini, "switch:switch_kv_bucket_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switch_kv_bucket_num]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

uint32_t IniparserWrapper::get_switch_pipeline_num() {
	int tmp = iniparser_getint(ini, "switch:switch_pipeline_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switch_pipeline_num]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

uint16_t IniparserWrapper::get_switch_max_vallen() {
	int tmp = iniparser_getint(ini, "switch:switch_max_vallen", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switch_max_vallen]: %d\n", tmp);
		exit(-1);
	}
	return uint16_t(tmp);
}

uint32_t IniparserWrapper::get_switchos_sample_cnt() {
	int tmp = iniparser_getint(ini, "switch:switchos_sample_cnt", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switchos_sample_cnt]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

short IniparserWrapper::get_switchos_popserver_port() {
	int tmp = iniparser_getint(ini, "switch:switchos_popserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switchos_popserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_switchos_snapshotserver_port() {
	int tmp = iniparser_getint(ini, "switch:switchos_snapshotserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switchos_snapshotserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_switchos_specialcaseserver_port() {
	int tmp = iniparser_getint(ini, "switch:switchos_specialcaseserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switchos_specialcaseserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_switchos_ptf_popserver_port() {
	int tmp = iniparser_getint(ini, "switch:switchos_ptf_popserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switchos_ptf_popserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_switchos_ptf_snapshotserver_port() {
	int tmp = iniparser_getint(ini, "switch:switchos_ptf_snapshotserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switchos_ptf_snapshotserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

uint32_t IniparserWrapper::get_spineswitch_total_logical_num() {
	int tmp = iniparser_getint(ini, "switch:spineswitch_total_logical_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:spineswitch_total_logical_num]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

uint32_t IniparserWrapper::get_leafswitch_total_logical_num() {
	int tmp = iniparser_getint(ini, "switch:leafswitch_total_logical_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:leafswitch_total_logical_num]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

// spineswitch

std::vector<uint16_t> IniparserWrapper::get_spineswitch_logical_idxes() {
	char key[256] = "spineswitch:switch_logical_idxes";
	const char *tmpstr = iniparser_getstring(ini, key, nullptr);
	if (tmpstr == nullptr) {
		printf("Invalid entry of [%s]\n", key);
		exit(-1);
	}

	std::vector<uint16_t> result;
	const char *begin = tmpstr;
	while (true) {
		char *end = strchr((char *)begin, ':');
		if (end == NULL) {
			end = (char *)tmpstr + strlen(tmpstr);
		}

		INVARIANT(end - begin > 0);
		std::string idxstr(begin, end - begin);
		uint16_t tmpidx = uint16_t(std::stoul(idxstr));
		result.push_back(tmpidx);

		begin = end + 1;
		if (begin - tmpstr >= strlen(tmpstr)) {
			break;
		}
	}
	return result;
}

const char *IniparserWrapper::get_spineswitchos_ip() {
	const char *spineswitchos_ip = iniparser_getstring(ini, "spineswitch:switchos_ip", nullptr);
	if (spineswitchos_ip == nullptr) {
		printf("Invalid entry of spineswitch:switchos_ip\n");
		exit(-1);
	}
	return spineswitchos_ip;
}

const char *IniparserWrapper::get_spineswitch_fpport_to_leaf() {
	const char *spineswitch_fpport_to_leaf = iniparser_getstring(ini, "spineswitch:spineswitch_fpport_to_leaf", nullptr);
	if (spineswitch_fpport_to_leaf == nullptr) {
		printf("Invalid entry of spineswitch:spineswitch_fpport_to_leaf\n");
		exit(-1);
	}
	return spineswitch_fpport_to_leaf;
}

// leafswitch

std::vector<uint16_t> IniparserWrapper::get_leafswitch_logical_idxes() {
	char key[256] = "leafswitch:switch_logical_idxes";
	const char *tmpstr = iniparser_getstring(ini, key, nullptr);
	if (tmpstr == nullptr) {
		printf("Invalid entry of [%s]\n", key);
		exit(-1);
	}

	std::vector<uint16_t> result;
	const char *begin = tmpstr;
	while (true) {
		char *end = strchr((char *)begin, ':');
		if (end == NULL) {
			end = (char *)tmpstr + strlen(tmpstr);
		}

		INVARIANT(end - begin > 0);
		std::string idxstr(begin, end - begin);
		uint16_t tmpidx = uint16_t(std::stoul(idxstr));
		result.push_back(tmpidx);

		begin = end + 1;
		if (begin - tmpstr >= strlen(tmpstr)) {
			break;
		}
	}
	return result;
}

const char *IniparserWrapper::get_leafswitchos_ip() {
	const char *leafswitchos_ip = iniparser_getstring(ini, "leafswitch:switchos_ip", nullptr);
	if (leafswitchos_ip == nullptr) {
		printf("Invalid entry of leafswitch:switchos_ip\n");
		exit(-1);
	}
	return leafswitchos_ip;
}

const char *IniparserWrapper::get_leafswitch_fpport_to_spine() {
	const char *leafswitch_fpport_to_spine = iniparser_getstring(ini, "leafswitch:leafswitch_fpport_to_spine", nullptr);
	if (leafswitch_fpport_to_spine == nullptr) {
		printf("Invalid entry of leafswitch:leafswitch_fpport_to_spine\n");
		exit(-1);
	}
	return leafswitch_fpport_to_spine;
}

uint32_t IniparserWrapper::get_leafswitch_pipeidx() {
	int tmp = iniparser_getint(ini, "leafswitch:leafswitch_pipeidx", -1);
	if (tmp == -1) {
		printf("Invalid entry of [leafswitch:leafswitch_pipeidx]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

// Reflector

const char* IniparserWrapper::get_reflector_ip_for_switchos() {
	const char *reflector_ip_for_switchos = iniparser_getstring(ini, "reflector:reflector_ip_for_switchos", nullptr);
	if (reflector_ip_for_switchos == nullptr) {
		printf("Invalid entry of [reflector:reflector_ip_for_switchos]\n");
		exit(-1);
	}
	return reflector_ip_for_switchos;
}

short IniparserWrapper::get_reflector_dp2cpserver_port() {
	int tmp = iniparser_getint(ini, "reflector:reflector_dp2cpserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [reflector:reflector_dp2cpserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_reflector_cp2dpserver_port() {
	int tmp = iniparser_getint(ini, "reflector:reflector_cp2dpserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [reflector:reflector_cp2dpserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

/* control_type.ini */

// switchos

/*int IniparserWrapper::get_switchos_get_freeidx() {
	int tmp = iniparser_getint(ini, "switchos:switchos_get_freeidx", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_get_freeidx]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_get_key_freeidx() {
	int tmp = iniparser_getint(ini, "switchos:switchos_get_key_freeidx", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_get_key_freeidx]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_set_evictdata() {
	int tmp = iniparser_getint(ini, "switchos:switchos_set_evictdata", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_set_evictdata]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_get_evictkey() {
	int tmp = iniparser_getint(ini, "switchos:switchos_get_evictkey", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_get_evictkey]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_get_cachedemptyindex() {
	int tmp = iniparser_getint(ini, "switchos:switchos_get_cachedemptyindex", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_get_cachedemptyindex]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}*/

int IniparserWrapper::get_switchos_setvalid0() {
	int tmp = iniparser_getint(ini, "switchos:switchos_setvalid0", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_setvalid0]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_setvalid0_ack() {
	int tmp = iniparser_getint(ini, "switchos:switchos_setvalid0_ack", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_setvalid0_ack]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

/*int IniparserWrapper::get_switchos_add_cache_lookup_setvalid1() {
	int tmp = iniparser_getint(ini, "switchos:switchos_add_cache_lookup_setvalid1", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_add_cache_lookup_setvalid1]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_add_cache_lookup_setvalid1_ack() {
	int tmp = iniparser_getint(ini, "switchos:switchos_add_cache_lookup_setvalid1_ack", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_add_cache_lookup_setvalid1_ack]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}*/

int IniparserWrapper::get_switchos_add_cache_lookup() {
	int tmp = iniparser_getint(ini, "switchos:switchos_add_cache_lookup", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_add_cache_lookup]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_add_cache_lookup_ack() {
	int tmp = iniparser_getint(ini, "switchos:switchos_add_cache_lookup_ack", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_add_cache_lookup_ack]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

/*int IniparserWrapper::get_switchos_get_evictdata_setvalid3() {
	int tmp = iniparser_getint(ini, "switchos:switchos_get_evictdata_setvalid3", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_get_evictdata_setvalid3]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_get_evictdata_setvalid3_ack() {
	int tmp = iniparser_getint(ini, "switchos:switchos_get_evictdata_setvalid3_ack", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_get_evictdata_setvalid3_ack]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}*/

int IniparserWrapper::get_switchos_setvalid3() {
	int tmp = iniparser_getint(ini, "switchos:switchos_setvalid3", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_setvalid3]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_setvalid3_ack() {
	int tmp = iniparser_getint(ini, "switchos:switchos_setvalid3_ack", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_setvalid3_ack]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_remove_cache_lookup() {
	int tmp = iniparser_getint(ini, "switchos:switchos_remove_cache_lookup", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_remove_cache_lookup]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_remove_cache_lookup_ack() {
	int tmp = iniparser_getint(ini, "switchos:switchos_remove_cache_lookup_ack", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_remove_cache_lookup_ack]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_cleanup() {
	int tmp = iniparser_getint(ini, "switchos:switchos_cleanup", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_cleanup]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_cleanup_ack() {
	int tmp = iniparser_getint(ini, "switchos:switchos_cleanup_ack", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_cleanup_ack]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_enable_singlepath() {
	int tmp = iniparser_getint(ini, "switchos:switchos_enable_singlepath", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_enable_singlepath]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_enable_singlepath_ack() {
	int tmp = iniparser_getint(ini, "switchos:switchos_enable_singlepath_ack", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_enable_singlepath_ack]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_set_snapshot_flag() {
	int tmp = iniparser_getint(ini, "switchos:switchos_set_snapshot_flag", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_set_snapshot_flag]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_set_snapshot_flag_ack() {
	int tmp = iniparser_getint(ini, "switchos:switchos_set_snapshot_flag_ack", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_set_snapshot_flag_ack]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_disable_singlepath() {
	int tmp = iniparser_getint(ini, "switchos:switchos_disable_singlepath", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_disable_singlepath]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_disable_singlepath_ack() {
	int tmp = iniparser_getint(ini, "switchos:switchos_disable_singlepath_ack", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_disable_singlepath_ack]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_load_snapshot_data() {
	int tmp = iniparser_getint(ini, "switchos:switchos_load_snapshot_data", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_load_snapshot_data]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_load_snapshot_data_ack() {
	int tmp = iniparser_getint(ini, "switchos:switchos_load_snapshot_data_ack", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_load_snapshot_data_ack]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_reset_snapshot_flag_and_reg() {
	int tmp = iniparser_getint(ini, "switchos:switchos_reset_snapshot_flag_and_reg", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_reset_snapshot_flag_and_reg]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_reset_snapshot_flag_and_reg_ack() {
	int tmp = iniparser_getint(ini, "switchos:switchos_reset_snapshot_flag_and_reg_ack", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_reset_snapshot_flag_and_reg_ack]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_ptf_popserver_end() {
	int tmp = iniparser_getint(ini, "switchos:switchos_ptf_popserver_end", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_ptf_popserver_end]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_switchos_ptf_snapshotserver_end() {
	int tmp = iniparser_getint(ini, "switchos:switchos_ptf_snapshotserver_end", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switchos:switchos_ptf_snapshotserver_end]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

// snapshot

int IniparserWrapper::get_snapshot_cleanup() {
	int tmp = iniparser_getint(ini, "snapshot:SNAPSHOT_CLEANUP", -1);
	if (tmp == -1) {
		printf("Invalid entry of [snapshot:SNAPSHOT_CLEANUP]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_snapshot_cleanup_ack() {
	int tmp = iniparser_getint(ini, "snapshot:SNAPSHOT_CLEANUP_ACK", -1);
	if (tmp == -1) {
		printf("Invalid entry of [snapshot:SNAPSHOT_CLEANUP_ACK]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_snapshot_prepare() {
	int tmp = iniparser_getint(ini, "snapshot:SNAPSHOT_PREPARE", -1);
	if (tmp == -1) {
		printf("Invalid entry of [snapshot:SNAPSHOT_PREPARE]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_snapshot_prepare_ack() {
	int tmp = iniparser_getint(ini, "snapshot:SNAPSHOT_PREPARE_ACK", -1);
	if (tmp == -1) {
		printf("Invalid entry of [snapshot:SNAPSHOT_PREPARE_ACK]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_snapshot_setflag() {
	int tmp = iniparser_getint(ini, "snapshot:SNAPSHOT_SETFLAG", -1);
	if (tmp == -1) {
		printf("Invalid entry of [snapshot:SNAPSHOT_SETFLAG]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_snapshot_setflag_ack() {
	int tmp = iniparser_getint(ini, "snapshot:SNAPSHOT_SETFLAG_ACK", -1);
	if (tmp == -1) {
		printf("Invalid entry of [snapshot:SNAPSHOT_SETFLAG_ACK]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_snapshot_start() {
	int tmp = iniparser_getint(ini, "snapshot:SNAPSHOT_START", -1);
	if (tmp == -1) {
		printf("Invalid entry of [snapshot:SNAPSHOT_START]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_snapshot_start_ack() {
	int tmp = iniparser_getint(ini, "snapshot:SNAPSHOT_START_ACK", -1);
	if (tmp == -1) {
		printf("Invalid entry of [snapshot:SNAPSHOT_START_ACK]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_snapshot_getdata() {
	int tmp = iniparser_getint(ini, "snapshot:SNAPSHOT_GETDATA", -1);
	if (tmp == -1) {
		printf("Invalid entry of [snapshot:SNAPSHOT_GETDATA]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_snapshot_getdata_ack() {
	int tmp = iniparser_getint(ini, "snapshot:SNAPSHOT_GETDATA_ACK", -1);
	if (tmp == -1) {
		printf("Invalid entry of [snapshot:SNAPSHOT_GETDATA_ACK]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_snapshot_senddata() {
	int tmp = iniparser_getint(ini, "snapshot:SNAPSHOT_SENDDATA", -1);
	if (tmp == -1) {
		printf("Invalid entry of [snapshot:SNAPSHOT_SENDDATA]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

int IniparserWrapper::get_snapshot_senddata_ack() {
	int tmp = iniparser_getint(ini, "snapshot:SNAPSHOT_SENDDATA_ACK", -1);
	if (tmp == -1) {
		printf("Invalid entry of [snapshot:SNAPSHOT_SENDDATA_ACK]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
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
