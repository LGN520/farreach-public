register case3_reg {
	width: 1;
	instance_count: 1;
}

blackbox stateful_alu read_case3_alu {
	reg: case3_reg;

	update_lo_1_value: read_bit;

	output_value: alu_lo;
	output_dst: meta.iscase3;
}

blackbox stateful_alu try_case3_alu {
	reg: case3_reg;

	update_lo_1_value: set_bit;

	output_value: alu_lo;
	output_dst: meta.iscase3;
}
