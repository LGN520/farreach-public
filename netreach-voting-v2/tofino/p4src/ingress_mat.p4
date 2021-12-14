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
	modify_field(meta.iszerovote, 1); // 1 for non-zero vote
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

action cache_lookup() {
	modify_field(meta.iscached, 1);
}

action cache_lookup_default() {
	modify_field(meta.iscached, 0);
}

table cache_lookup_tbl {
	reads {
		op_hdr.keylololo: exact;
		op_hdr.keylolohi: exact;
		op_hdr.keylohilo: exact;
		op_hdr.keylohihi: exact;
		op_hdr.keyhilolo: exact;
		op_hdr.keyhilohi: exact;
		op_hdr.keyhihilo: exact;
		op_hdr.keyhihihi: exact;
	}
	actions {
		cache_lookup;
		cache_lookup_default;
	}
	default_action: cache_lookup_default();
}

// Stage 11

action update_getreq_to_getres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	add_to_field(udp_hdr.hdrlen, VAL_PKTLEN);

	modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);

	modify_field(op_hdr.optype, GETRES_TYPE);
	add_header(vallen_hdr);
	add_header(val1_hdr);
	/*add_header(val2_hdr);
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

action update_getreq_to_getreq_pop(port) {
	modify_field(op_hdr.optype, GETREQ_POP_TYPE); // Trigger eviction
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

action update_getreq_to_getreq_nlatest(port) {
	modify_field(op_hdr.optype, GETREQ_NLATEST_TYPE); // Trigger eviction
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

action update_getres_npop_to_getres(port) {
	modify_field(op_hdr.optype, GETRES_TYPE); // Not exist
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

action port_forward(port) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

table port_forward_tbl {
	reads {
		op_hdr.optype: exact;
		meta.iscached: exact;
		meta.isvalid: exact;
		meta.islatest: exact;
		meta.iszerovote: exact;
		meta.islock: exact;
		meta.being_evicted: exact;
	}
	actions {
		update_getreq_to_getres;
		update_getreq_to_getreq_pop; // trigger eviction
		update_getreq_to_getreq_nlatest;
		update_getres_npop_to_getres;
		port_forward;
		nop;
	}
	default_action: nop();
	size: 1024;  
}
