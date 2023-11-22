from p4utils.utils.sswitch_thrift_API import SimpleSwitchThriftAPI


# Copyright 2013-present Barefoot Networks, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# TODO: Replace PROC, ACTION, and TABLE

"""
Thrift PD interface DV test
"""

import logging
import os
import random
import sys
import time
import unittest
import re

# remember to install it to
import p4runtime_lib


this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
# print(sys.path)
# import common
from common import *

rack_physical_num = int(server_physical_num / 2)
# print(client_physical_num)
cached_list = [0, 1]
hot_list = [0, 1]
validvalue_list = [0, 1, 3]
# validvalue_list = [0, 1, 2, 3] # If with PUTREQ_LARGE
latest_list = [0, 1]
deleted_list = [0, 1]
sampled_list = [0, 1]
lastclone_list = [0, 1]
snapshot_flag_list = [0, 1]
case1_list = [0, 1]
access_val_mode_list = [0, 1, 2, 3]


class TableConfigure:
    def __init__(self, radk_idx):
        self.radk_idx = radk_idx
        self.controller = SimpleSwitchThriftAPI(9100 + radk_idx + 1, "192.168.122.229")

    def create_mirror_session(self):
        self.client_sids = range(10, 10 + client_physical_num)
        # Mirror_add
        for i in range(client_physical_num):
            print(
                "Binding sid {} with client devport {} for both direction mirroring".format(
                    self.client_sids[i], 1
                )
            )  # clone to client
            self.controller.mirroring_add(self.client_sids[i], 1)

    def setUp(self):
        print("\nSetup")
        # initialize the connection
        self.client_devports = []
        self.server_devports = []
        self.rack_devports = [2, 3, 4, 5]
        # get the device ports from front panel ports
        for client_fpport in client_fpports:
            port, chnl = client_fpport.split("/")
            self.client_devports.append(port)
        for server_fpport in server_fpports:
            port, chnl = server_fpport.split("/")
            self.server_devports.append(port)

        # NOTE: in each pipeline, 64-67 are recir/cpu ports, 68-71 are recir/pktgen ports
        # self.cpuPorts = [64, 192] # CPU port is 100G

        # NOTE: data plane communicate with switchos by software-based reflector, which is deployed in one server machine

    def configure_l2l3_forward_tbl(self):
        for i in range(client_physical_num):
            self.controller.table_add(
                "l2l3_forward_tbl",
                "l2l3_forward",
                [client_macs[i], client_ips[i] + "/32"],
                ["1"],
            )
        for i in range(server_physical_num):
            self.controller.table_add(
                "l2l3_forward_tbl",
                "l2l3_forward",
                [server_macs[i], server_ips[i] + "/32"],
                [hex(self.rack_devports[int(i / 2)])],
            )

    def configure_hash_for_partition_tbl(self):
        # req
        for tmpoptype in [
            GETREQ,
            GETREQ_SPINE,
            PUTREQ,
            DELREQ,
            LOADREQ,
            PUTREQ_LARGEVALUE,
            CACHE_POP_INSWITCH,
            WARMUPREQ,
            CACHE_EVICT_LOADFREQ_INSWITCH,
            SETVALID_INSWITCH,
            NETCACHE_CACHE_POP_INSWITCH_NLATEST,
            CACHE_EVICT_LOADDATA_INSWITCH,
            LOADSNAPSHOTDATA_INSWITCH,
            GETRES_LATEST_SEQ,
            GETRES_DELETED_SEQ,
        ]:
            matchspec0 = [hex(tmpoptype)]
            self.controller.table_add(
                "hash_for_partition_tbl", "hash_for_partition", matchspec0
            )

    # server -> client
    def configure_ipv4_forward_tbl(self):
        for tmp_client_physical_idx in range(client_physical_num):
            ipv4addr0 = client_ips[tmp_client_physical_idx]
            eport = "1"
            tmpsid = self.client_sids[tmp_client_physical_idx]
            # 1 client
            for tmpoptype in [
                GETRES_SEQ,
                PUTRES_SEQ,
                DELRES_SEQ,
                WARMUPACK,
                SCANRES_SPLIT,
                LOADACK,
                GETRES_LARGEVALUE_SEQ,
                GETRES,
                PUTRES,
                DELRES,
                GETRES_LARGEVALUE,
            ]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + client_ips[tmp_client_physical_idx] + "/32",
                ]
                actnspec0 = [eport]
                # print('eport',eport)
                self.controller.table_add(
                    "ipv4_forward_tbl", "forward_normal_response", matchspec0, actnspec0
                )
            # 2 client
            for tmpoptype in [GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
                matchspec0 = [
                    hex(tmpoptype),
                    "" + client_ips[tmp_client_physical_idx] + "/32",
                    hex(0),
                ]
                actnspec0 = [hex(tmpsid)]
                self.controller.table_add(
                    "ipv4_forward_tbl",
                    "forward_special_get_response",
                    matchspec0,
                    actnspec0,
                )

    def configure_hash_leaf_partition_tbl(self):
        # partition for rack 3 4 5 6
        hash_range_per_server = switch_partition_count / server_physical_num
        for tmpoptype in [
            GETREQ,
            GETREQ_SPINE,
            PUTREQ,
            DELREQ,
            LOADREQ,
            PUTREQ_LARGEVALUE,
            CACHE_POP_INSWITCH,
            WARMUPREQ,
            CACHE_EVICT_LOADFREQ_INSWITCH,
            CACHE_EVICT_LOADDATA_INSWITCH,
            LOADSNAPSHOTDATA_INSWITCH,
            SETVALID_INSWITCH,
            GETRES_LATEST_SEQ,
            NETCACHE_CACHE_POP_INSWITCH_NLATEST,
        ]:
            hash_start = 0  # [0, partition_count-1]
            for server_idx in range(server_physical_num):
                if server_idx == server_physical_num - 1:
                    hash_end = (
                        switch_partition_count - 1
                    )  # if end is not included, then it is just processed by port 1111
                else:
                    hash_end = hash_start + hash_range_per_server - 1
                # NOTE: both start and end are included
                op_hdr_optype = tmpoptype
                meta_hashval_for_partition_start = int(hash_start)
                meta_hashval_for_partition_end = int(hash_end)
                self.controller.table_add(
                    "hash_leaf_partition_tbl",
                    "hash_leaf_partition",
                    [
                        hex(op_hdr_optype),
                        ""
                        + hex(meta_hashval_for_partition_start)
                        + "->"
                        + hex(meta_hashval_for_partition_end),
                    ],
                    [hex(int(2 + server_idx / 2))],
                    0,
                )
                hash_start = hash_end + 1

    def configure_eg_port_forward_tbl(self):
        for tmpoptype in [GETREQ_SPINE,]:
            matchspec0 = [hex(tmpoptype)]
            self.controller.table_add("eg_port_forward_tbl","update_netcache_getreq_spine_to_getreq",matchspec0)
    def configure_update_vallen_tbl(self):
        for is_cached in cached_list:
            # for is_latest in latest_list:
            matchspec0 = [hex(GETREQ_SPINE), hex(is_cached)]
            if is_cached == 1:
                self.controller.table_add("update_vallen_tbl", "get_vallen", matchspec0)
    def configure_add_and_remove_value_header_tbl(self):
        # NOTE: egress pipeline must not output PUTREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, CACHE_POP_INSWITCH, and PUTREQ_INSWITCH
        # NOTE: even for future PUTREQ_LARGE/GETRES_LARGE, as their values should be in payload, we should invoke add_only_vallen() for vallen in [0, global_max_vallen]
        # , LOADREQ
        for tmpoptype in [GETRES]:
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
    
    def configure_update_pktlen_tbl(self):
        for i in range(int(switch_max_vallen / 8 + 1)):  # i from 0 to 16
            if i == 0:
                vallen_start = 0
                vallen_end = 0
                aligned_vallen = 0
            else:
                vallen_start = (i - 1) * 8 + 1  # 1, 9, ..., 121
                vallen_end = (i - 1) * 8 + 8  # 8, 16, ..., 128
                aligned_vallen = vallen_end  # 8, 16, ..., 128
            val_stat_udplen = aligned_vallen + 34
            val_stat_iplen = aligned_vallen + 54
            val_seq_udplen = aligned_vallen + 34
            val_seq_iplen = aligned_vallen + 54
            matchspec0 = [
                hex(GETRES),
                "" + hex(vallen_start) + "->" + hex(vallen_end),
            ]  # [vallen_start, vallen_end]
            actnspec0 = [hex(val_stat_udplen), hex(val_stat_iplen)]
            self.controller.table_add(
                "update_pktlen_tbl", "update_pktlen", matchspec0, actnspec0, 0
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

    ### MAIN ###
    def runTest(self):
        ################################
        ### Normal MAT Configuration ###
        ################################

        # Ingress pipeline

        # Stage 0
        print(self.client_devports, self.server_devports)
        # Table: l2l3_forward_tbl (default: nop; size: client_physical_num+server_physical_num = 4 < 16)
        print("Configuring l2l3_forward_tbl")
        self.configure_l2l3_forward_tbl()

        print("Configuring hash_for_partition_tbl")
        self.configure_hash_for_partition_tbl()
        # client -> server
        # Table: hash_partition_tbl (default: nop; size <= 5 * 128)
        print("Configuring hash_leaf_partition_tbl")
        self.configure_hash_leaf_partition_tbl()
        # server -> client
        # Table: ipv4_forward_tbl (default: nop; size: 6*client_physical_num=12 < 6*8=48)
        print("Configuring ipv4_forward_tbl")
        self.configure_ipv4_forward_tbl()
        
        print("Configuring prepare_for_cachehit_tbl")
        for client_physical_idx in range(client_physical_num):
            tmp_clientsid = self.client_sids[client_physical_idx]
            for tmpoptype in [GETREQ]:
                self.controller.table_add(
                    "prepare_for_cachehit_tbl",
                    "set_client_sid",
                    [hex(tmpoptype), "" + client_ips[client_physical_idx] + "/32"],
                    [hex(tmp_clientsid)],
                )
        print("Configuring access_deleted_tbl")
        # self.configure_access_deleted_tbl()
        
        # Table: update_ipmac_srcport_tbl (default: nop; 5*client_physical_num+5*server_physical_num=20 < 10*8=80 < 128)
        # NOTE: udp.dstport is updated by eg_port_forward_tbl (only required by switch2switchos)
        # NOTE: update_ipmac_srcport_tbl focues on src/dst ip/mac and udp.srcport
        print("Configuring eg_port_forward_tbl")
        self.configure_eg_port_forward_tbl()
        
        print("Configuring update_vallen_tbl")
        self.configure_update_vallen_tbl()

        for i in range(1, 17):
            print("Configuring update_vallo{}_tbl".format(i))
            self.configure_update_val_tbl("lo{}".format(i))
            print("Configuring update_valhi{}_tbl".format(i))
            self.configure_update_val_tbl("hi{}".format(i))

for i in range(int(server_physical_num / 2)):
    print("Configuring RACK {}".format(i))
    tableconfig = TableConfigure(i)
    tableconfig.setUp()
    # tableconfig.clean_all_tables()
    tableconfig.create_mirror_session()
    tableconfig.runTest()
