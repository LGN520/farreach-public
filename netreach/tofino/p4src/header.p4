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

header_type res_t {
	fields {
		stat: 8;
	}
}

header_type metadata_t {
	fields {
		hashidx: 16;
		server_idx: 8; // add to dst port for request
		ismatch_keylololo: 4; // predicate 
		ismatch_keylolohi: 4; // predicate 
		ismatch_keylohilo: 4; // predicate 
		ismatch_keylohihi: 4; // predicate 
		ismatch_keyhilolo: 4; // predicate
		ismatch_keyhilohi: 4; // predicate
		ismatch_keyhihilo: 4; // predicate
		ismatch_keyhihihi: 4; // predicate
		isvalid: 1;
		origin_keylololo: 16;
		origin_keylolohi: 16;
		origin_keylohilo: 16;
		origin_keylohihi: 16;
		origin_keyhilolo: 16;
		origin_keyhilohi: 16;
		origin_keyhihilo: 16;
		origin_keyhihihi: 16;
		origin_vallen: 8;
		origin_vallo1: 32;
		origin_valhi1: 32;
		/*origin_vallo2: 32;
		origin_valhi2: 32;
		origin_vallo3: 32;
		origin_valhi3: 32;
		origin_vallo4: 32;
		origin_valhi4: 32;
		origin_vallo5: 32;
		origin_valhi5: 32;
		origin_vallo6: 32;
		origin_valhi6: 32;
		origin_vallo7: 32;
		origin_valhi7: 32;
		origin_vallo8: 32;
		origin_valhi8: 32;
		origin_vallo9: 32;
		origin_valhi9: 32;
		origin_vallo10: 32;
		origin_valhi10: 32;
		origin_vallo11: 32;
		origin_valhi11: 32;
		origin_vallo12: 32;
		origin_valhi12: 32;
		origin_vallo13: 32;
		origin_valhi13: 32;
		origin_vallo14: 32;
		origin_valhi14: 32;
		origin_vallo15: 32;
		origin_valhi15: 32;
		origin_vallo16: 32;
		origin_valhi16: 32;*/
		tmp_sport: 16;
		tmp_dport: 16;
		tmp_port: 16;
		is_clone: 2;
	}
}
