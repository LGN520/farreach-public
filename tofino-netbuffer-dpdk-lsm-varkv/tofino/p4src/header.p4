/* Packet Header Types */

header_type udp_t {
	fields {
		srcPort: 16;
		dstPort: 16;
	}
}

header_type op_t {
	fields {
		optype: 8;
		threadid: 8;
		keylolo: 32;
		keylohi: 32;
		keyhilo: 32;
		keyhihi: 32;
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
		ismatch_keylolo: 4; // predicate 
		ismatch_keylohi: 4; // predicate 
		ismatch_keyhilo: 4; // predicate
		ismatch_keyhihi: 4; // predicate
		isvalid: 1;
		origin_keylolo: 32;
		origin_keylohi: 32;
		origin_keyhilo: 32;
		origin_keyhihi: 32;
		origin_vallen: 8;
		origin_vallo1: 32;
		origin_valhi1: 32;
		origin_vallo2: 32;
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
		/*origin_vallo12: 32;
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
