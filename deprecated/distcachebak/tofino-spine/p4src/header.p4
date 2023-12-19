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
    bit<16> spineswitchidx;
    bit<16> leafswitchidx;
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
    bit<32> client_ip;
    bit<48> client_mac;
}

header frequency_t {
    bit<32> frequency;
}

header switchload_t {
    bit<32> spineload;
    bit<32> leafload;
}

header fraginfo_t {
    bit<16> padding1; // clientlogicalidx in T-PHV
    bit<32> padding2; // fragseq in T-PHV
    bit<16> cur_fragidx;
    bit<16> max_fragnum;
}

struct metadata {
#ifndef RANGE_SUPPORT
    bit<16> hashval_for_partition; // at most 32K
#endif
    bit<1> is_report1;
    bit<1> is_report2;
    bit<1> is_report3;
    bit<1> is_report;
    bit<1> is_latest; // if the entry is latest
    bit<1> is_deleted; // if the entry is deleted
    bit<1> is_lastclone_for_pktloss;
    bit<4> access_val_mode; // 0: not access val_reg; 1: get; 2: set_and_get; 3: reset_and_get
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
	frequency_t frequency_hdr;
	switchload_t switchload_hdr;

	fraginfo_t fraginfo_hdr;
}
// metadata metadata_t meta;

//header debug_t debug_hdr;
