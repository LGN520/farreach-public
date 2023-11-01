register<bit<1>>(BF_BUCKET_COUNT) bf1_reg;
register<bit<1>>(BF_BUCKET_COUNT) bf2_reg;
register<bit<1>>(BF_BUCKET_COUNT) bf3_reg;
// bit<1> bf1_flag;
// bit<1> bf2_flag;
// bit<1> bf3_flag;

action update_bf1() {
	// update_bf1_alu.execute_stateful_alu(hdr.inswitch_hdr.hashval_for_bf1);
	bf1_reg.write((bit<32>)hdr.inswitch_hdr.hashval_for_bf1,1);
	meta.meta.is_report1 = 1;
}

action reset_is_report1() {
	meta.meta.is_report1 = 0;
}

@pragma stage 3
table access_bf1_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.meta.is_hot: exact;
	}
	actions = {
		update_bf1;
		reset_is_report1;
	}
	default_action = reset_is_report1;
	size = 4;
}



action update_bf2() {
	// update_bf2_alu.execute_stateful_alu(hdr.inswitch_hdr.hashval_for_bf2);
	bf2_reg.write((bit<32>)hdr.inswitch_hdr.hashval_for_bf2,1);
	meta.meta.is_report2 = 1;
}

action reset_is_report2() {
	meta.meta.is_report2 = 0;
}

@pragma stage 3
table access_bf2_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.meta.is_hot: exact;
	}
	actions = {
		update_bf2;
		reset_is_report2;
	}
	default_action = reset_is_report2;
	size = 4;
}

action update_bf3() {
	// update_bf3_alu.execute_stateful_alu(hdr.inswitch_hdr.hashval_for_bf3);
	bf3_reg.write((bit<32>)hdr.inswitch_hdr.hashval_for_bf3,1);
	meta.meta.is_report3 = 1;
}

action reset_is_report3() {
	meta.meta.is_report3 = 0;
}

@pragma stage 3
table access_bf3_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.meta.is_hot: exact;
	}
	actions = {
		update_bf3;
		reset_is_report3;
	}
	default_action = reset_is_report3;
	size = 4;
}
