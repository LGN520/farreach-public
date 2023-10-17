register<bit<32>>(KV_BUCKET_COUNT) largevalueseq_reg;

action get_largevalueseq() {
	largevalueseq_reg.read(meta.meta.largevalueseq,(bit<32>)hdr.inswitch_hdr.idx);
}

action set_largevalueseq() {
	largevalueseq_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.seq_hdr.seq);
	meta.meta.largevalueseq = 0;
	meta.meta.is_largevalueblock = 0;
}

// CACHE_POP_INSWITCH 
action reset_largevalueseq() {
	largevalueseq_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
	meta.meta.largevalueseq = 0;
	meta.meta.is_largevalueblock = 0;
	hdr.clone_hdr.assignedseq_for_farreach = hdr.seq_hdr.seq;
}

action reset_meta_largevalueseq() {
	meta.meta.largevalueseq = 0;
	meta.meta.is_largevalueblock = 0;
}

@pragma stage 2
table access_largevalueseq_and_save_assignedseq_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		hdr.validvalue_hdr.validvalue: exact;
		hdr.fraginfo_hdr.cur_fragidx: exact;
	}	
	actions = {
		get_largevalueseq;
		set_largevalueseq;
		reset_largevalueseq; // PUT/DELREQ_INSWITCH will invoke resset_largevalueseq() and save the assignedseq into clone_hdr
		reset_meta_largevalueseq; // not touch largevalueseq_reg
	}
	default_action = reset_meta_largevalueseq();
	size = 32;
}
