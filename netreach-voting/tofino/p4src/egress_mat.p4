// NOTE: due to the hardware limitation of Tofino, we can transfer at most 32 bytes to cloned packet.
// So we cannot use the straightforward solution for GETRES_S, i.e., convert it to GETRES and clone a pkt
// for PUTREQ_GS. Instead, we convert GETRES_S to PUTREQ_GS to ensure that server has the correct data, and
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

action sendback_cloned_putres_from_n() {
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	subtract_from_field(udp_hdr.hdrlen, VAL_PKTLEN_MINUS_ONE);

	remove_header(vallen_hdr);
	remove_header(val1_hdr);
	/*remove_header(val2_hdr);
	remove_header(val3_hdr);
	remove_header(val4_hdr);
	remove_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);*/

	modify_field(op_hdr.optype, PUTRES_TYPE);
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

action sendback_cloned_putres_from_ps() {
	modify_field(udp_hdr.srcPort, meta.tmp_dport);
	modify_field(udp_hdr.dstPort, meta.tmp_sport);
	subtract_from_field(udp_hdr.hdrlen, KEY_VAL_PKTLEN_MINUS_ONE);

	remove_header(vallen_hdr);
	remove_header(val1_hdr);
	/*remove_header(val2_hdr);
	remove_header(val3_hdr);
	remove_header(val4_hdr);
	remove_header(val5_hdr);
	remove_header(val6_hdr);
	remove_header(val7_hdr);
	remove_header(val8_hdr);
	remove_header(val9_hdr);
	remove_header(val10_hdr);
	remove_header(val11_hdr);
	remove_header(val12_hdr);
	remove_header(val13_hdr);
	remove_header(val14_hdr);
	remove_header(val15_hdr);
	remove_header(val16_hdr);*/
	remove_header(evicted_key_hdr);

	modify_field(op_hdr.optype, PUTRES_TYPE);
	modify_field(res_hdr.stat, 1);
	add_header(res_hdr);
}

table sendback_cloned_putres_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		sendback_cloned_putres_from_n;
		sendback_cloned_putres_from_ps;
		nop;
	}
	default_action: nop();
	size: 1;
}

action update_macaddr(tmp_srcmac, tmp_dstmac) {
	modify_field(ethernet_hdr.dstAddr, tmp_srcmac);
	modify_field(ethernet_hdr.srcAddr, tmp_dstmac);
}

table update_macaddr_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		update_macaddr;
		nop;
	}
	default_action: nop();
	size: 8;
}
