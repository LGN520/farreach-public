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

//uint32_t IniparserWrapper::get_max_vallen() {
uint16_t IniparserWrapper::get_max_vallen() {
	int tmp = iniparser_getint(ini, "global:max_vallen", -1);
	if (tmp == -1) {
		printf("Invalid entry of [global:max_vallen]: %d\n", tmp);
		exit(-1);
	}
	//return uint32_t(tmp);
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

/*const char *IniparserWrapper::get_client_ip_for_server() {
	const char *client_ip_for_server = iniparser_getstring(ini, "client:client_ip_for_server", nullptr);
	if (client_ip_for_server == nullptr) {
		printf("Invalid entry of [client:client_ip_for_server]\n");
		exit(-1);
	}
	return client_ip_for_server;
}*/

// Server

int IniparserWrapper::get_server_cores() {
	int tmp = iniparser_getint(ini, "server:server_cores", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_cores]: %d\n", tmp);
		exit(-1);
	}
	return tmp;
}

uint32_t IniparserWrapper::get_load_factor() {
	int tmp = iniparser_getint(ini, "server:load_factor", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:load_factor]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}

/*uint32_t IniparserWrapper::get_split_num() {
	int tmp = iniparser_getint(ini, "server:split_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:split_num]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
}*/

uint32_t IniparserWrapper::get_server_num() {
	int tmp = iniparser_getint(ini, "server:server_num", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_num]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
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

const char *IniparserWrapper::get_server_ip_for_controller() {
	const char *server_ip = iniparser_getstring(ini, "server:server_ip_for_controller", nullptr);
	if (server_ip == nullptr) {
		printf("Invalid entry of [server:server_ip_for_controller]\n");
		exit(-1);
	}
	return server_ip;
}

short IniparserWrapper::get_server_evictserver_port() {
	int tmp = iniparser_getint(ini, "server:server_evictserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_evictserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_server_snapshotserver_port() {
	int tmp = iniparser_getint(ini, "server:server_snapshotserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_snapshotserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_server_snapshotdataserver_port() {
	int tmp = iniparser_getint(ini, "server:server_snapshotdataserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_snapshotdataserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

/*short IniparserWrapper::get_server_dynamicserver_port() {
	int tmp = iniparser_getint(ini, "server:server_dynamicserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [server:server_dynamicserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

const char *IniparserWrapper::get_server_ip_for_client() {
	const char *server_ip_for_client = iniparser_getstring(ini, "server:server_ip_for_client", nullptr);
	if (server_ip_for_client == nullptr) {
		printf("Invalid entry of [server:server_ip_for_client]\n");
		exit(-1);
	}
	return server_ip_for_client;
}*/

// Switch

uint32_t IniparserWrapper::get_partition_count() {
	int tmp = iniparser_getint(ini, "switch:partition_count", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:partition_count]: %d\n", tmp);
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

//uint32_t IniparserWrapper::get_switch_max_vallen() {
uint16_t IniparserWrapper::get_switch_max_vallen() {
	int tmp = iniparser_getint(ini, "switch:switch_max_vallen", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switch_max_vallen]: %d\n", tmp);
		exit(-1);
	}
	//return uint32_t(tmp);
	return uint16_t(tmp);
}

short IniparserWrapper::get_switchos_popserver_port() {
	int tmp = iniparser_getint(ini, "switch:switchos_popserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switchos_popserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

/*short IniparserWrapper::get_switchos_paramserver_port() {
	int tmp = iniparser_getint(ini, "switch:switchos_paramserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switchos_paramserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}*/

const char* IniparserWrapper::get_switchos_ip() {
	const char *switchos_ip = iniparser_getstring(ini, "switch:switchos_ip", nullptr);
	if (switchos_ip == nullptr) {
		printf("Invalid entry of [switch:switchos_ip]\n");
		exit(-1);
	}
	return switchos_ip;
}

uint32_t IniparserWrapper::get_switchos_sample_cnt() {
	int tmp = iniparser_getint(ini, "switch:switchos_sample_cnt", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switchos_sample_cnt]: %d\n", tmp);
		exit(-1);
	}
	return uint32_t(tmp);
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

/*short IniparserWrapper::get_switchos_snapshotdataserver_port() {
	int tmp = iniparser_getint(ini, "switch:switchos_snapshotdataserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [switch:switchos_snapshotdataserver_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}*/

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

// Controller

const char* IniparserWrapper::get_controller_ip_for_server() {
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

short IniparserWrapper::get_controller_popserver_port() {
	int tmp = iniparser_getint(ini, "controller:controller_popserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [controller:controller_popserver_port]: %d\n", tmp);
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

// Reflector

const char* IniparserWrapper::get_reflector_ip_for_switchos() {
	const char *reflector_ip_for_switchos = iniparser_getstring(ini, "reflector:reflector_ip_for_switchos", nullptr);
	if (reflector_ip_for_switchos == nullptr) {
		printf("Invalid entry of [reflector:reflector_ip_for_switchos]\n");
		exit(-1);
	}
	return reflector_ip_for_switchos;
}

short IniparserWrapper::get_reflector_port() {
	int tmp = iniparser_getint(ini, "reflector:reflector_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [reflector:reflector_port]: %d\n", tmp);
		exit(-1);
	}
	return short(tmp);
}

short IniparserWrapper::get_reflector_popserver_port() {
	int tmp = iniparser_getint(ini, "reflector:reflector_popserver_port", -1);
	if (tmp == -1) {
		printf("Invalid entry of [reflector:reflector_popserver_port]: %d\n", tmp);
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

int IniparserWrapper::get_switchos_add_cache_lookup_setvalid1() {
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
}

int IniparserWrapper::get_switchos_get_evictdata_setvalid3() {
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
