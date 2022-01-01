register case12_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu read_case12_alu {
	reg: case12_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.iscase12;
}

action read_case12() {
	read_case12_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu try_case12_alu {
	reg: case12_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.iscase12;
}

action try_case12() {
	try_case12_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_case12_tbl {
	reads {
		op_hdr.optype: exact;
		meta.isvalid: exact;
		meta.iskeymatch: exact;
		meta.canput: exact;
		meta.isbackup: exact;
	}
	actions {
		try_case12;
		read_case12;
		nop;
	}
	default_action: nop();
	size: 2048;
}
