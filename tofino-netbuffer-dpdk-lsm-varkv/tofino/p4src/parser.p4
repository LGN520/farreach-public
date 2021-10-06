/* Parser */

parser start {
	return parse_udp;
}

parser parse_udp {
	extract(udp_hdr);
	return parse_op;
}

parser parse_op {
	extract(op_hdr);
	return select(op_hdr.optype) {
		PUTREQ_TYPE: parse_vallen;
		PUTREQ_S_TYPE: parse_vallen;
		GETRES_TYPE: parse_vallen;
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
		9: parse_val_len9;
		10: parse_val_len10;
		11: parse_val_len11;
		12: parse_val_len12;
		/*13: parse_val_len13;
		14: parse_val_len14;
		15: parse_val_len15;
		16: parse_val_len16;*/
		default: ingress; // 0
	}
}

parser parse_val_len1 {
	extract(val1_hdr);
	return ingress;
}

parser parse_val_len2 {
	extract(val1_hdr);
	extract(val2_hdr);
	return ingress;
}

parser parse_val_len3 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	return ingress;
}

parser parse_val_len4 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	return ingress;
}

parser parse_val_len5 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	extract(val5_hdr);
	return ingress;
}

parser parse_val_len6 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	extract(val5_hdr);
	extract(val6_hdr);
	return ingress;
}

parser parse_val_len7 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	extract(val5_hdr);
	extract(val6_hdr);
	extract(val7_hdr);
	return ingress;
}

parser parse_val_len8 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	extract(val5_hdr);
	extract(val6_hdr);
	extract(val7_hdr);
	extract(val8_hdr);
	return ingress;
}

parser parse_val_len9 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	extract(val5_hdr);
	extract(val6_hdr);
	extract(val7_hdr);
	extract(val8_hdr);
	extract(val9_hdr);
	return ingress;
}

parser parse_val_len10 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	extract(val5_hdr);
	extract(val6_hdr);
	extract(val7_hdr);
	extract(val8_hdr);
	extract(val9_hdr);
	extract(val10_hdr);
	return ingress;
}

parser parse_val_len11 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	extract(val5_hdr);
	extract(val6_hdr);
	extract(val7_hdr);
	extract(val8_hdr);
	extract(val9_hdr);
	extract(val10_hdr);
	extract(val11_hdr);
	return ingress;
}

parser parse_val_len12 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	extract(val5_hdr);
	extract(val6_hdr);
	extract(val7_hdr);
	extract(val8_hdr);
	extract(val9_hdr);
	extract(val10_hdr);
	extract(val11_hdr);
	extract(val12_hdr);
	return ingress;
}

/*parser parse_val_len13 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	extract(val5_hdr);
	extract(val6_hdr);
	extract(val7_hdr);
	extract(val8_hdr);
	extract(val9_hdr);
	extract(val10_hdr);
	extract(val11_hdr);
	extract(val12_hdr);
	extract(val13_hdr);
	return ingress;
}

parser parse_val_len14 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	extract(val5_hdr);
	extract(val6_hdr);
	extract(val7_hdr);
	extract(val8_hdr);
	extract(val9_hdr);
	extract(val10_hdr);
	extract(val11_hdr);
	extract(val12_hdr);
	extract(val13_hdr);
	extract(val14_hdr);
	return ingress;
}

parser parse_val_len15 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	extract(val5_hdr);
	extract(val6_hdr);
	extract(val7_hdr);
	extract(val8_hdr);
	extract(val9_hdr);
	extract(val10_hdr);
	extract(val11_hdr);
	extract(val12_hdr);
	extract(val13_hdr);
	extract(val14_hdr);
	extract(val15_hdr);
	return ingress;
}

/*parser parse_val_len16 {
	extract(val1_hdr);
	extract(val2_hdr);
	extract(val3_hdr);
	extract(val4_hdr);
	extract(val5_hdr);
	extract(val6_hdr);
	extract(val7_hdr);
	extract(val8_hdr);
	extract(val9_hdr);
	extract(val10_hdr);
	extract(val11_hdr);
	extract(val12_hdr);
	extract(val13_hdr);
	extract(val14_hdr);
	extract(val15_hdr);
	extract(val16_hdr);
	return ingress;
}*/

parser parse_res {
	extract(res_hdr);
	return ingress;
}