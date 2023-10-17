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

class RegisterUpdate():
    # def __init__(self):
    #     # initialize the thrift data plane
    #     pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netcache"])

    def setUp(self):
        print('\nSetup')


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

registerupdate = RegisterUpdate()
registerupdate.runTest()