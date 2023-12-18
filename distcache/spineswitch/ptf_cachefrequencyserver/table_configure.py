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
            9100 + rack_idx + 1, "192.168.122.229"
        )  # 9090，127.0.0.1

    def setUp(self):
        print("\nSetup")

    def get_cache_frequency(self, received_idx):
        return self.controller.register_read("cache_frequency_reg", received_idx)

    def set_deleted(self, received_idx):
        return self.controller.register_write("deleted_reg", received_idx, 1)

    def runTest(self):
        print("[ptf.cachefrequencyserver] ready fot spine switch", flush =True)

        while True:
            # receive control packet
            recvbuf, client_addr = switchos_ptf_cachefrequencyserver_udpsock.recvfrom(
                1024
            )
            control_type, recvbuf = struct.unpack(
                "=i{}s".format(len(recvbuf) - 4), recvbuf
            )
            print(control_type, flush =True)
            if control_type == CACHE_FREQUENCYREQ:
                received_idx = struct.unpack("=i", recvbuf)[0]
                frequency_num = self.get_cache_frequency(received_idx)

                print(f"Received idx: {received_idx}, frequency: {frequency_num}")

                response_data = struct.pack(
                    "=iii", CACHE_FREQUENCYACK, received_idx, frequency_num
                )
                switchos_ptf_cachefrequencyserver_udpsock.sendto(
                    response_data, client_addr
                )
            if control_type == SETDELETEDREQ:
                received_idx = struct.unpack("=i", recvbuf)[0]
                self.set_deleted(received_idx)

                print(f"Received idx: {received_idx} set deleted")

                response_data = struct.pack("=i", SETDELETEDACK)
                switchos_ptf_cachefrequencyserver_udpsock.sendto(
                    response_data, client_addr
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
