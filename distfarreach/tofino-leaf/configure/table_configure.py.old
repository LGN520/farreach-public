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

from distfarreachleaf.p4_pd_rpc.ttypes import *
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

cached_list = [0, 1]
hot_list = [0, 1]
validvalue_list = [0, 1, 3]
#validvalue_list = [0, 1, 2, 3] # If with PUTREQ_LARGE
latest_list = [0, 1]
stat_list = [0, 1]
deleted_list = [0, 1]
sampled_list = [0, 1]
lastclone_list = [0, 1]
snapshot_flag_list = [0, 1]
case1_list = [0, 1]
access_val_mode_list = [0, 1, 2, 3]

reflector_ip_for_switchos = leaf_reflector_ip_for_switchos
reflector_dp2cpserver_port = leaf_reflector_dp2cpserver_port
reflector_cp2dpserver_port = leaf_reflector_cp2dpserver_port
reflector_cp2dp_dstip = leaf_reflector_cp2dp_dstip
reflector_cp2dp_dstmac = leaf_reflector_cp2dp_dstmac

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
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["distfarreachleaf"])

    def configure_update_val_tbl(self, valname):
#        # size: 30
#        for is_cached in cached_list:
#            for validvalue in validvalue_list:
#                for is_latest in latest_list:
#                    matchspec0 = eval("distfarreachleaf_update_val{}_tbl_match_spec_t".format(valname))(
#                            op_hdr_optype = GETREQ_INSWITCH,
#                            inswitch_hdr_is_cached = is_cached,
#                            validvalue_hdr_validvalue = validvalue,
#                            meta_is_latest = is_latest)
#                    if is_cached == 1:
#                        eval("self.client.update_val{}_tbl_table_add_with_get_val{}".format(valname, valname))(\
#                                self.sess_hdl, self.dev_tgt, matchspec0)
#                    for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
#                        matchspec0 = eval("distfarreachleaf_update_val{}_tbl_match_spec_t".format(valname))(
#                                op_hdr_optype = tmpoptype,
#                                inswitch_hdr_is_cached = is_cached,
#                                validvalue_hdr_validvalue = validvalue,
#                                meta_is_latest = is_latest)
#                        if is_cached == 1 and validvalue == 1 and is_latest == 0:
#                            eval("self.client.update_val{}_tbl_table_add_with_set_and_get_val{}".format(valname, valname))(\
#                                    self.sess_hdl, self.dev_tgt, matchspec0)
#                    matchspec0 = eval("distfarreachleaf_update_val{}_tbl_match_spec_t".format(valname))(
#                            op_hdr_optype = PUTREQ_INSWITCH,
#                            inswitch_hdr_is_cached = is_cached,
#                            validvalue_hdr_validvalue = validvalue,
#                            meta_is_latest = is_latest)
#                    if is_cached == 1 and validvalue == 1:
#                        eval("self.client.update_val{}_tbl_table_add_with_set_and_get_val{}".format(valname, valname))(\
#                                self.sess_hdl, self.dev_tgt, matchspec0)
#                    matchspec0 = eval("distfarreachleaf_update_val{}_tbl_match_spec_t".format(valname))(
#                            op_hdr_optype = DELREQ_INSWITCH,
#                            inswitch_hdr_is_cached = is_cached,
#                            validvalue_hdr_validvalue = validvalue,
#                            meta_is_latest = is_latest)
#                    if is_cached == 1 and validvalue == 1:
#                        eval("self.client.update_val{}_tbl_table_add_with_reset_and_get_val{}".format(valname, valname))(\
#                                self.sess_hdl, self.dev_tgt, matchspec0)
#                    matchspec0 = eval("distfarreachleaf_update_val{}_tbl_match_spec_t".format(valname))(
#                            op_hdr_optype = CACHE_POP_INSWITCH,
#                            inswitch_hdr_is_cached = is_cached,
#                            validvalue_hdr_validvalue = validvalue,
#                            meta_is_latest = is_latest)
#                    eval("self.client.update_val{}_tbl_table_add_with_set_and_get_val{}".format(valname, valname))(\
#                            self.sess_hdl, self.dev_tgt, matchspec0)
        # size: 3
        for access_val_mode in access_val_mode_list:
            matchspec0 = eval("distfarreachleaf_update_val{}_tbl_match_spec_t".format(valname))(
                    meta_access_val_mode = access_val_mode)
            # NOTE: not access val_reg if access_val_mode == 0
            if access_val_mode == 1:
                eval("self.client.update_val{}_tbl_table_add_with_get_val{}".format(valname, valname))(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
            elif access_val_mode == 2:
                eval("self.client.update_val{}_tbl_table_add_with_set_and_get_val{}".format(valname, valname))(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
            elif access_val_mode == 3:
                eval("self.client.update_val{}_tbl_table_add_with_reset_and_get_val{}".format(valname, valname))(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

    def setUp(self):
        print '\nSetup'

        # initialize the connection
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.sess_hdl = self.conn_mgr.client_init()
        self.dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        self.client_devports = []
        self.server_devports = []

        self.platform_type = "mavericks"
        board_type = self.pltfm_pm.pltfm_pm_board_type_get()
        if re.search("0x0234|0x1234|0x4234|0x5234", hex(board_type)):
            self.platform_type = "mavericks"
        elif re.search("0x2234|0x3234", hex(board_type)):
            self.platform_type = "montara"

        # get the device ports from front panel ports
        for client_fpport in client_fpports:
            port, chnl = client_fpport.split("/")
            devport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
            self.client_devports.append(devport)
        for server_fpport in server_fpports:
            port, chnl = server_fpport.split("/")
            devport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
            self.server_devports.append(devport)
        port, chnl = leafswitch_fpport_to_spine.split("/")
        devport = self.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
        self.spineswitch_devport = devport

        self.recirPorts = [64, 192]

        # NOTE: in each pipeline, 64-67 are recir/cpu ports, 68-71 are recir/pktgen ports
        #self.cpuPorts = [64, 192] # CPU port is 100G

        sidnum = len(self.client_devports) + len(self.server_devports) + 1
        sids = random.sample(xrange(BASE_SID_NORM, MAX_SID_NORM), sidnum)
        self.client_sids = sids[0:len(self.client_devports)]
        self.server_sids = sids[len(self.client_devports):sidnum-1]
        self.spineswitch_sid = sids[sidnum-1]

        # NOTE: data plane communicate with switchos by software-based reflector, which is deployed in one server machine
        isvalid = False
        for i in range(server_physical_num):
            if reflector_ip_for_switchos == server_ip_for_controller_list[i]:
                isvalid = True
                self.reflector_ip_for_switch = server_ips[i]
                self.reflector_mac_for_switch = server_macs[i]
                self.reflector_devport = self.server_devports[i]
                self.reflector_sid = self.server_sids[i] # clone to switchos (i.e., reflector at [the first] physical server)
        if isvalid == False:
            for i in range(client_physical_num):
                if reflector_ip_for_switchos == client_ip_for_client0_list[i]:
                    isvalid = True
                    self.reflector_ip_for_switch = client_ips[i]
                    self.reflector_mac_for_switch = client_macs[i]
                    self.reflector_devport = self.client_devports[i]
                    self.reflector_sid = self.client_sids[i]
        if isvalid == False:
            print "[ERROR] invalid reflector configuration"
            exit(-1)

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
            for i in self.client_devports:
               self.pal.pal_port_add(0, i,
                                     pal_port_speed_t.BF_SPEED_40G,
                                     pal_fec_type_t.BF_FEC_TYP_NONE)
               self.pal.pal_port_enable(0, i)
            for i in self.server_devports:
               self.pal.pal_port_add(0, i,
                                     pal_port_speed_t.BF_SPEED_40G,
                                     pal_fec_type_t.BF_FEC_TYP_NONE)
               self.pal.pal_port_enable(0, i)
            self.pal.pal_port_add(0, self.spineswitch_devport,
                                 pal_port_speed_t.BF_SPEED_40G,
                                 pal_fec_type_t.BF_FEC_TYP_NONE)
            self.pal.pal_port_enable(0, self.spineswitch_devport)

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
#            print "Binding sid {} with port {} for ingress mirroring".format(self.sids[0], self.devPorts[0]) # clone to client
#            info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
#                                  Direction_e.PD_DIR_INGRESS,
#                                  self.sids[0],
#                                  self.devPorts[0],
#                                  True)
#            self.mirror.mirror_session_create(self.sess_hdl, self.dev_tgt, info)
#            print "Binding sid {} with port {} for egress mirroring".format(self.sids[0], self.devPorts[0]) # clone to client
#            info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
#                                  Direction_e.PD_DIR_EGRESS,
#                                  self.sids[0],
#                                  self.devPorts[0],
#                                  True)
#            self.mirror.mirror_session_create(self.sess_hdl, self.dev_tgt, info)
#            print "Binding sid {} with port {} for ingress mirroring".format(self.sids[1], self.devPorts[1]) # clone to server
#            info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
#                                  Direction_e.PD_DIR_INGRESS,
#                                  self.sids[1],
#                                  self.devPorts[1],
#                                  True)
#            self.mirror.mirror_session_create(self.sess_hdl, self.dev_tgt, info)
#            print "Binding sid {} with port {} for egress mirroring".format(self.sids[1], self.devPorts[1]) # clone to server
#            info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
#                                  Direction_e.PD_DIR_EGRESS,
#                                  self.sids[1],
#                                  self.devPorts[1],
#                                  True)
#            self.mirror.mirror_session_create(self.sess_hdl, self.dev_tgt, info)
            for i in range(client_physical_num):
                print "Binding sid {} with client devport {} for both direction mirroring".format(self.client_sids[i], self.client_devports[i]) # clone to client
                info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
                                      Direction_e.PD_DIR_BOTH,
                                      self.client_sids[i],
                                      self.client_devports[i],
                                      True)
                self.mirror.mirror_session_create(self.sess_hdl, self.dev_tgt, info)
            for i in range(server_physical_num):
                print "Binding sid {} with server devport {} for both direction mirroring".format(self.server_sids[i], self.server_devports[i]) # clone to server
                info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
                                      Direction_e.PD_DIR_BOTH,
                                      self.server_sids[i],
                                      self.server_devports[i],
                                      True)
                self.mirror.mirror_session_create(self.sess_hdl, self.dev_tgt, info)
            print "Binding sid {} with spineswitch devport {} for both direction mirroring".format(self.spineswitch_sid, self.spineswitch_devport) # clone to spineswitch
            info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
                                  Direction_e.PD_DIR_BOTH,
                                  self.spineswitch_sid,
                                  self.spineswitch_devport,
                                  True)
            self.mirror.mirror_session_create(self.sess_hdl, self.dev_tgt, info)

            ################################
            ### Normal MAT Configuration ###
            ################################

            # Ingress pipeline

            # Stage 0

            # Table: l2l3_forward_tbl (default: nop; size: client_physical_num+server_physical_num = 4 < 16)
            print "Configuring l2l3_forward_tbl"
            for i in range(client_physical_num):
                matchspec0 = distfarreachleaf_l2l3_forward_tbl_match_spec_t(\
                        ethernet_hdr_dstAddr = macAddr_to_string(client_macs[i]),
                        ipv4_hdr_dstAddr = ipv4Addr_to_i32(client_ips[i]),
                        ipv4_hdr_dstAddr_prefix_length = 32)
                actnspec0 = distfarreachleaf_l2l3_forward_action_spec_t(self.client_devports[i])
                self.client.l2l3_forward_tbl_table_add_with_l2l3_forward(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for i in range(server_physical_num):
                matchspec0 = distfarreachleaf_l2l3_forward_tbl_match_spec_t(\
                        ethernet_hdr_dstAddr = macAddr_to_string(server_macs[i]),
                        ipv4_hdr_dstAddr = ipv4Addr_to_i32(server_ips[i]),
                        ipv4_hdr_dstAddr_prefix_length = 32)
                actnspec0 = distfarreachleaf_l2l3_forward_action_spec_t(self.server_devports[i])
                self.client.l2l3_forward_tbl_table_add_with_l2l3_forward(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            
            # Table: need_recirculate_tbl (default: reset_need_recirculate; size: <=8)
            # See enable/disable_singlepath in ptf_snapshotserver/table_configure.py

            # Table: set_hot_threshold_tbl (default: set_hot_threshold; size: 1)
            print "Configuring set_hot_threshold_tbl"
            actnspec0 = distfarreachleaf_set_hot_threshold_action_spec_t(hot_threshold)
            self.client.set_hot_threshold_tbl_set_default_action_set_hot_threshold(\
                    self.sess_hdl, self.dev_tgt, actnspec0)

            # Table: hash_for_spineselect_tbl (default: nop; size: 10)
            # NOTE: we calculate meta.spineswitchidx for special get responses here
            print "Configuring hash_for_spineselect_tbl"
            for tmpoptype in [GETREQ, PUTREQ, DELREQ, SCANREQ, WARMUPREQ, LOADREQ, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER, GETRES_LATEST_SEQ_SERVER_INSWITCH, GETRES_DELETED_SEQ_SERVER_INSWITCH]:
                matchspec0 = distfarreachleaf_hash_for_spineselect_tbl_match_spec_t(\
                        op_hdr_optype = convert_u16_to_i16(tmpoptype))
                self.client.hash_for_spineselect_tbl_table_add_with_hash_for_spineselect(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 1

            # Table: recirculate_tbl (default: nop; size: 4)
            print "Configuring recirculate_tbl"
            for tmpoptype in [PUTREQ_SEQ, DELREQ_SEQ, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER]:
                matchspec0 = distfarreachleaf_recirculate_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_need_recirculate = 1)
                # forward to the first spine switch for atomicity of setting snapshot flag
                actnspec0 = distfarreachleaf_recirculate_pkt_action_spec_t(self.spineswitch_devport)
                self.client.recirculate_tbl_table_add_with_recirculate_pkt(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Stage 1

            if RANGE_SUPPORT == False:
                # Table: hash_for_partition_tbl (default: nop; size: 17)
                print "Configuring hash_for_partition_tbl"
                for tmpoptype in [GETREQ_SPINE, CACHE_POP_INSWITCH, PUTREQ_SEQ, DELREQ_SEQ, WARMUPREQ_SPINE, LOADREQ_SPINE, CACHE_EVICT_LOADFREQ_INSWITCH, CACHE_EVICT_LOADDATA_INSWITCH, LOADSNAPSHOTDATA_INSWITCH, SETVALID_INSWITCH, PUTREQ_SEQ_INSWITCH, DELREQ_SEQ_INSWITCH, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER, GETRES_LATEST_SEQ_SERVER_INSWITCH, GETRES_DELETED_SEQ_SERVER_INSWITCH, GETREQ_NLATEST]:
                    matchspec0 = distfarreachleaf_hash_for_partition_tbl_match_spec_t(\
                            op_hdr_optype = convert_u16_to_i16(tmpoptype),
                            meta_need_recirculate = 0)
                    self.client.hash_for_partition_tbl_table_add_with_hash_for_partition(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: spineselect_tbl (default: nop; size <= 10 * spineswitch_total_logical_num)
            # NOTE: we calculate meta.spineswitchidx for special get responses here
            print "Configuring spineselect_tbl"
            key_range_per_spineswitch = switch_partition_count / spineswitch_total_logical_num
            for tmpoptype in [GETREQ, PUTREQ, DELREQ, SCANREQ, WARMUPREQ, LOADREQ, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER, GETRES_LATEST_SEQ_SERVER_INSWITCH, GETRES_DELETED_SEQ_SERVER_INSWITCH]:
                key_start = 0 # [0, 2^16-1]
                for i in range(spineswitch_total_logical_num):
                    global_spineswitch_logical_idx = spineswitch_logical_idxes[i]
                    if i == spineswitch_total_logical_num - 1:
                        key_end = switch_partition_count - 1
                    else:
                        key_end = key_start + key_range_per_spineswitch - 1
                    # NOTE: both start and end are included
                    matchspec0 = distfarreachleaf_spineselect_tbl_match_spec_t(\
                            op_hdr_optype = tmpoptype,
                            meta_hashval_for_spineselect_start = convert_u16_to_i16(key_start),
                            meta_hashval_for_spineselect_end = convert_u16_to_i16(key_end),
                            meta_need_recirculate = 0)
                    # Forward to the egress pipeline of spine switch (NOT touch any MAT in egress pipelines)
                    eport = self.spineswitch_devport
                    if tmpoptype == GETRES_LATEST_SEQ_SERVER or tmpoptype == GETRES_DELETED_SEQ_SERVER or tmpoptype == GETRES_LATEST_SEQ_SERVER_INSWITCH or tmpoptype == GETRES_DELETED_SEQ_SERVER_INSWITCH:
                        actnspec0 = distfarreachleaf_spineselect_for_special_response_action_spec_t(global_spineswitch_logical_idx)
                        self.client.spineselect_tbl_table_add_with_spineselect_for_special_response(\
                                self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                    else:
                        actnspec0 = distfarreachleaf_spineselect_action_spec_t(eport, global_spineswitch_logical_idx)
                        self.client.spineselect_tbl_table_add_with_spineselect(\
                                self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                    key_start = key_end + 1

            # Stage 2

            if RANGE_SUPPORT == True:
                # Table: range_partition_tbl (default: nop; size <= 18 * 128)
                print "Configuring range_partition_tbl"
                key_range_per_server = pow(2, 16) / server_total_logical_num
                key_range_per_leafswitch = pow(2, 16) / leafswitch_total_logical_num
                servernum_per_leafswitch = server_total_logical_num / leafswitch_total_logical_num
                for tmpoptype in [GETREQ_SPINE, CACHE_POP_INSWITCH, PUTREQ_SEQ, DELREQ_SEQ, WARMUPREQ_SPINE, SCANREQ_SPLIT, LOADREQ_SPINE, CACHE_EVICT_LOADFREQ_INSWITCH, CACHE_EVICT_LOADDATA_INSWITCH, LOADSNAPSHOTDATA_INSWITCH, SETVALID_INSWITCH, PUTREQ_SEQ_INSWITCH, DELREQ_SEQ_INSWITCH, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER, GETRES_LATEST_SEQ_SERVER_INSWITCH, GETRES_DELETED_SEQ_SERVER_INSWITCH, GETREQ_NLATEST]:
                    valid_serveridx_start = 0 # [0, server_total_logical_num-1]
                    valid_key_start = 0 # [0, 2^16-1]
                    for i in range(leafswitch_total_logical_num):
                        # Get valid key range and serveridx range
                        global_leafswitch_logical_idx = leafswitch_logical_idxes[i]
                        if i == leafswitch_total_logical_num - 1:
                            valid_serveridx_end = server_total_logical_num - 1
                            valid_key_end = pow(2, 16) - 1
                        else:
                            valid_serveridx_end = valid_serveridx_start + servernum_per_leafswitch - 1
                            valid_key_end = valid_key_start + key_range_per_leafswitch - 1
                        # For each server in [valid_serveridx_start, valid_serveridx_end]
                        for j in range(servernum_per_leafswitch):
                            global_server_logical_idx = valid_serveridx_start + j
                            if global_server_logical_idx > valid_serveridx_end:
                                print "[ERROR] invalid serveridx[{}] {} for leafswitch[{}] {} which should <= {}".format(j, global_server_logical_idx, i, global_leafswitch_logical_idx, valid_serveridx_end)
                                exit(-1)

                            # Get key_start and key_end for match_spec_t
                            key_start = valid_key_start + j * key_range_per_server
                            key_end = key_start + key_range_per_server - 1
                            if j == 0:
                                key_start = 0
                                #key_end = valid_key_start + key_range_per_server - 1
                            if j == servernum_per_leafswitch - 1:
                                #key_start = valid_key_start + j * key_range_per_server
                                key_end = pow(2, 16) - 1

                            # NOTE: both start and end are included
                            matchspec0 = distfarreachleaf_range_partition_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    op_hdr_keyhihihi_start = convert_u16_to_i16(key_start),
                                    op_hdr_keyhihihi_end = convert_u16_to_i16(key_end),
                                    op_hdr_globalswitchidx = convert_u16_to_i16(global_leafswitch_logical_idx),
                                    meta_need_recirculate = 0)
                            # Forward to the egress pipeline of server
                            server_physical_idx = -1
                            local_server_logical_idx = -1
                            for tmp_server_physical_idx in range(server_physical_num):
                                for tmp_local_server_logical_idx in range(len(server_logical_idxes_list[tmp_server_physical_idx])):
                                    if global_server_logical_idx == server_logical_idxes_list[tmp_server_physical_idx][tmp_local_server_logical_idx]:
                                        server_physical_idx = tmp_server_physical_idx
                                        local_server_logical_idx = tmp_local_server_logical_idx
                                        break
                            if server_physical_idx == -1:
                                print "WARNING: no physical server covers global_server_logical_idx {} -> no corresponding MAT entries in range_partition_tbl".format(global_server_logical_idx)
                            else:
                                #udp_dstport = server_worker_port_start + global_server_logical_idx
                                udp_dstport = server_worker_port_start + local_server_logical_idx
                                eport = self.server_devports[server_physical_idx]
                                if tmpoptype == GETRES_LATEST_SEQ_SERVER or tmpoptype == GETRES_DELETED_SEQ_SERVER or tmpoptype == GETRES_LATEST_SEQ_SERVER_INSWITCH or tmpoptype == GETRES_DELETED_SEQ_SERVER_INSWITCH:
                                    actnspec0 = distfarreachleaf_range_partition_for_special_response_action_spec_t(eport)
                                    self.client.range_partition_tbl_table_add_with_range_partition_for_special_response(\
                                            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                                elif tmpoptype != SCANREQ_SPLIT:
                                    actnspec0 = distfarreachleaf_range_partition_action_spec_t(udp_dstport, eport)
                                    self.client.range_partition_tbl_table_add_with_range_partition(\
                                            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                                else:
                                    actnspec0 = distfarreachleaf_range_partition_for_scan_action_spec_t(udp_dstport, eport, global_server_logical_idx)
                                    self.client.range_partition_tbl_table_add_with_range_partition_for_scan(\
                                            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                        valid_serveridx_start = valid_serveridx_end + 1
                        valid_key_start = valid_key_end + 1
            else:
                # Table: hash_partition_tbl (default: nop; size <= 17 * 128)
                print "Configuring hash_partition_tbl"
                hash_range_per_server = switch_partition_count / server_total_logical_num
                for tmpoptype in [GETREQ_SPINE, CACHE_POP_INSWITCH, PUTREQ_SEQ, DELREQ_SEQ, WARMUPREQ_SPINE, LOADREQ_SPINE, CACHE_EVICT_LOADFREQ_INSWITCH, CACHE_EVICT_LOADDATA_INSWITCH, LOADSNAPSHOTDATA_INSWITCH, SETVALID_INSWITCH, PUTREQ_SEQ_INSWITCH, DELREQ_SEQ_INSWITCH, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER, GETRES_LATEST_SEQ_SERVER_INSWITCH, GETRES_DELETED_SEQ_SERVER_INSWITCH, GETREQ_NLATEST]:
                    hash_start = 0 # [0, partition_count-1]
                    for global_server_logical_idx in range(server_total_logical_num):
                        if global_server_logical_idx == server_total_logical_num - 1:
                            hash_end = switch_partition_count - 1 # if end is not included, then it is just processed by port 1111
                        else:
                            hash_end = hash_start + hash_range_per_server - 1
                        # NOTE: both start and end are included
                        matchspec0 = distfarreachleaf_hash_partition_tbl_match_spec_t(\
                                op_hdr_optype = convert_u16_to_i16(tmpoptype),
                                meta_hashval_for_partition_start = convert_u16_to_i16(hash_start),
                                meta_hashval_for_partition_end = convert_u16_to_i16(hash_end),
                                meta_need_recirculate = 0)
                        # Forward to the egress pipeline of server
                        server_physical_idx = -1
                        local_server_logical_idx = -1
                        for tmp_server_physical_idx in range(server_physical_num):
                            for tmp_local_server_logical_idx in range(len(server_logical_idxes_list[tmp_server_physical_idx])):
                                if global_server_logical_idx == server_logical_idxes_list[tmp_server_physical_idx][tmp_local_server_logical_idx]:
                                    server_physical_idx = tmp_server_physical_idx
                                    local_server_logical_idx = tmp_local_server_logical_idx
                                    break
                        if server_physical_idx == -1:
                            print "WARNING: no physical server covers global_server_logical_idx {} -> no corresponding MAT entries in hash_partition_tbl".format(global_server_logical_idx)
                        else:
                            #udp_dstport = server_worker_port_start + global_server_logical_idx
                            udp_dstport = server_worker_port_start + local_server_logical_idx
                            eport = self.server_devports[server_physical_idx]
                            if tmpoptype == GETRES_LATEST_SEQ_SERVER or tmpoptype == GETRES_DELETED_SEQ_SERVER or tmpoptype == GETRES_LATEST_SEQ_SERVER_INSWITCH or tmpoptype == GETRES_DELETED_SEQ_SERVER_INSWITCH:
                                actnspec0 = distfarreachleaf_hash_partition_for_special_response_action_spec_t(eport)
                                self.client.hash_partition_tbl_table_add_with_hash_partition_for_special_response(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                            else:
                                actnspec0 = distfarreachleaf_hash_partition_action_spec_t(udp_dstport, eport)
                                self.client.hash_partition_tbl_table_add_with_hash_partition(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                        hash_start = hash_end + 1

            # Stage 3

            if RANGE_SUPPORT == True:
                # Table: range_partition_for_scan_endkey_tbl (default: nop; size <= 1 * 128)
                # TODO: limit max_scannum <= constant (e.g., 32)
                print "Configuring range_partition_for_scan_endkey_tbl"
                key_range_per_server = pow(2, 16) / server_total_logical_num
                key_range_per_leafswitch = pow(2, 16) / leafswitch_total_logical_num
                servernum_per_leafswitch = server_total_logical_num / leafswitch_total_logical_num
                valid_serveridx_start = 0 # [0, server_total_logical_num-1]
                valid_key_start = 0 # [0, 2^16-1]
                for i in range(leafswitch_total_logical_num):
                    # Get valid key range and serveridx range
                    global_leafswitch_logical_idx = leafswitch_logical_idxes[i]
                    if i == leafswitch_total_logical_num - 1:
                        valid_serveridx_end = server_total_logical_num - 1
                        valid_key_end = pow(2, 16) - 1
                    else:
                        valid_serveridx_end = valid_serveridx_start + servernum_per_leafswitch - 1
                        valid_key_end = valid_key_start + key_range_per_leafswitch - 1
                    # For each server in [valid_serveridx_start, valid_serveridx_end]
                    for j in range(servernum_per_leafswitch):
                        global_server_logical_idx = valid_serveridx_start + j
                        if global_server_logical_idx > valid_serveridx_end:
                            print "[ERROR] invalid serveridx[{}] {} for leafswitch[{}] {} which should <= {}".format(j, global_server_logical_idx, i, global_leafswitch_logical_idx, valid_serveridx_end)
                            exit(-1)

                        # Get endkey_start and endkey_end for match_spec_t
                        endkey_start = valid_key_start + j * key_range_per_server
                        endkey_end = endkey_start + key_range_per_server - 1
                        if j == 0:
                            endkey_start = 0
                            #endkey_end = valid_key_start + key_range_per_server - 1
                        if j == servernum_per_leafswitch - 1:
                            #endkey_start = valid_key_start + j * key_range_per_server
                            endkey_end = pow(2, 16) - 1

                        # NOTE: both start and end are included
                        matchspec0 = distfarreachleaf_range_partition_for_scan_endkey_tbl_match_spec_t(\
                                op_hdr_optype = SCANREQ_SPLIT,
                                scan_hdr_keyhihihi_start = convert_u16_to_i16(endkey_start),
                                scan_hdr_keyhihihi_end = convert_u16_to_i16(endkey_end),
                                op_hdr_globalswitchidx = convert_u16_to_i16(global_leafswitch_logical_idx),
                                meta_need_recirculate = 0)
                        end_globalserveridx_plus_one = global_server_logical_idx + 1 # used to calculate max_scannum in data plane
                        actnspec0 = distfarreachleaf_range_partition_for_scan_endkey_action_spec_t(end_globalserveridx_plus_one)
                        # set cur_scanidx = 0; set max_scannum = last_udpport_plus_one - udp_hdr.dstPort (first_udpport)
                        self.client.range_partition_for_scan_endkey_tbl_table_add_with_range_partition_for_scan_endkey(\
                                self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                    valid_serveridx_start = valid_serveridx_end + 1
                    valid_key_start = valid_key_end + 1

            # Table: cache_lookup_tbl (default: uncached_action; size: 32K/64K)
            print "Leave cache_lookup_tbl managed by controller in runtime"

            # Table: hash_for_cm1/2/3/4_tbl (default: nop; size: 4)
            for i in range(1, 5):
                print "Configuring hash_for_cm{}_tbl".format(i)
                for tmpoptype in [GETREQ_SPINE, PUTREQ_SEQ, PUTREQ_SEQ_INSWITCH]:
                    matchspec0 = eval("distfarreachleaf_hash_for_cm{}_tbl_match_spec_t".format(i))(\
                            op_hdr_optype = tmpoptype,
                            meta_need_recirculate = 0)
                    eval("self.client.hash_for_cm{}_tbl_table_add_with_hash_for_cm{}".format(i, i))(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 3

            # Table: snapshot_flag_tbl (default: reset_snapshot_flag; size: 8)
            print "Configuring snapshot_flag_tbl"
            # See ptf_snapshotserver/table_configure.py for details, where we set snapshot_flag for PUT/DELREQ and GETRES_LATEST/GETRES_DELETED_SEQ_SERVER
            # NOTE: we explicitly invoke nop() for these pkts to avoid from overwriting inswitch_hdr.snapshot_flag set by spine switch
            # NOTE: update initial_entrynum_for_snapshot_flag in ptf_snapshotserver accordingly
            for tmpoptype in [PUTREQ_SEQ_INSWITCH, DELREQ_SEQ_INSWITCH, GETRES_LATEST_SEQ_SERVER_INSWITCH, GETRES_DELETED_SEQ_SERVER_INSWITCH]:
                matchspec0 = distfarreachleaf_snapshot_flag_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_need_recirculate = 0)
                self.client.snapshot_flag_tbl_table_add_with_nop(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: prepare_for_cachehit_tbl (default: set_client_sid(0); size: 5*client_physical_num=10 < 5*8=40 < 64)
            print "Configuring prepare_for_cachehit_tbl"
            for client_physical_idx in range(client_physical_num):
                tmp_clientip = client_ips[client_physical_idx]
                for tmpoptype in [GETREQ_SPINE, PUTREQ_SEQ, DELREQ_SEQ, PUTREQ_SEQ_INSWITCH, DELREQ_SEQ_INSWITCH]:
                    matchspec0 = distfarreachleaf_prepare_for_cachehit_tbl_match_spec_t(\
                            op_hdr_optype = tmpoptype,
                            #ig_intr_md_ingress_port = self.spineswitch_devport,
                            ipv4_hdr_srcAddr = ipv4Addr_to_i32(tmp_clientip),
                            ipv4_hdr_srcAddr_prefix_length = 32,
                            meta_need_recirculate = 0)
                    actnspec0 = distfarreachleaf_set_client_sid_action_spec_t(self.spineswitch_sid)
                    self.client.prepare_for_cachehit_tbl_table_add_with_set_client_sid(\
                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Table: ipv4_forward_tbl (default: nop; size: 2*(16*client_physical_num)=64 < 256)
            print "Configuring ipv4_forward_tbl"
            for is_cached in cached_list:
                for tmp_client_physical_idx in range(client_physical_num):
                    ipv4addr0 = ipv4Addr_to_i32(client_ips[tmp_client_physical_idx])
                    eport = self.client_devports[tmp_client_physical_idx]
                    tmpsid = self.client_sids[tmp_client_physical_idx]
                    for tmpoptype in [GETRES, PUTRES, DELRES, WARMUPACK, SCANRES_SPLIT, LOADACK]:
                        matchspec0 = distfarreachleaf_ipv4_forward_tbl_match_spec_t(\
                                op_hdr_optype = convert_u16_to_i16(tmpoptype),
                                ipv4_hdr_dstAddr = ipv4addr0,
                                ipv4_hdr_dstAddr_prefix_length = 32,
                                inswitch_hdr_is_cached = is_cached,
                                meta_need_recirculate = 0) # NOTE: meta.need_recirculate must be 0 for those packets
                        actnspec0 = distfarreachleaf_forward_normal_response_action_spec_t(eport)
                        self.client.ipv4_forward_tbl_table_add_with_forward_normal_response(\
                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                    eport = self.spineswitch_devport
                    tmpsid = self.spineswitch_sid
                    for tmpoptype in [GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER, GETRES_LATEST_SEQ_SERVER_INSWITCH, GETRES_DELETED_SEQ_SERVER_INSWITCH]:
                        matchspec0 = distfarreachleaf_ipv4_forward_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                ipv4_hdr_dstAddr = ipv4addr0,
                                ipv4_hdr_dstAddr_prefix_length = 32,
                                inswitch_hdr_is_cached = is_cached,
                                meta_need_recirculate = 0)
                        if is_cached == 0:
                            actnspec0 = distfarreachleaf_forward_special_get_response_to_spine_action_spec_t(eport)
                            self.client.ipv4_forward_tbl_table_add_with_forward_special_get_response_to_spine(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                        elif is_cached == 1:
                            actnspec0 = distfarreachleaf_forward_special_get_response_to_server_and_clone_to_spine_action_spec_t(tmpsid)
                            self.client.ipv4_forward_tbl_table_add_with_forward_special_get_response_to_server_and_clone_to_spine(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                    for tmpoptype in [GETRES_SERVER, SCANRES_SPLIT_SERVER, PUTRES_SERVER, DELRES_SERVER, WARMUPACK_SERVER, LOADACK_SERVER]:
                        matchspec0 = distfarreachleaf_ipv4_forward_tbl_match_spec_t(\
                                op_hdr_optype = convert_u16_to_i16(tmpoptype),
                                ipv4_hdr_dstAddr = ipv4addr0,
                                ipv4_hdr_dstAddr_prefix_length = 32,
                                inswitch_hdr_is_cached = is_cached,
                                meta_need_recirculate = 0) # NOTE: meta.need_recirculate must be 0 for those packets
                        actnspec0 = distfarreachleaf_forward_normal_response_action_spec_t(eport)
                        self.client.ipv4_forward_tbl_table_add_with_forward_normal_response(\
                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Stage 4

            # Table: sample_tbl (default: nop; size: 3)
            print "Configuring sample_tbl"
            for tmpoptype in [GETREQ_SPINE, PUTREQ_SEQ, PUTREQ_SEQ_INSWITCH]:
                matchspec0 = distfarreachleaf_sample_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_need_recirculate = 0)
                self.client.sample_tbl_table_add_with_sample(\
                        self.sess_hdl, self.dev_tgt, matchspec0)


            # Table: ig_port_forward_tbl (default: nop; size: 15)
            print "Configuring ig_port_forward_tbl"
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETREQ_SPINE,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_getreq_spine_to_getreq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_LATEST_SEQ_SERVER,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_getres_latest_seq_server_to_getres_latest_seq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_DELETED_SEQ_SERVER,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_getres_deleted_seq_server_to_getres_deleted_seq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_SEQ,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_putreq_seq_to_putreq_seq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = DELREQ_SEQ,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_delreq_seq_to_delreq_seq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_SERVER,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_getres_server_to_getres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = SCANRES_SPLIT_SERVER,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_scanres_split_server_to_scanres_split(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = PUTRES_SERVER,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_putres_server_to_putres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = DELRES_SERVER,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_delres_server_to_delres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = WARMUPREQ_SPINE,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_warmupreq_spine_to_warmupreq(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = LOADREQ_SPINE,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_loadreq_spine_to_loadreq(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = WARMUPACK_SERVER,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_warmupack_server_to_warmupack(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = LOADACK_SERVER,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_loadack_server_to_loadack(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_LATEST_SEQ_SERVER_INSWITCH,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_getres_latest_seq_server_inswitch_to_getres_latest_seq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_DELETED_SEQ_SERVER_INSWITCH,
                    meta_need_recirculate = 0)
            self.client.ig_port_forward_tbl_table_add_with_update_getres_deleted_seq_server_inswitch_to_getres_deleted_seq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Egress pipeline

            # Stage 0

            # Table: access_cmi_tbl (default: initialize_cmi_predicate; size: 2)
            cm_hashnum = 4
            for i in range(1, cm_hashnum+1):
                print "Configuring access_cm{}_tbl".format(i)
                for tmpoptype in [GETREQ_INSWITCH, PUTREQ_SEQ_INSWITCH]:
                    matchspec0 = eval("distfarreachleaf_access_cm{}_tbl_match_spec_t".format(i))(\
                            op_hdr_optype = tmpoptype,
                            inswitch_hdr_is_sampled = 1,
                            inswitch_hdr_is_cached = 0)
                    eval("self.client.access_cm{}_tbl_table_add_with_update_cm{}".format(i, i))(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            if RANGE_SUPPORT:
                # Table: process_scanreq_split_tbl (default: reset_meta_serversid_remainscannum; size <= 2 * 128)
                print "Configuring process_scanreq_split_tbl"
                #for clone_src in [NOT_CLONED, CLONED_FROM_EGRESS]:
                for is_clone in [0, 1]:
                    for global_server_logical_idx in range(server_total_logical_num):
                        #dstport = server_worker_port_start + global_server_logical_idx
                        matchspec0 = distfarreachleaf_process_scanreq_split_tbl_match_spec_t(\
                                op_hdr_optype = SCANREQ_SPLIT,
                                #udp_hdr_dstPort = dstport,
                                split_hdr_globalserveridx = global_server_logical_idx,
                                split_hdr_is_clone = is_clone)
                                #eg_intr_md_from_parser_aux_clone_src = clone_src)
                        #if clone_src == NOT_CLONED:
                        if is_clone == 0:
                            ## get server logical idx for dstport + 1 (aka global_server_logical_idx + 1)
                            # get server logical idx for split_hdr.globalserveridx + 1 (next SCANREQ_SPLIT)
                            tmpidx = global_server_logical_idx + 1
                            if global_server_logical_idx >= server_total_logical_num - 1: # NOTE: we do not check tmpidx here
                                # max_scannum must be 1 -> is_last_scansplit must be 1 -> direct forward SCANREQ_SPLIT without cloning (meta.server_sid is not used in eg_port_forward_tbl)
                                tmpidx = server_total_logical_num - 1
                            # get server sid for dstport + 1 (aka global_server_logical_idx + 1)
                            server_physical_idx = -1
                            for tmp_server_physical_idx in range(server_physical_num):
                                if tmpidx in server_logical_idxes_list[tmp_server_physical_idx]:
                                    server_physical_idx = tmp_server_physical_idx
                                    break
                            if server_physical_idx == -1:
                                print "WARNING: no physical server covers global_server_logical_idx {} -> no corresponding MAT entries in process_scanreq_split_tbl".format(global_server_logical_idx)
                            else:
                                tmp_server_sid = self.server_sids[server_physical_idx]
                                actnspec0 = distfarreachleaf_process_scanreq_split_action_spec_t(tmp_server_sid)
                                self.client.process_scanreq_split_tbl_table_add_with_process_scanreq_split(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                        #elif clone_src == CLONED_FROM_EGRESS:
                        elif is_clone == 1:
                            ## get server logical idx for dstport + 2 (aka global_server_logical_idx + 2)
                            #tmpidx = global_server_logical_idx + 2
                            #if global_server_logical_idx >= server_total_logical_num - 2: # NOTE: we do not check tmpidx here
                                ## dstport == the last second logical server -> current pkt must be cloned to the last logical server -> is_last_scansplit must be 1 -> direct forward SCANREQ_SPLIT without cloning (meta.server_sid is not used in eg_port_forward_tbl)
                                ## actually dstport cannot be the last logical server, as the pkt to the last logical server will not clone any packet
                                #tmpidx = server_total_logical_num - 1

                            # get server logical idx for split_hdr.globalserveridx + 1 (NOTE: we increase split_hdr.globalserveridx in eg_port_forward_tbl as split_hdr.cur_scanidx)
                            tmpidx = global_server_logical_idx + 1
                            if global_server_logical_idx >= server_total_logical_num - 1: # NOTE: we do not check tmpidx here
                                # split_hdr.globalserveridx == the last logical server -> current pkt must be cloned to the last logical server -> is_last_scansplit must be 1 -> direct forward SCANREQ_SPLIT without cloning (meta.server_sid is not used in eg_port_forward_tbl)
                                tmpidx = server_total_logical_num - 1

                            # get udp.dstport for global_server_logical_idx (serveridx of current SCANREQ_SPLIT)
                            current_local_server_logical_idx = -1
                            for tmp_server_physical_idx in range(server_physical_num):
                                for tmp_local_server_logical_idx in range(len(server_logical_idxes_list[tmp_server_physical_idx])):
                                    if global_server_logical_idx == server_logical_idxes_list[tmp_server_physical_idx][tmp_local_server_logical_idx]:
                                        current_local_server_logical_idx = tmp_local_server_logical_idx
                                        break
                            ## get server sid for dstport + 2 (aka global_server_logical_idx + 2)
                            # get server sid for global_server_logical_idx + 1 (next SCANREQ_SPLIT)
                            next_server_physical_idx = -1
                            for tmp_server_physical_idx in range(server_physical_num):
                                if tmpidx in server_logical_idxes_list[tmp_server_physical_idx]:
                                    next_server_physical_idx = tmp_server_physical_idx
                                    break
                            if current_local_server_logical_idx == -1 or next_server_physical_idx == -1:
                                print "WARNING: no physical server covers global_server_logical_idx {} -> no corresponding MAT entries in process_scanreq_split_tbl".format(global_server_logical_idx)
                            else:
                                tmp_udpport = server_worker_port_start + current_local_server_logical_idx
                                tmp_server_sid = self.server_sids[next_server_physical_idx]
                                actnspec0 = distfarreachleaf_process_cloned_scanreq_split_action_spec_t(tmp_udpport, tmp_server_sid)
                                self.client.process_scanreq_split_tbl_table_add_with_process_cloned_scanreq_split(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Stgae 1

            # Table: is_hot_tbl (default: reset_is_hot; size: 1)
            print "Configuring is_hot_tbl"
            matchspec0 = distfarreachleaf_is_hot_tbl_match_spec_t(\
                    meta_cm1_predicate = 2,
                    meta_cm2_predicate = 2,
                    meta_cm3_predicate = 2,
                    meta_cm4_predicate = 2)
            self.client.is_hot_tbl_table_add_with_set_is_hot(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_cache_frequency_tbl (default: nop; size: 10)
            print "Configuring access_cache_frequency_tbl"
            for tmpoptype in [GETREQ_INSWITCH, PUTREQ_SEQ_INSWITCH]:
                matchspec0 = distfarreachleaf_access_cache_frequency_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        inswitch_hdr_is_sampled = 1,
                        inswitch_hdr_is_cached = 1)
                self.client.access_cache_frequency_tbl_table_add_with_update_cache_frequency(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
            for is_sampled in sampled_list:
                for is_cached in cached_list:
                    matchspec0 = distfarreachleaf_access_cache_frequency_tbl_match_spec_t(\
                            op_hdr_optype = CACHE_POP_INSWITCH,
                            inswitch_hdr_is_sampled = is_sampled,
                            inswitch_hdr_is_cached = is_cached)
                    self.client.access_cache_frequency_tbl_table_add_with_reset_cache_frequency(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
                    matchspec0 = distfarreachleaf_access_cache_frequency_tbl_match_spec_t(\
                            op_hdr_optype = CACHE_EVICT_LOADFREQ_INSWITCH,
                            inswitch_hdr_is_sampled = is_sampled,
                            inswitch_hdr_is_cached = is_cached)
                    self.client.access_cache_frequency_tbl_table_add_with_get_cache_frequency(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_validvalue_tbl (default: reset_meta_validvalue; size: 7)
            print "Configuring access_validvalue_tbl"
            for tmpoptype in [GETREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, PUTREQ_SEQ_INSWITCH, DELREQ_SEQ_INSWITCH]:
                matchspec0 = distfarreachleaf_access_validvalue_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        inswitch_hdr_is_cached = 1)
                self.client.access_validvalue_tbl_table_add_with_get_validvalue(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
            for is_cached in cached_list:
                # NOTE: set_validvalue does not change validvalue_hdr.validvalue
                matchspec0 = distfarreachleaf_access_validvalue_tbl_match_spec_t(\
                        op_hdr_optype = SETVALID_INSWITCH,
                        inswitch_hdr_is_cached = is_cached) # key may or may not be cached for SETVALID_INSWITCH
                self.client.access_validvalue_tbl_table_add_with_set_validvalue(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Stgae 2

            # Table: save_client_info_tbl (default: nop; size: 4)
            print "Configuring save_client_info_tbl"
            for tmpoptype in[GETREQ_INSWITCH, PUTREQ_SEQ_INSWITCH, DELREQ_SEQ_INSWITCH]:
                matchspec0 = distfarreachleaf_save_client_info_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype)
                self.client.save_client_info_tbl_table_add_with_save_client_info(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_latest_tbl (default: reset_is_latest; size: 18)
            print "Configuring access_latest_tbl"
            for is_cached in cached_list:
                for validvalue in validvalue_list:
                    matchspec0 = distfarreachleaf_access_latest_tbl_match_spec_t(\
                            op_hdr_optype = GETREQ_INSWITCH,
                            inswitch_hdr_is_cached = is_cached,
                            validvalue_hdr_validvalue = validvalue)
                    if is_cached == 1:
                        self.client.access_latest_tbl_table_add_with_get_latest(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                    for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                        matchspec0 = distfarreachleaf_access_latest_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                inswitch_hdr_is_cached = is_cached,
                                validvalue_hdr_validvalue = validvalue)
                        if is_cached == 1 and validvalue == 1:
                            self.client.access_latest_tbl_table_add_with_set_and_get_latest(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                    for tmpoptype in [PUTREQ_SEQ_INSWITCH, DELREQ_SEQ_INSWITCH]:
                        matchspec0 = distfarreachleaf_access_latest_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                inswitch_hdr_is_cached = is_cached,
                                validvalue_hdr_validvalue = validvalue)
                        if is_cached == 1 and validvalue == 1:
                            self.client.access_latest_tbl_table_add_with_set_and_get_latest(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        elif is_cached == 1 and validvalue == 3:
                            self.client.access_latest_tbl_table_add_with_reset_and_get_latest(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                    matchspec0 = distfarreachleaf_access_latest_tbl_match_spec_t(\
                            op_hdr_optype = CACHE_POP_INSWITCH,
                            inswitch_hdr_is_cached = is_cached,
                            validvalue_hdr_validvalue = validvalue)
                    self.client.access_latest_tbl_table_add_with_reset_and_get_latest(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 3

            # Table: access_deleted_tbl (default: reset_is_deleted; size: 122)
            print "Configuring access_deleted_tbl"
            for is_cached in cached_list:
                for validvalue in validvalue_list:
                    for is_latest in latest_list:
                        for is_stat in stat_list:
                            matchspec0 = distfarreachleaf_access_deleted_tbl_match_spec_t(\
                                    op_hdr_optype = GETREQ_INSWITCH,
                                    inswitch_hdr_is_cached = is_cached,
                                    validvalue_hdr_validvalue = validvalue,
                                    meta_is_latest = is_latest,
                                    stat_hdr_stat = is_stat)
                            if is_cached == 1:
                                self.client.access_deleted_tbl_table_add_with_get_deleted(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            matchspec0 = distfarreachleaf_access_deleted_tbl_match_spec_t(\
                                    op_hdr_optype = GETRES_LATEST_SEQ_INSWITCH,
                                    inswitch_hdr_is_cached = is_cached,
                                    validvalue_hdr_validvalue = validvalue,
                                    meta_is_latest = is_latest,
                                    stat_hdr_stat = is_stat)
                            if is_cached == 1 and validvalue == 1 and is_latest == 0 and is_stat == 1:
                                self.client.access_deleted_tbl_table_add_with_reset_and_get_deleted(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            matchspec0 = distfarreachleaf_access_deleted_tbl_match_spec_t(\
                                    op_hdr_optype = GETRES_DELETED_SEQ_INSWITCH,
                                    inswitch_hdr_is_cached = 1,
                                    validvalue_hdr_validvalue = validvalue,
                                    meta_is_latest = is_latest,
                                    stat_hdr_stat = is_stat)
                            if is_cached == 1 and validvalue == 1 and is_latest == 0 and is_stat == 0:
                                self.client.access_deleted_tbl_table_add_with_set_and_get_deleted(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            matchspec0 = distfarreachleaf_access_deleted_tbl_match_spec_t(\
                                    op_hdr_optype = PUTREQ_SEQ_INSWITCH,
                                    inswitch_hdr_is_cached = is_cached,
                                    validvalue_hdr_validvalue = validvalue,
                                    meta_is_latest = is_latest,
                                    stat_hdr_stat = is_stat)
                            if is_cached == 1 and validvalue == 1:
                                self.client.access_deleted_tbl_table_add_with_reset_and_get_deleted(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            matchspec0 = distfarreachleaf_access_deleted_tbl_match_spec_t(\
                                    op_hdr_optype = DELREQ_SEQ_INSWITCH,
                                    inswitch_hdr_is_cached = is_cached,
                                    validvalue_hdr_validvalue = validvalue,
                                    meta_is_latest = is_latest,
                                    stat_hdr_stat = is_stat)
                            if is_cached == 1 and validvalue == 1:
                                self.client.access_deleted_tbl_table_add_with_set_and_get_deleted(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            matchspec0 = distfarreachleaf_access_deleted_tbl_match_spec_t(\
                                    op_hdr_optype = CACHE_POP_INSWITCH,
                                    inswitch_hdr_is_cached = is_cached,
                                    validvalue_hdr_validvalue = validvalue,
                                    meta_is_latest = is_latest,
                                    stat_hdr_stat = is_stat)
                            if is_stat == 1:
                                self.client.access_deleted_tbl_table_add_with_reset_and_get_deleted(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            elif is_stat == 0:
                                self.client.access_deleted_tbl_table_add_with_set_and_get_deleted(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            for tmpoptype in [CACHE_EVICT_LOADDATA_INSWITCH, LOADSNAPSHOTDATA_INSWITCH]:
                                matchspec0 = distfarreachleaf_access_deleted_tbl_match_spec_t(\
                                        op_hdr_optype = tmpoptype,
                                        inswitch_hdr_is_cached = is_cached,
                                        validvalue_hdr_validvalue = validvalue,
                                        meta_is_latest = is_latest,
                                        stat_hdr_stat = is_stat)
                                self.client.access_deleted_tbl_table_add_with_get_deleted(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: update_vallen_tbl (default: reset_access_val_mode; 62)
            print "Configuring update_vallen_tbl"
            for is_cached in cached_list:
                for validvalue in validvalue_list:
                    for is_latest in latest_list:
                        matchspec0 = distfarreachleaf_update_vallen_tbl_match_spec_t(\
                                op_hdr_optype = GETREQ_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                validvalue_hdr_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        if is_cached == 1:
                            self.client.update_vallen_tbl_table_add_with_get_vallen(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                            matchspec0 = distfarreachleaf_update_vallen_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    inswitch_hdr_is_cached = is_cached,
                                    validvalue_hdr_validvalue = validvalue,
                                    meta_is_latest = is_latest)
                            if is_cached == 1 and validvalue == 1 and is_latest == 0:
                                self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = distfarreachleaf_update_vallen_tbl_match_spec_t(\
                                op_hdr_optype = PUTREQ_SEQ_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                validvalue_hdr_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        if is_cached == 1 and validvalue == 1:
                            self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = distfarreachleaf_update_vallen_tbl_match_spec_t(\
                                op_hdr_optype = DELREQ_SEQ_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                validvalue_hdr_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        if is_cached == 1 and validvalue == 1:
                            self.client.update_vallen_tbl_table_add_with_reset_and_get_vallen(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = distfarreachleaf_update_vallen_tbl_match_spec_t(\
                                op_hdr_optype = CACHE_POP_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                validvalue_hdr_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                        for tmpoptype in [CACHE_EVICT_LOADDATA_INSWITCH, LOADSNAPSHOTDATA_INSWITCH]:
                            matchspec0 = distfarreachleaf_update_vallen_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    inswitch_hdr_is_cached = is_cached,
                                    validvalue_hdr_validvalue = validvalue,
                                    meta_is_latest = is_latest)
                            self.client.update_vallen_tbl_table_add_with_get_vallen(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_savedseq_tbl (default: nop; size: 54)
            print "Configuring access_savedseq_tbl"
            for is_cached in cached_list:
                for validvalue in validvalue_list:
                    for is_latest in latest_list:
                        for tmpoptype in [PUTREQ_SEQ_INSWITCH, DELREQ_SEQ_INSWITCH]:
                            matchspec0 = distfarreachleaf_access_savedseq_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    inswitch_hdr_is_cached = is_cached,
                                    validvalue_hdr_validvalue = validvalue,
                                    meta_is_latest = is_latest)
                            if is_cached == 1 and validvalue == 1:
                                self.client.access_savedseq_tbl_table_add_with_set_and_get_savedseq(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                        for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                            matchspec0 = distfarreachleaf_access_savedseq_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    inswitch_hdr_is_cached = is_cached,
                                    validvalue_hdr_validvalue = validvalue,
                                    meta_is_latest = is_latest)
                            if is_cached == 1 and validvalue == 1 and is_latest == 0:
                                self.client.access_savedseq_tbl_table_add_with_set_and_get_savedseq(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = distfarreachleaf_access_savedseq_tbl_match_spec_t(\
                                op_hdr_optype = CACHE_POP_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                validvalue_hdr_validvalue = validvalue,
                                meta_is_latest = is_latest)
                        self.client.access_savedseq_tbl_table_add_with_set_and_get_savedseq(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                        for tmpoptype in [CACHE_EVICT_LOADDATA_INSWITCH, LOADSNAPSHOTDATA_INSWITCH]:
                            matchspec0 = distfarreachleaf_access_savedseq_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    inswitch_hdr_is_cached = is_cached,
                                    validvalue_hdr_validvalue = validvalue,
                                    meta_is_latest = is_latest)
                            self.client.access_savedseq_tbl_table_add_with_get_savedseq(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_case1_tbl (default: reset_is_case1; 6)
            print "Configuring access_case1_tbl"
            for is_latest in latest_list:
                for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                    matchspec0 = distfarreachleaf_access_case1_tbl_match_spec_t(\
                            op_hdr_optype = tmpoptype,
                            inswitch_hdr_is_cached = 1,
                            validvalue_hdr_validvalue = 1,
                            meta_is_latest = is_latest,
                            inswitch_hdr_snapshot_flag = 1)
                    if is_latest == 0:
                        self.client.access_case1_tbl_table_add_with_try_case1(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                for tmpoptype in [PUTREQ_SEQ_INSWITCH, DELREQ_SEQ_INSWITCH]:
                    matchspec0 = distfarreachleaf_access_case1_tbl_match_spec_t(\
                            op_hdr_optype = tmpoptype,
                            inswitch_hdr_is_cached = 1,
                            validvalue_hdr_validvalue = 1,
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

            # Table: lastclone_lastscansplit_tbl (default: reset_is_lastclone_lastscansplit; size: 4)
            print "Configuring lastclone_lastscansplit_tbl"
            if RANGE_SUPPORT == False:
                # CACHE_POP_INSWITCH_ACK
                for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1]:
                    matchspec0 = distfarreachleaf_lastclone_lastscansplit_tbl_match_spec_t(\
                            op_hdr_optype = tmpoptype,
                            clone_hdr_clonenum_for_pktloss = 0)
                    self.client.lastclone_lastscansplit_tbl_table_add_with_set_is_lastclone(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
            else:
                # CACHE_POP_INSWITCH_ACK
                for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1]:
                    matchspec0 = distfarreachleaf_lastclone_lastscansplit_tbl_match_spec_t(\
                            op_hdr_optype = tmpoptype,
                            clone_hdr_clonenum_for_pktloss = 0,
                            meta_remain_scannum = 0)
                    self.client.lastclone_lastscansplit_tbl_table_add_with_set_is_lastclone(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
                matchspec0 = distfarreachleaf_lastclone_lastscansplit_tbl_match_spec_t(\
                        op_hdr_optype = SCANREQ_SPLIT,
                        clone_hdr_clonenum_for_pktloss = 0,
                        meta_remain_scannum = 1)
                self.client.lastclone_lastscansplit_tbl_table_add_with_set_is_lastscansplit(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 10

            # Table: eg_port_forward_tbl (default: nop; size: < 2048)
            print "Configuring eg_port_forward_tbl"
            if RANGE_SUPPORT == False:
                self.configure_eg_port_forward_tbl()
            else:
                self.configure_eg_port_forward_tbl_with_range()

            # Table: update_pktlen_tbl (default: nop; 195)
            print "Configuring update_pktlen_tbl"
            for i in range(switch_max_vallen/8 + 1): # i from 0 to 16
                if i == 0:
                    vallen_start = 0
                    vallen_end = 0
                    aligned_vallen = 0
                else:
                    vallen_start = (i-1)*8+1 # 1, 9, ..., 121
                    vallen_end = (i-1)*8+8 # 8, 16, ..., 128
                    aligned_vallen = vallen_end # 8, 16, ..., 128
                val_stat_udplen = aligned_vallen + 36
                val_stat_iplen = aligned_vallen + 56
                val_seq_inswitch_stat_clone_udplen = aligned_vallen + 70
                val_seq_inswitch_stat_clone_iplen = aligned_vallen + 90
                val_seq_udplen = aligned_vallen + 36
                val_seq_iplen = aligned_vallen + 56
                val_seq_stat_udplen = aligned_vallen + 40
                val_seq_stat_iplen = aligned_vallen + 60
                val_seq_inswitch_stat_udplen = aligned_vallen + 56
                val_seq_inswitch_stat_iplen = aligned_vallen + 76
                matchspec0 = distfarreachleaf_update_pktlen_tbl_match_spec_t(\
                        op_hdr_optype=GETRES,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end) # [vallen_start, vallen_end]
                actnspec0 = distfarreachleaf_update_pktlen_action_spec_t(val_stat_udplen, val_stat_iplen)
                self.client.update_pktlen_tbl_table_add_with_update_pktlen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1]:
                    matchspec0 = distfarreachleaf_update_pktlen_tbl_match_spec_t(\
                            op_hdr_optype=tmpoptype,
                            vallen_hdr_vallen_start=vallen_start,
                            vallen_hdr_vallen_end=vallen_end) # [vallen_start, vallen_end]
                    actnspec0 = distfarreachleaf_update_pktlen_action_spec_t(val_seq_inswitch_stat_clone_udplen, val_seq_inswitch_stat_clone_iplen)
                    self.client.update_pktlen_tbl_table_add_with_update_pktlen(\
                            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                for tmpoptype in [PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3]:
                    matchspec0 = distfarreachleaf_update_pktlen_tbl_match_spec_t(\
                            op_hdr_optype=tmpoptype,
                            vallen_hdr_vallen_start=vallen_start,
                            vallen_hdr_vallen_end=vallen_end) # [vallen_start, vallen_end]
                    actnspec0 = distfarreachleaf_update_pktlen_action_spec_t(val_seq_udplen, val_seq_iplen)
                    self.client.update_pktlen_tbl_table_add_with_update_pktlen(\
                            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                matchspec0 = distfarreachleaf_update_pktlen_tbl_match_spec_t(\
                        op_hdr_optype=CACHE_EVICT_LOADDATA_INSWITCH_ACK,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end) # [vallen_start, vallen_end]
                actnspec0 = distfarreachleaf_update_pktlen_action_spec_t(val_seq_stat_udplen, val_seq_stat_iplen)
                self.client.update_pktlen_tbl_table_add_with_update_pktlen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                matchspec0 = distfarreachleaf_update_pktlen_tbl_match_spec_t(\
                        op_hdr_optype=LOADSNAPSHOTDATA_INSWITCH_ACK,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end) # [vallen_start, vallen_end]
                actnspec0 = distfarreachleaf_update_pktlen_action_spec_t(val_seq_inswitch_stat_udplen, val_seq_inswitch_stat_iplen)
                self.client.update_pktlen_tbl_table_add_with_update_pktlen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
            onlyop_udplen = 28
            onlyop_iplen = 48
            stat_udplen = 34
            stat_iplen = 54
            seq_udplen = 34
            seq_iplen = 54
            scanreqsplit_udplen = 56
            scanreqsplit_iplen = 76
            frequency_udplen = 32
            frequency_iplen = 52
            matchspec0 = distfarreachleaf_update_pktlen_tbl_match_spec_t(\
                    op_hdr_optype=CACHE_POP_INSWITCH_ACK,
                    vallen_hdr_vallen_start=0,
                    vallen_hdr_vallen_end=switch_max_vallen) # [0, 128]
            actnspec0 = distfarreachleaf_update_pktlen_action_spec_t(onlyop_udplen, onlyop_iplen)
            self.client.update_pktlen_tbl_table_add_with_update_pktlen(\
                    self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
            for tmpoptype in [PUTRES, DELRES]:
                matchspec0 = distfarreachleaf_update_pktlen_tbl_match_spec_t(\
                        op_hdr_optype=tmpoptype,
                        vallen_hdr_vallen_start=0,
                        vallen_hdr_vallen_end=switch_max_vallen) # [0, 128]
                actnspec0 = distfarreachleaf_update_pktlen_action_spec_t(stat_udplen, stat_iplen)
                self.client.update_pktlen_tbl_table_add_with_update_pktlen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
            for tmpoptype in [DELREQ_SEQ, DELREQ_SEQ_CASE3]:
                matchspec0 = distfarreachleaf_update_pktlen_tbl_match_spec_t(\
                        op_hdr_optype=tmpoptype,
                        vallen_hdr_vallen_start=0,
                        vallen_hdr_vallen_end=switch_max_vallen) # [0, 128]
                actnspec0 = distfarreachleaf_update_pktlen_action_spec_t(seq_udplen, seq_iplen)
                self.client.update_pktlen_tbl_table_add_with_update_pktlen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
            matchspec0 = distfarreachleaf_update_pktlen_tbl_match_spec_t(\
                    op_hdr_optype=SCANREQ_SPLIT,
                    vallen_hdr_vallen_start=0,
                    vallen_hdr_vallen_end=switch_max_vallen) # [0, 128]
            actnspec0 = distfarreachleaf_update_pktlen_action_spec_t(scanreqsplit_udplen, scanreqsplit_iplen)
            self.client.update_pktlen_tbl_table_add_with_update_pktlen(\
                    self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
            matchspec0 = distfarreachleaf_update_pktlen_tbl_match_spec_t(\
                    op_hdr_optype=CACHE_EVICT_LOADFREQ_INSWITCH_ACK,
                    vallen_hdr_vallen_start=0,
                    vallen_hdr_vallen_end=switch_max_vallen) # [0, 128]
            actnspec0 = distfarreachleaf_update_pktlen_action_spec_t(frequency_udplen, frequency_iplen)
            self.client.update_pktlen_tbl_table_add_with_update_pktlen(\
                    self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
            matchspec0 = distfarreachleaf_update_pktlen_tbl_match_spec_t(\
                    op_hdr_optype=SETVALID_INSWITCH_ACK,
                    vallen_hdr_vallen_start=0,
                    vallen_hdr_vallen_end=switch_max_vallen) # [0, 128]
            actnspec0 = distfarreachleaf_update_pktlen_action_spec_t(onlyop_udplen, onlyop_iplen)
            self.client.update_pktlen_tbl_table_add_with_update_pktlen(\
                    self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)

            # Table: update_ipmac_srcport_tbl (default: nop; 6*client_physical_num+12*server_physical_num+7=43 < 18*8+7=151 < 256)
            # NOTE: udp.dstport is updated by eg_port_forward_tbl (only required by switch2switchos)
            # NOTE: update_ipmac_srcport_tbl focues on src/dst ip/mac and udp.srcport
            print "Configuring update_ipmac_srcport_tbl"
            # (1) for response from server to client, egress port has been set based on ip.dstaddr (or by clone_i2e) in ingress pipeline
            # (2) for response from switch to client, egress port has been set by clone_e2e in egress pipeline
            for tmp_client_physical_idx in range(client_physical_num):
                tmp_devport = self.client_devports[tmp_client_physical_idx]
                tmp_server_mac = server_macs[0]
                tmp_server_ip = server_ips[0]
                actnspec0 = distfarreachleaf_update_srcipmac_srcport_server2client_action_spec_t(\
                        macAddr_to_string(tmp_server_mac), \
                        ipv4Addr_to_i32(tmp_server_ip), \
                        server_worker_port_start)
                for tmpoptype in [GETRES, PUTRES, DELRES, SCANRES_SPLIT, WARMUPACK, LOADACK]:
                    matchspec0 = distfarreachleaf_update_ipmac_srcport_tbl_match_spec_t(\
                            op_hdr_optype = convert_u16_to_i16(tmpoptype), 
                            eg_intr_md_egress_port = tmp_devport)
                    self.client.update_ipmac_srcport_tbl_table_add_with_update_srcipmac_srcport_server2client(\
                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            # for request from client to server, egress port has been set by partition_tbl in ingress pipeline
            # NOTE: for each pkt from client to spine, as its eport must be spineswitch.devport instead of server.devport, it will not invoke update_dstipmac_client2server() here
            for tmp_server_physical_idx in range(server_physical_num):
                tmp_devport = self.server_devports[tmp_server_physical_idx]
                tmp_server_mac = server_macs[tmp_server_physical_idx]
                tmp_server_ip = server_ips[tmp_server_physical_idx]
                actnspec1 = distfarreachleaf_update_dstipmac_client2server_action_spec_t(\
                        macAddr_to_string(tmp_server_mac), \
                        ipv4Addr_to_i32(tmp_server_ip))
                for tmpoptype in [GETREQ, GETREQ_NLATEST, PUTREQ_SEQ, DELREQ_SEQ, SCANREQ_SPLIT, GETREQ_POP, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3, DELREQ_SEQ_CASE3, WARMUPREQ, LOADREQ]:
                    matchspec0 = distfarreachleaf_update_ipmac_srcport_tbl_match_spec_t(\
                            op_hdr_optype = convert_u16_to_i16(tmpoptype), 
                            eg_intr_md_egress_port = tmp_devport)
                    self.client.update_ipmac_srcport_tbl_table_add_with_update_dstipmac_client2server(\
                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec1)
            # Here we use server_mac/ip to simulate reflector_mac/ip = switchos_mac/ip
            # (1) eg_intr_md.egress_port of the first GETRES_CASE1 is set by ipv4_forward_tbl (as ingress port), which will be finally dropped -> update ip/mac/srcport or not is not important
            # (2) eg_intr_md.egress_port of cloned GETRES_CASE1s is set by clone_e2e, which must be the devport towards switchos (aka reflector)
            # (3) eg_intr_md.egress_port of the first ACK for cache population/eviction is set by partition_tbl in ingress pipeline, which will be finally dropped -> update ip/mac/srcport or not is not important
            # (4) eg_intr_md.egress_port of the cloned ACK for cache population/eviction is set by clone_e2e, which must be the devport towards switchos (aka reflector)
            tmp_devport = self.reflector_devport
            #tmp_client_ip = client_ips[0]
            #tmp_client_mac = client_macs[0]
            tmp_client_ip = reflector_cp2dp_dstip
            tmp_client_mac = reflector_cp2dp_dstmac
            tmp_client_port = 123 # not cared by servers
            actnspec2 = distfarreachleaf_update_ipmac_srcport_switch2switchos_action_spec_t(\
                    macAddr_to_string(tmp_client_mac), \
                    macAddr_to_string(self.reflector_mac_for_switch), \
                    ipv4Addr_to_i32(tmp_client_ip), \
                    ipv4Addr_to_i32(self.reflector_ip_for_switch), \
                    tmp_client_port)
            for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1, CACHE_POP_INSWITCH_ACK, CACHE_EVICT_LOADFREQ_INSWITCH_ACK, CACHE_EVICT_LOADDATA_INSWITCH_ACK, LOADSNAPSHOTDATA_INSWITCH_ACK, SETVALID_INSWITCH_ACK]:
                matchspec0 = distfarreachleaf_update_ipmac_srcport_tbl_match_spec_t(\
                        op_hdr_optype=convert_u16_to_i16(tmpoptype), 
                        eg_intr_md_egress_port=tmp_devport)
                self.client.update_ipmac_srcport_tbl_table_add_with_update_ipmac_srcport_switch2switchos(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec2)

            # Table: add_and_remove_value_header_tbl (default: remove_all; 17*17=289)
            print "Configuring add_and_remove_value_header_tbl"
            # NOTE: egress pipeline must not output PUTREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, CACHE_POP_INSWITCH, and PUTREQ_SEQ_INSWITCH
            # NOTE: even for future PUTREQ_LARGE/GETRES_LARGE, as their values should be in payload, we should invoke add_only_vallen() for vallen in [0, global_max_vallen]
            for tmpoptype in [PUTREQ, PUTREQ_SEQ, PUTREQ_POP_SEQ, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ_CASE3, GETRES_LATEST_SEQ_INSWITCH_CASE1, GETRES_DELETED_SEQ_INSWITCH_CASE1, PUTREQ_SEQ_INSWITCH_CASE1, DELREQ_SEQ_INSWITCH_CASE1, GETRES, LOADREQ, CACHE_EVICT_LOADDATA_INSWITCH_ACK, LOADSNAPSHOTDATA_INSWITCH_ACK, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ_SERVER]:
                for i in range(switch_max_vallen/8 + 1): # i from 0 to 16
                    if i == 0:
                        vallen_start = 0
                        vallen_end = 0
                    else:
                        vallen_start = (i-1)*8+1 # 1, 9, ..., 121
                        vallen_end = (i-1)*8+8 # 8, 16, ..., 128
                    matchspec0 = distfarreachleaf_add_and_remove_value_header_tbl_match_spec_t(\
                            op_hdr_optype=tmpoptype,
                            vallen_hdr_vallen_start=vallen_start,
                            vallen_hdr_vallen_end=vallen_end) # [vallen_start, vallen_end]
                    if i == 0:
                        self.client.add_and_remove_value_header_tbl_table_add_with_add_only_vallen(\
                                self.sess_hdl, self.dev_tgt, matchspec0, 0)
                    else:
                        eval("self.client.add_and_remove_value_header_tbl_table_add_with_add_to_val{}".format(i))(\
                                self.sess_hdl, self.dev_tgt, matchspec0, 0)

            # Table: drop_tbl (default: nop; size: 2)
            print "Configuring drop_tbl"
            matchspec0 = distfarreachleaf_drop_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_LATEST_SEQ_INSWITCH)
            self.client.drop_tbl_table_add_with_drop_getres_latest_seq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = distfarreachleaf_drop_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_DELETED_SEQ_INSWITCH)
            self.client.drop_tbl_table_add_with_drop_getres_deleted_seq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            

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

    def configure_eg_port_forward_tbl(self):
        # Table: eg_port_forward_tbl (default: nop; size: 131+340*spine_physical_num=471 < 2048)
        tmp_client_sids = [0, self.spineswitch_sid]
        for is_cached in cached_list:
            for is_hot in hot_list:
                for validvalue in validvalue_list:
                    for is_latest in latest_list:
                        for is_deleted in deleted_list:
                            # Use tmpstat as action data to reduce action number
                            if is_deleted == 1:
                                tmpstat = 0
                            else:
                                tmpstat = 1
                            # NOTE: eg_intr_md.egress_port is read-only
                            #for is_wrong_pipeline in pipeline_list:
                            #for tmp_client_sid in self.sids:
                            for tmp_client_sid in tmp_client_sids:
                                for is_lastclone_for_pktloss in lastclone_list:
                                    for snapshot_flag in snapshot_flag_list:
                                        for is_case1 in case1_list:
                                            # is_lastclone_for_pktloss, snapshot_flag, and is_case1 should be 0 for GETREQ_INSWITCH
                                            if is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and tmp_client_sid != 0:
                                                # size: 48*spine_physical_num=48
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = GETREQ_INSWITCH,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                if is_cached == 0:
                                                    if is_hot == 1:
                                                        # Update GETREQ_INSWITCH as GETREQ_POP to server
                                                        #actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getreq_pop_action_spec_t(self.devPorts[1])
                                                        self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_pop(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                    else:
                                                        # Update GETREQ_INSWITCH as GETREQ to server
                                                        #actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getreq_action_spec_t(self.devPorts[1])
                                                        self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                else:
                                                    if validvalue == 0:
                                                        # Update GETREQ_INSWITCH as GETREQ to server
                                                        #actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getreq_action_spec_t(self.devPorts[1])
                                                        self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                    elif validvalue == 1:
                                                        if is_latest == 0:
                                                            # Update GETREQ_INSWITCH as GETREQ_NLATEST to server
                                                            #actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getreq_nlatest_action_spec_t(self.devPorts[1])
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_nlatest(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        else:
                                                            # Update GETREQ_INSWITCH as GETRES to client by mirroring
                                                            actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t(tmp_client_sid, server_worker_port_start, tmpstat)
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    elif validvalue == 3:
                                                        if is_latest == 0:
                                                            # Update GETREQ_INSWITCH as GETREQ to server
                                                            #actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getreq_action_spec_t(self.devPorts[1])
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        else:
                                                            # Update GETREQ_INSWITCH as GETRES to client by mirroring
                                                            actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t(tmp_client_sid, server_worker_port_start, tmpstat)
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            # is_cached (no inswitch_hdr due to no field list when clone_i2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline (no inswitch_hdr), tmp_client_sid=0 (no inswitch hdr), is_lastclone_for_pktloss, snapshot_flag (no inswitch_hdr), is_case1 should be 0 for GETRES_LATEST_SEQ_SERVER
                                            # NOTE: we use sid == self.sids[0] to avoid duplicate entry; we use inswitch_hdr_client_sid = 0 to match the default value of inswitch_hdr.client_sid
                                            # size: 1
                                            #if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                            if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = GETRES_LATEST_SEQ_SERVER,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Update GETRES_LATEST_SEQ_SERVER (by clone_i2e) as GETRES to client
                                                self.client.eg_port_forward_tbl_table_add_with_update_getres_latest_seq_server_to_getres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                            # size: 4
                                            # NOTE: for GETRES_LATEST_SEQ_SERVER_INSWITCH, inswitch_hdr is set by spine switch
                                            # tmp_client_sid = 0 (spine NOT set client_sid in prepare_for_cachehit_tbl for GETRES_LATEST_SEQ_SERVER); yet spine switch could set is_cached and snapshot_flag for GETRES_LATEST_SEQ_SERVER, which is changed as GETRES_LATEST_SEQ_SERVER_INSWITCH to leaf switch
                                            if is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = GETRES_LATEST_SEQ_SERVER_INSWITCH,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Update GETRES_LATEST_SEQ_SERVER_INSWITCH (by clone_i2e) as GETRES to client
                                                self.client.eg_port_forward_tbl_table_add_with_update_getres_latest_seq_server_inswitch_to_getres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                            # is_hot (cm_predicate=1), is_wrong_pipeline (not need range/hash partition), tmp_client_sid=0 (not need mirroring for res), is_lastclone_for_pktloss should be 0 for GETRES_LATEST_SEQ_INSWITCH
                                            # size: 48 + 2 = 50
                                            #if is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                            if is_hot == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = GETRES_LATEST_SEQ_INSWITCH,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                if is_cached == 1 and validvalue == 1 and is_latest == 0 and snapshot_flag == 1 and is_case1 == 0:
                                                    # Update GETRES_LATEST_SEQ_INSWITCH as GETRES_LATEST_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                    if is_deleted == 0: # is_deleted=0 -> stat=1
                                                        #actnspec0 = distfarreachleaf_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, self.devPorts[1], 1)
                                                        actnspec0 = distfarreachleaf_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, 1, reflector_dp2cpserver_port)
                                                    elif is_deleted == 1: # is_deleted=1 -> stat=0
                                                        #actnspec0 = distfarreachleaf_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, self.devPorts[1], 0)
                                                        actnspec0 = distfarreachleaf_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, 0, reflector_dp2cpserver_port)
                                                    self.client.eg_port_forward_tbl_table_add_with_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                elif is_cached == 0:
                                                    # Update GETRES_LATEST_SEQ_INSWITCH as GETRES_LATEST_SEQ to spine (NOT dropped by drop_tbl)
                                                    self.client.eg_port_forward_tbl_table_add_with_update_getres_latest_seq_inswitch_to_getres_latest_seq(self.sess_hdl, self.dev_tgt, matchspec0)
                                                # Keep GETERS_LATEST_SEQ_INSWITCH unchanged, and resort to drop_tbl to drop it
                                                #else:
                                                #    # Drop GETRES_LATEST_SEQ_INSWITCH
                                                #    self.client.eg_port_forward_tbl_table_add_with_drop_getres_latest_seq_inswitch(\
                                                #            self.sess_hdl, self.dev_tgt, matchspec0)
                                            # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_wrong_pipeline=0, tmp_client_sid=0, and snapshot_flag=1 (same inswitch_hdr as GETRES_LATEST_SEQ_INSWITCH); is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_case1 should be 0 for GETRES_LATEST_SEQ_INSWITCH_CASE1
                                            # size: 1
                                            #if is_cached == 1 and is_wrong_pipeline == 0 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_case1 == 0:
                                            if is_cached == 1 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = GETRES_LATEST_SEQ_INSWITCH_CASE1,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                if is_lastclone_for_pktloss == 0:
                                                    # Forward GETRES_LATEST_SEQ_INSWITCH_CASE0 (by clone_e2e) to reflector (w/ clone)
                                                    actnspec0 = distfarreachleaf_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid)
                                                    self.client.eg_port_forward_tbl_table_add_with_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                elif is_lastclone_for_pktloss == 1:
                                                    # Forward GETRES_LATEST_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector
                                                    #self.client.eg_port_forward_tbl_table_add_with_forward_getres_latest_seq_inswitch_case1(\
                                                    #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                    # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                    pass
                                            # is_cached (no inswitch_hdr due to no field list when clone_i2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline (no inswitch_hdr), tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktloss, snapshot_flag (no inswitch_hdr), is_case1 should be 0 for GETRES_DELETED_SEQ_SERVER
                                            # size: 1
                                            #if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                            if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = GETRES_DELETED_SEQ_SERVER,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Update GETRES_DELETED_SEQ_SERVER (by clone_i2e) as GETRES to client
                                                self.client.eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_server_to_getres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                            # size: 4
                                            # NOTE: for GETRES_DELETED_SEQ_SERVER_INSWITCH, inswitch_hdr is set by spine switch
                                            # tmp_client_sid = 0 (spine NOT set client_sid in prepare_for_cachehit_tbl for GETRES_DELETED_SEQ_SERVER); yet spine switch could set is_cached and snapshot_flag for GETRES_DELETED_SEQ_SERVER, which is changed as GETRES_DELETED_SEQ_SERVER_INSWITCH to leaf switch
                                            if is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = GETRES_DELETED_SEQ_SERVER_INSWITCH,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Update GETRES_DELETED_SEQ_SERVER_INSWITCH (by clone_i2e) as GETRES to client
                                                self.client.eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_server_inswitch_to_getres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                            # is_hot (cm_predicate=1), is_wrong_pipeline (not need range/hash partition), tmp_client_sid=0 (not need mirroring for res), is_lastclone_for_pktloss should be 0 for GETRES_DELETED_SEQ_INSWITCH
                                            # size: 48 + 2 = 50
                                            #if is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                            if is_hot == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = GETRES_DELETED_SEQ_INSWITCH,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                if is_cached == 1 and validvalue == 1 and is_latest == 0 and snapshot_flag == 1 and is_case1 == 0:
                                                    # Update GETRES_DELETED_SEQ_INSWITCH as GETRES_DELETED_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                    if is_deleted == 0: # is_deleted=0 -> stat=1
                                                        #actnspec0 = distfarreachleaf_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, self.devPorts[1], 1)
                                                        actnspec0 = distfarreachleaf_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, 1, reflector_dp2cpserver_port)
                                                    elif is_deleted == 1: # is_deleted=1 -> stat=0
                                                        #actnspec0 = distfarreachleaf_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, self.devPorts[1], 0)
                                                        actnspec0 = distfarreachleaf_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, 0, reflector_dp2cpserver_port)
                                                    self.client.eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                elif is_cached == 0:
                                                    # Update GETRES_DELETED_SEQ_INSWITCH as GETRES_DELETED_SEQ to spine (NOT dropped by drop_tbl)
                                                    self.client.eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq(self.sess_hdl, self.dev_tgt, matchspec0)
                                                # Keep GETERS_DELETED_SEQ_INSWITCH unchanged, and resort to drop_tbl to drop it
                                                #else:
                                                #    # Drop GETRES_DELETED_SEQ_INSWITCH
                                                #    self.client.eg_port_forward_tbl_table_add_with_drop_getres_deleted_seq_inswitch(\
                                                #            self.sess_hdl, self.dev_tgt, matchspec0)
                                            # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_wrong_pipeline=0, tmp_client_sid=0, and snapshot_flag=1 (same inswitch_hdr as GETRES_LATEST_SEQ_INSWITCH); is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_case1 should be 0 for GETRES_DELETED_SEQ_INSWITCH_CASE1
                                            # size: 1
                                            #if is_cached == 1 and is_wrong_pipeline == 0 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_case1 == 0:
                                            if is_cached == 1 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = GETRES_DELETED_SEQ_INSWITCH_CASE1,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                if is_lastclone_for_pktloss == 0:
                                                    # Forward GETRES_DELETED_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                    actnspec0 = distfarreachleaf_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid)
                                                    self.client.eg_port_forward_tbl_table_add_with_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                elif is_lastclone_for_pktloss == 1:
                                                    # Forward GETRES_DELETED_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector
                                                    #self.client.eg_port_forward_tbl_table_add_with_forward_getres_deleted_seq_inswitch_case1(\
                                                    #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                    # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                    pass
                                            # is_cached=0 (memset inswitch_hdr by end-host, and key must not be cached in cache_lookup_tbl for CACHE_POP_INSWITCH), is_hot (cm_predicate=1), validvalue, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_POP_INSWITCH
                                            # size: 4
                                            #if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                            if is_cached == 0 and is_hot == 0 and validvalue == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = CACHE_POP_INSWITCH,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Update CACHE_POP_INSWITCH as CACHE_POP_INSWITCH_ACK to reflector (deprecated: w/ clone)
                                                actnspec0 = distfarreachleaf_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec_t(self.reflector_sid, reflector_dp2cpserver_port)
                                                self.client.eg_port_forward_tbl_table_add_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), snapshot_flag, is_case1, is_lastclone_for_pktloss should be 0 for CACHE_POP_INSWITCH_ACK
                                            # size: 0
                                            #if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and snapshot_flag == 0 and is_case1 == 0:
                                            if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and snapshot_flag == 0 and is_case1 == 0 and is_lastclone_for_pktloss == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = CACHE_POP_INSWITCH_ACK,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                #if is_lastclone_for_pktloss == 0:
                                                #    # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector (w/ clone)
                                                #    actnspec0 = distfarreachleaf_forward_cache_pop_inswitch_ack_clone_for_pktloss_action_spec_t(self.reflector_sid)
                                                #    self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack_clone_for_pktloss(\
                                                #            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                #elif is_lastclone_for_pktloss == 1:
                                                #    # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                                #    self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack(\
                                                #            self.sess_hdl, self.dev_tgt, matchspec0)

                                                # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                                #self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack(\
                                                #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                pass
                                            # is_lastclone_for_pktloss should be 0 for PUTREQ_INSWITCH
                                            # size: 192*spine_physical_num=192
                                            if is_lastclone_for_pktloss == 0 and tmp_client_sid != 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = PUTREQ_SEQ_INSWITCH,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                if is_cached == 0:
                                                    if snapshot_flag == 1:
                                                        if is_hot == 1:
                                                            # Update PUTREQ_SEQ_INSWITCH as PUTREQ_POP_SEQ_CASE3 to server
                                                            self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_pop_seq_case3(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        elif is_hot == 0:
                                                            # Update PUTREQ_SEQ_INSWITCH as PUTREQ_SEQ_CASE3 to server
                                                            self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_seq_case3(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                    elif snapshot_flag == 0:
                                                        if is_hot == 1:
                                                            # Update PUTREQ_SEQ_INSWITCH as PUTREQ_POP_SEQ to server
                                                            self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_pop_seq(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        elif is_hot == 0:
                                                            # Update PUTREQ_SEQ_INSWITCH as PUTREQ_SEQ to server
                                                            self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_seq(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                elif is_cached == 1:
                                                    if validvalue == 0 or validvalue == 3:
                                                        if snapshot_flag == 1:
                                                            # Update PUTREQ_SEQ_INSWITCH as PUTREQ_SEQ_CASE3 to server
                                                            self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_seq_case3(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        elif snapshot_flag == 0:
                                                            # Update PUTREQ_SEQ_INSWITCH as PUTREQ_SEQ to server
                                                            self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_seq(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                    elif validvalue == 1:
                                                        if snapshot_flag == 1 and is_case1 == 0:
                                                            # Update PUTREQ_SEQ_INSWITCH as PUTREQ_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                            if is_deleted == 0: # is_deleted=0 -> stat=1
                                                                #actnspec0 = distfarreachleaf_update_putreq_seq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.reflector_sid, self.devPorts[1], 1)
                                                                actnspec0 = distfarreachleaf_update_putreq_seq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.reflector_sid, 1, reflector_dp2cpserver_port)
                                                            elif is_deleted == 1: # is_deleted=1 -> stat=0
                                                                #actnspec0 = distfarreachleaf_update_putreq_seq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.reflector_sid, self.devPorts[1], 0)
                                                                actnspec0 = distfarreachleaf_update_putreq_seq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.reflector_sid, 0, reflector_dp2cpserver_port)
                                                            self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                        else:
                                                            # Update PUTREQ_SEQ_INSWITCH as PUTRES to client by mirroring
                                                            actnspec0 = distfarreachleaf_update_putreq_seq_inswitch_to_putres_by_mirroring_action_spec_t(tmp_client_sid, server_worker_port_start)
                                                            self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putres_by_mirroring(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, snapshot_flag=1, is_case1 should be 0 for PUTREQ_SEQ_INSWITCH_CASE1
                                            # size: 2*spine_physical_num=2
                                            # TODO: check tmp_client_sid != 0
                                            if is_cached == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and snapshot_flag == 1 and is_case1 == 0 and tmp_client_sid != 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = PUTREQ_SEQ_INSWITCH_CASE1,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                if is_lastclone_for_pktloss == 0:
                                                    # Forward PUTREQ_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                    actnspec0 = distfarreachleaf_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.reflector_sid)
                                                    self.client.eg_port_forward_tbl_table_add_with_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                elif is_lastclone_for_pktloss == 1:
                                                    # Update PUTREQ_SEQ_INSWITCH_CASE1 as PUTRES to client by mirroring
                                                    actnspec0 = distfarreachleaf_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec_t(tmp_client_sid, server_worker_port_start)
                                                    self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            # is_hot (cm_predicate=1), is_lastclone_for_pktloss should be 0 for DELREQ_INSWITCH
                                            # size: 96*spine_physical_num=96
                                            if is_hot == 0 and is_lastclone_for_pktloss == 0 and tmp_client_sid != 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = DELREQ_SEQ_INSWITCH,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                if is_cached == 0:
                                                    if snapshot_flag == 1:
                                                        # Update DELREQ_SEQ_INSWITCH as DELREQ_SEQ_CASE3 to server
                                                        self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_to_delreq_seq_case3(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                    elif snapshot_flag == 0:
                                                        # Update DELREQ_SEQ_INSWITCH as DELREQ_SEQ to server
                                                        self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_to_delreq_seq(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                elif is_cached == 1:
                                                    if validvalue == 0 or validvalue == 3:
                                                        if snapshot_flag == 1:
                                                            # Update DELREQ_SEQ_INSWITCH as DELREQ_SEQ_CASE3 to server
                                                            self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_to_delreq_seq_case3(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        elif snapshot_flag == 0:
                                                            # Update DELREQ_SEQ_INSWITCH as DELREQ_SEQ to server
                                                            self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_to_delreq_seq(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                    elif validvalue == 1:
                                                        if snapshot_flag == 1 and is_case1 == 0:
                                                            # Update DELREQ_SEQ_INSWITCH as DELREQ_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                            if is_deleted == 0: # is_deleted=0 -> stat=1
                                                                #actnspec0 = distfarreachleaf_update_delreq_seq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.reflector_sid, self.devPorts[1], 1)
                                                                actnspec0 = distfarreachleaf_update_delreq_seq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.reflector_sid, 1, reflector_dp2cpserver_port)
                                                            elif is_deleted == 1: # is_deleted=1 -> stat=0
                                                                #actnspec0 = distfarreachleaf_update_delreq_seq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.reflector_sid, self.devPorts[1], 0)
                                                                actnspec0 = distfarreachleaf_update_delreq_seq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.reflector_sid, 0, reflector_dp2cpserver_port)
                                                            self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                        else:
                                                            # Update DELREQ_SEQ_INSWITCH as DELRES to client by mirroring
                                                            actnspec0 = distfarreachleaf_update_delreq_seq_inswitch_to_delres_by_mirroring_action_spec_t(tmp_client_sid, server_worker_port_start)
                                                            self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_to_delres_by_mirroring(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, snapshot_flag=1, is_case1 should be 0 for DELREQ_SEQ_INSWITCH_CASE1
                                            # size: 2*spine_physical_num=2
                                            # TODO: check tmp_client_sid != 0
                                            if is_cached == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and snapshot_flag == 1 and is_case1 == 0 and tmp_client_sid != 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = DELREQ_SEQ_INSWITCH_CASE1,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                if is_lastclone_for_pktloss == 0:
                                                    # Forward DELREQ_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                    actnspec0 = distfarreachleaf_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.reflector_sid)
                                                    self.client.eg_port_forward_tbl_table_add_with_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                elif is_lastclone_for_pktloss == 1:
                                                    # Update DELREQ_SEQ_INSWITCHCASE1 as DELRES to client by mirroring
                                                    actnspec0 = distfarreachleaf_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec_t(tmp_client_sid, server_worker_port_start)
                                                    self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            # is_cached=1 (key must be cached in cache_lookup_tbl for CACHE_EVICT_LOADFREQ_INSWITCH), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH
                                            # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                            # size: 1
                                            if is_cached == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = CACHE_EVICT_LOADFREQ_INSWITCH,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Update CACHE_EVICT_LOADFREQ_INSWITCH as CACHE_EVICT_LOADFREQ_INSWITCH_ACK to reflector (w/ frequency)
                                                actnspec0 = distfarreachleaf_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec_t(self.reflector_sid, reflector_dp2cpserver_port)
                                                self.client.eg_port_forward_tbl_table_add_with_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH_ACK
                                            # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                            # size: 0
                                            if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = CACHE_EVICT_LOADFREQ_INSWITCH_ACK,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Forward CACHE_EVICT_LOADFREQ_INSWITCH_ACK (by clone_e2e) to reflector
                                                #self.client.eg_port_forward_tbl_table_add_with_forward_cache_evict_loadfreq_inswitch_ack(\
                                                #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                pass
                                            # is_cached=1 (key must be cached in cache_lookup_tbl for CACHE_EVICT_LOADDATA_INSWITCH), is_hot (cm_predicate=1), validvalue, is_latest, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADDATA_INSWITCH
                                            # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                            # size: 2
                                            if is_cached == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = CACHE_EVICT_LOADDATA_INSWITCH,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Update CACHE_EVICT_LOADDATA_INSWITCH as CACHE_EVICT_LOADDATA_INSWITCH_ACK to reflector
                                                actnspec0 = distfarreachleaf_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec_t(self.reflector_sid, reflector_dp2cpserver_port, tmpstat)
                                                self.client.eg_port_forward_tbl_table_add_with_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADDATA_INSWITCH_ACK
                                            # size: 0
                                            if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = CACHE_EVICT_LOADDATA_INSWITCH_ACK,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Forward CACHE_EVICT_LOADDATA_INSWITCH_ACK (by clone_e2e) to reflector
                                                #self.client.eg_port_forward_tbl_table_add_with_forward_cache_evict_loaddata_inswitch_ack(\
                                                #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                pass
                                            # is_hot (cm_predicate=1), validvalue, is_latest, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for LOADSNAPSHOTDATA_INSWITCH
                                            # NOTE: is_cached can be 0 or 1 (key may be / may not be evicted after snapshot timepoint)
                                            # size: 4
                                            if is_hot == 0 and validvalue == 0 and is_latest == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = LOADSNAPSHOTDATA_INSWITCH,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Update LOADSNAPSHOTDATA_INSWITCH as LOADSNAPSHOTDATA_INSWITCH_ACK to reflector
                                                actnspec0 = distfarreachleaf_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec_t(self.reflector_sid, reflector_dp2cpserver_port, tmpstat)
                                                self.client.eg_port_forward_tbl_table_add_with_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            # is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for LOADSNAPSHOTDATA_INSWITCH_ACK
                                            # NOTE: is_cached can be 0 or 1 (inswitch_hdr inherited from LOADSNAPSHOTDATA_INSWITCH)
                                            # size: 0
                                            if is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = LOADSNAPSHOTDATA_INSWITCH_ACK,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Forward LOADSNAPSHOTDATA_INSWITCH_ACK (by clone_e2e) to reflector
                                                #self.client.eg_port_forward_tbl_table_add_with_forward_loadsnapshotdata_inswitch_ack(\
                                                #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                pass
                                            # is_hot (cm_predicate=1), is_latest, is_deleted, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for SETVALID_INSWITCH
                                            # NOTE: is_cached can be 0 or 1 (key may be / may not be cached for SETVALID_INSWITCH)
                                            # NOTE: validvalue can be 0/1/3 for SETVALID_INSWITCH
                                            # size: 8
                                            if is_hot == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = SETVALID_INSWITCH,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Update SETVALID_INSWITCH as SETVALID_INSWITCH_ACK to reflector
                                                actnspec0 = distfarreachleaf_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec_t(self.reflector_sid, reflector_dp2cpserver_port)
                                                self.client.eg_port_forward_tbl_table_add_with_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            # is_cached=0 (no inswtich_hdr), is_hot (cm_predicate=1), validvalue (no validvalue_hdr), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for SETVALID_INSWITCH_ACK
                                            # NOTE: is_cached must be 0 (SETVALID_INSWITCH_ACK does not have inswitch_hdr)
                                            # NOTE: validvalue must be 0 (no validvalue_hdr and not touch validvalue_reg)
                                            # size: 0
                                            if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = SETVALID_INSWITCH_ACK,
                                                    inswitch_hdr_is_cached = is_cached,
                                                    meta_is_hot = is_hot,
                                                    validvalue_hdr_validvalue = validvalue,
                                                    meta_is_latest = is_latest,
                                                    meta_is_deleted = is_deleted,
                                                    #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                    inswitch_hdr_client_sid = tmp_client_sid,
                                                    meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                    inswitch_hdr_snapshot_flag = snapshot_flag,
                                                    meta_is_case1 = is_case1)
                                                # Forward SETVALID_INSWITCH_ACK (by clone_e2e) to reflector
                                                #self.client.eg_port_forward_tbl_table_add_with_forward_setvalid_inswitch_ack(\
                                                #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                pass

    def configure_eg_port_forward_tbl_with_range(self):
        # Table: eg_port_forward_tbl (default: nop; size: 131+340*spine_physical_num+2*server_physical_num=475 < 2048)
        tmp_client_sids = [0, self.spineswitch_sid]
        tmp_server_sids = [0] + self.server_sids
        for is_cached in cached_list:
            for is_hot in hot_list:
                for validvalue in validvalue_list:
                    for is_latest in latest_list:
                        for is_deleted in deleted_list:
                            # Use tmpstat as action data to reduce action number
                            if is_deleted == 1:
                                tmpstat = 0
                            else:
                                tmpstat = 1
                            # NOTE: eg_intr_md.egress_port is read-only
                            #for is_wrong_pipeline in pipeline_list:
                            #for tmp_client_sid in self.sids:
                            for tmp_client_sid in tmp_client_sids:
                                for is_lastclone_for_pktloss in lastclone_list:
                                    for snapshot_flag in snapshot_flag_list:
                                        for is_case1 in case1_list:
                                            for is_last_scansplit in [0, 1]:
                                                for tmp_server_sid in tmp_server_sids:
                                                    # is_lastclone_for_pktloss, snapshot_flag, and is_case1 should be 0 for GETREQ_INSWITCH
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    if is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0 and tmp_client_sid != 0:
                                                        # size: 48*spine_physical_num=48
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = GETREQ_INSWITCH,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        if is_cached == 0:
                                                            if is_hot == 1:
                                                                # Update GETREQ_INSWITCH as GETREQ_POP to server
                                                                #actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getreq_pop_action_spec_t(self.devPorts[1])
                                                                self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_pop(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                            else:
                                                                # Update GETREQ_INSWITCH as GETREQ to server
                                                                #actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getreq_action_spec_t(self.devPorts[1])
                                                                self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                        else:
                                                            if validvalue == 0:
                                                                # Update GETREQ_INSWITCH as GETREQ to server
                                                                #actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getreq_action_spec_t(self.devPorts[1])
                                                                self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                            elif validvalue == 1:
                                                                if is_latest == 0:
                                                                    # Update GETREQ_INSWITCH as GETREQ_NLATEST to server
                                                                    #actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getreq_nlatest_action_spec_t(self.devPorts[1])
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_nlatest(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                                else:
                                                                    # Update GETREQ_INSWITCH as GETRES to client by mirroring
                                                                    actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t(tmp_client_sid, server_worker_port_start, tmpstat)
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                            elif validvalue == 3:
                                                                if is_latest == 0:
                                                                    # Update GETREQ_INSWITCH as GETREQ to server
                                                                    #actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getreq_action_spec_t(self.devPorts[1])
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                                else:
                                                                    # Update GETREQ_INSWITCH as GETRES to client by mirroring
                                                                    actnspec0 = distfarreachleaf_update_getreq_inswitch_to_getres_by_mirroring_action_spec_t(tmp_client_sid, server_worker_port_start, tmpstat)
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    # is_cached (no inswitch_hdr due to no field list when clone_i2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline (no inswitch_hdr), tmp_client_sid=0 (no inswitch hdr), is_lastclone_for_pktloss, snapshot_flag (no inswitch_hdr), is_case1 should be 0 for GETRES_LATEST_SEQ_SERVER
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # NOTE: we use sid == self.sids[0] to avoid duplicate entry; we use inswitch_hdr_client_sid = 0 to match the default value of inswitch_hdr.client_sid
                                                    # size: 1
                                                    #if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                    if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = GETRES_LATEST_SEQ_SERVER,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # Update GETRES_LATEST_SEQ_SERVER (by clone_i2e) as GETRES to client
                                                        self.client.eg_port_forward_tbl_table_add_with_update_getres_latest_seq_server_to_getres(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                    # size: 4
                                                    # NOTE: for GETRES_LATEST_SEQ_SERVER_INSWITCH, inswitch_hdr is set by spine switch
                                                    # tmp_client_sid = 0 (spine NOT set client_sid in prepare_for_cachehit_tbl for GETRES_LATEST_SEQ_SERVER); yet spine switch could set is_cached and snapshot_flag for GETRES_LATEST_SEQ_SERVER, which is changed as GETRES_LATEST_SEQ_SERVER_INSWITCH to leaf switch
                                                    if is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = GETRES_LATEST_SEQ_SERVER_INSWITCH,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # Update GETRES_LATEST_SEQ_SERVER_INSWITCH (by clone_i2e) as GETRES to client
                                                        self.client.eg_port_forward_tbl_table_add_with_update_getres_latest_seq_server_inswitch_to_getres(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                    # is_hot (cm_predicate=1), is_wrong_pipeline (not need range/hash partition), tmp_client_sid=0 (not need mirroring for res), is_lastclone_for_pktloss should be 0 for GETRES_LATEST_SEQ_INSWITCH
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size: 48 + 2 = 50
                                                    #if is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                                    if is_hot == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = GETRES_LATEST_SEQ_INSWITCH,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        if is_cached == 1 and validvalue == 1 and is_latest == 0 and snapshot_flag == 1 and is_case1 == 0:
                                                            # Update GETRES_LATEST_SEQ_INSWITCH as GETRES_LATEST_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                            if is_deleted == 0: # is_deleted=0 -> stat=1
                                                                #actnspec0 = distfarreachleaf_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, self.devPorts[1], 1)
                                                                actnspec0 = distfarreachleaf_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, 1, reflector_dp2cpserver_port)
                                                            elif is_deleted == 1: # is_deleted=1 -> stat=0
                                                                #actnspec0 = distfarreachleaf_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, self.devPorts[1], 0)
                                                                actnspec0 = distfarreachleaf_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, 0, reflector_dp2cpserver_port)
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getres_latest_seq_inswitch_to_getres_latest_seq_inswitch_case1_clone_for_pktloss(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                        elif is_cached == 0:
                                                            # Update GETRES_LATEST_SEQ_INSWITCH as GETRES_LATEST_SEQ to spine (NOT dropped by drop_tbl)
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getres_latest_seq_inswitch_to_getres_latest_seq(self.sess_hdl, self.dev_tgt, matchspec0)
                                                        # Keep GETERS_LATEST_SEQ_INSWITCH unchanged, and resort to drop_tbl to drop it
                                                        #else:
                                                        #    # Drop GETRES_LATEST_SEQ_INSWITCH
                                                        #    self.client.eg_port_forward_tbl_table_add_with_drop_getres_latest_seq_inswitch(\
                                                        #            self.sess_hdl, self.dev_tgt, matchspec0)
                                                    # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_wrong_pipeline=0, tmp_client_sid=0, and snapshot_flag=1 (same inswitch_hdr as GETRES_LATEST_SEQ_INSWITCH); is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_case1 should be 0 for GETRES_LATEST_SEQ_INSWITCH_CASE1
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size: 1
                                                    #if is_cached == 1 and is_wrong_pipeline == 0 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_case1 == 0:
                                                    if is_cached == 1 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = GETRES_LATEST_SEQ_INSWITCH_CASE1,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        if is_lastclone_for_pktloss == 0:
                                                            # Forward GETRES_LATEST_SEQ_INSWITCH_CASE0 (by clone_e2e) to reflector (w/ clone)
                                                            actnspec0 = distfarreachleaf_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid)
                                                            self.client.eg_port_forward_tbl_table_add_with_forward_getres_latest_seq_inswitch_case1_clone_for_pktloss(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                        elif is_lastclone_for_pktloss == 1:
                                                            # Forward GETRES_LATEST_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector
                                                            #self.client.eg_port_forward_tbl_table_add_with_forward_getres_latest_seq_inswitch_case1(\
                                                            #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                            # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                            pass
                                                    # is_cached (no inswitch_hdr due to no field list when clone_i2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline (no inswitch_hdr), tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktloss, snapshot_flag (no inswitch_hdr), is_case1 should be 0 for GETRES_DELETED_SEQ_SERVER
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size: 1
                                                    #if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                    if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = GETRES_DELETED_SEQ_SERVER,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # TODO: check if we need to set egress port for packet cloned by clone_i2e
                                                        # Update GETRES_DELETED_SEQ_SERVER (by clone_i2e) as GETRES to client
                                                        self.client.eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_server_to_getres(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                    # size: 4
                                                    # NOTE: for GETRES_DELETED_SEQ_SERVER_INSWITCH, inswitch_hdr is set by spine switch
                                                    # tmp_client_sid = 0 (spine NOT set client_sid in prepare_for_cachehit_tbl for GETRES_DELETED_SEQ_SERVER); yet spine switch could set is_cached and snapshot_flag for GETRES_DELETED_SEQ_SERVER, which is changed as GETRES_DELETED_SEQ_SERVER_INSWITCH to leaf switch
                                                    if is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = GETRES_DELETED_SEQ_SERVER_INSWITCH,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # Update GETRES_DELETED_SEQ_SERVER_INSWITCH (by clone_i2e) as GETRES to client
                                                        self.client.eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_server_inswitch_to_getres(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                    # is_hot (cm_predicate=1), is_wrong_pipeline (not need range/hash partition), tmp_client_sid=0 (not need mirroring for res), is_lastclone_for_pktloss should be 0 for GETRES_DELETED_SEQ_INSWITCH
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size: 50
                                                    #if is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
                                                    if is_hot == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = GETRES_DELETED_SEQ_INSWITCH,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        if is_cached == 1 and validvalue == 1 and is_latest == 0 and snapshot_flag == 1 and is_case1 == 0:
                                                            # Update GETRES_DELETED_SEQ_INSWITCH as GETRES_DELETED_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                            if is_deleted == 0: # is_deleted=0 -> stat=1
                                                                #actnspec0 = distfarreachleaf_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, self.devPorts[1], 1)
                                                                actnspec0 = distfarreachleaf_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, 1, reflector_dp2cpserver_port)
                                                            elif is_deleted == 1: # is_deleted=1 -> stat=0
                                                                #actnspec0 = distfarreachleaf_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, self.devPorts[1], 0)
                                                                actnspec0 = distfarreachleaf_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid, 0, reflector_dp2cpserver_port)
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq_inswitch_case1_clone_for_pktloss(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                        elif is_cached == 0:
                                                            # Update GETRES_DELETED_SEQ_INSWITCH as GETRES_DELETED_SEQ to spine (NOT dropped by drop_tbl)
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_inswitch_to_getres_deleted_seq(self.sess_hdl, self.dev_tgt, matchspec0)
                                                        # Keep GETERS_DELETED_SEQ_INSWITCH unchanged, and resort to drop_tbl to drop it
                                                        #else:
                                                        #    # Drop GETRES_DELETED_SEQ_INSWITCH
                                                        #    self.client.eg_port_forward_tbl_table_add_with_drop_getres_deleted_seq_inswitch(\
                                                        #            self.sess_hdl, self.dev_tgt, matchspec0)
                                                    # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_wrong_pipeline=0, tmp_client_sid=0, and snapshot_flag=1 (same inswitch_hdr as GETRES_LATEST_SEQ_INSWITCH); is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_case1 should be 0 for GETRES_DELETED_SEQ_INSWITCH_CASE1
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size: 1
                                                    #if is_cached == 1 and is_wrong_pipeline == 0 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_case1 == 0:
                                                    if is_cached == 1 and snapshot_flag == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = GETRES_DELETED_SEQ_INSWITCH_CASE1,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        if is_lastclone_for_pktloss == 0:
                                                            # Forward GETRES_DELETED_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                            actnspec0 = distfarreachleaf_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss_action_spec_t(self.reflector_sid)
                                                            self.client.eg_port_forward_tbl_table_add_with_forward_getres_deleted_seq_inswitch_case1_clone_for_pktloss(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                        elif is_lastclone_for_pktloss == 1:
                                                            # Forward GETRES_DELETED_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector
                                                            #self.client.eg_port_forward_tbl_table_add_with_forward_getres_deleted_seq_inswitch_case1(\
                                                            #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                            # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                            pass
                                                    # is_cached=0 (memset inswitch_hdr by end-host, and key must not be cached in cache_lookup_tbl for CACHE_POP_INSWITCH), is_hot (cm_predicate=1), validvalue, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_POP_INSWITCH
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size: 4
                                                    #if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                    if is_cached == 0 and is_hot == 0 and validvalue == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = CACHE_POP_INSWITCH,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # Update CACHE_POP_INSWITCH as CACHE_POP_INSWITCH_ACK to reflector (deprecated: w/ clone)
                                                        actnspec0 = distfarreachleaf_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone_action_spec_t(self.reflector_sid, reflector_dp2cpserver_port)
                                                        self.client.eg_port_forward_tbl_table_add_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), snapshot_flag, is_case1, is_lastclone_for_pktloss should be 0 for CACHE_POP_INSWITCH_ACK
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size: 0
                                                    #if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0 and snapshot_flag == 0 and is_case1 == 0:
                                                    if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and snapshot_flag == 0 and is_case1 == 0 and is_lastclone_for_pktloss == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = CACHE_POP_INSWITCH_ACK,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        #if is_lastclone_for_pktloss == 0:
                                                        #    # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector (w/ clone)
                                                        #    actnspec0 = distfarreachleaf_forward_cache_pop_inswitch_ack_clone_for_pktloss_action_spec_t(self.reflector_sid)
                                                        #    self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack_clone_for_pktloss(\
                                                        #            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                        #elif is_lastclone_for_pktloss == 1:
                                                        #    # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                                        #    self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack(\
                                                        #            self.sess_hdl, self.dev_tgt, matchspec0)

                                                        # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                                        #self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack(\
                                                        #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                        # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                        pass
                                                    # is_lastclone_for_pktloss should be 0 for PUTREQ_INSWITCH
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size: 192*spine_physical_num=192
                                                    if is_lastclone_for_pktloss == 0 and tmp_client_sid != 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = PUTREQ_SEQ_INSWITCH,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        if is_cached == 0:
                                                            if snapshot_flag == 1:
                                                                if is_hot == 1:
                                                                    # Update PUTREQ_SEQ_INSWITCH as PUTREQ_POP_SEQ_CASE3 to server
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_pop_seq_case3(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                                elif is_hot == 0:
                                                                    # Update PUTREQ_SEQ_INSWITCH as PUTREQ_SEQ_CASE3 to server
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_seq_case3(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                            elif snapshot_flag == 0:
                                                                if is_hot == 1:
                                                                    # Update PUTREQ_SEQ_INSWITCH as PUTREQ_POP_SEQ to server
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_pop_seq(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                                elif is_hot == 0:
                                                                    # Update PUTREQ_SEQ_INSWITCH as PUTREQ_SEQ to server
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_seq(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                        elif is_cached == 1:
                                                            if validvalue == 0 or validvalue == 3:
                                                                if snapshot_flag == 1:
                                                                    # Update PUTREQ_SEQ_INSWITCH as PUTREQ_SEQ_CASE3 to server
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_seq_case3(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                                elif snapshot_flag == 0:
                                                                    # Update PUTREQ_SEQ_INSWITCH as PUTREQ_SEQ to server
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_seq(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                            elif validvalue == 1:
                                                                if snapshot_flag == 1 and is_case1 == 0:
                                                                    # Update PUTREQ_SEQ_INSWITCH as PUTREQ_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                                    if is_deleted == 0: # is_deleted=0 -> stat=1
                                                                        #actnspec0 = distfarreachleaf_update_putreq_seq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.reflector_sid, self.devPorts[1], 1)
                                                                        actnspec0 = distfarreachleaf_update_putreq_seq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.reflector_sid, 1, reflector_dp2cpserver_port)
                                                                    elif is_deleted == 1: # is_deleted=1 -> stat=0
                                                                        #actnspec0 = distfarreachleaf_update_putreq_seq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.reflector_sid, self.devPorts[1], 0)
                                                                        actnspec0 = distfarreachleaf_update_putreq_seq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.reflector_sid, 0, reflector_dp2cpserver_port)
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                                else:
                                                                    # Update PUTREQ_SEQ_INSWITCH as PUTRES to client by mirroring
                                                                    actnspec0 = distfarreachleaf_update_putreq_seq_inswitch_to_putres_by_mirroring_action_spec_t(tmp_client_sid, server_worker_port_start)
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_to_putres_by_mirroring(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, snapshot_flag=1, is_case1 should be 0 for PUTREQ_SEQ_INSWITCH_CASE1
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size: 2*spine_physical_num=2
                                                    # TODO: check tmp_client_sid != 0
                                                    if is_cached == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and snapshot_flag == 1 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0 and tmp_client_sid != 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = PUTREQ_SEQ_INSWITCH_CASE1,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        if is_lastclone_for_pktloss == 0:
                                                            # Forward PUTREQ_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                            actnspec0 = distfarreachleaf_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres_action_spec_t(self.reflector_sid)
                                                            self.client.eg_port_forward_tbl_table_add_with_forward_putreq_seq_inswitch_case1_clone_for_pktloss_and_putres(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                        elif is_lastclone_for_pktloss == 1:
                                                            # Update PUTREQ_SEQ_INSWITCH_CASE1 as PUTRES to client by mirroring
                                                            actnspec0 = distfarreachleaf_update_putreq_seq_inswitch_case1_to_putres_by_mirroring_action_spec_t(tmp_client_sid, server_worker_port_start)
                                                            self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_inswitch_case1_to_putres_by_mirroring(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    # is_hot (cm_predicate=1), is_lastclone_for_pktloss should be 0 for DELREQ_INSWITCH
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size: 96*spine_physical_num=96
                                                    if is_hot == 0 and is_lastclone_for_pktloss == 0 and tmp_client_sid != 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = DELREQ_SEQ_INSWITCH,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        if is_cached == 0:
                                                            if snapshot_flag == 1:
                                                                # Update DELREQ_SEQ_INSWITCH as DELREQ_SEQ_CASE3 to server
                                                                self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_to_delreq_seq_case3(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                            elif snapshot_flag == 0:
                                                                # Update DELREQ_SEQ_INSWITCH as DELREQ_SEQ to server
                                                                self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_to_delreq_seq(\
                                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                        elif is_cached == 1:
                                                            if validvalue == 0 or validvalue == 3:
                                                                if snapshot_flag == 1:
                                                                    # Update DELREQ_SEQ_INSWITCH as DELREQ_SEQ_CASE3 to server
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_to_delreq_seq_case3(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                                elif snapshot_flag == 0:
                                                                    # Update DELREQ_SEQ_INSWITCH as DELREQ_SEQ to server
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_to_delreq_seq(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                            elif validvalue == 1:
                                                                if snapshot_flag == 1 and is_case1 == 0:
                                                                    # Update DELREQ_SEQ_INSWITCH as DELREQ_SEQ_INSWITCH_CASE1 to reflector (w/ clone)
                                                                    if is_deleted == 0: # is_deleted=0 -> stat=1
                                                                        #actnspec0 = distfarreachleaf_update_delreq_seq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.reflector_sid, self.devPorts[1], 1)
                                                                        actnspec0 = distfarreachleaf_update_delreq_seq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.reflector_sid, 1, reflector_dp2cpserver_port)
                                                                    elif is_deleted == 1: # is_deleted=1 -> stat=0
                                                                        #actnspec0 = distfarreachleaf_update_delreq_seq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.reflector_sid, self.devPorts[1], 0)
                                                                        actnspec0 = distfarreachleaf_update_delreq_seq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.reflector_sid, 0, reflector_dp2cpserver_port)
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_to_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                                else:
                                                                    # Update DELREQ_SEQ_INSWITCH as DELRES to client by mirroring
                                                                    actnspec0 = distfarreachleaf_update_delreq_seq_inswitch_to_delres_by_mirroring_action_spec_t(tmp_client_sid, server_worker_port_start)
                                                                    self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_to_delres_by_mirroring(\
                                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    # is_cached=1 (trigger CASE1 only if is_cached=1, inherited from clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, snapshot_flag=1, is_case1 should be 0 for DELREQ_SEQ_INSWITCH_CASE1
                                                    # is_last_scansplit and tmp_server_sid must be 0
                                                    # size: 2*spine_physical_num=2
                                                    # TODO: check tmp_client_sid != 0
                                                    if is_cached == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and snapshot_flag == 1 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0 and tmp_client_sid != 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = DELREQ_SEQ_INSWITCH_CASE1,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        if is_lastclone_for_pktloss == 0:
                                                            # Forward DELREQ_SEQ_INSWITCH_CASE1 (by clone_e2e) to reflector (w/ clone)
                                                            actnspec0 = distfarreachleaf_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres_action_spec_t(self.reflector_sid)
                                                            self.client.eg_port_forward_tbl_table_add_with_forward_delreq_seq_inswitch_case1_clone_for_pktloss_and_delres(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                        elif is_lastclone_for_pktloss == 1:
                                                            # Update DELREQ_SEQ_INSWITCHCASE1 as DELRES to client by mirroring
                                                            actnspec0 = distfarreachleaf_update_delreq_seq_inswitch_case1_to_delres_by_mirroring_action_spec_t(tmp_client_sid, server_worker_port_start)
                                                            self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_inswitch_case1_to_delres_by_mirroring(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    # is_cached=0 (no inswitch_hdr after entering egress pipeline), is_hot, validvalue,  is_latest, is_deleted, client_sid, is_lastclone_for_pktloss, snapshot_flag, is_case1 must be 0 for SCANREQ_SPLIT
                                                    # size: 2*server_physical_num=4
                                                    if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and tmp_server_sid != 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = SCANREQ_SPLIT,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        if is_last_scansplit == 1:
                                                            self.client.eg_port_forward_tbl_table_add_with_forward_scanreq_split(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        elif is_last_scansplit == 0:
                                                            actnspec0 = distfarreachleaf_forward_scanreq_split_and_clone_action_spec_t(tmp_server_sid)
                                                            self.client.eg_port_forward_tbl_table_add_with_forward_scanreq_split_and_clone(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    # is_cached=1 (key must be cached in cache_lookup_tbl for CACHE_EVICT_LOADFREQ_INSWITCH), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH
                                                    # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                                    # size: 1
                                                    if is_cached == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = CACHE_EVICT_LOADFREQ_INSWITCH,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # Update CACHE_EVICT_LOADFREQ_INSWITCH as CACHE_EVICT_LOADFREQ_INSWITCH_ACK to reflector (w/ clone)
                                                        actnspec0 = distfarreachleaf_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone_action_spec_t(self.reflector_sid, reflector_dp2cpserver_port)
                                                        self.client.eg_port_forward_tbl_table_add_with_update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH_ACK
                                                    # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                                    # size: 0
                                                    if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = CACHE_EVICT_LOADFREQ_INSWITCH_ACK,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # Forward CACHE_EVICT_LOADFREQ_INSWITCH_ACK (by clone_e2e) to reflector
                                                        #self.client.eg_port_forward_tbl_table_add_with_forward_cache_evict_loadfreq_inswitch_ack(\
                                                        #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                        # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                        pass
                                                    # is_cached=1 (key must be cached in cache_lookup_tbl for CACHE_EVICT_LOADDATA_INSWITCH), is_hot (cm_predicate=1), validvalue, is_latest, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADDATA_INSWITCH
                                                    # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
                                                    # size: 2
                                                    if is_cached == 1 and is_hot == 0 and validvalue == 0 and is_latest == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = CACHE_EVICT_LOADDATA_INSWITCH,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # Update CACHE_EVICT_LOADDATA_INSWITCH as CACHE_EVICT_LOADDATA_INSWITCH_ACK to reflector
                                                        actnspec0 = distfarreachleaf_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone_action_spec_t(self.reflector_sid, reflector_dp2cpserver_port, tmpstat)
                                                        self.client.eg_port_forward_tbl_table_add_with_update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for CACHE_EVICT_LOADDATA_INSWITCH_ACK
                                                    # size: 0
                                                    if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = CACHE_EVICT_LOADDATA_INSWITCH_ACK,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # Forward CACHE_EVICT_LOADDATA_INSWITCH_ACK (by clone_e2e) to reflector
                                                        #self.client.eg_port_forward_tbl_table_add_with_forward_cache_evict_loaddata_inswitch_ack(\
                                                        #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                        # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                        pass
                                                    # is_hot (cm_predicate=1), validvalue, is_latest, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for LOADSNAPSHOTDATA_INSWITCH
                                                    # NOTE: is_cached can be 0 or 1 (key may be / may not be evicted after snapshot timepoint)
                                                    # size: 4
                                                    if is_hot == 0 and validvalue == 0 and is_latest == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = LOADSNAPSHOTDATA_INSWITCH,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # Update LOADSNAPSHOTDATA_INSWITCH as LOADSNAPSHOTDATA_INSWITCH_ACK to reflector
                                                        actnspec0 = distfarreachleaf_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone_action_spec_t(self.reflector_sid, reflector_dp2cpserver_port, tmpstat)
                                                        self.client.eg_port_forward_tbl_table_add_with_update_loadsnapshotdata_inswitch_to_loadsnapshotdata_inswitch_ack_drop_and_clone(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    # is_hot (cm_predicate=1), validvalue, is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for LOADSNAPSHOTDATA_INSWITCH_ACK
                                                    # NOTE: is_cached can be 0 or 1 (inswitch_hdr inherited from LOADSNAPSHOTDATA_INSWITCH)
                                                    # size: 0
                                                    if is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = LOADSNAPSHOTDATA_INSWITCH_ACK,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # Forward LOADSNAPSHOTDATA_INSWITCH_ACK (by clone_e2e) to reflector
                                                        #self.client.eg_port_forward_tbl_table_add_with_forward_loadsnapshotdata_inswitch_ack(\
                                                        #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                        # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                        pass
                                                    # is_hot (cm_predicate=1), is_latest, is_deleted, tmp_client_sid=0, is_lastclone_for_pktloss, snapshot_flag, is_case1 should be 0 for SETVALID_INSWITCH
                                                    # NOTE: is_cached can be 0 or 1 (key may be / may not be cached for SETVALID_INSWITCH)
                                                    # NOTE: validvalue can be 0/1/3 for SETVALID_INSWITCH
                                                    # size: 8
                                                    if is_hot == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = SETVALID_INSWITCH,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # Update SETVALID_INSWITCH as SETVALID_INSWITCH_ACK to reflector
                                                        actnspec0 = distfarreachleaf_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone_action_spec_t(self.reflector_sid, reflector_dp2cpserver_port)
                                                        self.client.eg_port_forward_tbl_table_add_with_update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                    # is_cached=0 (no inswtich_hdr), is_hot (cm_predicate=1), validvalue (no validvalue_hdr), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos, snapshot_flag, is_case1 should be 0 for SETVALID_INSWITCH_ACK
                                                    # NOTE: is_cached must be 0 (SETVALID_INSWITCH_ACK does not have inswitch_hdr)
                                                    # NOTE: validvalue must be 0 (no validvalue_hdr and not touch validvalue_reg)
                                                    # size: 0
                                                    if is_cached == 0 and is_hot == 0 and validvalue == 0 and is_latest == 0 and is_deleted == 0 and tmp_client_sid == 0 and is_lastclone_for_pktloss == 0 and snapshot_flag == 0 and is_case1 == 0 and is_last_scansplit == 0 and tmp_server_sid == 0:
                                                        matchspec0 = distfarreachleaf_eg_port_forward_tbl_match_spec_t(\
                                                            op_hdr_optype = SETVALID_INSWITCH_ACK,
                                                            inswitch_hdr_is_cached = is_cached,
                                                            meta_is_hot = is_hot,
                                                            validvalue_hdr_validvalue = validvalue,
                                                            meta_is_latest = is_latest,
                                                            meta_is_deleted = is_deleted,
                                                            #inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                                            inswitch_hdr_client_sid = tmp_client_sid,
                                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss,
                                                            inswitch_hdr_snapshot_flag = snapshot_flag,
                                                            meta_is_case1 = is_case1,
                                                            meta_is_last_scansplit = is_last_scansplit,
                                                            meta_server_sid = tmp_server_sid)
                                                        # Forward SETVALID_INSWITCH_ACK (by clone_e2e) to reflector
                                                        #self.client.eg_port_forward_tbl_table_add_with_forward_setvalid_inswitch_ack(\
                                                        #        self.sess_hdl, self.dev_tgt, matchspec0)
                                                        # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                                                        pass

