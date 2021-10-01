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

@pragma stage 2
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
		clear_valid;
		nop;
	}
	default_action: nop();
}
