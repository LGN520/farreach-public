/* Ingress Processing (Normal Operation) */

// Stage 0

action save_client_info() {
	modify_field(clone_hdr.client_ip, ipv4_hdr.srcAddr);
	modify_field(clone_hdr.client_mac, ethernet_hdr.srcAddr);
	modify_field(clone_hdr.client_udpport, udp_hdr.srcPort);
}

@pragma stage 0
table save_client_info_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		save_client_info;
		nop;
	}
	default_action: nop();
	size: 4;
}

#ifdef RANGE_SUPPORT
action process_scanreq_split(server_sid) {
	modify_field(clone_hdr.server_sid, server_sid); // clone to server for next SCANREQ_SPLIT
	subtract(meta.remain_scannum, split_hdr.max_scannum, split_hdr.cur_scanidx);
	modify_field(clone_hdr.clonenum_for_pktloss, 0);
}
action process_cloned_scanreq_split(udpport, server_sid) {
	//add_to_field(udp_hdr.dstPort, 1);
	modify_field(udp_hdr.dstPort, udpport); // set udpport for current SCANREQ_SPLIT
	modify_field(clone_hdr.server_sid, server_sid); // clone to server for next SCANREQ_SPLIT
	subtract(meta.remain_scannum, split_hdr.max_scannum, split_hdr.cur_scanidx);
	modify_field(clone_hdr.clonenum_for_pktloss, 0);
}
/*action reset_meta_serversid_remainscannum() {
	modify_field(clone_hdr.server_sid, 0);
	modify_field(meta.remain_scannum, 0);
}*/
action reset_meta_remainscannum() {
	modify_field(meta.remain_scannum, 0);
}
#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter process_scanreq_split_counter {
	type : packets_and_bytes;
	direct: process_scanreq_split_tbl;
}
#endif
@pragma stage 0
table process_scanreq_split_tbl {
	reads {
		op_hdr.optype: exact;
		//udp_hdr.dstPort: exact;
		split_hdr.globalserveridx: exact;
		//eg_intr_md_from_parser_aux.clone_src: exact; // NOTE: access intrinsic metadata
		split_hdr.is_clone: exact;
	}
	actions {
		process_scanreq_split;
		process_cloned_scanreq_split;
		//reset_meta_serversid_remainscannum;
		reset_meta_remainscannum;
		nop;
	}
	//default_action: reset_meta_serversid_remainscannum();
	default_action: reset_meta_remainscannum();
	size: PROCESS_SCANREQ_SPLIT_ENTRY_NUM;
}
#endif

// Stage 1

action set_server_sid_and_port(server_sid) {
	modify_field(clone_hdr.server_sid, server_sid);
	modify_field(clone_hdr.server_udpport, udp_hdr.dstPort); // dstport is serverport for GETREQ_INSWITCH
}

action reset_server_sid() {
	modify_field(clone_hdr.server_sid, 0);
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter prepare_for_cachepop_counter {
	type : packets_and_bytes;
	direct: prepare_for_cachepop_tbl;
}
#endif

@pragma stage 1
table prepare_for_cachepop_tbl {
	reads {
		op_hdr.optype: exact;
		eg_intr_md.egress_port: exact;
	}
	actions {
		set_server_sid_and_port;
		reset_server_sid;
		nop;
	}
	default_action: reset_server_sid();
	size: 32;
}

// Stage 2

action set_is_hot() {
	modify_field(meta.is_hot, 1);
	//modify_field(debug_hdr.is_hot, 1);
}

action reset_is_hot() {
	modify_field(meta.is_hot, 0);
	//modify_field(debug_hdr.is_hot, 0);
}

@pragma stage 2
table is_hot_tbl {
	reads {
		meta.cm1_predicate: exact;
		meta.cm2_predicate: exact;
		meta.cm3_predicate: exact;
		meta.cm4_predicate: exact;
	}
	actions {
		set_is_hot;
		reset_is_hot;
	}
	default_action: reset_is_hot();
	size: 1;
}

// Stage 7

action set_is_report() {
	modify_field(meta.is_report, 1);
}

action reset_is_report() {
	modify_field(meta.is_report, 0);
}

@pragma stage 7
table is_report_tbl {
	reads {
		meta.is_report1: exact;
		meta.is_report2: exact;
		meta.is_report3: exact;
	}
	actions {
		set_is_report;
		reset_is_report;
	}
	default_action: reset_is_report();
	size: 1;
}

// Stage 8

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter lastclone_lastscansplit_counter {
	type : packets_and_bytes;
	direct: lastclone_lastscansplit_tbl;
}
#endif

action set_is_lastclone() {
	modify_field(meta.is_lastclone_for_pktloss, 1);
	//modify_field(debug_hdr.is_lastclone_for_pktloss, 1);
#ifdef RANGE_SUPPORT
	modify_field(meta.is_last_scansplit, 0);
#endif
}

#ifdef RANGE_SUPPORT
action set_is_lastscansplit() {
	modify_field(meta.is_last_scansplit, 1);
	modify_field(meta.is_lastclone_for_pktloss, 0);
}
#endif

action reset_is_lastclone_lastscansplit() {
	modify_field(meta.is_lastclone_for_pktloss, 0);
	//modify_field(debug_hdr.is_lastclone_for_pktloss, 0);
#ifdef RANGE_SUPPORT
	modify_field(meta.is_last_scansplit, 0);
#endif
}

@pragma stage 8
table lastclone_lastscansplit_tbl {
	reads {
		op_hdr.optype: exact;
		clone_hdr.clonenum_for_pktloss: exact;
#ifdef RANGE_SUPPORT
		meta.remain_scannum: exact;
#endif
	}
	actions {
		set_is_lastclone;
#ifdef RANGE_SUPPORT
		set_is_lastscansplit;
#endif
		reset_is_lastclone_lastscansplit;
	}
	default_action: reset_is_lastclone_lastscansplit();
	size: 8;
}

// Stage 9

action update_netcache_warmupreq_inswitch_to_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack(switchos_sid, reflector_port) {
	modify_field(op_hdr.optype, NETCACHE_WARMUPREQ_INSWITCH_POP);
	modify_field(shadowtype_hdr.shadowtype, NETCACHE_WARMUPREQ_INSWITCH_POP);
	modify_field(udp_hdr.dstPort, reflector_port);
	modify_field(clone_hdr.clonenum_for_pktloss, 3); // 3 ACKs (drop w/ 3 -> clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> drop and clone for WARMUPACK to client)

	add_header(clone_hdr); // NOTE: clone_hdr.server_sid is reset as 0 in process_scanreq_split_tbl and prepare_for_cachepop_tbl

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action forward_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack(switchos_sid) {
	subtract_from_field(clone_hdr.clonenum_for_pktloss, 1);

	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action update_netcache_warmupreq_inswitch_pop_to_warmupack_by_mirroring(client_sid, server_port) {
	modify_field(op_hdr.optype, WARMUPACK);
	// DEPRECATED: udp.srcport will be set as server_worker_port_start in update_ipmac_srcport_tbl
	// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports
	modify_field(udp_hdr.srcPort, server_port);
	modify_field(udp_hdr.dstPort, clone_hdr.client_udpport);

	remove_header(shadowtype_hdr);
	remove_header(inswitch_hdr);
	remove_header(clone_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(client_sid); // clone to client (inswitch_hdr.client_sid)
}

action update_getreq_inswitch_to_getreq() {
	modify_field(op_hdr.optype, GETREQ);

	remove_header(shadowtype_hdr);
	remove_header(inswitch_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_getreq_inswitch_to_netcache_getreq_pop_clone_for_pktloss_and_getreq(switchos_sid, reflector_port) {
	modify_field(op_hdr.optype, NETCACHE_GETREQ_POP);
	modify_field(udp_hdr.dstPort, reflector_port);
	modify_field(clone_hdr.clonenum_for_pktloss, 3); // 3 ACKs (drop w/ 3 -> clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> drop and clone for GETREQ to server)

	remove_header(shadowtype_hdr);
	remove_header(inswitch_hdr);
	add_header(clone_hdr); // NOTE: clone_hdr.server_sid has been set in prepare_for_cachepop_tbl

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action forward_netcache_getreq_pop_clone_for_pktloss_and_getreq(switchos_sid) {
	subtract_from_field(clone_hdr.clonenum_for_pktloss, 1);

	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action update_netcache_getreq_pop_to_getreq_by_mirroring(server_sid) {
	modify_field(op_hdr.optype, GETREQ);
	// Keep original udp.srcport (aka client udp port)
	modify_field(udp_hdr.dstPort, clone_hdr.server_udpport);

	remove_header(clone_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(server_sid); // clone to client (inswitch_hdr.client_sid)
}

action update_getreq_inswitch_to_getres_by_mirroring(client_sid, stat) {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
	modify_field(stat_hdr.stat, stat);
	modify_field(stat_hdr.nodeidx_foreval, SWITCHIDX_FOREVAL);

	modify_field(ipv4_hdr.dstAddr, clone_hdr.client_ip);
	modify_field(ethernet_hdr.dstAddr, clone_hdr.client_mac);
	modify_field(udp_hdr.dstPort, clone_hdr.client_udpport);

	remove_header(inswitch_hdr);
	add_header(stat_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(client_sid); // clone to client (inswitch_hdr.client_sid)
}

/*action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss(switchos_sid, reflector_port) {
	modify_field(op_hdr.optype, CACHE_POP_INSWITCH_ACK);
	modify_field(udp_hdr.dstPort, reflector_port);
	//modify_field(meta.clonenum_for_pktloss, 1); // 3 ACKs (clone w/ 1 -> clone w/ 0 -> no clone w/ ack)
	modify_field(clone_hdr.clonenum_for_pktloss, 2); // 3 ACKs (drop w/ 2 -> clone w/ 1 -> clone w/ 0 -> no clone w/ ack)

	//remove_header(vallen_hdr);
	//remove_header(val1_hdr);
	//remove_header(val2_hdr);
	//remove_header(val3_hdr);
	//remove_header(val4_hdr);
	//remove_header(val5_hdr);
	//remove_header(val6_hdr);
	//remove_header(val7_hdr);
	//remove_header(val8_hdr);
	//remove_header(val9_hdr);
	//remove_header(val10_hdr);
	//remove_header(val11_hdr);
	//remove_header(val12_hdr);
	//remove_header(val13_hdr);
	//remove_header(val14_hdr);
	//remove_header(val15_hdr);
	//remove_header(val16_hdr);
	remove_header(shadowtype_hdr);
	remove_header(seq_hdr);
	remove_header(inswitch_hdr);
	add_header(clone_hdr);

	//modify_field(eg_intr_md.egress_port, port); // set eport to switchos
	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	//clone_egress_pkt_to_egress(switchos_sid, clone_field_list_for_pktloss); // clone to switchos
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

action forward_cache_pop_inswitch_ack_clone_for_pktloss(switchos_sid) {
	subtract_from_field(clone_hdr.clonenum_for_pktloss, 1);

	//clone_egress_pkt_to_egress(switchos_sid, clone_field_list_for_pktloss); // clone to switchos
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}*/

action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone(switchos_sid, reflector_port) {
	modify_field(op_hdr.optype, CACHE_POP_INSWITCH_ACK);
	modify_field(udp_hdr.dstPort, reflector_port);

	// NOTE: we add/remove vallen and value headers in add_remove_value_header_tbl
	remove_header(shadowtype_hdr);
	remove_header(seq_hdr);
	remove_header(inswitch_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

//action forward_cache_pop_inswitch_ack() {
//}

action update_putreq_seq_inswitch_to_putreq_seq() {
	modify_field(op_hdr.optype, PUTREQ_SEQ);
	modify_field(shadowtype_hdr.shadowtype, PUTREQ_SEQ);

	remove_header(inswitch_hdr);
	//add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_putreq_inswitch_to_netcache_putreq_seq_cached() {
	modify_field(op_hdr.optype, NETCACHE_PUTREQ_SEQ_CACHED);
	modify_field(shadowtype_hdr.shadowtype, NETCACHE_PUTREQ_SEQ_CACHED);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_delreq_seq_inswitch_to_delreq_seq() {
	modify_field(op_hdr.optype, DELREQ_SEQ);
	modify_field(shadowtype_hdr.shadowtype, DELREQ_SEQ);

	remove_header(inswitch_hdr);
	//add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

action update_delreq_inswitch_to_netcache_delreq_seq_cached() {
	modify_field(op_hdr.optype, NETCACHE_DELREQ_SEQ_CACHED);
	modify_field(shadowtype_hdr.shadowtype, NETCACHE_DELREQ_SEQ_CACHED);

	remove_header(inswitch_hdr);
	add_header(seq_hdr);

	//modify_field(eg_intr_md.egress_port, eport);
}

#ifdef RANGE_SUPPORT
action forward_scanreq_split_and_clone(server_sid) {
	modify_field(split_hdr.is_clone, 1);
	add_to_field(split_hdr.cur_scanidx, 1);
	add_to_field(split_hdr.globalserveridx, 1);
	// NOTE: eg_intr_md.egress_port has been set by process_(cloned)_scanreq_split_tbl in stage 0
	clone_egress_pkt_to_egress(server_sid); // clone to server (clone_hdr.server_sid)
}
action forward_scanreq_split() {
	modify_field(split_hdr.is_clone, 1);
	add_to_field(split_hdr.cur_scanidx, 1);
	add_to_field(split_hdr.globalserveridx, 1);
	// NOTE: eg_intr_md.egress_port has been set by process_(cloned)_scanreq_split_tbl in stage 0
}
#endif

action update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone(switchos_sid, reflector_port) {
	modify_field(op_hdr.optype, CACHE_EVICT_LOADFREQ_INSWITCH_ACK);
	modify_field(udp_hdr.dstPort, reflector_port);

	remove_header(shadowtype_hdr);
	remove_header(inswitch_hdr);
	add_header(frequency_hdr);

	modify_field(eg_intr_md_for_oport.drop_ctl, 1); // Disable unicast, but enable mirroring
	clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
}

//action forward_cache_evict_loadfreq_inswitch_ack() {
//}

action update_netcache_valueupdate_inswitch_to_netcache_valueupdate_ack() {
	modify_field(op_hdr.optype, NETCACHE_VALUEUPDATE_ACK);

	remove_header(shadowtype_hdr);
	remove_header(seq_hdr);
	remove_header(inswitch_hdr);
	remove_header(stat_hdr);

	// NOTE: egress_port has already been set in ig_port_forward_tbl at ingress pipeline
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter eg_port_forward_counter {
	type : packets_and_bytes;
	direct: eg_port_forward_tbl;
}
#endif

@pragma stage 9
table eg_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
		meta.is_hot: exact;
		//debug_hdr.is_hot: exact;
		meta.is_report: exact; // only work for GETREQ_INSWITCH
		meta.is_latest: exact;
		meta.is_deleted: exact;
		//inswitch_hdr.is_wrong_pipeline: exact;
		inswitch_hdr.client_sid: exact;
		meta.is_lastclone_for_pktloss: exact;
		//debug_hdr.is_lastclone_for_pktloss: exact;
#ifdef RANGE_SUPPORT
		meta.is_last_scansplit: exact;
#endif
		clone_hdr.server_sid: exact;
	}
	actions {
		update_netcache_warmupreq_inswitch_to_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack;
		forward_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack;
		update_netcache_warmupreq_inswitch_pop_to_warmupack_by_mirroring;
		update_getreq_inswitch_to_getreq;
		update_getreq_inswitch_to_netcache_getreq_pop_clone_for_pktloss_and_getreq;
		forward_netcache_getreq_pop_clone_for_pktloss_and_getreq;
		update_netcache_getreq_pop_to_getreq_by_mirroring;
		update_getreq_inswitch_to_getres_by_mirroring;
		//update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss; // clone for first CACHE_POP_INSWITCH_ACK
		//forward_cache_pop_inswitch_ack_clone_for_pktloss; // not last clone of CACHE_POP_INSWITCH_ACK
		update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone; // clone for first CACHE_POP_INSWITCH_ACK (not need to clone for duplication due to switchos-side timeout-and-retry)
		//forward_cache_pop_inswitch_ack; // last clone of CACHE_POP_INSWITCH_ACK
		update_putreq_seq_inswitch_to_putreq_seq;
		update_putreq_inswitch_to_netcache_putreq_seq_cached;
		update_delreq_seq_inswitch_to_delreq_seq;
		update_delreq_inswitch_to_netcache_delreq_seq_cached;
#ifdef RANGE_SUPPORT
		forward_scanreq_split_and_clone;
		forward_scanreq_split;
#endif
		update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone; // clone to reflector and hence switchos; but not need clone for pktloss due to switchos-side timeout-and-retry
		//forward_cache_evict_loadfreq_inswitch_ack;
		update_netcache_valueupdate_inswitch_to_netcache_valueupdate_ack;
		nop;
	}
	default_action: nop();
	size: 4096;
}

// stage 10

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter update_ipmac_srcport_counter {
	type : packets_and_bytes;
	direct: update_ipmac_srcport_tbl;
}
#endif

// NOTE: we need to set dstip/mac for pkt from server to client especially for those w/ cache hit
action update_srcipmac_srcport_server2client(server_mac, server_ip, server_port) {
	modify_field(ethernet_hdr.srcAddr, server_mac);
	modify_field(ipv4_hdr.srcAddr, server_ip);
	modify_field(udp_hdr.srcPort, server_port);
}

// NOTE: as we use software link, switch_mac/ip = reflector_mac/ip
// NOTE: although we use client_port to update srcport here, reflector does not care about the specific value of srcport
// NOTE: we must reset srcip/mac for pkt from reflector.cp2dpserver, otherwise the ACK pkt will be dropped by reflector.NIC as srcip/mac = NIC.ip/mac
action update_ipmac_srcport_switch2switchos(client_mac, switch_mac, client_ip, switch_ip, client_port) {
	modify_field(ethernet_hdr.srcAddr, client_mac);
	modify_field(ethernet_hdr.dstAddr, switch_mac);
	modify_field(ipv4_hdr.srcAddr, client_ip);
	modify_field(ipv4_hdr.dstAddr, switch_ip);
	modify_field(udp_hdr.srcPort, client_port);
}
// NOTE: for NETCACHE_GETREQ_POP, we need to keep the original srcip/mac, i.e., client ip/mac, which will be used by server for GETRES and by switch for ipv4_forward_tbl
// NOTE: for NETCACHE_WARMUPREQ_INSWITCH_POP, both keep or reset original srcip/mac are ok as src/dst.ip/mac of WARMUPACK will be set by switch based on client_sid/egress_port
action update_dstipmac_switch2switchos(switch_mac, switch_ip) {
	modify_field(ethernet_hdr.dstAddr, switch_mac);
	modify_field(ipv4_hdr.dstAddr, switch_ip);
}

// NOTE: for pkt from client, we should keep original srcip/mac, i.e., client ip/mac, which will be used by server later for response and by switch for ipv4_forward_tbl
action update_dstipmac_client2server(server_mac, server_ip) {
	modify_field(ethernet_hdr.dstAddr, server_mac);
	modify_field(ipv4_hdr.dstAddr, server_ip);
}
// NOTE: for NETCACHE_VALUEUPDATE_ACK, we must reset srcip/mac, i.e., server ip/mac, otherwise it will be ignored by server NIC
action update_ipmac_srcport_client2server(client_mac, server_mac, client_ip, server_ip, client_port) {
	modify_field(ethernet_hdr.srcAddr, client_mac);
	modify_field(ethernet_hdr.dstAddr, server_mac);
	modify_field(ipv4_hdr.srcAddr, client_ip);
	modify_field(ipv4_hdr.dstAddr, server_ip);
	modify_field(udp_hdr.srcPort, client_port);
}

// NOTE: dstport of REQ, RES, and notification has been updated in partition_tbl, server, and eg_port_forward_tbl
@pragma stage 10
table update_ipmac_srcport_tbl {
	reads {
		op_hdr.optype: exact;
		eg_intr_md.egress_port: exact;
	}
	actions {
		update_srcipmac_srcport_server2client; // NOT change dstip and dstmac; use server[0] as srcip and srcmac; use server_worker_port_start as srcport
		update_ipmac_srcport_switch2switchos; // focus on dstip and dstmac to reflector; use client[0] as srcip and srcmac; use constant (123) as srcport
		update_dstipmac_switch2switchos; // keep original srcip/mac for NETCACHE_GETREQ_POP
		update_dstipmac_client2server; // focus on dstip and dstmac to corresponding server; NOT change srcip, srcmac, and srcport
		update_ipmac_srcport_client2server; // reset srcip/mac for NETCACHE_VALUEUPDATE_ACK
		nop;
	}
	default_action: nop();
	size: 256;
}

// stage 11

// NOTE: only one operand in add can be action parameter or constant -> resort to controller to configure different hdrlen
/*
// CACHE_POP_INSWITCH_ACK, GETREQ (cloned by NETCACHE_GETREQ_POP), WARMUPACK (cloned by NETCACHE_WARMUPREQ_INSWITCH_POP), NETCACHE_VALUEUPDATE_ACK
action update_onlyop_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 20(ophdr)
	modify_field(udp_hdr.hdrlen, 28);
	modify_field(ipv4_hdr.totalLen, 48);
}

// GETRES
action update_val_stat_pktlen(aligned_vallen) {
	// 20[iphdr] + 8(udphdr) + 20(ophdr) + 2(vallen) + aligned_vallen(val) + 2(shadowtype) + 4(stat)
	add(udp_hdr.hdrlen, aligned_vallen, 36);
	add(ipv4_hdr.totalLen, aligned_vallen, 56);
}

// PUTREQ_SEQ, NETCACHE_PUTREQ_SEQ_CACHED
action update_val_seq_pktlen(aligned_vallen) {
	// [20(iphdr)] + 8(udphdr) + 20(ophdr) + 2(vallen) + aligned_vallen(val) + 2(shadowtype) + 4(seq)
	add(udp_hdr.hdrlen, aligned_vallen, 36);
	add(ipv4_hdr.totalLen, aligned_vallen, 56);
}

// PUTRES, DELRES (DistCache does NOT need)
action update_stat_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 20(ophdr) + 2(shadowtype) + 4(stat)
	modify_field(udp_hdr.hdrlen, 34);
	modify_field(ipv4_hdr.totalLen, 54);
}

// DELREQ_SEQ, NETCACHE_DELREQ_SEQ_CACHED
action update_seq_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 20(ophdr) + 2(shadowtype) + 4(seq) 
	modify_field(udp_hdr.hdrlen, 34);
	modify_field(ipv4_hdr.totalLen, 54);
}
 
// SCANREQ_SPLIT
action update_scanreqsplit_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 20(ophdr) + 16(endkey) + 11(split_hdr)
	modify_field(udp_hdr.hdrlen, 55);
	modify_field(ipv4_hdr.totalLen, 75);
}
 
// CACHE_EVICT_LOADFREQ_INSWITCH_ACK
action update_frequency_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 20(ophdr) + 4(frequency)
	modify_field(udp_hdr.hdrlen, 32);
	modify_field(ipv4_hdr.totalLen, 52);
}

// NETCACHE_GETREQ_POP
action update_ophdr_clonehdr_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 20(ophdr) + 18(clonehdr)
	modify_field(udp_hdr.hdrlen, 46);
	modify_field(ipv4_hdr.totalLen, 66);
}

// NETCACHE_WARMUPREQ_INSWITCH_POP
action update_ophdr_inswitchhdr_clonehdr_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 20(ophdr) + 2(shadowtype) + 28(inswitchhdr) + 18(clonehdr)
	modify_field(udp_hdr.hdrlen, 76);
	modify_field(ipv4_hdr.totalLen, 96);
}
*/

action update_pktlen(udplen, iplen) {
	modify_field(udp_hdr.hdrlen, udplen);
	modify_field(ipv4_hdr.totalLen, iplen);
}

@pragma stage 11
table update_pktlen_tbl {
	reads {
		op_hdr.optype: exact;
		vallen_hdr.vallen: range;
	}
	actions {
		/*update_onlyop_pktlen;
		update_val_stat_pktlen;
		update_val_seq_inswitch_stat_pktlen;
		update_val_seq_pktlen;
		update_stat_pktlen;
		update_seq_pktlen;*/
		update_pktlen;
		nop;
	}
	default_action: nop(); // not change udp_hdr.hdrlen (GETREQ/GETREQ_POP/GETREQ_NLATEST)
	size: 256;
}

action add_only_vallen() {
	add_header(vallen_hdr);
	remove_header(val1_hdr);
	remove_header(val2_hdr);
	remove_header(val3_hdr);
	remove_header(val4_hdr);
	remove_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val1() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	remove_header(val2_hdr);
	remove_header(val3_hdr);
	remove_header(val4_hdr);
	remove_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val2() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	remove_header(val3_hdr);
	remove_header(val4_hdr);
	remove_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val3() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	remove_header(val4_hdr);
	remove_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val4() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	remove_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val5() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val6() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val7() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val8() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	add_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val9() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	add_header(val8_hdr);
	add_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val10() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	add_header(val8_hdr);
	add_header(val9_hdr);
	add_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val11() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	add_header(val8_hdr);
	add_header(val9_hdr);
	add_header(val10_hdr);
	add_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val12() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	add_header(val8_hdr);
	add_header(val9_hdr);
	add_header(val10_hdr);
	add_header(val11_hdr);
	add_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val13() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	add_header(val8_hdr);
	add_header(val9_hdr);
	add_header(val10_hdr);
	add_header(val11_hdr);
	add_header(val12_hdr);
	add_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val14() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	add_header(val8_hdr);
	add_header(val9_hdr);
	add_header(val10_hdr);
	add_header(val11_hdr);
	add_header(val12_hdr);
	add_header(val13_hdr);
	add_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val15() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	add_header(val8_hdr);
	add_header(val9_hdr);
	add_header(val10_hdr);
	add_header(val11_hdr);
	add_header(val12_hdr);
	add_header(val13_hdr);
	add_header(val14_hdr);
	add_header(val15_hdr);
	remove_header(val16_hdr);
}

action add_to_val16() {
	add_header(vallen_hdr);
	add_header(val1_hdr);
	add_header(val2_hdr);
	add_header(val3_hdr);
	add_header(val4_hdr);
	add_header(val5_hdr);
	add_header(val6_hdr);
	add_header(val7_hdr);
	add_header(val8_hdr);
	add_header(val9_hdr);
	add_header(val10_hdr);
	add_header(val11_hdr);
	add_header(val12_hdr);
	add_header(val13_hdr);
	add_header(val14_hdr);
	add_header(val15_hdr);
	add_header(val16_hdr);
}

action remove_all() {
	remove_header(vallen_hdr);
	remove_header(val1_hdr);
	remove_header(val2_hdr);
	remove_header(val3_hdr);
	remove_header(val4_hdr);
	remove_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);
}

@pragma stage 11
table add_and_remove_value_header_tbl {
	reads {
		op_hdr.optype: exact;
		vallen_hdr.vallen: range;
	}
	actions {
		add_only_vallen;
		add_to_val1;
		add_to_val2;
		add_to_val3;
		add_to_val4;
		add_to_val5;
		add_to_val6;
		add_to_val7;
		add_to_val8;
		add_to_val9;
		add_to_val10;
		add_to_val11;
		add_to_val12;
		add_to_val13;
		add_to_val14;
		add_to_val15;
		add_to_val16;
		remove_all;
	}
	default_action: remove_all();
	size: 256;
}
