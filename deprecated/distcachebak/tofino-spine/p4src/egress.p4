/* Egress Processing */
control distcachespineEgress (inout headers hdr,
                 inout metadata meta,
                 inout standard_metadata_t standard_metadata){ 

    // registers and MATs

    #include "regs/cache_frequency.p4"
    #include "regs/latest.p4"
    #include "regs/deleted.p4"
    #include "regs/seq.p4"
    #include "regs/val.p4"
    #include "egress_mat.p4"
	apply{
		// [IMPORTANT]
		// Only prepare_for_cachepop_tbl will reset hdr.clone_hdr.server_sid as 0 by default, while process_scanreq_split_tbl only resets meta.remain_scannum by default (it ony modifies hdr.clone_hdr.server_sid for SCANREQ_SPLIT) -> MUST be very careful for all pkt types which will use hdr.clone_hdr.server_sid
		// For GETREQ_INSWITCH, hdr.clone_hdr.server_sid is NOT reset at process_scanreq_split_tbl, and is only set based on eport at prepare_for_cachepop_tbl -> OK
		// For SCANREQ_SPLIT, after setting server_sid based on split_hdr.globalserveridx at process_scanreq_split_tbl, it needs to invoke NoAction() explicitly in prepare_for_cachepop_tbl to avoid reset server_sid
		// For NETCACHE_GETREQ_POP, after inheriting hdr.clone_hdr.server_sid from GETREQ_INSWITCH, process_scanreq_split does NOT reset hdr.clone_hdr.server_sid by default, and it needs to invoke NoAction() explicitly in prepare_for_cachepop_tbl to avoid reset server_sid

		// Stage 0
		access_latest_tbl.apply(); // NOTE: latest_reg corresponds to stats.validity in netcache paper, which will be used to *invalidate* the value by PUT/DELREQ
		access_seq_tbl.apply();
		save_client_info_tbl.apply(); // save srcip/srcmac/udp.dstport (client ip/mac/udpport) for cache hit response of GET/PUT/DELREQ_INSWITCH

		// Stage 1
		prepare_for_cachepop_tbl.apply(); // reset hdr.clone_hdr.server_sid by default here

		// Stage 2
		access_cache_frequency_tbl.apply();
		access_deleted_tbl.apply();
		access_savedseq_tbl.apply();

		// Stage 3
		update_vallen_tbl.apply();

		// Stage 4
		// NOTE: value registers do not reply on hdr.op_hdr.optype, they only rely on meta.access_val_mode, which is set by update_vallen_tbl in stage 3
		update_vallo1_tbl.apply();
		update_valhi1_tbl.apply();
		update_vallo2_tbl.apply();
		update_valhi2_tbl.apply();

		// Stage 5
		update_vallo3_tbl.apply();
		update_valhi3_tbl.apply();
		update_vallo4_tbl.apply();
		update_valhi4_tbl.apply();

		// Stage 6
		update_vallo5_tbl.apply();
		update_valhi5_tbl.apply();
		update_vallo6_tbl.apply();
		update_valhi6_tbl.apply();

		// Stage 7
		is_report_tbl.apply(); // NOTE: place is_report_tbl here due to tricky Tofino MAT placement limitation -> not sure the reason
		update_vallo7_tbl.apply();
		update_valhi7_tbl.apply();
		update_vallo8_tbl.apply();
		update_valhi8_tbl.apply();

		// Stage 8
		lastclone_lastscansplit_tbl.apply(); // including is_last_scansplit
		update_vallo9_tbl.apply();
		update_valhi9_tbl.apply();
		update_vallo10_tbl.apply();
		update_valhi10_tbl.apply();

		// Stage 9
		eg_port_forward_tbl.apply(); // including scan forwarding
		// NOTE: Comment val11 and val12 in debug mode to save resources for eg_port_forward_counter -> you need to disable debug mode in evaluation
	#ifndef DEBUG
		update_vallo11_tbl.apply();
		update_valhi11_tbl.apply();
		update_vallo12_tbl.apply();
		update_valhi12_tbl.apply();
	#endif

		// stage 10
		// NOTE: resource in stage 11 is not enough for update_ipmac_src_port_tbl, so we place it into stage 10
		update_ipmac_srcport_tbl.apply(); // Update ip, mac, and srcport for RES to client and notification to switchos
		update_vallo13_tbl.apply();
		update_valhi13_tbl.apply();
		update_vallo14_tbl.apply();
		update_valhi14_tbl.apply();

		// Stage 11
		update_pktlen_tbl.apply(); // Update udl_hdr.hdrLen for pkt with variable-length value
		add_and_remove_value_header_tbl.apply(); // Add or remove vallen and val according to optype and vallen
		update_vallo15_tbl.apply();
		update_valhi15_tbl.apply();
		update_vallo16_tbl.apply();
		update_valhi16_tbl.apply();
	}
}
