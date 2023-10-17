// #include "tofino/constants.p4"
// #include "tofino/intrinsic_metadata.p4"
// #include "tofino/stateful_alu_blackbox.p4"
// #include "tofino/primitives.p4"

#include <core.p4>
#include <v1model.p4>

// macro
#include "p4src/macro.p4"
// headers
#include "p4src/header.p4"

// parsers
#include "p4src/parser.p4"

#include "p4src/ingress_mat.p4"
#include "p4src/egress_mat.p4"


/* Ingress Processing */

// control ingress {

// 	// Stage 0
// 	if (not valid(op_hdr)) {
// 		apply(l2l3_forward_tbl); // forward traditional packet
// 	}
// #ifndef RANGE_SUPPORT
// 	apply(hash_for_partition_tbl); // for hash partition (including startkey of SCANREQ)
// #endif

// 	// Stage 1 (not sure why we cannot place cache_lookup_tbl, hash_for_cm_tbl, and hash_for_seq_tbl in stage 1; follow automatic placement of bmv2 compiler)
// #ifdef RANGE_SUPPORT
// 	apply(range_partition_tbl); // for range partition (GET/PUT/DEL)
// #else
// 	apply(hash_partition_tbl);
// #endif

// 	// Stage 2
// #ifdef RANGE_SUPPORT
// 	apply(range_partition_for_scan_endkey_tbl); // perform range partition for endkey of SCANREQ
// #endif

// 	// Stage 3
// 	apply(ipv4_forward_tbl); // update egress_port for normal/speical response packets

// 	// Stage 4
// #ifdef RANGE_SUPPORT
// 	apply(ig_port_forward_tbl); // update op_hdr.optype
// #endif
// }

/* Egress Processing */

// control egress {

// 	// Stage 0
// #ifdef RANGE_SUPPORT
// 	apply(process_scanreq_split_tbl);
// #endif

// 	// Stage 1
// #ifdef RANGE_SUPPORT
// 	apply(lastscansplit_tbl); // including is_last_scansplit
// #endif

// 	// Stage 2
// #ifdef RANGE_SUPPORT
// 	apply(eg_port_forward_tbl); // including scan forwarding
// #endif

// 	// stage 3
// 	// NOTE: resource in stage 11 is not enough for update_ipmac_src_port_tbl, so we place it into stage 10
// 	apply(update_ipmac_srcport_tbl); // Update ip, mac, and srcport for RES to client and notification to switchos

// 	// Stage 4
// 	apply(update_pktlen_tbl); // Update udl_hdr.hdrLen for pkt with variable-length value
// }

//blank checksum
control nocahceVerifyChecksum(inout headers hdr, inout metadata meta) {
    apply {  }
}
control nocahceComputeChecksum(inout headers hdr, inout metadata meta) {
    apply {    }
}
//switch architecture
V1Switch(
nocahceParser(),
nocahceVerifyChecksum(),
nocahceIngress(),
nocahceEgress(),
nocahceComputeChecksum(),
nocahceDeparser()
) main;