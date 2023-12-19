from typing import Any
from p4utils.utils.sswitch_thrift_API import SimpleSwitchThriftAPI
import logging
import os
import random
import sys
import time
import unittest
import re
import p4runtime_lib

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *

cached_list = [0, 1]
hot_list = [0, 1]
report_list = [0, 1]
latest_list = [0, 1]
stat_list = [0, 1]
deleted_list = [0, 1]
sampled_list = [0, 1]
lastclone_list = [0, 1]
access_val_mode_list = [0, 1, 2, 3]

reflector_ip_for_switchos = spine_reflector_ip_for_switchos
reflector_dp2cpserver_port = spine_reflector_dp2cpserver_port
reflector_cp2dpserver_port = spine_reflector_cp2dpserver_port
reflector_cp2dp_dstip = spine_reflector_cp2dp_dstip
reflector_cp2dp_dstmac = spine_reflector_cp2dp_dstmac


class TableConfigure:
    def __init__(self, rack_idx):
        self.rack_idx = rack_idx
        self.controller = SimpleSwitchThriftAPI(
            9090 + rack_idx + 1, "192.168.122.229"
        )  # 9090，127.0.0.1

    def configure_update_val_tbl(self, valname):
        for access_val_mode in access_val_mode_list:
            matchspec0 = [hex(access_val_mode)]
            # NOTE: not access val_reg if access_val_mode == 0
            if access_val_mode == 1:
                self.controller.table_add(
                    "update_val{}_tbl".format(valname),
                    "get_val{}".format(valname),
                    matchspec0,
                    [],
                )
            elif access_val_mode == 2:
                self.controller.table_add(
                    "update_val{}_tbl".format(valname),
                    "set_and_get_val{}".format(valname),
                    matchspec0,
                    [],
                )
            elif access_val_mode == 3:
                self.controller.table_add(
                    "update_val{}_tbl".format(valname),
                    "reset_and_get_val{}".format(valname),
                    matchspec0,
                    [],
                )

    def setUp(self):
        print("\nSetup")

        # get the device ports from front panel ports
        port, chnl = spineswitch_fpport_to_leaf.split("/")
        self.clientleafswitch_devport = port
        self.serverleafswitch_devport = port
        port, chnl = spine_reflector_fpport_for_switch.split("/")
        self.reflector_devport = port

        # NOTE: in each pipeline, 64-67 are recir/cpu ports, 68-71 are recir/pktgen ports
        # self.cpuPorts = [64, 192] # CPU port is 100G

        # sidnum = len(self.client_devports) + len(self.server_devports)
        # sids = random.sample(xrange(BASE_SID_NORM, MAX_SID_NORM), sidnum)
        # self.client_sids = sids[0:len(self.client_devports)]
        # self.server_sids = sids[len(self.client_devports):sidnum]
        sidnum = 2
        sids = [41, 42]
        self.clientleafswitch_sid = sids[0]
        self.serverleafswitch_sid = sids[0]
        self.reflector_sid = sids[1]

        # NOTE: data plane communicate with switchos by software-based reflector, which is deployed in one server machine
        self.reflector_ip_for_switch = spine_reflector_ip_for_switch
        self.reflector_mac_for_switch = spine_reflector_mac_for_switch

    ### MAIN ###

    def runTest(self):
        #####################
        ### Prepare ports ###
        #####################

        # Bind sid with platform port for packet mirror
        # NOTE: clientleafswitch.sid/devport = serverleafswitch.sid/devport as we only have one physical leaf switch to simulate both client-/server-leaf switch
        print(
            "Binding sid {} with leafswitch devport {} for both direction mirroring".format(
                self.clientleafswitch_sid, self.clientleafswitch_devport
            )
        )  # clone to leafswitch
        self.controller.mirroring_add(
            self.clientleafswitch_sid,
            self.clientleafswitch_devport,
        )
        print(
            "Binding sid {} with reflector devport {} for both direction mirroring".format(
                self.reflector_sid, self.reflector_devport
            )
        )  # clone to reflector
        self.controller.mirroring_add(self.reflector_sid, self.reflector_devport)
        ################################
        ### Normal MAT Configuration ###
        ################################

        # Ingress pipeline

        # Stage 0

        # Table: l2l3_forward_tbl (default: NoAction; size: client_physical_num+server_physical_num = 4 < 16)
        print("Configuring l2l3_forward_tbl")
        for i in range(client_physical_num):
            actnspec0 = [self.clientleafswitch_devport]
            self.controller.table_add(
                "l2l3_forward_tbl",
                "l2l3_forward",
                [client_macs[i], client_ips[i] + "/32"],
                actnspec0,
            )
        for i in range(server_physical_num):
            actnspec0 = [self.serverleafswitch_devport]
            self.controller.table_add(
                "l2l3_forward_tbl",
                "l2l3_forward",
                [server_macs[i], server_ips[i] + "/32"],
                actnspec0,
            )
        # Table: set_hot_threshold_tbl (default: set_hot_threshold; size: 1)
        print("Configuring set_hot_threshold_tbl")
        actnspec0 = [hot_threshold]
        self.controller.table_set_default(
            "set_hot_threshold_tbl", "set_hot_threshold", [hex(hot_threshold)]
        )
        # Table: access_spineload_tbl (default: NoAction; size: 1)
        print("Configuring access_spineload_tbl")
        matchspec0 = [hex(GETREQ)]  # GETREQ from client-leaf
        self.controller.table_add(
            "access_spineload_tbl", "set_and_get_spineload", matchspec0
        )

        # Stage 1

        if RANGE_SUPPORT == False:
            # Table: hash_for_partition_tbl (default: NoAction; size: 11)
            print("Configuring hash_for_partition_tbl")
            # , NETCACHE_VALUEUPDATE, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH
            for tmpoptype in [
                GETREQ,
                CACHE_POP_INSWITCH,
                PUTREQ,
                DELREQ,
                WARMUPREQ,
                LOADREQ,
                CACHE_EVICT_LOADFREQ_INSWITCH,
                SETVALID_INSWITCH,
                DISTCACHE_INVALIDATE,
                DISTCACHE_VALUEUPDATE_INSWITCH,
                PUTREQ_LARGEVALUE,
            ]:
                matchspec0 = [hex(tmpoptype)]
                self.controller.table_add(
                    "hash_for_partition_tbl", "hash_for_partition", matchspec0
                )

        # Stage 2

        # Table: hash_partition_tbl (default: NoAction; size <= 11 * 128)
        print("Configuring hash_partition_tbl")
        hash_range_per_leafswitch = int(
            switch_partition_count / leafswitch_total_logical_num
        )
        # , NETCACHE_VALUEUPDATE, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH
        for tmpoptype in [
            GETREQ,
            CACHE_POP_INSWITCH,
            PUTREQ,
            DELREQ,
            WARMUPREQ,
            LOADREQ,
            CACHE_EVICT_LOADFREQ_INSWITCH,
            SETVALID_INSWITCH,
            DISTCACHE_INVALIDATE,
            DISTCACHE_VALUEUPDATE_INSWITCH,
            PUTREQ_LARGEVALUE,
        ]:
            hash_start = 0  # [0, partition_count-1]
            for i in range(leafswitch_total_logical_num):
                global_leafswitch_logical_idx = leafswitch_logical_idxes[i]
                if i == leafswitch_total_logical_num - 1:
                    hash_end = (
                        switch_partition_count - 1
                    )  # if end is not included, then it is just processed by port 1111
                else:
                    hash_end = hash_start + hash_range_per_leafswitch - 1
                # NOTE: both start and end are included
                matchspec0 = [
                    hex(tmpoptype),
                    "" + hex(hash_start) + "->" + hex(hash_end),
                ]
                # Forward to the egress pipeline of leaf switch
                eport = self.serverleafswitch_devport
                if tmpoptype == DISTCACHE_INVALIDATE:
                    actnspec0 = [eport]
                    self.controller.table_add(
                        "hash_partition_tbl",
                        "hash_partition_for_distcache_invalidate",
                        matchspec0,
                        actnspec0,
                        0,
                    )  # 0 is priority (range may be overlapping)
                elif tmpoptype == DISTCACHE_VALUEUPDATE_INSWITCH:
                    actnspec0 = [eport]
                    self.controller.table_add(
                        "hash_partition_tbl",
                        "hash_partition_for_distcache_valueupdate_inswitch",
                        matchspec0,
                        actnspec0,
                        0,
                    )  # 0 is priority (range may be overlapping)
                else:
                    actnspec0 = [eport, hex(global_leafswitch_logical_idx)]
                    self.controller.table_add(
                        "hash_partition_tbl", "hash_partition", matchspec0, actnspec0, 0
                    )  # 0 is priority (range may be overlapping)
                hash_start = hash_end + 1

        # Stage 3
        # Table: cache_lookup_tbl (default: uncached_action; size: 32K/64K)
        print("Leave cache_lookup_tbl managed by controller in runtime")

        # Table: hash_for_seq_tbl (default: NoAction; size: 3)
        print("Configuring hash_for_seq_tbl")
        for tmpoptype in [PUTREQ, DELREQ, PUTREQ_LARGEVALUE]:
            matchspec0 = [hex(tmpoptype)]
            self.controller.table_add("hash_for_seq_tbl", "hash_for_seq", matchspec0)

        # Stage 3

        # Table: prepare_for_cachehit_tbl (default: set_client_sid(0); size: 2*client_physical_num=4 < 2*8=16 < 32)
        print("Configuring prepare_for_cachehit_tbl")
        for client_physical_idx in range(client_physical_num):
            tmp_clientip = client_ips[client_physical_idx]
            for tmpoptype in [GETREQ, WARMUPREQ]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + client_ips[client_physical_idx] + "/32",
                ]
                # ig_intr_md_ingress_port = self.clientleafswitch_devport)
                actnspec0 = [hex(self.clientleafswitch_sid)]
                self.controller.table_add(
                    "prepare_for_cachehit_tbl", "set_client_sid", matchspec0, actnspec0
                )

        # Table: ipv4_forward_tbl (default: NoAction; size: 7*client_physical_num=14 < 7*8=56)
        print("Configuring ipv4_forward_tbl")
        for tmp_client_physical_idx in range(client_physical_num):
            # ipv4addr0 = ipv4Addr_to_i32(client_ips[tmp_client_physical_idx])
            # eport = self.client_devports[tmp_client_physical_idx]
            # tmpsid = self.client_sids[tmp_client_physical_idx]
            eport = self.clientleafswitch_devport
            tmpsid = self.clientleafswitch_sid
            for tmpoptype in [
                GETRES,
                PUTRES,
                DELRES,
                WARMUPACK,
                SCANRES_SPLIT,
                LOADACK,
                GETRES_LARGEVALUE,
            ]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + client_ips[tmp_client_physical_idx] + "/32",
                ]
                actnspec0 = [eport]
                self.controller.table_add(
                    "ipv4_forward_tbl", "forward_normal_response", matchspec0, actnspec0
                )

        # Stage 4

        # Table: sample_tbl (default: NoAction; size: 1)
        print("Configuring sample_tbl")
        for tmpoptype in [GETREQ]:
            matchspec0 = [hex(tmpoptype)]
            self.controller.table_add("sample_tbl", "sample", matchspec0)

        # Table: ig_port_forward_tbl (default: NoAction; size: 9)
        print("Configuring ig_port_forward_tbl")
        matchspec0 = [hex(GETREQ)]
        self.controller.table_add(
            "ig_port_forward_tbl", "update_getreq_to_getreq_inswitch", matchspec0
        )
        matchspec0 = [hex(PUTREQ)]
        self.controller.table_add(
            "ig_port_forward_tbl", "update_putreq_to_putreq_inswitch", matchspec0
        )
        matchspec0 = [hex(DELREQ)]
        self.controller.table_add(
            "ig_port_forward_tbl", "update_delreq_to_delreq_inswitch", matchspec0
        )

        matchspec0 = [hex(WARMUPREQ)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_warmupreq_to_netcache_warmupreq_inswitch",
            matchspec0,
        )
        matchspec0 = [hex(DISTCACHE_VALUEUPDATE_INSWITCH)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_distcache_valueupdate_inswitch_to_distcache_valueupdate_inswitch_origin",
            matchspec0,
        )
        matchspec0 = [hex(LOADREQ)]
        self.controller.table_add(
            "ig_port_forward_tbl", "update_loadreq_to_loadreq_spine", matchspec0
        )
        matchspec0 = [hex(DISTCACHE_INVALIDATE)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_distcache_invalidate_to_distcache_invalidate_inswitch",
            matchspec0,
        )
        matchspec0 = [hex(PUTREQ_LARGEVALUE)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_putreq_largevalue_to_putreq_largevalue_inswitch",
            matchspec0,
        )

        # Egress pipeline

        # Stage 0

        # Table: access_latest_tbl (default: reset_is_latest; size: 6)
        print("Configuring access_latest_tbl")
        for is_cached in cached_list:
            matchspec0 = [hex(GETREQ_INSWITCH), hex(is_cached), hex(0)]
            if is_cached == 1:
                self.controller.table_add("access_latest_tbl", "get_latest", matchspec0)
            ## NOTE: write queries of NetCache "invalidates" in-switch value by setting latest=0
            # NOTE: in DistCache, write queries do NOT invalidate in-switch value on path, which is covered by server-issued DISTCACHE_INVALIDATE
            # for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH]:
            #    matchspec0 = distcachespine_access_latest_tbl_match_spec_t(\
            #            op_hdr_optype = tmpoptype,
            #            inswitch_hdr_is_cached = is_cached)
            #    if is_cached == 1:
            #        self.client.access_latest_tbl_table_add_with_reset_and_get_latest(\
            #                self.sess_hdl, self.dev_tgt, matchspec0)
            # NOTE: cache population of NetCache directly sets latest=1 due to blocking-based cache update
            matchspec0 = [hex(CACHE_POP_INSWITCH), hex(is_cached), hex(0)]
            # TRICK for Tofino hardware bug (DISTCACHE_VALUEUPDATE_INSWITCH cannot set latest=1 immediately after populating the new record) -> NOT degrade perf
            # self.client.access_latest_tbl_table_add_with_reset_and_get_latest(\
            self.controller.table_add(
                "access_latest_tbl", "set_and_get_latest", matchspec0
            )
            matchspec0 = [
                hex(DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN),
                hex(is_cached),
                hex(0),
            ]
            self.controller.table_add(
                "access_latest_tbl", "set_and_get_latest", matchspec0
            )
            matchspec0 = [hex(DISTCACHE_INVALIDATE_INSWITCH), hex(is_cached), hex(0)]
            if is_cached == 1:
                self.controller.table_add(
                    "access_latest_tbl", "reset_and_get_latest", matchspec0
                )
                # self.controller.table_add("access_latest_tbl","set_and_get_latest",matchspec0)
            # NOTE: DistCache resorts server to invalidate inswitch cache for PUTREQ_LARGEVALUE_INSWITCH instead of on path

        # Table: access_seq_tbl (default: NoAction; size: 3)
        # NOTE: PUT/DELREQ_INSWITCH do NOT have fraginfo_hdr, while we ONLY assign seq for fragment 0 of PUTREQ_LARGEVALUE_INSWITCH
        print("Configuring access_seq_tbl")
        for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH, PUTREQ_LARGEVALUE_INSWITCH]:
            matchspec0 = [hex(tmpoptype), hex(0)]
            self.controller.table_add("access_seq_tbl", "assign_seq", matchspec0)

        # Table: save_client_info_tbl (default: NoAction; size: 4)
        print("Configuring save_client_info_tbl")
        for tmpoptype in [GETREQ_INSWITCH, NETCACHE_WARMUPREQ_INSWITCH]:
            matchspec0 = [hex(tmpoptype)]
            self.controller.table_add(
                "save_client_info_tbl", "save_client_info", matchspec0
            )

        # Stage 1

        # Table: prepare_for_cachepop_tbl (default: reset_server_sid(); size: 2*server_physical_num+1=5 < 17)
        # NOTE: spine switch in DistCache does NOT need to prepare_for_cachepop for GETREQ_INSWITCH
        for tmpoptype in [GETREQ_INSWITCH, SCANREQ_SPLIT]:
            tmp_devport = self.serverleafswitch_devport
            tmp_server_sid = self.serverleafswitch_sid
            matchspec0 = [hex(tmpoptype), tmp_devport]
            if tmpoptype == GETREQ_INSWITCH:
                actnspec0 = [hex(tmp_server_sid)]
                self.controller.table_add(
                    "prepare_for_cachepop_tbl",
                    "set_server_sid_and_port",
                    matchspec0,
                    actnspec0,
                )
                # NOTE: we explictly invoke NoAction() for SCANREQ_SPLIT and NETCACHE_GETREQ_POP to avoid reset their clone_hdr.server_sid: SCANREQ_SPLIT.server_sid is set by process_scanreq_split to clone next SCANREQ_SPLIT to next server; NETCACHE_GETREQ_POP.server_sid is inherited from original GETREQ_INSWITCH to clone alst NETCACHE_GETREQ_POP as GETREQ to corresponding server

        matchspec0 = [hex(NETCACHE_GETREQ_POP), self.reflector_devport]
        self.controller.table_add("prepare_for_cachepop_tbl", "NoAction", matchspec0)

        # Stgae 2

        # Table: access_cache_frequency_tbl (default: NoAction; size: 17)
        print("Configuring access_cache_frequency_tbl")
        for tmpoptype in [GETREQ_INSWITCH]:
            matchspec0 = [hex(tmpoptype), hex(1), hex(1), hex(1)]
            self.controller.table_add(
                "access_cache_frequency_tbl", "update_cache_frequency", matchspec0
            )
        for is_sampled in sampled_list:
            for is_cached in cached_list:
                for is_latest in latest_list:
                    matchspec0 = [
                        hex(CACHE_POP_INSWITCH),
                        hex(is_sampled),
                        hex(is_cached),
                        hex(is_latest),
                    ]
                    self.controller.table_add(
                        "access_cache_frequency_tbl",
                        "reset_cache_frequency",
                        matchspec0,
                    )
                    matchspec0 = [
                        hex(CACHE_EVICT_LOADFREQ_INSWITCH),
                        hex(is_sampled),
                        hex(is_cached),
                        hex(is_latest),
                    ]
                    self.controller.table_add(
                        "access_cache_frequency_tbl", "get_cache_frequency", matchspec0
                    )

        # Table: access_deleted_tbl (default: reset_is_deleted; size: 16)
        print("Configuring access_deleted_tbl")
        for is_cached in cached_list:
            for is_latest in latest_list:
                for is_stat in stat_list:
                    matchspec0 = [
                        hex(GETREQ_INSWITCH),
                        hex(is_cached),
                        hex(is_latest),
                        hex(is_stat),
                    ]
                    if is_cached == 1:
                        self.controller.table_add(
                            "access_deleted_tbl", "get_deleted", matchspec0
                        )
                    matchspec0 = [
                        hex(CACHE_POP_INSWITCH),
                        hex(is_cached),
                        hex(is_latest),
                        hex(is_stat),
                    ]
                    if is_stat == 1:
                        self.controller.table_add(
                            "access_deleted_tbl", "reset_and_get_deleted", matchspec0
                        )
                    elif is_stat == 0:
                        self.controller.table_add(
                            "access_deleted_tbl", "set_and_get_deleted", matchspec0
                        )

                    matchspec0 = [
                        hex(DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN),
                        hex(is_cached),
                        hex(is_latest),
                        hex(is_stat),
                    ]
                    if is_stat == 1:
                        self.controller.table_add(
                            "access_deleted_tbl", "reset_and_get_deleted", matchspec0
                        )
                    elif is_stat == 0:
                        self.controller.table_add(
                            "access_deleted_tbl", "set_and_get_deleted", matchspec0
                        )

        # Table: access_savedseq_tbl (default: NoAction; size: 6)
        print("Configuring access_savedseq_tbl")
        for is_cached in cached_list:
            for is_latest in latest_list:
                matchspec0 = [
                    hex(CACHE_POP_INSWITCH),
                    hex(is_cached),
                    hex(is_latest),
                ]
                self.controller.table_add(
                    "access_savedseq_tbl", "set_and_get_savedseq", matchspec0
                )
                # matchspec0 = distcachespine_access_savedseq_tbl_match_spec_t(\
                #        op_hdr_optype = NETCACHE_VALUEUPDATE_INSWITCH,
                #        inswitch_hdr_is_cached = is_cached,
                #        meta_is_latest = is_latest)
                # if is_cached == 1:
                #    self.client.access_savedseq_tbl_table_add_with_set_and_get_savedseq(\
                #            self.sess_hdl, self.dev_tgt, matchspec0)
                # matchspec0 = distcachespine_access_savedseq_tbl_match_spec_t(\
                #        op_hdr_optype = DISTCACHE_SPINE_VALUEUPDATE_INSWITCH,
                #        inswitch_hdr_is_cached = is_cached,
                #        meta_is_latest = is_latest)
                # self.client.access_savedseq_tbl_table_add_with_set_and_get_savedseq(\
                #        self.sess_hdl, self.dev_tgt, matchspec0)
                matchspec0 = [
                    hex(DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN),
                    hex(is_cached),
                    hex(is_latest),
                ]
                self.controller.table_add(
                    "access_savedseq_tbl", "set_and_get_savedseq", matchspec0
                )

        # Stage 3

        # Table: update_vallen_tbl (default: reset_access_val_mode; 10)
        print("Configuring update_vallen_tbl")
        for is_cached in cached_list:
            for is_latest in latest_list:
                matchspec0 = [
                    hex(GETREQ_INSWITCH),
                    hex(is_cached),
                    hex(is_latest),
                ]
                if is_cached == 1:
                    self.controller.table_add(
                        "update_vallen_tbl", "get_vallen", matchspec0
                    )
                matchspec0 = [
                    hex(CACHE_POP_INSWITCH),
                    hex(is_cached),
                    hex(is_latest),
                ]
                self.controller.table_add(
                    "update_vallen_tbl", "set_and_get_vallen", matchspec0
                )

                matchspec0 = [
                    hex(DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN),
                    hex(is_cached),
                    hex(is_latest),
                ]
                self.controller.table_add(
                    "update_vallen_tbl", "set_and_get_vallen", matchspec0
                )

        # Stage 4

        # Table: is_report_tbl (default: reset_is_report; size: 1)
        print("Configuring is_report_tbl")
        matchspec0 = [hex(1), hex(1), hex(1)]
        # meta_is_report3 = 1)
        self.controller.table_add("is_report_tbl", "set_is_report", matchspec0)

        # Stage 4-11

        # Table: update_vallo1_tbl (default: NoAction; 14)
        for i in range(1, 17):
            print("Configuring update_vallo{}_tbl".format(i))
            self.configure_update_val_tbl("lo{}".format(i))
            print("Configuring update_valhi{}_tbl".format(i))
            self.configure_update_val_tbl("hi{}".format(i))

        # Stage 9

        # Table: lastclone_lastscansplit_tbl (default: reset_is_lastclone_lastscansplit; size: 2/3)
        print("Configuring lastclone_lastscansplit_tbl")
        if RANGE_SUPPORT == False:
            for tmpoptype in [NETCACHE_GETREQ_POP, NETCACHE_WARMUPREQ_INSWITCH_POP]:
                matchspec0 = [hex(tmpoptype), hex(0)]
                self.controller.table_add(
                    "lastclone_lastscansplit_tbl", "set_is_lastclone", matchspec0
                )

        # Stage 10

        # Table: eg_port_forward_tbl (default: NoAction; size: < 2048 < 8192)
        print("Configuring eg_port_forward_tbl")
        if RANGE_SUPPORT == False:
            self.configure_eg_port_forward_tbl()

        # Table: update_pktlen_tbl (default: NoAction; 4*17+10 = 78)
        print("Configuring update_pktlen_tbl")
        for i in range(int(switch_max_vallen / 8) + 1):  # i from 0 to 16
            if i == 0:
                vallen_start = 0
                vallen_end = 0
                aligned_vallen = 0
            else:
                vallen_start = (i - 1) * 8 + 1  # 1, 9, ..., 121
                vallen_end = (i - 1) * 8 + 8  # 8, 16, ..., 128
                aligned_vallen = vallen_end  # 8, 16, ..., 128
            val_stat_udplen = aligned_vallen + 46
            val_stat_iplen = aligned_vallen + 66
            val_seq_udplen = aligned_vallen + 38
            val_seq_iplen = aligned_vallen + 58
            # for tmpoptype in [GETRES, DISTCACHE_GETRES_SPINE]:
            for tmpoptype in [GETRES]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + hex(vallen_start) + "->" + hex(vallen_end),
                ]  # [vallen_start, vallen_end]
                actnspec0 = [hex(val_stat_udplen), hex(val_stat_iplen)]
                self.controller.table_add(
                    "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
                )  # 0 is priority (range may be overlapping)
            for tmpoptype in [PUTREQ_SEQ, NETCACHE_PUTREQ_SEQ_CACHED]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + hex(vallen_start) + "->" + hex(vallen_end),
                ]  # [vallen_start, vallen_end]
                actnspec0 = [hex(val_seq_udplen), hex(val_seq_iplen)]
                self.controller.table_add(
                    "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
                )  # 0 is priority (range may be overlapping)
        onlyop_udplen = 30
        onlyop_iplen = 50
        op_inswitch_udplen = 60
        op_inswitch_iplen = 80
        op_switchload_udplen = 40
        op_switchload_iplen = 60
        seq_udplen = 36
        seq_iplen = 56
        scanreqsplit_udplen = 58
        scanreqsplit_iplen = 78
        frequency_udplen = 34
        frequency_iplen = 54
        op_switchload_clone_udplen = 58
        op_switchload_clone_iplen = 78
        op_inswitch_clone_udplen = 78
        op_inswitch_clone_iplen = 98
        # , NETCACHE_VALUEUPDATE_ACK, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK
        for tmpoptype in [CACHE_POP_INSWITCH_ACK, WARMUPACK]:
            matchspec0 = [
                hex(tmpoptype),
                "" + hex(0) + "->" + hex(switch_max_vallen),
            ]  # [0, 128]
            actnspec0 = [hex(onlyop_udplen), hex(onlyop_iplen)]
            self.controller.table_add(
                "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
            )  # 0 is priority (range may be overlapping)

        matchspec0 = [
            hex(DISTCACHE_VALUEUPDATE_INSWITCH_ACK),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]  # [0, 128]
        actnspec0 = [hex(op_inswitch_udplen), hex(op_inswitch_iplen)]
        self.controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping)
        matchspec0 = [
            hex(GETREQ),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]  # [0, 128]
        actnspec0 = [hex(op_switchload_udplen), hex(op_switchload_iplen)]
        self.controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping)
        for tmpoptype in [DELREQ_SEQ, NETCACHE_DELREQ_SEQ_CACHED]:
            matchspec0 = [
                hex(DISTCACHE_VALUEUPDATE_INSWITCH_ACK),
                "" + hex(0) + "->" + hex(switch_max_vallen),
            ]  # [0, 128]
            actnspec0 = [hex(seq_udplen), hex(seq_iplen)]
            self.controller.table_add(
                "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
            )  # 0 is priority (range may be overlapping)
        matchspec0 = [
            hex(SCANREQ_SPLIT),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]  # [0, 128]
        actnspec0 = [hex(scanreqsplit_udplen), hex(scanreqsplit_iplen)]
        self.controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping)
        matchspec0 = [
            hex(CACHE_EVICT_LOADFREQ_INSWITCH_ACK),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]  # [0, 128]
        actnspec0 = [hex(frequency_udplen), hex(frequency_iplen)]
        self.controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping)
        matchspec0 = [
            hex(NETCACHE_GETREQ_POP),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]  # [0, 128]
        actnspec0 = [hex(op_switchload_clone_udplen), hex(op_switchload_clone_iplen)]
        self.controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping)
        matchspec0 = [
            hex(NETCACHE_WARMUPREQ_INSWITCH_POP),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]  # [0, 128]
        actnspec0 = [hex(op_inswitch_clone_udplen), hex(op_inswitch_clone_iplen)]
        self.controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping)
        # For large value
        shadowtype_seq_udp_delta = 6
        shadowtype_seq_ip_delta = 6
        for tmpoptype in [PUTREQ_LARGEVALUE_SEQ, PUTREQ_LARGEVALUE_SEQ_CACHED]:
            matchspec0 = [
                hex(tmpoptype),
                "" + hex(0) + "->" + hex(65535),
            ]  # [0, 65535] (NOTE: vallen MUST = 0 for PUTREQ_LARGEVALUE_INSWITCH)
            actnspec0 = [hex(shadowtype_seq_udp_delta), hex(shadowtype_seq_ip_delta)]
            self.controller.table_add(
                "update_pktlen_tbl", "add_pktlen", matchspec0, actnspec0, 0
            )  # 0 is priority (range may be overlapping)

        # Table: update_ipmac_srcport_tbl (default: NoAction; 7*client_physical_num+12*server_physical_num+9=47 < 19*8+9=161 < 256)
        # NOTE: udp.dstport is updated by eg_port_forward_tbl (only required by switch2switchos)
        # NOTE: update_ipmac_srcport_tbl focues on src/dst ip/mac and udp.srcport
        print("Configuring update_ipmac_srcport_tbl")
        # (1) for response from server to client, egress port has been set based on ip.dstaddr (or by clone_i2e) in ingress pipeline
        # (2) for response from switch to client, egress port has been set by clone_e2e in egress pipeline
        tmp_devport = self.clientleafswitch_devport
        tmp_server_mac = server_macs[0]
        tmp_server_ip = server_ips[0]
        actnspec0 = [tmp_server_mac, tmp_server_ip, hex(server_worker_port_start)]
        # for tmpoptype in [GETRES, DISTCACHE_GETRES_SPINE, PUTRES, DELRES, SCANRES_SPLIT, WARMUPACK, LOADACK]:
        for tmpoptype in [GETRES, PUTRES, DELRES, SCANRES_SPLIT, WARMUPACK, LOADACK]:
            matchspec0 = [hex(tmpoptype), tmp_devport]
            self.controller.table_add(
                "update_ipmac_srcport_tbl",
                "update_srcipmac_srcport_server2client",
                matchspec0,
                actnspec0,
            )
        # Here we use server_mac/ip to simulate reflector_mac/ip = switchos_mac/ip
        # (1) eg_intr_md.egress_port of the first GETRES_CASE1 is set by ipv4_forward_tbl (as ingress port), which will be finally dropped -> update ip/mac/srcport or not is not important
        # (2) eg_intr_md.egress_port of cloned GETRES_CASE1s is set by clone_e2e, which must be the devport towards switchos (aka reflector)
        # (3) eg_intr_md.egress_port of the first ACK for cache population/eviction is set by partition_tbl in ingress pipeline, which will be finally dropped -> update ip/mac/srcport or not is not important
        # (4) eg_intr_md.egress_port of the cloned ACK for cache population/eviction is set by clone_e2e, which must be the devport towards switchos (aka reflector)
        tmp_devport = self.reflector_devport
        # tmp_client_ip = client_ips[0]
        # tmp_client_mac = client_macs[0]
        tmp_client_ip = reflector_cp2dp_dstip
        tmp_client_mac = reflector_cp2dp_dstmac
        tmp_client_port = str(123)  # not cared by servers
        actnspec2 = [
            (tmp_client_mac),
            (self.reflector_mac_for_switch),
            (tmp_client_ip),
            (self.reflector_ip_for_switch),
            tmp_client_port,
        ]
        for tmpoptype in [
            CACHE_POP_INSWITCH_ACK,
            CACHE_EVICT_LOADFREQ_INSWITCH_ACK,
        ]:  # simulate client/switch -> switchos
            matchspec0 = [
                (hex(tmpoptype)),
                tmp_devport,
            ]
            self.controller.table_add(
                "update_ipmac_srcport_tbl",
                "update_ipmac_srcport_switch2switchos",
                matchspec0,
                actnspec2,
            )
        actnspec2 = [
            (self.reflector_mac_for_switch),
            (self.reflector_ip_for_switch),
        ]
        for tmpoptype in [NETCACHE_WARMUPREQ_INSWITCH_POP]:
            matchspec0 = [
                (hex(tmpoptype)),
                tmp_devport,
            ]
            self.controller.table_add(
                "update_ipmac_srcport_tbl",
                "update_dstipmac_switch2switchos",
                matchspec0,
                actnspec2,
            )

        # Table: add_and_remove_value_header_tbl (default: remove_all; 17*4=68)
        print("Configuring add_and_remove_value_header_tbl")
        # NOTE: egress pipeline must not output PUTREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, CACHE_POP_INSWITCH, and PUTREQ_INSWITCH
        # NOTE: even for future PUTREQ_LARGE/GETRES_LARGE, as their values should be in payload, we should invoke add_only_vallen() for vallen in [0, global_max_vallen]
        # , LOADREQ_SPINE
        for tmpoptype in [PUTREQ_SEQ, NETCACHE_PUTREQ_SEQ_CACHED, GETRES]:
            for i in range(int(switch_max_vallen / 8) + 1):  # i from 0 to 16
                if i == 0:
                    vallen_start = 0
                    vallen_end = 0
                else:
                    vallen_start = (i - 1) * 8 + 1  # 1, 9, ..., 121
                    vallen_end = (i - 1) * 8 + 8  # 8, 16, ..., 128
                matchspec0 = [
                    hex(tmpoptype),
                    "" + hex(vallen_start) + "->" + hex(vallen_end),
                ]  # [vallen_start, vallen_end]
                if i == 0:
                    self.controller.table_add(
                        "add_and_remove_value_header_tbl",
                        "add_only_vallen",
                        matchspec0,
                        [],
                        0,
                    )
                else:
                    self.controller.table_add(
                        "add_and_remove_value_header_tbl",
                        "add_to_val{}".format(i),
                        matchspec0,
                        [],
                        0,
                    )

    def configure_eg_port_forward_tbl(self):
        # Table: eg_port_forward_tbl (default: NoAction; size: 27+852*client_physical_num=27+852*2=1731 < 2048 < 27+852*8=6843 < 8192)
        tmp_client_sids = [0, self.clientleafswitch_sid]
        tmp_server_sids = [0, self.serverleafswitch_sid]
        for is_cached in cached_list:
            for is_report in report_list:
                for is_latest in latest_list:
                    for is_deleted in deleted_list:
                        # Use tmpstat as action data to reduce action number
                        if is_deleted == 1:
                            tmpstat = 0
                        else:
                            tmpstat = 1
                        # NOTE: eg_intr_md.egress_port is read-only
                        # for is_wrong_pipeline in pipeline_list:
                        # for tmp_client_sid in self.sids:
                        for tmp_client_sid in tmp_client_sids:
                            for is_lastclone_for_pktloss in lastclone_list:
                                for (
                                    tmp_server_sid
                                ) in (
                                    tmp_server_sids
                                ):  # Only work for NETCACHE_GETREQ_POP
                                    # is_report=0, is_latest=0, is_deleted=0, is_lastclone_for_pktloss=0, tmp_server_sid=0 for NETCACHE_WARMUPREQ_INSWITCH
                                    # NOTE: tmp_server_sid must be 0 as the last NETCACHE_WARMUPREQ_INSWITCH_POP is cloned as WARMUPACK to client instead of server
                                    if (
                                        is_report == 0
                                        and is_latest == 0
                                        and is_deleted == 0
                                        and is_lastclone_for_pktloss == 0
                                        and tmp_server_sid == 0
                                        and tmp_client_sid != 0
                                    ):
                                        # size: 2*client_physical_num=4 < 2*8=16
                                        matchspec0 = [
                                            hex(NETCACHE_WARMUPREQ_INSWITCH),
                                            hex(is_cached),
                                            hex(is_report),
                                            hex(is_latest),
                                            hex(is_deleted),
                                            hex(tmp_client_sid),
                                            hex(is_lastclone_for_pktloss),
                                            hex(tmp_server_sid),
                                        ]
                                        # Update NETCACHE_WARMUP_INSWITCH as NETCACHE_WARMUP_INSWITCH_POP to switchos by cloning
                                        actnspec0 = [
                                            hex(self.reflector_sid),
                                            hex(reflector_dp2cpserver_port),
                                        ]
                                        self.controller.table_add(
                                            "eg_port_forward_tbl",
                                            "update_netcache_warmupreq_inswitch_to_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack",
                                            matchspec0,
                                            actnspec0,
                                        )
                                    # is_report=0, is_latest=0, is_deleted=0, tmp_server_sid=0 for NETCACHE_WARMUPREQ_INSWITCH_POP
                                    if (
                                        is_report == 0
                                        and is_latest == 0
                                        and is_deleted == 0
                                        and tmp_server_sid == 0
                                        and tmp_client_sid != 0
                                    ):
                                        # size: 4*client_physical_num=8 < 4*8=32
                                        matchspec0 = [
                                            hex(NETCACHE_WARMUPREQ_INSWITCH_POP),
                                            hex(is_cached),
                                            hex(is_report),
                                            hex(is_latest),
                                            hex(is_deleted),
                                            hex(tmp_client_sid),
                                            hex(is_lastclone_for_pktloss),
                                            hex(tmp_server_sid),
                                        ]
                                        if is_lastclone_for_pktloss == 0:
                                            # Forward NETCACHE_WARMUP_INSWITCH_POP to switchos and clone
                                            actnspec0 = [hex(self.reflector_sid)]
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "forward_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack",
                                                matchspec0,
                                                actnspec0,
                                            )
                                        elif is_lastclone_for_pktloss == 1:
                                            # Update NETCACHE_WARMUP_INSWITCH_POP as WARMUPACK to client by mirroring
                                            # NOTE: WARMUPACK performs default action NoAction() to be forwarded to client
                                            actnspec0 = [
                                                hex(tmp_client_sid),
                                                hex(server_worker_port_start),
                                            ]
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_netcache_warmupreq_inswitch_pop_to_warmupack_by_mirroring",
                                                matchspec0,
                                                actnspec0,
                                            )
                                    # is_lastclone_for_pktloss should be 0 for GETREQ_INSWITCH
                                    if (
                                        is_report == 0
                                        and is_lastclone_for_pktloss == 0
                                        and tmp_client_sid != 0
                                        and tmp_server_sid != 0
                                    ):
                                        # size: 32*client_physical_num*server_physical_num=128 < 32*8*8=2048
                                        # NOTE: tmp_client_sid != 0 to prepare for cache hit; tmp_server_sid != 0 to prepare for cache pop (clone last NETCACHE_GETREQ_POP as GETREQ to server)
                                        matchspec0 = [
                                            hex(GETREQ_INSWITCH),
                                            hex(is_cached),
                                            hex(is_report),
                                            hex(is_latest),
                                            hex(is_deleted),
                                            hex(tmp_client_sid),
                                            hex(is_lastclone_for_pktloss),
                                            hex(tmp_server_sid),
                                        ]
                                        if is_cached == 0:
                                            # Update GETREQ_INSWITCH as GETREQ_SPINE to leaf
                                            # actnspec0 = [self.devPorts[1]]
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_getreq_inswitch_to_getreq_spine",
                                                matchspec0,
                                            )
                                        else:  # is_cached == 1
                                            if (
                                                is_latest == 0
                                            ):  # follow algorithm 1 in NetCache paper to report hot key if necessary
                                                # Update GETREQ_INSWITCH as GETREQ_SPINE to server-leaf
                                                # actnspec0 = [self.devPorts[1]]
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_getreq_inswitch_to_getreq_spine",
                                                    matchspec0,
                                                )
                                            else:  # is_cached == 1 and is_latest == 1
                                                ## Update GETREQ_INSWITCH as DISTCACHE_GETRES_SPINE to client by mirroring
                                                # actnspec0 = [tmp_client_sid, server_worker_port_start, tmpstat]
                                                # self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_distcache_getres_spine_by_mirroring(\
                                                #        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                # Update GETREQ_INSWITCH as GETRES to client by mirroring
                                                actnspec0 = [
                                                    hex(tmp_client_sid),
                                                    hex(server_worker_port_start),
                                                    hex(tmpstat),
                                                ]
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_getreq_inswitch_to_getres_by_mirroring",
                                                    matchspec0,
                                                    actnspec0,
                                                )
                                    # is_cached=0 (memset inswitch_hdr by end-host, and key must not be cached in cache_lookup_tbl for CACHE_POP_INSWITCH), is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss should be 0 for CACHE_POP_INSWITCH
                                    # size: 4
                                    # if is_cached == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                    if (
                                        is_cached == 0
                                        and is_report == 0
                                        and tmp_client_sid == 0
                                        and is_lastclone_for_pktloss == 0
                                        and tmp_server_sid == 0
                                    ):
                                        matchspec0 = [
                                            hex(CACHE_POP_INSWITCH),
                                            hex(is_cached),
                                            hex(is_report),
                                            hex(is_latest),
                                            hex(is_deleted),
                                            hex(tmp_client_sid),
                                            hex(is_lastclone_for_pktloss),
                                            hex(tmp_server_sid),
                                        ]
                                        # Update CACHE_POP_INSWITCH as CACHE_POP_INSWITCH_ACK to reflector (deprecated: w/ clone)
                                        actnspec0 = [
                                            hex(self.reflector_sid),
                                            hex(reflector_dp2cpserver_port),
                                        ]
                                        self.controller.table_add(
                                            "eg_port_forward_tbl",
                                            "update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone",
                                            matchspec0,
                                            actnspec0,
                                        )
                                    # is_cached=0 (no inswitch_hdr after clone_e2e), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktloss should be 0 for CACHE_POP_INSWITCH_ACK
                                    # size: 0
                                    # if is_cached == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0:
                                    if (
                                        is_cached == 0
                                        and is_report == 0
                                        and is_latest == 0
                                        and is_deleted == 0
                                        and tmp_client_sid == 0
                                        and is_lastclone_for_pktloss == 0
                                        and tmp_server_sid == 0
                                    ):
                                        matchspec0 = [
                                            hex(CACHE_POP_INSWITCH_ACK),
                                            hex(is_cached),
                                            hex(is_report),
                                            hex(is_latest),
                                            hex(is_deleted),
                                            hex(tmp_client_sid),
                                            hex(is_lastclone_for_pktloss),
                                            hex(tmp_server_sid),
                                        ]
                                        # if is_lastclone_for_pktloss == 0:
                                        #    # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector (w/ clone)
                                        #    actnspec0 = [self.reflector_sid]
                                        #    self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack_clone_for_pktloss(\
                                        #            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        # elif is_lastclone_for_pktloss == 1:
                                        #    # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                        #    self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack(\
                                        #            self.sess_hdl, self.dev_tgt, matchspec0)

                                        # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                        # self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack(\
                                        #        self.sess_hdl, self.dev_tgt, matchspec0)
                                        # NOTE: default action is NoAction -> forward the packet to sid set by clone_e2e
                                        pass
                                    # is_deleted, tmp_client_sid, is_lastclone_for_pktloss should be 0 for PUTREQ_INSWITCH
                                    # size: 4
                                    if (
                                        is_report == 0
                                        and is_deleted == 0
                                        and tmp_client_sid == 0
                                        and is_lastclone_for_pktloss == 0
                                        and tmp_server_sid == 0
                                    ):
                                        matchspec0 = [
                                            hex(PUTREQ_INSWITCH),
                                            hex(is_cached),
                                            hex(is_report),
                                            hex(is_latest),
                                            hex(is_deleted),
                                            hex(tmp_client_sid),
                                            hex(is_lastclone_for_pktloss),
                                            hex(tmp_server_sid),
                                        ]
                                        if is_cached == 0:
                                            # Update PUTREQ_INSWITCH as PUTREQ_SEQ to server
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_putreq_inswitch_to_putreq_seq",
                                                matchspec0,
                                            )
                                        elif is_cached == 1:
                                            # Update PUTREQ_INSWITCH as NETCACHE_PUTREQ_SEQ_CACHED to server
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_putreq_inswitch_to_netcache_putreq_seq_cached",
                                                matchspec0,
                                            )
                                    # (cm_predicate=1), is_deleted, tmp_client_sid, is_lastclone_for_pktloss should be 0 for DELREQ_INSWITCH
                                    # size: 4
                                    if (
                                        is_report == 0
                                        and is_deleted == 0
                                        and tmp_client_sid == 0
                                        and is_lastclone_for_pktloss == 0
                                        and tmp_server_sid == 0
                                    ):
                                        matchspec0 = [
                                            hex(DELREQ_INSWITCH),
                                            hex(is_cached),
                                            hex(is_report),
                                            hex(is_latest),
                                            hex(is_deleted),
                                            hex(tmp_client_sid),
                                            hex(is_lastclone_for_pktloss),
                                            hex(tmp_server_sid),
                                        ]
                                        if is_cached == 0:
                                            # Update DELREQ_INSWITCH as DELREQ_SEQ to server
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_delreq_inswitch_to_delreq_seq",
                                                matchspec0,
                                            )
                                        elif is_cached == 1:
                                            # Update DELREQ_INSWITCH as NETCACHE_DELREQ_SEQ_CACHED to server
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_delreq_inswitch_to_netcache_delreq_seq_cached",
                                                matchspec0,
                                            )
                                    # is_cached=1 (key must be cached in cache_lookup_tbl for CACHE_EVICT_LOADFREQ_INSWITCH), s_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH
                                    # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                    # size: 1
                                    if (
                                        is_cached == 1
                                        and is_report == 0
                                        and is_latest == 0
                                        and is_deleted == 0
                                        and tmp_client_sid == 0
                                        and is_lastclone_for_pktloss == 0
                                        and tmp_server_sid == 0
                                    ):
                                        matchspec0 = [
                                            hex(CACHE_EVICT_LOADFREQ_INSWITCH),
                                            hex(is_cached),
                                            hex(is_report),
                                            hex(is_latest),
                                            hex(is_deleted),
                                            hex(tmp_client_sid),
                                            hex(is_lastclone_for_pktloss),
                                            hex(tmp_server_sid),
                                        ]
                                        # Update CACHE_EVICT_LOADFREQ_INSWITCH as CACHE_EVICT_LOADFREQ_INSWITCH_ACK to reflector (w/ frequency)
                                        actnspec0 = [
                                            hex(self.reflector_sid),
                                            hex(reflector_dp2cpserver_port),
                                        ]
                                        self.controller.table_add(
                                            "eg_port_forward_tbl",
                                            "update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone",
                                            matchspec0,
                                            actnspec0,
                                        )
                                    # is_cached=0 (no inswitch_hdr after clone_e2e), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH_ACK
                                    # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                    # size: 0
                                    if (
                                        is_cached == 0
                                        and is_report == 0
                                        and is_latest == 0
                                        and is_deleted == 0
                                        and tmp_client_sid == 0
                                        and is_lastclone_for_pktloss == 0
                                        and tmp_server_sid == 0
                                    ):
                                        matchspec0 = [
                                            hex(CACHE_EVICT_LOADFREQ_INSWITCH_ACK),
                                            hex(is_cached),
                                            hex(is_report),
                                            hex(is_latest),
                                            hex(is_deleted),
                                            hex(tmp_client_sid),
                                            hex(is_lastclone_for_pktloss),
                                            hex(tmp_server_sid),
                                        ]
                                        # Forward CACHE_EVICT_LOADFREQ_INSWITCH_ACK (by clone_e2e) to reflector
                                        # self.client.eg_port_forward_tbl_table_add_with_forward_cache_evict_loadfreq_inswitch_ack(\
                                        #        self.sess_hdl, self.dev_tgt, matchspec0)
                                        # NOTE: default action is NoAction -> forward the packet to sid set by clone_e2e
                                        pass

                                    # is_report=0, tmp_client_sid=0, is_lastclone_for_pktloss=0, tmp_server_sid=0 for NETCACHE_VALUEUPDATE_INSWITCH
                                    if (
                                        is_report == 0
                                        and tmp_client_sid == 0
                                        and is_lastclone_for_pktloss == 0
                                        and tmp_server_sid == 0
                                    ):
                                        # size: 8
                                        matchspec0 = [
                                            hex(DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN),
                                            hex(is_cached),
                                            hex(is_report),
                                            hex(is_latest),
                                            hex(is_deleted),
                                            hex(tmp_client_sid),
                                            hex(is_lastclone_for_pktloss),
                                            hex(tmp_server_sid),
                                        ]
                                        # Update DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN as DISTCACHE_VALUEUPDATE_INSWITCH_ACK to server
                                        self.controller.table_add(
                                            "eg_port_forward_tbl",
                                            "update_distcache_valueupdate_inswitch_origin_to_distcache_valueupdate_inswitch_ack",
                                            matchspec0,
                                        )
                                    # is_report=0, is_deleted=0, tmp_client_sid=0, is_lastclone_for_pktloss=0, tmp_server_sid=0 for DISTCACHE_INVALIDATE_INSWITCH
                                    if (
                                        is_report == 0
                                        and is_deleted == 0
                                        and tmp_client_sid == 0
                                        and is_lastclone_for_pktloss == 0
                                        and tmp_server_sid == 0
                                    ):
                                        # size: 4
                                        matchspec0 = [
                                            hex(DISTCACHE_INVALIDATE_INSWITCH),
                                            hex(is_cached),
                                            hex(is_report),
                                            hex(is_latest),
                                            hex(is_deleted),
                                            hex(tmp_client_sid),
                                            hex(is_lastclone_for_pktloss),
                                            hex(tmp_server_sid),
                                        ]
                                        # Update DISTCACHE_INVALIDATE_INSWITCH as DISTCACHE_INVALIDATE_ACK to server
                                        self.controller.table_add(
                                            "eg_port_forward_tbl",
                                            "update_distcache_invalidate_inswitch_to_distcache_invalidate_ack",
                                            matchspec0,
                                        )
                                    # is_report=0, is_deleted=0, tmp_client_sid=0, is_lastclone_for_pktloss=0, tmp_server_sid=0 for PUTREQ_LARGEVALUE_INSWITCH
                                    if (
                                        is_report == 0
                                        and is_deleted == 0
                                        and tmp_client_sid == 0
                                        and is_lastclone_for_pktloss == 0
                                        and tmp_server_sid == 0
                                    ):
                                        # size: 4
                                        matchspec0 = [
                                            hex(PUTREQ_LARGEVALUE_INSWITCH),
                                            hex(is_cached),
                                            hex(is_report),
                                            hex(is_latest),
                                            hex(is_deleted),
                                            hex(tmp_client_sid),
                                            hex(is_lastclone_for_pktloss),
                                            hex(tmp_server_sid),
                                        ]
                                        if is_cached == 0:
                                            # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ to server-leaf
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_putreq_largevalue_inswitch_to_putreq_largevalue_seq",
                                                matchspec0,
                                            )
                                        elif is_cached == 1:
                                            # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ_CACHED to server-leaf
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_cached",
                                                matchspec0,
                                            )

    def configure_eg_port_forward_tbl_with_range(self):
        # Table: eg_port_forward_tbl (default: NoAction; size: 27+852*client_physical_num+2*server_physical_num=27+854*2=1735 < 2048 < 21+854*8=6859 < 8192)
        tmp_client_sids = [0, self.clientleafswitch_sid]
        tmp_server_sids = [0, self.serverleafswitch_sid]
        for is_cached in cached_list:
            for is_report in report_list:
                for is_latest in latest_list:
                    for is_deleted in deleted_list:
                        # Use tmpstat as action data to reduce action number
                        if is_deleted == 1:
                            tmpstat = 0
                        else:
                            tmpstat = 1
                        # NOTE: eg_intr_md.egress_port is read-only
                        # for is_wrong_pipeline in pipeline_list:
                        # for tmp_client_sid in self.sids:
                        for tmp_client_sid in tmp_client_sids:
                            for is_lastclone_for_pktloss in lastclone_list:
                                for is_last_scansplit in [0, 1]:
                                    for (
                                        tmp_server_sid
                                    ) in (
                                        tmp_server_sids
                                    ):  # Only work for NETCACHE_GETREQ_POP and SCANREQ_SPLIT
                                        # is_report=0, is_latest=0, is_deleted=0, is_lastclone_for_pktloss=0, is_last_scansplit=0, tmp_server_sid=0 for NETCACHE_WARMUPREQ_INSWITCH
                                        # NOTE: tmp_server_sid must be 0 as the last NETCACHE_WARMUPREQ_INSWITCH_POP is cloned as WARMUPACK to client instead of server
                                        if (
                                            is_report == 0
                                            and is_latest == 0
                                            and is_deleted == 0
                                            and is_lastclone_for_pktloss == 0
                                            and is_last_scansplit == 0
                                            and tmp_server_sid == 0
                                            and tmp_client_sid != 0
                                        ):
                                            # size: 2*client_physical_num=4 < 2*8=16
                                            matchspec0 = [
                                                hex(NETCACHE_WARMUPREQ_INSWITCH),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            # Update NETCACHE_WARMUP_INSWITCH as NETCACHE_WARMUP_INSWITCH_POP to switchos by cloning
                                            actnspec0 = [
                                                hex(self.reflector_sid),
                                                hex(reflector_dp2cpserver_port),
                                            ]
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_netcache_warmupreq_inswitch_to_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack",
                                                matchspec0,
                                                actnspec0,
                                            )
                                        # is_report=0, is_latest=0, is_deleted=0, is_last_scansplit=0, tmp_server_sid=0 for NETCACHE_WARMUPREQ_INSWITCH_POP
                                        if (
                                            is_report == 0
                                            and is_latest == 0
                                            and is_deleted == 0
                                            and is_last_scansplit == 0
                                            and tmp_server_sid == 0
                                            and tmp_client_sid != 0
                                        ):
                                            # size: 4*client_physical_num=8 < 4*8=32
                                            matchspec0 = [
                                                hex(NETCACHE_WARMUPREQ_INSWITCH_POP),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            if is_lastclone_for_pktloss == 0:
                                                # Forward NETCACHE_WARMUP_INSWITCH_POP to switchos and clone
                                                actnspec0 = [hex(self.reflector_sid)]
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "forward_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack",
                                                    matchspec0,
                                                    actnspec0,
                                                )
                                            elif is_lastclone_for_pktloss == 1:
                                                # Update NETCACHE_WARMUP_INSWITCH_POP as WARMUPACK to client by mirroring
                                                # NOTE: WARMUPACK performs default action NoAction() to be forwarded to client
                                                actnspec0 = [
                                                    hex(tmp_client_sid),
                                                    hex(server_worker_port_start),
                                                ]
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_netcache_warmupreq_inswitch_pop_to_warmupack_by_mirroring",
                                                    matchspec0,
                                                    actnspec0,
                                                )
                                        # is_lastclone_for_pktloss and is_last_scansplit should be 0 for GETREQ_INSWITCH
                                        # NOTE: tmp_client_sid != 0 to prepare for cache hit; tmp_server_sid != 0 to prepare for cache pop (clone last NETCACHE_GETREQ_POP as GETREQ to server)
                                        if (
                                            is_report == 0
                                            and is_lastclone_for_pktloss == 0
                                            and is_last_scansplit == 0
                                            and tmp_server_sid != 0
                                            and tmp_client_sid != 0
                                        ):
                                            # size: 32*client_physical_num*server_physical_num=128 < 32*8*8=2048
                                            matchspec0 = [
                                                hex(GETREQ_INSWITCH),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            if is_cached == 0:
                                                # Update GETREQ_INSWITCH as GETREQ_SPINE to server-leaf
                                                # actnspec0 = [self.devPorts[1]]
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_getreq_inswitch_to_getreq_spine",
                                                    matchspec0,
                                                )
                                            else:
                                                if is_latest == 0:
                                                    # Update GETREQ_INSWITCH as GETREQ_SPINE to server-leaf
                                                    # actnspec0 = [self.devPorts[1]]
                                                    self.controller.table_add(
                                                        "eg_port_forward_tbl",
                                                        "update_getreq_inswitch_to_getreq_spine",
                                                        matchspec0,
                                                    )
                                                else:  # is_cached == 1 and is_latest == 1
                                                    ## Update GETREQ_INSWITCH as DISTCACHE_GETRES_SPINE to client by mirroring
                                                    # actnspec0 = [tmp_client_sid, server_worker_port_start, tmpstat]
                                                    # self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_distcache_getres_spine_by_mirroring(\
                                                    #        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    # Update GETREQ_INSWITCH as GETRES to client by mirroring
                                                    actnspec0 = [
                                                        hex(tmp_client_sid),
                                                        hex(server_worker_port_start),
                                                        hex(tmpstat),
                                                    ]
                                                    self.controller.table_add(
                                                        "eg_port_forward_tbl",
                                                        "update_getreq_inswitch_to_getres_by_mirroring",
                                                        matchspec0,
                                                        actnspec0,
                                                    )
                                        # is_cached=0 (memset inswitch_hdr by end-host, and key must not be cached in cache_lookup_tbl for CACHE_POP_INSWITCH), is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss should be 0 for CACHE_POP_INSWITCH
                                        # is_last_scansplit and tmp_server_sid must be 0
                                        # size: 4
                                        # if is_cached == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                        if (
                                            is_cached == 0
                                            and is_report == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and is_last_scansplit == 0
                                            and tmp_server_sid == 0
                                        ):
                                            matchspec0 = [
                                                hex(CACHE_POP_INSWITCH),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            # Update CACHE_POP_INSWITCH as CACHE_POP_INSWITCH_ACK to reflector (deprecated: w/ clone)
                                            actnspec0 = [
                                                hex(self.reflector_sid),
                                                hex(reflector_dp2cpserver_port),
                                            ]
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone",
                                                matchspec0,
                                                actnspec0,
                                            )
                                        # is_cached=0 (no inswitch_hdr after clone_e2e), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktloss should be 0 for CACHE_POP_INSWITCH_ACK
                                        # is_last_scansplit and tmp_server_sid must be 0
                                        # size: 0
                                        # if is_cached == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0:
                                        if (
                                            is_cached == 0
                                            and is_report == 0
                                            and is_latest == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and is_last_scansplit == 0
                                            and tmp_server_sid == 0
                                        ):
                                            matchspec0 = [
                                                hex(CACHE_POP_INSWITCH_ACK),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            # if is_lastclone_for_pktloss == 0:
                                            #    # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector (w/ clone)
                                            #    actnspec0 = [self.reflector_sid]
                                            #    self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack_clone_for_pktloss(\
                                            #            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            # elif is_lastclone_for_pktloss == 1:
                                            #    # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                            #    self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack(\
                                            #            self.sess_hdl, self.dev_tgt, matchspec0)

                                            # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                            # self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack(\
                                            #        self.sess_hdl, self.dev_tgt, matchspec0)
                                            # NOTE: default action is NoAction -> forward the packet to sid set by clone_e2e
                                            pass
                                        # is_deleted, tmp_client_sid, is_lastclone_for_pktloss should be 0 for PUTREQ_INSWITCH
                                        # is_last_scansplit and tmp_server_sid must be 0
                                        # size: 4
                                        if (
                                            is_report == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and is_last_scansplit == 0
                                            and tmp_server_sid == 0
                                        ):
                                            matchspec0 = [
                                                hex(PUTREQ_INSWITCH),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            if is_cached == 0:
                                                # Update PUTREQ_INSWITCH as PUTREQ_SEQ to server
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_putreq_inswitch_to_putreq_seq",
                                                    matchspec0,
                                                )
                                            elif is_cached == 1:
                                                # Update PUTREQ_INSWITCH as NETCACHE_PUTREQ_SEQ_CACHED to server
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_putreq_inswitch_to_netcache_putreq_seq_cached",
                                                    matchspec0,
                                                )
                                        # (cm_predicate=1), is_deleted, tmp_client_sid, is_lastclone_for_pktloss should be 0 for DELREQ_INSWITCH
                                        # is_last_scansplit and tmp_server_sid must be 0
                                        # size: 4
                                        if (
                                            is_report == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and is_last_scansplit == 0
                                            and tmp_server_sid == 0
                                        ):
                                            matchspec0 = [
                                                hex(DELREQ_INSWITCH),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            if is_cached == 0:
                                                # Update DELREQ_INSWITCH as DELREQ_SEQ to server
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_delreq_inswitch_to_delreq_seq",
                                                    matchspec0,
                                                )
                                            elif is_cached == 1:
                                                # Update DELREQ_INSWITCH as NETCACHE_DELREQ_SEQ_CACHED to server
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_delreq_inswitch_to_netcache_delreq_seq_cached",
                                                    matchspec0,
                                                )
                                        # is_cached=0 (no inswitch_hdr after entering egress pipeline), is_latest, is_deleted, client_sid, is_lastclone_for_pktloss must be 0 for SCANREQ_SPLIT
                                        # size: 2*server_physical_num=4 < 2*8=16
                                        if (
                                            is_cached == 0
                                            and is_report == 0
                                            and is_latest == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and tmp_server_sid != 0
                                        ):
                                            matchspec0 = [
                                                hex(SCANREQ_SPLIT),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            if is_last_scansplit == 1:
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "forward_scanreq_split",
                                                    matchspec0,
                                                )
                                            elif is_last_scansplit == 0:
                                                actnspec0 = [hex(tmp_server_sid)]
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "forward_scanreq_split_and_clone",
                                                    matchspec0,
                                                    actnspec0,
                                                )
                                        # is_cached=1 (key must be cached in cache_lookup_tbl for CACHE_EVICT_LOADFREQ_INSWITCH), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH
                                        # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                        # size: 1
                                        if (
                                            is_cached == 1
                                            and is_report == 0
                                            and is_latest == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and is_last_scansplit == 0
                                            and tmp_server_sid == 0
                                        ):
                                            matchspec0 = [
                                                hex(CACHE_EVICT_LOADFREQ_INSWITCH),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            # Update CACHE_EVICT_LOADFREQ_INSWITCH as CACHE_EVICT_LOADFREQ_INSWITCH_ACK to reflector (w/ clone)
                                            actnspec0 = [
                                                hex(self.reflector_sid),
                                                hex(reflector_dp2cpserver_port),
                                            ]
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone",
                                                matchspec0,
                                                actnspec0,
                                            )
                                        # is_cached=0 (no inswitch_hdr after clone_e2e), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH_ACK
                                        # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                        # size: 0
                                        if (
                                            is_cached == 0
                                            and is_report == 0
                                            and is_latest == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and is_last_scansplit == 0
                                            and tmp_server_sid == 0
                                        ):
                                            matchspec0 = [
                                                hex(CACHE_EVICT_LOADFREQ_INSWITCH_ACK),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            # Forward CACHE_EVICT_LOADFREQ_INSWITCH_ACK (by clone_e2e) to reflector
                                            # self.client.eg_port_forward_tbl_table_add_with_forward_cache_evict_loadfreq_inswitch_ack(\
                                            #        self.sess_hdl, self.dev_tgt, matchspec0)
                                            # NOTE: default action is NoAction -> forward the packet to sid set by clone_e2e
                                            pass

                                        # is_report=0, is_deleted=0, tmp_client_sid=0, is_lastclone_for_pktloss=0, tmp_server_sid=0 for DISTCACHE_INVALIDATE_INSWITCH
                                        if (
                                            is_report == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and tmp_server_sid == 0
                                            and is_last_scansplit == 0
                                        ):
                                            # size: 4
                                            matchspec0 = [
                                                hex(DISTCACHE_INVALIDATE_INSWITCH),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            # Update DISTCACHE_INVALIDATE_INSWITCH as DISTCACHE_INVALIDATE_ACK to server
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_distcache_invalidate_inswitch_to_distcache_invalidate_ack",
                                                matchspec0,
                                            )
                                        # is_report=0, tmp_client_sid=0, is_lastclone_for_pktloss=0, tmp_server_sid=0 for NETCACHE_VALUEUPDATE_INSWITCH
                                        # is_report=0, tmp_client_sid=0, is_lastclone_for_pktloss=0, tmp_server_sid=0 for NETCACHE_VALUEUPDATE_INSWITCH
                                        if (
                                            is_report == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and tmp_server_sid == 0
                                            and is_last_scansplit == 0
                                        ):
                                            # size: 8
                                            matchspec0 = [
                                                hex(
                                                    DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN
                                                ),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            # Update DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN as DISTCACHE_VALUEUPDATE_INSWITCH_ACK to server
                                            self.controller.table_add(
                                                "eg_port_forward_tbl",
                                                "update_distcache_valueupdate_inswitch_origin_to_distcache_valueupdate_inswitch_ack",
                                                matchspec0,
                                            )
                                        # is_report=0, is_deleted=0, tmp_client_sid=0, is_lastclone_for_pktloss=0, tmp_server_sid=0 for PUTREQ_LARGEVALUE_INSWITCH
                                        if (
                                            is_report == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and tmp_server_sid == 0
                                            and is_last_scansplit == 0
                                        ):
                                            # size: 4
                                            matchspec0 = [
                                                hex(PUTREQ_LARGEVALUE_INSWITCH),
                                                hex(is_cached),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(is_last_scansplit),
                                                hex(tmp_server_sid),
                                            ]
                                            if is_cached == 0:
                                                # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ to server-leaf
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_putreq_largevalue_inswitch_to_putreq_largevalue_seq",
                                                    matchspec0,
                                                )
                                            elif is_cached == 1:
                                                # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ_CACHED to server-leaf
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_cached",
                                                    matchspec0,
                                                )


tableconfig = TableConfigure(0)
tableconfig.setUp()
# tableconfig.clean_all_tables()
# tableconfig.create_mirror_session()
tableconfig.runTest()
