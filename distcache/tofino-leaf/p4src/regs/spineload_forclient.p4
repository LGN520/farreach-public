register spineload_forclient_reg {
	width: 32;
	instance_count: MAX_SPINESWITCH_NUM;
}

blackbox stateful_alu set_spineload_forclient_alu {
	reg: spineload_forclient_reg;

	update_lo_1_value: switchload_hdr.spineload;
}

// for DISTCACHE_GETRES_SPINE/GETRES from spine switch 
action set_spineload_forclient() {
	set_spineload_forclient_alu.execute_stateful_alu(op_hdr.spineswitchidx);
}

blackbox stateful_alu get_spineload_forclient_alu {
	reg: spineload_forclient_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.spineload_forclient;
}

// for GETREQ from client
action get_spineload_forclient() {
	get_spineload_forclient_alu.execute_stateful_alu(op_hdr.spineswitchidx);
}

@pragma stage 0
table access_spineload_forclient_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		set_spineload_forclient;
		get_spineload_forclient;
		nop;
	}
	default_action: nop();
	size: 4;
}
