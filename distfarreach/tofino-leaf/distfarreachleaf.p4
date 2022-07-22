#include "tofino/constants.p4"
#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"
#include "tofino/primitives.p4"

// Uncomment it if support range query, or comment it otherwise
// Change distfarreachleaf.p4, common.py, and helper.h accordingly
//#define RANGE_SUPPORT

// Uncomment it before evaluation
// NOTE: update config.ini accordingly
//#define DEBUG

// NOTE: 1B optype does not need endian conversion
// 0b0001
#define PUTREQ 0x0001
//#define WARMUPREQ 0x0011
#define LOADREQ 0x0021
#define LOADREQ_SPINE 0x0031
// 0b0011
#define PUTREQ_SEQ 0x0003
#define PUTREQ_POP_SEQ 0x0013
#define PUTREQ_SEQ_CASE3 0x0023
#define PUTREQ_POP_SEQ_CASE3 0x0033
#define NETCACHE_PUTREQ_SEQ_CACHED 0x0043
// 0b0110
#define DELREQ_SEQ_INSWITCH 0x0006
// 0b0111
#define PUTREQ_SEQ_INSWITCH 0x0007
// 0b1111
#define GETRES_LATEST_SEQ_INSWITCH 0x000f
#define GETRES_DELETED_SEQ_INSWITCH 0x001f
#define GETRES_LATEST_SEQ_INSWITCH_CASE1 0x002f
#define GETRES_DELETED_SEQ_INSWITCH_CASE1 0x003f
#define PUTREQ_SEQ_INSWITCH_CASE1 0x004f
#define DELREQ_SEQ_INSWITCH_CASE1 0x005f
#define LOADSNAPSHOTDATA_INSWITCH_ACK 0x006f
#define CACHE_POP_INSWITCH 0x007f
#define NETCACHE_VALUEUPDATE_INSWITCH 0x008f
#define GETRES_LATEST_SEQ_SERVER_INSWITCH 0x009f
#define GETRES_DELETED_SEQ_SERVER_INSWITCH 0x010f
// 0b1011
#define GETRES_LATEST_SEQ 0x000b
#define GETRES_DELETED_SEQ 0x001b
#define CACHE_EVICT_LOADDATA_INSWITCH_ACK 0x002b
#define NETCACHE_VALUEUPDATE 0x003b
#define GETRES_LATEST_SEQ_SERVER 0x004b
#define GETRES_DELETED_SEQ_SERVER 0x005b
// 0b1001
#define GETRES 0x0009
#define GETRES_SERVER 0x0019
// 0b0101
#define PUTREQ_INSWITCH 0x0005
// 0b0100
#define GETREQ_INSWITCH 0x0004
#define DELREQ_INSWITCH 0x0014
#define CACHE_EVICT_LOADFREQ_INSWITCH 0x0024
#define CACHE_EVICT_LOADDATA_INSWITCH 0x0034
#define LOADSNAPSHOTDATA_INSWITCH 0x0044
#define SETVALID_INSWITCH 0x0054
#define NETCACHE_WARMUPREQ_INSWITCH 0x0064
#define NETCACHE_WARMUPREQ_INSWITCH_POP 0x0074
// 0b0010
#define DELREQ_SEQ 0x0002
#define DELREQ_SEQ_CASE3 0x0012
#define NETCACHE_DELREQ_SEQ_CACHED 0x0022
// 0b1000
#define PUTRES 0x0008
#define DELRES 0x0018
#define PUTRES_SERVER 0x0028
#define DELRES_SERVER 0x0038
// 0b0000
#define WARMUPREQ 0x0000
#define SCANREQ 0x0010
#define SCANREQ_SPLIT 0x0020
#define GETREQ 0x0030
#define DELREQ 0x0040
#define GETREQ_POP 0x0050
#define GETREQ_NLATEST 0x0060
#define CACHE_POP_INSWITCH_ACK 0x0070
#define SCANRES_SPLIT 0x0080
#define CACHE_POP 0x0090
#define CACHE_EVICT 0x00a0
#define CACHE_EVICT_ACK 0x00b0
#define CACHE_EVICT_CASE2 0x00c0
#define WARMUPACK 0x00d0
#define LOADACK 0x00e0
#define CACHE_POP_ACK 0x00f0
#define CACHE_EVICT_LOADFREQ_INSWITCH_ACK 0x0100
#define SETVALID_INSWITCH_ACK 0x0110
#define NETCACHE_GETREQ_POP 0x0120
// NOTE: NETCACHE_CACHE_POP/_ACK, NETCACHE_CACHE_POP_FINISH/_ACK, NETCACHE_CACHE_EVICT/_ACK only used by end-hosts
#define NETCACHE_CACHE_POP 0x0130
#define NETCACHE_CACHE_POP_ACK 0x0140
#define NETCACHE_CACHE_POP_FINISH 0x0150
#define NETCACHE_CACHE_POP_FINISH_ACK 0x0160
#define NETCACHE_CACHE_EVICT 0x0170
#define NETCACHE_CACHE_EVICT_ACK 0x0180
#define NETCACHE_VALUEUPDATE_ACK 0x0190
// For distributed extension
#define GETREQ_SPINE 0x0200
#define SCANRES_SPLIT_SERVER 0x0210
#define WARMUPREQ_SPINE 0x0220
#define WARMUPACK_SERVER 0x0230
#define LOADACK_SERVER 0x0240
// NOTE: DISTCACHE_CACHE_EVICT_VICTIM/_ACK only used by end-hosts
#define DISTCACHE_CACHE_EVICT_VICTIM 0x0250
#define DISTCACHE_CACHE_EVICT_VICTIM_ACK 0x0260

/*
#define GETREQ 0x00
#define PUTREQ 0x01
#define DELREQ 0x02
#define SCANREQ 0x03
#define GETRES 0x04
#define PUTRES 0x05
#define DELRES 0x06
#define SCANRES_SPLIT 0x07
#define GETREQ_INSWITCH 0x08
#define GETREQ_POP 0x09
#define GETREQ_NLATEST 0x0a
#define GETRES_LATEST_SEQ 0x0b
#define GETRES_LATEST_SEQ_INSWITCH 0x0c
#define GETRES_LATEST_SEQ_INSWITCH_CASE1 0x0d
#define GETRES_DELETED_SEQ 0x0e
#define GETRES_DELETED_SEQ_INSWITCH 0x0f
#define GETRES_DELETED_SEQ_INSWITCH_CASE1 0x10
#define PUTREQ_INSWITCH 0x11
#define PUTREQ_SEQ 0x12
#define PUTREQ_POP_SEQ 0x13
#define PUTREQ_SEQ_INSWITCH_CASE1 0x14
#define PUTREQ_SEQ_CASE3 0x15
#define PUTREQ_POP_SEQ_CASE3 0x16
#define DELREQ_INSWITCH 0x17
#define DELREQ_SEQ 0x18
#define DELREQ_SEQ_INSWITCH_CASE1 0x19
#define DELREQ_SEQ_CASE3 0x1a
#define SCANREQ_SPLIT 0x1b
#define CACHE_POP 0x1c
#define CACHE_POP_INSWITCH 0x1d
#define CACHE_POP_INSWITCH_ACK 0x1e
#define CACHE_EVICT 0x1f
#define CACHE_EVICT_ACK 0x20
#define CACHE_EVICT_CASE2 0x21
*/

#ifndef DEBUG

// NOTE: limited by 12 stages and 64*4B PHV (not T-PHV) (fields in the same ALU must be in the same PHV group)
// 32K * (2B vallen + 128B value + 4B frequency + 1B status)
#define KV_BUCKET_COUNT 32768
// 64K * 2B counter
#define CM_BUCKET_COUNT 65536
//#define HH_THRESHOLD 10
// 32K * 4B counter
#define SEQ_BUCKET_COUNT 32768

#else

#define KV_BUCKET_COUNT 1
#define CM_BUCKET_COUNT 1
//#define HH_THRESHOLD 1
#define SEQ_BUCKET_COUNT 1

#endif

// hot_threshold=10 + sampling_ratio=0.5 -> hot_pktcnt=20 during each clean period (NOTE: cached key will not update CM)
// NOTE: it can be reconfigured by MAT
#define DEFAULT_HH_THRESHOLD 10

// egress_pipeline_num * kv_bucket_count
//#define LOOKUP_ENTRY_COUNT 65536
#define LOOKUP_ENTRY_COUNT 32768

// MAX_SERVER_NUM <= 128
#define MAX_SERVER_NUM 128
// SPINESELECT_ENTRY_NUM = 6 * MAX_SERVER_NUM < 8 * MAX_SERVER_NUM
#define SPINESELECT_ENTRY_NUM 1024
// RANGE_PARTITION_ENTRY_NUM = 11 * MAX_SERVER_NUM < 16 * MAX_SERVER_NUM
#define RANGE_PARTITION_ENTRY_NUM 2048
// RANGE_PARTITION_FOR_SCAN_ENDKEY_ENTRY_NUM = 1 * MAX_SERVER_NUM
#define RANGE_PARTITION_FOR_SCAN_ENDKEY_ENTRY_NUM 128
// PROCESS_SCANREQ_SPLIT_ENTRY_NUM = 2 * MAX_SERVER_NUM
#define PROCESS_SCANREQ_SPLIT_ENTRY_NUM 256
// HASH_PARTITION_ENTRY_NUM = 10 * MAX_SERVER_NUM < 16 * MAX_SERVER_NUM
#define HASH_PARTITION_ENTRY_NUM 2048

// hash partition range
#define PARTITION_COUNT 32768

#define SWITCHIDX_FOREVAL 0xFFFF

//#define CPU_PORT 192

// headers
#include "p4src/header.p4"

// parsers
#include "p4src/parser.p4"

// registers and MATs
#include "p4src/regs/cm.p4"
#include "p4src/regs/cache_frequency.p4"
#include "p4src/regs/validvalue.p4"
#include "p4src/regs/latest.p4"
#include "p4src/regs/deleted.p4"
#include "p4src/regs/val.p4"
#include "p4src/regs/case1.p4"

#include "p4src/ingress_mat.p4"
#include "p4src/egress_mat.p4"

/* Ingress Processing */

control ingress {

	// Stage 0
	if (not valid(op_hdr)) {
		apply(l2l3_forward_tbl); // forward traditional packet
	}
	apply(need_recirculate_tbl); // set meta.need_recirculate
	apply(set_hot_threshold_tbl); // set inswitch_hdr.hot_threshold
	apply(hash_for_spineselect_tbl); // set meta.hashval_for_spineselect

	/* if meta.need_recirculate == 1 */

	// Stage 1
	apply(recirculate_tbl); // recirculate for atomic snapshot (NOTE: recirculate will collide with modifying egress port)
	apply(spineselect_tbl); // forward requests from client to spine switch

	/* else if meta.need_recirculate == 0 */

	// Stage 1
#ifndef RANGE_SUPPORT
	apply(hash_for_partition_tbl); // for hash partition (including startkey of SCANREQ)
#endif

	// Stage 2 (not sure why we cannot place cache_lookup_tbl, hash_for_cm_tbl, and hash_for_seq_tbl in stage 1; follow automatic placement of tofino compiler)
#ifdef RANGE_SUPPORT
	apply(range_partition_tbl); // for range partition (GET/PUT/DEL)
#else
	apply(hash_partition_tbl);
#endif
	// IMPORTANT: to save TCAM, we do not match op_hdr.optype in cache_lookup_tbl 
	// -> so as long as op_hdr.key matches an entry in cache_lookup_tbl, inswitch_hdr.is_cached must be 1 (e.g., CACHE_EVICT_LOADXXX)
	// -> but note that if the optype does not have inswitch_hdr, is_cached of 1 will be dropped after entering egress pipeline, and is_cached is still 0 (e.g., SCANREQ_SPLIT)
	apply(cache_lookup_tbl); // managed by controller (access inswitch_hdr.is_cached, inswitch_hdr.idx)
	apply(hash_for_cm1_tbl); // for CM (access inswitch_hdr.hashval_for_cm1)

	// Stage 3
#ifdef RANGE_SUPPORT
	apply(range_partition_for_scan_endkey_tbl); // perform range partition for endkey of SCANREQ
#endif
	apply(hash_for_cm2_tbl); // for CM (access inswitch_hdr.hashval_for_cm2)

	// Stage 4
	apply(hash_for_cm3_tbl); // for CM (access inswitch_hdr.hashval_for_cm3)
	apply(snapshot_flag_tbl); // for snapshot (access inswitch_hdr.snapshot_flag)

	// Stage 5
	apply(hash_for_cm4_tbl); // for CM (access inswitch_hdr.hashval_for_cm4)
	apply(prepare_for_cachehit_tbl); // for response of cache hit (access inswitch_hdr.client_sid)
	apply(ipv4_forward_tbl); // update egress_port for normal/speical response packets

	// Stage 6
	apply(sample_tbl); // for CM and cache_frequency (access inswitch_hdr.is_sampled)
	apply(ig_port_forward_tbl); // update op_hdr.optype
}

/* Egress Processing */

control egress {

	// Stage 0
	apply(access_cm1_tbl);
	apply(access_cm2_tbl);
	apply(access_cm3_tbl);
	apply(access_cm4_tbl);

	// Stage 1
	apply(is_hot_tbl);
	apply(access_cache_frequency_tbl);
	apply(access_validvalue_tbl);
#ifdef RANGE_SUPPORT
	apply(process_scanreq_split_tbl);
#endif

	// Stage 2
	apply(access_latest_tbl);
	apply(save_client_info_tbl); // save srcip/srcmac/udp.dstport (client ip/mac/udpport) for cache hit response of GET/PUT/DELREQ_INSWITCH

	// Stage 3
	apply(access_deleted_tbl);
	apply(update_vallen_tbl);
	apply(access_savedseq_tbl);
	apply(access_case1_tbl);

	// Stage 4-7
	// NOTE: value registers do not reply on op_hdr.optype, they only rely on meta.access_val_mode, which is set by update_vallen_tbl in stage 3
	apply(update_vallo1_tbl);
	apply(update_valhi1_tbl);
	apply(update_vallo2_tbl);
	apply(update_valhi2_tbl);
	apply(update_vallo3_tbl);
	apply(update_valhi3_tbl);
	apply(update_vallo4_tbl);
	apply(update_valhi4_tbl);
	apply(update_vallo5_tbl);
	apply(update_valhi5_tbl);
	apply(update_vallo6_tbl);
	apply(update_valhi6_tbl);
	apply(update_vallo7_tbl);
	apply(update_valhi7_tbl);
	apply(update_vallo8_tbl);
	apply(update_valhi8_tbl);

	// Stage 8
	apply(lastclone_lastscansplit_tbl); // including is_last_scansplit
	apply(update_vallo9_tbl);
	apply(update_valhi9_tbl);
	apply(update_vallo10_tbl);
	apply(update_valhi10_tbl);

	// Stage 9
	apply(eg_port_forward_tbl); // including scan forwarding
	// NOTE: Comment val11 and val12 in debug mode to save resources for eg_port_forward_counter -> you need to disable debug mode in evaluation
#ifndef DEBUG
	apply(update_vallo11_tbl);
	apply(update_valhi11_tbl);
	apply(update_vallo12_tbl);
	apply(update_valhi12_tbl);
#endif

	// stage 10
	// NOTE: resource in stage 11 is not enough for update_ipmac_src_port_tbl, so we place it into stage 10
	// NOTE: for GET/PUT/DEL/SCAN/WARMUP/LOADREQ from client, they do NOT perform client2server in update_ipmac_srcport_tbl as their eport must be the devport of spine switch instead of a server
	apply(update_ipmac_srcport_tbl); // Update ip, mac, and srcport for RES to client and notification to switchos
	apply(update_vallo13_tbl);
	apply(update_valhi13_tbl);
	apply(update_vallo14_tbl);
	apply(update_valhi14_tbl);

	// Stage 11
	apply(update_pktlen_tbl); // Update udl_hdr.hdrLen for pkt with variable-length value
	apply(add_and_remove_value_header_tbl); // Add or remove vallen and val according to optype and vallen
	apply(drop_tbl); // drop GETRES_LATEST_SEQ_INSWITCH and GETRES_DELETED_SEQ_INSWITCH
	apply(update_vallo15_tbl);
	apply(update_valhi15_tbl);
	apply(update_vallo16_tbl);
	apply(update_valhi16_tbl);
}
