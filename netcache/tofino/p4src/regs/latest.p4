Register<bit<1>,bit<32>>(KV_BUCKET_COUNT) latest_reg;
RegisterAction<bit<1>, bit<32>, bit<1>>(latest_reg) latest_reg_get_alu = {
	void apply(inout bit<1> register_data, out bit<1> result) {
		result = register_data;
	}
};
RegisterAction<bit<1>, bit<32>, bit<1>>(latest_reg) latest_reg_get_set_alu = {
	void apply(inout bit<1> register_data, out bit<1> result) {
		register_data = 1; 
		result = 1;
	}
};
RegisterAction<bit<1>, bit<32>, bit<1>>(latest_reg) latest_reg_get_reset_alu = {
	void apply(inout bit<1> register_data, out bit<1> result) {
		register_data = 0; 
		result = 0;
	}
};
action get_latest() {
	meta.is_latest = latest_reg_get_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}

action set_and_get_latest() {
	meta.is_latest = latest_reg_get_set_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}

action reset_and_get_latest() {
	meta.is_latest = latest_reg_get_reset_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}

action reset_is_latest() {
	meta.is_latest = 0;
}

@pragma stage 0
table access_latest_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		hdr.fraginfo_hdr.cur_fragidx: exact;
	}
	actions = {
		get_latest;
		set_and_get_latest;
		reset_and_get_latest;
		reset_is_latest; // not touch latest_reg
	}
	default_action = reset_is_latest();
	size = 32;
}
