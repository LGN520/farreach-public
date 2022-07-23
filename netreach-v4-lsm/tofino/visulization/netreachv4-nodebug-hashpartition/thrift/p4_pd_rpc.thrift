# BFN Thrift RPC Input

include "res.thrift"


namespace py p4_pd_rpc
namespace cpp p4_pd_rpc
namespace c_glib p4_pd_rpc

typedef i32 EntryHandle_t
typedef i32 MemberHandle_t
typedef i32 GroupHandle_t
typedef binary MacAddr_t
typedef binary IPv6_t
typedef i32 SnapshotHandle_t
typedef i32 PvsHandle_t

struct netbufferv4_counter_value_t {
  1: required i64 packets;
  2: required i64 bytes;
}

struct netbufferv4_packets_meter_spec_t {
  1: required i64 cir_pps;
  2: required i64 cburst_pkts;
  3: required i64 pir_pps;
  4: required i64 pburst_pkts;
  5: required bool color_aware;
  6: optional bool is_set = 1;
}

struct netbufferv4_bytes_meter_spec_t {
  1: required i64 cir_kbps;
  2: required i64 cburst_kbits;
  3: required i64 pir_kbps;
  4: required i64 pburst_kbits;
  5: required bool color_aware;
  6: optional bool is_set = 1;
}

enum netbufferv4_lpf_type {
  TYPE_RATE = 0,
  TYPE_SAMPLE = 1
}

struct netbufferv4_lpf_spec_t {
  1: required bool gain_decay_separate_time_constant;
  2: required double gain_time_constant;
  3: required double decay_time_constant;
  4: required double time_constant;
  5: required i32 output_scale_down_factor;
  6: required netbufferv4_lpf_type lpf_type;
  7: optional bool is_set = 1;
}

struct netbufferv4_wred_spec_t {
  1: required double time_constant;
  2: required i32 red_min_threshold;
  3: required i32 red_max_threshold;
  4: required double max_probability;
  5: optional bool is_set = 1;
}


enum netbufferv4_idle_time_mode {
  POLL_MODE = 0,
  NOTIFY_MODE = 1,
  INVALID_MODE = 2
}

enum netbufferv4_idle_time_hit_state {
  ENTRY_IDLE = 0,
  ENTRY_ACTIVE = 1
}

struct netbufferv4_idle_time_params_t {
  1: required netbufferv4_idle_time_mode mode;
  2: optional i32 ttl_query_interval;
  3: optional i32 max_ttl;
  4: optional i32 min_ttl;
  5: optional i32 cookie;
}

struct netbufferv4_idle_tmo_expired_t {
  1: required i32 dev_id;
  2: required EntryHandle_t entry;
  3: required i32 cookie;
}

struct netbufferv4_sel_update_t {
  1: required res.SessionHandle_t  sess_hdl;
  2: required res.DevTarget_t      dev_tgt;
  3: required i32                  cookie;
  4: required i32                  grp_hdl;
  5: required i32                  mbr_hdl;
  6: required i32                  index;
  7: required bool                 is_add;
}

enum netbufferv4_grp_mbr_state {
  MBR_ACTIVE = 0,
  MBR_INACTIVE = 1
}


enum tbl_property_t
{
   TBL_PROP_TBL_ENTRY_SCOPE = 1,
   TBL_PROP_TERN_TABLE_ENTRY_PLACEMENT = 2,
   TBL_PROP_DUPLICATE_ENTRY_CHECK = 3,
   TBL_PROP_IDLETIME_REPEATED_NOTIFICATION = 4
}

enum tbl_property_value_t
{
   ENTRY_SCOPE_ALL_PIPELINES=0,
   ENTRY_SCOPE_SINGLE_PIPELINE=1,
   ENTRY_SCOPE_USER_DEFINED=2,
   TERN_ENTRY_PLACEMENT_DRV_MANAGED=0,
   TERN_ENTRY_PLACEMENT_APP_MANAGED=1,
   DUPLICATE_ENTRY_CHECK_DISABLE=0,
   DUPLICATE_ENTRY_CHECK_ENABLE=1,
   IDLETIME_REPEATED_NOTIFICATION_DISABLE = 0,
   IDLETIME_REPEATED_NOTIFICATION_ENABLE = 1
}

struct tbl_property_value_args_t
{
  1: required tbl_property_value_t value;
  2: required i32                  scope_args;
}

enum pvs_gress_t
{
   PVS_GRESS_INGRESS = 0,
   PVS_GRESS_EGRESS = 1,
   PVS_GRESS_ALL = 0xff
}

enum pvs_property_t {
  PVS_PROP_NONE = 0,
  PVS_GRESS_SCOPE,
  PVS_PIPE_SCOPE,
  PVS_PARSER_SCOPE
}

enum pvs_property_value_t {
  PVS_SCOPE_ALL_GRESS = 0,
  PVS_SCOPE_SINGLE_GRESS = 1,
  PVS_SCOPE_ALL_PIPELINES = 0,
  PVS_SCOPE_SINGLE_PIPELINE = 1,
  PVS_SCOPE_ALL_PARSERS = 0,
  PVS_SCOPE_SINGLE_PARSER = 1
}  

enum tbl_dbg_counter_type_t {
  TBL_DBG_CNTR_DISABLED = 0,
  TBL_DBG_CNTR_LOG_TBL_MISS,
  TBL_DBG_CNTR_LOG_TBL_HIT,
  TBL_DBG_CNTR_GW_TBL_MISS,
  TBL_DBG_CNTR_GW_TBL_HIT,
  TBL_DBG_CNTR_GW_TBL_INHIBIT,
  TBL_DBG_CNTR_MAX
}

struct PVSSpec_t {
  1: required i32 parser_value;
  2: required i32 parser_value_mask;
}

struct TblCntrInfo_t {
  1: required tbl_dbg_counter_type_t type;
  2: required i32 value;
}

struct TblDbgStageInfo_t {
  1: required i32 num_counters;
  2: required list<string> tbl_name;
  3: required list<tbl_dbg_counter_type_t> type;
  4: required list<i32> value;
}

# not very space efficient but convenient
struct netbufferv4_counter_flags_t {
  1: required bool read_hw_sync;
}

struct netbufferv4_register_flags_t {
  1: required bool read_hw_sync;
}

struct netbufferv4_snapshot_trig_spec_t {
  1: required string field_name;
  2: required i64 field_value;
  3: required i64 field_mask;
}

struct netbufferv4_snapshot_tbl_data_t {
  1: required bool hit;
  2: required bool inhibited;
  3: required bool executed;
  4: required i32 hit_entry_handle;
}


enum netbufferv4_input_field_attr_type_t {
  INPUT_FIELD_ATTR_TYPE_MASK
}

enum netbufferv4_input_field_attr_value_mask_t {
  INPUT_FIELD_EXCLUDED = 0,
  INPUT_FIELD_INCLUDED
}



# Match structs

struct netbufferv4_access_cache_frequency_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte inswitch_hdr_is_sampled;
  3: required byte inswitch_hdr_is_cached;
}

struct netbufferv4_access_case1_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte inswitch_hdr_is_cached;
  3: required byte validvalue_hdr_validvalue;
  4: required byte meta_is_latest;
  5: required byte inswitch_hdr_snapshot_flag;
}

struct netbufferv4_access_cm1_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte inswitch_hdr_is_sampled;
  3: required byte inswitch_hdr_is_cached;
}

struct netbufferv4_access_cm2_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte inswitch_hdr_is_sampled;
  3: required byte inswitch_hdr_is_cached;
}

struct netbufferv4_access_cm3_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte inswitch_hdr_is_sampled;
  3: required byte inswitch_hdr_is_cached;
}

struct netbufferv4_access_cm4_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte inswitch_hdr_is_sampled;
  3: required byte inswitch_hdr_is_cached;
}

struct netbufferv4_access_deleted_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte inswitch_hdr_is_cached;
  3: required byte validvalue_hdr_validvalue;
  4: required byte meta_is_latest;
  5: required byte stat_hdr_stat;
}

struct netbufferv4_access_latest_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte inswitch_hdr_is_cached;
  3: required byte validvalue_hdr_validvalue;
}

struct netbufferv4_access_savedseq_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte inswitch_hdr_is_cached;
  3: required byte validvalue_hdr_validvalue;
  4: required byte meta_is_latest;
}

struct netbufferv4_access_seq_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
}

struct netbufferv4_access_validvalue_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte inswitch_hdr_is_cached;
}

struct netbufferv4_add_and_remove_value_header_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required i16 vallen_hdr_vallen_start;
  3: required i16 vallen_hdr_vallen_end;
}

struct netbufferv4_cache_lookup_tbl_match_spec_t {
  1: required i32 op_hdr_keylolo;
  2: required i32 op_hdr_keylohi;
  3: required i32 op_hdr_keyhilo;
  4: required i16 op_hdr_keyhihilo;
  5: required i16 op_hdr_keyhihihi;
  6: required byte meta_need_recirculate;
}

struct netbufferv4_drop_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
}

struct netbufferv4_eg_port_forward_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte inswitch_hdr_is_cached;
  3: required byte meta_is_hot;
  4: required byte validvalue_hdr_validvalue;
  5: required byte meta_is_latest;
  6: required byte meta_is_deleted;
  7: required i16 inswitch_hdr_client_sid;
  8: required byte meta_is_lastclone_for_pktloss;
  9: required byte inswitch_hdr_snapshot_flag;
  10: required byte meta_is_case1;
}

struct netbufferv4_hash_for_cm1_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte meta_need_recirculate;
}

struct netbufferv4_hash_for_cm2_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte meta_need_recirculate;
}

struct netbufferv4_hash_for_cm3_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte meta_need_recirculate;
}

struct netbufferv4_hash_for_cm4_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte meta_need_recirculate;
}

struct netbufferv4_hash_for_partition_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte meta_need_recirculate;
}

struct netbufferv4_hash_for_seq_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte meta_need_recirculate;
}

struct netbufferv4_hash_partition_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required i16 meta_hashval_for_partition_start;
  3: required i16 meta_hashval_for_partition_end;
  4: required byte meta_need_recirculate;
}

struct netbufferv4_ig_port_forward_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte meta_need_recirculate;
}

struct netbufferv4_ipv4_forward_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required i32 ipv4_hdr_dstAddr;
  3: required i16 ipv4_hdr_dstAddr_prefix_length;
  4: required byte meta_need_recirculate;
}

struct netbufferv4_is_hot_tbl_match_spec_t {
  1: required byte meta_cm1_predicate;
  2: required byte meta_cm2_predicate;
  3: required byte meta_cm3_predicate;
  4: required byte meta_cm4_predicate;
}

struct netbufferv4_l2l3_forward_tbl_match_spec_t {
  1: required MacAddr_t ethernet_hdr_dstAddr;
  2: required i32 ipv4_hdr_dstAddr;
  3: required i16 ipv4_hdr_dstAddr_prefix_length;
}

struct netbufferv4_lastclone_lastscansplit_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required i16 clone_hdr_clonenum_for_pktloss;
}

struct netbufferv4_need_recirculate_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required i16 ig_intr_md_ingress_port;
}

struct netbufferv4_prepare_for_cachehit_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required i32 ipv4_hdr_srcAddr;
  3: required i16 ipv4_hdr_srcAddr_prefix_length;
  4: required byte meta_need_recirculate;
}

struct netbufferv4_recirculate_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte meta_need_recirculate;
}

struct netbufferv4_sample_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte meta_need_recirculate;
}

struct netbufferv4_save_client_udpport_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
}

struct netbufferv4_snapshot_flag_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte meta_need_recirculate;
}

struct netbufferv4_update_ipmac_srcport_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required i16 eg_intr_md_egress_port;
}

struct netbufferv4_update_pktlen_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required i16 vallen_hdr_vallen_start;
  3: required i16 vallen_hdr_vallen_end;
}

struct netbufferv4_update_valhi10_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi11_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi12_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi13_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi14_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi15_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi16_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi1_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi2_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi3_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi4_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi5_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi6_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi7_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi8_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_valhi9_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallen_tbl_match_spec_t {
  1: required i16 op_hdr_optype;
  2: required byte inswitch_hdr_is_cached;
  3: required byte validvalue_hdr_validvalue;
  4: required byte meta_is_latest;
}

struct netbufferv4_update_vallo10_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo11_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo12_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo13_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo14_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo15_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo16_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo1_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo2_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo3_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo4_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo5_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo6_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo7_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo8_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}

struct netbufferv4_update_vallo9_tbl_match_spec_t {
  1: required byte meta_access_val_mode;
}


# Match struct for Dynamic Key Mask Exm Table.


# Action structs

struct netbufferv4_cached_action_action_spec_t {
  1: required i16 action_idx;
}

struct netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t {
  1: required i32 action_client_sid;
  2: required i16 action_server_port;
  3: required byte action_stat;
}

struct netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t {
  1: required i32 action_switchos_sid;
  2: required byte action_stat;
  3: required i16 action_reflector_port;
}

struct netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t {
  1: required i32 action_switchos_sid;
}

struct netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t {
  1: required i32 action_switchos_sid;
  2: required byte action_stat;
  3: required i16 action_reflector_port;
}

struct netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t {
  1: required i32 action_switchos_sid;
}

struct netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec_t {
  1: required i32 action_switchos_sid;
  2: required i16 action_reflector_port;
}

struct netbufferv4_update_putreq_inswitch_to_putres_by_mirroring_action_spec_t {
  1: required i32 action_client_sid;
  2: required i16 action_server_port;
}

struct netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t {
  1: required i32 action_switchos_sid;
  2: required byte action_stat;
  3: required i16 action_reflector_port;
}

struct netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t {
  1: required i32 action_switchos_sid;
}

struct netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec_t {
  1: required i32 action_client_sid;
  2: required i16 action_server_port;
}

struct netbufferv4_update_delreq_inswitch_to_delres_by_mirroring_action_spec_t {
  1: required i32 action_client_sid;
  2: required i16 action_server_port;
}

struct netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t {
  1: required i32 action_switchos_sid;
  2: required byte action_stat;
  3: required i16 action_reflector_port;
}

struct netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t {
  1: required i32 action_switchos_sid;
}

struct netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec_t {
  1: required i32 action_client_sid;
  2: required i16 action_server_port;
}

struct netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec_t {
  1: required i32 action_switchos_sid;
  2: required i16 action_reflector_port;
}

struct netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec_t {
  1: required i32 action_switchos_sid;
  2: required i16 action_reflector_port;
  3: required byte action_stat;
}

struct netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec_t {
  1: required i32 action_switchos_sid;
  2: required i16 action_reflector_port;
  3: required byte action_stat;
}

struct netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec_t {
  1: required i32 action_switchos_sid;
  2: required i16 action_reflector_port;
}

struct netbufferv4_hash_partition_action_spec_t {
  1: required i16 action_udpport;
  2: required i16 action_eport;
}

struct netbufferv4_hash_partition_for_special_response_action_spec_t {
  1: required i16 action_eport;
}

struct netbufferv4_forward_normal_response_action_spec_t {
  1: required i16 action_eport;
}

struct netbufferv4_forward_special_get_response_action_spec_t {
  1: required i32 action_client_sid;
}

struct netbufferv4_l2l3_forward_action_spec_t {
  1: required i16 action_eport;
}

struct netbufferv4_set_client_sid_action_spec_t {
  1: required i16 action_client_sid;
}

struct netbufferv4_recirculate_pkt_action_spec_t {
  1: required byte action_port;
}

struct netbufferv4_set_hot_threshold_action_spec_t {
  1: required i16 action_hot_threshold;
}

struct netbufferv4_update_ipmac_srcport_server2client_action_spec_t {
  1: required MacAddr_t action_client_mac;
  2: required MacAddr_t action_server_mac;
  3: required i32 action_client_ip;
  4: required i32 action_server_ip;
  5: required i16 action_server_port;
}

struct netbufferv4_update_ipmac_srcport_switch2switchos_action_spec_t {
  1: required MacAddr_t action_client_mac;
  2: required MacAddr_t action_switch_mac;
  3: required i32 action_client_ip;
  4: required i32 action_switch_ip;
  5: required i16 action_client_port;
}

struct netbufferv4_update_dstipmac_client2server_action_spec_t {
  1: required MacAddr_t action_server_mac;
  2: required i32 action_server_ip;
}

struct netbufferv4_update_pktlen_action_spec_t {
  1: required i16 action_udplen;
  2: required i16 action_iplen;
}

union netbufferv4_action_specs_t {
  1: netbufferv4_cached_action_action_spec_t netbufferv4_cached_action;
  2: netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t netbufferv4_update_getreq_inswitch_to_getres_by_mirroring;
  3: netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss;
  4: netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss;
  5: netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss;
  6: netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss;
  7: netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec_t netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone;
  8: netbufferv4_update_putreq_inswitch_to_putres_by_mirroring_action_spec_t netbufferv4_update_putreq_inswitch_to_putres_by_mirroring;
  9: netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres;
  10: netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres;
  11: netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec_t netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring;
  12: netbufferv4_update_delreq_inswitch_to_delres_by_mirroring_action_spec_t netbufferv4_update_delreq_inswitch_to_delres_by_mirroring;
  13: netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres;
  14: netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres;
  15: netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec_t netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring;
  16: netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec_t netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone;
  17: netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec_t netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone;
  18: netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec_t netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone;
  19: netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec_t netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone;
  20: netbufferv4_hash_partition_action_spec_t netbufferv4_hash_partition;
  21: netbufferv4_hash_partition_for_special_response_action_spec_t netbufferv4_hash_partition_for_special_response;
  22: netbufferv4_forward_normal_response_action_spec_t netbufferv4_forward_normal_response;
  23: netbufferv4_forward_special_get_response_action_spec_t netbufferv4_forward_special_get_response;
  24: netbufferv4_l2l3_forward_action_spec_t netbufferv4_l2l3_forward;
  25: netbufferv4_set_client_sid_action_spec_t netbufferv4_set_client_sid;
  26: netbufferv4_recirculate_pkt_action_spec_t netbufferv4_recirculate_pkt;
  27: netbufferv4_set_hot_threshold_action_spec_t netbufferv4_set_hot_threshold;
  28: netbufferv4_update_ipmac_srcport_server2client_action_spec_t netbufferv4_update_ipmac_srcport_server2client;
  29: netbufferv4_update_ipmac_srcport_switch2switchos_action_spec_t netbufferv4_update_ipmac_srcport_switch2switchos;
  30: netbufferv4_update_dstipmac_client2server_action_spec_t netbufferv4_update_dstipmac_client2server;
  31: netbufferv4_update_pktlen_action_spec_t netbufferv4_update_pktlen;
}

struct netbufferv4_action_desc_t {
  1: required string name;
  2: required netbufferv4_action_specs_t data;
}


# Register values


# Entry Descriptions

struct netbufferv4_access_cache_frequency_tbl_entry_desc_t {
  1: required netbufferv4_access_cache_frequency_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_access_case1_tbl_entry_desc_t {
  1: required netbufferv4_access_case1_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_access_cm1_tbl_entry_desc_t {
  1: required netbufferv4_access_cm1_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_access_cm2_tbl_entry_desc_t {
  1: required netbufferv4_access_cm2_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_access_cm3_tbl_entry_desc_t {
  1: required netbufferv4_access_cm3_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_access_cm4_tbl_entry_desc_t {
  1: required netbufferv4_access_cm4_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_access_deleted_tbl_entry_desc_t {
  1: required netbufferv4_access_deleted_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_access_latest_tbl_entry_desc_t {
  1: required netbufferv4_access_latest_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_access_savedseq_tbl_entry_desc_t {
  1: required netbufferv4_access_savedseq_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_access_seq_tbl_entry_desc_t {
  1: required netbufferv4_access_seq_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_access_validvalue_tbl_entry_desc_t {
  1: required netbufferv4_access_validvalue_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_add_and_remove_value_header_tbl_entry_desc_t {
  1: required netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required i32 priority;
  7: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_cache_lookup_tbl_entry_desc_t {
  1: required netbufferv4_cache_lookup_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_drop_tbl_entry_desc_t {
  1: required netbufferv4_drop_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_eg_port_forward_tbl_entry_desc_t {
  1: required netbufferv4_eg_port_forward_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_hash_for_cm1_tbl_entry_desc_t {
  1: required netbufferv4_hash_for_cm1_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_hash_for_cm2_tbl_entry_desc_t {
  1: required netbufferv4_hash_for_cm2_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_hash_for_cm3_tbl_entry_desc_t {
  1: required netbufferv4_hash_for_cm3_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_hash_for_cm4_tbl_entry_desc_t {
  1: required netbufferv4_hash_for_cm4_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_hash_for_partition_tbl_entry_desc_t {
  1: required netbufferv4_hash_for_partition_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_hash_for_seq_tbl_entry_desc_t {
  1: required netbufferv4_hash_for_seq_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_hash_partition_tbl_entry_desc_t {
  1: required netbufferv4_hash_partition_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required i32 priority;
  7: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_ig_port_forward_tbl_entry_desc_t {
  1: required netbufferv4_ig_port_forward_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_ipv4_forward_tbl_entry_desc_t {
  1: required netbufferv4_ipv4_forward_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_is_hot_tbl_entry_desc_t {
  1: required netbufferv4_is_hot_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_l2l3_forward_tbl_entry_desc_t {
  1: required netbufferv4_l2l3_forward_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_lastclone_lastscansplit_tbl_entry_desc_t {
  1: required netbufferv4_lastclone_lastscansplit_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_need_recirculate_tbl_entry_desc_t {
  1: required netbufferv4_need_recirculate_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_prepare_for_cachehit_tbl_entry_desc_t {
  1: required netbufferv4_prepare_for_cachehit_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_recirculate_tbl_entry_desc_t {
  1: required netbufferv4_recirculate_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_sample_tbl_entry_desc_t {
  1: required netbufferv4_sample_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_save_client_udpport_tbl_entry_desc_t {
  1: required netbufferv4_save_client_udpport_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_set_hot_threshold_tbl_entry_desc_t {
  1: required bool has_mbr_hdl;
  2: required bool has_grp_hdl;
  3: required MemberHandle_t selector_grp_hdl;
  4: required MemberHandle_t action_mbr_hdl;
  5: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_snapshot_flag_tbl_entry_desc_t {
  1: required netbufferv4_snapshot_flag_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_ipmac_srcport_tbl_entry_desc_t {
  1: required netbufferv4_update_ipmac_srcport_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_pktlen_tbl_entry_desc_t {
  1: required netbufferv4_update_pktlen_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required i32 priority;
  7: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi10_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi10_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi11_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi11_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi12_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi12_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi13_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi13_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi14_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi14_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi15_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi15_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi16_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi16_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi1_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi1_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi2_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi2_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi3_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi3_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi4_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi4_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi5_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi5_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi6_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi6_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi7_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi7_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi8_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi8_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_valhi9_tbl_entry_desc_t {
  1: required netbufferv4_update_valhi9_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallen_tbl_entry_desc_t {
  1: required netbufferv4_update_vallen_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo10_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo10_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo11_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo11_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo12_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo12_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo13_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo13_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo14_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo14_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo15_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo15_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo16_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo16_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo1_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo1_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo2_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo2_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo3_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo3_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo4_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo4_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo5_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo5_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo6_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo6_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo7_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo7_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo8_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo8_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}

struct netbufferv4_update_vallo9_tbl_entry_desc_t {
  1: required netbufferv4_update_vallo9_tbl_match_spec_t match_spec;
  2: required bool has_mbr_hdl;
  3: required bool has_grp_hdl;
  4: required MemberHandle_t selector_grp_hdl;
  5: required MemberHandle_t action_mbr_hdl;
  6: required netbufferv4_action_desc_t action_desc;
}




exception InvalidTableOperation {
 1:i32 code
}

exception InvalidLearnOperation {
 1:i32 code
}

exception InvalidDbgOperation {
 1:i32 code
}

exception InvalidSnapshotOperation {
 1:i32 code
}

exception InvalidCounterOperation {
 1:i32 code
}

exception InvalidRegisterOperation {
 1:i32 code
}

exception InvalidMeterOperation {
 1:i32 code
}

exception InvalidLPFOperation {
 1:i32 code
}

exception InvalidWREDOperation {
 1:i32 code
}


service netbufferv4 {

    # Idle time config



    EntryHandle_t access_cache_frequency_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cache_frequency_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_case1_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_case1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm1_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm2_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm3_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm4_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_deleted_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_deleted_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_latest_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_latest_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_savedseq_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_savedseq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_seq_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_seq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_validvalue_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_validvalue_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t cache_lookup_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_cache_lookup_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t drop_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_drop_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm1_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm2_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm3_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm4_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_partition_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_partition_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_seq_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_seq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_partition_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_partition_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ig_port_forward_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ipv4_forward_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ipv4_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t is_hot_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_is_hot_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t l2l3_forward_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_l2l3_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t lastclone_lastscansplit_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_lastclone_lastscansplit_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t need_recirculate_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_need_recirculate_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t prepare_for_cachehit_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_prepare_for_cachehit_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t recirculate_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_recirculate_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t sample_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_sample_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t save_client_udpport_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_save_client_udpport_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t snapshot_flag_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_snapshot_flag_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_ipmac_srcport_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_ipmac_srcport_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_pktlen_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_pktlen_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi10_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi11_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi12_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi13_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi14_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi15_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi16_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi1_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi2_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi3_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi4_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi5_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi6_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi7_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi8_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi9_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallen_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallen_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo10_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo11_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo12_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo13_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo14_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo15_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo16_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo1_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo2_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo3_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo4_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo5_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo6_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo7_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo8_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo9_tbl_match_spec_to_entry_hdl(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),


    # Dynamic Key Mask Exm Table.
      # set API

    # Table entry add functions

    EntryHandle_t access_cache_frequency_tbl_table_add_with_get_cache_frequency(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cache_frequency_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cache_frequency_tbl_table_add_with_update_cache_frequency(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cache_frequency_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cache_frequency_tbl_table_add_with_reset_cache_frequency(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cache_frequency_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cache_frequency_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cache_frequency_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_case1_tbl_table_add_with_try_case1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_case1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_case1_tbl_table_add_with_read_case1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_case1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_case1_tbl_table_add_with_reset_is_case1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_case1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm1_tbl_table_add_with_update_cm1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm1_tbl_table_add_with_initialize_cm1_predicate(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm2_tbl_table_add_with_update_cm2(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm2_tbl_table_add_with_initialize_cm2_predicate(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm3_tbl_table_add_with_update_cm3(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm3_tbl_table_add_with_initialize_cm3_predicate(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm4_tbl_table_add_with_update_cm4(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm4_tbl_table_add_with_initialize_cm4_predicate(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_deleted_tbl_table_add_with_get_deleted(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_deleted_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_deleted_tbl_table_add_with_set_and_get_deleted(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_deleted_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_deleted_tbl_table_add_with_reset_and_get_deleted(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_deleted_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_deleted_tbl_table_add_with_reset_is_deleted(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_deleted_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_latest_tbl_table_add_with_get_latest(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_latest_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_latest_tbl_table_add_with_set_and_get_latest(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_latest_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_latest_tbl_table_add_with_reset_and_get_latest(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_latest_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_latest_tbl_table_add_with_reset_is_latest(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_latest_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_savedseq_tbl_table_add_with_get_savedseq(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_savedseq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_savedseq_tbl_table_add_with_set_and_get_savedseq(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_savedseq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_savedseq_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_savedseq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_seq_tbl_table_add_with_assign_seq(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_seq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_seq_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_seq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_validvalue_tbl_table_add_with_get_validvalue(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_validvalue_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_validvalue_tbl_table_add_with_set_validvalue(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_validvalue_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_validvalue_tbl_table_add_with_reset_meta_validvalue(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_validvalue_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_only_vallen(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val2(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val3(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val4(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val5(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val6(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val7(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val8(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val9(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val10(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val11(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val12(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val13(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val14(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val15(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_add_to_val16(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_add_with_remove_all(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t cache_lookup_tbl_table_add_with_cached_action(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_cache_lookup_tbl_match_spec_t match_spec, 4:netbufferv4_cached_action_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t cache_lookup_tbl_table_add_with_uncached_action(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_cache_lookup_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t drop_tbl_table_add_with_drop_getres_latest_seq_inswitch(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_drop_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t drop_tbl_table_add_with_drop_getres_deleted_seq_inswitch(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_drop_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t drop_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_drop_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_pop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_nlatest(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_getres_latest_seq_to_getres(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_to_getres(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_pop_seq(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putres_by_mirroring(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_putreq_inswitch_to_putres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq_case3(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_pop_seq_case3(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delres_by_mirroring(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_delreq_inswitch_to_delres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq_case3(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm1_tbl_table_add_with_hash_for_cm1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm1_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm2_tbl_table_add_with_hash_for_cm2(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm2_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm3_tbl_table_add_with_hash_for_cm3(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm3_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm4_tbl_table_add_with_hash_for_cm4(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm4_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_partition_tbl_table_add_with_hash_for_partition(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_partition_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_partition_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_partition_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_seq_tbl_table_add_with_hash_for_seq(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_seq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_seq_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_seq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_partition_tbl_table_add_with_hash_partition(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_partition_tbl_match_spec_t match_spec, 4:i32 priority, 5:netbufferv4_hash_partition_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_partition_tbl_table_add_with_hash_partition_for_special_response(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_partition_tbl_match_spec_t match_spec, 4:i32 priority, 5:netbufferv4_hash_partition_for_special_response_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_partition_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_partition_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ig_port_forward_tbl_table_add_with_update_getreq_to_getreq_inswitch(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ig_port_forward_tbl_table_add_with_update_getres_latest_seq_to_getres_latest_seq_inswitch(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ig_port_forward_tbl_table_add_with_update_getres_deleted_seq_to_getres_deleted_seq_inswitch(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ig_port_forward_tbl_table_add_with_update_putreq_to_putreq_inswitch(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ig_port_forward_tbl_table_add_with_update_delreq_to_delreq_inswitch(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ig_port_forward_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ipv4_forward_tbl_table_add_with_forward_normal_response(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ipv4_forward_tbl_match_spec_t match_spec, 4:netbufferv4_forward_normal_response_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ipv4_forward_tbl_table_add_with_forward_special_get_response(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ipv4_forward_tbl_match_spec_t match_spec, 4:netbufferv4_forward_special_get_response_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ipv4_forward_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ipv4_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t is_hot_tbl_table_add_with_set_is_hot(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_is_hot_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t is_hot_tbl_table_add_with_reset_is_hot(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_is_hot_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t l2l3_forward_tbl_table_add_with_l2l3_forward(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_l2l3_forward_tbl_match_spec_t match_spec, 4:netbufferv4_l2l3_forward_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t l2l3_forward_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_l2l3_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t lastclone_lastscansplit_tbl_table_add_with_set_is_lastclone(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_lastclone_lastscansplit_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t lastclone_lastscansplit_tbl_table_add_with_reset_is_lastclone_lastscansplit(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_lastclone_lastscansplit_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t need_recirculate_tbl_table_add_with_set_need_recirculate(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_need_recirculate_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t need_recirculate_tbl_table_add_with_reset_need_recirculate(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_need_recirculate_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t prepare_for_cachehit_tbl_table_add_with_set_client_sid(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_prepare_for_cachehit_tbl_match_spec_t match_spec, 4:netbufferv4_set_client_sid_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t prepare_for_cachehit_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_prepare_for_cachehit_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t recirculate_tbl_table_add_with_recirculate_pkt(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_recirculate_tbl_match_spec_t match_spec, 4:netbufferv4_recirculate_pkt_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t recirculate_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_recirculate_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t sample_tbl_table_add_with_sample(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_sample_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t sample_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_sample_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t save_client_udpport_tbl_table_add_with_save_client_udpport(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_save_client_udpport_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t save_client_udpport_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_save_client_udpport_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t snapshot_flag_tbl_table_add_with_set_snapshot_flag(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_snapshot_flag_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t snapshot_flag_tbl_table_add_with_reset_snapshot_flag(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_snapshot_flag_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_ipmac_srcport_tbl_table_add_with_update_ipmac_srcport_server2client(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_ipmac_srcport_tbl_match_spec_t match_spec, 4:netbufferv4_update_ipmac_srcport_server2client_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_ipmac_srcport_tbl_table_add_with_update_ipmac_srcport_switch2switchos(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_ipmac_srcport_tbl_match_spec_t match_spec, 4:netbufferv4_update_ipmac_srcport_switch2switchos_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_ipmac_srcport_tbl_table_add_with_update_dstipmac_client2server(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_ipmac_srcport_tbl_match_spec_t match_spec, 4:netbufferv4_update_dstipmac_client2server_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_ipmac_srcport_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_ipmac_srcport_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_pktlen_tbl_table_add_with_update_pktlen(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_pktlen_tbl_match_spec_t match_spec, 4:i32 priority, 5:netbufferv4_update_pktlen_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_pktlen_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_pktlen_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi10_tbl_table_add_with_get_valhi10(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi10_tbl_table_add_with_set_and_get_valhi10(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi10_tbl_table_add_with_reset_and_get_valhi10(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi10_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi11_tbl_table_add_with_get_valhi11(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi11_tbl_table_add_with_set_and_get_valhi11(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi11_tbl_table_add_with_reset_and_get_valhi11(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi11_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi12_tbl_table_add_with_get_valhi12(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi12_tbl_table_add_with_set_and_get_valhi12(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi12_tbl_table_add_with_reset_and_get_valhi12(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi12_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi13_tbl_table_add_with_get_valhi13(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi13_tbl_table_add_with_set_and_get_valhi13(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi13_tbl_table_add_with_reset_and_get_valhi13(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi13_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi14_tbl_table_add_with_get_valhi14(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi14_tbl_table_add_with_set_and_get_valhi14(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi14_tbl_table_add_with_reset_and_get_valhi14(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi14_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi15_tbl_table_add_with_get_valhi15(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi15_tbl_table_add_with_set_and_get_valhi15(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi15_tbl_table_add_with_reset_and_get_valhi15(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi15_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi16_tbl_table_add_with_get_valhi16(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi16_tbl_table_add_with_set_and_get_valhi16(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi16_tbl_table_add_with_reset_and_get_valhi16(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi16_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi1_tbl_table_add_with_get_valhi1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi1_tbl_table_add_with_set_and_get_valhi1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi1_tbl_table_add_with_reset_and_get_valhi1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi1_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi2_tbl_table_add_with_get_valhi2(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi2_tbl_table_add_with_set_and_get_valhi2(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi2_tbl_table_add_with_reset_and_get_valhi2(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi2_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi3_tbl_table_add_with_get_valhi3(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi3_tbl_table_add_with_set_and_get_valhi3(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi3_tbl_table_add_with_reset_and_get_valhi3(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi3_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi4_tbl_table_add_with_get_valhi4(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi4_tbl_table_add_with_set_and_get_valhi4(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi4_tbl_table_add_with_reset_and_get_valhi4(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi4_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi5_tbl_table_add_with_get_valhi5(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi5_tbl_table_add_with_set_and_get_valhi5(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi5_tbl_table_add_with_reset_and_get_valhi5(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi5_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi6_tbl_table_add_with_get_valhi6(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi6_tbl_table_add_with_set_and_get_valhi6(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi6_tbl_table_add_with_reset_and_get_valhi6(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi6_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi7_tbl_table_add_with_get_valhi7(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi7_tbl_table_add_with_set_and_get_valhi7(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi7_tbl_table_add_with_reset_and_get_valhi7(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi7_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi8_tbl_table_add_with_get_valhi8(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi8_tbl_table_add_with_set_and_get_valhi8(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi8_tbl_table_add_with_reset_and_get_valhi8(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi8_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi9_tbl_table_add_with_get_valhi9(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi9_tbl_table_add_with_set_and_get_valhi9(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi9_tbl_table_add_with_reset_and_get_valhi9(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi9_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallen_tbl_table_add_with_get_vallen(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallen_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallen_tbl_table_add_with_set_and_get_vallen(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallen_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallen_tbl_table_add_with_reset_and_get_vallen(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallen_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallen_tbl_table_add_with_reset_access_val_mode(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallen_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallen_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallen_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo10_tbl_table_add_with_get_vallo10(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo10_tbl_table_add_with_set_and_get_vallo10(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo10_tbl_table_add_with_reset_and_get_vallo10(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo10_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo11_tbl_table_add_with_get_vallo11(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo11_tbl_table_add_with_set_and_get_vallo11(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo11_tbl_table_add_with_reset_and_get_vallo11(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo11_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo12_tbl_table_add_with_get_vallo12(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo12_tbl_table_add_with_set_and_get_vallo12(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo12_tbl_table_add_with_reset_and_get_vallo12(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo12_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo13_tbl_table_add_with_get_vallo13(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo13_tbl_table_add_with_set_and_get_vallo13(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo13_tbl_table_add_with_reset_and_get_vallo13(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo13_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo14_tbl_table_add_with_get_vallo14(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo14_tbl_table_add_with_set_and_get_vallo14(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo14_tbl_table_add_with_reset_and_get_vallo14(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo14_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo15_tbl_table_add_with_get_vallo15(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo15_tbl_table_add_with_set_and_get_vallo15(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo15_tbl_table_add_with_reset_and_get_vallo15(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo15_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo16_tbl_table_add_with_get_vallo16(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo16_tbl_table_add_with_set_and_get_vallo16(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo16_tbl_table_add_with_reset_and_get_vallo16(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo16_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo1_tbl_table_add_with_get_vallo1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo1_tbl_table_add_with_set_and_get_vallo1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo1_tbl_table_add_with_reset_and_get_vallo1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo1_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo2_tbl_table_add_with_get_vallo2(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo2_tbl_table_add_with_set_and_get_vallo2(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo2_tbl_table_add_with_reset_and_get_vallo2(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo2_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo3_tbl_table_add_with_get_vallo3(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo3_tbl_table_add_with_set_and_get_vallo3(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo3_tbl_table_add_with_reset_and_get_vallo3(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo3_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo4_tbl_table_add_with_get_vallo4(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo4_tbl_table_add_with_set_and_get_vallo4(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo4_tbl_table_add_with_reset_and_get_vallo4(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo4_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo5_tbl_table_add_with_get_vallo5(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo5_tbl_table_add_with_set_and_get_vallo5(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo5_tbl_table_add_with_reset_and_get_vallo5(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo5_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo6_tbl_table_add_with_get_vallo6(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo6_tbl_table_add_with_set_and_get_vallo6(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo6_tbl_table_add_with_reset_and_get_vallo6(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo6_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo7_tbl_table_add_with_get_vallo7(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo7_tbl_table_add_with_set_and_get_vallo7(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo7_tbl_table_add_with_reset_and_get_vallo7(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo7_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo8_tbl_table_add_with_get_vallo8(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo8_tbl_table_add_with_set_and_get_vallo8(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo8_tbl_table_add_with_reset_and_get_vallo8(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo8_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo9_tbl_table_add_with_get_vallo9(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo9_tbl_table_add_with_set_and_get_vallo9(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo9_tbl_table_add_with_reset_and_get_vallo9(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo9_tbl_table_add_with_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),

    # Table entry modify functions
    void access_cache_frequency_tbl_table_modify_with_get_cache_frequency(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cache_frequency_tbl_table_modify_with_get_cache_frequency_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cache_frequency_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_cache_frequency_tbl_table_modify_with_update_cache_frequency(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cache_frequency_tbl_table_modify_with_update_cache_frequency_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cache_frequency_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_cache_frequency_tbl_table_modify_with_reset_cache_frequency(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cache_frequency_tbl_table_modify_with_reset_cache_frequency_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cache_frequency_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_cache_frequency_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cache_frequency_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cache_frequency_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_case1_tbl_table_modify_with_try_case1(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_case1_tbl_table_modify_with_try_case1_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_case1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_case1_tbl_table_modify_with_read_case1(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_case1_tbl_table_modify_with_read_case1_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_case1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_case1_tbl_table_modify_with_reset_is_case1(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_case1_tbl_table_modify_with_reset_is_case1_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_case1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_cm1_tbl_table_modify_with_update_cm1(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cm1_tbl_table_modify_with_update_cm1_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_cm1_tbl_table_modify_with_initialize_cm1_predicate(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cm1_tbl_table_modify_with_initialize_cm1_predicate_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_cm2_tbl_table_modify_with_update_cm2(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cm2_tbl_table_modify_with_update_cm2_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_cm2_tbl_table_modify_with_initialize_cm2_predicate(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cm2_tbl_table_modify_with_initialize_cm2_predicate_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_cm3_tbl_table_modify_with_update_cm3(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cm3_tbl_table_modify_with_update_cm3_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_cm3_tbl_table_modify_with_initialize_cm3_predicate(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cm3_tbl_table_modify_with_initialize_cm3_predicate_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_cm4_tbl_table_modify_with_update_cm4(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cm4_tbl_table_modify_with_update_cm4_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_cm4_tbl_table_modify_with_initialize_cm4_predicate(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cm4_tbl_table_modify_with_initialize_cm4_predicate_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_deleted_tbl_table_modify_with_get_deleted(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_deleted_tbl_table_modify_with_get_deleted_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_deleted_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_deleted_tbl_table_modify_with_set_and_get_deleted(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_deleted_tbl_table_modify_with_set_and_get_deleted_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_deleted_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_deleted_tbl_table_modify_with_reset_and_get_deleted(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_deleted_tbl_table_modify_with_reset_and_get_deleted_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_deleted_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_deleted_tbl_table_modify_with_reset_is_deleted(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_deleted_tbl_table_modify_with_reset_is_deleted_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_deleted_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_latest_tbl_table_modify_with_get_latest(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_latest_tbl_table_modify_with_get_latest_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_latest_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_latest_tbl_table_modify_with_set_and_get_latest(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_latest_tbl_table_modify_with_set_and_get_latest_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_latest_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_latest_tbl_table_modify_with_reset_and_get_latest(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_latest_tbl_table_modify_with_reset_and_get_latest_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_latest_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_latest_tbl_table_modify_with_reset_is_latest(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_latest_tbl_table_modify_with_reset_is_latest_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_latest_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_savedseq_tbl_table_modify_with_get_savedseq(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_savedseq_tbl_table_modify_with_get_savedseq_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_savedseq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_savedseq_tbl_table_modify_with_set_and_get_savedseq(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_savedseq_tbl_table_modify_with_set_and_get_savedseq_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_savedseq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_savedseq_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_savedseq_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_savedseq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_seq_tbl_table_modify_with_assign_seq(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_seq_tbl_table_modify_with_assign_seq_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_seq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_seq_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_seq_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_seq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_validvalue_tbl_table_modify_with_get_validvalue(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_validvalue_tbl_table_modify_with_get_validvalue_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_validvalue_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_validvalue_tbl_table_modify_with_set_validvalue(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_validvalue_tbl_table_modify_with_set_validvalue_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_validvalue_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void access_validvalue_tbl_table_modify_with_reset_meta_validvalue(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_validvalue_tbl_table_modify_with_reset_meta_validvalue_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_validvalue_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_only_vallen(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_only_vallen_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val1(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val1_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val2(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val2_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val3(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val3_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val4(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val4_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val5(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val5_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val6(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val6_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val7(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val7_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val8(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val8_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val9(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val9_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val10(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val10_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val11(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val11_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val12(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val12_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val13(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val13_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val14(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val14_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val15(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val15_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val16(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_add_to_val16_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_remove_all(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_modify_with_remove_all_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void cache_lookup_tbl_table_modify_with_cached_action(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_cached_action_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void cache_lookup_tbl_table_modify_with_cached_action_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_cache_lookup_tbl_match_spec_t match_spec, 4:netbufferv4_cached_action_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void cache_lookup_tbl_table_modify_with_uncached_action(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void cache_lookup_tbl_table_modify_with_uncached_action_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_cache_lookup_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void drop_tbl_table_modify_with_drop_getres_latest_seq_inswitch(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void drop_tbl_table_modify_with_drop_getres_latest_seq_inswitch_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_drop_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void drop_tbl_table_modify_with_drop_getres_deleted_seq_inswitch(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void drop_tbl_table_modify_with_drop_getres_deleted_seq_inswitch_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_drop_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void drop_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void drop_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_drop_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_pop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_pop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_nlatest(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_nlatest_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getres_by_mirroring(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getres_by_mirroring_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getres_latest_seq_to_getres(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getres_latest_seq_to_getres_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getres_deleted_seq_to_getres(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getres_deleted_seq_to_getres_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_pop_seq(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_pop_seq_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putres_by_mirroring(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_putreq_inswitch_to_putres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putres_by_mirroring_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_putreq_inswitch_to_putres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_case3(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_case3_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_pop_seq_case3(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_pop_seq_case3_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delres_by_mirroring(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_delreq_inswitch_to_delres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delres_by_mirroring_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_delreq_inswitch_to_delres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_case3(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_case3_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec, 4:netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_for_cm1_tbl_table_modify_with_hash_for_cm1(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_cm1_tbl_table_modify_with_hash_for_cm1_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_for_cm1_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_cm1_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_for_cm2_tbl_table_modify_with_hash_for_cm2(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_cm2_tbl_table_modify_with_hash_for_cm2_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_for_cm2_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_cm2_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_for_cm3_tbl_table_modify_with_hash_for_cm3(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_cm3_tbl_table_modify_with_hash_for_cm3_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_for_cm3_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_cm3_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_for_cm4_tbl_table_modify_with_hash_for_cm4(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_cm4_tbl_table_modify_with_hash_for_cm4_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_for_cm4_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_cm4_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_for_partition_tbl_table_modify_with_hash_for_partition(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_partition_tbl_table_modify_with_hash_for_partition_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_partition_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_for_partition_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_partition_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_partition_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_for_seq_tbl_table_modify_with_hash_for_seq(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_seq_tbl_table_modify_with_hash_for_seq_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_seq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_for_seq_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_seq_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_seq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void hash_partition_tbl_table_modify_with_hash_partition(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_hash_partition_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void hash_partition_tbl_table_modify_with_hash_partition_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_partition_tbl_match_spec_t match_spec, 4:i32 priority, 5:netbufferv4_hash_partition_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void hash_partition_tbl_table_modify_with_hash_partition_for_special_response(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_hash_partition_for_special_response_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void hash_partition_tbl_table_modify_with_hash_partition_for_special_response_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_partition_tbl_match_spec_t match_spec, 4:i32 priority, 5:netbufferv4_hash_partition_for_special_response_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void hash_partition_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_partition_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_partition_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_modify_with_update_getreq_to_getreq_inswitch(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_modify_with_update_getreq_to_getreq_inswitch_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_modify_with_update_getres_latest_seq_to_getres_latest_seq_inswitch(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_modify_with_update_getres_latest_seq_to_getres_latest_seq_inswitch_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_modify_with_update_getres_deleted_seq_to_getres_deleted_seq_inswitch(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_modify_with_update_getres_deleted_seq_to_getres_deleted_seq_inswitch_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_modify_with_update_putreq_to_putreq_inswitch(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_modify_with_update_putreq_to_putreq_inswitch_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_modify_with_update_delreq_to_delreq_inswitch(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_modify_with_update_delreq_to_delreq_inswitch_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void ipv4_forward_tbl_table_modify_with_forward_normal_response(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_forward_normal_response_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void ipv4_forward_tbl_table_modify_with_forward_normal_response_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ipv4_forward_tbl_match_spec_t match_spec, 4:netbufferv4_forward_normal_response_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void ipv4_forward_tbl_table_modify_with_forward_special_get_response(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_forward_special_get_response_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void ipv4_forward_tbl_table_modify_with_forward_special_get_response_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ipv4_forward_tbl_match_spec_t match_spec, 4:netbufferv4_forward_special_get_response_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void ipv4_forward_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void ipv4_forward_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ipv4_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void is_hot_tbl_table_modify_with_set_is_hot(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void is_hot_tbl_table_modify_with_set_is_hot_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_is_hot_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void is_hot_tbl_table_modify_with_reset_is_hot(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void is_hot_tbl_table_modify_with_reset_is_hot_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_is_hot_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void l2l3_forward_tbl_table_modify_with_l2l3_forward(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_l2l3_forward_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void l2l3_forward_tbl_table_modify_with_l2l3_forward_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_l2l3_forward_tbl_match_spec_t match_spec, 4:netbufferv4_l2l3_forward_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void l2l3_forward_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void l2l3_forward_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_l2l3_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void lastclone_lastscansplit_tbl_table_modify_with_set_is_lastclone(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void lastclone_lastscansplit_tbl_table_modify_with_set_is_lastclone_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_lastclone_lastscansplit_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void lastclone_lastscansplit_tbl_table_modify_with_reset_is_lastclone_lastscansplit(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void lastclone_lastscansplit_tbl_table_modify_with_reset_is_lastclone_lastscansplit_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_lastclone_lastscansplit_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void need_recirculate_tbl_table_modify_with_set_need_recirculate(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void need_recirculate_tbl_table_modify_with_set_need_recirculate_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_need_recirculate_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void need_recirculate_tbl_table_modify_with_reset_need_recirculate(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void need_recirculate_tbl_table_modify_with_reset_need_recirculate_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_need_recirculate_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void prepare_for_cachehit_tbl_table_modify_with_set_client_sid(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_set_client_sid_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void prepare_for_cachehit_tbl_table_modify_with_set_client_sid_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_prepare_for_cachehit_tbl_match_spec_t match_spec, 4:netbufferv4_set_client_sid_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void prepare_for_cachehit_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void prepare_for_cachehit_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_prepare_for_cachehit_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void recirculate_tbl_table_modify_with_recirculate_pkt(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_recirculate_pkt_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void recirculate_tbl_table_modify_with_recirculate_pkt_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_recirculate_tbl_match_spec_t match_spec, 4:netbufferv4_recirculate_pkt_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void recirculate_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void recirculate_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_recirculate_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void sample_tbl_table_modify_with_sample(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void sample_tbl_table_modify_with_sample_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_sample_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void sample_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void sample_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_sample_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void save_client_udpport_tbl_table_modify_with_save_client_udpport(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void save_client_udpport_tbl_table_modify_with_save_client_udpport_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_save_client_udpport_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void save_client_udpport_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void save_client_udpport_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_save_client_udpport_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void snapshot_flag_tbl_table_modify_with_set_snapshot_flag(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void snapshot_flag_tbl_table_modify_with_set_snapshot_flag_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_snapshot_flag_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void snapshot_flag_tbl_table_modify_with_reset_snapshot_flag(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void snapshot_flag_tbl_table_modify_with_reset_snapshot_flag_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_snapshot_flag_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_ipmac_srcport_tbl_table_modify_with_update_ipmac_srcport_server2client(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_ipmac_srcport_server2client_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void update_ipmac_srcport_tbl_table_modify_with_update_ipmac_srcport_server2client_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_ipmac_srcport_tbl_match_spec_t match_spec, 4:netbufferv4_update_ipmac_srcport_server2client_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void update_ipmac_srcport_tbl_table_modify_with_update_ipmac_srcport_switch2switchos(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_ipmac_srcport_switch2switchos_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void update_ipmac_srcport_tbl_table_modify_with_update_ipmac_srcport_switch2switchos_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_ipmac_srcport_tbl_match_spec_t match_spec, 4:netbufferv4_update_ipmac_srcport_switch2switchos_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void update_ipmac_srcport_tbl_table_modify_with_update_dstipmac_client2server(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_dstipmac_client2server_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void update_ipmac_srcport_tbl_table_modify_with_update_dstipmac_client2server_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_ipmac_srcport_tbl_match_spec_t match_spec, 4:netbufferv4_update_dstipmac_client2server_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void update_ipmac_srcport_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_ipmac_srcport_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_ipmac_srcport_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_pktlen_tbl_table_modify_with_update_pktlen(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry, 4:netbufferv4_update_pktlen_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void update_pktlen_tbl_table_modify_with_update_pktlen_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_pktlen_tbl_match_spec_t match_spec, 4:i32 priority, 5:netbufferv4_update_pktlen_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    void update_pktlen_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_pktlen_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_pktlen_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
    void update_valhi10_tbl_table_modify_with_get_valhi10(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi10_tbl_table_modify_with_get_valhi10_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi10_tbl_table_modify_with_set_and_get_valhi10(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi10_tbl_table_modify_with_set_and_get_valhi10_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi10_tbl_table_modify_with_reset_and_get_valhi10(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi10_tbl_table_modify_with_reset_and_get_valhi10_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi10_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi10_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi11_tbl_table_modify_with_get_valhi11(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi11_tbl_table_modify_with_get_valhi11_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi11_tbl_table_modify_with_set_and_get_valhi11(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi11_tbl_table_modify_with_set_and_get_valhi11_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi11_tbl_table_modify_with_reset_and_get_valhi11(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi11_tbl_table_modify_with_reset_and_get_valhi11_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi11_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi11_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi12_tbl_table_modify_with_get_valhi12(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi12_tbl_table_modify_with_get_valhi12_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi12_tbl_table_modify_with_set_and_get_valhi12(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi12_tbl_table_modify_with_set_and_get_valhi12_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi12_tbl_table_modify_with_reset_and_get_valhi12(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi12_tbl_table_modify_with_reset_and_get_valhi12_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi12_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi12_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi13_tbl_table_modify_with_get_valhi13(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi13_tbl_table_modify_with_get_valhi13_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi13_tbl_table_modify_with_set_and_get_valhi13(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi13_tbl_table_modify_with_set_and_get_valhi13_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi13_tbl_table_modify_with_reset_and_get_valhi13(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi13_tbl_table_modify_with_reset_and_get_valhi13_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi13_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi13_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi14_tbl_table_modify_with_get_valhi14(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi14_tbl_table_modify_with_get_valhi14_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi14_tbl_table_modify_with_set_and_get_valhi14(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi14_tbl_table_modify_with_set_and_get_valhi14_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi14_tbl_table_modify_with_reset_and_get_valhi14(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi14_tbl_table_modify_with_reset_and_get_valhi14_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi14_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi14_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi15_tbl_table_modify_with_get_valhi15(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi15_tbl_table_modify_with_get_valhi15_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi15_tbl_table_modify_with_set_and_get_valhi15(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi15_tbl_table_modify_with_set_and_get_valhi15_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi15_tbl_table_modify_with_reset_and_get_valhi15(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi15_tbl_table_modify_with_reset_and_get_valhi15_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi15_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi15_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi16_tbl_table_modify_with_get_valhi16(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi16_tbl_table_modify_with_get_valhi16_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi16_tbl_table_modify_with_set_and_get_valhi16(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi16_tbl_table_modify_with_set_and_get_valhi16_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi16_tbl_table_modify_with_reset_and_get_valhi16(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi16_tbl_table_modify_with_reset_and_get_valhi16_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi16_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi16_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi1_tbl_table_modify_with_get_valhi1(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi1_tbl_table_modify_with_get_valhi1_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi1_tbl_table_modify_with_set_and_get_valhi1(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi1_tbl_table_modify_with_set_and_get_valhi1_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi1_tbl_table_modify_with_reset_and_get_valhi1(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi1_tbl_table_modify_with_reset_and_get_valhi1_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi1_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi1_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi2_tbl_table_modify_with_get_valhi2(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi2_tbl_table_modify_with_get_valhi2_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi2_tbl_table_modify_with_set_and_get_valhi2(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi2_tbl_table_modify_with_set_and_get_valhi2_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi2_tbl_table_modify_with_reset_and_get_valhi2(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi2_tbl_table_modify_with_reset_and_get_valhi2_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi2_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi2_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi3_tbl_table_modify_with_get_valhi3(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi3_tbl_table_modify_with_get_valhi3_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi3_tbl_table_modify_with_set_and_get_valhi3(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi3_tbl_table_modify_with_set_and_get_valhi3_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi3_tbl_table_modify_with_reset_and_get_valhi3(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi3_tbl_table_modify_with_reset_and_get_valhi3_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi3_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi3_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi4_tbl_table_modify_with_get_valhi4(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi4_tbl_table_modify_with_get_valhi4_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi4_tbl_table_modify_with_set_and_get_valhi4(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi4_tbl_table_modify_with_set_and_get_valhi4_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi4_tbl_table_modify_with_reset_and_get_valhi4(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi4_tbl_table_modify_with_reset_and_get_valhi4_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi4_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi4_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi5_tbl_table_modify_with_get_valhi5(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi5_tbl_table_modify_with_get_valhi5_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi5_tbl_table_modify_with_set_and_get_valhi5(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi5_tbl_table_modify_with_set_and_get_valhi5_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi5_tbl_table_modify_with_reset_and_get_valhi5(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi5_tbl_table_modify_with_reset_and_get_valhi5_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi5_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi5_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi6_tbl_table_modify_with_get_valhi6(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi6_tbl_table_modify_with_get_valhi6_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi6_tbl_table_modify_with_set_and_get_valhi6(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi6_tbl_table_modify_with_set_and_get_valhi6_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi6_tbl_table_modify_with_reset_and_get_valhi6(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi6_tbl_table_modify_with_reset_and_get_valhi6_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi6_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi6_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi7_tbl_table_modify_with_get_valhi7(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi7_tbl_table_modify_with_get_valhi7_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi7_tbl_table_modify_with_set_and_get_valhi7(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi7_tbl_table_modify_with_set_and_get_valhi7_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi7_tbl_table_modify_with_reset_and_get_valhi7(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi7_tbl_table_modify_with_reset_and_get_valhi7_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi7_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi7_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi8_tbl_table_modify_with_get_valhi8(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi8_tbl_table_modify_with_get_valhi8_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi8_tbl_table_modify_with_set_and_get_valhi8(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi8_tbl_table_modify_with_set_and_get_valhi8_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi8_tbl_table_modify_with_reset_and_get_valhi8(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi8_tbl_table_modify_with_reset_and_get_valhi8_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi8_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi8_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi9_tbl_table_modify_with_get_valhi9(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi9_tbl_table_modify_with_get_valhi9_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi9_tbl_table_modify_with_set_and_get_valhi9(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi9_tbl_table_modify_with_set_and_get_valhi9_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi9_tbl_table_modify_with_reset_and_get_valhi9(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi9_tbl_table_modify_with_reset_and_get_valhi9_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_valhi9_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi9_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallen_tbl_table_modify_with_get_vallen(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallen_tbl_table_modify_with_get_vallen_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallen_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallen_tbl_table_modify_with_set_and_get_vallen(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallen_tbl_table_modify_with_set_and_get_vallen_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallen_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallen_tbl_table_modify_with_reset_and_get_vallen(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallen_tbl_table_modify_with_reset_and_get_vallen_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallen_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallen_tbl_table_modify_with_reset_access_val_mode(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallen_tbl_table_modify_with_reset_access_val_mode_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallen_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallen_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallen_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallen_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo10_tbl_table_modify_with_get_vallo10(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo10_tbl_table_modify_with_get_vallo10_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo10_tbl_table_modify_with_set_and_get_vallo10(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo10_tbl_table_modify_with_set_and_get_vallo10_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo10_tbl_table_modify_with_reset_and_get_vallo10(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo10_tbl_table_modify_with_reset_and_get_vallo10_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo10_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo10_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo11_tbl_table_modify_with_get_vallo11(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo11_tbl_table_modify_with_get_vallo11_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo11_tbl_table_modify_with_set_and_get_vallo11(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo11_tbl_table_modify_with_set_and_get_vallo11_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo11_tbl_table_modify_with_reset_and_get_vallo11(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo11_tbl_table_modify_with_reset_and_get_vallo11_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo11_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo11_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo12_tbl_table_modify_with_get_vallo12(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo12_tbl_table_modify_with_get_vallo12_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo12_tbl_table_modify_with_set_and_get_vallo12(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo12_tbl_table_modify_with_set_and_get_vallo12_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo12_tbl_table_modify_with_reset_and_get_vallo12(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo12_tbl_table_modify_with_reset_and_get_vallo12_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo12_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo12_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo13_tbl_table_modify_with_get_vallo13(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo13_tbl_table_modify_with_get_vallo13_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo13_tbl_table_modify_with_set_and_get_vallo13(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo13_tbl_table_modify_with_set_and_get_vallo13_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo13_tbl_table_modify_with_reset_and_get_vallo13(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo13_tbl_table_modify_with_reset_and_get_vallo13_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo13_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo13_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo14_tbl_table_modify_with_get_vallo14(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo14_tbl_table_modify_with_get_vallo14_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo14_tbl_table_modify_with_set_and_get_vallo14(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo14_tbl_table_modify_with_set_and_get_vallo14_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo14_tbl_table_modify_with_reset_and_get_vallo14(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo14_tbl_table_modify_with_reset_and_get_vallo14_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo14_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo14_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo15_tbl_table_modify_with_get_vallo15(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo15_tbl_table_modify_with_get_vallo15_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo15_tbl_table_modify_with_set_and_get_vallo15(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo15_tbl_table_modify_with_set_and_get_vallo15_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo15_tbl_table_modify_with_reset_and_get_vallo15(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo15_tbl_table_modify_with_reset_and_get_vallo15_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo15_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo15_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo16_tbl_table_modify_with_get_vallo16(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo16_tbl_table_modify_with_get_vallo16_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo16_tbl_table_modify_with_set_and_get_vallo16(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo16_tbl_table_modify_with_set_and_get_vallo16_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo16_tbl_table_modify_with_reset_and_get_vallo16(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo16_tbl_table_modify_with_reset_and_get_vallo16_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo16_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo16_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo1_tbl_table_modify_with_get_vallo1(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo1_tbl_table_modify_with_get_vallo1_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo1_tbl_table_modify_with_set_and_get_vallo1(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo1_tbl_table_modify_with_set_and_get_vallo1_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo1_tbl_table_modify_with_reset_and_get_vallo1(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo1_tbl_table_modify_with_reset_and_get_vallo1_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo1_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo1_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo2_tbl_table_modify_with_get_vallo2(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo2_tbl_table_modify_with_get_vallo2_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo2_tbl_table_modify_with_set_and_get_vallo2(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo2_tbl_table_modify_with_set_and_get_vallo2_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo2_tbl_table_modify_with_reset_and_get_vallo2(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo2_tbl_table_modify_with_reset_and_get_vallo2_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo2_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo2_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo3_tbl_table_modify_with_get_vallo3(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo3_tbl_table_modify_with_get_vallo3_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo3_tbl_table_modify_with_set_and_get_vallo3(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo3_tbl_table_modify_with_set_and_get_vallo3_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo3_tbl_table_modify_with_reset_and_get_vallo3(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo3_tbl_table_modify_with_reset_and_get_vallo3_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo3_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo3_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo4_tbl_table_modify_with_get_vallo4(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo4_tbl_table_modify_with_get_vallo4_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo4_tbl_table_modify_with_set_and_get_vallo4(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo4_tbl_table_modify_with_set_and_get_vallo4_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo4_tbl_table_modify_with_reset_and_get_vallo4(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo4_tbl_table_modify_with_reset_and_get_vallo4_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo4_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo4_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo5_tbl_table_modify_with_get_vallo5(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo5_tbl_table_modify_with_get_vallo5_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo5_tbl_table_modify_with_set_and_get_vallo5(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo5_tbl_table_modify_with_set_and_get_vallo5_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo5_tbl_table_modify_with_reset_and_get_vallo5(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo5_tbl_table_modify_with_reset_and_get_vallo5_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo5_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo5_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo6_tbl_table_modify_with_get_vallo6(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo6_tbl_table_modify_with_get_vallo6_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo6_tbl_table_modify_with_set_and_get_vallo6(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo6_tbl_table_modify_with_set_and_get_vallo6_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo6_tbl_table_modify_with_reset_and_get_vallo6(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo6_tbl_table_modify_with_reset_and_get_vallo6_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo6_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo6_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo7_tbl_table_modify_with_get_vallo7(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo7_tbl_table_modify_with_get_vallo7_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo7_tbl_table_modify_with_set_and_get_vallo7(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo7_tbl_table_modify_with_set_and_get_vallo7_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo7_tbl_table_modify_with_reset_and_get_vallo7(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo7_tbl_table_modify_with_reset_and_get_vallo7_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo7_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo7_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo8_tbl_table_modify_with_get_vallo8(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo8_tbl_table_modify_with_get_vallo8_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo8_tbl_table_modify_with_set_and_get_vallo8(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo8_tbl_table_modify_with_set_and_get_vallo8_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo8_tbl_table_modify_with_reset_and_get_vallo8(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo8_tbl_table_modify_with_reset_and_get_vallo8_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo8_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo8_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo9_tbl_table_modify_with_get_vallo9(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo9_tbl_table_modify_with_get_vallo9_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo9_tbl_table_modify_with_set_and_get_vallo9(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo9_tbl_table_modify_with_set_and_get_vallo9_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo9_tbl_table_modify_with_reset_and_get_vallo9(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo9_tbl_table_modify_with_reset_and_get_vallo9_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
    void update_vallo9_tbl_table_modify_with_nop(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo9_tbl_table_modify_with_nop_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),

    # Table entry delete functions
# //::   if action_table_hdl: continue
    void access_cache_frequency_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cache_frequency_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cache_frequency_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_case1_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_case1_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_case1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_cm1_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cm1_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_cm2_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cm2_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_cm3_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cm3_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_cm4_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_cm4_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_cm4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_deleted_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_deleted_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_deleted_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_latest_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_latest_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_latest_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_savedseq_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_savedseq_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_savedseq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_seq_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_seq_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_seq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_validvalue_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void access_validvalue_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_access_validvalue_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void add_and_remove_value_header_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void add_and_remove_value_header_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_add_and_remove_value_header_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void cache_lookup_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void cache_lookup_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_cache_lookup_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void drop_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void drop_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_drop_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void eg_port_forward_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void eg_port_forward_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_eg_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_cm1_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_cm1_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_cm2_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_cm2_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_cm3_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_cm3_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_cm4_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_cm4_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_cm4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_partition_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_partition_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_partition_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_seq_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_for_seq_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_for_seq_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_partition_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void hash_partition_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_hash_partition_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void ig_port_forward_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void ig_port_forward_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ig_port_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void ipv4_forward_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void ipv4_forward_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_ipv4_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void is_hot_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void is_hot_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_is_hot_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void l2l3_forward_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void l2l3_forward_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_l2l3_forward_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void lastclone_lastscansplit_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void lastclone_lastscansplit_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_lastclone_lastscansplit_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void need_recirculate_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void need_recirculate_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_need_recirculate_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void prepare_for_cachehit_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void prepare_for_cachehit_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_prepare_for_cachehit_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void recirculate_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void recirculate_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_recirculate_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void sample_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void sample_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_sample_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void save_client_udpport_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void save_client_udpport_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_save_client_udpport_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
# //::   if action_table_hdl: continue
    void snapshot_flag_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void snapshot_flag_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_snapshot_flag_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_ipmac_srcport_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_ipmac_srcport_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_ipmac_srcport_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_pktlen_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_pktlen_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_pktlen_tbl_match_spec_t match_spec, 4:i32 priority) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi10_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi10_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi11_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi11_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi12_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi12_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi13_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi13_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi14_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi14_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi15_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi15_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi16_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi16_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi1_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi1_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi2_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi2_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi3_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi3_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi4_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi4_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi5_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi5_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi6_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi6_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi7_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi7_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi8_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi8_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi9_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_valhi9_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_valhi9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallen_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallen_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallen_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo10_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo10_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo10_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo11_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo11_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo11_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo12_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo12_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo12_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo13_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo13_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo13_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo14_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo14_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo14_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo15_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo15_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo15_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo16_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo16_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo16_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo1_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo1_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo1_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo2_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo2_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo2_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo3_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo3_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo3_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo4_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo4_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo4_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo5_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo5_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo5_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo6_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo6_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo6_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo7_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo7_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo7_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo8_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo8_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo8_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo9_tbl_table_delete(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry) throws (1:InvalidTableOperation ouch),
    void update_vallo9_tbl_table_delete_by_match_spec(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_update_vallo9_tbl_match_spec_t match_spec) throws (1:InvalidTableOperation ouch),

    # Table default entry get functions
    EntryHandle_t access_cache_frequency_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_access_cache_frequency_tbl_entry_desc_t access_cache_frequency_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_case1_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_access_case1_tbl_entry_desc_t access_case1_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm1_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_access_cm1_tbl_entry_desc_t access_cm1_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm2_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_access_cm2_tbl_entry_desc_t access_cm2_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm3_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_access_cm3_tbl_entry_desc_t access_cm3_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm4_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_access_cm4_tbl_entry_desc_t access_cm4_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_deleted_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_access_deleted_tbl_entry_desc_t access_deleted_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_latest_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_access_latest_tbl_entry_desc_t access_latest_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_savedseq_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_access_savedseq_tbl_entry_desc_t access_savedseq_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_seq_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_access_seq_tbl_entry_desc_t access_seq_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_validvalue_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_access_validvalue_tbl_entry_desc_t access_validvalue_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_add_and_remove_value_header_tbl_entry_desc_t add_and_remove_value_header_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t cache_lookup_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_cache_lookup_tbl_entry_desc_t cache_lookup_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t drop_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_drop_tbl_entry_desc_t drop_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_eg_port_forward_tbl_entry_desc_t eg_port_forward_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm1_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_hash_for_cm1_tbl_entry_desc_t hash_for_cm1_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm2_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_hash_for_cm2_tbl_entry_desc_t hash_for_cm2_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm3_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_hash_for_cm3_tbl_entry_desc_t hash_for_cm3_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm4_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_hash_for_cm4_tbl_entry_desc_t hash_for_cm4_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_partition_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_hash_for_partition_tbl_entry_desc_t hash_for_partition_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_seq_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_hash_for_seq_tbl_entry_desc_t hash_for_seq_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_partition_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_hash_partition_tbl_entry_desc_t hash_partition_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ig_port_forward_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_ig_port_forward_tbl_entry_desc_t ig_port_forward_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ipv4_forward_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_ipv4_forward_tbl_entry_desc_t ipv4_forward_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t is_hot_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_is_hot_tbl_entry_desc_t is_hot_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t l2l3_forward_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_l2l3_forward_tbl_entry_desc_t l2l3_forward_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t lastclone_lastscansplit_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_lastclone_lastscansplit_tbl_entry_desc_t lastclone_lastscansplit_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t need_recirculate_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_need_recirculate_tbl_entry_desc_t need_recirculate_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t prepare_for_cachehit_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_prepare_for_cachehit_tbl_entry_desc_t prepare_for_cachehit_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t recirculate_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_recirculate_tbl_entry_desc_t recirculate_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t sample_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_sample_tbl_entry_desc_t sample_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t save_client_udpport_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_save_client_udpport_tbl_entry_desc_t save_client_udpport_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t set_hot_threshold_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_set_hot_threshold_tbl_entry_desc_t set_hot_threshold_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t snapshot_flag_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_snapshot_flag_tbl_entry_desc_t snapshot_flag_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_ipmac_srcport_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_ipmac_srcport_tbl_entry_desc_t update_ipmac_srcport_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_pktlen_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_pktlen_tbl_entry_desc_t update_pktlen_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi10_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi10_tbl_entry_desc_t update_valhi10_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi11_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi11_tbl_entry_desc_t update_valhi11_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi12_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi12_tbl_entry_desc_t update_valhi12_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi13_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi13_tbl_entry_desc_t update_valhi13_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi14_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi14_tbl_entry_desc_t update_valhi14_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi15_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi15_tbl_entry_desc_t update_valhi15_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi16_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi16_tbl_entry_desc_t update_valhi16_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi1_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi1_tbl_entry_desc_t update_valhi1_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi2_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi2_tbl_entry_desc_t update_valhi2_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi3_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi3_tbl_entry_desc_t update_valhi3_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi4_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi4_tbl_entry_desc_t update_valhi4_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi5_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi5_tbl_entry_desc_t update_valhi5_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi6_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi6_tbl_entry_desc_t update_valhi6_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi7_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi7_tbl_entry_desc_t update_valhi7_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi8_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi8_tbl_entry_desc_t update_valhi8_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi9_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_valhi9_tbl_entry_desc_t update_valhi9_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallen_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallen_tbl_entry_desc_t update_vallen_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo10_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo10_tbl_entry_desc_t update_vallo10_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo11_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo11_tbl_entry_desc_t update_vallo11_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo12_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo12_tbl_entry_desc_t update_vallo12_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo13_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo13_tbl_entry_desc_t update_vallo13_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo14_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo14_tbl_entry_desc_t update_vallo14_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo15_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo15_tbl_entry_desc_t update_vallo15_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo16_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo16_tbl_entry_desc_t update_vallo16_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo1_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo1_tbl_entry_desc_t update_vallo1_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo2_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo2_tbl_entry_desc_t update_vallo2_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo3_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo3_tbl_entry_desc_t update_vallo3_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo4_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo4_tbl_entry_desc_t update_vallo4_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo5_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo5_tbl_entry_desc_t update_vallo5_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo6_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo6_tbl_entry_desc_t update_vallo6_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo7_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo7_tbl_entry_desc_t update_vallo7_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo8_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo8_tbl_entry_desc_t update_vallo8_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo9_tbl_table_get_default_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    netbufferv4_update_vallo9_tbl_entry_desc_t update_vallo9_tbl_table_get_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    # Table default entry clear functions
# //::   if action_table_hdl: continue
    void access_cache_frequency_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_case1_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_cm1_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_cm2_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_cm3_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_cm4_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_deleted_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_latest_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_savedseq_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_seq_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_validvalue_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void add_and_remove_value_header_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void cache_lookup_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void drop_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void eg_port_forward_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_cm1_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_cm2_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_cm3_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_cm4_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_partition_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_seq_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_partition_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void ig_port_forward_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void ipv4_forward_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void is_hot_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void l2l3_forward_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void lastclone_lastscansplit_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void need_recirculate_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void prepare_for_cachehit_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void recirculate_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void sample_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void save_client_udpport_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void set_hot_threshold_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void snapshot_flag_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_ipmac_srcport_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_pktlen_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi10_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi11_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi12_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi13_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi14_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi15_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi16_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi1_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi2_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi3_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi4_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi5_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi6_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi7_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi8_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi9_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallen_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo10_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo11_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo12_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo13_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo14_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo15_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo16_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo1_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo2_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo3_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo4_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo5_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo6_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo7_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo8_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo9_tbl_table_reset_default_entry(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    i32 access_cache_frequency_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 access_case1_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 access_cm1_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 access_cm2_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 access_cm3_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 access_cm4_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 access_deleted_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 access_latest_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 access_savedseq_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 access_seq_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 access_validvalue_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 add_and_remove_value_header_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 cache_lookup_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 drop_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 eg_port_forward_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 hash_for_cm1_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 hash_for_cm2_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 hash_for_cm3_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 hash_for_cm4_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 hash_for_partition_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 hash_for_seq_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 hash_partition_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 ig_port_forward_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 ipv4_forward_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 is_hot_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 l2l3_forward_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 lastclone_lastscansplit_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 need_recirculate_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 prepare_for_cachehit_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 recirculate_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 sample_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 save_client_udpport_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 set_hot_threshold_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 snapshot_flag_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_ipmac_srcport_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_pktlen_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi10_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi11_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi12_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi13_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi14_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi15_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi16_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi1_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi2_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi3_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi4_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi5_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi6_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi7_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi8_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_valhi9_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallen_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo10_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo11_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo12_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo13_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo14_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo15_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo16_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo1_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo2_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo3_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo4_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo5_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo6_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo7_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo8_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    i32 update_vallo9_tbl_get_entry_count(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),


    # Get first entry handle functions
    i32 access_cache_frequency_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> access_cache_frequency_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_access_cache_frequency_tbl_entry_desc_t access_cache_frequency_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 access_case1_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> access_case1_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_access_case1_tbl_entry_desc_t access_case1_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 access_cm1_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> access_cm1_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_access_cm1_tbl_entry_desc_t access_cm1_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 access_cm2_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> access_cm2_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_access_cm2_tbl_entry_desc_t access_cm2_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 access_cm3_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> access_cm3_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_access_cm3_tbl_entry_desc_t access_cm3_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 access_cm4_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> access_cm4_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_access_cm4_tbl_entry_desc_t access_cm4_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 access_deleted_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> access_deleted_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_access_deleted_tbl_entry_desc_t access_deleted_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 access_latest_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> access_latest_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_access_latest_tbl_entry_desc_t access_latest_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 access_savedseq_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> access_savedseq_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_access_savedseq_tbl_entry_desc_t access_savedseq_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 access_seq_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> access_seq_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_access_seq_tbl_entry_desc_t access_seq_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 access_validvalue_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> access_validvalue_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_access_validvalue_tbl_entry_desc_t access_validvalue_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 add_and_remove_value_header_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> add_and_remove_value_header_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_add_and_remove_value_header_tbl_entry_desc_t add_and_remove_value_header_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 cache_lookup_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> cache_lookup_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_cache_lookup_tbl_entry_desc_t cache_lookup_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 drop_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> drop_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_drop_tbl_entry_desc_t drop_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 eg_port_forward_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> eg_port_forward_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_eg_port_forward_tbl_entry_desc_t eg_port_forward_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 hash_for_cm1_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> hash_for_cm1_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_hash_for_cm1_tbl_entry_desc_t hash_for_cm1_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 hash_for_cm2_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> hash_for_cm2_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_hash_for_cm2_tbl_entry_desc_t hash_for_cm2_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 hash_for_cm3_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> hash_for_cm3_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_hash_for_cm3_tbl_entry_desc_t hash_for_cm3_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 hash_for_cm4_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> hash_for_cm4_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_hash_for_cm4_tbl_entry_desc_t hash_for_cm4_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 hash_for_partition_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> hash_for_partition_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_hash_for_partition_tbl_entry_desc_t hash_for_partition_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 hash_for_seq_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> hash_for_seq_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_hash_for_seq_tbl_entry_desc_t hash_for_seq_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 hash_partition_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> hash_partition_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_hash_partition_tbl_entry_desc_t hash_partition_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 ig_port_forward_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> ig_port_forward_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_ig_port_forward_tbl_entry_desc_t ig_port_forward_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 ipv4_forward_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> ipv4_forward_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_ipv4_forward_tbl_entry_desc_t ipv4_forward_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 is_hot_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> is_hot_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_is_hot_tbl_entry_desc_t is_hot_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 l2l3_forward_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> l2l3_forward_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_l2l3_forward_tbl_entry_desc_t l2l3_forward_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 lastclone_lastscansplit_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> lastclone_lastscansplit_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_lastclone_lastscansplit_tbl_entry_desc_t lastclone_lastscansplit_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 need_recirculate_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> need_recirculate_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_need_recirculate_tbl_entry_desc_t need_recirculate_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 prepare_for_cachehit_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> prepare_for_cachehit_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_prepare_for_cachehit_tbl_entry_desc_t prepare_for_cachehit_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 recirculate_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> recirculate_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_recirculate_tbl_entry_desc_t recirculate_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 sample_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> sample_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_sample_tbl_entry_desc_t sample_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 save_client_udpport_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> save_client_udpport_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_save_client_udpport_tbl_entry_desc_t save_client_udpport_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 set_hot_threshold_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> set_hot_threshold_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_set_hot_threshold_tbl_entry_desc_t set_hot_threshold_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 snapshot_flag_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> snapshot_flag_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_snapshot_flag_tbl_entry_desc_t snapshot_flag_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_ipmac_srcport_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_ipmac_srcport_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_ipmac_srcport_tbl_entry_desc_t update_ipmac_srcport_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_pktlen_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_pktlen_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_pktlen_tbl_entry_desc_t update_pktlen_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi10_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi10_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi10_tbl_entry_desc_t update_valhi10_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi11_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi11_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi11_tbl_entry_desc_t update_valhi11_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi12_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi12_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi12_tbl_entry_desc_t update_valhi12_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi13_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi13_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi13_tbl_entry_desc_t update_valhi13_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi14_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi14_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi14_tbl_entry_desc_t update_valhi14_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi15_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi15_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi15_tbl_entry_desc_t update_valhi15_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi16_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi16_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi16_tbl_entry_desc_t update_valhi16_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi1_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi1_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi1_tbl_entry_desc_t update_valhi1_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi2_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi2_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi2_tbl_entry_desc_t update_valhi2_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi3_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi3_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi3_tbl_entry_desc_t update_valhi3_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi4_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi4_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi4_tbl_entry_desc_t update_valhi4_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi5_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi5_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi5_tbl_entry_desc_t update_valhi5_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi6_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi6_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi6_tbl_entry_desc_t update_valhi6_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi7_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi7_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi7_tbl_entry_desc_t update_valhi7_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi8_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi8_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi8_tbl_entry_desc_t update_valhi8_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_valhi9_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_valhi9_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_valhi9_tbl_entry_desc_t update_valhi9_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallen_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallen_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallen_tbl_entry_desc_t update_vallen_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo10_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo10_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo10_tbl_entry_desc_t update_vallo10_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo11_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo11_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo11_tbl_entry_desc_t update_vallo11_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo12_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo12_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo12_tbl_entry_desc_t update_vallo12_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo13_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo13_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo13_tbl_entry_desc_t update_vallo13_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo14_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo14_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo14_tbl_entry_desc_t update_vallo14_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo15_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo15_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo15_tbl_entry_desc_t update_vallo15_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo16_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo16_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo16_tbl_entry_desc_t update_vallo16_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo1_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo1_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo1_tbl_entry_desc_t update_vallo1_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo2_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo2_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo2_tbl_entry_desc_t update_vallo2_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo3_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo3_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo3_tbl_entry_desc_t update_vallo3_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo4_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo4_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo4_tbl_entry_desc_t update_vallo4_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo5_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo5_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo5_tbl_entry_desc_t update_vallo5_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo6_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo6_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo6_tbl_entry_desc_t update_vallo6_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo7_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo7_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo7_tbl_entry_desc_t update_vallo7_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo8_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo8_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo8_tbl_entry_desc_t update_vallo8_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),

    i32 update_vallo9_tbl_get_first_entry_handle(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    list<i32> update_vallo9_tbl_get_next_entry_handles(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:EntryHandle_t entry_hdl, 4:i32 n) throws (1:InvalidTableOperation ouch),

    netbufferv4_update_vallo9_tbl_entry_desc_t update_vallo9_tbl_get_entry(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:EntryHandle_t entry_hdl, 4:bool read_from_hw) throws (1:InvalidTableOperation ouch),



    # Table set default action functions

    EntryHandle_t access_cache_frequency_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_case1_tbl_set_default_action_reset_is_case1(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm1_tbl_set_default_action_initialize_cm1_predicate(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm2_tbl_set_default_action_initialize_cm2_predicate(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm3_tbl_set_default_action_initialize_cm3_predicate(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_cm4_tbl_set_default_action_initialize_cm4_predicate(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_deleted_tbl_set_default_action_reset_is_deleted(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_latest_tbl_set_default_action_reset_is_latest(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_savedseq_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_seq_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t access_validvalue_tbl_set_default_action_reset_meta_validvalue(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t add_and_remove_value_header_tbl_set_default_action_remove_all(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t cache_lookup_tbl_set_default_action_uncached_action(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t drop_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t eg_port_forward_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm1_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm2_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm3_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_cm4_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_partition_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_for_seq_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t hash_partition_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ig_port_forward_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t ipv4_forward_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t is_hot_tbl_set_default_action_reset_is_hot(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t l2l3_forward_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t lastclone_lastscansplit_tbl_set_default_action_reset_is_lastclone_lastscansplit(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t need_recirculate_tbl_set_default_action_reset_need_recirculate(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t prepare_for_cachehit_tbl_set_default_action_set_client_sid(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_set_client_sid_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t recirculate_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t sample_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t save_client_udpport_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t set_hot_threshold_tbl_set_default_action_set_hot_threshold(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:netbufferv4_set_hot_threshold_action_spec_t action_spec) throws (1:InvalidTableOperation ouch),
    EntryHandle_t snapshot_flag_tbl_set_default_action_reset_snapshot_flag(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_ipmac_srcport_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_pktlen_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi10_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi11_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi12_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi13_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi14_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi15_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi16_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi1_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi2_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi3_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi4_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi5_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi6_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi7_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi8_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_valhi9_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallen_tbl_set_default_action_reset_access_val_mode(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo10_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo11_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo12_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo13_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo14_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo15_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo16_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo1_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo2_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo3_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo4_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo5_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo6_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo7_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo8_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),
    EntryHandle_t update_vallo9_tbl_set_default_action_nop(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidTableOperation ouch),

    
     # Table set/get property
# //::   if action_table_hdl: continue
    void access_cache_frequency_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t access_cache_frequency_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_case1_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t access_case1_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_cm1_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t access_cm1_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_cm2_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t access_cm2_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_cm3_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t access_cm3_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_cm4_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t access_cm4_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_deleted_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t access_deleted_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_latest_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t access_latest_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_savedseq_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t access_savedseq_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_seq_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t access_seq_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void access_validvalue_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t access_validvalue_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void add_and_remove_value_header_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t add_and_remove_value_header_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void cache_lookup_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t cache_lookup_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void drop_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t drop_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void eg_port_forward_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t eg_port_forward_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_cm1_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t hash_for_cm1_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_cm2_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t hash_for_cm2_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_cm3_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t hash_for_cm3_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_cm4_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t hash_for_cm4_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_partition_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t hash_for_partition_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_for_seq_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t hash_for_seq_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void hash_partition_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t hash_partition_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void ig_port_forward_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t ig_port_forward_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void ipv4_forward_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t ipv4_forward_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void is_hot_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t is_hot_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void l2l3_forward_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t l2l3_forward_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void lastclone_lastscansplit_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t lastclone_lastscansplit_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void need_recirculate_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t need_recirculate_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void prepare_for_cachehit_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t prepare_for_cachehit_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void recirculate_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t recirculate_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void sample_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t sample_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void save_client_udpport_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t save_client_udpport_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void set_hot_threshold_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t set_hot_threshold_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void snapshot_flag_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t snapshot_flag_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_ipmac_srcport_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_ipmac_srcport_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_pktlen_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_pktlen_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi10_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi10_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi11_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi11_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi12_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi12_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi13_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi13_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi14_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi14_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi15_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi15_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi16_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi16_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi1_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi1_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi2_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi2_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi3_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi3_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi4_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi4_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi5_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi5_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi6_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi6_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi7_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi7_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi8_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi8_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_valhi9_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_valhi9_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallen_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallen_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo10_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo10_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo11_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo11_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo12_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo12_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo13_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo13_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo14_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo14_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo15_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo15_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo16_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo16_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo1_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo1_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo2_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo2_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo3_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo3_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo4_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo4_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo5_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo5_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo6_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo6_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo7_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo7_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo8_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo8_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),
# //::   if action_table_hdl: continue
    void update_vallo9_tbl_set_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property, 4:tbl_property_value_t value, 5:i32 prop_args) throws (1:InvalidTableOperation ouch),

    tbl_property_value_args_t update_vallo9_tbl_get_property(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:tbl_property_t property) throws (1:InvalidTableOperation ouch),

    # INDIRECT ACTION DATA AND MATCH SELECT






    void set_learning_timeout(1:res.SessionHandle_t sess_hdl, 2:byte dev_id, 3:i32 usecs) throws (1:InvalidLearnOperation ouch),

    void tbl_dbg_counter_type_set(1:res.DevTarget_t dev_tgt, 2:string tbl_name, 3:tbl_dbg_counter_type_t type) throws (1:InvalidDbgOperation ouch),

    TblCntrInfo_t tbl_dbg_counter_get(1:res.DevTarget_t dev_tgt, 2:string tbl_name) throws (1:InvalidDbgOperation ouch),

    void tbl_dbg_counter_clear(1:res.DevTarget_t dev_tgt, 2:string tbl_name) throws (1:InvalidDbgOperation ouch),

    void tbl_dbg_counter_type_stage_set(1:res.DevTarget_t dev_tgt, 2:byte stage, 3:tbl_dbg_counter_type_t type) throws (1:InvalidDbgOperation ouch),

    TblDbgStageInfo_t tbl_dbg_counter_stage_get(1:res.DevTarget_t dev_tgt, 2:byte stage) throws (1:InvalidDbgOperation ouch),

    void tbl_dbg_counter_stage_clear(1:res.DevTarget_t dev_tgt, 2: byte stage) throws (1:InvalidDbgOperation ouch),

    SnapshotHandle_t snapshot_create(1:res.DevTarget_t dev_tgt, 2:byte start_stage, 3:byte end_stage, 4:byte direction) throws (1:InvalidSnapshotOperation ouch),

    void snapshot_delete(1:SnapshotHandle_t handle) throws (1:InvalidSnapshotOperation ouch),

    void snapshot_state_set(1: SnapshotHandle_t handle, 2:i32 state, 3:i32 usecs) throws (1:InvalidSnapshotOperation ouch),

    i32 snapshot_state_get(1:SnapshotHandle_t handle, 2:i16 pipe) throws (1:InvalidSnapshotOperation ouch),

    void snapshot_timer_enable(1: SnapshotHandle_t handle, 2:byte disable) throws (1:InvalidSnapshotOperation ouch),

    void snapshot_capture_trigger_set(1: SnapshotHandle_t handle,
                2:netbufferv4_snapshot_trig_spec_t trig_spec,
                3:netbufferv4_snapshot_trig_spec_t trig_spec2) throws (1:InvalidSnapshotOperation ouch),

    i64 snapshot_capture_data_get(1: SnapshotHandle_t handle, 2:i16 pipe, 3:i16 stage_id, 4:string field_name) throws (1:InvalidSnapshotOperation ouch),

    netbufferv4_snapshot_tbl_data_t snapshot_capture_tbl_data_get(1: SnapshotHandle_t handle, 2:i16 pipe, 3:string table_name) throws (1:InvalidSnapshotOperation ouch),

    void snapshot_capture_trigger_fields_clr(1:SnapshotHandle_t handle) throws (1:InvalidSnapshotOperation ouch),

    bool snapshot_field_in_scope(1:res.DevTarget_t dev_tgt, 2:byte stage,
                 3:byte direction, 4:string field_name) throws (1:InvalidSnapshotOperation ouch),

    # counters



    # registers

    void register_hw_sync_cm4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i16> register_read_cm4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_cm4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i16 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_cm4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_cm4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_cm4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i16 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i16> register_range_read_cm4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo14_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_cm3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i16> register_read_cm3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_cm3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i16 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_cm3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_cm3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_cm3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i16 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i16> register_range_read_cm3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi9_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_cache_frequency_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_cache_frequency_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_cache_frequency_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_cache_frequency_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_cache_frequency_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_cache_frequency_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_cache_frequency_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_cm2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i16> register_read_cm2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_cm2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i16 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_cm2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_cm2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_cm2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i16 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i16> register_range_read_cm2_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_deleted_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<byte> register_read_deleted_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_deleted_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:byte register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_deleted_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_deleted_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_deleted_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:byte register_value) throws (1:InvalidRegisterOperation ouch),
    list<byte> register_range_read_deleted_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo13_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo7_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo6_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_seq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_seq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_seq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_seq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_seq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_seq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_seq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo11_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo5_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_cm1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i16> register_read_cm1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_cm1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i16 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_cm1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_cm1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_cm1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i16 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i16> register_range_read_cm1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_validvalue_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<byte> register_read_validvalue_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_validvalue_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:byte register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_validvalue_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_validvalue_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_validvalue_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:byte register_value) throws (1:InvalidRegisterOperation ouch),
    list<byte> register_range_read_validvalue_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi12_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_latest_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<byte> register_read_latest_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_latest_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:byte register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_latest_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_latest_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_latest_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:byte register_value) throws (1:InvalidRegisterOperation ouch),
    list<byte> register_range_read_latest_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo10_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo4_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo16_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo3_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi8_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallen_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i16> register_read_vallen_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallen_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i16 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallen_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallen_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallen_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i16 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i16> register_range_read_vallen_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_vallo1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_vallo1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_vallo1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_vallo1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_vallo1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_vallo1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_vallo1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_savedseq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_savedseq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_savedseq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_savedseq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_savedseq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_savedseq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_savedseq_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_valhi15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_read_valhi15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_valhi15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_valhi15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_valhi15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_valhi15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 register_value) throws (1:InvalidRegisterOperation ouch),
    list<i32> register_range_read_valhi15_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_hw_sync_case1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    list<byte> register_read_case1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),
    void register_write_case1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:byte register_value) throws (1:InvalidRegisterOperation ouch),
    void register_reset_all_case1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt) throws (1:InvalidRegisterOperation ouch),
    void register_range_reset_case1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt 3:i32 index 4:i32 count) throws (1:InvalidRegisterOperation ouch),
    void register_write_all_case1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:byte register_value) throws (1:InvalidRegisterOperation ouch),
    list<byte> register_range_read_case1_reg(1:res.SessionHandle_t sess_hdl, 2:res.DevTarget_t dev_tgt, 3:i32 index, 4:i32 count, 5:netbufferv4_register_flags_t flags) throws (1:InvalidRegisterOperation ouch),








} 
