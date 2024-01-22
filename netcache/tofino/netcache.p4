// #include "tofino/constants.p4"
// #include "tofino/intrinsic_metadata.p4"
// #include "tofino/stateful_alu_blackbox.p4"
// #include "tofino/primitives.p4"

#include <core.p4>
#include <tna.p4>

// macro
#include "p4src/macro.p4"
// headers
#include "p4src/header.p4"

// parsers
#include "p4src/parser.p4"

#include "p4src/ingress_mat.p4"
#include "p4src/egress_mat.p4"


/* Ingress Processing */

//blank checksum

Pipeline(
    netcacheParser(),
    netcacheIngress(),
    IngressDeparser(),
    EgressParser(),
    netcacheEgress(),
    netcacheDeparser()
) pipe;

Switch(pipe) main;