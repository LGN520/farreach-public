/* Ingress Processing (Normal Operation) */

field_list hash_fields {
	op_hdr.keylolo;
	op_hdr.keylohi;
	op_hdr.keyhilo;
	//op_hdr.keyhihi;
	op_hdr.keyhihilo;
	op_hdr.keyhihihi;
}

field_list_calculation hash_calc {
	input {
		hash_fields;
	}
	algorithm: crc32;
	//output_width: 16;
	output_width: 32;
}

action nop() {}

// Stage 0

action l2l3_forward(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}

@pragma stage 0
table l2l3_forward_tbl {
	reads {
		ethernet_hdr.dstAddr: exact;
		ipv4_hdr.dstAddr: lpm;
	}
	actions {
		l2l3_forward;
		nop;
	}
	default_action: nop();
	size: 16;
}

#ifndef RANGE_SUPPORT
action hash_for_partition() {
	modify_field_with_hash_based_offset(meta.hashval_for_partition, 0, hash_calc, PARTITION_COUNT);
}
@pragma stage 0
table hash_for_partition_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		hash_for_partition;
		nop;
	}
	default_action: nop();
	size: 8;
}
#endif

action hash_for_spineselect() {
	modify_field_with_hash_based_offset(meta.hashval_for_spineselect, 0, hash_calc, PARTITION_COUNT);
}

@pragma stage 0
table hash_for_spineselect_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		hash_for_spineselect;
		nop;
	}
	default_action: nop();
	size: 8;
}

// Stage 1

action spineselect(eport, globalswitchidx) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	modify_field(op_hdr.globalswitchidx, globalswitchidx);
}

@pragma stage 1
table spineselect_tbl {
	reads {
		op_hdr.optype: exact;
		meta.hashval_for_spineselect: range;
	}
	actions {
		spineselect;
		nop;
	}
	default_action: nop();
	size: SPINESELECT_ENTRY_NUM;
}

// Stage 2

#ifdef RANGE_SUPPORT
action range_partition(udpport, eport) {
	modify_field(udp_hdr.dstPort, udpport);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
action range_partition_for_scan(udpport, eport, start_globalserveridx) {
	modify_field(udp_hdr.dstPort, udpport);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
	modify_field(split_hdr.globalserveridx, start_globalserveridx);
}
@pragma stage 2
table range_partition_tbl {
	reads {
		op_hdr.optype: exact;
		op_hdr.keyhihihi: range;
	}
	actions {
		range_partition;
		range_partition_for_scan;
		nop;
	}
	default_action: nop();
	size: RANGE_PARTITION_ENTRY_NUM;
}
#else
action hash_partition(udpport, eport) {
	modify_field(udp_hdr.dstPort, udpport);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
@pragma stage 2
table hash_partition_tbl {
	reads {
		op_hdr.optype: exact;
		meta.hashval_for_partition: range;
	}
	actions {
		hash_partition;
		nop;
	}
	default_action: nop();
	size: HASH_PARTITION_ENTRY_NUM;
}
#endif

// Stage 3

#ifdef RANGE_SUPPORT
//action range_partition_for_scan_endkey(last_udpport_plus_one) {
action range_partition_for_scan_endkey(end_globalserveridx_plus_one) {
	modify_field(split_hdr.is_clone, 0);
	modify_field(split_hdr.cur_scanidx, 0);
	//subtract(split_hdr.max_scannum, last_udpport_plus_one, udp_hdr.dstPort);
	subtract(split_hdr.max_scannum, end_globalserveridx_plus_one, split_hdr.globalserveridx);
}

@pragma stage 3
table range_partition_for_scan_endkey_tbl {
	reads {
		op_hdr.optype: exact;
		scan_hdr.keyhihihi: range;
	}
	actions {
		range_partition_for_scan_endkey;
		nop;
	}
	default_action: nop();
	size: RANGE_PARTITION_FOR_SCAN_ENDKEY_ENTRY_NUM;
}
#endif

// Stage 4

action forward_normal_response(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}

@pragma stage 4
table ipv4_forward_tbl {
	reads {
		op_hdr.optype: exact;
		ipv4_hdr.dstAddr: lpm;
	}
	actions {
		forward_normal_response;
		nop;
	}
	default_action: nop();
	size: 64;
}

// Stage 5

action update_getreq_spine_to_getreq() {
	modify_field(op_hdr.optype, GETREQ);
}

#ifdef RANGE_SUPPORT
action update_scanreq_to_scanreq_split() {
	modify_field(op_hdr.optype, SCANREQ_SPLIT);
	add_header(split_hdr);
}
#endif

action update_getres_server_to_getres() {
	modify_field(op_hdr.optype, GETRES);
	modify_field(shadowtype_hdr.shadowtype, GETRES);
}

@pragma stage 5
table ig_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		update_getreq_spine_to_getreq;
#ifdef RANGE_SUPPORT
		update_scanreq_to_scanreq_split;
#endif
		update_getres_server_to_getres;
		nop;
	}
	default_action: nop();
	size: 8;
}
