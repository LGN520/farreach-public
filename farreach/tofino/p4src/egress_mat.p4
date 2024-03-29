/* Ingress Processing (Normal Operation) */

// Stage 0

/*@pragma stage 0
table eg_copy_udplen_for_checksum_tbl {
	actions {
		copy_udplen_for_checksum;
	}
	default_action: copy_udplen_for_checksum();
	size: 1;
}*/

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
action process_scanreq_split(server_sid) {
	modify_field(meta.server_sid, server_sid); // clone to server for next SCANREQ_SPLIT
	subtract(meta.remain_scannum, split_hdr.max_scannum, split_hdr.cur_scanidx);
	modify_field(clone_hdr.clonenum_for_pktloss, 0);
}
action process_cloned_scanreq_split(udpport, server_sid) {
	//add_to_field(udp_hdr.dstPort, 1);
	modify_field(udp_hdr.dstPort, udpport); // set udpport for current SCANREQ_SPLIT
	modify_field(meta.server_sid, server_sid); // clone to server for next SCANREQ_SPLIT
	subtract(meta.remain_scannum, split_hdr.max_scannum, split_hdr.cur_scanidx);
	modify_field(clone_hdr.clonenum_for_pktloss, 0);
}
action reset_meta_serversid_remainscannum() {
	modify_field(meta.server_sid, 0);
	modify_field(meta.remain_scannum, 0);
}
@pragma stage 1
table process_scanreq_split_tbl {
	reads {
		op_hdr.optype: exact;
		//udp_hdr.dstPort: exact;
		split_hdr.globalserveridx: exact;
		//eg_intr_md_from_parser_aux.clone_src: exact; // NOTE: access intrinsic metadata
		split_hdr.is_clone: exact;
	}
	actions {
		process_scanreq_split;
		process_cloned_scanreq_split;
		reset_meta_serversid_remainscannum;
	}
	default_action: reset_meta_serversid_remainscannum();
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

// Stage 3

action set_is_largevalueblock() {
	modify_field(meta.is_largevalueblock, 1);
}

@pragma stage 3
table is_largevalueblock_tbl {
	actions {
		set_is_largevalueblock;
	}
	default_action: set_is_largevalueblock();
	size: 1;
}

// Stage 7

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

@pragma stage 7
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

// Stage 8

action update_getreq_inswitch_to_getreq() {
	modify_field(op_hdr.optype, GETREQ);

	remove_header(shadowtype_hdr);
	remove_header(inswitch_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

//action update_getreq_inswitch_to_getreq_beingevicted() {
//	modify_field(op_hdr.optype, GETREQ_BEINGEVICTED);
//
//	remove_header(shadowtype_hdr);
//	remove_header(inswitch_hdr);
//}

////#ifdef ENABLE_LARGEVALUEBLOCK
//action update_getreq_inswitch_to_getreq_largevalueblock_seq() {
//	modify_field(op_hdr.optype, GETREQ_LARGEVALUEBLOCK_SEQ);
//	modify_field(shadowtype_hdr.shadowtype, GETREQ_LARGEVALUEBLOCK_SEQ);
//
//	modify_field(seq_hdr.seq, meta.largevalueseq);
//
//	//remove_header(shadowtype_hdr);
//	add_header(seq_hdr);
//
//	remove_header(inswitch_hdr);
//}
////#endif

action update_getreq_inswitch_to_getreq_beingevicted_record(stat) {
	modify_field(op_hdr.optype, GETREQ_BEINGEVICTED_RECORD);
	modify_field(shadowtype_hdr.shadowtype, GETREQ_BEINGEVICTED_RECORD);
	modify_field(stat_hdr.stat, stat);

	//remove_header(shadowtype_hdr);
	add_header(seq_hdr);
	remove_header(inswitch_hdr);
	add_header(stat_hdr);
}

//#ifdef ENABLE_LARGEVALUEBLOCK
action update_getreq_inswitch_to_getreq_largevalueblock_record(stat) {
	modify_field(op_hdr.optype, GETREQ_LARGEVALUEBLOCK_RECORD);
	modify_field(shadowtype_hdr.shadowtype, GETREQ_LARGEVALUEBLOCK_RECORD);
	modify_field(stat_hdr.stat, stat);

	//remove_header(shadowtype_hdr);
	add_header(seq_hdr);
	remove_header(inswitch_hdr);
	add_header(stat_hdr);
}
//#endif

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

action update_getreq_inswitch_to_getres_seq_by_mirroring(client_sid, server_port, stat) {
	modify_field(op_hdr.optype, GETRES_SEQ);
	modify_field(shadowtype_hdr.shadowtype, GETRES_SEQ);
	modify_field(stat_hdr.stat, stat);
	modify_field(stat_hdr.nodeidx_foreval, SWITCHIDX_FOREVAL);
	// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, clone_hdr.client_udpport);

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
	remove_header(inswitch_hdr);
	add_header(stat_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(client_sid); // clone to client (inswitch_hdr.client_sid)
}

action update_getres_latest_seq_to_getres_seq() {
	modify_field(op_hdr.optype, GETRES_SEQ);
	modify_field(shadowtype_hdr.shadowtype, GETRES_SEQ);
	modify_field(stat_hdr.stat, 1);

	//remove_header(seq_hdr);
	add_header(seq_hdr);
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

//action forward_getres_latest_seq_inswitch_case1() {
//}

action update_getres_deleted_seq_to_getres_seq() {
	modify_field(op_hdr.optype, GETRES_SEQ);
	modify_field(shadowtype_hdr.shadowtype, GETRES_SEQ);
	modify_field(stat_hdr.stat, 0);

	//remove_header(seq_hdr);
	add_header(seq_hdr);
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

//action forward_getres_deleted_seq_inswitch_case1() {
//}

action update_putreq_largevalue_inswitch_to_putreq_largevalue_seq() {
	// NOTE: PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr
	modify_field(op_hdr.optype, PUTREQ_LARGEVALUE_SEQ);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_LARGEVALUE_SEQ);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);
}

action update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_case3() {
	// NOTE: PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ_CASE3 w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr
	modify_field(op_hdr.optype, PUTREQ_LARGEVALUE_SEQ_CASE3);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_LARGEVALUE_SEQ_CASE3);
	modify_field(seq_hdr.snapshot_token, inswitch_hdr.snapshot_token);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);
}

action update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_beingevicted() {
	// NOTE: PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr
	modify_field(op_hdr.optype, PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);
}

action update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_case3_beingevicted() {
	// NOTE: PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr
	modify_field(op_hdr.optype, PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED);
	modify_field(seq_hdr.snapshot_token, inswitch_hdr.snapshot_token);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter another_eg_port_forward_counter {
	type : packets_and_bytes;
	direct: another_eg_port_forward_tbl;
}
#endif

@pragma stage 8
table another_eg_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
		meta.is_hot: exact;
		validvalue_hdr.validvalue: exact;
		meta.is_latest: exact;
//#ifdef ENABLE_LARGEVALUEBLOCK
		meta.is_largevalueblock: exact;
//#endif
		meta.is_deleted: exact;
		inswitch_hdr.client_sid: exact;
		meta.is_lastclone_for_pktloss: exact;
		inswitch_hdr.snapshot_flag: exact;
		meta.is_case1: exact;
	}
	actions {
		update_getreq_inswitch_to_getreq;
//		update_getreq_inswitch_to_getreq_beingevicted;
////#ifdef ENABLE_LARGEVALUEBLOCK
//		update_getreq_inswitch_to_getreq_largevalueblock_seq;
////#endif
		update_getreq_inswitch_to_getreq_beingevicted_record;
//#ifdef ENABLE_LARGEVALUEBLOCK
		update_getreq_inswitch_to_getreq_largevalueblock_record;
//#endif
		update_getreq_inswitch_to_getreq_pop;
		update_getreq_inswitch_to_getreq_nlatest;
		update_getreq_inswitch_to_getres_seq_by_mirroring;
		update_getres_latest_seq_to_getres_seq; // GETRES_LATEST_SEQ must be cloned from ingress to egress
		update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss; // drop original packet of GETRES_LATEST_SEQ -> clone for first GETRES_LATEST_SEQ_INSWITCH_CASE1
		//drop_getres_latest_seq_inswitch; // drop original packet of GETRES_LATEST_SEQ
		forward_getres_latest_seq_inswitch_case1_clone_for_pktloss; // not last clone of GETRES_LATEST_SEQ_INSWITCH_CASE1
		//forward_getres_latest_seq_inswitch_case1; // last clone of GETRES_LATEST_SEQ_INSWITCH_CASE1
		update_getres_deleted_seq_to_getres_seq; // GETRES_DELETED_SEQ must be cloned from ingress to egress
		update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss; // drop original packet of GETRES_DELETED_SEQ -> clone for first GETRES_DELETED_SEQ_INSWITCH_CASE1
		//drop_getres_deleted_seq_inswitch; // original packet of GETRES_DELETED_SEQ
		forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss; // not last clone of GETRES_DELETED_SEQ_INSWITCH_CASE1
		//forward_getres_deleted_seq_inswitch_case1; // last clone of GETRES_DELETED_SEQ_INSWITCH_CASE1
		update_putreq_largevalue_inswitch_to_putreq_largevalue_seq;
		update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_case3;
		update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_beingevicted;
		update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_case3_beingevicted;
		nop;
	}
	default_action: nop();
//#ifdef ENABLE_LARGEVALUEBLOCK
	size: 512;
//#else
//	size: 256;
//#endif
}

// Stage 9

/*action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss(switchos_sid, reflector_port) {
	modify_field(op_hdr.optype, CACHE_POP_INSWITCH_ACK);
	modify_field(udp_hdr.dstPort, reflector_port);
	//modify_field(meta.clonenum_for_pktloss, 1); // 3 ACKs (clone w/ 1 -> clone w/ 0 -> no clone w/ ack)
	modify_field(clone_hdr.clonenum_for_pktloss, 2); // 3 ACKs (drop w/ 2 -> clone w/ 1 -> clone w/ 0 -> no clone w/ ack)

	//remove_header(vallen_hdr);
	//remove_header(val1_hdr);
	//remove_header(val2_hdr);
	//remove_header(val3_hdr);
	//remove_header(val4_hdr);
	//remove_header(val5_hdr);
	//remove_header(val6_hdr);
	//remove_header(val7_hdr);
	//remove_header(val8_hdr);
	//remove_header(val9_hdr);
	//remove_header(val10_hdr);
	//remove_header(val11_hdr);
	//remove_header(val12_hdr);
	//remove_header(val13_hdr);
	//remove_header(val14_hdr);
	//remove_header(val15_hdr);
	//remove_header(val16_hdr);
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
}*/

action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone(switchos_sid, reflector_port) {
	modify_field(op_hdr.optype, CACHE_POP_INSWITCH_ACK);
	modify_field(udp_hdr.dstPort, reflector_port);

	// NOTE: we add/remove vallen and value headers in add_remove_value_header_tbl
	remove_header(shadowtype_hdr);
	remove_header(seq_hdr);
	remove_header(inswitch_hdr);
	remove_header(stat_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

//action forward_cache_pop_inswitch_ack() {
//}

action update_putreq_inswitch_to_putreq_seq() {
	modify_field(op_hdr.optype, PUTREQ_SEQ);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_putreq_inswitch_to_putreq_seq_beingevicted() {
	modify_field(op_hdr.optype, PUTREQ_SEQ_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ_BEINGEVICTED);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);
}

action update_putreq_inswitch_to_putreq_pop_seq() {
	modify_field(op_hdr.optype, PUTREQ_POP_SEQ);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_POP_SEQ);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_putreq_inswitch_to_putres_seq_by_mirroring(client_sid, server_port) {
	modify_field(op_hdr.optype, PUTRES_SEQ);
	modify_field(shadowtype_hdr.shadowtype, PUTRES_SEQ);
	modify_field(seq_hdr.seq, clone_hdr.assignedseq_for_farreach);
	modify_field(stat_hdr.stat, 1);
	modify_field(stat_hdr.nodeidx_foreval, SWITCHIDX_FOREVAL);
	// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
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

	remove_header(inswitch_hdr);
	add_header(seq_hdr);
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

action update_putreq_seq_inswitch_case1_to_putres_seq_by_mirroring(client_sid, server_port) {
	modify_field(op_hdr.optype, PUTRES_SEQ);
	modify_field(shadowtype_hdr.shadowtype, PUTRES_SEQ);
	modify_field(seq_hdr.seq, clone_hdr.assignedseq_for_farreach);
	modify_field(stat_hdr.stat, 1);
	modify_field(stat_hdr.nodeidx_foreval, SWITCHIDX_FOREVAL);
	// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
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

	//remove_header(seq_hdr);
	add_header(seq_hdr);

	remove_header(inswitch_hdr);
	remove_header(clone_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(client_sid); // clone to client (inswitch_hdr.client_sid)
}

action update_putreq_inswitch_to_putreq_seq_case3() {
	modify_field(op_hdr.optype, PUTREQ_SEQ_CASE3);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ_CASE3);
	modify_field(seq_hdr.snapshot_token, inswitch_hdr.snapshot_token);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_putreq_inswitch_to_putreq_seq_case3_beingevicted() {
	modify_field(op_hdr.optype, PUTREQ_SEQ_CASE3_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ_CASE3_BEINGEVICTED);
	modify_field(seq_hdr.snapshot_token, inswitch_hdr.snapshot_token);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);
}

action update_putreq_inswitch_to_putreq_pop_seq_case3() {
	modify_field(op_hdr.optype, PUTREQ_POP_SEQ_CASE3);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_POP_SEQ_CASE3);
	modify_field(seq_hdr.snapshot_token, inswitch_hdr.snapshot_token);

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

action update_delreq_inswitch_to_delreq_seq_beingevicted() {
	modify_field(op_hdr.optype, DELREQ_SEQ_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ_BEINGEVICTED);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);
}

action update_delreq_inswitch_to_delres_seq_by_mirroring(client_sid, server_port) {
	modify_field(op_hdr.optype, DELRES_SEQ);
	modify_field(shadowtype_hdr.shadowtype, DELRES_SEQ);
	modify_field(seq_hdr.seq, clone_hdr.assignedseq_for_farreach);
	modify_field(stat_hdr.stat, 1);
	modify_field(stat_hdr.nodeidx_foreval, SWITCHIDX_FOREVAL);
	// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, clone_hdr.client_udpport);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);
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

action update_delreq_seq_inswitch_case1_to_delres_seq_by_mirroring(client_sid, server_port) {
	modify_field(op_hdr.optype, DELRES_SEQ);
	modify_field(shadowtype_hdr.shadowtype, DELRES_SEQ);
	modify_field(seq_hdr.seq, clone_hdr.assignedseq_for_farreach);
	modify_field(stat_hdr.stat, 1);
	modify_field(stat_hdr.nodeidx_foreval, SWITCHIDX_FOREVAL);
	// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
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

	//remove_header(seq_hdr);
	add_header(seq_hdr);

	remove_header(inswitch_hdr);
	remove_header(clone_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(client_sid); // clone to client (inswitch_hdr.client_sid)
}

action update_delreq_inswitch_to_delreq_seq_case3() {
	modify_field(op_hdr.optype, DELREQ_SEQ_CASE3);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ_CASE3);
	modify_field(seq_hdr.snapshot_token, inswitch_hdr.snapshot_token);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_delreq_inswitch_to_delreq_seq_case3_beingevicted() {
	modify_field(op_hdr.optype, DELREQ_SEQ_CASE3_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ_CASE3_BEINGEVICTED);
	modify_field(seq_hdr.snapshot_token, inswitch_hdr.snapshot_token);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);
}

#ifdef RANGE_SUPPORT
action forward_scanreq_split_and_clone(server_sid) {
	modify_field(split_hdr.is_clone, 1);
	add_to_field(split_hdr.cur_scanidx, 1);
	add_to_field(split_hdr.globalserveridx, 1);
	// NOTE: eg_intr_md.egress_port has been set by process_(cloned)_scanreq_split_tbl in stage 0
	clone_egress_pkt_to_egress(server_sid); // clone to server (meta.server_sid)
}
action forward_scanreq_split() {
	modify_field(split_hdr.is_clone, 1);
	add_to_field(split_hdr.cur_scanidx, 1);
	add_to_field(split_hdr.globalserveridx, 1);
	// NOTE: eg_intr_md.egress_port has been set by process_(cloned)_scanreq_split_tbl in stage 0
}
#endif

action update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone(switchos_sid, reflector_port) {
	modify_field(op_hdr.optype, CACHE_EVICT_LOADFREQ_INSWITCH_ACK);
	modify_field(udp_hdr.dstPort, reflector_port);

	remove_header(shadowtype_hdr);
	remove_header(inswitch_hdr);
	add_header(frequency_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

//action forward_cache_evict_loadfreq_inswitch_ack() {
//}

action update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone(switchos_sid, reflector_port, stat) {
	modify_field(op_hdr.optype, CACHE_EVICT_LOADDATA_INSWITCH_ACK);
	modify_field(udp_hdr.dstPort, reflector_port);
	modify_field(shadowtype_hdr.shadowtype, CACHE_EVICT_LOADDATA_INSWITCH_ACK);
	modify_field(stat_hdr.stat, stat);

	remove_header(inswitch_hdr);
	// NOTE: we add/remove vallen and value headers in add_remove_value_header_tbl
	add_header(seq_hdr);
	add_header(stat_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

//action forward_cache_evict_loaddata_inswitch_ack() {
//}

action update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone(switchos_sid, reflector_port, stat) {
	modify_field(op_hdr.optype, LOADSNAPSHOTDATA_INSWITCH_ACK);
	modify_field(udp_hdr.dstPort, reflector_port);
	modify_field(shadowtype_hdr.shadowtype, LOADSNAPSHOTDATA_INSWITCH_ACK);
	modify_field(stat_hdr.stat, stat);

	// NOTE: we add/remove vallen and value headers in add_remove_value_header_tbl
	add_header(seq_hdr);
	add_header(stat_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

//action forward_loadsnapshotdata_inswitch_ack() {
//}

action update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone(switchos_sid, reflector_port) {
	modify_field(op_hdr.optype, SETVALID_INSWITCH_ACK);
	modify_field(udp_hdr.dstPort, reflector_port);

	remove_header(shadowtype_hdr);
	remove_header(inswitch_hdr);
	remove_header(validvalue_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

//action forward_setvalid_inswitch_ack() {
//}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter eg_port_forward_counter {
	type : packets_and_bytes;
	direct: eg_port_forward_tbl;
}
#endif

@pragma stage 9
table eg_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
		meta.is_hot: exact;
		//debug_hdr.is_hot: exact;
		validvalue_hdr.validvalue: exact;
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
		//update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss; // clone for first CACHE_POP_INSWITCH_ACK
		//forward_cache_pop_inswitch_ack_clone_for_pktloss; // not last clone of CACHE_POP_INSWITCH_ACK
		update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone; // clone for first CACHE_POP_INSWITCH_ACK (not need to clone for duplication due to switchos-side timeout-and-retry)
		//forward_cache_pop_inswitch_ack; // last clone of CACHE_POP_INSWITCH_ACK
		update_putreq_inswitch_to_putreq_seq;
		update_putreq_inswitch_to_putreq_seq_beingevicted;
		update_putreq_inswitch_to_putreq_pop_seq;
		update_putreq_inswitch_to_putres_seq_by_mirroring;
		update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres;
		forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres;
		update_putreq_seq_inswitch_case1_to_putres_seq_by_mirroring;
		update_putreq_inswitch_to_putreq_seq_case3;
		update_putreq_inswitch_to_putreq_seq_case3_beingevicted;
		update_putreq_inswitch_to_putreq_pop_seq_case3;
		update_delreq_inswitch_to_delreq_seq;
		update_delreq_inswitch_to_delreq_seq_beingevicted;
		update_delreq_inswitch_to_delres_seq_by_mirroring;
		update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres;
		forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres;
		update_delreq_seq_inswitch_case1_to_delres_seq_by_mirroring;
		update_delreq_inswitch_to_delreq_seq_case3;
		update_delreq_inswitch_to_delreq_seq_case3_beingevicted;
#ifdef RANGE_SUPPORT
		forward_scanreq_split_and_clone;
		forward_scanreq_split;
#endif
		update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone; // clone to reflector and hence switchos; but not need clone for pktloss due to switchos-side timeout-and-retry
		//forward_cache_evict_loadfreq_inswitch_ack;
		update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone; // clone to reflector and hence switchos; but not need clone for pktloss due to switchos-side timeout-and-retry
		//forward_cache_evict_loaddata_inswitch_ack;
		update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone;
		//forward_loadsnapshotdata_inswitch_ack;
		update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone;
		//forward_setvalid_inswitch_ack;
		nop;
	}
	default_action: nop();
	size: 2048;
}

// stage 10

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter update_ipmac_srcport_counter {
	type : packets_and_bytes;
	direct: update_ipmac_srcport_tbl;
}
#endif

action update_ipmac_srcport_server2client(client_mac, server_mac, client_ip, server_ip, server_port) {
	modify_field(ethernet_hdr.srcAddr, server_mac);
	modify_field(ethernet_hdr.dstAddr, client_mac);
	modify_field(ipv4_hdr.srcAddr, server_ip);
	modify_field(ipv4_hdr.dstAddr, client_ip);
	modify_field(udp_hdr.srcPort, server_port);
}

// NOTE: as we use software link, switch_mac/ip = reflector_mac/ip
// NOTE: although we use client_port to update srcport here, reflector does not care about the specific value of srcport
action update_ipmac_srcport_switch2switchos(client_mac, switch_mac, client_ip, switch_ip, client_port) {
	modify_field(ethernet_hdr.srcAddr, client_mac);
	modify_field(ethernet_hdr.dstAddr, switch_mac);
	modify_field(ipv4_hdr.srcAddr, client_ip);
	modify_field(ipv4_hdr.dstAddr, switch_ip);
	modify_field(udp_hdr.srcPort, client_port);
}

action update_dstipmac_client2server(server_mac, server_ip) {
	modify_field(ethernet_hdr.dstAddr, server_mac);
	modify_field(ipv4_hdr.dstAddr, server_ip);
}

// NOTE: dstport of REQ, RES, and notification has been updated in partition_tbl, server, and eg_port_forward_tbl
@pragma stage 10
table update_ipmac_srcport_tbl {
	reads {
		op_hdr.optype: exact;
		eg_intr_md.egress_port: exact;
	}
	actions {
		update_ipmac_srcport_server2client; // focus on dstip and dstmac to corresponding client; use server[0] as srcip and srcmac; use server_worker_port_start as srcport
		update_ipmac_srcport_switch2switchos; // focus on dstip and dstmac to reflector; use client[0] as srcip and srcmac; use constant (123) as srcport
		update_dstipmac_client2server; // focus on dstip and dstmac to corresponding server; NOT change srcip, srcmac, and srcport
		nop;
	}
	default_action: nop();
	size: 256;
}

// stage 11

// NOTE: only one operand in add can be action parameter or constant -> resort to controller to configure different hdrlen
/*
// CACHE_POP_INSWITCH_ACK, SETVALID_INSWITCH_ACK
action update_onlyop_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 18(ophdr) + 1(debug_hdr)
	//modify_field(udp_hdr.hdrlen, 27);
	//modify_field(ipv4_hdr.totalLen, 47);
	modify_field(udp_hdr.hdrlen, 26);
	modify_field(ipv4_hdr.totalLen, 46);
}

// GETRES
action update_val_stat_pktlen(aligned_vallen) {
	// 20[iphdr] + 8(udphdr) + 18(ophdr) + 2(vallen) + aligned_vallen(val) + 2(shadowtype) + 4(stat) + 1(debug_hdr)
	//add(udp_hdr.hdrlen, aligned_vallen, 35);
	//add(ipv4_hdr.totalLen, aligned_vallen, 55);
	add(udp_hdr.hdrlen, aligned_vallen, 34);
	add(ipv4_hdr.totalLen, aligned_vallen, 54);
}

// GETRES_SEQ, GETREQ_BEINGEVICTED_RECORD, GETREQ_LARGEVALUEBLOCK_RECORD
action update_val_stat_seq_pktlen(aligned_vallen) {
	// 20[iphdr] + 8(udphdr) + 18(ophdr) + 2(vallen) + aligned_vallen(val) + 2(shadowtype) + 8(seq_hdr) + 4(stat)
	add(udp_hdr.hdrlen, aligned_vallen, 42);
	add(ipv4_hdr.totalLen, aligned_vallen, 62);
}

// GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1
action update_val_seq_inswitch_stat_clone_pktlen(aligned_vallen) {
	// [20(iphdr)] + 8(udphdr) + 18(ophdr) + 2(vallen) + aligned_vallen(val) + 2(shadowtype) + 8(seq_hdr) + 20(inswitch) + 4(stat) + 4(clone_hdr) + 1(debug_hdr)
	//add(udp_hdr.hdrlen, aligned_vallen, 67);
	//add(ipv4_hdr.totalLen, aligned_vallen, 87);
	add(udp_hdr.hdrlen, aligned_vallen, 66);
	add(ipv4_hdr.totalLen, aligned_vallen, 86);
}

// PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3, PUTREQ_SEQ_BEINGEVICTED, PUTREQ_SEQ_CASE3_BEINGEVICTED
action update_val_seq_pktlen(aligned_vallen) {
	// [20(iphdr)] + 8(udphdr) + 18(ophdr) + 2(vallen) + aligned_vallen(val) + 2(shadowtype) + 8(seq_hdr) + 1(debug_hdr)
	//add(udp_hdr.hdrlen, aligned_vallen, 39);
	//add(ipv4_hdr.totalLen, aligned_vallen, 59);
	add(udp_hdr.hdrlen, aligned_vallen, 38);
	add(ipv4_hdr.totalLen, aligned_vallen, 58);
}

// PUTRES, DELRES
action update_stat_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 18(ophdr) + 2(shadowtype) + 4(stat) + 1(debug_hdr)
	//modify_field(udp_hdr.hdrlen, 33);
	//modify_field(ipv4_hdr.totalLen, 53);
	modify_field(udp_hdr.hdrlen, 32);
	modify_field(ipv4_hdr.totalLen, 52);
}

// PUTRES_SEQ, DELRES_SEQ
action update_seq_stat_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 18(ophdr) + 2(shadowtype) + 8(seq_hdr) + 4(stat)
	modify_field(udp_hdr.hdrlen, 40);
	modify_field(ipv4_hdr.totalLen, 60);
}

// DELREQ_SEQ, DELREQ_SEQ_CASE3, DELREQ_SEQ_BEINGEVICTED, DELREQ_SEQ_CASE3_BEINGEVICTED, GETREQ_LARGEVALUEBLOCK_SEQ
action update_seq_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 18(ophdr) + 2(shadowtype) + 8(seq_hdr) + 1(debug_hdr)
	//modify_field(udp_hdr.hdrlen, 37);
	/modify_field(ipv4_hdr.totalLen, 57);
	modify_field(udp_hdr.hdrlen, 36);
	modify_field(ipv4_hdr.totalLen, 56);
}
 
// SCANREQ_SPLIT
action update_scanreqsplit_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 18(ophdr) + 16(endkey) + 7(split_hdr)
	modify_field(udp_hdr.hdrlen, 49);
	modify_field(ipv4_hdr.totalLen, 69);
}
 
// CACHE_EVICT_LOADFREQ_INSWITCH_ACK
action update_frequency_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 18(ophdr) + 4(frequency)
	modify_field(udp_hdr.hdrlen, 30);
	modify_field(ipv4_hdr.totalLen, 50);
}

// CACHE_EVICT_LOADDATA_INSWITCH_ACK
action update_val_seq_stat_pktlen(aligned_vallen) {
	// [20(iphdr)] + 8(udphdr) + 18(ophdr) + 2(vallen) + aligned_vallen(val) + 2(shadowtype) + 8(seq_hdr) + 4(stat)
	add(udp_hdr.hdrlen, aligned_vallen, 42);
	add(ipv4_hdr.totalLen, aligned_vallen, 62);
}

// LOADSNAPSHOTDATA_INSWITCH_ACK
action update_val_seq_inswitch_stat_pktlen(aligned_vallen) {
	// [20(iphdr)] + 8(udphdr) + 18(ophdr) + 2(vallen) + aligned_vallen(val) + 2(shadowtype) + 8(seq_hdr) + 20(inswitch) + 4(stat) + 1(debug_hdr)
	//add(udp_hdr.hdrlen, aligned_vallen, 63);
	//add(ipv4_hdr.totalLen, aligned_vallen, 83);
	add(udp_hdr.hdrlen, aligned_vallen, 62);
	add(ipv4_hdr.totalLen, aligned_vallen, 82);
}

// PUTREQ_LARGEVALUE_SEQ
action add_shadowtypehdr_seqhdr_pktlen() {
	// 2(shadowtype) + 8(seq_hdr)
	add_to_field(udp_hdr.hdrlen, 10);
	add_to_field(ipv4_hdr.totalLen, 10);
}
*/

action update_pktlen(udplen, iplen) {
	modify_field(udp_hdr.hdrlen, udplen);
	modify_field(ipv4_hdr.totalLen, iplen);
	//modify_field(meta.udp_hdrlen, udplen);
}

action add_pktlen(udplen_delta, iplen_delta) {
	add_to_field(udp_hdr.hdrlen, udplen_delta);
	add_to_field(ipv4_hdr.totalLen, iplen_delta);
	//add_to_field(meta.udp_hdrlen, udplen_delta);
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
		add_pktlen;
		nop;
	}
	default_action: nop(); // not change udp_hdr.hdrlen (GETREQ/GETREQ_POP/GETREQ_NLATEST)
	size: 512;
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
	size: 512;
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
