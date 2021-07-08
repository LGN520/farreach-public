#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11

#define REQ_PORT 1111
#define BF_BUCKET_COUNT 16
#define BF_HASH_WIDTH 4

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

header_type req_t {
	fields {
		reqtype: 32;
		threadid: 32;
		key: 64;
	}
}

header_type metadata_t {
	fields {
		hashidx: 32;
		isexist: 1;
	}
}

header ethernet_t ethernet_hdr;
header ipv4_t ipv4_hdr;
header udp_t udp_hdr;
header req_t req_hdr;
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
		REQ_PORT: parse_req;
		default: ingress;
	}
}

parser parse_req {
	extract(req_hdr);
	return ingress;
}

/* Ingress Processing (Normal Operation) */

action nop() {

}

action droppkt() {
	drop();
}

action mac_forward(port) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
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
		nop;
	}
	default_action: droppkt();
	size: BF_BUCKET_COUNT;
}

field_list hash_fields {
	req_hdr.key;
}

/* Bloomfilter */

field_list_calculation bloomfilter_field_calc {
	input {
		hash_fields;
	}
	algorithm: crc32;
	output_width: BF_HASH_WIDTH;
}

action calculate_hash() {
	modify_field_with_hash_based_offset(meta.hashidx, 0, bloomfilter_field_calc, BF_BUCKET_COUNT);
}

table calculate_hash_tbl {
	actions {
		calculate_hash;
	}
	default_action: calculate_hash();
}

register bloomfilter_reg {
	width: 1;
	instance_count: 1024; /* 1 Kb */
}

blackbox stateful_alu bloomfilter_alu {
	reg: bloomfilter_reg;
	
	update_lo_1_value: read_bit;
	output_value: alu_lo;
	output_dst: meta.isexist;
}

action check_bloomfilter() {
	//bloomfilter_alu.execute_stateful_alu_from_hash(bloomfilter_field_calc);
	bloomfilter_alu.execute_stateful_alu(meta.hashidx);
}

table check_bloomfilter_tbl {
	actions {
		check_bloomfilter;
	}
	default_action: check_bloomfilter();
}

/* Ingress Processing */

control ingress {
	apply(ipv4_lpm);
	if (valid(udp_hdr) and valid(req_hdr)) {
		apply(calculate_hash_tbl);
		apply(check_bloomfilter_tbl);
	}
}

/* Egress Processing */

control egress {
	// nothing
}
