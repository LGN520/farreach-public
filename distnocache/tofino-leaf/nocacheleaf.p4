#include "tofino/constants.p4"
#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"
#include "tofino/primitives.p4"

// Uncomment it if support range query, or comment it otherwise
// Change distnocacheleaf.p4, common.py, and helper.h accordingly
//#define RANGE_SUPPORT

// Uncomment it before evaluation
// NOTE: update config.ini accordingly
//#define DEBUG

// NOTE: 1B optype does not need endian conversion
// NOTE: we keep all optypes for distnocacheleaf, but only a few of them will be used
// 0b0001
#define PUTREQ 0x0001
//#define WARMUPREQ 0x0011
#define LOADREQ 0x0021
// 0b0011
#define PUTREQ_SEQ 0x0003
#define PUTREQ_POP_SEQ 0x0013
#define PUTREQ_SEQ_CASE3 0x0023
#define PUTREQ_POP_SEQ_CASE3 0x0033
#define NETCACHE_PUTREQ_SEQ_CACHED 0x0043
// 0b0111
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
// 0b1011
#define GETRES_LATEST_SEQ 0x000b
#define GETRES_DELETED_SEQ 0x001b
#define CACHE_EVICT_LOADDATA_INSWITCH_ACK 0x002b
#define NETCACHE_VALUEUPDATE 0x003b
// 0b1001
#define GETRES 0x09
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

// MAX_SERVER_NUM <= 128
#define MAX_SERVER_NUM 128
// RANGE_PARTITION_ENTRY_NUM = 5 * MAX_SERVER_NUM < 8 * MAX_SERVER_NUM
#define RANGE_PARTITION_ENTRY_NUM 1024
// RANGE_PARTITION_FOR_SCAN_ENDKEY_ENTRY_NUM = 1 * MAX_SERVER_NUM
#define RANGE_PARTITION_FOR_SCAN_ENDKEY_ENTRY_NUM 128
// PROCESS_SCANREQ_SPLIT_ENTRY_NUM = 2 * MAX_SERVER_NUM
#define PROCESS_SCANREQ_SPLIT_ENTRY_NUM 256
// HASH_PARTITION_ENTRY_NUM = 4 * MAX_SERVER_NUM < 8 * MAX_SERVER_NUM
#define HASH_PARTITION_ENTRY_NUM 1024

// hash partition range
#define PARTITION_COUNT 32768

#define SWITCHIDX_FOREVAL 0xFFFF

// headers
#include "p4src/header.p4"

// parsers
#include "p4src/parser.p4"

#include "p4src/ingress_mat.p4"
#include "p4src/egress_mat.p4"

/* Ingress Processing */

control ingress {

	// Stage 0
	if (not valid(op_hdr)) {
		apply(l2l3_forward_tbl); // forward traditional packet
	}
#ifndef RANGE_SUPPORT
	apply(hash_for_partition_tbl); // for hash partition (including startkey of SCANREQ)
#endif

	// Stage 1 (not sure why we cannot place cache_lookup_tbl, hash_for_cm_tbl, and hash_for_seq_tbl in stage 1; follow automatic placement of tofino compiler)
#ifdef RANGE_SUPPORT
	apply(range_partition_tbl); // for range partition (GET/PUT/DEL)
#else
	apply(hash_partition_tbl);
#endif

	// Stage 2
#ifdef RANGE_SUPPORT
	apply(range_partition_for_scan_endkey_tbl); // perform range partition for endkey of SCANREQ
#endif

	// Stage 3
	apply(ipv4_forward_tbl); // update egress_port for normal/speical response packets

	// Stage 4
#ifdef RANGE_SUPPORT
	apply(ig_port_forward_tbl); // update op_hdr.optype
#endif
}

/* Egress Processing */

control egress {

	// Stage 0
#ifdef RANGE_SUPPORT
	apply(process_scanreq_split_tbl);
#endif

	// Stage 1
#ifdef RANGE_SUPPORT
	apply(lastscansplit_tbl); // including is_last_scansplit
#endif

	// Stage 2
#ifdef RANGE_SUPPORT
	apply(eg_port_forward_tbl); // including scan forwarding
#endif

	// stage 3
	// NOTE: resource in stage 11 is not enough for update_ipmac_src_port_tbl, so we place it into stage 10
	apply(update_ipmac_srcport_tbl); // Update ip, mac, and srcport for RES to client and notification to switchos

	// Stage 4
	apply(update_pktlen_tbl); // Update udl_hdr.hdrLen for pkt with variable-length value
}
