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
    apply {    }
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

