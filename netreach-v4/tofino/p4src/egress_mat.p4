/* Ingress Processing (Normal Operation) */

// Stage 0

action is_hot() {
	modify_field(meta.is_hot, 1);
}

action not_hot() {
	modify_field(meta.is_hot, 0);
}

table is_hot_tbl {
	reads {
		meta.cm1_predicate: exact;
		meta.cm2_predicate: exact;
		meta.cm3_predicate: exact;
		meta.cm4_predicate: exact;
	}
	actions {
		is_hot;
		not_hot;
	}
	default_action: not_hot();
	size: 1;
}

// Stage 10

action update_getreq_inswitch_to_getreq(eport) {
	modify_field(op_hdr.optype, GETREQ);

	remove_header(inswitch_hdr);

	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}

action update_getreq_inswitch_to_getreq_pop(eport) {
	modify_field(op_hdr.optype, GETREQ_POP);

	remove_header(inswitch_hdr);

	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}

action update_getreq_inswitch_to_getreq_nlatest(eport) {
	modify_field(op_hdr.optype, GETREQ_NLATEST);

	remove_header(inswitch_hdr);

	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}

action update_getreq_inswitch_to_getres_for_deleted() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(result_hdr.result, 0);

	remove_header(inswitch_hdr);
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
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
	add_header(val16_hdr);
	add_header(result_hdr);

	modify_field(ig_intr_md_for_tm.ucast_egress_port, inswitch_hdr.eport_for_res);
}

action update_getreq_inswitch_to_getres_for_deleted_by_mirroring() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(result_hdr.result, 0);

	remove_header(inswitch_hdr);
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
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
	add_header(val16_hdr);
	add_header(result_hdr);

	modify_field(eg_intr_md.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(inswitch_hdr.sid); // clone for egress switching
}

action update_getreq_inswitch_to_getres() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(result_hdr.result, 1);

	remove_header(inswitch_hdr);
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
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
	add_header(val16_hdr);
	add_header(result_hdr);

	modify_field(ig_intr_md_for_tm.ucast_egress_port, inswitch_hdr.eport_for_res);
}

action update_getreq_inswitch_to_getres_by_mirroring() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(result_hdr.result, 1);

	remove_header(inswitch_hdr);
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
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
	add_header(val16_hdr);
	add_header(result_hdr);

	modify_field(eg_intr_md.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(inswitch_hdr.sid); // clone for egress switching
}

action update_getres_latest_seq_to_getres() {
	modify_field(op_hdr.optype, GETRES);

	remove_header(seq_hdr);
}

action drop_getres_latest_seq_inswitch() {
	drop();
}

table eg_port_forward_tbl {
	reads {
		op_hdr_optype: exact;
		inswitch_hdr.is_cached: exact;
		meta.is_hot: exact;
		status_hdr.valid: exact;
		status_hdr.is_latest: exact;
		status_hdr.is_deleted: exact;
		inswitch_hdr.is_wrong_pipeline: exact;
	}
	actions {
		update_getreq_inswitch_to_getreq;
		update_getreq_inswitch_to_getreq_pop;
		update_getreq_inswitch_to_getreq_nlatest;
		update_getreq_inswitch_to_getres_for_deleted;
		update_getreq_inswitch_to_getres_for_deleted_by_mirroring;
		update_getreq_inswitch_to_getres;
		update_getreq_inswitch_to_getres_by_mirroring;
		update_getres_latest_seq_to_getres; // GETRES_LATEST_SEQ must be cloned from ingress to egress
		drop_getres_latest_seq_inswitch; // original packet of GETRES_LATEST_SEQ
		nop;
	}
	default_action: nop();
	size: 0;
}














/*action sendback_cloned_getres() {
	modify_field(udp_hdr.srcPort, meta.tmp_sport);
	modify_field(udp_hdr.dstPort, meta.tmp_dport);
	modify_field(op_hdr.optype, GETRES_TYPE);
}*/

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
	//subtract_from_field(udp_hdr.hdrlen, VAL_PKTLEN_MINUS_STAT);

	modify_field(op_hdr.optype, PUTRES_TYPE);

	remove_header(vallen_hdr);
	remove_header(val1_hdr);
	remove_header(val2_hdr);
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
	remove_header(val16_hdr);
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

action update_cloned_putreq_recir_to_putres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	//subtract_from_field(udp_hdr.hdrlen, VAL_PKTLEN_MINUS_STAT_PLUS_SEQ);

	modify_field(op_hdr.optype, PUTRES_TYPE);

	remove_header(vallen_hdr);
	remove_header(val1_hdr);
	remove_header(val2_hdr);
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
	remove_header(val16_hdr);
	remove_header(seq_hdr);
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

action update_cloned_getres_pop_to_getres() {
	modify_field(op_hdr.optype, GETRES_TYPE);
	subtract_from_field(udp_hdr.hdrlen, SEQ_PKTLEN);
	remove_header(seq_hdr);
}

action update_cloned_putreq_pop_to_putres() {
	// Swap udp port
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	//subtract_from_field(udp_hdr.hdrlen, VAL_PKTLEN_MINUS_STAT);

	modify_field(op_hdr.optype, PUTRES_TYPE);

	remove_header(vallen_hdr);
	remove_header(val1_hdr);
	remove_header(val2_hdr);
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
	remove_header(val16_hdr);
	remove_header(seq_hdr);
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

action update_cloned_putreq_large_to_putreq_large_seq() {
	modify_field(op_hdr.optype, PUTREQ_LARGE_SEQ_TYPE);
	add_from_field(udp_hdr.hdrlen, SEQ_PKTLEN);
	add_header(seq_hdr);
	modify_field(seq_hdr.seq, meta.seq_large);
}

action update_cloned_putreq_large_recir_to_putreq_large_seq() {
	modify_field(op_hdr.optype, PUTREQ_LARGE_SEQ_TYPE);
	modify_field(seq_hdr.seq, meta.seq_large);
}

table process_i2e_cloned_packet_tbl {
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
		update_cloned_putreq_large_to_putreq_large_seq;
		update_cloned_putreq_large_recir_to_putreq_large_seq;
		nop;
	}
	default_action: nop();
	size: 8;
}

action update_cloned_getres_pop_evict_to_switch() {
	modify_field(op_hdr.optype, GETRES_POP_EVICT_SWITCH);
}

action update_cloned_getres_pop_evict_case2_to_switch() {
	modify_field(op_hdr.optype, GETRES_POP_EVICT_CASE2_SWITCH);
}

action update_cloned_putreq_pop_evict_to_switch() {
	modify_field(op_hdr.optype, PUTREQ_POP_EVICT_SWITCH);
}

action update_cloned_putreq_pop_evict_case2_to_switch() {
	modify_field(op_hdr.optype, PUTREQ_POP_EVICT_CASE2_SWITCH);
}

action update_cloned_putreq_large_evict_to_switch() {
	modify_field(op_hdr.optype, PUTREQ_LARGE_EVICT_SWITCH);
}

action update_cloned_putreq_large_evict_case2_to_switch() {
	modify_field(op_hdr.optype, PUTREQ_LARGE_EVICT_CASE2_SWITCH);
}

table process_e2e_cloned_packet_tbl {
	reads {
		op_hdr.optye: exact;
	}
	actions {
		update_cloned_getres_pop_evict_to_switch;
		update_cloned_getres_pop_evict_case2_to_switch;
		update_cloned_putreq_pop_evict_to_switch;
		update_cloned_putreq_pop_evict_case2_to_switch;
		update_cloned_putreq_large_evict_to_switch;
		update_cloned_putreq_large_evict_case2_to_switch;
		nop;
	}
	default_action: nop();
	size: 8;
		
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

action update_dstport(port, serveridx) {
	modify_field(udp_hdr.dstPort, port);
	modify_field(serveridx_hdr.serveridx, serveridx);
}

// Only for GETRES_POP_EVICT/CASE2
action update_dstport_reverse(port, serveridx) {
	// NOTE: GETRES_POP_EVICT/CASE2 does not have meta.tmp_dport, while server only cares about dstport instead of srcport to process it in corresponding server; it does not need correct srcport to send response to client; the cloned GETRES_POP has correct dstport for GETRES to corresponding client
	//modify_field(udp_hdr.srcPort, meta.tmp_dport);
	modify_field(udp_hdr.dstPort, port);
	modify_field(serveridx_hdr.serveridx, serveridx);
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

// Egress pipeline forwarding

action forward_to_server_clone_for_pktloss(sid, port) {
	modify_field(eg_intr_md.egress_port, port); // forward to server
	clone_egress_pkt_to_egress(sid); // clone to switch os to cope with packet loss
}

action update_putreq_seq_to_putreq_case3(port) {
	modify_field(op_hdr.optype, PUTREQ_CASE3_TYPE);
	modify_field(eg_intr_md.egress_port, port); // forward to server
}

action update_delreq_seq_to_delreq_case3(port) {
	modify_field(op_hdr.optype, DELREQ_CASE3_TYPE);
	modify_field(eg_intr_md.egress_port, port); // forward to server
}

action update_putreq_large_seq_to_putreq_large_case3(port) {
	modify_field(op_hdr.optype, PUTREQ_LARGE_CASE3_TYPE);
	modify_field(eg_intr_md.egress_port, port); // forward to server
}

action udpate_putres_case3_to_putres(port) {
	modify_field(op_hdr.optype, PUTRES_TYPE);
	subtract_from_field(udp_hdr.hdrlen, SERVERIDX_PKTLEN);
	remove_header(serveridx_hdr);
	modify_field(eg_intr_md.egress_port, port); // forward to client
}

action udpate_delres_case3_to_delres(port) {
	modify_field(op_hdr.optype, DELRES_TYPE);
	subtract_from_field(udp_hdr.hdrlen, SERVERIDX_PKTLEN);
	remove_header(serveridx_hdr);
	modify_field(eg_intr_md.egress_port, port); // forward to client
}

table eg_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
		meta.iscase3: exact;
		meta.isbackup: exact;
	}
	actions {
		forward_to_server_clone_for_pktloss;
		update_putreq_seq_to_putreq_case3;
		update_delreq_seq_to_delreq_case3;
		update_putreq_large_seq_to_putreq_large_case3;
		update_putres_case3_to_putres;
		update_delres_case3_to_delres;
		nop;
	}
	default_action: nop();
	size: 64;
}

action update_getres_udplen(aligned_vallen) {
	// 6(udphdr) + 19(ophdr) + 4(vallen) + aligned_vallen
	add(udp_hdr.hdrLen, aligned_vallen, 29);
}

action update_getres_pop_evict_udplen(aligned_vallen) {
	// 6(udphdr) + 19(ophdr) + 4(vallen) + aligned_vallen + 4(seq)
	add(udp_hdr.hdrLen, aligned_vallen, 33);
}

action update_getres_pop_evict_case2_udplen(aligned_vallen) {
	// 6(udphdr) + 19(ophdr) + 4(vallen) + aligned_vallen + 4(seq) + 1(other)
	add(udp_hdr.hdrLen, aligned_vallen, 34);
}

action update_putres_udplen() {
	// 6(udphdr) + 19(ophdr) + 1(stat)
	modify_field(udp_hdr.hdrLen, 26);
}

action update_putreq_pop_evict_udplen() {
	// 6(udphdr) + 19(ophdr) + 4(vallen) + aligned_vallen + 4(seq)
	add(udp_hdr.hdrLen, aligned_vallen, 33);
}

action update_putreq_pop_evict_case2_udplen() {
	// 6(udphdr) + 19(ophdr) + 4(vallen) + aligned_vallen + 4(seq) + 1(other)
	add(udp_hdr.hdrLen, aligned_vallen, 34);
}

action update_putreq_large_evict_udplen() {
	// 6(udphdr) + 19(ophdr) + 4(vallen) + aligned_vallen + 4(seq)
	add(udp_hdr.hdrLen, aligned_vallen, 33);
}

// NOTE: PUTREQ_LARGE_EVICT_CASE2 does not need other_hdr.isvalid
action update_putreq_large_evict_case2_udplen() {
	// 6(udphdr) + 19(ophdr) + 4(vallen) + aligned_vallen + 4(seq)
	add(udp_hdr.hdrLen, aligned_vallen, 33);
}

action update_putreq_case1_udplen() {
	// 6(udphdr) + 19(ophdr) + 4(vallen) + aligned_vallen
	// NOTE: case 1 does not need seq, as it is sent from switch to switch OS in design without packet loss
	// and we do not need to save it in server-side KVS and hence not need to cope with overwrite
	add(udp_hdr.hdrLen, aligned_vallen, 29);
}

action update_delres_udplen() {
	// 6(udphdr) + 19(ophdr) + 1(stat)
	modify_field(udp_hdr.hdrLen, 26);
}

action update_delreq_case1_udplen() {
	// 6(udphdr) + 19(ophdr) + 4(vallen) + aligned_vallen
	// NOTE: case 1 does not need seq, as it is sent from switch to switch OS in design without packet loss
	// and we do not need to save it in server-side KVS and hence not need to cope with overwrite
	add(udp_hdr.hdrLen, aligned_vallen, 29);
}

table update_udplen_tbl {
	reads {
		op_hdr.optype: exact;
		vallen_hdr.vallen: range;
	}
	actions {
		update_getres_udplen;
		update_getres_pop_evict_udplen;
		update_getres_pop_evict_case2_udplen;
		update_putres_udplen;
		update_putreq_pop_evict_udplen;
		update_putreq_pop_evict_case2_udplen;
		update_putreq_large_evict_udplen;
		update_putreq_large_evict_case2_udplen;
		update_putreq_case1_udplen;
		update_delreq_udplen;
		update_delreq_case1_udplen;
	}
	default_action: nop(); // not change udp_hdr.hdrLen
	size: 256;
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
