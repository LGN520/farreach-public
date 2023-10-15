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
        print "Get key and freeidx from paramserver"
        ptf_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sendbuf = struct.pack("=i", SWITCHOS_GET_KEY_FREEIDX) # 4-byte int
        ptf_sock.sendto(sendbuf, ("127.0.0.1", switchos_paramserver_port))
        recvbuf, switchos_paramserver_addr = ptf_sock.recvfrom(1024)
        # TODO: Check correctness of key
        ##keylolo, keylohi, keyhilo, keyhihi, freeidx = struct.unpack("!4I=H", recvbuf)
        #keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx = struct.unpack("!3I2H=H", recvbuf)
        keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, remainbuf = struct.unpack("!3I2H{}s".format(len(recvbuf)-16), recvbuf)
        freeidx = struct.unpack("=H", remainbuf)[0]

        print "Add {}, {} into cache_lookup_tbl".format(recvbuf[0:16], freeidx)
        matchspec0 = netbufferv4_cache_lookup_tbl_match_spec_t(\
                op_hdr_keylolo = convert_u32_to_i32(keylolo),
                op_hdr_keylohi = convert_u32_to_i32(keylohi),
                op_hdr_keyhilo = convert_u32_to_i32(keyhilo),
                #op_hdr_keyhihi = convert_u32_to_i32(keyhihi),
                op_hdr_keyhihilo = convert_u16_to_i16(keyhihilo),
                op_hdr_keyhihihi = convert_u16_to_i16(keyhihihi),
                meta_need_recirculate = 0)
        actnspec0 = netbufferv4_cached_action_action_spec_t(freeidx)
        self.client.cache_lookup_tbl_table_add_with_cached_action(\
                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

        # TODO: check API of register set
        print "Set validvalue_reg as 1"
        index = freeidx
        value = 1
        self.client.register_write_validvalue_reg(self.sess_hdl, self.dev_tgt, index, value)

        self.conn_mgr.complete_operations(self.sess_hdl)
        self.conn_mgr.client_cleanup(self.sess_hdl) # close session
