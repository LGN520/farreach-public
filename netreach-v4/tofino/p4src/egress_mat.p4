/* Ingress Processing (Normal Operation) */

// Stage 0

#ifdef RANGE_SUPPORT
/*action process_scanreq_split(eport, sid) {
	modify_field(eg_intr_md.egress_port, eport);
	modify_field(inswitch_hdr.sid, sid);
	subtract(meta.remain_scannum, split_hdr.max_scannum, split_hdr.cur_scanidx);
}*/
action process_scanreq_split(sid) {
	modify_field(inswitch_hdr.sid, sid);
	subtract(meta.remain_scannum, split_hdr.max_scannum, split_hdr.cur_scanidx);
}
@pragma stage 0
table process_scanreq_split_tbl {
	reads {
		op_hdr.optype: exact;
		udp_hdr.dstPort: exact;
	}
	actions {
		process_scanreq_split;
		nop;
	}
	default_action: nop();
	size: MAX_SERVERNUM;
}
/*action process_cloned_scanreq_split(eport, sid) {
	add_to_field(udp_hdr.dstPort, 1);
	modify_field(eg_intr_md.egress_port, eport);
	modify_field(inswitch_hdr.sid, sid);
	subtract(meta.remain_scannum, split_hdr.max_scannum, split_hdr.cur_scanidx);
}*/
action process_cloned_scanreq_split(sid) {
	add_to_field(udp_hdr.dstPort, 1);
	modify_field(inswitch_hdr.sid, sid);
	subtract(meta.remain_scannum, split_hdr.max_scannum, split_hdr.cur_scanidx);
}
@pragma stage 0
table process_cloned_scanreq_split_tbl {
	reads {
		op_hdr.optype: exact;
		udp_hdr.dstPort: exact;
	}
	actions {
		process_cloned_scanreq_split;
		nop;
	}
	default_action: nop();
	size: MAX_SERVERNUM;
}
#endif

// Stage 1

#ifdef RANGE_SUPPORT
action set_is_last_scansplit() {
	modify_field(meta.is_last_scansplit, 1);
}
action reset_is_last_scansplit() {
	modify_field(meta.is_last_scansplit, 0);
}
@pragma stage 1
table is_last_scansplit_tbl {
	reads {
		op_hdr.optype: exact;
		meta.remain_scannum: exact;
	}
	actions {
		set_is_last_scansplit;
		reset_is_last_scansplit;
	}
	default_action: reset_is_last_scansplit();
	size: 1;
}
#endif

action set_is_hot() {
	modify_field(debug_hdr.is_hot, 1);
}

action reset_is_hot() {
	modify_field(debug_hdr.is_hot, 0);
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

action save_client_port() {
	modify_field(meta.client_port, udp_hdr.srcPort);
}

@pragma stage 1
table save_client_port_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		save_client_port;
		nop;
	}
	default_action: nop();
	size: 4;
}

// Stage 9

action set_is_lastclone() {
	modify_field(debug_hdr.is_lastclone_for_pktloss, 1);
}

action reset_is_lastclone() {
	modify_field(debug_hdr.is_lastclone_for_pktloss, 0);
}

@pragma stage 9
table lastclone_tbl {
	reads {
		op_hdr.optype: exact;
		meta.clonenum_for_pktloss: exact;
	}
	actions {
		set_is_lastclone;
		reset_is_lastclone;
	}
	default_action: reset_is_lastclone();
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

action update_getreq_inswitch_to_getres_for_deleted_by_mirroring(sid, server_port) {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
	modify_field(stat_hdr.stat, 0);
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, meta.client_port);

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

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(sid); // clone to client (inswitch_hdr.sid)
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

action update_getreq_inswitch_to_getres_by_mirroring(sid, server_port) {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
	modify_field(stat_hdr.stat, 1);
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, meta.client_port);

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

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(sid); // clone to client (inswitch_hdr.sid)
}

action update_getres_latest_seq_to_getres() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
	modify_field(stat_hdr.stat, 1);

	remove_header(seq_hdr);
	add_header(stat_hdr);
}

field_list clone_field_list_for_pktloss {
	meta.clonenum_for_pktloss;
}

//action update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss(sid, port, stat) {
action update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss(sid, stat, reflector_port) {
	modify_field(op_hdr.optype, GETRES_LATEST_SEQ_INSWITCH_CASE1);
	modify_field(shadowtype_hdr.shadowtype, GETRES_LATEST_SEQ_INSWITCH_CASE1);
	modify_field(stat_hdr.stat, stat);
	modify_field(udp_hdr.dstPort, reflector_port);
	//modify_field(meta.clonenum_for_pktloss, 1); // 3 ACKs (clone w/ 1 -> clone w/ 0 -> no clone)
	modify_field(meta.clonenum_for_pktloss, 2); // 3 ACKs (drop w/ 2 -> clone w/ 1 -> clone w/ 0 -> no clone)

	//remove_header(inswitch_hdr);
	add_header(stat_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss); // clone to switchos
}

action drop_getres_latest_seq_inswitch() {
	drop();
}

action forward_getres_latest_seq_inswitch_case1_clone_for_pktloss(sid) {
	subtract_from_field(meta.clonenum_for_pktloss, 1);

	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss); // clone to switchos
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

//action update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss(sid, port, stat) {
action update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss(sid, stat, reflector_port) {
	modify_field(op_hdr.optype, GETRES_DELETED_SEQ_INSWITCH_CASE1);
	modify_field(shadowtype_hdr.shadowtype, GETRES_DELETED_SEQ_INSWITCH_CASE1);
	modify_field(stat_hdr.stat, stat);
	modify_field(udp_hdr.dstPort, reflector_port);
	//modify_field(meta.clonenum_for_pktloss, 1); // 3 ACKs (clone w/ 1 -> clone w/ 0 -> no clone)
	modify_field(meta.clonenum_for_pktloss, 2); // 3 ACKs (drop w/ 2 -> clone w/ 1 -> clone w/ 0 -> no clone)

	//remove_header(inswitch_hdr);
	add_header(stat_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss); // clone to switchos
}

action drop_getres_deleted_seq_inswitch() {
	drop();
}

action forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss(sid) {
	subtract_from_field(meta.clonenum_for_pktloss, 1);

	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss); // clone to switchos
}

action forward_getres_deleted_seq_inswitch_case1() {
}

//action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss(sid, port) {
action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss(sid, reflector_port) {
	modify_field(op_hdr.optype, CACHE_POP_INSWITCH_ACK);
	modify_field(udp_hdr.dstPort, reflector_port);
	//modify_field(meta.clonenum_for_pktloss, 1); // 3 ACKs (clone w/ 1 -> clone w/ 0 -> no clone)
	modify_field(meta.clonenum_for_pktloss, 2); // 3 ACKs (drop w/ 2 -> clone w/ 1 -> clone w/ 0 -> no clone)

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
	remove_header(shadowtype_hdr);
	remove_header(seq_hdr);
	remove_header(inswitch_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	//modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring // TODO: see if we can get cloned packet
	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss); // clone to switchos
}

action forward_cache_pop_inswitch_ack_clone_for_pktloss(sid) {
	subtract_from_field(meta.clonenum_for_pktloss, 1);

	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss); // clone to switchos
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

action update_putreq_inswitch_to_putres_by_mirroring(sid, server_port) {
	modify_field(op_hdr.optype, PUTRES);
	modify_field(shadowtype_hdr.shadowtype, PUTRES);
	modify_field(stat_hdr.stat, 1);
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, meta.client_port);

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

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(sid); // clone to client (inswitch_hdr.sid)
}

field_list clone_field_list_for_pktloss_and_res {
	meta.clonenum_for_pktloss;
	meta.client_port;
	// NOTE: extracted fields cannot be used as clone fields
	//inswitch_hdr.is_wrong_pipeline;
	//inswitch_hdr.eport_for_res;
	//inswitch_hdr.sid;
}

//action update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(sid, port, stat) {
action update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(sid, stat, reflector_port) {
	modify_field(op_hdr.optype, PUTREQ_SEQ_INSWITCH_CASE1);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ_INSWITCH_CASE1);
	modify_field(stat_hdr.stat, stat);
	modify_field(udp_hdr.dstPort, reflector_port);
	//modify_field(meta.clonenum_for_pktloss, 2); // 3 ACKs (clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> PUTRES)
	modify_field(meta.clonenum_for_pktloss, 3); // 3 ACKs (drop w/ 3 -> clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> PUTRES)

	//remove_header(inswitch_hdr);
	add_header(seq_hdr);
	add_header(stat_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss_and_res); // clone to switchos
}

action forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(sid) {
	subtract_from_field(meta.clonenum_for_pktloss, 1);

	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss_and_res); // clone to switchos
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

action update_putreq_seq_inswitch_case1_to_putres_by_mirroring(sid, server_port) {
	modify_field(op_hdr.optype, PUTRES);
	modify_field(shadowtype_hdr.shadowtype, PUTRES);
	modify_field(stat_hdr.stat, 1);
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, meta.client_port);

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

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(sid); // clone to client (inswitch_hdr.sid)
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

action update_delreq_inswitch_to_delres_by_mirroring(sid, server_port) {
	modify_field(op_hdr.optype, DELRES);
	modify_field(shadowtype_hdr.shadowtype, DELRES);
	modify_field(stat_hdr.stat, 1);
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, meta.client_port);

	remove_header(inswitch_hdr);
	add_header(stat_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(sid); // clone to client (inswitch_hdr.sid)
}

//action update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(sid, port, stat) {
action update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(sid, stat, reflector_port) {
	modify_field(op_hdr.optype, DELREQ_SEQ_INSWITCH_CASE1);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ_INSWITCH_CASE1);
	modify_field(stat_hdr.stat, stat);
	modify_field(udp_hdr.dstPort, reflector_port);
	//modify_field(meta.clonenum_for_pktloss, 2); // 3 ACKs (clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> DELRES)
	modify_field(meta.clonenum_for_pktloss, 3); // 3 ACKs (drop w/ 3 -> clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> DELRES)

	//remove_header(inswitch_hdr);
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
	add_header(seq_hdr);
	add_header(stat_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss_and_res); // clone to switchps
}

action forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(sid) {
	subtract_from_field(meta.clonenum_for_pktloss, 1);

	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss_and_res); // clone to switchos
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

action update_delreq_seq_inswitch_case1_to_delres_by_mirroring(sid, server_port) {
	modify_field(op_hdr.optype, DELRES);
	modify_field(shadowtype_hdr.shadowtype, DELRES);
	modify_field(stat_hdr.stat, 1);
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, meta.client_port);

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

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(sid); // clone to client (inswitch_hdr.sid)
}

action update_delreq_inswitch_to_delreq_seq_case3() {
	modify_field(op_hdr.optype, DELREQ_SEQ_CASE3);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ_CASE3);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

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
		debug_hdr.is_hot: exact;
		meta.validvalue: exact;
		meta.is_latest: exact;
		meta.is_deleted: exact;
		//inswitch_hdr.is_wrong_pipeline: exact;
		inswitch_hdr.sid: exact;
		debug_hdr.is_lastclone_for_pktloss: exact;
		inswitch_hdr.snapshot_flag: exact;
		meta.is_case1: exact;
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
		drop_getres_latest_seq_inswitch; // drop original packet of GETRES_LATEST_SEQ
		forward_getres_latest_seq_inswitch_case1_clone_for_pktloss; // not last clone of GETRES_LATEST_SEQ_INSWITCH_CASE1
		forward_getres_latest_seq_inswitch_case1; // last clone of GETRES_LATEST_SEQ_INSWITCH_CASE1
		update_getres_deleted_seq_to_getres; // GETRES_DELETED_SEQ must be cloned from ingress to egress
		update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss; // drop original packet of GETRES_DELETED_SEQ -> clone for first GETRES_DELETED_SEQ_INSWITCH_CASE1
		drop_getres_deleted_seq_inswitch; // original packet of GETRES_DELETED_SEQ
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
		nop;
	}
	default_action: nop();
#ifdef DEBUG
	size: 8192; // assign more entries for debugging
#else
	size: 2048;
#endif
}

#ifdef RANGE_SUPPORT
action forward_scanreq_split_and_clone(sid) {
	add_to_field(split_hdr.cur_scanidx, 1);
	// NOTE: eg_intr_md.egress_port has been set by process_(cloned)_scanreq_split_tbl in stage 0
	clone_egress_pkt_to_egress(sid); // clone to client (inswitch_hdr.sid)
}
action forward_scanreq_split() {
	add_to_field(split_hdr.cur_scanidx, 1);
	// NOTE: eg_intr_md.egress_port has been set by process_(cloned)_scanreq_split_tbl in stage 0
}
@pragma stage 10
table scan_forward_tbl {
	reads {
		op_hdr.optype: exact;
		meta.is_last_scansplit: exact;
		inswitch_hdr.sid: exact;
	}
	actions {
		forward_scanreq_split_and_clone;
		forward_scanreq_split;
	}
	default_action: forward_scanreq_split_and_clone();
	size: 2;
}
#endif


// NOTE: only one operand in add can be action parameter or constant -> resort to controller to configure different hdrlen
/*// CACHE_POP_INSWITCH_ACK
action update_onlyop_udplen() {
	// 6(udphdr) + 17(ophdr) + 1(debug_hdr)
	modify_field(udp_hdr.hdrlen, 24);
}

// GETRES
action update_val_stat_udplen(aligned_vallen) {
	// 6(udphdr) + 17(ophdr) + 2(vallen) + aligned_vallen(val) + 1(shadowtype) + 1(stat) + 1(debug_hdr)
	add(udp_hdr.hdrlen, aligned_vallen, 28);
}

// GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1
action update_val_seq_inswitch_stat_udplen(aligned_vallen) {
	// 6(udphdr) + 17(ophdr) + 2(vallen) + aligned_vallen(val) + 1(shadowtype) + 4(seq) + 9(inswitch) + 1(stat) + 1(debug_hdr)
	add(udp_hdr.hdrlen, aligned_vallen, 41);
}

// PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3
action update_val_seq_udplen(aligned_vallen) {
	// 6(udphdr) + 17(ophdr) + 2(vallen) + aligned_vallen(val) + 1(shadowtype) + 4(seq) + 1(debug_hdr)
	add(udp_hdr.hdrlen, aligned_vallen, 31);
}

// PUTRES, DELRES
action update_stat_udplen() {
	// 6(udphdr) + 17(ophdr) + 1(shadowtype) + 1(stat) + 1(debug_hdr)
	modify_field(udp_hdr.hdrlen, 26);
}

// DELREQ_SEQ, DELREQ_SEQ_CASE3
action update_seq_udplen() {
	// 6(udphdr) + 17(ophdr) + 1(shadowtype) + 4(seq) + 1(debug_hdr)
	modify_field(udp_hdr.hdrlen, 29);
}*/

action update_udplen(udplen) {
	modify_field(udp_hdr.hdrlen, udplen);
}

@pragma stage 11
table update_udplen_tbl {
	reads {
		op_hdr.optype: exact;
		vallen_hdr.vallen: range;
	}
	actions {
		/*update_onlyop_udplen;
		update_val_stat_udplen;
		update_val_seq_inswitch_stat_udplen;
		update_val_seq_udplen;
		update_stat_udplen;
		update_seq_udplen;*/
		update_udplen;
		nop;
	}
	default_action: nop(); // not change udp_hdr.hdrlen (GETREQ/GETREQ_POP/GETREQ_NLATEST)
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
