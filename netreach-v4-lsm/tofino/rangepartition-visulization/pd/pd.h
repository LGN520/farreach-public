#ifndef _PD_NETBUFFERV4_PD_H
#define _PD_NETBUFFERV4_PD_H

#include <stdint.h>

#include <tofino/pdfixed/pd_common.h>
#include <pipe_mgr/pipe_mgr_intf.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN_
#define LITTLE_ENDIAN_CALLER 1
#endif


/* MATCH STRUCTS */

typedef struct p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t inswitch_hdr_is_sampled;
  uint8_t inswitch_hdr_is_cached;
} p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_access_case1_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t inswitch_hdr_is_cached;
  uint8_t validvalue_hdr_validvalue;
  uint8_t meta_is_latest;
  uint8_t inswitch_hdr_snapshot_flag;
} p4_pd_netbufferv4_access_case1_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_access_cm1_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t inswitch_hdr_is_sampled;
  uint8_t inswitch_hdr_is_cached;
} p4_pd_netbufferv4_access_cm1_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_access_cm2_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t inswitch_hdr_is_sampled;
  uint8_t inswitch_hdr_is_cached;
} p4_pd_netbufferv4_access_cm2_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_access_cm3_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t inswitch_hdr_is_sampled;
  uint8_t inswitch_hdr_is_cached;
} p4_pd_netbufferv4_access_cm3_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_access_cm4_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t inswitch_hdr_is_sampled;
  uint8_t inswitch_hdr_is_cached;
} p4_pd_netbufferv4_access_cm4_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_access_deleted_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t inswitch_hdr_is_cached;
  uint8_t validvalue_hdr_validvalue;
  uint8_t meta_is_latest;
} p4_pd_netbufferv4_access_deleted_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_access_latest_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t inswitch_hdr_is_cached;
  uint8_t validvalue_hdr_validvalue;
} p4_pd_netbufferv4_access_latest_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_access_savedseq_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t inswitch_hdr_is_cached;
  uint8_t validvalue_hdr_validvalue;
  uint8_t meta_is_latest;
} p4_pd_netbufferv4_access_savedseq_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_access_seq_tbl_match_spec {
  uint16_t op_hdr_optype;
} p4_pd_netbufferv4_access_seq_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_access_validvalue_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t inswitch_hdr_is_cached;
} p4_pd_netbufferv4_access_validvalue_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint16_t vallen_hdr_vallen_start;
  uint16_t vallen_hdr_vallen_end;
} p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_cache_lookup_tbl_match_spec {
  uint32_t op_hdr_keylolo;
  uint32_t op_hdr_keylohi;
  uint32_t op_hdr_keyhilo;
  uint16_t op_hdr_keyhihilo;
  uint16_t op_hdr_keyhihihi;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_cache_lookup_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_drop_tbl_match_spec {
  uint16_t op_hdr_optype;
} p4_pd_netbufferv4_drop_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_eg_port_forward_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t inswitch_hdr_is_cached;
  uint8_t meta_is_hot;
  uint8_t validvalue_hdr_validvalue;
  uint8_t meta_is_latest;
  uint8_t meta_is_deleted;
  uint16_t inswitch_hdr_client_sid;
  uint8_t meta_is_lastclone_for_pktloss;
  uint8_t inswitch_hdr_snapshot_flag;
  uint8_t meta_is_case1;
  uint8_t meta_is_last_scansplit;
  uint16_t meta_server_sid;
} p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_hash_for_cm1_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_hash_for_cm1_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_hash_for_cm2_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_hash_for_cm2_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_hash_for_cm3_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_hash_for_cm3_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_hash_for_cm4_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_hash_for_cm4_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_hash_for_seq_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_hash_for_seq_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_ig_port_forward_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_ipv4_forward_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint32_t ipv4_hdr_dstAddr;
  uint16_t ipv4_hdr_dstAddr_prefix_length;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_ipv4_forward_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_is_hot_tbl_match_spec {
  uint8_t meta_cm1_predicate;
  uint8_t meta_cm2_predicate;
  uint8_t meta_cm3_predicate;
  uint8_t meta_cm4_predicate;
} p4_pd_netbufferv4_is_hot_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_lastclone_lastscansplit_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t clone_hdr_clonenum_for_pktloss;
  uint16_t meta_remain_scannum;
} p4_pd_netbufferv4_lastclone_lastscansplit_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_need_recirculate_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint16_t ig_intr_md_ingress_port;
} p4_pd_netbufferv4_need_recirculate_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_prepare_for_cachehit_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint16_t ig_intr_md_ingress_port;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_prepare_for_cachehit_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_process_scanreq_split_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint16_t udp_hdr_dstPort;
  uint8_t split_hdr_is_clone;
} p4_pd_netbufferv4_process_scanreq_split_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint16_t scan_hdr_keyhihihi_start;
  uint16_t scan_hdr_keyhihihi_end;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_range_partition_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint16_t op_hdr_keyhihihi_start;
  uint16_t op_hdr_keyhihihi_end;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_range_partition_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_recirculate_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_recirculate_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_sample_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_sample_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_save_client_udpport_tbl_match_spec {
  uint16_t op_hdr_optype;
} p4_pd_netbufferv4_save_client_udpport_tbl_match_spec_t;

/* set_hot_threshold_tbl has no match fields */

typedef struct p4_pd_netbufferv4_snapshot_flag_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t meta_need_recirculate;
} p4_pd_netbufferv4_snapshot_flag_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint16_t eg_intr_md_egress_port;
} p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_pktlen_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint16_t vallen_hdr_vallen_start;
  uint16_t vallen_hdr_vallen_end;
} p4_pd_netbufferv4_update_pktlen_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi10_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi10_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi11_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi11_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi12_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi12_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi13_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi13_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi14_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi14_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi15_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi15_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi16_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi16_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi1_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi1_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi2_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi2_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi3_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi3_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi4_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi4_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi5_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi5_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi6_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi6_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi7_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi7_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi8_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi8_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_valhi9_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_valhi9_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallen_tbl_match_spec {
  uint16_t op_hdr_optype;
  uint8_t inswitch_hdr_is_cached;
  uint8_t validvalue_hdr_validvalue;
  uint8_t meta_is_latest;
} p4_pd_netbufferv4_update_vallen_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo10_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo10_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo11_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo11_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo12_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo12_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo13_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo13_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo14_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo14_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo15_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo15_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo16_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo16_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo1_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo1_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo2_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo2_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo3_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo3_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo4_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo4_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo5_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo5_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo6_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo6_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo7_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo7_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo8_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo8_tbl_match_spec_t;

typedef struct p4_pd_netbufferv4_update_vallo9_tbl_match_spec {
  uint8_t meta_access_val_mode;
} p4_pd_netbufferv4_update_vallo9_tbl_match_spec_t;



/* Dynamic Exm Table Key Mask */

/* access_cache_frequency_tbl has no dynamic key masks */

/* access_case1_tbl has no dynamic key masks */

/* access_cm1_tbl has no dynamic key masks */

/* access_cm2_tbl has no dynamic key masks */

/* access_cm3_tbl has no dynamic key masks */

/* access_cm4_tbl has no dynamic key masks */

/* access_deleted_tbl has no dynamic key masks */

/* access_latest_tbl has no dynamic key masks */

/* access_savedseq_tbl has no dynamic key masks */

/* access_seq_tbl has no dynamic key masks */

/* access_validvalue_tbl has no dynamic key masks */

/* add_and_remove_value_header_tbl has no dynamic key masks */

/* cache_lookup_tbl has no dynamic key masks */

/* drop_tbl has no dynamic key masks */

/* eg_port_forward_tbl has no dynamic key masks */

/* hash_for_cm1_tbl has no dynamic key masks */

/* hash_for_cm2_tbl has no dynamic key masks */

/* hash_for_cm3_tbl has no dynamic key masks */

/* hash_for_cm4_tbl has no dynamic key masks */

/* hash_for_seq_tbl has no dynamic key masks */

/* ig_port_forward_tbl has no dynamic key masks */

/* ipv4_forward_tbl has no dynamic key masks */

/* is_hot_tbl has no dynamic key masks */

/* lastclone_lastscansplit_tbl has no dynamic key masks */

/* need_recirculate_tbl has no dynamic key masks */

/* prepare_for_cachehit_tbl has no dynamic key masks */

/* process_scanreq_split_tbl has no dynamic key masks */

/* range_partition_for_scan_endkey_tbl has no dynamic key masks */

/* range_partition_tbl has no dynamic key masks */

/* recirculate_tbl has no dynamic key masks */

/* sample_tbl has no dynamic key masks */

/* save_client_udpport_tbl has no dynamic key masks */

/* set_hot_threshold_tbl has no match fields */

/* snapshot_flag_tbl has no dynamic key masks */

/* update_ipmac_srcport_tbl has no dynamic key masks */

/* update_pktlen_tbl has no dynamic key masks */

/* update_valhi10_tbl has no dynamic key masks */

/* update_valhi11_tbl has no dynamic key masks */

/* update_valhi12_tbl has no dynamic key masks */

/* update_valhi13_tbl has no dynamic key masks */

/* update_valhi14_tbl has no dynamic key masks */

/* update_valhi15_tbl has no dynamic key masks */

/* update_valhi16_tbl has no dynamic key masks */

/* update_valhi1_tbl has no dynamic key masks */

/* update_valhi2_tbl has no dynamic key masks */

/* update_valhi3_tbl has no dynamic key masks */

/* update_valhi4_tbl has no dynamic key masks */

/* update_valhi5_tbl has no dynamic key masks */

/* update_valhi6_tbl has no dynamic key masks */

/* update_valhi7_tbl has no dynamic key masks */

/* update_valhi8_tbl has no dynamic key masks */

/* update_valhi9_tbl has no dynamic key masks */

/* update_vallen_tbl has no dynamic key masks */

/* update_vallo10_tbl has no dynamic key masks */

/* update_vallo11_tbl has no dynamic key masks */

/* update_vallo12_tbl has no dynamic key masks */

/* update_vallo13_tbl has no dynamic key masks */

/* update_vallo14_tbl has no dynamic key masks */

/* update_vallo15_tbl has no dynamic key masks */

/* update_vallo16_tbl has no dynamic key masks */

/* update_vallo1_tbl has no dynamic key masks */

/* update_vallo2_tbl has no dynamic key masks */

/* update_vallo3_tbl has no dynamic key masks */

/* update_vallo4_tbl has no dynamic key masks */

/* update_vallo5_tbl has no dynamic key masks */

/* update_vallo6_tbl has no dynamic key masks */

/* update_vallo7_tbl has no dynamic key masks */

/* update_vallo8_tbl has no dynamic key masks */

/* update_vallo9_tbl has no dynamic key masks */



/* ACTION STRUCTS */

/* Enum of all action names. */
typedef enum p4_pd_netbufferv4_action_names {
  p4_pd_netbufferv4_get_cache_frequency,
  p4_pd_netbufferv4_update_cache_frequency,
  p4_pd_netbufferv4_reset_cache_frequency,
  p4_pd_netbufferv4_nop,
  p4_pd_netbufferv4_try_case1,
  p4_pd_netbufferv4_read_case1,
  p4_pd_netbufferv4_reset_is_case1,
  p4_pd_netbufferv4_update_cm1,
  p4_pd_netbufferv4_initialize_cm1_predicate,
  p4_pd_netbufferv4_update_cm2,
  p4_pd_netbufferv4_initialize_cm2_predicate,
  p4_pd_netbufferv4_update_cm3,
  p4_pd_netbufferv4_initialize_cm3_predicate,
  p4_pd_netbufferv4_update_cm4,
  p4_pd_netbufferv4_initialize_cm4_predicate,
  p4_pd_netbufferv4_get_deleted,
  p4_pd_netbufferv4_set_and_get_deleted,
  p4_pd_netbufferv4_reset_and_get_deleted,
  p4_pd_netbufferv4_reset_is_deleted,
  p4_pd_netbufferv4_get_latest,
  p4_pd_netbufferv4_set_and_get_latest,
  p4_pd_netbufferv4_reset_and_get_latest,
  p4_pd_netbufferv4_reset_is_latest,
  p4_pd_netbufferv4_get_savedseq,
  p4_pd_netbufferv4_set_and_get_savedseq,
  p4_pd_netbufferv4_assign_seq,
  p4_pd_netbufferv4_get_validvalue,
  p4_pd_netbufferv4_set_validvalue,
  p4_pd_netbufferv4_reset_meta_validvalue,
  p4_pd_netbufferv4_add_only_vallen,
  p4_pd_netbufferv4_add_to_val1,
  p4_pd_netbufferv4_add_to_val2,
  p4_pd_netbufferv4_add_to_val3,
  p4_pd_netbufferv4_add_to_val4,
  p4_pd_netbufferv4_add_to_val5,
  p4_pd_netbufferv4_add_to_val6,
  p4_pd_netbufferv4_add_to_val7,
  p4_pd_netbufferv4_add_to_val8,
  p4_pd_netbufferv4_add_to_val9,
  p4_pd_netbufferv4_add_to_val10,
  p4_pd_netbufferv4_add_to_val11,
  p4_pd_netbufferv4_add_to_val12,
  p4_pd_netbufferv4_add_to_val13,
  p4_pd_netbufferv4_add_to_val14,
  p4_pd_netbufferv4_add_to_val15,
  p4_pd_netbufferv4_add_to_val16,
  p4_pd_netbufferv4_remove_all,
  p4_pd_netbufferv4_cached_action,
  p4_pd_netbufferv4_uncached_action,
  p4_pd_netbufferv4_drop_getres_latest_seq_inswitch,
  p4_pd_netbufferv4_drop_getres_deleted_seq_inswitch,
  p4_pd_netbufferv4_update_getreq_inswitch_to_getreq,
  p4_pd_netbufferv4_update_getreq_inswitch_to_getreq_pop,
  p4_pd_netbufferv4_update_getreq_inswitch_to_getreq_nlatest,
  p4_pd_netbufferv4_update_getreq_inswitch_to_getres_by_mirroring,
  p4_pd_netbufferv4_update_getres_latest_seq_to_getres,
  p4_pd_netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss,
  p4_pd_netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss,
  p4_pd_netbufferv4_update_getres_deleted_seq_to_getres,
  p4_pd_netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss,
  p4_pd_netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss,
  p4_pd_netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone,
  p4_pd_netbufferv4_update_putreq_inswitch_to_putreq_seq,
  p4_pd_netbufferv4_update_putreq_inswitch_to_putreq_pop_seq,
  p4_pd_netbufferv4_update_putreq_inswitch_to_putres_by_mirroring,
  p4_pd_netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres,
  p4_pd_netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres,
  p4_pd_netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring,
  p4_pd_netbufferv4_update_putreq_inswitch_to_putreq_seq_case3,
  p4_pd_netbufferv4_update_putreq_inswitch_to_putreq_pop_seq_case3,
  p4_pd_netbufferv4_update_delreq_inswitch_to_delreq_seq,
  p4_pd_netbufferv4_update_delreq_inswitch_to_delres_by_mirroring,
  p4_pd_netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres,
  p4_pd_netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres,
  p4_pd_netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring,
  p4_pd_netbufferv4_update_delreq_inswitch_to_delreq_seq_case3,
  p4_pd_netbufferv4_forward_scanreq_split_and_clone,
  p4_pd_netbufferv4_forward_scanreq_split,
  p4_pd_netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone,
  p4_pd_netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone,
  p4_pd_netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone,
  p4_pd_netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone,
  p4_pd_netbufferv4_hash_for_cm1,
  p4_pd_netbufferv4_hash_for_cm2,
  p4_pd_netbufferv4_hash_for_cm3,
  p4_pd_netbufferv4_hash_for_cm4,
  p4_pd_netbufferv4_hash_for_seq,
  p4_pd_netbufferv4_update_getreq_to_getreq_inswitch,
  p4_pd_netbufferv4_update_getres_latest_seq_to_getres_latest_seq_inswitch,
  p4_pd_netbufferv4_update_getres_deleted_seq_to_getres_deleted_seq_inswitch,
  p4_pd_netbufferv4_update_putreq_to_putreq_inswitch,
  p4_pd_netbufferv4_update_delreq_to_delreq_inswitch,
  p4_pd_netbufferv4_update_scanreq_to_scanreq_split,
  p4_pd_netbufferv4_forward_normal_response,
  p4_pd_netbufferv4_forward_special_get_response,
  p4_pd_netbufferv4_set_is_hot,
  p4_pd_netbufferv4_reset_is_hot,
  p4_pd_netbufferv4_set_is_lastclone,
  p4_pd_netbufferv4_set_is_lastscansplit,
  p4_pd_netbufferv4_reset_is_lastclone_lastscansplit,
  p4_pd_netbufferv4_set_need_recirculate,
  p4_pd_netbufferv4_reset_need_recirculate,
  p4_pd_netbufferv4_set_client_sid,
  p4_pd_netbufferv4_process_scanreq_split,
  p4_pd_netbufferv4_process_cloned_scanreq_split,
  p4_pd_netbufferv4_reset_meta_serversid_remainscannum,
  p4_pd_netbufferv4_range_partition_for_scan_endkey,
  p4_pd_netbufferv4_range_partition,
  p4_pd_netbufferv4_recirculate_pkt,
  p4_pd_netbufferv4_sample,
  p4_pd_netbufferv4_save_client_udpport,
  p4_pd_netbufferv4_set_hot_threshold,
  p4_pd_netbufferv4_set_snapshot_flag,
  p4_pd_netbufferv4_reset_snapshot_flag,
  p4_pd_netbufferv4_update_ipmac_srcport_server2client,
  p4_pd_netbufferv4_update_ipmac_srcport_switch2switchos,
  p4_pd_netbufferv4_update_dstipmac_client2server,
  p4_pd_netbufferv4_update_pktlen,
  p4_pd_netbufferv4_get_valhi10,
  p4_pd_netbufferv4_set_and_get_valhi10,
  p4_pd_netbufferv4_reset_and_get_valhi10,
  p4_pd_netbufferv4_get_valhi11,
  p4_pd_netbufferv4_set_and_get_valhi11,
  p4_pd_netbufferv4_reset_and_get_valhi11,
  p4_pd_netbufferv4_get_valhi12,
  p4_pd_netbufferv4_set_and_get_valhi12,
  p4_pd_netbufferv4_reset_and_get_valhi12,
  p4_pd_netbufferv4_get_valhi13,
  p4_pd_netbufferv4_set_and_get_valhi13,
  p4_pd_netbufferv4_reset_and_get_valhi13,
  p4_pd_netbufferv4_get_valhi14,
  p4_pd_netbufferv4_set_and_get_valhi14,
  p4_pd_netbufferv4_reset_and_get_valhi14,
  p4_pd_netbufferv4_get_valhi15,
  p4_pd_netbufferv4_set_and_get_valhi15,
  p4_pd_netbufferv4_reset_and_get_valhi15,
  p4_pd_netbufferv4_get_valhi16,
  p4_pd_netbufferv4_set_and_get_valhi16,
  p4_pd_netbufferv4_reset_and_get_valhi16,
  p4_pd_netbufferv4_get_valhi1,
  p4_pd_netbufferv4_set_and_get_valhi1,
  p4_pd_netbufferv4_reset_and_get_valhi1,
  p4_pd_netbufferv4_get_valhi2,
  p4_pd_netbufferv4_set_and_get_valhi2,
  p4_pd_netbufferv4_reset_and_get_valhi2,
  p4_pd_netbufferv4_get_valhi3,
  p4_pd_netbufferv4_set_and_get_valhi3,
  p4_pd_netbufferv4_reset_and_get_valhi3,
  p4_pd_netbufferv4_get_valhi4,
  p4_pd_netbufferv4_set_and_get_valhi4,
  p4_pd_netbufferv4_reset_and_get_valhi4,
  p4_pd_netbufferv4_get_valhi5,
  p4_pd_netbufferv4_set_and_get_valhi5,
  p4_pd_netbufferv4_reset_and_get_valhi5,
  p4_pd_netbufferv4_get_valhi6,
  p4_pd_netbufferv4_set_and_get_valhi6,
  p4_pd_netbufferv4_reset_and_get_valhi6,
  p4_pd_netbufferv4_get_valhi7,
  p4_pd_netbufferv4_set_and_get_valhi7,
  p4_pd_netbufferv4_reset_and_get_valhi7,
  p4_pd_netbufferv4_get_valhi8,
  p4_pd_netbufferv4_set_and_get_valhi8,
  p4_pd_netbufferv4_reset_and_get_valhi8,
  p4_pd_netbufferv4_get_valhi9,
  p4_pd_netbufferv4_set_and_get_valhi9,
  p4_pd_netbufferv4_reset_and_get_valhi9,
  p4_pd_netbufferv4_get_vallen,
  p4_pd_netbufferv4_set_and_get_vallen,
  p4_pd_netbufferv4_reset_and_get_vallen,
  p4_pd_netbufferv4_reset_access_val_mode,
  p4_pd_netbufferv4_get_vallo10,
  p4_pd_netbufferv4_set_and_get_vallo10,
  p4_pd_netbufferv4_reset_and_get_vallo10,
  p4_pd_netbufferv4_get_vallo11,
  p4_pd_netbufferv4_set_and_get_vallo11,
  p4_pd_netbufferv4_reset_and_get_vallo11,
  p4_pd_netbufferv4_get_vallo12,
  p4_pd_netbufferv4_set_and_get_vallo12,
  p4_pd_netbufferv4_reset_and_get_vallo12,
  p4_pd_netbufferv4_get_vallo13,
  p4_pd_netbufferv4_set_and_get_vallo13,
  p4_pd_netbufferv4_reset_and_get_vallo13,
  p4_pd_netbufferv4_get_vallo14,
  p4_pd_netbufferv4_set_and_get_vallo14,
  p4_pd_netbufferv4_reset_and_get_vallo14,
  p4_pd_netbufferv4_get_vallo15,
  p4_pd_netbufferv4_set_and_get_vallo15,
  p4_pd_netbufferv4_reset_and_get_vallo15,
  p4_pd_netbufferv4_get_vallo16,
  p4_pd_netbufferv4_set_and_get_vallo16,
  p4_pd_netbufferv4_reset_and_get_vallo16,
  p4_pd_netbufferv4_get_vallo1,
  p4_pd_netbufferv4_set_and_get_vallo1,
  p4_pd_netbufferv4_reset_and_get_vallo1,
  p4_pd_netbufferv4_get_vallo2,
  p4_pd_netbufferv4_set_and_get_vallo2,
  p4_pd_netbufferv4_reset_and_get_vallo2,
  p4_pd_netbufferv4_get_vallo3,
  p4_pd_netbufferv4_set_and_get_vallo3,
  p4_pd_netbufferv4_reset_and_get_vallo3,
  p4_pd_netbufferv4_get_vallo4,
  p4_pd_netbufferv4_set_and_get_vallo4,
  p4_pd_netbufferv4_reset_and_get_vallo4,
  p4_pd_netbufferv4_get_vallo5,
  p4_pd_netbufferv4_set_and_get_vallo5,
  p4_pd_netbufferv4_reset_and_get_vallo5,
  p4_pd_netbufferv4_get_vallo6,
  p4_pd_netbufferv4_set_and_get_vallo6,
  p4_pd_netbufferv4_reset_and_get_vallo6,
  p4_pd_netbufferv4_get_vallo7,
  p4_pd_netbufferv4_set_and_get_vallo7,
  p4_pd_netbufferv4_reset_and_get_vallo7,
  p4_pd_netbufferv4_get_vallo8,
  p4_pd_netbufferv4_set_and_get_vallo8,
  p4_pd_netbufferv4_reset_and_get_vallo8,
  p4_pd_netbufferv4_get_vallo9,
  p4_pd_netbufferv4_set_and_get_vallo9,
  p4_pd_netbufferv4_reset_and_get_vallo9,
  p4_pd_netbufferv4_action_names_t_invalid
} p4_pd_netbufferv4_action_names_t;

const char* p4_pd_netbufferv4_action_enum_to_string(p4_pd_netbufferv4_action_names_t e);

p4_pd_netbufferv4_action_names_t p4_pd_netbufferv4_action_string_to_enum(const char* s);

  /* get_cache_frequency has no parameters */

  /* update_cache_frequency has no parameters */

  /* reset_cache_frequency has no parameters */

  /* nop has no parameters */

  /* try_case1 has no parameters */

  /* read_case1 has no parameters */

  /* reset_is_case1 has no parameters */

  /* update_cm1 has no parameters */

  /* initialize_cm1_predicate has no parameters */

  /* update_cm2 has no parameters */

  /* initialize_cm2_predicate has no parameters */

  /* update_cm3 has no parameters */

  /* initialize_cm3_predicate has no parameters */

  /* update_cm4 has no parameters */

  /* initialize_cm4_predicate has no parameters */

  /* get_deleted has no parameters */

  /* set_and_get_deleted has no parameters */

  /* reset_and_get_deleted has no parameters */

  /* reset_is_deleted has no parameters */

  /* get_latest has no parameters */

  /* set_and_get_latest has no parameters */

  /* reset_and_get_latest has no parameters */

  /* reset_is_latest has no parameters */

  /* get_savedseq has no parameters */

  /* set_and_get_savedseq has no parameters */

  /* assign_seq has no parameters */

  /* get_validvalue has no parameters */

  /* set_validvalue has no parameters */

  /* reset_meta_validvalue has no parameters */

  /* add_only_vallen has no parameters */

  /* add_to_val1 has no parameters */

  /* add_to_val2 has no parameters */

  /* add_to_val3 has no parameters */

  /* add_to_val4 has no parameters */

  /* add_to_val5 has no parameters */

  /* add_to_val6 has no parameters */

  /* add_to_val7 has no parameters */

  /* add_to_val8 has no parameters */

  /* add_to_val9 has no parameters */

  /* add_to_val10 has no parameters */

  /* add_to_val11 has no parameters */

  /* add_to_val12 has no parameters */

  /* add_to_val13 has no parameters */

  /* add_to_val14 has no parameters */

  /* add_to_val15 has no parameters */

  /* add_to_val16 has no parameters */

  /* remove_all has no parameters */

typedef struct p4_pd_netbufferv4_cached_action_action_spec {
  uint16_t action_idx;
} p4_pd_netbufferv4_cached_action_action_spec_t;

  /* uncached_action has no parameters */

  /* drop_getres_latest_seq_inswitch has no parameters */

  /* drop_getres_deleted_seq_inswitch has no parameters */

  /* update_getreq_inswitch_to_getreq has no parameters */

  /* update_getreq_inswitch_to_getreq_pop has no parameters */

  /* update_getreq_inswitch_to_getreq_nlatest has no parameters */

typedef struct p4_pd_netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec {
  uint32_t action_client_sid;
  uint16_t action_server_port;
  uint8_t action_stat;
} p4_pd_netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t;

  /* update_getres_latest_seq_to_getres has no parameters */

typedef struct p4_pd_netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec {
  uint32_t action_switchos_sid;
  uint8_t action_stat;
  uint16_t action_reflector_port;
} p4_pd_netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t;

typedef struct p4_pd_netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec {
  uint32_t action_switchos_sid;
} p4_pd_netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t;

  /* update_getres_deleted_seq_to_getres has no parameters */

typedef struct p4_pd_netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec {
  uint32_t action_switchos_sid;
  uint8_t action_stat;
  uint16_t action_reflector_port;
} p4_pd_netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t;

typedef struct p4_pd_netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec {
  uint32_t action_switchos_sid;
} p4_pd_netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t;

typedef struct p4_pd_netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec {
  uint32_t action_switchos_sid;
  uint16_t action_reflector_port;
} p4_pd_netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec_t;

  /* update_putreq_inswitch_to_putreq_seq has no parameters */

  /* update_putreq_inswitch_to_putreq_pop_seq has no parameters */

typedef struct p4_pd_netbufferv4_update_putreq_inswitch_to_putres_by_mirroring_action_spec {
  uint32_t action_client_sid;
  uint16_t action_server_port;
} p4_pd_netbufferv4_update_putreq_inswitch_to_putres_by_mirroring_action_spec_t;

typedef struct p4_pd_netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec {
  uint32_t action_switchos_sid;
  uint8_t action_stat;
  uint16_t action_reflector_port;
} p4_pd_netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t;

typedef struct p4_pd_netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec {
  uint32_t action_switchos_sid;
} p4_pd_netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t;

typedef struct p4_pd_netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec {
  uint32_t action_client_sid;
  uint16_t action_server_port;
} p4_pd_netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec_t;

  /* update_putreq_inswitch_to_putreq_seq_case3 has no parameters */

  /* update_putreq_inswitch_to_putreq_pop_seq_case3 has no parameters */

  /* update_delreq_inswitch_to_delreq_seq has no parameters */

typedef struct p4_pd_netbufferv4_update_delreq_inswitch_to_delres_by_mirroring_action_spec {
  uint32_t action_client_sid;
  uint16_t action_server_port;
} p4_pd_netbufferv4_update_delreq_inswitch_to_delres_by_mirroring_action_spec_t;

typedef struct p4_pd_netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec {
  uint32_t action_switchos_sid;
  uint8_t action_stat;
  uint16_t action_reflector_port;
} p4_pd_netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t;

typedef struct p4_pd_netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec {
  uint32_t action_switchos_sid;
} p4_pd_netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t;

typedef struct p4_pd_netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec {
  uint32_t action_client_sid;
  uint16_t action_server_port;
} p4_pd_netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec_t;

  /* update_delreq_inswitch_to_delreq_seq_case3 has no parameters */

typedef struct p4_pd_netbufferv4_forward_scanreq_split_and_clone_action_spec {
  uint32_t action_server_sid;
} p4_pd_netbufferv4_forward_scanreq_split_and_clone_action_spec_t;

  /* forward_scanreq_split has no parameters */

typedef struct p4_pd_netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec {
  uint32_t action_switchos_sid;
  uint16_t action_reflector_port;
} p4_pd_netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec_t;

typedef struct p4_pd_netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec {
  uint32_t action_switchos_sid;
  uint16_t action_reflector_port;
  uint8_t action_stat;
} p4_pd_netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec_t;

typedef struct p4_pd_netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec {
  uint32_t action_switchos_sid;
  uint16_t action_reflector_port;
  uint32_t action_stat;
} p4_pd_netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec_t;

typedef struct p4_pd_netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec {
  uint32_t action_switchos_sid;
  uint16_t action_reflector_port;
} p4_pd_netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec_t;

  /* hash_for_cm1 has no parameters */

  /* hash_for_cm2 has no parameters */

  /* hash_for_cm3 has no parameters */

  /* hash_for_cm4 has no parameters */

  /* hash_for_seq has no parameters */

  /* update_getreq_to_getreq_inswitch has no parameters */

  /* update_getres_latest_seq_to_getres_latest_seq_inswitch has no parameters */

  /* update_getres_deleted_seq_to_getres_deleted_seq_inswitch has no parameters */

  /* update_putreq_to_putreq_inswitch has no parameters */

  /* update_delreq_to_delreq_inswitch has no parameters */

  /* update_scanreq_to_scanreq_split has no parameters */

typedef struct p4_pd_netbufferv4_forward_normal_response_action_spec {
  uint16_t action_eport;
} p4_pd_netbufferv4_forward_normal_response_action_spec_t;

typedef struct p4_pd_netbufferv4_forward_special_get_response_action_spec {
  uint32_t action_client_sid;
} p4_pd_netbufferv4_forward_special_get_response_action_spec_t;

  /* set_is_hot has no parameters */

  /* reset_is_hot has no parameters */

  /* set_is_lastclone has no parameters */

  /* set_is_lastscansplit has no parameters */

  /* reset_is_lastclone_lastscansplit has no parameters */

  /* set_need_recirculate has no parameters */

  /* reset_need_recirculate has no parameters */

typedef struct p4_pd_netbufferv4_set_client_sid_action_spec {
  uint16_t action_client_sid;
} p4_pd_netbufferv4_set_client_sid_action_spec_t;

typedef struct p4_pd_netbufferv4_process_scanreq_split_action_spec {
  uint16_t action_server_sid;
} p4_pd_netbufferv4_process_scanreq_split_action_spec_t;

typedef struct p4_pd_netbufferv4_process_cloned_scanreq_split_action_spec {
  uint16_t action_server_sid;
} p4_pd_netbufferv4_process_cloned_scanreq_split_action_spec_t;

  /* reset_meta_serversid_remainscannum has no parameters */

typedef struct p4_pd_netbufferv4_range_partition_for_scan_endkey_action_spec {
  uint16_t action_last_udpport_plus_one;
} p4_pd_netbufferv4_range_partition_for_scan_endkey_action_spec_t;

typedef struct p4_pd_netbufferv4_range_partition_action_spec {
  uint16_t action_udpport;
  uint16_t action_eport;
} p4_pd_netbufferv4_range_partition_action_spec_t;

typedef struct p4_pd_netbufferv4_recirculate_pkt_action_spec {
  uint8_t action_port;
} p4_pd_netbufferv4_recirculate_pkt_action_spec_t;

  /* sample has no parameters */

  /* save_client_udpport has no parameters */

typedef struct p4_pd_netbufferv4_set_hot_threshold_action_spec {
  uint16_t action_hot_threshold;
} p4_pd_netbufferv4_set_hot_threshold_action_spec_t;

  /* set_snapshot_flag has no parameters */

  /* reset_snapshot_flag has no parameters */

typedef struct p4_pd_netbufferv4_update_ipmac_srcport_server2client_action_spec {
  uint8_t action_client_mac[6];
  uint8_t action_server_mac[6];
  uint32_t action_client_ip;
  uint32_t action_server_ip;
  uint16_t action_server_port;
} p4_pd_netbufferv4_update_ipmac_srcport_server2client_action_spec_t;

typedef struct p4_pd_netbufferv4_update_ipmac_srcport_switch2switchos_action_spec {
  uint8_t action_client_mac[6];
  uint8_t action_switch_mac[6];
  uint32_t action_client_ip;
  uint32_t action_switch_ip;
  uint16_t action_client_port;
} p4_pd_netbufferv4_update_ipmac_srcport_switch2switchos_action_spec_t;

typedef struct p4_pd_netbufferv4_update_dstipmac_client2server_action_spec {
  uint8_t action_server_mac[6];
  uint32_t action_server_ip;
} p4_pd_netbufferv4_update_dstipmac_client2server_action_spec_t;

typedef struct p4_pd_netbufferv4_update_pktlen_action_spec {
  uint16_t action_udplen;
  uint16_t action_iplen;
} p4_pd_netbufferv4_update_pktlen_action_spec_t;

  /* get_valhi10 has no parameters */

  /* set_and_get_valhi10 has no parameters */

  /* reset_and_get_valhi10 has no parameters */

  /* get_valhi11 has no parameters */

  /* set_and_get_valhi11 has no parameters */

  /* reset_and_get_valhi11 has no parameters */

  /* get_valhi12 has no parameters */

  /* set_and_get_valhi12 has no parameters */

  /* reset_and_get_valhi12 has no parameters */

  /* get_valhi13 has no parameters */

  /* set_and_get_valhi13 has no parameters */

  /* reset_and_get_valhi13 has no parameters */

  /* get_valhi14 has no parameters */

  /* set_and_get_valhi14 has no parameters */

  /* reset_and_get_valhi14 has no parameters */

  /* get_valhi15 has no parameters */

  /* set_and_get_valhi15 has no parameters */

  /* reset_and_get_valhi15 has no parameters */

  /* get_valhi16 has no parameters */

  /* set_and_get_valhi16 has no parameters */

  /* reset_and_get_valhi16 has no parameters */

  /* get_valhi1 has no parameters */

  /* set_and_get_valhi1 has no parameters */

  /* reset_and_get_valhi1 has no parameters */

  /* get_valhi2 has no parameters */

  /* set_and_get_valhi2 has no parameters */

  /* reset_and_get_valhi2 has no parameters */

  /* get_valhi3 has no parameters */

  /* set_and_get_valhi3 has no parameters */

  /* reset_and_get_valhi3 has no parameters */

  /* get_valhi4 has no parameters */

  /* set_and_get_valhi4 has no parameters */

  /* reset_and_get_valhi4 has no parameters */

  /* get_valhi5 has no parameters */

  /* set_and_get_valhi5 has no parameters */

  /* reset_and_get_valhi5 has no parameters */

  /* get_valhi6 has no parameters */

  /* set_and_get_valhi6 has no parameters */

  /* reset_and_get_valhi6 has no parameters */

  /* get_valhi7 has no parameters */

  /* set_and_get_valhi7 has no parameters */

  /* reset_and_get_valhi7 has no parameters */

  /* get_valhi8 has no parameters */

  /* set_and_get_valhi8 has no parameters */

  /* reset_and_get_valhi8 has no parameters */

  /* get_valhi9 has no parameters */

  /* set_and_get_valhi9 has no parameters */

  /* reset_and_get_valhi9 has no parameters */

  /* get_vallen has no parameters */

  /* set_and_get_vallen has no parameters */

  /* reset_and_get_vallen has no parameters */

  /* reset_access_val_mode has no parameters */

  /* get_vallo10 has no parameters */

  /* set_and_get_vallo10 has no parameters */

  /* reset_and_get_vallo10 has no parameters */

  /* get_vallo11 has no parameters */

  /* set_and_get_vallo11 has no parameters */

  /* reset_and_get_vallo11 has no parameters */

  /* get_vallo12 has no parameters */

  /* set_and_get_vallo12 has no parameters */

  /* reset_and_get_vallo12 has no parameters */

  /* get_vallo13 has no parameters */

  /* set_and_get_vallo13 has no parameters */

  /* reset_and_get_vallo13 has no parameters */

  /* get_vallo14 has no parameters */

  /* set_and_get_vallo14 has no parameters */

  /* reset_and_get_vallo14 has no parameters */

  /* get_vallo15 has no parameters */

  /* set_and_get_vallo15 has no parameters */

  /* reset_and_get_vallo15 has no parameters */

  /* get_vallo16 has no parameters */

  /* set_and_get_vallo16 has no parameters */

  /* reset_and_get_vallo16 has no parameters */

  /* get_vallo1 has no parameters */

  /* set_and_get_vallo1 has no parameters */

  /* reset_and_get_vallo1 has no parameters */

  /* get_vallo2 has no parameters */

  /* set_and_get_vallo2 has no parameters */

  /* reset_and_get_vallo2 has no parameters */

  /* get_vallo3 has no parameters */

  /* set_and_get_vallo3 has no parameters */

  /* reset_and_get_vallo3 has no parameters */

  /* get_vallo4 has no parameters */

  /* set_and_get_vallo4 has no parameters */

  /* reset_and_get_vallo4 has no parameters */

  /* get_vallo5 has no parameters */

  /* set_and_get_vallo5 has no parameters */

  /* reset_and_get_vallo5 has no parameters */

  /* get_vallo6 has no parameters */

  /* set_and_get_vallo6 has no parameters */

  /* reset_and_get_vallo6 has no parameters */

  /* get_vallo7 has no parameters */

  /* set_and_get_vallo7 has no parameters */

  /* reset_and_get_vallo7 has no parameters */

  /* get_vallo8 has no parameters */

  /* set_and_get_vallo8 has no parameters */

  /* reset_and_get_vallo8 has no parameters */

  /* get_vallo9 has no parameters */

  /* set_and_get_vallo9 has no parameters */

  /* reset_and_get_vallo9 has no parameters */


typedef struct p4_pd_netbufferv4_action_specs_t {
  p4_pd_netbufferv4_action_names_t name;
  union {
  /* get_cache_frequency has no parameters */
  /* update_cache_frequency has no parameters */
  /* reset_cache_frequency has no parameters */
  /* nop has no parameters */
  /* try_case1 has no parameters */
  /* read_case1 has no parameters */
  /* reset_is_case1 has no parameters */
  /* update_cm1 has no parameters */
  /* initialize_cm1_predicate has no parameters */
  /* update_cm2 has no parameters */
  /* initialize_cm2_predicate has no parameters */
  /* update_cm3 has no parameters */
  /* initialize_cm3_predicate has no parameters */
  /* update_cm4 has no parameters */
  /* initialize_cm4_predicate has no parameters */
  /* get_deleted has no parameters */
  /* set_and_get_deleted has no parameters */
  /* reset_and_get_deleted has no parameters */
  /* reset_is_deleted has no parameters */
  /* get_latest has no parameters */
  /* set_and_get_latest has no parameters */
  /* reset_and_get_latest has no parameters */
  /* reset_is_latest has no parameters */
  /* get_savedseq has no parameters */
  /* set_and_get_savedseq has no parameters */
  /* assign_seq has no parameters */
  /* get_validvalue has no parameters */
  /* set_validvalue has no parameters */
  /* reset_meta_validvalue has no parameters */
  /* add_only_vallen has no parameters */
  /* add_to_val1 has no parameters */
  /* add_to_val2 has no parameters */
  /* add_to_val3 has no parameters */
  /* add_to_val4 has no parameters */
  /* add_to_val5 has no parameters */
  /* add_to_val6 has no parameters */
  /* add_to_val7 has no parameters */
  /* add_to_val8 has no parameters */
  /* add_to_val9 has no parameters */
  /* add_to_val10 has no parameters */
  /* add_to_val11 has no parameters */
  /* add_to_val12 has no parameters */
  /* add_to_val13 has no parameters */
  /* add_to_val14 has no parameters */
  /* add_to_val15 has no parameters */
  /* add_to_val16 has no parameters */
  /* remove_all has no parameters */
    struct p4_pd_netbufferv4_cached_action_action_spec p4_pd_netbufferv4_cached_action;
  /* uncached_action has no parameters */
  /* drop_getres_latest_seq_inswitch has no parameters */
  /* drop_getres_deleted_seq_inswitch has no parameters */
  /* update_getreq_inswitch_to_getreq has no parameters */
  /* update_getreq_inswitch_to_getreq_pop has no parameters */
  /* update_getreq_inswitch_to_getreq_nlatest has no parameters */
    struct p4_pd_netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec p4_pd_netbufferv4_update_getreq_inswitch_to_getres_by_mirroring;
  /* update_getres_latest_seq_to_getres has no parameters */
    struct p4_pd_netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec p4_pd_netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss;
    struct p4_pd_netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec p4_pd_netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss;
  /* update_getres_deleted_seq_to_getres has no parameters */
    struct p4_pd_netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec p4_pd_netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss;
    struct p4_pd_netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec p4_pd_netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss;
    struct p4_pd_netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec p4_pd_netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone;
  /* update_putreq_inswitch_to_putreq_seq has no parameters */
  /* update_putreq_inswitch_to_putreq_pop_seq has no parameters */
    struct p4_pd_netbufferv4_update_putreq_inswitch_to_putres_by_mirroring_action_spec p4_pd_netbufferv4_update_putreq_inswitch_to_putres_by_mirroring;
    struct p4_pd_netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec p4_pd_netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres;
    struct p4_pd_netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec p4_pd_netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres;
    struct p4_pd_netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec p4_pd_netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring;
  /* update_putreq_inswitch_to_putreq_seq_case3 has no parameters */
  /* update_putreq_inswitch_to_putreq_pop_seq_case3 has no parameters */
  /* update_delreq_inswitch_to_delreq_seq has no parameters */
    struct p4_pd_netbufferv4_update_delreq_inswitch_to_delres_by_mirroring_action_spec p4_pd_netbufferv4_update_delreq_inswitch_to_delres_by_mirroring;
    struct p4_pd_netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec p4_pd_netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres;
    struct p4_pd_netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec p4_pd_netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres;
    struct p4_pd_netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec p4_pd_netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring;
  /* update_delreq_inswitch_to_delreq_seq_case3 has no parameters */
    struct p4_pd_netbufferv4_forward_scanreq_split_and_clone_action_spec p4_pd_netbufferv4_forward_scanreq_split_and_clone;
  /* forward_scanreq_split has no parameters */
    struct p4_pd_netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec p4_pd_netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone;
    struct p4_pd_netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec p4_pd_netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone;
    struct p4_pd_netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec p4_pd_netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone;
    struct p4_pd_netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec p4_pd_netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone;
  /* hash_for_cm1 has no parameters */
  /* hash_for_cm2 has no parameters */
  /* hash_for_cm3 has no parameters */
  /* hash_for_cm4 has no parameters */
  /* hash_for_seq has no parameters */
  /* update_getreq_to_getreq_inswitch has no parameters */
  /* update_getres_latest_seq_to_getres_latest_seq_inswitch has no parameters */
  /* update_getres_deleted_seq_to_getres_deleted_seq_inswitch has no parameters */
  /* update_putreq_to_putreq_inswitch has no parameters */
  /* update_delreq_to_delreq_inswitch has no parameters */
  /* update_scanreq_to_scanreq_split has no parameters */
    struct p4_pd_netbufferv4_forward_normal_response_action_spec p4_pd_netbufferv4_forward_normal_response;
    struct p4_pd_netbufferv4_forward_special_get_response_action_spec p4_pd_netbufferv4_forward_special_get_response;
  /* set_is_hot has no parameters */
  /* reset_is_hot has no parameters */
  /* set_is_lastclone has no parameters */
  /* set_is_lastscansplit has no parameters */
  /* reset_is_lastclone_lastscansplit has no parameters */
  /* set_need_recirculate has no parameters */
  /* reset_need_recirculate has no parameters */
    struct p4_pd_netbufferv4_set_client_sid_action_spec p4_pd_netbufferv4_set_client_sid;
    struct p4_pd_netbufferv4_process_scanreq_split_action_spec p4_pd_netbufferv4_process_scanreq_split;
    struct p4_pd_netbufferv4_process_cloned_scanreq_split_action_spec p4_pd_netbufferv4_process_cloned_scanreq_split;
  /* reset_meta_serversid_remainscannum has no parameters */
    struct p4_pd_netbufferv4_range_partition_for_scan_endkey_action_spec p4_pd_netbufferv4_range_partition_for_scan_endkey;
    struct p4_pd_netbufferv4_range_partition_action_spec p4_pd_netbufferv4_range_partition;
    struct p4_pd_netbufferv4_recirculate_pkt_action_spec p4_pd_netbufferv4_recirculate_pkt;
  /* sample has no parameters */
  /* save_client_udpport has no parameters */
    struct p4_pd_netbufferv4_set_hot_threshold_action_spec p4_pd_netbufferv4_set_hot_threshold;
  /* set_snapshot_flag has no parameters */
  /* reset_snapshot_flag has no parameters */
    struct p4_pd_netbufferv4_update_ipmac_srcport_server2client_action_spec p4_pd_netbufferv4_update_ipmac_srcport_server2client;
    struct p4_pd_netbufferv4_update_ipmac_srcport_switch2switchos_action_spec p4_pd_netbufferv4_update_ipmac_srcport_switch2switchos;
    struct p4_pd_netbufferv4_update_dstipmac_client2server_action_spec p4_pd_netbufferv4_update_dstipmac_client2server;
    struct p4_pd_netbufferv4_update_pktlen_action_spec p4_pd_netbufferv4_update_pktlen;
  /* get_valhi10 has no parameters */
  /* set_and_get_valhi10 has no parameters */
  /* reset_and_get_valhi10 has no parameters */
  /* get_valhi11 has no parameters */
  /* set_and_get_valhi11 has no parameters */
  /* reset_and_get_valhi11 has no parameters */
  /* get_valhi12 has no parameters */
  /* set_and_get_valhi12 has no parameters */
  /* reset_and_get_valhi12 has no parameters */
  /* get_valhi13 has no parameters */
  /* set_and_get_valhi13 has no parameters */
  /* reset_and_get_valhi13 has no parameters */
  /* get_valhi14 has no parameters */
  /* set_and_get_valhi14 has no parameters */
  /* reset_and_get_valhi14 has no parameters */
  /* get_valhi15 has no parameters */
  /* set_and_get_valhi15 has no parameters */
  /* reset_and_get_valhi15 has no parameters */
  /* get_valhi16 has no parameters */
  /* set_and_get_valhi16 has no parameters */
  /* reset_and_get_valhi16 has no parameters */
  /* get_valhi1 has no parameters */
  /* set_and_get_valhi1 has no parameters */
  /* reset_and_get_valhi1 has no parameters */
  /* get_valhi2 has no parameters */
  /* set_and_get_valhi2 has no parameters */
  /* reset_and_get_valhi2 has no parameters */
  /* get_valhi3 has no parameters */
  /* set_and_get_valhi3 has no parameters */
  /* reset_and_get_valhi3 has no parameters */
  /* get_valhi4 has no parameters */
  /* set_and_get_valhi4 has no parameters */
  /* reset_and_get_valhi4 has no parameters */
  /* get_valhi5 has no parameters */
  /* set_and_get_valhi5 has no parameters */
  /* reset_and_get_valhi5 has no parameters */
  /* get_valhi6 has no parameters */
  /* set_and_get_valhi6 has no parameters */
  /* reset_and_get_valhi6 has no parameters */
  /* get_valhi7 has no parameters */
  /* set_and_get_valhi7 has no parameters */
  /* reset_and_get_valhi7 has no parameters */
  /* get_valhi8 has no parameters */
  /* set_and_get_valhi8 has no parameters */
  /* reset_and_get_valhi8 has no parameters */
  /* get_valhi9 has no parameters */
  /* set_and_get_valhi9 has no parameters */
  /* reset_and_get_valhi9 has no parameters */
  /* get_vallen has no parameters */
  /* set_and_get_vallen has no parameters */
  /* reset_and_get_vallen has no parameters */
  /* reset_access_val_mode has no parameters */
  /* get_vallo10 has no parameters */
  /* set_and_get_vallo10 has no parameters */
  /* reset_and_get_vallo10 has no parameters */
  /* get_vallo11 has no parameters */
  /* set_and_get_vallo11 has no parameters */
  /* reset_and_get_vallo11 has no parameters */
  /* get_vallo12 has no parameters */
  /* set_and_get_vallo12 has no parameters */
  /* reset_and_get_vallo12 has no parameters */
  /* get_vallo13 has no parameters */
  /* set_and_get_vallo13 has no parameters */
  /* reset_and_get_vallo13 has no parameters */
  /* get_vallo14 has no parameters */
  /* set_and_get_vallo14 has no parameters */
  /* reset_and_get_vallo14 has no parameters */
  /* get_vallo15 has no parameters */
  /* set_and_get_vallo15 has no parameters */
  /* reset_and_get_vallo15 has no parameters */
  /* get_vallo16 has no parameters */
  /* set_and_get_vallo16 has no parameters */
  /* reset_and_get_vallo16 has no parameters */
  /* get_vallo1 has no parameters */
  /* set_and_get_vallo1 has no parameters */
  /* reset_and_get_vallo1 has no parameters */
  /* get_vallo2 has no parameters */
  /* set_and_get_vallo2 has no parameters */
  /* reset_and_get_vallo2 has no parameters */
  /* get_vallo3 has no parameters */
  /* set_and_get_vallo3 has no parameters */
  /* reset_and_get_vallo3 has no parameters */
  /* get_vallo4 has no parameters */
  /* set_and_get_vallo4 has no parameters */
  /* reset_and_get_vallo4 has no parameters */
  /* get_vallo5 has no parameters */
  /* set_and_get_vallo5 has no parameters */
  /* reset_and_get_vallo5 has no parameters */
  /* get_vallo6 has no parameters */
  /* set_and_get_vallo6 has no parameters */
  /* reset_and_get_vallo6 has no parameters */
  /* get_vallo7 has no parameters */
  /* set_and_get_vallo7 has no parameters */
  /* reset_and_get_vallo7 has no parameters */
  /* get_vallo8 has no parameters */
  /* set_and_get_vallo8 has no parameters */
  /* reset_and_get_vallo8 has no parameters */
  /* get_vallo9 has no parameters */
  /* set_and_get_vallo9 has no parameters */
  /* reset_and_get_vallo9 has no parameters */
  } u;
} p4_pd_netbufferv4_action_specs_t;

void p4_pd_netbufferv4_init(void);

/* HA TESTING INFRASTRUCTURE */

/* REGISTER VALUES */


/* IDLE TIME CONFIG */


p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_case1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_deleted_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_latest_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_savedseq_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_seq_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_validvalue_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_cache_lookup_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_drop_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_seq_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ipv4_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_is_hot_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_lastclone_lastscansplit_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_need_recirculate_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_prepare_for_cachehit_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_process_scanreq_split_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_range_partition_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_recirculate_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_sample_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_save_client_udpport_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_snapshot_flag_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_pktlen_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi10_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi11_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi12_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi13_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi14_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi15_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi16_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi5_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi6_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi7_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi8_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi9_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo10_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo11_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo12_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo13_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo14_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo15_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo16_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo5_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo6_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo7_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo8_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_match_spec_to_entry_hdl
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo9_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);



/* Dynamic Exm Table Key Mask */


/* ADD ENTRIES */

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_add_with_get_cache_frequency
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_add_with_get_cache_frequency
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_add_with_update_cache_frequency
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_add_with_update_cache_frequency
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_add_with_reset_cache_frequency
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_add_with_reset_cache_frequency
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_case1_tbl_table_add_with_try_case1
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_add_with_try_case1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_case1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_case1_tbl_table_add_with_read_case1
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_add_with_read_case1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_case1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_case1_tbl_table_add_with_reset_is_case1
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_add_with_reset_is_case1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_case1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm1_tbl_table_add_with_update_cm1
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_table_add_with_update_cm1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm1_tbl_table_add_with_initialize_cm1_predicate
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_table_add_with_initialize_cm1_predicate
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm2_tbl_table_add_with_update_cm2
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_table_add_with_update_cm2
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm2_tbl_table_add_with_initialize_cm2_predicate
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_table_add_with_initialize_cm2_predicate
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm3_tbl_table_add_with_update_cm3
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_table_add_with_update_cm3
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm3_tbl_table_add_with_initialize_cm3_predicate
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_table_add_with_initialize_cm3_predicate
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm4_tbl_table_add_with_update_cm4
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_table_add_with_update_cm4
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm4_tbl_table_add_with_initialize_cm4_predicate
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_table_add_with_initialize_cm4_predicate
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_add_with_get_deleted
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_add_with_get_deleted
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_deleted_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_add_with_set_and_get_deleted
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_add_with_set_and_get_deleted
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_deleted_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_add_with_reset_and_get_deleted
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_add_with_reset_and_get_deleted
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_deleted_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_add_with_reset_is_deleted
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_add_with_reset_is_deleted
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_deleted_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_add_with_get_latest
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_add_with_get_latest
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_latest_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_add_with_set_and_get_latest
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_add_with_set_and_get_latest
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_latest_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_add_with_reset_and_get_latest
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_add_with_reset_and_get_latest
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_latest_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_add_with_reset_is_latest
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_add_with_reset_is_latest
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_latest_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_savedseq_tbl_table_add_with_get_savedseq
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_add_with_get_savedseq
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_savedseq_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_savedseq_tbl_table_add_with_set_and_get_savedseq
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_add_with_set_and_get_savedseq
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_savedseq_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_savedseq_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_savedseq_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_seq_tbl_table_add_with_assign_seq
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_table_add_with_assign_seq
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_seq_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_seq_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_seq_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_validvalue_tbl_table_add_with_get_validvalue
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_add_with_get_validvalue
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_validvalue_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_validvalue_tbl_table_add_with_set_validvalue
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_add_with_set_validvalue
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_validvalue_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_validvalue_tbl_table_add_with_reset_meta_validvalue
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_add_with_reset_meta_validvalue
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_validvalue_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_only_vallen
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_only_vallen
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val1
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val2
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val2
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val3
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val3
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val4
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val4
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val5
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val5
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val6
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val6
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val7
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val7
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val8
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val8
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val9
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val9
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val10
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val10
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val11
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val11
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val12
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val12
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val13
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val13
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val14
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val14
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val15
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val15
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val16
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_add_to_val16
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_remove_all
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_add_with_remove_all
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_cache_lookup_tbl_table_add_with_cached_action
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_table_add_with_cached_action
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_cache_lookup_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_cached_action_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_cache_lookup_tbl_table_add_with_uncached_action
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_table_add_with_uncached_action
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_cache_lookup_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_drop_tbl_table_add_with_drop_getres_latest_seq_inswitch
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_add_with_drop_getres_latest_seq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_drop_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_drop_tbl_table_add_with_drop_getres_deleted_seq_inswitch
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_add_with_drop_getres_deleted_seq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_drop_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_drop_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_drop_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_pop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_pop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_nlatest
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_nlatest
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getres_latest_seq_to_getres
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getres_latest_seq_to_getres
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_to_getres
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_to_getres
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_pop_seq
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_pop_seq
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putres_by_mirroring
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putres_by_mirroring
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_putreq_inswitch_to_putres_by_mirroring_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq_case3
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq_case3
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_pop_seq_case3
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_pop_seq_case3
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delres_by_mirroring
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delres_by_mirroring
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_delreq_inswitch_to_delres_by_mirroring_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq_case3
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq_case3
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_forward_scanreq_split_and_clone
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_forward_scanreq_split_and_clone
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_scanreq_split_and_clone_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_forward_scanreq_split
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_forward_scanreq_split
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm1_tbl_table_add_with_hash_for_cm1
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_table_add_with_hash_for_cm1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm1_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm2_tbl_table_add_with_hash_for_cm2
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_table_add_with_hash_for_cm2
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm2_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm3_tbl_table_add_with_hash_for_cm3
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_table_add_with_hash_for_cm3
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm3_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm4_tbl_table_add_with_hash_for_cm4
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_table_add_with_hash_for_cm4
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm4_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_seq_tbl_table_add_with_hash_for_seq
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_table_add_with_hash_for_seq
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_seq_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_seq_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_seq_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_update_getreq_to_getreq_inswitch
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_update_getreq_to_getreq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_update_getres_latest_seq_to_getres_latest_seq_inswitch
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_update_getres_latest_seq_to_getres_latest_seq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_update_getres_deleted_seq_to_getres_deleted_seq_inswitch
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_update_getres_deleted_seq_to_getres_deleted_seq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_update_putreq_to_putreq_inswitch
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_update_putreq_to_putreq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_update_delreq_to_delreq_inswitch
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_update_delreq_to_delreq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_update_scanreq_to_scanreq_split
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_update_scanreq_to_scanreq_split
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ipv4_forward_tbl_table_add_with_forward_normal_response
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_add_with_forward_normal_response
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ipv4_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_normal_response_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ipv4_forward_tbl_table_add_with_forward_special_get_response
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_add_with_forward_special_get_response
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ipv4_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_special_get_response_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ipv4_forward_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ipv4_forward_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_is_hot_tbl_table_add_with_set_is_hot
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_table_add_with_set_is_hot
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_is_hot_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_is_hot_tbl_table_add_with_reset_is_hot
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_table_add_with_reset_is_hot
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_is_hot_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_add_with_set_is_lastclone
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_add_with_set_is_lastclone
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_lastclone_lastscansplit_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_add_with_set_is_lastscansplit
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_add_with_set_is_lastscansplit
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_lastclone_lastscansplit_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_add_with_reset_is_lastclone_lastscansplit
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_add_with_reset_is_lastclone_lastscansplit
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_lastclone_lastscansplit_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_need_recirculate_tbl_table_add_with_set_need_recirculate
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_table_add_with_set_need_recirculate
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_need_recirculate_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_need_recirculate_tbl_table_add_with_reset_need_recirculate
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_table_add_with_reset_need_recirculate
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_need_recirculate_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_add_with_set_client_sid
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_add_with_set_client_sid
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_prepare_for_cachehit_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_set_client_sid_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_prepare_for_cachehit_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_process_scanreq_split_tbl_table_add_with_process_scanreq_split
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_add_with_process_scanreq_split
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_process_scanreq_split_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_process_scanreq_split_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_process_scanreq_split_tbl_table_add_with_process_cloned_scanreq_split
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_add_with_process_cloned_scanreq_split
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_process_scanreq_split_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_process_cloned_scanreq_split_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_process_scanreq_split_tbl_table_add_with_reset_meta_serversid_remainscannum
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_add_with_reset_meta_serversid_remainscannum
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_process_scanreq_split_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_add_with_range_partition_for_scan_endkey
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_add_with_range_partition_for_scan_endkey
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_netbufferv4_range_partition_for_scan_endkey_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_range_partition_tbl_table_add_with_range_partition
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_table_add_with_range_partition
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_range_partition_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_netbufferv4_range_partition_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_range_partition_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_range_partition_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_recirculate_tbl_table_add_with_recirculate_pkt
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_table_add_with_recirculate_pkt
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_recirculate_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_recirculate_pkt_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_recirculate_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_recirculate_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_sample_tbl_table_add_with_sample
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_table_add_with_sample
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_sample_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_sample_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_sample_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_save_client_udpport_tbl_table_add_with_save_client_udpport
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_table_add_with_save_client_udpport
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_save_client_udpport_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_save_client_udpport_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_save_client_udpport_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_snapshot_flag_tbl_table_add_with_set_snapshot_flag
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_table_add_with_set_snapshot_flag
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_snapshot_flag_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_snapshot_flag_tbl_table_add_with_reset_snapshot_flag
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_table_add_with_reset_snapshot_flag
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_snapshot_flag_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_add_with_update_ipmac_srcport_server2client
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_add_with_update_ipmac_srcport_server2client
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_ipmac_srcport_server2client_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_add_with_update_ipmac_srcport_switch2switchos
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_add_with_update_ipmac_srcport_switch2switchos
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_ipmac_srcport_switch2switchos_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_add_with_update_dstipmac_client2server
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_add_with_update_dstipmac_client2server
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_dstipmac_client2server_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_pktlen_tbl_table_add_with_update_pktlen
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_table_add_with_update_pktlen
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_pktlen_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_netbufferv4_update_pktlen_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_pktlen_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_pktlen_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_add_with_get_valhi10
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_add_with_get_valhi10
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi10_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_add_with_set_and_get_valhi10
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_add_with_set_and_get_valhi10
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi10_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_add_with_reset_and_get_valhi10
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_add_with_reset_and_get_valhi10
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi10_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi10_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_add_with_get_valhi11
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_add_with_get_valhi11
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi11_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_add_with_set_and_get_valhi11
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_add_with_set_and_get_valhi11
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi11_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_add_with_reset_and_get_valhi11
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_add_with_reset_and_get_valhi11
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi11_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi11_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_add_with_get_valhi12
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_add_with_get_valhi12
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi12_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_add_with_set_and_get_valhi12
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_add_with_set_and_get_valhi12
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi12_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_add_with_reset_and_get_valhi12
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_add_with_reset_and_get_valhi12
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi12_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi12_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_add_with_get_valhi13
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_add_with_get_valhi13
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi13_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_add_with_set_and_get_valhi13
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_add_with_set_and_get_valhi13
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi13_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_add_with_reset_and_get_valhi13
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_add_with_reset_and_get_valhi13
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi13_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi13_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_add_with_get_valhi14
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_add_with_get_valhi14
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi14_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_add_with_set_and_get_valhi14
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_add_with_set_and_get_valhi14
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi14_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_add_with_reset_and_get_valhi14
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_add_with_reset_and_get_valhi14
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi14_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi14_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_add_with_get_valhi15
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_add_with_get_valhi15
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi15_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_add_with_set_and_get_valhi15
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_add_with_set_and_get_valhi15
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi15_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_add_with_reset_and_get_valhi15
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_add_with_reset_and_get_valhi15
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi15_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi15_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_add_with_get_valhi16
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_add_with_get_valhi16
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi16_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_add_with_set_and_get_valhi16
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_add_with_set_and_get_valhi16
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi16_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_add_with_reset_and_get_valhi16
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_add_with_reset_and_get_valhi16
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi16_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi16_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_add_with_get_valhi1
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_add_with_get_valhi1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_add_with_set_and_get_valhi1
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_add_with_set_and_get_valhi1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_add_with_reset_and_get_valhi1
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_add_with_reset_and_get_valhi1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_add_with_get_valhi2
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_add_with_get_valhi2
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_add_with_set_and_get_valhi2
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_add_with_set_and_get_valhi2
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_add_with_reset_and_get_valhi2
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_add_with_reset_and_get_valhi2
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_add_with_get_valhi3
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_add_with_get_valhi3
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_add_with_set_and_get_valhi3
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_add_with_set_and_get_valhi3
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_add_with_reset_and_get_valhi3
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_add_with_reset_and_get_valhi3
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_add_with_get_valhi4
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_add_with_get_valhi4
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_add_with_set_and_get_valhi4
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_add_with_set_and_get_valhi4
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_add_with_reset_and_get_valhi4
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_add_with_reset_and_get_valhi4
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_add_with_get_valhi5
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_add_with_get_valhi5
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi5_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_add_with_set_and_get_valhi5
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_add_with_set_and_get_valhi5
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi5_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_add_with_reset_and_get_valhi5
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_add_with_reset_and_get_valhi5
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi5_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi5_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_add_with_get_valhi6
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_add_with_get_valhi6
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi6_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_add_with_set_and_get_valhi6
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_add_with_set_and_get_valhi6
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi6_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_add_with_reset_and_get_valhi6
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_add_with_reset_and_get_valhi6
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi6_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi6_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_add_with_get_valhi7
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_add_with_get_valhi7
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi7_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_add_with_set_and_get_valhi7
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_add_with_set_and_get_valhi7
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi7_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_add_with_reset_and_get_valhi7
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_add_with_reset_and_get_valhi7
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi7_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi7_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_add_with_get_valhi8
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_add_with_get_valhi8
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi8_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_add_with_set_and_get_valhi8
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_add_with_set_and_get_valhi8
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi8_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_add_with_reset_and_get_valhi8
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_add_with_reset_and_get_valhi8
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi8_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi8_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_add_with_get_valhi9
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_add_with_get_valhi9
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi9_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_add_with_set_and_get_valhi9
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_add_with_set_and_get_valhi9
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi9_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_add_with_reset_and_get_valhi9
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_add_with_reset_and_get_valhi9
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi9_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi9_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_add_with_get_vallen
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_add_with_get_vallen
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_add_with_set_and_get_vallen
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_add_with_set_and_get_vallen
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_add_with_reset_and_get_vallen
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_add_with_reset_and_get_vallen
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_add_with_reset_access_val_mode
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_add_with_reset_access_val_mode
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_add_with_get_vallo10
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_add_with_get_vallo10
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo10_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_add_with_set_and_get_vallo10
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_add_with_set_and_get_vallo10
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo10_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_add_with_reset_and_get_vallo10
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_add_with_reset_and_get_vallo10
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo10_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo10_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_add_with_get_vallo11
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_add_with_get_vallo11
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo11_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_add_with_set_and_get_vallo11
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_add_with_set_and_get_vallo11
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo11_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_add_with_reset_and_get_vallo11
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_add_with_reset_and_get_vallo11
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo11_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo11_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_add_with_get_vallo12
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_add_with_get_vallo12
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo12_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_add_with_set_and_get_vallo12
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_add_with_set_and_get_vallo12
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo12_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_add_with_reset_and_get_vallo12
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_add_with_reset_and_get_vallo12
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo12_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo12_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_add_with_get_vallo13
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_add_with_get_vallo13
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo13_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_add_with_set_and_get_vallo13
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_add_with_set_and_get_vallo13
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo13_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_add_with_reset_and_get_vallo13
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_add_with_reset_and_get_vallo13
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo13_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo13_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_add_with_get_vallo14
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_add_with_get_vallo14
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo14_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_add_with_set_and_get_vallo14
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_add_with_set_and_get_vallo14
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo14_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_add_with_reset_and_get_vallo14
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_add_with_reset_and_get_vallo14
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo14_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo14_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_add_with_get_vallo15
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_add_with_get_vallo15
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo15_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_add_with_set_and_get_vallo15
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_add_with_set_and_get_vallo15
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo15_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_add_with_reset_and_get_vallo15
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_add_with_reset_and_get_vallo15
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo15_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo15_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_add_with_get_vallo16
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_add_with_get_vallo16
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo16_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_add_with_set_and_get_vallo16
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_add_with_set_and_get_vallo16
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo16_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_add_with_reset_and_get_vallo16
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_add_with_reset_and_get_vallo16
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo16_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo16_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_add_with_get_vallo1
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_add_with_get_vallo1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_add_with_set_and_get_vallo1
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_add_with_set_and_get_vallo1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_add_with_reset_and_get_vallo1
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_add_with_reset_and_get_vallo1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo1_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_add_with_get_vallo2
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_add_with_get_vallo2
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_add_with_set_and_get_vallo2
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_add_with_set_and_get_vallo2
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_add_with_reset_and_get_vallo2
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_add_with_reset_and_get_vallo2
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo2_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_add_with_get_vallo3
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_add_with_get_vallo3
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_add_with_set_and_get_vallo3
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_add_with_set_and_get_vallo3
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_add_with_reset_and_get_vallo3
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_add_with_reset_and_get_vallo3
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo3_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_add_with_get_vallo4
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_add_with_get_vallo4
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_add_with_set_and_get_vallo4
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_add_with_set_and_get_vallo4
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_add_with_reset_and_get_vallo4
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_add_with_reset_and_get_vallo4
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo4_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_add_with_get_vallo5
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_add_with_get_vallo5
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo5_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_add_with_set_and_get_vallo5
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_add_with_set_and_get_vallo5
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo5_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_add_with_reset_and_get_vallo5
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_add_with_reset_and_get_vallo5
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo5_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo5_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_add_with_get_vallo6
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_add_with_get_vallo6
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo6_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_add_with_set_and_get_vallo6
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_add_with_set_and_get_vallo6
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo6_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_add_with_reset_and_get_vallo6
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_add_with_reset_and_get_vallo6
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo6_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo6_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_add_with_get_vallo7
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_add_with_get_vallo7
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo7_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_add_with_set_and_get_vallo7
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_add_with_set_and_get_vallo7
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo7_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_add_with_reset_and_get_vallo7
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_add_with_reset_and_get_vallo7
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo7_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo7_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_add_with_get_vallo8
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_add_with_get_vallo8
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo8_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_add_with_set_and_get_vallo8
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_add_with_set_and_get_vallo8
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo8_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_add_with_reset_and_get_vallo8
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_add_with_reset_and_get_vallo8
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo8_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo8_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_add_with_get_vallo9
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_add_with_get_vallo9
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo9_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_add_with_set_and_get_vallo9
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_add_with_set_and_get_vallo9
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo9_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_add_with_reset_and_get_vallo9
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_add_with_reset_and_get_vallo9
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo9_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_add_with_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_add_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo9_tbl_match_spec_t *match_spec,
 p4_pd_entry_hdl_t *entry_hdl
);


/* DELETE ENTRIES */

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_case1_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_case1_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_case1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cm1_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm1_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cm2_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm2_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cm3_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm3_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cm4_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm4_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_deleted_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_latest_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_savedseq_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_savedseq_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_savedseq_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_seq_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_seq_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_seq_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_validvalue_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_validvalue_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_validvalue_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_cache_lookup_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_cache_lookup_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_cache_lookup_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_drop_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_drop_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_drop_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm1_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm1_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm2_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm2_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm3_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm3_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm4_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm4_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_seq_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_seq_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_seq_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_ipv4_forward_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ipv4_forward_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ipv4_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_is_hot_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_is_hot_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_is_hot_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_lastclone_lastscansplit_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_need_recirculate_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_need_recirculate_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_need_recirculate_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_prepare_for_cachehit_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_process_scanreq_split_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_process_scanreq_split_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_process_scanreq_split_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_range_partition_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_range_partition_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_range_partition_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_recirculate_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_recirculate_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_recirculate_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_sample_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_sample_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_sample_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_save_client_udpport_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_save_client_udpport_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_save_client_udpport_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_snapshot_flag_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_snapshot_flag_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_snapshot_flag_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_pktlen_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_pktlen_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_pktlen_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi10_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi11_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi12_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi13_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi14_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi15_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi16_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi5_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi6_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi7_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi8_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi9_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo10_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo11_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo12_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo13_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo14_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo15_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo16_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo5_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo6_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo7_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo8_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_delete
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_delete
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_delete_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_delete_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo9_tbl_match_spec_t *match_spec
);


/* Get default entry handle */

p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_set_hot_threshold_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_set_hot_threshold_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_get_default_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 p4_pd_entry_hdl_t* entry_hdl
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_get_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);


/* Clear default entry */

p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_set_hot_threshold_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_reset_default_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t pd_dev_tgt
);


/* MODIFY TABLE PROPERTIES */

p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_set_hot_threshold_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_set_hot_threshold_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_set_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t value,
 p4_pd_tbl_prop_args_t args
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_get_property
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_tbl_prop_type_t property,
 p4_pd_tbl_prop_value_t *value,
 p4_pd_tbl_prop_args_t *args
);


/* MODIFY ENTRIES */

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_get_cache_frequency
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_get_cache_frequency
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_get_cache_frequency_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_get_cache_frequency_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_update_cache_frequency
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_update_cache_frequency
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_update_cache_frequency_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_update_cache_frequency_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_reset_cache_frequency
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_reset_cache_frequency
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_reset_cache_frequency_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_reset_cache_frequency_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_case1_tbl_table_modify_with_try_case1
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_modify_with_try_case1
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_case1_tbl_table_modify_with_try_case1_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_modify_with_try_case1_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_case1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_case1_tbl_table_modify_with_read_case1
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_modify_with_read_case1
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_case1_tbl_table_modify_with_read_case1_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_modify_with_read_case1_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_case1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_case1_tbl_table_modify_with_reset_is_case1
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_modify_with_reset_is_case1
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_case1_tbl_table_modify_with_reset_is_case1_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_table_modify_with_reset_is_case1_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_case1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cm1_tbl_table_modify_with_update_cm1
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_table_modify_with_update_cm1
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm1_tbl_table_modify_with_update_cm1_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_table_modify_with_update_cm1_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cm1_tbl_table_modify_with_initialize_cm1_predicate
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_table_modify_with_initialize_cm1_predicate
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm1_tbl_table_modify_with_initialize_cm1_predicate_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_table_modify_with_initialize_cm1_predicate_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cm2_tbl_table_modify_with_update_cm2
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_table_modify_with_update_cm2
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm2_tbl_table_modify_with_update_cm2_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_table_modify_with_update_cm2_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cm2_tbl_table_modify_with_initialize_cm2_predicate
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_table_modify_with_initialize_cm2_predicate
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm2_tbl_table_modify_with_initialize_cm2_predicate_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_table_modify_with_initialize_cm2_predicate_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cm3_tbl_table_modify_with_update_cm3
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_table_modify_with_update_cm3
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm3_tbl_table_modify_with_update_cm3_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_table_modify_with_update_cm3_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cm3_tbl_table_modify_with_initialize_cm3_predicate
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_table_modify_with_initialize_cm3_predicate
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm3_tbl_table_modify_with_initialize_cm3_predicate_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_table_modify_with_initialize_cm3_predicate_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cm4_tbl_table_modify_with_update_cm4
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_table_modify_with_update_cm4
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm4_tbl_table_modify_with_update_cm4_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_table_modify_with_update_cm4_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_cm4_tbl_table_modify_with_initialize_cm4_predicate
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_table_modify_with_initialize_cm4_predicate
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm4_tbl_table_modify_with_initialize_cm4_predicate_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_table_modify_with_initialize_cm4_predicate_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_cm4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_get_deleted
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_get_deleted
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_get_deleted_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_get_deleted_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_deleted_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_set_and_get_deleted
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_set_and_get_deleted
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_set_and_get_deleted_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_set_and_get_deleted_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_deleted_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_reset_and_get_deleted
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_reset_and_get_deleted
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_reset_and_get_deleted_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_reset_and_get_deleted_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_deleted_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_reset_is_deleted
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_reset_is_deleted
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_reset_is_deleted_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_table_modify_with_reset_is_deleted_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_deleted_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_modify_with_get_latest
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_modify_with_get_latest
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_modify_with_get_latest_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_modify_with_get_latest_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_latest_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_modify_with_set_and_get_latest
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_modify_with_set_and_get_latest
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_modify_with_set_and_get_latest_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_modify_with_set_and_get_latest_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_latest_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_modify_with_reset_and_get_latest
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_modify_with_reset_and_get_latest
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_modify_with_reset_and_get_latest_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_modify_with_reset_and_get_latest_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_latest_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_modify_with_reset_is_latest
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_modify_with_reset_is_latest
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_table_modify_with_reset_is_latest_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_table_modify_with_reset_is_latest_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_latest_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_savedseq_tbl_table_modify_with_get_savedseq
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_modify_with_get_savedseq
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_savedseq_tbl_table_modify_with_get_savedseq_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_modify_with_get_savedseq_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_savedseq_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_savedseq_tbl_table_modify_with_set_and_get_savedseq
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_modify_with_set_and_get_savedseq
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_savedseq_tbl_table_modify_with_set_and_get_savedseq_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_modify_with_set_and_get_savedseq_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_savedseq_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_savedseq_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_savedseq_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_savedseq_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_seq_tbl_table_modify_with_assign_seq
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_table_modify_with_assign_seq
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_seq_tbl_table_modify_with_assign_seq_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_table_modify_with_assign_seq_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_seq_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_seq_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_seq_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_seq_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_validvalue_tbl_table_modify_with_get_validvalue
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_modify_with_get_validvalue
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_validvalue_tbl_table_modify_with_get_validvalue_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_modify_with_get_validvalue_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_validvalue_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_validvalue_tbl_table_modify_with_set_validvalue
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_modify_with_set_validvalue
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_validvalue_tbl_table_modify_with_set_validvalue_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_modify_with_set_validvalue_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_validvalue_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_access_validvalue_tbl_table_modify_with_reset_meta_validvalue
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_modify_with_reset_meta_validvalue
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_validvalue_tbl_table_modify_with_reset_meta_validvalue_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_table_modify_with_reset_meta_validvalue_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_access_validvalue_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_only_vallen
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_only_vallen
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_only_vallen_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_only_vallen_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val1
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val1
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val1_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val1_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val2
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val2
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val2_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val2_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val3
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val3
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val3_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val3_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val4
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val4
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val4_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val4_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val5
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val5
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val5_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val5_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val6
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val6
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val6_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val6_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val7
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val7
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val7_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val7_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val8
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val8
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val8_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val8_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val9
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val9
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val9_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val9_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val10
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val10
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val10_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val10_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val11
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val11
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val11_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val11_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val12
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val12
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val12_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val12_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val13
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val13
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val13_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val13_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val14
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val14
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val14_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val14_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val15
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val15
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val15_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val15_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val16
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val16
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val16_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_add_to_val16_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_remove_all
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_remove_all
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_remove_all_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_table_modify_with_remove_all_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_cache_lookup_tbl_table_modify_with_cached_action
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_table_modify_with_cached_action
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_cached_action_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_cache_lookup_tbl_table_modify_with_cached_action_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_table_modify_with_cached_action_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_cache_lookup_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_cached_action_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_cache_lookup_tbl_table_modify_with_uncached_action
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_table_modify_with_uncached_action
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_cache_lookup_tbl_table_modify_with_uncached_action_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_table_modify_with_uncached_action_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_cache_lookup_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_drop_tbl_table_modify_with_drop_getres_latest_seq_inswitch
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_modify_with_drop_getres_latest_seq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_drop_tbl_table_modify_with_drop_getres_latest_seq_inswitch_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_modify_with_drop_getres_latest_seq_inswitch_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_drop_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_drop_tbl_table_modify_with_drop_getres_deleted_seq_inswitch
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_modify_with_drop_getres_deleted_seq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_drop_tbl_table_modify_with_drop_getres_deleted_seq_inswitch_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_modify_with_drop_getres_deleted_seq_inswitch_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_drop_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_drop_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_drop_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_drop_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_pop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_pop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_pop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_pop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_nlatest
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_nlatest
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_nlatest_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getreq_nlatest_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getres_by_mirroring
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getres_by_mirroring
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getres_by_mirroring_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getreq_inswitch_to_getres_by_mirroring_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_latest_seq_to_getres
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_latest_seq_to_getres
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_latest_seq_to_getres_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_latest_seq_to_getres_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_deleted_seq_to_getres
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_deleted_seq_to_getres
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_deleted_seq_to_getres_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_deleted_seq_to_getres_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_pop_seq
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_pop_seq
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_pop_seq_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_pop_seq_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putres_by_mirroring
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putres_by_mirroring
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_putreq_inswitch_to_putres_by_mirroring_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putres_by_mirroring_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putres_by_mirroring_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_putreq_inswitch_to_putres_by_mirroring_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_case3
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_case3
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_case3_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_seq_case3_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_pop_seq_case3
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_pop_seq_case3
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_pop_seq_case3_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_putreq_inswitch_to_putreq_pop_seq_case3_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delres_by_mirroring
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delres_by_mirroring
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_delreq_inswitch_to_delres_by_mirroring_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delres_by_mirroring_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delres_by_mirroring_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_delreq_inswitch_to_delres_by_mirroring_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_case3
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_case3
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_case3_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_delreq_inswitch_to_delreq_seq_case3_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_scanreq_split_and_clone
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_scanreq_split_and_clone
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_forward_scanreq_split_and_clone_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_scanreq_split_and_clone_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_scanreq_split_and_clone_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_scanreq_split_and_clone_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_scanreq_split
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_scanreq_split
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_scanreq_split_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_forward_scanreq_split_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm1_tbl_table_modify_with_hash_for_cm1
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_table_modify_with_hash_for_cm1
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm1_tbl_table_modify_with_hash_for_cm1_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_table_modify_with_hash_for_cm1_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm1_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm1_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm2_tbl_table_modify_with_hash_for_cm2
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_table_modify_with_hash_for_cm2
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm2_tbl_table_modify_with_hash_for_cm2_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_table_modify_with_hash_for_cm2_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm2_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm2_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm3_tbl_table_modify_with_hash_for_cm3
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_table_modify_with_hash_for_cm3
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm3_tbl_table_modify_with_hash_for_cm3_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_table_modify_with_hash_for_cm3_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm3_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm3_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm4_tbl_table_modify_with_hash_for_cm4
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_table_modify_with_hash_for_cm4
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm4_tbl_table_modify_with_hash_for_cm4_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_table_modify_with_hash_for_cm4_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm4_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm4_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_cm4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_seq_tbl_table_modify_with_hash_for_seq
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_table_modify_with_hash_for_seq
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_seq_tbl_table_modify_with_hash_for_seq_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_table_modify_with_hash_for_seq_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_seq_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_hash_for_seq_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_seq_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_hash_for_seq_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_getreq_to_getreq_inswitch
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_getreq_to_getreq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_getreq_to_getreq_inswitch_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_getreq_to_getreq_inswitch_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_getres_latest_seq_to_getres_latest_seq_inswitch
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_getres_latest_seq_to_getres_latest_seq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_getres_latest_seq_to_getres_latest_seq_inswitch_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_getres_latest_seq_to_getres_latest_seq_inswitch_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_getres_deleted_seq_to_getres_deleted_seq_inswitch
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_getres_deleted_seq_to_getres_deleted_seq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_getres_deleted_seq_to_getres_deleted_seq_inswitch_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_getres_deleted_seq_to_getres_deleted_seq_inswitch_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_putreq_to_putreq_inswitch
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_putreq_to_putreq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_putreq_to_putreq_inswitch_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_putreq_to_putreq_inswitch_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_delreq_to_delreq_inswitch
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_delreq_to_delreq_inswitch
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_delreq_to_delreq_inswitch_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_delreq_to_delreq_inswitch_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_scanreq_to_scanreq_split
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_scanreq_to_scanreq_split
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_scanreq_to_scanreq_split_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_update_scanreq_to_scanreq_split_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_ipv4_forward_tbl_table_modify_with_forward_normal_response
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_modify_with_forward_normal_response
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_forward_normal_response_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_ipv4_forward_tbl_table_modify_with_forward_normal_response_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_modify_with_forward_normal_response_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ipv4_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_normal_response_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_ipv4_forward_tbl_table_modify_with_forward_special_get_response
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_modify_with_forward_special_get_response
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_forward_special_get_response_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_ipv4_forward_tbl_table_modify_with_forward_special_get_response_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_modify_with_forward_special_get_response_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ipv4_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_forward_special_get_response_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_ipv4_forward_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_ipv4_forward_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_ipv4_forward_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_is_hot_tbl_table_modify_with_set_is_hot
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_table_modify_with_set_is_hot
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_is_hot_tbl_table_modify_with_set_is_hot_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_table_modify_with_set_is_hot_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_is_hot_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_is_hot_tbl_table_modify_with_reset_is_hot
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_table_modify_with_reset_is_hot
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_is_hot_tbl_table_modify_with_reset_is_hot_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_table_modify_with_reset_is_hot_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_is_hot_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_modify_with_set_is_lastclone
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_modify_with_set_is_lastclone
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_modify_with_set_is_lastclone_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_modify_with_set_is_lastclone_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_lastclone_lastscansplit_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_modify_with_set_is_lastscansplit
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_modify_with_set_is_lastscansplit
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_modify_with_set_is_lastscansplit_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_modify_with_set_is_lastscansplit_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_lastclone_lastscansplit_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_modify_with_reset_is_lastclone_lastscansplit
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_modify_with_reset_is_lastclone_lastscansplit
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_modify_with_reset_is_lastclone_lastscansplit_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_table_modify_with_reset_is_lastclone_lastscansplit_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_lastclone_lastscansplit_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_need_recirculate_tbl_table_modify_with_set_need_recirculate
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_table_modify_with_set_need_recirculate
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_need_recirculate_tbl_table_modify_with_set_need_recirculate_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_table_modify_with_set_need_recirculate_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_need_recirculate_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_need_recirculate_tbl_table_modify_with_reset_need_recirculate
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_table_modify_with_reset_need_recirculate
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_need_recirculate_tbl_table_modify_with_reset_need_recirculate_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_table_modify_with_reset_need_recirculate_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_need_recirculate_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_modify_with_set_client_sid
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_modify_with_set_client_sid
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_set_client_sid_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_modify_with_set_client_sid_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_modify_with_set_client_sid_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_prepare_for_cachehit_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_set_client_sid_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_prepare_for_cachehit_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_process_scanreq_split_tbl_table_modify_with_process_scanreq_split
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_modify_with_process_scanreq_split
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_process_scanreq_split_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_process_scanreq_split_tbl_table_modify_with_process_scanreq_split_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_modify_with_process_scanreq_split_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_process_scanreq_split_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_process_scanreq_split_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_process_scanreq_split_tbl_table_modify_with_process_cloned_scanreq_split
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_modify_with_process_cloned_scanreq_split
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_process_cloned_scanreq_split_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_process_scanreq_split_tbl_table_modify_with_process_cloned_scanreq_split_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_modify_with_process_cloned_scanreq_split_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_process_scanreq_split_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_process_cloned_scanreq_split_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_process_scanreq_split_tbl_table_modify_with_reset_meta_serversid_remainscannum
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_modify_with_reset_meta_serversid_remainscannum
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_process_scanreq_split_tbl_table_modify_with_reset_meta_serversid_remainscannum_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_table_modify_with_reset_meta_serversid_remainscannum_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_process_scanreq_split_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_modify_with_range_partition_for_scan_endkey
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_modify_with_range_partition_for_scan_endkey
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_range_partition_for_scan_endkey_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_modify_with_range_partition_for_scan_endkey_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_modify_with_range_partition_for_scan_endkey_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_netbufferv4_range_partition_for_scan_endkey_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_range_partition_tbl_table_modify_with_range_partition
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_table_modify_with_range_partition
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_range_partition_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_range_partition_tbl_table_modify_with_range_partition_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_table_modify_with_range_partition_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_range_partition_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_netbufferv4_range_partition_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_range_partition_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_range_partition_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_range_partition_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_recirculate_tbl_table_modify_with_recirculate_pkt
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_table_modify_with_recirculate_pkt
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_recirculate_pkt_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_recirculate_tbl_table_modify_with_recirculate_pkt_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_table_modify_with_recirculate_pkt_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_recirculate_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_recirculate_pkt_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_recirculate_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_recirculate_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_recirculate_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_sample_tbl_table_modify_with_sample
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_table_modify_with_sample
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_sample_tbl_table_modify_with_sample_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_table_modify_with_sample_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_sample_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_sample_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_sample_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_sample_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_save_client_udpport_tbl_table_modify_with_save_client_udpport
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_table_modify_with_save_client_udpport
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_save_client_udpport_tbl_table_modify_with_save_client_udpport_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_table_modify_with_save_client_udpport_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_save_client_udpport_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_save_client_udpport_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_save_client_udpport_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_save_client_udpport_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_snapshot_flag_tbl_table_modify_with_set_snapshot_flag
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_table_modify_with_set_snapshot_flag
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_snapshot_flag_tbl_table_modify_with_set_snapshot_flag_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_table_modify_with_set_snapshot_flag_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_snapshot_flag_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_snapshot_flag_tbl_table_modify_with_reset_snapshot_flag
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_table_modify_with_reset_snapshot_flag
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_snapshot_flag_tbl_table_modify_with_reset_snapshot_flag_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_table_modify_with_reset_snapshot_flag_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_snapshot_flag_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_update_ipmac_srcport_server2client
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_update_ipmac_srcport_server2client
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_ipmac_srcport_server2client_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_update_ipmac_srcport_server2client_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_update_ipmac_srcport_server2client_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_ipmac_srcport_server2client_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_update_ipmac_srcport_switch2switchos
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_update_ipmac_srcport_switch2switchos
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_ipmac_srcport_switch2switchos_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_update_ipmac_srcport_switch2switchos_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_update_ipmac_srcport_switch2switchos_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_ipmac_srcport_switch2switchos_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_update_dstipmac_client2server
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_update_dstipmac_client2server
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_dstipmac_client2server_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_update_dstipmac_client2server_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_update_dstipmac_client2server_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_update_dstipmac_client2server_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_pktlen_tbl_table_modify_with_update_pktlen
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_table_modify_with_update_pktlen
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl,
 p4_pd_netbufferv4_update_pktlen_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_update_pktlen_tbl_table_modify_with_update_pktlen_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
 * @param action_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_table_modify_with_update_pktlen_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_pktlen_tbl_match_spec_t *match_spec,
 int priority,
 p4_pd_netbufferv4_update_pktlen_action_spec_t *action_spec
);

/**
 * @brief p4_pd_netbufferv4_update_pktlen_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_pktlen_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
 * @param priority
*/
p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_pktlen_tbl_match_spec_t *match_spec,
 int priority
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_get_valhi10
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_get_valhi10
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_get_valhi10_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_get_valhi10_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi10_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_set_and_get_valhi10
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_set_and_get_valhi10
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_set_and_get_valhi10_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_set_and_get_valhi10_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi10_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_reset_and_get_valhi10
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_reset_and_get_valhi10
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_reset_and_get_valhi10_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_reset_and_get_valhi10_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi10_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi10_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_get_valhi11
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_get_valhi11
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_get_valhi11_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_get_valhi11_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi11_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_set_and_get_valhi11
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_set_and_get_valhi11
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_set_and_get_valhi11_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_set_and_get_valhi11_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi11_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_reset_and_get_valhi11
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_reset_and_get_valhi11
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_reset_and_get_valhi11_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_reset_and_get_valhi11_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi11_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi11_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_get_valhi12
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_get_valhi12
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_get_valhi12_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_get_valhi12_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi12_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_set_and_get_valhi12
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_set_and_get_valhi12
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_set_and_get_valhi12_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_set_and_get_valhi12_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi12_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_reset_and_get_valhi12
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_reset_and_get_valhi12
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_reset_and_get_valhi12_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_reset_and_get_valhi12_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi12_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi12_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_get_valhi13
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_get_valhi13
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_get_valhi13_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_get_valhi13_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi13_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_set_and_get_valhi13
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_set_and_get_valhi13
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_set_and_get_valhi13_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_set_and_get_valhi13_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi13_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_reset_and_get_valhi13
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_reset_and_get_valhi13
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_reset_and_get_valhi13_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_reset_and_get_valhi13_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi13_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi13_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_get_valhi14
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_get_valhi14
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_get_valhi14_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_get_valhi14_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi14_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_set_and_get_valhi14
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_set_and_get_valhi14
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_set_and_get_valhi14_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_set_and_get_valhi14_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi14_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_reset_and_get_valhi14
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_reset_and_get_valhi14
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_reset_and_get_valhi14_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_reset_and_get_valhi14_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi14_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi14_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_get_valhi15
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_get_valhi15
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_get_valhi15_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_get_valhi15_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi15_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_set_and_get_valhi15
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_set_and_get_valhi15
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_set_and_get_valhi15_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_set_and_get_valhi15_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi15_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_reset_and_get_valhi15
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_reset_and_get_valhi15
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_reset_and_get_valhi15_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_reset_and_get_valhi15_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi15_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi15_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_get_valhi16
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_get_valhi16
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_get_valhi16_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_get_valhi16_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi16_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_set_and_get_valhi16
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_set_and_get_valhi16
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_set_and_get_valhi16_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_set_and_get_valhi16_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi16_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_reset_and_get_valhi16
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_reset_and_get_valhi16
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_reset_and_get_valhi16_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_reset_and_get_valhi16_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi16_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi16_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_get_valhi1
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_get_valhi1
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_get_valhi1_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_get_valhi1_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_set_and_get_valhi1
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_set_and_get_valhi1
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_set_and_get_valhi1_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_set_and_get_valhi1_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_reset_and_get_valhi1
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_reset_and_get_valhi1
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_reset_and_get_valhi1_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_reset_and_get_valhi1_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_get_valhi2
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_get_valhi2
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_get_valhi2_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_get_valhi2_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_set_and_get_valhi2
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_set_and_get_valhi2
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_set_and_get_valhi2_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_set_and_get_valhi2_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_reset_and_get_valhi2
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_reset_and_get_valhi2
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_reset_and_get_valhi2_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_reset_and_get_valhi2_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_get_valhi3
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_get_valhi3
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_get_valhi3_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_get_valhi3_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_set_and_get_valhi3
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_set_and_get_valhi3
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_set_and_get_valhi3_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_set_and_get_valhi3_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_reset_and_get_valhi3
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_reset_and_get_valhi3
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_reset_and_get_valhi3_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_reset_and_get_valhi3_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_get_valhi4
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_get_valhi4
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_get_valhi4_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_get_valhi4_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_set_and_get_valhi4
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_set_and_get_valhi4
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_set_and_get_valhi4_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_set_and_get_valhi4_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_reset_and_get_valhi4
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_reset_and_get_valhi4
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_reset_and_get_valhi4_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_reset_and_get_valhi4_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_get_valhi5
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_get_valhi5
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_get_valhi5_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_get_valhi5_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi5_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_set_and_get_valhi5
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_set_and_get_valhi5
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_set_and_get_valhi5_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_set_and_get_valhi5_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi5_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_reset_and_get_valhi5
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_reset_and_get_valhi5
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_reset_and_get_valhi5_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_reset_and_get_valhi5_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi5_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi5_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_get_valhi6
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_get_valhi6
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_get_valhi6_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_get_valhi6_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi6_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_set_and_get_valhi6
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_set_and_get_valhi6
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_set_and_get_valhi6_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_set_and_get_valhi6_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi6_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_reset_and_get_valhi6
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_reset_and_get_valhi6
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_reset_and_get_valhi6_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_reset_and_get_valhi6_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi6_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi6_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_get_valhi7
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_get_valhi7
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_get_valhi7_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_get_valhi7_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi7_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_set_and_get_valhi7
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_set_and_get_valhi7
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_set_and_get_valhi7_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_set_and_get_valhi7_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi7_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_reset_and_get_valhi7
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_reset_and_get_valhi7
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_reset_and_get_valhi7_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_reset_and_get_valhi7_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi7_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi7_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_get_valhi8
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_get_valhi8
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_get_valhi8_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_get_valhi8_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi8_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_set_and_get_valhi8
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_set_and_get_valhi8
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_set_and_get_valhi8_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_set_and_get_valhi8_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi8_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_reset_and_get_valhi8
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_reset_and_get_valhi8
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_reset_and_get_valhi8_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_reset_and_get_valhi8_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi8_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi8_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_get_valhi9
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_get_valhi9
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_get_valhi9_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_get_valhi9_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi9_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_set_and_get_valhi9
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_set_and_get_valhi9
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_set_and_get_valhi9_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_set_and_get_valhi9_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi9_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_reset_and_get_valhi9
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_reset_and_get_valhi9
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_reset_and_get_valhi9_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_reset_and_get_valhi9_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi9_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_valhi9_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_get_vallen
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_get_vallen
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_get_vallen_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_get_vallen_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_set_and_get_vallen
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_set_and_get_vallen
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_set_and_get_vallen_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_set_and_get_vallen_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_reset_and_get_vallen
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_reset_and_get_vallen
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_reset_and_get_vallen_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_reset_and_get_vallen_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_reset_access_val_mode
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_reset_access_val_mode
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_reset_access_val_mode_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_reset_access_val_mode_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_get_vallo10
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_get_vallo10
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_get_vallo10_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_get_vallo10_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo10_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_set_and_get_vallo10
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_set_and_get_vallo10
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_set_and_get_vallo10_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_set_and_get_vallo10_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo10_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_reset_and_get_vallo10
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_reset_and_get_vallo10
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_reset_and_get_vallo10_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_reset_and_get_vallo10_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo10_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo10_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_get_vallo11
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_get_vallo11
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_get_vallo11_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_get_vallo11_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo11_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_set_and_get_vallo11
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_set_and_get_vallo11
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_set_and_get_vallo11_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_set_and_get_vallo11_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo11_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_reset_and_get_vallo11
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_reset_and_get_vallo11
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_reset_and_get_vallo11_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_reset_and_get_vallo11_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo11_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo11_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_get_vallo12
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_get_vallo12
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_get_vallo12_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_get_vallo12_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo12_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_set_and_get_vallo12
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_set_and_get_vallo12
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_set_and_get_vallo12_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_set_and_get_vallo12_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo12_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_reset_and_get_vallo12
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_reset_and_get_vallo12
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_reset_and_get_vallo12_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_reset_and_get_vallo12_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo12_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo12_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_get_vallo13
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_get_vallo13
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_get_vallo13_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_get_vallo13_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo13_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_set_and_get_vallo13
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_set_and_get_vallo13
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_set_and_get_vallo13_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_set_and_get_vallo13_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo13_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_reset_and_get_vallo13
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_reset_and_get_vallo13
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_reset_and_get_vallo13_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_reset_and_get_vallo13_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo13_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo13_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_get_vallo14
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_get_vallo14
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_get_vallo14_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_get_vallo14_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo14_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_set_and_get_vallo14
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_set_and_get_vallo14
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_set_and_get_vallo14_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_set_and_get_vallo14_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo14_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_reset_and_get_vallo14
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_reset_and_get_vallo14
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_reset_and_get_vallo14_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_reset_and_get_vallo14_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo14_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo14_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_get_vallo15
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_get_vallo15
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_get_vallo15_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_get_vallo15_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo15_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_set_and_get_vallo15
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_set_and_get_vallo15
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_set_and_get_vallo15_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_set_and_get_vallo15_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo15_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_reset_and_get_vallo15
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_reset_and_get_vallo15
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_reset_and_get_vallo15_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_reset_and_get_vallo15_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo15_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo15_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_get_vallo16
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_get_vallo16
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_get_vallo16_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_get_vallo16_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo16_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_set_and_get_vallo16
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_set_and_get_vallo16
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_set_and_get_vallo16_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_set_and_get_vallo16_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo16_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_reset_and_get_vallo16
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_reset_and_get_vallo16
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_reset_and_get_vallo16_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_reset_and_get_vallo16_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo16_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo16_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_get_vallo1
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_get_vallo1
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_get_vallo1_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_get_vallo1_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_set_and_get_vallo1
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_set_and_get_vallo1
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_set_and_get_vallo1_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_set_and_get_vallo1_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_reset_and_get_vallo1
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_reset_and_get_vallo1
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_reset_and_get_vallo1_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_reset_and_get_vallo1_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo1_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_get_vallo2
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_get_vallo2
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_get_vallo2_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_get_vallo2_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_set_and_get_vallo2
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_set_and_get_vallo2
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_set_and_get_vallo2_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_set_and_get_vallo2_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_reset_and_get_vallo2
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_reset_and_get_vallo2
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_reset_and_get_vallo2_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_reset_and_get_vallo2_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo2_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_get_vallo3
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_get_vallo3
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_get_vallo3_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_get_vallo3_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_set_and_get_vallo3
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_set_and_get_vallo3
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_set_and_get_vallo3_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_set_and_get_vallo3_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_reset_and_get_vallo3
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_reset_and_get_vallo3
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_reset_and_get_vallo3_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_reset_and_get_vallo3_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo3_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_get_vallo4
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_get_vallo4
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_get_vallo4_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_get_vallo4_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_set_and_get_vallo4
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_set_and_get_vallo4
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_set_and_get_vallo4_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_set_and_get_vallo4_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_reset_and_get_vallo4
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_reset_and_get_vallo4
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_reset_and_get_vallo4_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_reset_and_get_vallo4_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo4_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_get_vallo5
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_get_vallo5
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_get_vallo5_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_get_vallo5_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo5_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_set_and_get_vallo5
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_set_and_get_vallo5
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_set_and_get_vallo5_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_set_and_get_vallo5_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo5_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_reset_and_get_vallo5
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_reset_and_get_vallo5
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_reset_and_get_vallo5_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_reset_and_get_vallo5_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo5_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo5_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_get_vallo6
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_get_vallo6
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_get_vallo6_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_get_vallo6_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo6_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_set_and_get_vallo6
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_set_and_get_vallo6
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_set_and_get_vallo6_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_set_and_get_vallo6_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo6_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_reset_and_get_vallo6
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_reset_and_get_vallo6
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_reset_and_get_vallo6_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_reset_and_get_vallo6_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo6_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo6_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_get_vallo7
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_get_vallo7
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_get_vallo7_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_get_vallo7_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo7_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_set_and_get_vallo7
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_set_and_get_vallo7
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_set_and_get_vallo7_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_set_and_get_vallo7_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo7_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_reset_and_get_vallo7
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_reset_and_get_vallo7
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_reset_and_get_vallo7_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_reset_and_get_vallo7_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo7_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo7_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_get_vallo8
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_get_vallo8
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_get_vallo8_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_get_vallo8_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo8_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_set_and_get_vallo8
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_set_and_get_vallo8
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_set_and_get_vallo8_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_set_and_get_vallo8_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo8_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_reset_and_get_vallo8
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_reset_and_get_vallo8
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_reset_and_get_vallo8_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_reset_and_get_vallo8_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo8_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo8_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_get_vallo9
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_get_vallo9
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_get_vallo9_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_get_vallo9_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo9_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_set_and_get_vallo9
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_set_and_get_vallo9
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_set_and_get_vallo9_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_set_and_get_vallo9_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo9_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_reset_and_get_vallo9
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_reset_and_get_vallo9
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_reset_and_get_vallo9_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_reset_and_get_vallo9_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo9_tbl_match_spec_t *match_spec
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_nop
 * @param sess_hdl
 * @param dev_id
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t ent_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_nop_by_match_spec
 * @param sess_hdl
 * @param dev_tgt
 * @param match_spec
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_table_modify_with_nop_by_match_spec
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_update_vallo9_tbl_match_spec_t *match_spec
);



/* SET DEFAULT_ACTION */

/**
 * @brief p4_pd_netbufferv4_access_cache_frequency_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_case1_tbl_set_default_action_reset_is_case1
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_set_default_action_reset_is_case1
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm1_tbl_set_default_action_initialize_cm1_predicate
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_set_default_action_initialize_cm1_predicate
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm2_tbl_set_default_action_initialize_cm2_predicate
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_set_default_action_initialize_cm2_predicate
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm3_tbl_set_default_action_initialize_cm3_predicate
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_set_default_action_initialize_cm3_predicate
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_cm4_tbl_set_default_action_initialize_cm4_predicate
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_set_default_action_initialize_cm4_predicate
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_deleted_tbl_set_default_action_reset_is_deleted
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_set_default_action_reset_is_deleted
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_latest_tbl_set_default_action_reset_is_latest
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_set_default_action_reset_is_latest
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_savedseq_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_seq_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_access_validvalue_tbl_set_default_action_reset_meta_validvalue
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_set_default_action_reset_meta_validvalue
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_add_and_remove_value_header_tbl_set_default_action_remove_all
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_set_default_action_remove_all
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_cache_lookup_tbl_set_default_action_uncached_action
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_set_default_action_uncached_action
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_drop_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_eg_port_forward_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm1_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm2_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm3_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_cm4_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_hash_for_seq_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ig_port_forward_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_ipv4_forward_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_is_hot_tbl_set_default_action_reset_is_hot
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_set_default_action_reset_is_hot
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_lastclone_lastscansplit_tbl_set_default_action_reset_is_lastclone_lastscansplit
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_set_default_action_reset_is_lastclone_lastscansplit
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_need_recirculate_tbl_set_default_action_reset_need_recirculate
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_set_default_action_reset_need_recirculate
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_prepare_for_cachehit_tbl_set_default_action_set_client_sid
 * @param sess_hdl
 * @param dev_tgt
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_set_default_action_set_client_sid
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_set_client_sid_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_process_scanreq_split_tbl_set_default_action_reset_meta_serversid_remainscannum
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_set_default_action_reset_meta_serversid_remainscannum
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_range_partition_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_recirculate_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_sample_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_save_client_udpport_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_set_hot_threshold_tbl_set_default_action_set_hot_threshold
 * @param sess_hdl
 * @param dev_tgt
 * @param action_spec
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_set_hot_threshold_tbl_set_default_action_set_hot_threshold
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_netbufferv4_set_hot_threshold_action_spec_t *action_spec,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_snapshot_flag_tbl_set_default_action_reset_snapshot_flag
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_set_default_action_reset_snapshot_flag
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_ipmac_srcport_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_pktlen_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi10_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi11_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi12_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi13_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi14_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi15_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi16_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi1_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi2_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi3_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi4_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi5_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi6_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi7_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi8_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_valhi9_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallen_tbl_set_default_action_reset_access_val_mode
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_set_default_action_reset_access_val_mode
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo10_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo11_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo12_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo13_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo14_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo15_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo16_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo1_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo2_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo3_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo4_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo5_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo6_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo7_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo8_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);

/**
 * @brief p4_pd_netbufferv4_update_vallo9_tbl_set_default_action_nop
 * @param sess_hdl
 * @param dev_tgt
 * @param entry_hdl
*/
p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_set_default_action_nop
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t *entry_hdl
);



/* INDIRECT ACTION DATA AND MATCH SELECT */






p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_set_hot_threshold_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_get_entry_count
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *count
);



p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_access_cache_frequency_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_access_cache_frequency_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_access_case1_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_access_case1_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm1_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_access_cm1_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm2_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_access_cm2_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm3_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_access_cm3_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_access_cm4_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_access_cm4_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_access_deleted_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_access_deleted_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_access_latest_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_access_latest_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_access_savedseq_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_access_savedseq_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_access_seq_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_access_seq_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_access_validvalue_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_access_validvalue_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_add_and_remove_value_header_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_add_and_remove_value_header_tbl_match_spec_t *match_spec,
 int *priority,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_cache_lookup_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_cache_lookup_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_drop_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_drop_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_eg_port_forward_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_eg_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm1_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_hash_for_cm1_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm2_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_hash_for_cm2_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm3_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_hash_for_cm3_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_cm4_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_hash_for_cm4_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_hash_for_seq_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_hash_for_seq_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_ig_port_forward_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_ig_port_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_ipv4_forward_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_ipv4_forward_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_is_hot_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_is_hot_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_lastclone_lastscansplit_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_lastclone_lastscansplit_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_need_recirculate_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_need_recirculate_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_prepare_for_cachehit_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_prepare_for_cachehit_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_process_scanreq_split_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_process_scanreq_split_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_range_partition_for_scan_endkey_tbl_match_spec_t *match_spec,
 int *priority,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_range_partition_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_range_partition_tbl_match_spec_t *match_spec,
 int *priority,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_recirculate_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_recirculate_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_sample_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_sample_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_save_client_udpport_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_save_client_udpport_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_set_hot_threshold_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_set_hot_threshold_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_set_hot_threshold_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_snapshot_flag_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_snapshot_flag_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_ipmac_srcport_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_ipmac_srcport_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_pktlen_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_pktlen_tbl_match_spec_t *match_spec,
 int *priority,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi10_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi10_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi11_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi11_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi12_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi12_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi13_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi13_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi14_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi14_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi15_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi15_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi16_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi16_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi1_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi1_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi2_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi2_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi3_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi3_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi4_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi4_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi5_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi5_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi6_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi6_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi7_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi7_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi8_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi8_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_valhi9_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_valhi9_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallen_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallen_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo10_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo10_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo11_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo11_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo12_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo12_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo13_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo13_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo14_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo14_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo15_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo15_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo16_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo16_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo1_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo1_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo2_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo2_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo3_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo3_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo4_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo4_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo5_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo5_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo6_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo6_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo7_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo7_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo8_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo8_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_get_first_entry_handle
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int *index
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_get_next_entry_handles
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_entry_hdl_t entry_handle,
 int n,
 int *next_entry_handles
);

p4_pd_status_t
p4_pd_netbufferv4_update_vallo9_tbl_get_entry
(
 p4_pd_sess_hdl_t sess_hdl,
 uint8_t dev_id,
 p4_pd_entry_hdl_t entry_hdl,
 bool read_from_hw,
 p4_pd_netbufferv4_update_vallo9_tbl_match_spec_t *match_spec,
 p4_pd_netbufferv4_action_specs_t *action_spec
);






p4_pd_status_t
p4_pd_netbufferv4_set_learning_timeout(p4_pd_sess_hdl_t shdl,
                                    uint8_t          device_id,
                                    uint32_t         usecs);

/* COUNTERS */




// REGISTERS

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_cm4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_cm4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint16_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_cm4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint16_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_cm4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_cm4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_cm4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint16_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_cm4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint16_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo14_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_cm3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_cm3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint16_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_cm3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint16_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_cm3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_cm3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_cm3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint16_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_cm3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint16_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi9_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_cache_frequency_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_cache_frequency_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_cache_frequency_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_cache_frequency_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_cache_frequency_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_cache_frequency_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_cache_frequency_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_cm2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_cm2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint16_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_cm2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint16_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_cm2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_cm2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_cm2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint16_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_cm2_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint16_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_deleted_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_deleted_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint8_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_deleted_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint8_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_deleted_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_deleted_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_deleted_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint8_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_deleted_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint8_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo13_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo7_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo6_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_seq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_seq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_seq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_seq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_seq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_seq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_seq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo11_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo5_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_cm1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_cm1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint16_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_cm1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint16_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_cm1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_cm1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_cm1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint16_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_cm1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint16_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_validvalue_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_validvalue_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint8_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_validvalue_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint8_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_validvalue_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_validvalue_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_validvalue_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint8_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_validvalue_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint8_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi12_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_latest_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_latest_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint8_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_latest_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint8_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_latest_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_latest_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_latest_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint8_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_latest_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint8_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo10_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo4_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo16_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo3_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi8_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallen_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallen_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint16_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallen_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint16_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallen_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallen_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallen_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint16_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallen_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint16_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_vallo1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_vallo1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_vallo1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_vallo1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_vallo1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_vallo1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_vallo1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_savedseq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_savedseq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_savedseq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_savedseq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_savedseq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_savedseq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_savedseq_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_valhi15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_valhi15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_valhi15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_valhi15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_valhi15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_valhi15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint32_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_valhi15_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint32_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_hw_sync_case1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 p4_pd_register_sync_cb cb_fn,
 void *cb_cookie
);

p4_pd_status_t
p4_pd_netbufferv4_register_read_case1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int flags,
 uint8_t *register_values,
 int *value_count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_case1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 uint8_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_reset_all_case1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_reset_case1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count
);

p4_pd_status_t
p4_pd_netbufferv4_register_write_all_case1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 uint8_t *register_value
);

p4_pd_status_t
p4_pd_netbufferv4_register_range_read_case1_reg
(
 p4_pd_sess_hdl_t sess_hdl,
 p4_pd_dev_target_t dev_tgt,
 int index,
 int count,
 int flags,
 int *num_actually_read,
 uint8_t *register_values,
 int *value_count
);



/* METERS */


/* LPF */


/* WRED */



typedef struct __attribute__((__packed__)) p4_pd_netbufferv4_ig_snapshot_trig_spec {
  uint8_t ig_intr_md_resubmit_flag;
  uint8_t ig_intr_md__pad1;
  uint8_t ig_intr_md__pad2;
  uint8_t ig_intr_md__pad3;
  uint16_t ig_intr_md_ingress_port;
  uint16_t ig_intr_md_for_tm_ucast_egress_port;
  uint16_t ig_intr_md_for_mb_ingress_mirror_id;
  uint8_t ethernet_hdr_dstAddr[6];
  uint8_t ethernet_hdr_srcAddr[6];
  uint16_t ethernet_hdr_etherType;
  uint8_t ipv4_hdr_version;
  uint8_t ipv4_hdr_ihl;
  uint8_t ipv4_hdr_diffserv;
  uint16_t ipv4_hdr_totalLen;
  uint16_t ipv4_hdr_identification;
  uint8_t ipv4_hdr_flags;
  uint16_t ipv4_hdr_fragOffset;
  uint8_t ipv4_hdr_ttl;
  uint8_t ipv4_hdr_protocol;
  uint16_t ipv4_hdr_hdrChecksum;
  uint32_t ipv4_hdr_srcAddr;
  uint32_t ipv4_hdr_dstAddr;
  uint16_t udp_hdr_srcPort;
  uint16_t udp_hdr_dstPort;
  uint16_t udp_hdr_hdrlen;
  uint16_t udp_hdr_checksum;
  uint16_t op_hdr_optype;
  uint32_t op_hdr_keylolo;
  uint32_t op_hdr_keylohi;
  uint32_t op_hdr_keyhilo;
  uint16_t op_hdr_keyhihilo;
  uint16_t op_hdr_keyhihihi;
  uint32_t scan_hdr_keylolo;
  uint32_t scan_hdr_keylohi;
  uint32_t scan_hdr_keyhilo;
  uint16_t scan_hdr_keyhihilo;
  uint16_t scan_hdr_keyhihihi;
  uint8_t split_hdr_is_clone;
  uint16_t split_hdr_cur_scanidx;
  uint16_t split_hdr_max_scannum;
  uint16_t vallen_hdr_vallen;
  uint32_t val1_hdr_vallo;
  uint32_t val1_hdr_valhi;
  uint32_t val2_hdr_vallo;
  uint32_t val2_hdr_valhi;
  uint32_t val3_hdr_vallo;
  uint32_t val3_hdr_valhi;
  uint32_t val4_hdr_vallo;
  uint32_t val4_hdr_valhi;
  uint32_t val5_hdr_vallo;
  uint32_t val5_hdr_valhi;
  uint32_t val6_hdr_vallo;
  uint32_t val6_hdr_valhi;
  uint32_t val7_hdr_vallo;
  uint32_t val7_hdr_valhi;
  uint32_t val8_hdr_vallo;
  uint32_t val8_hdr_valhi;
  uint32_t val9_hdr_vallo;
  uint32_t val9_hdr_valhi;
  uint32_t val10_hdr_vallo;
  uint32_t val10_hdr_valhi;
  uint32_t val11_hdr_vallo;
  uint32_t val11_hdr_valhi;
  uint32_t val12_hdr_vallo;
  uint32_t val12_hdr_valhi;
  uint32_t val13_hdr_vallo;
  uint32_t val13_hdr_valhi;
  uint32_t val14_hdr_vallo;
  uint32_t val14_hdr_valhi;
  uint32_t val15_hdr_vallo;
  uint32_t val15_hdr_valhi;
  uint32_t val16_hdr_vallo;
  uint32_t val16_hdr_valhi;
  uint16_t shadowtype_hdr_shadowtype;
  uint32_t seq_hdr_seq;
  uint8_t inswitch_hdr_snapshot_flag;
  uint8_t inswitch_hdr_is_cached;
  uint8_t inswitch_hdr_is_sampled;
  uint16_t inswitch_hdr_client_sid;
  uint8_t inswitch_hdr_padding;
  uint16_t inswitch_hdr_hot_threshold;
  uint16_t inswitch_hdr_hashval_for_cm1;
  uint16_t inswitch_hdr_hashval_for_cm2;
  uint16_t inswitch_hdr_hashval_for_cm3;
  uint16_t inswitch_hdr_hashval_for_cm4;
  uint16_t inswitch_hdr_hashval_for_seq;
  uint16_t inswitch_hdr_idx;
  uint8_t stat_hdr_stat;
  uint16_t stat_hdr_nodeidx_foreval;
  uint8_t clone_hdr_clonenum_for_pktloss;
  uint16_t clone_hdr_client_udpport;
  uint32_t frequency_hdr_frequency;
  uint8_t validvalue_hdr_validvalue;
  uint8_t meta_need_recirculate;
  uint8_t _selector_CLONE_I2E_DIGEST_RCVR;
  /* POV fields */
  uint8_t ethernet_hdr_valid;
  uint8_t ipv4_hdr_valid;
  uint8_t udp_hdr_valid;
  uint8_t op_hdr_valid;
  uint8_t scan_hdr_valid;
  uint8_t split_hdr_valid;
  uint8_t vallen_hdr_valid;
  uint8_t val1_hdr_valid;
  uint8_t val2_hdr_valid;
  uint8_t val3_hdr_valid;
  uint8_t val4_hdr_valid;
  uint8_t val5_hdr_valid;
  uint8_t val6_hdr_valid;
  uint8_t val7_hdr_valid;
  uint8_t val8_hdr_valid;
  uint8_t val9_hdr_valid;
  uint8_t val10_hdr_valid;
  uint8_t val11_hdr_valid;
  uint8_t val12_hdr_valid;
  uint8_t val13_hdr_valid;
  uint8_t val14_hdr_valid;
  uint8_t val15_hdr_valid;
  uint8_t val16_hdr_valid;
  uint8_t shadowtype_hdr_valid;
  uint8_t seq_hdr_valid;
  uint8_t inswitch_hdr_valid;
  uint8_t stat_hdr_valid;
  uint8_t clone_hdr_valid;
  uint8_t frequency_hdr_valid;
  uint8_t validvalue_hdr_valid;

} p4_pd_netbufferv4_ig_snapshot_trig_spec_t;


typedef struct __attribute__((__packed__)) p4_pd_netbufferv4_eg_snapshot_trig_spec {
  uint8_t ethernet_hdr_dstAddr[6];
  uint8_t ethernet_hdr_srcAddr[6];
  uint16_t ethernet_hdr_etherType;
  uint8_t ipv4_hdr_version;
  uint8_t ipv4_hdr_ihl;
  uint8_t ipv4_hdr_diffserv;
  uint16_t ipv4_hdr_totalLen;
  uint16_t ipv4_hdr_identification;
  uint8_t ipv4_hdr_flags;
  uint16_t ipv4_hdr_fragOffset;
  uint8_t ipv4_hdr_ttl;
  uint8_t ipv4_hdr_protocol;
  uint16_t ipv4_hdr_hdrChecksum;
  uint32_t ipv4_hdr_srcAddr;
  uint32_t ipv4_hdr_dstAddr;
  uint16_t udp_hdr_srcPort;
  uint16_t udp_hdr_dstPort;
  uint16_t udp_hdr_hdrlen;
  uint16_t udp_hdr_checksum;
  uint16_t op_hdr_optype;
  uint32_t op_hdr_keylolo;
  uint32_t op_hdr_keylohi;
  uint32_t op_hdr_keyhilo;
  uint16_t op_hdr_keyhihilo;
  uint16_t op_hdr_keyhihihi;
  uint32_t scan_hdr_keylolo;
  uint32_t scan_hdr_keylohi;
  uint32_t scan_hdr_keyhilo;
  uint16_t scan_hdr_keyhihilo;
  uint16_t scan_hdr_keyhihihi;
  uint8_t split_hdr_is_clone;
  uint16_t split_hdr_cur_scanidx;
  uint16_t split_hdr_max_scannum;
  uint16_t vallen_hdr_vallen;
  uint32_t val1_hdr_vallo;
  uint32_t val1_hdr_valhi;
  uint32_t val2_hdr_vallo;
  uint32_t val2_hdr_valhi;
  uint32_t val3_hdr_vallo;
  uint32_t val3_hdr_valhi;
  uint32_t val4_hdr_vallo;
  uint32_t val4_hdr_valhi;
  uint32_t val5_hdr_vallo;
  uint32_t val5_hdr_valhi;
  uint32_t val6_hdr_vallo;
  uint32_t val6_hdr_valhi;
  uint32_t val7_hdr_vallo;
  uint32_t val7_hdr_valhi;
  uint32_t val8_hdr_vallo;
  uint32_t val8_hdr_valhi;
  uint32_t val9_hdr_vallo;
  uint32_t val9_hdr_valhi;
  uint32_t val10_hdr_vallo;
  uint32_t val10_hdr_valhi;
  uint32_t val11_hdr_vallo;
  uint32_t val11_hdr_valhi;
  uint32_t val12_hdr_vallo;
  uint32_t val12_hdr_valhi;
  uint32_t val13_hdr_vallo;
  uint32_t val13_hdr_valhi;
  uint32_t val14_hdr_vallo;
  uint32_t val14_hdr_valhi;
  uint32_t val15_hdr_vallo;
  uint32_t val15_hdr_valhi;
  uint32_t val16_hdr_vallo;
  uint32_t val16_hdr_valhi;
  uint16_t shadowtype_hdr_shadowtype;
  uint32_t seq_hdr_seq;
  uint8_t inswitch_hdr_snapshot_flag;
  uint8_t inswitch_hdr_is_cached;
  uint8_t inswitch_hdr_is_sampled;
  uint16_t inswitch_hdr_client_sid;
  uint8_t inswitch_hdr_padding;
  uint16_t inswitch_hdr_hot_threshold;
  uint16_t inswitch_hdr_hashval_for_cm1;
  uint16_t inswitch_hdr_hashval_for_cm2;
  uint16_t inswitch_hdr_hashval_for_cm3;
  uint16_t inswitch_hdr_hashval_for_cm4;
  uint16_t inswitch_hdr_hashval_for_seq;
  uint16_t inswitch_hdr_idx;
  uint8_t stat_hdr_stat;
  uint16_t stat_hdr_nodeidx_foreval;
  uint8_t clone_hdr_clonenum_for_pktloss;
  uint16_t clone_hdr_client_udpport;
  uint32_t frequency_hdr_frequency;
  uint8_t validvalue_hdr_validvalue;
  uint8_t meta_cm1_predicate;
  uint8_t meta_cm2_predicate;
  uint8_t meta_cm3_predicate;
  uint8_t meta_cm4_predicate;
  uint8_t meta_is_latest;
  uint8_t meta_is_deleted;
  uint8_t meta_access_val_mode;
  uint8_t meta_is_case1;
  uint8_t meta_is_hot;
  uint16_t meta_server_sid;
  uint16_t meta_remain_scannum;
  uint8_t meta_is_lastclone_for_pktloss;
  uint8_t meta_is_last_scansplit;
  uint8_t eg_intr_md_for_oport_drop_ctl;
  uint16_t eg_intr_md_for_mb_egress_mirror_id;
  uint16_t eg_intr_md_egress_port;
  uint8_t eg_intr_md__pad0;
  uint8_t eg_intr_md__pad7;
  uint8_t eg_intr_md_egress_cos;
  uint8_t eg_intr_md_for_mb__pad1;
  uint8_t _selector_CLONE_E2E_DIGEST_RCVR;
  /* POV fields */
  uint8_t ethernet_hdr_valid;
  uint8_t ipv4_hdr_valid;
  uint8_t udp_hdr_valid;
  uint8_t op_hdr_valid;
  uint8_t scan_hdr_valid;
  uint8_t split_hdr_valid;
  uint8_t vallen_hdr_valid;
  uint8_t val1_hdr_valid;
  uint8_t val2_hdr_valid;
  uint8_t val3_hdr_valid;
  uint8_t val4_hdr_valid;
  uint8_t val5_hdr_valid;
  uint8_t val6_hdr_valid;
  uint8_t val7_hdr_valid;
  uint8_t val8_hdr_valid;
  uint8_t val9_hdr_valid;
  uint8_t val10_hdr_valid;
  uint8_t val11_hdr_valid;
  uint8_t val12_hdr_valid;
  uint8_t val13_hdr_valid;
  uint8_t val14_hdr_valid;
  uint8_t val15_hdr_valid;
  uint8_t val16_hdr_valid;
  uint8_t shadowtype_hdr_valid;
  uint8_t seq_hdr_valid;
  uint8_t inswitch_hdr_valid;
  uint8_t stat_hdr_valid;
  uint8_t clone_hdr_valid;
  uint8_t frequency_hdr_valid;
  uint8_t validvalue_hdr_valid;

} p4_pd_netbufferv4_eg_snapshot_trig_spec_t;


typedef struct __attribute__((__packed__)) p4_pd_netbufferv4_snapshot_trig_spec {
    union {
        p4_pd_netbufferv4_ig_snapshot_trig_spec_t ig;
        p4_pd_netbufferv4_eg_snapshot_trig_spec_t eg;
    } u;
}  p4_pd_netbufferv4_snapshot_trig_spec_t;


typedef p4_pd_netbufferv4_ig_snapshot_trig_spec_t p4_pd_netbufferv4_ig_snapshot_capture_data_t;
typedef p4_pd_netbufferv4_eg_snapshot_trig_spec_t p4_pd_netbufferv4_eg_snapshot_capture_data_t;


typedef struct __attribute__ ((__packed__)) p4_pd_netbufferv4_snapshot_capture {
     union {
         p4_pd_netbufferv4_ig_snapshot_capture_data_t ig;
         p4_pd_netbufferv4_eg_snapshot_capture_data_t eg;
     } u;
} p4_pd_netbufferv4_snapshot_capture_t;

/* Array of snapshot captures if start and en stage are different */
typedef struct p4_pd_netbufferv4_snapshot_capture_arr {
    p4_pd_netbufferv4_snapshot_capture_t captures[BF_MAX_SNAPSHOT_CAPTURES];
} p4_pd_netbufferv4_snapshot_capture_arr_t;


/**
 * @brief Set snapshot trigger.
 * @param hdl Snapshot handle.
 * @param trig_spec Trigger spec.
 * @param trig_mask Trigger mask.
 * @return status.
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_capture_trigger_set(
              pipe_snapshot_hdl_t hdl,
              p4_pd_netbufferv4_snapshot_trig_spec_t *trig_spec,
              p4_pd_netbufferv4_snapshot_trig_spec_t *trig_mask);

/**
 * @brief Get snapshot capture data.
 * @param hdl Snapshot handle.
 * @param pipe Pipe.
 * @param capture Captured data
 * @param num_captures Num of captures
 * @return status.
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_capture_data_get(
              pipe_snapshot_hdl_t hdl,
              uint16_t dev_pipe_id,
              p4_pd_netbufferv4_snapshot_capture_arr_t *capture,
              bf_snapshot_capture_ctrl_info_arr_t *capture_ctrl_arr,
              int *num_captures);

/**
 * @brief Create a snapshot.
 * @param dev_tgt Device information.
 * @param start_stage_id Start stage.
 * @param end_stage_id End stage.
 * @param direction Ingress or egress
 * @param hdl Snapshot handle.
 * @return status.
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_create(
            p4_pd_dev_target_t dev_tgt,
            uint8_t start_stage_id, uint8_t end_stage_id,
            bf_snapshot_dir_t direction,
            pipe_snapshot_hdl_t *hdl);

/**
 * @brief Delete snapshot.
 * @param hdl Snapshot handle.
 * @return status.
*/
p4_pd_status_t
p4_pd_netbufferv4_snapshot_delete(
            pipe_snapshot_hdl_t hdl);






typedef enum p4_pd_netbufferv4_input_field_attr_type {
  P4_PD_INPUT_FIELD_ATTR_TYPE_MASK = 0,
} p4_pd_netbufferv4_input_field_attr_type_t;

typedef enum p4_pd_netbufferv4_input_field_attr_value_mask {
  P4_PD_INPUT_FIELD_EXCLUDED = 0,
  P4_PD_INPUT_FIELD_INCLUDED
} p4_pd_netbufferv4_input_field_attr_value_mask_t;


#endif
