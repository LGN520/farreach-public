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
import pd_base_tests
import pltfm_pm_rpc
import pal_rpc
import random
import sys
import time
import unittest

from netbuffer.p4_pd_rpc.ttypes import *
from pltfm_pm_rpc.ttypes import *
from pal_rpc.ttypes import *
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from res_pd_rpc.ttypes import *

import socket
import struct

this_dir = os.path.dirname(os.path.abspath(__file__))

import ConfigParser
config = ConfigParser.ConfigParser()
with open(os.path.join(os.path.dirname(os.path.dirname(this_dir)), "config.ini"), "r") as f:
    config.readfp(f)

bucket_count = int(config.get("switch", "bucket_num"))

# Front Panel Ports
#   List of front panel ports to use. Each front panel port has 4 channels.
#   Port 1 is broken to 1/0, 1/1, 1/2, 1/3. Test uses 2 ports.
#
#   ex: ["1/0", "1/1"]
#

fp_ports = ["2/0", "3/0"]
#switch_ip = "1.1.1.1" # useless
#switch_mac = "01:01:01:01:01:01" # useless
#switch_port = 1 #useless
server_ip = "172.16.112.32"
#server_mac = "9c:69:b4:60:ef:8d"
server_port = 3333
max_val_len = 12

class RegisterUpdate(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbuffer"])

    def setUp(self):
        print '\nSetup'

        # initialize the connection
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.sess_hdl = self.conn_mgr.client_init()
        self.dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))

        self.platform_type = "mavericks"
        board_type = self.pltfm_pm.pltfm_pm_board_type_get()
        if re.search("0x0234|0x1234|0x4234|0x5234", hex(board_type)):
            self.platform_type = "mavericks"
        elif re.search("0x2234|0x3234", hex(board_type)):
            self.platform_type = "montara"

        self.devPorts = []
        # get the device ports from front panel ports
        for fpPort in fp_ports:
            port, chnl = fpPort.split("/")
            devPort = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
            self.devPorts.append(devPort)

    @staticmethod
    def get_reg16(reglist, idx):
        tmpreg = reglist[idx]
        if (tmpreg < 0):
            tmpreg += pow(2, 16)
        return tmpreg

    @staticmethod
    def get_reg32(reglist, idx):
        tmpreg = reglist[idx]
        if (tmpreg < 0):
            tmpreg += pow(2, 32)
        return tmpreg

    def runTest(self):
        flags = netbuffer_register_flags_t(read_hw_sync=True)
        keylololo_list = self.client.register_range_read_keylololo_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        keylolohi_list = self.client.register_range_read_keylolohi_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        keylohilo_list = self.client.register_range_read_keylohilo_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        keylohihi_list = self.client.register_range_read_keylohihi_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        keyhilolo_list = self.client.register_range_read_keyhilolo_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        keyhilohi_list = self.client.register_range_read_keyhilohi_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        keyhihilo_list = self.client.register_range_read_keyhihilo_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        keyhihihi_list = self.client.register_range_read_keyhihihi_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        vallo_list_list = []
        valhi_list_list = []
        for i in range(max_val_len):
            vallo_list_list.append(getattr(self.client, "register_range_read_vallo{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags))
            valhi_list_list.append(getattr(self.client, "register_range_read_valhi{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags))
        vallen_list = self.client.register_range_read_vallen_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        valid_list = self.client.register_range_read_valid_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)

        self.conn_mgr.complete_operations(self.sess_hdl)

        count = 0
        buf = []
        for idx in range(bucket_count):
            i = idx + bucket_count # Our ports are in the 2nd pipeline
            tmpvalid = valid_list[i]
            if tmpvalid <= 0:
                continue
            tmpvallen = vallen_list[i]
            if tmpvallen <= 0:
                continue
            # each tmpkeyxxxxxx is 2B
            tmpkeylololo = RegisterUpdate.get_reg16(keylololo_list, i)
            tmpkeylolohi = RegisterUpdate.get_reg16(keylolohi_list, i)
            tmpkeylohilo = RegisterUpdate.get_reg16(keylohilo_list, i)
            tmpkeylohihi = RegisterUpdate.get_reg16(keylohihi_list, i)
            tmpkeyhilolo = RegisterUpdate.get_reg16(keyhilolo_list, i)
            tmpkeyhilohi = RegisterUpdate.get_reg16(keyhilohi_list, i)
            tmpkeyhihilo = RegisterUpdate.get_reg16(keyhihilo_list, i)
            tmpkeyhihihi = RegisterUpdate.get_reg16(keyhihihi_list, i)
            tmpkeylo = (((tmpkeylohihi << 16) + tmpkeylohilo) << 32) + ((tmpkeylolohi << 16) + tmpkeylololo)
            tmpkeyhi = (((tmpkeyhihihi << 16) + tmpkeyhihilo) << 32) + ((tmpkeyhilohi << 16) + tmpkeyhilolo)
            buf = buf + struct.pack("2QB", tmpkeylo, tmpkeyhi, tmpvallen)
            #print("keylo: {} keyhi: {} vallen: {}".format(tmpkeylo, tmpkeyhi, tmpvallen))
            for val_idx in range(tmpvallen):
                tmpvallo = RegisterUpdate.get_reg32(vallo_list_list[val_idx], i)
                tmpvalhi = RegisterUpdate.get_reg32(valhi_list_list[val_idx], i)
                tmpval = (tmpvalhi << 32) + tmpvallo
                buf = buf + struct.pack("Q", tmpval)
            count += 1
            if idx >= 1024:
                break
        buf = struct.pack("I", count) + buf

        sockfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sockfd.sendto(buf, (server_ip, server_port))

        self.conn_mgr.client_cleanup(self.sess_hdl)

        #pktlen = 14 + 20 + 8 + 4 + 17 * bucket_count
        #pkt = simple_udp_packet(pktlen, eth_dst=server_mac, eth_src=switch_mac, 
        #        ip_src=switch_ip, ip_dst=server_ip, ip_ttl=64, 
        #        udp_sport=switch_port, udp_dport=server_port,
        #        udp_payload=buf)
        #print(type(pkt))
        #print(len(pkt))
        #print(pkt)
        #print("send packet from {}".format(self.devPorts[0]))
        #sent = send_packet(self, (0, self.devPorts[0]), str(pkt))
        #print("sent: {}".format(sent))
        #verify_packets(self, pkt, [self.devPorts[1]])
