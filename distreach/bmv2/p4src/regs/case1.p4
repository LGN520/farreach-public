register<bit<1>>(KV_BUCKET_COUNT) case1_reg;

action read_case1() {
	case1_reg.read(meta.meta.is_case1,(bit<32>)hdr.inswitch_hdr.idx);
}

action try_case1() {
	latest_reg.write((bit<32>)hdr.inswitch_hdr.idx,1);
	meta.meta.is_case1=1;
}

action reset_is_case1() {
	meta.meta.is_case1 = 0;
}

@pragma stage 3
table access_case1_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		hdr.validvalue_hdr.validvalue: exact;
		meta.meta.is_latest: exact;
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
