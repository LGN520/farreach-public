
action sendback_cloned_getres() {
	modify_field(udp_hdr.srcPort, meta.tmp_sport);
	modify_field(udp_hdr.dstPort, meta.tmp_dport);
	modify_field(op_hdr.optype, GETRES_TYPE);
}

action update_cloned_delreq_to_delres() {
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	add_to_field(udp_hdr.hdrlen, STAT_PKTLEN);

	modify_field(op_hdr.optype, DELRES_TYPE);
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

action update_cloned_delreq_recir_to_delres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	subtract_from_field(udp_hdr.hdrlen, SEQ_PKTLEN_MINUS_STAT);

	modify_field(op_hdr.optype, DELRES_TYPE);

	remove_header(seq_hdr);
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

action update_cloned_putreq_to_putres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	subtract_from_field(udp_hdr.hdrlen, VAL_PKTLEN_MINUS_STAT);

	modify_field(op_hdr.optype, PUTRES_TYPE);

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
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

action update_cloned_putreq_recir_to_putres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	subtract_from_field(udp_hdr.hdrlen, VAL_PKTLEN_MINUS_STAT_PLUS_SEQ);

	modify_field(op_hdr.optype, PUTRES_TYPE);

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
	remove_header(seq_hdr);
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

action update_cloned_getres_pop_to_getres() {
	modify_field(op_hdr.optype, GETRES_TYPE);
}

action update_cloned_putreq_pop_to_putres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	subtract_from_field(udp_hdr.hdrlen, VAL_PKTLEN_MINUS_STAT);

	modify_field(op_hdr.optype, PUTRES_TYPE);

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
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

table process_cloned_packet_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		update_cloned_putreq_to_putres;
		update_cloned_putreq_recir_to_putres;
		update_cloned_delreq_to_delres;
		update_cloned_delreq_recir_to_delres;
		update_cloned_getres_pop_to_getres;
		update_cloned_putreq_pop_to_putres;
		nop;
	}
	default_action: nop();
	size: 128;
}

action update_putreq_may_case3_to_case3() {
	modify_field(op_hdr.optype, PUTREQ_CASE3_TYPE);
	subtract_from_field(udp_hdr.hdrlen, OTHER_PKTLEN);
	remove_header(other_hdr);
}

action update_putreq_may_case3_to_putreq() {
	modify_field(op_hdr.optype, PUTREQ_TYPE);
	subtract_from_field(udp_hdr.hdrlen, OTHER_PKTLEN);
	remove_header(other_hdr);
}

action update_delreq_may_case3_to_case3() {
	modify_field(op_hdr.optype, DELREQ_CASE3_TYPE);
	subtract_from_field(udp_hdr.hdrlen, OTHER_PKTLEN);
	remove_header(other_hdr);
}

action update_delreq_may_case3_to_delreq() {
	modify_field(op_hdr.optype, DELREQ_TYPE);
	subtract_from_field(udp_hdr.hdrlen, OTHER_PKTLEN);
	remove_header(other_hdr);
}

table process_may_case3_tbl {
	reads {
		op_hdr.optype: exact;
		other_hdr.iscase3: exact;
	}
	actions {
		update_putreq_may_case3_to_case3;
		update_putreq_may_case3_to_putreq;
		update_delreq_may_case3_to_case3;
		update_delreq_may_case3_to_delreq;
		nop;
	}
	default_action: nop();
	size: 128;
}

field_list hash_fields {
	op_hdr.keylolo;
	op_hdr.keylohi;
	op_hdr.keyhilo;
	op_hdr.keyhihi;
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

table eg_calculate_hash_tbl {
	actions {
		calculate_hash;
	}
	default_action: calculate_hash();
	size: 1;
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
		eg_intr_md.egress_port: exact;
		meta.hashidx: range;
	}
	actions {
		update_dstport;
		nop;
	}
	default_action: nop();
	size: 128;
}

table hash_partition_reverse_tbl {
	reads {
		udp_hdr.srcPort: exact;
		eg_intr_md.egress_port: exact;
		meta.hashidx: range;
	}
	actions {
		update_dstport_reverse;
		nop;
	}
	default_action: nop();
	size: 128;
}

action update_macaddr_s2c(tmp_srcmac, tmp_dstmac) {
	modify_field(ethernet_hdr.dstAddr, tmp_srcmac);
	modify_field(ethernet_hdr.srcAddr, tmp_dstmac);
}

action update_macaddr_c2s(tmp_srcmac, tmp_dstmac) {
	modify_field(ethernet_hdr.srcAddr, tmp_srcmac);
	modify_field(ethernet_hdr.dstAddr, tmp_dstmac);
}

table update_macaddr_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		update_macaddr_s2c;
		update_macaddr_c2s;
		nop;
	}
	default_action: nop();
	size: 8;
}
