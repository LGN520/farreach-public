// #include "tofino/constants.p4"
// #include "tofino/intrinsic_metadata.p4"
// #include "tofino/stateful_alu_blackbox.p4"
// #include "tofino/primitives.p4"

#include <core.p4>
#include <v1model.p4>

// macro
#include "p4src/macro.p4"
// headers
#include "p4src/header.p4"

// parsers
#include "p4src/parser.p4"

#include "p4src/ingress_mat.p4"
#include "p4src/egress_mat.p4"


control partitionswitchVerifyChecksum(inout headers hdr, inout metadata meta) {
    apply {  }
}
control partitionswitchComputeChecksum(inout headers hdr, inout metadata meta) {
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
                hdr.op_hdr
            }, 
            hdr.udp_hdr.checksum, 
            HashAlgorithm.csum16
        ); 
    }
}
//switch architecture
V1Switch(
partitionswitchParser(),
partitionswitchVerifyChecksum(),
partitionswitchIngress(),
partitionswitchEgress(),
partitionswitchComputeChecksum(),
partitionswitchDeparser()
) main;