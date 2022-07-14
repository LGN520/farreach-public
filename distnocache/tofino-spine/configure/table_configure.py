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

from distnocachespine.p4_pd_rpc.ttypes import *
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
deleted_list = [0, 1]
sampled_list = [0, 1]
lastclone_list = [0, 1]
snapshot_flag_list = [0, 1]
case1_list = [0, 1]
access_val_mode_list = [0, 1, 2, 3]


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
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["distnocachespine"])

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

        self.recirPorts = [64, 192]

        # NOTE: in each pipeline, 64-67 are recir/cpu ports, 68-71 are recir/pktgen ports
        #self.cpuPorts = [64, 192] # CPU port is 100G

        sidnum = len(self.client_devports) + len(self.server_devports)
        sids = random.sample(xrange(BASE_SID_NORM, MAX_SID_NORM), sidnum)
        self.client_sids = sids[0:len(self.client_devports)]
        self.server_sids = sids[len(self.client_devports):sidnum]

        # NOTE: data plane communicate with switchos by software-based reflector, which is deployed in one server machine
        isvalid = False
        for i in range(server_physical_num):
            if reflector_ip_for_switchos == server_ip_for_controller_list[i]:
                isvalid = True
                self.reflector_ip_for_switch = server_ips[i]
                self.reflector_mac_for_switch = server_macs[i]
                self.reflector_devport = self.server_devports[i]
                self.refletor_sid = self.server_sids[i] # clone to switchos (i.e., reflector at [the first] physical server)
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

            ################################
            ### Normal MAT Configuration ###
            ################################

            # Ingress pipeline

            # Stage 0

            # Table: l2l3_forward_tbl (default: nop; size: client_physical_num+server_physical_num = 4 < 16)
            print "Configuring l2l3_forward_tbl"
            for i in range(client_physical_num):
                matchspec0 = distnocachespine_l2l3_forward_tbl_match_spec_t(\
                        ethernet_hdr_dstAddr = macAddr_to_string(client_macs[i]),
                        ipv4_hdr_dstAddr = ipv4Addr_to_i32(client_ips[i]),
                        ipv4_hdr_dstAddr_prefix_length = 32)
                actnspec0 = distnocachespine_l2l3_forward_action_spec_t(self.client_devports[i])
                self.client.l2l3_forward_tbl_table_add_with_l2l3_forward(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for i in range(server_physical_num):
                matchspec0 = distnocachespine_l2l3_forward_tbl_match_spec_t(\
                        ethernet_hdr_dstAddr = macAddr_to_string(server_macs[i]),
                        ipv4_hdr_dstAddr = ipv4Addr_to_i32(server_ips[i]),
                        ipv4_hdr_dstAddr_prefix_length = 32)
                actnspec0 = distnocachespine_l2l3_forward_action_spec_t(self.server_devports[i])
                self.client.l2l3_forward_tbl_table_add_with_l2l3_forward(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            if RANGE_SUPPORT == False:
                # Table: hash_for_partition_tbl (default: nop; size: 4)
                print "Configuring hash_for_partition_tbl"
                for tmpoptype in [GETREQ, PUTREQ, DELREQ, LOADREQ]:
                    matchspec0 = distnocachespine_hash_for_partition_tbl_match_spec_t(\
                            op_hdr_optype = convert_u16_to_i16(tmpoptype))
                    self.client.hash_for_partition_tbl_table_add_with_hash_for_partition(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 1

            if RANGE_SUPPORT == True:
                # Table: range_partition_tbl (default: nop; size <= 5 * 128)
                print "Configuring range_partition_tbl"
                key_range_per_server = pow(2, 16) / server_total_logical_num
                for tmpoptype in [GETREQ, PUTREQ, DELREQ, SCANREQ, LOADREQ]:
                    key_start = 0 # [0, 2^16-1]
                    for global_server_logical_idx in range(server_total_logical_num):
                        if global_server_logical_idx == server_total_logical_num - 1:
                            key_end = pow(2, 16) - 1
                        else:
                            key_end = key_start + key_range_per_server - 1
                        # NOTE: both start and end are included
                        matchspec0 = distnocachespine_range_partition_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                op_hdr_keyhihihi_start = convert_u16_to_i16(key_start),
                                op_hdr_keyhihihi_end = convert_u16_to_i16(key_end))
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
                            if tmpoptype != SCANREQ:
                                actnspec0 = distnocachespine_range_partition_action_spec_t(udp_dstport, eport)
                                self.client.range_partition_tbl_table_add_with_range_partition(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                            else:
                                actnspec0 = distnocachespine_range_partition_for_scan_action_spec_t(udp_dstport, eport, global_server_logical_idx)
                                self.client.range_partition_tbl_table_add_with_range_partition_for_scan(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                        key_start = key_end + 1
            else:
                # Table: hash_partition_tbl (default: nop; size <= 4 * 128)
                print "Configuring hash_partition_tbl"
                hash_range_per_server = switch_partition_count / server_total_logical_num
                for tmpoptype in [GETREQ, PUTREQ, DELREQ, LOADREQ]:
                    hash_start = 0 # [0, partition_count-1]
                    for global_server_logical_idx in range(server_total_logical_num):
                        if global_server_logical_idx == server_total_logical_num - 1:
                            hash_end = switch_partition_count - 1 # if end is not included, then it is just processed by port 1111
                        else:
                            hash_end = hash_start + hash_range_per_server - 1
                        # NOTE: both start and end are included
                        matchspec0 = distnocachespine_hash_partition_tbl_match_spec_t(\
                                op_hdr_optype = convert_u16_to_i16(tmpoptype),
                                meta_hashval_for_partition_start = convert_u16_to_i16(hash_start),
                                meta_hashval_for_partition_end = convert_u16_to_i16(hash_end))
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
                            actnspec0 = distnocachespine_hash_partition_action_spec_t(udp_dstport, eport)
                            self.client.hash_partition_tbl_table_add_with_hash_partition(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                        hash_start = hash_end + 1

            # Stage 2

            if RANGE_SUPPORT == True:
                # Table: range_partition_for_scan_endkey_tbl (default: nop; size <= 1 * 128)
                # TODO: limit max_scannum <= constant (e.g., 32)
                print "Configuring range_partition_for_scan_endkey_tbl"
                key_range_per_server = pow(2, 16) / server_total_logical_num
                endkey_start = 0 # [0, 2^16-1]
                for global_server_logical_idx in range(server_total_logical_num):
                    if global_server_logical_idx == server_total_logical_num - 1:
                        endkey_end = pow(2, 16) - 1
                    else:
                        endkey_end = endkey_start + key_range_per_server - 1
                    # NOTE: both start and end are included
                    matchspec0 = distnocachespine_range_partition_for_scan_endkey_tbl_match_spec_t(\
                            op_hdr_optype = SCANREQ,
                            scan_hdr_keyhihihi_start = convert_u16_to_i16(endkey_start),
                            scan_hdr_keyhihihi_end = convert_u16_to_i16(endkey_end))
                    #last_udpport_plus_one = server_worker_port_start + global_server_logical_idx + 1 # used to calculate max_scannum in data plane
                    #actnspec0 = distnocachespine_range_partition_for_scan_endkey_action_spec_t(last_udpport_plus_one)
                    end_globalserveridx_plus_one = global_server_logical_idx + 1 # used to calculate max_scannum in data plane
                    actnspec0 = distnocachespine_range_partition_for_scan_endkey_action_spec_t(end_globalserveridx_plus_one)
                    # set cur_scanidx = 0; set max_scannum = last_udpport_plus_one - udp_hdr.dstPort (first_udpport)
                    self.client.range_partition_for_scan_endkey_tbl_table_add_with_range_partition_for_scan_endkey(\
                            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0) # 0 is priority (range may be overlapping)
                    endkey_start = endkey_end + 1

            # Stage 3

            # Table: ipv4_forward_tbl (default: nop; size: 5*client_physical_num=10 < 5*8=40)
            print "Configuring ipv4_forward_tbl"
            for tmp_client_physical_idx in range(client_physical_num):
                ipv4addr0 = ipv4Addr_to_i32(client_ips[tmp_client_physical_idx])
                eport = self.client_devports[tmp_client_physical_idx]
                tmpsid = self.client_sids[tmp_client_physical_idx]
                for tmpoptype in [GETRES, PUTRES, DELRES, SCANRES_SPLIT, LOADACK]:
                    matchspec0 = distnocachespine_ipv4_forward_tbl_match_spec_t(\
                            op_hdr_optype = convert_u16_to_i16(tmpoptype),
                            ipv4_hdr_dstAddr = ipv4addr0,
                            ipv4_hdr_dstAddr_prefix_length = 32)
                    actnspec0 = distnocachespine_forward_normal_response_action_spec_t(eport)
                    self.client.ipv4_forward_tbl_table_add_with_forward_normal_response(\
                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Stage 4

            # Table: ig_port_forward_tbl (default: nop; size: 1)
            if RANGE_SUPPORT:
                print "Configuring ig_port_forward_tbl"
                matchspec0 = distnocachespine_ig_port_forward_tbl_match_spec_t(\
                        op_hdr_optype = SCANREQ)
                self.client.ig_port_forward_tbl_table_add_with_update_scanreq_to_scanreq_split(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Egress pipeline

            # Stage 0

            if RANGE_SUPPORT:
                # Table: process_scanreq_split_tbl (default: reset_meta_serversid_remainscannum; size <= 2 * 128)
                print "Configuring process_scanreq_split_tbl"
                #for clone_src in [NOT_CLONED, CLONED_FROM_EGRESS]:
                for is_clone in [0, 1]:
                    for global_server_logical_idx in range(server_total_logical_num):
                        #dstport = server_worker_port_start + global_server_logical_idx
                        matchspec0 = distnocachespine_process_scanreq_split_tbl_match_spec_t(\
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
                                actnspec0 = distnocachespine_process_scanreq_split_action_spec_t(tmp_server_sid)
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
                                actnspec0 = distnocachespine_process_cloned_scanreq_split_action_spec_t(tmp_udpport, tmp_server_sid)
                                self.client.process_scanreq_split_tbl_table_add_with_process_cloned_scanreq_split(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Stgae 1

            # Table: lastscansplit_tbl (default: reset_is_lastscansplit; size: 1)
            if RANGE_SUPPORT == True:
                print "Configuring lastscansplit_tbl"
                matchspec0 = distnocachespine_lastscansplit_tbl_match_spec_t(\
                        op_hdr_optype = SCANREQ_SPLIT,
                        meta_remain_scannum = 1)
                self.client.lastscansplit_tbl_table_add_with_set_is_lastscansplit(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 10

            # Table: eg_port_forward_tbl (default: nop; size: < 2048 < 8192)
            if RANGE_SUPPORT == True:
                print "Configuring eg_port_forward_tbl"
                self.configure_eg_port_forward_tbl_with_range()

            # Table: update_pktlen_tbl (default: nop; 1)
            if RANGE_SUPPORT == True:
                print "Configuring update_pktlen_tbl"
                scanreqsplit_udplen = 49
                scanreqsplit_iplen = 69
                matchspec0 = distnocachespine_update_pktlen_tbl_match_spec_t(\
                        op_hdr_optype=SCANREQ_SPLIT)
                actnspec0 = distnocachespine_update_pktlen_action_spec_t(scanreqsplit_udplen, scanreqsplit_iplen)
                self.client.update_pktlen_tbl_table_add_with_update_pktlen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Table: update_ipmac_srcport_tbl (default: nop; 5*client_physical_num+5*server_physical_num=20 < 10*8=80 < 128)
            # NOTE: udp.dstport is updated by eg_port_forward_tbl (only required by switch2switchos)
            # NOTE: update_ipmac_srcport_tbl focues on src/dst ip/mac and udp.srcport
            print "Configuring update_ipmac_srcport_tbl"
            # (1) for response from server to client, egress port has been set based on ip.dstaddr (or by clone_i2e) in ingress pipeline
            # (2) for response from switch to client, egress port has been set by clone_e2e in egress pipeline
            for tmp_client_physical_idx in range(client_physical_num):
                tmp_devport = self.client_devports[tmp_client_physical_idx]
                tmp_client_mac = client_macs[tmp_client_physical_idx]
                tmp_client_ip = client_ips[tmp_client_physical_idx]
                tmp_server_mac = server_macs[0]
                tmp_server_ip = server_ips[0]
                actnspec0 = distnocachespine_update_ipmac_srcport_server2client_action_spec_t(\
                        macAddr_to_string(tmp_client_mac), \
                        macAddr_to_string(tmp_server_mac), \
                        ipv4Addr_to_i32(tmp_client_ip), \
                        ipv4Addr_to_i32(tmp_server_ip), \
                        server_worker_port_start)
                for tmpoptype in [GETRES, PUTRES, DELRES, SCANRES_SPLIT, LOADACK]:
                    matchspec0 = distnocachespine_update_ipmac_srcport_tbl_match_spec_t(\
                            op_hdr_optype = convert_u16_to_i16(tmpoptype), 
                            eg_intr_md_egress_port = tmp_devport)
                    self.client.update_ipmac_srcport_tbl_table_add_with_update_ipmac_srcport_server2client(\
                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            # for request from client to server, egress port has been set by partition_tbl in ingress pipeline
            for tmp_server_physical_idx in range(server_physical_num):
                tmp_devport = self.server_devports[tmp_server_physical_idx]
                tmp_server_mac = server_macs[tmp_server_physical_idx]
                tmp_server_ip = server_ips[tmp_server_physical_idx]
                actnspec1 = distnocachespine_update_dstipmac_client2server_action_spec_t(\
                        macAddr_to_string(tmp_server_mac), \
                        ipv4Addr_to_i32(tmp_server_ip))
                for tmpoptype in [GETREQ, PUTREQ, DELREQ, SCANREQ_SPLIT, LOADREQ]:
                    matchspec0 = distnocachespine_update_ipmac_srcport_tbl_match_spec_t(\
                            op_hdr_optype = convert_u16_to_i16(tmpoptype), 
                            eg_intr_md_egress_port = tmp_devport)
                    self.client.update_ipmac_srcport_tbl_table_add_with_update_dstipmac_client2server(\
                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec1)
            

            self.conn_mgr.complete_operations(self.sess_hdl)
            self.conn_mgr.client_cleanup(self.sess_hdl) # close session



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

    def configure_eg_port_forward_tbl_with_range(self):
        # Table: eg_port_forward_tbl (default: nop; size: 2*server_physical_num=4 < 2*8 = 16)
        tmp_server_sids = [0] + self.server_sids
        for is_last_scansplit in [0, 1]:
            for tmp_server_sid in tmp_server_sids:
                # size: 2*server_physical_num=4 < 2*8=16
                if tmp_server_sid != 0:
                    matchspec0 = distnocachespine_eg_port_forward_tbl_match_spec_t(\
                        op_hdr_optype = SCANREQ_SPLIT,
                        meta_is_last_scansplit = is_last_scansplit,
                        meta_server_sid = tmp_server_sid)
                    if is_last_scansplit == 1:
                        self.client.eg_port_forward_tbl_table_add_with_forward_scanreq_split(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                    elif is_last_scansplit == 0:
                        actnspec0 = distnocachespine_forward_scanreq_split_and_clone_action_spec_t(tmp_server_sid)
                        self.client.eg_port_forward_tbl_table_add_with_forward_scanreq_split_and_clone(\
                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
