/* Ingress Processing (Normal Operation) */

// Stage 0

action save_client_info() {
	hdr.clone_hdr.client_ip = hdr.ipv4_hdr.srcAddr;
	hdr.clone_hdr.client_mac = hdr.ethernet_hdr.srcAddr;
	hdr.clone_hdr.client_udpport = hdr.udp_hdr.srcPort;
}

@pragma stage 0
table save_client_info_tbl {
	key = {
		hdr.op_hdr.optype: exact;
	}
	actions = {
		save_client_info;
		NoAction;
	}
	default_action= NoAction();
	size = 4;
}
// Stage 1

action set_server_sid_and_port(bit<10> server_sid) {
	hdr.clone_hdr.server_sid = server_sid;
	hdr.clone_hdr.server_udpport = hdr.udp_hdr.dstPort; // dstport is serverport for GETREQ_INSWITCH;
}

action reset_server_sid() {
	hdr.clone_hdr.server_sid = 0;
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
	key = {
		hdr.op_hdr.optype: exact;
		standard_metadata.egress_port: exact;
	}
	actions = {
		set_server_sid_and_port;
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
		meta.is_report3: exact;
	}
	actions = {
		set_is_report;
		reset_is_report;
	}
	default_action= reset_is_report();
	size = 1;
}

// Stage 8


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

action update_netcache_warmupreq_inswitch_to_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack(bit<10> switchos_sid,bit<16> reflector_port) {
	hdr.op_hdr.optype = NETCACHE_WARMUPREQ_INSWITCH_POP;
	hdr.shadowtype_hdr.shadowtype = NETCACHE_WARMUPREQ_INSWITCH_POP;
	hdr.udp_hdr.dstPort = reflector_port;
	hdr.clone_hdr.clonenum_for_pktloss = 3; // 3 ACKs (drop w/ 3 -> clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> drop and clone for WARMUPACK to client);

	hdr.clone_hdr.setValid(); // NOTE: hdr.clone_hdr.server_sid is reset as 0 in process_scanreq_split_tbl and prepare_for_cachepop_tbl

	//standard_metadata.egress_port = port // set eport to switchos;
	mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring;
	clone(CloneType.E2E, (bit<32>)switchos_sid); // clone to switchos
}

action forward_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack(bit<10> switchos_sid) {
	hdr.clone_hdr.clonenum_for_pktloss = hdr.clone_hdr.clonenum_for_pktloss -  1;

	clone(CloneType.E2E, (bit<32>)switchos_sid); // clone to switchos
}

action update_netcache_warmupreq_inswitch_pop_to_warmupack_by_mirroring(bit<10> client_sid,bit<16> server_port) {
	hdr.op_hdr.optype = WARMUPACK;
	// DEPRECATED: udp.srcport will be set as server_worker_port_start in update_ipmac_srcport_tbl
	// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports

	// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
	hdr.udp_hdr.srcPort = server_port;
	hdr.ipv4_hdr.dstAddr = hdr.clone_hdr.client_ip;
	hdr.ethernet_hdr.dstAddr = hdr.clone_hdr.client_mac;
	hdr.udp_hdr.dstPort = hdr.clone_hdr.client_udpport;

	hdr.shadowtype_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();
	hdr.clone_hdr.setInvalid();

	mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring;
	clone(CloneType.E2E, (bit<32>)client_sid); // clone to client (hdr.inswitch_hdr.client_sid)
}

action update_getreq_inswitch_to_getreq_spine() {
	hdr.op_hdr.optype = GETREQ_SPINE;

	hdr.shadowtype_hdr.shadowtype = GETREQ_SPINE;
	//hdr.shadowtype_hdr.setInvalid();

	hdr.inswitch_hdr.setInvalid();
	//hdr.switchload_hdr.setInvalid();
}

//action update_getreq_inswitch_to_distcache_getres_spine_by_mirroring(bit<10> client_sid,bit<16> server_port,bit<8> stat) {
action update_getreq_inswitch_to_getres_by_mirroring(bit<10> client_sid,bit<16> server_port,bit<8> stat) {
	//hdr.op_hdr.optype = DISTCACHE_GETRES_SPINE;
	//hdr.shadowtype_hdr.shadowtype = DISTCACHE_GETRES_SPINE;
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
	//// NOTE: hold switchload_hdr from GETREQ_INSWITCH in DISTCACHE_GETRES_SPINE
	// NOTE: hold switchload_hdr from GETREQ_INSWITCH in GETRES

	mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring;
	clone(CloneType.E2E, (bit<32>)client_sid); // clone to client (hdr.inswitch_hdr.client_sid)
}

/*action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss(bit<10> switchos_sid,bit<16> reflector_port) {
	hdr.op_hdr.optype = CACHE_POP_INSWITCH_ACK;
	hdr.udp_hdr.dstPort = reflector_port;
	//meta.clonenum_for_pktloss = 1 // 3 ACKs (clone w/ 1 -> clone w/ 0 -> no clone w/ ack);
	hdr.clone_hdr.clonenum_for_pktloss = 2 // 3 ACKs (drop w/ 2 -> clone w/ 1 -> clone w/ 0 -> no clone w/ ack);

	//hdr.vallen_hdr.setInvalid();
	//hdr.val1_hdr.setInvalid();
	//hdr.val2_hdr.setInvalid();
	//hdr.val3_hdr.setInvalid();
	//hdr.val4_hdr.setInvalid();
	//hdr.val5_hdr.setInvalid();
	//hdr.val6_hdr.setInvalid();
	//hdr.val7_hdr.setInvalid();
	//hdr.val8_hdr.setInvalid();
	//hdr.val9_hdr.setInvalid();
	//hdr.val10_hdr.setInvalid();
	//hdr.val11_hdr.setInvalid();
	//hdr.val12_hdr.setInvalid();
	//hdr.val13_hdr.setInvalid();
	//hdr.val14_hdr.setInvalid();
	//hdr.val15_hdr.setInvalid();
	//hdr.val16_hdr.setInvalid();
	hdr.shadowtype_hdr.setInvalid();
	hdr.seq_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();
	hdr.clone_hdr.setValid();

	//standard_metadata.egress_port = port // set eport to switchos;
	mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring;
	//clone(CloneType.E2E, (bit<32>)switchos_sid, clone_field_list_for_pktloss); // clone to switchos
	clone(CloneType.E2E, (bit<32>)switchos_sid); // clone to switchos
}

action forward_cache_pop_inswitch_ack_clone_for_pktloss(bit<10> switchos_sid) {
	hdr.clone_hdr.clonenum_for_pktloss = hdr.clone_hdr.clonenum_for_pktloss -  1;

	//clone(CloneType.E2E, (bit<32>)switchos_sid, clone_field_list_for_pktloss); // clone to switchos
	clone(CloneType.E2E, (bit<32>)switchos_sid); // clone to switchos
}*/

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

action update_putreq_inswitch_to_putreq_seq() {
	hdr.op_hdr.optype = PUTREQ_SEQ;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_SEQ;

	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();

	//standard_metadata.egress_port = eport;
}

action update_putreq_inswitch_to_netcache_putreq_seq_cached() {
	hdr.op_hdr.optype = NETCACHE_PUTREQ_SEQ_CACHED;
	hdr.shadowtype_hdr.shadowtype = NETCACHE_PUTREQ_SEQ_CACHED;

	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();

	//standard_metadata.egress_port = eport;
}

action update_delreq_inswitch_to_delreq_seq() {
	hdr.op_hdr.optype = DELREQ_SEQ;
	hdr.shadowtype_hdr.shadowtype = DELREQ_SEQ;

	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();

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

//action forward_cache_evict_loadfreq_inswitch_ack() {
//}

/*action update_netcache_valueupdate_inswitch_to_netcache_valueupdate_ack() {
	hdr.op_hdr.optype = NETCACHE_VALUEUPDATE_ACK;

	hdr.shadowtype_hdr.setInvalid();
	hdr.seq_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();
	hdr.stat_hdr.setInvalid();

	// NOTE: egress_port has already been set in hash/range_partition_tbl at ingress pipeline
}*/

/*action update_distcache_spine_valueupdate_inswitch_to_distcache_spine_valueupdate_inswitch_ack() {
	hdr.op_hdr.optype = DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK;

	hdr.shadowtype_hdr.setInvalid();
	hdr.seq_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();
	hdr.stat_hdr.setInvalid();

	// NOTE: egress_port has already been set in hash/range_partition_tbl at ingress pipeline
}*/

action update_distcache_valueupdate_inswitch_origin_to_distcache_valueupdate_inswitch_ack() {
	hdr.op_hdr.optype = DISTCACHE_VALUEUPDATE_INSWITCH_ACK;
	hdr.shadowtype_hdr.shadowtype = DISTCACHE_VALUEUPDATE_INSWITCH_ACK;

	// NOTE: hold shadowtype_hdr and inswitch_hdr for debugging

	//hdr.shadowtype_hdr.setInvalid();
	hdr.seq_hdr.setInvalid();
	//hdr.inswitch_hdr.setInvalid();
	hdr.stat_hdr.setInvalid();

	// NOTE: egress_port has already been set in hash/range_partition_tbl at ingress pipeline
}

action update_distcache_invalidate_inswitch_to_distcache_invalidate_ack() {
	hdr.op_hdr.optype = DISTCACHE_INVALIDATE_ACK;

	hdr.shadowtype_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();

	// NOTE: egress_port/udp_dstport has already been set in partition_tbl/ig_port_forward_tbl at ingress pipeline
}

action update_putreq_largevalue_inswitch_to_putreq_largevalue_seq() {
	// NOTE: PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr
	hdr.op_hdr.optype = PUTREQ_LARGEVALUE_SEQ;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_SEQ;

	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();
}

action update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_cached() {
	// NOTE: PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ_CACHED w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr
	hdr.op_hdr.optype = PUTREQ_LARGEVALUE_SEQ_CACHED;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_SEQ_CACHED;

	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();
}


@pragma stage 9
table eg_port_forward_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
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
		update_netcache_warmupreq_inswitch_to_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack;
		forward_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack;
		update_netcache_warmupreq_inswitch_pop_to_warmupack_by_mirroring;
		update_getreq_inswitch_to_getreq_spine;
		//update_getreq_inswitch_to_distcache_getres_spine_by_mirroring;
		update_getreq_inswitch_to_getres_by_mirroring;
		//update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss; // clone for first CACHE_POP_INSWITCH_ACK
		//forward_cache_pop_inswitch_ack_clone_for_pktloss; // not last clone of CACHE_POP_INSWITCH_ACK
		update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone; // clone for first CACHE_POP_INSWITCH_ACK (not need to clone for duplication due to switchos-side timeout-and-retry)
		//forward_cache_pop_inswitch_ack; // last clone of CACHE_POP_INSWITCH_ACK
		update_putreq_inswitch_to_putreq_seq;
		update_putreq_inswitch_to_netcache_putreq_seq_cached;
		update_delreq_inswitch_to_delreq_seq;
		update_delreq_inswitch_to_netcache_delreq_seq_cached;

		update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone; // clone to reflector and hence switchos; but not need clone for pktloss due to switchos-side timeout-and-retry
		//forward_cache_evict_loadfreq_inswitch_ack;
		//update_netcache_valueupdate_inswitch_to_netcache_valueupdate_ack;
		update_distcache_invalidate_inswitch_to_distcache_invalidate_ack;
		//update_distcache_spine_valueupdate_inswitch_to_distcache_spine_valueupdate_inswitch_ack;
		update_distcache_valueupdate_inswitch_origin_to_distcache_valueupdate_inswitch_ack;
		update_putreq_largevalue_inswitch_to_putreq_largevalue_seq;
		update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_cached;
		NoAction;
	}
	default_action= NoAction();
	size = 4096;
}

// stage 10

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter update_ipmac_srcport_counter {
	type : packets_and_bytes;
	direct: update_ipmac_srcport_tbl;
}
#endif

action update_srcipmac_srcport_server2client(bit<48> server_mac, bit<32> server_ip,bit<16> server_port) {
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
		NoAction;
	}
	default_action= NoAction();
	size = 256;
}

// stage 11

// NOTE: only one operand in add can be action parameter or constant -> resort to controller to configure different hdrlen
/*
// CACHE_POP_INSWITCH_ACK, WARMUPACK (cloned by NETCACHE_WARMUPREQ_INSWITCH_POP), NETCACHE_VALUEUPDATE_ACK, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK
action update_onlyop_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 22(ophdr)
	hdr.udp_hdr.hdrlen = 30;
	hdr.ipv4_hdr.totalLen = 50;
}

// DISTCACHE_VALUEUPDATE_INSWITCH_ACK
action update_op_inswitch_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 22(ophdr) + 2(shadowtype) + 28(inswitchhdr)
	hdr.udp_hdr.hdrlen = 60;
	hdr.ipv4_hdr.totalLen = 80;
}

// GETRES, DISTCACHE_GETRES_SPINE
action update_val_stat_pktlen(aligned_vallen) {
	// 20[iphdr] + 8(udphdr) + 22(ophdr) + 2(vallen) + aligned_vallen(val) + 2(shadowtype) + 4(stat) + 8(switchload)
	hdr.udp_hdr.hdrlen = aligned_vallen + 46
	hdr.ipv4_hdr.totalLen = aligned_vallen + 66
}

// PUTREQ_SEQ, NETCACHE_PUTREQ_SEQ_CACHED
action update_val_seq_pktlen(aligned_vallen) {
	// [20(iphdr)] + 8(udphdr) + 22(ophdr) + 2(vallen) + aligned_vallen(val) + 2(shadowtype) + 4(seq)
	hdr.udp_hdr.hdrlen = aligned_vallen + 38
	hdr.ipv4_hdr.totalLen = aligned_vallen + 58
}

// PUTRES, DELRES (DistCache does NOT need)
action update_stat_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 22(ophdr) + 2(shadowtype) + 4(stat)
	hdr.udp_hdr.hdrlen = 36;
	hdr.ipv4_hdr.totalLen = 56;
}

// DELREQ_SEQ, NETCACHE_DELREQ_SEQ_CACHED
action update_seq_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 22(ophdr) + 2(shadowtype) + 4(seq) 
	hdr.udp_hdr.hdrlen = 36;
	hdr.ipv4_hdr.totalLen = 56;
}
 
// SCANREQ_SPLIT
action update_scanreqsplit_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 22(ophdr) + 16(endkey) + 12(split_hdr)
	hdr.udp_hdr.hdrlen = 58;
	hdr.ipv4_hdr.totalLen = 78;
}
 
// CACHE_EVICT_LOADFREQ_INSWITCH_ACK
action update_frequency_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 22(ophdr) + 4(frequency)
	hdr.udp_hdr.hdrlen = 34;
	hdr.ipv4_hdr.totalLen = 54;
}

// GETREQ (cloned by NETCACHE_GETREQ_POP)
action update_ophdr_switchloadhdr_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 22(ophdr) + 2(shadowtypehdr) + 8(switchloadhdr) 
	hdr.udp_hdr.hdrlen = 40;
	hdr.ipv4_hdr.totalLen = 60;
}

// NETCACHE_GETREQ_POP
action update_ophdr_switchloadhdr_clonehdr_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 22(ophdr) + 2(shadowtypehdr) + 8(switchloadhdr) + 18(clonehdr)
	hdr.udp_hdr.hdrlen = 58;
	hdr.ipv4_hdr.totalLen = 78;
}

// NETCACHE_WARMUPREQ_INSWITCH_POP
action update_ophdr_inswitchhdr_clonehdr_pktlen() {
	// [20(iphdr)] + 8(udphdr) + 22(ophdr) + 2(shadowtype) + 28(inswitchhdr) + 18(clonehdr)
	hdr.udp_hdr.hdrlen = 78;
	hdr.ipv4_hdr.totalLen = 98;
}

// PUTREQ_LARGEVALUE_SEQ, PUTREQ_LARGEVALUE_SEQ_CACHED
action add_shadowtypehdr_seqhdr_pktlen() {
	// 2(shadowtype) + 4(seq)
	add_to_field(hdr.udp_hdr.hdrlen, 6);
	add_to_field(hdr.ipv4_hdr.totalLen, 6);
}
*/

action update_pktlen(bit<16> udplen,bit<16> iplen) {
	hdr.udp_hdr.hdrlen = udplen;
	hdr.ipv4_hdr.totalLen = iplen;
}

action add_pktlen(bit<16> udplen_delta,bit<16>  iplen_delta) {
	hdr.udp_hdr.hdrlen = hdr.udp_hdr.hdrlen  + udplen_delta;
	hdr.ipv4_hdr.totalLen = hdr.ipv4_hdr.totalLen + iplen_delta;
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
		add_pktlen;
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
