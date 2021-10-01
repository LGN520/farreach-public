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
#define GETREQ_TYPE 0x00000000
#define PUTREQ_TYPE 0x01000000
#define DELREQ_TYPE 0x02000000
#define SCANREQ_TYPE 0x03000000
#define GETRES_TYPE 0x04000000
#define PUTRES_TYPE 0x05000000
#define DELRES_TYPE 0x06000000
#define SCANRES_TYPE 0x07000000
#define PUTREQ_S_TYPE 0x08000000
#define DELREQ_S_TYPE 0x09000000

// 64K * (2*4B keylo + 2*4B keyhi + 4B + 1B)
#define KV_BUCKET_COUNT 32768
//#define KV_BUCKET_COUNT 65536
//#define KV_BUCKET_COUNT 8

//#define CPU_PORT 192

/* Packet Header Types */

header_type ethernet_t {
	fields {
		dstAddr: 48;
		srcAddr: 48;
		etherType: 16;
	}
}

header_type ipv4_t {
	fields {
		version: 4;
		ihl: 4;
		diffserv: 8;
		totalLen: 16;
		identification: 16;
		flags: 3;
		fragOffset: 13;
		ttl: 8;
		protocol: 8;
		hdrChecksum: 16;
		srcAddr: 32;
		dstAddr: 32;
	}
}

header_type udp_t {
	fields {
		srcPort: 16;
		dstPort: 16;
		hdrLength: 16;
		checksum: 16;
	}
}

header_type op_t {
	fields {
		optype: 32;
		threadid: 32;
		keylolo: 32;
		keylohi: 32;
		keyhilo: 32;
		keyhihi: 32;
	}
}

header_type putreq_t {
	fields {
		vallo: 32;
		valhi: 32;
	}
}

header_type scanreq_t {
	fields {
		num: 32;
	}
}

header_type getres_t {
	fields {
		vallo: 32;
		valhi: 32;
	}
}

header_type putres_t {
	fields {
		stat: 8;
	}
}

header_type delres_t {
	fields {
		stat: 8;
	}
}

header_type metadata_t {
	fields {
		hashidx: 32;
		ismatch_keylolo: 4; // predicate 
		ismatch_keylohi: 4; // predicate 
		ismatch_keyhilo: 4; // predicate
		ismatch_keyhihi: 4; // predicate
		isvalid: 1;
		origin_keylolo: 32;
		origin_keylohi: 32;
		origin_keyhilo: 32;
		origin_keyhihi: 32;
		origin_vallo: 32;
		origin_valhi: 32;
		tmp_sipaddr: 32;
		tmp_dipaddr: 32;
		tmp_sport: 16;
		tmp_dport: 16;
		tmp_port: 16;
		is_clone: 2;
	}
}

header ethernet_t ethernet_hdr;
header ipv4_t ipv4_hdr;
header udp_t udp_hdr;
header op_t op_hdr;
header putreq_t putreq_hdr;
header scanreq_t scanreq_hdr;
header getres_t getres_hdr;
header putres_t putres_hdr;
header delres_t delres_hdr;
metadata metadata_t meta;

/* Parser */

parser start {
	return parse_ethernet;
}

parser parse_ethernet {
	extract(ethernet_hdr);
	return select(ethernet_hdr.etherType) {
		ETHERTYPE_IPV4: parse_ipv4;
		default: ingress;
	}
}

parser parse_ipv4 {
	extract(ipv4_hdr);
	return select(ipv4_hdr.protocol) {
		PROTOTYPE_UDP: parse_udp;
		//PROTOTYPE_NETBUFFER: parse_op;
		default: ingress;
	}
}

parser parse_udp {
	extract(udp_hdr);
	return select(udp_hdr.dstPort) {
		//OP_PORT: parse_op;
		OP_PORT mask OP_PORT_MASK: parse_op;
		default: ingress;
	}
}

parser parse_op {
	extract(op_hdr);
	return select(op_hdr.optype) {
		PUTREQ_TYPE: parse_putreq;
		SCANREQ_TYPE: parse_scanreq;
		GETRES_TYPE: parse_getres;
		PUTRES_TYPE: parse_putres;
		DELRES_TYPE: parse_delres;
		default: ingress;
	}
}

parser parse_putreq {
	extract(putreq_hdr);
	return ingress;
}

parser parse_scanreq {
	extract(scanreq_hdr);
	return ingress;
}

parser parse_getres {
	extract(getres_hdr);
	return ingress;
}

parser parse_putres {
	extract(putres_hdr);
	return ingress;
}
parser parse_delres {
	extract(delres_hdr);
	return ingress;
}

/* Ingress Processing (Normal Operation) */

field_list ipv4_field_list {
    ipv4_hdr.version;
    ipv4_hdr.ihl;
    ipv4_hdr.diffserv;
    ipv4_hdr.totalLen;
    ipv4_hdr.identification;
    ipv4_hdr.flags;
    ipv4_hdr.fragOffset;
    ipv4_hdr.ttl;
    ipv4_hdr.protocol;
    ipv4_hdr.srcAddr;
    ipv4_hdr.dstAddr;
}

field_list_calculation ipv4_chksum_calc {
    input {
        ipv4_field_list;
    }
#ifndef __p4c__
    algorithm : csum16;
#else
    algorithm : crc16;
#endif
    output_width: 16;
}

calculated_field ipv4_hdr.hdrChecksum {
    update ipv4_chksum_calc;
}

action nop() {}


action droppkt() {
	drop();
}

action ipv4_forward(port) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
	//add_to_field(ipv4_hdr.ttl, -1);
}

table ipv4_lpm {
	reads {
		ipv4_hdr.dstAddr: lpm;
	}
	actions {
		ipv4_forward;
		droppkt;
	}
	default_action: droppkt();
	size: 1024;
}

/* Hash */

field_list hash_fields {
	op_hdr.keylolo;
	op_hdr.keylohi;
	op_hdr.keyhilo;
	op_hdr.keyhihi;
}

field_list_calculation hash_field_calc {
	input {
		hash_fields;
	}
	algorithm: crc32;
	output_width: 32;
}

action calculate_hash() {
	modify_field_with_hash_based_offset(meta.hashidx, 0, hash_field_calc, KV_BUCKET_COUNT);
}

//@pragma stage 0
table calculate_hash_tbl {
	actions {
		calculate_hash;
	}
	default_action: calculate_hash();
}

action save_info() {
	modify_field(meta.tmp_sipaddr, ipv4_hdr.srcAddr);
	modify_field(meta.tmp_dipaddr, ipv4_hdr.dstAddr);
	modify_field(meta.tmp_sport, udp_hdr.srcPort);
	modify_field(meta.tmp_dport, udp_hdr.dstPort);
}

//@pragma stage 0
table save_info_tbl {
	actions {
		save_info;
	}
	default_action: save_info();
}

/* KV (hash table) */

// registers and MATs related with 16B key
#include "p4src/key.p4"

// registers and MATs related with 1-bit valid
#include "p4src/valid.p4"

register vallo_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo_alu {
	reg: vallo_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo;
}

action get_vallo() {
	get_vallo_alu.execute_stateful_alu(meta.hashidx);
}

/*table get_vallo_tbl {
	actions {
		get_vallo;
	}
	default_action: get_vallo();
}*/

blackbox stateful_alu put_vallo_alu {
	reg: vallo_reg;

	update_lo_1_value: putreq_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo;
}

action put_vallo() {
	put_vallo_alu.execute_stateful_alu(meta.hashidx);
}

/*@pragma stage 3
table put_vallo_tbl {
	actions {
		put_vallo;
	}
	default_action: put_vallo();
}*/

//@pragma stage 3
table update_vallo_tbl {
	reads {
		op_hdr.optype: exact;
		meta.isvalid: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
	}
	actions {
		get_vallo;
		put_vallo;
		nop;
	}
	default_action: nop();
}

register valhi_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi_alu {
	reg: valhi_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi;
}

action get_valhi() {
	get_valhi_alu.execute_stateful_alu(meta.hashidx);
}

/*table get_valhi_tbl {
	actions {
		get_valhi;
	}
	default_action: get_valhi();
}*/

blackbox stateful_alu put_valhi_alu {
	reg: valhi_reg;

	update_lo_1_value: putreq_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi;
}

action put_valhi() {
	put_valhi_alu.execute_stateful_alu(meta.hashidx);
}

/*@pragma stage 3
table put_valhi_tbl {
	actions {
		put_valhi;
	}
	default_action: put_valhi();
}*/

//@pragma stage 3
table update_valhi_tbl {
	reads {
		op_hdr.optype: exact;
		meta.isvalid: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
	}
	actions {
		get_valhi;
		put_valhi;
		nop;
	}
	default_action: nop();
}

/* Ingress Processing */

action sendback_getres(tmp_smacaddr, tmp_dmacaddr) {
	// Swap mac address
	modify_field(ethernet_hdr.srcAddr, tmp_dmacaddr);
	modify_field(ethernet_hdr.dstAddr, tmp_smacaddr);

	// Swap ip address
	modify_field(ipv4_hdr.srcAddr, meta.tmp_dipaddr);
	modify_field(ipv4_hdr.dstAddr, meta.tmp_sipaddr);
	add_to_field(ipv4_hdr.totalLen, 8); // Big endian: add an 8B value
	
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	add_to_field(udp_hdr.hdrLength, 8); // Big endian: add an 8B value
	//modify_field(udp_hdr.hdrLength, 0x28); // 8B udp + 24B OP (4B optype + 4B threadid + 16B key) + 8B val

	modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);

	modify_field(op_hdr.optype, GETRES_TYPE);
	modify_field(getres_hdr.vallo, meta.origin_vallo);
	modify_field(getres_hdr.valhi, meta.origin_valhi);
	add_header(getres_hdr);
}

table sendback_getres_tbl {
	reads {
		ig_intr_md.ingress_port: exact;
	}
	actions {
		sendback_getres;
		nop;
	}
	default_action: nop();
}

action sendback_putres(tmp_smacaddr, tmp_dmacaddr) {
	// Swap mac address
	modify_field(ethernet_hdr.srcAddr, tmp_dmacaddr);
	modify_field(ethernet_hdr.dstAddr, tmp_smacaddr);

	// Swap ip address
	modify_field(ipv4_hdr.srcAddr, meta.tmp_dipaddr);
	modify_field(ipv4_hdr.dstAddr, meta.tmp_sipaddr);
	add_to_field(ipv4_hdr.totalLen, 1); // 1B status
	//modify_field(ipv4_hdr.totalLen, 53); // Big endian: 20B IP + 8B UDP + 24B OP + 1B status
	
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	add_to_field(udp_hdr.hdrLength, 1); // 1B status
	//modify_field(udp_hdr.hdrLength, 33); // 8B UDP + 24B OP + 1B status

	modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);

	modify_field(op_hdr.optype, PUTRES_TYPE);
	modify_field(putres_hdr.stat, 1);
	remove_header(putreq_hdr);
	add_header(putres_hdr);
}

table sendback_putres_tbl {
	reads {
		ig_intr_md.ingress_port: exact;
	}
	actions {
		sendback_putres;
		nop;
	}
	default_action: nop();
}

action sendback_delres(tmp_smacaddr, tmp_dmacaddr) {
	// Swap mac address
	modify_field(ethernet_hdr.srcAddr, tmp_dmacaddr);
	modify_field(ethernet_hdr.dstAddr, tmp_smacaddr);

	// Swap ip address
	modify_field(ipv4_hdr.srcAddr, meta.tmp_dipaddr);
	modify_field(ipv4_hdr.dstAddr, meta.tmp_sipaddr);
	add_to_field(ipv4_hdr.totalLen, 1); // 1B status
	//modify_field(ipv4_hdr.totalLen, 53); // Big endian: 20B IP + 8B UDP + 24B OP + 1B status
	
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	add_to_field(udp_hdr.hdrLength, 1); // 1B status
	//modify_field(udp_hdr.hdrLength, 0x33); // 8B UDP + 24B OP + 1B status

	modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);

	modify_field(op_hdr.optype, DELRES_TYPE);
	modify_field(delres_hdr.stat, 1);
	add_header(delres_hdr);
}

table sendback_delres_tbl {
	reads {
		ig_intr_md.ingress_port: exact;
	}
	actions {
		sendback_delres;
		nop;
	}
	default_action: nop();
}

field_list clone_put_field_list {
	meta.origin_keylolo;
	meta.origin_keylohi;
	meta.origin_keyhilo;
	meta.origin_keyhihi;
	meta.origin_vallo;
	meta.origin_valhi;
	meta.is_clone;
}

action clone_putpkt(sid) {
	modify_field(meta.is_clone, 1);
	clone_ingress_pkt_to_egress(sid, clone_put_field_list);
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
}

field_list clone_del_field_list {
	meta.is_clone;
}

action clone_delpkt(sid) {
	modify_field(meta.is_clone, 2);
	clone_ingress_pkt_to_egress(sid, clone_del_field_list);
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
}

control ingress {
	if (valid(op_hdr)) {

		// Stage 0
		apply(calculate_hash_tbl);
		apply(save_info_tbl);

		// Stage 1
		// Different MAT entries for getreq/putreq
		apply(match_keylolo_tbl);
		apply(match_keylohi_tbl);
		apply(match_keyhilo_tbl);
		apply(match_keyhihi_tbl);

		// Stage 2
		// NOTE: we put valid_reg in stage 2 to support DEL operation
		apply(access_valid_tbl); 

		// Stage 3
		// NOTE: we must put in stage 3 since get_vallo/hi_tbl relies on isvalid and ismatch_keylo/hi
		apply(update_vallo_tbl);
		apply(update_valhi_tbl);

		if (op_hdr.optype == GETREQ_TYPE) {
			// Stage 4 (rely on val)
			if (meta.isvalid == 1) {
				if (meta.ismatch_keylolo == 2 and meta.ismatch_keylohi == 2) {
					if (meta.ismatch_keyhilo == 2 and meta.ismatch_keyhihi == 2) {
						apply(sendback_getres_tbl);
					}
					else {
						apply(ipv4_lpm);
					}
				}
				else {
					apply(ipv4_lpm);
				}
			}
			else {
				apply(ipv4_lpm);
			}
		}
		else if (op_hdr.optype == PUTREQ_TYPE) {
			// Stage 3 (should be stage 1?)
			apply(sendback_putres_tbl);

			// Stage 4 (rely on val)
			if (meta.isvalid == 1) { // pkt is cloned to egress and will not execute this code
				if (meta.origin_keylolo != op_hdr.keylolo) {
					apply(clone_putpkt_tbl);
				}
				else if (meta.origin_keylohi != op_hdr.keylohi) {
					apply(clone_putpkt_tbl);
				}
				else if (meta.origin_keyhilo != op_hdr.keyhilo) {
					apply(clone_putpkt_tbl);
				}
				else if (meta.origin_keyhihi != op_hdr.keyhihi) {
					apply(clone_putpkt_tbl);
				}
			}
		}
		else if (op_hdr.optype == DELREQ_TYPE) {
			// Stage 2 
			if (meta.ismatch_keylolo == 2 and meta.ismatch_keylohi == 2) {
				if (meta.ismatch_keyhilo == 2 and meta.ismatch_keyhihi == 2) {
					apply(sendback_delres_tbl);
					apply(clone_delpkt_tbl);
				}
				else {
					apply(ipv4_lpm);
				}
			}
			else {
				apply(ipv4_lpm);
			}
		}
		else if (op_hdr.optype == SCANREQ_TYPE) {
			// Stage 0 
			apply(send_scanpkt_tbl);
			apply(ipv4_lpm);
		}
		else {
			apply(ipv4_lpm);
		}
	}
	else {
		apply(ipv4_lpm);
	}
}

/* Egress Processing */

action update_putreq() {
	modify_field(op_hdr.optype, PUTREQ_S_TYPE);

	modify_field(op_hdr.keylolo, meta.origin_keylolo);
	modify_field(op_hdr.keylohi, meta.origin_keylohi);
	modify_field(op_hdr.keyhilo, meta.origin_keyhilo);
	modify_field(op_hdr.keyhihi, meta.origin_keyhihi);
	modify_field(putreq_hdr.vallo, meta.origin_vallo);
	modify_field(putreq_hdr.valhi, meta.origin_valhi);
}

table update_putreq_tbl {
	actions {
		update_putreq;
	}
	default_action: update_putreq();
}

action update_delreq() {
	modify_field(op_hdr.optype, DELREQ_S_TYPE);
}

table update_delreq_tbl {
	actions {
		update_delreq;
	}
	default_action: update_delreq();
}

control egress {
	if (meta.is_clone == 1) {
		apply(update_putreq_tbl);
	}
	else if (meta.is_clone == 2) {
		apply(update_delreq_tbl);
	}
}
