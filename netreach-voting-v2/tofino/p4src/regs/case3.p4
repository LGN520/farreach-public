register case3_reg {
	width: 1;
	instance_count: 1;
}

/*blackbox stateful_alu read_case3_alu {
	reg: case3_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.iscase3;
}

action read_case3() {
	read_case3_alu.execute_stateful_alu(0);
}*/

blackbox stateful_alu try_case3_alu {
	reg: case3_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.iscase3;
}

action try_case3() {
	try_case3_alu.execute_stateful_alu(0);
}

table access_case3_tbl {
	reads {
		op_hdr.optype: exact;
		meta.iscached: exact;
		meta.being_evicted: exact;
		meta.isbackup: exact;
	}
	actions {
		try_case3;
		nop;
	}
	default_action: nop();
	size: 4;
}
