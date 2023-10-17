#include <core.p4>
#include <v1model.p4>

// macros
#include "p4src/macro.p4"

// headers
#include "p4src/header.p4"

// parsers
#include "p4src/parser.p4"


#include "p4src/ingress.p4"
#include "p4src/egress.p4"
//blank checksum
control farreachVerifyChecksum(inout headers hdr, inout metadata meta) {
    apply {  }
}
control farreachComputeChecksum(inout headers hdr, inout metadata meta) {
    apply { 
        update_checksum(
	    hdr.ipv4_hdr.isValid(),
            { hdr.ipv4_hdr.version,
	      hdr.ipv4_hdr.ihl,
              hdr.ipv4_hdr.diffserv,
              hdr.ipv4_hdr.totalLen,
              hdr.ipv4_hdr.identification,
              hdr.ipv4_hdr.flags,
              hdr.ipv4_hdr.fragOffset,
              hdr.ipv4_hdr.ttl,
              hdr.ipv4_hdr.protocol,
              hdr.ipv4_hdr.srcAddr,
              hdr.ipv4_hdr.dstAddr },
            hdr.ipv4_hdr.hdrChecksum,
            HashAlgorithm.csum16);
        update_checksum_with_payload(
            hdr.udp_hdr.isValid(), 
            {  hdr.ipv4_hdr.srcAddr, 
                hdr.ipv4_hdr.dstAddr, 
                8w0, 
                hdr.ipv4_hdr.protocol, 
                hdr.udp_hdr.hdrlen, 
                hdr.udp_hdr.srcPort, 
                hdr.udp_hdr.dstPort, 
                hdr.udp_hdr.hdrlen,
                hdr.op_hdr,
                hdr.vallen_hdr,
                hdr.val1_hdr,
                hdr.val2_hdr,
                hdr.val3_hdr,
                hdr.val4_hdr,
                hdr.val5_hdr,
                hdr.val6_hdr,
                hdr.val7_hdr,
                hdr.val8_hdr,
                hdr.val9_hdr,
                hdr.val10_hdr,
                hdr.val11_hdr,
                hdr.val12_hdr,
                hdr.val13_hdr,
                hdr.val14_hdr,
                hdr.val15_hdr,
                hdr.val16_hdr,
                hdr.shadowtype_hdr,
                hdr.seq_hdr,
                hdr.inswitch_hdr,
                hdr.stat_hdr,
                hdr.clone_hdr,
                hdr.frequency_hdr,
                hdr.validvalue_hdr,
                hdr.fraginfo_hdr
                
            }, 
            hdr.udp_hdr.checksum, 
            HashAlgorithm.csum16
        ); 
    }
}
//switch architecture
V1Switch(
farreachParser(),
farreachVerifyChecksum(),
farreachIngress(),
farreachEgress(),
farreachComputeChecksum(),
farreachDeparser()
) main;

