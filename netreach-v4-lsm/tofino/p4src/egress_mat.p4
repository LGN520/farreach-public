/* Ingress Processing (Normal Operation) */

// Stage 1

action set_is_hot() {
	modify_field(meta.is_hot, 1);
	//modify_field(debug_hdr.is_hot, 1);
}

action reset_is_hot() {
	modify_field(meta.is_hot, 0);
	//modify_field(debug_hdr.is_hot, 0);
}

@pragma stage 1
table is_hot_tbl {
	reads {
		meta.cm1_predicate: exact;
		meta.cm2_predicate: exact;
		meta.cm3_predicate: exact;
		meta.cm4_predicate: exact;
	}
	actions {
		set_is_hot;
		reset_is_hot;
	}
	default_action: reset_is_hot();
	size: 1;
}

#ifdef RANGE_SUPPORT
#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter process_scanreq_split_counter {
	type : packets_and_bytes;
	direct: process_scanreq_split_tbl;
}
#endif
action process_scanreq_split(server_sid) {
	modify_field(meta.server_sid, server_sid); // clone to server for SCANREQ_SPLIT
	subtract(meta.remain_scannum, split_hdr.max_scannum, split_hdr.cur_scanidx);
}
action process_cloned_scanreq_split(server_sid) {
	add_to_field(udp_hdr.dstPort, 1);
	modify_field(meta.server_sid, server_sid);
	subtract(meta.remain_scannum, split_hdr.max_scannum, split_hdr.cur_scanidx);
}
action reset_meta_server_sid() {
	modify_field(meta.server_sid, 0);
}
@pragma stage 1
table process_scanreq_split_tbl {
	reads {
		op_hdr.optype: exact;
		udp_hdr.dstPort: exact;
		//eg_intr_md_from_parser_aux.clone_src: exact; // NOTE: access intrinsic metadata
		split_hdr.is_clone: exact;
	}
	actions {
		process_scanreq_split;
		process_cloned_scanreq_split;
		reset_meta_server_sid;
	}
	default_action: reset_meta_server_sid();
	size: PROCESS_SCANREQ_SPLIT_ENTRY_NUM;
}
#endif

// Stage 2

action save_client_udpport() {
	modify_field(clone_hdr.client_udpport, udp_hdr.srcPort);
}

@pragma stage 2
table save_client_udpport_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		save_client_udpport;
		nop;
	}
	default_action: nop();
	size: 4;
}

// Stage 9

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter lastclone_lastscansplit_counter {
	type : packets_and_bytes;
	direct: lastclone_lastscansplit_tbl;
}
#endif

action set_is_lastclone() {
	modify_field(meta.is_lastclone_for_pktloss, 1);
	//modify_field(debug_hdr.is_lastclone_for_pktloss, 1);
#ifdef RANGE_SUPPORT
	modify_field(meta.is_last_scansplit, 0);
#endif
}

#ifdef RANGE_SUPPORT
action set_is_lastscansplit() {
	modify_field(meta.is_last_scansplit, 1);
	modify_field(meta.is_lastclone_for_pktloss, 0);
}
#endif

action reset_is_lastclone_lastscansplit() {
	modify_field(meta.is_lastclone_for_pktloss, 0);
	//modify_field(debug_hdr.is_lastclone_for_pktloss, 0);
#ifdef RANGE_SUPPORT
	modify_field(meta.is_last_scansplit, 0);
#endif
}

@pragma stage 9
table lastclone_lastscansplit_tbl {
	reads {
		op_hdr.optype: exact;
		clone_hdr.clonenum_for_pktloss: exact;
#ifdef RANGE_SUPPORT
		meta.remain_scannum: exact;
#endif
	}
	actions {
		set_is_lastclone;
#ifdef RANGE_SUPPORT
		set_is_lastscansplit;
#endif
		reset_is_lastclone_lastscansplit;
	}
	default_action: reset_is_lastclone_lastscansplit();
	size: 8;
}

// Stage 10

action update_getreq_inswitch_to_getreq() {
	modify_field(op_hdr.optype, GETREQ);

	remove_header(shadowtype_hdr);
	remove_header(inswitch_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_getreq_inswitch_to_getreq_pop() {
	modify_field(op_hdr.optype, GETREQ_POP);

	remove_header(shadowtype_hdr);
	remove_header(inswitch_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_getreq_inswitch_to_getreq_nlatest() {
	modify_field(op_hdr.optype, GETREQ_NLATEST);

	remove_header(shadowtype_hdr);
	remove_header(inswitch_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

/*action update_getreq_inswitch_to_getres_for_deleted() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
	modify_field(stat_hdr.stat, 0);

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
	add_header(stat_hdr);

	modify_field(eg_intr_md.egress_port, inswitch_hdr.eport_for_res);
}*/

action update_getreq_inswitch_to_getres_for_deleted_by_mirroring(client_sid, server_port) {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
	modify_field(stat_hdr.stat, 0);
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, clone_hdr.client_udpport);

	remove_header(inswitch_hdr);
	/*add_header(vallen_hdr);
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
	add_header(val16_hdr);*/
	add_header(stat_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(client_sid); // clone to client (inswitch_hdr.client_sid)
}

/*action update_getreq_inswitch_to_getres() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
	modify_field(stat_hdr.stat, 1);

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
	add_header(stat_hdr);

	modify_field(eg_intr_md.egress_port, inswitch_hdr.eport_for_res);
}*/

action update_getreq_inswitch_to_getres_by_mirroring(client_sid, server_port) {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
	modify_field(stat_hdr.stat, 1);
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, clone_hdr.client_udpport);

	remove_header(inswitch_hdr);
	/*add_header(vallen_hdr);
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
	add_header(val16_hdr);*/
	add_header(stat_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(client_sid); // clone to client (inswitch_hdr.client_sid)
}

action update_getres_latest_seq_to_getres() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
	modify_field(stat_hdr.stat, 1);

	remove_header(seq_hdr);
	add_header(stat_hdr);
}

/*field_list clone_field_list_for_pktloss {
	meta.clonenum_for_pktloss;
}*/

//action update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss(switchos_sid, port, stat) {
action update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss(switchos_sid, stat, reflector_port) {
	modify_field(op_hdr.optype, GETRES_LATEST_SEQ_INSWITCH_CASE1);
	modify_field(shadowtype_hdr.shadowtype, GETRES_LATEST_SEQ_INSWITCH_CASE1);
	modify_field(stat_hdr.stat, stat);
	modify_field(udp_hdr.dstPort, reflector_port);
	//modify_field(meta.clonenum_for_pktloss, 1); // 3 ACKs (clone w/ 1 -> clone w/ 0 -> no clone w/ case1)
	modify_field(clone_hdr.clonenum_for_pktloss, 2); // 3 ACKs (drop w/ 2 -> clone w/ 1 -> clone w/ 0 -> no clone w/ case1)

	//remove_header(inswitch_hdr);
	add_header(stat_hdr);
	add_header(clone_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	//clone_egress_pkt_to_egress(switchos_sid, clone_field_list_for_pktloss); // clone to switchos
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action forward_getres_latest_seq_inswitch_case1_clone_for_pktloss(switchos_sid) {
	subtract_from_field(clone_hdr.clonenum_for_pktloss, 1);

	//clone_egress_pkt_to_egress(switchos_sid, clone_field_list_for_pktloss); // clone to switchos
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action forward_getres_latest_seq_inswitch_case1() {
}

action update_getres_deleted_seq_to_getres() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
	modify_field(stat_hdr.stat, 0);

	remove_header(seq_hdr);
	add_header(stat_hdr);
}

//action update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss(switchos_sid, port, stat) {
action update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss(switchos_sid, stat, reflector_port) {
	modify_field(op_hdr.optype, GETRES_DELETED_SEQ_INSWITCH_CASE1);
	modify_field(shadowtype_hdr.shadowtype, GETRES_DELETED_SEQ_INSWITCH_CASE1);
	modify_field(stat_hdr.stat, stat);
	modify_field(udp_hdr.dstPort, reflector_port);
	//modify_field(meta.clonenum_for_pktloss, 1); // 3 ACKs (clone w/ 1 -> clone w/ 0 -> no clone w/ case1)
	modify_field(clone_hdr.clonenum_for_pktloss, 2); // 3 ACKs (drop w/ 2 -> clone w/ 1 -> clone w/ 0 -> no clone w/ case1)

	//remove_header(inswitch_hdr);
	add_header(stat_hdr);
	add_header(clone_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	//clone_egress_pkt_to_egress(switchos_sid, clone_field_list_for_pktloss); // clone to switchos
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss(switchos_sid) {
	subtract_from_field(clone_hdr.clonenum_for_pktloss, 1);

	//clone_egress_pkt_to_egress(switchos_sid, clone_field_list_for_pktloss); // clone to switchos
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action forward_getres_deleted_seq_inswitch_case1() {
}

//action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss(switchos_sid, port) {
action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss(switchos_sid, reflector_port) {
	modify_field(op_hdr.optype, CACHE_POP_INSWITCH_ACK);
	modify_field(udp_hdr.dstPort, reflector_port);
	//modify_field(meta.clonenum_for_pktloss, 1); // 3 ACKs (clone w/ 1 -> clone w/ 0 -> no clone w/ ack)
	modify_field(clone_hdr.clonenum_for_pktloss, 2); // 3 ACKs (drop w/ 2 -> clone w/ 1 -> clone w/ 0 -> no clone w/ ack)

	/*remove_header(vallen_hdr);
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
	remove_header(val16_hdr);*/
	remove_header(shadowtype_hdr);
	remove_header(seq_hdr);
	remove_header(inswitch_hdr);
	add_header(clone_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	//clone_egress_pkt_to_egress(switchos_sid, clone_field_list_for_pktloss); // clone to switchos
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action forward_cache_pop_inswitch_ack_clone_for_pktloss(switchos_sid) {
	subtract_from_field(clone_hdr.clonenum_for_pktloss, 1);

	//clone_egress_pkt_to_egress(switchos_sid, clone_field_list_for_pktloss); // clone to switchos
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action forward_cache_pop_inswitch_ack() {
}

action update_putreq_inswitch_to_putreq_seq() {
	modify_field(op_hdr.optype, PUTREQ_SEQ);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_putreq_inswitch_to_putreq_pop_seq() {
	modify_field(op_hdr.optype, PUTREQ_POP_SEQ);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_POP_SEQ);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

/*action update_putreq_inswitch_to_putres() {
	modify_field(op_hdr.optype, PUTRES);
	modify_field(shadowtype_hdr.shadowtype, PUTRES);
	modify_field(stat_hdr.stat, 1);

	remove_header(inswitch_hdr);
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
	add_header(stat_hdr);

	modify_field(eg_intr_md.egress_port, inswitch_hdr.eport_for_res);
}*/

action update_putreq_inswitch_to_putres_by_mirroring(client_sid, server_port) {
	modify_field(op_hdr.optype, PUTRES);
	modify_field(shadowtype_hdr.shadowtype, PUTRES);
	modify_field(stat_hdr.stat, 1);
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, clone_hdr.client_udpport);

	remove_header(inswitch_hdr);
	/*remove_header(vallen_hdr);
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
	remove_header(val16_hdr);*/
	add_header(stat_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(client_sid); // clone to client (inswitch_hdr.client_sid)
}

/*field_list clone_field_list_for_pktloss_and_res {
	meta.clonenum_for_pktloss;
	meta.client_udpport;
	// NOTE: extracted fields cannot be used as clone fields
	//inswitch_hdr.is_wrong_pipeline;
	//inswitch_hdr.eport_for_res;
	//inswitch_hdr.client_sid;
}*/

//action update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(switchos_sid, port, stat) {
action update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(switchos_sid, stat, reflector_port) {
	modify_field(op_hdr.optype, PUTREQ_SEQ_INSWITCH_CASE1);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ_INSWITCH_CASE1);
	modify_field(stat_hdr.stat, stat);
	modify_field(udp_hdr.dstPort, reflector_port);
	//modify_field(meta.clonenum_for_pktloss, 2); // 3 ACKs (clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> drop w/ PUTRES)
	modify_field(clone_hdr.clonenum_for_pktloss, 3); // 3 ACKs (drop w/ 3 -> clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> drop w/ PUTRES)

	//remove_header(inswitch_hdr);
	add_header(seq_hdr);
	add_header(stat_hdr);
	add_header(clone_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	//clone_egress_pkt_to_egress(switchos_sid, clone_field_list_for_pktloss_and_res); // clone to switchos
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(switchos_sid) {
	subtract_from_field(clone_hdr.clonenum_for_pktloss, 1);

	//clone_egress_pkt_to_egress(switchos_sid, clone_field_list_for_pktloss_and_res); // clone to switchos
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

/*action update_putreq_seq_inswitch_case1_to_putres() {
	modify_field(op_hdr.optype, PUTRES);
	modify_field(shadowtype_hdr.shadowtype, PUTRES);
	modify_field(stat_hdr.stat, 1);

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

	remove_header(inswitch_hdr);

	modify_field(eg_intr_md.egress_port, inswitch_hdr.eport_for_res);
}*/

action update_putreq_seq_inswitch_case1_to_putres_by_mirroring(client_sid, server_port) {
	modify_field(op_hdr.optype, PUTRES);
	modify_field(shadowtype_hdr.shadowtype, PUTRES);
	modify_field(stat_hdr.stat, 1);
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, clone_hdr.client_udpport);

	/*remove_header(vallen_hdr);
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
	remove_header(val16_hdr);*/
	remove_header(seq_hdr);

	remove_header(inswitch_hdr);
	remove_header(clone_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(client_sid); // clone to client (inswitch_hdr.client_sid)
}

action update_putreq_inswitch_to_putreq_seq_case3() {
	modify_field(op_hdr.optype, PUTREQ_SEQ_CASE3);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ_CASE3);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_putreq_inswitch_to_putreq_pop_seq_case3() {
	modify_field(op_hdr.optype, PUTREQ_POP_SEQ_CASE3);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_POP_SEQ_CASE3);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_delreq_inswitch_to_delreq_seq() {
	modify_field(op_hdr.optype, DELREQ_SEQ);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

/*action update_delreq_inswitch_to_delres() {
	modify_field(op_hdr.optype, DELRES);
	modify_field(shadowtype_hdr.shadowtype, DELRES);
	modify_field(stat_hdr.stat, 1);

	remove_header(inswitch_hdr);
	add_header(stat_hdr);

	modify_field(eg_intr_md.egress_port, inswitch_hdr.eport_for_res);
}*/

action update_delreq_inswitch_to_delres_by_mirroring(client_sid, server_port) {
	modify_field(op_hdr.optype, DELRES);
	modify_field(shadowtype_hdr.shadowtype, DELRES);
	modify_field(stat_hdr.stat, 1);
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, clone_hdr.client_udpport);

	remove_header(inswitch_hdr);
	add_header(stat_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(client_sid); // clone to client (inswitch_hdr.client_sid)
}

//action update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(switchos_sid, port, stat) {
action update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(switchos_sid, stat, reflector_port) {
	modify_field(op_hdr.optype, DELREQ_SEQ_INSWITCH_CASE1);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ_INSWITCH_CASE1);
	modify_field(stat_hdr.stat, stat);
	modify_field(udp_hdr.dstPort, reflector_port);
	//modify_field(meta.clonenum_for_pktloss, 2); // 3 ACKs (clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> DELRES)
	modify_field(clone_hdr.clonenum_for_pktloss, 3); // 3 ACKs (drop w/ 3 -> clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> DELRES)

	//remove_header(inswitch_hdr);
	/*add_header(vallen_hdr);
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
	add_header(val16_hdr);*/
	add_header(seq_hdr);
	add_header(stat_hdr);
	add_header(clone_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	//clone_egress_pkt_to_egress(switchos_sid, clone_field_list_for_pktloss_and_res); // clone to switchps
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(switchos_sid) {
	subtract_from_field(clone_hdr.clonenum_for_pktloss, 1);

	//clone_egress_pkt_to_egress(switchos_sid, clone_field_list_for_pktloss_and_res); // clone to switchos
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

/*action update_delreq_seq_inswitch_case1_to_delres() {
	modify_field(op_hdr.optype, DELRES);
	modify_field(shadowtype_hdr.shadowtype, DELRES);
	modify_field(stat_hdr.stat, 1);

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

	remove_header(inswitch_hdr);

	modify_field(eg_intr_md.egress_port, inswitch_hdr.eport_for_res);
}*/

action update_delreq_seq_inswitch_case1_to_delres_by_mirroring(client_sid, server_port) {
	modify_field(op_hdr.optype, DELRES);
	modify_field(shadowtype_hdr.shadowtype, DELRES);
	modify_field(stat_hdr.stat, 1);
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, clone_hdr.client_udpport);

	/*remove_header(vallen_hdr);
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
	remove_header(val16_hdr);*/
	remove_header(seq_hdr);

	remove_header(inswitch_hdr);
	remove_header(clone_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(client_sid); // clone to client (inswitch_hdr.client_sid)
}

action update_delreq_inswitch_to_delreq_seq_case3() {
	modify_field(op_hdr.optype, DELREQ_SEQ_CASE3);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ_CASE3);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

#ifdef RANGE_SUPPORT
action forward_scanreq_split_and_clone(server_sid) {
	modify_field(split_hdr.is_clone, 1);
	add_to_field(split_hdr.cur_scanidx, 1);
	// NOTE: eg_intr_md.egress_port has been set by process_(cloned)_scanreq_split_tbl in stage 0
	clone_egress_pkt_to_egress(server_sid); // clone to server (meta.server_sid)
}
action forward_scanreq_split() {
	modify_field(split_hdr.is_clone, 1);
	add_to_field(split_hdr.cur_scanidx, 1);
	// NOTE: eg_intr_md.egress_port has been set by process_(cloned)_scanreq_split_tbl in stage 0
}
#endif

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter eg_port_forward_counter {
	type : packets_and_bytes;
	direct: eg_port_forward_tbl;
}
#endif

table eg_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
		meta.is_hot: exact;
		//debug_hdr.is_hot: exact;
		meta.validvalue: exact;
		meta.is_latest: exact;
		meta.is_deleted: exact;
		//inswitch_hdr.is_wrong_pipeline: exact;
		inswitch_hdr.client_sid: exact;
		meta.is_lastclone_for_pktloss: exact;
		//debug_hdr.is_lastclone_for_pktloss: exact;
		inswitch_hdr.snapshot_flag: exact;
		meta.is_case1: exact;
#ifdef RANGE_SUPPORT
		meta.is_last_scansplit: exact;
		meta.server_sid: exact;
#endif
	}
	actions {
		update_getreq_inswitch_to_getreq;
		update_getreq_inswitch_to_getreq_pop;
		update_getreq_inswitch_to_getreq_nlatest;
		//update_getreq_inswitch_to_getres_for_deleted;
		update_getreq_inswitch_to_getres_for_deleted_by_mirroring;
		//update_getreq_inswitch_to_getres;
		update_getreq_inswitch_to_getres_by_mirroring;
		update_getres_latest_seq_to_getres; // GETRES_LATEST_SEQ must be cloned from ingress to egress
		update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss; // drop original packet of GETRES_LATEST_SEQ -> clone for first GETRES_LATEST_SEQ_INSWITCH_CASE1
		//drop_getres_latest_seq_inswitch; // drop original packet of GETRES_LATEST_SEQ
		forward_getres_latest_seq_inswitch_case1_clone_for_pktloss; // not last clone of GETRES_LATEST_SEQ_INSWITCH_CASE1
		forward_getres_latest_seq_inswitch_case1; // last clone of GETRES_LATEST_SEQ_INSWITCH_CASE1
		update_getres_deleted_seq_to_getres; // GETRES_DELETED_SEQ must be cloned from ingress to egress
		update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss; // drop original packet of GETRES_DELETED_SEQ -> clone for first GETRES_DELETED_SEQ_INSWITCH_CASE1
		//drop_getres_deleted_seq_inswitch; // original packet of GETRES_DELETED_SEQ
		forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss; // not last clone of GETRES_DELETED_SEQ_INSWITCH_CASE1
		forward_getres_deleted_seq_inswitch_case1; // last clone of GETRES_DELETED_SEQ_INSWITCH_CASE1
		update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss; // clone for first CACHE_POP_INSWITCH_ACK
		forward_cache_pop_inswitch_ack_clone_for_pktloss; // not last clone of CACHE_POP_INSWITCH_ACK
		forward_cache_pop_inswitch_ack; // last clone of CACHE_POP_INSWITCH_ACK
		update_putreq_inswitch_to_putreq_seq;
		update_putreq_inswitch_to_putreq_pop_seq;
		//update_putreq_inswitch_to_putres;
		update_putreq_inswitch_to_putres_by_mirroring;
		update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres;
		forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres;
		//update_putreq_seq_inswitch_case1_to_putres;
		update_putreq_seq_inswitch_case1_to_putres_by_mirroring;
		update_putreq_inswitch_to_putreq_seq_case3;
		update_putreq_inswitch_to_putreq_pop_seq_case3;
		update_delreq_inswitch_to_delreq_seq;
		//update_delreq_inswitch_to_delres;
		update_delreq_inswitch_to_delres_by_mirroring;
		update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres;
		forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres;
		//update_delreq_seq_inswitch_case1_to_delres;
		update_delreq_seq_inswitch_case1_to_delres_by_mirroring;
		update_delreq_inswitch_to_delreq_seq_case3;
#ifdef RANGE_SUPPORT
		forward_scanreq_split_and_clone;
		forward_scanreq_split;
#endif
		nop;
	}
	default_action: nop();
#ifdef DEBUG
	size: 8192; // assign more entries for debugging
#else
	size: 1024;
#endif
}


// NOTE: only one operand in add can be action parameter or constant -> resort to controller to configure different hdrlen
/*// CACHE_POP_INSWITCH_ACK
action update_onlyop_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 17(ophdr) + 1(debug_hdr)
	//modify_field(udp_hdr.hdrlen, 26);
	//modify_field(ipv4_hdr.totalLen, 46)
	modify_field(udp_hdr.hdrlen, 25);
	modify_field(ipv4_hdr.totalLen, 45);
}

// GETRES
action update_val_stat_pktlen(aligned_vallen) {
	// 20[iphdr] + 8(udphdr) + 17(ophdr) + 2(vallen) + aligned_vallen(val) + 1(shadowtype) + 1(stat) + 1(debug_hdr)
	//add(udp_hdr.hdrlen, aligned_vallen, 30);
	//add(ipv4_hdr.totalLen, aligned_vallen, 50);
	add(udp_hdr.hdrlen, aligned_vallen, 29);
	add(ipv4_hdr.totalLen, aligned_vallen, 49);
}

// GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1
action update_val_seq_inswitch_stat_pktlen(aligned_vallen) {
	// [20(iphdr)] + 8(udphdr) + 17(ophdr) + 2(vallen) + aligned_vallen(val) + 1(shadowtype) + 4(seq) + 9(inswitch) + 1(stat) + 1(debug_hdr)
	//add(udp_hdr.hdrlen, aligned_vallen, 43);
	//add(ipv4_hdr.totalLen, aligned_vallen, 63);
	add(udp_hdr.hdrlen, aligned_vallen, 42);
	add(ipv4_hdr.totalLen, aligned_vallen, 62);
}

// PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3
action update_val_seq_pktlen(aligned_vallen) {
	// [20(iphdr)] + 8(udphdr) + 17(ophdr) + 2(vallen) + aligned_vallen(val) + 1(shadowtype) + 4(seq) + 1(debug_hdr)
	//add(udp_hdr.hdrlen, aligned_vallen, 33);
	//add(ipv4_hdr.totalLen, aligned_vallen, 53);
	add(udp_hdr.hdrlen, aligned_vallen, 32);
	add(ipv4_hdr.totalLen, aligned_vallen, 52);
}

// PUTRES, DELRES
action update_stat_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 17(ophdr) + 1(shadowtype) + 1(stat) + 1(debug_hdr)
	//modify_field(udp_hdr.hdrlen, 28);
	//modify_field(ipv4_hdr.totalLen, 48);
	modify_field(udp_hdr.hdrlen, 27);
	modify_field(ipv4_hdr.totalLen, 47);
}

// DELREQ_SEQ, DELREQ_SEQ_CASE3
action update_seq_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 17(ophdr) + 1(shadowtype) + 4(seq) + 1(debug_hdr)
	//modify_field(udp_hdr.hdrlen, 31);
	/modify_field(ipv4_hdr.totalLen, 51);
	modify_field(udp_hdr.hdrlen, 30);
	modify_field(ipv4_hdr.totalLen, 50);
}
 
// SCANREQ_SPLIT
action update_scanreqsplit_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 17(ophdr) + 16(endkey) + 5(split_hdr)
	modify_field(udp_hdr.hdrlen, 46);
	modify_field(ipv4_hdr.totalLen, 66);
}*/

action update_pktlen(udplen, iplen) {
	modify_field(udp_hdr.hdrlen, udplen);
	modify_field(ipv4_hdr.totalLen, iplen);
}

@pragma stage 11
table update_pktlen_tbl {
	reads {
		op_hdr.optype: exact;
		vallen_hdr.vallen: range;
	}
	actions {
		/*update_onlyop_pktlen;
		update_val_stat_pktlen;
		update_val_seq_inswitch_stat_pktlen;
		update_val_seq_pktlen;
		update_stat_pktlen;
		update_seq_pktlen;*/
		update_pktlen;
		nop;
	}
	default_action: nop(); // not change udp_hdr.hdrlen (GETREQ/GETREQ_POP/GETREQ_NLATEST)
	size: 256;
}

action update_ipmac_srcport_server2client(client_mac, server_mac, client_ip, server_ip, server_port) {
	modify_field(ethernet_hdr.srcAddr, server_mac);
	modify_field(ethernet_hdr.dstAddr, client_mac);
	modify_field(ipv4_hdr.srcAddr, server_ip);
	modify_field(ipv4_hdr.dstAddr, client_ip);
	modify_field(udp_hdr.srcPort, server_port);
}

// NOTE: as we use software link, switch_mac/ip = reflector_mac/ip
// NOTE: although we use client_port to update srcport here, reflector does not care about the specific value of srcport
action update_ipmac_srcport_client2switch(client_mac, switch_mac, client_ip, switch_ip, client_port) {
	modify_field(ethernet_hdr.srcAddr, client_mac);
	modify_field(ethernet_hdr.dstAddr, switch_mac);
	modify_field(ipv4_hdr.srcAddr, client_ip);
	modify_field(ipv4_hdr.dstAddr, switch_ip);
	modify_field(udp_hdr.srcPort, client_port);
}

// NOTE: dstport of REQ, RES, and notification has been updated in partition_tbl, server, and eg_port_forwardtbl
table update_ipmac_srcport_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		update_ipmac_srcport_server2client;
		update_ipmac_srcport_client2switch;
		nop;
	}
	default_action: nop();
	size: 8;
}

action add_only_vallen() {
	add_header(vallen_hdr);
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
}

action add_to_val1() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
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
}

action add_to_val2() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
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
}

action add_to_val3() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
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
}

action add_to_val4() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
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
}

action add_to_val5() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
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
}

action add_to_val6() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
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
}

action add_to_val7() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val8() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	add_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val9() {
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
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val10() {
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
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val11() {
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
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val12() {
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
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val13() {
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
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val14() {
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
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val15() {
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
	remove_header(val16_hdr);
}

action add_to_val16() {
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
}

action remove_all() {
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
}

@pragma stage 11
table add_and_remove_value_header_tbl {
	reads {
		op_hdr.optype: exact;
		vallen_hdr.vallen: range;
	}
	actions {
		add_only_vallen;
		add_to_val1;
		add_to_val2;
		add_to_val3;
		add_to_val4;
		add_to_val5;
		add_to_val6;
		add_to_val7;
		add_to_val8;
		add_to_val9;
		add_to_val10;
		add_to_val11;
		add_to_val12;
		add_to_val13;
		add_to_val14;
		add_to_val15;
		add_to_val16;
		remove_all;
	}
	default_action: remove_all();
	size: 256;
}

action drop_getres_latest_seq_inswitch() {
	// NOTE: MATs after drop will not be accessed
	drop();
}

action drop_getres_deleted_seq_inswitch() {
	drop();
}

@pragma stage 11
table drop_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		drop_getres_latest_seq_inswitch;
		drop_getres_deleted_seq_inswitch;
		nop;
	}
	default_action: nop();
	size: 2;
}
