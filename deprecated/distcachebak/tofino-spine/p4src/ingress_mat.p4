/* Ingress Processing (Normal Operation) */


// Stage 0

action l2l3_forward(bit<9> eport) {
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
	default_action= NoAction();
	size = 16;
}

action set_hot_threshold(bit<16> hot_threshold) {
	hdr.inswitch_hdr.hot_threshold = hot_threshold;
}

@pragma stage 0
table set_hot_threshold_tbl {
	actions = {
		set_hot_threshold;
	}
	default_action= set_hot_threshold(DEFAULT_HH_THRESHOLD);
	size = 1;
}

// Stage 1~2

/*action reset_is_wrong_pipeline() {
	hdr.inswitch_hdr.is_wrong_pipeline = 0;
}*/
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
	}
	actions = {
		hash_for_partition;
		NoAction;
	}
	default_action= NoAction();
	size = 16;
}
#endif

action cached_action(bit<16> idx) {
	hdr.inswitch_hdr.idx = idx;
	hdr.inswitch_hdr.is_cached = 1;
}

action uncached_action() {
	hdr.inswitch_hdr.is_cached = 0;
}

@pragma stage 1 16384
@pragma stage 2
table cache_lookup_tbl {
	key = {
		hdr.op_hdr.keylolo: exact;
		hdr.op_hdr.keylohi: exact;
		hdr.op_hdr.keyhilo: exact;
		//hdr.op_hdr.keyhihi: exact;
		hdr.op_hdr.keyhihilo: exact;
		hdr.op_hdr.keyhihihi: exact;
		hdr.op_hdr.spineswitchidx: exact;
	}
	actions = {
		cached_action;
		uncached_action;
	}
	default_action= uncached_action();
	size = LOOKUP_ENTRY_COUNT; // egress_pipenum * KV_BUCKET_COUNT
}

// Stage 3

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
	}
	actions = {
		hash_for_seq;
		NoAction;
	}
	default_action= NoAction();
	size = 4;
}

// Stage 4~5

action hash_partition(bit<9> eport,bit<16> leafswitchidx) {
	standard_metadata.egress_spec = eport;
	hdr.op_hdr.leafswitchidx = leafswitchidx;
}
action hash_partition_for_distcache_invalidate(bit<9> eport) {
	standard_metadata.egress_spec = eport;
}
/*action hash_partition_for_netcache_valueupdate(bit<9> eport) {
	standard_metadata.egress_spec = eport;
}*/
/*action hash_partition_for_distcache_spine_valueupdate_inswitch(bit<9> eport) {
	standard_metadata.egress_spec = eport;
}*/
action hash_partition_for_distcache_valueupdate_inswitch(bit<9> eport) {
	standard_metadata.egress_spec = eport;
}
@pragma stage 4 2048
@pragma stage 5
table hash_partition_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.hashval_for_partition: range;
	}
	actions = {
		hash_partition;
		hash_partition_for_distcache_invalidate;
		//hash_partition_for_netcache_valueupdate;
		//hash_partition_for_distcache_spine_valueupdate_inswitch;
		hash_partition_for_distcache_valueupdate_inswitch;
		NoAction;
	}
	default_action= NoAction();
	size = HASH_PARTITION_ENTRY_NUM;
}

// Stage 6
// Stage 7

/*action set_client_sid(bit<10> client_sid, eport) {
	hdr.inswitch_hdr.client_sid = client_sid;
	// NOTE: eport_for_res and client_sid must be in the same group for ALU access; as compiler aims to place them into the same container, they must come from the same source (action parameter or PHV)
	//hdr.inswitch_hdr.eport_for_res = ig_intr_md.ingress_port;
	hdr.inswitch_hdr.eport_for_res = eport;
}*/

// NOTE: standard_metadata.egress_port is a read-only field (we cannot directly set egress port in egress pipeline even if w/ correct pipeline)
// NOTE: using hdr.inswitch_hdr.client_sid for clone_e2e in ALU needs to maintain hdr.inswitch_hdr.client_sid and eg_intr_md_for_md.mirror_id into the same group, which violates PHV allocation constraints -> but MAU can access different groups
action set_client_sid(bit<10> client_sid) {
	hdr.inswitch_hdr.client_sid = client_sid;
}

@pragma stage 7
table prepare_for_cachehit_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		//ig_intr_md.ingress_port: exact;
		hdr.ipv4_hdr.srcAddr: lpm;
	}
	actions = {
		set_client_sid;
		NoAction;
	}
	default_action= set_client_sid(0); // deprecated: configured as set_client_sid(sids[0]) in ptf
	size = 32;
}

action forward_normal_response(bit<9> eport) {
	standard_metadata.egress_spec = eport;
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
	key = {
		hdr.op_hdr.optype: exact;
		hdr.ipv4_hdr.dstAddr: lpm;
	}
	actions = {
		forward_normal_response;
		NoAction;
	}
	default_action= NoAction();
	size = 64;
}

// Stage 8

action sample() {

	random(hdr.inswitch_hdr.is_sampled,(bit<1>)0,(bit<1>)1); // generate a random value in [0, 1] to sample packet
}

@pragma stage 8
table sample_tbl {
	key = {
		hdr.op_hdr.optype: exact;
	}
	actions = {
		sample;
		NoAction;
	}
	default_action= NoAction();
	size = 2;
}

action update_getreq_to_getreq_inswitch() {
	hdr.op_hdr.optype = GETREQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = GETREQ_INSWITCH;

	//hdr.shadowtype_hdr.setValid();
	hdr.inswitch_hdr.setValid();
	//hdr.switchload_hdr.setValid();
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


action update_warmupreq_to_netcache_warmupreq_inswitch() {
	hdr.op_hdr.optype = NETCACHE_WARMUPREQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = NETCACHE_WARMUPREQ_INSWITCH;
	hdr.shadowtype_hdr.setValid();
	hdr.inswitch_hdr.setValid();
}

action update_loadreq_to_loadreq_spine() {
	hdr.op_hdr.optype = LOADREQ_SPINE;
	hdr.shadowtype_hdr.shadowtype = LOADREQ_SPINE;
}

action update_distcache_invalidate_to_distcache_invalidate_inswitch() {
	hdr.op_hdr.optype = DISTCACHE_INVALIDATE_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = DISTCACHE_INVALIDATE_INSWITCH;

	hdr.shadowtype_hdr.setValid();
	hdr.inswitch_hdr.setValid();

	// swap to set dstport as corresponding server.invalidateserver port
	bit<16> tmp_port;
	tmp_port = hdr.udp_hdr.srcPort;
	hdr.udp_hdr.srcPort = hdr.udp_hdr.dstPort;
	hdr.udp_hdr.dstPort = tmp_port;
}


action update_distcache_valueupdate_inswitch_to_distcache_valueupdate_inswitch_origin() {
	hdr.op_hdr.optype = DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN;
	hdr.shadowtype_hdr.shadowtype = DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN;

	// NOTE: range/hash_partition_tbl sets eport for DISTCACHE_VALUEUPDATE_INSWITCH to forward it to egress pipeline of server-leaf
	//standard_metadata.egress_spec = ig_intr_md.ingress_port;

	// swap to set dstport as corresponding server.valueupdateserver port to convert DISTCACHE_VALUEUPDATE_INSWITCH as DISTCACHE_VALUEUPDATE_INSWITCH_ACK
	bit<16> tmp_port;
	tmp_port = hdr.udp_hdr.srcPort;
	hdr.udp_hdr.srcPort = hdr.udp_hdr.dstPort;
	hdr.udp_hdr.dstPort = tmp_port;
}

action update_putreq_largevalue_to_putreq_largevalue_inswitch() {
	// NOTE: PUTREQ_LARGEVALUE only w/ op_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr
	hdr.op_hdr.optype = PUTREQ_LARGEVALUE_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_INSWITCH;

	hdr.shadowtype_hdr.setValid();
	hdr.inswitch_hdr.setValid();
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
	key = {
		hdr.op_hdr.optype: exact;
	}
	actions = {
		update_getreq_to_getreq_inswitch;
		update_putreq_to_putreq_inswitch;
		update_delreq_to_delreq_inswitch;
		update_warmupreq_to_netcache_warmupreq_inswitch;
		update_loadreq_to_loadreq_spine;
		update_distcache_invalidate_to_distcache_invalidate_inswitch;
		//update_netcache_valueupdate_to_netcache_valueupdate_inswitch;
		//swap_udpport_for_distcache_spine_valueupdate_inswitch;
		update_distcache_valueupdate_inswitch_to_distcache_valueupdate_inswitch_origin;
		update_putreq_largevalue_to_putreq_largevalue_inswitch;
		NoAction;
	}
	default_action= NoAction();
	size = 16;
}
