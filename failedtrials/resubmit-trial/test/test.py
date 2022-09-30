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

"""
Thrift PD interface basic tests
"""

from collections import OrderedDict

import time
import sys
import logging

import unittest
import random

import pd_base_tests

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from pltfm_pm_rpc.ttypes import *
from mirror_pd_rpc.ttypes import *
from pal_rpc.ttypes import *

from resubmit.p4_pd_rpc.ttypes import *
from res_pd_rpc.ttypes import *

client_mac="9c:69:b4:60:ef:a5"
server_mac="9c:69:b4:60:ef:8d"

fp_ports = ["2/0", "3/0"]

dev_id=0
class ResubmitTest(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["resubmit"])

    def setUp(self):
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.sess_hdl = self.conn_mgr.client_init()
        self.dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        self.devPorts = []

        self.platform_type = "mavericks"
        board_type = self.pltfm_pm.pltfm_pm_board_type_get()
        if re.search("0x0234|0x1234|0x4234|0x5234", hex(board_type)):
            self.platform_type = "mavericks"
        elif re.search("0x2234|0x3234", hex(board_type)):
            self.platform_type = "montara"

        # get the device ports from front panel ports
        for fpPort in fp_ports:
            port, chnl = fpPort.split("/")
            devPort = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
            self.devPorts.append(devPort)

    """ Basic test """
    def runTest(self):

        # add and enable the platform ports
        for i in self.devPorts:
           self.pal.pal_port_add(0, i,
                                 pal_port_speed_t.BF_SPEED_40G,
                                 pal_fec_type_t.BF_FEC_TYP_NONE)
           self.pal.pal_port_enable(0, i)

        # Test
        ig_port = self.devPorts[0]

        # Resubmit with no fields
        dstAddr = macAddr_to_string(server_mac)
        #eg_port = 1
        eg_port = self.devPorts[1]

        resub_hdls = []
        nhop_hdls = []

        # First pass match resubmit table
        print "Configuring tables"
        #match_on = resubmit_l2_resubmit_match_spec_t(dstAddr)
        #h = self.client.l2_resubmit_table_add_with_do_resubmit(self.sess_hdl, self.dev_tgt, match_on)
        #resub_hdls.append(h)

        # Next pass send to egress
        #match_on = resubmit_l2_nhop_match_spec_t(dstAddr)
        #egr_action = resubmit_nhop_set_action_spec_t(eg_port)
        #h = self.client.l2_nhop_table_add_with_nhop_set(self.sess_hdl, self.dev_tgt, match_on, egr_action)
        #nhop_hdls.append(h)
        #self.conn_mgr.complete_operations(self.sess_hdl)

        # Resubmit with fields
        match_on = resubmit_l2_resubmit_match_spec_t(dstAddr)
        h = self.client.l2_resubmit_table_add_with_do_resubmit_with_fields(self.sess_hdl, self.dev_tgt, match_on)
        resub_hdls.append(h)

        match_on = resubmit_l2_nhop_match_spec_t(dstAddr)
        egr_action = resubmit_nhop_set_action_spec_t(eg_port)
        h = self.client.l2_nhop_table_add_with_nhop_set_with_type(self.sess_hdl, self.dev_tgt, match_on, egr_action)
        nhop_hdls.append(h)
        self.conn_mgr.complete_operations(self.sess_hdl)
