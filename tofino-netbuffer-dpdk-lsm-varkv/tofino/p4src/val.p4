

register vallo_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_vallo_alu {
	reg: vallo_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_vallo;
}

action get_vallo() {
	get_vallo_alu.execute_stateful_alu(meta.hashidx);
}

/*table get_vallo_tbl {
	actions {
		get_vallo;
	}
	default_action: get_vallo();
}*/

blackbox stateful_alu put_vallo_alu {
	reg: vallo_reg;

	update_lo_1_value: putreq_hdr.vallo;

	output_value: register_lo;
	output_dst: meta.origin_vallo;
}

action put_vallo() {
	put_vallo_alu.execute_stateful_alu(meta.hashidx);
}

/*@pragma stage 3
table put_vallo_tbl {
	actions {
		put_vallo;
	}
	default_action: put_vallo();
}*/

//@pragma stage 3
table update_vallo_tbl {
	reads {
		op_hdr.optype: exact;
		meta.isvalid: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
	}
	actions {
		get_vallo;
		put_vallo;
		nop;
	}
	default_action: nop();
	size: 2048;
}

register valhi_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_valhi_alu {
	reg: valhi_reg;
	
	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.origin_valhi;
}

action get_valhi() {
	get_valhi_alu.execute_stateful_alu(meta.hashidx);
}

/*table get_valhi_tbl {
	actions {
		get_valhi;
	}
	default_action: get_valhi();
}*/

blackbox stateful_alu put_valhi_alu {
	reg: valhi_reg;

	update_lo_1_value: putreq_hdr.valhi;

	output_value: register_lo;
	output_dst: meta.origin_valhi;
}

action put_valhi() {
	put_valhi_alu.execute_stateful_alu(meta.hashidx);
}

/*@pragma stage 3
table put_valhi_tbl {
	actions {
		put_valhi;
	}
	default_action: put_valhi();
}*/

//@pragma stage 3
table update_valhi_tbl {
	reads {
		op_hdr.optype: exact;
		meta.isvalid: exact;
		meta.ismatch_keylolo: exact;
		meta.ismatch_keylohi: exact;
		meta.ismatch_keyhilo: exact;
		meta.ismatch_keyhihi: exact;
	}
	actions {
		get_valhi;
		put_valhi;
		nop;
	}
	default_action: nop();
	size: 2048;
}
