register<bit<1>>(KV_BUCKET_COUNT) deleted_reg;
// blackbox stateful_alu get_deleted_alu {
// 	reg: deleted_reg;

// 	update_lo_1_value: read_bit;

// 	output_value: alu_lo;
// 	output_dst: meta.is_deleted;
// }

action get_deleted() {
	// get_deleted_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	deleted_reg.read(meta.meta.is_deleted,(bit<32>)hdr.inswitch_hdr.idx);
}

// blackbox stateful_alu set_and_get_deleted_alu {
// 	reg: deleted_reg;

// 	update_lo_1_value: set_bit;

// 	output_value: alu_lo;
// 	output_dst: meta.is_deleted;
// }

action set_and_get_deleted() {
	// set_and_get_deleted_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	deleted_reg.write((bit<32>)hdr.inswitch_hdr.idx,1);
	meta.meta.is_deleted=1;
}

// blackbox stateful_alu reset_and_get_deleted_alu {
// 	reg: deleted_reg;

// 	update_lo_1_value: clr_bit;

// 	output_value: alu_lo;
// 	output_dst: meta.is_deleted;
// }

action reset_and_get_deleted() {
	// reset_and_get_deleted_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	deleted_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
	meta.meta.is_deleted = 0;
}

action reset_is_deleted() {
	meta.meta.is_deleted = 0;
}

@pragma stage 2
table access_deleted_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.meta.is_latest: exact;
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
