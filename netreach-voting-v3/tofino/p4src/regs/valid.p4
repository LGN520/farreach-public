// Put it in stage 2 after key register in stage 1 (If key does not match, DEL will clear valid)

register valid_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valid_alu {
	reg: valid_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.isvalid;
}

action get_valid() {
	get_valid_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_valid_alu {
	reg: valid_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.isvalid;
}

action set_valid() {
	set_valid_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu reset_valid_alu {
	reg: valid_reg;

	update_lo_1_value: clr_bit;

	output_value: alu_lo;
	output_dst: meta.isvalid;
}

action reset_valid() {
	reset_valid_alu.execute_stateful_alu(op_hdr.hashidx);
}

@pragma stage 3
table access_valid_tbl {
	reads {
		op_hdr.optype: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
	}
	actions {
		get_valid;
		set_valid;
		reset_valid;
		nop;
	}
	default_action: nop();
	size: 1280;
}
