register<bit<32>>(MAX_SPINESWITCH_NUM) spineload_forclient_reg;
bit<32> spineload;
// Deprecated: for DISTCACHE_GETRES_SPINE/GETRES from spine switch 
// for DISTCACHE_UPDATE_TRAFFICLOAD from client
action set_spineload_forclient() {

	spineload_forclient_reg.read(spineload, (bit<32>)hdr.op_hdr.spineswitchidx);
	if(hdr.switchload_hdr.spineload != 0){
		spineload = hdr.switchload_hdr.spineload;
	}
	spineload_forclient_reg.write((bit<32>)hdr.op_hdr.spineswitchidx, spineload);
}

// for GETREQ from client
action get_spineload_forclient() {
	spineload_forclient_reg.read(meta.spineload_forclient, (bit<32>)hdr.op_hdr.spineswitchidx);
}

@pragma stage 0
table access_spineload_forclient_tbl {
	key = {
		hdr.op_hdr.optype: exact;
	}
	actions = {
		set_spineload_forclient;
		get_spineload_forclient;
		NoAction;
	}
	default_action= NoAction();
	size = 4;
}
