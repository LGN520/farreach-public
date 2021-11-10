register case2_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu read_case2_alu {
	reg: case2_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.iscase2;
}

action read_case2() {
	read_case2_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu try_case2_alu {
	reg: case2_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.iscase2;
}

action try_case2() {
	try_case2_alu.execute_stateful_alu(meta.hashidx);
}

table access_case2_tbl {
	reads {
		op_hdr.optype: exact;
		meta.isbackup: exact;
	}
	actions {
		try_case2;
		read_case2;
		nop;
	}
	default_action: nop();
	size: 4;
}
