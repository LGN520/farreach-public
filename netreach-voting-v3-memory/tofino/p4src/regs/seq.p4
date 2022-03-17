register seq_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu assign_seq_alu {
	reg: seq_reg;

	condition_lo: register_lo == 0x7FFFFFFF;

	update_lo_1_predicate: not condition_lo;
	update_lo_1_value: register_lo + 1;
	update_lo_2_predicate: condition_lo;
	update_lo_2_value: 0;

	output_value: alu_lo;
	output_dst: seq_hdr.seq;
}

action assign_seq() {
	assign_seq_alu.execute_stateful_alu(op_hdr.hashidx);
}

table assign_seq_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		assign_seq;
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
		//reset_savedseq;
		nop;
	}
	default_action: nop();
	size: 16;
}
