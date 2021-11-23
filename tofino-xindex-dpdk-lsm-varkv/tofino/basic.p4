#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11

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
#define PUTREQ_CASE1_TYPE 0x0e
#define DELREQ_CASE1_TYPE 0x0f
#define PUTREQ_GS_CASE2_TYPE 0x10
#define PUTREQ_PS_CASE2_TYPE 0x11
#define PUTREQ_CASE3_TYPE 0x12
#define DELREQ_CASE3_TYPE 0x13
// Only used in switch
#define PUTREQ_U_TYPE 0x20
#define PUTREQ_RU_TYPE 0x21
#define GETRES_S_CASE2_TYPE 0x22
#define PUTREQ_RU_CASE2_TYPE 0x23

#define DELTA_THRESHOLD 10
#define OP_PORT 1111
#define KV_BUCKET_COUNT 32768

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
		hdrlen: 16;
	}
}

header_type op_t {
	fields {
		optype: 8;
		threadid: 8;
		keylololo: 16;
		keylolohi: 16;
		keylohilo: 16;
		keylohihi: 16;
		keyhilolo: 16;
		keyhilohi: 16;
		keyhihilo: 16;
		keyhihihi: 16;
	}
}

header_type metadata_t {
	fields {
		// TODO: Your Metadata
		hashidx: 16;
	}
}

header ethernet_t ethernet_hdr;
header ipv4_t ipv4_hdr;
header udp_t udp_hdr;
header op_t op_hdr;
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
		OP_PORT: parse_op;
		default: ingress;
	}
}

parser parse_op {
	extract(op_hdr);
	return ingress;
}

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

/* Ingress Processing */

control ingress {
	/*if (valid(udp_hdr)) {
	}*/
	apply(calculate_hash_tbl);
	apply(port_forward_tbl);
	if (op_hdr.optype != SCANREQ_TYPE) {
		apply(hash_partition_tbl); // update dst port of UDP according to hash value of key, only if dst_port = 1111 and egress_port and server port
	}
}

/* Egress Processing */

control egress {
	// nothing
}
