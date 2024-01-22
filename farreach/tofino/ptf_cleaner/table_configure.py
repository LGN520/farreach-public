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

controller = SimpleSwitchThriftAPI(9090, "192.168.122.229")  # 9090，127.0.0.1


class RegisterUpdate:
    def runTest(self):
        print("[ptf.cleaner] ready")

        # Change clean period based on packet sending rate
        # first_period = 1
        # clean_period = 5
        # is_first = True
        clean_period = 1  # for threshold = 50
        # clean_period = 0.2 # for threshold = 10
        while True:
            # if is_first:
            #    time.sleep(first_period)
            #    is_first = False
            # else:
            #    time.sleep(clean_period)
            time.sleep(clean_period)

            # print("Start to reset all cm regs")
            controller.register_reset("cm1_reg")
            controller.register_reset("cm2_reg")
            controller.register_reset("cm3_reg")
            controller.register_reset("cm4_reg")

            # print("Start to reset all cache frequency reg")
            controller.register_reset("cache_frequency_reg")

            # print("Finish to reset all cm regs")


registerupdate = RegisterUpdate()

registerupdate.runTest()
