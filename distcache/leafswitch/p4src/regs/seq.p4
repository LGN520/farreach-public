register<bit<32>>(SEQ_BUCKET_COUNT) seq_reg;
bit<32>seq_res;

action assign_seq() {
	// assign_seq_alu.execute_stateful_alu(hdr.inswitch_hdr.hashval_for_seq);
	seq_reg.read(seq_res,(bit<32>)hdr.inswitch_hdr.hashval_for_seq);
	if(seq_res == 0x7FFFFFFF){
		hdr.seq_hdr.seq=0;
		seq_res =0;
	}
	else{
		hdr.seq_hdr.seq=seq_res+1;
		seq_res= seq_res+1;
	}
	seq_reg.write((bit<32>)hdr.inswitch_hdr.hashval_for_seq,seq_res);
}

@pragma stage 0
table access_seq_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.fraginfo_hdr.cur_fragidx: exact;
	}
	actions = {
		assign_seq;
		NoAction;
	}
	default_action = NoAction();
#ifdef DEBUG
	size = 32; // use seq_req for debugging (check whether some packet type enters the egress pipeline)
#else
	size = 4;
#endif
}

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
