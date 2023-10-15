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


// Used by PUTREQ and GETRES to save PHV
header vallen_t {
    bit<16> vallen;
}

header val_t {

    bit<32> vallo;
    bit<32> valhi;
}

header shadowtype_t {
    bit<16> shadowtype;
}

header seq_t {

    bit<32> seq;
    bit<32> snapshot_token;
}

// NOTE: inswicth_t affects INSWITCH_PREV_BYTES in packet_format.h
header inswitch_t {

		// 32-bit container
    bit<1> snapshot_flag;
    bit<1> is_cached;
    bit<1> is_sampled;
    bit<10> client_sid; // clone to client for cache hit; NOTE: clone_e2e sets eg_intr_md_for_mb.mirror_id w/ 10 bits
    bit<3> padding;
    bit<16> hot_threshold;
		// 32-bit containers
    bit<16> hashval_for_cm1; // at most 64K
    bit<16> hashval_for_cm2; // at most 64K
    bit<16> hashval_for_cm3; // at most 64K
    bit<16> hashval_for_cm4; // at most 64K
    bit<32> snapshot_token; // NOTE: only place it before inswitch_hdr.idx, we can simply change INSWITCH_PREVBYTES in libcommon
    bit<16> hashval_for_seq; // at most 32K
    bit<16> idx; // index for in-switch cache
}

header stat_t {

    bit<8> stat;
    bit<16> nodeidx_foreval; // cache hit: 0xFFFF; cache miss: [0, servernum-1]
    bit<8> padding;
}

header clone_t {

    bit<16> clonenum_for_pktloss;
    bit<16> client_udpport;
    bit<32> assignedseq_for_farreach;
}

header frequency_t {
    bit<32> frequency;
}

header validvalue_t {
    bit<8> validvalue; // used by SETVALID_INSWITCH to set validvalue_reg
}

header fraginfo_t {

    bit<16> padding1; // clientlogicalidx in T-PHV
    bit<32> padding2; // fragseq in T-PHV
    bit<16> cur_fragidx;
    bit<16> max_fragnum;
}

struct metadata_t {

    bit<1> need_recirculate;
#ifndef RANGE_SUPPORT
    bit<16> hashval_for_partition; // at most 32K
#endif
    bit<4> cm1_predicate;
    bit<4> cm2_predicate;
    bit<4> cm3_predicate;
    bit<4> cm4_predicate;
    bit<1> is_hot;
    bit<1> is_latest; // if the entry is latest
//#ifdef ENABLE_LARGEVALUEBLOCK
    bit<32> largevalueseq;
    bit<1> is_largevalueblock;
//#endif
    bit<1> is_deleted; // if the entry is deleted
    bit<1> is_case1;
    bit<1> is_lastclone_for_pktloss;
    bit<4> access_val_mode; // 0: not access val_reg; 1: get; 2: set_and_get; 3: reset_and_get
    bit<16> recirport;
}

/*header debug_t {

		// 8-bit container
    bit<1> is_hot;
    bit<1> is_lastclone_for_pktloss;
    bit<6> padding;
}*/

// Header instances
struct headers {
	#ifdef USE_BFSDE920
	@pragma pa_no_overlay ingress ethernet_hdr.dstAddr
	@pragma pa_no_overlay egress ethernet_hdr.dstAddr
	@pragma pa_no_overlay ingress ethernet_hdr.srcAddr
	@pragma pa_no_overlay egress ethernet_hdr.srcAddr
	@pragma pa_no_overlay ingress ethernet_hdr.etherType
	@pragma pa_no_overlay egress ethernet_hdr.etherType
	#endif
	ethernet_t ethernet_hdr;
	/*@pragma pa_no_overlay ingress ipv4_hdr.srcAddr
	@pragma pa_no_overlay egress ipv4_hdr.srcAddr
	@pragma pa_no_overlay ingress ipv4_hdr.dstAddr
	@pragma pa_no_overlay egress ipv4_hdr.dstAddr
	@pragma pa_no_overlay ingress ipv4_hdr.protocol
	@pragma pa_no_overlay egress ipv4_hdr.protocol*/
	ipv4_t ipv4_hdr;
	/*@pragma pa_no_overlay ingress udp_hdr.srcPort
	@pragma pa_no_overlay egress udp_hdr.srcPort
	@pragma pa_no_overlay ingress udp_hdr.dstPort
	@pragma pa_no_overlay egress udp_hdr.dstPort
	@pragma pa_no_overlay ingress udp_hdr.hdrlen
	@pragma pa_no_overlay egress udp_hdr.hdrlen*/
	udp_t udp_hdr;
	op_t op_hdr;
	vallen_t vallen_hdr;
	val_t val1_hdr;
	val_t val2_hdr;
	val_t val3_hdr;
	val_t val4_hdr;
	val_t val5_hdr;
	val_t val6_hdr;
	val_t val7_hdr;
	val_t val8_hdr;
	val_t val9_hdr;
	val_t val10_hdr;
	val_t val11_hdr;
	val_t val12_hdr;
	val_t val13_hdr;
	val_t val14_hdr;
	val_t val15_hdr;
	val_t val16_hdr;
	shadowtype_t shadowtype_hdr;
	seq_t seq_hdr;
	//@pragma pa_no_overlay ingress inswitch_hdr.idx
	//@pragma pa_no_overlay egress inswitch_hdr.idx
	inswitch_t inswitch_hdr;
	stat_t stat_hdr;
	clone_t clone_hdr;
	frequency_t frequency_hdr;
	validvalue_t validvalue_hdr;
	@pragma pa_no_overlay ingress fraginfo_hdr.padding1
	@pragma pa_no_overlay egress fraginfo_hdr.padding1
	@pragma pa_no_overlay ingress fraginfo_hdr.padding2
	@pragma pa_no_overlay egress fraginfo_hdr.padding2
	@pragma pa_no_overlay ingress fraginfo_hdr.cur_fragidx
	@pragma pa_no_overlay egress fraginfo_hdr.cur_fragidx
	@pragma pa_no_overlay ingress fraginfo_hdr.max_fragnum
	@pragma pa_no_overlay egress fraginfo_hdr.max_fragnum
	fraginfo_t fraginfo_hdr;
	//@pragma pa_no_overlay ingress meta.meta.udp_hdrlen
	//@pragma pa_no_overlay egress meta.meta.udp_hdrlen
}
struct metadata {
    /* empty */
    metadata_t meta;
}

//header debug_t debug_hdr;
