control nocahceEgress(    /* User */
    inout headers                          hdr,
    inout metadata                         meta,
    /* Intrinsic */    
    in    egress_intrinsic_metadata_t                  eg_intr_md,
    in    egress_intrinsic_metadata_from_parser_t      eg_prsr_md,
    inout egress_intrinsic_metadata_for_deparser_t     eg_dprsr_md,
    inout egress_intrinsic_metadata_for_output_port_t  eg_oport_md){ 
	/* Ingress Processing (Normal Operation) */

	// stage 0


	// Stage 1


	// Stage 2


	// stage 3

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
			// eg_intr_md.egress_port: exact;
			eg_intr_md.egress_port: exact;
		}
		actions =  {
			update_ipmac_srcport_server2client; // focus on dstip and dstmac to corresponding client; use server[0] as srcip and srcmac; use server_worker_port_start as srcport
			update_dstipmac_client2server; // focus on dstip and dstmac to corresponding server; NOT change srcip, srcmac, and srcport
			NoAction;
		}
		default_action = NoAction();
		size = 128;
	}

	action update_pktlen(bit<16> udplen,bit<16> iplen) {
		hdr.udp_hdr.hdrlen = udplen;
		hdr.ipv4_hdr.totalLen = iplen;
	}

	@pragma stage 4
	table update_pktlen_tbl {
		key = {
			hdr.op_hdr.optype: exact;
		}
		actions =  {
			update_pktlen;
			NoAction;
		}
		default_action = NoAction(); // not change udp_hdr.hdrlen (GETREQ/GETREQ_POP/GETREQ_NLATEST)
		size = 4;
	}
	apply{

		// stage 3
		// NOTE: resource in stage 11 is not enough for update_ipmac_src_port_tbl, so we place it into stage 10
		update_ipmac_srcport_tbl.apply(); // Update ip, mac, and srcport for RES to client and notification to switchos

		// Stage 4
		update_pktlen_tbl.apply(); // Update udl_hdr.hdrLen for pkt with variable-length value
	}
}