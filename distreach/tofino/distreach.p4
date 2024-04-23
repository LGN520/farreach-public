#include <core.p4>
#include <tna.p4>
#define ENABLE_LARGEVALUEBLOCK
// macros
#include "p4src/macro.p4"

// headers
#include "p4src/header.p4"

// parsers
#include "p4src/parser.p4"


#include "p4src/ingress.p4"
#include "p4src/egress.p4"


Pipeline(
    farreachParser(),
    farreachIngress(),
    IngressDeparser(),
    EgressParser(),
    farreachEgress(),
    farreachDeparser()
) pipe;

Switch(pipe) main;