register lock_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu read_lock_alu {
	reg: lock_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.islock;
}

action read_lock() {
	read_lock_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu try_lock_alu {
	reg: lock_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.islock;
}

action try_lock() {
	try_lock_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu clear_lock_alu {
	reg: lock_reg;

	update_lo_1_value: clr_bit;

	output_value: alu_lo;
	output_dst: meta.islock;
}

action clear_lock() {
	clear_lock_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_lock_tbl {
	reads {
		op_hdr.optype: exact;
		meta.isvalid: exact;
		meta.iszerovote: exact;
	}
	actions {
		try_lock;
		clear_lock;
		read_lock;
		nop;
	}
	default_action: nop();
	size: 16;
}
