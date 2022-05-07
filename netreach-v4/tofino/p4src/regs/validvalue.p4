register validvalue_reg {
	width: 8;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_validvalue_alu {
	reg: validvalue_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.validvalue;
}

action get_validvalue() {
	get_validvalue_alu.execute_stateful_alu(inswitch_hdr.idx);
}

// TODO: only used by PUTREQ/DELREQ if with PUTREQ_LARGE
blackbox stateful_alu set_and_get_validvalue_alu {
	reg: validvalue_reg;

	condition_lo: register_lo == 2;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: 1;
	update_lo_2_predicate: not condition_lo; // 0/1/3
	update_lo_2_value: register_lo;

	output_value: register_lo; // 0/1/3: keep original; 2: change to 1
	output_dst: meta.validvalue;
}

action set_and_get_validvalue() {
	set_and_get_validvalue_alu.execute_stateful_alu(inswitch_hdr.idx);
}

// TODO: only used by PUTREQ_LARGE
blackbox stateful_alu reset_and_get_validvalue_alu {
	reg: validvalue_reg;

	condition_lo: register_lo == 1;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: 2;
	update_lo_2_predicate: not condition_lo; // 0/2/3
	update_lo_2_value: register_lo;

	output_value: register_lo; // 0/2/3: keep original; 1: change to 2
	output_dst: meta.validvalue;
}

action reset_and_get_validvalue() {
	reset_and_get_validvalue_alu.execute_stateful_alu(inswitch_hdr.idx);
}

action reset_meta_validvalue() {
	modify_field(meta.validvalue, 0);
}

@pragma stage 1
table access_validvalue_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
	}
	actions {
		get_validvalue;
		set_and_get_validvalue;
		reset_and_get_validvalue;
		reset_meta_validvalue; // not touch validvalue_reg
	}
	default_action: reset_meta_validvalue();
	size: 8;
}
