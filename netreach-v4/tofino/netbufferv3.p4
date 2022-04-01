#include "tofino/constants.p4"
#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"
#include "tofino/primitives.p4"

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11
//#define PROTOTYPE_NETBUFFER 0x90

#define OP_PORT 1111 // 0x0457
//#define OP_PORT_MASK 0xF400 // 1024 ~ 1279 // OP_PORT must be 0x04XX

// NOTE: convert type into Big Endian (not use threadid by now; big endian of key does not make sense)
#define GETREQ_TYPE 0x00
#define PUTREQ_TYPE 0x01
#define DELREQ_TYPE 0x02
#define SCANREQ_TYPE 0x03
#define GETRES_TYPE 0x04
#define PUTRES_TYPE 0x05
#define DELRES_TYPE 0x06
#define SCANRES_TYPE 0x07
#define GETREQ_POP_TYPE 0x08
#define GETRES_POP_TYPE 0x09
#define GETRES_NPOP_TYPE 0x0a
#define GETRES_POP_LARGE_TYPE 0x0b
#define GETRES_POP_EVICT_TYPE 0x0c
#define PUTREQ_SEQ_TYPE 0x0d 
#define PUTREQ_POP_TYPE 0x0e
#define PUTREQ_RECIR_TYPE 0x0f
#define PUTREQ_POP_EVICT_TYPE 0x10
#define PUTREQ_LARGE_TYPE 0x11
#define PUTREQ_LARGE_SEQ_TYPE 0x12
#define PUTREQ_LARGE_RECIR_TYPE 0x13
#define PUTREQ_LARGE_EVICT_TYPE 0x14
#define DELREQ_SEQ_TYPE 0x15
#define DELREQ_RECIR_TYPE 0x16
#define PUTREQ_CASE1_TYPE 0x17
#define DELREQ_CASE1_TYPE 0x18
#define GETRES_POP_EVICT_CASE2_TYPE 0x19
#define PUTREQ_POP_EVICT_CASE2_TYPE 0x1a
#define PUTREQ_LARGE_EVICT_CASE2_TYPE 0x1b
#define PUTREQ_CASE3_TYPE 0x1c
#define DELREQ_CASE3_TYPE 0x1d
#define PUTREQ_LARGE_CASE3_TYPE 0x1e
#define PUTRES_CASE3_TYPE 0x1f
#define DELRES_CASE3_TYPE 0x20
#define GETRES_POP_EVICT_SWITCH_TYPE 0x21
#define GETRES_POP_EVICT_CASE2_SWITCH_TYPE 0x22
#define PUTREQ_POP_EVICT_SWITCH_TYPE 0x23
#define PUTREQ_POP_EVICT_CASE2_SWITCH_TYPE 0x24
#define PUTREQ_LARGE_EVICT_SWITCH_TYPE 0x25
#define PUTREQ_LARGE_EVICT_CASE2_SWITCH_TYPE 0x26

// NOTE: Here we use 8*2B keys, which occupies 2 stages
// NOTE: we only have 7.5 stages for val (at most 30 register arrays -> 120B val)
// NOTE: we only have 64 * 4B PHV (not T-PHV), we need 1 * 4B for op_hdr.type 
// (fields in the same ALU msut be in the same PHV group)
// -> (64-1)/2=31 -> 31*4=124B val -> 15*8=120B val
// 32K * (4*2B keylo + 4*2B keyhi + 96B val + 1bit valid)
//#define KV_BUCKET_COUNT 32768
#define KV_BUCKET_COUNT 1

#define MAX_SERVERNUM 256

// NOTE: you should change the two macros according to maximum val length
// VAL_PKTLEN: sizeof(vallen) + sizeof(val), e.g., GETREQ -> GETRES
// VAL_PKTLEN_MINUS_STAT: sizeof(vallen) + sizeof(val) - sizeof(stat), e.g., PUTREQ -> PUTRES
// SEQ_PKTLEN: sizeof(seq_hdr), e.g., PUTREQ -> PUTREQ_RECIR, PUTREQ_RECIR -> PUTREQ_POP/PUTREQ
// VAL_PKTLEN_MINUS_STAT_PLUS_SEQ: sizeof(vallen) + sizeof(val) - sizeof(stat) + sizeof(seq), e.g., PUTREQ_RECIR -> PUTRES
// STAT_PKTLEN: sizeof(stat), e.g., DELREQ -> DELRES
// SEQ_PKTLEN_MINUS_STAT: sizeof(seq) - sizeof(stat), e.g., DELREQ_RECIR -> DELRES
// VAL_PKTLEN_MINUS_SEQ: sizeof(vallen) + sizeof(value) - sizeof(seq), e.g., DELREQ_RECIR -> DELREQ_CASE1
// OTHER_PKTLEN: sizeof(other), e.g., PUTREQ -> PUTREQ_MAY_CASE3
// SEQ_PKTLEN_MINUS_OTHER: sizeof(seq) - sizeof(other), e.g., PUTREQ_RECIR -> PUTREQ_MAY_CASE3
//#define VAL_PKTLEN 129
//#define VAL_PKTLEN_MINUS_STAT 128
//#define VAL_PKTLEN_MINUS_STAT_PLUS_SEQ 132
//#define VAL_PKTLEN_MINUS_SEQ 125
//#define VAL_PKTLEN 9
//#define VAL_PKTLEN_MINUS_STAT 8
//#define VAL_PKTLEN_MINUS_STAT_PLUS_SEQ 12
//#define VAL_PKTLEN_MINUS_SEQ 5
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

#include "p4src/regs/seq.p4"
#include "p4src/regs/lock.p4"
#include "p4src/regs/case12.p4"
#include "p4src/regs/case3.p4"

// registers and MATs related with 16B key
#include "p4src/regs/key.p4"

// registers and MATs related with 1-bit valid
#include "p4src/regs/valid.p4"

// registers and MATs related with 124B val
#include "p4src/regs/val.p4"

// registers and MATs related with votes
#include "p4src/regs/vote.p4"

#include "p4src/ingress_mat.p4"
#include "p4src/egress_mat.p4"

/* Ingress Processing */

control ingress {

	// Stage 0 
	apply(access_keylolo_tbl);
	apply(access_keylohi_tbl);
	apply(access_keyhilo_tbl);
	apply(access_keyhihi_tbl);
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
}

/* Egress Processing */

control egress {
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
