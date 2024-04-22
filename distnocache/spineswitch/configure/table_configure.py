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
        self.controller = SimpleSwitchThriftAPI(9090 + radk_idx, "192.168.122.229")

    def create_mirror_session(self):
        self.client_sids = range(10, 10 + client_physical_num)
        # Mirror_add
        for i in range(client_physical_num):
            print(
                "Binding sid {} with client devport {} for both direction mirroring".format(
                    self.client_sids[i], self.client_devports[i]
                )
            )  # clone to client
            self.controller.mirroring_add(self.client_sids[i], self.client_devports[i])

    def setUp(self):
        print("\nSetup")
        # initialize the connection
        self.client_devports = []
        self.server_devports = []
        self.rack_devports = [3+i for i in range(int(server_physical_num/2))]
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
                [self.client_devports[i]],
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
            eport = self.client_devports[tmp_client_physical_idx]
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
            # for tmpoptype in [GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
            #     matchspec0 = [
            #         hex(tmpoptype),
            #         "" + client_ips[tmp_client_physical_idx] + "/32",
            #         hex(0),
            #     ]
            #     actnspec0 = [hex(tmpsid)]
            #     self.controller.table_add(
            #         "ipv4_forward_tbl",
            #         "forward_special_get_response",
            #         matchspec0,
            #         actnspec0,
            #     )

    def configure_hash_partition_tbl(self):
        # partition for rack 3 4 5 6
        hash_range_per_server = switch_partition_count / server_physical_num
        for tmpoptype in [
            GETREQ,
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
                    "hash_partition_tbl",
                    "hash_partition",
                    [
                        hex(op_hdr_optype),
                        ""
                        + hex(meta_hashval_for_partition_start)
                        + "->"
                        + hex(meta_hashval_for_partition_end),
                    ],
                    [hex(int(3 + server_idx / 2))],
                    0,
                )
                hash_start = hash_end + 1

    def configure_update_ipmac_srcport_tbl(self):
        # (1) for response from server to client, egress port has been set based on ip.dstaddr (or by clone_i2e) in ingress pipeline
        # (2) for response from switch to client, egress port has been set by clone_e2e in egress pipeline
        for tmp_client_physical_idx in range(client_physical_num):
            tmp_devport = self.client_devports[tmp_client_physical_idx]
            tmp_client_mac = client_macs[tmp_client_physical_idx]
            tmp_client_ip = client_ips[tmp_client_physical_idx]
            tmp_server_mac = server_macs[0]
            tmp_server_ip = server_ips[0]
            for tmpoptype in [
                GETRES,
                PUTRES,
                DELRES,
                SCANRES_SPLIT,
                LOADACK,
                GETRES_LARGEVALUE,
                GETRES_SEQ,
                PUTRES_SEQ,
                DELRES_SEQ,
                WARMUPACK,
                GETRES_LARGEVALUE_SEQ,
            ]:
                self.controller.table_add(
                    "update_ipmac_srcport_tbl",
                    "update_ipmac_srcport_server2client",
                    [hex(tmpoptype), tmp_devport],
                    [
                        tmp_client_mac,
                        tmp_server_mac,
                        tmp_client_ip,
                        tmp_server_ip,
                        hex(server_worker_port_start),
                    ],
                )
        # for request from client to server, egress port has been set by partition_tbl in ingress pipeline
        # it will be processed by ToR

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
        print("Configuring hash_partition_tbl")
        self.configure_hash_partition_tbl()
        # server -> client
        # Table: ipv4_forward_tbl (default: nop; size: 6*client_physical_num=12 < 6*8=48)
        print("Configuring ipv4_forward_tbl")
        self.configure_ipv4_forward_tbl()
        # Stage 10

        # Table: update_ipmac_srcport_tbl (default: nop; 5*client_physical_num+5*server_physical_num=20 < 10*8=80 < 128)
        # NOTE: udp.dstport is updated by eg_port_forward_tbl (only required by switch2switchos)
        # NOTE: update_ipmac_srcport_tbl focues on src/dst ip/mac and udp.srcport
        print("Configuring update_ipmac_srcport_tbl")
        self.configure_update_ipmac_srcport_tbl()


tableconfig = TableConfigure(0)
tableconfig.setUp()
tableconfig.create_mirror_session()
tableconfig.runTest()
