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
from p4utils.utils.sswitch_thrift_API import SimpleSwitchThriftAPI
controller = SimpleSwitchThriftAPI(9090, "192.168.122.229") 
import logging
import os

import random
import sys
import time
import unittest

import socket
import struct

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *

switchos_ptf_popserver_udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
switchos_ptf_popserver_udpsock.bind(("127.0.0.1", switchos_ptf_popserver_port))


class RegisterUpdate():


    # def setUp(self):
    #     print '\nSetup'

    #     # initialize the connection
    #     pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
    #     self.sess_hdl = self.conn_mgr.client_init()
    #     self.dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF)) # 0xFFFF means setting all pipelines

    #     self.platform_type = "mavericks"
    #     board_type = self.pltfm_pm.pltfm_pm_board_type_get()
    #     if re.search("0x0234|0x1234|0x4234|0x5234", hex(board_type)):
    #         self.platform_type = "mavericks"
    #     elif re.search("0x2234|0x3234", hex(board_type)):
    #         self.platform_type = "montara"

    def set_valid0(self, freeidx, pipeidx):
        # NOTE: if you want to set MAT in a specific pipeline, you must set it as an asymmetric table -> not supported now
        #self.client.access_validvalue_tbl_set_property(self.sess_hdl, self.dev_tgt.dev_id, tbl_property_t.TBL_PROP_TBL_ENTRY_SCOPE, tbl_property_value_t.ENTRY_SCOPE_SINGLE_PIPELINE, 0)

        print("[ERROR] you should setvalid0 by data plane instead of via ptf channel")
        exit(-1)

        
        index = freeidx
        value = 0
        controller.register_write('validvalue_reg', index, value)

    #def add_cache_lookup_setvalid1(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx, piptidx):
    def add_cache_lookup(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx):
        #print "Add key into cache_lookup_tbl for all pipelines"
        matchspec0 =[
                hex(keylolo),
                hex(keylohi),
                hex(keyhilo),
                hex(keyhihilo),
                hex(keyhihihi),
                hex(0)]
        actnspec0 = [hex(freeidx)]
        controller.table_add('cache_lookup_tbl','cached_action', matchspec0, actnspec0)

    #def get_evictdata_setvalid3(self, pipeidx):
    def setvalid3(self, evictidx, pipeidx):
        print("[ERROR] you should setvalid3 by data plane instead of via ptf channel")
        exit(-1)
        index = evictidx
        value = 3
        controller.register_write('validvalue_reg', index, value)

    def remove_cache_lookup(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi):
        #print "Remove key from cache_lookup_tbl for all pipelines"
        matchspec0 = [
                hex(keylolo),
                hex(keylohi),
                hex(keyhilo),
                hex(keyhihilo),
                hex(keyhihihi),
                hex(0)]
        #actnspec0 = netbufferv4_cached_action_action_spec_t(evictidx)
        controller.table_delete_match('cache_lookup_tbl', matchspec0)

    def writeallseq(self, maxseq):
        index=[0,32768-1]#32768
        controller.register_write('seq_reg',index,maxseq)
        # self.client.register_write_all_seq_reg(self.sess_hdl, self.dev_tgt, maxseq)

    def runTest(self):
        #with_switchos_addr = False
        print("[ptf.popserver] ready")

        #ptf_cached_keyset = set()

        while True:

            recvbuf, switchos_addr = switchos_ptf_popserver_udpsock.recvfrom(1024)

            control_type, recvbuf = struct.unpack("=i{}s".format(len(recvbuf) - 4), recvbuf)

            if control_type == SWITCHOS_ADD_CACHE_LOOKUP:
                # parse key and freeidx
                keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, recvbuf = struct.unpack("!3I2H{}s".format(len(recvbuf)-16), recvbuf)
                #freeidx, pipeidx = struct.unpack("=HI", recvbuf)
                freeidx = struct.unpack("=H", recvbuf)[0]

                self.add_cache_lookup(keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx)

                sendbuf = struct.pack("=i", SWITCHOS_ADD_CACHE_LOOKUP_ACK)
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
                switchos_ptf_popserver_udpsock.close()
                print("[ptf.popserver] END")
                break;
            elif control_type == SWITCHOS_WRITEALLSEQ:
                maxseq = struct.unpack("=I", recvbuf)[0]
                print("Write all seq_reg as {}".format(maxseq))
                self.writeallseq(maxseq)
                # NOTE: not need ack
            else:
                print("Invalid control type {}".format(control_type))
                exit(-1)

registerupdate = RegisterUpdate()

registerupdate.runTest()
