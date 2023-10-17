# TODO: Replace PROC, ACTION, and TABLE

"""
Thrift PD interface DV test
"""
import logging
import os
from p4utils.utils.sswitch_thrift_API import SimpleSwitchThriftAPI

import random
import sys
import time
import unittest


this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *

controller = SimpleSwitchThriftAPI(9090, "192.168.122.229")

cached_list = [0, 1]
hot_list = [0, 1]
validvalue_list = [0, 1, 3]
# validvalue_list = [0, 1, 2, 3] # If with PUTREQ_LARGE
latest_list = [0, 1]
stat_list = [0, 1]
deleted_list = [0, 1]
sampled_list = [0, 1]
lastclone_list = [0, 1]
snapshot_flag_list = [0, 1]
case1_list = [0, 1]
access_val_mode_list = [0, 1, 2, 3]
is_largevalueblock_list = [0, 1]


class TableConfigure:
    # def __init__(self):
    #     # initialize the thrift data plane
    #     pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbufferv4"])

    def configure_update_val_tbl(self, valname):
        # size = 3
        for access_val_mode in access_val_mode_list:
            matchspec0 = [hex(access_val_mode)]
            # NOTE: not access val_reg if access_val_mode == 0
            if access_val_mode == 1:
                controller.table_add(
                    "update_val{}_tbl".format(valname),
                    "get_val{}".format(valname),
                    matchspec0,
                    [],
                )
            elif access_val_mode == 2:
                controller.table_add(
                    "update_val{}_tbl".format(valname),
                    "set_and_get_val{}".format(valname),
                    matchspec0,
                    [],
                )
            elif access_val_mode == 3:
                controller.table_add(
                    "update_val{}_tbl".format(valname),
                    "reset_and_get_val{}".format(valname),
                    matchspec0,
                    [],
                )

    def clean_all_tables(self):
        for table_name in controller.get_tables():
            controller.table_clear(table_name)

    def create_mirror_session(self):
        # Mirror_add
        for i in range(client_physical_num):
            print(
                "Binding sid {} with client devport {} for both direction mirroring".format(
                    self.client_sids[i], self.client_devports[i]
                )
            )  # clone to client
            controller.mirroring_add(self.client_sids[i], self.client_devports[i])
        for i in range(server_physical_num):
            print(
                "Binding sid {} with server devport {} for both direction mirroring".format(
                    self.server_sids[i], self.server_devports[i]
                )
            )  # clone to server
            controller.mirroring_add(self.server_sids[i], self.server_devports[i])

    def setUp(self):
        print("\nSetup")

        self.client_devports = []
        self.server_devports = []
        self.recir_devports = []
        # get the device ports from front panel ports
        for client_fpport in client_fpports:
            port, chnl = client_fpport.split("/")
            self.client_devports.append(port)
        for server_fpport in server_fpports:
            port, chnl = server_fpport.split("/")
            self.server_devports.append(port)

        # get the device ports from pipeline_recirports_to/fromsingle
        for recirport_tosingle in pipeline_recirports_tosingle:
            if recirport_tosingle is not None:
                port, chnl = recirport_tosingle.split("/")
                self.recir_devports.append(port)
        for recirport_fromsingle in pipeline_recirports_fromsingle:
            if recirport_fromsingle is not None:
                port, chnl = recirport_fromsingle.split("/")
                self.recir_devports.append(port)

        # self.recirPorts = [64, 192]

        # NOTE: in each pipeline, 64-67 are recir/cpu ports, 68-71 are recir/pktgen ports
        # self.cpuPorts = [64, 192] # CPU port is 100G

        sidnum = len(self.client_devports) + len(self.server_devports)
        sids = random.sample(range(10, 20), sidnum)
        self.client_sids = sids[0 : len(self.client_devports)]
        self.server_sids = sids[len(self.client_devports) : sidnum]

        # NOTE: data plane communicate with switchos by software-based reflector, which is deployed in one server machine
        isvalid = False
        for i in range(server_physical_num):
            if reflector_ip_for_switchos == server_ip_for_controller_list[i]:
                isvalid = True
                self.reflector_ip_for_switch = server_ips[i]
                self.reflector_mac_for_switch = server_macs[i]
                self.reflector_devport = self.server_devports[i]
                # clone to switchos (i.e., reflector at [the first] physical server)
                self.reflector_sid = self.server_sids[i]
        if isvalid == False:
            print("[ERROR] invalid reflector configuration")
            exit(-1)

    ### MAIN ###

    def runTest(self):
        print("\nTest")

        #####################
        ### Prepare ports ###
        #####################

        self.create_mirror_session()

        ################################
        ### Normal MAT Configuration ###
        ################################

        # Ingress pipeline

        # Stage 0
        # print('client_macs',client_macs)
        # Table: l2l3_forward_tbl (default: NoAction	; size = client_physical_num+server_physical_num = 4 < 16)
        print("Configuring l2l3_forward_tbl")
        for i in range(client_physical_num):
            controller.table_add(
                "l2l3_forward_tbl",
                "l2l3_forward",
                [client_macs[i], client_ips[i] + "/32"],
                [self.client_devports[i]],
            )
        for i in range(server_physical_num):
            controller.table_add(
                "l2l3_forward_tbl",
                "l2l3_forward",
                [server_macs[i], server_ips[i] + "/32"],
                [self.server_devports[i]],
            )

        # Table: need_recirculate_tbl (default: reset_need_recirculate; size = <=8)
        # NOTE: we set it in tofino/ptf_snapshotserver/table_configure.py

        # Table: set_hot_threshold_tbl (default: set_hot_threshold; size = 1)
        print("Configuring set_hot_threshold_tbl")
        controller.table_set_default(
            "set_hot_threshold_tbl", "set_hot_threshold", [hex(hot_threshold)]
        )

        # Stage 1

        # Table: recirculate_tbl (default: NoAction	; size = 5)
        print("Configuring recirculate_tbl")
        for tmpoptype in [
            PUTREQ,
            DELREQ,
            GETRES_LATEST_SEQ,
            GETRES_DELETED_SEQ,
            PUTREQ_LARGEVALUE,
        ]:
            matchspec0 = [hex(tmpoptype), hex(1)]
            # recirculate to the pipeline of the first physical client for atomicity of setting snapshot flag
            controller.table_add("recirculate_tbl", "recirculate_pkt", matchspec0)

        # Stage 1

        if RANGE_SUPPORT == False:
            # Table: hash_for_partition_tbl (default: NoAction	; size = 13)
            print("Configuring hash_for_partition_tbl")
            for tmpoptype in [
                GETREQ,
                CACHE_POP_INSWITCH,
                PUTREQ,
                DELREQ,
                WARMUPREQ,
                LOADREQ,
                CACHE_EVICT_LOADFREQ_INSWITCH,
                CACHE_EVICT_LOADDATA_INSWITCH,
                LOADSNAPSHOTDATA_INSWITCH,
                SETVALID_INSWITCH,
                GETRES_LATEST_SEQ,
                GETRES_DELETED_SEQ,
                PUTREQ_LARGEVALUE,
            ]:
                matchspec0 = [hex(tmpoptype), hex(0)]
                controller.table_add(
                    "hash_for_partition_tbl", "hash_for_partition", matchspec0
                )

        # Stage 2
        # Table: hash_partition_tbl (default: NoAction	; size <= 13 * 128)
        print("Configuring hash_partition_tbl")
        hash_range_per_server = switch_partition_count / server_total_logical_num
        for tmpoptype in [
            GETREQ,
            CACHE_POP_INSWITCH,
            PUTREQ,
            DELREQ,
            WARMUPREQ,
            LOADREQ,
            CACHE_EVICT_LOADFREQ_INSWITCH,
            CACHE_EVICT_LOADDATA_INSWITCH,
            LOADSNAPSHOTDATA_INSWITCH,
            SETVALID_INSWITCH,
            GETRES_LATEST_SEQ,
            GETRES_DELETED_SEQ,
            PUTREQ_LARGEVALUE,
        ]:
            hash_start = 0  # [0, partition_count-1]
            for global_server_logical_idx in range(server_total_logical_num):
                if global_server_logical_idx == server_total_logical_num - 1:
                    # if end is not included, then it is just processed by port 1111
                    hash_end = switch_partition_count - 1
                else:
                    hash_end = hash_start + hash_range_per_server - 1
                # NOTE: both start and end are included
                matchspec0 = [
                    hex(tmpoptype),
                    "" + hex(int(hash_start)) + "->" + hex(int(hash_end)),
                    hex(0),
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
                        "WARNING: no physical server covers global_server_logical_idx {} -> no corresponding MAT entries in hash_partition_tbl"
                    .format(global_server_logical_idx))
                else:
                    # udp_dstport = server_worker_port_start + global_server_logical_idx
                    udp_dstport = server_worker_port_start + local_server_logical_idx
                    eport = self.server_devports[server_physical_idx]
                    if (
                        tmpoptype == GETRES_LATEST_SEQ
                        or tmpoptype == GETRES_DELETED_SEQ
                    ):
                        actnspec0 = [eport]
                        # 0 is priority (range may be overlapping)
                        controller.table_add(
                            "hash_partition_tbl",
                            "hash_partition_for_special_response",
                            matchspec0,
                            actnspec0,
                            0,
                        )
                    else:
                        actnspec0 = [hex(udp_dstport), eport]
                        # 0 is priority (range may be overlapping)
                        controller.table_add(
                            "hash_partition_tbl",
                            "hash_partition",
                            matchspec0,
                            actnspec0,
                            0,
                        )
                hash_start = hash_end + 1

        # Stage 3

        # Table: cache_lookup_tbl (default: uncached_action; size = 32K/64K)
        print("Leave cache_lookup_tbl managed by controller in runtime")

        # Table: hash_for_cm12/34_tbl (default: NoAction	; size = 2)
        for i in ["12", "34"]:
            print("Configuring hash_for_cm{}_tbl".format(i))
            for tmpoptype in [GETREQ]:
                matchspec0 = [hex(tmpoptype),hex(0)]
                controller.table_add(
                    "hash_for_cm{}_tbl".format(i), "hash_for_cm{}".format(i), matchspec0
                )

        # Table: hash_for_seq_tbl (default: NoAction	; size = 3)
        print("Configuring hash_for_seq_tbl")
        for tmpoptype in [PUTREQ, DELREQ, PUTREQ_LARGEVALUE]:
            matchspec0 = [hex(tmpoptype), hex(0)]
            controller.table_add("hash_for_seq_tbl", "hash_for_seq", matchspec0)

        # Stage 3

        # Table: prepare_for_cachehit_tbl (default: set_client_sid(0); size = 3*client_physical_num=6 < 3*8=24 < 32)
        print("Configuring prepare_for_cachehit_tbl")
        for client_physical_idx in range(client_physical_num):
            tmp_clientsid = self.client_sids[client_physical_idx]
            for tmpoptype in [GETREQ, PUTREQ, DELREQ]:
                controller.table_add(
                    "prepare_for_cachehit_tbl",
                    "set_client_sid",
                    [
                        hex(tmpoptype),
                        "" + client_ips[client_physical_idx] + "/32",
                        hex(0),
                    ],
                    [hex(tmp_clientsid)],
                )

        # Table: ipv4_forward_tbl (default: NoAction	; size = 9*client_physical_num=18 < 9*8=72)
        print("Configuring ipv4_forward_tbl")
        for tmp_client_physical_idx in range(client_physical_num):
            ipv4addr0 = client_ips[tmp_client_physical_idx]
            eport = self.client_devports[tmp_client_physical_idx]
            tmpsid = self.client_sids[tmp_client_physical_idx]
            # GETRES, GETRES_LARGEVALUE, PUTRES, DELRES
            for tmpoptype in [
                GETRES_SEQ,
                PUTRES_SEQ,
                DELRES_SEQ,
                WARMUPACK,
                SCANRES_SPLIT,
                LOADACK,
                GETRES_LARGEVALUE_SEQ,
            ]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + client_ips[tmp_client_physical_idx] + "/32",
                    hex(0),
                ]
                actnspec0 = [eport]
                # print('eport',eport)
                controller.table_add(
                    "ipv4_forward_tbl", "forward_normal_response", matchspec0, actnspec0
                )
            for tmpoptype in [GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + client_ips[tmp_client_physical_idx] + "/32",
                    hex(0),
                ]
                actnspec0 = [hex(tmpsid)]
                controller.table_add(
                    "ipv4_forward_tbl",
                    "forward_special_get_response",
                    matchspec0,
                    actnspec0,
                )

        # Stage 4

        # Table: sample_tbl (default: NoAction	; size = 2)
        print("Configuring sample_tbl")
        for tmpoptype in [GETREQ, PUTREQ]:
            matchspec0 = [hex(tmpoptype), hex(0)]
            controller.table_add("sample_tbl", "sample", matchspec0)

        # Table: ig_port_forward_tbl (default: NoAction	; size = 7)
        print("Configuring ig_port_forward_tbl")
        matchspec0 = [hex(GETREQ), hex(0)]
        controller.table_add(
            "ig_port_forward_tbl", "update_getreq_to_getreq_inswitch", matchspec0
        )
        matchspec0 = [hex(GETRES_LATEST_SEQ), hex(0)]
        controller.table_add(
            "ig_port_forward_tbl",
            "update_getres_latest_seq_to_getres_latest_seq_inswitch",
            matchspec0,
        )
        matchspec0 = [hex(GETRES_DELETED_SEQ), hex(0)]
        controller.table_add(
            "ig_port_forward_tbl",
            "update_getres_deleted_seq_to_getres_deleted_seq_inswitch",
            matchspec0,
        )
        matchspec0 = [hex(PUTREQ), hex(0)]
        controller.table_add(
            "ig_port_forward_tbl", "update_putreq_to_putreq_inswitch", matchspec0
        )
        matchspec0 = [hex(DELREQ), hex(0)]
        controller.table_add(
            "ig_port_forward_tbl", "update_delreq_to_delreq_inswitch", matchspec0
        )
        if RANGE_SUPPORT:
            matchspec0 = [hex(SCANREQ), hex(0)]
            controller.table_add(
                "ig_port_forward_tbl", "update_scanreq_to_scanreq_split", matchspec0
            )
        matchspec0 = [hex(PUTREQ_LARGEVALUE), hex(0)]
        controller.table_add(
            "ig_port_forward_tbl",
            "update_putreq_largevalue_to_putreq_largevalue_inswitch",
            matchspec0,
        )

        # Egress pipeline

        # Stage 0

        # Table: access_cmi_tbl (default: initialize_cmi_predicate; size = 2)
        cm_hashnum = 4
        for i in range(1, cm_hashnum + 1):
            print("Configuring access_cm{}_tbl".format(i))
            for tmpoptype in [GETREQ_INSWITCH, PUTREQ_INSWITCH]:
                matchspec0 = [hex(tmpoptype), hex(1), hex(0)]
                controller.table_add(
                    "access_cm{}_tbl".format(i), "update_cm{}".format(i), matchspec0
                )

        # Stgae 1

        # Table: is_hot_tbl (default: reset_is_hot; size = 1)
        print("Configuring is_hot_tbl")
        matchspec0 = [hex(2), hex(2), hex(2), hex(2)]
        controller.table_add("is_hot_tbl", "set_is_hot", matchspec0)

        # Table: access_cache_frequency_tbl (default: NoAction	; size = 10)
        print("Configuring access_cache_frequency_tbl")
        for tmpoptype in [GETREQ_INSWITCH, PUTREQ_INSWITCH]:
            matchspec0 = [hex(tmpoptype), hex(1), hex(1)]
            controller.table_add(
                "access_cache_frequency_tbl", "update_cache_frequency", matchspec0
            )
        for is_sampled in sampled_list:
            for is_cached in cached_list:
                matchspec0 = [hex(CACHE_POP_INSWITCH), hex(is_sampled), hex(is_cached)]
                controller.table_add(
                    "access_cache_frequency_tbl", "reset_cache_frequency", matchspec0
                )
                matchspec0 = [
                    hex(CACHE_EVICT_LOADFREQ_INSWITCH),
                    hex(is_sampled),
                    hex(is_cached),
                ]
                controller.table_add(
                    "access_cache_frequency_tbl", "get_cache_frequency", matchspec0
                )

        # Table: access_validvalue_tbl (default: reset_meta_validvalue; size = 8)
        print("Configuring access_validvalue_tbl")
        for tmpoptype in [
            GETREQ_INSWITCH,
            GETRES_LATEST_SEQ_INSWITCH,
            GETRES_DELETED_SEQ_INSWITCH,
            PUTREQ_INSWITCH,
            DELREQ_INSWITCH,
            PUTREQ_LARGEVALUE_INSWITCH,
        ]:
            matchspec0 = [hex(tmpoptype), hex(1)]
            controller.table_add("access_validvalue_tbl", "get_validvalue", matchspec0)
        for is_cached in cached_list:
            # NOTE: set_validvalue does not change validvalue_hdr.validvalue
            matchspec0 = [
                hex(SETVALID_INSWITCH),
                hex(is_cached),
            ]  # key may or may not be cached for SETVALID_INSWITCH
            controller.table_add("access_validvalue_tbl", "set_validvalue", matchspec0)

        # Table: access_seq_tbl (default: NoAction	; size = 3)
        # NOTE: PUT/DELREQ_INSWITCH do NOT have fraginfo_hdr, while we ONLY assign seq for fragment 0 of PUTREQ_LARGEVALUE_INSWITCH
        print("Configuring access_seq_tbl")
        for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH, PUTREQ_LARGEVALUE_INSWITCH]:
            matchspec0 = [hex(tmpoptype), hex(0)]
            controller.table_add("access_seq_tbl", "assign_seq", matchspec0)

        # Stgae 2

        # Table: save_client_udpport_tbl (default: NoAction	; size = 4)
        print("Configuring save_client_udpport_tbl")
        for tmpoptype in [GETREQ_INSWITCH, PUTREQ_INSWITCH, DELREQ_INSWITCH]:
            matchspec0 = [hex(tmpoptype)]
            controller.table_add(
                "save_client_udpport_tbl", "save_client_udpport", matchspec0
            )

        # Table: access_latest_tbl (default: reset_is_latest; size = 20)
        print("Configuring access_latest_tbl")
        for is_cached in cached_list:
            for validvalue in validvalue_list:
                matchspec0 = [
                    hex(GETREQ_INSWITCH),
                    hex(is_cached),
                    hex(validvalue),
                    hex(0),
                ]
                if is_cached == 1:
                    controller.table_add("access_latest_tbl", "get_latest", matchspec0)
                for tmpoptype in [
                    GETRES_LATEST_SEQ_INSWITCH,
                    GETRES_DELETED_SEQ_INSWITCH,
                ]:
                    matchspec0 = [
                        hex(tmpoptype),
                        hex(is_cached),
                        hex(validvalue),
                        hex(0),
                    ]
                    if is_cached == 1 and validvalue == 1:
                        controller.table_add(
                            "access_latest_tbl", "set_and_get_latest", matchspec0
                        )
                for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH]:
                    matchspec0 = [
                        hex(tmpoptype),
                        hex(is_cached),
                        hex(validvalue),
                        hex(0),
                    ]
                    if is_cached == 1 and validvalue == 1:
                        controller.table_add(
                            "access_latest_tbl", "set_and_get_latest", matchspec0
                        )
                    elif is_cached == 1 and validvalue == 3:
                        controller.table_add(
                            "access_latest_tbl", "reset_and_get_latest", matchspec0
                        )
                matchspec0 = [
                    hex(CACHE_POP_INSWITCH),
                    hex(is_cached),
                    hex(validvalue),
                    hex(0),
                ]
                controller.table_add(
                    "access_latest_tbl", "reset_and_get_latest", matchspec0
                )
                # on-path in-switch invalidation for fragment 0 of PUTREQ_LARGEVALUE_INSWITCH
                matchspec0 = [
                    hex(PUTREQ_LARGEVALUE_INSWITCH),
                    hex(is_cached),
                    hex(validvalue),
                    hex(0),
                ]
                if is_cached == 1 and validvalue == 1:
                    controller.table_add(
                        "access_latest_tbl", "reset_and_get_latest", matchspec0
                    )
                elif is_cached == 1 and validvalue == 3:
                    controller.table_add(
                        "access_latest_tbl", "reset_and_get_latest", matchspec0
                    )

        # Table: access_largevalueseq_and_save_assignedseq_tbl (default: reset_meta_largevalueseq; size = TODO)
        # if ENABLE_LARGEVALUEBLOCK:
        print("Configuring access_largevaluebseq_tbl")
        for is_cached in cached_list:
            for validvalue in validvalue_list:
                matchspec0 = [
                    hex(GETREQ_INSWITCH),
                    hex(is_cached),
                    hex(validvalue),
                    hex(0),
                ]
                if is_cached == 1 and validvalue == 1:
                    controller.table_add(
                        "access_largevalueseq_and_save_assignedseq_tbl",
                        "get_largevalueseq",
                        matchspec0,
                    )
                for tmpoptype in [
                    GETRES_LATEST_SEQ_INSWITCH,
                    GETRES_DELETED_SEQ_INSWITCH,
                    PUTREQ_INSWITCH,
                    DELREQ_INSWITCH,
                ]:
                    matchspec0 = [
                        hex(tmpoptype),
                        hex(is_cached),
                        hex(validvalue),
                        hex(0),
                    ]
                    if is_cached == 1 and validvalue == 1:
                        controller.table_add(
                            "access_largevalueseq_and_save_assignedseq_tbl",
                            "reset_largevalueseq",
                            matchspec0,
                        )
                matchspec0 = [
                    hex(CACHE_POP_INSWITCH),
                    hex(is_cached),
                    hex(validvalue),
                    hex(0),
                ]
                controller.table_add(
                    "access_largevalueseq_and_save_assignedseq_tbl",
                    "reset_largevalueseq",
                    matchspec0,
                )
                # on-path in-switch invalidation for fragment 0 of PUTREQ_LARGEVALUE_INSWITCH
                matchspec0 = [
                    hex(PUTREQ_LARGEVALUE_INSWITCH),
                    hex(is_cached),
                    hex(validvalue),
                    hex(0),
                ]
                if is_cached == 1 and validvalue == 1:
                    controller.table_add(
                        "access_largevalueseq_and_save_assignedseq_tbl",
                        "set_largevalueseq",
                        matchspec0,
                    )

        # Stage 3

        # Table: access_deleted_tbl (default: reset_is_deleted; size = 122)
        print("Configuring access_deleted_tbl")
        for is_cached in cached_list:
            for validvalue in validvalue_list:
                for is_latest in latest_list:
                    for is_stat in stat_list:
                        matchspec0 = [
                            hex(GETREQ_INSWITCH),
                            hex(is_cached),
                            hex(validvalue),
                            hex(is_latest),
                            hex(is_stat),
                        ]
                        if is_cached == 1:
                            controller.table_add(
                                "access_deleted_tbl", "get_deleted", matchspec0
                            )
                        matchspec0 = [
                            hex(GETRES_LATEST_SEQ_INSWITCH),
                            hex(is_cached),
                            hex(validvalue),
                            hex(is_latest),
                            hex(is_stat),
                        ]
                        if (
                            is_cached == 1
                            and validvalue == 1
                            and is_latest == 0
                            and is_stat == 1
                        ):
                            controller.table_add(
                                "access_deleted_tbl",
                                "reset_and_get_deleted",
                                matchspec0,
                            )
                        matchspec0 = [
                            hex(GETRES_DELETED_SEQ_INSWITCH),
                            hex(1),
                            hex(validvalue),
                            hex(is_latest),
                            hex(is_stat),
                        ]
                        if (
                            is_cached == 1
                            and validvalue == 1
                            and is_latest == 0
                            and is_stat == 0
                        ):
                            controller.table_add(
                                "access_deleted_tbl", "set_and_get_deleted", matchspec0
                            )
                        matchspec0 = [
                            hex(PUTREQ_INSWITCH),
                            hex(is_cached),
                            hex(validvalue),
                            hex(is_latest),
                            hex(is_stat),
                        ]
                        if is_cached == 1 and validvalue == 1:
                            controller.table_add(
                                "access_deleted_tbl",
                                "reset_and_get_deleted",
                                matchspec0,
                            )
                        matchspec0 = [
                            hex(DELREQ_INSWITCH),
                            hex(is_cached),
                            hex(validvalue),
                            hex(is_latest),
                            hex(is_stat),
                        ]
                        if is_cached == 1 and validvalue == 1:
                            controller.table_add(
                                "access_deleted_tbl", "set_and_get_deleted", matchspec0
                            )
                        matchspec0 = [
                            hex(CACHE_POP_INSWITCH),
                            hex(is_cached),
                            hex(validvalue),
                            hex(is_latest),
                            hex(is_stat),
                        ]
                        if is_stat == 1:
                            controller.table_add(
                                "access_deleted_tbl",
                                "reset_and_get_deleted",
                                matchspec0,
                            )
                        elif is_stat == 0:
                            controller.table_add(
                                "access_deleted_tbl", "set_and_get_deleted", matchspec0
                            )
                        for tmpoptype in [
                            CACHE_EVICT_LOADDATA_INSWITCH,
                            LOADSNAPSHOTDATA_INSWITCH,
                        ]:
                            matchspec0 = [
                                hex(tmpoptype),
                                hex(is_cached),
                                hex(validvalue),
                                hex(is_latest),
                                hex(is_stat),
                            ]
                            controller.table_add(
                                "access_deleted_tbl", "get_deleted", matchspec0
                            )

        # Table: update_vallen_tbl (default: reset_access_val_mode; 62)
        print("Configuring update_vallen_tbl")
        for is_cached in cached_list:
            for validvalue in validvalue_list:
                for is_latest in latest_list:
                    matchspec0 = [
                        hex(GETREQ_INSWITCH),
                        hex(is_cached),
                        hex(validvalue),
                        hex(is_latest),
                    ]
                    if is_cached == 1:
                        controller.table_add(
                            "update_vallen_tbl", "get_vallen", matchspec0
                        )
                    for tmpoptype in [
                        GETRES_LATEST_SEQ_INSWITCH,
                        GETRES_DELETED_SEQ_INSWITCH,
                    ]:
                        matchspec0 = [
                            hex(tmpoptype),
                            hex(is_cached),
                            hex(validvalue),
                            hex(is_latest),
                        ]
                        if is_cached == 1 and validvalue == 1 and is_latest == 0:
                            controller.table_add(
                                "update_vallen_tbl", "set_and_get_vallen", matchspec0
                            )
                    matchspec0 = [
                        hex(PUTREQ_INSWITCH),
                        hex(is_cached),
                        hex(validvalue),
                        hex(is_latest),
                    ]
                    if is_cached == 1 and validvalue == 1:
                        controller.table_add(
                            "update_vallen_tbl", "set_and_get_vallen", matchspec0
                        )
                    matchspec0 = [
                        hex(DELREQ_INSWITCH),
                        hex(is_cached),
                        hex(validvalue),
                        hex(is_latest),
                    ]
                    if is_cached == 1 and validvalue == 1:
                        controller.table_add(
                            "update_vallen_tbl", "reset_and_get_vallen", matchspec0
                        )
                    matchspec0 = [
                        hex(CACHE_POP_INSWITCH),
                        hex(is_cached),
                        hex(validvalue),
                        hex(is_latest),
                    ]
                    controller.table_add(
                        "update_vallen_tbl", "set_and_get_vallen", matchspec0
                    )
                    for tmpoptype in [
                        CACHE_EVICT_LOADDATA_INSWITCH,
                        LOADSNAPSHOTDATA_INSWITCH,
                    ]:
                        matchspec0 = [
                            hex(tmpoptype),
                            hex(is_cached),
                            hex(validvalue),
                            hex(is_latest),
                        ]
                        controller.table_add(
                            "update_vallen_tbl", "get_vallen", matchspec0
                        )

        # Table: access_savedseq_tbl (default: NoAction	; size = 56-2+8=62)
        print("Configuring access_savedseq_tbl")
        for is_cached in cached_list:
            for validvalue in validvalue_list:
                for is_latest in latest_list:
                    matchspec0 = [
                        hex(GETREQ_INSWITCH),
                        hex(is_cached),
                        hex(validvalue),
                        hex(is_latest),
                    ]
                    # For GETRES_SEQ
                    # if is_cached == 1 and (validvalue == 1 or validvalue == 3) and is_latest == 1:
                    # For GETRES_SEQ, GETREQ_BEINGEVICTED_RECORD, and GETREQ_LARGEVALUEBLOCK_RECORD
                    if is_cached == 1:
                        controller.table_add(
                            "access_savedseq_tbl", "get_savedseq", matchspec0
                        )
                    for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH]:
                        matchspec0 = [
                            hex(tmpoptype),
                            hex(is_cached),
                            hex(validvalue),
                            hex(is_latest),
                        ]
                        if is_cached == 1 and validvalue == 1:
                            controller.table_add(
                                "access_savedseq_tbl",
                                "set_and_get_savedseq",
                                matchspec0,
                            )
                    for tmpoptype in [
                        GETRES_LATEST_SEQ_INSWITCH,
                        GETRES_DELETED_SEQ_INSWITCH,
                    ]:
                        matchspec0 = [
                            hex(tmpoptype),
                            hex(is_cached),
                            hex(validvalue),
                            hex(is_latest),
                        ]
                        if is_cached == 1 and validvalue == 1 and is_latest == 0:
                            controller.table_add(
                                "access_savedseq_tbl",
                                "set_and_get_savedseq",
                                matchspec0,
                            )
                    matchspec0 = [
                        hex(CACHE_POP_INSWITCH),
                        hex(is_cached),
                        hex(validvalue),
                        hex(is_latest),
                    ]
                    controller.table_add(
                        "access_savedseq_tbl", "set_and_get_savedseq", matchspec0
                    )
                    for tmpoptype in [
                        CACHE_EVICT_LOADDATA_INSWITCH,
                        LOADSNAPSHOTDATA_INSWITCH,
                    ]:
                        matchspec0 = [
                            hex(tmpoptype),
                            hex(is_cached),
                            hex(validvalue),
                            hex(is_latest),
                        ]
                        controller.table_add(
                            "access_savedseq_tbl", "get_savedseq", matchspec0
                        )

        # Table: access_case1_tbl (default: reset_is_case1; 6)
        print("Configuring access_case1_tbl")
        for is_latest in latest_list:
            for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                matchspec0 = [hex(tmpoptype), hex(1), hex(1), hex(is_latest), hex(1)]
                if is_latest == 0:
                    controller.table_add("access_case1_tbl", "try_case1", matchspec0)
            for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH]:
                matchspec0 = [hex(tmpoptype), hex(1), hex(1), hex(is_latest), hex(1)]
                controller.table_add("access_case1_tbl", "try_case1", matchspec0)

        # Stage 4-11

        # Table: update_vallo1_tbl (default: nop; 14)
        for i in range(1, 17):
            print("Configuring update_vallo{}_tbl".format(i))
            self.configure_update_val_tbl("lo{}".format(i))
            print("Configuring update_valhi{}_tbl".format(i))
            self.configure_update_val_tbl("hi{}".format(i))

        # Stage 9

        # Table: lastclone_lastscansplit_tbl (default: reset_is_lastclone_lastscansplit; size = 4)
        print("Configuring lastclone_lastscansplit_tbl")

        # CACHE_POP_INSWITCH_ACK
        for tmpoptype in [
            GETRES_LATEST_SEQ_INSWITCH_CASE1,
            GETRES_DELETED_SEQ_INSWITCH_CASE1,
            PUTREQ_SEQ_INSWITCH_CASE1,
            DELREQ_SEQ_INSWITCH_CASE1,
        ]:
            matchspec0 = [hex(tmpoptype), hex(0)]
            controller.table_add(
                "lastclone_lastscansplit_tbl", "set_is_lastclone", matchspec0
            )

        # Stage 8

        # Table; another_eg_port_forward_tbl (default: NoAction	; size = < 4096)
        print("Configuring another_eg_port_forward_tbl")
        # if ENABLE_LARGEVALUEBLOCK == True:
        #    self.configure_another_eg_port_forward_tbl_largevalueblock()
        # else:
        #    self.configure_another_eg_port_forward_tbl()
        self.configure_another_eg_port_forward_tbl_largevalueblock()

        # Stage 9

        # Table: eg_port_forward_tbl (default: NoAction	; size = < 2048 < 8192)
        print("Configuring eg_port_forward_tbl")
        self.configure_eg_port_forward_tbl()

        # Table: update_pktlen_tbl (default: NoAction	; 15*17+14=269)
        print("Configuring update_pktlen_tbl")
        for i in range(int(int(switch_max_vallen / 8 + 1))):  # i from 0 to 16
            if i == 0:
                vallen_start = 0
                vallen_end = 0
                aligned_vallen = 0
            else:
                vallen_start = (i - 1) * 8 + 1  # 1, 9, ..., 121
                vallen_end = (i - 1) * 8 + 8  # 8, 16, ..., 128
                aligned_vallen = vallen_end  # 8, 16, ..., 128
            val_stat_seq_udplen = aligned_vallen + 42
            val_stat_seq_iplen = aligned_vallen + 62
            val_seq_inswitch_stat_clone_udplen = aligned_vallen + 66
            val_seq_inswitch_stat_clone_iplen = aligned_vallen + 86
            val_seq_udplen = aligned_vallen + 38
            val_seq_iplen = aligned_vallen + 58
            val_seq_stat_udplen = aligned_vallen + 42
            val_seq_stat_iplen = aligned_vallen + 62
            val_seq_inswitch_stat_udplen = aligned_vallen + 62
            val_seq_inswitch_stat_iplen = aligned_vallen + 82
            for tmpoptype in [
                GETRES_SEQ,
                GETREQ_BEINGEVICTED_RECORD,
                GETREQ_LARGEVALUEBLOCK_RECORD,
            ]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + hex(vallen_start) + "->" + hex(vallen_end),
                ]  # [vallen_start, vallen_end]
                actnspec0 = [hex(val_stat_seq_udplen), hex(val_stat_seq_iplen)]
                controller.table_add(
                    "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
                )  # 0 is priority (range may be overlapping]
            for tmpoptype in [
                GETRES_LATEST_SEQ_INSWITCH_CASE1,
                GETRES_DELETED_SEQ_INSWITCH_CASE1,
                PUTREQ_SEQ_INSWITCH_CASE1,
                DELREQ_SEQ_INSWITCH_CASE1,
            ]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + hex(vallen_start) + "->" + hex(vallen_end),
                ]  # [vallen_start, vallen_end]
                actnspec0 = [
                    hex(val_seq_inswitch_stat_clone_udplen),
                    hex(val_seq_inswitch_stat_clone_iplen),
                ]
                controller.table_add(
                    "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
                )  # 0 is priority (range may be overlapping]
            for tmpoptype in [
                PUTREQ_SEQ,
                PUTREQ_POP_SEQ,
                PUTREQ_SEQ_CASE3,
                PUTREQ_POP_SEQ_CASE3,
                PUTREQ_SEQ_BEINGEVICTED,
                PUTREQ_SEQ_CASE3_BEINGEVICTED,
            ]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + hex(vallen_start) + "->" + hex(vallen_end),
                ]  # [vallen_start, vallen_end]
                actnspec0 = [hex(val_seq_udplen), hex(val_seq_iplen)]
                controller.table_add(
                    "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
                )  # 0 is priority (range may be overlapping]
            matchspec0 = [
                hex(CACHE_EVICT_LOADDATA_INSWITCH_ACK),
                "" + hex(vallen_start) + "->" + hex(vallen_end),
            ]  # [vallen_start, vallen_end]
            actnspec0 = [hex(val_seq_stat_udplen), hex(val_seq_stat_iplen)]
            controller.table_add(
                "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
            )  # 0 is priority (range may be overlapping]
            matchspec0 = [
                hex(LOADSNAPSHOTDATA_INSWITCH_ACK),
                "" + hex(vallen_start) + "->" + hex(vallen_end),
            ]  # [vallen_start, vallen_end]
            actnspec0 = [
                hex(val_seq_inswitch_stat_udplen),
                hex(val_seq_inswitch_stat_iplen),
            ]
            controller.table_add(
                "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
            )  # 0 is priority (range may be overlapping]

        onlyop_udplen = 26
        onlyop_iplen = 46
        seq_stat_udplen = 40
        seq_stat_iplen = 60
        seq_udplen = 36
        seq_iplen = 56
        scanreqsplit_udplen = 49
        scanreqsplit_iplen = 69
        frequency_udplen = 30
        frequency_iplen = 50
        matchspec0 = [
            hex(CACHE_POP_INSWITCH_ACK),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]  # [0, 128]
        actnspec0 = [hex(onlyop_udplen), hex(onlyop_iplen)]
        controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping]
        for tmpoptype in [PUTRES_SEQ, DELRES_SEQ]:
            matchspec0 = [
                hex(tmpoptype),
                "" + hex(0) + "->" + hex(switch_max_vallen),
            ]  # [0, 128]
            actnspec0 = [hex(seq_stat_udplen), hex(seq_stat_iplen)]
            controller.table_add(
                "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
            )  # 0 is priority (range may be overlapping]
        # , GETREQ_LARGEVALUEBLOCK_SEQ
        for tmpoptype in [
            DELREQ_SEQ,
            DELREQ_SEQ_CASE3,
            DELREQ_SEQ_BEINGEVICTED,
            DELREQ_SEQ_CASE3_BEINGEVICTED,
        ]:
            matchspec0 = [
                hex(tmpoptype),
                "" + hex(0) + "->" + hex(switch_max_vallen),
            ]  # [0, 128]
            actnspec0 = [hex(seq_udplen), hex(seq_iplen)]
            controller.table_add(
                "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
            )  # 0 is priority (range may be overlapping]
        matchspec0 = [
            hex(SCANREQ_SPLIT),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]  # [0, 128]
        actnspec0 = [hex(scanreqsplit_udplen), hex(scanreqsplit_iplen)]
        controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping]
        matchspec0 = [
            hex(CACHE_EVICT_LOADFREQ_INSWITCH_ACK),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]  # [0, 128]
        actnspec0 = [hex(frequency_udplen), hex(frequency_iplen)]
        controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping]
        matchspec0 = [
            hex(SETVALID_INSWITCH_ACK),
            "" + hex(0) + "->" + hex(switch_max_vallen),
        ]  # [0, 128]
        actnspec0 = [hex(onlyop_udplen), hex(onlyop_iplen)]
        controller.table_add(
            "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
        )  # 0 is priority (range may be overlapping]
        # For large value
        shadowtype_seq_udp_delta = 10
        shadowtype_seq_ip_delta = 10
        for tmpoptype in [
            PUTREQ_LARGEVALUE_SEQ,
            PUTREQ_LARGEVALUE_SEQ_CASE3,
            PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED,
            PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED,
        ]:
            matchspec0 = [
                hex(tmpoptype),
                "" + hex(0) + "->" + hex(65535),
            ]  # [0, 65535] (NOTE: vallen MUST = 0 for PUTREQ_LARGEVALUE_INSWITCH]
            actnspec0 = [hex(shadowtype_seq_udp_delta), hex(shadowtype_seq_ip_delta)]
            controller.table_add(
                "update_pktlen_tbl", "add_pktlen", matchspec0, actnspec0, 0
            )  # 0 is priority (range may be overlapping)

        # Table: update_ipmac_srcport_tbl (default: NoAction	; 6*client_physical_num+20*server_physical_num+7=59 < 26*8+7=215 < 256)
        # NOTE: udp.dstport is updated by eg_port_forward_tbl (only required by switch2switchos)
        # NOTE: update_ipmac_srcport_tbl focues on src/dst ip/mac and udp.srcport
        print("Configuring update_ipmac_srcport_tbl")
        # (1) for response from server to client, egress port has been set based on ip.dstaddr (or by clone_i2e) in ingress pipeline
        # (2) for response from switch to client, egress port has been set by clone_e2e in egress pipeline
        for tmp_client_physical_idx in range(client_physical_num):
            tmp_devport = self.client_devports[tmp_client_physical_idx]
            tmp_client_mac = client_macs[tmp_client_physical_idx]
            tmp_client_ip = client_ips[tmp_client_physical_idx]
            tmp_server_mac = server_macs[0]
            tmp_server_ip = server_ips[0]
            actnspec0 = [
                tmp_client_mac,
                tmp_server_mac,
                tmp_client_ip,
                tmp_server_ip,
                hex(server_worker_port_start),
            ]
            # GETRES, GETRES_LARGEVALUE, PUTRES, DELRES
            for tmpoptype in [
                GETRES_SEQ,
                PUTRES_SEQ,
                DELRES_SEQ,
                SCANRES_SPLIT,
                WARMUPACK,
                LOADACK,
                GETRES_LARGEVALUE_SEQ,
            ]:
                matchspec0 = [hex(tmpoptype), tmp_devport]
                controller.table_add(
                    "update_ipmac_srcport_tbl",
                    "update_ipmac_srcport_server2client",
                    matchspec0,
                    actnspec0,
                )
        # for request from client to server, egress port has been set by partition_tbl in ingress pipeline
        for tmp_server_physical_idx in range(server_physical_num):
            tmp_devport = self.server_devports[tmp_server_physical_idx]
            tmp_server_mac = server_macs[tmp_server_physical_idx]
            tmp_server_ip = server_ips[tmp_server_physical_idx]
            actnspec1 = [tmp_server_mac, tmp_server_ip]
            # , GETREQ_BEINGEVICTED, GETREQ_LARGEVALUEBLOCK_SEQ
            for tmpoptype in [
                GETREQ,
                GETREQ_NLATEST,
                PUTREQ_SEQ,
                DELREQ_SEQ,
                SCANREQ_SPLIT,
                GETREQ_POP,
                PUTREQ_POP_SEQ,
                PUTREQ_SEQ_CASE3,
                PUTREQ_POP_SEQ_CASE3,
                DELREQ_SEQ_CASE3,
                WARMUPREQ,
                LOADREQ,
                PUTREQ_LARGEVALUE_SEQ,
                PUTREQ_LARGEVALUE_SEQ_CASE3,
                PUTREQ_SEQ_BEINGEVICTED,
                PUTREQ_SEQ_CASE3_BEINGEVICTED,
                DELREQ_SEQ_BEINGEVICTED,
                DELREQ_SEQ_CASE3_BEINGEVICTED,
                PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED,
                PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED,
                GETREQ_BEINGEVICTED_RECORD,
                GETREQ_LARGEVALUEBLOCK_RECORD,
            ]:
                matchspec0 = [hex(tmpoptype), tmp_devport]
                controller.table_add(
                    "update_ipmac_srcport_tbl",
                    "update_dstipmac_client2server",
                    matchspec0,
                    actnspec1,
                )
        # Here we use server_mac/ip to simulate reflector_mac/ip = switchos_mac/ip
        # (1) eg_intr_md.egress_port of the first GETRES_CASE1 is set by ipv4_forward_tbl (as ingress port), which will be finally dropped -> update ip/mac/srcport or not is not important
        # (2) eg_intr_md.egress_port of cloned GETRES_CASE1s is set by clone_e2e, which must be the devport towards switchos (aka reflector)
        # (3) eg_intr_md.egress_port of the first ACK for cache population/eviction is set by partition_tbl in ingress pipeline, which will be finally dropped -> update ip/mac/srcport or not is not important
        # (4) eg_intr_md.egress_port of the cloned ACK for cache population/eviction is set by clone_e2e, which must be the devport towards switchos (aka reflector)
        tmp_devport = self.reflector_devport
        tmp_client_ip = client_ips[0]
        tmp_client_mac = client_macs[0]
        tmp_client_port = 123  # not cared by servers
        actnspec2 = [
            tmp_client_mac,
            self.reflector_mac_for_switch,
            tmp_client_ip,
            self.reflector_ip_for_switch,
            hex(tmp_client_port),
        ]
        for tmpoptype in [
            GETRES_LATEST_SEQ_INSWITCH_CASE1,
            GETRES_DELETED_SEQ_INSWITCH_CASE1,
            PUTREQ_SEQ_INSWITCH_CASE1,
            DELREQ_SEQ_INSWITCH_CASE1,
            CACHE_POP_INSWITCH_ACK,
            CACHE_EVICT_LOADFREQ_INSWITCH_ACK,
            CACHE_EVICT_LOADDATA_INSWITCH_ACK,
            LOADSNAPSHOTDATA_INSWITCH_ACK,
            SETVALID_INSWITCH_ACK,
        ]:
            matchspec0 = [hex(tmpoptype), tmp_devport]
            controller.table_add(
                "update_ipmac_srcport_tbl",
                "update_ipmac_srcport_switch2switchos",
                matchspec0,
                actnspec2,
            )

        # Table: add_and_remove_value_header_tbl (default: remove_all; 17*16=272)
        print("Configuring add_and_remove_value_header_tbl")
        # NOTE: egress pipeline must not output PUTREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, CACHE_POP_INSWITCH, and PUTREQ_INSWITCH
        # NOTE: even for future PUTREQ_LARGE/GETRES_LARGE, as their values should be in payload, we should invoke add_only_vallen() for vallen in [0, global_max_vallen]
        # LOADREQ, GETRES
        for tmpoptype in [
            PUTREQ_SEQ,
            PUTREQ_POP_SEQ,
            PUTREQ_SEQ_CASE3,
            PUTREQ_POP_SEQ_CASE3,
            GETRES_LATEST_SEQ_INSWITCH_CASE1,
            GETRES_DELETED_SEQ_INSWITCH_CASE1,
            PUTREQ_SEQ_INSWITCH_CASE1,
            DELREQ_SEQ_INSWITCH_CASE1,
            GETRES_SEQ,
            CACHE_EVICT_LOADDATA_INSWITCH_ACK,
            LOADSNAPSHOTDATA_INSWITCH_ACK,
            PUTREQ_SEQ_BEINGEVICTED,
            PUTREQ_SEQ_CASE3_BEINGEVICTED,
            GETREQ_BEINGEVICTED_RECORD,
            GETREQ_LARGEVALUEBLOCK_RECORD,
        ]:
            for i in range(int(switch_max_vallen / 8 + 1)):  # i from 0 to 16
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
                    controller.table_add(
                        "add_and_remove_value_header_tbl",
                        "add_only_vallen",
                        matchspec0,
                        [],
                        0,
                    )
                else:
                    controller.table_add(
                        "add_and_remove_value_header_tbl",
                        "add_to_val{}".format(i),
                        matchspec0,
                        [],
                        0,
                    )

        # Table: drop_tbl (default: NoAction	; size = 2)
        print("Configuring drop_tbl")
        matchspec0 = [hex(GETRES_LATEST_SEQ_INSWITCH)]
        controller.table_add("drop_tbl", "drop_getres_latest_seq_inswitch", matchspec0)
        matchspec0 = [hex(GETRES_DELETED_SEQ_INSWITCH)]
        controller.table_add("drop_tbl", "drop_getres_deleted_seq_inswitch", matchspec0)

    def configure_another_eg_port_forward_tbl(self):
        # Table: another_eg_port_forward_tbl (default: NoAction	; size = 168)
        tmp_client_sids = [0] + self.client_sids
        for is_cached in cached_list:
            for is_hot in hot_list:
                for validvalue in validvalue_list:
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
                                    for snapshot_flag in snapshot_flag_list:
                                        for is_case1 in case1_list:
                                            # is_lastclone_for_pktloss, snapshot_flag, and is_case1 should be 0 for GETREQ_INSWITCH
                                            if (
                                                is_lastclone_for_pktloss == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                                and tmp_client_sid != 0
                                            ):
                                                # size = 64*client_physical_num=128 < 64*8=512
                                                matchspec0 = [
                                                    hex(GETREQ_INSWITCH),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                if is_cached == 0:
                                                    if is_hot == 1:
                                                        # Update GETREQ_INSWITCH as GETREQ_POP to server

                                                        controller.table_add(
                                                            "another_eg_port_forward_tbl",
                                                            "update_getreq_inswitch_to_getreq_pop",
                                                            matchspec0,
                                                        )
                                                    else:
                                                        # Update GETREQ_INSWITCH as GETREQ to server

                                                        controller.table_add(
                                                            "another_eg_port_forward_tbl",
                                                            "update_getreq_inswitch_to_getreq",
                                                            matchspec0,
                                                        )
                                                else:
                                                    if validvalue == 0:
                                                        # Update GETREQ_INSWITCH as GETREQ to server

                                                        controller.table_add(
                                                            "another_eg_port_forward_tbl",
                                                            "update_getreq_inswitch_to_getreq",
                                                            matchspec0,
                                                        )
                                                    elif validvalue == 1:
                                                        if is_latest == 0:
                                                            # Update GETREQ_INSWITCH as GETREQ_NLATEST to server

                                                            controller.table_add(
                                                                "another_eg_port_forward_tbl",
                                                                "update_getreq_inswitch_to_getreq_nlatest",
                                                                matchspec0,
                                                            )
                                                        else:
                                                            # Update GETREQ_INSWITCH as GETRES_SEQ to client by mirroring
                                                            actnspec0 = [
                                                                hex(tmp_client_sid),
                                                                hex(
                                                                    server_worker_port_start
                                                                ),
                                                                hex(tmpstat),
                                                            ]
                                                            controller.table_add(
                                                                "another_eg_port_forward_tbl",
                                                                "update_getreq_inswitch_to_getres_seq_by_mirroring",
                                                                matchspec0,
                                                                actnspec0,
                                                            )
                                                    elif validvalue == 3:
                                                        if is_latest == 0:
                                                            ## Update GETREQ_INSWITCH as GETREQ_BEINGEVICTED to server
                                                            # controller.table_add('eg_port_forward_tbl','update_getreq_inswitch_to_getreq_beingevicted',matchspec0)
                                                            # Update GETREQ_INSWITCH as GETREQ_BEINGEVICTED_RECORD to server
                                                            actnspec0 = [hex(tmpstat)]
                                                            controller.table_add(
                                                                "another_eg_port_forward_tbl",
                                                                "update_getreq_inswitch_to_getreq_beingevicted_record",
                                                                matchspec0,
                                                                actnspec0,
                                                            )
                                                        else:
                                                            # Update GETREQ_INSWITCH as GETRES_SEQ to client by mirroring
                                                            actnspec0 = [
                                                                hex(tmp_client_sid),
                                                                hex(
                                                                    server_worker_port_start
                                                                ),
                                                                hex(tmpstat),
                                                            ]
                                                            controller.table_add(
                                                                "another_eg_port_forward_tbl",
                                                                "update_getreq_inswitch_to_getres_seq_by_mirroring",
                                                                matchspec0,
                                                                actnspec0,
                                                            )
                                            # is_cached (no inswitch_hdr due to no field list when clone_i2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline (no inswitch_hdr), tmp_client_sid=0 (no inswitch hdr), is_lastclone_for_pktloss, snapshot_flag (no inswitch_hdr), is_case1 should be 0 for GETRES_LATEST_SEQ
                                            # NOTE: we use sid == self.sids[0] to avoid duplicate entry; we use inswitch_hdr_client_sid = 0 to match the default value of inswitch_hdr.client_sid
                                            # size = 1
                                            # if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                            if (
                                                is_cached == 0
                                                and is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(GETRES_LATEST_SEQ),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                # TODO: check if we need to set egress port for packet cloned by clone_i2e
                                                # Update GETRES_LATEST_SEQ (by clone_i2e) as GETRES_SEQ to client
                                                controller.table_add(
                                                    "another_eg_port_forward_tbl",
                                                    "update_getres_latest_seq_to_getres_seq",
                                                    matchspec0,
                                                )
                                            # is_hot (cm_predicate=1), is_wrong_pipeline (not need range/hash partition), tmp_client_sid=0 (not need mirroring for res), is_lastclone_for_pktloss should be 0 for GETRES_LATEST_SEQ_INSWITCH
                                            # size = 128 -> 2
                                            # if is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                            if (
                                                is_hot == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                            ):
                                                matchspec0 = [
                                                    hex(GETRES_LATEST_SEQ_INSWITCH),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                if (
                                                    is_cached == 1
                                                    and validvalue == 1
                                                    and is_latest == 0
                                                    and snapshot_flag == 1
                                                    and is_case1 == 0
                                                ):
                                                    # Update GETRES_LATEST_SEQ_INSWITCH as GETRES_LATEST_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                    if (
                                                        is_deleted == 0
                                                    ):  # is_deleted=0 -> stat=1
                                                        actnspec0 = [
                                                            hex(self.reflector_sid),
                                                            hex(1),
                                                            hex(
                                                                reflector_dp2cpserver_port
                                                            ),
                                                        ]
                                                    elif (
                                                        is_deleted == 1
                                                    ):  # is_deleted=1 -> stat=0
                                                        actnspec0 = [
                                                            hex(self.reflector_sid),
                                                            hex(0),
                                                            hex(
                                                                reflector_dp2cpserver_port
                                                            ),
                                                        ]
                                                    controller.table_add(
                                                        "another_eg_port_forward_tbl",
                                                        "update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss",
                                                        matchspec0,
                                                        actnspec0,
                                                    )
                                                # Keep GETERS_LATEST_SEQ_INSWITCH unchanged, and resort to drop_tbl to drop it
                                                # else:
                                                #    # Drop GETRES_LATEST_SEQ_INSWITCH
                                                #    controller.table_add('another_eg_port_forward_tbl','drop_getres_latest_seq_inswitch',matchspec0)
                                            # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_wrong_pipeline=0, tmp_client_sid=0, and snapshot_flag=1 (same inswitch_hdr as GETRES_LATEST_SEQ_INSWITCH); is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_case1 should be 0 for GETRES_LATEST_SEQ_INSWITCH_CASE1
                                            # size = 1
                                            # if is_cached == 1 and is_wrong_pipeline == 0 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_case1 == 0:
                                            if (
                                                is_cached == 1
                                                and snapshot_flag == 1
                                                and is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and tmp_client_sid == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(
                                                        GETRES_LATEST_SEQ_INSWITCH_CASE1
                                                    ),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                if is_lastclone_for_pktloss == 0:
                                                    # Forward GETRES_LATEST_SEQ_INSWITCH_CASE0 (by clone_e2e) to reflector (w/ clone)
                                                    actnspec0 = [
                                                        hex(self.reflector_sid)
                                                    ]
                                                    controller.table_add(
                                                        "another_eg_port_forward_tbl",
                                                        "forward_getres_latest_seq_inswitch_case1_clone_for_pktloss",
                                                        matchspec0,
                                                        actnspec0,
                                                    )
                                                elif is_lastclone_for_pktloss == 1:
                                                    # Forward GETRES_LATEST_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector
                                                    # controller.table_add('another_eg_port_forward_tbl','forward_getres_latest_seq_inswitch_case1',matchspec0)
                                                    # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                    pass
                                            # is_cached (no inswitch_hdr due to no field list when clone_i2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline (no inswitch_hdr), tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktloss, snapshot_flag (no inswitch_hdr), is_case1 should be 0 for GETRES_DELETED_SEQ
                                            # size = 1
                                            # if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                            if (
                                                is_cached == 0
                                                and is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(GETRES_DELETED_SEQ),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                # TODO: check if we need to set egress port for packet cloned by clone_i2e
                                                # Update GETRES_DELETED_SEQ (by clone_i2e) as GETRES_SEQ to client
                                                controller.table_add(
                                                    "another_eg_port_forward_tbl",
                                                    "update_getres_deleted_seq_to_getres_seq",
                                                    matchspec0,
                                                )
                                            # is_hot (cm_predicate=1), is_wrong_pipeline (not need range/hash partition), tmp_client_sid=0 (not need mirroring for res), is_lastclone_for_pktloss should be 0 for GETRES_DELETED_SEQ_INSWITCH
                                            # size = 128 -> 2
                                            # if is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                            if (
                                                is_hot == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                            ):
                                                matchspec0 = [
                                                    hex(GETRES_DELETED_SEQ_INSWITCH),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                if (
                                                    is_cached == 1
                                                    and validvalue == 1
                                                    and is_latest == 0
                                                    and snapshot_flag == 1
                                                    and is_case1 == 0
                                                ):
                                                    # Update GETRES_DELETED_SEQ_INSWITCH as GETRES_DELETED_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                    if (
                                                        is_deleted == 0
                                                    ):  # is_deleted=0 -> stat=1
                                                        actnspec0 = [
                                                            hex(self.reflector_sid),
                                                            hex(1),
                                                            hex(
                                                                reflector_dp2cpserver_port
                                                            ),
                                                        ]
                                                    elif (
                                                        is_deleted == 1
                                                    ):  # is_deleted=1 -> stat=0
                                                        actnspec0 = [
                                                            hex(self.reflector_sid),
                                                            hex(0),
                                                            hex(
                                                                reflector_dp2cpserver_port
                                                            ),
                                                        ]
                                                    controller.table_add(
                                                        "another_eg_port_forward_tbl",
                                                        "update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss",
                                                        matchspec0,
                                                        actnspec0,
                                                    )
                                                # Keep GETERS_DELETED_SEQ_INSWITCH unchanged, and resort to drop_tbl to drop it
                                                # else:
                                                #    # Drop GETRES_DELETED_SEQ_INSWITCH
                                                #    controller.table_add('another_eg_port_forward_tbl','drop_getres_deleted_seq_inswitch',matchspec0)
                                            # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_wrong_pipeline=0, tmp_client_sid=0, and snapshot_flag=1 (same inswitch_hdr as GETRES_LATEST_SEQ_INSWITCH); is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_case1 should be 0 for GETRES_DELETED_SEQ_INSWITCH_CASE1
                                            # size = 1
                                            # if is_cached == 1 and is_wrong_pipeline == 0 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_case1 == 0:
                                            if (
                                                is_cached == 1
                                                and snapshot_flag == 1
                                                and is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and tmp_client_sid == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(
                                                        GETRES_DELETED_SEQ_INSWITCH_CASE1
                                                    ),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                if is_lastclone_for_pktloss == 0:
                                                    # Forward GETRES_DELETED_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                    actnspec0 = [
                                                        hex(self.reflector_sid)
                                                    ]
                                                    controller.table_add(
                                                        "another_eg_port_forward_tbl",
                                                        "forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss",
                                                        matchspec0,
                                                        actnspec0,
                                                    )
                                                elif is_lastclone_for_pktloss == 1:
                                                    # Forward GETRES_DELETED_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector
                                                    # controller.table_add('another_eg_port_forward_tbl','forward_getres_deleted_seq_inswitch_case1',matchspec0)
                                                    # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                    pass
                                            # is_hot (cm_predicate=1), is_deleted, tmp_client_sid=0, is_lastclone_for_pktloss, is_case1 should be 0 for PUTREQ_LARGEVALUE_INSWITCH
                                            # NOTE: is_cached can be 0 or 1 (key may be / may not be cached for PUTREQ_LARGEVALUE_INSWITCH)
                                            # NOTE: validvalue can be 0/1/3 for PUTREQ_LARGEVALUE_INSWITCH
                                            # size = 32
                                            if (
                                                is_hot == 0
                                                and is_deleted == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(PUTREQ_LARGEVALUE_INSWITCH),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                if is_cached == 1 and validvalue == 3:
                                                    if snapshot_flag == 0:
                                                        # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED to server
                                                        controller.table_add(
                                                            "another_eg_port_forward_tbl",
                                                            "update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_beingevicted",
                                                            matchspec0,
                                                        )
                                                    elif snapshot_flag == 1:
                                                        # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED to server
                                                        controller.table_add(
                                                            "another_eg_port_forward_tbl",
                                                            "update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_case3_beingevicted",
                                                            matchspec0,
                                                        )
                                                else:
                                                    if snapshot_flag == 0:
                                                        # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ to server
                                                        controller.table_add(
                                                            "another_eg_port_forward_tbl",
                                                            "update_putreq_largevalue_inswitch_to_putreq_largevalue_seq",
                                                            matchspec0,
                                                        )
                                                    elif snapshot_flag == 1:
                                                        # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ_CASE3 to server
                                                        controller.table_add(
                                                            "another_eg_port_forward_tbl",
                                                            "update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_case3",
                                                            matchspec0,
                                                        )

    def configure_another_eg_port_forward_tbl_largevalueblock(self):
        # Table: another_eg_port_forward_tbl (default: NoAction	; size = 296)
        tmp_client_sids = [0] + self.client_sids
        for is_cached in cached_list:
            for is_hot in hot_list:
                for validvalue in validvalue_list:
                    for is_latest in latest_list:
                        for is_largevalueblock in is_largevalueblock_list:
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
                                        for snapshot_flag in snapshot_flag_list:
                                            for is_case1 in case1_list:
                                                # is_lastclone_for_pktloss, snapshot_flag, and is_case1 should be 0 for GETREQ_INSWITCH
                                                if (
                                                    is_lastclone_for_pktloss == 0
                                                    and snapshot_flag == 0
                                                    and is_case1 == 0
                                                    and tmp_client_sid != 0
                                                ):
                                                    # size = 128*client_physical_num=256 < 128*8=1024
                                                    matchspec0 = [
                                                        hex(GETREQ_INSWITCH),
                                                        hex(is_cached),
                                                        hex(is_hot),
                                                        hex(validvalue),
                                                        hex(is_latest),
                                                        hex(is_largevalueblock),
                                                        hex(is_deleted),
                                                        hex(tmp_client_sid),
                                                        hex(is_lastclone_for_pktloss),
                                                        hex(snapshot_flag),
                                                        hex(is_case1),
                                                    ]
                                                    if is_cached == 0:
                                                        if is_hot == 1:
                                                            # Update GETREQ_INSWITCH as GETREQ_POP to server

                                                            controller.table_add(
                                                                "another_eg_port_forward_tbl",
                                                                "update_getreq_inswitch_to_getreq_pop",
                                                                matchspec0,
                                                            )
                                                        else:
                                                            # Update GETREQ_INSWITCH as GETREQ to server

                                                            controller.table_add(
                                                                "another_eg_port_forward_tbl",
                                                                "update_getreq_inswitch_to_getreq",
                                                                matchspec0,
                                                            )
                                                    else:
                                                        if validvalue == 0:
                                                            # Update GETREQ_INSWITCH as GETREQ to server

                                                            controller.table_add(
                                                                "another_eg_port_forward_tbl",
                                                                "update_getreq_inswitch_to_getreq",
                                                                matchspec0,
                                                            )
                                                        elif validvalue == 1:
                                                            if is_latest == 0:
                                                                if (
                                                                    is_largevalueblock
                                                                    == 1
                                                                ):
                                                                    ## Update GETREQ_INSWITCH as GETREQ_LARGEVALUEBLOCK_SEQ to server
                                                                    # controller.table_add('another_eg_port_forward_tbl','update_getreq_inswitch_to_getreq_largevalueblock_seq',matchspec0)
                                                                    # Update GETREQ_INSWITCH as GETREQ_LARGEVALUEBLOCK_RECORD to server
                                                                    actnspec0 = [
                                                                        hex(tmpstat)
                                                                    ]
                                                                    controller.table_add(
                                                                        "another_eg_port_forward_tbl",
                                                                        "update_getreq_inswitch_to_getreq_largevalueblock_record",
                                                                        matchspec0,
                                                                        actnspec0,
                                                                    )
                                                                elif (
                                                                    is_largevalueblock
                                                                    == 0
                                                                ):
                                                                    # Update GETREQ_INSWITCH as GETREQ_NLATEST to server

                                                                    controller.table_add(
                                                                        "another_eg_port_forward_tbl",
                                                                        "update_getreq_inswitch_to_getreq_nlatest",
                                                                        matchspec0,
                                                                    )
                                                            else:
                                                                # Update GETREQ_INSWITCH as GETRES_SEQ to client by mirroring
                                                                actnspec0 = [
                                                                    hex(tmp_client_sid),
                                                                    hex(
                                                                        server_worker_port_start
                                                                    ),
                                                                    hex(tmpstat),
                                                                ]
                                                                controller.table_add(
                                                                    "another_eg_port_forward_tbl",
                                                                    "update_getreq_inswitch_to_getres_seq_by_mirroring",
                                                                    matchspec0,
                                                                    actnspec0,
                                                                )
                                                        elif validvalue == 3:
                                                            if is_latest == 0:
                                                                ## Update GETREQ_INSWITCH as GETREQ_BEINGEVICTED to server
                                                                # controller.table_add('another_eg_port_forward_tbl','update_getreq_inswitch_to_getreq_beingevicted',matchspec0)
                                                                # Update GETREQ_INSWITCH as GETREQ_BEINGEVICTED_RECORD to server
                                                                actnspec0 = [
                                                                    hex(tmpstat)
                                                                ]
                                                                controller.table_add(
                                                                    "another_eg_port_forward_tbl",
                                                                    "update_getreq_inswitch_to_getreq_beingevicted_record",
                                                                    matchspec0,
                                                                    actnspec0,
                                                                )
                                                            else:
                                                                # Update GETREQ_INSWITCH as GETRES_SEQ to client by mirroring
                                                                actnspec0 = [
                                                                    hex(tmp_client_sid),
                                                                    hex(
                                                                        server_worker_port_start
                                                                    ),
                                                                    hex(tmpstat),
                                                                ]
                                                                controller.table_add(
                                                                    "another_eg_port_forward_tbl",
                                                                    "update_getreq_inswitch_to_getres_seq_by_mirroring",
                                                                    matchspec0,
                                                                    actnspec0,
                                                                )
                                                # is_cached (no inswitch_hdr due to no field list when clone_i2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline (no inswitch_hdr), tmp_client_sid=0 (no inswitch hdr), is_lastclone_for_pktloss, snapshot_flag (no inswitch_hdr), is_case1 should be 0 for GETRES_LATEST_SEQ
                                                # NOTE: we use sid == self.sids[0] to avoid duplicate entry; we use inswitch_hdr_client_sid = 0 to match the default value of inswitch_hdr.client_sid
                                                # size = 1
                                                # if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                if (
                                                    is_cached == 0
                                                    and is_hot == 0
                                                    and validvalue == 0
                                                    and is_latest == 0
                                                    and is_largevalueblock == 0
                                                    and is_deleted == 0
                                                    and tmp_client_sid == 0
                                                    and is_lastclone_for_pktloss == 0
                                                    and snapshot_flag == 0
                                                    and is_case1 == 0
                                                ):
                                                    matchspec0 = [
                                                        hex(GETRES_LATEST_SEQ),
                                                        hex(is_cached),
                                                        hex(is_hot),
                                                        hex(validvalue),
                                                        hex(is_latest),
                                                        hex(is_largevalueblock),
                                                        hex(is_deleted),
                                                        # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        hex(tmp_client_sid),
                                                        hex(is_lastclone_for_pktloss),
                                                        hex(snapshot_flag),
                                                        hex(is_case1),
                                                    ]
                                                    # TODO: check if we need to set egress port for packet cloned by clone_i2e
                                                    # Update GETRES_LATEST_SEQ (by clone_i2e) as GETRES_SEQ to client
                                                    controller.table_add(
                                                        "another_eg_port_forward_tbl",
                                                        "update_getres_latest_seq_to_getres_seq",
                                                        matchspec0,
                                                    )
                                                # is_hot (cm_predicate=1), is_wrong_pipeline (not need range/hash partition), tmp_client_sid=0 (not need mirroring for res), is_lastclone_for_pktloss should be 0 for GETRES_LATEST_SEQ_INSWITCH
                                                # size = 128 -> 2
                                                # if is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                                if (
                                                    is_hot == 0
                                                    and is_largevalueblock == 0
                                                    and tmp_client_sid == 0
                                                    and is_lastclone_for_pktloss == 0
                                                ):
                                                    matchspec0 = [
                                                        hex(GETRES_LATEST_SEQ_INSWITCH),
                                                        hex(is_cached),
                                                        hex(is_hot),
                                                        hex(validvalue),
                                                        hex(is_latest),
                                                        hex(is_largevalueblock),
                                                        hex(is_deleted),
                                                        # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        hex(tmp_client_sid),
                                                        hex(is_lastclone_for_pktloss),
                                                        hex(snapshot_flag),
                                                        hex(is_case1),
                                                    ]
                                                    if (
                                                        is_cached == 1
                                                        and validvalue == 1
                                                        and is_latest == 0
                                                        and snapshot_flag == 1
                                                        and is_case1 == 0
                                                    ):
                                                        # Update GETRES_LATEST_SEQ_INSWITCH as GETRES_LATEST_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                        if (
                                                            is_deleted == 0
                                                        ):  # is_deleted=0 -> stat=1
                                                            actnspec0 = [
                                                                hex(self.reflector_sid),
                                                                hex(1),
                                                                hex(
                                                                    reflector_dp2cpserver_port
                                                                ),
                                                            ]
                                                        elif (
                                                            is_deleted == 1
                                                        ):  # is_deleted=1 -> stat=0
                                                            actnspec0 = [
                                                                hex(self.reflector_sid),
                                                                hex(0),
                                                                hex(
                                                                    reflector_dp2cpserver_port
                                                                ),
                                                            ]
                                                        controller.table_add(
                                                            "another_eg_port_forward_tbl",
                                                            "update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss",
                                                            matchspec0,
                                                            actnspec0,
                                                        )
                                                    # Keep GETERS_LATEST_SEQ_INSWITCH unchanged, and resort to drop_tbl to drop it
                                                    # else:
                                                    #    # Drop GETRES_LATEST_SEQ_INSWITCH
                                                    #    controller.table_add('another_eg_port_forward_tbl','drop_getres_latest_seq_inswitch',matchspec0)
                                                # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_wrong_pipeline=0, tmp_client_sid=0, and snapshot_flag=1 (same inswitch_hdr as GETRES_LATEST_SEQ_INSWITCH); is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_case1 should be 0 for GETRES_LATEST_SEQ_INSWITCH_CASE1
                                                # size = 1
                                                # if is_cached == 1 and is_wrong_pipeline == 0 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_case1 == 0:
                                                if (
                                                    is_cached == 1
                                                    and snapshot_flag == 1
                                                    and is_hot == 0
                                                    and validvalue == 0
                                                    and is_latest == 0
                                                    and is_largevalueblock == 0
                                                    and is_deleted == 0
                                                    and tmp_client_sid == 0
                                                    and is_case1 == 0
                                                ):
                                                    matchspec0 = [
                                                        hex(
                                                            GETRES_LATEST_SEQ_INSWITCH_CASE1
                                                        ),
                                                        hex(is_cached),
                                                        hex(is_hot),
                                                        hex(validvalue),
                                                        hex(is_latest),
                                                        hex(is_largevalueblock),
                                                        hex(is_deleted),
                                                        # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        hex(tmp_client_sid),
                                                        hex(is_lastclone_for_pktloss),
                                                        hex(snapshot_flag),
                                                        hex(is_case1),
                                                    ]
                                                    if is_lastclone_for_pktloss == 0:
                                                        # Forward GETRES_LATEST_SEQ_INSWITCH_CASE0 (by clone_e2e) to reflector (w/ clone)
                                                        actnspec0 = [
                                                            hex(self.reflector_sid)
                                                        ]
                                                        controller.table_add(
                                                            "another_eg_port_forward_tbl",
                                                            "forward_getres_latest_seq_inswitch_case1_clone_for_pktloss",
                                                            matchspec0,
                                                            actnspec0,
                                                        )
                                                    elif is_lastclone_for_pktloss == 1:
                                                        # Forward GETRES_LATEST_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector
                                                        # controller.table_add('another_eg_port_forward_tbl','forward_getres_latest_seq_inswitch_case1',matchspec0)
                                                        # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                        pass
                                                # is_cached (no inswitch_hdr due to no field list when clone_i2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline (no inswitch_hdr), tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktloss, snapshot_flag (no inswitch_hdr), is_case1 should be 0 for GETRES_DELETED_SEQ
                                                # size = 1
                                                # if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                if (
                                                    is_cached == 0
                                                    and is_hot == 0
                                                    and validvalue == 0
                                                    and is_latest == 0
                                                    and is_largevalueblock == 0
                                                    and is_deleted == 0
                                                    and tmp_client_sid == 0
                                                    and is_lastclone_for_pktloss == 0
                                                    and snapshot_flag == 0
                                                    and is_case1 == 0
                                                ):
                                                    matchspec0 = [
                                                        hex(GETRES_DELETED_SEQ),
                                                        hex(is_cached),
                                                        hex(is_hot),
                                                        hex(validvalue),
                                                        hex(is_latest),
                                                        hex(is_largevalueblock),
                                                        hex(is_deleted),
                                                        # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        hex(tmp_client_sid),
                                                        hex(is_lastclone_for_pktloss),
                                                        hex(snapshot_flag),
                                                        hex(is_case1),
                                                    ]
                                                    # TODO: check if we need to set egress port for packet cloned by clone_i2e
                                                    # Update GETRES_DELETED_SEQ (by clone_i2e) as GETRES_SEQ to client
                                                    controller.table_add(
                                                        "another_eg_port_forward_tbl",
                                                        "update_getres_deleted_seq_to_getres_seq",
                                                        matchspec0,
                                                    )
                                                # is_hot (cm_predicate=1), is_wrong_pipeline (not need range/hash partition), tmp_client_sid=0 (not need mirroring for res), is_lastclone_for_pktloss should be 0 for GETRES_DELETED_SEQ_INSWITCH
                                                # size = 128 -> 2
                                                # if is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                                if (
                                                    is_hot == 0
                                                    and is_largevalueblock == 0
                                                    and tmp_client_sid == 0
                                                    and is_lastclone_for_pktloss == 0
                                                ):
                                                    matchspec0 = [
                                                        hex(
                                                            GETRES_DELETED_SEQ_INSWITCH
                                                        ),
                                                        hex(is_cached),
                                                        hex(is_hot),
                                                        hex(validvalue),
                                                        hex(is_latest),
                                                        hex(is_largevalueblock),
                                                        hex(is_deleted),
                                                        # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        hex(tmp_client_sid),
                                                        hex(is_lastclone_for_pktloss),
                                                        hex(snapshot_flag),
                                                        hex(is_case1),
                                                    ]
                                                    if (
                                                        is_cached == 1
                                                        and validvalue == 1
                                                        and is_latest == 0
                                                        and snapshot_flag == 1
                                                        and is_case1 == 0
                                                    ):
                                                        # Update GETRES_DELETED_SEQ_INSWITCH as GETRES_DELETED_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                        if (
                                                            is_deleted == 0
                                                        ):  # is_deleted=0 -> stat=1
                                                            actnspec0 = [
                                                                hex(self.reflector_sid),
                                                                hex(1),
                                                                hex(
                                                                    reflector_dp2cpserver_port
                                                                ),
                                                            ]
                                                        elif (
                                                            is_deleted == 1
                                                        ):  # is_deleted=1 -> stat=0
                                                            actnspec0 = [
                                                                hex(self.reflector_sid),
                                                                hex(0),
                                                                hex(
                                                                    reflector_dp2cpserver_port
                                                                ),
                                                            ]
                                                        controller.table_add(
                                                            "another_eg_port_forward_tbl",
                                                            "update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss",
                                                            matchspec0,
                                                            actnspec0,
                                                        )
                                                    # Keep GETERS_DELETED_SEQ_INSWITCH unchanged, and resort to drop_tbl to drop it
                                                    # else:
                                                    #    # Drop GETRES_DELETED_SEQ_INSWITCH
                                                    #    controller.table_add('another_eg_port_forward_tbl','drop_getres_deleted_seq_inswitch',matchspec0)
                                                # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_wrong_pipeline=0, tmp_client_sid=0, and snapshot_flag=1 (same inswitch_hdr as GETRES_LATEST_SEQ_INSWITCH); is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_case1 should be 0 for GETRES_DELETED_SEQ_INSWITCH_CASE1
                                                # size = 1
                                                # if is_cached == 1 and is_wrong_pipeline == 0 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_case1 == 0:
                                                if (
                                                    is_cached == 1
                                                    and snapshot_flag == 1
                                                    and is_hot == 0
                                                    and validvalue == 0
                                                    and is_latest == 0
                                                    and is_largevalueblock == 0
                                                    and is_deleted == 0
                                                    and tmp_client_sid == 0
                                                    and is_case1 == 0
                                                ):
                                                    matchspec0 = [
                                                        hex(
                                                            GETRES_DELETED_SEQ_INSWITCH_CASE1
                                                        ),
                                                        hex(is_cached),
                                                        hex(is_hot),
                                                        hex(validvalue),
                                                        hex(is_latest),
                                                        hex(is_largevalueblock),
                                                        hex(is_deleted),
                                                        # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        hex(tmp_client_sid),
                                                        hex(is_lastclone_for_pktloss),
                                                        hex(snapshot_flag),
                                                        hex(is_case1),
                                                    ]
                                                    if is_lastclone_for_pktloss == 0:
                                                        # Forward GETRES_DELETED_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                        actnspec0 = [
                                                            hex(self.reflector_sid)
                                                        ]
                                                        controller.table_add(
                                                            "another_eg_port_forward_tbl",
                                                            "forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss",
                                                            matchspec0,
                                                            actnspec0,
                                                        )
                                                    elif is_lastclone_for_pktloss == 1:
                                                        # Forward GETRES_DELETED_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector
                                                        # controller.table_add('another_eg_port_forward_tbl','forward_getres_deleted_seq_inswitch_case1',matchspec0)
                                                        # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                        pass
                                                # is_hot (cm_predicate=1), is_deleted, tmp_client_sid=0, is_lastclone_for_pktloss, is_case1 should be 0 for PUTREQ_LARGEVALUE_INSWITCH
                                                # NOTE: is_cached can be 0 or 1 (key may be / may not be cached for PUTREQ_LARGEVALUE_INSWITCH)
                                                # NOTE: validvalue can be 0/1/3 for PUTREQ_LARGEVALUE_INSWITCH
                                                # size = 32
                                                if (
                                                    is_hot == 0
                                                    and is_largevalueblock == 0
                                                    and is_deleted == 0
                                                    and tmp_client_sid == 0
                                                    and is_lastclone_for_pktloss == 0
                                                    and is_case1 == 0
                                                ):
                                                    matchspec0 = [
                                                        hex(PUTREQ_LARGEVALUE_INSWITCH),
                                                        hex(is_cached),
                                                        hex(is_hot),
                                                        hex(validvalue),
                                                        hex(is_latest),
                                                        hex(is_largevalueblock),
                                                        hex(is_deleted),
                                                        hex(tmp_client_sid),
                                                        hex(is_lastclone_for_pktloss),
                                                        hex(snapshot_flag),
                                                        hex(is_case1),
                                                    ]
                                                    if (
                                                        is_cached == 1
                                                        and validvalue == 3
                                                    ):
                                                        if snapshot_flag == 0:
                                                            # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED to server
                                                            controller.table_add(
                                                                "another_eg_port_forward_tbl",
                                                                "update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_beingevicted",
                                                                matchspec0,
                                                            )
                                                        elif snapshot_flag == 1:
                                                            # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED to server
                                                            controller.table_add(
                                                                "another_eg_port_forward_tbl",
                                                                "update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_case3_beingevicted",
                                                                matchspec0,
                                                            )
                                                    else:
                                                        if snapshot_flag == 0:
                                                            # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ to server
                                                            controller.table_add(
                                                                "another_eg_port_forward_tbl",
                                                                "update_putreq_largevalue_inswitch_to_putreq_largevalue_seq",
                                                                matchspec0,
                                                            )
                                                        elif snapshot_flag == 1:
                                                            # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ_CASE3 to server
                                                            controller.table_add(
                                                                "another_eg_port_forward_tbl",
                                                                "update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_case3",
                                                                matchspec0,
                                                            )

    def configure_eg_port_forward_tbl(self):
        # Table: eg_port_forward_tbl (default: NoAction	; size = 27+852*client_physical_num=27+852*2=1731 < 2048 < 27+852*8=6843 < 8192)
        tmp_client_sids = [0] + self.client_sids
        for is_cached in cached_list:
            for is_hot in hot_list:
                for validvalue in validvalue_list:
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
                                    for snapshot_flag in snapshot_flag_list:
                                        for is_case1 in case1_list:
                                            # is_cached=0 (memset inswitch_hdr by end-host, and key must not be cached in cache_lookup_tbl for CACHE_POP_INSWITCH), is_hot (cm_predicate=1), validvalue, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_POP_INSWITCH
                                            # size = 4
                                            # if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                            if (
                                                is_cached == 0
                                                and is_hot == 0
                                                and validvalue == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(CACHE_POP_INSWITCH),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                # Update CACHE_POP_INSWITCH as CACHE_POP_INSWITCH_ACK to reflector (deprecated: w/ clone)
                                                actnspec0 = [
                                                    hex(self.reflector_sid),
                                                    hex(reflector_dp2cpserver_port),
                                                ]
                                                controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone",
                                                    matchspec0,
                                                    actnspec0,
                                                )
                                            # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), snapshot_flag, is_case1, is_lastclone_for_pktloss should be 0 for CACHE_POP_INSWITCH_ACK
                                            # size = 0
                                            # if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and snapshot_flag == 0 and is_case1 == 0:
                                            if (
                                                is_cached == 0
                                                and is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and tmp_client_sid == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                                and is_lastclone_for_pktloss == 0
                                            ):
                                                matchspec0 = [
                                                    hex(CACHE_POP_INSWITCH_ACK),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                # if is_lastclone_for_pktloss == 0:
                                                #    # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector (w/ clone)
                                                #    actnspec0 = [hex(self.reflector_sid)]
                                                #    controller.table_add('eg_port_forward_tbl','forward_cache_pop_inswitch_ack_clone_for_pktloss',matchspec0, actnspec0)
                                                # elif is_lastclone_for_pktloss == 1:
                                                #    # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                                #    controller.table_add('eg_port_forward_tbl','forward_cache_pop_inswitch_ack',matchspec0)

                                                # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                                # controller.table_add('eg_port_forward_tbl','forward_cache_pop_inswitch_ack',matchspec0)
                                                # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                pass
                                            # is_lastclone_for_pktloss should be 0 for PUTREQ_INSWITCH
                                            # size = 512*client_physical_num=1024 < 512*8 = 4096
                                            if (
                                                is_lastclone_for_pktloss == 0
                                                and tmp_client_sid != 0
                                            ):
                                                matchspec0 = [
                                                    hex(PUTREQ_INSWITCH),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                if is_cached == 0:
                                                    if snapshot_flag == 1:
                                                        if is_hot == 1:
                                                            # Update PUTREQ_INSWITCH as PUTREQ_POP_SEQ_CASE3 to server
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_putreq_inswitch_to_putreq_pop_seq_case3",
                                                                matchspec0,
                                                            )
                                                        elif is_hot == 0:
                                                            # Update PUTREQ_INSWITCH as PUTREQ_SEQ_CASE3 to server
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_putreq_inswitch_to_putreq_seq_case3",
                                                                matchspec0,
                                                            )
                                                    elif snapshot_flag == 0:
                                                        if is_hot == 1:
                                                            # Update PUTREQ_INSWITCH as PUTREQ_POP_SEQ to server
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_putreq_inswitch_to_putreq_pop_seq",
                                                                matchspec0,
                                                            )
                                                        elif is_hot == 0:
                                                            # Update PUTREQ_INSWITCH as PUTREQ_SEQ to server
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_putreq_inswitch_to_putreq_seq",
                                                                matchspec0,
                                                            )
                                                elif is_cached == 1:
                                                    if validvalue == 0:
                                                        if snapshot_flag == 1:
                                                            # Update PUTREQ_INSWITCH as PUTREQ_SEQ_CASE3 to server
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_putreq_inswitch_to_putreq_seq_case3",
                                                                matchspec0,
                                                            )
                                                        elif snapshot_flag == 0:
                                                            # Update PUTREQ_INSWITCH as PUTREQ_SEQ to server
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_putreq_inswitch_to_putreq_seq",
                                                                matchspec0,
                                                            )
                                                    elif validvalue == 3:
                                                        if snapshot_flag == 1:
                                                            # Update PUTREQ_INSWITCH as PUTREQ_SEQ_CASE3_BEINGEVICTED to server
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_putreq_inswitch_to_putreq_seq_case3_beingevicted",
                                                                matchspec0,
                                                            )
                                                        elif snapshot_flag == 0:
                                                            # Update PUTREQ_INSWITCH as PUTREQ_SEQ_BEINGEVICTED to server
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_putreq_inswitch_to_putreq_seq_beingevicted",
                                                                matchspec0,
                                                            )
                                                    elif validvalue == 1:
                                                        if (
                                                            snapshot_flag == 1
                                                            and is_case1 == 0
                                                        ):
                                                            # Update PUTREQ_INSWITCH as PUTREQ_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                            if (
                                                                is_deleted == 0
                                                            ):  # is_deleted=0 -> stat=1
                                                                actnspec0 = [
                                                                    hex(
                                                                        self.reflector_sid
                                                                    ),
                                                                    hex(1),
                                                                    hex(
                                                                        reflector_dp2cpserver_port
                                                                    ),
                                                                ]
                                                            elif (
                                                                is_deleted == 1
                                                            ):  # is_deleted=1 -> stat=0
                                                                actnspec0 = [
                                                                    hex(
                                                                        self.reflector_sid
                                                                    ),
                                                                    hex(0),
                                                                    hex(
                                                                        reflector_dp2cpserver_port
                                                                    ),
                                                                ]
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres",
                                                                matchspec0,
                                                                actnspec0,
                                                            )
                                                        else:
                                                            # Update PUTREQ_INSWITCH as PUTRES_SEQ to client by mirroring
                                                            actnspec0 = [
                                                                hex(tmp_client_sid),
                                                                hex(
                                                                    server_worker_port_start
                                                                ),
                                                            ]
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_putreq_inswitch_to_putres_seq_by_mirroring",
                                                                matchspec0,
                                                                actnspec0,
                                                            )
                                            # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, snapshot_flag=1, is_case1 should be 0 for PUTREQ_SEQ_INSWITCH_CASE1
                                            # size = 4*client_physical_num=8 < 4*8=32
                                            if (
                                                is_cached == 1
                                                and is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and snapshot_flag == 1
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(PUTREQ_SEQ_INSWITCH_CASE1),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                if is_lastclone_for_pktloss == 0:
                                                    # Forward PUTREQ_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                    actnspec0 = [
                                                        hex(self.reflector_sid)
                                                    ]
                                                    controller.table_add(
                                                        "eg_port_forward_tbl",
                                                        "forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres",
                                                        matchspec0,
                                                        actnspec0,
                                                    )
                                                elif is_lastclone_for_pktloss == 1:
                                                    # Update PUTREQ_SEQ_INSWITCH_CASE1 as PUTRES_SEQ to client by mirroring
                                                    actnspec0 = [
                                                        hex(tmp_client_sid),
                                                        hex(server_worker_port_start),
                                                    ]
                                                    controller.table_add(
                                                        "eg_port_forward_tbl",
                                                        "update_putreq_seq_inswitch_case1_to_putres_seq_by_mirroring",
                                                        matchspec0,
                                                        actnspec0,
                                                    )
                                            # is_hot (cm_predicate=1), is_lastclone_for_pktloss should be 0 for DELREQ_INSWITCH
                                            # size = 256*client_physical_num=512 < 256*8=2048
                                            if (
                                                is_hot == 0
                                                and is_lastclone_for_pktloss == 0
                                                and tmp_client_sid != 0
                                            ):
                                                matchspec0 = [
                                                    hex(DELREQ_INSWITCH),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                if is_cached == 0:
                                                    if snapshot_flag == 1:
                                                        # Update DELREQ_INSWITCH as DELREQ_SEQ_CASE3 to server
                                                        controller.table_add(
                                                            "eg_port_forward_tbl",
                                                            "update_delreq_inswitch_to_delreq_seq_case3",
                                                            matchspec0,
                                                        )
                                                    elif snapshot_flag == 0:
                                                        # Update DELREQ_INSWITCH as DELREQ_SEQ to server
                                                        controller.table_add(
                                                            "eg_port_forward_tbl",
                                                            "update_delreq_inswitch_to_delreq_seq",
                                                            matchspec0,
                                                        )
                                                elif is_cached == 1:
                                                    if validvalue == 0:
                                                        if snapshot_flag == 1:
                                                            # Update DELREQ_INSWITCH as DELREQ_SEQ_CASE3 to server
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_delreq_inswitch_to_delreq_seq_case3",
                                                                matchspec0,
                                                            )
                                                        elif snapshot_flag == 0:
                                                            # Update DELREQ_INSWITCH as DELREQ_SEQ to server
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_delreq_inswitch_to_delreq_seq",
                                                                matchspec0,
                                                            )
                                                    elif validvalue == 3:
                                                        if snapshot_flag == 1:
                                                            # Update DELREQ_INSWITCH as DELREQ_SEQ_CASE3_BEINGEVICTED to server
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_delreq_inswitch_to_delreq_seq_case3_beingevicted",
                                                                matchspec0,
                                                            )
                                                        elif snapshot_flag == 0:
                                                            # Update DELREQ_INSWITCH as DELREQ_SEQ_BEINGEVICTED to server
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_delreq_inswitch_to_delreq_seq_beingevicted",
                                                                matchspec0,
                                                            )
                                                    elif validvalue == 1:
                                                        if (
                                                            snapshot_flag == 1
                                                            and is_case1 == 0
                                                        ):
                                                            # Update DELREQ_INSWITCH as DELREQ_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                            if (
                                                                is_deleted == 0
                                                            ):  # is_deleted=0 -> stat=1
                                                                actnspec0 = [
                                                                    hex(
                                                                        self.reflector_sid
                                                                    ),
                                                                    hex(1),
                                                                    hex(
                                                                        reflector_dp2cpserver_port
                                                                    ),
                                                                ]
                                                            elif (
                                                                is_deleted == 1
                                                            ):  # is_deleted=1 -> stat=0
                                                                actnspec0 = [
                                                                    hex(
                                                                        self.reflector_sid
                                                                    ),
                                                                    hex(0),
                                                                    hex(
                                                                        reflector_dp2cpserver_port
                                                                    ),
                                                                ]
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres",
                                                                matchspec0,
                                                                actnspec0,
                                                            )
                                                        else:
                                                            # Update DELREQ_INSWITCH as DELRES_SEQ to client by mirroring
                                                            actnspec0 = [
                                                                hex(tmp_client_sid),
                                                                hex(
                                                                    server_worker_port_start
                                                                ),
                                                            ]
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_delreq_inswitch_to_delres_seq_by_mirroring",
                                                                matchspec0,
                                                                actnspec0,
                                                            )
                                            # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, snapshot_flag=1, is_case1 should be 0 for DELREQ_SEQ_INSWITCH_CASE1
                                            # size = 16*client_physical_num=32 < 16*8=128
                                            if (
                                                is_cached == 1
                                                and is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and snapshot_flag == 1
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(DELREQ_SEQ_INSWITCH_CASE1),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                if is_lastclone_for_pktloss == 0:
                                                    # Forward DELREQ_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                    actnspec0 = [
                                                        hex(self.reflector_sid)
                                                    ]
                                                    controller.table_add(
                                                        "eg_port_forward_tbl",
                                                        "forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres",
                                                        matchspec0,
                                                        actnspec0,
                                                    )
                                                elif is_lastclone_for_pktloss == 1:
                                                    # Update DELREQ_SEQ_INSWITCHCASE1 as DELRES_SEQ to client by mirroring
                                                    actnspec0 = [
                                                        hex(tmp_client_sid),
                                                        hex(server_worker_port_start),
                                                    ]
                                                    controller.table_add(
                                                        "eg_port_forward_tbl",
                                                        "update_delreq_seq_inswitch_case1_to_delres_seq_by_mirroring",
                                                        matchspec0,
                                                        actnspec0,
                                                    )
                                            # is_cached=1 (key must be cached in cache_lookup_tbl for CACHE_EVICT_LOADFREQ_INSWITCH), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH
                                            # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                            # size = 1
                                            if (
                                                is_cached == 1
                                                and is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(CACHE_EVICT_LOADFREQ_INSWITCH),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                # Update CACHE_EVICT_LOADFREQ_INSWITCH as CACHE_EVICT_LOADFREQ_INSWITCH_ACK to reflector (w/ frequency)
                                                actnspec0 = [
                                                    hex(self.reflector_sid),
                                                    hex(reflector_dp2cpserver_port),
                                                ]
                                                controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone",
                                                    matchspec0,
                                                    actnspec0,
                                                )
                                            # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH_ACK
                                            # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                            # size = 0
                                            if (
                                                is_cached == 0
                                                and is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(
                                                        CACHE_EVICT_LOADFREQ_INSWITCH_ACK
                                                    ),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                # Forward CACHE_EVICT_LOADFREQ_INSWITCH_ACK (by clone_e2e) to reflector
                                                # controller.table_add('eg_port_forward_tbl','forward_cache_evict_loadfreq_inswitch_ack',matchspec0)
                                                # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                pass
                                            # is_cached=1 (key must be cached in cache_lookup_tbl for CACHE_EVICT_LOADDATA_INSWITCH), is_hot (cm_predicate=1), validvalue, is_latest, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADDATA_INSWITCH
                                            # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                            # size = 2
                                            if (
                                                is_cached == 1
                                                and is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(CACHE_EVICT_LOADDATA_INSWITCH),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                # Update CACHE_EVICT_LOADDATA_INSWITCH as CACHE_EVICT_LOADDATA_INSWITCH_ACK to reflector
                                                actnspec0 = [
                                                    hex(self.reflector_sid),
                                                    hex(reflector_dp2cpserver_port),
                                                    hex(tmpstat),
                                                ]
                                                controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone",
                                                    matchspec0,
                                                    actnspec0,
                                                )
                                            # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADDATA_INSWITCH_ACK
                                            # size = 0
                                            if (
                                                is_cached == 0
                                                and is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(
                                                        CACHE_EVICT_LOADDATA_INSWITCH_ACK
                                                    ),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                # Forward CACHE_EVICT_LOADDATA_INSWITCH_ACK (by clone_e2e) to reflector
                                                # controller.table_add('eg_port_forward_tbl','forward_cache_evict_loaddata_inswitch_ack',matchspec0)
                                                # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                pass
                                            # is_hot (cm_predicate=1), validvalue, is_latest, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for LOADSNAPSHOTDATA_INSWITCH
                                            # NOTE: is_cached can be 0 or 1 (key may be / may not be evicted after snapshot timepoint)
                                            # size = 4
                                            if (
                                                is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(LOADSNAPSHOTDATA_INSWITCH),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                # Update LOADSNAPSHOTDATA_INSWITCH as LOADSNAPSHOTDATA_INSWITCH_ACK to reflector
                                                actnspec0 = [
                                                    hex(self.reflector_sid),
                                                    hex(reflector_dp2cpserver_port),
                                                    hex(tmpstat),
                                                ]
                                                controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone",
                                                    matchspec0,
                                                    actnspec0,
                                                )
                                            # is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for LOADSNAPSHOTDATA_INSWITCH_ACK
                                            # NOTE: is_cached can be 0 or 1 (inswitch_hdr inherited from LOADSNAPSHOTDATA_INSWITCH)
                                            # size = 0
                                            if (
                                                is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(LOADSNAPSHOTDATA_INSWITCH_ACK),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                # Forward LOADSNAPSHOTDATA_INSWITCH_ACK (by clone_e2e) to reflector
                                                # controller.table_add('eg_port_forward_tbl','forward_loadsnapshotdata_inswitch_ack',matchspec0)
                                                # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                pass
                                            # is_hot (cm_predicate=1), is_latest, is_deleted, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for SETVALID_INSWITCH
                                            # NOTE: is_cached can be 0 or 1 (key may be / may not be cached for SETVALID_INSWITCH)
                                            # NOTE: validvalue can be 0/1/3 for SETVALID_INSWITCH
                                            # size = 8
                                            if (
                                                is_hot == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(SETVALID_INSWITCH),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                # Update SETVALID_INSWITCH as SETVALID_INSWITCH_ACK to reflector
                                                actnspec0 = [
                                                    hex(self.reflector_sid),
                                                    hex(reflector_dp2cpserver_port),
                                                ]
                                                controller.table_add(
                                                    "eg_port_forward_tbl",
                                                    "update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone",
                                                    matchspec0,
                                                    actnspec0,
                                                )
                                            # is_cached=0 (no inswtich_hdr), is_hot (cm_predicate=1), validvalue (no validvalue_hdr), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for SETVALID_INSWITCH_ACK
                                            # NOTE: is_cached must be 0 (SETVALID_INSWITCH_ACK does not have inswitch_hdr)
                                            # NOTE: validvalue must be 0 (no validvalue_hdr and not touch validvalue_reg)
                                            # size = 0
                                            if (
                                                is_cached == 0
                                                and is_hot == 0
                                                and validvalue == 0
                                                and is_latest == 0
                                                and is_deleted == 0
                                                and tmp_client_sid == 0
                                                and is_lastclone_for_pktloss == 0
                                                and snapshot_flag == 0
                                                and is_case1 == 0
                                            ):
                                                matchspec0 = [
                                                    hex(SETVALID_INSWITCH_ACK),
                                                    hex(is_cached),
                                                    hex(is_hot),
                                                    hex(validvalue),
                                                    hex(is_latest),
                                                    hex(is_deleted),
                                                    # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    hex(tmp_client_sid),
                                                    hex(is_lastclone_for_pktloss),
                                                    hex(snapshot_flag),
                                                    hex(is_case1),
                                                ]
                                                # Forward SETVALID_INSWITCH_ACK (by clone_e2e) to reflector
                                                # controller.table_add('eg_port_forward_tbl','forward_setvalid_inswitch_ack',matchspec0)
                                                # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                pass

    def configure_eg_port_forward_tbl_with_range(self):
        # Table: eg_port_forward_tbl (default: NoAction	; size = 27+852*client_physical_num+2*server_physical_num=27+854*2=1735 < 2048 < 21+854*8=6859 < 8192)
        tmp_client_sids = [0] + self.client_sids
        tmp_server_sids = [0] + self.server_sids
        for is_cached in cached_list:
            for is_hot in hot_list:
                for validvalue in validvalue_list:
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
                                    for snapshot_flag in snapshot_flag_list:
                                        for is_case1 in case1_list:
                                            for is_last_scansplit in [0, 1]:
                                                for tmp_server_sid in tmp_server_sids:
                                                    # is_cached=0 (memset inswitch_hdr by end-host, and key must not be cached in cache_lookup_tbl for CACHE_POP_INSWITCH), is_hot (cm_predicate=1), validvalue, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_POP_INSWITCH
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size = 4
                                                    # if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                    if (
                                                        is_cached == 0
                                                        and is_hot == 0
                                                        and validvalue == 0
                                                        and tmp_client_sid == 0
                                                        and is_lastclone_for_pktloss
                                                        == 0
                                                        and snapshot_flag == 0
                                                        and is_case1 == 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(CACHE_POP_INSWITCH),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        # Update CACHE_POP_INSWITCH as CACHE_POP_INSWITCH_ACK to reflector (deprecated: w/ clone)
                                                        actnspec0 = [
                                                            hex(self.reflector_sid),
                                                            hex(
                                                                reflector_dp2cpserver_port
                                                            ),
                                                        ]
                                                        controller.table_add(
                                                            "eg_port_forward_tbl",
                                                            "update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone",
                                                            matchspec0,
                                                            actnspec0,
                                                        )
                                                    # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), snapshot_flag, is_case1, is_lastclone_for_pktloss should be 0 for CACHE_POP_INSWITCH_ACK
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size = 0
                                                    # if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                    if (
                                                        is_cached == 0
                                                        and is_hot == 0
                                                        and validvalue == 0
                                                        and is_latest == 0
                                                        and is_deleted == 0
                                                        and tmp_client_sid == 0
                                                        and snapshot_flag == 0
                                                        and is_case1 == 0
                                                        and is_lastclone_for_pktloss
                                                        == 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(CACHE_POP_INSWITCH_ACK),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        # if is_lastclone_for_pktloss == 0:
                                                        #    # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector (w/ clone)
                                                        #    actnspec0 = [hex(self.reflector_sid)]
                                                        #    controller.table_add('eg_port_forward_tbl','forward_cache_pop_inswitch_ack_clone_for_pktloss',matchspec0, actnspec0)
                                                        # elif is_lastclone_for_pktloss == 1:
                                                        #    # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                                        #    controller.table_add('eg_port_forward_tbl','forward_cache_pop_inswitch_ack',matchspec0)

                                                        # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                                        # controller.table_add('eg_port_forward_tbl','forward_cache_pop_inswitch_ack',matchspec0)
                                                        # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                        pass
                                                    # is_lastclone_for_pktloss should be 0 for PUTREQ_INSWITCH
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size = 512*client_physical_num=1024 < 512*8 = 4096
                                                    if (
                                                        is_lastclone_for_pktloss == 0
                                                        and tmp_client_sid != 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(PUTREQ_INSWITCH),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        if is_cached == 0:
                                                            if snapshot_flag == 1:
                                                                if is_hot == 1:
                                                                    # Update PUTREQ_INSWITCH as PUTREQ_POP_SEQ_CASE3 to server
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_putreq_inswitch_to_putreq_pop_seq_case3",
                                                                        matchspec0,
                                                                    )
                                                                elif is_hot == 0:
                                                                    # Update PUTREQ_INSWITCH as PUTREQ_SEQ_CASE3 to server
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_putreq_inswitch_to_putreq_seq_case3",
                                                                        matchspec0,
                                                                    )
                                                            elif snapshot_flag == 0:
                                                                if is_hot == 1:
                                                                    # Update PUTREQ_INSWITCH as PUTREQ_POP_SEQ to server
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_putreq_inswitch_to_putreq_pop_seq",
                                                                        matchspec0,
                                                                    )
                                                                elif is_hot == 0:
                                                                    # Update PUTREQ_INSWITCH as PUTREQ_SEQ to server
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_putreq_inswitch_to_putreq_seq",
                                                                        matchspec0,
                                                                    )
                                                        elif is_cached == 1:
                                                            if validvalue == 0:
                                                                if snapshot_flag == 1:
                                                                    # Update PUTREQ_INSWITCH as PUTREQ_SEQ_CASE3 to server
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_putreq_inswitch_to_putreq_seq_case3",
                                                                        matchspec0,
                                                                    )
                                                                elif snapshot_flag == 0:
                                                                    # Update PUTREQ_INSWITCH as PUTREQ_SEQ to server
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_putreq_inswitch_to_putreq_seq",
                                                                        matchspec0,
                                                                    )
                                                            elif validvalue == 3:
                                                                if snapshot_flag == 1:
                                                                    # Update PUTREQ_INSWITCH as PUTREQ_SEQ_CASE3_BEINGEVICTED to server
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_putreq_inswitch_to_putreq_seq_case3_beingevicted",
                                                                        matchspec0,
                                                                    )
                                                                elif snapshot_flag == 0:
                                                                    # Update PUTREQ_INSWITCH as PUTREQ_SEQ_BEINGEVICTED to server
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_putreq_inswitch_to_putreq_seq_beingevicted",
                                                                        matchspec0,
                                                                    )
                                                            elif validvalue == 1:
                                                                if (
                                                                    snapshot_flag == 1
                                                                    and is_case1 == 0
                                                                ):
                                                                    # Update PUTREQ_INSWITCH as PUTREQ_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                                    if (
                                                                        is_deleted == 0
                                                                    ):  # is_deleted=0 -> stat=1
                                                                        actnspec0 = [
                                                                            hex(
                                                                                self.reflector_sid
                                                                            ),
                                                                            hex(1),
                                                                            hex(
                                                                                reflector_dp2cpserver_port
                                                                            ),
                                                                        ]
                                                                    elif (
                                                                        is_deleted == 1
                                                                    ):  # is_deleted=1 -> stat=0
                                                                        actnspec0 = [
                                                                            hex(
                                                                                self.reflector_sid
                                                                            ),
                                                                            hex(0),
                                                                            hex(
                                                                                reflector_dp2cpserver_port
                                                                            ),
                                                                        ]
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres",
                                                                        matchspec0,
                                                                        actnspec0,
                                                                    )
                                                                else:
                                                                    # Update PUTREQ_INSWITCH as PUTRES_SEQ to client by mirroring
                                                                    actnspec0 = [
                                                                        hex(
                                                                            tmp_client_sid
                                                                        ),
                                                                        hex(
                                                                            server_worker_port_start
                                                                        ),
                                                                    ]
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_putreq_inswitch_to_putres_seq_by_mirroring",
                                                                        matchspec0,
                                                                        actnspec0,
                                                                    )
                                                    # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, snapshot_flag=1, is_case1 should be 0 for PUTREQ_SEQ_INSWITCH_CASE1
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size = 4*client_physical_num=8 < 4*8=32
                                                    if (
                                                        is_cached == 1
                                                        and is_hot == 0
                                                        and validvalue == 0
                                                        and is_latest == 0
                                                        and is_deleted == 0
                                                        and snapshot_flag == 1
                                                        and is_case1 == 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(
                                                                PUTREQ_SEQ_INSWITCH_CASE1
                                                            ),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        if (
                                                            is_lastclone_for_pktloss
                                                            == 0
                                                        ):
                                                            # Forward PUTREQ_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                            actnspec0 = [
                                                                hex(self.reflector_sid)
                                                            ]
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres",
                                                                matchspec0,
                                                                actnspec0,
                                                            )
                                                        elif (
                                                            is_lastclone_for_pktloss
                                                            == 1
                                                        ):
                                                            # Update PUTREQ_SEQ_INSWITCH_CASE1 as PUTRES_SEQ to client by mirroring
                                                            actnspec0 = [
                                                                hex(tmp_client_sid),
                                                                hex(
                                                                    server_worker_port_start
                                                                ),
                                                            ]
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_putreq_seq_inswitch_case1_to_putres_seq_by_mirroring",
                                                                matchspec0,
                                                                actnspec0,
                                                            )
                                                    # is_hot (cm_predicate=1), is_lastclone_for_pktloss should be 0 for DELREQ_INSWITCH
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size = 256*client_physical_num=512 < 256*8=2048
                                                    if (
                                                        is_hot == 0
                                                        and is_lastclone_for_pktloss
                                                        == 0
                                                        and tmp_client_sid != 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(DELREQ_INSWITCH),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        if is_cached == 0:
                                                            if snapshot_flag == 1:
                                                                # Update DELREQ_INSWITCH as DELREQ_SEQ_CASE3 to server
                                                                controller.table_add(
                                                                    "eg_port_forward_tbl",
                                                                    "update_delreq_inswitch_to_delreq_seq_case3",
                                                                    matchspec0,
                                                                )
                                                            elif snapshot_flag == 0:
                                                                # Update DELREQ_INSWITCH as DELREQ_SEQ to server
                                                                controller.table_add(
                                                                    "eg_port_forward_tbl",
                                                                    "update_delreq_inswitch_to_delreq_seq",
                                                                    matchspec0,
                                                                )
                                                        elif is_cached == 1:
                                                            if validvalue == 0:
                                                                if snapshot_flag == 1:
                                                                    # Update DELREQ_INSWITCH as DELREQ_SEQ_CASE3 to server
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_delreq_inswitch_to_delreq_seq_case3",
                                                                        matchspec0,
                                                                    )
                                                                elif snapshot_flag == 0:
                                                                    # Update DELREQ_INSWITCH as DELREQ_SEQ to server
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_delreq_inswitch_to_delreq_seq",
                                                                        matchspec0,
                                                                    )
                                                            elif validvalue == 3:
                                                                if snapshot_flag == 1:
                                                                    # Update DELREQ_INSWITCH as DELREQ_SEQ_CASE3_BEINGEVICTED to server
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_delreq_inswitch_to_delreq_seq_case3_beingevicted",
                                                                        matchspec0,
                                                                    )
                                                                elif snapshot_flag == 0:
                                                                    # Update DELREQ_INSWITCH as DELREQ_SEQ_BEINGEVICTED to server
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_delreq_inswitch_to_delreq_seq_beingevicted",
                                                                        matchspec0,
                                                                    )
                                                            elif validvalue == 1:
                                                                if (
                                                                    snapshot_flag == 1
                                                                    and is_case1 == 0
                                                                ):
                                                                    # Update DELREQ_INSWITCH as DELREQ_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                                    if (
                                                                        is_deleted == 0
                                                                    ):  # is_deleted=0 -> stat=1
                                                                        actnspec0 = [
                                                                            hex(
                                                                                self.reflector_sid
                                                                            ),
                                                                            hex(1),
                                                                            hex(
                                                                                reflector_dp2cpserver_port
                                                                            ),
                                                                        ]
                                                                    elif (
                                                                        is_deleted == 1
                                                                    ):  # is_deleted=1 -> stat=0
                                                                        actnspec0 = [
                                                                            hex(
                                                                                self.reflector_sid
                                                                            ),
                                                                            hex(0),
                                                                            hex(
                                                                                reflector_dp2cpserver_port
                                                                            ),
                                                                        ]
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres",
                                                                        matchspec0,
                                                                        actnspec0,
                                                                    )
                                                                else:
                                                                    # Update DELREQ_INSWITCH as DELRES_SEQ to client by mirroring
                                                                    actnspec0 = [
                                                                        hex(
                                                                            tmp_client_sid
                                                                        ),
                                                                        hex(
                                                                            server_worker_port_start
                                                                        ),
                                                                    ]
                                                                    controller.table_add(
                                                                        "eg_port_forward_tbl",
                                                                        "update_delreq_inswitch_to_delres_seq_by_mirroring",
                                                                        matchspec0,
                                                                        actnspec0,
                                                                    )
                                                    # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, snapshot_flag=1, is_case1 should be 0 for DELREQ_SEQ_INSWITCH_CASE1
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size = 16*client_physical_num=32 < 16*8=128
                                                    if (
                                                        is_cached == 1
                                                        and is_hot == 0
                                                        and validvalue == 0
                                                        and is_latest == 0
                                                        and is_deleted == 0
                                                        and snapshot_flag == 1
                                                        and is_case1 == 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(
                                                                DELREQ_SEQ_INSWITCH_CASE1
                                                            ),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        if (
                                                            is_lastclone_for_pktloss
                                                            == 0
                                                        ):
                                                            # Forward DELREQ_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                            actnspec0 = [
                                                                hex(self.reflector_sid)
                                                            ]
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres",
                                                                matchspec0,
                                                                actnspec0,
                                                            )
                                                        elif (
                                                            is_lastclone_for_pktloss
                                                            == 1
                                                        ):
                                                            # Update DELREQ_SEQ_INSWITCHCASE1 as DELRES_SEQ to client by mirroring
                                                            actnspec0 = [
                                                                hex(tmp_client_sid),
                                                                hex(
                                                                    server_worker_port_start
                                                                ),
                                                            ]
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "update_delreq_seq_inswitch_case1_to_delres_seq_by_mirroring",
                                                                matchspec0,
                                                                actnspec0,
                                                            )
                                                    # is_cached=0 (no inswitch_hdr after entering egress pipeline), is_hot, validvalue,  is_latest, is_deleted, client_sid, is_lastclone_for_pktloss, snapshot_flag, is_case1 must be 0 for SCANREQ_SPLIT
                                                    # size = 2*server_physical_num=4 < 2*8=16
                                                    if (
                                                        is_cached == 0
                                                        and is_hot == 0
                                                        and validvalue == 0
                                                        and is_latest == 0
                                                        and is_deleted == 0
                                                        and tmp_client_sid == 0
                                                        and is_lastclone_for_pktloss
                                                        == 0
                                                        and snapshot_flag == 0
                                                        and is_case1 == 0
                                                        and tmp_server_sid != 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(SCANREQ_SPLIT),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        if is_last_scansplit == 1:
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "forward_scanreq_split",
                                                                matchspec0,
                                                            )
                                                        elif is_last_scansplit == 0:
                                                            actnspec0 = [
                                                                hex(tmp_server_sid)
                                                            ]
                                                            controller.table_add(
                                                                "eg_port_forward_tbl",
                                                                "forward_scanreq_split_and_clone",
                                                                matchspec0,
                                                                actnspec0,
                                                            )
                                                    # is_cached=1 (key must be cached in cache_lookup_tbl for CACHE_EVICT_LOADFREQ_INSWITCH), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH
                                                    # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                                    # size = 1
                                                    if (
                                                        is_cached == 1
                                                        and is_hot == 0
                                                        and validvalue == 0
                                                        and is_latest == 0
                                                        and is_deleted == 0
                                                        and tmp_client_sid == 0
                                                        and is_lastclone_for_pktloss
                                                        == 0
                                                        and snapshot_flag == 0
                                                        and is_case1 == 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(
                                                                CACHE_EVICT_LOADFREQ_INSWITCH
                                                            ),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        # Update CACHE_EVICT_LOADFREQ_INSWITCH as CACHE_EVICT_LOADFREQ_INSWITCH_ACK to reflector (w/ clone)
                                                        actnspec0 = [
                                                            hex(self.reflector_sid),
                                                            hex(
                                                                reflector_dp2cpserver_port
                                                            ),
                                                        ]
                                                        controller.table_add(
                                                            "eg_port_forward_tbl",
                                                            "update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone",
                                                            matchspec0,
                                                            actnspec0,
                                                        )
                                                    # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH_ACK
                                                    # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                                    # size = 0
                                                    if (
                                                        is_cached == 0
                                                        and is_hot == 0
                                                        and validvalue == 0
                                                        and is_latest == 0
                                                        and is_deleted == 0
                                                        and tmp_client_sid == 0
                                                        and is_lastclone_for_pktloss
                                                        == 0
                                                        and snapshot_flag == 0
                                                        and is_case1 == 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(
                                                                CACHE_EVICT_LOADFREQ_INSWITCH_ACK
                                                            ),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        # Forward CACHE_EVICT_LOADFREQ_INSWITCH_ACK (by clone_e2e) to reflector
                                                        # controller.table_add('eg_port_forward_tbl','forward_cache_evict_loadfreq_inswitch_ack',matchspec0)
                                                        # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                        pass
                                                    # is_cached=1 (key must be cached in cache_lookup_tbl for CACHE_EVICT_LOADDATA_INSWITCH), is_hot (cm_predicate=1), validvalue, is_latest, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADDATA_INSWITCH
                                                    # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                                    # size = 2
                                                    if (
                                                        is_cached == 1
                                                        and is_hot == 0
                                                        and validvalue == 0
                                                        and is_latest == 0
                                                        and tmp_client_sid == 0
                                                        and is_lastclone_for_pktloss
                                                        == 0
                                                        and snapshot_flag == 0
                                                        and is_case1 == 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(
                                                                CACHE_EVICT_LOADDATA_INSWITCH
                                                            ),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        # Update CACHE_EVICT_LOADDATA_INSWITCH as CACHE_EVICT_LOADDATA_INSWITCH_ACK to reflector
                                                        actnspec0 = [
                                                            hex(self.reflector_sid),
                                                            hex(
                                                                reflector_dp2cpserver_port
                                                            ),
                                                            hex(tmpstat),
                                                        ]
                                                        controller.table_add(
                                                            "eg_port_forward_tbl",
                                                            "update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone",
                                                            matchspec0,
                                                            actnspec0,
                                                        )
                                                    # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADDATA_INSWITCH_ACK
                                                    # size = 0
                                                    if (
                                                        is_cached == 0
                                                        and is_hot == 0
                                                        and validvalue == 0
                                                        and is_latest == 0
                                                        and is_deleted == 0
                                                        and tmp_client_sid == 0
                                                        and is_lastclone_for_pktloss
                                                        == 0
                                                        and snapshot_flag == 0
                                                        and is_case1 == 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(
                                                                CACHE_EVICT_LOADDATA_INSWITCH_ACK
                                                            ),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        # Forward CACHE_EVICT_LOADDATA_INSWITCH_ACK (by clone_e2e) to reflector
                                                        # controller.table_add('eg_port_forward_tbl','forward_cache_evict_loaddata_inswitch_ack',matchspec0)
                                                        # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                        pass
                                                    # is_hot (cm_predicate=1), validvalue, is_latest, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for LOADSNAPSHOTDATA_INSWITCH
                                                    # NOTE: is_cached can be 0 or 1 (key may be / may not be evicted after snapshot timepoint)
                                                    # size = 4
                                                    if (
                                                        is_hot == 0
                                                        and validvalue == 0
                                                        and is_latest == 0
                                                        and tmp_client_sid == 0
                                                        and is_lastclone_for_pktloss
                                                        == 0
                                                        and snapshot_flag == 0
                                                        and is_case1 == 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(
                                                                LOADSNAPSHOTDATA_INSWITCH
                                                            ),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        # Update LOADSNAPSHOTDATA_INSWITCH as LOADSNAPSHOTDATA_INSWITCH_ACK to reflector
                                                        actnspec0 = [
                                                            hex(self.reflector_sid),
                                                            hex(
                                                                reflector_dp2cpserver_port
                                                            ),
                                                            hex(tmpstat),
                                                        ]
                                                        controller.table_add(
                                                            "eg_port_forward_tbl",
                                                            "update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone",
                                                            matchspec0,
                                                            actnspec0,
                                                        )
                                                    # is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for LOADSNAPSHOTDATA_INSWITCH_ACK
                                                    # NOTE: is_cached can be 0 or 1 (inswitch_hdr inherited from LOADSNAPSHOTDATA_INSWITCH)
                                                    # size = 0
                                                    if (
                                                        is_hot == 0
                                                        and validvalue == 0
                                                        and is_latest == 0
                                                        and is_deleted == 0
                                                        and tmp_client_sid == 0
                                                        and is_lastclone_for_pktloss
                                                        == 0
                                                        and snapshot_flag == 0
                                                        and is_case1 == 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(
                                                                LOADSNAPSHOTDATA_INSWITCH_ACK
                                                            ),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        # Forward LOADSNAPSHOTDATA_INSWITCH_ACK (by clone_e2e) to reflector
                                                        # controller.table_add('eg_port_forward_tbl','forward_loadsnapshotdata_inswitch_ack',matchspec0)
                                                        # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                        pass
                                                    # is_hot (cm_predicate=1), is_latest, is_deleted, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for SETVALID_INSWITCH
                                                    # NOTE: is_cached can be 0 or 1 (key may be / may not be cached for SETVALID_INSWITCH)
                                                    # NOTE: validvalue can be 0/1/3 for SETVALID_INSWITCH
                                                    # size = 8
                                                    if (
                                                        is_hot == 0
                                                        and is_latest == 0
                                                        and is_deleted == 0
                                                        and tmp_client_sid == 0
                                                        and is_lastclone_for_pktloss
                                                        == 0
                                                        and snapshot_flag == 0
                                                        and is_case1 == 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(SETVALID_INSWITCH),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        # Update SETVALID_INSWITCH as SETVALID_INSWITCH_ACK to reflector
                                                        actnspec0 = [
                                                            hex(self.reflector_sid),
                                                            hex(
                                                                reflector_dp2cpserver_port
                                                            ),
                                                        ]
                                                        controller.table_add(
                                                            "eg_port_forward_tbl",
                                                            "update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone",
                                                            matchspec0,
                                                            actnspec0,
                                                        )
                                                    # is_cached=0 (no inswtich_hdr), is_hot (cm_predicate=1), validvalue (no validvalue_hdr), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for SETVALID_INSWITCH_ACK
                                                    # NOTE: is_cached must be 0 (SETVALID_INSWITCH_ACK does not have inswitch_hdr)
                                                    # NOTE: validvalue must be 0 (no validvalue_hdr and not touch validvalue_reg)
                                                    # size = 0
                                                    if (
                                                        is_cached == 0
                                                        and is_hot == 0
                                                        and validvalue == 0
                                                        and is_latest == 0
                                                        and is_deleted == 0
                                                        and tmp_client_sid == 0
                                                        and is_lastclone_for_pktloss
                                                        == 0
                                                        and snapshot_flag == 0
                                                        and is_case1 == 0
                                                        and is_last_scansplit == 0
                                                        and tmp_server_sid == 0
                                                    ):
                                                        matchspec0 = [
                                                            hex(SETVALID_INSWITCH_ACK),
                                                            hex(is_cached),
                                                            hex(is_hot),
                                                            hex(validvalue),
                                                            hex(is_latest),
                                                            hex(is_deleted),
                                                            # inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            hex(tmp_client_sid),
                                                            hex(
                                                                is_lastclone_for_pktloss
                                                            ),
                                                            hex(snapshot_flag),
                                                            hex(is_case1),
                                                            hex(is_last_scansplit),
                                                            hex(tmp_server_sid),
                                                        ]
                                                        # Forward SETVALID_INSWITCH_ACK (by clone_e2e) to reflector
                                                        # controller.table_add('eg_port_forward_tbl','forward_setvalid_inswitch_ack',matchspec0)
                                                        # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                                                        pass


tableconfig = TableConfigure()
tableconfig.setUp()
tableconfig.runTest()
