register<bit<1>>(KV_BUCKET_COUNT) deleted_reg;

action get_deleted() {
	// get_deleted_alu.execute_stateful_alu(meta.idx);
	deleted_reg.read(meta.is_deleted,(bit<32>)meta.idx);
	if(meta.is_deleted == 1)
		meta.is_cached = 0;
}

action set_and_get_deleted() {
	// set_and_get_deleted_alu.execute_stateful_alu(meta.idx);
	deleted_reg.write((bit<32>)meta.idx,1);
	meta.is_deleted=1;
}


action reset_and_get_deleted() {
	// reset_and_get_deleted_alu.execute_stateful_alu(meta.idx);
	deleted_reg.write((bit<32>)meta.idx,0);
	meta.is_deleted = 0;
}

action reset_is_deleted() {
	meta.is_deleted = 0;
}

@pragma stage 2
table access_deleted_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.is_cached: exact;
		// meta.is_latest: exact;
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
