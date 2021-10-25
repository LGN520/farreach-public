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
#define PUTREQ_S_TYPE 0x08
#define DELREQ_S_TYPE 0x09

// NOTE: Here we use 8*2B keys, which occupies 2 stages
// NOTE: we only have 7.5 stages for val (at most 30 register arrays -> 120B val)
// NOTE: we only have 64 * 4B PHV (not T-PHV), we need 1 * 4B for op_hdr.type 
// (fields in the same ALU msut be in the same PHV group)
// -> (64-1)/2=31 -> 31*4=124B val -> 15*8=120B val
// 32K * (4*2B keylo + 4*2B keyhi + 96B val + 1bit valid)
//#define KV_BUCKET_COUNT 32768
#define KV_BUCKET_COUNT 1

//#define MAX_VAL_LEN 12
// NOTE: you should change the two macros according to maximum val length
//#define VAL_PKTLEN 97
//#define VAL_PKTLEN_MINUS_ONE 96
#define VAL_PKTLEN 9
#define VAL_PKTLEN_MINUS_ONE 8

//#define CPU_PORT 192

// headers
#include "p4src/header.p4"

// parsers
#include "p4src/parser.p4"

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

/* Ingress Processing */

action update_putreq() {
	modify_field(op_hdr.optype, PUTREQ_S_TYPE);

	modify_field(op_hdr.keylololo, meta.origin_keylololo);
	modify_field(op_hdr.keylolohi, meta.origin_keylolohi);
	modify_field(op_hdr.keylohilo, meta.origin_keylohilo);
	modify_field(op_hdr.keylohihi, meta.origin_keylohihi);
	modify_field(op_hdr.keyhilolo, meta.origin_keyhilolo);
	modify_field(op_hdr.keyhilohi, meta.origin_keyhilohi);
	modify_field(op_hdr.keyhihilo, meta.origin_keyhihilo);
	modify_field(op_hdr.keyhihihi, meta.origin_keyhihihi);
	modify_field(vallen_hdr.vallen, meta.origin_vallen);
	add_header(vallen_hdr);
	modify_field(val1_hdr.vallo, meta.origin_vallo1);
	modify_field(val1_hdr.valhi, meta.origin_valhi1);
	add_header(val1_hdr);
	/*modify_field(val2_hdr.vallo, meta.origin_vallo2);
	modify_field(val2_hdr.valhi, meta.origin_valhi2);
	add_header(val2_hdr);
	modify_field(val3_hdr.vallo, meta.origin_vallo3);
	modify_field(val3_hdr.valhi, meta.origin_valhi3);
	add_header(val3_hdr);
	modify_field(val4_hdr.vallo, meta.origin_vallo4);
	modify_field(val4_hdr.valhi, meta.origin_valhi4);
	add_header(val4_hdr);
	modify_field(val5_hdr.vallo, meta.origin_vallo5);
	modify_field(val5_hdr.valhi, meta.origin_valhi5);
	add_header(val5_hdr);
	modify_field(val6_hdr.vallo, meta.origin_vallo6);
	modify_field(val6_hdr.valhi, meta.origin_valhi6);
	add_header(val6_hdr);
	modify_field(val7_hdr.vallo, meta.origin_vallo7);
	modify_field(val7_hdr.valhi, meta.origin_valhi7);
	add_header(val7_hdr);
	modify_field(val8_hdr.vallo, meta.origin_vallo8);
	modify_field(val8_hdr.valhi, meta.origin_valhi8);
	add_header(val8_hdr);
	modify_field(val9_hdr.vallo, meta.origin_vallo9);
	modify_field(val9_hdr.valhi, meta.origin_valhi9);
	add_header(val9_hdr);
	modify_field(val10_hdr.vallo, meta.origin_vallo10);
	modify_field(val10_hdr.valhi, meta.origin_valhi10);
	add_header(val10_hdr);
	modify_field(val11_hdr.vallo, meta.origin_vallo11);
	modify_field(val11_hdr.valhi, meta.origin_valhi11);
	add_header(val11_hdr);
	modify_field(val12_hdr.vallo, meta.origin_vallo12);
	modify_field(val12_hdr.valhi, meta.origin_valhi12);
	add_header(val12_hdr);
	modify_field(val13_hdr.vallo, meta.origin_vallo13);
	modify_field(val13_hdr.valhi, meta.origin_valhi13);
	add_header(val13_hdr);
	modify_field(val14_hdr.vallo, meta.origin_vallo14);
	modify_field(val14_hdr.valhi, meta.origin_valhi14);
	add_header(val14_hdr);
	modify_field(val15_hdr.vallo, meta.origin_vallo15);
	modify_field(val15_hdr.valhi, meta.origin_valhi15);
	add_header(val15_hdr);
	modify_field(val16_hdr.vallo, meta.origin_vallo16);
	modify_field(val16_hdr.valhi, meta.origin_valhi16);
	add_header(val16_hdr);*/
}

table update_putreq_tbl {
	actions {
		update_putreq;
	}
	default_action: update_putreq();
	size: 1;
}

// NOTE: clone field list cannot exceed 32 bytes
field_list clone_field_list {
	meta.is_clone;
	meta.tmp_sport;
	meta.tmp_dport;
}

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

action hash_partition(port) {
	modify_field(udp_hdr.dstPort, port);
}

table hash_partition_tbl {
	reads {
		udp_hdr.dstPort: exact;
		ig_intr_md_for_tm.ucast_egress_port: exact;
		meta.hashidx: range;
	}
	actions {
		hash_partition;
	}
	size: 128;
}

control ingress {
	if (valid(op_hdr)) {

		// Stage 0
		apply(calculate_hash_tbl);
		apply(save_info_tbl); // save dst port
		apply(load_gthreshold_tbl);
		apply(load_pthreshold_tbl);

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
		apply(access_valid_tbl); 
		apply(access_dirty_tbl);
		apply(update_vallen_tbl);

		// Stage 4
		apply(access_gposvote_tbl);
		apply(access_gnegvote_tbl);
		apply(access_pposvote_tbl);
		apply(access_pnegvote_tbl);

		// Start from stage 5 (after keys, valid bits, and votes (occupying entire stage 4))
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

		// Stage 5 + n, where n is the number of stages for values
		// NOTE: it will change op_type from GETREQ/PUTREQ/DELREQ to
		// GETRES/PUTRES/DELREQ_S(cloned DELRES) if key matches, which
		// will not perform diff calculation, lock access, and cache update further
		apply(try_res_tbl);

		// Stage 5+n + 1
		// Do preliminary decision
		if (meta.isdirty == 1) {
			if (op_hdr.optype == GETREQ_TYPE) {
				if (meta.gnegvote > meta.pposvote) {
					apply(calculate_diff_tbl);
				}
			}
			else if (op_hdr.optype == PUTREQ_TYPE) {
				if (meta.pnegvote > meta.pposvote) {
					apply(calculate_diff_tbl);
				}
			}
		}
		else {
			if (op_hdr.optype == GETREQ_TYPE) {
				if (meta.gnegvote > meta.gposvote) {
					apply(calculate_diff_tbl);
				}
			}
			else if (op_hdr.optype == PUTREQ_TYPE) {
				if (meta.pnegvote > meta.gposvote) {
					apply(calculate_diff_tbl);
				}
			}
		}

		// Stage 5+n + 2
		apply(access_lock_tbl);

		// Stage 5+n + 3
		/*if (meta.islock == 0) {
			if (op_hdr.optype == GETREQ_TYPE) {
				if (meta.vote_diff >= meta.gthreshold) {
					// TODO: generate cache update for get
				}
			}
		}*/

		else if (op_hdr.optype == PUTREQ_TYPE) {
			// Stage 4 (rely on val)
			if (meta.isvalid == 1) { // pkt is cloned to egress and will not execute this code
				if (meta.origin_keylololo != op_hdr.keylololo) {
					apply(update_putreq_tbl);
				}
				else if (meta.origin_keylolohi != op_hdr.keylolohi) {
					apply(update_putreq_tbl);
				}
				else if (meta.origin_keylohilo != op_hdr.keylohilo) {
					apply(update_putreq_tbl);
				}
				else if (meta.origin_keylohihi != op_hdr.keylohihi) {
					apply(update_putreq_tbl);
				}
				else if (meta.origin_keyhilolo != op_hdr.keyhilolo) {
					apply(update_putreq_tbl);
				}
				else if (meta.origin_keyhilohi != op_hdr.keyhilohi) {
					apply(update_putreq_tbl);
				}
				else if (meta.origin_keyhihilo != op_hdr.keyhihilo) {
					apply(update_putreq_tbl);
				}
				else if (meta.origin_keyhihihi != op_hdr.keyhihihi) {
					apply(update_putreq_tbl);
				}
			}

			apply(clone_putpkt_tbl); // sendback PUTRES
			// NOTE: drop normal packet in ingress will not generate the cloned packet further (we need to set drop_ctl)
			// Only PUTREQ_S (aka meta.isvalid = 1 for PUT), port_forward; if PUTREQ (aka meta.isvalid = 0), drop
			apply(drop_put_tbl); 
			// NOTE: we must set egress port for normal packet; otherwise it will be dropped
			apply(port_forward_tbl);
		}
		else if (op_hdr.optype == SCANREQ_TYPE) {
			// Stage 3/0
			apply(send_scanpkt_tbl);
			apply(port_forward_tbl);
		}
		else {
			apply(port_forward_tbl);
		}
	}
	else {
		apply(port_forward_tbl);
	}

	apply(hash_partition_tbl); // update dst port of UDP according to hash value of key, only if dst_port = 1111 and egress_port and server port
}

/* Egress Processing */

/*action eg_drop_unicast {
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
}

table drop_put_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		eg_drop_unicast;
		nop;
	}
	default_action: nop();
	size: 4;
}*/

action sendback_delres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	add_to_field(udp_hdr.hdrlen, 1);

	modify_field(op_hdr.optype, DELRES_TYPE);
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

table sendback_delres_tbl {
	actions {
		sendback_delres;
	}
	default_action: sendback_delres();
	size: 1;
}

action swap_macaddr(tmp_srcmac, tmp_dstmac) {
	modify_field(ethernet_hdr.dstAddr, tmp_srcmac);
	modify_field(ethernet_hdr.srcAddr, tmp_dstmac);
}

table swap_macaddr_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		swap_macaddr;
		nop;
	}
	default_action: nop();
	size: 4;
}

control egress {
	// NOTE: make sure that normal packet will not apply these tables
	if (pkt_is_i2e_mirrored) {
		else if (meta.is_clone == 2) {
			apply(sendback_delres_tbl); // input is DELREQ or DELREQ_S
		}
	}
	/*else {
		apply(drop_put_tbl); // Drop PUTREQ (aka valid = 0 for PUT) 
	}*/
	apply(swap_macaddr_tbl); // Swap mac addr for res (not only PUTREQ and DELRES, but also GETRS)
}
