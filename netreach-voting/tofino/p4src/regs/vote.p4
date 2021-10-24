register gposvote_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox get_gposvote_alu {
	reg: gposvote_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.gposvote;
}

action get_gposvote() {
	get_gposvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox increase_gposvote_alu {
	reg: gposvote_reg;

	update_lo_1_value: register_lo + 1;

	output_value: alu_lo;
	output_dst: meta.gposvote;
}

action increase_gposvote() {
	increase_gposvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox reset_gposvote_alu {
	reg: gposvote_reg;

	update_lo_1_value: 0;

	output_value: alu_lo;
	output_dst: meta.gposvote;
}

table access_gposvote_alu {
	reads {
		meta.isvalid: exact;
		op_hdr.optype: exact;
		meta.ismatch_keylololo: exact;
		meta.ismatch_keylolohi: exact;
		meta.ismatch_keylohilo: exact;
		meta.ismatch_keylohihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_gposvote;
		increase_gposvote;
		reset_gposvote;
	}
	default_action = get_gposvote();
	size: 2048;
}

register gnegvote_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox get_gnegvote_alu {
	reg: gnegvote_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.gnegvote;
}

action get_gnegvote() {
	get_gnegvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox increase_gnegvote_alu {
	reg: gnegvote_reg;

	update_lo_1_value: register_lo + 1;

	output_value: alu_lo;
	output_dst: meta.gnegvote;
}

action increase_gnegvote() {
	increase_gnegvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox reset_gnegvote_alu {
	reg: gnegvote_reg;

	update_lo_1_value: 0;

	output_value: alu_lo;
	output_dst: meta.gnegvote;
}

table access_gnegvote_alu {
	reads {
		meta.isvalid: exact;
		op_hdr.optype: exact;
		meta.ismatch_keylololo: exact;
		meta.ismatch_keylolohi: exact;
		meta.ismatch_keylohilo: exact;
		meta.ismatch_keylohihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_gnegvote;
		increase_gnegvote;
		reset_gnegvote;
	}
	default_action: get_gnegvote();
	size: 2048;
}

register pposvote_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox increase_pposvote_alu {
	reg: pposvote_reg;

	update_lo_1_value: register_lo + 1;

	output_value: alu_lo;
	output_dst: meta.pposvote;
}

blackbox get_pposvote_alu {
	reg: pposvote_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.pposvote;
}

action get_pposvote() {
	get_pposvote_alu.execute_stateful_alu(meta.hashidx);
}

action increase_pposvote() {
	increase_pposvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox reset_pposvote_alu {
	reg: pposvote_reg;

	update_lo_1_value: 0;

	output_value: alu_lo;
	output_dst: meta.pposvote;
}

table access_pposvote_alu {
	reads {
		meta.isvalid: exact;
		op_hdr.optype: exact;
		meta.ismatch_keylololo: exact;
		meta.ismatch_keylolohi: exact;
		meta.ismatch_keylohilo: exact;
		meta.ismatch_keylohihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_pposvote;
		increase_pposvote;
		reset_pposvote;
	}
	default_action: get_pposvote();
	size: 2048;
}

register pnegvote_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox get_pnegvote_alu {
	reg: pnegvote_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.pnegvote;
}

action get_pnegvote() {
	get_pnegvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox increase_pnegvote_alu {
	reg: pnegvote_reg;

	update_lo_1_value: register_lo + 1;

	output_value: alu_lo;
	output_dst: meta.pnegvote;
}

action increase_pnegvote() {
	increase_pnegvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox reset_pnegvote_alu {
	reg: pnegvote_reg;

	update_lo_1_value: 0;

	output_value: alu_lo;
	output_dst: meta.pnegvote;
}

table access_pnegvote_alu {
	reads {
		meta.isvalid: exact;
		op_hdr.optype: exact;
		meta.ismatch_keylololo: exact;
		meta.ismatch_keylolohi: exact;
		meta.ismatch_keylohilo: exact;
		meta.ismatch_keylohihi: exact;
		meta.ismatch_keyhilolo: exact;
		meta.ismatch_keyhilohi: exact;
		meta.ismatch_keyhihilo: exact;
		meta.ismatch_keyhihihi: exact;
	}
	actions {
		get_pnegvote;
		increase_pnegvote;
		reset_pnegvote;
	}
	default_action: get_pnegvote();
	size: 2048;
}
