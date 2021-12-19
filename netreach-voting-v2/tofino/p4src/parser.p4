/* Parser */

parser start {
	return parse_ethernet;
}

parser parse_ethernet {
	extract(ethernet_hdr);
	return select(ethernet_hdr.etherType) {
		ETHERTYPE_IPV4: parse_ipv4;
		default: ingress;
	}
}

parser parse_ipv4 {
	extract(ipv4_hdr);
	return select(ipv4_hdr.protocol) {
		PROTOTYPE_UDP: parse_udp_dst;
		default: ingress;
	}
}

parser parse_udp_dst {
	extract(udp_hdr);
	return select(udp_hdr.dstPort) {
		OP_PORT: parse_op_req;
		default: parse_udp_src;
	}
}

parser parse_udp_src {
	return select(udp_hdr.srcPort) {
		OP_PORT: parse_op_rsp;
		default: ingress;
	}
}

parser parse_op_req {
	extract(op_hdr);
	return select(op_hdr.optype) {
		PUTREQ_TYPE: parse_vallen;
		PUTREQ_RU_TYPE: parse_vallen;
		PUTREQ_PS_TYPE: parse_vallen;
		PUTREQ_GS_TYPE: parse_vallen;
		PUTREQ_CASE1_TYPE: parse_vallen;
		DELREQ_CASE1_TYPE: parse_vallen;
		//PUTREQ_RU_CASE2_TYPE: parse_vallen;
		PUTREQ_PS_CASE2_TYPE: parse_vallen;
		PUTREQ_GS_CASE2_TYPE: parse_vallen;
		PUTREQ_CASE3_TYPE: parse_vallen;
		default: ingress;
	}
}

parser parse_op_rsp {
	extract(op_hdr);
	return select(op_hdr.optype) {
		GETRES_TYPE: parse_vallen;
		GETRES_NPOP_TYPE: parse_vallen;
		GETRES_LATEST_TYPE: parse_vallen;
		GETRES_NEXIST_TYPE: parse_vallen;
		//GETRES_S_CASE2_TYPE: parse_vallen;
		PUTRES_TYPE: parse_res;
		DELRES_TYPE: parse_res;
		default: ingress;
	}
}

parser parse_vallen {
	extract(vallen_hdr);
	return select(vallen_hdr.vallen) {
		1: parse_val_len1;
		2: parse_val_len2;
		3: parse_val_len3;
		4: parse_val_len4;
		5: parse_val_len5;
		6: parse_val_len6;
		7: parse_val_len7;
		8: parse_val_len8;
		/*9: parse_val_len9;
		10: parse_val_len10;
		11: parse_val_len11;
		12: parse_val_len12;
		13: parse_val_len13;
		14: parse_val_len14;
		15: parse_val_len15;
		16: parse_val_len16;*/
		default: ingress; // 0
	}
}

parser parse_val_len1 {
	extract(val1_hdr);
	return select(op_hdr.optype) {
		default: ingress;
	}
}

parser parse_val_len2 {
	extract(val2_hdr);
	return parse_val_len1;
}

parser parse_val_len3 {
	extract(val3_hdr);
	return parse_val_len2;
}

parser parse_val_len4 {
	extract(val4_hdr);
	return parse_val_len3;
}

parser parse_val_len5 {
	extract(val5_hdr);
	return parse_val_len4;
}

parser parse_val_len6 {
	extract(val6_hdr);
	return parse_val_len5;
}

parser parse_val_len7 {
	extract(val7_hdr);
	return parse_val_len6;
}

parser parse_val_len8 {
	extract(val8_hdr);
	return parse_val_len7;
}

/*parser parse_val_len9 {
	extract(val9_hdr);
	return parse_val_len8;
}

parser parse_val_len10 {
	extract(val10_hdr);
	return parse_val_len9;
}

parser parse_val_len11 {
	extract(val11_hdr);
	return parse_val_len10;
}

parser parse_val_len12 {
	extract(val12_hdr);
	return parse_val_len11;
}

parser parse_val_len13 {
	extract(val13_hdr);
	return parse_val_len12;
}

parser parse_val_len14 {
	extract(val14_hdr);
	return parse_val_len13;
}

parser parse_val_len15 {
	extract(val15_hdr);
	return parse_val_len14;
}

parser parse_val_len16 {
	extract(val16_hdr);
	return parse_val_len15;
}*/

parser parse_seq {
	extract(seq_hdr);
	return ingress;
}

parser parse_res {
	extract(res_hdr);
	return ingress;
}
