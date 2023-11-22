/* Packet Header Types */

// PUT ipv4 in T-PHV

header ethernet_t {
    bit<48> dstAddr;
    bit<48> srcAddr;
    bit<16> etherType;
}

header ipv4_t {
    bit<4> version;
    bit<4> ihl;
    bit<8> diffserv;
    bit<16> totalLen;
    bit<16> identification;
    bit<3> flags;
    bit<13> fragOffset;
    bit<8> ttl;
    bit<8> protocol;
    bit<16> hdrChecksum;
    bit<32> srcAddr;
    bit<32> dstAddr;
}

header udp_t {
    bit<16> srcPort;
    bit<16> dstPort;
    bit<16> hdrlen;
    bit<16> checksum;
}

header op_t {
    bit<16> optype;
    bit<32> keylolo;
    bit<32> keylohi;
    bit<32> keyhilo;
    bit<16> keyhihilo;
    bit<16> keyhihihi;
}

#ifdef RANGE_SUPPORT

header scan_t {
    bit<32> keylolo;
    bit<32> keylohi;
    bit<32> keyhilo;
	//  bit<32> keyhihi;
    bit<16> keyhihilo;
    bit<16> keyhihihi;
}
header split_t {
    bit<8> is_clone;
    bit<16> globalserveridx;
    bit<16> cur_scanidx;
    bit<16> max_scannum;
}

#endif

// Used by PUTREQ and GETRES to save PHV

header vallen_t {
    bit<16> vallen;
}

header val_t {
    bit<32> vallo;
    bit<32> valhi;
}

// Header instances
struct headers {
    ethernet_t ethernet_hdr;
    ipv4_t ipv4_hdr;
    udp_t udp_hdr;
    op_t op_hdr;
#ifdef RANGE_SUPPORT
    scan_t scan_hdr;
    split_t split_hdr;
#endif
}
// metadata metadata_t meta;
struct metadata {
    /* empty */
    bit<16> hashval_for_partition; // at most 32K
    bit<16> hashval_for_spine_partition; // at most 32K
    bit<16> spineswitchidx;
    bit<16> leafswitchidx;

    bit<1> is_spine;//0for leaf 1 for spine
}

