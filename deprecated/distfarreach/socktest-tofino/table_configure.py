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

from socktest.p4_pd_rpc.ttypes import *
from pltfm_pm_rpc.ttypes import *
from pal_rpc.ttypes import *
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from res_pd_rpc.ttypes import *

import ConfigParser

this_dir = os.path.dirname(os.path.abspath(__file__))

config = ConfigParser.ConfigParser()
with open(os.path.join(os.path.dirname(this_dir), "config.ini"), "r") as f:
    config.readfp(f)

# Front Panel Ports
#   List of front panel ports to use. Each front panel port has 4 channels.
#   Port 1 is broken to 1/0, 1/1, 1/2, 1/3. Test uses 2 ports.
#
#   ex: ["1/0", "1/1"]
#
fp_ports = []
src_fpport = str(config.get("hardware", "src_fpport"))
fp_ports.append(src_fpport)
dst_fpport = str(config.get("hardware", "dst_fpport"))
fp_ports.append(dst_fpport)

client_mac = str(config.get("client", "client_mac"))
server_mac = str(config.get("server", "server_mac"))

class TableConfigure(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["socktest"])

    def setUp(self):
        print '\nSetup'

        # initialize the connection
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

    def runTest(self):
        if test_param_get('cleanup') != True:
            print '\nTest'

            # add and enable the platform ports
            for i in self.devPorts:
               self.pal.pal_port_add(0, i,
                                     pal_port_speed_t.BF_SPEED_40G,
                                     pal_fec_type_t.BF_FEC_TYP_NONE)
               self.pal.pal_port_enable(0, i)

            print "Configuring port_forward_tbl"
            matchspec0 = socktest_port_forward_tbl_match_spec_t(\
                    ig_intr_md_ingress_port = self.devPorts[0],\
                    payload_hdr_payloadvalue = 0)
            actnspec0 = socktest_port_forward_action_spec_t(self.devPorts[1])
            self.client.port_forward_tbl_table_add_with_port_forward(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            matchspec0 = socktest_port_forward_tbl_match_spec_t(\
                    ig_intr_md_ingress_port = self.devPorts[1],\
                    payload_hdr_payloadvalue = 0)
            actnspec0 = socktest_port_forward_action_spec_t(self.devPorts[0])
            self.client.port_forward_tbl_table_add_with_port_forward(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            matchspec0 = socktest_port_forward_tbl_match_spec_t(\
                    ig_intr_md_ingress_port = self.devPorts[0],\
                    payload_hdr_payloadvalue = 1)
            actnspec0 = socktest_sendback_action_spec_t(macAddr_to_string(client_mac), macAddr_to_string(server_mac))
            self.client.port_forward_tbl_table_add_with_sendback(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            self.conn_mgr.complete_operations(self.sess_hdl)
            self.conn_mgr.client_cleanup(self.sess_hdl) # close session
