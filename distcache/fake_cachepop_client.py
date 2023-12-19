#!/bin/python3
from multiprocessing import Process

import logging
import os
import random
import sys
import time
import unittest
import zlib

import socket
import struct
import binascii

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from spineswitch.common import *

# #define WARMUP_RAW_WORKLOAD(buf, workload) \
# 	sprintf(buf, "../benchmark/output/%s-hotest.out", workload)

# #define DYNAMIC_RULEPATH(buf, workload, prefix) \
# 	sprintf(buf, "../benchmark/output/%s-%srules/", workload, prefix)
warmup_raw_workload = f"../benchmarkdist/output/{workload_name}-hotest.out"
dynamic_rulepath = f"../benchmarkdist/output/{workload_name}-{dynamic_ruleprefix}rules/"
fake_cachepop_client_recvport = 10080


def crc32(message: bytes) -> int:
    crc = 0xFFFFFFFF
    for byte in message:
        crc = crc ^ byte
        for _ in range(8):
            mask = -(crc & 1)
            crc = (crc >> 1) ^ (0xEDB88320 & mask)
    return ~crc


def get_hashpartition_idx(buf, partitionnum, servernum):
    hashresult = crc32(buf) % partitionnum
    targetidx = hashresult // (partitionnum // servernum)
    if targetidx == servernum:
        targetidx -= 1
    # print("key: %x, crc32 result: %u, targetidx: %d" % (keyhihi, hashresult, targetidx))
    assert targetidx >= 0 and targetidx < servernum
    return targetidx


fake_cachepop_client_udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
fake_cachepop_client_udpsock.bind(("0.0.0.0", fake_cachepop_client_recvport))


# [x] 预加载200个hotest key 和他们对应的每轮的规则
# [x] 接受client每轮开始时候传来的logicalidx，表示当前使用的rulemap
# [x] 可能有多个client，要对比一下传来的logicalidx是否变化
# 仿制假的cache_pop report发送给server对应的report端口
# 只需要发，因为是switch产生的所以不需要ack，无所谓
#
class RegisterUpdate:
    def build_mapping_table(self, filepath):
        mapping_table = {}
        with open(filepath, "r") as f:
            for _ in range(self.hotkey_scale):
                line = f.readline()
                mapping_table[int(line.split(" ")[0][4:])] = int(line.split(" ")[1][4:])
        return mapping_table

    def __init__(self, hotkey_scale):
        self.mapping_tables = []
        self.hotkeys = []
        self.ruleidx = 0
        self.hotkey_scale = hotkey_scale

        # init hotkey_scale(200) hotkeys
        with open(warmup_raw_workload, "r") as f:
            for i in range(hotkey_scale):
                line = f.readline()
                tmpkey = line.split(" ")[1][4:]
                self.hotkeys.append(int(tmpkey))
                # print(mapping_tables[self.ruleidx][int(tmpkey)])
        # init rulemap
        rules = os.listdir(dynamic_rulepath)
        rules.sort()
        for filename in rules:
            if filename.endswith(".out"):
                self.mapping_tables.append(
                    self.build_mapping_table(os.path.join(dynamic_rulepath, filename))
                )

    def trigger_admission(self):
        # send to 5008 reflector port
        i = 0
        for hotkey in self.hotkeys:
            i += 1
            truekey = self.mapping_tables[self.ruleidx][hotkey]
            hi = (truekey >> 32) & 0xFFFFFFFF
            hilo = hi & 0xFFFF
            hihi = (hi >> 16) & 0xFFFF
            lo = truekey & 0xFFFFFFFF
            # first Q is to makesure key is 128bit
            # second Q is for clonehdr
            packed_i = struct.pack(">HQI2HQ", NETCACHE_GETREQ_POP, 0, lo, hilo, hihi, 0)
            print(binascii.hexlify(packed_i))
            reflector_idx = get_hashpartition_idx(
                struct.pack(">QI2H", 0, lo, hilo, hihi), 32768, 4
            )
            print(
                f"target {(reflector_ips[reflector_idx], reflector_dp2cpserver_port)}"
            )
            fake_cachepop_client_udpsock.sendto(
                packed_i, (reflector_ips[reflector_idx], reflector_dp2cpserver_port)
            )
            if i == 10:
                break

    def setUp(self):
        print("\nSetup")

    def runTest(self):
        print("[ptf.cachefrequencyserver] ready")
        while True:
            # receive control packet
            recvbuf, client_addr = fake_cachepop_client_udpsock.recvfrom(1024)
            print(recvbuf)
            tmpidx, recvbuf = struct.unpack(">I{}s".format(len(recvbuf) - 4), recvbuf)
            if tmpidx != self.ruleidx:
                self.ruleidx = tmpidx
                print(f"ruleidx switch to {tmpidx}")
                self.trigger_admission()


registerupdate = RegisterUpdate(200)
registerupdate.runTest()
