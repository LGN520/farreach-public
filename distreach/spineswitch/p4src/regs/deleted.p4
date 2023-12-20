register<bit<1>>(KV_BUCKET_COUNT) deleted_reg;

action get_deleted() {
	// get_deleted_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	deleted_reg.read(meta.meta.is_deleted,(bit<32>)hdr.inswitch_hdr.idx);
}


action set_and_get_deleted() {
	// set_and_get_deleted_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	deleted_reg.write((bit<32>)hdr.inswitch_hdr.idx,1);
	meta.meta.is_deleted=1;
}


action reset_and_get_deleted() {
	// reset_and_get_deleted_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	deleted_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
	meta.meta.is_deleted = 0;
}

action reset_is_deleted() {
	meta.meta.is_deleted = 0;
}

@pragma stage 3
table access_deleted_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		hdr.validvalue_hdr.validvalue: exact;
		meta.meta.is_latest: exact;
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
