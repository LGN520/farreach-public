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

blackbox stateful_alu set_validvalue_alu {
	reg: validvalue_reg;

	update_lo_1_value: validvalue_hdr.validvalue;
}

action set_validvalue() {
	set_validvalue_alu.execute_stateful_alu(inswitch_hdr.idx);
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
		set_validvalue;
		reset_meta_validvalue; // not touch validvalue_reg
	}
	default_action: reset_meta_validvalue();
	size: 8;
}
