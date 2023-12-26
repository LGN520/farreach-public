
@pragma stage 1
table debug_tbl {
	key = {
        hdr.op_hdr.optype:exact;
		hdr.validvalue_hdr.validvalue:exact;
        hdr.vallen_hdr.vallen:exact;
	}
	actions = {
		NoAction;
	}
	default_action = NoAction();
	size = 1;
}

@pragma stage 1
table debugend_tbl {
	key = {
        hdr.op_hdr.optype:exact;
		hdr.validvalue_hdr.validvalue:exact;
        hdr.vallen_hdr.vallen:exact;
	}
	actions = {
		NoAction;
	}
	default_action = NoAction();
	size = 1;
}