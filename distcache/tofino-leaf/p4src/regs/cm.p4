register cm1_reg {
	width: 16;
	instance_count: CM_BUCKET_COUNT;
}

blackbox stateful_alu update_cm1_alu {
	reg: cm1_reg;

	// Equivalent to: alu_lo > inswitch_hdr.hot_threshold
	condition_lo: register_lo >= inswitch_hdr.hot_threshold;

	update_lo_1_value: register_lo + 1;

	output_value: predicate;
	output_dst: meta.cm1_predicate; // false: 1; true: 2
}

action update_cm1() {
	update_cm1_alu.execute_stateful_alu(inswitch_hdr.hashval_for_cm1);
}

action initialize_cm1_predicate() {
	modify_field(meta.cm1_predicate, 1); // default: false (1)
}

@pragma stage 1
table access_cm1_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_sampled: exact;
		inswitch_hdr.is_cached: exact;
		meta.is_latest: exact;
	}
	actions {
		update_cm1;
		initialize_cm1_predicate;
	}
	default_action: initialize_cm1_predicate();
	size: 4;
}

register cm2_reg {
	width: 16;
	instance_count: CM_BUCKET_COUNT;
}

blackbox stateful_alu update_cm2_alu {
	reg: cm2_reg;

	// Equivalent to: alu_lo > inswitch_hdr.hot_threshold
	condition_lo: register_lo >= inswitch_hdr.hot_threshold;

	update_lo_1_value: register_lo + 1;

	output_value: predicate;
	output_dst: meta.cm2_predicate; // false: 1; true: 2
}

action update_cm2() {
	update_cm2_alu.execute_stateful_alu(inswitch_hdr.hashval_for_cm2);
}

action initialize_cm2_predicate() {
	modify_field(meta.cm2_predicate, 1); // default: false (1)
}

@pragma stage 1
table access_cm2_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_sampled: exact;
		inswitch_hdr.is_cached: exact;
		meta.is_latest: exact;
	}
	actions {
		update_cm2;
		initialize_cm2_predicate;
	}
	default_action: initialize_cm2_predicate();
	size: 4;
}

register cm3_reg {
	width: 16;
	instance_count: CM_BUCKET_COUNT;
}

blackbox stateful_alu update_cm3_alu {
	reg: cm3_reg;

	// Equivalent to: alu_lo > inswitch_hdr.hot_threshold
	condition_lo: register_lo >= inswitch_hdr.hot_threshold;

	update_lo_1_value: register_lo + 1;

	output_value: predicate;
	output_dst: meta.cm3_predicate; // false: 1; true: 2
}

action update_cm3() {
	update_cm3_alu.execute_stateful_alu(inswitch_hdr.hashval_for_cm3);
}

action initialize_cm3_predicate() {
	modify_field(meta.cm3_predicate, 1); // default: false (1)
}

@pragma stage 1
table access_cm3_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_sampled: exact;
		inswitch_hdr.is_cached: exact;
		meta.is_latest: exact;
	}
	actions {
		update_cm3;
		initialize_cm3_predicate;
	}
	default_action: initialize_cm3_predicate();
	size: 4;
}

/*register cm4_reg {
	width: 16;
	instance_count: CM_BUCKET_COUNT;
}

blackbox stateful_alu update_cm4_alu {
	reg: cm4_reg;

	// Equivalent to: alu_lo > inswitch_hdr.hot_threshold
	condition_lo: register_lo >= inswitch_hdr.hot_threshold;

	update_lo_1_value: register_lo + 1;

	output_value: predicate;
	output_dst: meta.cm4_predicate; // false: 1; true: 2
}

action update_cm4() {
	update_cm4_alu.execute_stateful_alu(inswitch_hdr.hashval_for_cm4);
}

action initialize_cm4_predicate() {
	modify_field(meta.cm4_predicate, 1); // default: false (1)
}

@pragma stage 1
table access_cm4_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_sampled: exact;
		inswitch_hdr.is_cached: exact;
		meta.is_latest: exact;
	}
	actions {
		update_cm4;
		initialize_cm4_predicate;
	}
	default_action: initialize_cm4_predicate();
	size: 4;
}*/
