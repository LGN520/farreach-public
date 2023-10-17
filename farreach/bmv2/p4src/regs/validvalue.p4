register<bit<8>>(KV_BUCKET_COUNT) validvalue_reg;
bit<32> validvalue_res;

action get_validvalue() {
	validvalue_reg.read(hdr.validvalue_hdr.validvalue,(bit<32>)hdr.inswitch_hdr.idx);
}

action set_validvalue() {
	validvalue_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.validvalue_hdr.validvalue);
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
