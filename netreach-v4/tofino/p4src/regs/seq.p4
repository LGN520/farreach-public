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
	size: 0;
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
		op_hdr_optype: exact;
		inswitch_hdr.is_cached: exact;
		meta.valid: exact;
		meta.is_latest: exact;
	}
	actions {
		set_and_get_savedseq;
		nop;
	}
	default_action: nop();
	size: 0;
}












blackbox stateful_alu assign_seq_large_alu {
	reg: seq_reg;

	condition_lo: register_lo == 0x7FFFFFFF;

	update_lo_1_predicate: not condition_lo;
	update_lo_1_value: register_lo + 1;
	update_lo_2_predicate: condition_lo;
	update_lo_2_value: 0;

	output_value: alu_lo;
	output_dst: meta.seq_large;
}

action assign_seq_large() {
	assign_seq_large_alu.execute_stateful_alu(op_hdr.hashidx);
}

action copy_seq_large() {
	modify_field(meta.seq_large, seq_hdr.seq);
}

table assign_seq_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		assign_seq;
		assign_seq_large;
		copy_seq_large;
		nop;
	}
	default_action: nop();
	size: 4;
}

register savedseq_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

// key matches
blackbox stateful_alu try_update_savedseq_alu {
	reg: savedseq_reg;

	// NOTE: Tofino treats 32-bit field as signed integer, so seq is from [-0x80000000, 7fffffff]
	// TODO: now we do not consider seq overflow. If it occurs, we can use more register array to fix.
	condition_lo: seq_hdr.seq > register_lo;

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: seq_hdr.seq;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: register_lo;

	output_value: predicate;
	output_dst: meta.canput;
}

action try_update_savedseq() {
	try_update_savedseq_alu.execute_stateful_alu(op_hdr.hashidx);
}

// key not match
blackbox stateful_alu set_and_get_savedseq_alu {
	reg: savedseq_reg;

	update_lo_1_value: seq_hdr.seq;
	
	output_value: register_lo;
	output_dst: seq_hdr.seq;
}

action set_and_get_savedseq() {
	set_and_get_savedseq_alu.execute_stateful_alu(op_hdr.hashidx);
}

// key matches (for PUTREQ_LARGE/RECIR)
blackbox stateful_alu get_savedseq_alu {
	reg: savedseq_reg;

	output_value: register_lo;
	output_dst: seq_hdr.seq;
}

action get_savedseq() {
	get_savedseq.execute_stateful_alu(op_hdr.hashidx);
}

/*blackbox stateful_alu reset_savedseq_alu {
	reg: savedseq_reg;

	update_lo_1_value: 0;
}

action reset_savedseq() {
	reset_savedseq_alu.execute_stateful_alu(op_hdr.hashidx);
}*/

table access_savedseq_tbl {
	reads {
		op_hdr.optype: exact;
		other_hdr.isvalid: exact;
		meta.iskeymatch: exact;
	}
	actions {
		try_update_savedseq;
		set_and_get_savedseq;
		get_savedseq;
		//reset_savedseq;
		nop;
	}
	default_action: nop();
	size: 16;
}
