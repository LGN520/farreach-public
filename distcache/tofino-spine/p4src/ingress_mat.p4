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

// Stage 1~2

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
	}
	actions {
		hash_for_partition;
		nop;
	}
	default_action: nop();
	size: 16;
}
#endif

action cached_action(idx) {
	modify_field(inswitch_hdr.idx, idx);
	modify_field(inswitch_hdr.is_cached, 1);
}

action uncached_action() {
	modify_field(inswitch_hdr.is_cached, 0);
}

@pragma stage 1 16384
@pragma stage 2
table cache_lookup_tbl {
	reads {
		op_hdr.keylolo: exact;
		op_hdr.keylohi: exact;
		op_hdr.keyhilo: exact;
		//op_hdr.keyhihi: exact;
		op_hdr.keyhihilo: exact;
		op_hdr.keyhihihi: exact;
		op_hdr.spineswitchidx: exact;
	}
	actions {
		cached_action;
		uncached_action;
	}
	default_action: uncached_action();
	size: LOOKUP_ENTRY_COUNT; // egress_pipenum * KV_BUCKET_COUNT
}

// Stage 3

action hash_for_seq() {
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_seq, 0, hash_calc, SEQ_BUCKET_COUNT);
}

@pragma stage 3
table hash_for_seq_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		hash_for_seq;
		nop;
	}
	default_action: nop();
	size: 2;
}

// Stage 4~5

#ifdef RANGE_SUPPORT
action range_partition(eport, leafswitchidx) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	modify_field(op_hdr.leafswitchidx, leafswitchidx);
}
action range_partition_for_distcache_invalidate(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
/*action range_partition_for_netcache_valueupdate(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}*/
/*action range_partition_for_distcache_spine_valueupdate_inswitch(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}*/
action range_partition_for_distcache_valueupdate_inswitch(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
@pragma stage 4 2048
@pragma stage 5
table range_partition_tbl {
	reads {
		op_hdr.optype: exact;
		op_hdr.keyhihihi: range;
	}
	actions {
		range_partition;
		range_partition_for_distcache_invalidate;
		//range_partition_for_netcache_valueupdate;
		//range_partition_for_distcache_spine_valueupdate_inswitch;
		range_partition_for_distcache_valueupdate_inswitch;
		nop;
	}
	default_action: nop();
	size: RANGE_PARTITION_ENTRY_NUM;
}
#else
action hash_partition(eport, leafswitchidx) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	modify_field(op_hdr.leafswitchidx, leafswitchidx);
}
action hash_partition_for_distcache_invalidate(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
/*action hash_partition_for_netcache_valueupdate(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}*/
/*action hash_partition_for_distcache_spine_valueupdate_inswitch(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}*/
action hash_partition_for_distcache_valueupdate_inswitch(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
@pragma stage 4 2048
@pragma stage 5
table hash_partition_tbl {
	reads {
		op_hdr.optype: exact;
		meta.hashval_for_partition: range;
	}
	actions {
		hash_partition;
		hash_partition_for_distcache_invalidate;
		//hash_partition_for_netcache_valueupdate;
		//hash_partition_for_distcache_spine_valueupdate_inswitch;
		hash_partition_for_distcache_valueupdate_inswitch;
		nop;
	}
	default_action: nop();
	size: HASH_PARTITION_ENTRY_NUM;
}
#endif

// Stage 6

#ifdef RANGE_SUPPORT
action range_partition_for_scan_endkey(end_globalswitchidx_plus_one) {
	modify_field(split_hdr.is_clone, 0);
	modify_field(split_hdr.cur_scanswitchidx, 0);
	subtract(split_hdr.max_scanswitchnum, end_globalswitchidx_plus_one, op_hdr.leafswitchidx);
}

@pragma stage 6
table range_partition_for_scan_endkey_tbl {
	reads {
		op_hdr.optype: exact;
		//scan_hdr.keyhihi: range;
		scan_hdr.keyhihihi: range;
	}
	actions {
		range_partition_for_scan_endkey;
		nop;
	}
	default_action: nop();
	size: RANGE_PARTITION_FOR_SCAN_ENDKEY_ENTRY_NUM;
}
#endif

// Stage 7

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

@pragma stage 7
table prepare_for_cachehit_tbl {
	reads {
		op_hdr.optype: exact;
		//ig_intr_md.ingress_port: exact;
		ipv4_hdr.srcAddr: lpm;
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

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter ipv4_forward_counter {
	type : packets_and_bytes;
	direct: ipv4_forward_tbl;
}
#endif

@pragma stage 7
table ipv4_forward_tbl {
	reads {
		op_hdr.optype: exact;
		ipv4_hdr.dstAddr: lpm;
	}
	actions {
		forward_normal_response;
		nop;
	}
	default_action: nop();
	size: 64;
}

// Stage 8

action sample() {
	//modify_field_with_hash_based_offset(inswitch_hdr.is_sampled, 0, hash_calc, 2); // WRONG: we should not sample key
	modify_field_rng_uniform(inswitch_hdr.is_sampled, 0, 1); // generate a random value in [0, 1] to sample packet
}

@pragma stage 8
table sample_tbl {
	reads {
		op_hdr.optype: exact;
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

	//add_header(shadowtype_hdr);
	add_header(inswitch_hdr);
	//add_header(switchload_hdr);
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

#ifdef RANGE_SUPPORT
action update_scanreq_to_scanreq_split() {
	modify_field(op_hdr.optype, SCANREQ_SPLIT);
	add_header(split_hdr);
}
#endif

action update_warmupreq_to_netcache_warmupreq_inswitch() {
	modify_field(op_hdr.optype, NETCACHE_WARMUPREQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, NETCACHE_WARMUPREQ_INSWITCH);
	add_header(shadowtype_hdr);
	add_header(inswitch_hdr);
}

action update_loadreq_to_loadreq_spine() {
	modify_field(op_hdr.optype, LOADREQ_SPINE);
	modify_field(shadowtype_hdr.shadowtype, LOADREQ_SPINE);
}

action update_distcache_invalidate_to_distcache_invalidate_inswitch() {
	modify_field(op_hdr.optype, DISTCACHE_INVALIDATE_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, DISTCACHE_INVALIDATE_INSWITCH);

	add_header(shadowtype_hdr);
	add_header(inswitch_hdr);

	// swap to set dstport as corresponding server.invalidateserver port
	swap(udp_hdr.srcPort, udp_hdr.dstPort);
}

/*action update_netcache_valueupdate_to_netcache_valueupdate_inswitch() {
	modify_field(op_hdr.optype, NETCACHE_VALUEUPDATE_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, NETCACHE_VALUEUPDATE_INSWITCH);

	add_header(inswitch_hdr);

	//// NOTE: NETCACHE_VALUEUPDATE does not need partition_tbl, as in-switch record must in the same pipeline of the ingress port, which can also send NETCACHE_VALUEUPDATE_ACK back to corresponding server
	// NOTE: range/hash_partition_tbl sets eport for NETCACHE_VALUEUPDATE to forward it to egress pipeline of server-leaf
	//modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);

	// swap to set dstport as corresponding server.valueupdateserver port to convert NETCACHE_VALUEUPDATE as NETCACHE_VALUEUPDATE_ACK
	swap(udp_hdr.srcPort, udp_hdr.dstPort);
}*/

/*action swap_udpport_for_distcache_spine_valueupdate_inswitch() {
	// swap to set dstport as corresponding server.valueupdateserver port
	swap(udp_hdr.srcPort, udp_hdr.dstPort);
}*/

action update_distcache_valueupdate_inswitch_to_distcache_valueupdate_inswitch_origin() {
	modify_field(op_hdr.optype, DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN);
	modify_field(shadowtype_hdr.shadowtype, DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN);

	// NOTE: range/hash_partition_tbl sets eport for DISTCACHE_VALUEUPDATE_INSWITCH to forward it to egress pipeline of server-leaf
	//modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);

	// swap to set dstport as corresponding server.valueupdateserver port to convert DISTCACHE_VALUEUPDATE_INSWITCH as DISTCACHE_VALUEUPDATE_INSWITCH_ACK
	swap(udp_hdr.srcPort, udp_hdr.dstPort);
}

action update_putreq_largevalue_to_distnocache_putreq_largevalue_spine() {
	modify_field(op_hdr.optype, DISTNOCACHE_PUTREQ_LARGEVALUE_SPINE);
	// NOTE: NO shadowtype_hdr for DISTNOCACHE_PUTREQ_LARGEVALUE_SPINE
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter ig_port_forward_counter {
	type : packets_and_bytes;
	direct: ig_port_forward_tbl;
}
#endif

@pragma stage 8
table ig_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		update_getreq_to_getreq_inswitch;
		update_putreq_to_putreq_inswitch;
		update_delreq_to_delreq_inswitch;
#ifdef RANGE_SUPPORT
		update_scanreq_to_scanreq_split;
#endif
		update_warmupreq_to_netcache_warmupreq_inswitch;
		update_loadreq_to_loadreq_spine;
		update_distcache_invalidate_to_distcache_invalidate_inswitch;
		//update_netcache_valueupdate_to_netcache_valueupdate_inswitch;
		//swap_udpport_for_distcache_spine_valueupdate_inswitch;
		update_distcache_valueupdate_inswitch_to_distcache_valueupdate_inswitch_origin;
		update_putreq_largevalue_to_distnocache_putreq_largevalue_spine;
		nop;
	}
	default_action: nop();
	size: 16;
}
