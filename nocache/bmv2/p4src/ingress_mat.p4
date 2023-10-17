/* Ingress Processing (Normal Operation) */


typedef bit<9>  egressSpec_t;
// Stage 0

control nocahceIngress (inout headers hdr,
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

	// Stage 1

	action hash_partition(bit<16> udpport,egressSpec_t eport) {
		hdr.udp_hdr.dstPort = udpport;
		standard_metadata.egress_spec = eport;
	}
	@pragma stage 1
	table hash_partition_tbl {
		key = {
			hdr.op_hdr.optype: exact;
			meta.meta.hashval_for_partition: range;
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

	apply{
		// Stage 0
		if (!hdr.op_hdr.isValid()) {
			l2l3_forward_tbl.apply(); // forward traditional packet
		}
		// else{

			// Stage 1 (not sure why we cannot place cache_lookup_tbl, hash_for_cm_tbl, and hash_for_seq_tbl in stage 1; follow automatic placement of bmv2 compiler)

			hash_partition_tbl.apply();

			// Stage 3
			ipv4_forward_tbl.apply(); // update egress_spec for normal/speical response packets

		// }
	}
}