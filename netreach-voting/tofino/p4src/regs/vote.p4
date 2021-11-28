register vote_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu increase_vote_alu {
	reg: vote_reg;
	
	update_lo_1_value: register_lo + 1; // not need eviction
}

action increase_vote() {
	increase_vote_alu.execute_stateful_alu(meta.hashidx);
	modify_field(meta.isevict, 1);
}

blackbox stateful_alu decrease_vote_alu {
	reg: vote_reg;

	condition_lo: register_lo == 0;
	
	update_lo_1_predicate: condition_lo;
	update_lo_1_value: 0; // need eviction
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: register_lo - 1; // not need eviction

	output_value: predicate;
	output_dst: meta.isevict; // 2 for eviction; 0 for no eviction
}

action decrease_vote() {
	decrease_vote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu reset_vote_alu {
	reg: vote_reg;

	update_lo_1_value: 0;
}

action reset_vote() {
	reset_vote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu init_vote_alu {
	reg: vote_reg;

	update_lo_1_value: 1;
}

action init_vote() {
	init_vote_alu.execute_stateful_alu(meta.hashidx);
}

table access_vote_tbl {
	reads {
		meta.isvalid: exact;
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
		increase_vote;
		decrease_vote;
		init_vote;
		reset_vote;
		nop;
	}
	default_action: nop();
	size: 2176;
}
