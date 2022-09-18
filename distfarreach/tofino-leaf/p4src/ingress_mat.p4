/* Ingress Processing (Normal Operation) */

field_list hash_fields {
	op_hdr.keylolo;
	op_hdr.keylohi;
	op_hdr.keyhilo;
	//op_hdr.keyhihi;
	op_hdr.keyhihilo;
	op_hdr.keyhihihi;
}

field_list_calculation hash_calc {
	input {
		hash_fields;
	}
	algorithm: crc32;
	//output_width: 16;
	output_width: 32;
}

field_list_calculation hash_calc2 {
	input {
		hash_fields;
	}
	algorithm: crc32_extend;
	//output_width: 16;
	output_width: 32;
}

field_list_calculation hash_calc3 {
	input {
		hash_fields;
	}
	algorithm: identity;
	//output_width: 16;
	output_width: 32;
}

field_list_calculation hash_calc4 {
	input {
		hash_fields;
	}
	algorithm: identity_extend;
	//output_width: 16;
	output_width: 32;
}

action nop() {}

// Stage 0

action l2l3_forward(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}

@pragma stage 0
table l2l3_forward_tbl {
	reads {
		ethernet_hdr.dstAddr: exact;
		ipv4_hdr.dstAddr: lpm;
	}
	actions {
		l2l3_forward;
		nop;
	}
	default_action: nop();
	size: 16;
}

action set_need_recirculate() {
	modify_field(meta.need_recirculate, 1);
}

action reset_need_recirculate() {
	modify_field(meta.need_recirculate, 0);
}

@pragma stage 0
table need_recirculate_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		set_need_recirculate;
		reset_need_recirculate;
	}
	default_action: reset_need_recirculate();
	size: 16;
}

action set_hot_threshold(hot_threshold) {
	modify_field(inswitch_hdr.hot_threshold, hot_threshold);
}

@pragma stage 0
table set_hot_threshold_tbl {
	actions {
		set_hot_threshold;
	}
	default_action: set_hot_threshold(DEFAULT_HH_THRESHOLD);
	size: 1;
}

action hash_for_spineselect() {
	//modify_field_with_hash_based_offset(meta.hashval_for_spineselect, 0, hash_calc, PARTITION_COUNT);
	// NOTE: we use a different hash function to simulate independent hashing
	modify_field_with_hash_based_offset(meta.hashval_for_spineselect, 0, hash_calc3, PARTITION_COUNT);
}

@pragma stage 0
table hash_for_spineselect_tbl {
	/*reads {
		op_hdr.optype: exact;
	}*/
	actions {
		hash_for_spineselect;
		//nop;
	}
	//default_action: nop();
	//size: 16;
	default_action: hash_for_spineselect();
	size: 1;
}

// Stage 1 (need_recirculate = 1)

action recirculate_pkt(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport); // forward to the first spine switch
	bypass_egress(); // bypass egress pipeline (especially for GETRES_LATEST/DELETED_SEQ_SERVER)
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter recirculate_counter {
	type : packets_and_bytes;
	direct: recirculate_tbl;
}
#endif

@pragma stage 1
table recirculate_tbl {
	reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions {
		recirculate_pkt;
		nop;
	}
	default_action: nop();
	size: 16;
}

// Stage 1 (need_recirculate = 0)

/*action reset_is_wrong_pipeline() {
	modify_field(inswitch_hdr.is_wrong_pipeline, 0);
}*/
#ifndef RANGE_SUPPORT
action hash_for_partition() {
	modify_field_with_hash_based_offset(meta.hashval_for_partition, 0, hash_calc, PARTITION_COUNT);
}
@pragma stage 1
table hash_for_partition_tbl {
	/*reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}*/
	actions {
		hash_for_partition;
		//nop;
	}
	//default_action: nop();
	//size: 32;
	default_action: hash_for_partition();
	size: 1;
}
#endif

action hash_for_cm12() {
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm1, 0, hash_calc, CM_BUCKET_COUNT);
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm2, 0, hash_calc2, CM_BUCKET_COUNT);
}

@pragma stage 1
table hash_for_cm12_tbl {
	/*reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}*/
	actions {
		hash_for_cm12;
		//nop;
	}
	//default_action: nop();
	//size: 4;
	default_action: hash_for_cm12();
	size: 1;
}

// Stage 2

action spineselect(eport, globalswitchidx) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	modify_field(op_hdr.globalswitchidx, globalswitchidx);
}

action spineselect_for_special_response(globalswitchidx) {
	modify_field(meta.spineswitchidx, globalswitchidx);
}

@pragma stage 2
table spineselect_tbl {
	reads {
		op_hdr.optype: exact;
		meta.hashval_for_spineselect: range;
		meta.need_recirculate: exact;
	}
	actions {
		spineselect;
		spineselect_for_special_response;
		nop;
	}
	default_action: nop();
	size: SPINESELECT_ENTRY_NUM;
}

action hash_for_cm34() {
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm3, 0, hash_calc3, CM_BUCKET_COUNT);
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm4, 0, hash_calc4, CM_BUCKET_COUNT);
}

@pragma stage 2
table hash_for_cm34_tbl {
	/*reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}*/
	actions {
		hash_for_cm34;
		//nop;
	}
	//default_action: nop();
	//size: 4;
	default_action: hash_for_cm34();
	size: 1;
}

action set_snapshot_flag() {
	modify_field(inswitch_hdr.snapshot_flag, 1);
}

action reset_snapshot_flag() {
	modify_field(inswitch_hdr.snapshot_flag, 0);
}

@pragma stage 2
table snapshot_flag_tbl {
	reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions {
		set_snapshot_flag;
		reset_snapshot_flag;
		nop;
	}
	default_action: reset_snapshot_flag();
	size: 32;
}

// Stage 3~4

action cached_action(idx) {
	modify_field(inswitch_hdr.idx, idx);
	modify_field(inswitch_hdr.is_cached, 1);
}

action uncached_action() {
	modify_field(inswitch_hdr.is_cached, 0);
}

@pragma stage 3 16384
@pragma stage 4
table cache_lookup_tbl {
	reads {
		op_hdr.keylolo: exact;
		op_hdr.keylohi: exact;
		op_hdr.keyhilo: exact;
		//op_hdr.keyhihi: exact;
		op_hdr.keyhihilo: exact;
		op_hdr.keyhihihi: exact;
		op_hdr.globalswitchidx: exact;
		meta.need_recirculate: exact;
	}
	actions {
		cached_action;
		uncached_action;
	}
	default_action: uncached_action();
	size: LOOKUP_ENTRY_COUNT; // egress_pipenum * KV_BUCKET_COUNT
}

// Stage 5~6

#ifdef RANGE_SUPPORT
action range_partition(udpport, eport) {
	modify_field(udp_hdr.dstPort, udpport);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
action range_partition_for_scan(udpport, eport, start_globalserveridx) {
	modify_field(udp_hdr.dstPort, udpport);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	modify_field(split_hdr.globalserveridx, start_globalserveridx);
}
action range_partition_for_special_response(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
@pragma stage 5 2048
@pragma stage 6
table range_partition_tbl {
	reads {
		op_hdr.optype: exact;
		op_hdr.keyhihihi: range;
		op_hdr.globalswitchidx: exact;
		meta.need_recirculate: exact;
	}
	actions {
		range_partition;
		range_partition_for_scan;
		range_partition_for_special_response;
		nop;
	}
	default_action: nop();
	size: RANGE_PARTITION_ENTRY_NUM;
}
#else
action hash_partition(udpport, eport) {
	modify_field(udp_hdr.dstPort, udpport);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
action hash_partition_for_special_response(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
@pragma stage 5 2048
@pragma stage 6
table hash_partition_tbl {
	reads {
		op_hdr.optype: exact;
		meta.hashval_for_partition: range;
		//ig_intr_md.ingress_port: exact;
		meta.need_recirculate: exact;
	}
	actions {
		hash_partition;
		//reset_is_wrong_pipeline;
		hash_partition_for_special_response;
		nop;
	}
	//default_action: reset_is_wrong_pipeline();
	default_action: nop();
	size: HASH_PARTITION_ENTRY_NUM;
}
#endif

// Stage 7

#ifdef RANGE_SUPPORT
action range_partition_for_scan_endkey(end_globalserveridx_plus_one) {
	modify_field(split_hdr.is_clone, 0);
	modify_field(split_hdr.cur_scanidx, 0);
	subtract(split_hdr.max_scannum, end_globalserveridx_plus_one, split_hdr.globalserveridx);
}

@pragma stage 7
table range_partition_for_scan_endkey_tbl {
	reads {
		op_hdr.optype: exact;
		scan_hdr.keyhihihi: range;
		op_hdr.globalswitchidx: exact;
		meta.need_recirculate: exact;
	}
	actions {
		range_partition_for_scan_endkey;
		nop;
	}
	default_action: nop();
	size: RANGE_PARTITION_FOR_SCAN_ENDKEY_ENTRY_NUM;
}
#endif

// Stage 8

/*action set_client_sid(client_sid, eport) {
	modify_field(inswitch_hdr.client_sid, client_sid);
	// NOTE: eport_for_res and client_sid must be in the same group for ALU access; as compiler aims to place them into the same container, they must come from the same source (action parameter or PHV)
	//modify_field(inswitch_hdr.eport_for_res, ig_intr_md.ingress_port);
	modify_field(inswitch_hdr.eport_for_res, eport);
}*/

// NOTE: eg_intr_md.egress_port is a read-only field (we cannot directly set egress port in egress pipeline even if w/ correct pipeline)
// NOTE: using inswitch_hdr.client_sid for clone_e2e in ALU needs to maintain inswitch_hdr.client_sid and eg_intr_md_for_md.mirror_id into the same group, which violates PHV allocation constraints -> but MAU can access different groups
action set_client_sid(client_sid) {
	modify_field(inswitch_hdr.client_sid, client_sid);
}

@pragma stage 8
table prepare_for_cachehit_tbl {
	reads {
		op_hdr.optype: exact;
		//ig_intr_md.ingress_port: exact;
		ipv4_hdr.srcAddr: lpm;
		meta.need_recirculate: exact;
	}
	actions {
		set_client_sid;
		nop;
	}
	default_action: set_client_sid(0); // deprecated: configured as set_client_sid(sids[0]) in ptf
	size: 64;
}

action forward_normal_response(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}

action forward_special_get_response_to_server_and_clone_to_spine(client_sid) {
	// NOTE: eport to server has already been set by partition_tbl for GETRES_LATEST/DELETED_SEQ_SERVER/_INSWITCH
	//modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port); // Original packet enters the egress pipeline to server
	clone_ingress_pkt_to_egress(client_sid); // Cloned packet enter the egress pipeline to corresponding spine switch
}

action forward_special_get_response_to_spine(eport) {
	// NOTE: reset eport to spine switch instead of server (set by partition_tbl before)
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	// NOTE: server-leaf resets globalswitchidx as spineswitchidx (set by spineselect_tbl) for special responses to lookup cache in spine switch
	modify_field(op_hdr.globalswitchidx, meta.spineswitchidx);
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter ipv4_forward_counter {
	type : packets_and_bytes;
	direct: ipv4_forward_tbl;
}
#endif

@pragma stage 8
table ipv4_forward_tbl {
	reads {
		op_hdr.optype: exact;
		ipv4_hdr.dstAddr: lpm;
		inswitch_hdr.is_cached: exact;
		meta.need_recirculate: exact;
	}
	actions {
		forward_normal_response;
		forward_special_get_response_to_server_and_clone_to_spine;
		forward_special_get_response_to_spine;
		nop;
	}
	default_action: nop();
	size: 256;
}

// Stage 9

action sample() {
	//modify_field_with_hash_based_offset(inswitch_hdr.is_sampled, 0, hash_calc, 2); // WRONG: we should not sample key
	modify_field_rng_uniform(inswitch_hdr.is_sampled, 0, 1); // generate a random value in [0, 1] to sample packet
}

@pragma stage 9
table sample_tbl {
	/*reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}*/
	actions {
		sample;
		//nop;
	}
	//default_action: nop();
	//size: 4;
	default_action: sample();
	size: 1;
}

action update_getreq_spine_to_getreq_inswitch() {
	modify_field(op_hdr.optype, GETREQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, GETREQ_INSWITCH);
	add_header(shadowtype_hdr);
	add_header(inswitch_hdr);
}

action update_getres_latest_seq_server_to_getres_latest_seq_inswitch() {
	modify_field(op_hdr.optype, GETRES_LATEST_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, GETRES_LATEST_SEQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_getres_deleted_seq_server_to_getres_deleted_seq_inswitch() {
	modify_field(op_hdr.optype, GETRES_DELETED_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, GETRES_DELETED_SEQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_putreq_seq_to_putreq_seq_inswitch() {
	modify_field(op_hdr.optype, PUTREQ_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_delreq_seq_to_delreq_seq_inswitch() {
	modify_field(op_hdr.optype, DELREQ_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_getres_server_to_getres() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
}

action update_scanres_split_server_to_scanres_split() {
	modify_field(op_hdr.optype, SCANRES_SPLIT);
}

action update_putres_server_to_putres() {
	modify_field(op_hdr.optype, PUTRES);
	modify_field(shadowtype_hdr.shadowtype, PUTRES);
}

action update_delres_server_to_delres() {
	modify_field(op_hdr.optype, DELRES);
	modify_field(shadowtype_hdr.shadowtype, DELRES);
}

action update_warmupreq_spine_to_warmupreq() {
	modify_field(op_hdr.optype, WARMUPREQ);
}

action update_loadreq_spine_to_loadreq() {
	modify_field(op_hdr.optype, LOADREQ);
	modify_field(shadowtype_hdr.shadowtype, LOADREQ);
}

action update_warmupack_server_to_warmupack() {
	modify_field(op_hdr.optype, WARMUPACK);
}

action update_loadack_server_to_loadack() {
	modify_field(op_hdr.optype, LOADACK);
}

action update_getres_latest_seq_server_inswitch_to_getres_latest_seq_inswitch() {
	modify_field(op_hdr.optype, GETRES_LATEST_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, GETRES_LATEST_SEQ_INSWITCH);
	//add_header(inswitch_hdr);
}

action update_getres_deleted_seq_server_inswitch_to_getres_deleted_seq_inswitch() {
	modify_field(op_hdr.optype, GETRES_DELETED_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, GETRES_DELETED_SEQ_INSWITCH);
	//add_header(inswitch_hdr);
}

action update_putreq_largevalue_seq_to_putreq_largevalue_seq_inswitch() {
	// NOTE: PUTREQ_LARGEVALUE_SEQ w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ_INSWITCH w/ op_hdr + shadowtype_hdr + seq_hdr + inswitch_hdr + fraginfo_hdr
	modify_field(op_hdr.optype, PUTREQ_LARGEVALUE_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_LARGEVALUE_SEQ_INSWITCH);

	add_header(inswitch_hdr);
}

action update_getres_largevalue_server_to_getres_largevalue() {
	modify_field(op_hdr.optype, GETRES_LARGEVALUE);
	// NOTE: NO shadowtype_hdr for GETRES_LARGEVALUE
}

action update_putreq_seq_beingevicted_to_putreq_seq_case3_beingevicted() {
	modify_field(op_hdr.optype, PUTREQ_SEQ_CASE3_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ_CASE3_BEINGEVICTED);
}

action update_delreq_seq_beingevicted_to_delreq_seq_case3_beingevicted() {
	modify_field(op_hdr.optype, DELREQ_SEQ_CASE3_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ_CASE3_BEINGEVICTED);
}

action update_putreq_largevalue_seq_beingevicted_to_putreq_largevalue_seq_case3_beingevicted() {
	modify_field(op_hdr.optype, PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED);
}

action update_putreq_seq_beingevicted_spine_to_putreq_seq_beingevicted() {
	modify_field(op_hdr.optype, PUTREQ_SEQ_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ_BEINGEVICTED);
}

action update_putreq_seq_case3_beingevicted_spine_to_putreq_seq_case3_beingevicted() {
	modify_field(op_hdr.optype, PUTREQ_SEQ_CASE3_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ_CASE3_BEINGEVICTED);
}

action update_delreq_seq_beingevicted_spine_to_delreq_seq_beingevicted() {
	modify_field(op_hdr.optype, DELREQ_SEQ_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ_BEINGEVICTED);
}

action update_delreq_seq_case3_beingevicted_spine_to_delreq_seq_case3_beingevicted() {
	modify_field(op_hdr.optype, DELREQ_SEQ_CASE3_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ_CASE3_BEINGEVICTED);
}

action update_putreq_largevalue_seq_beingevicted_spine_to_putreq_largevalue_seq_beingevicted() {
	modify_field(op_hdr.optype, PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED);
}

action update_putreq_seq_case3_beingevicted_spine_to_putreq_seq_case3_beingevicted() {
	modify_field(op_hdr.optype, PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED);
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter ig_port_forward_counter {
	type : packets_and_bytes;
	direct: ig_port_forward_tbl;
}
#endif

@pragma stage 9
table ig_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.snapshot_flag: exact;
		meta.need_recirculate: exact;
	}
	actions {
		update_getreq_spine_to_getreq_inswitch;
		update_getres_latest_seq_server_to_getres_latest_seq_inswitch;
		update_getres_deleted_seq_server_to_getres_deleted_seq_inswitch;
		update_putreq_seq_to_putreq_seq_inswitch;
		update_delreq_seq_to_delreq_seq_inswitch;
		update_getres_server_to_getres;
		update_scanres_split_server_to_scanres_split;
		update_putres_server_to_putres;
		update_delres_server_to_delres;
		update_warmupreq_spine_to_warmupreq;
		update_loadreq_spine_to_loadreq;
		update_warmupack_server_to_warmupack;
		update_loadack_server_to_loadack;
		update_getres_latest_seq_server_inswitch_to_getres_latest_seq_inswitch;
		update_getres_deleted_seq_server_inswitch_to_getres_deleted_seq_inswitch;
		update_putreq_largevalue_seq_to_putreq_largevalue_seq_inswitch;
		update_getres_largevalue_server_to_getres_largevalue;
		update_putreq_seq_beingevicted_to_putreq_seq_case3_beingevicted;
		update_delreq_seq_beingevicted_to_delreq_seq_case3_beingevicted;
		update_putreq_largevalue_seq_beingevicted_to_putreq_largevalue_seq_case3_being_evicted;
		update_putreq_seq_beingevicted_spine_to_putreq_seq_beingevicted;
		update_putreq_seq_case3_beingevicted_spine_to_putreq_seq_case3_beingevicted;
		update_delreq_seq_beingevicted_spine_to_delreq_seq_beingevicted;
		update_delreq_seq_case3_beingevicted_spine_to_delreq_seq_case3_beingevicted;
		update_putreq_largevalue_seq_beingevicted_spine_to_putreq_largevalue_seq_beingevicted;
		update_putreq_largevalue_seq_case3_beingevicted_spine_to_putreq_largevalue_seq_case3_beingevicted;
		nop;
	}
	default_action: nop();
	size: 64;
}
