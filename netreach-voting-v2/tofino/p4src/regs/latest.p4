// NOTE: at most 4 interfaces can be added to register array
register latest_reg {
	width: 8;
	instance_count: KV_BUCKET_COUNT;
}

// For GET
blackbox stateful_alu get_latest_alu {
	reg: latest_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: latest_hdr.latest;
}

action get_latest() {
	get_latest_alu.execute_stateful_alu(op_hdr.hashidx);
}

// For PUTREQ/DELREQ
blackbox stateful_alu set_or_clear_latest_alu {
	reg: latest_reg;

	condition_lo: op_hdr.optype == PUTREQ_TYPE;

	update_lo_1_predicate: condition_lo; // PUTREQ
	update_lo_1_value: 1;
	update_lo_2_predicate: not condition_lo; // DELREQ
	update_lo_2_value: 2;

	output_value: register_lo;
	output_dst: latest_hdr.latest;
}

action set_or_clear_latest() {
	set_or_clear_latest_alu.execute_stateful_alu(op_hdr.hashidx);
}

// For GETRES_LATEST
blackbox stateful_alu try_set_latest_alu {
	reg: latest_reg;

	condition_lo: register_lo == 0;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: 1;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: register_lo;

	output_value: register_lo;
	output_dst: latest_hdr.latest;
}

action try_set_latest() {
	try_set_latest_alu.execute_stateful_alu(op_hdr.hashidx);
}

// For GETRES_NEXIST
blackbox stateful_alu try_clear_latest_alu {
	reg: latest_reg;

	condition_lo: register_lo == 0;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: 2;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: register_lo;

	output_value: register_lo;
	output_dst: latest_hdr.latest;
}

action try_clear_latest() {
	try_clear_latest_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_latest_tbl {
	reads {
		op_hdr.optype: exact;
		meta.iscached: exact;
		//meta.isvalid: exact;
		meta.being_evicted: exact;
	}
	actions {
		get_latest;
		set_or_clear_latest;
		try_set_latest;
		try_clear_latest;
		nop;
	}
	default_action: nop();
	size: 128;
}
