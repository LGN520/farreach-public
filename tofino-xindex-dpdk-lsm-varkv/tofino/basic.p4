#include "tofino/intrinsic_metadata.p4"
#include "tofino/stateful_alu_blackbox.p4"

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11

#define DELTA_THRESHOLD 10

/* Packet Header Types */

header_type udp_t {
	fields {
		srcPort: 16;
		dstPort: 16;
		hdrlen: 16;
	}
}

/*header_type metadata_t {
	fields {
		// TODO: Your Metadata
	}
}*/

header udp_t udp_hdr;
//metadata metadata_t meta;

/* Parser */

parser start {
	return parse_udp;
}

parser parse_udp {
	extract(udp_hdr);
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

/* Ingress Processing */

control ingress {
	/*if (valid(udp_hdr)) {
	}*/
	apply(port_forward_tbl);
}

/* Egress Processing */

control egress {
	// nothing
}
