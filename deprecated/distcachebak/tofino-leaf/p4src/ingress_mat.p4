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


action set_hot_threshold_and_spineswitchnum(bit<16> hot_threshold, bit<16> spineswitchnum) {
	hdr.inswitch_hdr.hot_threshold = hot_threshold;
	meta.spineswitchnum = spineswitchnum;
}

@pragma stage 0
table set_hot_threshold_and_spineswitchnum_tbl {
	actions = {
		set_hot_threshold_and_spineswitchnum;
	}
	default_action= set_hot_threshold_and_spineswitchnum(DEFAULT_HH_THRESHOLD, DEFAULT_SPINESWITCHNUM);
	size = 1;
}


action hash_for_spineselect() {
	// NOTE: we use a different hash function to simulate independent hashing
	bit<16> tmp;
	hash(tmp, HashAlgorithm.identity, (bit<16>)0, {
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	}, (bit<32>)PARTITION_COUNT);
	meta.hashval_for_spineselect = (bit<16>)tmp;
}

@pragma stage 0
table hash_for_spineselect_tbl {
	/*key = {
		hdr.op_hdr.optype: exact;
	}*/
	actions = {
		hash_for_spineselect;
		//NoAction;
	}
	//default_action= NoAction();
	//size = 16;
	default_action= hash_for_spineselect();
	size = 1;
}

// Stage 1

action hash_for_ecmp_and_partition() {
	hash(meta.hashval_for_ecmp, HashAlgorithm.identity, (bit<32>)0, {
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	}, (bit<32>) PARTITION_COUNT);
	// modify_field_with_hash_based_offset(meta.hashval_for_ecmp, 0, random_hash_calc, PARTITION_COUNT);
#ifndef RANGE_SUPPORT
	hash(meta.hashval_for_partition, HashAlgorithm.crc32, (bit<32>)0, {
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	}, (bit<32>) PARTITION_COUNT);
#endif
}

@pragma stage 1
table hash_for_ecmp_and_partition_tbl {
	/*key = {
		hdr.op_hdr.optype: exact;
	}*/
	actions = {
		hash_for_ecmp_and_partition;
		//NoAction;
	}
	//default_action= NoAction();
	//size = 16;
	default_action= hash_for_ecmp_and_partition();
	size = 1;
}

// Stage 2
bit<32>  tmp;
action hash_for_cm12() {
	// modify_field_with_hash_based_offset(hdr.inswitch_hdr.hashval_for_cm1, 0, hash_calc, CM_BUCKET_COUNT);
	
	hash(tmp, HashAlgorithm.crc32, (bit<32>)0, {
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	}, (bit<32>) CM_BUCKET_COUNT);
	hdr.inswitch_hdr.hashval_for_cm1 = (bit<16>) tmp;
	// modify_field_with_hash_based_offset(hdr.inswitch_hdr.hashval_for_cm2, 0, hash_calc2, CM_BUCKET_COUNT);
	hash(tmp, HashAlgorithm.crc32_custom, (bit<32>)0, {
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	}, (bit<32>)CM_BUCKET_COUNT);
	hdr.inswitch_hdr.hashval_for_cm2 = (bit<16>) tmp;
}


@pragma stage 2
table hash_for_cm12_tbl {
	/*key = {
		hdr.op_hdr.optype: exact;
	}*/
	actions = {
		hash_for_cm12;
		//NoAction;
	}
	//default_action= NoAction();
	//size = 4;
	default_action= hash_for_cm12();
	size = 1;
}

action set_toleaf_offset(bit<16> toleaf_offset) {
	meta.toleaf_offset = toleaf_offset;
}

@pragma stage 2
table ecmp_for_getreq_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.hashval_for_ecmp: range;
	}
	actions = {
		set_toleaf_offset;
		NoAction;
	}
	default_action= NoAction();
	size = MAX_SPINESWITCH_NUM;
}

// Stage 3

action spineselect(bit<9> eport, bit<16> spineswitchidx) {
	standard_metadata.egress_spec = eport;
	hdr.op_hdr.spineswitchidx = spineswitchidx;
}

/*action spineselect_for_getres_server(bit<16> spineswitchidx) {
	// NOTE: eport will be set by ipv4_forward_tbl for GETRES_SERVER
	hdr.op_hdr.spineswitchidx = spineswitchidx;
}*/

action spineselect_for_distcache_invalidate(bit<16> spineswitchidx) {
	// NOTE: eport will be set by range/hash_partition_tbl for DISTCACHE_INVALIDATE
	hdr.op_hdr.spineswitchidx = spineswitchidx;
}

/*action spineselect_for_netcache_valueupdate(bit<16> spineswitchidx) {
	// NOTE: eport will be set by range/hash_partition_tbl for NETCACHE_VALUEUPDATE
	hdr.op_hdr.spineswitchidx = spineswitchidx;
}*/

action spineselect_for_getreq_toleaf(bit<9> eport, bit<16> spineswitchidx) {
	standard_metadata.egress_spec = eport;
	hdr.op_hdr.spineswitchidx = spineswitchidx + meta.toleaf_offset; // [1, 2*spineswitchnum-2] <- [0, bit<16> spineswitchnum-1] + [1, bit<16> spineswitchnum-1]
}

/*action spineselect_for_distcache_spine_valueupdate_inswitch(bit<9> eport, bit<16> spineswitchidx) {
	standard_metadata.egress_spec = eport;
	hdr.op_hdr.spineswitchidx = spineswitchidx;
	bypass_egress(); // directly from ingress to spine
}*/

action spineselect_for_distcache_valueupdate_inswitch(bit<16> spineswitchidx) {
	// NOTE: eport will be set by range/hash_partition_tbl for DISTCACHE_VALUEUPDATE_INSWITCH
	hdr.op_hdr.spineswitchidx = spineswitchidx;
}

@pragma stage 3
table spineselect_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.toleaf_predicate: exact;
		meta.hashval_for_spineselect: range;
	}
	actions = {
		spineselect;
		//spineselect_for_getres_server; // not necessary for server-leaf (we need to hold GETRES_SERVER.spineswitchidx set by server inherited from GETREQ to update corresponding slot in spineload_forclient_reg in client-leaf)
		spineselect_for_distcache_invalidate; // not necessary for server-leaf
		//spineselect_for_netcache_valueupdate; // not necessary for server-leaf
		spineselect_for_getreq_toleaf;
		//spineselect_for_distcache_spine_valueupdate_inswitch;
		spineselect_for_distcache_valueupdate_inswitch; // not necessary for server-leaf
		NoAction;
	}
	default_action= NoAction();
	size = SPINESELECT_ENTRY_NUM;
}

// Stage 4~5

//action cutoff_spineswitchidx_for_ecmp(spineswitchnum) {
action cutoff_spineswitchidx_for_ecmp() {
	// NOTE: due to Tofino limitation, the 2nd parameter of subtract_from_field must be a PHV field instead of action parameter
	//hdr.op_hdr.spineswitchidx = hdr.op_hdr.spineswitchidx -  bit<16> spineswitchnum; // [1, 2*spineswitchnum-2] -> [1, bit<16> spineswitchnum-1] & [0, bit<16> spineswitchnum-2]
	hdr.op_hdr.spineswitchidx = hdr.op_hdr.spineswitchidx -  meta.spineswitchnum; // [1, 2*spineswitchnum-2] -> [1, bit<16> spineswitchnum-1] & [0, bit<16> spineswitchnum-2]
}


@pragma stage 4
table cutoff_spineswitchidx_for_ecmp_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.op_hdr.spineswitchidx: exact;
	}
	actions = {
		cutoff_spineswitchidx_for_ecmp;
		NoAction;
	}
	default_action= NoAction();
	size = MAX_SPINESWITCH_NUM;
}

action cached_action(bit<16> idx) {
	hdr.inswitch_hdr.idx = idx;
	hdr.inswitch_hdr.is_cached = 1;
}

action uncached_action() {
	hdr.inswitch_hdr.is_cached = 0;
}

@pragma stage 4 16384
@pragma stage 5
table cache_lookup_tbl {
	key = {
		hdr.op_hdr.keylolo: exact;
		hdr.op_hdr.keylohi: exact;
		hdr.op_hdr.keyhilo: exact;
		//hdr.op_hdr.keyhihi: exact;
		hdr.op_hdr.keyhihilo: exact;
		hdr.op_hdr.keyhihihi: exact;
		hdr.op_hdr.leafswitchidx: exact;
	}
	actions = {
		cached_action;
		uncached_action;
	}
	default_action= uncached_action();
	size = LOOKUP_ENTRY_COUNT; // egress_pipenum * KV_BUCKET_COUNT
}

// Stage 6~7

action hash_for_cm34() {
	hash(tmp, HashAlgorithm.identity, (bit<32>)0, {
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	}, (bit<32>)CM_BUCKET_COUNT);
	hdr.inswitch_hdr.hashval_for_cm3 = (bit<16>) tmp;
	hash(tmp, HashAlgorithm.csum16, (bit<32>)0, {
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	}, (bit<32>)CM_BUCKET_COUNT);
	hdr.inswitch_hdr.hashval_for_cm4 = (bit<16>) tmp;
}

@pragma stage 6
table hash_for_cm34_tbl {
	/*key = {
		hdr.op_hdr.optype: exact;
	}*/
	actions = {
		hash_for_cm34;
		//NoAction;
	}
	//default_action= NoAction();
	//size = 4;
	default_action= hash_for_cm34();
	size = 1;
}

action hash_partition(bit<16> udpport,bit<9> eport) {
	hdr.udp_hdr.dstPort = udpport;
	standard_metadata.egress_spec = eport;
}
action hash_partition_for_distcache_invalidate(bit<9> eport) {
	standard_metadata.egress_spec = eport;
}
action hash_partition_for_distcache_invalidate_ack(bit<9> eport) {
	standard_metadata.egress_spec = eport;
}
action hash_partition_for_distcache_valueupdate_inswitch(bit<9> eport) {
	standard_metadata.egress_spec = eport;
}
action hash_partition_for_distcache_valueupdate_inswitch_ack(bit<9> eport) {
	standard_metadata.egress_spec = eport;
}
@pragma stage 6 2048
@pragma stage 7
table hash_partition_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.hashval_for_partition: range;
		//ig_intr_md.ingress_port: exact;
	}
	actions = {
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
		NoAction;
	}
	//default_action= reset_is_wrong_pipeline();
	default_action= NoAction();
	size = HASH_PARTITION_ENTRY_NUM;
}

// Stage 8

// Stage 9

// NOTE: standard_metadata.egress_port is a read-only field (we cannot directly set egress port in egress pipeline even if w/ correct pipeline)
// NOTE: using hdr.inswitch_hdr.client_sid for clone_e2e in ALU needs to maintain hdr.inswitch_hdr.client_sid and eg_intr_md_for_md.mirror_id into the same group, which violates PHV allocation constraints -> but MAU can access different groups
action set_client_sid_and_hash_for_bf1(bit<10> client_sid) {
	hdr.inswitch_hdr.client_sid = client_sid;
	// NOTE: sum of hash results' bits CANNOT > 32-bits in one ALU due to Tofino limitation (18-bit hashval_for_bf = 32-bit cost)
	// NOTE: cannot pass action parameter into modify_field_with_hash_based_offset, which only accepts constant
	hash(hdr.inswitch_hdr.hashval_for_bf1, HashAlgorithm.crc32, (bit<18>)0, {
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	}, (bit<32>)BF_BUCKET_COUNT);
}

action reset_client_sid() {
	hdr.inswitch_hdr.client_sid = 0;
}

@pragma stage 9
table prepare_for_cachehit_and_hash_for_bf1_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		//ig_intr_md.ingress_port: exact;
		hdr.ipv4_hdr.srcAddr: lpm;
	}
	actions = {
		set_client_sid_and_hash_for_bf1;
		reset_client_sid;
	}
	default_action= reset_client_sid(); // deprecated: configured as set_client_sid(sids[0]) in ptf
	size = 32;
}

action forward_normal_response(bit<9> eport) {
	standard_metadata.egress_spec = eport;
}

action forward_distcache_invalidate_to_server_and_clone_to_spine(bit<10> client_sid) {
	// NOTE: eport to server has already been set by partition_tbl for DISTCACHE_INVALIDATE
	//standard_metadata.egress_spec = ig_intr_md.ingress_port // Original packet enters the egress pipeline to server;
	clone(CloneType.I2E, (bit<32>)client_sid); // Cloned packet enter the egress pipeline to corresponding spine switch
}

/*action forward_netcache_valueupdate_to_server_and_clone_to_spine(bit<10> client_sid) {
	// NOTE: eport to server has already been set by partition_tbl for NETCACHE_VALUEUPDATE
	//standard_metadata.egress_spec = ig_intr_md.ingress_port // Original packet enters the egress pipeline to server;
	clone(CloneType.I2E, (bit<32>)client_sid); // Cloned packet enter the egress pipeline to corresponding spine switch
}*/

action forward_distcache_valueupdate_inswitch_to_server_and_clone_to_spine(bit<10> client_sid) {
	// NOTE: eport to server has already been set by partition_tbl for DISTCACHE_VALUEUPDATE_INSWITCH
	//standard_metadata.egress_spec = ig_intr_md.ingress_port // Original packet enters the egress pipeline to server;
	clone(CloneType.I2E, (bit<32>)client_sid); // Cloned packet enter the egress pipeline to corresponding spine switch
}


@pragma stage 9
table ipv4_forward_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.ipv4_hdr.dstAddr: lpm;
	}
	actions = {
		forward_normal_response;
		forward_distcache_invalidate_to_server_and_clone_to_spine;
		//forward_netcache_valueupdate_to_server_and_clone_to_spine;
		forward_distcache_valueupdate_inswitch_to_server_and_clone_to_spine;
		NoAction;
	}
	default_action= NoAction();
	size = 256;
}

// Stage 10

action sample() {
// generate a random value in [0, 1] to sample packet
	random(hdr.inswitch_hdr.is_sampled,(bit<1>)0,(bit<1>)1);
}

@pragma stage 10
table sample_tbl {
	/*key = {
		hdr.op_hdr.optype: exact;
	}*/
	actions = {
		sample;
		//NoAction;
	}
	//default_action= NoAction();
	//size = 2;
	default_action= sample();
	size = 1;
}

// Stage 11

action update_getreq_spine_to_getreq_inswitch_and_hash_for_bf3() {
	hdr.op_hdr.optype = GETREQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = GETREQ_INSWITCH;

	// NOTE: sum of hash results' bits CANNOT > 32-bits in one ALU due to Tofino limitation (18-bit hashval_for_bf = 32-bit cost)
	// NOTE: cannot pass action parameter into modify_field_with_hash_based_offset, which only accepts constant
	//modify_field_with_hash_based_offset(hdr.inswitch_hdr.hashval_for_bf3, 0, hash_calc3, BF_BUCKET_COUNT);
	
	//hdr.shadowtype_hdr.setValid();
	hdr.inswitch_hdr.setValid();
	//hdr.switchload_hdr.setValid();
}

action update_putreq_seq_to_putreq_seq_inswitch() {
	hdr.op_hdr.optype = PUTREQ_SEQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_SEQ_INSWITCH;
	hdr.inswitch_hdr.setValid();
}

action update_netcache_putreq_seq_cached_to_putreq_seq_inswitch() {
	hdr.op_hdr.optype = PUTREQ_SEQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_SEQ_INSWITCH;
	hdr.inswitch_hdr.setValid();
}

action update_delreq_seq_to_delreq_seq_inswitch() {
	hdr.op_hdr.optype = DELREQ_SEQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = DELREQ_SEQ_INSWITCH;
	hdr.inswitch_hdr.setValid();
}

action update_netcache_delreq_seq_cached_to_delreq_seq_inswitch() {
	hdr.op_hdr.optype = DELREQ_SEQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = DELREQ_SEQ_INSWITCH;
	hdr.inswitch_hdr.setValid();
}

action update_getres_server_to_getres() {
	hdr.op_hdr.optype = GETRES;
	hdr.shadowtype_hdr.shadowtype = GETRES;
}

action update_scanres_split_server_to_scanres_split() {
	hdr.op_hdr.optype = SCANRES_SPLIT;
}

action update_putres_server_to_putres() {
	hdr.op_hdr.optype = PUTRES;
	hdr.shadowtype_hdr.shadowtype = PUTRES;
}

action update_delres_server_to_delres() {
	hdr.op_hdr.optype = DELRES;
	hdr.shadowtype_hdr.shadowtype = DELRES;
}

action update_loadreq_spine_to_loadreq() {
	hdr.op_hdr.optype = LOADREQ;
	hdr.shadowtype_hdr.shadowtype = LOADREQ;
}

action update_loadack_server_to_loadack() {
	hdr.op_hdr.optype = LOADACK;
}

/*action update_distcache_getres_spine_to_getres() {
	hdr.op_hdr.optype = GETRES;
	hdr.shadowtype_hdr.shadowtype = GETRES;
}*/

action update_distcache_invalidate_to_distcache_invalidate_inswitch() {
	hdr.op_hdr.optype = DISTCACHE_INVALIDATE_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = DISTCACHE_INVALIDATE_INSWITCH;
	
	hdr.shadowtype_hdr.setValid();
	hdr.inswitch_hdr.setValid();
}

action update_distcache_valueupdate_inswitch_to_distcache_valueupdate_inswitch_origin() {
	hdr.op_hdr.optype = DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN;
	hdr.shadowtype_hdr.shadowtype = DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN;

	// NOTE: range/hash_partition_tbl sets eport for DISTCACHE_VALUEUPDATE_INSWITCH to forward it to egress pipeline of storage server; ipv4_forward_tbl clones NETCACHE_VALUEUPDATE to spine switch
	//standard_metadata.egress_spec = ig_intr_md.ingress_port;
}

action update_putreq_largevalue_seq_to_putreq_largevalue_seq_inswitch() {
	// NOTE: PUTREQ_LARGEVALUE_SEQ w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ_INSWITCH w/ op_hdr + shadowtype_hdr + seq_hdr + inswitch_hdr + fraginfo_hdr
	hdr.op_hdr.optype = PUTREQ_LARGEVALUE_SEQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_SEQ_INSWITCH;

	hdr.inswitch_hdr.setValid();
}

action update_putreq_largevalue_seq_cached_to_putreq_largevalue_seq_inswitch() {
	// NOTE: PUTREQ_LARGEVALUE_SEQ_CACHED w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ_INSWITCH w/ op_hdr + shadowtype_hdr + seq_hdr + inswitch_hdr + fraginfo_hdr
	hdr.op_hdr.optype = PUTREQ_LARGEVALUE_SEQ_INSWITCH;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_SEQ_INSWITCH;

	hdr.inswitch_hdr.setValid();
}

action update_getres_largevalue_server_to_getres_largevalue() {
	hdr.op_hdr.optype = GETRES_LARGEVALUE;
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
	key = {
		hdr.op_hdr.optype: exact;
	}
	actions = {
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
		NoAction;
	}
	default_action= NoAction();
	size = 32;
}
