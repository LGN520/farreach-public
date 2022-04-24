register latest_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_latest_alu {
	reg: latest_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.is_latest;
}

action get_latest() {
	get_latest_alu.execute_stateful_alu(inswitch_hdr.idx);
}

blackbox stateful_alu set_and_get_latest_alu {
	reg: latest_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.is_latest;
}

action set_and_get_latest() {
	set_and_get_latest_alu.execute_stateful_alu(inswitch_hdr.idx);
}

// CACHE_POP_INSWITCH 
blackbox stateful_alu reset_and_get_latest_alu {
	reg: latest_reg;

	update_lo_1_value: clr_bit;

	output_value: alu_lo;
	output_dst: meta.is_latest;
}

action reset_and_get_latest() {
	reset_and_get_latest_alu.execute_stateful_alu(inswitch_hdr.idx);
}

@pragma stage 2
table access_latest_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
		meta.valid: exact;
	}
	actions {
		get_latest;
		set_and_get_latest;
		reset_and_get_latest;
		nop;
	}
	default_action: nop();
	size: 0;
}
