/* Ingress Processing (Normal Operation) */

// Stage 0

action nop() {}

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

action initialize() {
	// NOTE: condition_lo = false -> predicate = 1
	// Make such an initialization to reduce MAT entries
	modify_field(meta.ismatch_keylolo, 1);
	modify_field(meta.ismatch_keylohi, 1);
	modify_field(meta.ismatch_keyhilo, 1);
	modify_field(meta.ismatch_keyhihi, 1);
	modify_field(meta.canput, 1);
	modify_field(meta.zerovote, 1);
}

table initialize_tbl {
	actions {
		initialize;
	}
	default_action: initialize();
}

action load_backup_flag(flag) {
	modify_field(meta.isbackup, flag);
}

table load_backup_flag_tbl {
	actions {
		load_backup_flag;
	}
	default_action: load_backup_flag(0);
}

// Stage 1

action update_iskeymatch(iskeymatch) {
	modify_field(meta.iskeymatch, iskeymatch);
}

table update_iskeymatch_tbl {
	reads {
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
	}
	actions {
		update_iskeymatch;
	}
	default_action: update_iskeymatch(0);
	size: 1;
}

// Stage 11

action update_getreq_to_getreq_pop(port) {
	modify_field(op_hdr.optype, GETREQ_POP_TYPE);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

action update_getreq_to_getres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	add_to_field(udp_hdr.hdrlen, VAL_PKTLEN);

	modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);

	modify_field(op_hdr.optype, GETRES_TYPE);
	add_header(vallen_hdr);
	add_header(val1_hdr);
	*/add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	add_header(val8_hdr);
	add_header(val9_hdr);
	add_header(val10_hdr);
	add_header(val11_hdr);
	add_header(val12_hdr);
	add_header(val13_hdr);
	add_header(val14_hdr);
	add_header(val15_hdr);
	add_header(val16_hdr);*/
}

action recirculate_getreq(port) {
	// It is equivalent to ig_intro_md_for_tm.ucast_egress_port = (port & 0x7f) | (ingress_port & ~0x7f)
	recirculate(port);
}

// NOTE: clone field list cannot exceed 32 bytes
field_list clone_field_list {
	meta.is_clone;
	meta.tmp_sport;
	meta.tmp_dport;
}

action drop_getres_pop_clone_for_getres(sid) {
	modify_field(ig_intr_md_for_tm.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_ingress_pkt_to_egress(sid, clone_field_list);
}

action update_getres_pop_to_evict_clone_for_getres(sid) {
	modify_field(op_hdr.optype, GETRES_POP_EVICT);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);
	clone_ingress_pkt_to_egress(sid, clone_field_list);
}

action update_getres_npop_to_getres(port) {
	modify_field(op_hdr.optype, GETRES_TYPE);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

action port_forward(port) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

@pragma stage 11
table port_forward_tbl {
	reads {
		op_hdr.optype: exact;
		meta.isvalid: exact;
		meta.zerovote: exact;
		meta.iskeymatch: exact;
		meta.islock: exact;
	}
	actions {
		update_getreq_to_getreq_pop;
		update_getreq_to_getres;
		recirculate_getreq;
		drop_getres_pop_clone_for_getres;
		update_getres_pop_to_evict_clone_for_getres;
		update_getres_npop_to_getres;
		//update_getres_pop_to_case2_clone_for_getres;
		port_forward;
		nop;
	}
	default_action: nop();
	size: 1024;  
}

// Deprecated 

action update_putreq_to_putres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	subtract_from_field(udp_hdr.hdrlen, VAL_PKTLEN_MINUS_ONE);

	remove_header(vallen_hdr);
	remove_header(val1_hdr);
	remove_header(val2_hdr);
	remove_header(val3_hdr);
	remove_header(val4_hdr);
	remove_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	/*remove_header(val9_hdr);
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

action update_delreq_to_s_and_clone(sid) {
	// Update transferred packet as delreq_s
	modify_field(op_hdr.optype, DELREQ_S_TYPE);

	// Clone a packet for delres 
	modify_field(meta.is_clone, CLONE_FOR_DELRES);
	clone_ingress_pkt_to_egress(sid, clone_field_list);
}

action update_putreq_to_case1() {
	modify_field(op_hdr.optype, PUTREQ_CASE1_TYPE);

	// Set old value
	modify_field(vallen_hdr.vallen, meta.origin_vallen);
	add_header(vallen_hdr);
	modify_field(val1_hdr.vallo, meta.origin_vallo1);
	modify_field(val1_hdr.valhi, meta.origin_valhi1);
	add_header(val1_hdr);
	modify_field(val2_hdr.vallo, meta.origin_vallo2);
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
	/*modify_field(val9_hdr.vallo, meta.origin_vallo9);
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

	modify_field(seq_hdr.seq, meta.hashidx);
	modify_field(seq_hdr.is_assigned, meta.isdirty);
}

action update_delreq_to_case1() {
	modify_field(op_hdr.optype, DELREQ_CASE1_TYPE);
	add_to_field(udp_hdr.hdrlen, VAL_PKTLEN);

	// Set old value
	modify_field(vallen_hdr.vallen, meta.origin_vallen);
	add_header(vallen_hdr);
	modify_field(val1_hdr.vallo, meta.origin_vallo1);
	modify_field(val1_hdr.valhi, meta.origin_valhi1);
	add_header(val1_hdr);
	modify_field(val2_hdr.vallo, meta.origin_vallo2);
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
	/*modify_field(val9_hdr.vallo, meta.origin_vallo9);
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

	modify_field(seq_hdr.seq, meta.hashidx);
	modify_field(seq_hdr.is_assigned, meta.isdirty);
	add_header(seq_hdr);
}

action update_getres_s_to_case2(remember_bit) {
	modify_field(op_hdr.optype, GETRES_S_CASE2_TYPE);

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
	modify_field(val2_hdr.vallo, meta.origin_vallo2);
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
	/*modify_field(val9_hdr.vallo, meta.origin_vallo9);
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

	modify_field(seq_hdr.seq, meta.hashidx);
	modify_field(seq_hdr.is_assigned, remember_bit);
	add_header(seq_hdr);
}

action update_putreq_ru_to_case2(remember_bit) {
	modify_field(op_hdr.optype, PUTREQ_RU_CASE2_TYPE);

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
	modify_field(val2_hdr.vallo, meta.origin_vallo2);
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
	/*modify_field(val9_hdr.vallo, meta.origin_vallo9);
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

	modify_field(seq_hdr.seq, meta.hashidx);
	modify_field(seq_hdr.is_assigned, remember_bit);
	add_header(seq_hdr);
}

action update_getres_s_to_case2_clear_lock(remember_bit) {
	modify_field(op_hdr.optype, GETRES_S_CASE2_TYPE);

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
	modify_field(val2_hdr.vallo, meta.origin_vallo2);
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
	/*modify_field(val9_hdr.vallo, meta.origin_vallo9);
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

	modify_field(seq_hdr.seq, meta.hashidx);
	modify_field(seq_hdr.is_assigned, remember_bit);
	add_header(seq_hdr);

	clear_lock_alu.execute_stateful_alu(meta.hashidx);
}

action update_putreq_ru_to_case2_clear_lock(remember_bit) {
	modify_field(op_hdr.optype, PUTREQ_RU_CASE2_TYPE);

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
	modify_field(val2_hdr.vallo, meta.origin_vallo2);
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
	/*modify_field(val9_hdr.vallo, meta.origin_vallo9);
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

	modify_field(seq_hdr.seq, meta.hashidx);
	modify_field(seq_hdr.is_assigned, remember_bit);
	add_header(seq_hdr);

	clear_lock_alu.execute_stateful_alu(meta.hashidx);
}

// Last Stage of ingress pipeline

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
	modify_field(val2_hdr.vallo, meta.origin_vallo2);
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
	/*modify_field(val9_hdr.vallo, meta.origin_vallo9);
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
	remove_header(val2_hdr);
	remove_header(val3_hdr);
	remove_header(val4_hdr);
	remove_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	/*remove_header(val9_hdr);
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

action recirculate_putreq_u(port) {
	modify_field(op_hdr.optype, PUTREQ_RU_TYPE); // convert into PUTREQ_RU (recirculated update)
	// It is equivalent to ig_intro_md_for_tm.ucast_egress_port = (port & 0x7f) | (ingress_port & ~0x7f)
	recirculate(port);
}

action forward_putreq_case1_and_clone(sid, port) {
	// Forward PUTREQ_CASE1 to server
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);

	// Clone a packet for PUTRES to client
	modify_field(meta.is_clone, CLONE_FOR_PUTRES);
	clone_ingress_pkt_to_egress(sid, clone_field_list);
}

action forward_delreq_case1_and_clone(sid, port) {
	// Forward DELREQ_CASE1 to server
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);

	// Clone a packet for DELRES to client
	modify_field(meta.is_clone, CLONE_FOR_DELRES);
	clone_ingress_pkt_to_egress(sid, clone_field_list);
}

action update_getres_s_case2_to_putreq_gs_case2_and_clone(sid, port) {
	modify_field(op_hdr.optype, PUTREQ_GS_CASE2_TYPE);

	// Forward PUTREQ_GS_CASE2 to server
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);

	// Clone a packet for GETRES 
	modify_field(meta.is_clone, CLONE_FOR_GETRES);
	clone_ingress_pkt_to_egress(sid, clone_field_list);
}

action update_putreq_ru_case2_to_putreq_ps_case2_and_clone(sid, port) {
	modify_field(op_hdr.optype, PUTREQ_PS_CASE2_TYPE);

	// Forward PUTREQ_PS_CASE2 to server
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);

	// Clone a packet for PUTRES to client
	modify_field(meta.is_clone, CLONE_FOR_PUTRES);
	clone_ingress_pkt_to_egress(sid, clone_field_list);
}

action update_putreq_to_case3(port) {
	modify_field(op_hdr.optype, PUTREQ_CASE3_TYPE);

	// Forward PUTREQ_CASE3 to server
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

action update_delreq_to_case3(port) {
	modify_field(op_hdr.optype, DELREQ_CASE3_TYPE);

	// Forward DELREQ_CASE3 to server
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}
