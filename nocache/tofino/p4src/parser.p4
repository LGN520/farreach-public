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
	return select(ethernet_hdr.etherType) {
		ETHERTYPE_IPV4: parse_ipv4;
		default: ingress;
	}
}

parser parse_ipv4 {
	extract(ipv4_hdr);
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
#ifdef RANGE_SUPPORT
		SCANREQ: parse_scan;
		SCANREQ_SPLIT: parse_scan;
#endif
		default: ingress;
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
