/* Ingress Processing (Normal Operation) */

// Stage 0

action nop() {}

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

table calculate_hash_tbl {
	actions {
		calculate_hash;
	}
	default_action: calculate_hash();
	size: 1;
}

action save_info() {
	modify_field(meta.tmp_sport, udp_hdr.srcPort);
	modify_field(meta.tmp_dport, udp_hdr.dstPort);
}

table save_info_tbl {
	actions {
		save_info;
	}
	default_action: save_info();
	size: 1;
}

action load_gthreshold(threshold) {
	modify_field(meta.gthreshold, threshold);
}

table load_gthreshold_tbl {
	actions {
		load_gthreshold;
	}
	size: 1;
}

action load_pthreshold(threshold) {
	modify_field(meta.pthreshold, threshold);
}

table load_pthreshold_tbl {
	actions {
		load_pthreshold;
	}
	size: 1;
}

// Stage 5 + n

action sendback_getres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	add_to_field(udp_hdr.hdrlen, VAL_PKTLEN);

	modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);

	modify_field(op_hdr.optype, GETRES_TYPE);
	modify_field(vallen_hdr.vallen, meta.origin_vallen);
	add_header(vallen_hdr);
	modify_field(val1_hdr.vallo, meta.origin_vallo1);
	modify_field(val1_hdr.valhi, meta.origin_valhi1);
	add_header(val1_hdr);
	/*modify_field(val2_hdr.vallo, meta.origin_vallo2);
	modify_field(val2_hdr.valhi, meta.origin_valhi2);
	add_header(val2_hdr);
	modify_field(val3_hdr.vallo, meta.origin_vallo3);
	modify_field(val3_hdr.valhi, meta.origin_valhi3);
	add_header(val3_hdr);
	modify_field(val4_hdr.vallo, meta.origin_vallo4);
	modify_field(val4_hdr.valhi, meta.origin_valhi4);
	add_header(val4_hdr);
	modify_field(val5_hdr.vallo, meta.origin_vallo5);
	modify_field(val5_hdr.valhi, meta.origin_valhi5);
	add_header(val5_hdr);
	modify_field(val6_hdr.vallo, meta.origin_vallo6);
	modify_field(val6_hdr.valhi, meta.origin_valhi6);
	add_header(val6_hdr);
	modify_field(val7_hdr.vallo, meta.origin_vallo7);
	modify_field(val7_hdr.valhi, meta.origin_valhi7);
	add_header(val7_hdr);
	modify_field(val8_hdr.vallo, meta.origin_vallo8);
	modify_field(val8_hdr.valhi, meta.origin_valhi8);
	add_header(val8_hdr);
	modify_field(val9_hdr.vallo, meta.origin_vallo9);
	modify_field(val9_hdr.valhi, meta.origin_valhi9);
	add_header(val9_hdr);
	modify_field(val10_hdr.vallo, meta.origin_vallo10);
	modify_field(val10_hdr.valhi, meta.origin_valhi10);
	add_header(val10_hdr);
	modify_field(val11_hdr.vallo, meta.origin_vallo11);
	modify_field(val11_hdr.valhi, meta.origin_valhi11);
	add_header(val11_hdr);
	modify_field(val12_hdr.vallo, meta.origin_vallo12);
	modify_field(val12_hdr.valhi, meta.origin_valhi12);
	add_header(val12_hdr);
	modify_field(val13_hdr.vallo, meta.origin_vallo13);
	modify_field(val13_hdr.valhi, meta.origin_valhi13);
	add_header(val13_hdr);
	modify_field(val14_hdr.vallo, meta.origin_vallo14);
	modify_field(val14_hdr.valhi, meta.origin_valhi14);
	add_header(val14_hdr);
	modify_field(val15_hdr.vallo, meta.origin_vallo15);
	modify_field(val15_hdr.valhi, meta.origin_valhi15);
	add_header(val15_hdr);
	modify_field(val16_hdr.vallo, meta.origin_vallo16);
	modify_field(val16_hdr.valhi, meta.origin_valhi16);
	add_header(val16_hdr);*/
}

action sendback_putres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	subtract_from_field(udp_hdr.hdrlen, VAL_PKTLEN_MINUS_ONE);

	remove_header(vallen_hdr);
	remove_header(val1_hdr);
	/*remove_header(val2_hdr);
	remove_header(val3_hdr);
	remove_header(val4_hdr);
	remove_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);*/
	modify_field(op_hdr.optype, PUTRES_TYPE);
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

table sendback_putres_tbl {
	actions {
		sendback_putres;
	}
	default_action: sendback_putres();
	size: 1;
}

table try_res_tbl {
	reads {
		op_hdr.optype: exact;
		meta.isvalid: exact;
		meta.ismatch_keylololo: exact;
		meta.ismatch_keylolohi: exact;
		meta.ismatch_keylohilo: exact;
		meta.ismatch_keylohihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		sendback_getres;
		sendback_putres;
		nop;
	}
	default_action: nop();
	size: 8;
}

// Stage 5+n + 1

action gneg_ppos_diff() {
	subtract(meta.vote_diff, meta.gnegvote, meta.pposvote);
}

action pneg_ppos_diff() {
	subtract(meta.vote_diff, meta.pnegvote, meta.pposvote);
}

action gneg_gpos_diff() {
	subtract(meta.vote_diff, meta.gnegvote, meta.gposvote);
}

action pneg_gpos_diff() {
	subtract(meta.vote_diff, meta.pnegvote, meta.gposvote);
}

action default_diff() {
	modify_field(meta.vote_diff, 0);
}

table calculate_diff_tbl {
	reads {
		meta.isdirty: exact;
		op_hdr.optype: exact;
	}
	actions {
		gneg_ppos_diff;
		pneg_ppos_diff;
		gneg_gpos_diff;
		pneg_gpos_diff;
		default_diff;
	}
	default_action: default_diff();
	size: 4;
}

// Last Stage of ingress pipeline

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
