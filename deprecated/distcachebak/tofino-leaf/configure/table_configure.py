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

toleaf_predicate_list = [1, 2]
cached_list = [0, 1]
hot_list = [0, 1]
report_list = [0, 1]
latest_list = [0, 1]
stat_list = [0, 1]
deleted_list = [0, 1]
sampled_list = [0, 1]
lastclone_list = [0, 1]
access_val_mode_list = [0, 1, 2, 3]

reflector_ip_for_switchos = leaf_reflector_ip_for_switchos
reflector_dp2cpserver_port = leaf_reflector_dp2cpserver_port
reflector_cp2dpserver_port = leaf_reflector_cp2dpserver_port
reflector_cp2dp_dstip = leaf_reflector_cp2dp_dstip
reflector_cp2dp_dstmac = leaf_reflector_cp2dp_dstmac


class TableConfigure:
    def __init__(self, rack_idx):
        self.rack_idx = rack_idx
        self.controller = SimpleSwitchThriftAPI(
            9090 + rack_idx + 1, "192.168.122.229"
        )  # 9090，127.0.0.1

    def create_mirror_session(self):
        # Mirror_add
        for i in range(client_physical_num):
            print(
                "Binding sid {} with client devport {} for both direction mirroring".format(
                    self.client_sids[i], self.client_devports[i]
                )
            )  # clone to client
            self.controller.mirroring_add(self.client_sids[i], 1)
        for i in range(server_physical_num):
            print(
                "Binding sid {} with server devport {} for both direction mirroring".format(
                    self.server_sids[i % 2], self.server_devports[i]
                )
            )  # clone to server
            self.controller.mirroring_add(
                self.server_sids[i % 2], self.server_devports[i]
            )

        print(
            "Binding sid {} with spineswitch devport {} for both direction mirroring".format(
                self.spineswitch_sid, self.spineswitch_devport
            )
        )  # clone to spineswitch
        self.controller.mirroring_add(
            self.spineswitch_sid,
            self.spineswitch_devport,
        )

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

        # initialize the connection
        self.client_devports = []
        self.server_devports = []

        # get the device ports from front panel ports
        for client_fpport in client_fpports:
            port, chnl = client_fpport.split("/")
            self.client_devports.append(port)
        for server_fpport in server_fpports:
            port, chnl = server_fpport.split("/")
            self.server_devports.append(port)
        port, chnl = leafswitch_fpport_to_spine.split("/")
        self.spineswitch_devport = port

        # NOTE: in each pipeline, 64-67 are recir/cpu ports, 68-71 are recir/pktgen ports
        # self.cpuPorts = [64, 192] # CPU port is 100G
        sidnum = len(self.client_devports) + len(self.server_devports) + 1
        # sids = random.sample(range(10, 20), sidnum)
        # self.client_sids = sids[0 : len(self.client_devports)]
        # self.server_sids = sids[len(self.client_devports) : sidnum]
        self.client_sids = list(range(10, 10 + client_physical_num))
        self.server_sids = list(range(20, 20 + client_physical_num))
        self.spineswitch_sid = 31

        # NOTE: data plane communicate with switchos by software-based reflector, which is deployed in one server machine
        isvalid = False
        for i in range(server_physical_num):
            if reflector_ip_for_switchos == server_ip_for_controller_list[i]:
                isvalid = True
                self.reflector_ip_for_switch = server_ips[i]
                self.reflector_mac_for_switch = server_macs[i]
                self.reflector_devport = self.server_devports[i]
                self.reflector_sid = self.server_sids[
                    i
                ]  # clone to switchos (i.e., reflector at [the first] physical server)
        if isvalid == False:
            for i in range(client_physical_num):
                if reflector_ip_for_switchos == client_ip_for_client0_list[i]:
                    isvalid = True
                    self.reflector_ip_for_switch = client_ips[i]
                    self.reflector_mac_for_switch = client_macs[i]
                    self.reflector_devport = self.client_devports[i]
                    self.reflector_sid = self.client_sids[i]
        if isvalid == False:
            print("[ERROR] invalid reflector configuration")
            exit(-1)

    ### MAIN ###

    def runTest(self):
        #####################
        ### Prepare ports ###
        #####################

        # Enable recirculation before add special ports

        ################################
        ### Normal MAT Configuration ###
        ################################

        # Ingress pipeline

        # Stage 0

        # Table: l2l3_forward_tbl (default: NoAction; size: client_physical_num+server_physical_num = 4 < 16)
        print("Configuring l2l3_forward_tbl")
        for i in range(client_physical_num):
            self.controller.table_add(
                "l2l3_forward_tbl",
                "l2l3_forward",
                [client_macs[i], client_ips[i] + "/32"],
                [self.client_devports[i]],
            )
        for i in range(server_physical_num):
            self.controller.table_add(
                "l2l3_forward_tbl",
                "l2l3_forward",
                [server_macs[i], server_ips[i] + "/32"],
                [self.server_devports[i]],
            )

        # Table: set_hot_threshold_tbl (default: set_hot_threshold; size: 1)
        # print("Configuring set_hot_threshold_tbl")
        # actnspec0 = [hot_threshold]
        # self.client.set_hot_threshold_tbl_set_default_action_set_hot_threshold(\
        #        self.sess_hdl, self.dev_tgt, actnspec0)

        # Table: set_hot_threshold_and_spineswitchnum_tbl (default: set_hot_threshold_and_spineswitchnum; size: 1)
        print("Configuring set_hot_threshold_and_spineswitchnum_tbl")
        actnspec0 = [hex(hot_threshold), hex(spineswitch_total_logical_num)]
        self.controller.table_set_default(
            "set_hot_threshold_and_spineswitchnum_tbl",
            "set_hot_threshold_and_spineswitchnum",
            actnspec0,
        )
        ## Table: hash_for_spineselect_tbl (default: NoAction; size: 9)
        # print("Configuring hash_for_spineselect_tbl")
        ## Deprecated: GETRES_SERVER (inherit original spineswitchidx from GETREQ set by client-leaf to update corresponding register slot in spineload_reg of spine switch)
        ##, NETCACHE_VALUEUPDATE, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH

        # Table: hash_for_spineselect_tbl (default: hash_for_spineselect(); size: 1)
        # NOTE: although we set meta.hashval_for_spineselect for all packets, spineselect_tbl ONLY uses it for specific optypes
        print("Configuring hash_for_spineselect_tbl")

        # Table: access_leafload_tbl (default: NoAction; size: 1)
        print("Configuring access_leafload_tbl")
        matchspec0 = [hex(GETREQ_SPINE)]  # GETREQ_SPINE from spine switch
        self.controller.table_add(
            "access_leafload_tbl", "set_and_get_leafload_and_hash_for_bf2", matchspec0
        )
        # matchspec0 = distcacheleaf_access_leafload_tbl_match_spec_t(\
        #        op_hdr_optype = GETRES_SERVER) # Deprecated: GETRES_SERVER from storage server

        # Table: access_spineload_forclient_tbl (default: NoAction; size: 2)
        print("Configuring access_spineload_forclient_tbl")
        # for tmpoptype in [GETRES, DISTCACHE_GETRES_SPINE]:
        for tmpoptype in [DISTCACHE_UPDATE_TRAFFICLOAD]:
            matchspec0 = [hex(tmpoptype)]
        self.controller.table_add(
            "access_spineload_forclient_tbl", "set_spineload_forclient", matchspec0
        )
        matchspec0 = [hex(GETREQ)]
        self.controller.table_add(
            "access_spineload_forclient_tbl", "get_spineload_forclient", matchspec0
        )

        # Table: set_spineswitchnum_tbl (default: NoAction; size: 1)
        # matchspec0 = distcacheleaf_set_spineswitchnum_match_spec_t(\
        #        op_hdr_optype = GETREQ)
        # actnspec0 = distcacheleaf_set_spineswitchnum(spineswitch_total_logical_num)

        # Stage 1

        ## Table: hash_for_ecmp_tbl (default: NoAction; size: 1)
        # print("Configuring hash_for_ecmp_tbl")
        # matchspec0 = distcacheleaf_hash_for_ecmp_tbl_match_spec_t(\
        #        op_hdr_optype = GETREQ)

        print("Configuring hash_for_ecmp_and_partition_tbl")

        # Stage 2

        # Table: access_leafload_forclient_tbl (default: reset_meta_toleaf_predicate(); size: 2)
        print("Configuring access_leafload_forclient_tbl")
        # for tmpoptype in [GETRES]:
        for tmpoptype in [DISTCACHE_UPDATE_TRAFFICLOAD]:
            matchspec0 = [hex(tmpoptype)]
        self.controller.table_add(
            "access_leafload_forclient_tbl", "set_leafload_forclient", matchspec0
        )
        matchspec0 = [hex(GETREQ)]
        self.controller.table_add(
            "access_leafload_forclient_tbl", "get_leafload_forclient", matchspec0
        )

        # Table: ecmp_for_getreq_tbl (default: NoAction; size: spineswitch_total_logical_num - 1)
        print("Configuring ecmp_for_getreq_tbl")
        if spineswitch_total_logical_num <= 1:
            print(
                "spineswitch_total_logical_num = {}, which must >= 2 for power-of-two-choices".format(
                    spineswitch_total_logical_num
                )
            )
            exit(-1)
        offsetnum = (
            spineswitch_total_logical_num - 1
        )  # [1, spineswitch_total_logical_num - 1]
        key_range_per_offset = switch_partition_count / offsetnum
        key_start = 0  # [0, 2^16-1]
        for i in range(
            offsetnum
        ):  # [0, offsetnum-1] = [0, spineswitch_total_logicalnum-2]
            if i == offsetnum - 1:
                key_end = switch_partition_count - 1
            else:
                key_end = key_start + key_range_per_offset - 1
            # NOTE: both start and end are included
            matchspec0 = [
                hex(GETREQ),
                "" + hex(key_start) + "->" + hex(key_end),
            ]
            actnspec0 = [
                hex(i + 1)
            ]  # [1, offsetnum] = [1, spineswitch_total_logicalnum-1]
            self.controller.table_add(
                "ecmp_for_getreq_tbl", "set_toleaf_offset", matchspec0, actnspec0, 0
            )  # 0 is priority (range may be overlapping)
            key_start = key_end + 1

        # Stage 3

        # Table: spineselect_tbl (default: NoAction; size <= 10 * spineswitch_total_logical_num)
        print("Configuring spineselect_tbl")
        key_range_per_spineswitch = int(
            switch_partition_count / spineswitch_total_logical_num
        )
        # Deprecated: GETRES_SERVER (inherit original spineswitchidx from GETREQ set by client-leaf to update corresponding register slot in spineload_reg of spine switch)
        # , NETCACHE_VALUEUPDATE, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH
        for tmpoptype in [
            PUTREQ,
            DELREQ,
            SCANREQ,
            WARMUPREQ,
            LOADREQ,
            DISTCACHE_INVALIDATE,
            DISTCACHE_VALUEUPDATE_INSWITCH,
            PUTREQ_LARGEVALUE,
        ]:
            key_start = 0  # [0, 2^16-1]
            for i in range(spineswitch_total_logical_num):
                global_spineswitch_logical_idx = spineswitch_logical_idxes[i]
                if i == spineswitch_total_logical_num - 1:
                    key_end = switch_partition_count - 1
                else:
                    key_end = key_start + key_range_per_spineswitch - 1
                # NOTE: both start and end are included
                matchspec0 = [
                    hex(tmpoptype),
                    hex(1),
                    "" + hex(key_start) + "->" + hex(key_end),
                ]
                # if tmpoptype == GETRES_SERVER:
                #    # server-leaf sets spineswitchidx to read spineload_reg (NOTE: pkt will be forwarded to spine switch based on dstip in ipv4_forward_tbl later)
                #    actnspec0 = [global_spineswitch_logical_idx]
                #    self.client.spineselect_tbl_table_add_with_spineselect_for_getres_server(\
                #            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                # elif tmpoptype == DISTCACHE_INVALIDATE:
                if tmpoptype == DISTCACHE_INVALIDATE:
                    # server-leaf sets spineswitchidx to invalidate in-spine-switch value (NOTE: pkt will be forwarded to server pipeline by range/hash_partition_tbl; another pkt will be cloned to spine switch by ipv4_forward_tbl -> it is OK as we have set spineswitchidx to simulate multiple spine switches)
                    # TODO: we can also clone the pkt to spine switch in spineselect_tbl based on key
                    actnspec0 = [hex(global_spineswitch_logical_idx)]
                    self.controller.table_add(
                        "spineselect_tbl",
                        "spineselect_for_distcache_invalidate",
                        matchspec0,
                        actnspec0,
                        0,
                    )  # 0 is priority (range may be overlapping)
                # elif tmpoptype == NETCACHE_VALUEUPDATE:
                #    # server-leaf sets spineswitchidx to update in-spine-switch value (NOTE: pkt will be forwarded to server pipeline by range/hash_partition_tbl; another pkt will be cloned to spine switch by ipv4_forward_tbl -> it is OK as we have set spineswitchidx to simulate multiple spine switches)
                #    # TODO: we can also clone the pkt to spine switch in spineselect_tbl based on key
                #    actnspec0 = [global_spineswitch_logical_idx]
                #    self.client.spineselect_tbl_table_add_with_spineselect_for_netcache_valueupdate(\
                #            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                # elif tmpoptype == NETCACHE_VALUEUPDATE:
                #    # Directly from ingress to spine yet bypass egress
                #    actnspec0 = [eport, global_spineswitch_logical_idx]
                #    self.client.spineselect_tbl_table_add_with_spineselect_for_distcache_spine_valueupdate_inswitch(\
                #            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                elif tmpoptype == DISTCACHE_VALUEUPDATE_INSWITCH:
                    # server-leaf sets spineswitchidx to update in-spine-switch value (NOTE: pkt will be forwarded to server pipeline by range/hash_partition_tbl; another pkt will be cloned to spine switch by ipv4_forward_tbl -> it is OK as we have set spineswitchidx to simulate multiple spine switches)
                    # TODO: we can also clone the pkt to spine switch in spineselect_tbl based on key
                    actnspec0 = [hex(global_spineswitch_logical_idx)]
                    self.controller.table_add(
                        "spineselect_tbl",
                        "spineselect_for_distcache_valueupdate_inswitch",
                        matchspec0,
                        actnspec0,
                        0,
                    )  # 0 is priority (range may be overlapping)
                else:
                    # client-leaf forwards to the egress pipeline of spine switch by independent hashing (NOT touch any MAT in egress pipelines)
                    eport = self.spineswitch_devport
                    actnspec0 = [eport, hex(global_spineswitch_logical_idx)]
                    self.controller.table_add(
                        "spineselect_tbl", "spineselect", matchspec0, actnspec0, 0
                    )  # 0 is priority (range may be overlapping)
                key_start = key_end + 1
        for tmp_toleaf_predicate in toleaf_predicate_list:
            key_start = 0  # [0, 2^16-1]
            for i in range(spineswitch_total_logical_num):
                global_spineswitch_logical_idx = spineswitch_logical_idxes[i]
                if i == spineswitch_total_logical_num - 1:
                    key_end = switch_partition_count - 1
                else:
                    key_end = key_start + key_range_per_spineswitch - 1
                # NOTE: both start and end are included
                matchspec0 = [
                    hex(GETREQ),
                    hex(tmp_toleaf_predicate),
                    hex(key_start) + "->" + hex(key_end),
                ]
                if tmp_toleaf_predicate == 1:
                    # client-leaf forwards to the egress pipeline of spine switch by independent hashing (NOT touch any MAT in egress pipelines) w/ correct spineswitchidx
                    eport = self.spineswitch_devport
                    actnspec0 = [eport, hex(global_spineswitch_logical_idx)]
                    self.controller.table_add(
                        "spineselect_tbl", "spineselect", matchspec0, actnspec0, 0
                    )  # 0 is priority (range may be overlapping)
                elif tmp_toleaf_predicate == 2:
                    # client-leaf forwards to the egress pipeline of spine switch by independent hashing (NOT touch any MAT in egress pipelines) w/ incorrect spineswitchidx to avoid spine-switch cache hit
                    eport = self.spineswitch_devport

                    # Method A: incorrect = correct + 1
                    # incorrect_global_spineswitch_logical_idx = global_spineswitch_logical_idx + 1 # [0, spineswitch_total_logical_num - 1] -> [1, spineswitch_total_logical_num]
                    # if incorrect_global_spineswitch_logical_idx == spineswitch_total_logical_num:
                    #    incorrect_global_spineswitch_logical_idx = 0
                    # actnspec0 = [eport, incorrect_global_spineswitch_logical_idx]
                    # self.client.spineselect_tbl_table_add_with_spineselect(\
                    #        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)

                    # Method B: incorrect = correct + toleaf_offset
                    actnspec0 = [eport, hex(global_spineswitch_logical_idx)]
                    self.controller.table_add(
                        "spineselect_tbl",
                        "spineselect_for_getreq_toleaf",
                        matchspec0,
                        actnspec0,
                        0,
                    )
                key_start = key_end + 1

        # Stage 4

        # Table: cutoff_spineswitchidx_for_ecmp_tbl (default: NoAction; size: spineswitch_total_logical_num - 1)
        print("Configuring cutoff_spineswitchidx_for_ecmp_tbl")
        for tmp_spineswitchidx in range(
            spineswitch_total_logical_num, 2 * spineswitch_total_logical_num - 2 + 1
        ):
            matchspec0 = [hex(GETREQ), str(tmp_spineswitchidx)]
            # actnspec0 = [spineswitch_total_logical_num]
            self.controller.table_add(
                "cutoff_spineswitchidx_for_ecmp_tbl",
                "cutoff_spineswitchidx_for_ecmp",
                matchspec0,
            )

        # Stage 4~5

        # Table: cache_lookup_tbl (default: uncached_action; size: 32K/64K)
        print("Leave cache_lookup_tbl managed by controller in runtime")

        # Stage 6~7

        # Table: hash_partition_tbl (default: NoAction; size <= 15 * 128)
        print("Configuring hash_partition_tbl")
        hash_range_per_server = int(switch_partition_count / server_total_logical_num)
        # , NETCACHE_VALUEUPDATE, NETCACHE_VALUEUPDATE_ACK, DISTCACHE_LEAF_VALUEUPDATE_INSWITCH, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK
        for tmpoptype in [
            GETREQ_SPINE,
            CACHE_POP_INSWITCH,
            PUTREQ_SEQ,
            NETCACHE_PUTREQ_SEQ_CACHED,
            DELREQ_SEQ,
            NETCACHE_DELREQ_SEQ_CACHED,
            LOADREQ_SPINE,
            CACHE_EVICT_LOADFREQ_INSWITCH,
            SETVALID_INSWITCH,
            DISTCACHE_INVALIDATE,
            DISTCACHE_INVALIDATE_ACK,
            DISTCACHE_VALUEUPDATE_INSWITCH,
            DISTCACHE_VALUEUPDATE_INSWITCH_ACK,
            PUTREQ_LARGEVALUE_SEQ,
            PUTREQ_LARGEVALUE_SEQ_CACHED,
        ]:
            hash_start = 0  # [0, partition_count-1]
            for global_server_logical_idx in range(server_total_logical_num):
                if global_server_logical_idx == server_total_logical_num - 1:
                    hash_end = (
                        switch_partition_count - 1
                    )  # if end is not included, then it is just processed by port 1111
                else:
                    hash_end = hash_start + hash_range_per_server - 1
                # NOTE: both start and end are included
                matchspec0 = [
                    hex(tmpoptype),
                    "" + hex(hash_start) + "->" + hex(hash_end),
                ]
                # Forward to the egress pipeline of server
                server_physical_idx = -1
                local_server_logical_idx = -1
                for tmp_server_physical_idx in range(server_physical_num):
                    for tmp_local_server_logical_idx in range(
                        len(server_logical_idxes_list[tmp_server_physical_idx])
                    ):
                        if (
                            global_server_logical_idx
                            == server_logical_idxes_list[tmp_server_physical_idx][
                                tmp_local_server_logical_idx
                            ]
                        ):
                            server_physical_idx = tmp_server_physical_idx
                            local_server_logical_idx = tmp_local_server_logical_idx
                            break
                if server_physical_idx == -1:
                    print(
                        "WARNING: no physical server covers global_server_logical_idx {} -> no corresponding MAT entries in hash_partition_tbl".format(
                            global_server_logical_idx
                        )
                    )
                else:
                    # udp_dstport = server_worker_port_start + global_server_logical_idx
                    udp_dstport = server_worker_port_start + local_server_logical_idx
                    eport = self.server_devports[server_physical_idx]
                    if tmpoptype == DISTCACHE_INVALIDATE:
                        actnspec0 = [eport]
                        self.controller.table_add(
                            "hash_partition_tbl",
                            "hash_partition_for_distcache_invalidate",
                            matchspec0,
                            actnspec0,
                            0,
                        )
                        # 0 is priority (range may be overlapping)
                    elif tmpoptype == DISTCACHE_INVALIDATE_ACK:
                        actnspec0 = [eport]
                        self.controller.table_add(
                            "hash_partition_tbl",
                            "hash_partition_for_distcache_invalidate_ack",
                            matchspec0,
                            actnspec0,
                            0,
                        )
                        # 0 is priority (range may be overlapping)
                    # elif tmpoptype == NETCACHE_VALUEUPDATE:
                    #    actnspec0 = [eport]
                    #    self.client.hash_partition_tbl_table_add_with_hash_partition_for_netcache_valueupdate(\
                    #            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                    # elif tmpoptype == NETCACHE_VALUEUPDATE_ACK:
                    #    actnspec0 = [eport]
                    #    self.client.hash_partition_tbl_table_add_with_hash_partition_for_netcache_valueupdate_ack(\
                    #            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                    # elif tmpoptype == DISTCACHE_LEAF_VALUEUPDATE_INSWITCH:
                    #    actnspec0 = [eport]
                    #    self.client.hash_partition_tbl_table_add_with_hash_partition_for_distcache_leaf_valueupdate_inswitch(\
                    #            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                    # elif tmpoptype == DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK:
                    #    actnspec0 = [eport]
                    #    self.client.hash_partition_tbl_table_add_with_hash_partition_for_distcache_spine_valueupdate_inswitch_ack(\
                    #            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (hash may be overlapping)
                    elif tmpoptype == DISTCACHE_VALUEUPDATE_INSWITCH:
                        actnspec0 = [eport]
                        self.controller.table_add(
                            "hash_partition_tbl",
                            "hash_partition_for_distcache_valueupdate_inswitch",
                            matchspec0,
                            actnspec0,
                            0,
                        )
                        # 0 is priority (range may be overlapping)
                    elif tmpoptype == DISTCACHE_VALUEUPDATE_INSWITCH_ACK:
                        actnspec0 = [eport]
                        self.controller.table_add(
                            "hash_partition_tbl",
                            "hash_partition_for_distcache_valueupdate_inswitch_ack",
                            matchspec0,
                            actnspec0,
                            0,
                        )
                        # 0 is priority (range may be overlapping)
                    else:
                        actnspec0 = [str(udp_dstport), eport]
                        self.controller.table_add(
                            "hash_partition_tbl",
                            "hash_partition",
                            matchspec0,
                            actnspec0,
                            0,
                        )
                        # 0 is priority (range may be overlapping)
                hash_start = hash_end + 1

        # Stage 8

        ## Table: hash_for_cm12/34_tbl (default: NoAction; size: 4)
        # Table: hash_for_cm12/34_tbl (default: hash_for_cm12/34(); size: 1)
        # NOTE: although we set inswitch_hdr.hashval_for_cm1/2/3/4 for all packets, access_cmX_tbl ONLY uses it for specific optypes
        for funcname in ["12", "34"]:
            print("Configuring hash_for_cm{}_tbl".format(funcname))
            # for tmpoptype in [GETREQ_SPINE, PUTREQ_SEQ, PUTREQ_SEQ_INSWITCH]:
            #    matchspec0 = eval("distcacheleaf_hash_for_cm{}_tbl_match_spec_t".format(funcname))(\
            #            op_hdr_optype = tmpoptype)
            #    eval("self.client.hash_for_cm{}_tbl_table_add_with_hash_for_cm{}".format(funcname, funcname))(\
            #            self.sess_hdl, self.dev_tgt, matchspec0)

        # Table: hash_for_bfX_tbl (default: NoAction; size: 1)
        # for funcname in [1, 2, 3]:
        #    print("Configuring hash_for_bf{}_tbl".format(funcname))
        #    for tmpoptype in [GETREQ_SPINE]:
        #        matchspec0 = eval("distcacheleaf_hash_for_bf{}_tbl_match_spec_t".format(funcname))(\
        #                op_hdr_optype = tmpoptype)
        #        eval("self.client.hash_for_bf{}_tbl_table_add_with_hash_for_bf{}".format(funcname, funcname))(\
        #                self.sess_hdl, self.dev_tgt, matchspec0)

        # Stage 9

        # Table: prepare_for_cachehit_and_hash_for_bf1_tbl (default: reset_client_sid(); size: 2*client_physical_num=4 < 2*8=16 < 32)
        print("Configuring prepare_for_cachehit_tbl")
        for client_physical_idx in range(client_physical_num):
            tmp_clientip = client_ips[client_physical_idx]
            for tmpoptype in [GETREQ_SPINE]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + client_ips[client_physical_idx] + "/32",
                ]
                # ig_intr_md_ingress_port = self.spineswitch_devport)
                actnspec0 = [str(self.spineswitch_sid)]
        self.controller.table_add(
            "prepare_for_cachehit_and_hash_for_bf1_tbl",
            "set_client_sid_and_hash_for_bf1",
            matchspec0,
            actnspec0,
        )
        print("Configuring prepare_for_cachehit_tbl")

        # Table: ipv4_forward_tbl (default: NoAction; size: 15*client_physical_num=30 < 15*8=120)
        print("Configuring ipv4_forward_tbl")
        for tmp_client_physical_idx in range(client_physical_num):
            eport = self.client_devports[tmp_client_physical_idx]
            tmpsid = self.client_sids[tmp_client_physical_idx]
            # for tmpoptype in [GETRES, DISTCACHE_GETRES_SPINE, PUTRES, DELRES, WARMUPACK, SCANRES_SPLIT, LOADACK]:
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
            eport = self.spineswitch_devport
            tmpsid = str(self.spineswitch_sid)
            for tmpoptype in [
                GETRES_SERVER,
                SCANRES_SPLIT_SERVER,
                PUTRES_SERVER,
                DELRES_SERVER,
                LOADACK_SERVER,
                GETRES_LARGEVALUE_SERVER,
            ]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + client_ips[tmp_client_physical_idx] + "/32",
                ]
                actnspec0 = [eport]
                self.controller.table_add(
                    "ipv4_forward_tbl", "forward_normal_response", matchspec0, actnspec0
                )
            matchspec0 = [
                hex(DISTCACHE_INVALIDATE),
                "" + client_ips[tmp_client_physical_idx] + "/32",
            ]
            actnspec0 = [tmpsid]
            self.controller.table_add(
                "ipv4_forward_tbl",
                "forward_distcache_invalidate_to_server_and_clone_to_spine",
                matchspec0,
                actnspec0,
            )
            matchspec0 = [
                hex(DISTCACHE_VALUEUPDATE_INSWITCH),
                "" + client_ips[tmp_client_physical_idx] + "/32",
            ]
            actnspec0 = [tmpsid]
            self.controller.table_add(
                "ipv4_forward_tbl",
                "forward_distcache_valueupdate_inswitch_to_server_and_clone_to_spine",
                matchspec0,
                actnspec0,
            )

        # Stage 10

        ## Table: sample_tbl (default: NoAction; size: 1)
        # Table: sample_tbl (default: sample(); size: 1)
        # NOTE: although we set inswitch_hdr.is_sampled for all packets, CM/cache_frequency MATs ONLY use it for specific optypes
        print("Configuring sample_tbl")
        # for tmpoptype in [GETREQ_SPINE]:
        #    matchspec0 = distcacheleaf_sample_tbl_match_spec_t(\
        #            op_hdr_optype = tmpoptype)
        #    self.client.sample_tbl_table_add_with_sample(\
        #            self.sess_hdl, self.dev_tgt, matchspec0)

        # Table: ig_port_forward_tbl (default: NoAction; size: 16)
        print("Configuring ig_port_forward_tbl")
        matchspec0 = [hex(GETREQ_SPINE)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_getreq_spine_to_getreq_inswitch_and_hash_for_bf3",
            matchspec0,
        )
        matchspec0 = [hex(PUTREQ_SEQ)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_putreq_seq_to_putreq_seq_inswitch",
            matchspec0,
        )
        matchspec0 = [hex(NETCACHE_PUTREQ_SEQ_CACHED)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_netcache_putreq_seq_cached_to_putreq_seq_inswitch",
            matchspec0,
        )
        matchspec0 = [hex(DELREQ_SEQ)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_delreq_seq_to_delreq_seq_inswitch",
            matchspec0,
        )
        matchspec0 = [hex(NETCACHE_DELREQ_SEQ_CACHED)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_netcache_delreq_seq_cached_to_delreq_seq_inswitch",
            matchspec0,
        )
        # matchspec0 = distcacheleaf_ig_port_forward_tbl_match_spec_t(\
        #        op_hdr_optype = NETCACHE_VALUEUPDATE)
        # self.client.ig_port_forward_tbl_table_add_with_update_netcache_valueupdate_to_netcache_valueupdate_inswitch(\
        #        self.sess_hdl, self.dev_tgt, matchspec0)
        # matchspec0 = distcacheleaf_ig_port_forward_tbl_match_spec_t(\
        #        op_hdr_optype = DISTCACHE_LEAF_VALUEUPDATE_INSWITCH)
        # self.client.ig_port_forward_tbl_table_add_with_swap_udpport_for_distcache_leaf_valueupdate_inswitch(\
        #        self.sess_hdl, self.dev_tgt, matchspec0)
        matchspec0 = [hex(DISTCACHE_VALUEUPDATE_INSWITCH)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_distcache_valueupdate_inswitch_to_distcache_valueupdate_inswitch_origin",
            matchspec0,
        )
        matchspec0 = [hex(GETRES_SERVER)]
        self.controller.table_add(
            "ig_port_forward_tbl", "update_getres_server_to_getres", matchspec0
        )
        matchspec0 = [hex(SCANRES_SPLIT_SERVER)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_scanres_split_server_to_scanres_split",
            matchspec0,
        )
        matchspec0 = [hex(PUTRES_SERVER)]
        self.controller.table_add(
            "ig_port_forward_tbl", "update_putres_server_to_putres", matchspec0
        )
        matchspec0 = [hex(DELRES_SERVER)]
        self.controller.table_add(
            "ig_port_forward_tbl", "update_delres_server_to_delres", matchspec0
        )
        matchspec0 = [hex(LOADREQ_SPINE)]
        self.controller.table_add(
            "ig_port_forward_tbl", "update_loadreq_spine_to_loadreq", matchspec0
        )
        matchspec0 = [hex(LOADACK_SERVER)]
        self.controller.table_add(
            "ig_port_forward_tbl", "update_loadack_server_to_loadack", matchspec0
        )
        # matchspec0 = distcacheleaf_ig_port_forward_tbl_match_spec_t(\
        #        op_hdr_optype = DISTCACHE_GETRES_SPINE)
        # self.client.ig_port_forward_tbl_table_add_with_update_distcache_getres_spine_to_getres(\
        #        self.sess_hdl, self.dev_tgt, matchspec0)
        matchspec0 = [hex(DISTCACHE_INVALIDATE)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_distcache_invalidate_to_distcache_invalidate_inswitch",
            matchspec0,
        )
        matchspec0 = [hex(PUTREQ_LARGEVALUE_SEQ)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_putreq_largevalue_seq_to_putreq_largevalue_seq_inswitch",
            matchspec0,
        )
        matchspec0 = [hex(PUTREQ_LARGEVALUE_SEQ_CACHED)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_putreq_largevalue_seq_cached_to_putreq_largevalue_seq_inswitch",
            matchspec0,
        )
        matchspec0 = [hex(GETRES_LARGEVALUE_SERVER)]
        self.controller.table_add(
            "ig_port_forward_tbl",
            "update_getres_largevalue_server_to_getres_largevalue",
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
            # for tmpoptype in [PUTREQ_SEQ_INSWITCH, DELREQ_SEQ_INSWITCH]:
            #    matchspec0 = distcacheleaf_access_latest_tbl_match_spec_t(\
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
            # matchspec0 = distcacheleaf_access_latest_tbl_match_spec_t(\
            #        op_hdr_optype = NETCACHE_VALUEUPDATE_INSWITCH,
            #        inswitch_hdr_is_cached = is_cached)
            # if is_cached == 1: # cannot set latest = 1 even if is_cached == 0 -> Tofino bug?
            #    self.client.access_latest_tbl_table_add_with_set_and_get_latest(\
            #            self.sess_hdl, self.dev_tgt, matchspec0)
            # matchspec0 = distcacheleaf_access_latest_tbl_match_spec_t(\
            #        op_hdr_optype = DISTCACHE_LEAF_VALUEUPDATE_INSWITCH,
            #        inswitch_hdr_is_cached = is_cached)
            # self.client.access_latest_tbl_table_add_with_set_and_get_latest(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
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
            # NOTE: DistCache resorts server to invalidate inswitch cache for PUTREQ_LARGEVALUE_SEQ_INSWITCH instead of on path

        # Stage 1

        # Table: prepare_for_cachepop_and_save_client_info_tbl (default: reset_server_sid(); size: 2*server_physical_num+1=5 < 17)
        for tmpoptype in [GETREQ_INSWITCH, SCANREQ_SPLIT]:
            for tmp_server_physical_idx in range(server_physical_num):
                tmp_devport = self.server_devports[tmp_server_physical_idx]
                tmp_server_sid = str(self.server_sids[tmp_server_physical_idx])
                matchspec0 = [hex(tmpoptype), tmp_devport]
                if tmpoptype == GETREQ_INSWITCH:
                    actnspec0 = [tmp_server_sid]
                    self.controller.table_add(
                        "prepare_for_cachepop_and_save_client_info_tbl",
                        "set_server_sid_udpport_and_save_client_info",
                        matchspec0,
                        actnspec0,
                    )
                    # NOTE: we explictly invoke NoAction() for SCANREQ_SPLIT and NETCACHE_GETREQ_POP to avoid reset their clone_hdr.server_sid: SCANREQ_SPLIT.server_sid is set by process_scanreq_split to clone next SCANREQ_SPLIT to next server; NETCACHE_GETREQ_POP.server_sid is inherited from original GETREQ_INSWITCH to clone alst NETCACHE_GETREQ_POP as GETREQ to corresponding server
                elif tmpoptype == SCANREQ_SPLIT and RANGE_SUPPORT == True:
                    self.controller.table_add(
                        "prepare_for_cachepop_and_save_client_info_tbl",
                        "NoAction",
                        matchspec0,
                    )
        matchspec0 = [hex(NETCACHE_GETREQ_POP), self.reflector_devport]
        self.controller.table_add(
            "prepare_for_cachepop_and_save_client_info_tbl", "NoAction", matchspec0
        )

        # Table: access_cmi_tbl (default: initialize_cmi_predicate; size: 3)
        cm_hashnum = 4
        # cm_hashnum = 3
        for i in range(1, cm_hashnum + 1):
            print("Configuring access_cm{}_tbl".format(i))
            for tmpoptype in [GETREQ_INSWITCH]:
                for is_cached in cached_list:
                    for is_latest in latest_list:
                        if (
                            is_cached == 1 and is_latest == 1
                        ):  # follow algorithm 1 in NetCache paper to update CM
                            continue
                        else:
                            matchspec0 = [
                                hex(tmpoptype),
                                hex(1),
                                hex(is_cached),
                                hex(is_latest),
                            ]
                            self.controller.table_add(
                                "access_cm{}_tbl".format(i),
                                "update_cm{}".format(i),
                                matchspec0,
                            )

        # Stgae 2

        # Table: is_hot_tbl (default: reset_is_hot; size: 1)
        print("Configuring is_hot_tbl")
        matchspec0 = [hex(2), hex(2), hex(2), hex(2)]
        self.controller.table_add("is_hot_tbl", "set_is_hot", matchspec0)

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
                # matchspec0 = distcacheleaf_access_savedseq_tbl_match_spec_t(\
                #        op_hdr_optype = NETCACHE_VALUEUPDATE_INSWITCH,
                #        inswitch_hdr_is_cached = is_cached,
                #        meta_is_latest = is_latest)
                # if is_cached == 1:
                #    self.client.access_savedseq_tbl_table_add_with_set_and_get_savedseq(\
                #            self.sess_hdl, self.dev_tgt, matchspec0)
                # matchspec0 = distcacheleaf_access_savedseq_tbl_match_spec_t(\
                #        op_hdr_optype = DISTCACHE_LEAF_VALUEUPDATE_INSWITCH,
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
                # matchspec0 = distcacheleaf_update_vallen_tbl_match_spec_t(\
                #        op_hdr_optype = NETCACHE_VALUEUPDATE_INSWITCH,
                #        inswitch_hdr_is_cached = is_cached,
                #        meta_is_latest = is_latest)
                # if is_cached == 1:
                #    self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                #            self.sess_hdl, self.dev_tgt, matchspec0)
                # matchspec0 = distcacheleaf_update_vallen_tbl_match_spec_t(\
                #        op_hdr_optype = DISTCACHE_LEAF_VALUEUPDATE_INSWITCH,
                #        inswitch_hdr_is_cached = is_cached,
                #        meta_is_latest = is_latest)
                # self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                #        self.sess_hdl, self.dev_tgt, matchspec0)
                matchspec0 = [
                    hex(DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN),
                    hex(is_cached),
                    hex(is_latest),
                ]
                self.controller.table_add(
                    "update_vallen_tbl", "set_and_get_vallen", matchspec0
                )

        # Table: access_bfX_tbl (default: reset_is_reportX; size: 3)
        # bf_hashnum = 3
        bf_hashnum = 2
        for i in range(1, bf_hashnum + 1):
            print("Configuring access_bf{}_tbl".format(i))
            for tmpoptype in [GETREQ_INSWITCH]:
                matchspec0 = [hex(tmpoptype), hex(2), hex(2), hex(2), hex(2)]
                self.controller.table_add(
                    "access_bf{}_tbl".format(i), "update_bf{}".format(i), matchspec0, []
                )

        # Stage 4

        # Table: is_report_tbl (default: reset_is_report; size: 1)
        print("Configuring is_report_tbl")
        matchspec0 = [hex(1), hex(1)]
        # meta_is_report3 = 1)
        self.controller.table_add("is_report_tbl", "set_is_report", matchspec0)

        # Stage 4-11
        for i in range(1, 17):
            print("Configuring update_vallo{}_tbl".format(i))
            self.configure_update_val_tbl("lo{}".format(i))
            print("Configuring update_valhi{}_tbl".format(i))
            self.configure_update_val_tbl("hi{}".format(i))
        # Stage 9

        # Table: lastclone_lastscansplit_tbl (default: reset_is_lastclone_lastscansplit; size: 2/3)
        print("Configuring lastclone_lastscansplit_tbl")
        for tmpoptype in [NETCACHE_GETREQ_POP]:
            matchspec0 = [hex(tmpoptype), hex(0)]
            self.controller.table_add(
                "lastclone_lastscansplit_tbl", "set_is_lastclone", matchspec0
            )

        # Stage 10

        # Table: eg_port_forward_tbl (default: NoAction; size: < 2048)
        print("Configuring eg_port_forward_tbl")

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
                ]
                actnspec0 = [hex(val_stat_udplen), hex(val_stat_iplen)]
                self.controller.table_add(
                    "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
                )  # 0 is priority (range may be overlapping)
            for tmpoptype in [PUTREQ_SEQ, NETCACHE_PUTREQ_SEQ_CACHED]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + hex(vallen_start) + "->" + hex(vallen_end),
                ]
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
        # NETCACHE_WARMUPREQ_INSWITCH_POP is processed by spine switch
        # op_inswitch_clone_udplen = 74
        # op_inswitch_clone_iplen = 94
        # , NETCACHE_VALUEUPDATE_ACK, DISTCACHE_LEAF_VALUEUPDATE_INSWITCH_ACK, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK
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
                hex(tmpoptype),
                "" + hex(0) + "->" + hex(switch_max_vallen),
            ]
            actnspec0 = [hex(seq_udplen), hex(seq_iplen)]
            self.controller.table_add(
                "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
            )  # 0 is priority (range may be overlapping)

        matchspec0 = [
            hex(SCANREQ_SPLIT),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]
        actnspec0 = [hex(scanreqsplit_udplen), hex(scanreqsplit_iplen)]
        self.controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping)

        matchspec0 = [
            hex(CACHE_EVICT_LOADFREQ_INSWITCH_ACK),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]
        actnspec0 = [hex(frequency_udplen), hex(frequency_iplen)]
        self.controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping)
        matchspec0 = [
            hex(NETCACHE_GETREQ_POP),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]

        actnspec0 = [hex(op_switchload_clone_udplen), hex(op_switchload_clone_iplen)]
        self.controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping)

        # Table: update_ipmac_srcport_tbl (default: NoAction; 6*client_physical_num+9*server_physical_num+3=33 < 15*8+3=123 < 256)
        # NOTE: udp.dstport is updated by eg_port_forward_tbl (only required by switch2switchos)
        # NOTE: update_ipmac_srcport_tbl focues on src/dst ip/mac and udp.srcport
        print("Configuring update_ipmac_srcport_tbl")
        # (1) for response from server to client, egress port has been set based on ip.dstaddr (or by clone_i2e) in ingress pipeline
        # (2) for response from switch to client, egress port has been set by clone_e2e in egress pipeline
        for tmp_client_physical_idx in range(client_physical_num):
            tmp_devport = self.client_devports[tmp_client_physical_idx]
            tmp_server_mac = server_macs[0]
            tmp_server_ip = server_ips[0]
            actnspec0 = [tmp_server_mac, tmp_server_ip, hex(server_worker_port_start)]

            # for tmpoptype in [GETRES, DISTCACHE_GETRES_SPINE, PUTRES, DELRES, SCANRES_SPLIT, WARMUPACK, LOADACK]:
            for tmpoptype in [
                GETRES,
                PUTRES,
                DELRES,
                SCANRES_SPLIT,
                WARMUPACK,
                LOADACK,
                GETRES_LARGEVALUE,
            ]:
                matchspec0 = [hex(tmpoptype), tmp_devport]
                self.controller.table_add(
                    "update_ipmac_srcport_tbl",
                    "update_srcipmac_srcport_server2client",
                    matchspec0,
                    actnspec0,
                )
        # for request from client to server, egress port has been set by partition_tbl in ingress pipeline
        # NOTE: for each pkt from client to spine, as its eport must be spineswitch.devport instead of server.devport, it will not invoke update_dstipmac_client2server() here
        for tmp_server_physical_idx in range(server_physical_num):
            tmp_devport = self.server_devports[tmp_server_physical_idx]
            tmp_server_mac = server_macs[tmp_server_physical_idx]
            tmp_server_ip = server_ips[tmp_server_physical_idx]
            actnspec1 = [tmp_server_mac, tmp_server_ip]

            for tmpoptype in [
                GETREQ,
                PUTREQ_SEQ,
                NETCACHE_PUTREQ_SEQ_CACHED,
                DELREQ_SEQ,
                NETCACHE_DELREQ_SEQ_CACHED,
                SCANREQ_SPLIT,
                LOADREQ,
                PUTREQ_LARGEVALUE_SEQ,
                PUTREQ_LARGEVALUE_SEQ_CACHED,
            ]:
                matchspec0 = [hex(tmpoptype), tmp_devport]

                self.controller.table_add(
                    "update_ipmac_srcport_tbl",
                    "update_dstipmac_client2server",
                    matchspec0,
                    actnspec1,
                )
            tmp_client_mac = client_macs[0]
            tmp_client_ip = client_ips[0]
            tmp_client_port = str(123)  # not cared by switchos
            actnspec1 = [
                (tmp_client_mac),
                (tmp_server_mac),
                (tmp_client_ip),
                (tmp_server_ip),
                tmp_client_port,
            ]
            # NETCACHE_VALUEUPDATE_ACK, DISTCACHE_LEAF_VALUEUPDATE_INSWITCH_ACK, DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK
            for tmpoptype in [
                DISTCACHE_INVALIDATE_ACK,
                DISTCACHE_VALUEUPDATE_INSWITCH_ACK,
            ]:  # simulate client -> server
                matchspec0 = [
                    (hex(tmpoptype)),
                    tmp_devport,
                ]
                self.controller.table_add(
                    "update_ipmac_srcport_tbl",
                    "update_ipmac_srcport_client2server",
                    matchspec0,
                    actnspec1,
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
        for tmpoptype in [NETCACHE_GETREQ_POP]:
            matchspec0 = [
                hex(tmpoptype),
                tmp_devport,
            ]
            self.controller.table_add(
                "update_ipmac_srcport_tbl",
                "update_dstipmac_switch2switchos",
                matchspec0,
                actnspec2,
            )

        # Table: add_and_remove_value_header_tbl (default: remove_all; 17*6=102)
        print("Configuring add_and_remove_value_header_tbl")
        # NOTE: egress pipeline must not output PUTREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, CACHE_POP_INSWITCH, and PUTREQ_SEQ_INSWITCH
        # NOTE: even for future PUTREQ_LARGE/GETRES_LARGE, as their values should be in payload, we should invoke add_only_vallen() for vallen in [0, global_max_vallen]
        # , NETCACHE_VALUEUPDATE
        # NOTE: we do NOT access add_and_remove_value_header_tbl for DISTCACHE_SPINE_VALUEUPDATE_INSWITCH, as we bypass_egress in spineselect_tbl in ingress pipeline
        # , DISTCACHE_SPINE_VALUEUPDATE_INSWITCH, LOADREQ
        for tmpoptype in [
            PUTREQ,
            PUTREQ_SEQ,
            NETCACHE_PUTREQ_SEQ_CACHED,
            GETRES,
            DISTCACHE_VALUEUPDATE_INSWITCH,
        ]:
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
                ]
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

        # Stage 11

        # Table: drop_tbl (default: NoAction; size: 2)
        print("Configuring drop_tbl")
        matchspec0 = [hex(DISTCACHE_INVALIDATE_INSWITCH)]

        self.controller.table_add(
            "drop_tbl", "drop_distcache_invalidate_inswitch", matchspec0
        )
        # matchspec0 = distcacheleaf_drop_tbl_match_spec_t(\
        #        op_hdr_optype = NETCACHE_VALUEUPDATE_INSWITCH)
        # self.client.drop_tbl_table_add_with_drop_netcache_valueupdate_inswitch(\
        #        self.sess_hdl, self.dev_tgt, matchspec0)
        matchspec0 = [hex(DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN)]
        self.controller.table_add(
            "drop_tbl", "drop_distcache_valueupdate_inswitch_origin", matchspec0
        )

    # def cleanup_table(self, table, iscalled=False):

    def configure_eg_port_forward_tbl(self):
        # Table: eg_port_forward_tbl (default: NoAction; size: 21+2*server_physical_num+32*spine_physical_num*server_physical_num=89 < 2048)
        tmp_client_sids = [0, self.spineswitch_sid]
        tmp_server_sids = [0] + self.server_sids
        for is_cached in cached_list:
            for is_hot in hot_list:
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
                                        # is_lastclone_for_pktloss should be 0 for GETREQ_INSWITCH
                                        if (
                                            is_lastclone_for_pktloss == 0
                                            and tmp_client_sid != 0
                                            and tmp_server_sid != 0
                                        ):
                                            # size: 32*spine_physical_num*server_physical_num=64
                                            # NOTE: tmp_client_sid != 0 to prepare for cache hit; tmp_server_sid != 0 to prepare for cache pop (clone last NETCACHE_GETREQ_POP as GETREQ to server)

                                            matchspec0 = [
                                                hex(GETREQ_INSWITCH),
                                                hex(is_cached),
                                                hex(is_hot),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(tmp_server_sid),
                                            ]
                                            if is_cached == 0:
                                                if is_hot == 1 and is_report == 0:
                                                    # Update GETREQ_INSWITCH as NETCACHE_GETREQ_POP to switchos by cloning
                                                    actnspec0 = [
                                                        hex(self.reflector_sid),
                                                        hex(reflector_dp2cpserver_port),
                                                    ]
                                                    self.controller.table_add(
                                                        "eg_port_forward_tbl",
                                                        "update_getreq_inswitch_to_netcache_getreq_pop_clone_for_pktloss_and_getreq",
                                                        matchspec0,
                                                        actnspec0,
                                                    )
                                                else:
                                                    # Update GETREQ_INSWITCH as GETREQ to server
                                                    # actnspec0 = [self.devPorts[1]]
                                                    self.controller.table_add(
                                                        "eg_port_forward_tbl",
                                                        "update_getreq_inswitch_to_getreq",
                                                        matchspec0,
                                                    )
                                            else:  # is_cached == 1
                                                if (
                                                    is_latest == 0
                                                ):  # follow algorithm 1 in NetCache paper to report hot key if necessary
                                                    if is_hot == 1 and is_report == 0:
                                                        # Update GETREQ_INSWITCH as NETCACHE_GETREQ_POP to switchos by cloning
                                                        actnspec0 = [
                                                            hex(self.reflector_sid),
                                                            hex(reflector_dp2cpserver_port),
                                                        ]
                                                        self.controller.table_add(
                                                            "eg_port_forward_tbl",
                                                            "update_getreq_inswitch_to_netcache_getreq_pop_clone_for_pktloss_and_getreq",
                                                            matchspec0,
                                                            actnspec0,
                                                        )
                                                    else:
                                                        # Update GETREQ_INSWITCH as GETREQ to server
                                                        # actnspec0 = [self.devPorts[1]]
                                                        self.controller.table_add(
                                                            "eg_port_forward_tbl",
                                                            "update_getreq_inswitch_to_getreq",
                                                            matchspec0,
                                                        )
                                                else:  # is_cached == 1 and is_latest == 1
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
                                        # is_cached=0 (no inswitch_hdr), is_hot=0 (not access CM), is_report=0 (not access BF), is_latest=0, is_deleted=0, tmp_client_sid=0 (no inswitch_hdr), tmp_server_sid!=0 for NETCACHE_GETREQ_POP
                                        # size: 2*server_physical_num=4
                                        if (
                                            is_cached == 0
                                            and is_hot == 0
                                            and is_report == 0
                                            and is_latest == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and tmp_server_sid != 0
                                        ):
                                            matchspec0 = [
                                                hex(NETCACHE_GETREQ_POP),
                                                hex(is_cached),
                                                hex(is_hot),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(tmp_server_sid),
                                            ]
                                            if is_lastclone_for_pktloss == 0:
                                                actnspec0 = [hex(self.reflector_sid)]
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "forward_netcache_getreq_pop_clone_for_pktloss_and_getreq",
                                                    matchspec0,
                                                    actnspec0,
                                                )
                                            else:
                                                actnspec0 = [hex(tmp_server_sid)]
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_netcache_getreq_pop_to_getreq_by_mirroring",
                                                    matchspec0,
                                                    actnspec0,
                                                )
                                        # is_cached=0 (memset inswitch_hdr by end-host, and key must not be cached in cache_lookup_tbl for CACHE_POP_INSWITCH), is_hot (cm_predicate=1), is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss should be 0 for CACHE_POP_INSWITCH
                                        # size: 4
                                        # if is_cached == 0 and is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                        if (
                                            is_cached == 0
                                            and is_hot == 0
                                            and is_report == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and tmp_server_sid == 0
                                        ):
                                            matchspec0 = [
                                                hex(CACHE_POP_INSWITCH),
                                                hex(is_cached),
                                                hex(is_hot),
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
                                        # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktloss should be 0 for CACHE_POP_INSWITCH_ACK
                                        # size: 0
                                        # if is_cached == 0 and is_hot == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0:
                                        if (
                                            is_cached == 0
                                            and is_hot == 0
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
                                                hex(is_hot),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(tmp_server_sid),
                                            ]
                                            # NOTE: default action is NoAction -> forward the packet to sid set by clone_e2e
                                            pass
                                        # is_hot, is_deleted, tmp_client_sid, is_lastclone_for_pktloss should be 0 for PUTREQ_INSWITCH
                                        # size: 4
                                        if (
                                            is_hot == 0
                                            and is_report == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and tmp_server_sid == 0
                                        ):
                                            matchspec0 = [
                                                hex(PUTREQ_SEQ_INSWITCH),
                                                hex(is_cached),
                                                hex(is_hot),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(tmp_server_sid),
                                            ]
                                            if is_cached == 0:
                                                # Update PUTREQ_SEQ_INSWITCH as PUTREQ_SEQ to server
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_putreq_seq_inswitch_to_putreq_seq",
                                                    matchspec0,
                                                )
                                            elif is_cached == 1:
                                                # Update PUTREQ_INSWITCH as NETCACHE_PUTREQ_SEQ_CACHED to server
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_putreq_inswitch_to_netcache_putreq_seq_cached",
                                                    matchspec0,
                                                )
                                        # is_hot, (cm_predicate=1), is_deleted, tmp_client_sid, is_lastclone_for_pktloss should be 0 for DELREQ_INSWITCH
                                        # size: 4
                                        if (
                                            is_hot == 0
                                            and is_report == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and tmp_server_sid == 0
                                        ):
                                            matchspec0 = [
                                                hex(DELREQ_SEQ_INSWITCH),
                                                hex(is_cached),
                                                hex(is_hot),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(tmp_server_sid),
                                            ]
                                            if is_cached == 0:
                                                # Update DELREQ_SEQ_INSWITCH as DELREQ_SEQ to server
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_delreq_seq_inswitch_to_delreq_seq",
                                                    matchspec0,
                                                )
                                            elif is_cached == 1:
                                                # Update DELREQ_INSWITCH as NETCACHE_DELREQ_SEQ_CACHED to server
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_delreq_inswitch_to_netcache_delreq_seq_cached",
                                                    matchspec0,
                                                )
                                        # is_cached=1 (key must be cached in cache_lookup_tbl for CACHE_EVICT_LOADFREQ_INSWITCH), is_hot (cm_predicate=1), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH
                                        # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                        # size: 1
                                        if (
                                            is_cached == 1
                                            and is_report == 0
                                            and is_hot == 0
                                            and is_latest == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and tmp_server_sid == 0
                                        ):
                                            matchspec0 = [
                                                hex(CACHE_EVICT_LOADFREQ_INSWITCH),
                                                hex(is_cached),
                                                hex(is_hot),
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
                                        # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH_ACK
                                        # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                        # size: 0
                                        if (
                                            is_cached == 0
                                            and is_report == 0
                                            and is_hot == 0
                                            and is_latest == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and tmp_server_sid == 0
                                        ):
                                            matchspec0 = [
                                                hex(CACHE_EVICT_LOADFREQ_INSWITCH_ACK),
                                                hex(is_cached),
                                                hex(is_hot),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(tmp_server_sid),
                                            ]

                                            # Forward CACHE_EVICT_LOADFREQ_INSWITCH_ACK (by clone_e2e) to reflector

                                            # NOTE: default action is NoAction -> forward the packet to sid set by clone_e2e
                                            pass

                                        # is_hot (cm_predicate=1), is_report, is_deleted, tmp_client_sid=0, is_lastclone_for_pktloss, tmp_server_sid should be 0 for PUTREQ_LARGEVALUE_SEQ_INSWITCH
                                        # size: 4
                                        if (
                                            is_report == 0
                                            and is_hot == 0
                                            and is_deleted == 0
                                            and tmp_client_sid == 0
                                            and is_lastclone_for_pktloss == 0
                                            and tmp_server_sid == 0
                                        ):
                                            matchspec0 = [
                                                hex(PUTREQ_LARGEVALUE_SEQ_INSWITCH),
                                                hex(is_cached),
                                                hex(is_hot),
                                                hex(is_report),
                                                hex(is_latest),
                                                hex(is_deleted),
                                                hex(tmp_client_sid),
                                                hex(is_lastclone_for_pktloss),
                                                hex(tmp_server_sid),
                                            ]

                                            if is_cached == 0:
                                                # Update PUTREQ_LARGEVALUE_SEQ_INSWITCH as PUTREQ_LARGEVALUE_SEQ to server
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_putreq_largevalue_seq_inswitch_to_putreq_largevalue_seq",
                                                    matchspec0,
                                                )
                                            elif is_cached == 1:
                                                # Update PUTREQ_LARGEVALUE_SEQ_INSWITCH as PUTREQ_LARGEVALUE_SEQ_CACHED to server
                                                self.controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_putreq_largevalue_seq_inswitch_to_putreq_largevalue_seq_cached",
                                                    matchspec0,
                                                )


tableconfig = TableConfigure(-1)
tableconfig.setUp()
# tableconfig.clean_all_tables()
tableconfig.create_mirror_session()
tableconfig.runTest()