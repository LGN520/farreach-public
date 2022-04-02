register cache_frequency_reg {
	width: 32;
	instance_count: KV_BUCKET_COUNT;
}

blackbox stateful_alu update_cache_frequency_alu {
	reg: cache_frequency_reg;

	update_lo_1_value: register_lo + 1;
}

action update_cache_frequency() {
	update_cache_frequency_alu.execute_stateful_alu(inswitch_hdr.idx);
}

@pragma stage 1
table access_cache_frequency_tbl {
	reads {
		op_hdr.optype: exact;
		inswitch_hdr.is_sampled: exact;
		inswitch_hdr.is_cached: exact;
	}
	actions {
		update_cache_frequency;
		nop;
	}
	default_action: nop();
	size: 0;
}
