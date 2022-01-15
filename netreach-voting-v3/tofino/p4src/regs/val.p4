register vallen_reg {
	width: 8;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallen_alu {
	reg: vallen_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: vallen_hdr.vallen;
}

action get_vallen() {
	get_vallen_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallen_alu {
	reg: vallen_reg;

	update_lo_1_value: vallen_hdr.vallen;

	output_value: register_lo;
	output_dst: vallen_hdr.vallen;
}

action set_and_get_vallen() {
	set_and_get_vallen_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu reset_and_get_vallen_alu {
	reg: vallen_reg;

	update_lo_1_value: 0;

	output_value: register_lo;
	output_dst: vallen_hdr.vallen;
}

action reset_and_get_vallen() {
	reset_and_get_vallen_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallen_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.iskeymatch: exact;
	}
	actions {
		get_vallen;
		set_and_get_vallen;
		reset_and_get_vallen;
		nop;
	}
	default_action: nop();
	size: 2560;
}

register vallo1_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo1_alu {
	reg: vallo1_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: val1_hdr.vallo;
}

action get_vallo1() {
	get_vallo1_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo1_alu {
	reg: vallo1_reg;

	update_lo_1_value: val1_hdr.vallo;

	output_value: register_lo;
	output_dst: val1_hdr.vallo;
}

action set_and_get_vallo1() {
	set_and_get_vallo1_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo1_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.iskeymatch: exact;
	}
	actions {
		get_vallo1;
		set_and_get_vallo1;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi1_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi1_alu {
	reg: valhi1_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: val1_hdr.valhi;
}

action get_valhi1() {
	get_valhi1_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi1_alu {
	reg: valhi1_reg;

	update_lo_1_value: val1_hdr.valhi;

	output_value: register_lo;
	output_dst: val1_hdr.valhi;
}

action set_and_get_valhi1() {
	set_and_get_valhi1_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi1_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.iskeymatch: exact;
	}
	actions {
		get_valhi1;
		set_and_get_valhi1;
		nop;
	}
	default_action: nop();
	size: 1152;
}

/*register vallo2_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo2_alu {
	reg: vallo2_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo2;
}

action get_vallo2() {
	get_vallo2_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo2_alu {
	reg: vallo2_reg;

	update_lo_1_value: val2_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo2;
}

action set_and_get_vallo2() {
	set_and_get_vallo2_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo2_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo2;
		set_and_get_vallo2;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi2_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi2_alu {
	reg: valhi2_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi2;
}

action get_valhi2() {
	get_valhi2_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi2_alu {
	reg: valhi2_reg;

	update_lo_1_value: val2_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi2;
}

action set_and_get_valhi2() {
	set_and_get_valhi2_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi2_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi2;
		set_and_get_valhi2;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo3_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo3_alu {
	reg: vallo3_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo3;
}

action get_vallo3() {
	get_vallo3_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo3_alu {
	reg: vallo3_reg;

	update_lo_1_value: val3_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo3;
}

action set_and_get_vallo3() {
	set_and_get_vallo3_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo3_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo3;
		set_and_get_vallo3;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi3_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi3_alu {
	reg: valhi3_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi3;
}

action get_valhi3() {
	get_valhi3_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi3_alu {
	reg: valhi3_reg;

	update_lo_1_value: val3_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi3;
}

action set_and_get_valhi3() {
	set_and_get_valhi3_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi3_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi3;
		set_and_get_valhi3;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo4_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo4_alu {
	reg: vallo4_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo4;
}

action get_vallo4() {
	get_vallo4_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo4_alu {
	reg: vallo4_reg;

	update_lo_1_value: val4_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo4;
}

action set_and_get_vallo4() {
	set_and_get_vallo4_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo4_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo4;
		set_and_get_vallo4;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi4_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi4_alu {
	reg: valhi4_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi4;
}

action get_valhi4() {
	get_valhi4_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi4_alu {
	reg: valhi4_reg;

	update_lo_1_value: val4_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi4;
}

action set_and_get_valhi4() {
	set_and_get_valhi4_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi4_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi4;
		set_and_get_valhi4;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo5_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo5_alu {
	reg: vallo5_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo5;
}

action get_vallo5() {
	get_vallo5_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo5_alu {
	reg: vallo5_reg;

	update_lo_1_value: val5_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo5;
}

action set_and_get_vallo5() {
	set_and_get_vallo5_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo5_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo5;
		set_and_get_vallo5;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi5_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi5_alu {
	reg: valhi5_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi5;
}

action get_valhi5() {
	get_valhi5_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi5_alu {
	reg: valhi5_reg;

	update_lo_1_value: val5_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi5;
}

action set_and_get_valhi5() {
	set_and_get_valhi5_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi5_tbl {
	reads {
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
		op_hdr.optype: exact;
	}
	actions {
		get_valhi5;
		set_and_get_valhi5;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo6_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo6_alu {
	reg: vallo6_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo6;
}

action get_vallo6() {
	get_vallo6_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo6_alu {
	reg: vallo6_reg;

	update_lo_1_value: val6_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo6;
}

action set_and_get_vallo6() {
	set_and_get_vallo6_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo6_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo6;
		set_and_get_vallo6;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi6_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi6_alu {
	reg: valhi6_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi6;
}

action get_valhi6() {
	get_valhi6_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi6_alu {
	reg: valhi6_reg;

	update_lo_1_value: val6_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi6;
}

action set_and_get_valhi6() {
	set_and_get_valhi6_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi6_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi6;
		set_and_get_valhi6;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo7_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo7_alu {
	reg: vallo7_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo7;
}

action get_vallo7() {
	get_vallo7_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo7_alu {
	reg: vallo7_reg;

	update_lo_1_value: val7_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo7;
}

action set_and_get_vallo7() {
	set_and_get_vallo7_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo7_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo7;
		set_and_get_vallo7;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi7_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi7_alu {
	reg: valhi7_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi7;
}

action get_valhi7() {
	get_valhi7_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi7_alu {
	reg: valhi7_reg;

	update_lo_1_value: val7_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi7;
}

action set_and_get_valhi7() {
	set_and_get_valhi7_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi7_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi7;
		set_and_get_valhi7;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo8_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo8_alu {
	reg: vallo8_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo8;
}

action get_vallo8() {
	get_vallo8_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo8_alu {
	reg: vallo8_reg;

	update_lo_1_value: val8_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo8;
}

action set_and_get_vallo8() {
	set_and_get_vallo8_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo8_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo8;
		set_and_get_vallo8;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi8_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi8_alu {
	reg: valhi8_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi8;
}

action get_valhi8() {
	get_valhi8_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi8_alu {
	reg: valhi8_reg;

	update_lo_1_value: val8_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi8;
}

action set_and_get_valhi8() {
	set_and_get_valhi8_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi8_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi8;
		set_and_get_valhi8;
		nop;
	}
	default_action: nop();
	size: 1152;
}

/*register vallo9_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo9_alu {
	reg: vallo9_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo9;
}

action get_vallo9() {
	get_vallo9_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo9_alu {
	reg: vallo9_reg;

	update_lo_1_value: val9_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo9;
}

action set_and_get_vallo9() {
	set_and_get_vallo9_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo9_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo9;
		set_and_get_vallo9;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi9_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi9_alu {
	reg: valhi9_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi9;
}

action get_valhi9() {
	get_valhi9_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi9_alu {
	reg: valhi9_reg;

	update_lo_1_value: val9_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi9;
}

action set_and_get_valhi9() {
	set_and_get_valhi9_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi9_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi9;
		set_and_get_valhi9;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo10_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo10_alu {
	reg: vallo10_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo10;
}

action get_vallo10() {
	get_vallo10_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo10_alu {
	reg: vallo10_reg;

	update_lo_1_value: val10_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo10;
}

action set_and_get_vallo10() {
	set_and_get_vallo10_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo10_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo10;
		set_and_get_vallo10;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi10_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi10_alu {
	reg: valhi10_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi10;
}

action get_valhi10() {
	get_valhi10_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi10_alu {
	reg: valhi10_reg;

	update_lo_1_value: val10_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi10;
}

action set_and_get_valhi10() {
	set_and_get_valhi10_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi10_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi10;
		set_and_get_valhi10;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo11_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo11_alu {
	reg: vallo11_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo11;
}

action get_vallo11() {
	get_vallo11_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo11_alu {
	reg: vallo11_reg;

	update_lo_1_value: val11_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo11;
}

action set_and_get_vallo11() {
	set_and_get_vallo11_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo11_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo11;
		set_and_get_vallo11;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi11_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi11_alu {
	reg: valhi11_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi11;
}

action get_valhi11() {
	get_valhi11_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi11_alu {
	reg: valhi11_reg;

	update_lo_1_value: val11_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi11;
}

action set_and_get_valhi11() {
	set_and_get_valhi11_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi11_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi11;
		set_and_get_valhi11;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo12_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo12_alu {
	reg: vallo12_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo12;
}

action get_vallo12() {
	get_vallo12_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo12_alu {
	reg: vallo12_reg;

	update_lo_1_value: val12_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo12;
}

action set_and_get_vallo12() {
	set_and_get_vallo12_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo12_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo12;
		set_and_get_vallo12;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi12_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi12_alu {
	reg: valhi12_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi12;
}

action get_valhi12() {
	get_valhi12_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi12_alu {
	reg: valhi12_reg;

	update_lo_1_value: val12_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi12;
}

action set_and_get_valhi12() {
	set_and_get_valhi12_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi12_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi12;
		set_and_get_valhi12;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo13_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo13_alu {
	reg: vallo13_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo13;
}

action get_vallo13() {
	get_vallo13_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo13_alu {
	reg: vallo13_reg;

	update_lo_1_value: val13_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo13;
}

action set_and_get_vallo13() {
	set_and_get_vallo13_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo13_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo13;
		set_and_get_vallo13;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi13_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi13_alu {
	reg: valhi13_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi13;
}

action get_valhi13() {
	get_valhi13_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi13_alu {
	reg: valhi13_reg;

	update_lo_1_value: val13_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi13;
}

action set_and_get_valhi13() {
	set_and_get_valhi13_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi13_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi13;
		set_and_get_valhi13;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo14_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo14_alu {
	reg: vallo14_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo14;
}

action get_vallo14() {
	get_vallo14_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo14_alu {
	reg: vallo14_reg;

	update_lo_1_value: val14_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo14;
}

action set_and_get_vallo14() {
	set_and_get_vallo14_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo14_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo14;
		set_and_get_vallo14;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi14_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi14_alu {
	reg: valhi14_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi14;
}

action get_valhi14() {
	get_valhi14_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi14_alu {
	reg: valhi14_reg;

	update_lo_1_value: val14_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi14;
}

action set_and_get_valhi14() {
	set_and_get_valhi14_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi14_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi14;
		set_and_get_valhi14;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo15_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo15_alu {
	reg: vallo15_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo15;
}

action get_vallo15() {
	get_vallo15_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo15_alu {
	reg: vallo15_reg;

	update_lo_1_value: val15_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo15;
}

action set_and_get_vallo15() {
	set_and_get_vallo15_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo15_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo15;
		set_and_get_vallo15;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi15_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi15_alu {
	reg: valhi15_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi15;
}

action get_valhi15() {
	get_valhi15_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi15_alu {
	reg: valhi15_reg;

	update_lo_1_value: val15_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi15;
}

action set_and_get_valhi15() {
	set_and_get_valhi15_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi15_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_valhi15;
		set_and_get_valhi15;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register vallo16_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo16_alu {
	reg: vallo16_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo16;
}

action get_vallo16() {
	get_vallo16_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_vallo16_alu {
	reg: vallo16_reg;

	update_lo_1_value: val16_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo16;
}

action set_and_get_vallo16() {
	set_and_get_vallo16_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_vallo16_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_vallo16;
		set_and_get_vallo16;
		nop;
	}
	default_action: nop();
	size: 1152;
}

register valhi16_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi16_alu {
	reg: valhi16_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi16;
}

action get_valhi16() {
	get_valhi16_alu.execute_stateful_alu(op_hdr.hashidx);
}

blackbox stateful_alu set_and_get_valhi16_alu {
	reg: valhi16_reg;

	update_lo_1_value: val16_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi16;
}

action set_and_get_valhi16() {
	set_and_get_valhi16_alu.execute_stateful_alu(op_hdr.hashidx);
}

table update_valhi16_tbl {
	reads {
		op_hdr.optype: exact;
		meta.canput: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
	}
	actions {
		get_valhi16;
		set_and_get_valhi16;
		nop;
	}
	default_action: nop();
	size: 1152;
}*/
