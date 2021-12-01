// Put the 8 registers in stage 1 & 2 after hash in stage 0

register keylololo_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keylololo_alu {
	reg: keylololo_reg;

	condition_lo: register_lo == op_hdr.keylololo;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keylololo;
}

action get_match_keylololo() {
	get_match_keylololo_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu put_match_keylololo_alu {
	reg: keylololo_reg;

	condition_lo: register_lo == op_hdr.keylololo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylololo;
	
	output_value: register_lo;
	output_dst: meta.origin_keylololo;
}

action put_match_keylololo() {
	put_match_keylololo_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 1
table match_keylololo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_match_keylololo;
		put_match_keylololo;
		nop;
	}
	default_action: nop();
	size: 4;
}

register keylolohi_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keylolohi_alu {
	reg: keylolohi_reg;

	condition_lo: register_lo == op_hdr.keylolohi;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keylolohi;
}

action get_match_keylolohi() {
	get_match_keylolohi_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu put_match_keylolohi_alu {
	reg: keylolohi_reg;

	condition_lo: register_lo == op_hdr.keylolohi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylolohi;
	
	output_value: register_lo;
	output_dst: meta.origin_keylolohi;
}

action put_match_keylolohi() {
	put_match_keylolohi_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 1
table match_keylolohi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_match_keylolohi;
		put_match_keylolohi;
		nop;
	}
	default_action: nop();
	size: 4;
}

register keylohilo_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keylohilo_alu {
	reg: keylohilo_reg;

	condition_lo: register_lo == op_hdr.keylohilo;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keylohilo;
}

action get_match_keylohilo() {
	get_match_keylohilo_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu put_match_keylohilo_alu {
	reg: keylohilo_reg;

	condition_lo: register_lo == op_hdr.keylohilo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylohilo;
	
	output_value: register_lo;
	output_dst: meta.origin_keylohilo;
}

action put_match_keylohilo() {
	put_match_keylohilo_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 1
table match_keylohilo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_match_keylohilo;
		put_match_keylohilo;
		nop;
	}
	default_action: nop();
	size: 4;
}

register keylohihi_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keylohihi_alu {
	reg: keylohihi_reg;

	condition_lo: register_lo == op_hdr.keylohihi;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keylohihi;
}

action get_match_keylohihi() {
	get_match_keylohihi_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu put_match_keylohihi_alu {
	reg: keylohihi_reg;

	condition_lo: register_lo == op_hdr.keylohihi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylohihi;
	
	output_value: register_lo;
	output_dst: meta.origin_keylohihi;
}

action put_match_keylohihi() {
	put_match_keylohihi_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 1
table match_keylohihi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_match_keylohihi;
		put_match_keylohihi;
		nop;
	}
	default_action: nop();
	size: 4;
}

register keyhilolo_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keyhilolo_alu {
	reg: keyhilolo_reg;

	condition_lo: register_lo == op_hdr.keyhilolo;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keyhilolo;
}

action get_match_keyhilolo() {
	get_match_keyhilolo_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu put_match_keyhilolo_alu {
	reg: keyhilolo_reg;

	condition_lo: register_lo == op_hdr.keyhilolo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhilolo;
	
	output_value: register_lo;
	output_dst: meta.origin_keyhilolo;
}

action put_match_keyhilolo() {
	put_match_keyhilolo_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 2
table match_keyhilolo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_match_keyhilolo;
		put_match_keyhilolo;
		nop;
	}
	default_action: nop();
	size: 4;
}

register keyhilohi_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keyhilohi_alu {
	reg: keyhilohi_reg;

	condition_lo: register_lo == op_hdr.keyhilohi;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keyhilohi;
}

action get_match_keyhilohi() {
	get_match_keyhilohi_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu put_match_keyhilohi_alu {
	reg: keyhilohi_reg;

	condition_lo: register_lo == op_hdr.keyhilohi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhilohi;
	
	output_value: register_lo;
	output_dst: meta.origin_keyhilohi;
}

action put_match_keyhilohi() {
	put_match_keyhilohi_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 2
table match_keyhilohi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_match_keyhilohi;
		put_match_keyhilohi;
		nop;
	}
	default_action: nop();
	size: 4;
}

register keyhihilo_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keyhihilo_alu {
	reg: keyhihilo_reg;

	condition_lo: register_lo == op_hdr.keyhihilo;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keyhihilo;
}

action get_match_keyhihilo() {
	get_match_keyhihilo_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu put_match_keyhihilo_alu {
	reg: keyhihilo_reg;

	condition_lo: register_lo == op_hdr.keyhihilo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhihilo;
	
	output_value: register_lo;
	output_dst: meta.origin_keyhihilo;
}

action put_match_keyhihilo() {
	put_match_keyhihilo_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 2
table match_keyhihilo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_match_keyhihilo;
		put_match_keyhihilo;
		nop;
	}
	default_action: nop();
	size: 4;
}

register keyhihihi_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_match_keyhihihi_alu {
	reg: keyhihihi_reg;

	condition_lo: register_lo == op_hdr.keyhihihi;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keyhihihi;
}

action get_match_keyhihihi() {
	get_match_keyhihihi_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu put_match_keyhihihi_alu {
	reg: keyhihihi_reg;

	condition_lo: register_lo == op_hdr.keyhihihi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhihihi;
	
	output_value: register_lo;
	output_dst: meta.origin_keyhihihi;
}

action put_match_keyhihihi() {
	put_match_keyhihihi_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 2
table match_keyhihihi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_match_keyhihihi;
		put_match_keyhihihi;
		nop;
	}
	default_action: nop();
	size: 4;
}
