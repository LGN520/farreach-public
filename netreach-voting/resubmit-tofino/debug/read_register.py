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
        keylololo = self.client.register_read_keylololo_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        keylolohi = self.client.register_read_keylolohi_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        keylohilo = self.client.register_read_keylohilo_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        keylohihi = self.client.register_read_keylohihi_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        keyhilolo = self.client.register_read_keyhilolo_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        keyhilohi = self.client.register_read_keyhilohi_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        keyhihilo = self.client.register_read_keyhihilo_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        keyhihihi = self.client.register_read_keyhihihi_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        vallo1 = self.client.register_read_vallo1_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        valhi1 = self.client.register_read_valhi1_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        vallen = self.client.register_read_vallen_reg(self.sess_hdl, self.dev_tgt, 0, flags)
        valid = self.client.register_read_valid_reg(self.sess_hdl, self.dev_tgt, 0, flags)

        self.conn_mgr.complete_operations(self.sess_hdl)

        if keylololo[1] < 0:
            keylololo[1] += 2**16
        print "keylololo: {}".format(hex(keylololo[1]))
        if keylolohi[1] < 0:
            keylolohi[1] += 2**16
        print "keylolohi: {}".format(hex(keylolohi[1]))
        if keylohilo[1] < 0:
            keylohilo[1] += 2**16
        print "keylohilo: {}".format(hex(keylohilo[1]))
        if keylohihi[1] < 0:
            keylohihi[1] += 2**16
        print "keylohihi: {}".format(hex(keylohihi[1]))
        if keyhilolo[1] < 0:
            keyhilolo[1] += 2**16
        print "keyhilolo: {}".format(hex(keyhilolo[1]))
        if keyhilohi[1] < 0:
            keyhilohi[1] += 2**16
        print "keyhilohi: {}".format(hex(keyhilohi[1]))
        if keyhihilo[1] < 0:
            keyhihilo[1] += 2**16
        print "keyhihilo: {}".format(hex(keyhihilo[1]))
        if keyhihihi[1] < 0:
            keyhihihi[1] += 2**16
        print "keyhihihi: {}".format(hex(keyhihihi[1]))
        if vallo1[1] < 0:
            vallo1[1] += 2**32
        print "vallo1: {}".format(hex(vallo1[1]))
        if valhi1[1] < 0:
            valhi1[1] += 2**32
        print "valhi1: {}".format(hex(valhi1[1]))
        print "vallen: {}".format(vallen[1])
        print "valid: {}".format(valid[1])
