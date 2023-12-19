register<bit<32>>(MAX_SPINESWITCH_NUM) spineload_reg;
bit<32> spineload;
// for GETREQ_SPINE from spine switch
// NOTE: set AND GET for cache hit
action set_and_get_spineload() {
	spineload_reg.read(spineload,(bit<32>)hdr.op_hdr.spineswitchidx);
	spineload = spineload + 1;
	spineload_reg.write((bit<32>)hdr.op_hdr.spineswitchidx,spineload);
	hdr.switchload_hdr.spineload = spineload;
	hdr.switchload_hdr.leafload = 0;
}

// Deprecated: for GETRES_SERVER from storage server
// NOT used now
action get_spineload() {
	spineload_reg.read(hdr.switchload_hdr.spineload,(bit<32>)hdr.op_hdr.spineswitchidx);
}


@pragma stage 0
table access_spineload_tbl {
	key = {
		hdr.op_hdr.optype: exact;
	}
	actions = {
		set_and_get_spineload;
		get_spineload;
		NoAction;
	}
	default_action= NoAction();
	size = 2;
}
