register savedseq_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_savedseq_alu {
	reg: savedseq_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: seq_hdr.seq;
}

action get_savedseq() {
	get_savedseq_alu.execute_stateful_alu(inswitch_hdr.idx);
}

blackbox stateful_alu set_and_get_savedseq_alu {
	reg: savedseq_reg;

	update_lo_1_value: seq_hdr.seq;

	output_value: register_lo;
	output_dst: seq_hdr.seq;
}

action set_and_get_savedseq() {
	set_and_get_savedseq_alu.execute_stateful_alu(inswitch_hdr.idx);
}

@pragma stage 2
table access_savedseq_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
		meta.is_latest: exact;
	}
	actions {
		get_savedseq;
		set_and_get_savedseq;
		nop;
	}
	default_action: nop();
	size: 8;
}
