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
import redis

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

server_pop_ip = str(config.get("server", "server_backup_ip")) # Same IP
server_pop_port = int(config.get("server", "server_populator_port"))
redis_ip = str(config.get("controller", "redis_ip"))
redis_port = int(config.get("controller", "redis_port"))
hashidx_prefix = str(config.get("other", "hashidx_prefix"))
pop_prefix = str(config.get("other", "pop_prefix"))

class RegisterUpdate(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbufferv2"])

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
        tmpreg = reglist[idx] # Big-endian int
        tmpreg = (((tmpreg & 0xFF00) >> 8) & 0x00FF) | ((tmpreg & 0x00FF) << 8) # Small-endian int
        if tmpreg < 0:
            tmpreg += pow(2, 16) # int -> uint
        return tmpreg

    @staticmethod
    def get_reg32(reglist, idx):
        tmpreg = reglist[idx] # Big-endian int
        tmphihi = ((tmpreg & 0xFF000000) >> 24) & 0x000000FF
        tmphilo = ((tmpreg & 0x00FF0000) >> 16) & 0x000000FF
        tmplohi = ((tmpreg & 0x0000FF00) >> 8) & 0x000000FF
        tmplolo = tmpreg & 0x000000FF
        tmpreg = (tmplolo << 24) | (tmplohi << 16) | (tmphilo << 8) | tmphihi # Small-endian int
        if tmpreg < 0:
            tmpreg += pow(2, 32) # int -> uint
        return tmpreg

    @staticmethod
    def set_reg32(value):
        tmpreg = value # Small-endian uint
        tmphihi = ((tmpreg & 0xFF000000) >> 24) & 0x000000FF
        tmphilo = ((tmpreg & 0x00FF0000) >> 16) & 0x000000FF
        tmplohi = ((tmpreg & 0x0000FF00) >> 8) & 0x000000FF
        tmplolo = tmpreg & 0x000000FF
        tmpreg = (tmplolo << 24) | (tmplohi << 16) | (tmphilo << 8) | tmphihi # Big-endian uint
        if tmpreg >= pow(2, 31):
            tmpreg -= pow(2, 32) # uint -> int
        return tmpreg


    def runTest(self):
        # Decode key, vallen, value, hashidx, thread_id
        r = redis.Redis(host=redis_ip, port=redis_port, decode_responses=True)
        data = r.get(pop_prefix)
        items = data.split("-")
        keylo, keyhi, vallen = int(items[0]), int(items[1]), int(items[2])
        value_list = []
        for i in range(vallen):
            value_list.append(int(items[3+i]))
        hashidx, thread_id = int(items[-2]), int(items[-1])
        hashidx_key = "{}{}".format(hashidx_prefix, hashidx)

        # Set corresponding being_evicted bit as 1
        self.client.register_write_being_evicted_reg(self.sess_hdl, self.dev_tgt, hashidx, 1)

        # Deprecated: Load corresponding valid bit
        #flags = netbuffer_register_flags_t(read_hw_sync=True)
        #valid_list = self.client.register_read_valid_reg(self.sess_hdl, self.dev_tgt, hashidx, flags)

        # Check redis
        isexist = r.exists(hashidx_key)

        if isexist == 1: # cache eviction
            prev_keylo, prev_keyhi, prev_thread_id = r.get(hashidx_key).split(" ")
            prev_keylo = int(prev_keylo)
            prev_keyhi = int(prev_keyhi)
            latest_list = self.client.register_read_latest_reg(self.sess_hdl, self.dev_tgt, hashidx, flags) # Ptf converts big-endian int8 to small-endian int8
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            server_addr = (server_pop_ip, server_pop_port)
            buf = bytearray()

            if (latest_list[0] == 0 and latest_list[1] == 0): # 2 pipelines
                # Send <latest, thread_id, evicted keylo, evicted keyhi> to server
                buf = struct.pack("=BBQQ", 0, prev_thread_id, prev_keylo, prev_keyhi)
                s.sendto(buf, server_addr)

                _, _ = s.recvfrom(1024) # Wait for ACK
                r.delete(hashidx_key) # Remove evicted key from redis
            elif (latest_list[0] == 1 or latest_list[1] == 1):
                if latest_list[0] == 1:
                    pipeidx = 0
                else:
                    pipeidx = 1

                # Load seq, vallen, and value
                prev_seq = self.client.register_read_seq_reg(self.sess_hdl, self.dev_tgt, hashidx, flags)[pipeidx] # Ptf converts big-endian int32 to small-endian int32
                if prev_seq < 0:
                    prev_seq += pow(2, 32) # int32 -> uint32
                prev_vallen = self.client.register_read_vallen_reg(self.sess_hdl, self.dev_tgt, hashidx, flags)[pipeidx] # 8-bit vallen
                prev_value_list = []
                for i in range(prev_vallen):
                    prev_validx = prev_vallen - i
                    # Ptf converts small-endian int32 to big-endian int32
                    tmp_prev_vallo = eval("self.client.register_read_vallo{}_reg".format(prev_validx))(self.sess_hdl, self.dev_tgt, flags)[pipeidx]
                    tmp_prev_valhi = eval("self.client.register_read_valhi{}_reg".format(prev_validx))(self.sess_hdl, self.dev_tgt, flags)[pipeidx]
                    # Convert big-endian int32 to small-endian uint32
                    tmp_prev_vallo = RegisterUpdate.get_reg32(tmp_prev_vallo)
                    tmp_prev_valhi= RegisterUpdate.get_reg32(tmp_prev_valhi)
                    prev_value_list.append((tmp_prev_valhi << 32) | tmp_prev_vallo)

                # Send <latest, thread_id, evicted keylo, evicted keyhi, seq, vallen, val>
                buf = struct.pack("=BBQQIB", 1, prev_thread_id, prev_keylo, prev_keyhi, prev_seq, prev_vallen)
                for i in range(prev_vallen):
                    buf = buf + struct.pack("=Q", prev_value_list[i])

                _, _ = s.recvfrom(1024) # Wait for ACK
                r.delete(hashidx_key) # Remove evicted key from redis
            elif (latest_list[0] == 2 or latest_list[1] == 2):
                if latest_list[0] == 2:
                    pipeidx = 0
                else:
                    pipeidx = 1

                # Load seq
                prev_seq = self.client.register_read_seq_reg(self.sess_hdl, self.dev_tgt, hashidx, flags)[pipeidx] # Ptf converts big-endian int32 to small-endian int32
                if prev_seq < 0:
                    prev_seq += pow(2, 32) # int32 -> uint32

                # Send <latest, thread_id, evicted keylo, evicted keyhi, seq>
                buf = struct.pack("=BBQQB", 1, prev_thread_id, prev_keylo, prev_keyhi, prev_seq)

                _, _ = s.recvfrom(1024) # Wait for ACK
                r.delete(hashidx_key) # Remove evicted key from redis
            else:
                print("Invalid condition: latest[0] = {}, latest[1] = {}".format(latest_list[0], latest_list[1]))
                exit()

        # Reset registers
        self.client.register_write_vote_reg(self.sess_hdl, self.dev_tgt, hashidx, 1) # vote=1 (ptf converts it into big-endian in reg)
        self.client.register_reset_lastest_reg(self.sess_hdl, self.dev_tgt, hashidx) # latest=0
        self.client.register_reset_lock_reg(self.sess_hdl, self.dev_tgt, hashidx) # lock=0
        self.client.register_reset_seq_reg(self.sess_hdl, self.dev_tgt, hashidx) # seq=0

        # Set vallen
        self.client.register_write_vallen_reg(self.sess_hdl, self.dev_tgt, hashidx, vallen) # 8-bit vallen 

        # Set value (val_data[0] to val_data[vallen-1]: val{vallen} to val1)
        # NOTE: value_list follows the same order and endianess of val_data
        for i in range(vallen):
            validx = vallen - i
            eval("self.client.register_write_vallo{}_reg".format(validx))(\
                    self.sess_hdl, self.dev_tgt, RegisterUpdate.set_reg32(value_list[i] & 0xFFFFFFFF)) # Ptf converts big-endian int32 to small-endian int32 in reg
            eval("self.client.register_write_valhi{}_reg".format(validx))(\
                    self.sess_hdl, self.dev_tgt, RegisterUpdate.set_reg32((value_list[i] >> 32) & 0xFFFFFFFF)) # Ptf converts big-endian int32 to small-endian int32 in reg

        # Add new key into redis
        r.set(hashidx_key, "{} {} {}".format(keylo, keyhi, thread_id))

        # Add new key into MAT (TODO: use small-endian uint16)
        matchspec0 = netbufferv2_cache_lookup_tbl_match_spec_t(\
                op_hdr_keylololo = keylo & 0xFFFF,
                op_hdr_keylolohi = (keylo >> 16) & 0xFFFF,
                op_hdr_keylohilo = (keylo >> 32) & 0xFFFF,
                op_hdr_keylohihi = (keylo >> 48) & 0xFFFF,
                op_hdr_keyhilolo = keyhi & 0xFFFF,
                op_hdr_keyhilohi = (keyhi >> 16) & 0xFFFF,
                op_hdr_keyhihilo = (keyhi >> 32) & 0xFFFF,
                op_hdr_keyhihihi = (keyhi >> 48) & 0xFFFF)
        self.client.cache_lookup_tbl_table_add_with_cache_lookup(\
                self.sess_hdl, self.dev_tgt, matchspec0)

        # Set corresponding being_evicted bit as 0
        self.client.register_write_being_evicted_reg(self.sess_hdl, self.dev_tgt, hashidx, 0)

        r.delete(pop_prefix)
