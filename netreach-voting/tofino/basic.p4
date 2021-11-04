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
#define GETREQ_S_TYPE 0x08
#define PUTREQ_GS_TYPE 0x09
#define PUTREQ_PS_TYPE 0x0a
#define DELREQ_S_TYPE 0x0b
#define GETRES_S_TYPE 0x0c
#define GETRES_NS_TYPE 0x0d
// Only used in switch
#define PUTREQ_U_TYPE 0x20
#define PUTREQ_RU_TYPE 0x21

#define CLONE_FOR_GETRES 1
#define CLONE_FOR_DELRES 2
#define CLONE_FOR_PUTRES 3

// NOTE: Here we use 8*2B keys, which occupies 2 stages
// NOTE: we only have 7.5 stages for val (at most 30 register arrays -> 120B val)
// NOTE: we only have 64 * 4B PHV (not T-PHV), we need 1 * 4B for op_hdr.type 
// (fields in the same ALU msut be in the same PHV group)
// -> (64-1)/2=31 -> 31*4=124B val -> 15*8=120B val
// 32K * (4*2B keylo + 4*2B keyhi + 96B val + 1bit valid)
//#define KV_BUCKET_COUNT 32768
#define KV_BUCKET_COUNT 1

// NOTE: you should change the two macros according to maximum val length
// VAL_PKTLEN: sizeof(vallen) + sizeof(val) + sizeof(seq) + sizeof(is_assigned)
// VAL_PKTLEN_MINUS_ONE: sizeof(vallen) + sizeof(val) + sizeof(seq) + sizeof(is_assigned) - sizeof(stat)
// KEY_VAL_PKTLEN_MINUS_ONE: + sizeof(evcited_key) + sizeof(vallen) + sizeof(val) + sizeof(seq) + sizeof(is_assigned) - sizeof(stat)
//#define VAL_PKTLEN 102
//#define VAL_PKTLEN_MINUS_ONE 101
#define VAL_PKTLEN 14
#define VAL_PKTLEN_MINUS_ONE 13
#define KEY_VAL_PKTLEN_MINUS_ONE 29

//#define CPU_PORT 192

// headers
#include "p4src/header.p4"

// parsers
#include "p4src/parser.p4"

#include "p4src/regs/seq.p4"
#include "p4src/regs/lock.p4"

// registers and MATs related with 16B key
#include "p4src/regs/key.p4"

// registers and MATs related with 1-bit valid
#include "p4src/regs/valid.p4"

// registers and MATs related with 124B val
#include "p4src/regs/val.p4"

// registers and MATs related with votes
#include "p4src/regs/vote.p4"

// registers and MATs related with dirty
#include "p4src/regs/dirty.p4"

#include "p4src/ingress_mat.p4"
#include "p4src/egress_mat.p4"

/* Ingress Processing */

action clone_putpkt(sid) {
	modify_field(meta.is_clone, 1);
	clone_ingress_pkt_to_egress(sid, clone_field_list);
}

table clone_putpkt_tbl {
	reads {
		ig_intr_md.ingress_port: exact;
	}
	actions {
		clone_putpkt;
		nop;
	}
	default_action: nop();
	size: 2;
}

action clone_delpkt(sid) {
	modify_field(meta.is_clone, 2);
	clone_ingress_pkt_to_egress(sid, clone_field_list);
}

table clone_delpkt_tbl {
	reads {
		ig_intr_md.ingress_port: exact;
	}
	actions {
		clone_delpkt;
		nop;
	}
	default_action: nop();
	size: 2;
}

action send_scanpkt() {
	modify_field(ig_intr_md_for_tm.copy_to_cpu, 1);
	//modify_field(ig_intr_md_for_tm.ucast_egress_port, CPU_PORT);
}

table send_scanpkt_tbl {
	actions {
		send_scanpkt;
	}
	default_action: send_scanpkt();
	size: 2;
}

action ig_drop_unicast() {
	modify_field(ig_intr_md_for_tm.drop_ctl, 1); // Disable unicast, but enable mirroring
}

table drop_put_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		ig_drop_unicast;
		nop;
	}
	default_action: nop();
	size: 8;  
}

control ingress {
	if (valid(op_hdr)) {

		// Stage 0
		apply(calculate_hash_tbl);
		apply(save_info_tbl);
		apply(initialize_tbl);
		apply(assign_seq_tbl);

		// Stage 1 and 2
		// Different MAT entries for getreq/putreq
		apply(access_keylololo_tbl);
		apply(access_keylolohi_tbl);
		apply(access_keylohilo_tbl);
		apply(access_keylohihi_tbl);
		apply(access_keyhilolo_tbl);
		apply(access_keyhilohi_tbl);
		apply(access_keyhihilo_tbl);
		apply(access_keyhihihi_tbl);

		// Stage 3
		// NOTE: we put valid_reg in stage 3 to support DEL operation
		apply(calculate_origin_hash_tbl);
		apply(access_valid_tbl); 
		apply(access_dirty_tbl); // we need dirty to decide whether to evict when cache update
		apply(access_savedseq_tbl);

		// Start from stage 4 (after keys and savedseq)
		// NOTE: we just get/put val directly; we decide whether to put the original val in getres or 
		// putreq in control flow
		apply(update_vallen_tbl);
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

		// Stage 4 + n (after keys and valid)
		apply(access_vote_tbl);

		// Stage 5 + n, where n is the number of stages for values
		// NOTE: it will change op_type from GETREQ/PUTREQ/DELREQ to
		// GETRES/PUTRES/DELREQ_S(cloned DELRES) only if valid = 1 and
		// key matches, which will not perform diff calculation, lock 
		// access, and cache update further. If optype is changed to RES,
		// port_forward_tbl will forward it as usual.
		apply(try_res_tbl);

		// NOTE: if packet arriving here is still GETREQ/PUTREQ/DELREQ,
		// it means that valid is 0, or valid is 1 yet key does not match.
		// So we do not need to compare whether key matches for the following
		// tables.

		// Stage 5+n + 1
		apply(access_lock_tbl);

		// Stage 5+n + 2 (trigger cache update)
		apply(trigger_cache_update_tbl);

		// (1) For GETRES_S, only if valid = 1 and dirty = 1. we convert it as PUTREQ_GS and forward to 
		// server, ans also clone a packet for GETRES to client; otherwise, we convert it as GETRES and
		// forward it as usual
		// (2) For GETRES_NS, directly convert it as GETRES and forward it as usual
		// (3) For PUTREQ_U, we convert it as PUTREQ_RU and recirculate it to update cache
		// (4) For PUTREQ_RU, we drop original packet or convert it to PUTREQ_PS (only if valid = 1 and dirty = 1) 
		// and forward to server, and also clone a packet for PUTRES to client
		// (5) For GETREQ, PUTREQ, and DELREQ, only if (lock = 1 and valid = 0) or (lock = 1 and valid = 1 
		// yet key does not match), we recirculate it. But NOTE that if valid = 1 and key matches, optype has
		// been set as RES by try_res_tbl. So if pkt arriving here is still REQ, it must satisfy (valid = 0)
		// or (valid = 1 and key does not match) -> we only need to check whether lock is 1! (TODO: we need 
		// local seq number here)
		// (6) For other packets, we set egress_port as usual
		apply(port_forward_tbl);

		if (op_hdr.optype == PUTREQ_GS_TYPE) {
			apply(origin_hash_partition_reverse_tbl); // update src port as meta.tmp_dport; update dst port as hash value of origin key (evicted key)
		}
		else if (op_hdr.optype == PUTREQ_PS_TYPE) {
			apply(origin_hash_partition_tbl); // update dst port of UDP according to hash value of origin key (evicted key)
		}
		else if (op_hdr.optype != PUTREQ_RU_TYPE){ // Only if dst port = server port: GETREQ, PUTREQ, DELREQ, SCANREQ, GETREQ_S, and DELREQ_S (without PUTREQ_U)
			apply(hash_partition_tbl); // update dst port of UDP according to hash value of key, only if dst_port = 1111 and egress_port and server port
		}

		if (ig_intr_md.resubmit_flag != 0) {
			apply(forward_to_server_tbl); // TMPDEBUG
		}
	}
}

/* Egress Processing */

control egress {
	// NOTE: make sure that normal packet will not apply these tables
	if (pkt_is_i2e_mirrored) {
		if (meta.is_clone == CLONE_FOR_GETRES) {
			apply(sendback_cloned_getres_tbl); // input is PUTREQ_GS converted from GETRES_S (we do not need swap port, ip, and mac)
		}
		else if (meta.is_clone == CLONE_FOR_DELRES) {
			apply(sendback_cloned_delres_tbl); // input is DELREQ_S converted from DELREQ (we need swap port, ip, and mac)
		}
		else if (meta.is_clone == CLONE_FOR_PUTRES) {
			apply(sendback_cloned_putres_tbl); // input is PUTREQ_RU/PUTREQ_PS converted from PUTREQ_RU from PUTREQ_U from PUTREQ (we need to swap port, ip, and mac)
		}
	}
	apply(update_macaddr_tbl); // Update mac addr for responses
}
