register case3_reg {
	width: 1;
	instance_count: 1;
}

blackbox stateful_alu read_case3_alu {
	reg: case3_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: other_hdr.iscase3;
}

action read_case3() {
	read_case3_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu try_case3_alu {
	reg: case3_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: other_hdr.iscase3;
}

action try_case3() {
	try_case3_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_case3_tbl {
	reads {
		op_hdr.optype: exact;
		meta.isvalid: exact;
		meta.zerovote: exact;
		meta.iskeymatch: exact;
		meta.islock: exact;
		meta.isbackup: exact;
	}
	actions {
		read_case3;
		try_case3;
		nop;
	}
	default_action: nop();
	size: 128;
}
