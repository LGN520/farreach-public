Register<bit<32>,bit<32>>(KV_BUCKET_COUNT) cache_frequency_reg;
RegisterAction<bit<32>, bit<32>, bit<32>>(cache_frequency_reg) cache_frequency_reg_get_alu = {
	void apply(inout bit<32> register_data, out bit<32> result) {
		result = register_data;
	}
};
RegisterAction<bit<32>, bit<32>, bit<32>>(cache_frequency_reg) cache_frequency_reg_update_alu = {
	void apply(inout bit<32> register_data) {
		register_data = register_data + 1;
		// result = register_data;
	}
};
RegisterAction<bit<32>, bit<32>, bit<32>>(cache_frequency_reg) cache_frequency_reg_reset_alu = {
	void apply(inout bit<32> register_data) {
		register_data = 0;
		// result = register_data;
	}
};
action get_cache_frequency() {
	// get_cache_frequency_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.frequency_hdr.frequency = cache_frequency_reg_get_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
	// eg_dprsr_md.drop_ctl = 1;
}

action update_cache_frequency() {
	cache_frequency_reg_update_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}

action reset_cache_frequency() {
	cache_frequency_reg_reset_alu.execute((bit<32>)hdr.inswitch_hdr.idx);
}

@pragma stage 2
table access_cache_frequency_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_sampled: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.is_latest: exact;
	}
	actions = {
		get_cache_frequency;
		update_cache_frequency;
		reset_cache_frequency;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}
