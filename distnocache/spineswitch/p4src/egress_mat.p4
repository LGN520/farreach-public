control partitionswitchEgress(inout headers hdr,
                 inout metadata meta,
                 inout standard_metadata_t standard_metadata){ 
	/* Ingress Processing (Normal Operation) */

	#ifdef DEBUG
	// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
	counter update_ipmac_srcport_counter {
		type : packets_and_bytes;
		direct: update_ipmac_srcport_tbl;
	}
	#endif

	action update_ipmac_srcport_server2client(bit<48> client_mac,
												bit<48> server_mac,
												bit<32> client_ip,
												bit<32> server_ip,
												bit<16> server_port) {
		hdr.ethernet_hdr.srcAddr = server_mac;
		hdr.ethernet_hdr.dstAddr = client_mac;
		hdr.ipv4_hdr.srcAddr  = server_ip;
		hdr.ipv4_hdr.dstAddr =  client_ip;
		hdr.udp_hdr.srcPort = server_port;
	}

	action update_dstipmac_client2server(bit<48> server_mac,bit<32> server_ip) {
		hdr.ethernet_hdr.dstAddr = server_mac;
		hdr.ipv4_hdr.dstAddr = server_ip;
	}

	// NOTE: dstport of REQ, RES, and notification has been updated in partition_tbl, server, and eg_port_forward_tbl
	@pragma stage 3
	table update_ipmac_srcport_tbl {
		key = {
			hdr.op_hdr.optype: exact;
			standard_metadata.egress_port: exact;
		}
		actions =  {
			update_ipmac_srcport_server2client; // focus on dstip and dstmac to corresponding client; use server[0] as srcip and srcmac; use server_worker_port_start as srcport
			update_dstipmac_client2server; // focus on dstip and dstmac to corresponding server; NOT change srcip, srcmac, and srcport
			NoAction;
		}
		default_action = NoAction();
		size = 128;
	}


	apply{
		// stage 3
		// NOTE: resource in stage 11 is not enough for update_ipmac_src_port_tbl, so we place it into stage 10
		update_ipmac_srcport_tbl.apply(); // Update ip, mac, and srcport for RES to client and notification to switchos

		// Stage 4
	}
}