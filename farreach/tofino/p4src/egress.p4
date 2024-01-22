control farreachEgress(    /* User */
    inout headers                          hdr,
    inout egress_metadata                         meta,
    /* Intrinsic */    
    in    egress_intrinsic_metadata_t                  eg_intr_md,
    in    egress_intrinsic_metadata_from_parser_t      eg_prsr_md,
    inout egress_intrinsic_metadata_for_deparser_t     eg_dprsr_md,
    inout egress_intrinsic_metadata_for_output_port_t  eg_oport_md){ 
    #include"egress_mat.p4"
    // registers and MATs
    #include "regs/cm.p4"
    #include "regs/cache_frequency.p4"
    #include "regs/validvalue.p4"
    #include "regs/latest.p4"
    #include "regs/deleted.p4"
    #include "regs/seq.p4"
    #include "regs/val.p4"
    #include "regs/case1.p4"

    #ifdef ENABLE_LARGEVALUEBLOCK
        #include "regs/largevalueseq.p4"
    #endif
     /* Egress Processing */

    apply {

        // Stage 0
        access_cm1_tbl.apply();
        access_cm2_tbl.apply();
        access_cm3_tbl.apply();
        access_cm4_tbl.apply();

        // Stage 1
        is_hot_tbl.apply();
        access_cache_frequency_tbl.apply();
        access_validvalue_tbl.apply();
        access_seq_tbl.apply();

        // Stage 2
        access_latest_tbl.apply();
    #ifdef ENABLE_LARGEVALUEBLOCK
        // save seq_hdr.seq into clone_hdr.assignedseq_for_farreach for PUT/DELREQ_INSWITCH, which is used by cache hit response of PUT/DELREQ_INSWITCH and PUT/DELREQ_SEQ_INSWITCH_CASE1 -> [IMPORTANT] must be placed between access_seq_tbl and access_savedseq_tbl
        access_largevalueseq_and_save_assignedseq_tbl.apply(); // used for invalidation of PUTREQ_LARGEVALUE
    #endif
        // save udp.dstport (client port) for GET/PUT/DELREQ_INSWITCH, which is used by cache hit response of GET/PUT/DELREQ_INSWITCH and PUTREQ/DELREQ_SEQ_INSWITCH_CASE1
        save_client_udpport_tbl.apply();

        // Stage 3
    #ifdef ENABLE_LARGEVALUEBLOCK
        if (meta.largevalueseq != 0) {
            is_largevalueblock_tbl.apply(); // used for invalidation of PUTREQ_LARGEVALUE
        }
    #endif
        access_deleted_tbl.apply();
        update_vallen_tbl.apply();
        access_savedseq_tbl.apply();
        access_case1_tbl.apply();

        // Stage 4-6
        // NOTE: value registers do not reply on op_hdr.optype, they only rely on meta.access_val_mode, which is set by update_vallen_tbl in stage 3
        update_vallo1_tbl.apply();
        update_valhi1_tbl.apply();
        update_vallo2_tbl.apply();
        update_valhi2_tbl.apply();

        update_vallo3_tbl.apply();
        update_valhi3_tbl.apply();
        update_vallo4_tbl.apply();
        update_valhi4_tbl.apply();

        update_vallo5_tbl.apply();
        update_valhi5_tbl.apply();
        update_vallo6_tbl.apply();
        update_valhi6_tbl.apply();

        // Stage 7
        lastclone_lastscansplit_tbl.apply(); // including is_last_scansplit
        update_vallo7_tbl.apply();
        update_valhi7_tbl.apply();
        update_vallo8_tbl.apply();
        update_valhi8_tbl.apply();

        // Stage 8
        // NOTE: we must guarantee that the output optype of another_eg_port_forward_tbl will NOT be matched in eg_port_forward_tbl
        another_eg_port_forward_tbl.apply(); // used to reduce VLIW usage of eg_port_forward_tbl
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
        update_pktlen_tbl.apply(); // Update udl_hdr.en for pkt with variable-length value
        add_and_remove_value_header_tbl.apply(); // Add or remove vallen and val according to optype and vallen
        drop_tbl.apply(); // drop GETRES_LATEST_SEQ_INSWITCH and GETRES_DELETED_SEQ_INSWITCH
        update_vallo15_tbl.apply();
        update_valhi15_tbl.apply();
        update_vallo16_tbl.apply();
        update_valhi16_tbl.apply();
        if (hdr.op_hdr.isValid())
        {
            hdr.udp_hdr.checksum = 0;
        }
    }              
}