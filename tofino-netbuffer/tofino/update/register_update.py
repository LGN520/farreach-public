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

from C1.p4_pd_rpc.ttypes import *
from pltfm_pm_rpc.ttypes import *
from pal_rpc.ttypes import *
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from res_pd_rpc.ttypes import *

this_dir = os.path.dirname(os.path.abspath(__file__))

# Front Panel Ports
#   List of front panel ports to use. Each front panel port has 4 channels.
#   Port 1 is broken to 1/0, 1/1, 1/2, 1/3. Test uses 2 ports.
#
#   ex: ["1/0", "1/1"]
#

import socket
import struct

ptf_port = 3333

bf_cnt = 0
bf_idxes = []

def extract_bloomfilter(msg):
    global bf_cnt, bf_idxes
    msglen = len(msg)
    bf_cnt, remain = struct.unpack("I{}s".format(msglen-4), msg)
    for i in range(bf_cnt):
        bf_idx, remain = struct.unpack("I{}s".format(msglen-4), msg)
        bf_idxes.append(bf_idx)
    return remain


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

    def runTest(self):
        sockfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        sockfd.bind(("", ptf_port))
        msg = sockfd.recvfrom(1024)
        msg = extract_bloomfilter(msg)

        global bf_cnt, bf_idxes
        res = self.client.register_reset_all_bloomfilter_reg(self.sess_hdl, self.dev_tgt)
        for i in bf_cnt:
            res = self.client.register_write_bloomfilter_reg(self.sess_hdl, self.dev_tgt, bf_idxes[i], 1)

        self.conn_mgr.complete_operations(self.sess_hdl)
