register ratelimitload_reg {
	width: 32;
	instance_count: 128;
}

blackbox stateful_alu increase_ratelimitload_alu {
	reg: ratelimitload_reg;

	condition_lo: register_lo > meta.ratelimitload_threshold;
	
	update_lo_1_value: register_lo + 1;

	output_value: predicate;
	output_dst: meta.ratelimitload_predicate; // false: 1; true: 2
}

action increase_ratelimitload() {
	increase_ratelimitload_alu.execute_stateful_alu(op_hdr.globalswitchidx);
}

action reset_ratelimitload_predicate() {
	modify_field(meta.ratelimitload_predicate, 1);
}

@pragma stage 2
table access_ratelimitload_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		increase_ratelimitload;
		reset_ratelimitload_predicate;
	}
	default_action: reset_ratelimitload_predicate();
	size: 8;
}
