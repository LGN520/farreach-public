register deleted_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_deleted_alu {
	reg: deleted_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.is_deleted;
}

action get_deleted() {
	get_deleted_alu.execute_stateful_alu(inswitch_hdr.idx);
}

blackbox stateful_alu set_and_get_deleted_alu {
	reg: deleted_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.is_deleted;
}

action set_and_get_deleted() {
	set_and_get_deleted_alu.execute_stateful_alu(inswitch_hdr.idx);
}

blackbox stateful_alu reset_and_get_deleted_alu {
	reg: deleted_reg;

	update_lo_1_value: clr_bit;

	output_value: alu_lo;
	output_dst: meta.is_deleted;
}

action reset_and_get_deleted() {
	reset_and_get_deleted_alu.execute_stateful_alu(inswitch_hdr.idx);
}

action reset_is_deleted() {
	modify_field(meta.is_deleted, 0);
}

@pragma stage 3
table access_deleted_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
		validvalue_hdr.validvalue: exact;
		meta.is_latest: exact;
		stat_hdr.stat: exact;
	}
	actions {
		get_deleted;
		set_and_get_deleted;
		reset_and_get_deleted;
		reset_is_deleted;
	}
	default_action: reset_is_deleted();
	size: 256;
}
