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
sys.path.append(os.path.dirname(this_dir))
from common import *

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

    def runTest(self):
        flags = netbufferv4_register_flags_t(read_hw_sync=True)

        print "Reset snapshot_flag=0 for all ingress pipelines"
        for tmpoptype in [PUTREQ, DELREQ]:
            matchspec0 = netbufferv4_snapshot_flag_tbl_match_spec_t(\
                    op_hdr_optype = tmpoptype,
                    meta_need_recirculate = 0)
            self.client.snapshot_flag_tbl_table_delete_by_match_spec(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

        #print "Get cached_empty_index_backup from paramserver"
        #ptf_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        #sendbuf = struct.pack("=i", SWITCHOS_GET_CACHEDEMPTYINDEX) # 4-byte int
        #ptf_sockfd.sendto(sendbuf, ("127.0.0.1", switchos_paramserver_port))
        #recvbuf, switchos_paramserver_addr = ptf_sock.recvfrom(1024)
        #cached_empty_index_backup = struct.unpack("=I", recvbuf)[0] # must > 0
        #if cached_empty_index_backup <= 0 or cached_empty_index_backup > kv_bucket_num:
        #    print "Invalid cached_empty_index_backup: {}".format(cached_empty_index_backup)
        #    exit(-1)

        print "Reset case1_reg"
        self.client.register_reset_all_case1_reg(self.sess_hdl, self.dev_tgt)

        self.conn_mgr.complete_operations(self.sess_hdl)
        self.conn_mgr.client_cleanup(self.sess_hdl) # close session
