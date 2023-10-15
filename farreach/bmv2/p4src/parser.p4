/* Parser */

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11

// NOTE: due to hardware limitation, we cannot make too many branches in switch expression

// NOTE: carefully assign optype to reduce branches
// (1) vallen&value: &&& 0b0001; seq: &&& 0b0010; inswitch_hdr: &&& 0b0100; stat: &&& 0b1000;
// (2) scan/split: specific value (X + 0b0000); not parsed optypes: X + 0b0000
// op_hdr + vallen&value + shadowtype (0b0001): PUTREQ, WARMUPREQ (deprecated), LOADREQ
// op_hdr + vallen&value + shadowtype + seq (0b0011): PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3, NETCACHE_PUTREQ_SEQ_CACHED
// op_hdr + vallen&value + shadowtype + seq + inswitch_hdr (0b0111): NONE
// op_hdr + vallen&value + shadowtype + seq + inswitch_hdr + stat (0b1111) (XXX_CASE1 w/ clone_hdr): GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1, LOADSNAPSHOTDATA_INSWITCH_ACK, CACHE_POP_INSWITC, NETCACHE_VALUEUPDATE_INSWITCHH
// op_hdr + vallen&value + shadowtype + seq + stat (0b1011): GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, CACHE_EVICT_LOADDATA_INSWITCH_ACK, NETCACHE_VALUEUPDATE
// op_hdr + vallen&value + shadowtype + stat (0b1001): GETRES
// op_hdr + vallen&value + shadowtype + inswitch (0b0101): PUTREQ_INSWITCH
// op_hdr + shadowtype + inswitch_hdr (0b0100): GETREQ_INSWITCH, DELREQ_INSWITCH, CACHE_EVICT_LOADFREQ_INSWITCH, CACHE_EVICT_LOADDATA_INSWITCH, SETVALID_INSWITCH (w/ validvalue_hdr), NETCACHE_WARMUPREQ_INSWITCH, NETCACHE_WARMUPREQ_INSWITCH_POP (w/ clone_hdr)
// op_hdr + shadowtype + seq (0b0010): DELREQ_SEQ, DELREQ_SEQ_CASE3, NETCACHE_DELREQ_SEQ_CACHED
// op_hdr + shadowtype + stat (0b1000): PUTRES, DELRES
// NOTE: followings are ended with 0b0000
// op_hdr + scan_hdr (specific value): SCANREQ
// op_hdr + scan_hdr + split_hdr (specific value): SCANREQ_SPLIT
// only op_hdr (default): WARMUPREQ, GETREQ, DELREQ, GETREQ_POP, GETREQ_NLATEST, CACHE_POP_INSWITCH_ACK (deprecated: w/ clone_hdr), WARMUPACK, LOADACK, CACHE_POP_ACK, CACHE_EVICT_LOADFREQ_INSWITCH_ACK (w/ frequency_hdr), NETCACHE_GETREQ_POP, NETCACHE_VALUEUPATE_ACK
// not parsed in switch: SCANRES_SPLIT, CACHE_POP, CACHE_EVICT, CACHE_EVICT_ACK, CACHE_EVICT_CASE2, CACHE_POP_ACK, CACHE_EVICT_LOADFREQ_INSWITCH_ACK, SETVALID_INSWITCH_ACK, NETCACHE_CACHE_POP/_ACK, NETCACHE_CACHE_POP_FINISH/_ACK, NETCACHE_CACHE_EVICT/_ACK

parser farreachParser(packet_in packet,
                out headers hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata){
	state start {
		transition parse_ethernet;
	}

	state parse_ethernet {
		packet.extract(hdr.ethernet_hdr);
		transition select(hdr.ethernet_hdr.etherType) {
			ETHERTYPE_IPV4: parse_ipv4;
			default: accept;
		}
	}

	state parse_ipv4 {
		packet.extract(hdr.ipv4_hdr);
		transition select(hdr.ipv4_hdr.protocol) {
			PROTOTYPE_UDP: parse_udp_dstport;
			default: accept;
		}
	}

	state parse_udp_dstport {
		packet.extract(hdr.udp_hdr);
		transition select(hdr.udp_hdr.dstPort) {
			0x0480 &&& 0xFF80: parse_op; // reserve multiple udp port due to server simulation
			5008: parse_op; // reserve reflector.dp2cpserver_port due to hardware link simulation between switch and switchos
			default: parse_udp_srcport;
		}
	}

	state parse_udp_srcport {
		transition select(hdr.udp_hdr.srcPort) {
			0x0480 &&& 0xFF80: parse_op; // reserve multiple udp port due to server simulation
			5009: parse_op; // reserve reflector.cp2dpserver_port due to hardware link simulation between switch and switchos
			default: accept; // traditional packet
		}
	}

	// op_hdr -> scan_hdr -> split_hdr -> vallen_hdr -> val_hdr -> shadowtype_hdr -> seq_hdr -> inswitch_hdr -> stat_hdr -> clone_hdr/frequency_hdr/validvalue_hdr/fraginfo_hdr

	state parse_op {
		packet.extract(hdr.op_hdr);
		transition select(hdr.op_hdr.optype) {
			//CACHE_POP_INSWITCH_ACK: parse_clone;
			CACHE_EVICT_LOADFREQ_INSWITCH_ACK: parse_frequency;
			PUTREQ_LARGEVALUE: parse_fraginfo;
			1 &&& 0x01: parse_vallen;
			/*2 &&& 0x02: parse_seq;
			4 &&& 0x04: parse_inswitch;
			8 &&& 0x08: parse_stat;*/
			2 &&& 0x02: parse_shadowtype;
			4 &&& 0x04: parse_shadowtype;
			8 &&& 0x08: parse_shadowtype;
			default: accept;
		}
	}


	state parse_vallen {
		packet.extract(hdr.vallen_hdr);
		transition select(hdr.vallen_hdr.vallen) {
			//0: parse_val_len0;
			0: parse_shadowtype;
			0 &&& 0xFFF8: parse_val_len1;
			8: parse_val_len1;
			8 &&& 0xFFF8: parse_val_len2;
			16: parse_val_len2;
			16 &&& 0xFFF8: parse_val_len3;
			24: parse_val_len3;
			24 &&& 0xFFF8: parse_val_len4;
			32: parse_val_len4;
			32 &&& 0xFFF8: parse_val_len5;
			40: parse_val_len5;
			40 &&& 0xFFF8: parse_val_len6;
			48: parse_val_len6;
			48 &&& 0xFFF8: parse_val_len7;
			56: parse_val_len7;
			56 &&& 0xFFF8: parse_val_len8;
			64: parse_val_len8;
			64 &&& 0xFFF8: parse_val_len9;
			72: parse_val_len9;
			72 &&& 0xFFF8: parse_val_len10;
			80: parse_val_len10;
			80 &&& 0xFFF8: parse_val_len11;
			88: parse_val_len11;
			88 &&& 0xFFF8: parse_val_len12;
			96: parse_val_len12;
			96 &&& 0xFFF8: parse_val_len13;
			104: parse_val_len13;
			104 &&& 0xFFF8: parse_val_len14;
			112: parse_val_len14;
			112 &&& 0xFFF8: parse_val_len15;
			120: parse_val_len15;
			120 &&& 0xFFF8: parse_val_len16;
			128: parse_val_len16;
			//default: parse_val_len0; // > 128
			default: parse_shadowtype; // > 128
		}
	}

	/*parser parse_val_len0 {
		transition select(hdr.op_hdr.optype) {
			2 &&& 0x02: parse_seq;
			4 &&& 0x04: parse_inswitch;
			8 &&& 0x08: parse_stat;
			default: accept;
			//default: parse_debug;
			
			//GETRES: parse_stat;
			//GETRES_LATEST_SEQ: parse_seq;
			//GETRES_LATEST_SEQ_INSWITCH: parse_seq;
			//GETRES_LATEST_SEQ_INSWITCH_CASE1: parse_seq;
			//GETRES_DELETED_SEQ: parse_seq;
			//GETRES_DELETED_SEQ_INSWITCH: parse_seq;
			//GETRES_DELETED_SEQ_INSWITCH_CASE1: parse_seq;
			//CACHE_POP_INSWITCH: parse_seq;
			//PUTREQ_INSWITCH: parse_inswitch;
			//PUTREQ_SEQ: parse_seq;
			//PUTREQ_POP_SEQ: parse_seq;
			//PUTREQ_SEQ_INSWITCH_CASE1: parse_seq;
			//PUTREQ_SEQ_CASE3: parse_seq;
			//PUTREQ_POP_SEQ_CASE3: parse_seq;
			//DELREQ_INSWITCH: parse_inswitch;
			//DELREQ_SEQ_INSWITCH_CASE1: parse_seq;
			//default: accept; // PUTREQ
			////default: parse_debug; // PUTREQ
		}
	}*/

	state parse_val_len1 {
		packet.extract(hdr.val1_hdr);
		//return parse_val_len0;
		transition parse_shadowtype;
	}

	state parse_val_len2 {
		packet.extract(hdr.val2_hdr);
		transition parse_val_len1;
	}

	state parse_val_len3 {
		packet.extract(hdr.val3_hdr);
		transition parse_val_len2;
	}

	state parse_val_len4 {
		packet.extract(hdr.val4_hdr);
		transition parse_val_len3;
	}

	state parse_val_len5 {
		packet.extract(hdr.val5_hdr);
		transition parse_val_len4;
	}

	state parse_val_len6 {
		packet.extract(hdr.val6_hdr);
		transition parse_val_len5;
	}

	state parse_val_len7 {
		packet.extract(hdr.val7_hdr);
		transition parse_val_len6;
	}

	state parse_val_len8 {
		packet.extract(hdr.val8_hdr);
		transition parse_val_len7;
	}

	state parse_val_len9 {
		packet.extract(hdr.val9_hdr);
		transition parse_val_len8;
	}

	state parse_val_len10 {
		packet.extract(hdr.val10_hdr);
		transition parse_val_len9;
	}

	state parse_val_len11 {
		packet.extract(hdr.val11_hdr);
		transition parse_val_len10;
	}

	state parse_val_len12 {
		packet.extract(hdr.val12_hdr);
		transition parse_val_len11;
	}

	state parse_val_len13 {
		packet.extract(hdr.val13_hdr);
		transition parse_val_len12;
	}

	state parse_val_len14 {
		packet.extract(hdr.val14_hdr);
		transition parse_val_len13;
	}

	state parse_val_len15 {
		packet.extract(hdr.val15_hdr);
		transition parse_val_len14;
	}

	state parse_val_len16 {
		packet.extract(hdr.val16_hdr);
		transition parse_val_len15;
	}

	state parse_shadowtype {
		packet.extract(hdr.shadowtype_hdr);
		transition select(hdr.shadowtype_hdr.shadowtype) {
			2 &&& 0x02: parse_seq;
			4 &&& 0x04: parse_inswitch;
			8 &&& 0x08: parse_stat;
			default: accept;
			//default: parse_debug;
		}
	}

	state parse_seq {
		packet.extract(hdr.seq_hdr);
		transition select(hdr.shadowtype_hdr.shadowtype) {
			PUTREQ_LARGEVALUE_SEQ: parse_fraginfo;
			PUTREQ_LARGEVALUE_SEQ_CACHED: parse_fraginfo;
			PUTREQ_LARGEVALUE_SEQ_CASE3: parse_fraginfo;
			PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED: parse_fraginfo;
			PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED: parse_fraginfo;
			PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED_SPINE: parse_fraginfo; // only used by distfarreach
			PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED_SPINE: parse_fraginfo; // only used by distfarreach
			4 &&& 0x04: parse_inswitch;
			8 &&& 0x08: parse_stat;
			default: accept;
			//default: parse_debug;
			
			/*GETRES_LATEST_SEQ_INSWITCH: parse_inswitch;
			GETRES_LATEST_SEQ_INSWITCH_CASE1: parse_inswitch;
			GETRES_DELETED_SEQ_INSWITCH: parse_inswitch;
			GETRES_DELETED_SEQ_INSWITCH_CASE1: parse_inswitch;
		hdr.	CACHE_POP_INSWITCH: parse_inswitch; // inswitch_hdr is set by switchos
			PUTREQ_SEQ_INSWITCH_CASE1: parse_inswitch;
			DELREQ_SEQ_INSWITCH_CASE1: parse_inswitch;
			default: accept;
			//default: parse_debug; // GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3, DELREQ_SEQ, DELREQ_SEQ_CASE3 */
		}
	}

	state parse_inswitch {
		packet.extract(hdr.inswitch_hdr);
		
		transition select(hdr.shadowtype_hdr.shadowtype) {
			SETVALID_INSWITCH: parse_validvalue;
			PUTREQ_LARGEVALUE_INSWITCH: parse_fraginfo;
			PUTREQ_LARGEVALUE_SEQ_INSWITCH: parse_fraginfo;
			8 &&& 0x08: parse_stat;
			default: accept;
			//default: parse_debug;
			
			/*GETRES_LATEST_SEQ_INSWITCH_CASE1: parse_stat;
			GETRES_DELETED_SEQ_INSWITCH_CASE1: parse_stat;
			PUTREQ_SEQ_INSWITCH_CASE1: parse_stat;
			DELREQ_SEQ_INSWITCH_CASE1: parse_stat;
			default: accept;
			//default: parse_debug; // GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, PUTREQ_INSWITCH, DELREQ_INSWITCH, CACHE_POP_INSWITCH */
		}
	}

	state parse_stat {
		packet.extract(hdr.stat_hdr);
		transition select(hdr.shadowtype_hdr.shadowtype) {
			GETRES_LATEST_SEQ_INSWITCH_CASE1: parse_clone;
			GETRES_DELETED_SEQ_INSWITCH_CASE1: parse_clone;
			PUTREQ_SEQ_INSWITCH_CASE1: parse_clone;
			DELREQ_SEQ_INSWITCH_CASE1: parse_clone;
			default: accept; // CACHE_POP_INSWITCH
			//default: parse_debug;
		}
		//return ingress;
		////return parse_debug; // GETRES, PUTRES, DELRES, GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1
	}

	state parse_clone {
		packet.extract(hdr.clone_hdr);
		transition accept;
		//return parse_debug; // GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1
		
	}

	state parse_frequency {
		packet.extract(hdr.frequency_hdr);
		transition accept; // CACHE_EVICT_LOADFREQ_INSWITCH_ACK
	}

	state parse_validvalue {
		packet.extract(hdr.validvalue_hdr);
		transition accept; // SETVALID_INSWITCH
	}

	state parse_fraginfo {
		packet.extract(hdr.fraginfo_hdr);
		transition accept; // PUTREQ_LARGEVALUE, PUTREQ_LARGEVALUE_INSWITCH, PUTREQ_LARGEVALUE_SEQ, PUTREQ_LARGEVALUE_SEQ_INSWITCH, PUTREQ_LARGEVALUE_SEQ_CACHED, PUTREQ_LARGEVALUE_SEQ_CASE3
	}
}
/*parser parse_debug {
	packet.extract(hdr.debug_hdr);
	return ingress;
}*/
control farreachDeparser(packet_out packet, in headers hdr) {
    apply {
		packet.emit(hdr.ethernet_hdr);
		packet.emit(hdr.ipv4_hdr);
		packet.emit(hdr.udp_hdr);
		packet.emit(hdr.op_hdr);
		packet.emit(hdr.vallen_hdr);
		packet.emit(hdr.val1_hdr);
		packet.emit(hdr.val2_hdr);
		packet.emit(hdr.val3_hdr);
		packet.emit(hdr.val4_hdr);
		packet.emit(hdr.val5_hdr);
		packet.emit(hdr.val6_hdr);
		packet.emit(hdr.val7_hdr);
		packet.emit(hdr.val8_hdr);
		packet.emit(hdr.val9_hdr);
		packet.emit(hdr.val10_hdr);
		packet.emit(hdr.val11_hdr);
		packet.emit(hdr.val12_hdr);
		packet.emit(hdr.val13_hdr);
		packet.emit(hdr.val14_hdr);
		packet.emit(hdr.val15_hdr);
		packet.emit(hdr.val16_hdr);
		packet.emit(hdr.shadowtype_hdr);
		packet.emit(hdr.seq_hdr);
		packet.emit(hdr.inswitch_hdr);
		packet.emit(hdr.stat_hdr);
		packet.emit(hdr.clone_hdr);
		packet.emit(hdr.frequency_hdr);
		packet.emit(hdr.validvalue_hdr);
    }
}
