register leafload_forclient_reg {
	width: 32;
	instance_count: MAX_LEAFSWITCH_NUM;
}

blackbox stateful_alu set_leafload_forclient_alu {
	reg: leafload_forclient_reg;

	condition_lo: switchload_hdr.leafload != 0;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: switchload_hdr.leafload;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: register_lo;
}

// Deprecated: for GETRES from spine switch 
// for DISTCACHE_UPDATE_TRAFFICLOAD from client
action set_leafload_forclient() {
	set_leafload_forclient_alu.execute_stateful_alu(op_hdr.leafswitchidx);
	modify_field(meta.toleaf_predicate, 1);
}

blackbox stateful_alu get_leafload_forclient_alu {
	reg: leafload_forclient_reg;

	condition_lo: meta.spineload_forclient >= register_lo;

	update_lo_1_value: register_lo;

	output_value: predicate;
	output_dst: meta.toleaf_predicate; // 1: false; 2: true
}

// for GETREQ from client
action get_leafload_forclient() {
	get_leafload_forclient_alu.execute_stateful_alu(op_hdr.leafswitchidx);
}

// for optypes except GETRES/DISTCACHE_GETRES_SPINE/GETREQ
action reset_meta_toleaf_predicate() {
	modify_field(meta.toleaf_predicate, 1);
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter access_leafload_forclient_counter {
	type : packets_and_bytes;
	direct: access_leafload_forclient_tbl;
}
#endif

@pragma stage 2
table access_leafload_forclient_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		set_leafload_forclient;
		get_leafload_forclient;
		reset_meta_toleaf_predicate;
	}
	default_action: reset_meta_toleaf_predicate();
	size: 4;
}
