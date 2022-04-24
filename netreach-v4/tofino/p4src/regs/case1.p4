register case1_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu read_case1_alu {
	reg: case1_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.is_case1;
}

action read_case1() {
	read_case1_alu.execute_stateful_alu(inswitch_hdr.idx);
}

blackbox stateful_alu try_case1_alu {
	reg: case1_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.is_case1;
}

action try_case1() {
	try_case1_alu.execute_stateful_alu(inswitch_hdr.idx);
}

@pragma stage 3
table access_case1_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
		meta.valid: exact;
		meta.is_latest: exact;
		inswitch_hdr.snapshot_flag: exact;
	}
	actions {
		try_case1;
		read_case1;
		nop;
	}
	default_action: nop();
	size: 0;
}
