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

import ConfigParser
config = ConfigParser.ConfigParser()
with open(os.path.join(os.path.dirname(os.path.dirname(this_dir)), "config.ini"), "r") as f:
    config.readfp(f)

ingress_pipeidx = int(config.get("hardware", "ingress_pipeidx"))
egress_pipeidx = int(config.get("hardware", "egress_pipeidx"))

fp_ports = []
src_fpport = str(config.get("switch", "src_fpport"))
fp_ports.append(src_fpport)
dst_fpport = str(config.get("switch", "dst_fpport"))
fp_ports.append(dst_fpport)

port_pipeidx_map = {} # mapping between port and pipeline
pipeidx_ports_map = {} # mapping between pipeline and ports

PUTREQ = 0x01
DELREQ = 0x02
GETRES_LATEST_SEQ = 0x0b
GETRES_DELETED_SEQ = 0x0e

# Front Panel Ports
#   List of front panel ports to use. Each front panel port has 4 channels.
#   Port 1 is broken to 1/0, 1/1, 1/2, 1/3. Test uses 2 ports.
#
#   ex: ["1/0", "1/1"]
#

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

        # get the device ports from front panel ports
        for fpPort in fp_ports:
            port, chnl = fpPort.split("/")
            devPort = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
            self.devPorts.append(devPort)

        port_pipeidx_map[self.devPorts[0]] = ingress_pipeidx
        port_pipeidx_map[self.devPorts[1]] = egress_pipeidx
        pipeidx_ports_map[ingress_pipeidx] = [self.devPorts[0]]
        if egress_pipeidx not in pipeidx_ports_map:
            pipeidx_ports_map[egress_pipeidx] = [self.devPorts[1]]
        else:
            if self.devPorts[1] not in pipeidx_ports_map[egress_pipeidx]:
                pipeidx_ports_map[egress_pipeidx].append(self.devPorts[1])

    def runTest(self):
        print "Set need_recirculate=1 for iports in different ingress pipelines"
        for tmppipeidx in pipeidx_ports_map.keys():
            if tmppipeidx != ingress_pipeidx:
                tmpports = pipeidx_ports_map[tmppipeidx]
                for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
                    for iport in tmpports:
                        matchspec0 = netbufferv4_need_recirculate_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                ig_intr_md_ingress_port = iport)
                        self.client.need_recirculate_tbl_table_add_with_set_need_recirculate(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

        print "Set snapshot_flag=1 for all ingress pipelines"
        for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
            matchspec0 = netbufferv4_snapshot_flag_tbl_match_spec_t(\
                    op_hdr_optype = tmpoptype,
                    meta_need_recirculate = 0)
            #self.client.snapshot_flag_tbl_table_delete_by_match_spec(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.snapshot_flag_tbl_table_add_with_set_snapshot_flag(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

        # TODO: check API for delete_all_entries or modify_by_match_spec 
        print "Reset need_recirculate=0 for iports in different ingress pipelines"
        for tmppipeidx in pipeidx_ports_map.keys():
            if tmppipeidx != ingress_pipeidx:
                tmpports = pipeidx_ports_map[tmppipeidx]
                for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
                    for iport in tmpports:
                        matchspec0 = netbufferv4_need_recirculate_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                ig_intr_md_ingress_port = iport)
                        self.client.need_recirculate_tbl_table_delete_by_match_spec(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

        self.conn_mgr.complete_operations(self.sess_hdl)
        self.conn_mgr.client_cleanup(self.sess_hdl) # close session
