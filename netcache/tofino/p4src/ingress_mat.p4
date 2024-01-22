/* Ingress Processing (Normal Operation) */
/* Ingress Processing */

typedef bit<9>  egressSpec_t;
control netcacheIngress(  /* User */
    inout headers                       hdr,
    inout ingress_metadata                      meta,
    /* Intrinsic */
    in    ingress_intrinsic_metadata_t               ig_intr_md,
    in    ingress_intrinsic_metadata_from_parser_t   ig_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md,
    inout ingress_intrinsic_metadata_for_tm_t        ig_tm_md){
	bit<32> tmp;
	// Stage 0

	action l2l3_forward(egressSpec_t eport) {
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
	@pragma stage 5
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
		ig_tm_md.ucast_egress_port = eport;
	}
	@pragma stage 6
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

	@pragma stage 6
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

	// @pragma stage 2 
	@pragma stage 0
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
		hdr.inswitch_hdr.hashval_for_seq = meta.hashval_for_partition;
	}

	@pragma stage 7
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

	@pragma stage 8
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
	Hash<bit<18>>(HashAlgorithm_t.CRC32) hash_bf1_calc;
	action hash_for_bf1() {
		// modify_field_with_hash_based_offset(hdr.inswitch_hdr.hashval_for_bf1, 0, hash_calc, BF_BUCKET_COUNT);
		hdr.inswitch_hdr.hashval_for_bf1 = hash_bf1_calc.get({
			hdr.op_hdr.keylolo,
			hdr.op_hdr.keylohi,
			hdr.op_hdr.keyhilo,
			hdr.op_hdr.keyhihilo,
			hdr.op_hdr.keyhihihi
		});
	}

	@pragma stage 9
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

	// CRCPolynomial<bit<18>>(18w0x34578, true, false, false,18w0x3FFFF, 18w0x3FFFF) crc32_bf2;
	Hash<bit<18>>(HashAlgorithm_t.CRC16) hash_bf2_calc;
	action hash_for_bf2() {
		hdr.inswitch_hdr.hashval_for_bf2 = hash_bf2_calc.get({
			hdr.op_hdr.keylolo,
			hdr.op_hdr.keylohi,
			hdr.op_hdr.keyhilo,
			hdr.op_hdr.keyhihilo,
			hdr.op_hdr.keyhihihi
		});
	}

	@pragma stage 10
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
	Hash<bit<18>>(HashAlgorithm_t.IDENTITY) hash_bf3_calc;
	action hash_for_bf3() {
		// modify_field_with_hash_based_offset(hdr.inswitch_hdr.hashval_for_bf3, 0, hash_calc3, BF_BUCKET_COUNT);
		hdr.inswitch_hdr.hashval_for_bf3 = hash_bf3_calc.get({
			hdr.op_hdr.keylolo,
			hdr.op_hdr.keylohi,
			hdr.op_hdr.keyhilo,
			hdr.op_hdr.keyhihilo,
			hdr.op_hdr.keyhihihi
		});
	}

	@pragma stage 11
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

	action set_client_sid(bit<10> client_sid) {
		hdr.inswitch_hdr.client_sid = client_sid;
	}

	@pragma stage 2
	table prepare_for_cachehit_tbl {
		key =  {
			hdr.op_hdr.optype: exact;
			//ig_intr_md.ingress_port: exact;
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
		ig_tm_md.ucast_egress_port = eport;
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
	Random<bit<1>>() rnd1;
	action sample() {
		//modify_field_with_hash_based_offset(hdr.inswitch_hdr.is_sampled, 0, hash_calc, 2); // WRONG: we should not sample key
		// modify_field_rng_uniform(hdr.inswitch_hdr.is_sampled, 0, 1); // generate a random value in [0, 1] to sample packet
		// random(hdr.inswitch_hdr.is_sampled,(bit<1>)0,(bit<1>)1);
		hdr.inswitch_hdr.is_sampled = rnd1.get();
	}

	@pragma stage 1
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
		// ig_tm_md.ucast_egress_port = standard_metadata.ingress_port;
		ig_tm_md.ucast_egress_port = ig_intr_md.ingress_port;
		// swap to set dstport as corresponding server.valueupdateserver port
		bit<16> tmp_port;
		// swap(hdr.udp_hdr.srcPort,hdr.udp_hdr.dstPort);
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

	@pragma stage 11
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
			hdr.udp_hdr.checksum = 0;
			// // Stage 0
			 // for response of cache hit (access inswitch_hdr.client_sid)
			set_hot_threshold_tbl.apply(); // set inswitch_hdr.hot_threshold
			cache_lookup_tbl.apply(); // managed by controller (access inswitch_hdr.is_cached, inswitch_hdr.idx)
			// stage 1
			prepare_for_cachehit_tbl.apply();
			sample_tbl.apply(); // for CM and cache_frequency (access inswitch_hdr.is_sampled)
			// // Stage 5
			hash_for_partition_tbl.apply(); // for hash partition (including startkey of SCANREQ)

			// // Stage 2 (not sure why we cannot place cache_lookup_tbl, hash_for_cm_tbl, and hash_for_seq_tbl in stage 1; follow automatic placement of tofino compiler)
			hash_partition_tbl.apply();
			ipv4_forward_tbl.apply(); // update egress_port for normal/speical response packets

			// // Stage 6
			hash_for_cm12_tbl.apply(); // for CM (access inswitch_hdr.hashval_for_cm1/2)

			// // IMPORTANT: to save TCAM, we do not match op_hdr.optype in cache_lookup_tbl 
			// // -> so as long as op_hdr.key matches an entry in cache_lookup_tbl, inswitch_hdr.is_cached must be 1 (e.g., CACHE_EVICT_LOADXXX)
			// // -> but note that if the optype does not have inswitch_hdr, is_cached of 1 will be dropped after entering egress pipeline, and is_cached is still 0 (e.g., SCANREQ_SPLIT)
			// // Stage 3
			// // Stage 7
			
			hash_for_seq_tbl.apply(); // for seq (access inswitch_hdr.hashval_for_seq)

			// // // Stgae 8
			hash_for_cm34_tbl.apply(); // for CM (access inswitch_hdr.hashval_for_cm3/4)
			// Stage 9
			hash_for_bf1_tbl.apply();

			// // // Stage 10

			hash_for_bf2_tbl.apply();

			// // // Stage 11
			hash_for_bf3_tbl.apply();
			// Stage 11
			ig_port_forward_tbl.apply(); // update op_hdr.optype (update egress_port for NETCACHE_VALUEUPDATE)
		}
	}
}