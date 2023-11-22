register<bit<32>>(MAX_SPINESWITCH_NUM) spineload_reg;
register<bit<32>>(MAX_SPINESWITCH_NUM) leafload_reg;
bit<32> spineload;
bit<32> leafload;
// for GETREQ_SPINE from spine switch
// NOTE: set AND GET for cache hit

// Deprecated: for GETRES_SERVER from storage server
// NOT used now
// action get_spineload() {
// 	spineload_reg.read(hdr.switchload_hdr.spineload,(bit<32>)meta.spineswitchidx);
// }
action poweroftwochoice(){
    spineload_reg.read(spineload,(bit<32>)meta.spineswitchidx);
    leafload_reg.read(leafload,(bit<32>)meta.leafswitchidx);
    if(leafload > spineload){
        spineload = spineload + 1;
        meta.is_spine = 1;
    }else{
        leafload = leafload + 1;
        meta.is_spine = 0;
    }
    spineload_reg.write((bit<32>)meta.spineswitchidx,spineload);
    leafload_reg.write((bit<32>)meta.leafswitchidx,leafload);
}

table poweroftwochoice_tbl {
	key = {
		hdr.op_hdr.optype: exact;
	}
	actions = {
		poweroftwochoice;
		NoAction;
	}
	default_action= NoAction();
	size = 2;
}
