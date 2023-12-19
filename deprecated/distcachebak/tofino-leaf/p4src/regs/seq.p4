register<bit<32>>(KV_BUCKET_COUNT) savedseq_reg;


action get_savedseq() {
	// get_savedseq_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	savedseq_reg.read(hdr.seq_hdr.seq,(bit<32>)hdr.inswitch_hdr.idx);
}

action set_and_get_savedseq() {
	// set_and_get_savedseq_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	savedseq_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.seq_hdr.seq);
}

@pragma stage 2
table access_savedseq_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.is_latest: exact;
	}
	actions = {
		get_savedseq;
		set_and_get_savedseq;
		NoAction;
	}
	default_action = NoAction();
	size = 16;
}
