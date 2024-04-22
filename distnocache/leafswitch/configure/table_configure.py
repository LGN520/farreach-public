from p4utils.utils.sswitch_thrift_API import SimpleSwitchThriftAPI

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
    def __init__(self, rack_idx):
        self.rack_idx = rack_idx
        self.controller = SimpleSwitchThriftAPI(9090 + rack_idx + 1, "192.168.122.229")

    def setUp(self):
        print("\nSetup")
        # initialize the connection
        self.client_devports = []
        self.server_devports = []

        # get the device ports from front panel ports
        for client_fpport in client_fpports:
            port, chnl = client_fpport.split("/")
            self.client_devports.append("3")
        for server_fpport in server_fpports:
            port, chnl = server_fpport.split("/")
            self.server_devports.append(port)

        # NOTE: in each pipeline, 64-67 are recir/cpu ports, 68-71 are recir/pktgen ports
        # self.cpuPorts = [64, 192] # CPU port is 100G

        # NOTE: data plane communicate with switchos by software-based reflector, which is deployed in one server machine

    ### MAIN ###
    def runTest(self):
        ################################
        ### Normal MAT Configuration ###
        ################################

        # Ingress pipeline

        # Stage 0
        print(self.client_devports, self.server_devports)
        # Table: l2l3_forward_tbl (default: nop; size: client_physical_num+(self.rack_idx-1)*2,(self.rack_idx-1)*2+2 = 4 < 16)
        print("Configuring l2l3_forward_tbl")
        # -> client == -> spine swicth 1
        for i in range(client_physical_num):
            self.controller.table_add(
                "l2l3_forward_tbl",
                "l2l3_forward",
                [client_macs[i], client_ips[i] + "/32"],
                [self.client_devports[i]],
            )
        for i in range((self.rack_idx) * 2, (self.rack_idx) * 2 + 2):
            self.controller.table_add(
                "l2l3_forward_tbl",
                "l2l3_forward",
                [server_macs[i], server_ips[i] + "/32"],
                [self.server_devports[i]],
            )

        print("Configuring hash_for_partition_tbl")
        for tmpoptype in [GETREQ, PUTREQ, DELREQ, LOADREQ, PUTREQ_LARGEVALUE]:
            matchspec0 = [hex(tmpoptype)]
            self.controller.table_add(
                "hash_for_partition_tbl", "hash_for_partition", matchspec0
            )
        # Stage 1
        # Table: hash_partition_tbl (default: nop; size <= 5 * 128)
        print("Configuring hash_partition_tbl")
        hash_range_per_server = switch_partition_count / server_total_logical_num
        for tmpoptype in [GETREQ, PUTREQ, DELREQ, LOADREQ, PUTREQ_LARGEVALUE]:
            hash_start = 0  # [0, partition_count-1]
            for global_server_logical_idx in range(server_total_logical_num):
                if global_server_logical_idx == server_total_logical_num - 1:
                    hash_end = (
                        switch_partition_count - 1
                    )  # if end is not included, then it is just processed by port 1111
                else:
                    hash_end = hash_start + hash_range_per_server - 1
                # NOTE: both start and end are included
                op_hdr_optype = tmpoptype
                meta_hashval_for_partition_start = int(hash_start)
                meta_hashval_for_partition_end = int(hash_end)

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
                        [hex(udp_dstport), eport],
                        0,
                    )
                hash_start = hash_end + 1

        # Table: ipv4_forward_tbl (default: nop; size: 6*client_physical_num=12 < 6*8=48)
        print("Configuring ipv4_forward_tbl")
        for tmp_client_physical_idx in range(client_physical_num):
            eport = self.client_devports[tmp_client_physical_idx] # from ToR to spine switch
            for tmpoptype in [
                GETRES,
                PUTRES,
                DELRES,
                SCANRES_SPLIT,
                LOADACK,
                GETRES_LARGEVALUE,
            ]:
                self.controller.table_add(
                    "ipv4_forward_tbl",
                    "forward_normal_response",
                    [
                        hex(tmpoptype),
                        "" + client_ips[tmp_client_physical_idx] + "/32",
                    ],
                    [eport],
                )

        # Stage 10

        # Table: update_pktlen_tbl (default: nop; 1)
        print("Configuring update_pktlen_tbl")
        scanreqsplit_udplen = 49
        scanreqsplit_iplen = 69
        self.controller.table_add(
            "update_pktlen_tbl",
            "update_pktlen",
            [hex(SCANREQ_SPLIT)],
            [hex(scanreqsplit_udplen), hex(scanreqsplit_iplen)],
        )
        # Table: update_ipmac_srcport_tbl (default: nop; 5*client_physical_num+5*(self.rack_idx)*2,(self.rack_idx)*2+2=20 < 10*8=80 < 128)
        # NOTE: udp.dstport is updated by eg_port_forward_tbl (only required by switch2switchos)
        # NOTE: update_ipmac_srcport_tbl focues on src/dst ip/mac and udp.srcport
        print("Configuring update_ipmac_srcport_tbl")
        # (1) for response from server to client, egress port has been set based on ip.dstaddr (or by clone_i2e) in ingress pipeline
        # (2) for response from switch to client, egress port has been set by clone_e2e in egress pipeline
        # ToR switch doesnt need to update pkt to client
        # it will be processed by spine switch
        # for tmp_client_physical_idx in range(client_physical_num):
        #     tmp_devport = self.client_devports[tmp_client_physical_idx]
        #     tmp_client_mac = client_macs[tmp_client_physical_idx]
        #     tmp_client_ip = client_ips[tmp_client_physical_idx]
        #     tmp_server_mac = server_macs[0]
        #     tmp_server_ip = server_ips[0]
        #     for tmpoptype in [
        #         GETRES,
        #         PUTRES,
        #         DELRES,
        #         SCANRES_SPLIT,
        #         LOADACK,
        #         GETRES_LARGEVALUE,
        #     ]:
        #         self.controller.table_add(
        #             "update_ipmac_srcport_tbl",
        #             "update_ipmac_srcport_server2client",
        #             [hex(tmpoptype), tmp_devport],
        #             [
        #                 tmp_client_mac,
        #                 tmp_server_mac,
        #                 tmp_client_ip,
        #                 tmp_server_ip,
        #                 hex(server_worker_port_start),
        #             ],
        #         )
        # for request from client to server, egress port has been set by partition_tbl in ingress pipeline
        for tmp_server_physical_idx in range(
            (self.rack_idx) * 2, (self.rack_idx) * 2 + 2
        ):
            tmp_devport = self.server_devports[tmp_server_physical_idx]
            tmp_server_mac = server_macs[tmp_server_physical_idx]
            tmp_server_ip = server_ips[tmp_server_physical_idx]
            for tmpoptype in [
                GETREQ,
                PUTREQ,
                DELREQ,
                SCANREQ_SPLIT,
                LOADREQ,
                PUTREQ_LARGEVALUE,
            ]:
                self.controller.table_add(
                    "update_ipmac_srcport_tbl",
                    "update_dstipmac_client2server",
                    [hex(tmpoptype), tmp_devport],
                    [tmp_server_mac, tmp_server_ip],
                )


for i in range(int(server_physical_num / 2)):
    print("Configuring RACK {}".format(i))
    tableconfig = TableConfigure(i)
    tableconfig.setUp()
    tableconfig.runTest()
