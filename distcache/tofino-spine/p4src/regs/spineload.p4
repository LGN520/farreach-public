register spineload_reg {
	width: 32;
	instance_count: MAX_SPINESWITCH_NUM;
}

blackbox stateful_alu set_and_get_spineload_alu {
	reg: spineload_reg;

	update_lo_1_value: register_lo + 1;

	output_value: alu_lo;
	output_dst: switchload_hdr.spineload;
}

// for GETREQ from client-leaf
// NOTE: set AND GET for cache hit
action set_and_get_spineload() {
	set_and_get_spineload_alu.execute_stateful_alu(op_hdr.spineswitchidx);
	modify_field(switchload_hdr.leafload, 0);
}

blackbox stateful_alu get_spineload_alu {
	reg: spineload_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: switchload_hdr.spineload;
}

// Deprecated: for GETRES from server-leaf
// NOT used now
action get_spineload() {
	get_spineload_alu.execute_stateful_alu(op_hdr.spineswitchidx);
}

#ifdef DEBUG
// Only used for debugging (comment 1 stateful ALU in the same stage of egress pipeline if necessary)
counter access_spineload_counter {
	type : packets_and_bytes;
	direct: access_spineload_tbl;
}
#endif

@pragma stage 0
table access_spineload_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		set_and_get_spineload;
		get_spineload;
		nop;
	}
	default_action: nop();
	size: 2;
}
