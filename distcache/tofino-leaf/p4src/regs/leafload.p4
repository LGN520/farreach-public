register leafload_reg {
	width: 32;
	instance_count: MAX_LEAFSWITCH_NUM;
}

blackbox stateful_alu set_and_get_leafload_alu {
	reg: leafload_reg;

	update_lo_1_value: register_lo + 1;

	output_value: alu_lo;
	output_dst: switchload_hdr.leafload;
}

// for GETREQ_SPINE from spine switch
// NOTE: set AND GET for cache hit
action set_and_get_leafload_and_hash_for_bf2() {
	set_and_get_leafload_alu.execute_stateful_alu(op_hdr.leafswitchidx);
	// NOTE: sum of hash results' bits CANNOT > 32-bits in one ALU due to Tofino limitation (18-bit hashval_for_bf = 32-bit cost)
	// NOTE: cannot pass action parameter into modify_field_with_hash_based_offset, which only accepts constant
	modify_field_with_hash_based_offset(inswitch_hdr.hashval_for_bf2, 0, hash_calc2, BF_BUCKET_COUNT);
}

blackbox stateful_alu get_leafload_alu {
	reg: leafload_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: switchload_hdr.leafload;
}

// for GETRES_SERVER from storage server
action get_leafload() {
	get_leafload_alu.execute_stateful_alu(op_hdr.leafswitchidx);
}

@pragma stage 0
table access_leafload_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		set_and_get_leafload_and_hash_for_bf2;
		get_leafload;
		nop;
	}
	default_action: nop();
	size: 2;
}
