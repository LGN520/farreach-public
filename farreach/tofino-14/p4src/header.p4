/* Packet Header Types */

// PUT ipv4 in T-PHV

header_type ethernet_t {
	fields {
		dstAddr: 48;
		srcAddr: 48;
		etherType: 16;
	}
}

header_type ipv4_t {
	fields {
		version: 4;
		ihl: 4;
		diffserv: 8;
		totalLen: 16;
		identification: 16;
		flags: 3;
		fragOffset: 13;
		ttl: 8;
		protocol: 8;
		hdrChecksum: 16;
		srcAddr: 32;
		dstAddr: 32;
	}
}

header_type udp_t {
	fields {
		srcPort: 16;
		dstPort: 16;
		hdrlen: 16;
		checksum: 16;
	}
}

header_type op_t {
	fields {
		optype: 16;
		keylolo: 32;
		keylohi: 32;
		keyhilo: 32;
		//keyhihi: 32;
		keyhihilo: 16;
		keyhihihi: 16;
	}
}

#ifdef RANGE_SUPPORT
header_type scan_t {
	fields {
		keylolo: 32;
		keylohi: 32;
		keyhilo: 32;
		//keyhihi: 32;
		keyhihilo: 16;
		keyhihihi: 16;
	}
}
header_type split_t {
	fields {
		is_clone: 8;
		globalserveridx: 16;
		cur_scanidx: 16;
		max_scannum: 16;
	}
}
#endif

// Used by PUTREQ and GETRES to save PHV
header_type vallen_t {
	fields {
		vallen: 16;
	}
}

header_type val_t {
	fields {
		vallo: 32;
		valhi: 32;
	}
}

header_type shadowtype_t {
	fields {
		shadowtype: 16;
	}
}

header_type seq_t {
	fields {
		seq: 32;
		snapshot_token: 32;
	}
}

// NOTE: inswicth_t affects INSWITCH_PREV_BYTES in packet_format.h
header_type inswitch_t {
	fields {
		// 32-bit container
		snapshot_flag: 1;
		is_cached: 1;
		is_sampled: 1;
		client_sid: 10; // clone to client for cache hit; NOTE: clone_e2e sets eg_intr_md_for_mb.mirror_id w/ 10 bits
		padding: 3;
		hot_threshold: 16;
		// 32-bit containers
		hashval_for_cm1: 16; // at most 64K
		hashval_for_cm2: 16; // at most 64K
		hashval_for_cm3: 16; // at most 64K
		hashval_for_cm4: 16; // at most 64K
		snapshot_token: 32; // NOTE: only place it before inswitch_hdr.idx, we can simply change INSWITCH_PREVBYTES in libcommon
		hashval_for_seq: 16; // at most 32K
		idx: 16; // index for in-switch cache
	}
}

header_type stat_t {
	fields {
		stat: 8;
		nodeidx_foreval: 16; // cache hit: 0xFFFF; cache miss: [0, servernum-1]
		padding: 8;
	}
}

header_type clone_t {
	fields {
		clonenum_for_pktloss: 16;
		client_udpport: 16;
		assignedseq_for_farreach: 32;
	}
}

header_type frequency_t {
	fields {
		frequency: 32;
	}
}

header_type validvalue_t {
	fields {
		validvalue: 8; // used by SETVALID_INSWITCH to set validvalue_reg
	}
}

header_type fraginfo_t {
	fields {
		padding1: 16; // clientlogicalidx in T-PHV
		padding2: 32; // fragseq in T-PHV
		cur_fragidx: 16;
		max_fragnum: 16;
	}
}

header_type metadata_t {
	fields {
		need_recirculate: 1;
#ifndef RANGE_SUPPORT
		hashval_for_partition: 16; // at most 32K
#endif
		cm1_predicate: 4;
		cm2_predicate: 4;
		cm3_predicate: 4;
		cm4_predicate: 4;
		is_hot: 1;
		//validvalue: 8; // validvalue of the entry
		is_latest: 1; // if the entry is latest
//#ifdef ENABLE_LARGEVALUEBLOCK
		largevalueseq: 32;
		is_largevalueblock: 1;
//#endif
		is_deleted: 1; // if the entry is deleted
		is_case1: 1;
		is_lastclone_for_pktloss: 1;
		access_val_mode: 4; // 0: not access val_reg; 1: get; 2: set_and_get; 3: reset_and_get
#ifdef RANGE_SUPPORT
		server_sid: 10; // clone to server for SCANREQ_SPLIT
		remain_scannum: 16;
		is_last_scansplit: 1;
#endif
		//udp_hdrlen: 16;
		recirport: 16;
	}
}

/*header_type debug_t {
	fields {
		// 8-bit container
		is_hot: 1;
		is_lastclone_for_pktloss: 1;
		padding: 6;
	}
}*/

// Header instances

#ifdef USE_BFSDE920
// @pragma pa_no_overlay ingress ethernet_hdr.dstAddr
// @pragma pa_no_overlay egress ethernet_hdr.dstAddr
// @pragma pa_no_overlay ingress ethernet_hdr.srcAddr
// @pragma pa_no_overlay egress ethernet_hdr.srcAddr
// @pragma pa_no_overlay ingress ethernet_hdr.etherType
// @pragma pa_no_overlay egress ethernet_hdr.etherType
#endif
header ethernet_t ethernet_hdr;
/*// @pragma pa_no_overlay ingress ipv4_hdr.srcAddr
// @pragma pa_no_overlay egress ipv4_hdr.srcAddr
// @pragma pa_no_overlay ingress ipv4_hdr.dstAddr
// @pragma pa_no_overlay egress ipv4_hdr.dstAddr
// @pragma pa_no_overlay ingress ipv4_hdr.protocol
// @pragma pa_no_overlay egress ipv4_hdr.protocol*/
header ipv4_t ipv4_hdr;
/*// @pragma pa_no_overlay ingress udp_hdr.srcPort
// @pragma pa_no_overlay egress udp_hdr.srcPort
// @pragma pa_no_overlay ingress udp_hdr.dstPort
// @pragma pa_no_overlay egress udp_hdr.dstPort
// @pragma pa_no_overlay ingress udp_hdr.hdrlen
// @pragma pa_no_overlay egress udp_hdr.hdrlen*/
header udp_t udp_hdr;
header op_t op_hdr;
#ifdef RANGE_SUPPORT
header scan_t scan_hdr;
header split_t split_hdr;
#endif
header vallen_t vallen_hdr;
header val_t val1_hdr;
header val_t val2_hdr;
header val_t val3_hdr;
header val_t val4_hdr;
header val_t val5_hdr;
header val_t val6_hdr;
header val_t val7_hdr;
header val_t val8_hdr;
header val_t val9_hdr;
header val_t val10_hdr;
header val_t val11_hdr;
header val_t val12_hdr;
header val_t val13_hdr;
header val_t val14_hdr;
header val_t val15_hdr;
header val_t val16_hdr;
header shadowtype_t shadowtype_hdr;
header seq_t seq_hdr;
//// @pragma pa_no_overlay ingress inswitch_hdr.idx
//// @pragma pa_no_overlay egress inswitch_hdr.idx
header inswitch_t inswitch_hdr;
header stat_t stat_hdr;
header clone_t clone_hdr;
header frequency_t frequency_hdr;
header validvalue_t validvalue_hdr;
// @pragma pa_no_overlay ingress fraginfo_hdr.padding1
// @pragma pa_no_overlay egress fraginfo_hdr.padding1
// @pragma pa_no_overlay ingress fraginfo_hdr.padding2
// @pragma pa_no_overlay egress fraginfo_hdr.padding2
// @pragma pa_no_overlay ingress fraginfo_hdr.cur_fragidx
// @pragma pa_no_overlay egress fraginfo_hdr.cur_fragidx
// @pragma pa_no_overlay ingress fraginfo_hdr.max_fragnum
// @pragma pa_no_overlay egress fraginfo_hdr.max_fragnum
header fraginfo_t fraginfo_hdr;
//// @pragma pa_no_overlay ingress meta.udp_hdrlen
//// @pragma pa_no_overlay egress meta.udp_hdrlen
metadata metadata_t meta;

//header debug_t debug_hdr;
