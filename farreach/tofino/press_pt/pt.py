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

logger = logging.getLogger('Test')
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
        p4_name = "netbufferv4"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        bfrt_info = self.interface.bfrt_info_get("netbufferv4")
        
        self.target = gc.Target(device_id=0, pipe_id=0xffff)
        self.cache_lookup_tbl = bfrt_info.table_get("farreachIngress.cache_lookup_tbl")
    
    def add_cache_lookup(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx):
        key = self.cache_lookup_tbl.make_key([
            gc.KeyTuple('hdr.op_hdr.keylolo', keylolo),
            gc.KeyTuple('hdr.op_hdr.keylohi', keylohi),
            gc.KeyTuple('hdr.op_hdr.keyhilo', keyhilo),
            gc.KeyTuple('hdr.op_hdr.keyhihilo', keyhihilo),
            gc.KeyTuple('hdr.op_hdr.keyhihihi', keyhihihi),])
        data = self.cache_lookup_tbl.make_data(
            [gc.DataTuple('idx', freeidx)],
            'farreachIngress.cached_action')
        self.cache_lookup_tbl.entry_add(self.target, [key], [data])

    def remove_cache_lookup(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi,):
        # print("Remove key")
        key = self.cache_lookup_tbl.make_key([
            gc.KeyTuple('hdr.op_hdr.keylolo', keylolo),
            gc.KeyTuple('hdr.op_hdr.keylohi', keylohi),
            gc.KeyTuple('hdr.op_hdr.keyhilo', keyhilo),
            gc.KeyTuple('hdr.op_hdr.keyhihilo', keyhihilo),
            gc.KeyTuple('hdr.op_hdr.keyhihihi', keyhihihi),])
        self.cache_lookup_tbl.entry_del(self.target, [key])
    
    def add_cache_lookup(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx):
        key = self.cache_lookup_tbl.make_key([
            gc.KeyTuple('hdr.op_hdr.keylolo', keylolo),
            gc.KeyTuple('hdr.op_hdr.keylohi', keylohi),
            gc.KeyTuple('hdr.op_hdr.keyhilo', keyhilo),
            gc.KeyTuple('hdr.op_hdr.keyhihilo', keyhihilo),
            gc.KeyTuple('hdr.op_hdr.keyhihihi', keyhihihi),])
        data = self.cache_lookup_tbl.make_data(
            [gc.DataTuple('idx', freeidx)],
            'farreachIngress.cached_action')
        self.cache_lookup_tbl.entry_add(self.target, [key], [data])


    def runTest(self):
        delay_list_add = []
        delay_list_rm = []
        print("[ptf.popserver] ready")
        for circle in range(10):
            print("circle#",circle)
            #
            start = time.time()
            for i in range(0, 9999):
                self.add_cache_lookup(i,1,2,3,4,i)
            end = time.time()
            delay_list_add.append(end - start)

            print("table write")
            print(end - start)

            #
            start1 = time.time()
            for i in range(0, 9999):
                self.remove_cache_lookup(i,1,2,3,4)
            end1 = time.time()
            delay_list_rm.append(end1 - start1)

            print("table delete")
            print(end1 - start1)

        print(delay_list_add)
        print(delay_list_rm)