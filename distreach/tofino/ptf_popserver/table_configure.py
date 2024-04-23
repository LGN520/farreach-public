import os
import time
import json
import math
from itertools import product
from time import sleep
import logging
import ptf
import grpc
from ptf import config
import ptf.testutils as testutils

from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as gc
import time
from ptf.thriftutils import *
from ptf.testutils import *
from ptf_port import *
this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *


import threading
import socket
import struct

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *

switchos_ptf_popserver_udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# switchos_ptf_popserver_udpsock.settimeout(10)
switchos_ptf_popserver_udpsock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 20480000)
switchos_ptf_popserver_udpsock.bind(("0.0.0.0", switchos_ptf_popserver_port))


KV_BUCKET_COUNT = 32768
CM_BUCKET_COUNT = 65536
class RegisterUpdate(BfRuntimeTest):
   
    def setUp(self):
        print('\nSetup')
        self.clean_period = 1 
        client_id = 1
        p4_name = "distreach"
        prefix="farreachIngress"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        bfrt_info = self.interface.bfrt_info_get("distreach")
        
        self.target = gc.Target(device_id=0, pipe_id=0xffff)
        self.cache_lookup_tbl = bfrt_info.table_get("farreachIngress.cache_lookup_tbl")
        
        self.cm1_reg_batch = []
        self.cm2_reg_batch = []
        self.cm3_reg_batch = []
        self.cm4_reg_batch = []
        self.cache_frequency_reg_batch = []
        self.cm1_reg_batch_data = []
        self.cm2_reg_batch_data = []
        self.cm3_reg_batch_data = []
        self.cm4_reg_batch_data = []
        self.cache_frequency_reg_batch_data = []
        self.cm1_reg = bfrt_info.table_get("cm1_reg")
        self.cm2_reg = bfrt_info.table_get("cm2_reg")
        self.cm3_reg = bfrt_info.table_get("cm3_reg")
        self.cm4_reg = bfrt_info.table_get("cm4_reg")
        self.cache_frequency_reg = bfrt_info.table_get("cache_frequency_reg")
        # for register_idx in range(KV_BUCKET_COUNT):
        #     self.cache_frequency_reg_batch.append(self.cache_frequency_reg.make_key([gc.KeyTuple('$REGISTER_INDEX', register_idx)]))
        # for register_idx in range(CM_BUCKET_COUNT):
        #     self.cm1_reg_batch.append(self.cm1_reg.make_key([gc.KeyTuple('$REGISTER_INDEX', register_idx)]))
        #     self.cm2_reg_batch.append(self.cm2_reg.make_key([gc.KeyTuple('$REGISTER_INDEX', register_idx)]))
        #     self.cm3_reg_batch.append(self.cm3_reg.make_key([gc.KeyTuple('$REGISTER_INDEX', register_idx)]))
        #     self.cm4_reg_batch.append(self.cm4_reg.make_key([gc.KeyTuple('$REGISTER_INDEX', register_idx)]))
        # self.cache_frequency_reg_batch_data = [self.cache_frequency_reg.make_data(
        #                                 [gc.DataTuple(f'{prefix + f".cache_frequency_reg"}.f1', 0)])
        #              ] * KV_BUCKET_COUNT
        # self.cm1_reg_batch_data =  [self.cm1_reg.make_data(
        #                                 [gc.DataTuple(f'{prefix + f".cm1_reg"}.f1', 0)])
        #              ] * CM_BUCKET_COUNT
        # self.cm2_reg_batch_data = [self.cm2_reg.make_data(
        #                                 [gc.DataTuple(f'{prefix + f".cm2_reg"}.f1', 0)])
        #              ] * CM_BUCKET_COUNT
        # self.cm3_reg_batch_data = [self.cm3_reg.make_data(
        #                                 [gc.DataTuple(f'{prefix + f".cm3_reg"}.f1', 0)])
        #              ] * CM_BUCKET_COUNT
        # self.cm4_reg_batch_data = [self.cm4_reg.make_data(
        #                                 [gc.DataTuple(f'{prefix + f".cm4_reg"}.f1', 0)])
        #              ] * CM_BUCKET_COUNT
        # self.cm1_reg =bfrt.netbufferv4.pipe.farreachEgress.cm1_reg
        # self.cm2_reg =bfrt.netbufferv4.pipe.farreachEgress.cm2_reg
        # self.cm3_reg =bfrt.netbufferv4.pipe.farreachEgress.cm3_reg
        # self.cm4_reg =bfrt.netbufferv4.pipe.farreachEgress.cm4_reg
        # self.cache_frequency_reg =bfrt.netbufferv4.pipe.farreachEgress.cache_frequency_reg
        

        self.unmatched_devports = []
        self.recirports_for_unmatched_devports = []
        for client_physical_idx in range(client_physical_num):
            if client_pipeidxes[client_physical_idx] != single_ingress_pipeidx:
                # get devport fro front panel port
                port, chnl = client_fpports[client_physical_idx].split("/")
                # tmp_devport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
                self.unmatched_devports.append(port)
                recirport = pipeline_recirports_tosingle[client_pipeidxes[client_physical_idx]]
                if recirport is None:
                    print("[ERROR] pipeline_recirports_tosingle[{}] is None!").format(client_pipeidxes[client_physical_idx])
                    exit(-1)
                port, chnl = recirport.split("/")
                # recirdevport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
                self.recirports_for_unmatched_devports.append(port)
        # GETRES_LATEST/DELETED_SEQ may also incur CASE1s (need to read snapshot flag)
        for server_physical_idx in range(server_physical_num):
            if server_pipeidxes[server_physical_idx] != single_ingress_pipeidx:
                # get devport fro front panel port
                port, chnl = server_fpports[server_physical_idx].split("/")
                # tmp_devport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
                self.unmatched_devports.append(port)
                recirport = pipeline_recirports_tosingle[server_pipeidxes[server_physical_idx]]
                if recirport is None:
                    print("[ERROR] pipeline_recirports_tosingle[{}] is None!").format(server_pipeidxes[server_physical_idx])
                    exit(-1)
                port, chnl = recirport.split("/")
                # recirdevport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
                self.recirports_for_unmatched_devports.append(port)
        # self.snapshot_flag_tbl = bfrt_info.table_get("farreachIngress.snapshot_flag_tbl")
        # self.need_recirculate_tbl = bfrt_info.table_get("farreachIngress.need_recirculate_tbl")
        # self.case1_reg = bfrt_info.table_get("farreachEgress.case1_reg")
    def cleaner(self):
        print("[cleaner start]")
        while True:
            sleep(self.clean_period)
            # start = time.time()
            self.cm1_reg.entry_del(self.target)
            self.cm2_reg.entry_del(self.target)
            self.cm3_reg.entry_del(self.target)
            self.cm4_reg.entry_del(self.target)
            sleep(self.clean_period)
            self.cache_frequency_reg.entry_del(self.target)
            # end = time.time()
            # print(start - end)

    def add_cache_lookup(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx):
        print("add",freeidx)
        key = self.cache_lookup_tbl.make_key([
            gc.KeyTuple('hdr.op_hdr.keylolo', keylolo),
            gc.KeyTuple('hdr.op_hdr.keylohi', keylohi),
            gc.KeyTuple('hdr.op_hdr.keyhilo', keyhilo),
            gc.KeyTuple('hdr.op_hdr.keyhihilo', keyhihilo),
            gc.KeyTuple('hdr.op_hdr.keyhihihi', keyhihihi),])
        data = self.cache_lookup_tbl.make_data(
            [gc.DataTuple('idx', freeidx)],
            'farreachIngress.cached_action')
        self.cache_lookup_tbl.entry_add(self.target, [key], [data])

    def remove_cache_lookup(self, keylolo, keylohi, keyhilo, keyhihilo, keyhihihi,):
        key = self.cache_lookup_tbl.make_key([
            gc.KeyTuple('hdr.op_hdr.keylolo', keylolo),
            gc.KeyTuple('hdr.op_hdr.keylohi', keylohi),
            gc.KeyTuple('hdr.op_hdr.keyhilo', keyhilo),
            gc.KeyTuple('hdr.op_hdr.keyhihilo', keyhihilo),
            gc.KeyTuple('hdr.op_hdr.keyhihihi', keyhihihi),])
        self.cache_lookup_tbl.entry_del(self.target, [key])
        
    def runpopserver(self):
        print("[ptf.popserver] ready") 
        keys = []
        datas = []
        cnt = 0
        last_target = 10000
        while True:
            # receive control packet
            recvbuf, switchos_addr = switchos_ptf_popserver_udpsock.recvfrom(1024)
            control_type, recvbuf = struct.unpack("=i{}s".format(len(recvbuf) - 4), recvbuf)
            # print(control_type)
            
            if control_type == SWITCHOS_ADD_CACHE_LOOKUP and switchos_addr[0] == reflector_ip_for_switchos:
                keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, recvbuf = struct.unpack("!3I2H{}s".format(len(recvbuf)-16), recvbuf)
                freeidx = struct.unpack("=H", recvbuf)[0]
                keys.append(self.cache_lookup_tbl.make_key([
                    gc.KeyTuple('hdr.op_hdr.keylolo', 0xff),
                    gc.KeyTuple('hdr.op_hdr.keylohi', keylohi),
                    gc.KeyTuple('hdr.op_hdr.keyhilo', keyhilo),
                    gc.KeyTuple('hdr.op_hdr.keyhihilo', keyhihilo),
                    gc.KeyTuple('hdr.op_hdr.keyhihihi', keyhihihi),]))
                datas.append(self.cache_lookup_tbl.make_data(
                    [gc.DataTuple('idx', freeidx)],
                    'farreachIngress.cached_action'))
                cnt +=1
                # print(cnt)
                if freeidx == 199:
                    last_target = 100
                elif freeidx == 1999:
                    last_target = 1000
                elif freeidx == 19999:
                    last_target = 10000
                if cnt == last_target:
                    print(cnt)
                    cnt = 0
                    sendbuf = struct.pack("=i", SWITCHOS_ADD_CACHE_LOOKUP_ACK)
                    switchos_ptf_popserver_udpsock.sendto(sendbuf, switchos_addr)
                    start = time.time()
                    # self.cache_lookup_tbl.entry_add(self.target, keys, datas)
                    end = time.time()
                    print(end - start)
                    
            elif control_type == SWITCHOS_ADD_CACHE_LOOKUP:
                # parse key and freeidx
                keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, recvbuf = struct.unpack("!3I2H{}s".format(len(recvbuf)-16), recvbuf)
                freeidx = struct.unpack("=H", recvbuf)[0]
                
                # keylolo = 0xff if freeidx >= 10000 else keylolo
                # print("debug"freeidx,keylolo)
                self.add_cache_lookup(keylolo, keylohi, keyhilo, keyhihilo, keyhihihi, freeidx)
                sendbuf = struct.pack("=i", SWITCHOS_ADD_CACHE_LOOKUP_ACK)
                switchos_ptf_popserver_udpsock.sendto(sendbuf, switchos_addr)
            elif control_type == SWITCHOS_REMOVE_CACHE_LOOKUP:
                # parse key
                keylolo, keylohi, keyhilo, keyhihilo, keyhihihi = struct.unpack("!3I2H", recvbuf)
                # remove key from cache_lookup_tbl
                self.remove_cache_lookup(keylolo, keylohi, keyhilo, keyhihilo, keyhihihi)
                # send back SWITCHOS_REMOVE_CACHE_LOOKUP_ACK
                sendbuf = struct.pack("=i", SWITCHOS_REMOVE_CACHE_LOOKUP_ACK)
                switchos_ptf_popserver_udpsock.sendto(sendbuf, switchos_addr)
            elif control_type == SWITCHOS_PTF_POPSERVER_END:
                switchos_ptf_popserver_udpsock.close()
                print("[ptf.popserver] END")
                break;
            else:
                print("Invalid control type {}".format(control_type))
                exit(-1)





    def runTest(self):
        cleanert1 = threading.Thread(target=self.cleaner) 
        cleanert1.start() 
        popserver = threading.Thread(target=self.runpopserver) 
        popserver.start() 
        cleanert1.join()
        popserver.join() 
