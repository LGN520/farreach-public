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

	action hash_partition(egressSpec_t eport) {
		standard_metadata.egress_spec = eport;
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
		standard_metadata.egress_spec = eport;
	}
	action forward_special_get_response(bit<10> client_sid) {
		// NOTE: eport to server has already been set by partition_tbl for GETRES_LATEST/DELETED_SEQ
		//standard_metadata.egress_spec = standard_metadata.ingress_port; // Original packet enters the egress pipeline to server
		clone(CloneType.I2E, (bit<32>)client_sid); 
		// clone_ingress_pkt_to_egress(client_sid); // Cloned packet enter the egress pipeline to corresponding client
	}
	@pragma stage 3
	table ipv4_forward_tbl {
		key = {
			hdr.op_hdr.optype: exact;
			hdr.ipv4_hdr.dstAddr: lpm;
		}
		actions = {
			forward_normal_response;
			forward_special_get_response;
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
		else{

			// Stage 0 
			hash_for_partition_tbl.apply();
			hash_partition_tbl.apply();
			// Stage 1
			ipv4_forward_tbl.apply(); // update egress_spec for normal/speical response packets

		}
	}
}