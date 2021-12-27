/* Packet Header Types */

// PUT ipv4 in T-PHV

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
		hashidx: 16;
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

// Used by PUTREQ and GETRES to save PHV
header_type vallen_t {
	fields {
		vallen: 8; // 0 - 16
	}
}

header_type val_t {
	fields {
		vallo: 32;
		valhi: 32;
	}
}

header_type seq_t {
	fields {
		seq: 32;
	}
}

header_type res_t {
	fields {
		stat: 8;
	}
}

header_type metadata_t {
	fields {
		iscached: 1;
		being_evicted: 1;
		tmp_sport: 16;
		tmp_dport: 16;
		//isvalid: 1;
		iszerovote: 4;
		islock: 1;
		islatest: 8;
		isbackup: 1;
		iscase1: 1;
		is_clone: 4;
	}
}

// Header instances

header ethernet_t ethernet_hdr;
header ipv4_t ipv4_hdr;
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
/*header val_t val9_hdr;
header val_t val10_hdr;
header val_t val11_hdr;
header val_t val12_hdr;
header val_t val13_hdr;
header val_t val14_hdr;
header val_t val15_hdr;
header val_t val16_hdr;*/
header seq_t seq_hdr;
header res_t res_hdr;
metadata metadata_t meta;
