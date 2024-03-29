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

import struct

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *

flags = netbufferv4_register_flags_t(read_hw_sync=True)

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

        self.unmatched_devports = []
        self.recirports_for_unmatched_devports = []
        for client_physical_idx in range(client_physical_num):
            if client_pipeidxes[client_physical_idx] != single_ingress_pipeidx:
                # get devport fro front panel port
                port, chnl = client_fpports[client_physical_idx].split("/")
                tmp_devport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
                self.unmatched_devports.append(tmp_devport)
                recirport = pipeline_recirports_tosingle[client_pipeidxes[client_physical_idx]]
                if recirport is None:
                    print "[ERROR] pipeline_recirports_tosingle[{}] is None!".format(client_pipeidxes[client_physical_idx])
                    exit(-1)
                port, chnl = recirport.split("/")
                recirdevport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
                self.recirports_for_unmatched_devports.append(recirdevport)
        # GETRES_LATEST/DELETED_SEQ may also incur CASE1s (need to read snapshot flag)
        for server_physical_idx in range(server_physical_num):
            if server_pipeidxes[server_physical_idx] != single_ingress_pipeidx:
                # get devport fro front panel port
                port, chnl = server_fpports[server_physical_idx].split("/")
                tmp_devport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
                self.unmatched_devports.append(tmp_devport)
                recirport = pipeline_recirports_tosingle[server_pipeidxes[server_physical_idx]]
                if recirport is None:
                    print "[ERROR] pipeline_recirports_tosingle[{}] is None!".format(server_pipeidxes[server_physical_idx])
                    exit(-1)
                port, chnl = recirport.split("/")
                recirdevport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
                self.recirports_for_unmatched_devports.append(recirdevport)

    def cleanup(self):
        print "Reset need_recirculate=0 for iports in different ingress pipelines"
        # get entry count
        entrynum = self.client.need_recirculate_tbl_get_entry_count(self.sess_hdl, self.dev_tgt)
        if entrynum > 0:
            for i in range(len(self.unmatched_devports)):
                iport = self.unmatched_devports[i]
                for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_LARGEVALUE]:
                    matchspec0 = netbufferv4_need_recirculate_tbl_match_spec_t(\
                            op_hdr_optype = tmpoptype,
                            ig_intr_md_ingress_port = iport)
                    self.client.need_recirculate_tbl_table_delete_by_match_spec(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
        print "Reset snapshot_flag=0 for all ingress pipelines"
        entrynum = self.client.snapshot_flag_tbl_get_entry_count(self.sess_hdl, self.dev_tgt)
        if entrynum > 0:
            for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_LARGEVALUE]:
                matchspec0 = netbufferv4_snapshot_flag_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_need_recirculate = 0)
                self.client.snapshot_flag_tbl_table_delete_by_match_spec(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
        print "Reset case1_reg for all pipelines"
        self.client.register_reset_all_case1_reg(self.sess_hdl, self.dev_tgt)

    def runTest(self):
        self.cleanup()

        self.conn_mgr.complete_operations(self.sess_hdl)
        self.conn_mgr.client_cleanup(self.sess_hdl) # close session
