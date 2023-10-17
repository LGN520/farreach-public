from p4utils.utils.sswitch_thrift_API import SimpleSwitchThriftAPI
controller = SimpleSwitchThriftAPI(9090, "192.168.122.229") 


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
# print(client_physical_num)
cached_list = [0, 1]
hot_list = [0, 1]
validvalue_list = [0, 1, 3]
#validvalue_list = [0, 1, 2, 3] # If with PUTREQ_LARGE
latest_list = [0, 1]
deleted_list = [0, 1]
sampled_list = [0, 1]
lastclone_list = [0, 1]
snapshot_flag_list = [0, 1]
case1_list = [0, 1]
access_val_mode_list = [0, 1, 2, 3]




class TableConfigure():
    # def __init__(self):
    #     # initialize the thrift data plane
    def setUp(self):
        print ('\nSetup')
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


        # NOTE: in each pipeline, 64-67 are recir/cpu ports, 68-71 are recir/pktgen ports
        #self.cpuPorts = [64, 192] # CPU port is 100G

        # NOTE: data plane communicate with switchos by software-based reflector, which is deployed in one server machine

   
    ### MAIN ###
    def runTest(self):
        ################################
        ### Normal MAT Configuration ###
        ################################

        # Ingress pipeline

        # Stage 0
        print(self.client_devports,self.server_devports)
        # Table: l2l3_forward_tbl (default: nop; size: client_physical_num+server_physical_num = 4 < 16)
        print ("Configuring l2l3_forward_tbl")
        for i in range(client_physical_num):
            controller.table_add('l2l3_forward_tbl', 'l2l3_forward', [client_macs[i],client_ips[i]+'/32'], [self.client_devports[i]])
        for i in range(server_physical_num):
            controller.table_add('l2l3_forward_tbl', 'l2l3_forward', [server_macs[i],server_ips[i]+'/32'], [self.server_devports[i]])
            
        # Stage 1

        if RANGE_SUPPORT == False:
            # Table: hash_partition_tbl (default: nop; size <= 5 * 128)
            print ("Configuring hash_partition_tbl")
            hash_range_per_server = switch_partition_count / server_total_logical_num
            for tmpoptype in [GETREQ, PUTREQ, DELREQ, LOADREQ, PUTREQ_LARGEVALUE]:
                hash_start = 0 # [0, partition_count-1]
                for global_server_logical_idx in range(server_total_logical_num):
                    if global_server_logical_idx == server_total_logical_num - 1:
                        hash_end = switch_partition_count - 1 # if end is not included, then it is just processed by port 1111
                    else:
                        hash_end = hash_start + hash_range_per_server - 1
                    # NOTE: both start and end are included
                    op_hdr_optype = tmpoptype
                    meta_hashval_for_partition_start =int(hash_start)
                    meta_hashval_for_partition_end =int(hash_end)
                       
                    # Forward to the egress pipeline of server
                    server_physical_idx = -1
                    local_server_logical_idx = -1
                    for tmp_server_physical_idx in range(server_physical_num):
                        for tmp_local_server_logical_idx in range(len(server_logical_idxes_list[tmp_server_physical_idx])):
                            if global_server_logical_idx == server_logical_idxes_list[tmp_server_physical_idx][tmp_local_server_logical_idx]:
                                server_physical_idx = tmp_server_physical_idx
                                local_server_logical_idx = tmp_local_server_logical_idx
                                break
                    if server_physical_idx == -1:
                        print ("WARNING: no physical server covers global_server_logical_idx {} -> no corresponding MAT entries in hash_partition_tbl".format(global_server_logical_idx))
                    else:
                        #udp_dstport = server_worker_port_start + global_server_logical_idx
                        udp_dstport = server_worker_port_start + local_server_logical_idx
                        eport = self.server_devports[server_physical_idx]
                        controller.table_add('hash_partition_tbl', 'hash_partition', \
                                [hex(op_hdr_optype),''+hex(meta_hashval_for_partition_start) +'->'+ hex(meta_hashval_for_partition_end)], \
                                [hex(udp_dstport), eport],0)
                    hash_start = hash_end + 1

        # Stage 2 


        # Stage 3

        # Table: ipv4_forward_tbl (default: nop; size: 6*client_physical_num=12 < 6*8=48)
        print ("Configuring ipv4_forward_tbl")
        for tmp_client_physical_idx in range(client_physical_num):
            eport = self.client_devports[tmp_client_physical_idx]
            for tmpoptype in [GETRES, PUTRES, DELRES, SCANRES_SPLIT, LOADACK, GETRES_LARGEVALUE]:
                controller.table_add('ipv4_forward_tbl','forward_normal_response',\
                    [hex(convert_u16_to_i16(tmpoptype)),''+client_ips[tmp_client_physical_idx]+'/32'],[eport])

    

        # Stage 10



        # Table: update_pktlen_tbl (default: nop; 1)
        print ("Configuring update_pktlen_tbl")
        scanreqsplit_udplen = 49
        scanreqsplit_iplen = 69
        controller.table_add('update_pktlen_tbl','update_pktlen',\
                    [hex(SCANREQ_SPLIT)], \
                    [hex(scanreqsplit_udplen), hex(scanreqsplit_iplen)])
        # Table: update_ipmac_srcport_tbl (default: nop; 5*client_physical_num+5*server_physical_num=20 < 10*8=80 < 128)
        # NOTE: udp.dstport is updated by eg_port_forward_tbl (only required by switch2switchos)
        # NOTE: update_ipmac_srcport_tbl focues on src/dst ip/mac and udp.srcport
        print ("Configuring update_ipmac_srcport_tbl")
        # (1) for response from server to client, egress port has been set based on ip.dstaddr (or by clone_i2e) in ingress pipeline
        # (2) for response from switch to client, egress port has been set by clone_e2e in egress pipeline
        for tmp_client_physical_idx in range(client_physical_num):
            tmp_devport = self.client_devports[tmp_client_physical_idx]
            tmp_client_mac = client_macs[tmp_client_physical_idx]
            tmp_client_ip = client_ips[tmp_client_physical_idx]
            tmp_server_mac = server_macs[0]
            tmp_server_ip = server_ips[0]
            for tmpoptype in [GETRES, PUTRES, DELRES, SCANRES_SPLIT, LOADACK, GETRES_LARGEVALUE]:

                controller.table_add('update_ipmac_srcport_tbl','update_ipmac_srcport_server2client',\
                    [hex(tmpoptype), tmp_devport], \
                    [tmp_client_mac, \
                    tmp_server_mac, \
                    tmp_client_ip, \
                    tmp_server_ip, \
                    hex(server_worker_port_start)])
        # for request from client to server, egress port has been set by partition_tbl in ingress pipeline
        for tmp_server_physical_idx in range(server_physical_num):
            tmp_devport = self.server_devports[tmp_server_physical_idx]
            tmp_server_mac = server_macs[tmp_server_physical_idx]
            tmp_server_ip = server_ips[tmp_server_physical_idx]
            for tmpoptype in [GETREQ, PUTREQ, DELREQ, SCANREQ_SPLIT, LOADREQ, PUTREQ_LARGEVALUE]:
                controller.table_add('update_ipmac_srcport_tbl','update_dstipmac_client2server',\
                    [hex(tmpoptype), tmp_devport], \
                    [tmp_server_mac, \
                    tmp_server_ip])
            


tableconfig = TableConfigure()
tableconfig.setUp()
tableconfig.runTest()

