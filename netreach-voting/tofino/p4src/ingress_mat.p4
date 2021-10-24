/* Ingress Processing (Normal Operation) */

// Stage 0

action nop() {}

field_list hash_fields {
	op_hdr.keylololo;
	op_hdr.keylolohi;
	op_hdr.keylohilo;
	op_hdr.keylohihi;
	op_hdr.keyhilolo;
	op_hdr.keyhilohi;
	op_hdr.keyhihilo;
	op_hdr.keyhihihi;
}

field_list_calculation hash_field_calc {
	input {
		hash_fields;
	}
	algorithm: crc32;
	output_width: 16;
}

action calculate_hash() {
	modify_field_with_hash_based_offset(meta.hashidx, 0, hash_field_calc, KV_BUCKET_COUNT);
	// NOTE: we cannot use dynamic hash
	// modify_field_with_hash_based_offset(meta.hashidx, 0, hash_field_calc, KV_BUCKET_COUNT - ipv4_hdr.totalLen);
}

table calculate_hash_tbl {
	actions {
		calculate_hash;
	}
	default_action: calculate_hash();
	size: 1;
}

action save_info() {
	modify_field(meta.tmp_sport, udp_hdr.srcPort);
	modify_field(meta.tmp_dport, udp_hdr.dstPort);
}

table save_info_tbl {
	actions {
		save_info;
	}
	default_action: save_info();
	size: 1;
}

action load_gthreshold(threshold) {
	modify_field(meta.gthreshold, threshold);
}

table load_gthreshold_tbl {
	actions {
		load_gthreshold;
	}
	size: 1;
}

action load_pthreshold(threshold) {
	modify_field(meta.pthreshold, threshold);
}

table load_pthreshold_tbl {
	actions {
		load_pthreshold;
	}
	size: 1;
}

// Stage 5

action gneg_ppos_diff() {
	subtract(meta.vote_diff, meta.gnegvote, meta.pposvote);
}

action pneg_ppos_diff() {
	subtract(meta.vote_diff, meta.pnegvote, meta.pposvote);
}

action gneg_gpos_diff() {
	subtract(meta.vote_diff, meta.gnegvote, meta.gposvote);
}

action pneg_gpos_diff() {
	subtract(meta.vote_diff, meta.pnegvote, meta.gposvote);
}

action default_diff() {
	modify_field(meta.vote_diff, 0);
}

table calculate_diff_tbl {
	reads {
		meta.isdirty: exact;
		op_hdr.optype: exact;
	}
	actions {
		gneg_ppos_diff;
		pneg_ppos_diff;
		gneg_gpos_diff;
		pneg_gpos_diff;
		default_diff;
	}
	default_action: default_diff();
	size: 4;
}

// Last Stage of ingress pipeline

action port_forward(port) {
	modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
}

action droppkt() {
	drop();
}

table port_forward_tbl {
	reads {
		ig_intr_md.ingress_port: exact;
	}
	actions {
		port_forward;
		droppkt;
		nop;
	}
	default_action: nop();
	size: 4;  
}
