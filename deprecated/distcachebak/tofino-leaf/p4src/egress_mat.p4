/* Ingress Processing (Normal Operation) */

// Stage 0


// Stage 3

action set_is_hot() {
	meta.is_hot = 1;
	//debug_hdr.is_hot = 1;
}

action reset_is_hot() {
	meta.is_hot = 0;
	//debug_hdr.is_hot = 0;
}

@pragma stage 3
table is_hot_tbl {
	key = {
		meta.cm1_predicate: exact;
		meta.cm2_predicate: exact;
		meta.cm3_predicate: exact;
		meta.cm4_predicate: exact;
	}
	actions = {
		set_is_hot;
		reset_is_hot;
	}
	default_action= reset_is_hot();
	size = 1;
}

// Stage 4

action set_server_sid_udpport_and_save_client_info(bit<10> server_sid) {
	hdr.clone_hdr.server_sid = server_sid;
	hdr.clone_hdr.server_udpport = hdr.udp_hdr.dstPort; // dstport is serverport for GETREQ_INSWITCH;

	hdr.clone_hdr.client_ip = hdr.ipv4_hdr.srcAddr;
	hdr.clone_hdr.client_mac = hdr.ethernet_hdr.srcAddr;
	hdr.clone_hdr.client_udpport = hdr.udp_hdr.srcPort;
}

action reset_server_sid() {
	hdr.clone_hdr.server_sid = 0;
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter prepare_for_cachepop_and_save_client_info_counter {
	type : packets_and_bytes;
	direct: prepare_for_cachepop_and_save_client_info_tbl;
}
#endif

@pragma stage 4
table prepare_for_cachepop_and_save_client_info_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		standard_metadata.egress_port: exact;
	}
	actions = {
		set_server_sid_udpport_and_save_client_info;
		reset_server_sid;
		NoAction;
	}
	default_action= reset_server_sid();
	size = 32;
}

// Stage 7

action set_is_report() {
	meta.is_report = 1;
}

action reset_is_report() {
	meta.is_report = 0;
}

@pragma stage 7
table is_report_tbl {
	key = {
		meta.is_report1: exact;
		meta.is_report2: exact;
		//meta.is_report3: exact;
	}
	actions = {
		set_is_report;
		reset_is_report;
	}
	default_action= reset_is_report();
	size = 1;
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
	meta.is_lastclone_for_pktloss = 1;
	//debug_hdr.is_lastclone_for_pktloss = 1;
}

action reset_is_lastclone_lastscansplit() {
	meta.is_lastclone_for_pktloss = 0;
	//debug_hdr.is_lastclone_for_pktloss = 0;
}

@pragma stage 8
table lastclone_lastscansplit_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.clone_hdr.clonenum_for_pktloss: exact;

	}
	actions = {
		set_is_lastclone;
		reset_is_lastclone_lastscansplit;
	}
	default_action= reset_is_lastclone_lastscansplit();
	size = 8;
}

// Stage 9

action update_getreq_inswitch_to_getreq() {
	hdr.op_hdr.optype = GETREQ;

	hdr.shadowtype_hdr.shadowtype = GETREQ;
	//hdr.shadowtype_hdr.setInvalid();

	hdr.inswitch_hdr.setInvalid();
	//hdr.switchload_hdr.setInvalid();

	//standard_metadata.egress_port = eport;
}

action update_getreq_inswitch_to_netcache_getreq_pop_clone_for_pktloss_and_getreq(bit<10> switchos_sid,bit<16> reflector_port) {
	hdr.op_hdr.optype = NETCACHE_GETREQ_POP;
	hdr.udp_hdr.dstPort = reflector_port;
	hdr.clone_hdr.clonenum_for_pktloss = 3; // 3 ACKs (drop w/ 3 -> clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> drop and clone for GETREQ to server);

	hdr.shadowtype_hdr.shadowtype = NETCACHE_GETREQ_POP;
	//hdr.shadowtype_hdr.setInvalid();

	hdr.inswitch_hdr.setInvalid();
	//hdr.switchload_hdr.setInvalid();
	hdr.clone_hdr.setValid(); // NOTE: hdr.clone_hdr.server_sid has been set in prepare_for_cachepop_tbl

	//standard_metadata.egress_port = port // set eport to switchos;
	mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring;
	clone(CloneType.E2E, (bit<32>)switchos_sid); // clone to switchos
}

action forward_netcache_getreq_pop_clone_for_pktloss_and_getreq(bit<10> switchos_sid) {
	hdr.clone_hdr.clonenum_for_pktloss = hdr.clone_hdr.clonenum_for_pktloss -  1;

	clone(CloneType.E2E, (bit<32>)switchos_sid); // clone to switchos
}

action update_netcache_getreq_pop_to_getreq_by_mirroring(bit<10> server_sid) {
	hdr.op_hdr.optype = GETREQ;
	// Keep original udp.srcport (aka client udp port)
	hdr.udp_hdr.dstPort = hdr.clone_hdr.server_udpport;

	hdr.shadowtype_hdr.shadowtype = GETREQ;

	hdr.clone_hdr.setInvalid();
	// NOTE: hold switchload_hdr from NETCACHE_GETREQ_POP for GETREQ

	mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring;
	clone(CloneType.E2E, (bit<32>)server_sid); // clone to client (hdr.inswitch_hdr.client_sid)
}

action update_getreq_inswitch_to_getres_by_mirroring(bit<10> client_sid,bit<16> server_port,bit<8> stat) {
	hdr.op_hdr.optype = GETRES;
	hdr.shadowtype_hdr.shadowtype = GETRES;
	hdr.stat_hdr.stat = stat;
	hdr.stat_hdr.nodeidx_foreval = SWITCHIDX_FOREVAL;

	// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
	hdr.udp_hdr.srcPort = server_port;
	hdr.ipv4_hdr.dstAddr = hdr.clone_hdr.client_ip;
	hdr.ethernet_hdr.dstAddr = hdr.clone_hdr.client_mac;
	hdr.udp_hdr.dstPort = hdr.clone_hdr.client_udpport;

	hdr.inswitch_hdr.setInvalid();
	hdr.stat_hdr.setValid();
	// NOTE: hold switchload_hdr from GETREQ_INSWITCH for GETRES

	mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring;
	clone(CloneType.E2E, (bit<32>)client_sid); // clone to client (hdr.inswitch_hdr.client_sid)
}


action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone(bit<10> switchos_sid,bit<16> reflector_port) {
	hdr.op_hdr.optype = CACHE_POP_INSWITCH_ACK;
	hdr.udp_hdr.dstPort = reflector_port;

	// NOTE: we add/remove vallen and value headers in add_remove_value_header_tbl
	hdr.shadowtype_hdr.setInvalid();
	hdr.seq_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();
	hdr.stat_hdr.setInvalid();

	mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring;
	clone(CloneType.E2E, (bit<32>)switchos_sid); // clone to switchos
}

//action forward_cache_pop_inswitch_ack() {
//}

action update_putreq_seq_inswitch_to_putreq_seq() {
	hdr.op_hdr.optype = PUTREQ_SEQ;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_SEQ;

	hdr.inswitch_hdr.setInvalid();
	//hdr.seq_hdr.setValid();

	//standard_metadata.egress_port = eport;
}

action update_putreq_inswitch_to_netcache_putreq_seq_cached() {
	hdr.op_hdr.optype = NETCACHE_PUTREQ_SEQ_CACHED;
	hdr.shadowtype_hdr.shadowtype = NETCACHE_PUTREQ_SEQ_CACHED;

	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();

	//standard_metadata.egress_port = eport;
}

action update_delreq_seq_inswitch_to_delreq_seq() {
	hdr.op_hdr.optype = DELREQ_SEQ;
	hdr.shadowtype_hdr.shadowtype = DELREQ_SEQ;

	hdr.inswitch_hdr.setInvalid();
	//hdr.seq_hdr.setValid();

	//standard_metadata.egress_port = eport;
}

action update_delreq_inswitch_to_netcache_delreq_seq_cached() {
	hdr.op_hdr.optype = NETCACHE_DELREQ_SEQ_CACHED;
	hdr.shadowtype_hdr.shadowtype = NETCACHE_DELREQ_SEQ_CACHED;

	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();

	//standard_metadata.egress_port = eport;
}


action update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone(bit<10> switchos_sid,bit<16> reflector_port) {
	hdr.op_hdr.optype = CACHE_EVICT_LOADFREQ_INSWITCH_ACK;
	hdr.udp_hdr.dstPort = reflector_port;

	hdr.shadowtype_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();
	hdr.frequency_hdr.setValid();

	mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring;
	clone(CloneType.E2E, (bit<32>)switchos_sid); // clone to switchos
}



action update_putreq_largevalue_seq_inswitch_to_putreq_largevalue_seq() {
	// NOTE: PUTREQ_LARGEVALUE_SEQ_INSWITCH w/ op_hdr + shadowtype_hdr + seq_hdr + inswitch_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr
	hdr.op_hdr.optype = PUTREQ_LARGEVALUE_SEQ;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_SEQ;

	hdr.inswitch_hdr.setInvalid();
}

action update_putreq_largevalue_seq_inswitch_to_putreq_largevalue_seq_cached() {
	// NOTE: PUTREQ_LARGEVALUE_SEQ_INSWITCH w/ op_hdr + shadowtype_hdr + seq_hdr + inswitch_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ_CACHED w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr
	hdr.op_hdr.optype = PUTREQ_LARGEVALUE_SEQ_CACHED;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_SEQ_CACHED;

	hdr.inswitch_hdr.setInvalid();
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
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.is_hot: exact;
		//debug_hdr.is_hot: exact;
		meta.is_report: exact; // only work for GETREQ_INSWITCH
		meta.is_latest: exact;
		meta.is_deleted: exact;
		//hdr.inswitch_hdr.is_wrong_pipeline: exact;
		hdr.inswitch_hdr.client_sid: exact;
		meta.is_lastclone_for_pktloss: exact;
		//debug_hdr.is_lastclone_for_pktloss: exact;

		hdr.clone_hdr.server_sid: exact;
	}
	actions = {
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

		update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone; // clone to reflector and hence switchos; but not need clone for pktloss due to switchos-side timeout-and-retry
		//forward_cache_evict_loadfreq_inswitch_ack;
		//update_distcache_leaf_valueupdate_inswitch_to_distcache_leaf_valueupdate_inswitch_ack;
		update_putreq_largevalue_seq_inswitch_to_putreq_largevalue_seq;
		update_putreq_largevalue_seq_inswitch_to_putreq_largevalue_seq_cached;
		NoAction;
	}
	default_action= NoAction();
	size = 1024;
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
action update_srcipmac_srcport_server2client(bit<48> server_mac, bit<32> server_ip, bit<16> server_port) {
	hdr.ethernet_hdr.srcAddr = server_mac;
	hdr.ipv4_hdr.srcAddr = server_ip;
	hdr.udp_hdr.srcPort = server_port;
}

// NOTE: as we use software link, bit<48> switch_mac/ip = reflector_mac/ip
// NOTE: although we use client_port to update srcport here, reflector does not care about the specific value of srcport
// NOTE: we must reset srcip/mac for pkt from reflector.cp2dpserver, otherwise the ACK pkt will be dropped by reflector.NIC as srcip/mac = NIC.ip/mac
action update_ipmac_srcport_switch2switchos(bit<48> client_mac, bit<48> switch_mac, bit<32> client_ip, bit<32> switch_ip, bit<16> client_port) {
	hdr.ethernet_hdr.srcAddr = client_mac;
	hdr.ethernet_hdr.dstAddr = switch_mac;
	hdr.ipv4_hdr.srcAddr = client_ip;
	hdr.ipv4_hdr.dstAddr = switch_ip;
	hdr.udp_hdr.srcPort = client_port;
}
// NOTE: for NETCACHE_GETREQ_POP, we need to keep the original srcip/mac, i.e., client ip/mac, which will be used by server for GETRES and by switch for ipv4_forward_tbl
// NOTE: for NETCACHE_WARMUPREQ_INSWITCH_POP, both keep or reset original srcip/mac are ok as src/dst.ip/mac of WARMUPACK will be set by switch based on client_sid/egress_port
action update_dstipmac_switch2switchos(bit<48> switch_mac, bit<32> switch_ip) {
	hdr.ethernet_hdr.dstAddr = switch_mac;
	hdr.ipv4_hdr.dstAddr = switch_ip;
}

// NOTE: for pkt from client, we should keep original srcip/mac, i.e., client ip/mac, which will be used by server later for response and by switch for ipv4_forward_tbl
action update_dstipmac_client2server(bit<48> server_mac, bit<32> server_ip) {
	hdr.ethernet_hdr.dstAddr = server_mac;
	hdr.ipv4_hdr.dstAddr = server_ip;
}
// NOTE: for NETCACHE_VALUEUPDATE_ACK, we must reset srcip/mac, i.e., server ip/mac, otherwise it will be ignored by server NIC
action update_ipmac_srcport_client2server(bit<48> client_mac, bit<48> server_mac, bit<32> client_ip, bit<32> server_ip, bit<16> client_port) {
	hdr.ethernet_hdr.srcAddr = client_mac;
	hdr.ethernet_hdr.dstAddr = server_mac;
	hdr.ipv4_hdr.srcAddr = client_ip;
	hdr.ipv4_hdr.dstAddr = server_ip;
	hdr.udp_hdr.srcPort = client_port;
}

// NOTE: dstport of REQ, RES, and notification has been updated in partition_tbl, server, and eg_port_forward_tbl
@pragma stage 10
table update_ipmac_srcport_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		standard_metadata.egress_port: exact;
	}
	actions = {
		update_srcipmac_srcport_server2client; // NOT change dstip and dstmac; use server[0] as srcip and srcmac; use server_worker_port_start as srcport
		update_ipmac_srcport_switch2switchos; // focus on dstip and dstmac to reflector; use client[0] as srcip and srcmac; use constant (123) as srcport
		update_dstipmac_switch2switchos; // keep original srcip/mac for NETCACHE_GETREQ_POP
		update_dstipmac_client2server; // focus on dstip and dstmac to corresponding server; NOT change srcip, srcmac, and srcport
		update_ipmac_srcport_client2server; // reset srcip/mac for NETCACHE_VALUEUPDATE_ACK
		NoAction;
	}
	default_action= NoAction();
	size = 256;
}

// stage 11

// NOTE: only one operand in add can be action parameter or constant -> resort to controller to configure different hdrlen


action update_pktlen(bit<16> udplen,bit<16> iplen) {
	hdr.udp_hdr.hdrlen = udplen;
	hdr.ipv4_hdr.totalLen = iplen;
}

@pragma stage 11
table update_pktlen_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.vallen_hdr.vallen: range;
	}
	actions = {
		/*update_onlyop_pktlen;
		update_val_stat_pktlen;
		update_val_seq_inswitch_stat_pktlen;
		update_val_seq_pktlen;
		update_stat_pktlen;
		update_seq_pktlen;*/
		update_pktlen;
		NoAction;
	}
	default_action= NoAction(); // not change hdr.udp_hdr.hdrlen (GETREQ/GETREQ_POP/GETREQ_NLATEST)
	size = 256;
}

action add_only_vallen() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setInvalid();
	hdr.val2_hdr.setInvalid();
	hdr.val3_hdr.setInvalid();
	hdr.val4_hdr.setInvalid();
	hdr.val5_hdr.setInvalid();
	hdr.val6_hdr.setInvalid();
	hdr.val7_hdr.setInvalid();
	hdr.val8_hdr.setInvalid();
	hdr.val9_hdr.setInvalid();
	hdr.val10_hdr.setInvalid();
	hdr.val11_hdr.setInvalid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val1() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setInvalid();
	hdr.val3_hdr.setInvalid();
	hdr.val4_hdr.setInvalid();
	hdr.val5_hdr.setInvalid();
	hdr.val6_hdr.setInvalid();
	hdr.val7_hdr.setInvalid();
	hdr.val8_hdr.setInvalid();
	hdr.val9_hdr.setInvalid();
	hdr.val10_hdr.setInvalid();
	hdr.val11_hdr.setInvalid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val2() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setInvalid();
	hdr.val4_hdr.setInvalid();
	hdr.val5_hdr.setInvalid();
	hdr.val6_hdr.setInvalid();
	hdr.val7_hdr.setInvalid();
	hdr.val8_hdr.setInvalid();
	hdr.val9_hdr.setInvalid();
	hdr.val10_hdr.setInvalid();
	hdr.val11_hdr.setInvalid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val3() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setInvalid();
	hdr.val5_hdr.setInvalid();
	hdr.val6_hdr.setInvalid();
	hdr.val7_hdr.setInvalid();
	hdr.val8_hdr.setInvalid();
	hdr.val9_hdr.setInvalid();
	hdr.val10_hdr.setInvalid();
	hdr.val11_hdr.setInvalid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val4() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setInvalid();
	hdr.val6_hdr.setInvalid();
	hdr.val7_hdr.setInvalid();
	hdr.val8_hdr.setInvalid();
	hdr.val9_hdr.setInvalid();
	hdr.val10_hdr.setInvalid();
	hdr.val11_hdr.setInvalid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val5() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setValid();
	hdr.val6_hdr.setInvalid();
	hdr.val7_hdr.setInvalid();
	hdr.val8_hdr.setInvalid();
	hdr.val9_hdr.setInvalid();
	hdr.val10_hdr.setInvalid();
	hdr.val11_hdr.setInvalid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val6() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setValid();
	hdr.val6_hdr.setValid();
	hdr.val7_hdr.setInvalid();
	hdr.val8_hdr.setInvalid();
	hdr.val9_hdr.setInvalid();
	hdr.val10_hdr.setInvalid();
	hdr.val11_hdr.setInvalid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val7() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setValid();
	hdr.val6_hdr.setValid();
	hdr.val7_hdr.setValid();
	hdr.val8_hdr.setInvalid();
	hdr.val9_hdr.setInvalid();
	hdr.val10_hdr.setInvalid();
	hdr.val11_hdr.setInvalid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val8() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setValid();
	hdr.val6_hdr.setValid();
	hdr.val7_hdr.setValid();
	hdr.val8_hdr.setValid();
	hdr.val9_hdr.setInvalid();
	hdr.val10_hdr.setInvalid();
	hdr.val11_hdr.setInvalid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val9() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setValid();
	hdr.val6_hdr.setValid();
	hdr.val7_hdr.setValid();
	hdr.val8_hdr.setValid();
	hdr.val9_hdr.setValid();
	hdr.val10_hdr.setInvalid();
	hdr.val11_hdr.setInvalid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val10() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setValid();
	hdr.val6_hdr.setValid();
	hdr.val7_hdr.setValid();
	hdr.val8_hdr.setValid();
	hdr.val9_hdr.setValid();
	hdr.val10_hdr.setValid();
	hdr.val11_hdr.setInvalid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val11() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setValid();
	hdr.val6_hdr.setValid();
	hdr.val7_hdr.setValid();
	hdr.val8_hdr.setValid();
	hdr.val9_hdr.setValid();
	hdr.val10_hdr.setValid();
	hdr.val11_hdr.setValid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val12() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setValid();
	hdr.val6_hdr.setValid();
	hdr.val7_hdr.setValid();
	hdr.val8_hdr.setValid();
	hdr.val9_hdr.setValid();
	hdr.val10_hdr.setValid();
	hdr.val11_hdr.setValid();
	hdr.val12_hdr.setValid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val13() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setValid();
	hdr.val6_hdr.setValid();
	hdr.val7_hdr.setValid();
	hdr.val8_hdr.setValid();
	hdr.val9_hdr.setValid();
	hdr.val10_hdr.setValid();
	hdr.val11_hdr.setValid();
	hdr.val12_hdr.setValid();
	hdr.val13_hdr.setValid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val14() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setValid();
	hdr.val6_hdr.setValid();
	hdr.val7_hdr.setValid();
	hdr.val8_hdr.setValid();
	hdr.val9_hdr.setValid();
	hdr.val10_hdr.setValid();
	hdr.val11_hdr.setValid();
	hdr.val12_hdr.setValid();
	hdr.val13_hdr.setValid();
	hdr.val14_hdr.setValid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val15() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setValid();
	hdr.val6_hdr.setValid();
	hdr.val7_hdr.setValid();
	hdr.val8_hdr.setValid();
	hdr.val9_hdr.setValid();
	hdr.val10_hdr.setValid();
	hdr.val11_hdr.setValid();
	hdr.val12_hdr.setValid();
	hdr.val13_hdr.setValid();
	hdr.val14_hdr.setValid();
	hdr.val15_hdr.setValid();
	hdr.val16_hdr.setInvalid();
}

action add_to_val16() {
	hdr.vallen_hdr.setValid();
	hdr.val1_hdr.setValid();
	hdr.val2_hdr.setValid();
	hdr.val3_hdr.setValid();
	hdr.val4_hdr.setValid();
	hdr.val5_hdr.setValid();
	hdr.val6_hdr.setValid();
	hdr.val7_hdr.setValid();
	hdr.val8_hdr.setValid();
	hdr.val9_hdr.setValid();
	hdr.val10_hdr.setValid();
	hdr.val11_hdr.setValid();
	hdr.val12_hdr.setValid();
	hdr.val13_hdr.setValid();
	hdr.val14_hdr.setValid();
	hdr.val15_hdr.setValid();
	hdr.val16_hdr.setValid();
}

action remove_all() {
	hdr.vallen_hdr.setInvalid();
	hdr.val1_hdr.setInvalid();
	hdr.val2_hdr.setInvalid();
	hdr.val3_hdr.setInvalid();
	hdr.val4_hdr.setInvalid();
	hdr.val5_hdr.setInvalid();
	hdr.val6_hdr.setInvalid();
	hdr.val7_hdr.setInvalid();
	hdr.val8_hdr.setInvalid();
	hdr.val9_hdr.setInvalid();
	hdr.val10_hdr.setInvalid();
	hdr.val11_hdr.setInvalid();
	hdr.val12_hdr.setInvalid();
	hdr.val13_hdr.setInvalid();
	hdr.val14_hdr.setInvalid();
	hdr.val15_hdr.setInvalid();
	hdr.val16_hdr.setInvalid();
}

@pragma stage 11
table add_and_remove_value_header_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.vallen_hdr.vallen: range;
	}
	actions = {
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
	default_action= remove_all();
	size = 256;
}

action drop_distcache_invalidate_inswitch() {
	mark_to_drop(standard_metadata);
}

/*action drop_netcache_valueupdate_inswitch() {
	mark_to_drop(standard_metadata);
}*/

action drop_distcache_valueupdate_inswitch_origin() {
	mark_to_drop(standard_metadata);
}

@pragma stage 11
table drop_tbl {
	key = {
		hdr.op_hdr.optype: exact;
	}
	actions = {
		drop_distcache_invalidate_inswitch;
		//drop_netcache_valueupdate_inswitch;
		drop_distcache_valueupdate_inswitch_origin;
		NoAction;
	}
	default_action= NoAction();
	size = 2;
}
