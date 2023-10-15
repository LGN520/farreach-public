register<bit<32>>(KV_BUCKET_COUNT) cache_frequency_reg;
bit<32> cache_frequency_res;

action get_cache_frequency() {
	// get_cache_frequency_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	cache_frequency_reg.read(hdr.frequency_hdr.frequency,(bit<32>)hdr.inswitch_hdr.idx);
}

action update_cache_frequency() {
	cache_frequency_reg.read(cache_frequency_res,(bit<32>)hdr.inswitch_hdr.idx);
	cache_frequency_reg.write((bit<32>)hdr.inswitch_hdr.idx,cache_frequency_res+1);
}

action reset_cache_frequency() {
	// reset_cache_frequency_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	cache_frequency_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}
@pragma stage 1
table access_cache_frequency_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_sampled: exact;
		hdr.inswitch_hdr.is_cached: exact;
	}
	actions = {
		get_cache_frequency;
		update_cache_frequency;
		reset_cache_frequency;
		NoAction;
	}
	default_action = NoAction();
	size = 16;
}
