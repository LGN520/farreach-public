action get_backup(){
    cache_frequency_reg.read(hdr.backup_hdr.cache_frequency,(bit<32>)hdr.inswitch_hdr.idx);
    largevalueseq_reg.read(hdr.backup_hdr.largevalueseq,(bit<32>)hdr.inswitch_hdr.idx);
    deleted_reg.read(hdr.backup_hdr.is_deleted,(bit<32>)hdr.inswitch_hdr.idx);
    latest_reg.read(hdr.backup_hdr.is_latest,(bit<32>)hdr.inswitch_hdr.idx);
    validvalue_reg.read(hdr.backup_hdr.validvalue,(bit<32>)hdr.inswitch_hdr.idx);
    vallen_reg.read(hdr.backup_hdr.vallen,(bit<32>)hdr.inswitch_hdr.idx);
    hdr.vallen_hdr.vallen = hdr.backup_hdr.vallen;
    meta.access_val_mode = 1; //get val
    hdr.backup_hdr.is_found = 1; //founded it 
}
action recover(){
    cache_frequency_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.backup_hdr.cache_frequency);
    largevalueseq_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.backup_hdr.largevalueseq);
    deleted_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.backup_hdr.is_deleted);
    latest_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.backup_hdr.is_latest);
    validvalue_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.backup_hdr.validvalue);
    vallen_reg.write((bit<32>)hdr.inswitch_hdr.idx,hdr.backup_hdr.vallen);
    meta.access_val_mode = 2; //set val
    // hdr.backup_hdr.is_found = 1; //founded it 
}
// (is_cached,is_found)
// (0,0) -> (1,0) -> (1,1) found &get backup
// (1,1) -> (0,1) recover with backup
table recover_tbl {
	key = {
		hdr.op_hdr.optype: exact;
		hdr.inswitch_hdr.is_cached: exact;
        hdr.backup_hdr.is_found: exact;
	}
	actions = {
		get_backup;
        recover;
        NoAction;
	}
	default_action = NoAction();
	size = 12;
}

