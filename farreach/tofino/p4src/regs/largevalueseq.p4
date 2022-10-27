register largevalueseq_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_largevalueseq_alu {
	reg: largevalueseq_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.largevalueseq;
}

action get_largevalueseq() {
	get_largevalueseq_alu.execute_stateful_alu(inswitch_hdr.idx);
}

blackbox stateful_alu set_largevalueseq_alu {
	reg: largevalueseq_reg;

	update_lo_1_value: seq_hdr.seq;
}

action set_largevalueseq() {
	set_largevalueseq_alu.execute_stateful_alu(inswitch_hdr.idx);
	modify_field(meta.largevalueseq, 0);
	modify_field(meta.is_largevalueblock, 0);
}

// CACHE_POP_INSWITCH 
blackbox stateful_alu reset_largevalueseq_alu {
	reg: largevalueseq_reg;

	update_lo_1_value: 0;
}

action reset_largevalueseq() {
	reset_largevalueseq_alu.execute_stateful_alu(inswitch_hdr.idx);
	modify_field(meta.largevalueseq, 0);
	modify_field(meta.is_largevalueblock, 0);
}

action reset_meta_largevalueseq() {
	modify_field(meta.largevalueseq, 0);
	modify_field(meta.is_largevalueblock, 0);
}

@pragma stage 2
table access_largevalueseq_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
		validvalue_hdr.validvalue: exact;
		fraginfo_hdr.cur_fragidx: exact;
	}	
	actions {
		get_largevalueseq;
		set_largevalueseq;
		reset_largevalueseq;
		reset_meta_largevalueseq; // not touch largevalueseq_reg
	}
	default_action: reset_meta_largevalueseq();
	size: 32;
}
