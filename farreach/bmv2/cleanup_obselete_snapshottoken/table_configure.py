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
from p4utils.utils.sswitch_thrift_API import SimpleSwitchThriftAPI

import random
import sys
import time
import unittest


this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *

controller = SimpleSwitchThriftAPI(9090, "192.168.122.229")  


class RegisterUpdate():
    # def __init__(self):
    #     # initialize the thrift data plane
    #     pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbufferv4"])

    def setUp(self):
        print('\nSetup')


        self.unmatched_devports = []
        self.recirports_for_unmatched_devports = []
        for client_physical_idx in range(client_physical_num):
            if client_pipeidxes[client_physical_idx] != single_ingress_pipeidx:
                # get devport fro front panel port
                port, chnl = client_fpports[client_physical_idx].split("/")
                # tmp_devport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
                self.unmatched_devports.append(port)
                recirport = pipeline_recirports_tosingle[client_pipeidxes[client_physical_idx]]
                if recirport is None:
                    print("[ERROR] pipeline_recirports_tosingle[{}] is None!").format(client_pipeidxes[client_physical_idx])
                    exit(-1)
                port, chnl = recirport.split("/")
                # recirdevport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
                self.recirports_for_unmatched_devports.append(recirdevport)
        # GETRES_LATEST/DELETED_SEQ may also incur CASE1s (need to read snapshot flag)
        for server_physical_idx in range(server_physical_num):
            if server_pipeidxes[server_physical_idx] != single_ingress_pipeidx:
                # get devport fro front panel port
                port, chnl = server_fpports[server_physical_idx].split("/")
                # tmp_devport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
                self.unmatched_devports.append(port)
                recirport = pipeline_recirports_tosingle[server_pipeidxes[server_physical_idx]]
                if recirport is None:
                    print("[ERROR] pipeline_recirports_tosingle[{}] is None!").format(server_pipeidxes[server_physical_idx])
                    exit(-1)
                port, chnl = recirport.split("/")
                # recirdevport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
                self.recirports_for_unmatched_devports.append(recirport)

    def cleanup(self):
        print("Reset need_recirculate=0 for iports in different ingress pipelines")
        # get entry count
        entrynum = controller.table_num_entries("need_recirculate_tbl")
        if entrynum > 0:
            for i in range(len(self.unmatched_devports)):
                iport = self.unmatched_devports[i]
                for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_LARGEVALUE]:
                    matchspec0 = [hex(tmpoptype),iport]
                    controller.table_delete_match('need_recirculate_tbl', matchspec0)
        print("Reset snapshot_flag=0 for all ingress pipelines")
        entrynum = controller.table_num_entries("snapshot_flag_tbl")
        # entrynum = self.client.snapshot_flag_tbl_get_entry_count(self.sess_hdl, self.dev_tgt)
        if entrynum > 0:
            for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_LARGEVALUE]:
                matchspec0 =[hex(tmpoptype),hex(0)]
                controller.table_delete_match('snapshot_flag_tbl', matchspec0)
        print("Reset case1_reg for all pipelines")
        controller.register_reset("case1_reg")
    def runTest(self):
        self.cleanup()
registerupdate = RegisterUpdate()
registerupdate.setUp()
registerupdate.runTest()