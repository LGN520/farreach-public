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

// NOTE: inswicth_t affects CACHE_POP_INSWITCH in end-hosts
header_type inswitch_t {
	fields {
		snapshot_flag: 1;
		is_cached: 1;
		is_sampled: 1;
		is_wrong_pipeline: 1;
		eport_for_res: 9;
		sid: 9;
		hashval_for_cm: 17; // at most 64K
		padding: 1;
		hashval_for_partition: 16; // at most 32K
		hashval_for_seq: 16; // at most 32K
		idx: 16; // index for in-switch cache
	}
}

header_type seq_t {
	fields {
		seq: 32;
	}
}

header_type result_t {
	fields {
		result: 8;
	}
}

/*header_type status_t {
	fields {
		valid: 8; // if the entry is valid
		is_latest: 1; // if the entry is latest
		is_deleted: 1; // if the entry is deleted
		padding: 6;
	}
}*/

header_type metadata_t {
	fields {
		need_recirculate: 1;
		cm1_predicate: 4;
		cm2_predicate: 4;
		cm3_predicate: 4;
		cm4_predicate: 4;
		is_hot: 1;
		valid: 8; // if the entry is valid
		is_latest: 1; // if the entry is latest
		is_deleted: 1; // if the entry is deleted
		is_case1: 1;
		clonenum_for_pktloss: 8;
		is_lastclone_for_pktloss: 1;

		tmp_sport: 16;
		tmp_dport: 16;
		canput: 4; // predicate for seq (update vallen and val only if with valid seq)
		zerovote: 4; // predicate (if we need to trigger cache update)
		islock: 1; // if the entry is locked before
		isbackup: 1; // backup flag
		iscase12: 1; // case 1/2 of backup
		iscase3: 1; // case3 of backup
		hashidx: 16; // for hash partition in egress pipeline
		seq_large: 32; // seq number for PUTREQ_LARGE/_RECIR/_SEQ
	}
}

// Header instances

header ethernet_t ethernet_hdr;
header ipv4_t ipv4_hdr;
header udp_t udp_hdr;
header op_t op_hdr;
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
header result_t result_hdr;
header seq_t seq_hdr;
header inswitch_t inswitch_hdr;
//header status_t status_hdr;
metadata metadata_t meta;
