/* Packet Header Types */
typedef bit<8>  pkt_type_t;
const pkt_type_t PKT_TYPE_NORMAL = 1;
const pkt_type_t PKT_TYPE_MIRROR = 2;
header mirror_h {
  	pkt_type_t  pkt_type;
}

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
}
// NOTE: inswicth_t affects INSWITCH_PREV_BYTES in packet_format.h
header inswitch_t {
	// 32-bit container
	bit<1> is_cached;
	bit<1> is_sampled;
	bit<10> client_sid; // clone to client for cache hit; NOTE: clone_e2e sets eg_intr_md_for_mb.mirror_id w/ 10 bits
	bit<4> padding1;
	bit<16> hot_threshold;
	// 32-bit containers
	bit<16> hashval_for_cm1; // at most 64K
	bit<16> hashval_for_cm2; // at most 64K
	bit<16> hashval_for_cm3; // at most 64K
	bit<16> hashval_for_cm4; // at most 64K
	// using multiple paddings due to PHV limitation: total bit width of split containers of a header field cannot exceed 32 bits
	bit<18> hashval_for_bf1; // at most 256K
	bit<14> padding2;
	bit<18> hashval_for_bf2; // at most 256K
	bit<14> padding3;
	bit<18> hashval_for_bf3; // at most 256K
	bit<14> padding4;
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
	bit<10> server_sid; // clone to server for SCANREQ_SPLIT or last cloned NETCACHE_GETREQ_POP
	bit<6> padding;
	bit<16> server_udpport;
}

header frequency_t {
	bit<32> frequency;
}

header fraginfo_t {
	bit<16> padding1; // clientlogicalidx in T-PHV
	bit<32> padding2; // fragseq in T-PHV
	bit<16> cur_fragidx;
	bit<16> max_fragnum;
}

struct ingress_metadata {
#ifndef RANGE_SUPPORT
	bit<16>	hashval_for_partition; // at most 32K
#endif
}

struct egress_metadata {
	MirrorId_t egr_mir_ses;
	bit<6> padding1;
	bit<2> cm1_predicate;
	bit<2> cm2_predicate;
	bit<2> cm3_predicate;
	bit<2> cm4_predicate;
	bit<1> is_hot;
	bit<1> is_report1;
	bit<1> is_report2;
	bit<1> is_report3;
	bit<1> is_report;
	bit<1> is_latest; // if the entry is latest
	bit<1> is_deleted; // if the entry is deleted
	bit<1> is_lastclone_for_pktloss;
	bit<4> access_val_mode; // 0: not access val_reg; 1: get; 2: set_and_get; 3: reset_and_get
	bit<4> padding;
	pkt_type_t  pkt_type;
}
// Header instances
struct headers {
	ethernet_t ethernet_hdr;
	ipv4_t ipv4_hdr;
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
	inswitch_t inswitch_hdr;
	stat_t stat_hdr;
	clone_t clone_hdr;

	
	// @pa_no_overlay("ingress","fraginfo_hdr.padding1")
	// @pa_no_overlay("egress","fraginfo_hdr.padding1")
	// @pa_no_overlay("ingress","fraginfo_hdr.padding2")
	// @pa_no_overlay("egress","fraginfo_hdr.padding2")
	// @pa_no_overlay("ingress","fraginfo_hdr.cur_fragidx")
	// @pa_no_overlay("egress","fraginfo_hdr.cur_fragidx")
	// @pa_no_overlay("ingress","fraginfo_hdr.max_fragnum")
	// @pa_no_overlay("egress","fraginfo_hdr.max_fragnum")
	frequency_t frequency_hdr;
	fraginfo_t fraginfo_hdr;
}
@pa_no_overlay("egress","eg_intr_md_for_dprsr.drop_ctl")
@pa_no_overlay("egress","eg_intr_md.egress_port")
@pa_no_overlay("egress","meta.egr_mir_ses")
@pa_no_overlay("ingress","op_hdr.optype")
@pa_no_overlay("egress","op_hdr.optype")
@pa_no_overlay("egress","hdr.op_hdr.optype")
@pa_no_overlay("egress","hdr.inswitch_hdr.is_cached")
@pa_no_overlay("egress","meta.is_hot")
@pa_no_overlay("egress","meta.is_report") 
@pa_no_overlay("egress","meta.is_latest")
@pa_no_overlay("egress","meta.is_deleted")
@pa_no_overlay("egress","hdr.inswitch_hdr.client_sid")
@pa_no_overlay("egress","meta.is_lastclone_for_pktloss")
@pa_no_overlay("egress","hdr.clone_hdr.server_sid")

