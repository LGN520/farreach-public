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

from distfarreachspine.p4_pd_rpc.ttypes import *
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

switchos_ptf_snapshotserver_udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
switchos_ptf_snapshotserver_udpsock.bind(("127.0.0.1", switchos_ptf_snapshotserver_port))

flags = distfarreachspine_register_flags_t(read_hw_sync=True)

class RegisterUpdate(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["distfarreachspine"])

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

        # force to recirculate to ingress pipeline of the first physical client fpport
        single_ingress_pipeidx = leafswitch_pipeidx

        self.unmatched_devports = []
        # NOTE: as we only have a single physical leaf switch, we do not have unmatched devports in spine switch
        #for leafswitch_physical_idx in range(leafswitch_physical_num):
        #    if leafswitch_pipeidxes[leafswitch_physical_idx] != single_ingress_pipeidx:
        #        # get devport fro front panel port
        #        port, chnl = spineswitch_fpports_to_leaf[leafswitch_physical_idx].split("/")
        #        tmp_devport = sel.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
        #        self.unmatched_devports.append(tmp_devport)

    def cleanup(self):
        print "Reset need_recirculate=0 for iports in different ingress pipelines"
        # get entry count
        entrynum = self.client.need_recirculate_tbl_get_entry_count(self.sess_hdl, self.dev_tgt)
        if entrynum > 0:
            for iport in self.unmatched_devports:
                for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_SEQ, DELREQ_SEQ, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER, PUTREQ_LARGEVALUE, PUTREQ_LARGEVALUE_SEQ]:
                    matchspec0 = distfarreachspine_need_recirculate_tbl_match_spec_t(\
                            op_hdr_optype = tmpoptype,
                            ig_intr_md_ingress_port = iport)
                    self.client.need_recirculate_tbl_table_delete_by_match_spec(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
        print "Reset snapshot_flag=0 for all ingress pipelines"
        entrynum = self.client.snapshot_flag_tbl_get_entry_count(self.sess_hdl, self.dev_tgt)
        if entrynum > 0:
            for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_SEQ, DELREQ_SEQ, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER, PUTREQ_LARGEVALUE_SEQ]:
                matchspec0 = distfarreachspine_snapshot_flag_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_need_recirculate = 0)
                self.client.snapshot_flag_tbl_table_delete_by_match_spec(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
        print "Reset case1_reg for all pipelines"
        self.client.register_reset_all_case1_reg(self.sess_hdl, self.dev_tgt)

    def enable_singlepath(self):
        print "Set need_recirculate=1 for iports in different ingress pipelines"
        for iport in self.unmatched_devports:
            for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_SEQ, DELREQ_SEQ, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER, PUTREQ_LARGEVALUE, PUTREQ_LARGEVALUE_SEQ]:
                matchspec0 = distfarreachspine_need_recirculate_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        ig_intr_md_ingress_port = iport)
                self.client.need_recirculate_tbl_table_add_with_set_need_recirculate(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

    def set_snapshot_flag(self):
        print "Set snapshot_flag=1 for all ingress pipelines"
        for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_SEQ, DELREQ_SEQ, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER, PUTREQ_LARGEVALUE_SEQ]:
            matchspec0 = distfarreachspine_snapshot_flag_tbl_match_spec_t(\
                    op_hdr_optype = tmpoptype,
                    meta_need_recirculate = 0)
            #self.client.snapshot_flag_tbl_table_delete_by_match_spec(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.snapshot_flag_tbl_table_add_with_set_snapshot_flag(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

    def disable_singlepath(self):
        print "Reset need_recirculate=0 for iports in different ingress pipelines"
        for iport in self.unmatched_devports:
            for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_SEQ, DELREQ_SEQ, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER, PUTREQ_LARGEVALUE, PUTREQ_LARGEVALUE_SEQ]:
                matchspec0 = distfarreachspine_need_recirculate_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        ig_intr_md_ingress_port = iport)
                self.client.need_recirculate_tbl_table_delete_by_match_spec(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

    def load_snapshot_data(self, cached_empty_index_backup, pipeidx):
        print "[ERROR] now we directly load snapshot data from data plane instead of via ptf channel"
        exit(-1)

#        tmp_devtgt = DevTarget_t(0, pipeidx)
#
#        start_index = 0
#        end_index = cached_empty_index_backup - 1
#        print "Load snapshot data in [{}, {}] from data plane's pipeline {}".format(start_index, end_index, pipeidx)
#        record_cnt = end_index - start_index + 1
#        # TODO: switchos should give per-pipeline empty index to support multi-pipeline
#        egress_pipeidx = 0
#        vallen_list = self.client.register_range_read_vallen_reg(self.sess_hdl, tmp_devtgt, start_index, record_cnt, flags)[0 * record_cnt:0 * record_cnt + record_cnt]
#        for i in range(len(vallen_list)):
#            #vallen_list[i] = convert_i32_to_u32(vallen_list[i])
#            vallen_list[i] = convert_i16_to_u16(vallen_list[i])
#        vallo_list_list = []
#        valhi_list_list = []
#        for i in range(switch_max_vallen/8): # 128 bytes / 8 = 16 register arrays
#            vallo_list_list.append(eval("self.client.register_range_read_vallo{}_reg".format(i+1))(self.sess_hdl, tmp_devtgt, 0, record_cnt, flags)[0 * record_cnt:0 * record_cnt + record_cnt])
#            valhi_list_list.append(eval("self.client.register_range_read_valhi{}_reg".format(i+1))(self.sess_hdl, tmp_devtgt, 0, record_cnt, flags)[0 * record_cnt:0 * record_cnt + record_cnt])
#        deleted_list = self.client.register_range_read_deleted_reg(self.sess_hdl, tmp_devtgt, start_index, record_cnt, flags)[0 * record_cnt:0 * record_cnt + record_cnt]
#        stat_list = []
#        for i in range(len(deleted_list)):
#            if deleted_list[i] == 0:
#                stat_list.append(True)
#            elif deleted_list[i] == 1:
#                stat_list.append(False)
#            else:
#                print "Invalid deleted_list[{}]: {}".format(i, deleted_list[i])
#                exit(-1)
#        savedseq_list = self.client.register_range_read_savedseq_reg(self.sess_hdl, tmp_devtgt, start_index, record_cnt, flags)[0 * record_cnt:0 * record_cnt + record_cnt]
#        for i in range(len(savedseq_list)):
#            savedseq_list[i] = convert_i32_to_u32(savedseq_list[i])
#
#        print "Prepare sendbuf to switchos.snapshotdataserver"
#        sendbuf = bytes()
#        # <SWITCHOS_LOAD_SNAPSHOT_DATA_ACK, total_bytesnum, records> -> for each record: <uint16_t vallen (big-endian for Val::deserialize), valbytes (same order), uint32_t seq (little-endian), result>
#        for i in range(record_cnt):
#            tmpvallen = vallen_list[i]
#            #sendbuf = sendbuf + struct.pack("!I", tmpvallen)
#            sendbuf = sendbuf + struct.pack("!H", tmpvallen)
#            tmp_eightbyte_cnt = (tmpvallen + 7) / 8
#            for j in range(tmp_eightbyte_cnt):
#                # NOTE: we serialize each 4B value as big-endian to keep the same byte order as end-hosts
#                # NOTE: deparser valbytes from val16 to val1
#                sendbuf = sendbuf + struct.pack("!2i", vallo_list_list[tmp_eightbyte_cnt-1-j][i], valhi_list_list[tmp_eightbyte_cnt-1-j][i])
#            sendbuf = sendbuf + struct.pack("=I?", savedseq_list[i], stat_list[i])
#        total_bytesnum = 4 + 4 + len(sendbuf) # total # of bytes in sendbuf including total_bytesnum itself
#        sendbuf = struct.pack("=2i", SWITCHOS_LOAD_SNAPSHOT_DATA_ACK, total_bytesnum) + sendbuf
#        return sendbuf

    def reset_snapshot_flag_and_reg(self):
        print "Reset snapshot_flag=0 for all ingress pipelines"
        for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, PUTREQ_SEQ, DELREQ_SEQ, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER, PUTREQ_LARGEVALUE_SEQ]:
            matchspec0 = distfarreachspine_snapshot_flag_tbl_match_spec_t(\
                    op_hdr_optype = tmpoptype,
                    meta_need_recirculate = 0)
            self.client.snapshot_flag_tbl_table_delete_by_match_spec(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

        print "Reset case1_reg"
        self.client.register_reset_all_case1_reg(self.sess_hdl, self.dev_tgt)

    # Refer to udpsendlarge_udpfrag in socket_helper.c
    def send_snapshotdata(self, udpsock, buf, dstaddr):
        print "Send snapshot data to switchos.snapshotserver"
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
            fragbuf = fragbuf + buf[buf_sentsize:buf_sentsize + cur_fragbodysize]
            buf_sentsize += cur_fragbodysize
            udpsock.sendto(fragbuf, dstaddr)

    def runTest(self):
        #with_switchos_addr = False
        print "[ptf.snapshotserver] ready"

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
                self.set_snapshot_flag()

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
                    print "Invalid cached_empty_index_backup: {}".format(cached_empty_index_backup)
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

        self.conn_mgr.complete_operations(self.sess_hdl)
        self.conn_mgr.client_cleanup(self.sess_hdl) # close session
