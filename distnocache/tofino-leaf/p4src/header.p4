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
		checksum: 16;
	}
}

header_type op_t {
	fields {
		optype: 16;
		keylolo: 32;
		keylohi: 32;
		keyhilo: 32;
		//keyhihi: 32;
		keyhihilo: 16;
		keyhihihi: 16;
	}
}

#ifdef RANGE_SUPPORT
header_type scan_t {
	fields {
		keylolo: 32;
		keylohi: 32;
		keyhilo: 32;
		//keyhihi: 32;
		keyhihilo: 16;
		keyhihihi: 16;
	}
}
header_type split_t {
	fields {
		is_clone: 8;
		globalserveridx: 16;
		cur_scanidx: 16;
		max_scannum: 16;
	}
}
#endif

// Used by PUTREQ and GETRES to save PHV
header_type vallen_t {
	fields {
		vallen: 16;
	}
}

header_type val_t {
	fields {
		vallo: 32;
		valhi: 32;
	}
}

header_type metadata_t {
	fields {
#ifndef RANGE_SUPPORT
		hashval_for_partition: 16; // at most 32K
#else
		server_sid: 10; // clone to server for SCANREQ_SPLIT
		remain_scannum: 16;
		is_last_scansplit: 1;
#endif
	}
}

// Header instances

header ethernet_t ethernet_hdr;
header ipv4_t ipv4_hdr;
header udp_t udp_hdr;
header op_t op_hdr;
#ifdef RANGE_SUPPORT
header scan_t scan_hdr;
header split_t split_hdr;
#endif
metadata metadata_t meta;
