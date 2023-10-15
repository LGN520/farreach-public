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

from netbufferv4.p4_pd_rpc.ttypes import *
from pltfm_pm_rpc.ttypes import *
from pal_rpc.ttypes import *
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from res_pd_rpc.ttypes import *

import socket
import struct

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *

switchos_ptf_snapshotserver_tcpsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
switchos_ptf_snapshotserver_tcpsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
switchos_ptf_snapshotserver_tcpsock.bind(("127.0.0.1", switchos_ptf_snapshotserver_port))
switchos_ptf_snapshotserver_tcpsock.listen(1) # MAXIMUM_PENDING_CONNECTION = 1

flags = netbufferv4_register_flags_t(read_hw_sync=True)

port_pipeidx_map = {} # mapping between port and pipeline
pipeidx_ports_map = {} # mapping between pipeline and ports

class RegisterUpdate(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbufferv4"])

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

        # get the device ports from front panel ports
        self.devPorts = []
        for fpPort in fp_ports:
            port, chnl = fpPort.split("/")
            devPort = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
            self.devPorts.append(devPort)

        port_pipeidx_map[self.devPorts[0]] = ingress_pipeidx
        port_pipeidx_map[self.devPorts[1]] = egress_pipeidx
        pipeidx_ports_map[ingress_pipeidx] = [self.devPorts[0]]
        if egress_pipeidx not in pipeidx_ports_map:
            pipeidx_ports_map[egress_pipeidx] = [self.devPorts[1]]
        else:
            if self.devPorts[1] not in pipeidx_ports_map[egress_pipeidx]:
                pipeidx_ports_map[egress_pipeidx].append(self.devPorts[1])

    def set_snapshot_flag(self):
        print "Set need_recirculate=1 for iports in different ingress pipelines"
        for tmppipeidx in pipeidx_ports_map.keys():
            if tmppipeidx != ingress_pipeidx:
                tmpports = pipeidx_ports_map[tmppipeidx]
                for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
                    for iport in tmpports:
                        matchspec0 = netbufferv4_need_recirculate_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                ig_intr_md_ingress_port = iport)
                        self.client.need_recirculate_tbl_table_add_with_set_need_recirculate(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

        print "Set snapshot_flag=1 for all ingress pipelines"
        for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
            matchspec0 = netbufferv4_snapshot_flag_tbl_match_spec_t(\
                    op_hdr_optype = tmpoptype,
                    meta_need_recirculate = 0)
            #self.client.snapshot_flag_tbl_table_delete_by_match_spec(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.snapshot_flag_tbl_table_add_with_set_snapshot_flag(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

        print "Reset need_recirculate=0 for iports in different ingress pipelines"
        for tmppipeidx in pipeidx_ports_map.keys():
            if tmppipeidx != ingress_pipeidx:
                tmpports = pipeidx_ports_map[tmppipeidx]
                for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
                    for iport in tmpports:
                        matchspec0 = netbufferv4_need_recirculate_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                ig_intr_md_ingress_port = iport)
                        self.client.need_recirculate_tbl_table_delete_by_match_spec(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

    def load_snapshot_data(self, cached_empty_index_backup):
        start_index = 0
        end_index = cached_empty_index_backup - 1
        print "Load snapshot data in [{}, {}] from data plane".format(start_index, end_index)
        record_cnt = end_index - start_index + 1
        # TODO: check len of vallen_list
        vallen_list = self.client.register_range_read_vallen_reg(self.sess_hdl, self.dev_tgt, start_index, record_cnt, flags)[egress_pipeidx * record_cnt:egress_pipeidx * record_cnt + record_cnt]
        for i in range(len(vallen_list)):
            #vallen_list[i] = convert_i32_to_u32(vallen_list[i])
            vallen_list[i] = convert_i16_to_u16(vallen_list[i])
        vallo_list_list = []
        valhi_list_list = []
        for i in range(switch_max_vallen/8): # 128 bytes / 8 = 16 register arrays
            vallo_list_list.append(eval("self.client.register_range_read_vallo{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, 0, record_cnt, flags)[egress_pipeidx * record_cnt:egress_pipeidx * record_cnt + record_cnt])
            valhi_list_list.append(eval("self.client.register_range_read_valhi{}_reg".format(i+1))(self.sess_hdl, self.dev_tgt, 0, record_cnt, flags)[egress_pipeidx * record_cnt:egress_pipeidx * record_cnt + record_cnt])
        deleted_list = self.client.register_range_read_deleted_reg(self.sess_hdl, self.dev_tgt, start_index, record_cnt, flags)[egress_pipeidx * record_cnt:egress_pipeidx * record_cnt + record_cnt]
        stat_list = []
        for i in range(len(deleted_list)):
            if deleted_list[i] == 0:
                stat_list.append(True)
            elif deleted_list[i] == 1:
                stat_list.append(False)
            else:
                print "Invalid deleted_list[{}]: {}".format(i, deleted_list[i])
                exit(-1)
        savedseq_list = self.client.register_range_read_savedseq_reg(self.sess_hdl, self.dev_tgt, start_index, record_cnt, flags)[egress_pipeidx * record_cnt:egress_pipeidx * record_cnt + record_cnt]
        for i in range(len(savedseq_list)):
            savedseq_list[i] = convert_i32_to_u32(savedseq_list[i])

        print "Prepare sendbuf to switchos.snapshotdataserver"
        sendbuf = bytes()
        # <SWITCHOS_LOAD_SNAPSHOT_DATA_ACK, total_bytesnum, records> -> for each record: <uint16_t vallen (big-endian for Val::deserialize), valbytes (same order), uint32_t seq (little-endian), result>
        for i in range(record_cnt):
            tmpvallen = vallen_list[i]
            #sendbuf = sendbuf + struct.pack("!I", tmpvallen)
            sendbuf = sendbuf + struct.pack("!H", tmpvallen)
            tmp_eightbyte_cnt = (tmpvallen + 7) / 8
            for j in range(tmp_eightbyte_cnt):
                # NOTE: we serialize each 4B value as big-endian to keep the same byte order as end-hosts
                # NOTE: deparser valbytes from val16 to val1
                sendbuf = sendbuf + struct.pack("!2i", vallo_list_list[tmp_eightbyte_cnt-1-j][i], valhi_list_list[tmp_eightbyte_cnt-1-j][i])
            sendbuf = sendbuf + struct.pack("=I?", savedseq_list[i], stat_list[i])
        total_bytesnum = 4 + 4 + len(sendbuf) # total # of bytes in sendbuf including total_bytesnum itself
        sendbuf = struct.pack("=2i", SWITCHOS_LOAD_SNAPSHOT_DATA_ACK, total_bytesnum) + sendbuf
        return sendbuf

    def reset_snapshot_flag_and_reg(self):
        print "Reset snapshot_flag=0 for all ingress pipelines"
        for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
            matchspec0 = netbufferv4_snapshot_flag_tbl_match_spec_t(\
                    op_hdr_optype = tmpoptype,
                    meta_need_recirculate = 0)
            self.client.snapshot_flag_tbl_table_delete_by_match_spec(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

        print "Reset case1_reg"
        self.client.register_reset_all_case1_reg(self.sess_hdl, self.dev_tgt)

    def runTest(self):
        print "[ptf.snapshotserver] ready"
        connfd, switchos_addr = switchos_ptf_snapshotserver_tcpsock.accept()

        control_type = -1
        recvbuf = bytes()
        while True:
            # receive control packet
            tmp_recvbuf = connfd.recv(1024)
            recvbuf = recvbuf + tmp_recvbuf

            if control_type == -1 and len(recvbuf) >= 4:
                control_type, recvbuf = struct.unpack("=i{}s".format(len(recvbuf) - 4), recvbuf)

            if control_type != -1:
                if control_type == SWITCHOS_SET_SNAPSHOT_FLAG:
                    # set snapshot_flag as true atomically
                    self.set_snapshot_flag()

                    # send back SWITCHOS_SET_SNAPSHOT_FLAG_ACK
                    sendbuf = struct.pack("=i", SWITCHOS_SET_SNAPSHOT_FLAG_ACK)
                    connfd.sendall(sendbuf)

                    control_type = -1
                elif control_type == SWITCHOS_LOAD_SNAPSHOT_DATA:
                    if len(recvbuf) >= 4:
                        # parse empty index
                        cached_empty_index_backup, recvbuf = struct.unpack("=I{}s".format(len(recvbuf) - 4), recvbuf) # must > 0
                        if cached_empty_index_backup <= 0 or cached_empty_index_backup > kv_bucket_num:
                            print "Invalid cached_empty_index_backup: {}".format(cached_empty_index_backup)
                            exit(-1)

                        # load snapshot data from data plane
                        sendbuf = self.load_snapshot_data(cached_empty_index_backup)

                        # send back snapshot data
                        connfd.sendall(sendbuf)

                        control_type = -1
                elif control_type == SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG:
                    # reset snapshot_flag and case1_reg
                    self.reset_snapshot_flag_and_reg()

                    # send back SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG_ACK
                    sendbuf = struct.pack("=i", SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG_ACK)
                    connfd.sendall(sendbuf)

                    control_type = -1
                elif control_type == SWITCHOS_PTF_SNAPSHOTSERVER_END:
                    print("[ptf.snapshotserver] END")
                    break;
                else:
                    print("Invalid control type {}".format(control_type))
                    exit(-1)

        self.conn_mgr.complete_operations(self.sess_hdl)
        self.conn_mgr.client_cleanup(self.sess_hdl) # close session
