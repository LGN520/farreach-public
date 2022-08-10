register leafload_forclient_reg {
	width: 32;
	instance_count: MAX_LEAFSWITCH_NUM;
}

blackbox stateful_alu set_and_get_leafload_forclient_alu {
	reg: leafload_forclient_reg;

	update_lo_1_value: switchload_hdr.leafload;

	output_value: alu_lo;
	output_dst: meta.leafload_forclient;
}

// for DISTCACHE_GETRES_SPINE/GETRES from spine switch 
action set_and_get_leafload_forclient() {
	set_and_get_leafload_forclient_alu.execute_stateful_alu(op_hdr.leafswitchidx);
}

blackbox stateful_alu get_leafload_forclient_alu {
	reg: leafload_forclient_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.leafload_forclient;
}

// for GETREQ from client
action get_leafload_forclient() {
	get_leafload_forclient_alu.execute_stateful_alu(op_hdr.leafswitchidx);
}

@pragma stage 0
table access_leafload_forclient_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		set_and_get_leafload_forclient;
		get_leafload_forclient;
		nop;
	}
	default_action: nop();
	size: 4;
}
