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

# remember to install it to

import socket
import struct

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *


class RegisterUpdate(BfRuntimeTest):
    def setUp(self):
        print('\nSetup')
        client_id = 2
        p4_name = "netcache"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        bfrt_info = self.interface.bfrt_info_get("netcache")
        
        self.target = gc.Target(device_id=0, pipe_id=0xffff)

        self.cm1_reg = bfrt_info.table_get("Egress.cm1_reg")
        self.cm2_reg = bfrt_info.table_get("Egress.cm2_reg")
        self.cm3_reg = bfrt_info.table_get("Egress.cm3_reg")
        self.cm4_reg = bfrt_info.table_get("Egress.cm4_reg")
        self.cache_frequency_reg = bfrt_info.table_get("Egress.cache_frequency_reg")
        self.bf1_reg = bfrt_info.table_get("Egress.bf1_reg")
        self.bf2_reg = bfrt_info.table_get("Egress.bf2_reg")
        self.bf2_reg = bfrt_info.table_get("Egress.bf3_reg")

    def runTest(self):
        print("[ptf.cleaner] ready")

        # Change clean period based on packet sending rate
        #first_period = 1
        #clean_period = 5
        #is_first = True
        clean_period = 1 # for threshold = 50
        #clean_period = 0.2 # for threshold = 10
        while True:
            time.sleep(clean_period)
            controller.register_reset('cm1_reg')
            controller.register_reset('cm2_reg')
            controller.register_reset('cm3_reg')
            controller.register_reset('cm4_reg')
            controller.register_reset('cache_frequency_reg')
            controller.register_reset('bf1_reg')
            controller.register_reset('bf2_reg')
            controller.register_reset('bf3_reg')

