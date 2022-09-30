register case1_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

/*blackbox stateful_alu read_case1_alu {
	reg: case1_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.iscase1;
}

action read_case1() {
	read_case1_alu.execute_stateful_alu(op_hdr.hashidx);
}*/

blackbox stateful_alu try_case1_alu {
	reg: case1_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.iscase1;
}

action try_case1() {
	try_case1_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_case1_tbl {
	reads {
		op_hdr.optype: exact;
		meta.iscached: exact;
		meta.being_evicted: exact;
		meta.isbackup: exact;
	}
	actions {
		try_case1;
		nop;
	}
	default_action: nop();
	size: 4;
}
