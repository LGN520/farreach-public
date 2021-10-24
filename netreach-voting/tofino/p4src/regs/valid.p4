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
	get_valid_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu set_valid_alu {
	reg: valid_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.isvalid;
}

action set_valid() {
	set_valid_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu clear_valid_alu {
	reg: valid_reg;

	update_lo_1_value: clr_bit;

	output_value: alu_lo;
	output_dst: meta.isvalid;
}

action clear_valid() {
	clear_valid_alu.execute_stateful_alu(meta.hashidx);
}

/*@pragma stage 2
table clear_valid_tbl {
	actions {
		clear_valid;
	}
	default_action: clear_valid();
}*/

table access_valid_tbl {
	reads {
		op_hdr.optype: exact;
		meta.ismatch_keylololo: exact;
		meta.ismatch_keylolohi: exact;
		meta.ismatch_keylohilo: exact;
		meta.ismatch_keylohihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valid;
		set_valid;
		clear_valid;
	}
	default_action: get_valid();
	size: 2048;
}
