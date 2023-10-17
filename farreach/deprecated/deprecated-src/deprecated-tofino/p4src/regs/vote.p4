register gposvote_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_gposvote_alu {
	reg: gposvote_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.gposvote;
}

action get_gposvote() {
	get_gposvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu increase_gposvote_alu {
	reg: gposvote_reg;

	update_lo_1_value: register_lo + 1;

	output_value: alu_lo;
	output_dst: meta.gposvote;
}

action increase_gposvote() {
	increase_gposvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu reset_gposvote_alu {
	reg: gposvote_reg;

	update_lo_1_value: 0;

	output_value: alu_lo;
	output_dst: meta.gposvote;
}

action reset_gposvote() {
	reset_gposvote_alu.execute_stateful_alu(meta.hashidx);
}

table access_gposvote_tbl {
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
	default_action: get_gposvote();
	size: 2048;
}

register gnegvote_reg {
	width: 16;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu get_gnegvote_alu {
	reg: gnegvote_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.gnegvote;
}

action get_gnegvote() {
	get_gnegvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu increase_gnegvote_alu {
	reg: gnegvote_reg;

	update_lo_1_value: register_lo + 1;

	output_value: alu_lo;
	output_dst: meta.gnegvote;
}

action increase_gnegvote() {
	increase_gnegvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu reset_gnegvote_alu {
	reg: gnegvote_reg;

	update_lo_1_value: 0;

	output_value: alu_lo;
	output_dst: meta.gnegvote;
}

action reset_gnegvote() {
	reset_gnegvote_alu.execute_stateful_alu(meta.hashidx);
}

table access_gnegvote_tbl {
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

blackbox stateful_alu increase_pposvote_alu {
	reg: pposvote_reg;

	update_lo_1_value: register_lo + 1;

	output_value: alu_lo;
	output_dst: meta.pposvote;
}

blackbox stateful_alu get_pposvote_alu {
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

blackbox stateful_alu reset_pposvote_alu {
	reg: pposvote_reg;

	update_lo_1_value: 0;

	output_value: alu_lo;
	output_dst: meta.pposvote;
}

action reset_pposvote() {
	reset_pposvote_alu.execute_stateful_alu(meta.hashidx);
}

table access_pposvote_tbl {
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

blackbox stateful_alu get_pnegvote_alu {
	reg: pnegvote_reg;

	update_lo_1_value: register_lo;

	output_value: register_lo;
	output_dst: meta.pnegvote;
}

action get_pnegvote() {
	get_pnegvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu increase_pnegvote_alu {
	reg: pnegvote_reg;

	update_lo_1_value: register_lo + 1;

	output_value: alu_lo;
	output_dst: meta.pnegvote;
}

action increase_pnegvote() {
	increase_pnegvote_alu.execute_stateful_alu(meta.hashidx);
}

blackbox stateful_alu reset_pnegvote_alu {
	reg: pnegvote_reg;

	update_lo_1_value: 0;

	output_value: alu_lo;
	output_dst: meta.pnegvote;
}

action reset_pnegvote() {
	reset_pnegvote_alu.execute_stateful_alu(meta.hashidx);
}

table access_pnegvote_tbl {
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
