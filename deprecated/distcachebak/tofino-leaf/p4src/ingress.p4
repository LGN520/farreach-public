
/* Ingress Processing */

control distcacheleafIngress (inout headers hdr,
                 inout metadata meta,
                 inout standard_metadata_t standard_metadata){ 
    #include "regs/leafload.p4"
    #include "regs/spineload_forclient.p4"
    #include "regs/leafload_forclient.p4"
    #include "ingress_mat.p4"
    apply{
        // Stage 0
        if (! hdr.op_hdr.isValid()) {
            l2l3_forward_tbl.apply(); // forward traditional packet
        }
        hash_for_spineselect_tbl.apply(); // set meta.hashval_for_spineselect
        ////set_hot_threshold_tbl.apply(); // set hdr.inswitch_hdr.hot_threshold
        // For power-of-two-choices in client-leaf for GETREQ from client
        set_hot_threshold_and_spineswitchnum_tbl.apply(); // set hdr.inswitch_hdr.hot_threshold and meta.spineswitchnum
        // (1) access leafload_reg for power-of-two-choices by server-leaf
        // (2) for GETREQ_SPINE to increase leafload_reg, set hdr.inswitch_hdr.hashval_for_bf2
        access_leafload_tbl.apply(); 
        // NOTE: we CANNOT merge spine/leafload_forclient into one 64-bit register array, as we use different indexes (spine/leafswitchidx)
        access_spineload_forclient_tbl.apply(); // set meta.spineload_forclient

        // Stage 1
    //#ifndef RANGE_SUPPORT
    //	hash_for_partition_tbl.apply(); // for hash partition (including startkey of SCANREQ)
    //#endif
        hash_for_ecmp_and_partition_tbl.apply(); 
        // method B for incorrect spineswitchidx to set meta.hashval_for_ecmp; and set meta.hashval_for_partition if with hash partition
        // For power-of-two-choices in client-leaf for GETREQ from client
        ////set_spineswitchnum_tbl.apply(); // set meta.spineswitchnum for cutoff_spineswitchidx_for_ecmp_tbl (NOTE: merged into set_hot_threshold_tbl to save power budget)

        // Stage 2
        hash_for_cm12_tbl.apply(); // for CM (access hdr.inswitch_hdr.hashval_for_cm1)
        // For power-of-two-choices in client-leaf for GETREQ from client
        // NOTE: we CANNOT merge spine/leafload_forclient into one 64-bit register array, as we use different indexes (spine/leafswitchidx)
        access_leafload_forclient_tbl.apply(); // set meta.toleaf_predicate
        ecmp_for_getreq_tbl.apply(); // method B for incorrect spineswitchidx to set meta.toleaf_offset

        // Stage 3
        // NOTE: we cannot compare meta.spine/leafload_forclient by gateway table due to Tofino limitation (>/< can only be performed between one PHV and one constant, whose total bits <= 12 bits)
        spineselect_tbl.apply(); // change spineswitchidx and eport to forward requests from client to spine switch

        // Stage 4~5
        // For power-of-two-choices in client-leaf for GETREQ from client
        cutoff_spineswitchidx_for_ecmp_tbl.apply(); // cutoff spineswitchidx from [1, 2*spineswitchnum-2] -> [1, bit<16> spineswitchnum-1] & [0, bit<16> spineswitchnum-2]
        // IMPORTANT: to save TCAM, we do not match hdr.op_hdr.optype in cache_lookup_tbl 
        // -> so as long as hdr.op_hdr.key matches an entry in cache_lookup_tbl, hdr.inswitch_hdr.is_cached must be 1 (e.g., CACHE_EVICT_LOADXXX)
        // -> but note that if the optype does not have inswitch_hdr, is_cached of 1 will be dropped after entering egress pipeline, and is_cached is still 0 (e.g., SCANREQ_SPLIT)
        cache_lookup_tbl.apply(); // managed by controller (access hdr.inswitch_hdr.is_cached, hdr.inswitch_hdr.idx)

        // Stage 6~7 (not sure why we cannot place cache_lookup_tbl, hash_for_cm_tbl, and hash_for_seq_tbl in stage 1; follow automatic placement of tofino compiler)
        hash_for_cm34_tbl.apply(); // for CM (access hdr.inswitch_hdr.hashval_for_cm2)
        // NOTE: we reserve two stages for partition_tbl now as range matching needs sufficient TCAM

        hash_partition_tbl.apply();


        // Stage 8

        // Stage 9
        // (1) for response of cache hit (access hdr.inswitch_hdr.client_sid)
        // (2) set hdr.inswitch_hdr.hashval_for_bf1
        prepare_for_cachehit_and_hash_for_bf1_tbl.apply(); 
        ipv4_forward_tbl.apply(); // update egress_port for normal/speical response packets

        // Stage 10
        sample_tbl.apply(); // for CM and cache_frequency (access hdr.inswitch_hdr.is_sampled)

        // Stage 11
        // (1) update hdr.op_hdr.optype (update egress_port for distcacheleaf_VALUEUPDATE)
        // (2) for GETREQ_SPINE -> GETREQ_INSWITCH, set hdr.inswitch_hdr.hashval_for_bf3
        ig_port_forward_tbl.apply(); 
    }
}

