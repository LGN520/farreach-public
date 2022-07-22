/* Parser */

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11

// NOTE: due to hardware limitation, we cannot make too many branches in switch expression

// NOTE: carefully assign optype to reduce branches
// (1) vallen&value: mask 0b0001; seq: mask 0b0010; inswitch_hdr: mask 0b0100; stat: mask 0b1000;
// (2) scan/split: specific value (X + 0b0000); not parsed optypes: X + 0b0000
// op_hdr + vallen&value + shadowtype (0b0001): PUTREQ, WARMUPREQ (deprecated), LOADREQ, LOADREQ_SPINE
// op_hdr + vallen&value + shadowtype + seq (0b0011): PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3, NETCACHE_PUTREQ_SEQ_CACHED
// op_hdr + shadowtype + seq + inswitch_hdr (0b0110): DELREQ_SEQ_INSWITCH
// op_hdr + vallen&value + shadowtype + seq + inswitch_hdr (0b0111): PUTREQ_SEQ_INSWITCH
// op_hdr + vallen&value + shadowtype + seq + inswitch_hdr + stat (0b1111) (XXX_CASE1 w/ clone_hdr): GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1, LOADSNAPSHOTDATA_INSWITCH_ACK, CACHE_POP_INSWITCH, NETCACHE_VALUEUPDATE_INSWITCH
// op_hdr + vallen&value + shadowtype + seq + stat (0b1011): GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, CACHE_EVICT_LOADDATA_INSWITCH_ACK, NETCACHE_VALUEUPDATE, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER
// op_hdr + vallen&value + shadowtype + stat (0b1001): GETRES, GETRES_SERVER
// op_hdr + vallen&value + shadowtype + inswitch (0b0101): PUTREQ_INSWITCH
// op_hdr + shadowtype + inswitch_hdr (0b0100): GETREQ_INSWITCH, DELREQ_INSWITCH, CACHE_EVICT_LOADFREQ_INSWITCH, CACHE_EVICT_LOADDATA_INSWITCH, SETVALID_INSWITCH (w/ validvalue_hdr), NETCACHE_WARMUPREQ_INSWITCH, NETCACHE_WARMUPREQ_INSWITCH_POP (w/ clone_hdr)
// op_hdr + shadowtype + seq (0b0010): DELREQ_SEQ, DELREQ_SEQ_CASE3, NETCACHE_DELREQ_SEQ_CACHED
// op_hdr + shadowtype + stat (0b1000): PUTRES, DELRES, PUTRES_SERVER, DELRES_SERVER
// NOTE: followings are ended with 0b0000
// op_hdr + scan_hdr (specific value): SCANREQ
// op_hdr + scan_hdr + split_hdr (specific value): SCANREQ_SPLIT
// only op_hdr (default): WARMUPREQ, GETREQ, DELREQ, GETREQ_POP, GETREQ_NLATEST, CACHE_POP_INSWITCH_ACK (deprecated: w/ clone_hdr), WARMUPACK, LOADACK, CACHE_POP_ACK, CACHE_EVICT_LOADFREQ_INSWITCH_ACK (w/ frequency_hdr), NETCACHE_GETREQ_POP, NETCACHE_VALUEUPATE_ACK, GETREQ_SPINE, SCANRES_SPLIT/_SERVER (w/ split_hdr + scan data), WAMRUPREQ_SPINE, WARMUPACK_SERVER, LOADACK_SERVER
// not parsed in switch: CACHE_POP, CACHE_EVICT, CACHE_EVICT_ACK, CACHE_EVICT_CASE2, CACHE_POP_ACK, CACHE_EVICT_LOADFREQ_INSWITCH_ACK, SETVALID_INSWITCH_ACK, NETCACHE_CACHE_POP/_ACK, NETCACHE_CACHE_POP_FINISH/_ACK, NETCACHE_CACHE_EVICT/_ACK, DISTCACHE_CACHE_EVICT_VICTIM/_ACK

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
		PROTOTYPE_UDP: parse_udp_dstport;
		default: ingress;
	}
}

parser parse_udp_dstport {
	extract(udp_hdr);
	return select(udp_hdr.dstPort) {
		0x0480 mask 0xFF80: parse_op; // reserve multiple udp port due to server simulation
		5008: parse_op; // reserve reflector.dp2cpserver_port due to hardware link simulation between switch and switchos
		default: parse_udp_srcport;
	}
}

parser parse_udp_srcport {
	return select(udp_hdr.srcPort) {
		0x0480 mask 0xFF80: parse_op; // reserve multiple udp port due to server simulation
		5009: parse_op; // reserve reflector.cp2dpserver_port due to hardware link simulation between switch and switchos
		default: ingress; // traditional packet
	}
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
