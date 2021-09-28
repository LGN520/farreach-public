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

"""
Thrift PD interface basic tests
"""
from time import sleep
import unittest
import random

import pd_base_tests

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

from netbuffer.p4_pd_rpc.ttypes import *
from knet_mgr_pd_rpc.ttypes import *
from pkt_pd_rpc.ttypes import *
from mc_pd_rpc.ttypes import *
from res_pd_rpc.ttypes import *
from devport_mgr_pd_rpc.ttypes import *


swports = [x for x in range(65)]
dev_id = 0


def dumpObject(obj):
    for attr in dir(obj):
        if hasattr(obj, attr):
            print("obj.%s = %s" % (attr, getattr(obj, attr)))

def open_packet_socket(hostif_name):
    s = socket.socket(socket.AF_PACKET, socket.SOCK_RAW,
                      socket.htons(ETH_P_ALL))
    s.bind((hostif_name, ETH_P_ALL))
    s.setblocking(0)
    return s

class TestCpuPort(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(
            self, ["netbuffer"])

    def runTest(self):
        #os.system("sudo ifconfig test_hostif1 down")
        # Delete hostif
        #self.knet_mgr.knet_hostif_kndev_delete(
        #    cpuif_ndev.knet_cpuif_id, hostif_ndev.knet_hostif_id)
        # Delete cpuif (This will clear out all state for KNET)

        cpuif_list_res = self.knet_mgr.knet_cpuif_list_get(10)
        #dumpObject(cpuif_list_res)
        cpuif_list = cpuif_list_res.cpuif_list
        for i in range(len(cpuif_list)):
          self.knet_mgr.knet_cpuif_ndev_delete(cpuif_list[i].id)
          #os.system("sudo ip link del {}".format(cputif_list[i].name))
