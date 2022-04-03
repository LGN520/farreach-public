#include "tofino/constants.p4"
#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"
#include "tofino/primitives.p4"

// 1B optype does not need endian conversion
#define GETREQ 0x00
#define PUTREQ 0x01
#define DELREQ 0x02
#define SCANREQ 0x03
#define GETRES 0x04
#define PUTRES 0x05
#define DELRES 0x06
#define SCANRES 0x07
#define GETREQ_INSWITCH 0x08

// NOTE: limited by 12 stages and 64*4B PHV (not T-PHV) (fields in the same ALU must be in the same PHV group)
// 32K * (4B vallen + 128B value + 4B frequency + 1B status)
//#define KV_BUCKET_COUNT 32768
#define KV_BUCKET_COUNT 1
// pipeline_num * kv_bucket_count
#define LOOKUP_ENTRY_COUNT 65536

// 64K * 2B counter
#define CM_BUCKET_COUNT 65536
#define HH_THRESHOLD 100

#define MAX_SERVERNUM 256

// NOTE: you should change the two macros according to maximum val length
// SEQ_PKTLEN: sizeof(seq_hdr), e.g., PUTREQ -> PUTREQ_RECIR, PUTREQ_RECIR -> PUTREQ_POP/PUTREQ
// STAT_PKTLEN: sizeof(stat), e.g., DELREQ -> DELRES
// SEQ_PKTLEN_MINUS_STAT: sizeof(seq) - sizeof(stat), e.g., DELREQ_RECIR -> DELRES
// OTHER_PKTLEN: sizeof(other), e.g., PUTREQ -> PUTREQ_MAY_CASE3
// SEQ_PKTLEN_MINUS_OTHER: sizeof(seq) - sizeof(other), e.g., PUTREQ_RECIR -> PUTREQ_MAY_CASE3
#define SEQ_PKTLEN 4
#define STAT_PKTLEN 1
#define SEQ_PKTLEN_MINUS_STAT 3
#define OTHER_PKTLEN 1
#define SEQ_PKTLEN_MINUS_OTHER 3
#define SERVERIDX_PKTLEN 2

//#define CPU_PORT 192

// headers
#include "p4src/header.p4"

// parsers
#include "p4src/parser.p4"

// registers and MATs
#include "p4src/regs/cm.p4"
#include "p4src/regs/cache_frequency.p4"
#include "p4src/regs/valid.p4"


#include "p4src/regs/seq.p4"
#include "p4src/regs/lock.p4"
#include "p4src/regs/case12.p4"
#include "p4src/regs/case3.p4"

// registers and MATs related with 124B val
#include "p4src/regs/val.p4"

// registers and MATs related with votes
#include "p4src/regs/vote.p4"

#include "p4src/ingress_mat.p4"
#include "p4src/egress_mat.p4"

/* Ingress Processing */

control ingress {

	// Stage 0
	apply(cache_lookup_tbl); // managed by controller
	apply(hash_tbl); // for both partition and CM
	apply(sample_tbl); // for CM and cache_frequency

	// Stage 1
	apply(hash_partition_tbl);

	// Stgae 2
	apply(ig_port_forward_tbl);
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
	apply(access_valid_tbl);

	// Stage 2
	apply(access_latest_tbl);
	apply(access_deleted_tbl);










	// Stage 0 
	apply(save_info_tbl);
	apply(initialize_tbl);
	apply(load_backup_flag_tbl);

	// Stage 1
	apply(access_valid_tbl); 
	apply(access_vote_tbl);
	apply(assign_seq_tbl);
	apply(update_iskeymatch_tbl);

	// Stage 2
	apply(access_savedseq_tbl);
	apply(access_lock_tbl);

	// Stage 3
	// Case 1 of backup: first matched PUT/DEL of this bucket
	// Case 2 of backup: cache population by GETRES_POP or PUTREQ_POP
	apply(access_case12_tbl); 

	// Start from stage 3
	apply(update_vallen_tbl);
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
	apply(update_vallo13_tbl);
	apply(update_valhi13_tbl);
	apply(update_vallo14_tbl);
	apply(update_valhi14_tbl);
	apply(update_vallo15_tbl);
	apply(update_valhi15_tbl);
	apply(update_vallo16_tbl);
	apply(update_valhi16_tbl);

	// Stage 11
	apply(port_forward_tbl);

	// Stage 0
	// NOTE: make sure that normal packet will not apply these tables
	if (pkt_is_i2e_mirrored) {
		apply(process_i2e_cloned_packet_tbl);
	}
	else if (pkt_is_e2e_mirrored) {
		apply(process_e2e_cloned_packet_tbl);
	}

	// NOTE: PUTREQ_LARGE_SEQ from cloend PUTREQ_LARGE/RECIR and EVICT/CASE2_SWITCH from cloned EVICT/CASE2 should access following MATs

	// Stage 1
	// NOTE: for packets requring origin_hash, the key and value in packet header have already been set as origin_key/val
	apply(eg_calculate_hash_tbl);
	if (op_hdr.optype == GETRES_POP_EVICT_TYPE) {
		apply(hash_partition_reverse_tbl); // update src port as meta.tmp_dport; update dst port as hash value of origin key (evicted key)
	}
	else if (op_hdr.optype == GETRES_POP_EVICT_CASE2_TYPE) {
		apply(hash_partition_reverse_tbl); // update src port as meta.tmp_dport; update dst port as hash value of origin key (evicted key)
	}
	else if (op_hdr.optype != SCANREQ_TYPE){ // NOTE: even we invoke this MAT for PUTREQ_U, it does not affect the recirculated packet (PUTREQ + meta.is_putreq_ru of 1)
		apply(hash_partition_tbl); // update dst port of UDP according to hash value of key, only if dst_port = 1111 and egress_port and server port
	}

	// Stage 2
	apply(access_case3_tbl);

	// Stage 3
	apply(eg_port_forward_tbl);

	// Stage 4
	// The same stage
	apply(update_udplen_tbl); // Update udl_hdr.hdrLen for pkt with variable-length value
	apply(update_macaddr_tbl); // Update mac addr for responses and PUTREQ_GS/GS_CASE2
}
