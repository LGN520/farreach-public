
action update_dstport(port) {
	modify_field(udp_hdr.dstPort, port);
}

table hash_partition_tbl {
	reads {
		udp_hdr.dstPort: exact;
		eg_intr_md.egress_port: exact;
		op_hdr.hashidx: range;
	}
	actions {
		update_dstport;
		nop;
	}
	default_action: nop();
	size: 128;
}

action update_macaddr_s2c(tmp_srcmac, tmp_dstmac) {
	modify_field(ethernet_hdr.dstAddr, tmp_srcmac);
	modify_field(ethernet_hdr.srcAddr, tmp_dstmac);
}

action update_macaddr_c2s(tmp_srcmac, tmp_dstmac) {
	modify_field(ethernet_hdr.srcAddr, tmp_srcmac);
	modify_field(ethernet_hdr.dstAddr, tmp_dstmac);
}

table update_macaddr_tbl {
	reads {
		op_hdr.optype: exact;
	}
	actions {
		update_macaddr_s2c;
		update_macaddr_c2s;
		nop;
	}
	default_action: nop();
	size: 8;
}
