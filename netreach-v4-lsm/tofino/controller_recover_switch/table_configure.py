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
        #flags = netbufferv4_register_flags_t(read_hw_sync=True)

        snapshotid_path = "/tmp/netreach-v4-lsm/controller.snapshotid"
        with open(snapshotid_path, "rb") as f:
            buf = f.read(4)
            snapshotid = struct.unpack("=i", buf)[0]

        snapshotdata_path = "/tmp/netreach-v4-lsm/controller.snapshotdata{}".format(snapshotid)

        # TODO: extract snapshot data and set data plane

        print "Get cached_empty_index_backup from paramserver"
        ptf_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sendbuf = struct.pack("=i", SWITCHOS_GET_CACHEDEMPTYINDEX) # 4-byte int
        ptf_sock.sendto(sendbuf, ("127.0.0.1", switchos_paramserver_port))
        recvbuf, switchos_paramserver_addr = ptf_sock.recvfrom(1024)
        cached_empty_index_backup = struct.unpack("=I", recvbuf)[0] # must > 0
        if cached_empty_index_backup <= 0 or cached_empty_index_backup > kv_bucket_num:
            print "Invalid cached_empty_index_backup: {}".format(cached_empty_index_backup)
            exit(-1)

        start_index = 0
        end_index = cached_empty_index_backup - 1
        print "Load snapshot data in [{}, {}] from data plane".format(start_index, end_index)
        record_cnt = end_index - start_index + 1
        # TODO: check len of vallen_list
        vallen_list = self.client.register_range_read_vallen_reg(self.sess_hdl, self.dev_tgt, start_index, record_cnt, flags)[egress_pipeidx * record_cnt:egress_pipeidx * record_cnt + record_cnt]
        for i in range(len(vallen_list)):
            #vallen_list[i] = convert_i32_to_u32(vallen_list[i])
            vallen_list[i] = convert_i16_to_u16(vallen_list[i])
        vallo_list_list = []
        valhi_list_list = []
        for i in range(switch_max_vallen/8): # 128 bytes / 8 = 16 register arrays
            vallo_list_list.append(eval("self.client.register_range_read_vallo{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, 0, record_cnt, flags)[egress_pipeidx * record_cnt:egress_pipeidx * record_cnt + record_cnt])
            valhi_list_list.append(eval("self.client.register_range_read_valhi{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, 0, record_cnt, flags)[egress_pipeidx * record_cnt:egress_pipeidx * record_cnt + record_cnt])
        deleted_list = self.client.register_range_read_deleted_reg(self.sess_hdl, self.dev_tgt, start_index, record_cnt, flags)[egress_pipeidx * record_cnt:egress_pipeidx * record_cnt + record_cnt]
        stat_list = []
        for i in range(len(deleted_list)):
            if deleted_list[i] == 0:
                stat_list.append(True)
            elif deleted_list[i] == 1:
                stat_list.append(False)
            else:
                print "Invalid deleted_list[{}]: {}".format(i, deleted_list[i])
                exit(-1)
        savedseq_list = self.client.register_range_read_savedseq_reg(self.sess_hdl, self.dev_tgt, start_index, record_cnt, flags)[egress_pipeidx * record_cnt:egress_pipeidx * record_cnt + record_cnt]
        for i in range(len(savedseq_list)):
            savedseq_list[i] = convert_i32_to_u32(savedseq_list[i])

        print "Prepare sendbuf to switchos.snapshotdataserver"
        sendbuf = bytes()
        ## <total_bytesnum, records> -> for each record: <uint32_t vallen (big-endian for Val::deserialize), valbytes (same order), uint32_t seq (little-endian), result>
        # <total_bytesnum, records> -> for each record: <uint16_t vallen (big-endian for Val::deserialize), valbytes (same order), uint32_t seq (little-endian), result>
        for i in range(record_cnt):
            tmpvallen = vallen_list[i]
            #sendbuf = sendbuf + struct.pack("!I", tmpvallen)
            sendbuf = sendbuf + struct.pack("!H", tmpvallen)
            tmp_eightbyte_cnt = (tmpvallen + 7) / 8
            for j in range(tmp_eightbyte_cnt):
                # NOTE: we serialize each 4B value as big-endian to keep the same byte order as end-hosts
                # NOTE: deparser valbytes from val16 to val1
                sendbuf = sendbuf + struct.pack("!2i", vallo_list_list[tmp_eightbyte_cnt-1-j][i], valhi_list_list[tmp_eightbyte_cnt-1-j][i])
            sendbuf = sendbuf + struct.pack("=I?", savedseq_list[i], stat_list[i])
        total_bytesnum = 4 + len(sendbuf) # total # of bytes in sendbuf including total_bytesnum itself
        sendbuf = struct.pack("=i", total_bytesnum) + sendbuf

        print "Send sendbuf w/ {} bytes to switchos.snapshotdataserver".format(total_bytesnum)
        ptf_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        ptf_sock.connect(("127.0.0.1", switchos_snapshotdataserver_port))
        ptf_sock.sendall(sendbuf)

        self.conn_mgr.complete_operations(self.sess_hdl)
        self.conn_mgr.client_cleanup(self.sess_hdl) # close session
