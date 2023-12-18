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

switchos_ptf_cachefrequencyserver_udpsock = socket.socket(
    socket.AF_INET, socket.SOCK_DGRAM
)
switchos_ptf_cachefrequencyserver_udpsock.bind(
    ("", switchos_ptf_cachefrequencyserver_port)
)


class RegisterUpdate:
    def __init__(self, rack_idx):
        self.rack_idx = rack_idx
        self.controller = SimpleSwitchThriftAPI(
            9090 + rack_idx + 1, "192.168.122.229"
        )  # 9090，127.0.0.1

    def setUp(self):
        print("\nSetup")

    def get_cache_frequency(self, received_idx):
        return self.controller.register_read("cache_frequency_reg", received_idx)

    def runTest(self):
        print("[ptf.cachefrequencyserver] ready")

        while True:
            # receive control packet
            data, client_addr = switchos_ptf_cachefrequencyserver_udpsock.recvfrom(1024)
            received_idx = int.from_bytes(data, byteorder="little")
            frequency_num = self.get_cache_frequency(received_idx)

            print(f"Received idx: {received_idx}, frequency: {frequency_num}")

            switchos_ptf_cachefrequencyserver_udpsock.sendto(
                frequency_num.to_bytes(4, byteorder="little"), client_addr
            )


registerupdate = RegisterUpdate(int(sys.argv[1]))
registerupdate.runTest()
# process_list = []
# for i in range(3):
#     p = RegisterUpdate(i)
#     p.start()
#     process_list.append(p)

# for i in process_list:
#     p.join()
