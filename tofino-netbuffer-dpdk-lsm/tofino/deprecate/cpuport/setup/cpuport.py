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

#swports = [x for x in range(65)]
dev_id = 0

def dumpObject(obj):
    for attr in dir(obj):
        if hasattr(obj, attr):
            print("obj.%s = %s" % (attr, getattr(obj, attr)))

class TestCpuPort(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(
            self, ["netbuffer"])

    def runTest(self):
        try:
            sess_hdl = self.conn_mgr.client_init()

            # Wait for all pipe APIs to complete.
            self.conn_mgr.complete_operations(sess_hdl)

            # Create CPU interface
            cpuif_ndev = self.knet_mgr.knet_cpuif_ndev_add("veth251") # veth250 and veth251 are for port 64
            #dumpObject(cpuif_ndev)
            self.assertTrue(cpuif_ndev.status == 0,
                            "Cpu netdev add failed for veth251")

            # Create host interface
            hostif_ndev = self.knet_mgr.knet_hostif_kndev_add(
                cpuif_ndev.knet_cpuif_id, "test_hostif1")
            self.assertTrue(hostif_ndev.status == 0,
                            "hostif netdev add failed for test_hostif1")
            os.system("sudo ifconfig test_hostif1 up")
        finally:
            sess_hdl = self.conn_mgr.client_cleanup(sess_hdl)
