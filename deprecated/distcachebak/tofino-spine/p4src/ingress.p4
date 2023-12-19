
/* Ingress Processing */
control distcachespineIngress(inout headers hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata){
    #include "regs/spineload.p4"
    #include "ingress_mat.p4"
	// Stage 0
    apply {
        if (! hdr.op_hdr.isValid()) {
            l2l3_forward_tbl.apply(); // forward traditional packet
        }else{
            set_hot_threshold_tbl.apply(); // set hdr.inswitch_hdr.hot_threshold
            access_spineload_tbl.apply(); // access spineload_reg for power-of-two-choices

            // Stage 1~2
        #ifndef RANGE_SUPPORT
            hash_for_partition_tbl.apply(); // for hash partition (including startkey of SCANREQ)
        #endif
            // IMPORTANT: to save TCAM, we do not match hdr.op_hdr.optype in cache_lookup_tbl 
            // -> so as long as hdr.op_hdr.key matches an entry in cache_lookup_tbl, hdr.inswitch_hdr.is_cached must be 1 (e.g., CACHE_EVICT_LOADXXX)
            // -> but note that if the optype does not have inswitch_hdr, is_cached of 1 will be dropped after entering egress pipeline, and is_cached is still 0 (e.g., SCANREQ_SPLIT)
            cache_lookup_tbl.apply(); // managed by controller (access hdr.inswitch_hdr.is_cached, hdr.inswitch_hdr.idx)

            // Stage 3
            hash_for_seq_tbl.apply(); // for seq (access hdr.inswitch_hdr.hashval_for_seq)

            // Stage 4~5 (not sure why we cannot place cache_lookup_tbl, hash_for_cm_tbl, and hash_for_seq_tbl in stage 1; follow automatic placement of tofino compiler)
            // NOTE: we reserve two stages for partition_tbl now as range matching needs sufficient TCAM
            // NOTE: change hdr.op_hdr.leafswitchidx as leafswitchidx

            hash_partition_tbl.apply();

            // Stage 6

            // Stage 7
            prepare_for_cachehit_tbl.apply(); // for response of cache hit (access hdr.inswitch_hdr.client_sid)
            ipv4_forward_tbl.apply(); // update egress_port for normal/speical response packets

            // Stage 8
            sample_tbl.apply(); // for CM and cache_frequency (access hdr.inswitch_hdr.is_sampled)
            ig_port_forward_tbl.apply(); // update hdr.op_hdr.optype (update egress_port for NETCACHE_VALUEUPDATE)
        }
    }
}
