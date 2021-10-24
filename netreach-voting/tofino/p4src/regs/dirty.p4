register dirty_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_dirty_alu {
	reg: dirty_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.isdirty;
}

action get_dirty() {
	get_dirty_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu set_dirty_alu {
	reg: dirty_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.isdirty;
}

action set_dirty() {
	set_dirty_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu clear_dirty_alu {
	reg: dirty_reg;

	update_lo_1_value: clr_bit;

	output_value: alu_lo;
	output_dst: meta.isdirty;
}

action clear_dirty() {
	clear_dirty_alu.execute_stateful_alu(meta.hashidx);
}
table access_dirty_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_dirty;
		set_dirty;
		clear_dirty;
	}
	default_action: get_dirty();
	size: 8;
}
