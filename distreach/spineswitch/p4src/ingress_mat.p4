/* Ingress Processing (Normal Operation) */

// Stage 0

action l2l3_forward(bit<9>eport) {
	standard_metadata.egress_spec = eport;
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
	// meta.need_recirculate = 1;
	// meta.recirport = eport;
	// ???
	meta.need_recirculate = 0;
}

action reset_need_recirculate() {
	meta.need_recirculate = 0;
}

@pragma stage 0
table need_recirculate_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		standard_metadata.ingress_port: exact;
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
	// standard_metadata.egress_spec = (bit<9>) meta.recirport;
	// bypass_egress();
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
action hash_for_partition() {
	hash(meta.hashval_for_partition, HashAlgorithm.crc32, (bit<32>)0, {
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	}, (bit<32>) PARTITION_COUNT);
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
	standard_metadata.egress_spec = eport;
}
action hash_partition_for_special_response(bit<9> eport) {
	standard_metadata.egress_spec = eport;
}
@pragma stage 2
table hash_partition_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.hashval_for_partition: range;
		meta.need_recirculate: exact;
	}
	actions = {
		hash_partition;
		hash_partition_for_special_response;
		NoAction;
	}

	default_action = NoAction();
	size = HASH_PARTITION_ENTRY_NUM;
}
// Stage 3

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

action hash_for_seq() {
	hash(hdr.inswitch_hdr.hashval_for_seq, HashAlgorithm.crc32, (bit<32>)0, {
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	}, (bit<32>) SEQ_BUCKET_COUNT);
}

@pragma stage 3
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

@pragma stage 5
table prepare_for_cachehit_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		//standard_metadata.ingress_port: exact;
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
	standard_metadata.egress_spec = eport;
}

action forward_special_get_response(bit<10> client_sid) {
	// NOTE: eport to server has already been set by partition_tbl for GETRES_LATEST/DELETED_SEQ
	//standard_metadata.egress_spec = standard_metadata.ingress_port; // Original packet enters the egress pipeline to server
	clone(CloneType.I2E, (bit<32>)client_sid); 
	// clone_ingress_pkt_to_egress(client_sid); // Cloned packet enter the egress pipeline to corresponding client
}

@pragma stage 5
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

action sample() {
	random(hdr.inswitch_hdr.is_sampled,(bit<1>)0,(bit<1>)1);; // generate a random value in [0, 1] to sample packet
}

@pragma stage 6
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

@pragma stage 6
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

action update_cache_pop_inswitch_to_cache_pop_inswitch_forward(){
	hdr.op_hdr.optype = CACHE_POP_INSWITCH_FORWARD;
}
action update_cache_pop_inswitch_forward_to_cache_pop_inswitch(){
	hdr.op_hdr.optype = CACHE_POP_INSWITCH;
	standard_metadata.egress_spec = standard_metadata.ingress_port;
}
action update_setvalid_inswitch_to_setvalid_inswitch_forward(){
	hdr.op_hdr.optype = SETVALID_INSWITCH_FORWARD;
}
action update_setvalid_inswitch_forward_to_setvalid_inswitch(){
	hdr.op_hdr.optype = SETVALID_INSWITCH;
	standard_metadata.egress_spec = standard_metadata.ingress_port;
}
action update_cache_evict_inswitch_to_cache_evict_inswitch_forward(){
	hdr.op_hdr.optype = CACHE_EVICT_FORWARD;
}
action update_cache_evict_inswitch_forward_to_cache_evict_inswitch(){
	hdr.op_hdr.optype = CACHE_EVICT;
	standard_metadata.egress_spec = standard_metadata.ingress_port;
}
@pragma stage 6
table cache_pop_ig_port_forward_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		standard_metadata.ingress_port:exact;
	}
	actions = {
		update_cache_pop_inswitch_to_cache_pop_inswitch_forward;
		update_cache_pop_inswitch_forward_to_cache_pop_inswitch;
		update_setvalid_inswitch_to_setvalid_inswitch_forward;
		update_setvalid_inswitch_forward_to_setvalid_inswitch;
		update_cache_evict_inswitch_to_cache_evict_inswitch_forward;
		update_cache_evict_inswitch_forward_to_cache_evict_inswitch;
		NoAction;
	}
	default_action = NoAction();
	size = 8;
}



