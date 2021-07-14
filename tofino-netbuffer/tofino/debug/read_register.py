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

this_dir = os.path.dirname(os.path.abspath(__file__))

# Front Panel Ports
#   List of front panel ports to use. Each front panel port has 4 channels.
#   Port 1 is broken to 1/0, 1/1, 1/2, 1/3. Test uses 2 ports.
#
#   ex: ["1/0", "1/1"]
#


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
        flags = netbuffer_register_flags_t(read_hw_sync=True)
        keylo = self.client.register_read_keylo_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        keyhi = self.client.register_read_keyhi_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        vallo = self.client.register_read_vallo_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        valhi = self.client.register_read_valhi_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        valid = self.client.register_read_valid_reg(self.sess_hdl, self.dev_tgt, 0, flags)

        self.conn_mgr.complete_operations(self.sess_hdl)

        if keylo[1] < 0:
            keylo[1] += 2**32
        print "keylo: {}".format(hex(keylo[1]))
        if keyhi[1] < 0:
            keyhi[1] += 2**32
        print "keyhi: {}".format(hex(keyhi[1]))
        if vallo[1] < 0:
            vallo[1] += 2**32
        print "vallo: {}".format(hex(vallo[1]))
        if valhi[1] < 0:
            valhi[1] += 2**32
        print "valhi: {}".format(hex(valhi[1]))
        print "valid: {}".format(valid[1])
