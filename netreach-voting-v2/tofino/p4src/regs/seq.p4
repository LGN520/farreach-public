register seq_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu increase_seq_alu {
	reg: seq_reg;

	update_lo_1_value: register_lo + 1;
}

action increase_seq() {
	increase_seq_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu read_seq_alu {
	reg: seq_reg;

	output_value: register_lo;
	output_dst: seq_hdr.seq;
}

action read_seq() {
	read_seq_alu.execute_stateful_alu(op_hdr.hashidx);
}

table access_seq_tbl {
	reads {
		op_hdr.optype: exact;
		meta.iscached: exact;
		//meta.isvalid: exact;
		meta.being_evicted: exact;
	}
	actions {
		increase_seq;
		read_seq;
		nop;
	}
	default_action: nop();
	size: 128;
}
