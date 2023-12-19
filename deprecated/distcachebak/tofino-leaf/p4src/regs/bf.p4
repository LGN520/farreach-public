register<bit<1>>(BF_BUCKET_COUNT) bf1_reg;
register<bit<1>>(BF_BUCKET_COUNT) bf2_reg;

action update_bf1() {
	// update_bf1_alu.execute_stateful_alu(hdr.inswitch_hdr.hashval_for_bf1);
	bf1_reg.write((bit<32>)hdr.inswitch_hdr.hashval_for_bf1,1);
	meta.is_report1 = 1;
}
action reset_is_report1() {
	meta.is_report1 = 0;
}

@pragma stage 3
table access_bf1_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		//meta.is_hot: exact;
		meta.cm1_predicate: exact;
		meta.cm2_predicate: exact;
		meta.cm3_predicate: exact;
		meta.cm4_predicate: exact;
	}
	actions = {
		update_bf1;
		reset_is_report1;
	}
	default_action= reset_is_report1;
	size = 4;
}


action update_bf2() {
	// update_bf2_alu.execute_stateful_alu(hdr.inswitch_hdr.hashval_for_bf2);
	bf2_reg.write((bit<32>)hdr.inswitch_hdr.hashval_for_bf2,1);
	meta.is_report2 = 1;
}

action reset_is_report2() {
	meta.is_report2 = 0;
}

@pragma stage 3
table access_bf2_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		//meta.is_hot: exact;
		meta.cm1_predicate: exact;
		meta.cm2_predicate: exact;
		meta.cm3_predicate: exact;
		meta.cm4_predicate: exact;
	}
	actions = {
		update_bf2;
		reset_is_report2;
	}
	default_action= reset_is_report2;
	size = 4;
}

