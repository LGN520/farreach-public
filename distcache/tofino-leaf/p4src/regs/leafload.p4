register leafload_reg {
	width: 32;
	instance_count: MAX_LEAFSWITCH_NUM;
}

blackbox stateful_alu set_and_get_leafload_alu {
	reg: leafload_reg;

	update_lo_1_value: register_lo + 1;

	output_value: alu_lo;
	output_dst: switchload_hdr.leafload;
}

// for GETREQ_SPINE from spine switch
action set_and_get_leafload() {
	set_and_get_leafload_alu.execute_stateful_alu(op_hdr.leafswitchidx);
}

blackbox stateful_alu get_leafload_alu {
	reg: leafload_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: switchload_hdr.leafload;
}

// for GETRES_SERVER from storage server
action get_leafload() {
	get_leafload_alu.execute_stateful_alu(op_hdr.leafswitchidx);
}

@pragma stage 0
table access_leafload_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		set_and_get_leafload;
		get_leafload;
		nop;
	}
	default_action: nop();
	size: 2;
}
