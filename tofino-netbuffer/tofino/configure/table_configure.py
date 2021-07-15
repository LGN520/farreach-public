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
fp_ports = ["1/0", "3/0"]
src_ip = "10.0.0.31"
dst_ip = "10.0.0.32"

GETREQ_TYPE = 0x00000000
PUTREQ_TYPE = 0x01000000
DELREQ_TYPE = 0x02000000
SCANREQ_TYPE = 0x03000000
GETRES_TYPE = 0x04000000
PUTRES_TYPE = 0x05000000
DELRES_TYPE = 0x06000000
SCANRES_TYPE = 0x07000000

class TableConfigure(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbuffer"])

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

            # Table: ipv4_lpm
            print "Configuring ipv4_lpm"
            ipv4addr0 = ipv4Addr_to_i32(src_ip)
            matchspec0 = netbuffer_ipv4_lpm_match_spec_t(ipv4_hdr_dstAddr=ipv4addr0, ipv4_hdr_dstAddr_prefix_length=32)
            actnspec0 = netbuffer_ipv4_forward_action_spec_t(self.devPorts[0])
            ipv4addr1 = ipv4Addr_to_i32(dst_ip)
            matchspec1 = netbuffer_ipv4_lpm_match_spec_t(ipv4_hdr_dstAddr=ipv4addr1, ipv4_hdr_dstAddr_prefix_length=32)
            actnspec1 = netbuffer_ipv4_forward_action_spec_t(self.devPorts[1])
            self.client.ipv4_lpm_table_add_with_ipv4_forward(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            self.client.ipv4_lpm_table_add_with_ipv4_forward(\
                    self.sess_hdl, self.dev_tgt, matchspec1, actnspec1)

            # Table: match_keylo_tbl
            print "Configuring match_keylo_tbl"
            matchspec0 = netbuffer_match_keylo_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            matchspec1 = netbuffer_match_keylo_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            self.client.match_keylo_tbl_table_add_with_get_match_keylo(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.match_keylo_tbl_table_add_with_put_match_keylo(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            # Table: match_keyhi_tbl
            print "Configuring match_keyhi_tbl"
            matchspec0 = netbuffer_match_keyhi_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            matchspec1 = netbuffer_match_keyhi_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            self.client.match_keyhi_tbl_table_add_with_get_match_keyhi(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.match_keyhi_tbl_table_add_with_put_match_keyhi(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            # Table: access_valid_tbl
            print "Configuring access_valid_tbl"
            matchspec0 = netbuffer_access_valid_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            matchspec1 = netbuffer_access_valid_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            self.client.access_valid_tbl_table_add_with_get_valid(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.access_valid_tbl_table_add_with_set_valid(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            # Table: clone_pkt_tbl
            print "Configuring clone_pkt_tbl"
            matchspec0 = netbuffer_clone_pkt_tbl_match_spec_t(ig_intr_md_ingress_port=self.devPorts[0])
            actnspec0 = netbuffer_clone_pkt_action_spec_t(self.devPorts[1])
            self.client.clone_pkt_tbl_table_add_with_clone_pkt(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            self.conn_mgr.complete_operations(self.sess_hdl)

    def tearDown(self):
        if test_param_get('cleanup') == True:

            print "\nCleaning up"

            # delete the programmed forward table entry
            self.cleanup_table("ipv4_lpm", True)
            # delete the platform ports
            self.conn_mgr.client_cleanup(self.sess_hdl)
            for i in self.devPorts:
                self.pal.pal_port_del(0, i)
            self.pal.pal_port_del_all(0)

    def cleanup_table(self, table, iscalled=False):
        if iscalled:
            table = 'self.client.' + table
            # get entry count
            num_entries = eval(table + '_get_entry_count')\
                          (self.sess_hdl, self.dev_tgt)
            print "Number of entries : {}".format(num_entries)
            if num_entries == 0:
                return
            # get the entry handles
            hdl = eval(table + '_get_first_entry_handle')\
                    (self.sess_hdl, self.dev_tgt)
            if num_entries > 1:
                hdls = eval(table + '_get_next_entry_handles')\
                    (self.sess_hdl, self.dev_tgt, hdl, num_entries - 1)
                hdls.insert(0, hdl)
            else:
                hdls = [hdl]
            # delete the table entries
            for hdl in hdls:
                entry = eval(table + '_get_entry')\
                    (self.sess_hdl, self.dev_tgt.dev_id, hdl, True)
                eval(table + '_table_delete_by_match_spec')\
                    (self.sess_hdl, self.dev_tgt, entry.match_spec)
