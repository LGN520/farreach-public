/* Parser */

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_VLAN 0x8100
#define PROTOTYPE_TCP 0x06
#define PROTOTYPE_UDP 0x11

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
		PROTOTYPE_UDP: parse_udp;
		default: ingress;
	}
}

parser parse_udp {
	extract(udp_hdr);
	return parse_op;
}

parser parse_op {
	extract(op_hdr);
	return select(op_hdr.optype) {
		GETREQ_INSWITCH: parse_inswitch;
		GETRES: parse_vallen;
		GETRES_LATEST_SEQ: parse_vallen;
		GETRES_LATEST_SEQ_INSWITCH: parse_vallen;
		default: ingress; // GETREQ, GETREQ_POP, GETREQ_NLATEST





		PUTREQ_TYPE: parse_vallen;
		GETRES_TYPE: parse_vallen;
		PUTRES_TYPE: parse_res;
		DELRES_TYPE: parse_res;
		GETRES_POP_TYPE: parse_vallen;
		GETRES_NPOP_TYPE: parse_vallen;
		GETRES_POP_LARGE_TYPE: parse_vallen; // must enter parse_val_len0 (vallen + val payload)
		GETRES_POP_EVICT_TYPE: parse_vallen;
		PUTREQ_SEQ_TYPE: parse_vallen;
		PUTREQ_POP_TYPE: parse_vallen;
		PUTREQ_RECIR_TYPE: parse_vallen;
		PUTREQ_POP_EVICT_TYPE: parse_vallen;
		PUTREQ_LARGE_TYPE: parse_vallen; // must enter parse_val_len0 (vallen + val payload)
		PUTREQ_LARGE_SEQ_TYPE: parse_vallen; // must enter parse_val_len0 (vallen + seq + val payload)
		PUTREQ_LARGE_RECIR_TYPE: parse_vallen; // must enter parse_val_len0 (vallen + seq + val payload)
		PUTREQ_LARGE_EVICT:TYPE: parse_vallen; // vallen + val header + seq + val payload (ignored by udp hdrlen)
		DELREQ_RECIR_TYPE: parse_seq;
		DELREQ_SEQ_TYPE: parse_seq;
		PUTREQ_CASE1_TYPE: parse_vallen;
		DELREQ_CASE1_TYPE: parse_vallen;
		GETRES_POP_EVICT_CASE2_TYPE: parse_vallen;
		PUTREQ_POP_EVICT_CASE2_TYPE: parse_vallen;
		PUTREQ_LARGE_EVICT_CASE2_TYPE: parse_vallen;
		PUTREQ_CASE3_TYPE: parse_vallen;
		DELREQ_CASE3_TYPE: parse_seq;
		PUTREQ_LARGE_CASE3_TYPE: parse_vallen; // must enter parse_val_len0 (vallen + seq + val payload)
		PUTRES_CASE3_TYPE: parse_serveridx;
		DELRES_CASE3_TYPE: parse_serveridx;
		GETRES_POP_EVICT_SWITCH_TYPE: parse_vallen;
		GETRES_POP_EVICT_CASE2_SWITCH_TYPE: parse_vallen;
		PUTREQ_POP_EVICT_SWITCH_TYPE: parse_vallen;
		PUTREQ_POP_EVICT_CASE2_SWITCH_TYPE: parse_vallen;
		PUTREQ_LARGE_EVICT_SWITCH_TYPE: parse_vallen;
		PUTREQ_LARGE_EVICT_CASE2_SWITCH_TYPE: parse_vallen;
		default: ingress; // GETREQ
	}
}

parser parse_vallen {
	extract(vallen_hdr);
	return select(vallen_hdr.vallen) {
		0: parse_val_len0;
		0 mask 0xFFFFFFF8: parse_val_len1;
		8: parse_val_len1;
		8 mask 0xFFFFFFF8: parse_val_len2;
		16: parse_val_len2;
		16 mask 0xFFFFFFF8: parse_val_len3;
		24: parse_val_len3;
		24 mask 0xFFFFFFF8: parse_val_len4;
		32: parse_val_len4;
		32 mask 0xFFFFFFF8: parse_val_len5;
		40: parse_val_len5;
		40 mask 0xFFFFFFF8: parse_val_len6;
		48: parse_val_len6;
		48 mask 0xFFFFFFF8: parse_val_len7;
		56: parse_val_len7;
		56 mask 0xFFFFFFF8: parse_val_len8;
		64: parse_val_len8;
		64 mask 0xFFFFFFF8: parse_val_len9;
		72: parse_val_len9;
		72 mask 0xFFFFFFF8: parse_val_len10;
		80: parse_val_len10;
		80 mask 0xFFFFFFF8: parse_val_len11;
		88: parse_val_len11;
		88 mask 0xFFFFFFF8: parse_val_len12;
		96: parse_val_len12;
		96 mask 0xFFFFFFF8: parse_val_len13;
		104: parse_val_len13;
		104 mask 0xFFFFFFF8: parse_val_len14;
		112: parse_val_len14;
		112 mask 0xFFFFFFF8: parse_val_len15;
		120: parse_val_len15;
		120 mask 0xFFFFFFF8: parse_val_len16;
		128: parse_val_len16;
		default: parse_val_len0; // > 128
	}
}

parser parse_val_len0 {
	return select(op_hdr.optype) {
		GETRES_LATEST_SEQ: parse_seq;
		GETRES_LATEST_SEQ_INSWITCH: parse_seq;
		default: ingress; // GETRES


		GETRES_POP_TYPE: parse_seq;
		GETRES_POP_EVICT_TYPE: parse_seq;
		GETRES_POP_EVICT_CASE2_TYPE: parse_seq;
		PUTREQ_SEQ_TYPE: parse_seq;
		PUTREQ_POP_TYPE: parse_seq;
		PUTREQ_POP_EVICT_TYPE: parse_seq;
		PUTREQ_POP_EVICT_CASE2_TYPE: parse_seq;
		PUTREQ_RECIR_TYPE: parse_seq;
		PUTREQ_LARGE_SEQ_TYPE: parse_seq;
		PUTREQ_LARGE_RECIR_TYPE: parse_seq;
		PUTREQ_LARGE_EVICT_TYPE: parse_seq;
		PUTREQ_LARGE_EVICT_CASE2_TYPE: parse_seq; // no other_hdr
		PUTREQ_CASE3_TYPE: parse_seq;
		PUTREQ_LARGE_CASE3_TYPE: parse_seq;
		GETRES_POP_EVICT_SWITCH_TYPE: parse_seq;
		GETRES_POP_EVICT_CASE2_SWITCH_TYPE: parse_seq;
		PUTREQ_POP_EVICT_SWITCH_TYPE: parse_seq;
		PUTREQ_POP_EVICT_CASE2_SWITCH_TYPE: parse_seq;
		PUTREQ_LARGE_EVICT_SWITCH_TYPE: parse_seq;
		PUTREQ_LARGE_EVICT_CASE2_SWITCH_TYPE: parse_seq;
		default: ingress;
	}
}

parser parse_val_len1 {
	extract(val1_hdr);
	return paser_val_len0;
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

parser parse_val_len9 {
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
}

parser parse_seq {
	extract(seq_hdr);
	return select(op_hdr.optype) {
		GETRES_LATEST_SEQ_INSWITCH: parse_inswitch;
		default: ingress; // GETRES_LATEST_SEQ
		


		GETRES_POP_EVICT_CASE2_TYPE: parse_other;
		GETRES_POP_EVICT_CASE2_SWITCH_TYPE: parse_other;
		PUTREQ_POP_EVICT_CASE2_TYPE: parse_other;
		PUTREQ_POP_EVICT_CASE2_SWITCH_TYPE: parse_other;
	}
}

parser parse_result {
	extract(result_hdr);
	return ingress;
}

parser parse_inswitch {
	extract(inswitch_hdr);
	return ingress; // GETRES_LATEST_SEQ_INSWITCH
}

parser parse_other {
	extract(other_hdr);
	return ingress;
}
