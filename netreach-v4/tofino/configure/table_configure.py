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
from mirror_pd_rpc.ttypes import *
from pal_rpc.ttypes import *
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from res_pd_rpc.ttypes import *
from conn_mgr_pd_rpc.ttypes import *
from mc_pd_rpc.ttypes import *
from devport_mgr_pd_rpc.ttypes import *
from ptf_port import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *

port_pipeidx_map = {} # mapping between port and pipeline
pipeidx_ports_map = {} # mapping between pipeline and ports

cached_list = [0, 1]
hot_list = [0, 1]
validvalue_list = [0, 1, 3]
#validvalue_list = [0, 1, 2, 3] # If with PUTREQ_LARGE
latest_list = [0, 1]
deleted_list = [0, 1]
#wrong_pipeline_list = [0, 1]
sampled_list = [0, 1]
lastclone_list = [0, 1]
snapshot_flag_list = [0, 1]
case1_list = [0, 1]


if test_param_get("arch") == "tofino":
  MIR_SESS_COUNT = 1024
  MAX_SID_NORM = 1015
  MAX_SID_COAL = 1023
  BASE_SID_NORM = 1
  BASE_SID_COAL = 1016
  EXP_LEN1 = 127
  EXP_LEN2 = 63
elif test_param_get("arch") == "tofino2":
  MIR_SESS_COUNT = 256
  MAX_SID_NORM = 255
  MAX_SID_COAL = 255
  BASE_SID_NORM = 0
  BASE_SID_COAL = 0
  EXP_LEN1 = 127
  EXP_LEN2 = 59
else:
  assert False, "Unsupported arch %s" % test_param_get("arch")

def mirror_session(mir_type, mir_dir, sid, egr_port=0, egr_port_v=False,
                   egr_port_queue=0, packet_color=0, mcast_grp_a=0,
                   mcast_grp_a_v=False, mcast_grp_b=0, mcast_grp_b_v=False,
                   max_pkt_len=0, level1_mcast_hash=0, level2_mcast_hash=0,
                   mcast_l1_xid=0, mcast_l2_xid=0, mcast_rid=0, cos=0, c2c=False, extract_len=0, timeout=0,
                   int_hdr=[], hdr_len=0):
  return MirrorSessionInfo_t(mir_type,
                             mir_dir,
                             sid,
                             egr_port,
                             egr_port_v,
                             egr_port_queue,
                             packet_color,
                             mcast_grp_a,
                             mcast_grp_a_v,
                             mcast_grp_b,
                             mcast_grp_b_v,
                             max_pkt_len,
                             level1_mcast_hash,
                             level2_mcast_hash,
                             mcast_l1_xid,
                             mcast_l2_xid,
                             mcast_rid,
                             cos,
                             c2c,
                             extract_len,
                             timeout,
                             int_hdr,
                             hdr_len)

class TableConfigure(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbufferv4"])

    def configure_update_val_tbl(self, valname):
        # size: 30
        for is_cached in cached_list:
            for validvalue in validvalue_list:
                for is_latest in latest_list:
                    matchspec0 = eval("netbufferv4_update_val{}_tbl_match_spec_t".format(valname))(
                            op_hdr_optype = GETREQ_INSWITCH,
                            inswitch_hdr_is_cached = is_cached,
                            meta_validvalue = validvalue,
                            meta_is_latest = is_latest)
                    if is_cached == 1:
                        eval("self.client.update_val{}_tbl_table_add_with_get_val{}".format(valname, valname))(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                    for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                        matchspec0 = eval("netbufferv4_update_val{}_tbl_match_spec_t".format(valname))(
                                op_hdr_optype = tmpoptype,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        if is_cached == 1 and validvalue == 1 and is_latest == 0:
                            eval("self.client.update_val{}_tbl_table_add_with_set_and_get_val{}".format(valname, valname))(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                    matchspec0 = eval("netbufferv4_update_val{}_tbl_match_spec_t".format(valname))(
                            op_hdr_optype = PUTREQ_INSWITCH,
                            inswitch_hdr_is_cached = is_cached,
                            meta_validvalue = validvalue,
                            meta_is_latest = is_latest)
                    if is_cached == 1 and validvalue == 1:
                        eval("self.client.update_val{}_tbl_table_add_with_set_and_get_val{}".format(valname, valname))(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                    matchspec0 = eval("netbufferv4_update_val{}_tbl_match_spec_t".format(valname))(
                            op_hdr_optype = DELREQ_INSWITCH,
                            inswitch_hdr_is_cached = is_cached,
                            meta_validvalue = validvalue,
                            meta_is_latest = is_latest)
                    if is_cached == 1 and validvalue == 1:
                        eval("self.client.update_val{}_tbl_table_add_with_reset_and_get_val{}".format(valname, valname))(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                    matchspec0 = eval("netbufferv4_update_val{}_tbl_match_spec_t".format(valname))(
                            op_hdr_optype = CACHE_POP_INSWITCH,
                            inswitch_hdr_is_cached = is_cached,
                            meta_validvalue = validvalue,
                            meta_is_latest = is_latest)
                    eval("self.client.update_val{}_tbl_table_add_with_set_and_get_val{}".format(valname, valname))(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

    def setUp(self):
        print '\nSetup'

        # initialize the connection
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.sess_hdl = self.conn_mgr.client_init()
        self.dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        self.devPorts = []

        self.platform_type = "mavericks"
        board_type = self.pltfm_pm.pltfm_pm_board_type_get()
        if re.search("0x0234|0x1234|0x4234|0x5234", hex(board_type)):
            self.platform_type = "mavericks"
        elif re.search("0x2234|0x3234", hex(board_type)):
            self.platform_type = "montara"

        # get the device ports from front panel ports
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

        self.recirPorts = [64, 192]

        # NOTE: in each pipeline, 64-67 are recir/cpu ports, 68-71 are recir/pktgen ports
        #self.cpuPorts = [64, 192] # CPU port is 100G

        sidnum = 2
        self.sids = random.sample(xrange(BASE_SID_NORM, MAX_SID_NORM), sidnum)

    ### MAIN ###

    def runTest(self):
        if test_param_get('cleanup') != True:
            print '\nTest'

            #####################
            ### Prepare ports ###
            #####################

            # Remove special ports
            for i in range(64, 72):
                try:
                    self.devport_mgr.devport_mgr_remove_port(0, i)
                except InvalidDevportMgrOperation as e:
                    pass
            for i in range(192, 200):
                try:
                    self.devport_mgr.devport_mgr_remove_port(0, i)
                except InvalidDevportMgrOperation as e:
                    pass

            # Enable recirculation before add special ports
            for i in self.recirPorts:
                self.conn_mgr.recirculation_enable(self.sess_hdl, 0, i);

            # Add and enable the platform ports
            for i in self.devPorts:
               self.pal.pal_port_add(0, i,
                                     pal_port_speed_t.BF_SPEED_40G,
                                     pal_fec_type_t.BF_FEC_TYP_NONE)
               self.pal.pal_port_enable(0, i)

            # Add special ports
            speed_10g = 2
            speed_25g = 4
            speed_40g = 8
            speed_40g_nb = 16
            speed_50g = 32
            speed_100g = 64
            for i in self.recirPorts:
               self.devport_mgr.devport_mgr_add_port(0, i, speed_100g, 0)
            #for i in self.cpuPorts:
            #    self.devport_mgr.devport_mgr_set_copy_to_cpu(0, True, i)

            # Bind sid with platform port for packet mirror
            print "Binding sid {} with port {}".format(self.sids[0], self.devPorts[0]) # clone to client
            info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
                                  Direction_e.PD_DIR_INGRESS,
                                  self.sids[0],
                                  self.devPorts[0],
                                  True)
            print "Binding sid {} with port {}".format(self.sids[1], self.devPorts[1]) # clone to server
            info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
                                  Direction_e.PD_DIR_INGRESS,
                                  self.sids[1],
                                  self.devPorts[1],
                                  True)
            self.mirror.mirror_session_create(self.sess_hdl, self.dev_tgt, info)


            ################################
            ### Normal MAT Configuration ###
            ################################

            # Ingress pipeline

            # Stage 0
            
            # Table: need_recirculate_tbl (default: reset_need_recirculate; size: <=8)
            #print "Configuring need_recirculate_tbl"
            #for tmpoptype in [GETREQ, PUTREQ, DELREQ]:
            #    for iport in self.devPorts:
            #        matchspec0 = netbufferv4_need_recirculate_tbl_match_spec_t(\
            #                op_hdr_optype = tmpoptype,
            #                ig_intr_md_ingress_port = iport)
            #        if (tmpoptype == GETREQ) or (iport in pipeidx_ports_map[ingress_pipeidx]):
            #            self.client.need_recirculate_tbl_table_add_with_reset_need_recirculate(\
            #                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 1

            # Table: recirculate_tbl (default: nop; size: 2)
            print "Configuring recirculate_tbl"
            for tmpoptype in [PUTREQ, DELREQ]:
                matchspec0 = netbufferv4_recirculate_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_need_recirculate = 1)
                actnspec0 = netbufferv4_recirculate_pkt_action_spec_t(self.recirPorts[ingress_pipeidx])
                self.client.recirculate_tbl_table_add_with_recirculate_pkt(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Stage 1

            if RANGE_SUPPORT:
                # # Table: range_partition_tbl (default: reset_is_wrong_pipeline; size <= 8 * 128)
                # Table: range_partition_tbl (default: nop; size <= 8 * 128)
                print "Configuring range_partition_tbl"
                #key_range_per_server = pow(2, 32) / server_num
                key_range_per_server = pow(2, 16) / server_num
                for tmpoptype in [GETREQ, CACHE_POP_INSWITCH, PUTREQ, DELREQ]:
                    for iport in self.devPorts:
                        ##key_start = -pow(2, 31) # [-2^31, 2^31-1]
                        #key_start = 0 # [0, 2^32-1]
                        key_start = 0 # [0, 2^16-1]
                        for i in range(server_num):
                            if i == server_num - 1:
                                ##key_end = pow(2, 31) - 1 # if end is not included, then it is just processed by port 1111
                                #key_end = pow(2, 32) - 1
                                key_end = pow(2, 16) - 1
                            else:
                                key_end = key_start + key_range_per_server - 1
                            # NOTE: both start and end are included
                            matchspec0 = netbufferv4_range_partition_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    #op_hdr_keyhihi_start = convert_u32_to_i32(key_start),
                                    #op_hdr_keyhihi_end = convert_u32_to_i32(key_end),
                                    op_hdr_keyhihihi_start = convert_u16_to_i16(key_start),
                                    op_hdr_keyhihihi_end = convert_u16_to_i16(key_end),
                                    ig_intr_md_ingress_port = iport,
                                    meta_need_recirculate = 0)
                            # Forward to the egress pipeline of server
                            eport = self.devPorts[1]
                            #if port_pipeidx_map[iport] == port_pipeidx_map[eport]: # in correct pipeline
                            #    actnspec0 = netbufferv4_hash_partition_action_spec_t(\
                            #            server_port + i, eport, 0)
                            #else: # in wrong pipeline
                            #    actnspec0 = netbufferv4_hash_partition_action_spec_t(\
                            #            server_port + i, eport, 1)
                            actnspec0 = netbufferv4_hash_partition_action_spec_t(\
                                    server_port + i, eport)
                            self.client.range_partition_tbl_table_add_with_range_partition(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                            key_start = key_end + 1
            else:
                # Table: hash_for_partition_tbl (default: nop; size: 4)
                print "Configuring hash_for_partition_tbl"
                for tmpoptype in [GETREQ, CACHE_POP_INSWITCH, PUTREQ, DELREQ]:
                    matchspec0 = netbufferv4_hash_for_partition_tbl_match_spec_t(\
                            op_hdr_optype = tmpoptype,
                            meta_need_recirculate = 0)
                    self.client.hash_for_partition_tbl_table_add_with_hash_for_partition(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 2

            if RANGE_SUPPORT:
                # Table: range_partition_for_scan_tbl (default: nop; size <= 2 * 128)
                # TODO: limit max_scannum <= constant (e.g., 32)
                print "Configuring range_partition_for_scan_tbl"
                #key_range_per_server = pow(2, 32) / server_num
                key_range_per_server = pow(2, 16) / server_num
                for iport in self.devPorts:
                    ##startkey_start = -pow(2, 31) # [-2^31, 2^31-1]
                    #startkey_start = 0 # [0, 2^32-1]
                    startkey_start = 0 # [0, 2^16-1]
                    for i in range(server_num):
                        if i == server_num - 1:
                            ##startkey_end = pow(2, 31) - 1 # if end is not included, then it is just processed by port 1111
                            #startkey_end = pow(2, 32) - 1
                            startkey_end = pow(2, 16) - 1
                        else:
                            startkey_end = startkey_start + key_range_per_server - 1
                        endkey_start = startkey_start
                        for j in range(i, server_num):
                            if j == server_num - 1:
                                ##endkey_end = pow(2, 31) - 1
                                #endkey_end = pow(2, 32) - 1
                                endkey_end = pow(2, 16) - 1
                            else:
                                endkey_end = endkey_start + key_range_per_server - 1
                            # NOTE: both start and end are included
                            matchspec0 = netbufferv4_range_partition_for_scan_tbl_match_spec_t(\
                                    op_hdr_optype = SCANREQ,
                                    #op_hdr_keyhihi_start = convert_u32_to_i32(startkey_start),
                                    #op_hdr_keyhihi_end = convert_u32_to_i32(startkey_end),
                                    #scan_hdr_keyhihi_start = convert_u32_to_i32(endkey_start),
                                    #scan_hdr_keyhihi_end = convert_u32_to_i32(endkey_end),
                                    op_hdr_keyhihihi_start = convert_u32_to_i32(startkey_start),
                                    op_hdr_keyhihihi_end = convert_u32_to_i32(startkey_end),
                                    scan_hdr_keyhihihi_start = convert_u32_to_i32(endkey_start),
                                    scan_hdr_keyhihihi_end = convert_u32_to_i32(endkey_end),
                                    meta_need_recirculate = 0)
                            # Forward to the egress pipeline of server
                            # serveridx = i to j
                            actnspec0 = netbufferv4_range_partition_for_scan_action_spec_t(\
                                    server_port + i, self.devPorts[1], j-i+1)
                            self.client.range_partition_for_scan_tbl_table_add_with_range_partition_for_scan(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                            endkey_start = endkey_end + 1
                        startkey_start = startkey_end + 1
            else:
                # # Table: hash_partition_tbl (default: reset_is_wrong_pipeline; size <= 8 * 128)
                # Table: hash_partition_tbl (default: nop; size <= 8 * 128)
                print "Configuring hash_partition_tbl"
                hash_range_per_server = partition_count / server_num
                for tmpoptype in [GETREQ, CACHE_POP_INSWITCH, PUTREQ, DELREQ]:
                    for iport in self.devPorts:
                        hash_start = 0 # [0, partition_count-1]
                        for i in range(server_num):
                            if i == server_num - 1:
                                hash_end = partition_count - 1 # if end is not included, then it is just processed by port 1111
                            else:
                                hash_end = hash_start + hash_range_per_server - 1
                            # NOTE: both start and end are included
                            matchspec0 = netbufferv4_hash_partition_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    meta_hashval_for_partition_start = convert_u16_to_i16(hash_start),
                                    meta_hashval_for_partition_end = convert_u16_to_i16(hash_end),
                                    ig_intr_md_ingress_port = iport,
                                    meta_need_recirculate = 0)
                            # Forward to the egress pipeline of server
                            eport = self.devPorts[1]
                            #if port_pipeidx_map[iport] == port_pipeidx_map[eport]: # in correct pipeline
                            #    actnspec0 = netbufferv4_hash_partition_action_spec_t(\
                            #            server_port + i, eport, 0)
                            #else: # in wrong pipeline
                            #    actnspec0 = netbufferv4_hash_partition_action_spec_t(\
                            #            server_port + i, eport, 1)
                            actnspec0 = netbufferv4_hash_partition_action_spec_t(\
                                    server_port + i, eport)
                            self.client.hash_partition_tbl_table_add_with_hash_partition(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                            hash_start = hash_end + 1

            # Table: cache_lookup_tbl (default: uncached_action; size: 32K/64K)
            print "Leave cache_lookup_tbl managed by controller in runtime"

            # Table: hash_for_cm_tbl (default: nop; size: 2)
            print "Configuring hash_for_cm_tbl"
            for tmpoptype in [GETREQ, PUTREQ]:
                matchspec0 = netbufferv4_hash_for_cm_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_need_recirculate = 0)
                self.client.hash_for_cm_tbl_table_add_with_hash_for_cm(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: hash_for_seq_tbl (default: nop; size: 2)
            print "Configuring hash_for_seq_tbl"
            for tmpoptype in [PUTREQ, DELREQ]:
                matchspec0 = netbufferv4_hash_for_seq_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_need_recirculate = 0)
                self.client.hash_for_seq_tbl_table_add_with_hash_for_seq(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 3

            # Table: snapshot_flag_tbl (default: reset_snapshot_flag; size: <=2)
            #print "Configuring snapshot_flag_tbl"
            #for tmpoptype in [PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
            #    matchspec0 = netbufferv4_snapshot_flag_tbl_match_spec_t(\
            #            op_hdr_optype = tmpoptype,
            #            meta_need_recirculate = 0)
            #    self.client.snapshot_flag_tbl_table_add_with_reset_snapshot_flag(\
            #            self.sess_hdl, self.dev_tgt, matchspec0)

            ## Table: prepare_for_cachehit_tbl (default: set_sid[self.sids[0]]; size: 6)
            #print "Configuring prepare_for_cachehit_tbl w/ default sid {}".format(self.sids[0])
            # Table: prepare_for_cachehit_tbl (default: set_sid(0); size: 6)
            print "Configuring prepare_for_cachehit_tbl"
            for tmpoptype in [GETREQ, PUTREQ, DELREQ]:
                matchspec0 = netbufferv4_prepare_for_cachehit_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        ig_intr_md_ingress_port = self.devPorts[0],
                        meta_need_recirculate = 0)
                #actnspec0 = netbufferv4_set_sid_action_spec_t(self.sids[0], self.devPorts[0])
                actnspec0 = netbufferv4_set_sid_action_spec_t(self.sids[0])
                self.client.prepare_for_cachehit_tbl_table_add_with_set_sid(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                # Should not used: no req from server
                matchspec0 = netbufferv4_prepare_for_cachehit_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        ig_intr_md_ingress_port = self.devPorts[1],
                        meta_need_recirculate = 0)
                #actnspec0 = netbufferv4_set_sid_action_spec_t(self.sids[1], self.devPorts[1])
                actnspec0 = netbufferv4_set_sid_action_spec_t(self.sids[1])
                self.client.prepare_for_cachehit_tbl_table_add_with_set_sid(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            # set default sid as sids[0]
            #actnspec0 = netbufferv4_set_sid_action_spec_t(self.sids[0])
            #self.client.prepare_for_cachehit_tbl_set_default_action_set_sid(\
            #        self.sess_hdl, self.dev_tgt, actnspec0)

            # Table: ipv4_forward_tbl (default: nop; size: 5)
            print "Configuring ipv4_forward_tbl"
            ipv4addr0 = ipv4Addr_to_i32(src_ip)
            for tmpoptype in [GETRES, PUTRES, DELRES]:
                matchspec0 = netbufferv4_ipv4_forward_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        ipv4_hdr_dstAddr = ipv4addr0,
                        ipv4_hdr_dstAddr_prefix_length = 32,
                        meta_need_recirculate = 0)
                actnspec0 = netbufferv4_forward_normal_response_action_spec_t(self.devPorts[0])
                self.client.ipv4_forward_tbl_table_add_with_forward_normal_response(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for tmpoptype in [GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
                matchspec0 = netbufferv4_ipv4_forward_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        ipv4_hdr_dstAddr = ipv4addr0,
                        ipv4_hdr_dstAddr_prefix_length = 32,
                        meta_need_recirculate = 0)
                actnspec0 = netbufferv4_forward_special_get_response_action_spec_t(self.sids[0])
                self.client.ipv4_forward_tbl_table_add_with_forward_special_get_response(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Stage 4

            # Table: sample_tbl (default: nop; size: 2)
            print "Configuring sample_tbl"
            for tmpoptype in [GETREQ, PUTREQ]:
                matchspec0 = netbufferv4_sample_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_need_recirculate = 0)
                self.client.sample_tbl_table_add_with_sample(\
                        self.sess_hdl, self.dev_tgt, matchspec0)


            # Table: ig_port_forward_tbl (default: nop; size: 6)
            print "Configuring ig_port_forward_tbl"
            matchspec0 = netbufferv4_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETREQ,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_getreq_to_getreq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_LATEST_SEQ,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_getres_latest_seq_to_getres_latest_seq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_DELETED_SEQ,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_getres_deleted_seq_to_getres_deleted_seq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_putreq_to_putreq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = DELREQ,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_delreq_to_delreq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            if RANGE_SUPPORT:
                matchspec0 = netbufferv4_ig_port_forward_tbl_match_spec_t(\
                        op_hdr_optype = SCANREQ,
                        meta_need_recirculate = 0)
                self.client.ig_port_forward_tbl_table_add_with_update_scanreq_to_scanreq_split(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Egress pipeline

            # Stage 0

            # Table: access_cmi_tbl (default: initialize_cmi_predicate; size: 2)
            cm_hashnum = 4
            for i in range(1, cm_hashnum+1):
                print "Configuring access_cm{}_tbl".format(i)
                for tmpoptype in [GETREQ_INSWITCH, PUTREQ_INSWITCH]:
                    matchspec0 = eval("netbufferv4_access_cm{}_tbl_match_spec_t".format(i))(\
                            op_hdr_optype = tmpoptype,
                            inswitch_hdr_is_sampled = 1,
                            inswitch_hdr_is_cached = 0)
                    eval("self.client.access_cm{}_tbl_table_add_with_update_cm{}".format(i, i))(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            if RANGE_SUPPORT:
                # Table: process_scanreq_split_tbl (default: nop; size <= 1 * 128)
                print "Configuring process_scanreq_split_tbl"
                for i in range(server_num):
                    dstport = server_port + i
                    matchspec0 = netbufferv4_process_scanreq_split_tbl_match_spec_t(\
                            op_hdr_optype = SCANREQ_SPLIT,
                            udp_hdr_dstPort = dstport)
                    # Set eport and sid for dstport
                    #actnspec0 = netbufferv4_process_scanreq_split_action_spec_t(self.devPorts[1], self.sids[1])
                    # Set sid for dstport+1
                    actnspec0 = netbufferv4_process_scanreq_split_action_spec_t(self.sids[1])
                    self.client.process_scanreq_split_tbl_table_add_with_process_scanreq_split(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
                    # Table: process_cloned_scanreq_split_tbl (default: nop; size <= 1 * 128)
                print "Configuring process_cloned_scanreq_split_tbl"
                for i in range(server_num):
                    dstport = server_port + i
                    matchspec0 = netbufferv4_process_cloned_scanreq_split_tbl_match_spec_t(\
                            op_hdr_optype = SCANREQ_SPLIT,
                            udp_hdr_dstPort = dstport)
                    # Set eport and sid for dstport+1
                    #actnspec0 = netbufferv4_process_cloned_scanreq_split_action_spec_t(self.devPorts[1], self.sids[1])
                    # Set sid for dstport+2
                    actnspec0 = netbufferv4_process_cloned_scanreq_split_action_spec_t(self.sids[1])
                    self.client.process_cloned_scanreq_split_tbl_table_add_with_process_cloned_scanreq_split(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Stgae 1

            if RANGE_SUPPORT:
                # Table: is_last_scansplit_tbl (default: reset_is_last_scansplit; size: 1)
                print "Configuring is_last_scansplit_tbl"
                matchspec0 = netbufferv4_is_last_scansplit_tbl_match_spec_t(\
                        op_hdr_optype = SCANREQ_SPLIT,
                        meta_remain_scannum = 1)
                self.client.is_last_scansplit_tbl_table_add_with_set_is_last_scansplit(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: is_hot_tbl (default: reset_is_hot; size: 1)
            print "Configuring is_hot_tbl"
            matchspec0 = netbufferv4_is_hot_tbl_match_spec_t(\
                    meta_cm1_predicate = 2,
                    meta_cm2_predicate = 2,
                    meta_cm3_predicate = 2,
                    meta_cm4_predicate = 2)
            self.client.is_hot_tbl_table_add_with_set_is_hot(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_cache_frequency_tbl (default: nop; size: 6)
            print "Configuring access_cache_frequency_tbl"
            for tmpoptype in [GETREQ_INSWITCH, PUTREQ_INSWITCH]:
                matchspec0 = netbufferv4_access_cache_frequency_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        inswitch_hdr_is_sampled = 1,
                        inswitch_hdr_is_cached = 1)
                self.client.access_cache_frequency_tbl_table_add_with_update_cache_frequency(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
            for is_sampled in sampled_list:
                for is_cached in cached_list:
                    matchspec0 = netbufferv4_access_cache_frequency_tbl_match_spec_t(\
                            op_hdr_optype = CACHE_POP_INSWITCH,
                            inswitch_hdr_is_sampled = is_sampled,
                            inswitch_hdr_is_cached = is_cached)
                    self.client.access_cache_frequency_tbl_table_add_with_reset_cache_frequency(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_validvalue_tbl (default: reset_meta_validvalue; size: 5)
            print "Configuring access_validvalue_tbl"
            for tmpoptype in [GETREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, PUTREQ_INSWITCH, DELREQ_INSWITCH]:
                matchspec0 = netbufferv4_access_validvalue_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        inswitch_hdr_is_cached = 1)
                self.client.access_validvalue_tbl_table_add_with_get_validvalue(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_seq_tbl (default: nop; size: 2)
            print "Configuring access_seq_tbl"
            for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH]:
                matchspec0 = netbufferv4_access_seq_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype)
                self.client.access_seq_tbl_table_add_with_assign_seq(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Stgae 2

            # Table: access_latest_tbl (default: reset_is_latest; size: 18)
            print "Configuring access_latest_tbl"
            for is_cached in cached_list:
                for validvalue in validvalue_list:
                    matchspec0 = netbufferv4_access_latest_tbl_match_spec_t(\
                            op_hdr_optype = GETREQ_INSWITCH,
                            inswitch_hdr_is_cached = is_cached,
                            meta_validvalue = validvalue)
                    if is_cached == 1:
                        self.client.access_latest_tbl_table_add_with_get_latest(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                    for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                        matchspec0 = netbufferv4_access_latest_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue)
                        if is_cached == 1 and validvalue == 1:
                            self.client.access_latest_tbl_table_add_with_set_and_get_latest(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                    for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH]:
                        matchspec0 = netbufferv4_access_latest_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue)
                        if is_cached == 1 and validvalue == 1:
                            self.client.access_latest_tbl_table_add_with_set_and_get_latest(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        elif is_cached == 1 and validvalue == 3:
                            self.client.access_latest_tbl_table_add_with_reset_and_get_latest(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                    matchspec0 = netbufferv4_access_latest_tbl_match_spec_t(\
                            op_hdr_optype = CACHE_POP_INSWITCH,
                            inswitch_hdr_is_cached = is_cached,
                            meta_validvalue = validvalue)
                    self.client.access_latest_tbl_table_add_with_reset_and_get_latest(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 3

            # Table: access_deleted_tbl (default: reset_is_deleted; size: 30)
            print "Configuring access_deleted_tbl"
            for is_cached in cached_list:
                for validvalue in validvalue_list:
                    for is_latest in latest_list:
                        matchspec0 = netbufferv4_access_deleted_tbl_match_spec_t(\
                                op_hdr_optype = GETREQ_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        if is_cached == 1:
                            self.client.access_deleted_tbl_table_add_with_get_deleted(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = netbufferv4_access_deleted_tbl_match_spec_t(\
                                op_hdr_optype = GETRES_LATEST_SEQ_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        if is_cached == 1 and validvalue == 1 and is_latest == 0:
                            self.client.access_deleted_tbl_table_add_with_reset_and_get_deleted(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = netbufferv4_access_deleted_tbl_match_spec_t(\
                                op_hdr_optype = GETRES_DELETED_SEQ_INSWITCH,
                                inswitch_hdr_is_cached = 1,
                                meta_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        if is_cached == 1 and validvalue == 1 and is_latest == 0:
                            self.client.access_deleted_tbl_table_add_with_set_and_get_deleted(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = netbufferv4_access_deleted_tbl_match_spec_t(\
                                op_hdr_optype = PUTREQ_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        if is_cached == 1 and validvalue == 1:
                            self.client.access_deleted_tbl_table_add_with_reset_and_get_deleted(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = netbufferv4_access_deleted_tbl_match_spec_t(\
                                op_hdr_optype = DELREQ_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        if is_cached == 1 and validvalue == 1:
                            self.client.access_deleted_tbl_table_add_with_set_and_get_deleted(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = netbufferv4_access_deleted_tbl_match_spec_t(\
                                op_hdr_optype = CACHE_POP_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        self.client.access_deleted_tbl_table_add_with_reset_and_get_deleted(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: update_vallen_tbl (default: nop; 30)
            print "Configuring update_vallen_tbl"
            for is_cached in cached_list:
                for validvalue in validvalue_list:
                    for is_latest in latest_list:
                        matchspec0 = netbufferv4_update_vallen_tbl_match_spec_t(\
                                op_hdr_optype = GETREQ_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        if is_cached == 1:
                            self.client.update_vallen_tbl_table_add_with_get_vallen(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                            matchspec0 = netbufferv4_update_vallen_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    inswitch_hdr_is_cached = is_cached,
                                    meta_validvalue = validvalue,
                                    meta_is_latest = is_latest)
                            if is_cached == 1 and validvalue == 1 and is_latest == 0:
                                self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = netbufferv4_update_vallen_tbl_match_spec_t(\
                                op_hdr_optype = PUTREQ_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        if is_cached == 1 and validvalue == 1:
                            self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = netbufferv4_update_vallen_tbl_match_spec_t(\
                                op_hdr_optype = DELREQ_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        if is_cached == 1 and validvalue == 1:
                            self.client.update_vallen_tbl_table_add_with_reset_and_get_vallen(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = netbufferv4_update_vallen_tbl_match_spec_t(\
                                op_hdr_optype = CACHE_POP_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_savedseq_tbl (default: nop; size: 22)
            print "Configuring access_savedseq_tbl"
            for is_cached in cached_list:
                for validvalue in validvalue_list:
                    for is_latest in latest_list:
                        for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH]:
                            matchspec0 = netbufferv4_access_savedseq_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    inswitch_hdr_is_cached = is_cached,
                                    meta_validvalue = validvalue,
                                    meta_is_latest = is_latest)
                            if is_cached == 1 and validvalue == 1:
                                self.client.access_savedseq_tbl_table_add_with_set_and_get_savedseq(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                        for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                            matchspec0 = netbufferv4_access_savedseq_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    inswitch_hdr_is_cached = is_cached,
                                    meta_validvalue = validvalue,
                                    meta_is_latest = is_latest)
                            if is_cached == 1 and validvalue == 1 and is_latest == 0:
                                self.client.access_savedseq_tbl_table_add_with_set_and_get_savedseq(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = netbufferv4_access_savedseq_tbl_match_spec_t(\
                                op_hdr_optype = CACHE_POP_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                meta_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        self.client.access_savedseq_tbl_table_add_with_set_and_get_savedseq(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_case1_tbl (default: reset_is_case1; 6)
            print "Configuring access_case1_tbl"
            for is_latest in latest_list:
                for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                    matchspec0 = netbufferv4_access_case1_tbl_match_spec_t(\
                            op_hdr_optype = tmpoptype,
                            inswitch_hdr_is_cached = 1,
                            meta_validvalue = 1,
                            meta_is_latest = is_latest,
                            inswitch_hdr_snapshot_flag = 1)
                    if is_latest == 0:
                        self.client.access_case1_tbl_table_add_with_try_case1(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH]:
                    matchspec0 = netbufferv4_access_case1_tbl_match_spec_t(\
                            op_hdr_optype = tmpoptype,
                            inswitch_hdr_is_cached = 1,
                            meta_validvalue = 1,
                            meta_is_latest = is_latest,
                            inswitch_hdr_snapshot_flag = 1)
                    self.client.access_case1_tbl_table_add_with_try_case1(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 4-11

            # Table: update_vallo1_tbl (default: nop; 14)
            print "Configuring update_vallo1_tbl"
            self.configure_update_val_tbl("lo1")

            # Table: update_vallo2_tbl (default: nop; 14)
            print "Configuring update_vallo2_tbl"
            self.configure_update_val_tbl("lo2")

            # Table: update_vallo3_tbl (default: nop; 14)
            print "Configuring update_vallo3_tbl"
            self.configure_update_val_tbl("lo3")

            # Table: update_vallo4_tbl (default: nop; 14)
            print "Configuring update_vallo4_tbl"
            self.configure_update_val_tbl("lo4")

            # Table: update_vallo5_tbl (default: nop; 14)
            print "Configuring update_vallo5_tbl"
            self.configure_update_val_tbl("lo5")

            # Table: update_vallo6_tbl (default: nop; 14)
            print "Configuring update_vallo6_tbl"
            self.configure_update_val_tbl("lo6")

            # Table: update_vallo7_tbl (default: nop; 14)
            print "Configuring update_vallo7_tbl"
            self.configure_update_val_tbl("lo7")

            # Table: update_vallo8_tbl (default: nop; 14)
            print "Configuring update_vallo8_tbl"
            self.configure_update_val_tbl("lo8")

            # Table: update_vallo9_tbl (default: nop; 14)
            print "Configuring update_vallo9_tbl"
            self.configure_update_val_tbl("lo9")

            # Table: update_vallo10_tbl (default: nop; 14)
            print "Configuring update_vallo10_tbl"
            self.configure_update_val_tbl("lo10")

            # Table: update_vallo11_tbl (default: nop; 14)
            print "Configuring update_vallo11_tbl"
            self.configure_update_val_tbl("lo11")

            # Table: update_vallo12_tbl (default: nop; 14)
            print "Configuring update_vallo12_tbl"
            self.configure_update_val_tbl("lo12")

            # Table: update_vallo13_tbl (default: nop; 14)
            print "Configuring update_vallo13_tbl"
            self.configure_update_val_tbl("lo13")

            # Table: update_vallo14_tbl (default: nop; 14)
            print "Configuring update_vallo14_tbl"
            self.configure_update_val_tbl("lo14")

            # Table: update_vallo15_tbl (default: nop; 14)
            print "Configuring update_vallo15_tbl"
            self.configure_update_val_tbl("lo15")

            # Table: update_vallo16_tbl (default: nop; 14)
            print "Configuring update_vallo16_tbl"
            self.configure_update_val_tbl("lo16")

            # Table: update_valhi1_tbl (default: nop; 14)
            print "Configuring update_valhi1_tbl"
            self.configure_update_val_tbl("hi1")

            # Table: update_valhi2_tbl (default: nop; 14)
            print "Configuring update_valhi2_tbl"
            self.configure_update_val_tbl("hi2")

            # Table: update_valhi3_tbl (default: nop; 14)
            print "Configuring update_valhi3_tbl"
            self.configure_update_val_tbl("hi3")

            # Table: update_valhi4_tbl (default: nop; 14)
            print "Configuring update_valhi4_tbl"
            self.configure_update_val_tbl("hi4")

            # Table: update_valhi5_tbl (default: nop; 14)
            print "Configuring update_valhi5_tbl"
            self.configure_update_val_tbl("hi5")

            # Table: update_valhi6_tbl (default: nop; 14)
            print "Configuring update_valhi6_tbl"
            self.configure_update_val_tbl("hi6")

            # Table: update_valhi7_tbl (default: nop; 14)
            print "Configuring update_valhi7_tbl"
            self.configure_update_val_tbl("hi7")

            # Table: update_valhi8_tbl (default: nop; 14)
            print "Configuring update_valhi8_tbl"
            self.configure_update_val_tbl("hi8")

            # Table: update_valhi9_tbl (default: nop; 14)
            print "Configuring update_valhi9_tbl"
            self.configure_update_val_tbl("hi9")

            # Table: update_valhi10_tbl (default: nop; 14)
            print "Configuring update_valhi10_tbl"
            self.configure_update_val_tbl("hi10")

            # Table: update_valhi11_tbl (default: nop; 14)
            print "Configuring update_valhi11_tbl"
            self.configure_update_val_tbl("hi11")

            # Table: update_valhi12_tbl (default: nop; 14)
            print "Configuring update_valhi12_tbl"
            self.configure_update_val_tbl("hi12")

            # Table: update_valhi13_tbl (default: nop; 14)
            print "Configuring update_valhi13_tbl"
            self.configure_update_val_tbl("hi13")

            # Table: update_valhi14_tbl (default: nop; 14)
            print "Configuring update_valhi14_tbl"
            self.configure_update_val_tbl("hi14")

            # Table: update_valhi15_tbl (default: nop; 14)
            print "Configuring update_valhi15_tbl"
            self.configure_update_val_tbl("hi15")

            # Table: update_valhi16_tbl (default: nop; 14)
            print "Configuring update_valhi16_tbl"
            self.configure_update_val_tbl("hi16")

            # Stage 9

            # Table: lastclone_tbl (default: reset_is_lastclone; size: 5)
            print "Configuring lastclone_tbl"
            for tmpoptype in [CACHE_POP_INSWITCH_ACK, GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1]:
                matchspec0 = netbufferv4_lastclone_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_clonenum_for_pktloss = 0)
                self.client.lastclone_tbl_table_add_with_set_is_lastclone(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 10

            # Table: eg_port_forward_tbl (default: nop; size: 1184)
            print "Configuring eg_port_forward_tbl"
            for is_cached in cached_list:
                for is_hot in hot_list:
                    for validvalue in validvalue_list:
                        for is_latest in latest_list:
                            for is_deleted in deleted_list:
                                # NOTE: eg_intr_md.egress_port is read-only
                                #for is_wrong_pipeline in pipeline_list:
                                for sid in self.sids:
                                    for is_lastclone_for_pktloss in lastclone_list:
                                        for snapshot_flag in snapshot_flag_list:
                                            for is_case1 in case1_list:
                                                # is_lastclone_for_pktloss, snapshot_flag, and is_case1 should be 0 for GETREQ_INSWITCH
                                                if is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                    # size: 128
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = GETREQ_INSWITCH,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = sid,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    if is_cached == 0:
                                                        if is_hot == 1:
                                                            # Update GETREQ_INSWITCH as GETREQ_POP to server
                                                            #actnspec0 = netbufferv4_update_getreq_inswitch_to_getreq_pop_action_spec_t(self.devPorts[1])
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_pop(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        else:
                                                            # Update GETREQ_INSWITCH as GETREQ to server
                                                            #actnspec0 = netbufferv4_update_getreq_inswitch_to_getreq_action_spec_t(self.devPorts[1])
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                    else:
                                                        if validvalue == 0:
                                                            # Update GETREQ_INSWITCH as GETREQ to server
                                                            #actnspec0 = netbufferv4_update_getreq_inswitch_to_getreq_action_spec_t(self.devPorts[1])
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        elif validvalue == 1:
                                                            if is_latest == 0:
                                                                # Update GETREQ_INSWITCH as GETREQ_NLATEST to server
                                                                #actnspec0 = netbufferv4_update_getreq_inswitch_to_getreq_nlatest_action_spec_t(self.devPorts[1])
                                                                self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_nlatest(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                            else:
                                                                #if is_wrong_pipeline == 0:
                                                                #    if is_deleted == 1:
                                                                #        # Update GETREQ_INSWITCH as GETRES for deleted value to client
                                                                #        self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_for_deleted(\
                                                                #                self.sess_hdl, self.dev_tgt, matchspec0)
                                                                #    else:
                                                                #        # Update GETREQ_INSWITCH as GETRES to client
                                                                #        self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres(\
                                                                #                self.sess_hdl, self.dev_tgt, matchspec0)
                                                                #elif is_wrong_pipeline == 1:
                                                                #    if is_deleted == 1:
                                                                #        # Update GETREQ_INSWITCH as GETRES for deleted value to client by mirroring
                                                                #        self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_for_deleted_by_mirroring(\
                                                                #                self.sess_hdl, self.dev_tgt, matchspec0)
                                                                #    else:
                                                                #        # Update GETREQ_INSWITCH as GETRES to client by mirroring
                                                                #        self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring(\
                                                                #                self.sess_hdl, self.dev_tgt, matchspec0)
                                                                if is_deleted == 1:
                                                                    # Update GETREQ_INSWITCH as GETRES for deleted value to client by mirroring
                                                                    actnspec0 = netbufferv4_update_getreq_inswitch_to_getres_for_deleted_by_mirroring_action_spec_t(sid)
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_for_deleted_by_mirroring(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                                else:
                                                                    # Update GETREQ_INSWITCH as GETRES to client by mirroring
                                                                    actnspec0 = netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t(sid)
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                        elif validvalue == 3:
                                                            if is_latest == 0:
                                                                # Update GETREQ_INSWITCH as GETREQ to server
                                                                #actnspec0 = netbufferv4_update_getreq_inswitch_to_getreq_action_spec_t(self.devPorts[1])
                                                                self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                            else:
                                                                #if is_wrong_pipeline == 0:
                                                                #    if is_deleted == 1:
                                                                #        # Update GETREQ_INSWITCH as GETRES for deleted value to client
                                                                #        self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_for_deleted(\
                                                                #                self.sess_hdl, self.dev_tgt, matchspec0)
                                                                #    else:
                                                                #        # Update GETREQ_INSWITCH as GETRES to client
                                                                #        self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres(\
                                                                #                self.sess_hdl, self.dev_tgt, matchspec0)
                                                                #elif is_wrong_pipeline == 1:
                                                                #    if is_deleted == 1:
                                                                #        # Update GETREQ_INSWITCH as GETRES for deleted value to client by mirroring
                                                                #        self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_for_deleted_by_mirroring(\
                                                                #                self.sess_hdl, self.dev_tgt, matchspec0)
                                                                #    else:
                                                                #        # Update GETREQ_INSWITCH as GETRES to client by mirroring
                                                                #        self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring(\
                                                                #                self.sess_hdl, self.dev_tgt, matchspec0)
                                                                if is_deleted == 1:
                                                                    # Update GETREQ_INSWITCH as GETRES for deleted value to client by mirroring
                                                                    actnspec0 = netbufferv4_update_getreq_inswitch_to_getres_for_deleted_by_mirroring_action_spec_t(sid)
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_for_deleted_by_mirroring(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                                else:
                                                                    # Update GETREQ_INSWITCH as GETRES to client by mirroring
                                                                    actnspec0 = netbufferv4_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t(sid)
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                # is_cached (no inswitch_hdr due to no field list when clone_i2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline (no inswitch_hdr), sid=0 (no inswitch hdr), is_lastclone_for_pktloss, snapshot_flag (no inswitch_hdr), is_case1 should be 0 for GETRES_LATEST_SEQ
                                                # NOTE: we use sid == self.sids[0] to avoid duplicate entry; we use inswitch_hdr_sid = 0 to match the default value of inswitch_hdr.sid
                                                # size: 1
                                                #if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and sid == self.sids[0] and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = GETRES_LATEST_SEQ,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = 0,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    # TODO: check if we need to set egress port for packet cloned by clone_i2e
                                                    # Update GETRES_LATEST_SEQ (by clone_i2e) as GETRES to client
                                                    self.client.eg_port_forward_tbl_table_add_with_update_getres_latest_seq_to_getres(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                # is_hot (cm_predicate=1), is_wrong_pipeline (not need range/hash partition), sid=0 (not need mirroring for res), is_lastclone_for_pktloss should be 0 for GETRES_LATEST_SEQ_INSWITCH
                                                # size: 128
                                                #if is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                                if is_hot == 0 and sid == self.sids[0] and is_lastclone_for_pktloss == 0:
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = GETRES_LATEST_SEQ_INSWITCH,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = 0,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    if is_cached == 1 and validvalue == 1 and is_latest == 0 and snapshot_flag == 1 and is_case1 == 0:
                                                        # Update GETRES_LATEST_SEQ_INSWITCH as GETRES_LATEST_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                        if is_deleted == 0: # is_deleted=0 -> stat=1
                                                            #actnspec0 = netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.sids[1], self.devPorts[1], 1)
                                                            actnspec0 = netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.sids[1], 1)
                                                        elif is_deleted == 1: # is_deleted=1 -> stat=0
                                                            #actnspec0 = netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.sids[1], self.devPorts[1], 0)
                                                            actnspec0 = netbufferv4_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.sids[1], 0)
                                                        self.client.eg_port_forward_tbl_table_add_with_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    else:
                                                        # Drop GETRES_LATEST_SEQ_INSWITCH
                                                        self.client.eg_port_forward_tbl_table_add_with_drop_getres_latest_seq_inswitch(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                # is_cached=1, is_wrong_pipeline=0, sid=0, and snapshot_flag=1 (same inswitch_hdr as GETRES_LATEST_SEQ_INSWITCH); is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_case1 should be 0 for GETRES_LATEST_SEQ_INSWITCH_CASE1
                                                # size: 2
                                                #if is_cached == 1 and is_wrong_pipeline == 0 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_case1 == 0:
                                                if is_cached == 1 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and sid == self.sids[0] and is_case1 == 0:
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = GETRES_LATEST_SEQ_INSWITCH_CASE1,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = 0,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    if is_lastclone_for_pktloss == 0:
                                                        # Forward GETRES_LATEST_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                        actnspec0 = netbufferv4_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.sids[1])
                                                        self.client.eg_port_forward_tbl_table_add_with_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    elif is_lastclone_for_pktloss == 1:
                                                        # Forward GETRES_LATEST_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector
                                                        self.client.eg_port_forward_tbl_table_add_with_forward_getres_latest_seq_inswitch_case1(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                # is_cached (no inswitch_hdr due to no field list when clone_i2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline (no inswitch_hdr), sid=0 (no inswitch_hdr), is_lastclone_for_pktloss, snapshot_flag (no inswitch_hdr), is_case1 should be 0 for GETRES_DELETED_SEQ
                                                # size: 1
                                                #if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and sid == self.sids[0] and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = GETRES_DELETED_SEQ,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = 0,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    # TODO: check if we need to set egress port for packet cloned by clone_i2e
                                                    # Update GETRES_DELETED_SEQ (by clone_i2e) as GETRES to client
                                                    self.client.eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_to_getres(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                # is_hot (cm_predicate=1), is_wrong_pipeline (not need range/hash partition), sid=0 (not need mirroring for res), is_lastclone_for_pktloss should be 0 for GETRES_DELETED_SEQ_INSWITCH
                                                # size: 128
                                                #if is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                                if is_hot == 0 and sid == self.sids[0] and is_lastclone_for_pktloss == 0:
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = GETRES_DELETED_SEQ_INSWITCH,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = 0,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    if is_cached == 1 and validvalue == 1 and is_latest == 0 and snapshot_flag == 1 and is_case1 == 0:
                                                        # Update GETRES_DELETED_SEQ_INSWITCH as GETRES_DELETED_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                        if is_deleted == 0: # is_deleted=0 -> stat=1
                                                            #actnspec0 = netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.sids[1], self.devPorts[1], 1)
                                                            actnspec0 = netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.sids[1], 1)
                                                        elif is_deleted == 1: # is_deleted=1 -> stat=0
                                                            #actnspec0 = netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.sids[1], self.devPorts[1], 0)
                                                            actnspec0 = netbufferv4_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.sids[1], 0)
                                                        self.client.eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    else:
                                                        # Drop GETRES_DELETED_SEQ_INSWITCH
                                                        self.client.eg_port_forward_tbl_table_add_with_drop_getres_deleted_seq_inswitch(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                # is_cached=1, is_wrong_pipeline=0, sid=0, and snapshot_flag=1 (same inswitch_hdr as GETRES_LATEST_SEQ_INSWITCH); is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_case1 should be 0 for GETRES_DELETED_SEQ_INSWITCH_CASE1
                                                # size: 2
                                                #if is_cached == 1 and is_wrong_pipeline == 0 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_case1 == 0:
                                                if is_cached == 1 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and sid == self.sids[0] and is_case1 == 0:
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = GETRES_DELETED_SEQ_INSWITCH_CASE1,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = 0,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    if is_lastclone_for_pktloss == 0:
                                                        # Forward GETRES_DELETED_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                        actnspec0 = netbufferv4_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.sids[1])
                                                        self.client.eg_port_forward_tbl_table_add_with_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    elif is_lastclone_for_pktloss == 1:
                                                        # Forward GETRES_DELETED_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector
                                                        self.client.eg_port_forward_tbl_table_add_with_forward_getres_deleted_seq_inswitch_case1(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                # is_cached (memset inswitch_hdr by end-host), is_hot (cm_predicate=1), validvalue, is_wrong_pipeline, sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_POP_INSWITCH
                                                # size: 4
                                                #if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                if is_cached == 0 and is_hot == 0 and validvalue == 0 and sid == self.sids[0] and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = CACHE_POP_INSWITCH,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = 0,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    # Update CACHE_POP_INSWITCH as CACHE_POP_INSWITCH_ACK to reflector (w/ clone)
                                                    #actnspec0 = netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_action_spec_t(self.sids[1], self.devPorts[1])
                                                    actnspec0 = netbufferv4_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss_action_spec_t(self.sids[1])
                                                    self.client.eg_port_forward_tbl_table_add_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                # is_cached (no inswitch_hdr), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, sid=0 (no inswitch_hdr), snapshot_flag, is_case1 should be 0 for CACHE_POP_INSWITCH_ACK
                                                # size: 2
                                                #if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and sid == self.sids[0] and snapshot_flag == 0 and is_case1 == 0:
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = CACHE_POP_INSWITCH_ACK,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = 0,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    # TODO: check if we need to set egress port for packet cloned by clone_e2e
                                                    if is_lastclone_for_pktloss == 0:
                                                        # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector (w/ clone)
                                                        actnspec0 = netbufferv4_forward_cache_pop_inswitch_ack_clone_for_pktloss_action_spec_t(self.sids[1])
                                                        self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack_clone_for_pktloss(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    elif is_lastclone_for_pktloss == 1:
                                                        # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                                        self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                # is_lastclone_for_pktloss should be 0 for PUTREQ_INSWITCH
                                                # size: 512
                                                if is_lastclone_for_pktloss == 0:
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = PUTREQ_INSWITCH,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = sid,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    if is_cached == 0:
                                                        if snapshot_flag == 1:
                                                            if is_hot == 1:
                                                                # Update PUTREQ_INSWITCH as PUTREQ_POP_SEQ_CASE3 to server
                                                                self.client.eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_pop_seq_case3(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                            elif is_hot == 0:
                                                                # Update PUTREQ_INSWITCH as PUTREQ_SEQ_CASE3 to server
                                                                self.client.eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq_case3(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                        elif snapshot_flag == 0:
                                                            if is_hot == 1:
                                                                # Update PUTREQ_INSWITCH as PUTREQ_POP_SEQ to server
                                                                self.client.eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_pop_seq(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                            elif is_hot == 0:
                                                                # Update PUTREQ_INSWITCH as PUTREQ_SEQ to server
                                                                self.client.eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                    elif is_cached == 1:
                                                        if validvalue == 0 or validvalue == 3:
                                                            if snapshot_flag == 1:
                                                                # Update PUTREQ_INSWITCH as PUTREQ_SEQ_CASE3 to server
                                                                self.client.eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq_case3(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                            elif snapshot_flag == 0:
                                                                # Update PUTREQ_INSWITCH as PUTREQ_SEQ to server
                                                                self.client.eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                        elif validvalue == 1:
                                                            if snapshot_flag == 1 and is_case1 == 0:
                                                                # Update PUTREQ_INSWITCH as PUTREQ_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                                if is_deleted == 0: # is_deleted=0 -> stat=1
                                                                    #actnspec0 = netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.sids[1], self.devPorts[1], 1)
                                                                    actnspec0 = netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.sids[1], 1)
                                                                elif is_deleted == 1: # is_deleted=1 -> stat=0
                                                                    #actnspec0 = netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.sids[1], self.devPorts[1], 0)
                                                                    actnspec0 = netbufferv4_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.sids[1], 0)
                                                                self.client.eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                            else:
                                                                #if is_wrong_pipeline == 0:
                                                                #    # Update PUTREQ_INSWITCH as PUTRES to client
                                                                #    self.client.eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putres(\
                                                                #            self.sess_hdl, self.dev_tgt, matchspec0)
                                                                #elif is_wrong_pipeline == 1:
                                                                #    # Update PUTREQ_INSWITCH as PUTRES to client by mirroring
                                                                #    self.client.eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putres_by_mirroring(\
                                                                #            self.sess_hdl, self.dev_tgt, matchspec0)
                                                                # Update PUTREQ_INSWITCH as PUTRES to client by mirroring
                                                                actnspec0 = netbufferv4_update_putreq_inswitch_to_putres_by_mirroring_action_spec_t(sid)
                                                                self.client.eg_port_forward_tbl_table_add_with_update_putreq_inswitch_to_putres_by_mirroring(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                # is_cached=1, is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, snapshot_flag=1, is_case1 should be 0 for PUTREQ_SEQ_INSWITCH_CASE1
                                                # size: 4
                                                if is_cached == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and snapshot_flag == 1 and is_case1 == 0:
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = PUTREQ_SEQ_INSWITCH_CASE1,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = sid,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    if is_lastclone_for_pktloss == 0:
                                                        # Forward PUTREQ_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                        actnspec0 = netbufferv4_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.sids[1])
                                                        self.client.eg_port_forward_tbl_table_add_with_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    elif is_lastclone_for_pktloss == 1:
                                                        #if is_wrong_pipeline == 0:
                                                        #    # Update PUTREQ_SEQ_INSWITCH_CASE1 as PUTRES to client
                                                        #    self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_case1_to_putres(\
                                                        #            self.sess_hdl, self.dev_tgt, matchspec0)
                                                        #elif is_wrong_pipeline == 1:
                                                        #    # Update PUTREQ_SEQ_INSWITCH_CASE1 as PUTRES to client by mirroring
                                                        #    self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring(\
                                                        #            self.sess_hdl, self.dev_tgt, matchspec0)
                                                        # Update PUTREQ_SEQ_INSWITCH_CASE1 as PUTRES to client by mirroring
                                                        actnspec0 = netbufferv4_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec_t(sid)
                                                        self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                # is_hot (cm_predicate=1), is_lastclone_for_pktloss should be 0 for DELREQ_INSWITCH
                                                # size: 256
                                                if is_hot == 0 and is_lastclone_for_pktloss == 0:
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = DELREQ_INSWITCH,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = sid,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    if is_cached == 0:
                                                        if snapshot_flag == 1:
                                                            # Update DELREQ_INSWITCH as DELREQ_SEQ_CASE3 to server
                                                            self.client.eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq_case3(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        elif snapshot_flag == 0:
                                                            # Update DELREQ_INSWITCH as DELREQ_SEQ to server
                                                            self.client.eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                    elif is_cached == 1:
                                                        if validvalue == 0 or validvalue == 3:
                                                            if snapshot_flag == 1:
                                                                # Update DELREQ_INSWITCH as DELREQ_SEQ_CASE3 to server
                                                                self.client.eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq_case3(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                            elif snapshot_flag == 0:
                                                                # Update DELREQ_INSWITCH as DELREQ_SEQ to server
                                                                self.client.eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                        elif validvalue == 1:
                                                            if snapshot_flag == 1 and is_case1 == 0:
                                                                # Update DELREQ_INSWITCH as DELREQ_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                                if is_deleted == 0: # is_deleted=0 -> stat=1
                                                                    #actnspec0 = netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.sids[1], self.devPorts[1], 1)
                                                                    actnspec0 = netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.sids[1], 1)
                                                                elif is_deleted == 1: # is_deleted=1 -> stat=0
                                                                    #actnspec0 = netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.sids[1], self.devPorts[1], 0)
                                                                    actnspec0 = netbufferv4_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.sids[1], 0)
                                                                self.client.eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                            else:
                                                                #if is_wrong_pipeline == 0:
                                                                #    # Update DELREQ_INSWITCH as DELRES to client
                                                                #    self.client.eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delres(\
                                                                #            self.sess_hdl, self.dev_tgt, matchspec0)
                                                                #elif is_wrong_pipeline == 1:
                                                                #    # Update DELREQ_INSWITCH as DELRES to client by mirroring
                                                                #    self.client.eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delres_by_mirroring(\
                                                                #            self.sess_hdl, self.dev_tgt, matchspec0)
                                                                # Update DELREQ_INSWITCH as DELRES to client by mirroring
                                                                actnspec0 = netbufferv4_update_delreq_inswitch_to_delres_by_mirroring_action_spec_t(sid)
                                                                self.client.eg_port_forward_tbl_table_add_with_update_delreq_inswitch_to_delres_by_mirroring(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                # is_cached=1, is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, snapshot_flag=1, is_case1 should be 0 for DELREQ_SEQ_INSWITCH_CASE1
                                                # size: 16
                                                if is_cached == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and snapshot_flag == 1 and is_case1 == 0:
                                                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                                        op_hdr_optype = DELREQ_SEQ_INSWITCH_CASE1,
                                                        inswitch_hdr_is_cached = is_cached,
                                                        debug_hdr_is_hot = is_hot,
                                                        meta_validvalue = validvalue,
                                                        meta_is_latest = is_latest,
                                                        meta_is_deleted = is_deleted,
                                                        #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                        inswitch_hdr_sid = sid,
                                                        debug_hdr_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                        inswitch_hdr_snapshot_flag = snapshot_flag,
                                                        meta_is_case1 = is_case1)
                                                    if is_lastclone_for_pktloss == 0:
                                                        # Forward DELREQ_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                        actnspec0 = netbufferv4_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.sids[1])
                                                        self.client.eg_port_forward_tbl_table_add_with_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    elif is_lastclone_for_pktloss == 1:
                                                        #if is_wrong_pipeline == 0:
                                                        #    # Update DELREQ_SEQ_INSWITCH_CASE1 as DELRES to client
                                                        #    self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_case1_to_delres(\
                                                        #            self.sess_hdl, self.dev_tgt, matchspec0)
                                                        #elif is_wrong_pipeline == 1:
                                                        #    # Update DELREQ_SEQ_INSWITCHCASE1 as DELRES to client by mirroring
                                                        #    self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring(\
                                                        #            self.sess_hdl, self.dev_tgt, matchspec0)
                                                        # Update DELREQ_SEQ_INSWITCHCASE1 as DELRES to client by mirroring
                                                        actnspec0 = netbufferv4_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec_t(sid)
                                                        self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            if RANGE_SUPPORT:
                # Table: scan_forward_tbl (default: nop; size: 2)
                for sid in self.sids:
                    matchspec0 = netbufferv4_scan_forward_tbl_match_spec_t(\
                            op_hdr_optype = SCANREQ_SPLIT,
                            meta_is_last_scansplit = 1,
                            inswitch_hdr_sid = sid)
                    actnspec0 = netbufferv4_forward_scanreq_split_action_spec_t(sid)
                    self.client.scan_forward_tbl_table_add_with_forward_scanreq_split(\
                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Table: update_udplen_tbl (default: nop; 158)
            print "Configuring update_udplen_tbl"
            for i in range(switch_max_vallen/8 + 1): # i from 0 to 16
                if i == 0:
                    vallen_start = 0
                    vallen_end = 0
                    aligned_vallen = 0
                else:
                    vallen_start = (i-1)*8+1 # 1, 9, ..., 121
                    vallen_end = (i-1)*8+8 # 8, 16, ..., 128
                    aligned_vallen = vallen_end # 8, 16, ..., 128
                # NOTE: including 1B debug_hdr
                val_stat_udplen = aligned_vallen + 28
                val_seq_inswitch_stat_udplen = aligned_vallen + 41
                val_seq_udplen = aligned_vallen + 31
                matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=GETRES,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end) # [vallen_start, vallen_end]
                actnspec0 = netbufferv4_update_udplen_action_spec_t(val_stat_udplen)
                # TODO: check parameter 0
                self.client.update_udplen_tbl_table_add_with_update_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1]:
                    matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                            op_hdr_optype=tmpoptype,
                            vallen_hdr_vallen_start=vallen_start,
                            vallen_hdr_vallen_end=vallen_end) # [vallen_start, vallen_end]
                    actnspec0 = netbufferv4_update_udplen_action_spec_t(val_seq_inswitch_stat_udplen)
                    # TODO: check parameter 0
                    self.client.update_udplen_tbl_table_add_with_update_udplen(\
                            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                for tmpoptype in [PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3]:
                    matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                            op_hdr_optype=tmpoptype,
                            vallen_hdr_vallen_start=vallen_start,
                            vallen_hdr_vallen_end=vallen_end) # [vallen_start, vallen_end]
                    actnspec0 = netbufferv4_update_udplen_action_spec_t(val_seq_udplen)
                    # TODO: check parameter 0
                    self.client.update_udplen_tbl_table_add_with_update_udplen(\
                            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
            onlyop_udplen = 24
            stat_udplen = 26
            seq_udplen = 29
            matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                    op_hdr_optype=CACHE_POP_INSWITCH_ACK,
                    vallen_hdr_vallen_start=0,
                    vallen_hdr_vallen_end=switch_max_vallen) # [0, 128]
            actnspec0 = netbufferv4_update_udplen_action_spec_t(onlyop_udplen)
            # TODO: check parameter 0
            self.client.update_udplen_tbl_table_add_with_update_udplen(\
                    self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
            for tmpoptype in [PUTRES, DELRES]:
                matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=tmpoptype,
                        vallen_hdr_vallen_start=0,
                        vallen_hdr_vallen_end=switch_max_vallen) # [0, 128]
                actnspec0 = netbufferv4_update_udplen_action_spec_t(stat_udplen)
                # TODO: check parameter 0
                self.client.update_udplen_tbl_table_add_with_update_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
            for tmpoptype in [DELREQ_SEQ, DELREQ_SEQ_CASE3]:
                matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=tmpoptype,
                        vallen_hdr_vallen_start=0,
                        vallen_hdr_vallen_end=switch_max_vallen) # [0, 128]
                actnspec0 = netbufferv4_update_udplen_action_spec_t(seq_udplen)
                # TODO: check parameter 0
                self.client.update_udplen_tbl_table_add_with_update_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)





#            # Table: update_macaddr_tbl (default: nop; 5)
#            print "Configuring update_macaddr_tbl"
#            actnspec0 = netbufferv4_update_macaddr_s2c_action_spec_t(\
#                    macAddr_to_string(src_mac), \
#                    macAddr_to_string(dst_mac))
#            actnspec1 = netbufferv4_update_macaddr_c2s_action_spec_t(\
#                    macAddr_to_string(src_mac), \
#                    macAddr_to_string(dst_mac))
#            matchspec0 = netbufferv4_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_TYPE)
#            matchspec1 = netbufferv4_update_macaddr_tbl_match_spec_t(op_hdr_optype=PUTRES_TYPE)
#            matchspec2 = netbufferv4_update_macaddr_tbl_match_spec_t(op_hdr_optype=DELRES_TYPE)
#            self.client.update_macaddr_tbl_table_add_with_update_macaddr_s2c(\
#                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
#            self.client.update_macaddr_tbl_table_add_with_update_macaddr_s2c(\
#                    self.sess_hdl, self.dev_tgt, matchspec1, actnspec0)
#            self.client.update_macaddr_tbl_table_add_with_update_macaddr_s2c(\
#                    self.sess_hdl, self.dev_tgt, matchspec2, actnspec0)
#            #matchspec3 = netbufferv4_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_POP_TYPE)
#            #self.client.update_macaddr_tbl_table_add_with_update_macaddr_c2s(\
#            #        self.sess_hdl, self.dev_tgt, matchspec3, actnspec1)
#            matchspec3 = netbufferv4_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_POP_EVICT_TYPE)
#            self.client.update_macaddr_tbl_table_add_with_update_macaddr_c2s(\
#                    self.sess_hdl, self.dev_tgt, matchspec3, actnspec1)
#            matchspec4 = netbufferv4_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_POP_EVICT_CASE2_TYPE)
#            self.client.update_macaddr_tbl_table_add_with_update_macaddr_c2s(\
#                    self.sess_hdl, self.dev_tgt, matchspec4, actnspec1)

            

            self.conn_mgr.complete_operations(self.sess_hdl)
            self.conn_mgr.client_cleanup(self.sess_hdl) # close session

    #def tearDown(self):
    #    if test_param_get('cleanup') == True:

    #        print "\nCleaning up"

    #        # delete the programmed forward table entry
    #        self.cleanup_table("ipv4_lpm", True)
    #        # delete the platform ports
    #        self.conn_mgr.client_cleanup(self.sess_hdl)
    #        for i in self.devPorts:
    #            self.pal.pal_port_del(0, i)
    #        self.pal.pal_port_del_all(0)

    def cleanup_table(self, table, iscalled=False):
        if iscalled:
            table = 'self.client.' + table
            # get entry count
            num_entries = eval(table + '_get_entry_count')\
                          (self.sess_hdl, self.dev_tgt)
            print "Number of entries : {}".format(num_entries)
            if num_entries == 0:
                return
            # get the entry handles
            hdl = eval(table + '_get_first_entry_handle')\
                    (self.sess_hdl, self.dev_tgt)
            if num_entries > 1:
                hdls = eval(table + '_get_next_entry_handles')\
                    (self.sess_hdl, self.dev_tgt, hdl, num_entries - 1)
                hdls.insert(0, hdl)
            else:
                hdls = [hdl]
            # delete the table entries
            for hdl in hdls:
                entry = eval(table + '_get_entry')\
                    (self.sess_hdl, self.dev_tgt.dev_id, hdl, True)
                eval(table + '_table_delete_by_match_spec')\
                    (self.sess_hdl, self.dev_tgt, entry.match_spec)
