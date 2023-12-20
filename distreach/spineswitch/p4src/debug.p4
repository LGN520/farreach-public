
@pragma stage 1
table debug_tbl {
	key = {
        hdr.op_hdr.optype:exact;
		hdr.inswitch_hdr.idx:exact;
        hdr.vallen_hdr.vallen:exact;
	}
	actions = {
		NoAction;
	}
	default_action = NoAction();
	size = 1;
}