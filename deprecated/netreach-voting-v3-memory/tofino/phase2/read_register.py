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

from netbufferv3.p4_pd_rpc.ttypes import *
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
server_notified_port = int(config.get("server", "server_notified_port"))
bucket_count = int(config.get("switch", "bucket_num"))
switch_max_vallen = int(config.get("global", "switch_max_vallen"))
server_num = int(config.get("server", "server_num"))
ingress_pipeidx = int(config.get("switch", "ingress_pipeidx"))
egress_pipeidx = int(config.get("switch", "egress_pipeidx"))

fp_ports = ["2/0", "3/0"]

class RegisterUpdate(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbufferv3"])

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
            tmpreg += pow(2, 16) # int16 -> uint16
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
            tmpreg += pow(2, 32) # int32 -> uint32
        return tmpreg

    def runTest(self):
        print "Reading reagisters"
        flags = netbufferv3_register_flags_t(read_hw_sync=True)
        # Ingress
        keylolo_list = self.client.register_range_read_keylolo_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        keylohi_list = self.client.register_range_read_keylohi_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        keyhilo_list = self.client.register_range_read_keyhilo_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        keyhihi_list = self.client.register_range_read_keyhihi_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        vallo_list_list = []
        valhi_list_list = []
        for i in range(switch_max_vallen/8): # 128 bytes / 8 = 16 register arrays
            vallo_list_list.append(eval("self.client.register_range_read_vallo{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags))
            valhi_list_list.append(eval("self.client.register_range_read_valhi{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags))
        vallen_list = self.client.register_range_read_vallen_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        valid_list = self.client.register_range_read_valid_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        # Egress
        case3_list = self.client.register_range_read_case3_reg(self.sess_hdl, self.dev_tgt, 0, server_num, flags)

        # Send notification for server-side snapshot if necessary
        notification_num = 0
        for i in range(server_num): # Two pipelines
            idx = i + egress_pipeidx * server_num
            tmpcase3 = case3_list[idx]
            if tmpcase3 == 0:
                notification_num += 1
        print "Notify server-side snapshot (#: {})".format(notification_num)
        if notification_num > 0:
            notify_sockfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            notify_buf = struct.pack("=c", 1) # 1 byte for notification
            notify_sockfd.sendto(notify_buf, (server_backup_ip, server_notified_port))
            notify_sockfd.close()

        print "Reset flag"
        actnspec0 = netbufferv3_load_backup_flag_action_spec_t(0)
        self.client.load_backup_flag_tbl_set_default_action_load_backup_flag(\
                self.sess_hdl, self.dev_tgt, actnspec0)

        self.conn_mgr.complete_operations(self.sess_hdl)

        print "Filtering data"
        count = 0
        buf = bytearray()
        #pipeidx = None
        for i in range(bucket_count): # Two pipelines
            #if pipeidx is None:
            #    tmpvalid0 = valid_list[i]
            #    tmpvalid1 = valid_list[i+bucket_count]
            #    if tmpvalid0 <= 0 and tmpvalid1 <= 0:
            #        continue
            #    elif tmpvalid0 > 0:
            #        pipeidx = 0
            #    else:
            #        pipeidx = 1
            idx = i + ingress_pipeidx * bucket_count
            tmpvalid = valid_list[idx]
            if tmpvalid <= 0:
                continue
            # each tmpkeyxxxxxx is 2B
            tmpkeylolo = RegisterUpdate.get_reg16(keylolo_list, idx)
            tmpkeylohi = RegisterUpdate.get_reg16(keylohi_list, idx)
            tmpkeyhilo = RegisterUpdate.get_reg16(keyhilo_list, idx)
            tmpkeyhihi = RegisterUpdate.get_reg16(keyhihi_list, idx)
            tmpkeylo = (tmpkeylohi << 32) + tmpkeylolo
            tmpkeyhi = (tmpkeyhihi << 32) + tmpkeyhilo
            tmpvallen = vallen_list[idx]
            buf = buf + struct.pack("=H2QB", i, tmpkeylo, tmpkeyhi, tmpvallen) # i is hashidx
            #print("i: {} keylo: {:016x} keyhi: {:016x} vallen: {:02x}".format(hashidx, tmpkeylo, tmpkeyhi, tmpvallen))
            for j in range(tmpvallen):
                val_idx = tmpvallen - 1 - j
                tmpvallo = RegisterUpdate.get_reg32(vallo_list_list[val_idx], idx)
                tmpvalhi = RegisterUpdate.get_reg32(valhi_list_list[val_idx], idx)
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
        sockfd.close()







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
