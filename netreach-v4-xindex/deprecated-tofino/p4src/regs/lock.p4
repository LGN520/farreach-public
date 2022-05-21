register lock_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu try_glock_alu {
	reg: lock_reg;

	condition_lo: meta.vote_diff >= meta.gthreshold;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: set_bit;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: register_lo; // read_bit

	output_value: register_lo; // alu_lo
	output_dst: meta.islock;
}

action try_glock() {
	try_glock_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu try_plock_alu {
	reg: lock_reg;

	condition_lo: meta.vote_diff >= meta.pthreshold;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: set_bit;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: register_lo; // read_bit

	output_value: register_lo; // alu_lo
	output_dst: meta.islock;
}

action try_plock() {
	try_plock_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu clear_lock_alu {
	reg: lock_reg;

	update_lo_1_value: clr_bit;

	output_value: alu_lo;
	output_dst: meta.islock;
}

action clear_lock() {
	clear_lock_alu.execute_stateful_alu(meta.hashidx);
}

table access_lock_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		try_glock;
		try_plock;
		clear_lock;
		nop;
	}
	default_action: nop();
	size: 2048;
}
