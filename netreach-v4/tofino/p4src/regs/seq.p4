register seq_reg {
	width: 32;
	instance_count: SEQ_BUCKET_COUNT;
}

blackbox stateful_alu assign_seq_alu {
	reg: seq_reg;

	// TODO: we can use 0xFFFFFFFF or remove condition_lo if data plane does not use seq -> we can resort to servers to solve seq overflow issue
	condition_lo: register_lo == 0x7FFFFFFF;

	update_lo_1_predicate: not condition_lo;
	update_lo_1_value: register_lo + 1;
	update_lo_2_predicate: condition_lo;
	update_lo_2_value: 0;

	output_value: alu_lo;
	output_dst: seq_hdr.seq;
}

action assign_seq() {
	assign_seq_alu.execute_stateful_alu(inswitch_hdr.hashval_for_seq);
}

@pragma stage 1
table access_seq_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		assign_seq;
		nop;
	}
	default_action: nop();
	//size: 2;
	size: 32; // use seq_req for debugging (check whether some packet type enters the egress pipeline)
}

register savedseq_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
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

@pragma stage 3
table access_savedseq_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_cached: exact;
		meta.validvalue: exact;
		meta.is_latest: exact;
	}
	actions {
		set_and_get_savedseq;
		nop;
	}
	default_action: nop();
	size: 32;
}
