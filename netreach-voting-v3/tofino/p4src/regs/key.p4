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

blackbox stateful_alu set_and_get_keylolo_alu {
	reg: keylolo_reg;

	update_lo_1_value: op_hdr.keylolo; 
	
	output_value: register_lo;
	output_dst: op_hdr.keylolo;
}

action set_and_get_keylolo() {
	set_and_get_keylolo_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_keylolo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keylolo;
		set_and_get_keylolo;
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

blackbox stateful_alu set_and_get_keylohi_alu {
	reg: keylohi_reg;

	update_lo_1_value: op_hdr.keylohi; 
	
	output_value: register_lo;
	output_dst: op_hdr.keylohi;
}

action set_and_get_keylohi() {
	set_and_get_keylohi_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_keylohi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keylohi;
		set_and_get_keylohi;
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

blackbox stateful_alu set_and_get_keyhilo_alu {
	reg: keyhilo_reg;

	update_lo_1_value: op_hdr.keyhilo; 
	
	output_value: register_lo;
	output_dst: op_hdr.keyhilo;
}

action set_and_get_keyhilo() {
	set_and_get_keyhilo_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_keyhilo_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keyhilo;
		set_and_get_keyhilo;
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

blackbox stateful_alu set_and_get_keyhihi_alu {
	reg: keyhihi_reg;

	update_lo_1_value: op_hdr.keyhihi; 
	
	output_value: register_lo;
	output_dst: op_hdr.keyhihi;
}

action set_and_get_keyhihi() {
	set_and_get_keyhihi_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_keyhihi_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		match_keyhihi;
		set_and_get_keyhihi;
		nop;
	}
	default_action: nop();
	size: 8;
}
