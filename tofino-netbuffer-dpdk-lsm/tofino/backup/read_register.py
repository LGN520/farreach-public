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

# Front Panel Ports
#   List of front panel ports to use. Each front panel port has 4 channels.
#   Port 1 is broken to 1/0, 1/1, 1/2, 1/3. Test uses 2 ports.
#
#   ex: ["1/0", "1/1"]
#

bucket_count = 8 # Must be the same as basic.p4
fp_ports = ["2/0", "3/0"]
#switch_ip = "1.1.1.1" # useless
#switch_mac = "01:01:01:01:01:01" # useless
#switch_port = 1 #useless
server_ip = "172.16.112.32"
#server_mac = "9c:69:b4:60:ef:8d"
server_port = 3333

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

    def runTest(self):
        flags = netbuffer_register_flags_t(read_hw_sync=True)
        keylo_list = self.client.register_range_read_keylo_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        keyhi_list = self.client.register_range_read_keyhi_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        vallo_list = self.client.register_range_read_vallo_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        valhi_list = self.client.register_range_read_valhi_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        valid_list = self.client.register_range_read_valid_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)

        self.conn_mgr.complete_operations(self.sess_hdl)

        print("keylo_list size: {} keyhi_list size: {}".format(len(keylo_list), len(keyhi_list)))
        buf = struct.pack("I", bucket_count)
        for idx in range(bucket_count):
            i = idx + bucket_count # Our ports are in the 2nd pipeline
            tmpkeylo = keylo_list[i]
            tmpkeyhi = keyhi_list[i]
            tmpvallo = vallo_list[i]
            tmpvalhi = valhi_list[i]
            tmpvalid = valid_list[i]
            print("keylo: {}, keyhi: {}, vallo: {}, valhi: {}, valid: {}".format(tmpkeylo, tmpkeyhi, tmpvallo, tmpvalhi, tmpvalid))
            if (tmpkeylo < 0):
                tmpkeylo += pow(2, 32)
            if (tmpkeyhi < 0):
                tmpkeyhi += pow(2, 32)
            if (tmpvallo < 0):
                tmpvallo += pow(2, 32)
            if (tmpvalhi < 0):
                tmpvalhi += pow(2, 32)
            tmpkey = (tmpkeyhi << 32) + tmpkeylo
            tmpval = (tmpvalhi << 32) + tmpvallo
            print("final key: {} final val: {}".format(tmpkey, tmpval))
            buf = buf + struct.pack("2QB", tmpkey, tmpval, tmpvalid)

        sockfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sockfd.sendto(buf, (server_ip, server_port))

        self.conn_mgr.complete_operations(self.sess_hdl)
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
