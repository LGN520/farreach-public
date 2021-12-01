register latest_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_latest_alu {
	reg: latest_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.islatest;
}

action get_latest() {
	get_latest_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_latest_alu {
	reg: latest_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.islatest;
}

action set_latest() {
	set_latest_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu clear_latest_alu {
	reg: latest_reg;

	update_lo_1_value: clr_bit;

	output_value: alu_lo;
	output_dst: meta.islatest;
}

action clear_latest() {
	clear_latest_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_latest_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_latest;
		set_latest;
		clear_latest;
		nop;
	}
	default_action: nop();
	size: 128;
}
