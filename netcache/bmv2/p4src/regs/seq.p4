// register seq_reg {
// 	width: 32;
// 	instance_count: SEQ_BUCKET_COUNT;
// }
register<bit<32>>(SEQ_BUCKET_COUNT) seq_reg;
bit<32>seq_res;
// blackbox stateful_alu assign_seq_alu {
// 	reg: seq_reg;

// 	// TODO: we can use 0xFFFFFFFF or remove condition_lo if data plane does not use seq -> we can resort to servers to solve seq overflow issue
// 	condition_lo: register_lo == 0x7FFFFFFF;

// 	update_lo_1_predicate: not condition_lo;
// 	update_lo_1_value: register_lo + 1;
// 	update_lo_2_predicate: condition_lo;
// 	update_lo_2_value: 0;

// 	output_value: alu_lo;
// 	output_dst: seq_hdr.seq;
// }

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

// register savedseq_reg {
// 	width: 32;
// 	instance_count: KV_BUCKET_COUNT;
// }
register<bit<32>>(KV_BUCKET_COUNT) savedseq_reg;
// blackbox stateful_alu get_savedseq_alu {
// 	reg: savedseq_reg;

// 	update_lo_1_value: register_lo;

// 	output_value: register_lo;
// 	output_dst: seq_hdr.seq;
// }

action get_savedseq() {
	// get_savedseq_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	savedseq_reg.read(hdr.seq_hdr.seq,(bit<32>)hdr.inswitch_hdr.idx);
}

// blackbox stateful_alu set_and_get_savedseq_alu {
// 	reg: savedseq_reg;

// 	update_lo_1_value: seq_hdr.seq;

// 	output_value: register_lo;
// 	output_dst: seq_hdr.seq;
// }

action set_and_get_savedseq() {
	// set_and_get_savedseq_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	savedseq_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.seq_hdr.seq);
}

@pragma stage 2
table access_savedseq_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.meta.is_latest: exact;
	}
	actions = {
		get_savedseq;
		set_and_get_savedseq;
		NoAction;
	}
	default_action = NoAction();
	size = 16;
}
