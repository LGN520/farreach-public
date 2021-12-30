register keylolo_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu match_keylolo_alu {
	reg: keylololo_reg;

	condition_lo: register_lo == op_hdr.keylolo;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keylolo;
}

action match_keylolo() {
	match_keylolo_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu modify_keylolo_alu {
	reg: keylolo_reg;

	condition_lo: register_lo == op_hdr.keylolo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylolo;
	
	output_value: register_lo;
	output_dst: op_hdr.keylolo;
}

action modify_keylolo() {
	modify_keylolo_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_keylolo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keylolo;
		modify_keylolo;
		nop;
	}
	default_action: nop();
	size: 8;
}

register keylohi_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu match_keylohi_alu {
	reg: keylohilo_reg;

	condition_lo: register_lo == op_hdr.keylohi;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keylohi;
}

action match_keylohi() {
	match_keylohi_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu modify_keylohi_alu {
	reg: keylohi_reg;

	condition_lo: register_lo == op_hdr.keylohi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keylohi;
	
	output_value: register_lo;
	output_dst: op_hdr.keylohi;
}

action modify_keylohi() {
	modify_keylohi_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_keylohi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keylohi;
		modify_keylohi;
		nop;
	}
	default_action: nop();
	size: 8;
}

register keyhilo_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu match_keyhilo_alu {
	reg: keyhilolo_reg;

	condition_lo: register_lo == op_hdr.keyhilo;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keyhilo;
}

action match_keyhilo() {
	match_keyhilo_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu modify_keyhilo_alu {
	reg: keyhilo_reg;

	condition_lo: register_lo == op_hdr.keyhilo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhilo;
	
	output_value: register_lo;
	output_dst: op_hdr.keyhilo;
}

action modify_keyhilo() {
	modify_keyhilo_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_keyhilo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keyhilo;
		modify_keyhilo;
		nop;
	}
	default_action: nop();
	size: 8;
}

register keyhihi_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu match_keyhihi_alu {
	reg: keyhihilo_reg;

	condition_lo: register_lo == op_hdr.keyhihi;

	update_lo_1_value: register_lo;
	
	output_value: predicate;
	output_dst: meta.ismatch_keyhihi;
}

action match_keyhihi() {
	match_keyhihi_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu modify_keyhihi_alu {
	reg: keyhihi_reg;

	condition_lo: register_lo == op_hdr.keyhihi;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: register_lo;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: op_hdr.keyhihi;
	
	output_value: register_lo;
	output_dst: op_hdr.keyhihi;
}

action modify_keyhihi() {
	modify_keyhihi_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_keyhihi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keyhihi;
		modify_keyhihi;
		nop;
	}
	default_action: nop();
	size: 8;
}
