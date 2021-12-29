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
import redis

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
hashidx_prefix = str(config.get("other", "hashidx_prefix"))

fp_ports = ["2/0", "3/0"]

class RegisterUpdate(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbufferv2"])

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
        tmpreg = reglist[idx] # Big-endian int
        tmpreg = (((tmpreg & 0xFF00) >> 8) & 0x00FF) | ((tmpreg & 0x00FF) << 8) # Small-endian int
        if tmpreg < 0:
            tmpreg += pow(2, 16) # int -> uint
        return tmpreg

    @staticmethod
    def get_reg32(reglist, idx):
        tmpreg = reglist[idx] # Big-endian int
        tmphihi = ((tmpreg & 0xFF000000) >> 24) & 0x000000FF
        tmphilo = ((tmpreg & 0x00FF0000) >> 16) & 0x000000FF
        tmplohi = ((tmpreg & 0x0000FF00) >> 8) & 0x000000FF
        tmplolo = tmpreg & 0x000000FF
        tmpreg = (tmplolo << 24) | (tmplohi << 16) | (tmphilo << 8) | tmphihi # Small-endian int
        if tmpreg < 0:
            tmpreg += pow(2, 32) # int -> uint
        return tmpreg

    def runTest(self):
        # TODO:load value from val{vallen} to val1
        print "Reading reagisters"
        flags = netbuffer_register_flags_t(read_hw_sync=True)
        vallo_list_list = []
        valhi_list_list = []
        for i in range(max_val_len):
            vallo_list_list.append(eval("self.client.register_range_read_vallo{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags))
            valhi_list_list.append(eval("self.client.register_range_read_valhi{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags))
        vallen_list = self.client.register_range_read_vallen_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)
        latest_list = self.client.register_range_read_latest_reg(self.sess_hdl, self.dev_tgt, 0, bucket_count, flags)

        print "Reset flag"
        actnspec0 = netbuffer_load_backup_flag_action_spec_t(0)
        self.client.load_backup_flag_tbl_set_default_action_load_backup_flag(\
                self.sess_hdl, self.dev_tgt, actnspec0)

        self.conn_mgr.complete_operations(self.sess_hdl)

        print "Filtering data"
        count = 0
        buf = bytearray()
        pipeidx = None
        r = redis.Redis(host=redis_ip, port=redis_port, decode_responses=True)
        for hashidx_key in r.scan_iter("{}*".format(hashidx_prefix):
            hashidx = int(hashidx_key[8:]) # Get string after "hashidx:"
            if pipeidx is None:
                latests = [] # Two pipelines
                latests.append(latest_list[hashidx])
                latests.append(latest_list[hashidx + bucket_count])
                if latests[0] == 0 and lastest[1] == 0: # Empty entry
                    continue
                else: # Set pipeidx
                    if lastest[0] != 0:
                        pipeidx = 0
                    else:
                        pipeidx = 1
            lastest = latests[pipeidx]
            keylo, keyhi, thread_id = r.get(hashidx_key).split(" ")
            if latest == 1:
                vallen = vallen_list[hashidx + pipeidx*bucket_count]
                buf = buf + struct.pack("=BH2QB", latest, hashidx, keylo, keyhi, vallen)
                for i in range(vallen):
                    validx = vallen - i
                    # Ptf converts small-endian int32 to big-endian int32
                    tmp_vallo = vallo_list_list[validx][hashidx + pipeidx*bucket_count]
                    tmp_valhi = valhi_list_list[validx][hashidx + pipeidx*bucket_count]
                    # Convert big-endian int32 to small-endian uint32
                    tmp_vallo = RegisterUpdate.get_reg32(tmp_vallo)
                    tmp_valhi = RegisterUpdate.get_reg32(tmp_valhi)
                    tmpval = ((tmp_valhi << 32) | tmp_vallo)
                    buf = buf + struct.pack("Q", tmpval)
                count += 1
            else if latest == 2:
                buf = buf + struct.pack("=BH2Q", latest, hashidx, keylo, keyhi)
                count += 1
            else:
                print("[ERROR] Invalid latest: {}".format(latest))
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
