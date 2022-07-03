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

// Stage 1

#ifdef RANGE_SUPPORT
action range_partition(udpport, eport) {
	modify_field(udp_hdr.dstPort, udpport);
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}
@pragma stage 1
table range_partition_tbl {
	reads {
		op_hdr.optype: exact;
		op_hdr.keyhihihi: range;
	}
	actions {
		range_partition;
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
@pragma stage 1
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

// Stage 2

#ifdef RANGE_SUPPORT
action range_partition_for_scan_endkey(last_udpport_plus_one) {
	modify_field(split_hdr.is_clone, 0);
	modify_field(split_hdr.cur_scanidx, 0);
	subtract(split_hdr.max_scannum, last_udpport_plus_one, udp_hdr.dstPort);
}

@pragma stage 2
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

// Stage 3

action forward_normal_response(eport) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, eport);
}

@pragma stage 3
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

// Stage 4

#ifdef RANGE_SUPPORT
action update_scanreq_to_scanreq_split() {
	modify_field(op_hdr.optype, SCANREQ_SPLIT);
	add_header(split_hdr);
}

@pragma stage 4
table ig_port_forward_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
#ifdef RANGE_SUPPORT
		update_scanreq_to_scanreq_split;
#endif
		nop;
	}
	default_action: nop();
	size: 8;
}
#endif
