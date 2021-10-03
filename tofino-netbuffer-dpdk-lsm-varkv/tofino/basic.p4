#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11
//#define PROTOTYPE_NETBUFFER 0x90

#define OP_PORT 1111 // 0x0457
#define OP_PORT_MASK 0xF400 // 1024 ~ 1279 // OP_PORT must be 0x04XX

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
// 32K * (4*2B keylo + 4*2B keyhi + 120B val + 1bit valid)
#define KV_BUCKET_COUNT 32768
//#define KV_BUCKET_COUNT 8

//#define CPU_PORT 192

// headers
#include "p4src/header.p4"

header udp_t udp_hdr;
header op_t op_hdr;
header vallen_t vallen_hdr;
header val_t val1_hdr;
header val_t val2_hdr;
header val_t val3_hdr;
header val_t val4_hdr;
header val_t val5_hdr;
header val_t val6_hdr;
header val_t val7_hdr;
header val_t val8_hdr;
header val_t val9_hdr;
header val_t val10_hdr;
header val_t val11_hdr;
header val_t val12_hdr;
/*header val_t val13_hdr;
header val_t val14_hdr;
header val_t val15_hdr;
header val_t val16_hdr;*/
header res_t res_hdr;
metadata metadata_t meta;

// parsers
#include "p4src/parser.p4"

/* Ingress Processing (Normal Operation) */

action nop() {}

action port_forward(port) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

action droppkt() {
	drop();
}

table port_forward_tbl {
	reads {
		ig_intr_md.ingress_port: exact;
	}
	actions {
		port_forward;
		droppkt;
		nop;
	}
	default_action: nop();
	size: 4;  
}

/* Hash */

field_list hash_fields {
	op_hdr.keylololo;
	op_hdr.keylolohi;
	op_hdr.keylohilo;
	op_hdr.keylohihi;
	op_hdr.keyhilolo;
	op_hdr.keyhilohi;
	op_hdr.keyhihilo;
	op_hdr.keyhihihi;
}

field_list_calculation hash_field_calc {
	input {
		hash_fields;
	}
	algorithm: crc32;
	output_width: 16;
}

action calculate_hash() {
	modify_field_with_hash_based_offset(meta.hashidx, 0, hash_field_calc, KV_BUCKET_COUNT);
	// NOTE: we cannot use dynamic hash
	// modify_field_with_hash_based_offset(meta.hashidx, 0, hash_field_calc, KV_BUCKET_COUNT - ipv4_hdr.totalLen);
}

//@pragma stage 0
table calculate_hash_tbl {
	actions {
		calculate_hash;
	}
	default_action: calculate_hash();
	size: 1;
}

action save_info() {
	modify_field(meta.tmp_sport, udp_hdr.srcPort);
	modify_field(meta.tmp_dport, udp_hdr.dstPort);
}

//@pragma stage 0
table save_info_tbl {
	actions {
		save_info;
	}
	default_action: save_info();
	size: 1;
}

/* KV (hash table) */

// registers and MATs related with 16B key
#include "p4src/regs/key.p4"

// registers and MATs related with 1-bit valid
#include "p4src/regs/valid.p4"

// registers and MATs related with 124B val
#include "p4src/regs/val.p4"

/* Ingress Processing */

action sendback_getres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);

	modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);

	modify_field(op_hdr.optype, GETRES_TYPE);
	modify_field(vallen_hdr.vallen, meta.origin_vallen);
	add_header(vallen_hdr);
	modify_field(val1_hdr.vallo, meta.origin_vallo1);
	modify_field(val1_hdr.valhi, meta.origin_valhi1);
	add_header(val1_hdr);
	modify_field(val2_hdr.vallo, meta.origin_vallo2);
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
	/*modify_field(val13_hdr.vallo, meta.origin_vallo13);
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

table sendback_getres_tbl {
	actions {
		sendback_getres;
	}
	default_action: sendback_getres();
	size: 1;
}

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
	modify_field(val2_hdr.vallo, meta.origin_vallo2);
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
	/*modify_field(val13_hdr.vallo, meta.origin_vallo13);
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

action update_delreq() {
	modify_field(op_hdr.optype, DELREQ_S_TYPE);
}

table update_delreq_tbl {
	actions {
		update_delreq;
	}
	default_action: update_delreq();
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

control ingress {
	if (valid(op_hdr)) {

		// Stage 0
		apply(calculate_hash_tbl);
		apply(save_info_tbl);

		// Stage 1 and 2
		// Different MAT entries for getreq/putreq
		apply(match_keylololo_tbl);
		apply(match_keylolohi_tbl);
		apply(match_keylohilo_tbl);
		apply(match_keylohihi_tbl);
		apply(match_keyhilolo_tbl);
		apply(match_keyhilohi_tbl);
		apply(match_keyhihilo_tbl);
		apply(match_keyhihihi_tbl);

		// Stage 3
		// NOTE: we put valid_reg in stage 2 to support DEL operation
		apply(access_valid_tbl); 
		apply(update_vallen_tbl);

		// Start from stage 3
		// NOTE: we just get/put val directly; we decide whether to put the original val in getres or 
		// putreq in control flow
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
		/*apply(update_vallo13_tbl);
		apply(update_valhi13_tbl);
		apply(update_vallo14_tbl);
		apply(update_valhi14_tbl);
		apply(update_vallo15_tbl);
		apply(update_valhi15_tbl);
		apply(update_vallo16_tbl);
		apply(update_valhi16_tbl);*/

		if (op_hdr.optype == GETREQ_TYPE) {
			// Stage 4 (rely on val)
			if (meta.isvalid == 1) {
				if (meta.ismatch_keylololo == 2 and meta.ismatch_keylolohi == 2) {
					if (meta.ismatch_keylohilo == 2 and meta.ismatch_keylohihi == 2) {
						if (meta.ismatch_keyhilolo == 2 and meta.ismatch_keyhilohi == 2) {
							if (meta.ismatch_keyhihilo == 2 and meta.ismatch_keyhihihi == 2) {
								apply(sendback_getres_tbl);
							}
							else {
								apply(port_forward_tbl);
							}
						}
						else {
							apply(port_forward_tbl);
						}
					}
					else {
						apply(port_forward_tbl);
					}
				}
				else {
					apply(port_forward_tbl);
				}
			}
			else {
				apply(port_forward_tbl);
			}
		}
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

			apply(clone_putpkt_tbl);
			apply(port_forward_tbl);
		}
		else if (op_hdr.optype == DELREQ_TYPE) {
			// Stage 3
			if (meta.isvalid == 1) { // Only if key matches and original valid bit is 1, meta.isvalid is 1
				apply(update_delreq_tbl);
				apply(clone_delpkt_tbl);
			}
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
}

/* Egress Processing */

action sendback_putres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);

	remove_header(vallen_hdr);
	remove_header(val1_hdr);
	remove_header(val2_hdr);
	remove_header(val3_hdr);
	remove_header(val4_hdr);
	remove_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	/*remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);*/
	modify_field(op_hdr.optype, PUTRES_TYPE);
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

table sendback_putres_tbl {
	actions {
		sendback_putres;
	}
	default_action: sendback_putres();
	size: 1;
}

action sendback_delres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);

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

control egress {
	if (meta.is_clone == 1) {
		apply(sendback_putres_tbl);
	}
	else if (meta.is_clone == 2) {
		apply(sendback_delres_tbl);
	}
}
