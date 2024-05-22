// register cm1_reg {
// 	width: 16;
// 	instance_count: CM_BUCKET_COUNT;
// }
register<bit<16>>(CM_BUCKET_COUNT) cm1_reg;
register<bit<16>>(CM_BUCKET_COUNT) cm2_reg;
register<bit<16>>(CM_BUCKET_COUNT) cm3_reg;
register<bit<16>>(CM_BUCKET_COUNT) cm4_reg;
bit<16> cm1_res;
bit<16> cm2_res;
bit<16> cm3_res;
bit<16> cm4_res;


action update_cm1() {
	// update_cm1_alu.execute_stateful_alu(hdr.inswitch_hdr.hashval_for_cm1);
	cm1_reg.read(cm1_res,(bit<32>)hdr.inswitch_hdr.hashval_for_cm1);
	meta.meta.cm1_predicate = 1;
	if(cm1_res >= hdr.inswitch_hdr.hot_threshold){
		meta.meta.cm1_predicate = 2;
	}
	cm1_reg.write((bit<32>)hdr.inswitch_hdr.hashval_for_cm1,cm1_res+1);
}

action initialize_cm1_predicate() {
	meta.meta.cm1_predicate = 1; // default: false (1)
}

@pragma stage 1
table access_cm1_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_sampled: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.meta.is_latest: exact;
	}
	actions = {
		update_cm1;
		initialize_cm1_predicate;
	}
	default_action = initialize_cm1_predicate();
	size = 4;
}

action update_cm2() {

	cm2_reg.read(cm2_res,(bit<32>)hdr.inswitch_hdr.hashval_for_cm2);
	meta.meta.cm2_predicate = 1;
	if(cm2_res >= hdr.inswitch_hdr.hot_threshold){
		meta.meta.cm2_predicate = 2;
	}
	cm2_reg.write((bit<32>)hdr.inswitch_hdr.hashval_for_cm2,cm2_res+1);
}

action initialize_cm2_predicate() {
	meta.meta.cm2_predicate = 1; // default: false (1)
}

@pragma stage 1
table access_cm2_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_sampled: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.meta.is_latest: exact;
	}
	actions = {
		update_cm2;
		initialize_cm2_predicate;
	}
	default_action = initialize_cm2_predicate();
	size = 4;
}


action update_cm3() {
	cm3_reg.read(cm3_res,(bit<32>)hdr.inswitch_hdr.hashval_for_cm3);
	meta.meta.cm3_predicate = 1;
	if(cm3_res >= hdr.inswitch_hdr.hot_threshold){
		meta.meta.cm3_predicate = 2;
	}
	cm3_reg.write((bit<32>)hdr.inswitch_hdr.hashval_for_cm3,cm3_res+1);
}

action initialize_cm3_predicate() {
	meta.meta.cm3_predicate = 1; // default: false (1)
}

@pragma stage 1
table access_cm3_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_sampled: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.meta.is_latest: exact;
	}
	actions = {
		update_cm3;
		initialize_cm3_predicate;
	}
	default_action = initialize_cm3_predicate();
	size = 4;
}

action update_cm4() {
	cm4_reg.read(cm4_res,(bit<32>)hdr.inswitch_hdr.hashval_for_cm4);
	meta.meta.cm4_predicate = 1;
	if(cm4_res >= hdr.inswitch_hdr.hot_threshold){
		meta.meta.cm4_predicate = 2;
	}
	cm4_reg.write((bit<32>)hdr.inswitch_hdr.hashval_for_cm4,cm4_res+1);
}

action initialize_cm4_predicate() {
	meta.meta.cm4_predicate = 1; // default: false (1)
}

@pragma stage 1
table access_cm4_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_sampled: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.meta.is_latest: exact;
	}
	actions = {
		update_cm4;
		initialize_cm4_predicate;
	}
	default_action = initialize_cm4_predicate();
	size = 4;
}
