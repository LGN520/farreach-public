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
	modify_field(meta.is_hot, 1);
}

action reset_is_hot() {
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
		set_is_hot;
		reset_is_hot;
	}
	default_action: reset_is_hot();
	size: 1;
}

// Stage 9

action set_is_lastclone() {
	modify_field(meta.is_lastclone_for_pktloss, 1);
}

action reset_is_lastclone() {
	modify_field(meta.is_lastclone_for_pktloss, 0);
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

	remove_header(inswitch_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_getreq_inswitch_to_getreq_pop() {
	modify_field(op_hdr.optype, GETREQ_POP);

	remove_header(inswitch_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_getreq_inswitch_to_getreq_nlatest() {
	modify_field(op_hdr.optype, GETREQ_NLATEST);

	remove_header(inswitch_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

/*action update_getreq_inswitch_to_getres_for_deleted() {
	modify_field(op_hdr.optype, GETRES);
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

action update_getreq_inswitch_to_getres_for_deleted_by_mirroring() {
	modify_field(op_hdr.optype, GETRES);
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

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(inswitch_hdr.sid); // clone for egress switching
}

/*action update_getreq_inswitch_to_getres() {
	modify_field(op_hdr.optype, GETRES);
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

action update_getreq_inswitch_to_getres_by_mirroring() {
	modify_field(op_hdr.optype, GETRES);
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

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(inswitch_hdr.sid); // clone for egress switching
}

action update_getres_latest_seq_to_getres() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(stat_hdr.stat, 1);

	remove_header(seq_hdr);
	add_header(stat_hdr);
}

field_list clone_field_list_for_pktloss {
	meta.clonenum_for_pktloss;
}

//action update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss(sid, port, stat) {
action update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss(sid, stat) {
	modify_field(op_hdr.optype, GETRES_LATEST_SEQ_INSWITCH_CASE1);
	modify_field(stat_hdr.stat, stat);
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

	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss);
}

action forward_getres_latest_seq_inswitch_case1() {
}

action update_getres_deleted_seq_to_getres() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(stat_hdr.stat, 0);

	remove_header(seq_hdr);
	add_header(stat_hdr);
}

//action update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss(sid, port, stat) {
action update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss(sid, stat) {
	modify_field(op_hdr.optype, GETRES_DELETED_SEQ_INSWITCH_CASE1);
	modify_field(stat_hdr.stat, stat);
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

	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss);
}

action forward_getres_deleted_seq_inswitch_case1() {
}

//action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss(sid, port) {
action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss(sid) {
	modify_field(op_hdr.optype, CACHE_POP_INSWITCH_ACK);
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
	remove_header(seq_hdr);
	remove_header(inswitch_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss); // clone to switchos
}

action forward_cache_pop_inswitch_ack_clone_for_pktloss(sid) {
	subtract_from_field(meta.clonenum_for_pktloss, 1);

	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss);
}

action forward_cache_pop_inswitch_ack() {
}

action update_putreq_inswitch_to_putreq_seq() {
	modify_field(op_hdr.optype, PUTREQ_SEQ);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_putreq_inswitch_to_putreq_pop_seq() {
	modify_field(op_hdr.optype, PUTREQ_POP_SEQ);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

/*action update_putreq_inswitch_to_putres() {
	modify_field(op_hdr.optype, PUTRES);
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

action update_putreq_inswitch_to_putres_by_mirroring() {
	modify_field(op_hdr.optype, PUTRES);
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

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(inswitch_hdr.sid); // clone for egress switching
}

field_list clone_field_list_for_pktloss_and_res {
	meta.clonenum_for_pktloss;
	// NOTE: extracted fields cannot be used as clone fields
	//inswitch_hdr.is_wrong_pipeline;
	//inswitch_hdr.eport_for_res;
	//inswitch_hdr.sid;
}

//action update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(sid, port, stat) {
action update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(sid, stat) {
	modify_field(op_hdr.optype, PUTREQ_SEQ_INSWITCH_CASE1);
	modify_field(stat_hdr.stat, stat);
	//modify_field(meta.clonenum_for_pktloss, 2); // 3 ACKs (clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> PUTRES)
	modify_field(meta.clonenum_for_pktloss, 3); // 3 ACKs (drop w/ 3 -> clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> PUTRES)

	//remove_header(inswitch_hdr);
	add_header(seq_hdr);
	add_header(stat_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss_and_res);
}

action forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(sid) {
	subtract_from_field(meta.clonenum_for_pktloss, 1);

	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss_and_res);
}

/*action update_putreq_seq_inswitch_case1_to_putres() {
	modify_field(op_hdr.optype, PUTRES);
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

action update_putreq_seq_inswitch_case1_to_putres_by_mirroring() {
	modify_field(op_hdr.optype, PUTRES);
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

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(inswitch_hdr.sid); // clone for egress switching
}

action update_putreq_inswitch_to_putreq_seq_case3() {
	modify_field(op_hdr.optype, PUTREQ_SEQ_CASE3);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_putreq_inswitch_to_putreq_pop_seq_case3() {
	modify_field(op_hdr.optype, PUTREQ_POP_SEQ_CASE3);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_delreq_inswitch_to_delreq_seq() {
	modify_field(op_hdr.optype, DELREQ_SEQ);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

/*action update_delreq_inswitch_to_delres() {
	modify_field(op_hdr.optype, DELRES);
	modify_field(stat_hdr.stat, 1);

	remove_header(inswitch_hdr);
	add_header(stat_hdr);

	modify_field(eg_intr_md.egress_port, inswitch_hdr.eport_for_res);
}*/

action update_delreq_inswitch_to_delres_by_mirroring() {
	modify_field(op_hdr.optype, DELRES);
	modify_field(stat_hdr.stat, 1);

	remove_header(inswitch_hdr);
	add_header(stat_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(inswitch_hdr.sid); // clone for egress switching
}

//action update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(sid, port, stat) {
action update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(sid, stat) {
	modify_field(op_hdr.optype, DELREQ_SEQ_INSWITCH_CASE1);
	modify_field(stat_hdr.stat, stat);
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
	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss_and_res);
}

action forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(sid) {
	subtract_from_field(meta.clonenum_for_pktloss, 1);

	clone_egress_pkt_to_egress(sid, clone_field_list_for_pktloss_and_res);
}

/*action update_delreq_seq_inswitch_case1_to_delres() {
	modify_field(op_hdr.optype, DELRES);
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

action update_delreq_seq_inswitch_case1_to_delres_by_mirroring() {
	modify_field(op_hdr.optype, DELRES);
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

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(inswitch_hdr.sid); // clone for egress switching
}

action update_delreq_inswitch_to_delreq_seq_case3() {
	modify_field(op_hdr.optype, DELREQ_SEQ_CASE3);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

table eg_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
		meta.is_hot: exact;
		meta.validvalue: exact;
		meta.is_latest: exact;
		meta.is_deleted: exact;
		//inswitch_hdr.is_wrong_pipeline: exact;
		meta.is_lastclone_for_pktloss: exact;
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
	size: 2048;
}

#ifdef RANGE_SUPPORT
action forward_scanreq_split_and_clone() {
	add_to_field(split_hdr.cur_scanidx, 1);
	// NOTE: eg_intr_md.egress_port has been set by process_(cloned)_scanreq_split_tbl in stage 0
	clone_egress_pkt_to_egress(inswitch_hdr.sid);
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
	}
	actions {
		forward_scanreq_split_and_clone;
		forward_scanreq_split;
	}
	default_action: forward_scanreq_split_and_clone();
	size: 1;
}
#endif






/*action update_getres_udplen(aligned_vallen) {
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
}*/
