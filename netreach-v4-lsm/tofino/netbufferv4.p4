#include "tofino/constants.p4"
#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"
#include "tofino/primitives.p4"

// Uncomment it if support range query, or comment it otherwise
#define RANGE_SUPPORT

// Uncomment it before evaluation
// NOTE: update config.ini accordingly
//#define DEBUG

// NOTE: 1B optype does not need endian conversion
// 0b0001
#define PUTREQ 0x01
#define WARMUPREQ 0x11
#define LOADREQ 0x21
// 0b0011
#define GETRES_LATEST_SEQ 0x03
#define GETRES_DELETED_SEQ 0x13
#define PUTREQ_SEQ 0x23
#define PUTREQ_POP_SEQ 0x33
#define PUTREQ_SEQ_CASE3 0x43
#define PUTREQ_POP_SEQ_CASE3 0x53
// 0b0111
#define GETRES_LATEST_SEQ_INSWITCH 0x07
#define GETRES_DELETED_SEQ_INSWITCH 0x17
#define CACHE_POP_INSWITCH 0x27
// 0b1111
#define GETRES_LATEST_SEQ_INSWITCH_CASE1 0x0f
#define GETRES_DELETED_SEQ_INSWITCH_CASE1 0x1f
#define PUTREQ_SEQ_INSWITCH_CASE1 0x2f
#define DELREQ_SEQ_INSWITCH_CASE1 0x3f
// 0b1001
#define GETRES 0x09
// 0b0101
#define PUTREQ_INSWITCH 0x05
// 0b0100
#define GETREQ_INSWITCH 0x04
#define DELREQ_INSWITCH 0x14
// 0b0010
#define DELREQ_SEQ 0x02
#define DELREQ_SEQ_CASE3 0x12
// 0b1000
#define PUTRES 0x08
#define DELRES 0x18
// 0b0000
#define SCANREQ 0x10
#define SCANREQ_SPLIT 0x20
#define GETREQ 0x30
#define DELREQ 0x40
#define GETREQ_POP 0x50
#define GETREQ_NLATEST 0x60
#define CACHE_POP_INSWITCH_ACK 0x70
#define SCANRES_SPLIT 0x80
#define CACHE_POP 0x90
#define CACHE_EVICT 0xa0
#define CACHE_EVICT_ACK 0xb0
#define CACHE_EVICT_CASE2 0xc0
#define WARMUPACK 0xd0
#define LOADACK 0xe0
#define CACHE_POP_ACK 0xf0

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
// RANGE_PARTITION_ENTRY_NUM = 6 * MAX_SERVER_NUM < 8 * MAX_SERVER_NUM
#define RANGE_PARTITION_ENTRY_NUM 1024
// RANGE_PARTITION_FOR_SCAN_ENDKEY_ENTRY_NUM = 1 * MAX_SERVER_NUM
#define RANGE_PARTITION_FOR_SCAN_ENDKEY_ENTRY_NUM 128
// PROCESS_SCANREQ_SPLIT_ENTRY_NUM = 2 * MAX_SERVER_NUM
#define PROCESS_SCANREQ_SPLIT_ENTRY_NUM 256
// HASH_PARTITION_ENTRY_NUM = 5 * MAX_SERVER_NUM < 8 * MAX_SERVER_NUM
#define HASH_PARTITION_ENTRY_NUM 1024

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
#include "p4src/regs/seq.p4"
#include "p4src/regs/val.p4"
#include "p4src/regs/case1.p4"

#include "p4src/ingress_mat.p4"
#include "p4src/egress_mat.p4"

/* Ingress Processing */

control ingress {

	// Stage 0
	/*if (!valid(op_hdr)) {
		apply(l2_forward_tbl); // forward non-udp packet
	}*/
	apply(need_recirculate_tbl); // set meta.need_recirculate
	apply(set_hot_threshold_tbl); // set inswitch_hdr.hot_threshold

	/* if meta.need_recirculate == 1 */

	// Stage 1
	apply(recirculate_tbl); // recirculate for atomic snapshot (NOTE: recirculate will collide with modifying egress port)

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
	apply(cache_lookup_tbl); // managed by controller (access inswitch_hdr.is_cached, inswitch_hdr.idx)
	apply(hash_for_cm1_tbl); // for CM (access inswitch_hdr.hashval_for_cm1)

	// Stage 3
#ifdef RANGE_SUPPORT
	apply(range_partition_for_scan_endkey_tbl); // perform range partition for endkey of SCANREQ
#endif
	apply(hash_for_cm2_tbl); // for CM (access inswitch_hdr.hashval_for_cm2)
	apply(hash_for_seq_tbl); // for seq (access inswitch_hdr.hashval_for_seq)

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
	apply(access_seq_tbl);
#ifdef RANGE_SUPPORT
	apply(process_scanreq_split_tbl);
#endif

	// Stage 2
	apply(access_latest_tbl);
	apply(save_client_udpport_tbl);

	// Stage 3
	apply(access_deleted_tbl);
	apply(update_vallen_tbl);
	apply(access_savedseq_tbl);
	apply(access_case1_tbl);

	// Stage 4-9
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
	apply(update_vallo9_tbl);
	apply(update_valhi9_tbl);
	apply(update_vallo10_tbl);
	apply(update_valhi10_tbl);
	apply(update_vallo11_tbl);
	apply(update_valhi11_tbl);
	apply(update_vallo12_tbl);
	apply(update_valhi12_tbl);

	// Stage 9
	apply(lastclone_lastscansplit_tbl); // including is_last_scansplit

	// Stage 10
	apply(update_vallo13_tbl);
	apply(update_valhi13_tbl);
	apply(update_vallo14_tbl);
	apply(update_valhi14_tbl);
	apply(eg_port_forward_tbl); // including scan forwarding

	// Stage 11
	apply(update_vallo15_tbl);
	apply(update_valhi15_tbl);
	apply(update_vallo16_tbl);
	apply(update_valhi16_tbl);
	apply(update_pktlen_tbl); // Update udl_hdr.hdrLen for pkt with variable-length value
	apply(update_ipmac_srcport_tbl); // Update ip, mac, and srcport for RES to client and notification to switchos
	apply(add_and_remove_value_header_tbl); // Add or remove vallen and val according to optype and vallen
	apply(drop_tbl); // drop GETRES_LATEST_SEQ_INSWITCH and GETRES_DELETED_SEQ_INSWITCH
}
