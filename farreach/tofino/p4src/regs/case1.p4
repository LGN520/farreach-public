Register<bit<1>,bit<32>>(KV_BUCKET_COUNT) case1_reg;
RegisterAction<bit<1>, bit<32>, bit<1>>(case1_reg) case1_reg_get_alu = {
	void apply(inout bit<1> register_data, out bit<1> result) {
		result = register_data;
	}
};
RegisterAction<bit<1>, bit<32>, bit<1>>(case1_reg) case1_reg_set_alu = {
	void apply(inout bit<1> register_data) {
		register_data = 1; 
	}
};
action read_case1() {
	meta.is_case1 = case1_reg_get_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}

action try_case1() {
	case1_reg_set_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
	meta.is_case1=1;
}

action reset_is_case1() {
	meta.is_case1 = 0;
}

@pragma stage 3
table access_case1_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		hdr.validvalue_hdr.validvalue: exact;
		meta.is_latest: exact;
		hdr.inswitch_hdr.snapshot_flag: exact;
	}
	actions = {
		try_case1; // touch case1_reg
		read_case1; // touch case1_reg
		reset_is_case1; // not touch case1_reg
	}
	default_action = reset_is_case1();
	size = 8;
}
