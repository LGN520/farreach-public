#include "tofino/constants.p4"
#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"
#include "tofino/primitives.p4"

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11
//#define PROTOTYPE_NETBUFFER 0x90

// NOTE: convert type into Big Endian (not use threadid by now; big endian of key does not make sense)
#define GETREQ_TYPE 0x00
#define PUTREQ_TYPE 0x01
#define DELREQ_TYPE 0x02
#define SCANREQ_TYPE 0x03
#define GETRES_TYPE 0x04
#define PUTRES_TYPE 0x05
#define DELRES_TYPE 0x06
#define SCANRES_TYPE 0x07
#define GETREQ_S_TYPE 0x08

// NOTE: Here we use 8*2B keys, which occupies 2 stages
// NOTE: we only have 7.5 stages for val (at most 30 register arrays -> 120B val)
// NOTE: we only have 64 * 4B PHV (not T-PHV), we need 1 * 4B for op_hdr.type 
// (fields in the same ALU msut be in the same PHV group)
// -> (64-1)/2=31 -> 31*4=124B val -> 15*8=120B val
// 32K * (4*2B keylo + 4*2B keyhi + 96B val + 1bit valid)
//#define KV_BUCKET_COUNT 32768
#define KV_BUCKET_COUNT 1

// NOTE: you should change the two macros according to maximum val length
// VAL_PKTLEN: sizeof(vallen) + sizeof(val)
// VAL_PKTLEN_MINUS_ONE: sizeof(vallen) + sizeof(val) - sizeof(stat)
//#define VAL_PKTLEN 97
//#define VAL_PKTLEN_MINUS_ONE 96
#define VAL_PKTLEN 9
#define VAL_PKTLEN_MINUS_ONE 8

//#define CPU_PORT 192

// headers
#include "p4src/header.p4"

// parsers
#include "p4src/parser.p4"

#include "p4src/regs/valid.p4"
#include "p4src/regs/vote.p4"
#include "p4src/regs/lock.p4"
#include "p4src/regs/latest.p4"
#include "p4src/regs/val.p4"

#include "p4src/ingress_mat.p4"
#include "p4src/egress_mat.p4"

/* Ingress Processing */

control ingress {
	// Stage 0
	apply(calculate_hash_tbl);
	apply(cache_lookup_tbl);
	apply(save_info_tbl);
	apply(initialize_tbl);
	apply(load_backup_flag_tbl);

	// Stage 1 (rely on hashidx)
	apply(access_valid_tbl); 
	//apply(access_dirty_tbl);
	apply(access_vote_tbl);
	apply(access_being_evicted_tbl);

	// Stage 2 (rely on valid and vote)
	apply(access_lock_tbl);
	apply(update_vallen_tbl);
	//apply(access_case_tbl); // Case 1/3 for backup
	apply(access_latest_tbl);

	// Start from stage 4 (after keys and savedseq)
	// NOTE: we just get/put val directly; we decide whether to put the original val in getres or 
	// putreq in control flow
	apply(update_vallo1_tbl);
	apply(update_valhi1_tbl);
	/*apply(update_vallo2_tbl);
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
	apply(update_valhi16_tbl);*/

	// Stage 11
	// GETREQ: if iscached=1 and isvalid=1, sendback GETRES; if isvalid=0 and islock=0, 
	// or iszerovote=2 and islock=0, send GETREQ_S for eviction; otherwise, forward GETREQ to server
	apply(port_forward_tbl);
}

/* Egress Processing */

control egress {
	// NOTE: make sure that normal packet will not apply these tables
	if (pkt_is_i2e_mirrored) {
		if (meta.is_clone == CLONE_FOR_GETRES) {
			apply(sendback_cloned_getres_tbl); // input is GETRES_S (original pkt becomes PUTREQ_GS/PUTREQ_GS_CASE2) (we do not need swap port, ip, and mac)
		}
		else if (meta.is_clone == CLONE_FOR_DELRES) {
			apply(sendback_cloned_delres_tbl); // input is DELREQ (original pkt becomes DELREQ_S/DELREQ_CASE1) (we need swap port, ip, and mac)
		}
		else if (meta.is_clone == CLONE_FOR_PUTRES) {
			apply(sendback_cloned_putres_tbl); // input is PUTREQ (original pkt becomes PUTREQ_PS/PUTREQ_CASE1/PUTREQ_PS_CASE2) (we need to swap port, ip, and mac)
		}
	}
	else {
		// NOTE: for packets requring origin_hash, the key and value in packet header have already been set as origin_key/val
		apply(eg_calculate_hash_tbl);
		if (op_hdr.optype == PUTREQ_GS_TYPE) {
			apply(hash_partition_reverse_tbl); // update src port as meta.tmp_dport; update dst port as hash value of origin key (evicted key)
		}
		else if (op_hdr.optype == PUTREQ_GS_CASE2_TYPE) {
			apply(hash_partition_reverse_tbl); // update src port as meta.tmp_dport; update dst port as hash value of origin key (evicted key)
		}
		else if (op_hdr.optype != SCANREQ_TYPE){ // NOTE: even we invoke this MAT for PUTREQ_U, it does not affect the recirculated packet (PUTREQ + meta.is_putreq_ru of 1)
			apply(hash_partition_tbl); // update dst port of UDP according to hash value of key, only if dst_port = 1111 and egress_port and server port
		}
	}
	apply(update_macaddr_tbl); // Update mac addr for responses and PUTREQ_GS/GS_CASE2
}
