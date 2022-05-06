/* Ingress Processing (Normal Operation) */

action nop() {}

// Stage 0

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
		ig_intr_md.ingress_port: exact;
	}
	actions {
		set_need_recirculate;
		reset_need_recirculate;
	}
	default_action: reset_need_recirculate();
	size: 8;
}

// Stage 1

action recirculate_pkt(port) {
	recirculate(port);
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
	size: 2;
}

// Stage 1

action set_snapshot_flag() {
	modify_field(inswitch_hdr.snapshot_flag, 1);
}

action reset_snapshot_flag() {
	modify_field(inswitch_hdr.snapshot_flag, 0);
}

@pragma stage 1
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
	size: 2;
}

action set_sid(sid) {
	modify_field(inswitch_hdr.sid, sid);
	modify_field(inswitch_hdr.eport_for_res, ig_intr_md.ingress_port);
}

@pragma stage 1
table sid_tbl {
	reads {
		op_hdr.optype: exact;
		ig_intr_md.ingress_port: exact;
		meta.need_recirculate: exact;
	}
	actions {
		set_sid;
		nop;
	}
	default_action: nop();
	size: 8;
}

action cached_action(idx) {
	modify_field(inswitch_hdr.idx, idx);
	modify_field(inswitch_hdr.is_cached, 1);
}

action uncached_action() {
	modify_field(inswitch_hdr.is_cached, 0);
}

@pragma stage 1
table cache_lookup_tbl {
	reads {
		op_hdr.keylolo: exact;
		op_hdr.keylohi: exact;
		op_hdr.keyhilo: exact;
		op_hdr.keyhihi: exact;
		meta.need_recirculate: exact;
	}
	actions {
		cached_action;
		uncached_action;
	}
	default_action: uncached_action();
	size: LOOKUP_ENTRY_COUNT; // egress_pipenum * KV_BUCKET_COUNT
}

field_list hash_fields {
	op_hdr.keylolo;
	op_hdr.keylohi;
	op_hdr.keyhilo;
	op_hdr.keyhihi;
}

field_list_calculation hash_calc {
	input {
		hash_fields;
	}
	algorithm: crc32;
	//output_width: 16;
	output_width: 32;
}

/*field_list_calculation sample_calc {
	input {
		hash_fields;
	}
	algorithm: crc32;
	output_width: 1;
}*/

#ifdef RANGE_SUPPORT
action range_partition(udpport, eport, is_wrong_pipeline) {
	modify_field(udp_hdr.dstPort, udpport);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	modify_field(inswitch_hdr.is_wrong_pipeline, is_wrong_pipeline);
}
@pragma stage 1
table range_partition_tbl {
	reads {
		op_hdr.optype: exact;
		op_hdr.keyhihi: range;
		ig_intr_md.ingress_port: exact;
		meta.need_recirculate: exact;
	}
	actions {
		range_partition;
		nop;
	}
	default_action: nop();
	size: RANGE_PARTITION_ENTRY_NUM;
}
#else
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
	size: 4;
}
#endif

action hash_for_cm() {
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm, 0, hash_calc, CM_BUCKET_COUNT);
}

@pragma stage 1
table hash_for_cm_tbl {
	reads {
		op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions {
		hash_for_cm;
		nop;
	}
	default_action: nop();
	size: 2;
}

action hash_for_seq() {
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_seq, 0, hash_calc, SEQ_BUCKET_COUNT);
}

@pragma stage 1
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
	size: 2;
}

action sample() {
	//modify_field_with_hash_based_offset(inswitch_hdr.is_sampled, 0, sample_calc, 2);
	modify_field_with_hash_based_offset(inswitch_hdr.is_sampled, 0, hash_calc, 2);
}

@pragma stage 1
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

// Stage 2

#ifdef RANGE_SUPPORT
action range_partition_for_scan(udpport, eport, max_scannum) {
	modify_field(udp_hdr.dstPort, udpport);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	modify_field(split_hdr.cur_scanidx, 0);
	modify_field(split_hdr.max_scannum, max_scannum);
}

@pragma stage 2
table range_partition_for_scan_tbl {
	reads {
		op_hdr.optype: exact;
		op_hdr.keyhihi: range;
		scan_hdr.keyhihi: range;
		meta.need_recirculate: exact;
	}
	actions {
		range_partition_for_scan;
		nop;
	}
	default_action: nop();
	size: RANGE_PARTITION_FOR_SCAN_ENTRY_NUM;
}
#else
action hash_partition(udpport, eport, is_wrong_pipeline) {
	modify_field(udp_hdr.dstPort, udpport);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	modify_field(inswitch_hdr.is_wrong_pipeline, is_wrong_pipeline);
}

@pragma stage 2
table hash_partition_tbl {
	reads {
		op_hdr.optype: exact;
		meta.hashval_for_partition: range;
		ig_intr_md.ingress_port: exact;
		meta.need_recirculate: exact;
	}
	actions {
		hash_partition;
		nop;
	}
	default_action: nop();
	size: HASH_PARTITION_ENTRY_NUM;
}
#endif

// Stage 3

action update_getreq_to_getreq_inswitch() {
	modify_field(op_hdr.optype, GETREQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_getres_latest_seq_to_getres_latest_seq_inswitch() {
	modify_field(op_hdr.optype, GETRES_LATEST_SEQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_getres_deleted_seq_to_getres_deleted_seq_inswitch() {
	modify_field(op_hdr.optype, GETRES_DELETED_SEQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_putreq_to_putreq_inswitch() {
	modify_field(op_hdr.optype, PUTREQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_delreq_to_delreq_inswitch() {
	modify_field(op_hdr.optype, DELREQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_scanreq_to_scanreq_split() {
	modify_field(op_hdr.optype, SCANREQ_SPLIT);
	add_header(split_hdr);
}

@pragma stage 3
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
		update_scanreq_to_scanreq_split;
		nop;
	}
	default_action: nop();
	size: 8;
}

// Stage 4

action forward_normal_response(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}

action forward_special_get_response(sid) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port); // Original packet enters the egress pipeline to server
	clone_ingress_pkt_to_egress(sid); // Cloned packet enter the egress pipeline to corresponding client
}

@pragma stage 4
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
	size: 8;
}

