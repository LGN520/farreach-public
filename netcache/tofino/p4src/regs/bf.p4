Register<bit<1>,bit<32>>(BF_BUCKET_COUNT) bf1_reg;
Register<bit<1>,bit<32>>(BF_BUCKET_COUNT) bf2_reg;
Register<bit<1>,bit<32>>(BF_BUCKET_COUNT) bf3_reg;

RegisterAction<bit<1>, bit<32>, bit<1>>(bf1_reg) bf1_reg_update_alu = {
	void apply(inout bit<1> register_data,out bit<1> result) {
		result = register_data;
		register_data = 1;
	}
};
action update_bf1() {
	meta.is_report1 = bf1_reg_update_alu.execute((bit<32>)hdr.inswitch_hdr.hashval_for_bf1);
}

action reset_is_report1() {
	meta.is_report1 = 0;
}

@pragma stage 3
table access_bf1_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.is_hot: exact;
	}
	actions = {
		update_bf1;
		reset_is_report1;
	}
	default_action = reset_is_report1;
	size = 4;
}

RegisterAction<bit<1>, bit<32>, bit<1>>(bf2_reg) bf2_reg_update_alu = {
	void apply(inout bit<1> register_data,out bit<1> result) {
		result = register_data;
		register_data = 1;
	}
};
action update_bf2() {
	meta.is_report2 = bf2_reg_update_alu.execute((bit<32>)hdr.inswitch_hdr.hashval_for_bf2);
}

action reset_is_report2() {
	meta.is_report2 = 0;
}

@pragma stage 3
table access_bf2_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.is_hot: exact;
	}
	actions = {
		update_bf2;
		reset_is_report2;
	}
	default_action = reset_is_report2;
	size = 4;
}

RegisterAction<bit<1>, bit<32>, bit<1>>(bf3_reg) bf3_reg_update_alu = {
	void apply(inout bit<1> register_data,out bit<1> result) {
		result = register_data;
		register_data = 1;
	}
};
action update_bf3() {
	// meta.is_report1 = 1;
	// meta.is_report2 = 1;
	meta.is_report3 = bf3_reg_update_alu.execute((bit<32>)hdr.inswitch_hdr.hashval_for_bf3);
}
action reset_is_report3() {
	// meta.is_report1 = 0;
	// meta.is_report2 = 0;
	meta.is_report3 = 0;
}

@pragma stage 3
table access_bf3_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		meta.is_hot: exact;
	}
	actions = {
		update_bf3;
		reset_is_report3;
	}
	default_action = reset_is_report3;
	size = 4;
}
