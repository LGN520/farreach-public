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

header_type seq_t {
	fields {
		seq: 32;
		is_assigned: 8;
	}
}

header_type res_t {
	fields {
		stat: 8;
	}
}

header_type key_t {
	fields {
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
		hashidx: 16;
		origin_hashidx: 16;
		tmp_sport: 16;
		tmp_dport: 16;
		ismatch_keylololo: 4; // predicate 
		ismatch_keylolohi: 4; // predicate 
		ismatch_keylohilo: 4; // predicate 
		ismatch_keylohihi: 4; // predicate 
		ismatch_keyhilolo: 4; // predicate
		ismatch_keyhilohi: 4; // predicate
		ismatch_keyhihilo: 4; // predicate
		ismatch_keyhihihi: 4; // predicate
		origin_keylololo: 16;
		origin_keylolohi: 16;
		origin_keylohilo: 16;
		origin_keylohihi: 16;
		origin_keyhilolo: 16;
		origin_keyhilohi: 16;
		origin_keyhihilo: 16;
		origin_keyhihihi: 16;
		isvalid: 1; // if the entry is valid
		isdirty: 1; // if the entry is dirty
		canput: 4; // predicate for seq (update vallen and val only if with valid seq)
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
		isevict: 4; // predicate (if we need to trigger cache update)
		islock: 1; // if the entry is locked before
		/*isfuture_valid: 1;
		ismatch_future_keylololo: 4; // predicate 
		ismatch_future_keylolohi: 4; // predicate 
		ismatch_future_keylohilo: 4; // predicate 
		ismatch_future_keylohihi: 4; // predicate 
		ismatch_future_keyhilolo: 4; // predicate
		ismatch_future_keyhilohi: 4; // predicate
		ismatch_future_keyhihilo: 4; // predicate
		ismatch_future_keyhihihi: 4; // predicate*/
		is_clone: 2;
		isbackup: 1; // backup flag
		iscase1: 1; // case 1 of backup
		iscase2: 1; // case 2 of backup
		iscase3: 1; // case 3 of backup
	}
}

// Header instances

header ethernet_t ethernet_hdr;
header ipv4_t ipv4_hdr;
header udp_t udp_hdr;
header op_t op_hdr;
header vallen_t vallen_hdr;
header val_t val1_hdr;
/*header val_t val2_hdr;
header val_t val3_hdr;
header val_t val4_hdr;
header val_t val5_hdr;
header val_t val6_hdr;
header val_t val7_hdr;
header val_t val8_hdr;
header val_t val9_hdr;
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
