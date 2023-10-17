register<bit<1>>(KV_BUCKET_COUNT) latest_reg;

action get_latest() {
	// get_latest_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	latest_reg.read(meta.meta.is_latest,(bit<32>)hdr.inswitch_hdr.idx);
}

action set_and_get_latest() {
	// set_and_get_latest_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	latest_reg.write((bit<32>)hdr.inswitch_hdr.idx,1);
	meta.meta.is_latest=1;
}


// CACHE_POP_INSWITCH 
action reset_and_get_latest() {
	// reset_and_get_latest_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	latest_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
	meta.meta.is_latest=0;
}

action reset_is_latest() {
	meta.meta.is_latest = 0;
}

@pragma stage 2
table access_latest_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		hdr.validvalue_hdr.validvalue: exact;
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
