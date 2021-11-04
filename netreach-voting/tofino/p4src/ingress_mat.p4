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

field_list origin_hash_fields {
	meta.origin_keylololo;
	meta.origin_keylolohi;
	meta.origin_keylohilo;
	meta.origin_keylohihi;
	meta.origin_keyhilolo;
	meta.origin_keyhilohi;
	meta.origin_keyhihilo;
	meta.origin_keyhihihi;
}

field_list_calculation origin_hash_field_calc {
	input {
		origin_hash_fields;
	}
	algorithm: crc32;
	output_width: 16;
}

action calculate_origin_hash() {
	modify_field_with_hash_based_offset(meta.origin_hashidx, 0, origin_hash_field_calc, KV_BUCKET_COUNT);
}

table calculate_origin_hash_tbl {
	actions {
		calculate_origin_hash;
	}
	default_action: calculate_origin_hash();
	size: 1;
}

action save_info() {
	modify_field(meta.tmp_sport, udp_hdr.srcPort);
	modify_field(meta.tmp_dport, udp_hdr.dstPort);
}

//@pragma stage 0
table save_info_tbl {
	actions {
		save_info;
	}
	default_action: save_info();
	size: 1;
}

// Stage 5 + n

action update_getreq_to_getres() {
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

action update_putreq_to_putres() {
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

// NOTE: clone field list cannot exceed 32 bytes
field_list clone_field_list {
	meta.is_clone;
	meta.tmp_sport;
	meta.tmp_dport;
}

action update_delreq_to_s_and_clone(sid) {
	// Update transferred packet as delreq_s
	modify_field(op_hdr.optype, DELREQ_S_TYPE);

	// Clone a packet for delres 
	modify_field(meta.is_clone, CLONE_FOR_DELRES);
	clone_ingress_pkt_to_egress(sid, clone_field_list);
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
		update_getreq_to_getres;
		update_putreq_to_putres;
		update_delreq_to_s_and_clone;
		nop;
	}
	default_action: nop();
	size: 4;
}

// Stage 5+n + 2

action update_getreq() {
	modify_field(op_hdr.optype, GETREQ_S_TYPE);
}

action update_putreq() {
	modify_field(op_hdr.optype, PUTREQ_U_TYPE);
}

table trigger_cache_update_tbl {
	reads {
		meta.islock: exact;
		op_hdr.optype: exact;
		meta.isevict: exact;
	}
	actions {
		update_getreq;
		update_putreq;
		nop;
	}
	default_action: nop();
	size: 2;
}

// Last Stage of ingress pipeline

// Used by GETRES_S when original cached data is not dirty, and GETRES_NS
action update_getres_s(port) {
	modify_field(op_hdr.optype, GETRES_TYPE);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

action update_getres_s_and_clone(sid, port) {
	modify_field(op_hdr.optype, PUTREQ_GS_TYPE);

	modify_field(op_hdr.keylololo, meta.origin_keylololo);
	modify_field(op_hdr.keylolohi, meta.origin_keylolohi);
	modify_field(op_hdr.keylohilo, meta.origin_keylohilo);
	modify_field(op_hdr.keylohihi, meta.origin_keylohihi);
	modify_field(op_hdr.keyhilolo, meta.origin_keyhilolo);
	modify_field(op_hdr.keyhilohi, meta.origin_keyhilohi);
	modify_field(op_hdr.keyhihilo, meta.origin_keyhihilo);
	modify_field(op_hdr.keyhihihi, meta.origin_keyhihihi);
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

	// Forward PUTREQ_GS to server
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);

	// Clone a packet for GETRES 
	modify_field(meta.is_clone, CLONE_FOR_GETRES);
	clone_ingress_pkt_to_egress(sid, clone_field_list);
}

action update_putreq_ru_to_ps_and_clone(sid, port) {
	modify_field(op_hdr.optype, PUTREQ_PS_TYPE);

	// Format: evicted key (op_hdr) - evicted vallen - evicted val

	// Overwrite original key as evicted key
	modify_field(op_hdr.keylololo, meta.origin_keylololo);
	modify_field(op_hdr.keylolohi, meta.origin_keylolohi);
	modify_field(op_hdr.keylohilo, meta.origin_keylohilo);
	modify_field(op_hdr.keylohihi, meta.origin_keylohihi);
	modify_field(op_hdr.keyhilolo, meta.origin_keyhilolo);
	modify_field(op_hdr.keyhilohi, meta.origin_keyhilohi);
	modify_field(op_hdr.keyhihilo, meta.origin_keyhihilo);
	modify_field(op_hdr.keyhihihi, meta.origin_keyhihihi);

	// Overwrite original vallen and val as evicted vallen and val
	modify_field(vallen_hdr.vallen, meta.origin_vallen);
	modify_field(val1_hdr.vallo, meta.origin_vallo1);
	modify_field(val1_hdr.valhi, meta.origin_valhi1);
	/*modify_field(val2_hdr.vallo, meta.origin_vallo2);
	modify_field(val2_hdr.valhi, meta.origin_valhi2);
	modify_field(val3_hdr.vallo, meta.origin_vallo3);
	modify_field(val3_hdr.valhi, meta.origin_valhi3);
	modify_field(val4_hdr.vallo, meta.origin_vallo4);
	modify_field(val4_hdr.valhi, meta.origin_valhi4);
	modify_field(val5_hdr.vallo, meta.origin_vallo5);
	modify_field(val5_hdr.valhi, meta.origin_valhi5);
	modify_field(val6_hdr.vallo, meta.origin_vallo6);
	modify_field(val6_hdr.valhi, meta.origin_valhi6);
	modify_field(val7_hdr.vallo, meta.origin_vallo7);
	modify_field(val7_hdr.valhi, meta.origin_valhi7);
	modify_field(val8_hdr.vallo, meta.origin_vallo8);
	modify_field(val8_hdr.valhi, meta.origin_valhi8);
	modify_field(val9_hdr.vallo, meta.origin_vallo9);
	modify_field(val9_hdr.valhi, meta.origin_valhi9);
	modify_field(val10_hdr.vallo, meta.origin_vallo10);
	modify_field(val10_hdr.valhi, meta.origin_valhi10);
	modify_field(val11_hdr.vallo, meta.origin_vallo11);
	modify_field(val11_hdr.valhi, meta.origin_valhi11);
	modify_field(val12_hdr.vallo, meta.origin_vallo12);
	modify_field(val12_hdr.valhi, meta.origin_valhi12);
	modify_field(val13_hdr.vallo, meta.origin_vallo13);
	modify_field(val13_hdr.valhi, meta.origin_valhi13);
	modify_field(val14_hdr.vallo, meta.origin_vallo14);
	modify_field(val14_hdr.valhi, meta.origin_valhi14);
	modify_field(val15_hdr.vallo, meta.origin_vallo15);
	modify_field(val15_hdr.valhi, meta.origin_valhi15);
	modify_field(val16_hdr.vallo, meta.origin_vallo16);
	modify_field(val16_hdr.valhi, meta.origin_valhi16);*/

	// Forward PUTREQ_PS to server
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);

	// Clone a packet for PUTRES to client
	modify_field(meta.is_clone, CLONE_FOR_PUTRES);
	clone_ingress_pkt_to_egress(sid, clone_field_list);
}

action update_putreq_ru_to_putres(port) {
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

	// Forward PUTRES to client
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

action port_forward(port) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

action recirculate_putreq_u(port) {
	modify_field(op_hdr.optype, PUTREQ_RU_TYPE); // convert into PUTREQ_RU (recirculated update)
	// It is equivalent to ig_intro_md_for_tm.ucast_egress_port = (port & 0x7f) | (ingress_port & ~0x7f)
	recirculate(port);
}

action recirculate_pkt(port) {
	// It is equivalent to ig_intro_md_for_tm.ucast_egress_port = (port & 0x7f) | (ingress_port & ~0x7f)
	recirculate(port);
}

table port_forward_tbl {
	reads {
		op_hdr.optype: exact;
		meta.isvalid: exact;
		meta.isdirty: exact;
		meta.islock: exact;
	}
	actions {
		update_getres_s;
		update_getres_s_and_clone;
		update_putreq_ru_to_putres;
		update_putreq_ru_to_ps_and_clone;
		recirculate_putreq_u;
		recirculate_pkt;
		port_forward;
		nop;
	}
	default_action: nop();
	size: 128;  
}


action update_dstport(port) {
	modify_field(udp_hdr.dstPort, port);
}

action update_dstport_reverse(port) {
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	modify_field(udp_hdr.dstPort, port);
}

table hash_partition_tbl {
	reads {
		udp_hdr.dstPort: exact;
		ig_intr_md_for_tm.ucast_egress_port: exact;
		meta.hashidx: range;
	}
	actions {
		update_dstport;
		nop;
	}
	default_action: nop();
	size: 128;
}

table origin_hash_partition_tbl {
	reads {
		udp_hdr.dstPort: exact;
		ig_intr_md_for_tm.ucast_egress_port: exact;
		meta.origin_hashidx: range;
	}
	actions {
		update_dstport;
		nop;
	}
	default_action: nop();
	size: 128;
}

table origin_hash_partition_reverse_tbl {
	reads {
		udp_hdr.srcPort: exact;
		ig_intr_md_for_tm.ucast_egress_port: exact;
		meta.origin_hashidx: range;
	}
	actions {
		update_dstport_reverse;
		nop;
	}
	default_action: nop();
	size: 128;
}
