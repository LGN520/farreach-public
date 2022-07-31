register seq_reg {
	width: 32;
	instance_count: 1;
}

blackbox stateful_alu assign_seq_alu {
	reg: seq_reg;

	update_lo_1_value: register_lo + 1;

	output_value: alu_lo;
	output_dst: seq_hdr.seq;
}

action assign_seq() {
	assign_seq_alu.execute_stateful_alu(0);
	modify_field(seq_hdr.is_assigned, 1);
}

table assign_seq_tbl {
	reads {
		op_hdr.optype: exact;
		seq_hdr.is_assigned: exact;
	}
	actions {
		assign_seq;
	}
	size: 1;
}

register savedseq_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu try_update_savedseq_alu {
	reg: savedseq_reg;

	condition_lo: (seq_hdr.seq > register_lo) or ((register_lo - seq_hdr.seq) == 0xFFFFFFFF);

	update_lo_1_predicate: condition_lo;
	update_lo_1_value: seq_hdr.seq;
	update_lo_2_predicate: not condition_lo;
	update_lo_2_value: register_lo;

	output_value: predicate;
	output_dst: meta.canput;
}

action try_update_savedseq() {
	try_update_savedseq_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu reset_savedseq_alu {
	reg: savedseq_reg;

	update_lo_1_value: 0;
}

action reset_savedseq() {
	reset_savedseq_alu.execute_stateful_alu(meta.hashidx);
}

table access_savedseq_tbl {
	reads {
		op_hdr.optype: exact;
		seq_hdr.is_assigned: exact;
		meta.ismatch_keylololo: exact;
		meta.ismatch_keylolohi: exact;
		meta.ismatch_keylohilo: exact;
		meta.ismatch_keylohihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		try_update_savedseq;
		reset_savedseq;
	}
	size: 1;
}
