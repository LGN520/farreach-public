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
import struct

from netbuffer.p4_pd_rpc.ttypes import *
from pltfm_pm_rpc.ttypes import *
from pal_rpc.ttypes import *
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from res_pd_rpc.ttypes import *

import socket
import struct

this_dir = os.path.dirname(os.path.abspath(__file__))

# Front Panel Ports
#   List of front panel ports to use. Each front panel port has 4 channels.
#   Port 1 is broken to 1/0, 1/1, 1/2, 1/3. Test uses 2 ports.
#
#   ex: ["1/0", "1/1"]
#

import ConfigParser
config = ConfigParser.ConfigParser()
with open(os.path.join(os.path.dirname(os.path.dirname(this_dir)), "config.ini"), "r") as f:
    config.readfp(f)

server_ip = str(config.get("server", "server_ip"))
server_port = int(config.get("server", "server_port"))

class RegisterUpdate(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbuffer"])

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

    @staticmethod
    def get_reg16(reglist, idx):
        tmpreg = reglist[idx] # Big-endian
        tmpreg = (((tmpreg & 0xFF00) >> 8) & 0x00FF) | ((tmpreg & 0x00FF) << 8) # Small-endian
        if (tmpreg < 0):
            tmpreg += pow(2, 16)
        return tmpreg

    @staticmethod
    def get_reg32(reglist, idx):
        tmpreg = reglist[idx] # Big-endian
        tmphihi = ((tmpreg & 0xFF000000) >> 24) & 0x000000FF
        tmphilo = ((tmpreg & 0x00FF0000) >> 16) & 0x000000FF
        tmplohi = ((tmpreg & 0x0000FF00) >> 8) & 0x000000FF
        tmplolo = tmpreg & 0x000000FF
        tmpreg = (tmplolo << 24) | (tmplohi << 16) | (tmphilo << 8) | tmphihi # Small-endian
        if (tmpreg < 0):
            tmpreg += pow(2, 32)
        return tmpreg

    def runTest(self):
        # TODO: Parse key, value, hashidx
        # TMP
        data = sys.argv[sys.argv.index("--data") + 1]
        a = struct.unpack("=HB", data)
        print(a)
