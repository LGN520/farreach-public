// Put the 4 registers in stage 1 after hash in stage 0

register keylolo_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keylolo_alu {
	reg: keylolo_reg;

	condition_lo: register_lo == op_hdr.keylolo;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keylolo;
}

action get_match_keylolo() {
	get_match_keylolo_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu put_match_keylolo_alu {
	reg: keylolo_reg;

	condition_lo: register_lo == op_hdr.keylolo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylolo;
	
	output_value: register_lo;
	output_dst: meta.origin_keylolo;
}

action put_match_keylolo() {
	put_match_keylolo_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 1
table match_keylolo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_match_keylolo;
		put_match_keylolo;
		nop;
	}
	default_action: nop();
	size: 4;
}

register keylohi_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keylohi_alu {
	reg: keylohi_reg;

	condition_lo: register_lo == op_hdr.keylohi;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keylohi;
}

action get_match_keylohi() {
	get_match_keylohi_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu put_match_keylohi_alu {
	reg: keylohi_reg;

	condition_lo: register_lo == op_hdr.keylohi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylohi;
	
	output_value: register_lo;
	output_dst: meta.origin_keylohi;
}

action put_match_keylohi() {
	put_match_keylohi_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 1
table match_keylohi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_match_keylohi;
		put_match_keylohi;
		nop;
	}
	default_action: nop();
	size: 4;
}

register keyhilo_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keyhilo_alu {
	reg: keyhilo_reg;

	condition_lo: register_lo == op_hdr.keyhilo;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keyhilo;
}

action get_match_keyhilo() {
	get_match_keyhilo_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu put_match_keyhilo_alu {
	reg: keyhilo_reg;

	condition_lo: register_lo == op_hdr.keyhilo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhilo;
	
	output_value: register_lo;
	output_dst: meta.origin_keyhilo;
}

action put_match_keyhilo() {
	put_match_keyhilo_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 1
table match_keyhilo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_match_keyhilo;
		put_match_keyhilo;
		nop;
	}
	default_action: nop();
	size: 4;
}

register keyhihi_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keyhihi_alu {
	reg: keyhihi_reg;

	condition_lo: register_lo == op_hdr.keyhihi;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keyhihi;
}

action get_match_keyhihi() {
	get_match_keyhihi_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu put_match_keyhihi_alu {
	reg: keyhihi_reg;

	condition_lo: register_lo == op_hdr.keyhihi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhihi;
	
	output_value: register_lo;
	output_dst: meta.origin_keyhihi;
}

action put_match_keyhihi() {
	put_match_keyhihi_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 1
table match_keyhihi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_match_keyhihi;
		put_match_keyhihi;
		nop;
	}
	default_action: nop();
	size: 4;
}