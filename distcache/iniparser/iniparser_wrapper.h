#ifndef INIPARSER_WRAPPER_H
#define INIPARSER_WRAPPER_H

#include <stdint.h>
#include <vector>
#include <string>

#include "iniparser.h"
#include "../helper.h"

class IniparserWrapper {
	public:
		IniparserWrapper();
		
		void load(const char* filename);

		/* config.ini */

		// global
		const char *get_workload_name();
		int get_workload_mode();
		int get_dynamic_periodnum();
		int get_dynamic_periodinterval();
		const char *get_dynamic_ruleprefix();
		uint16_t get_max_vallen();
		uint32_t get_load_batch_size();
		uint32_t get_client_physical_num();
		uint32_t get_server_physical_num();
		uint32_t get_client_total_logical_num();
		uint32_t get_server_total_logical_num();
		uint32_t get_server_total_logical_num_for_rotation();

		// client common configuration
		short get_client_rotationdataserver_port();
		short get_client_sendpktserver_port_start();
		short get_client_rulemapserver_port_start();
		short get_client_worker_port_start();

		// each physical client
		uint32_t get_client_logical_num(uint32_t client_physical_idx);
		const char *get_client_ip(uint32_t client_physical_idx);
		void get_client_mac(uint8_t* macaddr, uint32_t client_physical_idx);
		const char *get_client_fpport(uint32_t client_physical_idx);
		uint32_t get_client_pipeidx(uint32_t client_physical_idx);
		const char *get_client_ip_for_client0(uint32_t client_physical_idx);
		
		// server common configuration
		uint32_t get_server_load_factor();
		short get_server_worker_port_start();
		short get_server_evictserver_port_start();
		short get_server_popserver_port_start();
		short get_server_valueupdateserver_port_start();
		short get_transaction_loadfinishserver_port();
		
		// each physical server
		uint32_t get_server_worker_corenum(uint32_t server_physical_idx);
		uint32_t get_server_total_corenum(uint32_t server_physical_idx);
		std::vector<uint16_t> get_server_logical_idxes(uint32_t server_physical_idx);
		const char *get_server_ip(uint32_t server_physical_idx);
		void get_server_mac(uint8_t* macaddr, uint32_t server_physical_idx);
		const char *get_server_fpport(uint32_t server_physical_idx);
		uint32_t get_server_pipeidx(uint32_t server_physical_idx);
		const char *get_server_ip_for_controller(uint32_t server_physical_idx);

		// controller
		const char *get_controller_ip_for_server();
		const char *get_controller_ip_for_switchos();
		short get_controller_popserver_port_start();
		short get_controller_evictserver_port();
		uint32_t get_controller_snapshot_period();
		short get_controller_warmupfinishserver_port();

		// switch
		uint32_t get_switch_partition_count();
		uint32_t get_switch_kv_bucket_num();
		uint32_t get_switch_pipeline_num();
		uint16_t get_switch_max_vallen();
		uint32_t get_switchos_sample_cnt();
		short get_switchos_dppopserver_port();
		short get_switchos_cppopserver_port();
		short get_switchos_snapshotserver_port();
		short get_switchos_specialcaseserver_port();
		short get_switchos_ptf_popserver_port();
		short get_switchos_ptf_snapshotserver_port();
		uint32_t get_spineswitch_total_logical_num();
		uint32_t get_leafswitch_total_logical_num();

		// spineswitch
		std::vector<uint16_t> get_spineswitch_logical_idxes();
		const char *get_spineswitchos_ip();
		const char *get_spineswitch_fpport_to_leaf();

		// leafswitch
		std::vector<uint16_t> get_leafswitch_logical_idxes();
		const char *get_leafswitchos_ip();
		const char *get_leafswitch_fpport_to_spine();
		uint32_t get_leafswitch_pipeidx();

		// reflector_for_leaf
		const char *get_leaf_reflector_ip_for_switchos();
		short get_leaf_reflector_dp2cpserver_port();
		short get_leaf_reflector_cp2dpserver_port();
		const char *get_leaf_reflector_cp2dp_dstip();

		// reflector_for_spine
		const char *get_spine_reflector_ip_for_switchos();
		short get_spine_reflector_dp2cpserver_port();
		short get_spine_reflector_cp2dpserver_port();
		const char *get_spine_reflector_ip_for_switch();
		void get_spine_reflector_mac_for_switch(uint8_t* macaddr);
		const char *get_spine_reflector_fpport_for_switch();
		const char *get_spine_reflector_cp2dp_dstip();

		/* control_type.ini */

		// switchos
		/*int get_switchos_get_freeidx();
		int get_switchos_get_key_freeidx();
		int get_switchos_set_evictdata();
		int get_switchos_get_evictkey();
		int get_switchos_get_cachedemptyindex();*/
		int get_switchos_setvalid0();
		int get_switchos_setvalid0_ack();
		//int get_switchos_add_cache_lookup_setvalid1();
		//int get_switchos_add_cache_lookup_setvalid1_ack();
		int get_switchos_add_cache_lookup();
		int get_switchos_add_cache_lookup_ack();
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
