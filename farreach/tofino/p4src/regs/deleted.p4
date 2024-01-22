Register<bit<1>,bit<32>>(KV_BUCKET_COUNT) deleted_reg;
RegisterAction<bit<1>, bit<32>, bit<1>>(deleted_reg) deleted_reg_get_alu = {
	void apply(inout bit<1> register_data, out bit<1> result) {
		result = register_data;
	}
};
RegisterAction<bit<1>, bit<32>, bit<1>>(deleted_reg) deleted_reg_get_set_alu = {
	void apply(inout bit<1> register_data, out bit<1> result) {
		register_data = 1; 
		result = 1;
	}
};
RegisterAction<bit<1>, bit<32>, bit<1>>(deleted_reg) deleted_reg_get_reset_alu = {
	void apply(inout bit<1> register_data, out bit<1> result) {
		register_data = 0; 
		result = 0;
	}
};
action get_deleted() {
	meta.is_deleted = deleted_reg_get_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}

action set_and_get_deleted() {
	meta.is_deleted = deleted_reg_get_set_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}


action reset_and_get_deleted() {
	meta.is_deleted =deleted_reg_get_reset_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}

action reset_is_deleted() {
	meta.is_deleted = 0;
}

@pragma stage 3
table access_deleted_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		hdr.validvalue_hdr.validvalue: exact;
		meta.is_latest: exact;
		hdr.stat_hdr.stat: exact;
	}
	actions = {
		get_deleted;
		set_and_get_deleted;
		reset_and_get_deleted;
		reset_is_deleted;
	}
	default_action = reset_is_deleted();
	size = 256;
}
