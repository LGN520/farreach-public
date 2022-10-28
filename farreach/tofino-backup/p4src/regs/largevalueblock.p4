register largevalueblock_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_largevalueblock_alu {
	reg: largevalueblock_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.is_largevalueblock;
}

action get_largevalueblock() {
	get_largevalueblock_alu.execute_stateful_alu(inswitch_hdr.idx);
}

blackbox stateful_alu set_and_get_largevalueblock_alu {
	reg: largevalueblock_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.is_largevalueblock;
}

action set_and_get_largevalueblock() {
	set_and_get_largevalueblock_alu.execute_stateful_alu(inswitch_hdr.idx);
}

// CACHE_POP_INSWITCH 
blackbox stateful_alu reset_and_get_largevalueblock_alu {
	reg: largevalueblock_reg;

	update_lo_1_value: clr_bit;

	output_value: alu_lo;
	output_dst: meta.is_largevalueblock;
}

action reset_and_get_largevalueblock() {
	reset_and_get_largevalueblock_alu.execute_stateful_alu(inswitch_hdr.idx);
}

action reset_is_largevalueblock() {
	modify_field(meta.is_largevalueblock, 0);
}

@pragma stage 2
table access_largevalueblock_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
		validvalue_hdr.validvalue: exact;
		fraginfo_hdr.cur_fragidx: exact;
	}	
	actions {
		get_largevalueblock;
		set_and_get_largevalueblock;
		reset_and_get_largevalueblock;
		reset_is_largevalueblock; // not touch largevalueblock_reg
	}
	default_action: reset_is_largevalueblock();
	size: 32;
}
