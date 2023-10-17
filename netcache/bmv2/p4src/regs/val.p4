// NOTE: we do not match optype for val_reg (especialy for val16_reg), as eg_port_forward_tbl may change optype before accessing val16_reg
// NOTE: we also place drop() in the last stage to avoid skipping val16_reg due to immediate drop semantic

// register vallen_reg {
// 	width: 16;
// 	instance_count: KV_BUCKET_COUNT;
// }
register<bit<16>>(KV_BUCKET_COUNT) vallen_reg;
// blackbox stateful_alu get_vallen_alu {
// 	reg: vallen_reg;
	
// 	update_lo_1_value: register_lo;

// 	output_value: register_lo;
// 	output_dst: vallen_hdr.vallen;
// }

action get_vallen() {
	// get_vallen_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallen_reg.read(hdr.vallen_hdr.vallen,(bit<32>)hdr.inswitch_hdr.idx);
	meta.meta.access_val_mode = 1; // get val_reg
}

// blackbox stateful_alu set_and_get_vallen_alu {
// 	reg: vallen_reg;

// 	update_lo_1_value: vallen_hdr.vallen;

// 	output_value: register_lo;
// 	output_dst: vallen_hdr.vallen;
// }

action set_and_get_vallen() {
	// set_and_get_vallen_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallen_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.vallen_hdr.vallen);
	meta.meta.access_val_mode =  2; // set_and_get val_reg
}

// blackbox stateful_alu reset_and_get_vallen_alu {
// 	reg: vallen_reg;

// 	update_lo_1_value: 0;

// 	output_value: register_lo;
// 	output_dst: vallen_hdr.vallen;
// }

action reset_and_get_vallen() {
	// reset_and_get_vallen_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallen_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
	hdr.vallen_hdr.vallen=0;
	meta.meta.access_val_mode =  3; // reset_and_get val_reg
}

action reset_access_val_mode() {
	meta.meta.access_val_mode = 0; // not access val_reg
}

@pragma stage 3
table update_vallen_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
		meta.meta.is_latest: exact;
	}
	actions = {
		get_vallen;
		set_and_get_vallen;
		reset_and_get_vallen;
		reset_access_val_mode;
		NoAction;
	}
	default_action = reset_access_val_mode();
	size = 16;
}

// register vallo1_reg {
// 	width: 32;
// 	instance_count: KV_BUCKET_COUNT;
// }
register<bit<32>>(KV_BUCKET_COUNT) vallo1_reg;
// blackbox stateful_alu get_vallo1_alu {
// 	reg: vallo1_reg;
	
// 	update_lo_1_value: register_lo;

// 	output_value: register_lo;
// 	output_dst: val1_hdr.vallo;
// }

action get_vallo1() {
	// get_vallo1_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo1_reg.read(hdr.val1_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}

// blackbox stateful_alu set_and_get_vallo1_alu {
// 	reg: vallo1_reg;

// 	update_lo_1_value: val1_hdr.vallo;

// 	output_value: register_lo;
// 	output_dst: val1_hdr.vallo;
// }

action set_and_get_vallo1() {
	// set_and_get_vallo1_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo1_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val1_hdr.vallo);
}

// blackbox stateful_alu reset_and_get_vallo1_alu {
// 	reg: vallo1_reg;

// 	update_lo_1_value: 0;

// 	output_value: register_lo;
// 	output_dst: val1_hdr.vallo;
// }

action reset_and_get_vallo1() {
	// reset_and_get_vallo1_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val1_hdr.vallo=0;
	vallo1_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 4
table update_vallo1_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo1;
		set_and_get_vallo1;
		reset_and_get_vallo1;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

// register valhi1_reg {
// 	width: 32;
// 	instance_count: KV_BUCKET_COUNT;
// }
register<bit<32>>(KV_BUCKET_COUNT) valhi1_reg;

action get_valhi1() {
	// get_valhi1_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi1_reg.read(hdr.val1_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi1() {
	// set_and_get_valhi1_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi1_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val1_hdr.valhi);
}
action reset_and_get_valhi1() {
	// reset_and_get_valhi1_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val1_hdr.valhi=0;
	valhi1_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 4
table update_valhi1_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi1;
		set_and_get_valhi1;
		reset_and_get_valhi1;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo2_reg;

action get_vallo2() {
	// get_vallo2_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo2_reg.read(hdr.val2_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo2() {
	// set_and_get_vallo2_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo2_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val2_hdr.vallo);
}
action reset_and_get_vallo2() {
	// reset_and_get_vallo2_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val2_hdr.vallo=0;
	vallo2_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 4
table update_vallo2_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo2;
		set_and_get_vallo2;
		reset_and_get_vallo2;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi2_reg;

action get_valhi2() {
	// get_valhi2_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi2_reg.read(hdr.val2_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi2() {
	// set_and_get_valhi2_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi2_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val2_hdr.valhi);
}
action reset_and_get_valhi2() {
	// reset_and_get_valhi2_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val2_hdr.valhi=0;
	valhi2_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}


@pragma stage 4
table update_valhi2_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi2;
		set_and_get_valhi2;
		reset_and_get_valhi2;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo3_reg;

action get_vallo3() {
	// get_vallo3_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo3_reg.read(hdr.val3_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo3() {
	// set_and_get_vallo3_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo3_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val3_hdr.vallo);
}
action reset_and_get_vallo3() {
	// reset_and_get_vallo3_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val3_hdr.vallo=0;
	vallo3_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 5
table update_vallo3_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo3;
		set_and_get_vallo3;
		reset_and_get_vallo3;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi3_reg;

action get_valhi3() {
	// get_valhi3_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi3_reg.read(hdr.val3_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi3() {
	// set_and_get_valhi3_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi3_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val3_hdr.valhi);
}
action reset_and_get_valhi3() {
	// reset_and_get_valhi3_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val3_hdr.valhi=0;
	valhi3_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}


@pragma stage 5
table update_valhi3_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi3;
		set_and_get_valhi3;
		reset_and_get_valhi3;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo4_reg;

action get_vallo4() {
	// get_vallo4_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo4_reg.read(hdr.val4_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo4() {
	// set_and_get_vallo4_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo4_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val4_hdr.vallo);
}
action reset_and_get_vallo4() {
	// reset_and_get_vallo4_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val4_hdr.vallo=0;
	vallo4_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}


@pragma stage 5
table update_vallo4_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo4;
		set_and_get_vallo4;
		reset_and_get_vallo4;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi4_reg;

action get_valhi4() {
	// get_valhi4_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi4_reg.read(hdr.val4_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi4() {
	// set_and_get_valhi4_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi4_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val4_hdr.valhi);
}
action reset_and_get_valhi4() {
	// reset_and_get_valhi4_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val4_hdr.valhi=0;
	valhi4_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}


@pragma stage 5
table update_valhi4_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi4;
		set_and_get_valhi4;
		reset_and_get_valhi4;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo5_reg;

action get_vallo5() {
	// get_vallo5_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo5_reg.read(hdr.val5_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo5() {
	// set_and_get_vallo5_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo5_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val5_hdr.vallo);
}
action reset_and_get_vallo5() {
	// reset_and_get_vallo5_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val5_hdr.vallo=0;
	vallo5_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}
@pragma stage 6
table update_vallo5_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo5;
		set_and_get_vallo5;
		reset_and_get_vallo5;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi5_reg;

action get_valhi5() {
	// get_valhi5_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi5_reg.read(hdr.val5_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi5() {
	// set_and_get_valhi5_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi5_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val5_hdr.valhi);
}
action reset_and_get_valhi5() {
	// reset_and_get_valhi5_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val5_hdr.valhi=0;
	valhi5_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}
@pragma stage 6
table update_valhi5_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi5;
		set_and_get_valhi5;
		reset_and_get_valhi5;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo6_reg;

action get_vallo6() {
	// get_vallo6_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo6_reg.read(hdr.val6_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo6() {
	// set_and_get_vallo6_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo6_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val6_hdr.vallo);
}
action reset_and_get_vallo6() {
	// reset_and_get_vallo6_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val6_hdr.vallo=0;
	vallo6_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}
@pragma stage 6
table update_vallo6_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo6;
		set_and_get_vallo6;
		reset_and_get_vallo6;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi6_reg;

action get_valhi6() {
	// get_valhi6_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi6_reg.read(hdr.val6_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi6() {
	// set_and_get_valhi6_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi6_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val6_hdr.valhi);
}
action reset_and_get_valhi6() {
	// reset_and_get_valhi6_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val6_hdr.valhi=0;
	valhi6_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 6
table update_valhi6_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi6;
		set_and_get_valhi6;
		reset_and_get_valhi6;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo7_reg;

action get_vallo7() {
	// get_vallo7_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo7_reg.read(hdr.val7_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo7() {
	// set_and_get_vallo7_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo7_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val7_hdr.vallo);
}
action reset_and_get_vallo7() {
	// reset_and_get_vallo7_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val7_hdr.vallo=0;
	vallo7_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}



@pragma stage 7
table update_vallo7_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo7;
		set_and_get_vallo7;
		reset_and_get_vallo7;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi7_reg;

action get_valhi7() {
	// get_valhi7_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi7_reg.read(hdr.val7_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi7() {
	// set_and_get_valhi7_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi7_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val7_hdr.valhi);
}
action reset_and_get_valhi7() {
	// reset_and_get_valhi7_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val7_hdr.valhi=0;
	valhi7_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}



@pragma stage 7
table update_valhi7_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi7;
		set_and_get_valhi7;
		reset_and_get_valhi7;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo8_reg;

action get_vallo8() {
	// get_vallo8_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo8_reg.read(hdr.val8_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo8() {
	// set_and_get_vallo8_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo8_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val8_hdr.vallo);
}
action reset_and_get_vallo8() {
	// reset_and_get_vallo8_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val8_hdr.vallo=0;
	vallo8_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}


@pragma stage 7
table update_vallo8_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo8;
		set_and_get_vallo8;
		reset_and_get_vallo8;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi8_reg;

action get_valhi8() {
	// get_valhi8_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi8_reg.read(hdr.val8_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi8() {
	// set_and_get_valhi8_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi8_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val8_hdr.valhi);
}
action reset_and_get_valhi8() {
	// reset_and_get_valhi8_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val8_hdr.valhi=0;
	valhi8_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}


@pragma stage 7
table update_valhi8_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi8;
		set_and_get_valhi8;
		reset_and_get_valhi8;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo9_reg;

action get_vallo9() {
	// get_vallo9_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo9_reg.read(hdr.val9_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo9() {
	// set_and_get_vallo9_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo9_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val9_hdr.vallo);
}
action reset_and_get_vallo9() {
	// reset_and_get_vallo9_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val9_hdr.vallo=0;
	vallo9_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 8
table update_vallo9_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo9;
		set_and_get_vallo9;
		reset_and_get_vallo9;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi9_reg;

action get_valhi9() {
	// get_valhi9_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi9_reg.read(hdr.val9_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi9() {
	// set_and_get_valhi9_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi9_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val9_hdr.valhi);
}
action reset_and_get_valhi9() {
	// reset_and_get_valhi9_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val9_hdr.valhi=0;
	valhi9_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 8
table update_valhi9_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi9;
		set_and_get_valhi9;
		reset_and_get_valhi9;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo10_reg;

action get_vallo10() {
	// get_vallo10_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo10_reg.read(hdr.val10_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo10() {
	// set_and_get_vallo10_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo10_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val10_hdr.vallo);
}
action reset_and_get_vallo10() {
	// reset_and_get_vallo10_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val10_hdr.vallo=0;
	vallo10_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}


@pragma stage 8
table update_vallo10_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo10;
		set_and_get_vallo10;
		reset_and_get_vallo10;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi10_reg;

action get_valhi10() {
	// get_valhi10_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi10_reg.read(hdr.val10_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi10() {
	// set_and_get_valhi10_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi10_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val10_hdr.valhi);
}
action reset_and_get_valhi10() {
	// reset_and_get_valhi10_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val10_hdr.valhi=0;
	valhi10_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 8
table update_valhi10_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi10;
		set_and_get_valhi10;
		reset_and_get_valhi10;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo11_reg;

action get_vallo11() {
	// get_vallo11_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo11_reg.read(hdr.val11_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo11() {
	// set_and_get_vallo11_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo11_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val11_hdr.vallo);
}
action reset_and_get_vallo11() {
	// reset_and_get_vallo11_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val11_hdr.vallo=0;
	vallo11_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 9
table update_vallo11_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo11;
		set_and_get_vallo11;
		reset_and_get_vallo11;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi11_reg;

action get_valhi11() {
	// get_valhi11_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi11_reg.read(hdr.val11_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi11() {
	// set_and_get_valhi11_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi11_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val11_hdr.valhi);
}
action reset_and_get_valhi11() {
	// reset_and_get_valhi11_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val11_hdr.valhi=0;
	valhi11_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}


@pragma stage 9
table update_valhi11_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi11;
		set_and_get_valhi11;
		reset_and_get_valhi11;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo12_reg;

action get_vallo12() {
	// get_vallo12_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo12_reg.read(hdr.val12_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo12() {
	// set_and_get_vallo12_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo12_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val12_hdr.vallo);
}
action reset_and_get_vallo12() {
	// reset_and_get_vallo12_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val12_hdr.vallo=0;
	vallo12_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}



@pragma stage 9
table update_vallo12_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo12;
		set_and_get_vallo12;
		reset_and_get_vallo12;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi12_reg;

action get_valhi12() {
	// get_valhi12_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi12_reg.read(hdr.val12_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi12() {
	// set_and_get_valhi12_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi12_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val12_hdr.valhi);
}
action reset_and_get_valhi12() {
	// reset_and_get_valhi12_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val12_hdr.valhi=0;
	valhi12_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 9
table update_valhi12_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi12;
		set_and_get_valhi12;
		reset_and_get_valhi12;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo13_reg;

action get_vallo13() {
	// get_vallo13_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo13_reg.read(hdr.val13_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo13() {
	// set_and_get_vallo13_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo13_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val13_hdr.vallo);
}
action reset_and_get_vallo13() {
	// reset_and_get_vallo13_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val13_hdr.vallo=0;
	vallo13_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 10
table update_vallo13_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo13;
		set_and_get_vallo13;
		reset_and_get_vallo13;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi13_reg;

action get_valhi13() {
	// get_valhi13_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi13_reg.read(hdr.val13_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi13() {
	// set_and_get_valhi13_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi13_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val13_hdr.valhi);
}
action reset_and_get_valhi13() {
	// reset_and_get_valhi13_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val13_hdr.valhi=0;
	valhi13_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}@pragma stage 10
table update_valhi13_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi13;
		set_and_get_valhi13;
		reset_and_get_valhi13;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo14_reg;

action get_vallo14() {
	// get_vallo14_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo14_reg.read(hdr.val14_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo14() {
	// set_and_get_vallo14_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo14_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val14_hdr.vallo);
}
action reset_and_get_vallo14() {
	// reset_and_get_vallo14_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val14_hdr.vallo=0;
	vallo14_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 10
table update_vallo14_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo14;
		set_and_get_vallo14;
		reset_and_get_vallo14;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi14_reg;

action get_valhi14() {
	// get_valhi14_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi14_reg.read(hdr.val14_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi14() {
	// set_and_get_valhi14_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi14_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val14_hdr.valhi);
}
action reset_and_get_valhi14() {
	// reset_and_get_valhi14_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val14_hdr.valhi=0;
	valhi14_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}


@pragma stage 10
table update_valhi14_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi14;
		set_and_get_valhi14;
		reset_and_get_valhi14;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo15_reg;

action get_vallo15() {
	// get_vallo15_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo15_reg.read(hdr.val15_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo15() {
	// set_and_get_vallo15_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo15_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val15_hdr.vallo);
}
action reset_and_get_vallo15() {
	// reset_and_get_vallo15_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val15_hdr.vallo=0;
	vallo15_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 11
table update_vallo15_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo15;
		set_and_get_vallo15;
		reset_and_get_vallo15;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi15_reg;

action get_valhi15() {
	// get_valhi15_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi15_reg.read(hdr.val15_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi15() {
	// set_and_get_valhi15_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi15_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val15_hdr.valhi);
}
action reset_and_get_valhi15() {
	// reset_and_get_valhi15_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val15_hdr.valhi=0;
	valhi15_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}


@pragma stage 11
table update_valhi15_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi15;
		set_and_get_valhi15;
		reset_and_get_valhi15;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) vallo16_reg;

action get_vallo16() {
	// get_vallo16_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo16_reg.read(hdr.val16_hdr.vallo,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_vallo16() {
	// set_and_get_vallo16_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	vallo16_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val16_hdr.vallo);
}
action reset_and_get_vallo16() {
	// reset_and_get_vallo16_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val16_hdr.vallo=0;
	vallo16_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 11
table update_vallo16_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_vallo16;
		set_and_get_vallo16;
		reset_and_get_vallo16;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}

register<bit<32>>(KV_BUCKET_COUNT) valhi16_reg;

action get_valhi16() {
	// get_valhi16_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi16_reg.read(hdr.val16_hdr.valhi,(bit<32>)hdr.inswitch_hdr.idx);
}
action set_and_get_valhi16() {
	// set_and_get_valhi16_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	valhi16_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.val16_hdr.valhi);
}
action reset_and_get_valhi16() {
	// reset_and_get_valhi16_alu.execute_stateful_alu(hdr.inswitch_hdr.idx);
	hdr.val16_hdr.valhi=0;
	valhi16_reg.write((bit<32>)hdr.inswitch_hdr.idx,0);
}

@pragma stage 11
table update_valhi16_tbl {
	key = {
		//op_hdr.optype: exact;
		//inswitch_hdr.is_cached: exact;
		//meta.is_latest: exact;
		meta.meta.access_val_mode: exact;
	}
	actions = {
		get_valhi16;
		set_and_get_valhi16;
		reset_and_get_valhi16;
		NoAction;
	}
	default_action = NoAction();
	size = 32;
}
