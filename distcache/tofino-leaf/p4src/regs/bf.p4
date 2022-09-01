register bf1_reg {
	width: 1;
	instance_count: BF_BUCKET_COUNT;
}

blackbox stateful_alu update_bf1_alu {
	reg: bf1_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.is_report1;
}

action update_bf1() {
	update_bf1_alu.execute_stateful_alu(inswitch_hdr.hashval_for_bf1);
}

action reset_is_report1() {
	modify_field(meta.is_report1, 0);
}

@pragma stage 3
table access_bf1_tbl {
	reads {
		op_hdr.optype: exact;
		meta.is_hot: exact;
	}
	actions {
		update_bf1;
		reset_is_report1;
	}
	default_action: reset_is_report1;
	size: 4;
}

register bf2_reg {
	width: 1;
	instance_count: BF_BUCKET_COUNT;
}

blackbox stateful_alu update_bf2_alu {
	reg: bf2_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.is_report2;
}

action update_bf2() {
	update_bf2_alu.execute_stateful_alu(inswitch_hdr.hashval_for_bf2);
}

action reset_is_report2() {
	modify_field(meta.is_report2, 0);
}

@pragma stage 3
table access_bf2_tbl {
	reads {
		op_hdr.optype: exact;
		meta.is_hot: exact;
	}
	actions {
		update_bf2;
		reset_is_report2;
	}
	default_action: reset_is_report2;
	size: 4;
}

/*register bf3_reg {
	width: 1;
	instance_count: BF_BUCKET_COUNT;
}

blackbox stateful_alu update_bf3_alu {
	reg: bf3_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.is_report3;
}

action update_bf3() {
	update_bf3_alu.execute_stateful_alu(inswitch_hdr.hashval_for_bf3);
}

action reset_is_report3() {
	modify_field(meta.is_report3, 0);
}

@pragma stage 3
table access_bf3_tbl {
	reads {
		op_hdr.optype: exact;
		meta.is_hot: exact;
	}
	actions {
		update_bf3;
		reset_is_report3;
	}
	default_action: reset_is_report3;
	size: 4;
}*/
