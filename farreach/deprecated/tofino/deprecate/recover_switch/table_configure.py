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

from netbufferv4.p4_pd_rpc.ttypes import *
from pltfm_pm_rpc.ttypes import *
from pal_rpc.ttypes import *
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from res_pd_rpc.ttypes import *

import socket
import struct

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *

class RegisterUpdate(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbufferv4"])

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

    def runTest(self):
        #flags = netbufferv4_register_flags_t(read_hw_sync=True)

        snapshotid_path = "/tmp/farreach/controller.snapshotid"
        with open(snapshotid_path, "rb") as f:
            buf = f.read(4)
            snapshotid = struct.unpack("=i", buf)[0]

        # extract snapshot data
        snapshotdata_path = "/tmp/farreach/controller.snapshotdata{}".format(snapshotid)
        key_list_list = [] # n*5
        vallen_list = [] # n
        vallo_list_list = [] # n*16
        valhi_list_list = [] # n*16
        seq_list = [] # n
        deleted_list = [] # n
        with open(snapshotdata_path, "rb") as f:
            buf = f.read(8)
            control_type, total_bytes = struct.unpack("=2i", buf)
            if control_type != SNAPSHOT_GETDATA_ACK:
                print "Invalid control type: {}".format(control_type)
                exit(-1)
            buf = f.read(total_bytes - 8)
            remainbytes = total_bytes - 8
            while True:
                perserver_bytes, serveridx, recordcnt, specialcase_bwcost, buf = struct.unpack("=iHiQ{}s".format(remainbytes-18), buf)
                remainbytes -= 18
                for _ in range(recordcnt):
                    keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, buf = struct.unpack("!3I2H{}s".format(remainbytes-16), buf)
                    key_list_list.append([keylolo, keylohi, keyhilo, keyhihilo, keyhihihi])
                    remainbytes -= 16

                    vallen, buf = struct.unpack("!H{}s".format(remainbytes-2), buf)
                    vallen_list.append(vallen)
                    remainbytes -= 2

                    valbyteslen = (vallen+7)/8*8
                    valbytes, seq, stat, buf = struct.unpack("={}sI?{}s".format(valbyteslen, remainbytes-valbyteslen-4-1), buf)
                    tmp_val_list = struct.unpack("!{}I".format(valbyteslen/8*2)) # 16*2
                    tmp_vallo_list = []
                    tmp_valhi_list = []
                    for i in range(len(tmp_val_list)/2): # 0-15
                        j = len(tmp_val_list) - 1 - 2*i # 31, 29, ..., 3, 1
                        tmp_vallo_list.append(tmp_val_list[j-1]) # 30, 28, ..., 2, 0
                        tmp_valhi_list.append(tmp_val_list[j])
                    vallo_list_list.append(tmp_vallo_list)
                    valhi_list_list.append(tmp_valhi_list)
                    seq_list.append(seq)
                    deleted_list.append(!stat)
                    remainbytes -= (valbyteslen + 4 + 1)
                if remainbytes <= 0:
                    break

        # set data plane
        n = len(key_list_list)
        for idx in range(n):
            keylolo, keylohi, keyhilo, keyhihilo, keyhihihi = key_list_list[idx]
            matchspec0 = netbufferv4_cache_lookup_tbl_match_spec_t(\
                    op_hdr_keylolo = convert_u32_to_i32(keylolo),
                    op_hdr_keylohi = convert_u32_to_i32(keylohi),
                    op_hdr_keyhilo = convert_u32_to_i32(keyhilo),
                    #op_hdr_keyhihi = convert_u32_to_i32(keyhihi),
                    op_hdr_keyhihilo = convert_u16_to_i16(keyhihilo),
                    op_hdr_keyhihihi = convert_u16_to_i16(keyhihihi),
                    meta_need_recirculate = 0)
            actnspec0 = netbufferv4_cached_action_action_spec_t(idx)
            self.client.cache_lookup_tbl_table_add_with_cached_action(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            self.client.register_write_vallen_reg(self.sess_hdl, self.dev_tgt, idx, vallen_list[idx])

            for i in range(len(vallo_list_list[idx])): # 0-15
                eval("self.client.register_write_vallo{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, idx, vallo_list_list[idx][i])
                eval("self.client.register_write_valhi{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, idx, valhi_list_list[idx][i])

            self.client.register_write_savedseq_reg(self.sess_hdl, self.dev_tgt, idx, seq_list[idx])
            self.client.register_write_deleted_reg(self.sess_hdl, self.dev_tgt, idx, deleted_list[idx])

        self.conn_mgr.complete_operations(self.sess_hdl)
        self.conn_mgr.client_cleanup(self.sess_hdl) # close session
