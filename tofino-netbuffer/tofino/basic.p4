#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11

#define OP_PORT 1111

// NOTE: Big Endian of type (not use threadid by now; big endian of key does not make sense)
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

header ethernet_t ethernet_hdr;
header ipv4_t ipv4_hdr;

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
	return ingress;
}

/* Ingress Processing (Normal Operation) */

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

action sendback_putres() {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);
}

table sendback_putres_tbl {
	actions {
		sendback_putres;
	}
	default_action: sendback_putres();
}

control ingress {
	//apply(sendback_putres_tbl);
	apply(ipv4_lpm);
}

/* Egress Processing */

control egress {
	// nothing
}
