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

import ConfigParser
config = ConfigParser.ConfigParser()
with open(os.path.join(os.path.dirname(os.path.dirname(this_dir)), "config.ini"), "r") as f:
    config.readfp(f)

server_backup_ip = str(config.get("server", "server_backup_ip"))
server_backup_port = int(config.get("server", "server_backup_port"))
bucket_count = int(config.get("switch", "bucket_num"))
max_val_len = int(config.get("global", "max_val_length"))

fp_ports = ["2/0", "3/0"]

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

    @staticmethod
    def get_reg16(reglist, idx):
        tmpreg = reglist[idx] # Big-endian
        tmpreg = (((tmpreg & 0xFF00) >> 8) & 0x00FF) | ((tmpreg & 0x00FF) << 8) # Small-endian
        if (tmpreg < 0):
            tmpreg += pow(2, 16)
        return tmpreg

    @staticmethod
    def get_reg32(reglist, idx):
        tmpreg = reglist[idx] # Big-endian
        tmphihi = ((tmpreg & 0xFF000000) >> 24) & 0x000000FF
        tmphilo = ((tmpreg & 0x00FF0000) >> 16) & 0x000000FF
        tmplohi = ((tmpreg & 0x0000FF00) >> 8) & 0x000000FF
        tmplolo = tmpreg & 0x000000FF
        tmpreg = (tmplolo << 24) | (tmplohi << 16) | (tmphilo << 8) | tmphihi # Small-endian
        if (tmpreg < 0):
            tmpreg += pow(2, 32)
        return tmpreg

    def runTest(self):
        print "Reading reagisters"
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
            vallo_list_list.append(eval("self.client.register_range_read_vallo{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags))
            valhi_list_list.append(eval("self.client.register_range_read_valhi{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags))
        vallen_list = self.client.register_range_read_vallen_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        valid_list = self.client.register_range_read_valid_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        dirty_list = self.client.register_range_read_dirty_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)

        print "Reset flag"
        actnspec0 = netbuffer_load_backup_flag_action_spec_t(0)
        self.client.load_backup_flag_tbl_set_default_action_load_backup_flag(\
                self.sess_hdl, self.dev_tgt, actnspec0)

        self.conn_mgr.complete_operations(self.sess_hdl)

        print "Filtering data"
        count = 0
        buf = bytearray()
        for i in range(2*bucket_count): # Two pipelines
            tmpvalid = valid_list[i]
            if tmpvalid <= 0:
                continue
            tmpdirty = dirty_list[i]
            if tmpdirty <= 0:
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
            hashidx = i
            if hashidx >= bucket_count:
                hashidx = (hashidx - bucket_count)
            buf = buf + struct.pack("=H2QB", hashidx, tmpkeylo, tmpkeyhi, tmpvallen)
            #print("i: {} keylo: {:016x} keyhi: {:016x} vallen: {:02x}".format(hashidx, tmpkeylo, tmpkeyhi, tmpvallen))
            for val_idx in range(tmpvallen):
                tmpvallo = RegisterUpdate.get_reg32(vallo_list_list[val_idx], i)
                tmpvalhi = RegisterUpdate.get_reg32(valhi_list_list[val_idx], i)
                tmpval = (tmpvalhi << 32) + tmpvallo
                buf = buf + struct.pack("Q", tmpval)
            count += 1
        bufsize = len(buf) + 4 + 4
        buf = struct.pack("=I", bufsize) + struct.pack("=I", count) + buf

        self.conn_mgr.client_cleanup(self.sess_hdl)

        print "Connecting server"
        sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sockfd.connect((server_backup_ip, server_backup_port))

        print "Sending backup data"
        startidx = 0
        totalnum = len(buf)
        sentnum = 0
        while True:
            sentnum += sockfd.send(buf[startidx:])
            if sentnum >= totalnum:
                print "sentnum: {} totalnum: {}".format(sentnum, totalnum)
                break
            else:
                startidx = sentnum

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
