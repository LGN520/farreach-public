Register<bit<32>,bit<32>>(SEQ_BUCKET_COUNT) seq_reg;
RegisterAction<bit<32>, bit<32>, bit<32>>(seq_reg) seq_reg_get_set_alu = {
	void apply(inout bit<32> register_data, out bit<32> result) {
		if(register_data == 0x7FFFFFFF){
			register_data = 0;result = register_data;
		}
		else{
			register_data = register_data + 1;result = register_data;
		}
		
	}
};
action assign_seq() {
	hdr.seq_hdr.seq = seq_reg_get_set_alu.execute((bit<32>)hdr.inswitch_hdr.hashval_for_seq);
}

@pragma stage 1
table access_seq_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.fraginfo_hdr.cur_fragidx: exact;
		hdr.inswitch_hdr.is_found: exact;
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


Register<bit<32>,bit<32>>(KV_BUCKET_COUNT) savedseq_reg;
RegisterAction<bit<32>, bit<32>, bit<32>>(savedseq_reg) savedseq_reg_get_alu = {
	void apply(inout bit<32> register_data, out bit<32> result) {
		result = register_data;
	}
};
action get_savedseq() {
	hdr.seq_hdr.seq = savedseq_reg_get_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}
RegisterAction<bit<32>, bit<32>, bit<32>>(savedseq_reg) savedseq_reg_get_set_alu = {
	void apply(inout bit<32> register_data){
        register_data = hdr.seq_hdr.seq;
	}
};
action set_and_get_savedseq() {
	savedseq_reg_get_set_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}

@pragma stage 3
table access_savedseq_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		hdr.validvalue_hdr.validvalue: exact;
		hdr.inswitch_hdr.is_latest: exact;
		hdr.inswitch_hdr.is_found: exact;
	}
	actions = {
		get_savedseq;
		set_and_get_savedseq;
		NoAction;
	}
	default_action = NoAction();
	size = 64;
}
