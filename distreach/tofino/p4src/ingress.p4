/* Ingress Processing */

control farreachIngress(  /* User */
    inout headers                       hdr,
    inout ingress_metadata                      meta,
    /* Intrinsic */
    in    ingress_intrinsic_metadata_t               ig_intr_md,
    in    ingress_intrinsic_metadata_from_parser_t   ig_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md,
    inout ingress_intrinsic_metadata_for_tm_t        ig_tm_md){
    #include"ingress_mat.p4"
    apply{
        // Stage 0
        if (!hdr.op_hdr.isValid()) {
            l2l3_forward_tbl.apply(); // forward traditional packet
        }else {
            
            set_hot_threshold_tbl.apply(); // set inswitch_hdr.hot_threshold

            // Stage 1

            // Stage 1
        #ifndef RANGE_SUPPORT
            hash_for_partition_tbl.apply(); // for hash partition (including startkey of SCANREQ)
        #endif

            // Stage 2 (not sure why we cannot place cache_lookup_tbl, hash_for_cm_tbl, and hash_for_seq_tbl in stage 1; follow automatic placement of tofino compiler)
            hash_partition_tbl.apply();
            // Stage 2
            hash_for_cm12_tbl.apply(); // for CM (access inswitch_hdr.hashval_for_cm1/2)
        //     // IMPORTANT: to save TCAM, we do not match op_hdr.optype in cache_lookup_tbl 
        //     // -> so as long as op_hdr.key matches an entry in cache_lookup_tbl, inswitch_hdr.is_cached must be 1 (e.g., CACHE_EVICT_LOADXXX)
        //     // -> but note that if the optype does not have inswitch_hdr, is_cached of 1 will be dropped after entering egress pipeline, and is_cached is still 0 (e.g., SCANREQ_SPLIT)
            // Stage 3
            cache_lookup_tbl.apply(); // managed by controller (access inswitch_hdr.is_cached, inswitch_hdr.idx)


            // Stage 4
            hash_for_seq_tbl.apply(); // for seq (access inswitch_hdr.hashval_for_seq)
            hash_for_cm34_tbl.apply(); // for CM (access inswitch_hdr.hashval_for_cm3/4)
            
        //     // Stage 10
            prepare_for_cachehit_tbl.apply(); // for response of cache hit (access inswitch_hdr.client_sid)

        //     // Stage 11
            sample_tbl.apply(); // for CM and cache_frequency (access inswitch_hdr.is_sampled)
            ig_port_forward_tbl.apply(); // update op_hdr.optype
            special_ig_port_forward_tbl.apply();
            ipv4_forward_tbl.apply(); // update egress_port for normal/speical response packets
        }
    }
}
