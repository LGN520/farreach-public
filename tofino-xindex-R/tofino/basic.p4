#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11

#define DELTA_THRESHOLD 10

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

/*header_type metadata_t {
	fields {
		// TODO: Your Metadata
	}
}*/

header ethernet_t ethernet_hdr;
header ipv4_t ipv4_hdr;
header udp_t udp_hdr;
//metadata metadata_t meta;

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
	return ingress;
}

/* Ingress Processing (Normal Operation) */

action nop() {

}

action droppkt() {
	drop();
}

action ipv4_forward(port) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
	add_to_field(ipv4_hdr.ttl, -1);
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
	size: 1024;
}

/* Ingress Processing */

control ingress {
	if (valid(udp_hdr)) {
		apply(ipv4_lpm);
	}
}

/* Egress Processing */

control egress {
	// nothing
}
