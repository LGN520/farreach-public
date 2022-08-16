register spineload_forclient_reg {
	width: 32;
	instance_count: MAX_SPINESWITCH_NUM;
}

blackbox stateful_alu set_spineload_forclient_alu {
	reg: spineload_forclient_reg;

	condition_lo: switchload_hdr.spineload != 0;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: switchload_hdr.spineload;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: register_lo;
}

// Deprecated: for DISTCACHE_GETRES_SPINE/GETRES from spine switch 
// for DISTCACHE_UPDATE_TRAFFICLOAD from client
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

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter access_spineload_forclient_counter {
	type : packets_and_bytes;
	direct: access_spineload_forclient_tbl;
}
#endif

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
