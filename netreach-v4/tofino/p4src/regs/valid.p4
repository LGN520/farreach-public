register valid_reg {
	width: 8;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valid_alu {
	reg: valid_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.valid;
}

action get_valid() {
	get_valid_alu.execute_stateful_alu(inswitch_hdr.idx);
}

// TODO: only used by PUTREQ/DELREQ if with PUTREQ_LARGE
blackbox stateful_alu set_and_get_valid_alu {
	reg: valid_reg;

	condition_lo: register_lo == 2;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: 1;
	update_lo_2_predicate: not condition_lo; // 0/1/3
	update_lo_2_value: register_lo;

	output_value: register_lo; // 0/1/3: keep original; 2: change to 1
	output_dst: meta.valid;
}

action set_and_get_valid() {
	set_and_get_valid_alu.execute_stateful_alu(inswitch_hdr.idx);
}

// TODO: only used by PUTREQ_LARGE
blackbox stateful_alu reset_and_get_valid_alu {
	reg: valid_reg;

	condition_lo: register_lo == 1;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: 2;
	update_lo_2_predicate: not condition_lo; // 0/2/3
	update_lo_2_value: register_lo;

	output_value: register_lo; // 0/2/3: keep original; 1: change to 2
	output_dst: meta.valid;
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
