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

action set_need_recirculate(eport) {
	modify_field(meta.need_recirculate, 1);
	modify_field(meta.recirport, eport);
}

action reset_need_recirculate() {
	modify_field(meta.need_recirculate, 0);
}

@pragma stage 0
table need_recirculate_tbl {
	reads {
		op_hdr.optype: exact;
		ig_intr_md.ingress_port: exact;
	}
	actions {
		set_need_recirculate;
		reset_need_recirculate;
	}
	default_action: reset_need_recirculate();
	size: 8;
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

// Stage 1 (need_recirculate = 1)

//action recirculate_pkt(port) {
//	recirculate(port);
//}

// NOTE: as our Tofino does not support cross-ingress-pipeline recirculation, we use hardware link to simluate it
action recirculate_pkt() {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, meta.recirport);
	bypass_egress();
}

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
	reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions {
		hash_for_partition;
		nop;
	}
	default_action: nop();
	size: 16;
}
#endif

// Stage 2

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
@pragma stage 2
table range_partition_tbl {
	reads {
		op_hdr.optype: exact;
		//op_hdr.keyhihi: range;
		op_hdr.keyhihihi: range;
		//ig_intr_md.ingress_port: exact;
		meta.need_recirculate: exact;
	}
	actions {
		range_partition;
		//reset_is_wrong_pipeline;
		range_partition_for_scan;
		range_partition_for_special_response;
		nop;
	}
	//default_action: reset_is_wrong_pipeline();
	default_action: nop();
	size: RANGE_PARTITION_ENTRY_NUM;
}
#else
/*action hash_partition(udpport, eport, is_wrong_pipeline) {
	modify_field(udp_hdr.dstPort, udpport);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	modify_field(inswitch_hdr.is_wrong_pipeline, is_wrong_pipeline);
}*/
action hash_partition(udpport, eport) {
	modify_field(udp_hdr.dstPort, udpport);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
action hash_partition_for_special_response(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
@pragma stage 2
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

action hash_for_cm12() {
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm1, 0, hash_calc, CM_BUCKET_COUNT);
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm2, 0, hash_calc2, CM_BUCKET_COUNT);
}

@pragma stage 2
table hash_for_cm12_tbl {
	reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions {
		hash_for_cm12;
		nop;
	}
	default_action: nop();
	size: 2;
}

#ifndef USE_BFSDE920
action cached_action(idx) {
	modify_field(inswitch_hdr.idx, idx);
	modify_field(inswitch_hdr.is_cached, 1);
}

action uncached_action() {
	modify_field(inswitch_hdr.is_cached, 0);
}

@pragma stage 2 16384
@pragma stage 3
table cache_lookup_tbl {
	reads {
		op_hdr.keylolo: exact;
		op_hdr.keylohi: exact;
		op_hdr.keyhilo: exact;
		//op_hdr.keyhihi: exact;
		op_hdr.keyhihilo: exact;
		op_hdr.keyhihihi: exact;
		meta.need_recirculate: exact;
	}
	actions {
		cached_action;
		uncached_action;
	}
	default_action: uncached_action();
	size: LOOKUP_ENTRY_COUNT; // egress_pipenum * KV_BUCKET_COUNT
}
#endif

// Stage 3

#ifdef USE_BFSDE920
action cached_action(idx) {
	modify_field(inswitch_hdr.idx, idx);
	modify_field(inswitch_hdr.is_cached, 1);
}

action uncached_action() {
	modify_field(inswitch_hdr.is_cached, 0);
}

@pragma stage 3
table cache_lookup_tbl {
	reads {
		op_hdr.keylolo: exact;
		op_hdr.keylohi: exact;
		op_hdr.keyhilo: exact;
		//op_hdr.keyhihi: exact;
		op_hdr.keyhihilo: exact;
		op_hdr.keyhihihi: exact;
		meta.need_recirculate: exact;
	}
	actions {
		cached_action;
		uncached_action;
	}
	default_action: uncached_action();
	size: LOOKUP_ENTRY_COUNT; // egress_pipenum * KV_BUCKET_COUNT
}
#endif

#ifdef RANGE_SUPPORT
//action range_partition_for_scan_endkey(last_udpport_plus_one) {
action range_partition_for_scan_endkey(end_globalserveridx_plus_one) {
	modify_field(split_hdr.is_clone, 0);
	modify_field(split_hdr.cur_scanidx, 0);
	//subtract(split_hdr.max_scannum, last_udpport_plus_one, udp_hdr.dstPort);
	subtract(split_hdr.max_scannum, end_globalserveridx_plus_one, split_hdr.globalserveridx);
}

@pragma stage 3
table range_partition_for_scan_endkey_tbl {
	reads {
		op_hdr.optype: exact;
		//scan_hdr.keyhihi: range;
		scan_hdr.keyhihihi: range;
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

/*action hash_for_cm2() {
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm2, 0, hash_calc2, CM_BUCKET_COUNT);
}

@pragma stage 3
table hash_for_cm2_tbl {
	reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions {
		hash_for_cm2;
		nop;
	}
	default_action: nop();
	size: 2;
}*/

action hash_for_seq() {
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_seq, 0, hash_calc, SEQ_BUCKET_COUNT);
}

@pragma stage 3
table hash_for_seq_tbl {
	reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions {
		hash_for_seq;
		nop;
	}
	default_action: nop();
	size: 4;
}

// Stage 4

action hash_for_cm34() {
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm3, 0, hash_calc3, CM_BUCKET_COUNT);
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm4, 0, hash_calc4, CM_BUCKET_COUNT);
}

@pragma stage 4
table hash_for_cm34_tbl {
	reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions {
		hash_for_cm34;
		nop;
	}
	default_action: nop();
	size: 2;
}

action set_snapshot_flag(snapshotid) {
	modify_field(inswitch_hdr.snapshot_flag, 1);
	//modify_field(seq_hdr.snapshot_token, snapshotid);
	modify_field(inswitch_hdr.snapshot_token, snapshotid);
}

action reset_snapshot_flag() {
	modify_field(inswitch_hdr.snapshot_flag, 0);
	//modify_field(seq_hdr.snapshot_token, 0);
	modify_field(inswitch_hdr.snapshot_token, 0);
}

@pragma stage 4
table snapshot_flag_tbl {
	reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions {
		set_snapshot_flag;
		reset_snapshot_flag;
	}
	default_action: reset_snapshot_flag();
	size: 8;
}

// Stage 5

/*action hash_for_cm4() {
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm4, 0, hash_calc4, CM_BUCKET_COUNT);
}

@pragma stage 5
table hash_for_cm4_tbl {
	reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions {
		hash_for_cm4;
		nop;
	}
	default_action: nop();
	size: 2;
}*/

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

@pragma stage 5
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
	size: 32;
}

action forward_normal_response(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}

action forward_special_get_response(client_sid) {
	// NOTE: eport to server has already been set by partition_tbl for GETRES_LATEST/DELETED_SEQ
	//modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port); // Original packet enters the egress pipeline to server
	clone_ingress_pkt_to_egress(client_sid); // Cloned packet enter the egress pipeline to corresponding client
}

@pragma stage 5
table ipv4_forward_tbl {
	reads {
		op_hdr.optype: exact;
		ipv4_hdr.dstAddr: lpm;
		meta.need_recirculate: exact;
	}
	actions {
		forward_normal_response;
		forward_special_get_response;
		nop;
	}
	default_action: nop();
	size: 128;
}

// Stage 6

action sample() {
	//modify_field_with_hash_based_offset(inswitch_hdr.is_sampled, 0, hash_calc, 2); // WRONG: we should not sample key
	modify_field_rng_uniform(inswitch_hdr.is_sampled, 0, 1); // generate a random value in [0, 1] to sample packet
}

@pragma stage 6
table sample_tbl {
	reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions {
		sample;
		nop;
	}
	default_action: nop();
	size: 2;
}

action update_getreq_to_getreq_inswitch() {
	modify_field(op_hdr.optype, GETREQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, GETREQ_INSWITCH);
	add_header(shadowtype_hdr);
	add_header(inswitch_hdr);
}

action update_getres_latest_seq_to_getres_latest_seq_inswitch() {
	modify_field(op_hdr.optype, GETRES_LATEST_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, GETRES_LATEST_SEQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_getres_deleted_seq_to_getres_deleted_seq_inswitch() {
	modify_field(op_hdr.optype, GETRES_DELETED_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, GETRES_DELETED_SEQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_putreq_to_putreq_inswitch() {
	modify_field(op_hdr.optype, PUTREQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_INSWITCH);
	add_header(shadowtype_hdr);
	add_header(inswitch_hdr);
}

action update_delreq_to_delreq_inswitch() {
	modify_field(op_hdr.optype, DELREQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_INSWITCH);
	add_header(shadowtype_hdr);
	add_header(inswitch_hdr);
}

action update_putreq_largevalue_to_putreq_largevalue_inswitch() {
	// NOTE: PUTREQ_LARGEVALUE only w/ op_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr
	modify_field(op_hdr.optype, PUTREQ_LARGEVALUE_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_LARGEVALUE_INSWITCH);

	add_header(shadowtype_hdr);
	add_header(inswitch_hdr);
}

#ifdef RANGE_SUPPORT
action update_scanreq_to_scanreq_split() {
	modify_field(op_hdr.optype, SCANREQ_SPLIT);
	add_header(split_hdr);
}
#endif

@pragma stage 6
table ig_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions {
		update_getreq_to_getreq_inswitch;
		update_getres_latest_seq_to_getres_latest_seq_inswitch;
		update_getres_deleted_seq_to_getres_deleted_seq_inswitch;
		update_putreq_to_putreq_inswitch;
		update_delreq_to_delreq_inswitch;
#ifdef RANGE_SUPPORT
		update_scanreq_to_scanreq_split;
#endif
		update_putreq_largevalue_to_putreq_largevalue_inswitch;
		nop;
	}
	default_action: nop();
	size: 8;
}

/*action copy_udplen_for_checksum() {
	modify_field(meta.udp_hdrlen, udp_hdr.hdrlen);
}

@pragma stage 7
table ig_copy_udplen_for_checksum_tbl {
	actions {
		copy_udplen_for_checksum;
	}
	default_action: copy_udplen_for_checksum();
	size: 1;
}*/
