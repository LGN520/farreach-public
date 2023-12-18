/* Ingress Processing (Normal Operation) */


typedef bit<9>  egressSpec_t;
// Stage 0

control partitionswitchIngress (inout headers hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata){


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
	@pragma stage 0
	table hash_for_partition_tbl {
		key = {
			hdr.op_hdr.optype: exact;
		}
		actions = {
			hash_for_partition;
			NoAction;
		}
		default_action = NoAction();
		size = 32;
	}
	#endif
	// Stage 1

	action hash_leaf_partition(egressSpec_t eport) {
		standard_metadata.egress_spec = eport;
	}
	table hash_leaf_partition_tbl {
		key = {
			hdr.op_hdr.optype: exact;
			meta.hashval_for_partition: range;
		}
		actions = {
			hash_leaf_partition;
			NoAction;
		}
		default_action = NoAction();
		size = HASH_PARTITION_ENTRY_NUM;
	}
	// Stage 3

	action forward_normal_response(egressSpec_t eport) {
		standard_metadata.egress_spec = eport;
	}
	@pragma stage 3
	table ipv4_forward_tbl {
		key = {
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

	action cached_action(bit<16> idx) {
		meta.idx = idx;
		meta.is_cached = 1;
	}

	action uncached_action() {
		meta.is_cached = 0;
	}

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

	action set_client_sid(bit<10> client_sid) {
		meta.client_sid = client_sid;
	}

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
	action set_spine(){
		meta.is_spine = 1;
	}
	table set_spine_tbl {
		key =  {
			hdr.op_hdr.optype: exact;
		}
		actions = {
			set_spine;
			NoAction;
		}
		default_action = NoAction; // deprecated: configured as set_client_sid(sids[0]) in ptf
		size = 32;
	}
	apply{
		// Stage 0
		if (!hdr.op_hdr.isValid()) {
			l2l3_forward_tbl.apply(); // forward traditional packet
		}
		else{

			// Stage 0 
			
			hash_for_partition_tbl.apply();
			hash_leaf_partition_tbl.apply();

			// Stage 1
			ipv4_forward_tbl.apply(); // update egress_spec for normal/speical response packets
			set_spine_tbl.apply();
			cache_lookup_tbl.apply();
			if(meta.is_spine == 1){
				prepare_for_cachehit_tbl.apply();
			}

		}
	}
}