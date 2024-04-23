Register<bit<32>,bit<32>>(KV_BUCKET_COUNT) largevalueseq_reg;
RegisterAction<bit<32>, bit<32>, bit<32>>(largevalueseq_reg) largevalueseq_reg_get_alu = {
	void apply(inout bit<32> register_data, out bit<32> result) {
		result = register_data;
	}
};
RegisterAction<bit<32>, bit<32>, bit<32>>(largevalueseq_reg) largevalueseq_reg_set_alu = {
	void apply(inout bit<32> register_data){
        register_data = hdr.seq_hdr.seq;
	}
};
RegisterAction<bit<32>, bit<32>, bit<32>>(largevalueseq_reg) largevalueseq_reg_reset_alu = {
	void apply(inout bit<32> register_data){
        register_data = 0;
	}
};
action get_largevalueseq() {
	// largevalueseq_reg.read(meta.largevalueseq,(bit<32>)hdr.inswitch_hdr.idx);
	hdr.seq_hdr.largevalueseq = largevalueseq_reg_get_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
	// hdr.seq_hdr.largevalueseq = meta.largevalueseq;
}

action set_largevalueseq() {
	largevalueseq_reg_set_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
	hdr.seq_hdr.largevalueseq = 0;	
	// meta.largevalueseq = 0;
	meta.is_largevalueblock = 0;
}

// CACHE_POP_INSWITCH 
action reset_largevalueseq() {
	largevalueseq_reg_reset_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
	hdr.seq_hdr.largevalueseq = 0;
	// meta.largevalueseq = 0;
	meta.is_largevalueblock = 0;
	hdr.clone_hdr.assignedseq_for_farreach = hdr.seq_hdr.seq;
}

action reset_meta_largevalueseq() {
	hdr.seq_hdr.largevalueseq = 0;
	// meta.largevalueseq = 0;
	meta.is_largevalueblock = 0;
}

@pragma stage 2
table access_largevalueseq_and_save_assignedseq_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		hdr.validvalue_hdr.validvalue: exact;
		hdr.fraginfo_hdr.cur_fragidx: exact;
		hdr.inswitch_hdr.is_found: exact;
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
