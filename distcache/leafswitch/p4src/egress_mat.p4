/* Ingress Processing (Normal Operation) */

/* Egress Processing */

control netcacheEgress (inout headers hdr,
                 inout metadata meta,
                 inout standard_metadata_t standard_metadata){ 
	#include "regs/cm.p4"
	#include "regs/bf.p4"
	#include "regs/cache_frequency.p4"
	#include "regs/latest.p4"
	#include "regs/deleted.p4"
	#include "regs/seq.p4"
	#include "regs/val.p4"

	
	// Stage 0
	action save_client_udpport() {
		hdr.clone_hdr.client_udpport = hdr.udp_hdr.srcPort;
	}

	@pragma stage 0
	table save_client_udpport_tbl {
		key = {
			hdr.op_hdr.optype: exact;
		}
		actions = {
			save_client_udpport;
			NoAction;
		}
		default_action = NoAction();
		size = 4;
	}

	// Stage 1

	action set_server_sid_and_port(bit<10> server_sid) {
		
		hdr.clone_hdr.server_sid = server_sid;
		hdr.clone_hdr.server_udpport = hdr.udp_hdr.dstPort; // dstport is serverport for GETREQ_INSWITCH
		hdr.clone_hdr.setValid();
	}

	action reset_server_sid() {
		hdr.clone_hdr.server_sid = 0;
	}

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
		default_action = reset_server_sid();
		size = 32;
	}

	// Stage 2

	action set_is_hot() {
		meta.is_hot = 1;
		//hdr.debug_hdr.is_hot = 1;
	}

	action reset_is_hot() {
		meta.is_hot = 0;
		//hdr.debug_hdr.is_hot = 0;
	}

	@pragma stage 2
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
		default_action = reset_is_hot();
		size = 1;
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
		default_action = reset_is_report();
		size = 1;
	}

	// Stage 8

	action set_is_lastclone() {
		meta.is_lastclone_for_pktloss = 1;
		//hdr.debug_hdr.is_lastclone_for_pktloss = 1;
	}

	action reset_is_lastclone_lastscansplit() {
		meta.is_lastclone_for_pktloss = 0;
		//hdr.debug_hdr.is_lastclone_for_pktloss = 0;
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
		default_action = reset_is_lastclone_lastscansplit();
		size = 8;
	}

	// Stage 9
	action reverse_ip(){
		bit<32> tmp_ip;
		tmp_ip = hdr.ipv4_hdr.srcAddr;
		hdr.ipv4_hdr.srcAddr = hdr.ipv4_hdr.dstAddr;
		hdr.ipv4_hdr.dstAddr = tmp_ip;
	}
	action reverse_mac(){
		bit<48> tmp_mac;
		tmp_mac = hdr.ethernet_hdr.srcAddr;
		hdr.ethernet_hdr.srcAddr = hdr.ethernet_hdr.dstAddr;
		hdr.ethernet_hdr.dstAddr = tmp_mac;
	}
	action reverse_port(){
		bit<16> tmp_port;
		tmp_port = hdr.udp_hdr.srcPort;
		hdr.udp_hdr.srcPort = hdr.udp_hdr.dstPort;
		hdr.udp_hdr.dstPort = tmp_port;
	}
	action update_netcache_warmupreq_inswitch_to_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack(bit<10> switchos_sid,bit<16> reflector_port) {
		hdr.op_hdr.optype = NETCACHE_WARMUPREQ_INSWITCH_POP;
		hdr.shadowtype_hdr.shadowtype = NETCACHE_WARMUPREQ_INSWITCH_POP;
		hdr.udp_hdr.dstPort = reflector_port;
		hdr.clone_hdr.clonenum_for_pktloss = 3; // 3 ACKs (drop w/ 3 -> clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> drop and clone for WARMUPACK to client)

		hdr.clone_hdr.setValid(); // NOTE: clone_hdr.server_sid is reset as 0 in process_scanreq_split_tbl and prepare_for_cachepop_tbl
		hdr.frequency_hdr.setInvalid();
		//hdr.standard_metadata.egress_port = hdr.port; // set eport to switchos
		mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring
		// clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
		clone(CloneType.E2E, (bit<32>)switchos_sid);

	}

	action forward_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack(bit<10> switchos_sid) {
		hdr.clone_hdr.clonenum_for_pktloss =hdr.clone_hdr.clonenum_for_pktloss - 1;

		// clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
		clone(CloneType.E2E, (bit<32>)switchos_sid);
	}

	action update_netcache_warmupreq_inswitch_pop_to_warmupack_by_mirroring(bit<10> client_sid,bit<16> server_port) {
		hdr.op_hdr.optype = WARMUPACK;
		// DEPRECATED: udp.srcport will be set as server_worker_port_start in update_ipmac_srcport_tbl
		// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
		hdr.udp_hdr.srcPort = server_port;
		hdr.udp_hdr.dstPort = hdr.clone_hdr.client_udpport;
		hdr.frequency_hdr.setInvalid();
		hdr.shadowtype_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.clone_hdr.setInvalid();

		mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring
		// clone_egress_pkt_to_egress(client_sid); // clone to client (hdr.inswitch_hdr.client_sid)
		clone(CloneType.E2E, (bit<32>)client_sid);
	}

	action update_getreq_inswitch_to_getreq() {
		hdr.op_hdr.optype = GETREQ;
		hdr.frequency_hdr.setInvalid();
		hdr.shadowtype_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.clone_hdr.setInvalid();
		//hdr.standard_metadata.egress_port = hdr.eport;
	}

	action update_getreq_inswitch_to_netcache_getreq_pop_clone_for_pktloss_and_getreq(bit<10> switchos_sid,bit<16> reflector_port) {
		hdr.op_hdr.optype = NETCACHE_GETREQ_POP;
		hdr.udp_hdr.dstPort = reflector_port;
		hdr.clone_hdr.clonenum_for_pktloss = 3; // 3 ACKs (drop w/ 3 -> clone w/ 2 -> clone w/ 1 -> clone w/ 0 -> drop and clone for GETREQ to server)
		hdr.frequency_hdr.setInvalid();
		hdr.shadowtype_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.clone_hdr.setValid(); // NOTE: clone_hdr.server_sid has been set in prepare_for_cachepop_tbl

		//hdr.standard_metadata.egress_port = hdr.port; // set eport to switchos
		mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring
		// clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
		clone(CloneType.E2E, (bit<32>)switchos_sid);
	}

	action forward_netcache_getreq_pop_clone_for_pktloss_and_getreq(bit<10> switchos_sid) {
		hdr.clone_hdr.clonenum_for_pktloss = hdr.clone_hdr.clonenum_for_pktloss - 1;

		// clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
		clone(CloneType.E2E, (bit<32>)switchos_sid);
	}

	action update_netcache_getreq_pop_to_getreq_by_mirroring(bit<10> server_sid) {
		hdr.op_hdr.optype = GETREQ;
		// Keep original udp.srcport (aka client udp port)
		hdr.udp_hdr.dstPort = hdr.clone_hdr.server_udpport;

		hdr.clone_hdr.setInvalid();
		hdr.frequency_hdr.setInvalid();
		mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring
		// clone_egress_pkt_to_egress(server_sid); // clone to client (hdr.inswitch_hdr.client_sid)
		clone(CloneType.E2E, (bit<32>)server_sid);
	}

	action update_getreq_inswitch_to_getres_by_mirroring(bit<10> client_sid,bit<16> server_port,bit<8> stat) {
		hdr.op_hdr.optype = GETRES;
		hdr.shadowtype_hdr.shadowtype = GETRES;
		hdr.stat_hdr.stat = stat;
		hdr.stat_hdr.nodeidx_foreval = SWITCHIDX_FOREVAL;
		// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports
		// (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
		hdr.udp_hdr.srcPort = server_port;
		hdr.udp_hdr.dstPort = hdr.clone_hdr.client_udpport;
		reverse_ip();
		reverse_mac();
		hdr.frequency_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.stat_hdr.setValid();
		hdr.clone_hdr.setInvalid();
		mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring
		// clone_egress_pkt_to_egress(client_sid); // clone to client (hdr.inswitch_hdr.client_sid)
		clone(CloneType.E2E, (bit<32>)client_sid);
	}


	action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone(bit<10>switchos_sid,bit<16> reflector_port) {
		hdr.op_hdr.optype = CACHE_POP_INSWITCH_ACK;
		hdr.udp_hdr.dstPort = reflector_port;

		// NOTE: we add/remove vallen and value headers in add_remove_value_header_tbl
		hdr.shadowtype_hdr.setInvalid();
		hdr.seq_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.stat_hdr.setInvalid();
		hdr.frequency_hdr.setInvalid();
		hdr.clone_hdr.setInvalid();
		mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring
		// clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
		clone(CloneType.E2E, (bit<32>)switchos_sid);
	}

	//action forward_cache_pop_inswitch_ack() {
	//}

	action update_putreq_inswitch_to_putreq_seq() {
		hdr.op_hdr.optype = PUTREQ_SEQ;
		hdr.shadowtype_hdr.shadowtype = PUTREQ_SEQ;
		hdr.frequency_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.seq_hdr.setValid();
		hdr.clone_hdr.setInvalid();
		//hdr.standard_metadata.egress_port = hdr.eport;
	}

	action update_putreq_inswitch_to_netcache_putreq_seq_cached() {
		hdr.op_hdr.optype = NETCACHE_PUTREQ_SEQ_CACHED;
		hdr.shadowtype_hdr.shadowtype = NETCACHE_PUTREQ_SEQ_CACHED;
		hdr.frequency_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.seq_hdr.setValid();
		hdr.clone_hdr.setInvalid();
		//hdr.standard_metadata.egress_port = hdr.eport;
	}

	action update_delreq_inswitch_to_delreq_seq() {
		hdr.op_hdr.optype = DELREQ_SEQ;
		hdr.shadowtype_hdr.shadowtype = DELREQ_SEQ;
		hdr.frequency_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.seq_hdr.setValid();
		hdr.clone_hdr.setInvalid();
		//hdr.standard_metadata.egress_port = hdr.eport;
	}

	action update_delreq_inswitch_to_netcache_delreq_seq_cached() {
		hdr.op_hdr.optype = NETCACHE_DELREQ_SEQ_CACHED;
		hdr.shadowtype_hdr.shadowtype = NETCACHE_DELREQ_SEQ_CACHED;
		hdr.frequency_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.seq_hdr.setValid();
		hdr.clone_hdr.setInvalid();
		//hdr.standard_metadata.egress_port = hdr.eport;
	}


	action update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone(bit<10> switchos_sid,bit<16> reflector_port) {
		hdr.op_hdr.optype = CACHE_EVICT_LOADFREQ_INSWITCH_ACK;
		hdr.udp_hdr.dstPort = reflector_port;
		
		hdr.shadowtype_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.frequency_hdr.setValid();
		hdr.clone_hdr.setInvalid();
		mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring
		// clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
		clone(CloneType.E2E, (bit<32>)switchos_sid);
	}

	//action forward_cache_evict_loadfreq_inswitch_ack() {
	//}

	action update_netcache_valueupdate_inswitch_to_netcache_valueupdate_ack() {
		hdr.op_hdr.optype = NETCACHE_VALUEUPDATE_ACK;
		hdr.frequency_hdr.setInvalid();
		hdr.shadowtype_hdr.setInvalid();
		hdr.seq_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.stat_hdr.setInvalid();
		hdr.clone_hdr.setInvalid();
		// NOTE: egress_port has already been set in ig_port_forward_tbl at ingress pipeline
	}

	action update_putreq_largevalue_inswitch_to_putreq_largevalue_seq() {
		// NOTE: PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr
		hdr.op_hdr.optype = PUTREQ_LARGEVALUE_SEQ;
		hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_SEQ;
		hdr.frequency_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.seq_hdr.setValid();
		hdr.clone_hdr.setInvalid();
	}

	action update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_cached() {
		// NOTE: PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ_CACHED w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr
		hdr.op_hdr.optype = PUTREQ_LARGEVALUE_SEQ_CACHED;
		hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_SEQ_CACHED;
		hdr.frequency_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.seq_hdr.setValid();
		hdr.clone_hdr.setInvalid();
	}

	action update_netcache_cache_pop_inswitch_nlatest_to_cache_pop_inswitch_ack_drop_and_clone(bit<10> switchos_sid,bit<16> reflector_port) {
		hdr.op_hdr.optype = CACHE_POP_INSWITCH_ACK;
		hdr.udp_hdr.dstPort = reflector_port;
		hdr.frequency_hdr.setInvalid();
		// NOTE: we add/remove vallen and value headers in add_remove_value_header_tbl
		hdr.shadowtype_hdr.setInvalid();
		hdr.seq_hdr.setInvalid();
		hdr.inswitch_hdr.setInvalid();
		hdr.stat_hdr.setInvalid();
		hdr.clone_hdr.setInvalid();
		mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring
		// clone_egress_pkt_to_egress(switchos_sid); // clone to switchos
		clone(CloneType.E2E, (bit<32>)switchos_sid);
	}


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
			//inswitch_hdr.is_wrong_pipeline: exact;
			hdr.inswitch_hdr.client_sid: exact;
			meta.is_lastclone_for_pktloss: exact;
			//debug_hdr.is_lastclone_for_pktloss: exact;
			hdr.clone_hdr.server_sid: exact;
		}
		actions = {
			update_netcache_warmupreq_inswitch_to_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack;
			forward_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack;
			update_netcache_warmupreq_inswitch_pop_to_warmupack_by_mirroring;
			update_getreq_inswitch_to_getreq;
			update_getreq_inswitch_to_netcache_getreq_pop_clone_for_pktloss_and_getreq;
			forward_netcache_getreq_pop_clone_for_pktloss_and_getreq;
			update_netcache_getreq_pop_to_getreq_by_mirroring;
			update_getreq_inswitch_to_getres_by_mirroring;
			update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone; // clone for first CACHE_POP_INSWITCH_ACK (not need to clone for duplication due to switchos-side timeout-and-retry)
			
			update_putreq_inswitch_to_putreq_seq;
			update_putreq_inswitch_to_netcache_putreq_seq_cached;
			update_delreq_inswitch_to_delreq_seq;
			update_delreq_inswitch_to_netcache_delreq_seq_cached;

			update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone; // clone to reflector and hence switchos; but not need clone for pktloss due to switchos-side timeout-and-retry
			update_netcache_valueupdate_inswitch_to_netcache_valueupdate_ack;
			update_putreq_largevalue_inswitch_to_putreq_largevalue_seq;
			update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_cached;
			update_netcache_cache_pop_inswitch_nlatest_to_cache_pop_inswitch_ack_drop_and_clone; // clone for first CACHE_POP_INSWITCH_ACK (not need to clone for duplication due to switchos-side timeout-and-retry)
			NoAction;
		}
		default_action = NoAction();
		size = 4096;
	}

	// stage 10

	// NOTE: we need to set dstip/mac for pkt from server to client especially for those w/ cache hit
	action update_ipmac_srcport_server2client(bit<48> client_mac,bit<48> server_mac, bit<32> client_ip,bit<32> server_ip, bit<16> server_port) {
		bit <48> tmp_mac;
		bit <32> tmp_ip;
		// hdr.ethernet_hdr.srcAddr = server_mac;
		// hdr.ethernet_hdr.dstAddr = client_mac;
		// hdr.ipv4_hdr.srcAddr = server_ip;
		// hdr.ipv4_hdr.dstAddr = client_ip;
		tmp_mac = hdr.ethernet_hdr.srcAddr;
		hdr.ethernet_hdr.srcAddr = hdr.ethernet_hdr.dstAddr;
		hdr.ethernet_hdr.dstAddr = tmp_mac;
		tmp_ip = hdr.ipv4_hdr.srcAddr;
		hdr.ipv4_hdr.srcAddr = hdr.ipv4_hdr.dstAddr;
		hdr.ipv4_hdr.dstAddr = tmp_ip;
		hdr.udp_hdr.srcPort = server_port;
	}

	// NOTE: as we use software link, switch_mac/ip = reflector_mac/ip
	// NOTE: although we use client_port to update srcport here, reflector does not care about the specific value of srcport
	// NOTE: we must reset srcip/mac for pkt from reflector.cp2dpserver, otherwise the ACK pkt will be dropped by reflector.NIC as srcip/mac = NIC.ip/mac
	action update_ipmac_srcport_switch2switchos(bit<48> client_mac,bit<48> switch_mac, bit<32> client_ip,bit<32> switch_ip, bit<16> client_port) {
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
	action update_ipmac_srcport_client2server(bit<48> client_mac,bit<48> server_mac, bit<32> client_ip,bit<32> server_ip, bit<16> client_port) {
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
			update_ipmac_srcport_server2client; // focus on dstip and dstmac to corresponding client; use server[0] as srcip and srcmac; use server_worker_port_start as srcport
			update_ipmac_srcport_switch2switchos; // focus on dstip and dstmac to reflector; use client[0] as srcip and srcmac; use constant (123) as srcport
			update_dstipmac_switch2switchos; // keep original srcip/mac for NETCACHE_GETREQ_POP
			update_dstipmac_client2server; // focus on dstip and dstmac to corresponding server; NOT change srcip, srcmac, and srcport
			update_ipmac_srcport_client2server; // reset srcip/mac for NETCACHE_VALUEUPDATE_ACK
			NoAction;
		}
		default_action = NoAction();
		size = 256;
	}

	// stage 11


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
		default_action = NoAction(); // not change udp_hdr.hdrlen (GETREQ/GETREQ_POP/GETREQ_NLATEST)
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
		default_action = remove_all();
		size = 256;
	}
	apply{
		
		// [IMPORTANT]
		// Only prepare_for_cachepop_tbl will reset clone_hdr.server_sid as 0 by default, while process_scanreq_split_tbl only resets meta.remain_scannum by default (it ony modifies clone_hdr.server_sid for SCANREQ_SPLIT) -> MUST be very careful for all pkt types which will use clone_hdr.server_sid
		// For GETREQ_INSWITCH, clone_hdr.server_sid is NOT reset at process_scanreq_split_tbl, and is only set based on eport at prepare_for_cachepop_tbl -> OK
		// For SCANREQ_SPLIT, after setting server_sid based on split_hdr.globalserveridx at process_scanreq_split_tbl, it needs to invoke NoAction() explicitly in prepare_for_cachepop_tbl to avoid reset server_sid
		// For NETCACHE_GETREQ_POP, after inheriting clone_hdr.server_sid from GETREQ_INSWITCH, process_scanreq_split does NOT reset clone_hdr.server_sid by default, and it needs to invoke NoAction() explicitly in prepare_for_cachepop_tbl to avoid reset server_sid

		// Stage 0
		access_latest_tbl.apply(); // NOTE: latest_reg corresponds to stats.validity in NetCache paper, which will be used to *invalidate* the value by PUT/DELREQ
		access_seq_tbl.apply();
		save_client_udpport_tbl.apply(); // save udp.dstport (client port) for cache hit response of GETREQ/PUTREQ/DELREQ and PUTREQ/DELREQ_CASE1


		// Stage 1
		prepare_for_cachepop_tbl.apply(); // reset clone_hdr.server_sid by default here
		access_cm1_tbl.apply();
		access_cm2_tbl.apply();
		access_cm3_tbl.apply();
		access_cm4_tbl.apply();

		// Stage 2
		is_hot_tbl.apply();
		access_cache_frequency_tbl.apply();
		access_deleted_tbl.apply();
		access_savedseq_tbl.apply();

		// Stage 3
		update_vallen_tbl.apply();
		access_bf1_tbl.apply();
		access_bf2_tbl.apply();
		access_bf3_tbl.apply();
		is_report_tbl.apply(); // NOTE: place is_report_tbl here due to tricky Tofino MAT placement limitation -> not sure the reason
		lastclone_lastscansplit_tbl.apply(); // including is_last_scansplit
		eg_port_forward_tbl.apply(); // including scan forwarding
		update_ipmac_srcport_tbl.apply(); // Update ip, mac, and srcport for RES to client and notification to switchos
		update_pktlen_tbl.apply(); // Update udl_hdr.hdrLen for pkt with variable-length value
		add_and_remove_value_header_tbl.apply(); // Add or remove vallen and val according to optype and vallen

		// Stage 4
		// NOTE: value registers do not reply on op_hdr.optype, they only rely on meta.access_val_mode, which is set by update_vallen_tbl in stage 3
		update_vallo1_tbl.apply();
		update_valhi1_tbl.apply();
		update_vallo2_tbl.apply();
		update_valhi2_tbl.apply();

		// Stage 5
		update_vallo3_tbl.apply();
		update_valhi3_tbl.apply();
		update_vallo4_tbl.apply();
		update_valhi4_tbl.apply();

		// Stage 6
		update_vallo5_tbl.apply();
		update_valhi5_tbl.apply();
		update_vallo6_tbl.apply();
		update_valhi6_tbl.apply();

		// Stage 7
		
		update_vallo7_tbl.apply();
		update_valhi7_tbl.apply();
		update_vallo8_tbl.apply();
		update_valhi8_tbl.apply();

		// Stage 8
		
		update_vallo9_tbl.apply();
		update_valhi9_tbl.apply();
		update_vallo10_tbl.apply();
		update_valhi10_tbl.apply();

		// Stage 9
		
		// NOTE: Comment val11 and val12 in debug mode to save resources for eg_port_forward_counter -> you need to disable debug mode in evaluation
	#ifndef DEBUG
		update_vallo11_tbl.apply();
		update_valhi11_tbl.apply();
		update_vallo12_tbl.apply();
		update_valhi12_tbl.apply();
	#endif

		// stage 10
		// NOTE: resource in stage 11 is not enough for update_ipmac_src_port_tbl, so we place it into stage 10
		
		update_vallo13_tbl.apply();
		update_valhi13_tbl.apply();
		update_vallo14_tbl.apply();
		update_valhi14_tbl.apply();

		// Stage 11
		
		update_vallo15_tbl.apply();
		update_valhi15_tbl.apply();
		update_vallo16_tbl.apply();
		update_valhi16_tbl.apply();
	}
}