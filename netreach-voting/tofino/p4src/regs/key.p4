// Put the 8 registers in stage 1 & 2 after hash in stage 0

register keylololo_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu match_keylololo_alu {
	reg: keylololo_reg;

	condition_lo: register_lo == op_hdr.keylololo;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keylololo;
}

action match_keylololo() {
	match_keylololo_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu modify_keylololo_alu {
	reg: keylololo_reg;

	condition_lo: register_lo == op_hdr.keylololo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylololo;
	
	output_value: register_lo;
	output_dst: meta.origin_keylololo;
}

action modify_keylololo() {
	modify_keylololo_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 1
table access_keylololo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keylololo;
		modify_keylololo;
	}
	default_action: match_keylololo();
	size: 4;
}

register keylolohi_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu match_keylolohi_alu {
	reg: keylolohi_reg;

	condition_lo: register_lo == op_hdr.keylolohi;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keylolohi;
}

action match_keylolohi() {
	match_keylolohi_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu modify_keylolohi_alu {
	reg: keylolohi_reg;

	condition_lo: register_lo == op_hdr.keylolohi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylolohi;
	
	output_value: register_lo;
	output_dst: meta.origin_keylolohi;
}

action modify_keylolohi() {
	modify_keylolohi_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 1
table access_keylolohi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keylolohi;
		modify_keylolohi;
	}
	default_action: match_keylolohi();
	size: 4;
}

register keylohilo_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu match_keylohilo_alu {
	reg: keylohilo_reg;

	condition_lo: register_lo == op_hdr.keylohilo;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keylohilo;
}

action match_keylohilo() {
	match_keylohilo_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu modify_keylohilo_alu {
	reg: keylohilo_reg;

	condition_lo: register_lo == op_hdr.keylohilo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylohilo;
	
	output_value: register_lo;
	output_dst: meta.origin_keylohilo;
}

action modify_keylohilo() {
	modify_keylohilo_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 1
table access_keylohilo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keylohilo;
		modify_keylohilo;
	}
	default_action: match_keylohilo();
	size: 4;
}

register keylohihi_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu match_keylohihi_alu {
	reg: keylohihi_reg;

	condition_lo: register_lo == op_hdr.keylohihi;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keylohihi;
}

action match_keylohihi() {
	match_keylohihi_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu modify_keylohihi_alu {
	reg: keylohihi_reg;

	condition_lo: register_lo == op_hdr.keylohihi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylohihi;
	
	output_value: register_lo;
	output_dst: meta.origin_keylohihi;
}

action modify_keylohihi() {
	modify_keylohihi_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 1
table access_keylohihi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keylohihi;
		modify_keylohihi;
	}
	default_action: match_keylohihi();
	size: 4;
}

register keyhilolo_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu match_keyhilolo_alu {
	reg: keyhilolo_reg;

	condition_lo: register_lo == op_hdr.keyhilolo;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keyhilolo;
}

action match_keyhilolo() {
	match_keyhilolo_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu modify_keyhilolo_alu {
	reg: keyhilolo_reg;

	condition_lo: register_lo == op_hdr.keyhilolo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhilolo;
	
	output_value: register_lo;
	output_dst: meta.origin_keyhilolo;
}

action modify_keyhilolo() {
	modify_keyhilolo_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 2
table access_keyhilolo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keyhilolo;
		modify_keyhilolo;
	}
	default_action: match_keyhilolo();
	size: 4;
}

register keyhilohi_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu match_keyhilohi_alu {
	reg: keyhilohi_reg;

	condition_lo: register_lo == op_hdr.keyhilohi;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keyhilohi;
}

action match_keyhilohi() {
	match_keyhilohi_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu modify_keyhilohi_alu {
	reg: keyhilohi_reg;

	condition_lo: register_lo == op_hdr.keyhilohi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhilohi;
	
	output_value: register_lo;
	output_dst: meta.origin_keyhilohi;
}

action modify_keyhilohi() {
	modify_keyhilohi_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 2
table access_keyhilohi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keyhilohi;
		modify_keyhilohi;
	}
	default_action: match_keyhilohi();
	size: 4;
}

register keyhihilo_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu match_keyhihilo_alu {
	reg: keyhihilo_reg;

	condition_lo: register_lo == op_hdr.keyhihilo;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keyhihilo;
}

action match_keyhihilo() {
	match_keyhihilo_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu modify_keyhihilo_alu {
	reg: keyhihilo_reg;

	condition_lo: register_lo == op_hdr.keyhihilo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhihilo;
	
	output_value: register_lo;
	output_dst: meta.origin_keyhihilo;
}

action modify_keyhihilo() {
	modify_keyhihilo_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 2
table access_keyhihilo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keyhihilo;
		modify_keyhihilo;
	}
	default_action: match_keyhihilo();
	size: 4;
}

register keyhihihi_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu match_keyhihihi_alu {
	reg: keyhihihi_reg;

	condition_lo: register_lo == op_hdr.keyhihihi;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keyhihihi;
}

action match_keyhihihi() {
	match_keyhihihi_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu modify_keyhihihi_alu {
	reg: keyhihihi_reg;

	condition_lo: register_lo == op_hdr.keyhihihi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhihihi;
	
	output_value: register_lo;
	output_dst: meta.origin_keyhihihi;
}

action modify_keyhihihi() {
	modify_keyhihihi_alu.execute_stateful_alu(meta.hashidx);
}

@pragma stage 2
table access_keyhihihi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keyhihihi;
		modify_keyhihihi;
	}
	default_action: match_keyhihihi();
	size: 4;
}
