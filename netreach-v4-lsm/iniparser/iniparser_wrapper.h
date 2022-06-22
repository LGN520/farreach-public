#ifndef INIPARSER_WRAPPER_H
#define INIPARSER_WRAPPER_H

#include <stdint.h>
#include "iniparser.h"

class IniparserWrapper {
	public:
		IniparserWrapper();
		
		void load(const char* filename);

		/* config.ini */

		// global
		const char *get_workload_name();
		int get_workload_mode();
		//uint32_t get_max_vallen();
		uint16_t get_max_vallen();
		uint32_t get_load_batch_size();
		int get_dynamic_periodnum();
		int get_dynamic_periodinterval();
		const char *get_dynamic_ruleprefix();

		// client
		size_t get_client_num();
		short get_client_port();
		const char *get_client_ip();
		void get_client_mac(uint8_t* macaddr);
		//const char *get_client_ip_for_server();
		
		// server
		int get_server_cores();
		uint32_t get_load_factor();
		//uint32_t get_split_num();
		uint32_t get_server_num();
		short get_server_port();
		const char *get_server_ip();
		void get_server_mac(uint8_t* macaddr);
		const char *get_server_ip_for_controller();
		short get_server_evictserver_port();
		short get_server_snapshotserver_port();
		short get_server_snapshotdataserver_port();
		short get_transaction_loadfinishserver_port();
		//short get_server_dynamicserver_port();
		//const char *get_server_ip_for_client();

		// controller
		const char *get_controller_ip_for_server();
		const char *get_controller_ip_for_switchos();
		short get_controller_popserver_port();
		short get_controller_evictserver_port();
		uint32_t get_controller_snapshot_period();

		// reflector
		const char *get_reflector_ip_for_switchos();
		short get_reflector_port();
		short get_reflector_popserver_port();

		// switch
		uint32_t get_partition_count();
		uint32_t get_switch_kv_bucket_num();
		//uint32_t get_switch_max_vallen();
		uint16_t get_switch_max_vallen();
		short get_switchos_popserver_port();
		//short get_switchos_paramserver_port();
		const char *get_switchos_ip();
		uint32_t get_switchos_sample_cnt();
		short get_switchos_snapshotserver_port();
		short get_switchos_specialcaseserver_port();
		//short get_switchos_snapshotdataserver_port();
		short get_switchos_ptf_popserver_port();
		short get_switchos_ptf_snapshotserver_port();

		/* control_type.ini */

		// switchos
		/*int get_switchos_get_freeidx();
		int get_switchos_get_key_freeidx();
		int get_switchos_set_evictdata();
		int get_switchos_get_evictkey();
		int get_switchos_get_cachedemptyindex();*/
		int get_switchos_setvalid0();
		int get_switchos_setvalid0_ack();
		int get_switchos_add_cache_lookup_setvalid1();
		int get_switchos_add_cache_lookup_setvalid1_ack();
		//int get_switchos_get_evictdata_setvalid3();
		//int get_switchos_get_evictdata_setvalid3_ack();
		int get_switchos_setvalid3();
		int get_switchos_setvalid3_ack();
		int get_switchos_remove_cache_lookup();
		int get_switchos_remove_cache_lookup_ack();
		int get_switchos_cleanup();
		int get_switchos_cleanup_ack();
		int get_switchos_enable_singlepath();
		int get_switchos_enable_singlepath_ack();
		int get_switchos_set_snapshot_flag();
		int get_switchos_set_snapshot_flag_ack();
		int get_switchos_disable_singlepath();
		int get_switchos_disable_singlepath_ack();
		int get_switchos_load_snapshot_data();
		int get_switchos_load_snapshot_data_ack();
		int get_switchos_reset_snapshot_flag_and_reg();
		int get_switchos_reset_snapshot_flag_and_reg_ack();
		int get_switchos_ptf_popserver_end();
		int get_switchos_ptf_snapshotserver_end();

		// snapshot
		int get_snapshot_cleanup();
		int get_snapshot_cleanup_ack();
		int get_snapshot_prepare();
		int get_snapshot_prepare_ack();
		int get_snapshot_setflag();
		int get_snapshot_setflag_ack();
		int get_snapshot_start();
		int get_snapshot_start_ack();
		int get_snapshot_getdata();
		int get_snapshot_getdata_ack();
		int get_snapshot_senddata();
		int get_snapshot_senddata_ack();

	private:
		dictionary *ini = nullptr;

		void parse_mac(uint8_t* macaddr, const char* macstr);
};

#endif
