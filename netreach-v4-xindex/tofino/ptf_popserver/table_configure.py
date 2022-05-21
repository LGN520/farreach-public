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

switchos_ptf_popserver_udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
switchos_ptf_popserver_udpsock.bind(("127.0.0.1", switchos_ptf_popserver_port))

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

    def set_valid0(self, freeidx):
        print "Set validvalue_reg as 0"
        index = freeidx
        value = 0
        self.client.register_write_validvalue_reg(self.sess_hdl, self.dev_tgt, index, value)

    def add_cache_lookup_setvalid1(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx):
        print "Add key into cache_lookup_tbl"
        matchspec0 = netbufferv4_cache_lookup_tbl_match_spec_t(\
                op_hdr_keylolo = convert_u32_to_i32(keylolo),
                op_hdr_keylohi = convert_u32_to_i32(keylohi),
                op_hdr_keyhilo = convert_u32_to_i32(keyhilo),
                #op_hdr_keyhihi = convert_u32_to_i32(keyhihi),
                op_hdr_keyhihilo = convert_u16_to_i16(keyhihilo),
                op_hdr_keyhihihi = convert_u16_to_i16(keyhihihi),
                meta_need_recirculate = 0)
        actnspec0 = netbufferv4_cached_action_action_spec_t(freeidx)
        self.client.cache_lookup_tbl_table_add_with_cached_action(\
                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

        print "Set validvalue_reg as 1"
        index = freeidx
        value = 1
        self.client.register_write_validvalue_reg(self.sess_hdl, self.dev_tgt, index, value)

    def get_evictdata_setvalid3(self):
        # NOTE: cache must be full (i.e., all idxes are valid) when cache eviction
        print "Get sampled indexes for in-switch cache eviction"
        cur_sample_cnt = switchos_sample_cnt
        if kv_bucket_num < cur_sample_cnt:
            cur_sample_cnt = kv_bucket_num
        random.seed(time.time())
        sampled_idxes = random.sample(range(0, kv_bucket_num), cur_sample_cnt)

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
        for i in range(len(val_list)):
            # NOTE: we serialize each 4B value as big-endian to keep the same byte order as end-hosts
            evictvalbytes = evictvalbytes + struct.pack("!i", val_list[i])
        # load savedseq
        evictseq = convert_i32_to_u32(self.client.register_read_savedseq_reg(self.sess_hdl, self.dev_tgt, evictidx, flags)[egress_pipeidx])

        print "Serialize evicted data"
        sendbuf = struct.pack("=iH", SWITCHOS_GET_EVICTDATA_SETVALID3_ACK, evictidx)
        sendbuf = sendbuf + struct.pack("!H", evictvallen)
        sendbuf = sendbuf + struct.pack("={}sI?".format(len(evictvalbytes)), evictvalbytes, evictseq, evictstat)
        return sendbuf

    def remove_cache_lookup(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi):
        print "Remove key from cache_lookup_tbl"
        matchspec0 = netbufferv4_cache_lookup_tbl_match_spec_t(\
                op_hdr_keylolo = convert_u32_to_i32(keylolo),
                op_hdr_keylohi = convert_u32_to_i32(keylohi),
                op_hdr_keyhilo = convert_u32_to_i32(keyhilo),
                #op_hdr_keyhihi = convert_u32_to_i32(keyhihi),
                op_hdr_keyhihilo = convert_u16_to_i16(keyhihilo),
                op_hdr_keyhihihi = convert_u16_to_i16(keyhihihi),
                meta_need_recirculate = 0)
        #actnspec0 = netbufferv4_cached_action_action_spec_t(evictidx)
        self.client.cache_lookup_tbl_table_delete_by_match_spec(\
                self.sess_hdl, self.dev_tgt, matchspec0)

    def runTest(self):
        with_switchos_addr = False
        print "[ptf.popserver] ready"

        while True:
            # receive control packet
            if with_switchos_addr == False:
                recvbuf, switchos_addr = switchos_ptf_popserver_udpsock.recvfrom(1024)
                with_switchos_addr = True
            else:
                recvbuf, _ = switchos_ptf_popserver_udpsock.recvfrom(1024)
            control_type, recvbuf = struct.unpack("=i{}s".format(len(recvbuf) - 4), recvbuf)

            if control_type == SWITCHOS_SETVALID0:
                # parse freeidx
                freeidx = struct.unpack("=H", recvbuf)[0]

                # set valid = 0
                self.set_valid0(freeidx)

                # send back SWITCHOS_SETVALID0_ACK
                sendbuf = struct.pack("=i", SWITCHOS_SETVALID0_ACK)
                switchos_ptf_popserver_udpsock.sendto(sendbuf, switchos_addr)
            elif control_type == SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1:
                # parse key and freeidx
                keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, recvbuf = struct.unpack("!3I2H{}s".format(len(recvbuf)-16), recvbuf)
                freeidx = struct.unpack("=H", recvbuf)[0]

                # add <key, idx> into cache_lookup_tbl, and set valid = 1
                self.add_cache_lookup_setvalid1(keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx)

                # send back SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK
                sendbuf = struct.pack("=i", SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK)
                switchos_ptf_popserver_udpsock.sendto(sendbuf, switchos_addr)
            elif control_type == SWITCHOS_GET_EVICTDATA_SETVALID3:
                # calculate sample index, set valid = 3, and load evict data from data plane
                sendbuf = self.get_evictdata_setvalid3()

                # send back SWITCHOS_GET_EVICTDATA_SETVALID3_ACK
                switchos_ptf_popserver_udpsock.sendto(sendbuf, switchos_addr)
            elif control_type == SWITCHOS_REMOVE_CACHE_LOOKUP:
                # parse key
                keylolo, keylohi, keyhilo, keyhihilo, keyhihihi = struct.unpack("!3I2H", recvbuf)

                # remove key from cache_lookup_tbl
                self.remove_cache_lookup(keylolo, keylohi, keyhilo, keyhihilo, keyhihihi)

                # send back SWITCHOS_REMOVE_CACHE_LOOKUP_ACK
                sendbuf = struct.pack("=i", SWITCHOS_REMOVE_CACHE_LOOKUP_ACK)
                switchos_ptf_popserver_udpsock.sendto(sendbuf, switchos_addr)
            elif control_type == SWITCHOS_PTF_POPSERVER_END:
                print("[ptf.popserver] END")
                break;
            else:
                print("Invalid control type {}".format(control_type))
                exit(-1)

        self.conn_mgr.complete_operations(self.sess_hdl)
        self.conn_mgr.client_cleanup(self.sess_hdl) # close session
