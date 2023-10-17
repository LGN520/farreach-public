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
from p4utils.utils.sswitch_thrift_API import SimpleSwitchThriftAPI
controller = SimpleSwitchThriftAPI(9090, "192.168.122.229") 
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

switchos_ptf_snapshotserver_udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
switchos_ptf_snapshotserver_udpsock.bind(("127.0.0.1", switchos_ptf_snapshotserver_port))



class RegisterUpdate():

    def setUp(self):
        print('\nSetup')



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

    def cleanup(self):
        print("Reset need_recirculate=0 for iports in different ingress pipelines")
        # get entry count
        
        entrynum = controller.table_num_entries('need_recirculate_tbl')
        if entrynum > 0:
            for i in range(len(self.unmatched_devports)):
                iport = self.unmatched_devports[i]
                for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_LARGEVALUE]:
                    matchspec0 = [
                            hex(tmpoptype),
                            hex(iport)]
                    controller.table_delete_match('need_recirculate_tbl', matchspec0)
        print("Reset snapshot_flag=0 for all ingress pipelines")
        entrynum = controller.table_num_entries('snapshot_flag_tbl')
        
        if entrynum > 0:
            for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_LARGEVALUE]:
                matchspec0 = [
                        hex(tmpoptype),
                        hex(0)]
                controller.table_delete_match('snapshot_flag_tbl',matchspec0)

        print("Reset case1_reg for all pipelines")
        controller.register_reset('case1_reg')

    def enable_singlepath(self):
        print("Set need_recirculate=1 for iports in different ingress pipelines")
        for i in range(len(self.unmatched_devports)):
            iport = self.unmatched_devports[i]
            recirport = self.recirports_for_unmatched_devports[i]
            for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_LARGEVALUE]:
                matchspec0 = [
                        hex(tmpoptype),
                        hex(iport)]
                actnspec0 = [recirport]
                controller.table_add('need_recirculate_tbl','set_need_recirculate',matchspec0, actnspec0)

    def set_snapshot_flag(self, snapshotid):
        print("Set snapshot_flag=1 for all ingress pipelines")
        for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_LARGEVALUE]:
            matchspec0 = [
                    hex(tmpoptype),
                    hex(0)]
            #self.client.snapshot_flag_tbl_table_delete_by_match_spec(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            actnspec0 = [
                    hex(snapshotid)]
            controller.table_add('snapshot_flag_tbl','set_snapshot_flag',matchspec0, actnspec0)

    def disable_singlepath(self):
        # get entry count
        entrynum = controller.table_num_entries('need_recirculate_tbl')

        if entrynum > 0:
            print("Reset need_recirculate=0 for iports in different ingress pipelines")
            for i in range(len(self.unmatched_devports)):
                iport = self.unmatched_devports[i]
                for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_LARGEVALUE]:
                    matchspec0 = [
                            hex(tmpoptype),
                            hex(iport)]
                    controller.table_delete_match('need_recirculate_tbl', matchspec0)

    def load_snapshot_data(self, cached_empty_index_backup, pipeidx):
        print("[ERROR] now we directly load snapshot data from data plane instead of via ptf channel")
        exit(-1)


    def reset_snapshot_flag_and_reg(self):
        entrynum = controller.table_num_entries('snapshot_flag_tbl')
        if entrynum > 0:
            print("Reset snapshot_flag=0 for all ingress pipelines")
            for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_LARGEVALUE]:
                matchspec0 = [
                        hex(tmpoptype),
                        hex(0)]
                controller.table_delete_match('snapshot_flag_tbl', matchspec0)


        print("Reset case1_reg")
        controller.register_reset('case1_reg')

    # Refer to udpsendlarge_udpfrag in socket_helper.c
    def send_snapshotdata(self, udpsock, buf, dstaddr):
        print("Send snapshot data to switchos.snapshotserver")
        frag_hdrsize = 0
        frag_maxsize = 65507 # 65536(ipmax) - 20(iphdr) - 8(udphdr)
        final_frag_hdrsize = frag_hdrsize + 2 + 2 # + cur_fragidx + max_fragnum
        frag_bodysize = frag_maxsize - final_frag_hdrsize
        total_bodysize = len(buf) - frag_hdrsize
        fragnum = 1
        if total_bodysize > 0:
            fragnum = (total_bodysize + frag_bodysize - 1) / frag_bodysize

        buf_sentsize = 0 # NOTE: frag_hdrsize = 0 for snapshot data
        for cur_fragidx in range(fragnum):
            fragbuf = struct.pack("=2H", cur_fragidx, fragnum)
            cur_fragbodysize = frag_bodysize
            if cur_fragidx == fragnum - 1:
                cur_fragbodysize = total_bodysize - frag_bodysize * cur_fragidx
            buf_sentsize =buf_sentsize + cur_fragbodysize
            fragbuf = fragbuf + buf[buf_sentsize]
            buf_sentsize += cur_fragbodysize
            udpsock.sendto(fragbuf, dstaddr)

    def runTest(self):
        #with_switchos_addr = False
        print("[ptf.snapshotserver] ready")

        control_type = -1
        recvbuf = bytes()
        while True:
            #if with_switchos_addr == False:
            #    recvbuf, switchos_addr = switchos_ptf_snapshotserver_udpsock.recvfrom(1024)
            #    with_switchos_addr = True
            #else:
            #    recvbuf, _ = switchos_ptf_snapshotserver_udpsock.recvfrom(1024)
            recvbuf, switchos_addr = switchos_ptf_snapshotserver_udpsock.recvfrom(1024)

            control_type, recvbuf = struct.unpack("=i{}s".format(len(recvbuf) - 4), recvbuf)

            if control_type == SWITCHOS_CLEANUP:
                self.cleanup()

                # send back SWITCHOS_CLEANUP_ACK
                sendbuf = struct.pack("=i", SWITCHOS_CLEANUP_ACK)
                switchos_ptf_snapshotserver_udpsock.sendto(sendbuf, switchos_addr)
            elif control_type == SWITCHOS_ENABLE_SINGLEPATH:
                self.enable_singlepath()

                # send back SWITCHOS_ENABLE_SINGLEPATH_ACK
                sendbuf = struct.pack("=i", SWITCHOS_ENABLE_SINGLEPATH_ACK)
                switchos_ptf_snapshotserver_udpsock.sendto(sendbuf, switchos_addr)
            elif control_type == SWITCHOS_SET_SNAPSHOT_FLAG:
                # set snapshot_flag as true atomically
                snapshotid = struct.unpack("=I", recvbuf)[0]
                self.set_snapshot_flag(snapshotid)

                # send back SWITCHOS_SET_SNAPSHOT_FLAG_ACK
                sendbuf = struct.pack("=i", SWITCHOS_SET_SNAPSHOT_FLAG_ACK)
                switchos_ptf_snapshotserver_udpsock.sendto(sendbuf, switchos_addr)
            elif control_type == SWITCHOS_DISABLE_SINGLEPATH:
                self.disable_singlepath()

                # send back SWITCHOS_DISABLE_SINGLEPATH_ACK
                sendbuf = struct.pack("=i", SWITCHOS_DISABLE_SINGLEPATH_ACK)
                switchos_ptf_snapshotserver_udpsock.sendto(sendbuf, switchos_addr)
            elif control_type == SWITCHOS_LOAD_SNAPSHOT_DATA:
                # parse empty index
                cached_empty_index_backup, pipeidx = struct.unpack("=2I", recvbuf) # must > 0
                if cached_empty_index_backup <= 0 or cached_empty_index_backup > switch_kv_bucket_num:
                    print("Invalid cached_empty_index_backup: {}").format(cached_empty_index_backup)
                    exit(-1)

                # load snapshot data from data plane
                sendbuf = self.load_snapshot_data(cached_empty_index_backup, pipeidx)

                # send back snapshot data
                self.send_snapshotdata(switchos_ptf_snapshotserver_udpsock, sendbuf, switchos_addr)
            elif control_type == SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG:
                # reset snapshot_flag and case1_reg
                self.reset_snapshot_flag_and_reg()

                # send back SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG_ACK
                sendbuf = struct.pack("=i", SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG_ACK)
                switchos_ptf_snapshotserver_udpsock.sendto(sendbuf, switchos_addr)
            elif control_type == SWITCHOS_PTF_SNAPSHOTSERVER_END:
                switchos_ptf_snapshotserver_udpsock.close()
                print("[ptf.snapshotserver] END")
                break;
            else:
                print("Invalid control type {}".format(control_type))
                exit(-1)

registerupdate = RegisterUpdate()
registerupdate.setUp()
registerupdate.runTest()