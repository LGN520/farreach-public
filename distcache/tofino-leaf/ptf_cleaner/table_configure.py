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

from distcacheleaf.p4_pd_rpc.ttypes import *
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

flags = distcacheleaf_register_flags_t(read_hw_sync=True)

class RegisterUpdate(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["distcacheleaf"])

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
        print "[ptf.cleaner] ready"

        # Change clean period based on packet sending rate
        #first_period = 1
        #clean_period = 5
        #is_first = True
        clean_period = 1 # for threshold = 50
        #clean_period = 0.2 # for threshold = 10
        while True:
            #if is_first:
            #    time.sleep(first_period)
            #    is_first = False
            #else:
            #    time.sleep(clean_period)
            time.sleep(clean_period)

            #print "Start to reset all cm regs"
            self.client.register_reset_all_cm1_reg(self.sess_hdl, self.dev_tgt)
            self.client.register_reset_all_cm2_reg(self.sess_hdl, self.dev_tgt)
            self.client.register_reset_all_cm3_reg(self.sess_hdl, self.dev_tgt)
            self.client.register_reset_all_cm4_reg(self.sess_hdl, self.dev_tgt)

            #print "Start to reset all cache frequency reg"
            self.client.register_reset_all_cache_frequency_reg(self.sess_hdl, self.dev_tgt)

            #print "Start to reset all bf regs"
            self.client.register_reset_all_bf1_reg(self.sess_hdl, self.dev_tgt)
            self.client.register_reset_all_bf2_reg(self.sess_hdl, self.dev_tgt)
            self.client.register_reset_all_bf3_reg(self.sess_hdl, self.dev_tgt)

            self.conn_mgr.complete_operations(self.sess_hdl)
            #print "Finish to reset all cm regs"
        self.conn_mgr.client_cleanup(self.sess_hdl) # close session
