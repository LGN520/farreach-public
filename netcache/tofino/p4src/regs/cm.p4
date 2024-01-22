Register<bit<16>,bit<32>>(CM_BUCKET_COUNT) cm1_reg;
Register<bit<16>,bit<32>>(CM_BUCKET_COUNT) cm2_reg;
Register<bit<16>,bit<32>>(CM_BUCKET_COUNT) cm3_reg;
Register<bit<16>,bit<32>>(CM_BUCKET_COUNT) cm4_reg;
RegisterAction<bit<16>, bit<32>, bit<2>>(cm1_reg) cm1_reg_update_alu = {
	void apply(inout bit<16> register_data, out bit<2> result) {
		register_data = register_data + 1; 
		if(register_data >= hdr.inswitch_hdr.hot_threshold){
			result = 2;
		}else{
			result = 1;
		}
	}
};

action update_cm1() {
	meta.cm1_predicate = cm1_reg_update_alu.execute((bit<32>)hdr.inswitch_hdr.hashval_for_cm1);
}

action initialize_cm1_predicate() {
	meta.cm1_predicate = 1; // default: false (1)
}

@pragma stage 1
table access_cm1_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_sampled: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.is_latest: exact;
	}
	actions = {
		update_cm1;
		initialize_cm1_predicate;
	}
	default_action = initialize_cm1_predicate();
	size = 4;
}

RegisterAction<bit<16>, bit<32>, bit<2>>(cm2_reg) cm2_reg_update_alu = {
	void apply(inout bit<16> register_data, out bit<2> result) {
		register_data = register_data + 1; 
		if(register_data >= hdr.inswitch_hdr.hot_threshold){
			result = 2;
		}else{
				result = 1;
		}
	}
};

action update_cm2() {
	meta.cm2_predicate = cm2_reg_update_alu.execute((bit<32>)hdr.inswitch_hdr.hashval_for_cm2);
}

action initialize_cm2_predicate() {
	meta.cm2_predicate = 1; // default: false (1)
}

@pragma stage 1
table access_cm2_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_sampled: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.is_latest: exact;
	}
	actions = {
		update_cm2;
		initialize_cm2_predicate;
	}
	default_action = initialize_cm2_predicate();
	size = 4;
}

RegisterAction<bit<16>, bit<32>, bit<2>>(cm3_reg) cm3_reg_update_alu = {
	void apply(inout bit<16> register_data, out bit<2> result) {
		register_data = register_data + 1; 
		if(register_data >= hdr.inswitch_hdr.hot_threshold){
			result = 2;
		}else{
				result = 1;
		}
	}
};

action update_cm3() {
	meta.cm3_predicate = cm3_reg_update_alu.execute((bit<32>)hdr.inswitch_hdr.hashval_for_cm3);
}
action initialize_cm3_predicate() {
	meta.cm3_predicate = 1; // default: false (1)
}

@pragma stage 1
table access_cm3_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_sampled: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.is_latest: exact;
	}
	actions = {
		update_cm3;
		initialize_cm3_predicate;
	}
	default_action = initialize_cm3_predicate();
	size = 4;
}

RegisterAction<bit<16>, bit<32>, bit<2>>(cm4_reg) cm4_reg_update_alu = {
	void apply(inout bit<16> register_data, out bit<2> result) {
		register_data = register_data + 1; 
		if(register_data >= hdr.inswitch_hdr.hot_threshold){
			result = 2;
		}else{
				result = 1;
		}
	}
};

action update_cm4() {
	meta.cm4_predicate = cm4_reg_update_alu.execute((bit<32>)hdr.inswitch_hdr.hashval_for_cm4);
}

action initialize_cm4_predicate() {
	meta.cm4_predicate = 1; // default: false (1)
}

@pragma stage 1
table access_cm4_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_sampled: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.is_latest: exact;
	}
	actions = {
		update_cm4;
		initialize_cm4_predicate;
	}
	default_action = initialize_cm4_predicate();
	size = 4;
}
