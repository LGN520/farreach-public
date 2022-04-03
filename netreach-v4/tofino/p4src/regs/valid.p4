register valid_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valid_alu {
	reg: valid_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: status_hdr.is_valid;
}

action get_valid() {
	get_valid_alu.execute_stateful_alu(inswitch_hdr.idx);
}

blackbox stateful_alu set_and_get_valid_alu {
	reg: valid_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: status_hdr.is_valid;
}

action set_and_get_valid() {
	set_and_get_valid_alu.execute_stateful_alu(inswitch_hdr.idx);
}

blackbox stateful_alu reset_and_get_valid_alu {
	reg: valid_reg;

	update_lo_1_value: clr_bit;

	output_value: alu_lo;
	output_dst: status_hdr.is_valid;
}

action reset_and_get_valid() {
	reset_and_get_valid_alu.execute_stateful_alu(inswitch_hdr.idx);
}

@pragma stage 1
table access_valid_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
	}
	actions {
		get_valid;
		set_and_get_valid;
		reset_and_get_valid;
		nop;
	}
	default_action: nop();
	size: 0;
}
