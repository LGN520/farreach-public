from multiprocessing import Process

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


class RegisterUpdate(Process):
    def __init__(self, rack_idx):
        super(RegisterUpdate, self).__init__()
        self.rack_idx = rack_idx
        self.controller = SimpleSwitchThriftAPI(
            9090 + rack_idx + 1, "192.168.122.229"
        )  # 9090，127.0.0.1

    def setUp(self):
        print("\nSetup")

    def run(self):
        print("[ptf.cleaner] ready")

        # Change clean period based on packet sending rate
        # first_period = 1
        # clean_period = 5
        # is_first = True
        clean_period = 1  # for threshold = 50
        # clean_period = 0.2 # for threshold = 10
        while True:
            time.sleep(clean_period)
            self.controller.register_reset("cm1_reg")
            self.controller.register_reset("cm2_reg")
            self.controller.register_reset("cm3_reg")
            self.controller.register_reset("cm4_reg")
            self.controller.register_reset("cache_frequency_reg")
            self.controller.register_reset("bf1_reg")
            self.controller.register_reset("bf2_reg")
            self.controller.register_reset("bf3_reg")


process_list = []
for i in range(int(server_physical_num/2)):
    p = RegisterUpdate(i)
    p.start()
    process_list.append(p)

for i in process_list:
    p.join()
