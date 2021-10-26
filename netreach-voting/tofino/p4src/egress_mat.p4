// NOTE: due to the hardware limitation of Tofino, we can transfer at most 32 bytes to cloned packet.
// So we cannot use the straightforward solution for GETRES_S, i.e., convert it to GETRES and clone a pkt
// for PUTREQ_S. Instead, we convert GETRES_S to PUTREQ_S to ensure that server has the correct data, and
// we clone a pkt for GETRES, which does not have correct key-value pair.
// We ignore it since it is just due to limitation of Tofino itself, which is irrevelant with our design.
action sendback_cloned_getres() {
	modify_field(udp_hdr.srcPort, meta.tmp_sport);
	modify_field(udp_hdr.dstPort, meta.tmp_dport);
	modify_field(op_hdr.optype, GETRES_TYPE);
}

table sendback_cloned_getres_tbl {
	actions {
		sendback_cloned_getres;
	}
	default_action: sendback_cloned_getres();
	size: 1;
}

action sendback_cloned_delres() {
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	modify_field(udp_hdr.dstPort, meta.tmp_sport);

	add_to_field(udp_hdr.hdrlen, 1);

	modify_field(op_hdr.optype, DELRES_TYPE);
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

table sendback_cloned_delres_tbl {
	actions {
		sendback_cloned_delres;
	}
	default_action: sendback_cloned_delres();
	size: 1;
}
