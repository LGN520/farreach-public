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

field_list_calculation random_hash_calc {
	input {
		hash_fields;
	}
	algorithm: random;
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

/*action set_hot_threshold(hot_threshold) {
	modify_field(inswitch_hdr.hot_threshold, hot_threshold);
}

@pragma stage 0
table set_hot_threshold_tbl {
	actions {
		set_hot_threshold;
	}
	default_action: set_hot_threshold(DEFAULT_HH_THRESHOLD);
	size: 1;
}*/

action set_hot_threshold_and_spineswitchnum(hot_threshold, spineswitchnum) {
	modify_field(inswitch_hdr.hot_threshold, hot_threshold);
	modify_field(meta.spineswitchnum, spineswitchnum);
}

@pragma stage 0
table set_hot_threshold_and_spineswitchnum_tbl {
	actions {
		set_hot_threshold_and_spineswitchnum;
	}
	default_action: set_hot_threshold_and_spineswitchnum(DEFAULT_HH_THRESHOLD, DEFAULT_SPINESWITCHNUM);
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

// Stage 1

/*action set_spineswitchnum(spineswitchnum) {
	modify_field(meta.spineswitchnum, spineswitchnum);
}

@pragma stage 1
table set_spineswitchnum_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		set_spineswitchnum;
		nop;
	}
	default_action: nop();
	size: 1;
}*/

/*action reset_is_wrong_pipeline() {
	modify_field(inswitch_hdr.is_wrong_pipeline, 0);
}*/
/*
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
*/

action hash_for_ecmp_and_partition() {
	modify_field_with_hash_based_offset(meta.hashval_for_ecmp, 0, random_hash_calc, PARTITION_COUNT);
#ifndef RANGE_SUPPORT
	modify_field_with_hash_based_offset(meta.hashval_for_partition, 0, hash_calc, PARTITION_COUNT);
#endif
}

@pragma stage 1
table hash_for_ecmp_and_partition_tbl {
	/*reads {
		op_hdr.optype: exact;
	}*/
	actions {
		hash_for_ecmp_and_partition;
		//nop;
	}
	//default_action: nop();
	//size: 16;
	default_action: hash_for_ecmp_and_partition();
	size: 1;
}

// Stage 2

action hash_for_cm12() {
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm1, 0, hash_calc, CM_BUCKET_COUNT);
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm2, 0, hash_calc2, CM_BUCKET_COUNT);
}

@pragma stage 2
table hash_for_cm12_tbl {
	/*reads {
		op_hdr.optype: exact;
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

action set_toleaf_offset(toleaf_offset) {
	modify_field(meta.toleaf_offset, toleaf_offset);
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter ecmp_for_getreq_counter {
	type : packets_and_bytes;
	direct: ecmp_for_getreq_tbl;
}
#endif

@pragma stage 2
table ecmp_for_getreq_tbl {
	reads {
		op_hdr.optype: exact;
		meta.hashval_for_ecmp: range;
	}
	actions {
		set_toleaf_offset;
		nop;
	}
	default_action: nop();
	size: MAX_SPINESWITCH_NUM;
}

// Stage 3

action spineselect(eport, spineswitchidx) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	modify_field(op_hdr.spineswitchidx, spineswitchidx);
}

/*action spineselect_for_getres_server(spineswitchidx) {
	// NOTE: eport will be set by ipv4_forward_tbl for GETRES_SERVER
	modify_field(op_hdr.spineswitchidx, spineswitchidx);
}*/

action spineselect_for_distcache_invalidate(spineswitchidx) {
	// NOTE: eport will be set by range/hash_partition_tbl for DISTCACHE_INVALIDATE
	modify_field(op_hdr.spineswitchidx, spineswitchidx);
}

/*action spineselect_for_netcache_valueupdate(spineswitchidx) {
	// NOTE: eport will be set by range/hash_partition_tbl for NETCACHE_VALUEUPDATE
	modify_field(op_hdr.spineswitchidx, spineswitchidx);
}*/

action spineselect_for_getreq_toleaf(eport, spineswitchidx) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	add(op_hdr.spineswitchidx, spineswitchidx, meta.toleaf_offset); // [1, 2*spineswitchnum-2] <- [0, spineswitchnum-1] + [1, spineswitchnum-1]
}

/*action spineselect_for_distcache_spine_valueupdate_inswitch(eport, spineswitchidx) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	modify_field(op_hdr.spineswitchidx, spineswitchidx);
	bypass_egress(); // directly from ingress to spine
}*/

action spineselect_for_distcache_valueupdate_inswitch(spineswitchidx) {
	// NOTE: eport will be set by range/hash_partition_tbl for DISTCACHE_VALUEUPDATE_INSWITCH
	modify_field(op_hdr.spineswitchidx, spineswitchidx);
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter spineselect_counter {
	type : packets_and_bytes;
	direct: spineselect_tbl;
}
#endif

@pragma stage 3
table spineselect_tbl {
	reads {
		op_hdr.optype: exact;
		meta.toleaf_predicate: exact;
		meta.hashval_for_spineselect: range;
	}
	actions {
		spineselect;
		//spineselect_for_getres_server; // not necessary for server-leaf (we need to hold GETRES_SERVER.spineswitchidx set by server inherited from GETREQ to update corresponding slot in spineload_forclient_reg in client-leaf)
		spineselect_for_distcache_invalidate; // not necessary for server-leaf
		//spineselect_for_netcache_valueupdate; // not necessary for server-leaf
		spineselect_for_getreq_toleaf;
		//spineselect_for_distcache_spine_valueupdate_inswitch;
		spineselect_for_distcache_valueupdate_inswitch; // not necessary for server-leaf
		nop;
	}
	default_action: nop();
	size: SPINESELECT_ENTRY_NUM;
}

// Stage 4~5

//action cutoff_spineswitchidx_for_ecmp(spineswitchnum) {
action cutoff_spineswitchidx_for_ecmp() {
	// NOTE: due to Tofino limitation, the 2nd parameter of subtract_from_field must be a PHV field instead of action parameter
	//subtract_from_field(op_hdr.spineswitchidx, spineswitchnum); // [1, 2*spineswitchnum-2] -> [1, spineswitchnum-1] & [0, spineswitchnum-2]
	subtract_from_field(op_hdr.spineswitchidx, meta.spineswitchnum); // [1, 2*spineswitchnum-2] -> [1, spineswitchnum-1] & [0, spineswitchnum-2]
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter cutoff_spineswitchidx_for_ecmp_counter {
	type : packets_and_bytes;
	direct: cutoff_spineswitchidx_for_ecmp_tbl;
}
#endif

@pragma stage 4
table cutoff_spineswitchidx_for_ecmp_tbl {
	reads {
		op_hdr.optype: exact;
		op_hdr.spineswitchidx: exact;
	}
	actions {
		cutoff_spineswitchidx_for_ecmp;
		nop;
	}
	default_action: nop();
	size: MAX_SPINESWITCH_NUM;
}

action cached_action(idx) {
	modify_field(inswitch_hdr.idx, idx);
	modify_field(inswitch_hdr.is_cached, 1);
}

action uncached_action() {
	modify_field(inswitch_hdr.is_cached, 0);
}

@pragma stage 4 16384
@pragma stage 5
table cache_lookup_tbl {
	reads {
		op_hdr.keylolo: exact;
		op_hdr.keylohi: exact;
		op_hdr.keyhilo: exact;
		//op_hdr.keyhihi: exact;
		op_hdr.keyhihilo: exact;
		op_hdr.keyhihihi: exact;
		op_hdr.leafswitchidx: exact;
	}
	actions {
		cached_action;
		uncached_action;
	}
	default_action: uncached_action();
	size: LOOKUP_ENTRY_COUNT; // egress_pipenum * KV_BUCKET_COUNT
}

// Stage 6~7

action hash_for_cm34() {
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm3, 0, hash_calc3, CM_BUCKET_COUNT);
	//modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_cm4, 0, hash_calc4, CM_BUCKET_COUNT);
}

@pragma stage 6
table hash_for_cm34_tbl {
	/*reads {
		op_hdr.optype: exact;
	}*/
	actions {
		hash_for_cm34;
		//nop;
	}
	//default_action: nop();
	//size: 4;
	default_action: hash_for_cm34();
	size: 4;
}

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
action range_partition_for_distcache_invalidate(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
action range_partition_for_distcache_invalidate_ack(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
/*action range_partition_for_netcache_valueupdate(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
action range_partition_for_netcache_valueupdate_ack(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}*/
/*action range_partition_for_distcache_leaf_valueupdate_inswitch(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
action range_partition_for_distcache_spine_valueupdate_inswitch_ack(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}*/
action range_partition_for_distcache_valueupdate_inswitch(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
action range_partition_for_distcache_valueupdate_inswitch_ack(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
@pragma stage 6 2048
@pragma stage 7
table range_partition_tbl {
	reads {
		op_hdr.optype: exact;
		op_hdr.keyhihihi: range;
		op_hdr.leafswitchidx: exact;
	}
	actions {
		range_partition;
		//reset_is_wrong_pipeline;
		range_partition_for_scan;
		range_partition_for_distcache_invalidate;
		range_partition_for_distcache_invalidate_ack;
		//range_partition_for_netcache_valueupdate;
		//range_partition_for_netcache_valueupdate_ack;
		//range_partition_for_distcache_leaf_valueupdate_inswitch;
		//range_partition_for_distcache_spine_valueupdate_inswitch_ack;
		range_partition_for_distcache_valueupdate_inswitch;
		range_partition_for_distcache_valueupdate_inswitch_ack;
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
action hash_partition_for_distcache_invalidate(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
action hash_partition_for_distcache_invalidate_ack(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
/*action hash_partition_for_netcache_valueupdate(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
action hash_partition_for_netcache_valueupdate_ack(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}*/
/*action hash_partition_for_distcache_leaf_valueupdate_inswitch(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
action hash_partition_for_distcache_spine_valueupdate_inswitch_ack(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}*/
action hash_partition_for_distcache_valueupdate_inswitch(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
action hash_partition_for_distcache_valueupdate_inswitch_ack(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
@pragma stage 6 2048
@pragma stage 7
table hash_partition_tbl {
	reads {
		op_hdr.optype: exact;
		meta.hashval_for_partition: range;
		//ig_intr_md.ingress_port: exact;
	}
	actions {
		hash_partition;
		//reset_is_wrong_pipeline;
		hash_partition_for_distcache_invalidate;
		hash_partition_for_distcache_invalidate_ack;
		//hash_partition_for_netcache_valueupdate;
		//hash_partition_for_netcache_valueupdate_ack;
		//hash_partition_for_distcache_leaf_valueupdate_inswitch;
		//hash_partition_for_distcache_spine_valueupdate_inswitch_ack;
		hash_partition_for_distcache_valueupdate_inswitch;
		hash_partition_for_distcache_valueupdate_inswitch_ack;
		nop;
	}
	//default_action: reset_is_wrong_pipeline();
	default_action: nop();
	size: HASH_PARTITION_ENTRY_NUM;
}
#endif

// Stage 8

#ifdef RANGE_SUPPORT
//action range_partition_for_scan_endkey(last_udpport_plus_one) {
action range_partition_for_scan_endkey(end_globalserveridx_plus_one) {
	modify_field(split_hdr.is_clone, 0);
	modify_field(split_hdr.cur_scanidx, 0);
	//subtract(split_hdr.max_scannum, last_udpport_plus_one, udp_hdr.dstPort);
	subtract(split_hdr.max_scannum, end_globalserveridx_plus_one, split_hdr.globalserveridx);
}

@pragma stage 8
table range_partition_for_scan_endkey_tbl {
	reads {
		op_hdr.optype: exact;
		scan_hdr.keyhihihi: range;
		op_hdr.leafswitchidx: exact;
	}
	actions {
		range_partition_for_scan_endkey;
		nop;
	}
	default_action: nop();
	size: RANGE_PARTITION_FOR_SCAN_ENDKEY_ENTRY_NUM;
}
#endif

// Stage 9

/*action set_client_sid(client_sid, eport) {
	modify_field(inswitch_hdr.client_sid, client_sid);
	// NOTE: eport_for_res and client_sid must be in the same group for ALU access; as compiler aims to place them into the same container, they must come from the same source (action parameter or PHV)
	//modify_field(inswitch_hdr.eport_for_res, ig_intr_md.ingress_port);
	modify_field(inswitch_hdr.eport_for_res, eport);
}*/

// NOTE: eg_intr_md.egress_port is a read-only field (we cannot directly set egress port in egress pipeline even if w/ correct pipeline)
// NOTE: using inswitch_hdr.client_sid for clone_e2e in ALU needs to maintain inswitch_hdr.client_sid and eg_intr_md_for_md.mirror_id into the same group, which violates PHV allocation constraints -> but MAU can access different groups
action set_client_sid_and_hash_for_bf1(client_sid) {
	modify_field(inswitch_hdr.client_sid, client_sid);
	// NOTE: sum of hash results' bits CANNOT > 32-bits in one ALU due to Tofino limitation (18-bit hashval_for_bf = 32-bit cost)
	// NOTE: cannot pass action parameter into modify_field_with_hash_based_offset, which only accepts constant
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_bf1, 0, hash_calc, BF_BUCKET_COUNT);
}

action reset_client_sid() {
	modify_field(inswitch_hdr.client_sid, 0);
}

@pragma stage 9
table prepare_for_cachehit_and_hash_for_bf1_tbl {
	reads {
		op_hdr.optype: exact;
		//ig_intr_md.ingress_port: exact;
		ipv4_hdr.srcAddr: lpm;
	}
	actions {
		set_client_sid_and_hash_for_bf1;
		reset_client_sid;
	}
	default_action: reset_client_sid(); // deprecated: configured as set_client_sid(sids[0]) in ptf
	size: 32;
}

action forward_normal_response(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}

action forward_distcache_invalidate_to_server_and_clone_to_spine(client_sid) {
	// NOTE: eport to server has already been set by partition_tbl for DISTCACHE_INVALIDATE
	//modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port); // Original packet enters the egress pipeline to server
	clone_ingress_pkt_to_egress(client_sid); // Cloned packet enter the egress pipeline to corresponding spine switch
}

/*action forward_netcache_valueupdate_to_server_and_clone_to_spine(client_sid) {
	// NOTE: eport to server has already been set by partition_tbl for NETCACHE_VALUEUPDATE
	//modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port); // Original packet enters the egress pipeline to server
	clone_ingress_pkt_to_egress(client_sid); // Cloned packet enter the egress pipeline to corresponding spine switch
}*/

action forward_distcache_valueupdate_inswitch_to_server_and_clone_to_spine(client_sid) {
	// NOTE: eport to server has already been set by partition_tbl for DISTCACHE_VALUEUPDATE_INSWITCH
	//modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port); // Original packet enters the egress pipeline to server
	clone_ingress_pkt_to_egress(client_sid); // Cloned packet enter the egress pipeline to corresponding spine switch
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter ipv4_forward_counter {
	type : packets_and_bytes;
	direct: ipv4_forward_tbl;
}
#endif

@pragma stage 9
table ipv4_forward_tbl {
	reads {
		op_hdr.optype: exact;
		ipv4_hdr.dstAddr: lpm;
	}
	actions {
		forward_normal_response;
		forward_distcache_invalidate_to_server_and_clone_to_spine;
		//forward_netcache_valueupdate_to_server_and_clone_to_spine;
		forward_distcache_valueupdate_inswitch_to_server_and_clone_to_spine;
		nop;
	}
	default_action: nop();
	size: 256;
}

// Stage 10

action sample() {
	//modify_field_with_hash_based_offset(inswitch_hdr.is_sampled, 0, hash_calc, 2); // WRONG: we should not sample key
	modify_field_rng_uniform(inswitch_hdr.is_sampled, 0, 1); // generate a random value in [0, 1] to sample packet
}

@pragma stage 10
table sample_tbl {
	/*reads {
		op_hdr.optype: exact;
	}*/
	actions {
		sample;
		//nop;
	}
	//default_action: nop();
	//size: 2;
	default_action: sample();
	size: 1;
}

// Stage 11

action update_getreq_spine_to_getreq_inswitch_and_hash_for_bf3() {
	modify_field(op_hdr.optype, GETREQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, GETREQ_INSWITCH);

	// NOTE: sum of hash results' bits CANNOT > 32-bits in one ALU due to Tofino limitation (18-bit hashval_for_bf = 32-bit cost)
	// NOTE: cannot pass action parameter into modify_field_with_hash_based_offset, which only accepts constant
	//modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_bf3, 0, hash_calc3, BF_BUCKET_COUNT);
	
	//add_header(shadowtype_hdr);
	add_header(inswitch_hdr);
	//add_header(switchload_hdr);
}

action update_putreq_seq_to_putreq_seq_inswitch() {
	modify_field(op_hdr.optype, PUTREQ_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_netcache_putreq_seq_cached_to_putreq_seq_inswitch() {
	modify_field(op_hdr.optype, PUTREQ_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_delreq_seq_to_delreq_seq_inswitch() {
	modify_field(op_hdr.optype, DELREQ_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ_INSWITCH);
	add_header(inswitch_hdr);
}

action update_netcache_delreq_seq_cached_to_delreq_seq_inswitch() {
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

action update_loadreq_spine_to_loadreq() {
	modify_field(op_hdr.optype, LOADREQ);
	modify_field(shadowtype_hdr.shadowtype, LOADREQ);
}

action update_loadack_server_to_loadack() {
	modify_field(op_hdr.optype, LOADACK);
}

/*action update_distcache_getres_spine_to_getres() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
}*/

action update_distcache_invalidate_to_distcache_invalidate_inswitch() {
	modify_field(op_hdr.optype, DISTCACHE_INVALIDATE_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, DISTCACHE_INVALIDATE_INSWITCH);
	
	add_header(shadowtype_hdr);
	add_header(inswitch_hdr);
}

/*action update_netcache_valueupdate_to_netcache_valueupdate_inswitch() {
	modify_field(op_hdr.optype, NETCACHE_VALUEUPDATE_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, NETCACHE_VALUEUPDATE_INSWITCH);

	add_header(inswitch_hdr);

	//// NOTE: NETCACHE_VALUEUPDATE does not need partition_tbl, as in-switch record must in the same pipeline of the ingress port, which can also send NETCACHE_VALUEUPDATE_ACK back to corresponding server
	// NOTE: range/hash_partition_tbl sets eport for NETCACHE_VALUEUPDATE to forward it to egress pipeline of storage server; ipv4_forward_tbl clones NETCACHE_VALUEUPDATE to spine switch
	//modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);

	//// swap to set dstport as corresponding server.valueupdateserver port
	// NOTE: spine switch will swap udp.src/dstport to convert NETCACHE_VALUEUPDATE as NETCACHE_VALUEUPDATE_ACK
	//swap(udp_hdr.srcPort, udp_hdr.dstPort);
}*/

/*action swap_udpport_for_distcache_leaf_valueupdate_inswitch() {
	// swap to set dstport as corresponding server.valueupdateserver port
	swap(udp_hdr.srcPort, udp_hdr.dstPort);
}*/

action update_distcache_valueupdate_inswitch_to_distcache_valueupdate_inswitch_origin() {
	modify_field(op_hdr.optype, DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN);
	modify_field(shadowtype_hdr.shadowtype, DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN);

	// NOTE: range/hash_partition_tbl sets eport for DISTCACHE_VALUEUPDATE_INSWITCH to forward it to egress pipeline of storage server; ipv4_forward_tbl clones NETCACHE_VALUEUPDATE to spine switch
	//modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);
}

action update_putreq_largevalue_seq_to_putreq_largevalue_seq_inswitch() {
	// NOTE: PUTREQ_LARGEVALUE_SEQ w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ_INSWITCH w/ op_hdr + shadowtype_hdr + seq_hdr + inswitch_hdr + fraginfo_hdr
	modify_field(op_hdr.optype, PUTREQ_LARGEVALUE_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_LARGEVALUE_SEQ_INSWITCH);

	add_header(inswitch_hdr);
}

action update_putreq_largevalue_seq_cached_to_putreq_largevalue_seq_inswitch() {
	// NOTE: PUTREQ_LARGEVALUE_SEQ_CACHED w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ_INSWITCH w/ op_hdr + shadowtype_hdr + seq_hdr + inswitch_hdr + fraginfo_hdr
	modify_field(op_hdr.optype, PUTREQ_LARGEVALUE_SEQ_INSWITCH);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_LARGEVALUE_SEQ_INSWITCH);

	add_header(inswitch_hdr);
}

action update_getres_largevalue_server_to_getres_largevalue() {
	modify_field(op_hdr.optype, GETRES_LARGEVALUE);
	// NOTE: NO shadowtype_hdr for GETRES_LARGEVALUE
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter ig_port_forward_counter {
	type : packets_and_bytes;
	direct: ig_port_forward_tbl;
}
#endif

@pragma stage 11
table ig_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		update_getreq_spine_to_getreq_inswitch_and_hash_for_bf3;
		update_putreq_seq_to_putreq_seq_inswitch;
		update_netcache_putreq_seq_cached_to_putreq_seq_inswitch;
		update_delreq_seq_to_delreq_seq_inswitch;
		update_netcache_delreq_seq_cached_to_delreq_seq_inswitch;
		update_getres_server_to_getres;
		update_scanres_split_server_to_scanres_split;
		update_putres_server_to_putres;
		update_delres_server_to_delres;
		update_loadreq_spine_to_loadreq;
		update_loadack_server_to_loadack;
		//update_distcache_getres_spine_to_getres;
		update_distcache_invalidate_to_distcache_invalidate_inswitch;
		//update_netcache_valueupdate_to_netcache_valueupdate_inswitch;
		//swap_udpport_for_distcache_leaf_valueupdate_inswitch;
		update_distcache_valueupdate_inswitch_to_distcache_valueupdate_inswitch_origin;
		update_putreq_largevalue_seq_to_putreq_largevalue_seq_inswitch;
		update_putreq_largevalue_seq_cached_to_putreq_largevalue_seq_inswitch;
		update_getres_largevalue_server_to_getres_largevalue;
		nop;
	}
	default_action: nop();
	size: 32;
}
