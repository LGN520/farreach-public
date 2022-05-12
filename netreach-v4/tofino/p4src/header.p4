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
	}
}

header_type op_t {
	fields {
		optype: 8;
		keylolo: 32;
		keylohi: 32;
		keyhilo: 32;
		keyhihi: 32;
	}
}

#ifdef RANGE_SUPPORT
header_type scan_t {
	fields {
		keylolo: 32;
		keylohi: 32;
		keyhilo: 32;
		keyhihi: 32;
	}
}
header_type split_t {
	fields {
		cur_scanidx: 16;
		max_scannum: 16;
	}
}
#endif

// Used by PUTREQ and GETRES to save PHV
header_type vallen_t {
	fields {
		vallen: 32;
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
		shadowtype: 8;
	}
}

header_type seq_t {
	fields {
		seq: 32;
	}
}

// NOTE: inswicth_t affects INSWITCH_PREV_BYTES in packet_format.h
header_type inswitch_t {
	fields {
		// 8-bit container
		snapshot_flag: 1;
		is_cached: 1;
		is_sampled: 1;
		//is_wrong_pipeline: 1;
		//eport_for_res: 9;
		padding: 5;
		// two 16-bit containers
		sid: 10; // clone_e2e sets eg_intr_md_for_mb.mirror_id w/ 10 bits
		hashval_for_cm: 22; // at most 64K
		// 16-bit container
		hashval_for_seq: 16; // at most 32K
		// 16-bit container
		idx: 16; // index for in-switch cache
	}
}

header_type stat_t {
	fields {
		stat: 8;
	}
}

header_type metadata_t {
	fields {
		need_recirculate: 1;
#ifdef RANGE_SUPPORT
		remain_scannum: 16;
		is_last_scansplit: 1;
#else
		hashval_for_partition: 16; // at most 32K
#endif
		cm1_predicate: 4;
		cm2_predicate: 4;
		cm3_predicate: 4;
		cm4_predicate: 4;
		validvalue: 8; // validvalue of the entry
		is_latest: 1; // if the entry is latest
		is_deleted: 1; // if the entry is deleted
		is_case1: 1;
		clonenum_for_pktloss: 8;
	}
}

header_type debug_t {
	fields {
		// 8-bit container
		is_hot: 1;
		is_lastclone_for_pktloss: 1;
		padding: 6;
	}
}

// Header instances

header ethernet_t ethernet_hdr;
header ipv4_t ipv4_hdr;
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
header inswitch_t inswitch_hdr;
header stat_t stat_hdr;
metadata metadata_t meta;
header debug_t debug_hdr;
