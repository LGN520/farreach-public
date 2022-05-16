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

        # NOTE: cache must be full (i.e., all idxes are valid) when cache eviction
        print "Get sampled indexes for in-switch"
        random.seed(time.time())
        sampled_idxes = random.sample(range(0, kv_bucket_num), switchos_sample_cnt)

        print "Load frequency counters for sampled indexes"
        frequency_counters = []
        for i in range(len(sampled_idxes)):
            tmp_frequency_counter = convert_i32_to_u32(self.client.register_read_cache_frequency_reg(self.sess_hdl, self.dev_tgt, sampled_idxes[i], flags)[egress_pipeidx])
            frequency_counters.append(tmp_frequency_counter)

        print "Get evictidx by approximate LRF"
        min_frequency = frequency_counters[0]
        evictidx = 0
        for i in range(1, len(frequency_counters)):
            if min_frequency > frequency_counters[i]:
                min_frequency = frequency_counters[i]
                evictidx = i

        print "Set validvalue[{}] = 3 for atomicity".format(evictidx)
        index = evictidx
        value = 3
        self.client.register_write_validvalue_reg(self.sess_hdl, self.dev_tgt, index, value)

        print "Load evicted data"
        tmp_deleted = self.client.register_read_deleted_reg(self.sess_hdl, self.dev_tgt, evictidx, flags)[egress_pipeidx]
        if tmp_deleted == 0:
            evictstat = True
        elif tmp_deleted == 1:
            evictstat = False
        else:
            print "Invalid tmp_deleted: {}".format(tmp_deleted)
            exit(-1)
        #evictvallen = convert_i32_to_u32(self.client.register_read_vallen_reg(self.sess_hdl, self.dev_tgt, evictidx, flags)[egress_pipeidx])
        evictvallen = convert_i16_to_u16(self.client.register_read_vallen_reg(self.sess_hdl, self.dev_tgt, evictidx, flags)[egress_pipeidx])
        eightbyte_cnt = (evictvallen+7) / 8;
        val_list = []
        for i in range(1, eightbyte_cnt+1):
            # int32_t
            tmp_vallo = eval("self.client.register_read_vallo{}_reg".format(i))(self.sess_hdl, self.dev_tgt, evictidx, flags)[egress_pipeidx]
            tmp_valhi = eval("self.client.register_read_valhi{}_reg".format(i))(self.sess_hdl, self.dev_tgt, evictidx, flags)[egress_pipeidx]
            val_list.append(tmp_vallo)
            val_list.append(tmp_valhi)
        evictvalbytes = bytes()
        for i in range(val_list):
            # NOTE: we serialize each 4B value as big-endian to keep the same byte order as end-hosts
            evictvalbytes = evictvalbytes + struct.pack("!i", val_list[i])
        # load savedseq
        evictseq = convert_i32_to_u32(self.client.register_read_savedseq_reg(self.sess_hdl, self.dev_tgt, evictidx, flags)[egress_pipeidx])

        print "Set evictdata to paramserver"
        ptf_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        ## <int type, int16_t evictidx, uint32_t vallen (big-endian for Val::deserialize), valbytes (same order), int32_t savedseq (little-endian), bool status>
        #sendbuf = struct.pack("=iH!I={}cI?".format(len(evictvalbytes)), SWITCHOS_SET_EVICTDATA, evictidx, evictvallen, evictvalbytes, evictseq, evictstat)
        # <int type, int16_t evictidx, uint16_t vallen (big-endian for Val::deserialize), valbytes (same order), int32_t savedseq (little-endian), bool status>
        #sendbuf = struct.pack("=ih!h={}ci?".format(len(evictvalbytes)), switchos_set_evictdata, evictidx, evictvallen, evictvalbytes, evictseq, evictstat)
        sendbuf = struct.pack("=ih", switchos_set_evictdata, evictidx)
        sendbuf = sendbuf + struct.pack("!h", evictvallen)
        sendbuf = struct.pack("={}ci?", evictvalbytes, evictseq, evictstat)
        ptf_sock.sendto(sendbuf, ("127.0.0.1", switchos_paramserver_port))

        self.conn_mgr.complete_operations(self.sess_hdl)
        self.conn_mgr.client_cleanup(self.sess_hdl) # close session
