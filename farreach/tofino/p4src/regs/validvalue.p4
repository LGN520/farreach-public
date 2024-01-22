Register<bit<8>,bit<32>>(KV_BUCKET_COUNT) validvalue_reg;

RegisterAction<bit<8>, bit<32>, bit<8>>(validvalue_reg) validvalue_reg_get_alu = {
	void apply(inout bit<8> register_data, out bit<8> result) {
		result = register_data;
	}
};
RegisterAction<bit<8>, bit<32>, bit<8>>(validvalue_reg) validvalue_reg_set_alu = {
	void apply(inout bit<8> register_data) {
		register_data = hdr.validvalue_hdr.validvalue; 
	}
};
action get_validvalue() {
	// hdr.validvalue_hdr.setValid();
	hdr.validvalue_hdr.validvalue = validvalue_reg_get_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}

action set_validvalue() {
	// hdr.validvalue_hdr.setValid();
	validvalue_reg_set_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}
action reset_meta_validvalue() {
	hdr.validvalue_hdr.validvalue = 0;
}

@pragma stage 1
table access_validvalue_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
	}
	actions = {
		get_validvalue;
		set_validvalue;
		reset_meta_validvalue; // not touch validvalue_reg
	}
	default_action = reset_meta_validvalue();
	size = 8;
}
