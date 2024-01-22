/* Ingress Processing (Normal Operation) */


typedef bit<9>  egressSpec_t;
// Stage 0

control nocahceIngress (    /* User */
    inout headers                       hdr,
    inout metadata                      meta,
    /* Intrinsic */
    in    ingress_intrinsic_metadata_t               ig_intr_md,
    in    ingress_intrinsic_metadata_from_parser_t   ig_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md,
    inout ingress_intrinsic_metadata_for_tm_t        ig_tm_md){

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
	#ifndef RANGE_SUPPORT
	Hash<bit<15>>(HashAlgorithm_t.CRC32) hash_partition_calc;
	action hash_for_partition() {
		// hash(meta.hashval_for_partition, HashAlgorithm.crc32, (bit<32>)0, {
		// 	hdr.op_hdr.keylolo,
		// 	hdr.op_hdr.keylohi,
		// 	hdr.op_hdr.keyhilo,
		// 	hdr.op_hdr.keyhihilo,
		// 	hdr.op_hdr.keyhihihi
		// }, (bit<32>) PARTITION_COUNT);
		meta.hashval_for_partition[14:0] = hash_partition_calc.get({
			hdr.op_hdr.keylolo,
			hdr.op_hdr.keylohi,
			hdr.op_hdr.keyhilo,
			hdr.op_hdr.keyhihilo,
			hdr.op_hdr.keyhihihi
		});
		// meta.hashval_for_partition = meta.hashval_for_partition % (bit<16>) PARTITION_COUNT;
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
		size = 16;
	}
	#endif
	// Stage 1

	action hash_partition(bit<16> udpport,egressSpec_t eport) {
		hdr.udp_hdr.dstPort = udpport;
		ig_tm_md.ucast_egress_port = eport;
	}
	@pragma stage 1
	table hash_partition_tbl {
		key = {
			hdr.op_hdr.optype: exact;
			meta.hashval_for_partition: range;
		}
		actions = {
			hash_partition;
			NoAction;
		}
		default_action = NoAction();
		size = HASH_PARTITION_ENTRY_NUM;
	}

	// Stage 3

	action forward_normal_response(egressSpec_t eport) {
		ig_tm_md.ucast_egress_port = eport;
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

	apply{
		// Stage 0
		if (!hdr.op_hdr.isValid()) {
			l2l3_forward_tbl.apply(); // forward traditional packet
		}
		// else{

		// Stage 1 (not sure why we cannot place cache_lookup_tbl, hash_for_cm_tbl, and hash_for_seq_tbl in stage 1; follow automatic placement of bmv2 compiler)
		hash_for_partition_tbl.apply();
		hash_partition_tbl.apply();

		// Stage 3
		ipv4_forward_tbl.apply(); // update egress_spec for normal/speical response packets

		// }
	}
}