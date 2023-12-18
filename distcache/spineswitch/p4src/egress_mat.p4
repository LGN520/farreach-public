control partitionswitchEgress(inout headers hdr,
                 inout metadata meta,
                 inout standard_metadata_t standard_metadata){ 
	/* Ingress Processing (Normal Operation) */
	#include "regs/val.p4"
	#include "regs/deleted.p4"
	#include "regs/cache_frequency.p4"
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
	action update_netcache_getreq_spine_to_getreq(){
		hdr.op_hdr.optype = GETREQ;
	}
	action update_netcache_getreq_spine_to_getres_by_mirroring(bit<8> stat){
		hdr.op_hdr.optype = GETRES;
		hdr.shadowtype_hdr.shadowtype = GETRES;
		hdr.stat_hdr.stat = stat;
		hdr.stat_hdr.nodeidx_foreval = SWITCHIDX_FOREVAL;
		// NOTE: we must set udp.srcPort now, otherwise it will dropped by parser/deparser due to NO reserved udp ports
		// (current pkt will NOT access update_ipmac_srcport_tbl for server2client as current devport is server instead of client)
		reverse_port();
		reverse_ip();
		reverse_mac();
		hdr.shadowtype_hdr.setValid();
		hdr.stat_hdr.setValid();
		mark_to_drop(standard_metadata); // Disable unicast, but enable mirroring
		// clone_egress_pkt_to_egress(client_sid); // clone to client (hdr.inswitch_hdr.client_sid)
		clone(CloneType.E2E, (bit<32>)meta.client_sid);
	}
	table eg_port_forward_tbl {
		key = {
			hdr.op_hdr.optype: exact;
			meta.is_cached:exact;
		}
		actions =  {
			update_netcache_getreq_spine_to_getreq;
			update_netcache_getreq_spine_to_getres_by_mirroring;
			NoAction;
		}
		default_action = NoAction();
		size = 128;
	}
	
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
			update_pktlen;
			add_pktlen;
			NoAction;
		}
		default_action = NoAction(); // not change udp_hdr.hdrlen (GETREQ/GETREQ_POP/GETREQ_NLATEST)
		size = 256;
	}
	apply{
		// stage 3
		// NOTE: resource in stage 11 is not enough for update_ipmac_src_port_tbl, so we place it into stage 10
		access_deleted_tbl.apply();
		access_cache_frequency_tbl.apply();
		if(meta.is_spine == 1){
			update_vallen_tbl.apply();
		}
		eg_port_forward_tbl.apply();
		// update_ipmac_srcport_tbl.apply(); // Update ip, mac, and srcport for RES to client and notification to switchos
		if(meta.is_spine == 1){
			update_pktlen_tbl.apply();
			add_and_remove_value_header_tbl.apply(); // Add or remove vallen and val according to optype and vallen

			// Stage 4
			// NOTE: value registers do not reply on op_hdr.optype, they only rely on meta.access_val_mode, which is set by update_vallen_tbl in stage 3
			update_vallo1_tbl.apply();
			update_valhi1_tbl.apply();
			update_vallo2_tbl.apply();
			update_valhi2_tbl.apply();
			update_vallo3_tbl.apply();
			update_valhi3_tbl.apply();
			update_vallo4_tbl.apply();
			update_valhi4_tbl.apply();
			update_vallo5_tbl.apply();
			update_valhi5_tbl.apply();
			update_vallo6_tbl.apply();
			update_valhi6_tbl.apply();
			update_vallo7_tbl.apply();
			update_valhi7_tbl.apply();
			update_vallo8_tbl.apply();
			update_valhi8_tbl.apply();
			update_vallo9_tbl.apply();
			update_valhi9_tbl.apply();
			update_vallo10_tbl.apply();
			update_valhi10_tbl.apply();
			update_vallo11_tbl.apply();
			update_valhi11_tbl.apply();
			update_vallo12_tbl.apply();
			update_valhi12_tbl.apply();
			update_vallo13_tbl.apply();
			update_valhi13_tbl.apply();
			update_vallo14_tbl.apply();
			update_valhi14_tbl.apply();
			update_vallo15_tbl.apply();
			update_valhi15_tbl.apply();
			update_vallo16_tbl.apply();
			update_valhi16_tbl.apply();
		}

	}
}