import os
import time
import json
import math
from itertools import product

import logging
import ptf
import grpc
from ptf import config
import ptf.testutils as testutils

from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as gc
import time
from ptf.thriftutils import *
from ptf.testutils import *
from ptf_port import *
this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *


import socket
import struct

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *

switchos_ptf_popserver_udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
switchos_ptf_popserver_udpsock.bind(("127.0.0.1", switchos_ptf_popserver_port))


class RegisterUpdate(BfRuntimeTest):

    def setUp(self):
        print('\nSetup')
        client_id = 1
        p4_name = "netcache"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        bfrt_info = self.interface.bfrt_info_get("netcache")
        
        self.target = gc.Target(device_id=0, pipe_id=0xffff)
        self.cache_lookup_tbl = bfrt_info.table_get("netcacheIngress.cache_lookup_tbl")
    
    def add_cache_lookup(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx):
        print("Add key",freeidx)
        key = self.cache_lookup_tbl.make_key([
            gc.KeyTuple('hdr.op_hdr.keylolo', keylolo),
            gc.KeyTuple('hdr.op_hdr.keylohi', keylohi),
            gc.KeyTuple('hdr.op_hdr.keyhilo', keyhilo),
            gc.KeyTuple('hdr.op_hdr.keyhihilo', keyhihilo),
            gc.KeyTuple('hdr.op_hdr.keyhihihi', keyhihihi),])
        data = self.cache_lookup_tbl.make_data(
            [gc.DataTuple('idx', freeidx)],
            'netcacheIngress.cached_action')
        self.cache_lookup_tbl.entry_add(self.target, [key], [data])

    def remove_cache_lookup(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi):
        print("Remove key")
        key = self.cache_lookup_tbl.make_key([
            gc.KeyTuple('hdr.op_hdr.keylolo', keylolo),
            gc.KeyTuple('hdr.op_hdr.keylohi', keylohi),
            gc.KeyTuple('hdr.op_hdr.keyhilo', keyhilo),
            gc.KeyTuple('hdr.op_hdr.keyhihilo', keyhihilo),
            gc.KeyTuple('hdr.op_hdr.keyhihihi', keyhihihi),])
        self.cache_lookup_tbl.entry_del(self.target, [key])
        
    def runTest(self):
        print("[ptf.popserver] ready")
        while True:
            # receive control packet
            recvbuf, switchos_addr = switchos_ptf_popserver_udpsock.recvfrom(1024)
            control_type, recvbuf = struct.unpack("=i{}s".format(len(recvbuf) - 4), recvbuf)
            # print(control_type)
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
