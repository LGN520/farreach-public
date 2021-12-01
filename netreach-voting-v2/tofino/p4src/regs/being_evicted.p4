register being_evicted_reg {
	width: 1;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_being_evicted_alu {
	reg: being_evicted_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.being_evicted;
}

action get_being_evicted() {
	get_being_evicted_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_being_evicted_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		get_being_evicted;
		nop;
	}
	default_action: nop();
	size: 128;
}
