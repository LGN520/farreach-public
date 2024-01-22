/* Parser */

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11

// NOTE: due to hardware limitation, we cannot make too many branches in switch expression

// NOTE: carefully assign optype to reduce branches
// (1) vallen&value: mask 0b0001; seq: mask 0b0010; inswitch_hdr: mask 0b0100; stat: mask 0b1000;
// (2) scan/split: specific value (X + 0b0000); not parsed optypes: X + 0b0000
// op_hdr + vallen&value + shadowtype (0b0001): PUTREQ, WARMUPREQ (deprecated), LOADREQ
// op_hdr + vallen&value + shadowtype + seq (0b0011): PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3, NETCACHE_PUTREQ_SEQ_CACHED
// op_hdr + vallen&value + shadowtype + seq + inswitch_hdr (0b0111): NONE
// op_hdr + vallen&value + shadowtype + seq + inswitch_hdr + stat (0b1111) (XXX_CASE1 w/ clone_hdr): GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1, LOADSNAPSHOTDATA_INSWITCH_ACK, CACHE_POP_INSWITCH, NETCACHE_VALUEUPDATE_INSWITCH
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

parser nocahceParser(packet_in packet,
                out headers hdr,
                out metadata meta,
                out ingress_intrinsic_metadata_t  ig_intr_md){
    state start {
        packet.extract(ig_intr_md);
        packet.advance(PORT_METADATA_SIZE);
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
	// op_hdr -> scan_hdr -> split_hdr -> vallen_hdr -> val_hdr -> shadowtype_hdr -> seq_hdr -> inswitch_hdr -> stat_hdr -> clone_hdr/frequency_hdr/validvalue_hdr
	state parse_op {
		packet.extract(hdr.op_hdr);
		transition select(hdr.op_hdr.optype) {
			default: accept;
		}
	}
}
control IngressDeparser(packet_out pkt,
    /* User */
    inout headers                       hdr,
    in    metadata                      meta,
    /* Intrinsic */
    in    ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md)
{
	apply{
		pkt.emit(hdr);
	}
}

parser EgressParser(packet_in      packet,
    /* User */
    out headers          hdr,
    out metadata         meta,
    /* Intrinsic */
    out egress_intrinsic_metadata_t  eg_intr_md)
{
    /* This is a mandatory state, required by Tofino Architecture */
	state start {
		packet.extract(eg_intr_md);
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
	// op_hdr -> scan_hdr -> split_hdr -> vallen_hdr -> val_hdr -> shadowtype_hdr -> seq_hdr -> inswitch_hdr -> stat_hdr -> clone_hdr/frequency_hdr/validvalue_hdr
	state parse_op {
		packet.extract(hdr.op_hdr);
		transition select(hdr.op_hdr.optype) {
			default: accept;
		}
	}
}
control nocahceDeparser(packet_out packet,
    /* User */
    inout headers                       hdr,
    in    metadata                      meta,
    /* Intrinsic */
    in    egress_intrinsic_metadata_for_deparser_t  eg_dprsr_md) {
    Checksum() ipv4_checksum;

    apply {
        if (hdr.ipv4_hdr.isValid())
        {
            hdr.ipv4_hdr.hdrChecksum = ipv4_checksum.update({
                hdr.ipv4_hdr.version,
                hdr.ipv4_hdr.ihl,
                hdr.ipv4_hdr.diffserv,
                hdr.ipv4_hdr.totalLen,
                hdr.ipv4_hdr.identification,
                hdr.ipv4_hdr.flags,
                hdr.ipv4_hdr.fragOffset,
                hdr.ipv4_hdr.ttl,
                hdr.ipv4_hdr.protocol,
                hdr.ipv4_hdr.srcAddr,
                hdr.ipv4_hdr.dstAddr
            });
        }
        packet.emit(hdr);
    }
	// apply {

    //     //parsed headers have to be added again into the packet.
	// 	packet.emit(hdr);
    // }
}