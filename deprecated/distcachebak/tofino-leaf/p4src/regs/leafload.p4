register<bit<32>>(MAX_LEAFSWITCH_NUM) leafload_reg;
bit<32> leafload;
// for GETREQ_SPINE from spine switch
// NOTE: set AND GET for cache hit
action set_and_get_leafload_and_hash_for_bf2() {
	leafload_reg.read(leafload,(bit<32>)hdr.op_hdr.leafswitchidx);
	leafload = leafload + 1;
	leafload_reg.write((bit<32>)hdr.op_hdr.leafswitchidx,leafload);
	hdr.switchload_hdr.leafload = leafload;

	// NOTE: sum of hash results' bits CANNOT > 32-bits in one ALU due to Tofino limitation (18-bit hashval_for_bf = 32-bit cost)
	// NOTE: cannot pass action parameter into modify_field_with_hash_based_offset, which only accepts constant
	hash(leafload, HashAlgorithm.crc32_custom, (bit<32>)0, {
		hdr.op_hdr.keylolo,
		hdr.op_hdr.keylohi,
		hdr.op_hdr.keyhilo,
		hdr.op_hdr.keyhihilo,
		hdr.op_hdr.keyhihihi
	}, (bit<32>)BF_BUCKET_COUNT);
	hdr.inswitch_hdr.hashval_for_bf2 = (bit<18>)leafload;
}

// Deprecated: for GETRES_SERVER from storage server
// NOT used now
action get_leafload() {
	leafload_reg.read(hdr.switchload_hdr.leafload,(bit<32>)hdr.op_hdr.leafswitchidx);
}


@pragma stage 0
table access_leafload_tbl {
	key = {
		hdr.op_hdr.optype: exact;
	}
	actions = {
		set_and_get_leafload_and_hash_for_bf2;
		get_leafload;
		NoAction;
	}
	default_action= NoAction();
	size = 2;
}
