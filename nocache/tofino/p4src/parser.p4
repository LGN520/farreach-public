/* Parser */

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11

// NOTE: due to hardware limitation, we cannot make too many branches in switch expression

// NOTE: carefully assign optype to reduce branches
// (1) vallen&value: mask 0b0001; seq: mask 0b0010; inswitch_hdr: mask 0b0100; stat: mask 0b1000;
// (2) scan/split: specific value (X + 0b0000); not parsed optypes: X + 0b0000
// op_hdr + vallen&value + shadowtype (0b0001): PUTREQ, WARMUPREQ, LOADREQ
// op_hdr + vallen&value + shadowtype + seq (0b0011): GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3
// op_hdr + vallen&value + shadowtype + seq + inswitch_hdr (0b0111): GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, CACHE_POP_INSWITCH
// op_hdr + vallen&value + shadowtype + seq + inswitch_hdr + stat (0b1111) (XXX_CASE1 w/ clone_hdr): GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1
// op_hdr + vallen&value + shadowtype + seq + stat (0b1011): CACHE_EVICT_LOADDATA_INSWITCH_ACK
// op_hdr + vallen&value + shadowtype + stat (0b1001): GETRES
// op_hdr + vallen&value + shadowtype + inswitch (0b0101): PUTREQ_INSWITCH
// op_hdr + shadowtype + inswitch_hdr (0b0100): GETREQ_INSWITCH, DELREQ_INSWITCH, CACHE_EVICT_LOADFREQ_INSWITCH, CACHE_EVICT_LOADDATA_INSWITCH, SETVALID_INSWITCH (w/ validvalue_hdr)
// op_hdr + shadowtype + seq (0b0010): DELREQ_SEQ, DELREQ_SEQ_CASE3
// op_hdr + shadowtype + stat (0b1000): PUTRES, DELRES
// NOTE: followings are ended with 0b0000
// op_hdr + scan_hdr (specific value): SCANREQ
// op_hdr + scan_hdr + split_hdr (specific value): SCANREQ_SPLIT
// only op_hdr (default): GETREQ, DELREQ, GETREQ_POP, GETREQ_NLATEST, CACHE_POP_INSWITCH_ACK (deprecated: w/ clone_hdr), WARMUPACK, LOADACK, CACHE_POP_ACK, CACHE_EVICT_LOADFREQ_INSWITCH_ACK (w/ frequency_hdr)
// not parsed in switch: SCANRES_SPLIT, CACHE_POP, CACHE_EVICT, CACHE_EVICT_ACK, CACHE_EVICT_CASE2, CACHE_POP_ACK

parser start {
	return parse_ethernet;
}

parser parse_ethernet {
	extract(ethernet_hdr);
	//return parse_ipv4;
	return select(ethernet_hdr.etherType) {
		ETHERTYPE_IPV4: parse_ipv4;
		default: ingress;
	}
}

parser parse_ipv4 {
	extract(ipv4_hdr);
	//return parse_udp;
	return select(ipv4_hdr.protocol) {
		PROTOTYPE_UDP: parse_udp;
		default: ingress;
	}
}

parser parse_udp {
	extract(udp_hdr);
	return parse_op;
}

// op_hdr -> scan_hdr -> split_hdr -> vallen_hdr -> val_hdr -> shadowtype_hdr -> seq_hdr -> inswitch_hdr -> stat_hdr -> clone_hdr/frequency_hdr/validvalue_hdr

parser parse_op {
	extract(op_hdr);
	return select(op_hdr.optype) {
		//CACHE_POP_INSWITCH_ACK: parse_clone;
		CACHE_EVICT_LOADFREQ_INSWITCH_ACK: parse_frequency;
		1 mask 0x01: parse_vallen;
		/*2 mask 0x02: parse_seq;
		4 mask 0x04: parse_inswitch;
		8 mask 0x08: parse_stat;*/
		2 mask 0x02: parse_shadowtype;
		4 mask 0x04: parse_shadowtype;
		8 mask 0x08: parse_shadowtype;
#ifdef RANGE_SUPPORT
		SCANREQ: parse_scan;
		SCANREQ_SPLIT: parse_scan;
#endif
		default: ingress;
		//default: parse_debug;
		
		/*GETREQ_INSWITCH: parse_inswitch;
		GETRES: parse_vallen;
		GETRES_LATEST_SEQ: parse_vallen;
		GETRES_LATEST_SEQ_INSWITCH: parse_vallen;
		GETRES_LATEST_SEQ_INSWITCH_CASE1: parse_vallen;
		GETRES_DELETED_SEQ: parse_vallen;
		GETRES_DELETED_SEQ_INSWITCH: parse_vallen;
		GETRES_DELETED_SEQ_INSWITCH_CASE1: parse_vallen;
		CACHE_POP_INSWITCH: parse_vallen;
		PUTREQ: parse_vallen;
		PUTREQ_INSWITCH: parse_vallen;
		PUTREQ_SEQ: parse_vallen;
		PUTREQ_POP_SEQ: parse_vallen;
		PUTREQ_SEQ_INSWITCH_CASE1: parse_vallen;
		PUTREQ_SEQ_CASE3: parse_vallen;
		PUTREQ_POP_SEQ_CASE3: parse_vallen;
		PUTRES: parse_stat;
		DELREQ_INSWITCH: parse_inswitch;
		DELREQ_SEQ: parse_seq;
		DELREQ_SEQ_INSWITCH_CASE1: parse_vallen;
		DELREQ_SEQ_CASE3: parse_seq;
		DELRES: parse_stat;
#ifdef RANGE_SUPPORT
		SCANREQ: parse_scan;
		SCANREQ_SPLIT: parse_scan;
#endif
		default: ingress;
		//default: parse_debug; // GETREQ, GETREQ_POP, GETREQ_NLATEST, DELREQ*/
	}
}

#ifdef RANGE_SUPPORT
parser parse_scan {
	extract(scan_hdr);
	return select(op_hdr.optype) {
		SCANREQ_SPLIT: parse_split;
		default: ingress; // SCANREQ
	}
}

parser parse_split {
	extract(split_hdr);
	return ingress;
}
#endif

parser parse_vallen {
	extract(vallen_hdr);
	return select(vallen_hdr.vallen) {
		//0: parse_val_len0;
		0: parse_shadowtype;
		0 mask 0xFFFFFFF8: parse_val_len1;
		8: parse_val_len1;
		8 mask 0xFFFFFFF8: parse_val_len2;
		16: parse_val_len2;
		16 mask 0xFFFFFFF8: parse_val_len3;
		24: parse_val_len3;
		24 mask 0xFFFFFFF8: parse_val_len4;
		32: parse_val_len4;
		32 mask 0xFFFFFFF8: parse_val_len5;
		40: parse_val_len5;
		40 mask 0xFFFFFFF8: parse_val_len6;
		48: parse_val_len6;
		48 mask 0xFFFFFFF8: parse_val_len7;
		56: parse_val_len7;
		56 mask 0xFFFFFFF8: parse_val_len8;
		64: parse_val_len8;
		64 mask 0xFFFFFFF8: parse_val_len9;
		72: parse_val_len9;
		72 mask 0xFFFFFFF8: parse_val_len10;
		80: parse_val_len10;
		80 mask 0xFFFFFFF8: parse_val_len11;
		88: parse_val_len11;
		88 mask 0xFFFFFFF8: parse_val_len12;
		96: parse_val_len12;
		96 mask 0xFFFFFFF8: parse_val_len13;
		104: parse_val_len13;
		104 mask 0xFFFFFFF8: parse_val_len14;
		112: parse_val_len14;
		112 mask 0xFFFFFFF8: parse_val_len15;
		120: parse_val_len15;
		120 mask 0xFFFFFFF8: parse_val_len16;
		128: parse_val_len16;
		//default: parse_val_len0; // > 128
		default: parse_shadowtype; // > 128
	}
}

/*parser parse_val_len0 {
	return select(op_hdr.optype) {
		2 mask 0x02: parse_seq;
		4 mask 0x04: parse_inswitch;
		8 mask 0x08: parse_stat;
		default: ingress;
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
		//default: ingress; // PUTREQ
		////default: parse_debug; // PUTREQ
	}
}*/

parser parse_val_len1 {
	extract(val1_hdr);
	//return parse_val_len0;
	return parse_shadowtype;
}

parser parse_val_len2 {
	extract(val2_hdr);
	return parse_val_len1;
}

parser parse_val_len3 {
	extract(val3_hdr);
	return parse_val_len2;
}

parser parse_val_len4 {
	extract(val4_hdr);
	return parse_val_len3;
}

parser parse_val_len5 {
	extract(val5_hdr);
	return parse_val_len4;
}

parser parse_val_len6 {
	extract(val6_hdr);
	return parse_val_len5;
}

parser parse_val_len7 {
	extract(val7_hdr);
	return parse_val_len6;
}

parser parse_val_len8 {
	extract(val8_hdr);
	return parse_val_len7;
}

parser parse_val_len9 {
	extract(val9_hdr);
	return parse_val_len8;
}

parser parse_val_len10 {
	extract(val10_hdr);
	return parse_val_len9;
}

parser parse_val_len11 {
	extract(val11_hdr);
	return parse_val_len10;
}

parser parse_val_len12 {
	extract(val12_hdr);
	return parse_val_len11;
}

parser parse_val_len13 {
	extract(val13_hdr);
	return parse_val_len12;
}

parser parse_val_len14 {
	extract(val14_hdr);
	return parse_val_len13;
}

parser parse_val_len15 {
	extract(val15_hdr);
	return parse_val_len14;
}

parser parse_val_len16 {
	extract(val16_hdr);
	return parse_val_len15;
}

parser parse_shadowtype {
	extract(shadowtype_hdr);
	return select(shadowtype_hdr.shadowtype) {
		2 mask 0x02: parse_seq;
		4 mask 0x04: parse_inswitch;
		8 mask 0x08: parse_stat;
		default: ingress;
		//default: parse_debug;
	}
}

parser parse_seq {
	extract(seq_hdr);
	//return select(op_hdr.optype) {
	return select(shadowtype_hdr.shadowtype) {
		4 mask 0x04: parse_inswitch;
		8 mask 0x08: parse_stat;
		default: ingress;
		//default: parse_debug;
		
		/*GETRES_LATEST_SEQ_INSWITCH: parse_inswitch;
		GETRES_LATEST_SEQ_INSWITCH_CASE1: parse_inswitch;
		GETRES_DELETED_SEQ_INSWITCH: parse_inswitch;
		GETRES_DELETED_SEQ_INSWITCH_CASE1: parse_inswitch;
		CACHE_POP_INSWITCH: parse_inswitch; // inswitch_hdr is set by switchos
		PUTREQ_SEQ_INSWITCH_CASE1: parse_inswitch;
		DELREQ_SEQ_INSWITCH_CASE1: parse_inswitch;
		default: ingress;
		//default: parse_debug; // GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3, DELREQ_SEQ, DELREQ_SEQ_CASE3 */
	}
}

parser parse_inswitch {
	extract(inswitch_hdr);
	//return select(op_hdr.optype) {
	return select(shadowtype_hdr.shadowtype) {
		SETVALID_INSWITCH: parse_validvalue;
		8 mask 0x08: parse_stat;
		default: ingress;
		//default: parse_debug;
		
		/*GETRES_LATEST_SEQ_INSWITCH_CASE1: parse_stat;
		GETRES_DELETED_SEQ_INSWITCH_CASE1: parse_stat;
		PUTREQ_SEQ_INSWITCH_CASE1: parse_stat;
		DELREQ_SEQ_INSWITCH_CASE1: parse_stat;
		default: ingress;
		//default: parse_debug; // GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, PUTREQ_INSWITCH, DELREQ_INSWITCH, CACHE_POP_INSWITCH */
	}
}

parser parse_stat {
	extract(stat_hdr);
	return select(shadowtype_hdr.shadowtype) {
		GETRES_LATEST_SEQ_INSWITCH_CASE1: parse_clone;
		GETRES_DELETED_SEQ_INSWITCH_CASE1: parse_clone;
		PUTREQ_SEQ_INSWITCH_CASE1: parse_clone;
		DELREQ_SEQ_INSWITCH_CASE1: parse_clone;
		default: ingress;
		//default: parse_debug;
	}
	//return ingress;
	////return parse_debug; // GETRES, PUTRES, DELRES, GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1
}

parser parse_clone {
	extract(clone_hdr);
	return ingress;
	//return parse_debug; // GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1
	// CACHE_POP_INSWITCH_ACK does not need clond_hdr now
}

parser parse_frequency {
	extract(frequency_hdr);
	return ingress; // CACHE_EVICT_LOADFREQ_INSWITCH_ACK
}

parser parse_validvalue {
	extract(validvalue_hdr);
	return ingress; // SETVALID_INSWITCH
}

/*parser parse_debug {
	extract(debug_hdr);
	return ingress;
}*/

// checksum calculation (deparser phase)

field_list ipv4_field_list {
    ipv4_hdr.version;
    ipv4_hdr.ihl;
    ipv4_hdr.diffserv;
    ipv4_hdr.totalLen;
    ipv4_hdr.identification;
    ipv4_hdr.flags;
    ipv4_hdr.fragOffset;
    ipv4_hdr.ttl;
    ipv4_hdr.protocol;
    ipv4_hdr.srcAddr;
    ipv4_hdr.dstAddr;
}

field_list_calculation ipv4_chksum_calc {
    input {
        ipv4_field_list;
    }
#ifndef __p4c__
    algorithm : csum16;
#else
    algorithm : crc16;
#endif
    output_width: 16;
}

// NOTE: verify means check the calculated_field at packet ingress (aka parser)
// NOTE: update means change the calculated_field at packet egress (akak deparser)
calculated_field ipv4_hdr.hdrChecksum {
    update ipv4_chksum_calc;
}
