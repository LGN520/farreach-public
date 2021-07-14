#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11

#define OP_PORT 1111

// NOTE: convert type into Big Endian (not use threadid by now; big endian of key does not make sense)
#define GETREQ_TYPE 0x00000000
#define PUTREQ_TYPE 0x01000000
#define DELREQ_TYPE 0x02000000
#define SCANREQ_TYPE 0x03000000
#define GETRES_TYPE 0x04000000
#define PUTRES_TYPE 0x05000000
#define DELRES_TYPE 0x06000000
#define SCANRES_TYPE 0x07000000

#define KV_BUCKET_COUNT 16

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
		keylo: 32;
		keyhi: 32;
	}
}

header_type putreq_t {
	fields {
		vallo: 32;
		valhi: 32;
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
		stat: 1;
		pad: 7;
	}
}

header_type metadata_t {
	fields {
		hashidx: 32;
		ismatch_keylo: 1;
		ismatch_keyhi: 1;
		isvalid: 1;
		origin_keylo: 32;
		origin_keyhi: 32;
		origin_vallo: 32;
		origin_valhi: 32;
		tmp_macaddr: 48;
		tmp_ipaddr: 32;
		tmp_port: 16;
	}
}

header ethernet_t ethernet_hdr;
header ipv4_t ipv4_hdr;
header udp_t udp_hdr;
header op_t op_hdr;
header putreq_t putreq_hdr;
header getres_t getres_hdr;
header putres_t putres_hdr;
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
		default: ingress;
	}
}

parser parse_udp {
	extract(udp_hdr);
	return select(udp_hdr.dstPort) {
		OP_PORT: parse_op;
		default: ingress;
	}
}

parser parse_op {
	extract(op_hdr);
	return select(op_hdr.optype) {
		PUTREQ_TYPE: parse_putreq;
		GETRES_TYPE: parse_getres;
		PUTRES_TYPE: parse_putres;
		default: ingress;
	}
}

parser parse_putreq {
	extract(putreq_hdr);
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

action droppkt() {
	drop();
}

table droppkt_tbl {
	actions {
		droppkt;
	}
	default_action: droppkt();
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
	op_hdr.keylo;
	op_hdr.keyhi;
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

table calculate_hash_tbl {
	actions {
		calculate_hash;
	}
	default_action: calculate_hash();
}

/* KV (hash table) */

register keylo_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keylo_alu {
	reg: keylo_reg;

	condition_lo: register_lo == op_hdr.keylo;

	update_lo_1_value: register_lo;
	
	output_value: combined_predicate;
	output_dst: meta.ismatch_keylo;
}

action get_match_keylo() {
	get_match_keylo_alu.execute_stateful_alu(meta.hashidx);
}

table get_match_keylo_tbl {
	actions {
		get_match_keylo;
	}
	default_action: get_match_keylo();
}

blackbox stateful_alu put_match_keylo_alu {
	reg: keylo_reg;

	condition_lo: register_lo == op_hdr.keylo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylo;
	
	output_value: register_lo;
	output_dst: meta.origin_keylo;
}

action put_match_keylo() {
	put_match_keylo_alu.execute_stateful_alu(meta.hashidx);
}

table put_match_keylo_tbl {
	actions {
		put_match_keylo;
	}
	default_action: put_match_keylo();
}

register keyhi_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keyhi_alu {
	reg: keyhi_reg;

	condition_lo: register_lo == op_hdr.keyhi;

	update_lo_1_value: register_lo;
	
	output_value: combined_predicate;
	output_dst: meta.ismatch_keyhi;
}

action get_match_keyhi() {
	get_match_keyhi_alu.execute_stateful_alu(meta.hashidx);
}

table get_match_keyhi_tbl {
	actions {
		get_match_keyhi;
	}
	default_action: get_match_keyhi();
}

blackbox stateful_alu put_match_keyhi_alu {
	reg: keyhi_reg;

	condition_lo: register_lo == op_hdr.keyhi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhi;
	
	output_value: register_lo;
	output_dst: meta.origin_keyhi;
}

action put_match_keyhi() {
	put_match_keyhi_alu.execute_stateful_alu(meta.hashidx);
}

table put_match_keyhi_tbl {
	actions {
		put_match_keyhi;
	}
	default_action: put_match_keyhi();
}

register valid_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valid_alu {
	reg: valid_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.isvalid;
}

action get_valid() {
	get_valid_alu.execute_stateful_alu(meta.hashidx);
}

table get_valid_tbl {
	actions {
		get_valid;
	}
	default_action: get_valid();
}

blackbox stateful_alu set_valid_alu {
	reg: valid_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.isvalid;
}

action set_valid() {
	set_valid_alu.execute_stateful_alu(meta.hashidx);
}

table set_valid_tbl {
	actions {
		set_valid;
	}
	default_action: set_valid();
}

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

table get_vallo_tbl {
	actions {
		get_vallo;
	}
	default_action: get_vallo();
}

blackbox stateful_alu put_vallo_alu {
	reg: vallo_reg;

	update_lo_1_value: putreq_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo;
}

action put_vallo() {
	put_vallo_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 2
table put_vallo_tbl {
	actions {
		put_vallo;
	}
	default_action: put_vallo();
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

table get_valhi_tbl {
	actions {
		get_valhi;
	}
	default_action: get_valhi();
}

blackbox stateful_alu put_valhi_alu {
	reg: valhi_reg;

	update_lo_1_value: putreq_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi;
}

action put_valhi() {
	put_valhi_alu.execute_stateful_alu(meta.hashidx);
}

table put_valhi_tbl {
	actions {
		put_valhi;
	}
	default_action: put_valhi();
}

/* Ingress Processing */

action save_dstinfo() {
	//modify_field(meta.tmp_macaddr, ethernet_hdr.dstAddr);
	modify_field(meta.tmp_ipaddr, ipv4_hdr.dstAddr);
	modify_field(meta.tmp_port, udp_hdr.dstPort);
}

table save_dstinfo_tbl {
	actions {
		save_dstinfo;
	}
	default_action: save_dstinfo();
}

action sendback_getres() {
	modify_field(ethernet_hdr.srcAddr, ethernet_hdr.dstAddr);

	// Swap ip address
	modify_field(ipv4_hdr.dstAddr, ipv4_hdr.srcAddr);
	modify_field(ipv4_hdr.srcAddr, meta.tmp_ipaddr);
	add_to_field(ipv4_hdr.totalLen, 8); // Big endian: add an 8B value
	
	// Swap udp port
	modify_field(udp_hdr.dstPort, udp_hdr.srcPort);
	modify_field(udp_hdr.srcPort, meta.tmp_port);
	modify_field(udp_hdr.hdrLength, 0x2000); // Convert 0x0020 into big endian

	modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);

	modify_field(op_hdr.optype, GETRES_TYPE);
	modify_field(getres_hdr.vallo, meta.origin_vallo);
	modify_field(getres_hdr.valhi, meta.origin_valhi);
	add_header(getres_hdr);
}

table sendback_getres_tbl {
	actions {
		sendback_getres;
	}
	default_action: sendback_getres();
}

action sendback_putres() {
	modify_field(ethernet_hdr.srcAddr, ethernet_hdr.dstAddr);

	// Swap ip address
	modify_field(ipv4_hdr.dstAddr, ipv4_hdr.srcAddr);
	modify_field(ipv4_hdr.srcAddr, meta.tmp_ipaddr);
	modify_field(ipv4_hdr.totalLen, 45); // Big endian: 20B IP + 8B UDP + 16B OP + 1B status
	
	// Swap udp port
	modify_field(udp_hdr.dstPort, udp_hdr.srcPort);
	modify_field(udp_hdr.srcPort, meta.tmp_port);
	modify_field(udp_hdr.hdrLength, 0x1900); // Convert 0x0019 into big endian

	modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);

	modify_field(op_hdr.optype, PUTRES_TYPE);
	modify_field(putres_hdr.stat, 1);
	remove_header(putreq_hdr);
	add_header(putres_hdr);
}

table sendback_putres_tbl {
	actions {
		sendback_putres;
	}
	default_action: sendback_putres();
}

action update_putreq() {
	modify_field(op_hdr.keylo, meta.origin_keylo);
	modify_field(op_hdr.keyhi, meta.origin_keyhi);
	modify_field(putreq_hdr.vallo, meta.origin_vallo);
	modify_field(putreq_hdr.valhi, meta.origin_valhi);
}

table update_putreq_tbl {
	actions {
		update_putreq;
	}
	default_action: update_putreq();
}

control ingress {
	if (valid(op_hdr)) {
		apply(calculate_hash_tbl);
		apply(save_dstinfo_tbl);
		if (op_hdr.optype == GETREQ_TYPE) {
			apply(get_valid_tbl);
			apply(get_match_keylo_tbl);
			apply(get_match_keyhi_tbl);
			if (meta.isvalid == 1) {
				if (meta.ismatch_keylo == 1 and meta.ismatch_keyhi == 1) {
					apply(get_vallo_tbl);
					apply(get_valhi_tbl);
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
		else if (op_hdr.optype == PUTREQ_TYPE) {
			apply(set_valid_tbl);
			apply(put_match_keylo_tbl);
			apply(put_match_keyhi_tbl);
			apply(put_vallo_tbl);
			apply(put_valhi_tbl);
			if (meta.isvalid == 1) {
				if (meta.origin_keylo == op_hdr.keylo) { 
					if (meta.origin_keyhi == op_hdr.keyhi) {
						apply(sendback_putres_tbl);
					}
					else {
						apply(update_putreq_tbl);
						apply(ipv4_lpm);
					}
				}
				else {
					apply(update_putreq_tbl);
					apply(ipv4_lpm);
				}
			}
			else {
				apply(sendback_putres_tbl);
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

/* Egress Processing */

control egress {
	// nothing
}
