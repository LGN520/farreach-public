/* Ingress Processing (Normal Operation) */
/* Ingress Processing */

typedef bit<9>  egressSpec_t;
control netcacheIngress(inout headers hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata){
	bit<32> tmp;
	// Stage 0

	action l2l3_forward(egressSpec_t eport) {
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

	// Stage 1

	/*action reset_is_wrong_pipeline() {
		modify_field(hdr.inswitch_hdr.is_wrong_pipeline, 0);
	}*/
#ifndef RANGE_SUPPORT
	action hash_for_partition() {
		// modify_field_with_hash_based_offset(meta.hashval_for_partition, 0, hash_calc, PARTITION_COUNT);
		hash(meta.hashval_for_partition, HashAlgorithm.crc32, (bit<32>)0, {
			hdr.op_hdr.keylolo,
			hdr.op_hdr.keylohi,
			hdr.op_hdr.keyhilo,
			hdr.op_hdr.keyhihilo,
			hdr.op_hdr.keyhihihi
		}, (bit<32>) PARTITION_COUNT);
		hash(meta.hashval_for_spine_partition, HashAlgorithm.csum16, (bit<32>)0, {
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
		default_action = NoAction();
		size = 16;
	}
#endif
	// Stage 2

	action hash_partition(bit<16> udpport,egressSpec_t eport) {
		hdr.udp_hdr.dstPort = udpport;
		standard_metadata.egress_spec = eport;
	}
	@pragma stage 1
	table hash_partition_tbl {
		key = {
			hdr.op_hdr.optype: exact;
			// RANGE_SURPPORT
			meta.hashval_for_partition: range;
		}
		actions = {
			hash_partition;
			NoAction;
		}
		default_action = NoAction();
		size = HASH_PARTITION_ENTRY_NUM;
	}

	action hash_spine_partition(bit<10> spine_sid, egressSpec_t eport) {
		meta.spine_sid = spine_sid;
		standard_metadata.egress_spec = eport;
		clone(CloneType.I2E,(bit<32>) spine_sid);
	}
	@pragma stage 1
	table hash_spine_partition_tbl {
		key = {
			hdr.op_hdr.optype: exact;
			// RANGE_SURPPORT
			meta.hashval_for_spine_partition: range;
		}
		actions = {
			hash_spine_partition;
			NoAction;
		}
		default_action = NoAction();
		size = HASH_PARTITION_ENTRY_NUM;
	}

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
		key =  {
			hdr.op_hdr.optype: exact;
		}
		actions = {
			hash_for_cm12;
			NoAction;
		}
		default_action = NoAction();
		size = 2;
	}

	action cached_action(bit<16> idx) {
		hdr.inswitch_hdr.idx = idx;
		hdr.inswitch_hdr.is_cached = 1;
	}

	action uncached_action() {
		hdr.inswitch_hdr.is_cached = 0;
	}

	@pragma stage 2 16384
	@pragma stage 3
	table cache_lookup_tbl {
		key =  {
			hdr.op_hdr.keylolo: exact;
			hdr.op_hdr.keylohi: exact;
			hdr.op_hdr.keyhilo: exact;
			hdr.op_hdr.keyhihilo: exact;
			hdr.op_hdr.keyhihihi: exact;
		}
		actions = {
			cached_action;
			uncached_action;
		}
		default_action = uncached_action();
		size = LOOKUP_ENTRY_COUNT; // egress_pipenum * KV_BUCKET_COUNT
	}

	// Stage 3


	// Stage 4

	action hash_for_seq() {
		// modify_field_with_hash_based_offset(hdr.inswitch_hdr.hashval_for_seq, 0, hash_calc, SEQ_BUCKET_COUNT);
		hash(hdr.inswitch_hdr.hashval_for_seq, HashAlgorithm.crc32, (bit<16>)0, {
			hdr.op_hdr.keylolo,
			hdr.op_hdr.keylohi,
			hdr.op_hdr.keyhilo,
			hdr.op_hdr.keyhihilo,
			hdr.op_hdr.keyhihihi
		}, (bit<16>)SEQ_BUCKET_COUNT);
	}

	@pragma stage 4
	table hash_for_seq_tbl {
		key =  {
			hdr.op_hdr.optype: exact;
		}
		actions = {
			hash_for_seq;
			NoAction;
		}
		default_action = NoAction();
		size = 4;
	}

	// Stage 5

	action hash_for_cm34() {
		// modify_field_with_hash_based_offset(hdr.inswitch_hdr.hashval_for_cm3, 0, hash_calc3, CM_BUCKET_COUNT);
		hash(tmp, HashAlgorithm.identity, (bit<32>)0, {
			hdr.op_hdr.keylolo,
			hdr.op_hdr.keylohi,
			hdr.op_hdr.keyhilo,
			hdr.op_hdr.keyhihilo,
			hdr.op_hdr.keyhihihi
		}, (bit<32>)CM_BUCKET_COUNT);
		hdr.inswitch_hdr.hashval_for_cm3 = (bit<16>) tmp;
		// modify_field_with_hash_based_offset(hdr.inswitch_hdr.hashval_for_cm4, 0, hash_calc4, CM_BUCKET_COUNT);
		hash(tmp, HashAlgorithm.csum16, (bit<32>)0, {
			hdr.op_hdr.keylolo,
			hdr.op_hdr.keylohi,
			hdr.op_hdr.keyhilo,
			hdr.op_hdr.keyhihilo,
			hdr.op_hdr.keyhihihi
		}, (bit<32>)CM_BUCKET_COUNT);
		hdr.inswitch_hdr.hashval_for_cm4 = (bit<16>) tmp;
	}

	@pragma stage 5
	table hash_for_cm34_tbl {
		key =  {
			hdr.op_hdr.optype: exact;
		}
		actions = {
			hash_for_cm34;
			NoAction;
		}
		default_action = NoAction();
		size = 2;
	}

	action hash_for_bf1() {
		// modify_field_with_hash_based_offset(hdr.inswitch_hdr.hashval_for_bf1, 0, hash_calc, BF_BUCKET_COUNT);
		hash(hdr.inswitch_hdr.hashval_for_bf1, HashAlgorithm.crc32, (bit<18>)0, {
			hdr.op_hdr.keylolo,
			hdr.op_hdr.keylohi,
			hdr.op_hdr.keyhilo,
			hdr.op_hdr.keyhihilo,
			hdr.op_hdr.keyhihihi
		}, (bit<32>)BF_BUCKET_COUNT);
	}

	@pragma stage 5
	table hash_for_bf1_tbl {
		key =  {
			hdr.op_hdr.optype: exact;
		}
		actions = {
			hash_for_bf1;
			NoAction;
		}
		default_action = NoAction();
		size = 2;
	}

	// Stage 6

	action hash_for_bf2() {
		hash(tmp, HashAlgorithm.crc32_custom, (bit<32>)0, {
			hdr.op_hdr.keylolo,
			hdr.op_hdr.keylohi,
			hdr.op_hdr.keyhilo,
			hdr.op_hdr.keyhihilo,
			hdr.op_hdr.keyhihihi
		}, (bit<32>)BF_BUCKET_COUNT);
		hdr.inswitch_hdr.hashval_for_bf2=(bit<18>)tmp;
	}

	@pragma stage 6
	table hash_for_bf2_tbl {
		key =  {
			hdr.op_hdr.optype: exact;
		}
		actions = {
			hash_for_bf2;
			NoAction;
		}
		default_action = NoAction();
		size = 2;
	}

	// Stage 7

	action hash_for_bf3() {
		hash(hdr.inswitch_hdr.hashval_for_bf3, HashAlgorithm.identity, (bit<18>)0, {
			hdr.op_hdr.keylolo,
			hdr.op_hdr.keylohi,
			hdr.op_hdr.keyhilo,
			hdr.op_hdr.keyhihilo,
			hdr.op_hdr.keyhihihi
		}, (bit<32>)BF_BUCKET_COUNT);
	}

	@pragma stage 7
	table hash_for_bf3_tbl {
		key =  {
			hdr.op_hdr.optype: exact;
		}
		actions = {
			hash_for_bf3;
			NoAction;
		}
		default_action = NoAction();
		size = 2;
	}

	// NOTE: standard_metadata.egress_spec is a read-only field (we cannot directly set egress port in egress pipeline even if w/ correct pipeline)
	// NOTE: using inswitch_hdr.client_sid for clone_e2e in ALU needs to maintain inswitch_hdr.client_sid and eg_intr_md_for_md.mirror_id into the same group, which violates PHV allocation constraints -> but MAU can access different groups
	action set_client_sid(bit<10> client_sid) {
		hdr.inswitch_hdr.client_sid = client_sid;
	}

	@pragma stage 7
	table prepare_for_cachehit_tbl {
		key =  {
			hdr.op_hdr.optype: exact;
			hdr.ipv4_hdr.srcAddr: lpm;
		}
		actions = {
			set_client_sid;
			NoAction;
		}
		default_action = set_client_sid(0); // deprecated: configured as set_client_sid(sids[0]) in ptf
		size = 32;
	}

	action forward_normal_response(egressSpec_t eport) {
		standard_metadata.egress_spec = eport;
	}

	@pragma stage 7
	table ipv4_forward_tbl {
		key =  {
			hdr.op_hdr.optype: exact;
			hdr.ipv4_hdr.dstAddr: lpm;
		}
		actions = {
			forward_normal_response;
			NoAction;
		}
		default_action = NoAction();
		size = 64;
	}

	// Stage 8

	action sample() {
		//modify_field_with_hash_based_offset(hdr.inswitch_hdr.is_sampled, 0, hash_calc, 2); // WRONG: we should not sample key
		// modify_field_rng_uniform(hdr.inswitch_hdr.is_sampled, 0, 1); // generate a random value in [0, 1] to sample packet
		// random(hdr.inswitch_hdr.is_sampled,(bit<1>)0,(bit<1>)1);
		hdr.inswitch_hdr.is_sampled = 1;
	}

	@pragma stage 8
	table sample_tbl {
		key =  {
			hdr.op_hdr.optype: exact;
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

	action update_netcache_valueupdate_to_netcache_valueupdate_inswitch() {
		hdr.op_hdr.optype = NETCACHE_VALUEUPDATE_INSWITCH;
		hdr.shadowtype_hdr.shadowtype = NETCACHE_VALUEUPDATE_INSWITCH;

		hdr.inswitch_hdr.setValid();

		// NOTE: NETCACHE_VALUEUPDATE does not need partition_tbl, as in-switch record must in the same pipeline of the ingress port, which can also send NETCACHE_VALUEUPDATE_ACK back to corresponding server
		standard_metadata.egress_spec = standard_metadata.ingress_port;
		// swap to set dstport as corresponding server.valueupdateserver port
		bit<16> tmp_port;
		tmp_port = hdr.udp_hdr.srcPort;
		hdr.udp_hdr.srcPort= hdr.udp_hdr.dstPort;
		hdr.udp_hdr.dstPort=tmp_port;
	}

	action update_putreq_largevalue_to_putreq_largevalue_inswitch() {
		// NOTE: PUTREQ_LARGEVALUE only w/ op_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr
		hdr.op_hdr.optype = PUTREQ_LARGEVALUE_INSWITCH;
		hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_INSWITCH;

		hdr.shadowtype_hdr.setValid();
		hdr.inswitch_hdr.setValid();
	}

	@pragma stage 8
	table ig_port_forward_tbl {
		key =  {
			hdr.op_hdr.optype: exact;
		}
		actions = {
			update_getreq_to_getreq_inswitch;
			update_putreq_to_putreq_inswitch;
			update_delreq_to_delreq_inswitch;
			update_warmupreq_to_netcache_warmupreq_inswitch;
			update_netcache_valueupdate_to_netcache_valueupdate_inswitch;
			update_putreq_largevalue_to_putreq_largevalue_inswitch;
			NoAction;
		}
		default_action = NoAction();
		size = 8;
	}
	apply{
		// Stage 0
		if (!hdr.op_hdr.isValid()) {
			l2l3_forward_tbl.apply(); // forward traditional packet
		}else{
			set_hot_threshold_tbl.apply(); // set inswitch_hdr.hot_threshold
			// Stage 1
			#ifndef RANGE_SUPPORT
			hash_for_partition_tbl.apply(); // for hash partition (including startkey of SCANREQ)
			#endif
			// Stage 2 (not sure why we cannot place cache_lookup_tbl, hash_for_cm_tbl, and hash_for_seq_tbl in stage 1; follow automatic placement of tofino compiler)
			hash_partition_tbl.apply();
			hash_spine_partition_tbl.apply();
			
			hash_for_cm12_tbl.apply(); // for CM (access inswitch_hdr.hashval_for_cm1/2)
			// IMPORTANT: to save TCAM, we do not match op_hdr.optype in cache_lookup_tbl 
			// -> so as long as op_hdr.key matches an entry in cache_lookup_tbl, inswitch_hdr.is_cached must be 1 (e.g., CACHE_EVICT_LOADXXX)
			// -> but note that if the optype does not have inswitch_hdr, is_cached of 1 will be dropped after entering egress pipeline, and is_cached is still 0 (e.g., SCANREQ_SPLIT)
			cache_lookup_tbl.apply(); // managed by controller (access inswitch_hdr.is_cached, inswitch_hdr.idx)

			// Stage 3

			// Stage 4
			//hash_for_cm2_tbl.apply(); // for CM (access inswitch_hdr.hashval_for_cm2)
			hash_for_seq_tbl.apply(); // for seq (access inswitch_hdr.hashval_for_seq)

			// Stgae 5
			hash_for_cm34_tbl.apply(); // for CM (access inswitch_hdr.hashval_for_cm3/4)
			hash_for_bf1_tbl.apply();

			// Stage 6
			//hash_for_cm4_tbl.apply(); // for CM (access inswitch_hdr.hashval_for_cm4)
			hash_for_bf2_tbl.apply();

			// Stage 7
			hash_for_bf3_tbl.apply();
			prepare_for_cachehit_tbl.apply(); // for response of cache hit (access inswitch_hdr.client_sid)
			ipv4_forward_tbl.apply(); // update egress_port for normal/speical response packets

			// Stage 8
			sample_tbl.apply(); // for CM and cache_frequency (access inswitch_hdr.is_sampled)
			ig_port_forward_tbl.apply(); // update op_hdr.optype (update egress_port for NETCACHE_VALUEUPDATE)
		}
	}
}