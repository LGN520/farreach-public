# TODO: Replace PROC, ACTION, and TABLE

"""
Thrift PD interface DV test
"""

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

from p4utils.utils.sswitch_thrift_API import SimpleSwitchThriftAPI
controller = SimpleSwitchThriftAPI(9090, "192.168.122.229") # 9090，127.0.0.1

switchos_ptf_popserver_udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
switchos_ptf_popserver_udpsock.bind(("127.0.0.1", switchos_ptf_popserver_port))



class RegisterUpdate():
    # def __init__(self):
    #     # initialize the thrift data plane
    #     pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netcache"])

    def setUp(self):
        print('\nSetup')

    def set_valid0(self, freeidx, pipeidx):
        # NOTE: if you want to set MAT in a specific pipeline, you must set it as an asymmetric table -> not supported now
        #self.client.access_validvalue_tbl_set_property(self.sess_hdl, self.dev_tgt.dev_id, tbl_property_t.TBL_PROP_TBL_ENTRY_SCOPE, tbl_property_value_t.ENTRY_SCOPE_SINGLE_PIPELINE, 0)

        print("[ERROR] you should setvalid0 by data plane instead of via ptf channel")
        exit(-1)

        #print "Set validvalue_reg as 0 for pipeline {}".format(pipeidx)
        # tmp_devtgt = DevTarget_t(0, hex_to_i16(pipeidx))
        index = freeidx
        value = 0
        controller.register_write('validvalue_reg',index, value)
        # self.client.register_write_validvalue_reg(self.sess_hdl, tmp_devtgt, index, value)

    #def add_cache_lookup_setvalid1(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx, piptidx):
    def add_cache_lookup(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx):
        #print "Add key into cache_lookup_tbl for all pipelines"
        matchspec0 = [hex(keylolo),
                hex(keylohi),
                hex(keyhilo),
                hex(keyhihilo),
                hex(keyhihihi)]
        actnspec0 = [hex(freeidx)]
        controller.table_add('cache_lookup_tbl','cached_action',matchspec0, actnspec0)

    #def get_evictdata_setvalid3(self, pipeidx):
    def setvalid3(self, evictidx, pipeidx):
        print("[ERROR] you should setvalid3 by data plane instead of via ptf channel")
        exit(-1)

        # NOTE: cache must be full (i.e., all idxes are valid) when cache eviction

        # tmp_devtgt = DevTarget_t(0, hex_to_i16(pipeidx))
        index = evictidx
        value = 3
        # self.client.register_write_validvalue_reg(self.sess_hdl, tmp_devtgt, index, value)
        controller.register_write('validvalue_reg',index, value)


    def remove_cache_lookup(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi):
        #print "Remove key from cache_lookup_tbl for all pipelines"
        matchspec0 = [hex(keylolo),
                hex(keylohi),
                hex(keyhilo),
                hex(keyhihilo),
                hex(keyhihihi)]
        controller.table_delete_match('cache_lookup_tbl',matchspec0)
        # self.client.cache_lookup_tbl_table_delete_by_match_spec(\
        #         self.sess_hdl, self.dev_tgt, matchspec0)

    def runTest(self):
        print("[ptf.popserver] ready")

        while True:
            # receive control packet
            recvbuf, switchos_addr = switchos_ptf_popserver_udpsock.recvfrom(1024)
            control_type, recvbuf = struct.unpack("=i{}s".format(len(recvbuf) - 4), recvbuf)

            if control_type == SWITCHOS_ADD_CACHE_LOOKUP:
                # parse key and freeidx
                keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, recvbuf = struct.unpack("!3I2H{}s".format(len(recvbuf)-16), recvbuf)
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
            else:
                print("Invalid control type {}".format(control_type))
                exit(-1)

registerupdate = RegisterUpdate()
registerupdate.runTest()