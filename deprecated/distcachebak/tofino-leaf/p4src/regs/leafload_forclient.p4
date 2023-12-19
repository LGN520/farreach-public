register<bit<32>>(MAX_LEAFSWITCH_NUM) leafload_forclient_reg;
// Deprecated: for GETRES from spine switch 
// for DISTCACHE_UPDATE_TRAFFICLOAD from client
action set_leafload_forclient() {
	leafload_forclient_reg.read(leafload, (bit<32>)hdr.op_hdr.leafswitchidx);
	if(hdr.switchload_hdr.leafload != 0){
		leafload = hdr.switchload_hdr.leafload;
	}
	leafload_forclient_reg.write((bit<32>)hdr.op_hdr.leafswitchidx, leafload);
	meta.toleaf_predicate = 1;
}


// for GETREQ from client
action get_leafload_forclient() {
	leafload_forclient_reg.read(leafload, (bit<32>)hdr.op_hdr.leafswitchidx);
	if(meta.spineload_forclient >= leafload){
		meta.toleaf_predicate = 2;
	}else{
		meta.toleaf_predicate = 1;
	}
}

// for optypes except GETRES/DISTCACHE_GETRES_SPINE/GETREQ
action reset_meta_toleaf_predicate() {
	meta.toleaf_predicate = 1;
}

@pragma stage 2
table access_leafload_forclient_tbl {
	key = {
		hdr.op_hdr.optype: exact;
	}
	actions = {
		set_leafload_forclient;
		get_leafload_forclient;
		reset_meta_toleaf_predicate;
	}
	default_action= reset_meta_toleaf_predicate();
	size = 4;
}
