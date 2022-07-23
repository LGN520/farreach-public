/* Ingress Processing (Normal Operation) */

// stage 0

#ifdef RANGE_SUPPORT
action process_scanreq_split(server_sid) {
	modify_field(meta.server_sid, server_sid); // clone to server for next SCANREQ_SPLIT
	subtract(meta.remain_scannum, split_hdr.max_scannum, split_hdr.cur_scanidx);
}
action process_cloned_scanreq_split(udpport, server_sid) {
	//add_to_field(udp_hdr.dstPort, 1);
	modify_field(udp_hdr.dstPort, udpport); // set udpport for current SCANREQ_SPLIT
	modify_field(meta.server_sid, server_sid); // clone to server for next SCANREQ_SPLIT
	subtract(meta.remain_scannum, split_hdr.max_scannum, split_hdr.cur_scanidx);
}
action reset_meta_serversid_remainscannum() {
	modify_field(meta.server_sid, 0);
	modify_field(meta.remain_scannum, 0);
}
@pragma stage 0
table process_scanreq_split_tbl {
	reads {
		op_hdr.optype: exact;
		//udp_hdr.dstPort: exact;
		split_hdr.globalserveridx: exact;
		split_hdr.is_clone: exact;
	}
	actions {
		process_scanreq_split;
		process_cloned_scanreq_split;
		reset_meta_serversid_remainscannum;
	}
	default_action: reset_meta_serversid_remainscannum();
	size: PROCESS_SCANREQ_SPLIT_ENTRY_NUM;
}
#endif

// Stage 1

#ifdef RANGE_SUPPORT

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter lastscansplit_counter {
	type : packets_and_bytes;
	direct: lastscansplit_tbl;
}
#endif

action set_is_lastscansplit() {
	modify_field(meta.is_last_scansplit, 1);
}

action reset_is_lastscansplit() {
	modify_field(meta.is_last_scansplit, 0);
}

@pragma stage 1
table lastscansplit_tbl {
	reads {
		op_hdr.optype: exact;
		meta.remain_scannum: exact;
	}
	actions {
		set_is_lastscansplit;
		reset_is_lastscansplit;
	}
	default_action: reset_is_lastscansplit();
	size: 8;
}
#endif

// Stage 2

#ifdef RANGE_SUPPORT
action forward_scanreq_split_and_clone(server_sid) {
	modify_field(split_hdr.is_clone, 1);
	add_to_field(split_hdr.cur_scanidx, 1);
	add_to_field(split_hdr.globalserveridx, 1);
	// NOTE: eg_intr_md.egress_port has been set by process_(cloned)_scanreq_split_tbl in stage 0
	clone_egress_pkt_to_egress(server_sid); // clone to server (meta.server_sid)
}
action forward_scanreq_split() {
	modify_field(split_hdr.is_clone, 1);
	add_to_field(split_hdr.cur_scanidx, 1);
	add_to_field(split_hdr.globalserveridx, 1);
	// NOTE: eg_intr_md.egress_port has been set by process_(cloned)_scanreq_split_tbl in stage 0
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter eg_port_forward_counter {
	type : packets_and_bytes;
	direct: eg_port_forward_tbl;
}
#endif

@pragma stage 2
table eg_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
		meta.is_last_scansplit: exact;
		meta.server_sid: exact;
	}
	actions {
		forward_scanreq_split_and_clone;
		forward_scanreq_split;
		nop;
	}
	default_action: nop();
	size: 32;
}
#endif

// stage 3

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter update_ipmac_srcport_counter {
	type : packets_and_bytes;
	direct: update_ipmac_srcport_tbl;
}
#endif

action update_srcipmac_srcport_server2client(server_mac, server_ip, server_port) {
	modify_field(ethernet_hdr.srcAddr, server_mac);
	modify_field(ipv4_hdr.srcAddr, server_ip);
	modify_field(udp_hdr.srcPort, server_port);
}

action update_dstipmac_client2server(server_mac, server_ip) {
	modify_field(ethernet_hdr.dstAddr, server_mac);
	modify_field(ipv4_hdr.dstAddr, server_ip);
}

// NOTE: dstport of REQ, RES, and notification has been updated in partition_tbl, server, and eg_port_forward_tbl
@pragma stage 3
table update_ipmac_srcport_tbl {
	reads {
		op_hdr.optype: exact;
		eg_intr_md.egress_port: exact;
	}
	actions {
		update_srcipmac_srcport_server2client; // NOT change dstip and dstmac; use server[0] as srcip and srcmac; use server_worker_port_start as srcport
		update_dstipmac_client2server; // focus on dstip and dstmac to corresponding server; NOT change srcip, srcmac, and srcport
		nop;
	}
	default_action: nop();
	size: 128;
}

// stage 4

// NOTE: only one operand in add can be action parameter or constant -> resort to controller to configure different hdrlen
/*
// SCANREQ_SPLIT
action update_scanreqsplit_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 20(ophdr) + 16(endkey) + 11(split_hdr)
	modify_field(udp_hdr.hdrlen, 55);
	modify_field(ipv4_hdr.totalLen, 75);
}
*/

action update_pktlen(udplen, iplen) {
	modify_field(udp_hdr.hdrlen, udplen);
	modify_field(ipv4_hdr.totalLen, iplen);
}

@pragma stage 4
table update_pktlen_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		update_pktlen;
		nop;
	}
	default_action: nop(); // not change udp_hdr.hdrlen (GETREQ/GETREQ_POP/GETREQ_NLATEST)
	size: 4;
}
