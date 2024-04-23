/* Egress Processing (Normal Operation) */

// Stage 0


// Stage 1

action set_is_hot() {
	meta.is_hot = 1;
	//debug_hdr.is_hot = 1;
}

action reset_is_hot() {
	meta.is_hot = 0;
	//debug_hdr.is_hot = 0;
}

@pragma stage 1
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


// Stage 2

action save_client_udpport() {
	hdr.clone_hdr.client_udpport = hdr.udp_hdr.srcPort;
}

@pragma stage 2
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

// Stage 3

action set_is_largevalueblock() {
	meta.is_largevalueblock = 1;
}

@pragma stage 3
table is_largevalueblock_tbl {
	key = {
		hdr.seq_hdr.largevalueseq: exact;
	}
	actions = {
		set_is_largevalueblock;
		NoAction;
	}
	default_action = set_is_largevalueblock();
	size = 2;
}

// Stage 7
// Stage 3

action set_is_found() {
	hdr.inswitch_hdr.is_found = 1;
}

@pragma stage 5
table recover_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached:exact;
	}
	actions = {
		set_is_found;
		NoAction;
	}
	default_action = NoAction();
	size = 1;
}


action set_is_lastclone() {
	meta.is_lastclone_for_pktloss = 1;
	//debug_hdr.is_lastclone_for_pktloss = 1;
}



action reset_is_lastclone_lastscansplit() {
	meta.is_lastclone_for_pktloss = 0;
	//debug_hdr.is_lastclone_for_pktloss = 0;

}

@pragma stage 7
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

// Stage 8
// action clone_e2e(MirrorId_t mir_ses){
// 	eg_dprsr_md.mirror_type = MIRROR_TYPE_E2E;
// 	meta.egr_mir_ses = mir_ses;
// }
#define clone_e2e(mir_ses)                      \
    eg_dprsr_md.mirror_type = MIRROR_TYPE_E2E;  \
	meta.egr_mir_ses = ##mir_ses;  				\
	meta.pkt_type = PKT_TYPE_MIRROR;		
action update_getreq_inswitch_to_getreq() {
	hdr.op_hdr.optype = GETREQ;

	hdr.shadowtype_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();
	//hdr.validvalue_hdr.setInvalid();
	//standard_metadata.engress_port = eport;
}


action update_getreq_inswitch_to_getreq_beingevicted_record(bit<8> stat) {
	hdr.op_hdr.optype = GETREQ_BEINGEVICTED_RECORD;
	hdr.shadowtype_hdr.shadowtype = GETREQ_BEINGEVICTED_RECORD;
	hdr.stat_hdr.stat = stat;

	//shadowtype_hdr.setInvalid();
	hdr.seq_hdr.setValid();
	hdr.inswitch_hdr.setInvalid();
	hdr.stat_hdr.setValid();
	//hdr.validvalue_hdr.setInvalid();
}

//#ifdef ENABLE_LARGEVALUEBLOCK
action update_getreq_inswitch_to_getreq_largevalueblock_record(bit<8> stat) {
	hdr.op_hdr.optype = GETREQ_LARGEVALUEBLOCK_RECORD;
	hdr.shadowtype_hdr.shadowtype = GETREQ_LARGEVALUEBLOCK_RECORD;
	hdr.stat_hdr.stat = stat;

	//shadowtype_hdr.setInvalid();
	hdr.seq_hdr.setValid();
	hdr.inswitch_hdr.setInvalid();
	hdr.stat_hdr.setValid();
	//hdr.validvalue_hdr.setInvalid();
}
//#endif

action update_getreq_inswitch_to_getreq_pop() {
	hdr.op_hdr.optype = GETREQ_POP;

	hdr.shadowtype_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();
	//hdr.validvalue_hdr.setInvalid(); //add
	//standard_metadata.engress_port = eport;
}

action update_getreq_inswitch_to_getreq_nlatest() {
	hdr.op_hdr.optype = GETREQ_NLATEST;

	hdr.shadowtype_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();
	//hdr.validvalue_hdr.setInvalid(); //add
	//standard_metadata.engress_port = eport;
}

action update_getreq_inswitch_to_getres_seq_by_mirroring(bit<10> client_sid,bit<16> server_port,bit<8> stat) {
	hdr.op_hdr.optype = GETRES_SEQ;
	hdr.shadowtype_hdr.shadowtype = GETRES_SEQ;
	hdr.stat_hdr.stat = stat;
	hdr.stat_hdr.nodeidx_foreval = SWITCHIDX_FOREVAL;
	// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
	hdr.udp_hdr.srcPort = server_port;
	hdr.udp_hdr.dstPort = hdr.clone_hdr.client_udpport;


	hdr.seq_hdr.setValid();
	hdr.inswitch_hdr.setInvalid();
	hdr.stat_hdr.setValid();
	//hdr.validvalue_hdr.setInvalid(); //add
	eg_dprsr_md.drop_ctl = 0x1; // Disable unicast, but enable mirroring
	clone_e2e(client_sid); // clone to client (inswitch_hdr.client_sid)
}

action update_getres_latest_seq_to_getres_seq() {
	hdr.op_hdr.optype = GETRES_SEQ;
	hdr.shadowtype_hdr.shadowtype = GETRES_SEQ;
	hdr.stat_hdr.stat = 1;

	//seq_hdr.setInvalid();
	hdr.seq_hdr.setValid();
	hdr.stat_hdr.setValid();
	//hdr.validvalue_hdr.setInvalid(); //add
}

action update_getres_deleted_seq_to_getres_seq() {
	hdr.op_hdr.optype = GETRES_SEQ;
	hdr.shadowtype_hdr.shadowtype = GETRES_SEQ;
	hdr.stat_hdr.stat = 0;
	//hdr.validvalue_hdr.setInvalid(); 
	//seq_hdr.setInvalid();
	hdr.seq_hdr.setValid();
	hdr.stat_hdr.setValid();
}


action update_putreq_largevalue_inswitch_to_putreq_largevalue_seq() {
	// NOTE: PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr
	hdr.op_hdr.optype = PUTREQ_LARGEVALUE_SEQ;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_SEQ;
	//hdr.validvalue_hdr.setInvalid(); 
	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();
}


action update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_beingevicted() {
	// NOTE: PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr + shadowtype_hdr + inswitch_hdr + fraginfo_hdr -> PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED w/ op_hdr + shadowtype_hdr + seq_hdr + fraginfo_hdr
	hdr.op_hdr.optype = PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED;
	//hdr.validvalue_hdr.setInvalid(); 
	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();
}


@pragma stage 8
table another_eg_port_forward_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.is_hot: exact;
		hdr.validvalue_hdr.validvalue: exact;
		hdr.inswitch_hdr.is_latest: exact;
//#ifdef ENABLE_LARGEVALUEBLOCK
		meta.is_largevalueblock: exact;
//#endif
		hdr.inswitch_hdr.is_deleted: exact;
		hdr.inswitch_hdr.client_sid: exact;
		meta.is_lastclone_for_pktloss: exact;
	}
	actions = {
		update_getreq_inswitch_to_getreq;
//		update_getreq_inswitch_to_getreq_beingevicted;
////#ifdef ENABLE_LARGEVALUEBLOCK
//		update_getreq_inswitch_to_getreq_largevalueblock_seq;
////#endif
		update_getreq_inswitch_to_getreq_beingevicted_record;
//#ifdef ENABLE_LARGEVALUEBLOCK
		update_getreq_inswitch_to_getreq_largevalueblock_record;
//#endif
		update_getreq_inswitch_to_getreq_pop;
		update_getreq_inswitch_to_getreq_nlatest;
		update_getreq_inswitch_to_getres_seq_by_mirroring;
		update_getres_latest_seq_to_getres_seq; // GETRES_LATEST_SEQ must be cloned from ingress to egress
		update_getres_deleted_seq_to_getres_seq; // GETRES_DELETED_SEQ must be cloned from ingress to egress
		update_putreq_largevalue_inswitch_to_putreq_largevalue_seq;
		update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_beingevicted;
		NoAction;
	}
	default_action = NoAction();
//#ifdef ENABLE_LARGEVALUEBLOCK
	size = 512;
//#else
//	size = 256;
//#endif
}

// Stage 9


action update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone(bit<10> switchos_sid,bit<16> reflector_port) {
	hdr.op_hdr.optype = CACHE_POP_INSWITCH_ACK;
	hdr.udp_hdr.dstPort = reflector_port;

	// NOTE: we add/remove vallen and value headers in add_remove_value_header_tbl
	hdr.shadowtype_hdr.setInvalid();
	hdr.seq_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();
	hdr.stat_hdr.setInvalid();
	// //hdr.validvalue_hdr.setInvalid();
	eg_dprsr_md.drop_ctl = 0x1; // Disable unicast, but enable mirroring
	clone_e2e(switchos_sid); // clone to switchos
}

//action forward_cache_pop_inswitch_ack() {
//}

action update_putreq_inswitch_to_putreq_seq() {
	hdr.op_hdr.optype = PUTREQ_SEQ;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_SEQ;

	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();
	//hdr.validvalue_hdr.setInvalid(); 
	//standard_metadata.engress_port = eport;
}

action update_putreq_inswitch_to_putreq_seq_beingevicted() {
	hdr.op_hdr.optype = PUTREQ_SEQ_BEINGEVICTED;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_SEQ_BEINGEVICTED;
	//hdr.validvalue_hdr.setInvalid(); 
	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();
}

action update_putreq_inswitch_to_putreq_pop_seq() {
	hdr.op_hdr.optype = PUTREQ_POP_SEQ;
	hdr.shadowtype_hdr.shadowtype = PUTREQ_POP_SEQ;

	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();
	//hdr.validvalue_hdr.setInvalid(); 
	//standard_metadata.engress_port = eport;
}

action update_putreq_inswitch_to_putres_seq_by_mirroring(bit<10> client_sid,bit<16> server_port) {
	hdr.op_hdr.optype = PUTRES_SEQ;
	hdr.shadowtype_hdr.shadowtype = PUTRES_SEQ;
	hdr.seq_hdr.seq = hdr.clone_hdr.assignedseq_for_farreach;
	hdr.stat_hdr.stat = 1;
	hdr.stat_hdr.nodeidx_foreval = SWITCHIDX_FOREVAL;
	// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
	hdr.udp_hdr.srcPort = server_port;
	hdr.udp_hdr.dstPort = hdr.clone_hdr.client_udpport;


	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();
	hdr.stat_hdr.setValid();
	//hdr.validvalue_hdr.setInvalid(); 
	eg_dprsr_md.drop_ctl = 0x1; // Disable unicast, but enable mirroring
	clone_e2e(client_sid); // clone to client (inswitch_hdr.client_sid)
}


action update_delreq_inswitch_to_delreq_seq() {
	hdr.op_hdr.optype = DELREQ_SEQ;
	hdr.shadowtype_hdr.shadowtype = DELREQ_SEQ;

	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();
	//hdr.validvalue_hdr.setInvalid(); 
	//standard_metadata.engress_port = eport;
}

action update_delreq_inswitch_to_delreq_seq_beingevicted() {
	hdr.op_hdr.optype = DELREQ_SEQ_BEINGEVICTED;
	hdr.shadowtype_hdr.shadowtype = DELREQ_SEQ_BEINGEVICTED;
	//hdr.validvalue_hdr.setInvalid(); 
	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();
}

action update_delreq_inswitch_to_delres_seq_by_mirroring(bit<10> client_sid,bit<16> server_port) {
	hdr.op_hdr.optype = DELRES_SEQ;
	hdr.shadowtype_hdr.shadowtype = DELRES_SEQ;
	hdr.seq_hdr.seq = hdr.clone_hdr.assignedseq_for_farreach;
	hdr.stat_hdr.stat = 1;
	hdr.stat_hdr.nodeidx_foreval = SWITCHIDX_FOREVAL;
	// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
	hdr.udp_hdr.srcPort = server_port;
	hdr.udp_hdr.dstPort = hdr.clone_hdr.client_udpport;

	hdr.inswitch_hdr.setInvalid();
	hdr.seq_hdr.setValid();
	hdr.stat_hdr.setValid();
	//hdr.validvalue_hdr.setInvalid(); 
	eg_dprsr_md.drop_ctl = 0x1; // Disable unicast, but enable mirroring
	clone_e2e(client_sid); // clone to client (inswitch_hdr.client_sid)
}


action update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone(bit<10> switchos_sid,bit<16> reflector_port) {
	hdr.op_hdr.optype = CACHE_EVICT_LOADFREQ_INSWITCH_ACK;
	hdr.udp_hdr.dstPort = reflector_port;

	hdr.shadowtype_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();
	hdr.frequency_hdr.setValid();
	//hdr.validvalue_hdr.setInvalid(); 
	eg_dprsr_md.drop_ctl = 0x1; // Disable unicast, but enable mirroring
	clone_e2e(switchos_sid); // clone to switchos
}

//action forward_cache_evict_loadfreq_inswitch_ack() {
//}

action update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone(bit<10> switchos_sid,bit<16> reflector_port,bit<8> stat) {
	hdr.op_hdr.optype = CACHE_EVICT_LOADDATA_INSWITCH_ACK;
	hdr.udp_hdr.dstPort = reflector_port;
	hdr.shadowtype_hdr.shadowtype = CACHE_EVICT_LOADDATA_INSWITCH_ACK;
	hdr.stat_hdr.stat = stat;

	hdr.inswitch_hdr.setInvalid();
	// NOTE: we add/remove vallen and value headers in add_remove_value_header_tbl
	hdr.seq_hdr.setValid();
	hdr.stat_hdr.setValid();
	//hdr.validvalue_hdr.setInvalid(); 
	eg_dprsr_md.drop_ctl = 0x1; // Disable unicast, but enable mirroring
	clone_e2e(switchos_sid); // clone to switchos
}

action update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone(bit<10> switchos_sid,bit<16>  reflector_port) {
	hdr.op_hdr.optype = SETVALID_INSWITCH_ACK;
	hdr.udp_hdr.dstPort = reflector_port;

	hdr.shadowtype_hdr.setInvalid();
	hdr.inswitch_hdr.setInvalid();
	hdr.validvalue_hdr.setInvalid(); //origin
	
	eg_dprsr_md.drop_ctl = 0x1; // Disable unicast, but enable mirroring
	clone_e2e(switchos_sid); // clone to switchos
}


@pragma stage 9
table eg_port_forward_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.is_hot: exact;
		
		hdr.validvalue_hdr.validvalue: exact;
		hdr.inswitch_hdr.is_latest: exact;
		hdr.inswitch_hdr.is_deleted: exact;
		
		hdr.inswitch_hdr.client_sid: exact;
		meta.is_lastclone_for_pktloss: exact;
		
	}
	actions = {
		//update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss; // clone for first CACHE_POP_INSWITCH_ACK
		//forward_cache_pop_inswitch_ack_clone_for_pktloss; // not last clone of CACHE_POP_INSWITCH_ACK
		update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone; // clone for first CACHE_POP_INSWITCH_ACK (not need to clone for duplication due to switchos-side timeout-and-retry)
		//forward_cache_pop_inswitch_ack; // last clone of CACHE_POP_INSWITCH_ACK
		update_putreq_inswitch_to_putreq_seq;
		update_putreq_inswitch_to_putreq_seq_beingevicted;
		update_putreq_inswitch_to_putreq_pop_seq;
		update_putreq_inswitch_to_putres_seq_by_mirroring;

		update_delreq_inswitch_to_delreq_seq;
		update_delreq_inswitch_to_delreq_seq_beingevicted;
		update_delreq_inswitch_to_delres_seq_by_mirroring;
		update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone; // clone to reflector and hence switchos; but not need clone for pktloss due to switchos-side timeout-and-retry
		//forward_cache_evict_loadfreq_inswitch_ack;
		update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone; // clone to reflector and hence switchos; but not need clone for pktloss due to switchos-side timeout-and-retry
		//forward_cache_evict_loaddata_inswitch_ack;
		
		update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone;
		//forward_setvalid_inswitch_ack;
		NoAction;
	}
	default_action = NoAction();
	size = 2048;
}

// stage 10

action update_ipmac_srcport_server2client(bit<48> client_mac, bit<48> server_mac,bit<32> client_ip,bit<32> server_ip, bit<16> server_port) {
	hdr.ethernet_hdr.srcAddr = server_mac;
	hdr.ethernet_hdr.dstAddr = client_mac;
	hdr.ipv4_hdr.srcAddr = server_ip;
	hdr.ipv4_hdr.dstAddr = client_ip;
	hdr.udp_hdr.srcPort = server_port;
}

// NOTE: as we use software link, switch_mac/ip = reflector_mac/ip
// NOTE: although we use client_port to update srcport here, reflector does not care about the specific value of srcport
action update_ipmac_srcport_switch2switchos(bit<48> client_mac, bit<48> switch_mac,bit<32> client_ip,bit<32> switch_ip, bit<16> client_port) {
	hdr.ethernet_hdr.srcAddr = client_mac;
	hdr.ethernet_hdr.dstAddr = switch_mac;
	hdr.ipv4_hdr.srcAddr = client_ip;
	hdr.ipv4_hdr.dstAddr = switch_ip;
	hdr.udp_hdr.srcPort = client_port;
}

action update_dstipmac_client2server(bit<48> server_mac,bit<32> server_ip) {
	hdr.ethernet_hdr.dstAddr = server_mac;
	hdr.ipv4_hdr.dstAddr = server_ip;
}

// NOTE: dstport of REQ, RES, and notification has been updated in partition_tbl, server, and eg_port_forward_tbl
@pragma stage 10
table update_ipmac_srcport_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		eg_intr_md.egress_port: exact;
	}
	actions = {
		update_ipmac_srcport_server2client; // focus on dstip and dstmac to corresponding client; use server[0] as srcip and srcmac; use server_worker_port_start as srcport
		update_ipmac_srcport_switch2switchos; // focus on dstip and dstmac to reflector; use client[0] as srcip and srcmac; use constant (123) as srcport
		update_dstipmac_client2server; // focus on dstip and dstmac to corresponding server; NOT change srcip, srcmac, and srcport
		NoAction;
	}
	default_action = NoAction();
	size = 256;
}

// stage 11

// NOTE: only one operand in add can be action parameter or constant -> resort to controller to configure different hdrlen

action update_pktlen(bit<16> udplen,bit<16> iplen) {
	hdr.udp_hdr.hdrlen = udplen;
	hdr.ipv4_hdr.totalLen = iplen;
	// hdr.vallen_hdr.setValid();
	//meta.udp_hdrlen = udplen;
}

action add_pktlen(bit<16> udplen_delta,bit<16> iplen_delta) {
	hdr.udp_hdr.hdrlen = hdr.udp_hdr.hdrlen +  udplen_delta;
	hdr.ipv4_hdr.totalLen = hdr.ipv4_hdr.totalLen +  iplen_delta;
	//hdr.meta.udp_hdrlen = hdr.meta.udp_hdrlen +  udplen_delta
}

@pragma stage 10
table update_pktlen_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.vallen_hdr.vallen: range;
	}
	actions = {
		update_pktlen;
		add_pktlen;
		NoAction;
	}
	default_action = NoAction(); // not change udp_hdr.hdrlen (GETREQ/GETREQ_POP/GETREQ_NLATEST)
	size = 512;
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
	size = 512;
}

action forward_getres_latest_seq_inswitch() {
	// NOTE: MATs after drop will not be accessed
	hdr.op_hdr.optype=GETRES_LATEST_SEQ;	
	hdr.shadowtype_hdr.shadowtype=GETRES_LATEST_SEQ;
	hdr.inswitch_hdr.setInvalid();
	hdr.validvalue_hdr.setInvalid();
}

action forward_getres_deleted_seq_inswitch() {
	hdr.op_hdr.optype=GETRES_DELETED_SEQ;	
	hdr.shadowtype_hdr.shadowtype=GETRES_DELETED_SEQ;
	hdr.inswitch_hdr.setInvalid();
	hdr.validvalue_hdr.setInvalid();
}

@pragma stage 11
table forward_tbl {
	key = {
		hdr.op_hdr.optype: exact;
	}
	actions = {
		forward_getres_latest_seq_inswitch;
		forward_getres_deleted_seq_inswitch;
		NoAction;
	}
	default_action = NoAction();
	size = 2;
}