/* Ingress Processing (Normal Operation) */

// Stage 0

action l2l3_forward(bit<9>eport) {
	ig_tm_md.ucast_egress_port = eport;
}

@pragma stage 0
table l2l3_forward_tbl {
	key = {
		hdr.ethernet_hdr.dstAddr: exact;
		hdr.ipv4_hdr.dstAddr: lpm;
	}
	actions = {
		l2l3_forward;
		NoAction;
	}
	default_action = NoAction();
	size = 16;
}

action set_need_recirculate(bit<16> eport) {
	meta.need_recirculate = 1;
	meta.recirport = eport;
	// ???
	// meta.need_recirculate = 0;
}

action reset_need_recirculate() {
	meta.need_recirculate = 0;
}

@pragma stage 0
table need_recirculate_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		ig_intr_md.ingress_port: exact;
	}
	actions = {
		set_need_recirculate;
		reset_need_recirculate;
	}
	default_action = reset_need_recirculate();
	size = 8;
}

action set_hot_threshold(bit<16> hot_threshold) {
	hdr.inswitch_hdr.hot_threshold = hot_threshold;
}

@pragma stage 0
table set_hot_threshold_tbl {
	actions = {
		set_hot_threshold;
	}
	default_action = set_hot_threshold(DEFAULT_HH_THRESHOLD);
	size = 1;
}

// Stage 1 (need_recirculate = 1)


// NOTE: as our Tofino does not support cross-ingress-pipeline recirculation, we use hardware link to simluate it
action recirculate_pkt() {
	ig_tm_md.ucast_egress_port = (bit<9>) meta.recirport;
	// bypass_egress();
	ig_tm_md.bypass_egress = 1;
	// ???
}

@pragma stage 1
table recirculate_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions = {
		recirculate_pkt;
		NoAction;
	}
	default_action = NoAction();
	size = 16;
}

// Stage 1 (need_recirculate = 0)


#ifndef RANGE_SUPPORT
Hash<bit<15>>(HashAlgorithm_t.CRC32) hash_partition_calc;
action hash_for_partition() {
	meta.hashval_for_partition[14:0] = hash_partition_calc.get({
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	});
}
@pragma stage 1
table hash_for_partition_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions = {
		hash_for_partition;
		NoAction;
	}
	default_action = NoAction();
	size = 16;
}
#endif

// Stage 2



action hash_partition(bit<16> udpport,bit<9> eport) {
	hdr.udp_hdr.dstPort = udpport;
	ig_tm_md.ucast_egress_port = eport;
}
action hash_partition_for_special_response(bit<9> eport) {
	ig_tm_md.ucast_egress_port = eport;
}
@pragma stage 2
table hash_partition_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.hashval_for_partition: range;
		//ig_intr_md.ingress_port: exact;
		meta.need_recirculate: exact;
	}
	actions = {
		hash_partition;
		//reset_is_wrong_pipeline;
		hash_partition_for_special_response;
		NoAction;
	}
	//default_action = reset_is_wrong_pipeline();
	default_action = NoAction();
	size = HASH_PARTITION_ENTRY_NUM;
}


Hash<bit<16>>(HashAlgorithm_t.CRC32) hash_cm1_calc;
Hash<bit<16>>(HashAlgorithm_t.CRC16) hash_cm2_calc;
action hash_for_cm12() {
	hdr.inswitch_hdr.hashval_for_cm1 = hash_cm1_calc.get({
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	});
	hdr.inswitch_hdr.hashval_for_cm2 =  hash_cm2_calc.get({
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	});
}

@pragma stage 2
table hash_for_cm12_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions = {
		hash_for_cm12;
		NoAction;
	}
	default_action = NoAction();
	size = 2;
}

#ifndef USE_BFSDE920
action cached_action(bit<16> idx) {
	hdr.inswitch_hdr.idx = idx;
	hdr.inswitch_hdr.is_cached = 1;
}

action uncached_action() {
	hdr.inswitch_hdr.is_cached = 0;
}

// @pragma stage 2 16384
@pragma stage 3
table cache_lookup_tbl {
	key = {
		hdr.op_hdr.keylolo: exact;
		hdr.op_hdr.keylohi: exact;
		hdr.op_hdr.keyhilo: exact;
		
		hdr.op_hdr.keyhihilo: exact;
		hdr.op_hdr.keyhihihi: exact;
		meta.need_recirculate: exact;
	}
	actions = {
		cached_action;
		uncached_action;
	}
	default_action = uncached_action();
	size = LOOKUP_ENTRY_COUNT; // egress_pipenum * KV_BUCKET_COUNT
}
#endif

// Stage 3

#ifdef USE_BFSDE920
action cached_action(bit<16> idx) {
	hdr.inswitch_hdr.idx = idx;
	hdr.inswitch_hdr.is_cached = 1;
}

action uncached_action() {
	hdr.inswitch_hdr.is_cached = 0;
}

@pragma stage 3
table cache_lookup_tbl {
	key = {
		hdr.op_hdr.keylolo: exact;
		hdr.op_hdr.keylohi: exact;
		hdr.op_hdr.keyhilo: exact;
		
		hdr.op_hdr.keyhihilo: exact;
		hdr.op_hdr.keyhihihi: exact;
		meta.need_recirculate: exact;
	}
	actions = {
		cached_action;
		uncached_action;
	}
	default_action = uncached_action();
	size = LOOKUP_ENTRY_COUNT; // egress_pipenum * KV_BUCKET_COUNT
}
#endif


action hash_for_seq() {
	hdr.inswitch_hdr.hashval_for_seq = meta.hashval_for_partition;
}
@pragma stage 4
table hash_for_seq_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions = {
		hash_for_seq;
		NoAction;
	}
	default_action = NoAction();
	size = 4;
}

// Stage 4

Hash<bit<16>>(HashAlgorithm_t.IDENTITY) hash_cm3_calc;
Hash<bit<16>>(HashAlgorithm_t.CRC16) hash_cm4_calc;
action hash_for_cm34() {
	hdr.inswitch_hdr.hashval_for_cm3 = hash_cm3_calc.get({
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	});
	hdr.inswitch_hdr.hashval_for_cm4 =  hash_cm4_calc.get({
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	});
}
@pragma stage 4
table hash_for_cm34_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions = {
		hash_for_cm34;
		NoAction;
	}
	default_action = NoAction();
	size = 2;
}

action set_snapshot_flag(bit<32> snapshotid) {
	hdr.inswitch_hdr.snapshot_flag = 1;
	//seq_hdr.snapshot_token = snapshotid;
	hdr.inswitch_hdr.snapshot_token = snapshotid;
}

action reset_snapshot_flag() {
	hdr.inswitch_hdr.snapshot_flag = 0;
	//seq_hdr.snapshot_token = 0;
	hdr.inswitch_hdr.snapshot_token = 0;
}

@pragma stage 4
table snapshot_flag_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions = {
		set_snapshot_flag;
		reset_snapshot_flag;
	}
	default_action = reset_snapshot_flag();
	size = 8;
}

// Stage 5



// NOTE: standard_metadata.engress_port is a read-only field (we cannot directly set egress port in egress pipeline even if w/ correct pipeline)
// NOTE: using inswitch_hdr.client_sid for clone_e2e in ALU needs to maintain inswitch_hdr.client_sid and eg_intr_md_for_md.mirror_id into the same group, which violates PHV allocation constraints -> but MAU can access different groups
action set_client_sid(bit<10> client_sid) {
	hdr.inswitch_hdr.client_sid = client_sid;
}

@pragma stage 10
table prepare_for_cachehit_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		//ig_intr_md.ingress_port: exact;
		hdr.ipv4_hdr.srcAddr: lpm;
		meta.need_recirculate: exact;
	}
	actions = {
		set_client_sid;
		NoAction;
	}
	default_action = set_client_sid(0); // deprecated: configured as set_client_sid(sids[0]) in ptf
	size = 32;
}

action forward_normal_response(bit<9> eport) {
	ig_tm_md.ucast_egress_port = eport;
}

#define clone_i2e(mir_ses)                      \
    ig_dprsr_md.mirror_type = MIRROR_TYPE_I2E;  \
	meta.igr_mir_ses = ##mir_ses;  				\
	meta.pkt_type = PKT_TYPE_MIRROR;		
// action clone_i2e(MirrorId_t mir_ses){
// 	ig_dprsr_md.mirror_type = MIRROR_TYPE_I2E;
// 	meta.igr_mir_ses = mir_ses;
// }
action forward_special_get_response(bit<10> client_sid) {
	// NOTE: eport to server has already been set by partition_tbl for GETRES_LATEST/DELETED_SEQ
	//ig_tm_md.ucast_egress_port = ig_intr_md.ingress_port; // Original packet enters the egress pipeline to server
	// ig_dprsr_md.drop_ctl=1;
	clone_i2e(client_sid); 
	// clone_ingress_pkt_to_egress(client_sid); // Cloned packet enter the egress pipeline to corresponding client
}

@pragma stage 10
table ipv4_forward_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.ipv4_hdr.dstAddr: lpm;
		meta.need_recirculate: exact;
	}
	actions = {
		forward_normal_response;
		forward_special_get_response;
		NoAction;
	}
	default_action = NoAction();
	size = 128;
}

// Stage 6

Random<bit<1>>() rnd1;
action sample() {
	//modify_field_with_hash_based_offset(hdr.inswitch_hdr.is_sampled, 0, hash_calc, 2); // WRONG: we should not sample key
	// modify_field_rng_uniform(hdr.inswitch_hdr.is_sampled, 0, 1); // generate a random value in [0, 1] to sample packet
	// random(hdr.inswitch_hdr.is_sampled,(bit<1>)0,(bit<1>)1);
	hdr.inswitch_hdr.is_sampled = rnd1.get();
}
@pragma stage 11
table sample_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions = {
		sample;
		NoAction;
	}
	default_action = NoAction();
	size = 2;
}

action update_getreq_to_getreq_inswitch() {
	hdr.op_hdr.optype = GETREQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = GETREQ_INSWITCH;
	hdr.shadowtype_hdr.setValid();
	hdr.inswitch_hdr.setValid();
}

action update_getres_latest_seq_to_getres_latest_seq_inswitch() {
	hdr.op_hdr.optype = GETRES_LATEST_SEQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = GETRES_LATEST_SEQ_INSWITCH;
	hdr.inswitch_hdr.setValid();
}

action update_getres_deleted_seq_to_getres_deleted_seq_inswitch() {
	hdr.op_hdr.optype = GETRES_DELETED_SEQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = GETRES_DELETED_SEQ_INSWITCH;
	hdr.inswitch_hdr.setValid();
}

action update_putreq_to_putreq_inswitch() {
	hdr.op_hdr.optype = PUTREQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_INSWITCH;
	hdr.shadowtype_hdr.setValid();
	hdr.inswitch_hdr.setValid();
}

action update_delreq_to_delreq_inswitch() {
	hdr.op_hdr.optype = DELREQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = DELREQ_INSWITCH;
	hdr.shadowtype_hdr.setValid();
	hdr.inswitch_hdr.setValid();
}

action update_putreq_largevalue_to_putreq_largevalue_inswitch() {
	// NOTE: PUTREQ_LARGEVALUE only w/ op_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr
	hdr.op_hdr.optype = PUTREQ_LARGEVALUE_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_INSWITCH;

	hdr.shadowtype_hdr.setValid();
	hdr.inswitch_hdr.setValid();
}


@pragma stage 11
table ig_port_forward_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.need_recirculate: exact;
	}
	actions = {
		update_getreq_to_getreq_inswitch;
		update_getres_latest_seq_to_getres_latest_seq_inswitch;
		update_getres_deleted_seq_to_getres_deleted_seq_inswitch;
		update_putreq_to_putreq_inswitch;
		update_delreq_to_delreq_inswitch;

		update_putreq_largevalue_to_putreq_largevalue_inswitch;
		NoAction;
	}
	default_action = NoAction();
	size = 8;
}


