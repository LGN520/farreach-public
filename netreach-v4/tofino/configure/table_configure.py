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

import ConfigParser
config = ConfigParser.ConfigParser()
with open(os.path.join(os.path.dirname(os.path.dirname(this_dir)), "config.ini"), "r") as f:
    config.readfp(f)

server_num = int(config.get("server", "server_num"))
server_port = int(config.get("server", "server_port"))
kv_bucket_num = int(config.get("switch", "kv_bucket_num"))
src_mac = str(config.get("client", "client_mac"))
dst_mac = str(config.get("server", "server_mac"))
src_ip = str(config.get("client", "client_ip"))
dst_ip = str(config.get("server", "server_ip"))
switch_max_vallen = int(config.get("switch", "switch_max_vallen"))
#gthreshold = int(config.get("switch", "gthreshold"))
#pthreshold = int(config.get("switch", "pthreshold"))

# Front Panel Ports
#   List of front panel ports to use. Each front panel port has 4 channels.
#   Port 1 is broken to 1/0, 1/1, 1/2, 1/3. Test uses 2 ports.
#
#   ex: ["1/0", "1/1"]
#
fp_ports = []
src_fpport = str(config.get("switch", "src_fpport"))
fp_ports.append(src_fpport)
dst_fpport = str(config.get("switch", "dst_fpport"))
fp_ports.append(dst_fpport)
#fp_ports = ["2/0", "3/0"]

iport_eports_map = {} # mapping between iport and eports in the same pipeline

GETREQ = 0x00
PUTREQ = 0x01
DELREQ = 0x02
SCANREQ = 0x03
GETRES = 0x04
PUTRES = 0x05
DELRES = 0x06
SCANRES = 0x07
GETREQ_INSWITCH = 0x08
GETREQ_POP = 0x09
GETREQ_NLATEST = 0x0a
GETRES_LATEST_SEQ = 0x0b
GETRES_DELETED_SEQ = 0x0c
GETRES_LATEST_SEQ_INSWITCH = 0x0d
GETRES_DELETED_SEQ_INSWITCH = 0x0e



GETREQ_POP_TYPE = 0x08
GETRES_POP_TYPE = 0x09
GETRES_NPOP_TYPE = 0x0a
GETRES_POP_LARGE_TYPE = 0x0b
GETRES_POP_EVICT_TYPE = 0x0c
PUTREQ_SEQ_TYPE = 0x0d 
PUTREQ_POP_TYPE = 0x0e
PUTREQ_RECIR_TYPE = 0x0f
PUTREQ_POP_EVICT_TYPE = 0x10
PUTREQ_LARGE_TYPE = 0x11
PUTREQ_LARGE_SEQ_TYPE = 0x12
PUTREQ_LARGE_RECIR_TYPE = 0x13
PUTREQ_LARGE_EVICT_TYPE = 0x14
DELREQ_SEQ_TYPE = 0x15
DELREQ_RECIR_TYPE = 0x16
PUTREQ_CASE1_TYPE = 0x17
DELREQ_CASE1_TYPE = 0x18
GETRES_POP_EVICT_CASE2_TYPE = 0x19
PUTREQ_POP_EVICT_CASE2_TYPE = 0x1a
PUTREQ_LARGE_EVICT_CASE2_TYPE = 0x1b
PUTREQ_CASE3_TYPE = 0x1c
DELREQ_CASE3_TYPE = 0x1d
PUTREQ_LARGE_CASE3_TYPE = 0x1e
PUTRES_CASE3_TYPE = 0x1f
DELRES_CASE3_TYPE = 0x20
GETRES_POP_EVICT_SWITCH_TYPE = 0x21
GETRES_POP_EVICT_CASE2_SWITCH_TYPE = 0x22
PUTREQ_POP_EVICT_SWITCH_TYPE = 0x23
PUTREQ_POP_EVICT_CASE2_SWITCH_TYPE = 0x24
PUTREQ_LARGE_EVICT_SWITCH_TYPE = 0x25
PUTREQ_LARGE_EVICT_CASE2_SWITCH_TYPE = 0x26

cached_list = [0, 1]
hot_list = [0, 1]
valid_list = [0, 1, 3]
#valid_list = [0, 1, 2, 3] # If with PUTREQ_LARGE
latest_list = [0, 1]
deleted_lsit = [0, 1]
wrong_pipeline_list = [0, 1]
sampled_list = [0, 1]
lastclone_list = [0, 1]






keymatch_list = [0, 1]
lock_list = [0, 1]
predicate_list = [1, 2]
backup_list = [0, 1]
case12_list = [0, 1]
case3_list = [0, 1]

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
        # size: ?
        for valid in valid_list:
            for is_latest in latest_list:
                matchspec0 = eval("netbufferv4_update_val{}_tbl_match_spec_t".format(valname))(
                        op_hdr_optype = GETREQ_INSWITCH,
                        inswitch_hdr_is_cached = 1,
                        status_hdr_valid = valid,
                        status_hdr_is_latest = is_latest)
                eval("self.client.update_val{}_tbl_table_add_with_get_val{}".format(valname, valname))(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
                for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                    if valid == 1 and is_latest == 0:
                        matchspec0 = eval("netbufferv4_update_val{}_tbl_match_spec_t".format(valname))(
                                op_hdr_optype = tmpoptype,
                                inswitch_hdr_is_cached = 1,
                                status_hdr_valid = valid,
                                status_hdr_is_latest = is_latest)
                        eval("self.client.update_val{}_tbl_table_add_with_set_and_get_val{}".format(valname, valname))(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
        for is_cached in cached_list:
            for valid in valid_list:
                for is_latest in latest_list:
                    matchspec0 = eval("netbufferv4_update_val{}_tbl_match_spec_t".format(valname))(
                            op_hdr_optype = CACHE_POP_INSWITCH,
                            inswitch_hdr_is_cached = is_cached,
                            status_hdr_valid = valid,
                            status_hdr_is_latest = is_latest)
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

        for iport in self.devPorts:
            iport_eports_map[iport] = []
            for eport in self.devPorts:
                iport_eports_map[iport].append(eport)

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

            # Table: sid_tbl (default: nop; size: ?)
            print "Configuring sid_tbl"
            for tmpoptype in [GETREQ]:
                matchspec0 = netbufferv4_sid_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        ig_intr_md_ingress_port = self.devPorts[0])
                actnspec0 = netbufferv4_set_sid_action_spec_t(self.sids[0])
                self.client.sid_tbl_table_add_with_set_sid(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                # Should not used: no req from server
                matchspec0 = netbufferv4_sid_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        ig_intr_md_ingress_port = self.devPorts[1])
                actnspec0 = netbufferv4_set_sid_action_spec_t(self.sids[1])
                self.client.sid_tbl_table_add_with_set_sid(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Table: cache_lookup_tbl (default: uncached_action; size: 64K)
            print "Leave cache_lookup_tbl managed by controller in runtime"

            # Table: hash_tbl (default: nop; size: ?)
            print "Configuring hash_tbl"
            for tmpoptype in [GETREQ, CACHE_POP_INSWITCH]:
                matchspec0 = netbufferv4_hash_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype)
                self.client.hash_tbl_table_add_with_hash(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: sample_tbl (default: nop; size: ?)
            print "Configuring sample_tbl"
            for tmpoptype in [GETREQ]:
                matchspec0 = netbufferv4_sample_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype)
                self.client.sample_tbl_table_add_with_sample(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 1

            # Table: hash_partition_tbl (default: nop; server_num <= 128)
            print "Configuring hash_partition_tbl"
            hash_start = 0
            hash_range_per_server = kv_bucket_num / server_num
            for tmpoptype in [GETREQ, CACHE_POP_INSWITCH]:
                for iport in self.devPorts:
                    for i in range(server_num):
                        if i == server_num - 1:
                            hash_end = kv_bucket_num - 1 # if end is not included, then it is just processed by port 1111
                        else:
                            hash_end = hash_start + hash_range_per_server
                        # NOTE: both start and end are included
                        matchspec0 = netbufferv4_hash_partition_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                inswitch_hdr_hashval_start = hash_start,
                                inswitch_hdr_hashval_end = hash_end,
                                ig_intr_md_ingress_port = iport)
                        # Forward to the egress pipeline of server
                        # serveridx = i
                        eport = self.devPorts[1]
                        if eport in iport_eports_map[iport]: # in correct pipeline
                            actnspec0 = netbufferv4_hash_partition_action_spec_t(\
                                    server_port + i, self.devPorts[1], 0)
                        else: # in wrong pipeline
                            actnspec0 = netbufferv4_hash_partition_action_spec_t(\
                                    server_port + i, self.devPorts[1], 1)
                        self.client.hash_partition_tbl_table_add_with_hash_partition(\
                                self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                        hash_start = hash_end

            # Stage 2

            # Table: ig_port_forward_tbl (default: nop; size: ?)
            print "Configuring ig_port_forward_tbl"
            matchspec0 = netbufferv4_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETREQ)
            self.client.ig_port_forward_tbl_table_add_with_update_getreq_to_getreq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_LATEST_SEQ)
            self.client.ig_port_forward_tbl_table_add_with_update_getres_latest_seq_to_getres_latest_seq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_ig_port_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_DELETED_SEQ)
            self.client.ig_port_forward_tbl_table_add_with_update_getres_deleted_seq_to_getres_deleted_seq_inswitch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 3

            # Table: ipv4_forward_tbl (default: nop; size: ?)
            print "Configuring ipv4_forward_tbl"
            ipv4addr0 = ipv4Addr_to_i32(src_ip)
            matchspec0 = netbufferv4_ipv4_forward_tbl_match_spec_t(\
                    op_hdr_optype = GETRES,
                    ipv4_hdr_dstAddr = ipv4addr0,
                    ipv4_hdr_dstAddr_prefix_length = 32)
            actnspec0 = netbufferv4_forward_normal_response_action_spec_t(self.devPorts[0])
            self.client.ipv4_forward_tbl_table_add_with_forward_normal_response(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                matchspec0 = netbufferv4_ipv4_forward_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        ipv4_hdr_dstAddr = ipv4addr0,
                        ipv4_hdr_dstAddr_prefix_length = 32)
                actnspec0 = netbufferv4_ipv4_forward_special_get_response_action_spec_t(self.sids[0])
                self.client.ipv4_forward_tbl_table_add_with_forward_special_get_response(\
                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Egress pipeline

            # Stage 0

            # Table: access_cm1_tbl (default: initialize_cm1_predicate; size: ?)
            print "Configuring access_cm1_tbl"
            for tmpoptype in [GETREQ_INSWITCH]:
                matchspec0 = netbufferv4_access_cm1_tbl_match_spec_t(\
                        op_hdr.optype = tmpoptype,
                        inswitch_hdr_is_sampled = 1,
                        inswitch_hdr_is_cached = 0)
                self.client.access_cm1_tbl_table_add_with_update_cm1(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_cm2_tbl (default: initialize_cm2_predicate; size: ?)
            print "Configuring access_cm2_tbl"
            for tmpoptype in [GETREQ_INSWITCH]:
                matchspec0 = netbufferv4_access_cm2_tbl_match_spec_t(\
                        op_hdr.optype = tmpoptype,
                        inswitch_hdr_is_sampled = 1,
                        inswitch_hdr_is_cached = 0)
                self.client.access_cm2_tbl_table_add_with_update_cm2(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_cm3_tbl (default: initialize_cm3_predicate; size: ?)
            print "Configuring access_cm3_tbl"
            for tmpoptype in [GETREQ_INSWITCH]:
                matchspec0 = netbufferv4_access_cm3_tbl_match_spec_t(\
                        op_hdr.optype = tmpoptype,
                        inswitch_hdr_is_sampled = 1,
                        inswitch_hdr_is_cached = 0)
                self.client.access_cm3_tbl_table_add_with_update_cm3(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_cm4_tbl (default: initialize_cm4_predicate; size: ?)
            print "Configuring access_cm4_tbl"
            for tmpoptype in [GETREQ_INSWITCH]:
                matchspec0 = netbufferv4_access_cm4_tbl_match_spec_t(\
                        op_hdr.optype = tmpoptype,
                        inswitch_hdr_is_sampled = 1,
                        inswitch_hdr_is_cached = 0)
                self.client.access_cm4_tbl_table_add_with_update_cm4(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Stgae 1

            # Table: is_hot_tbl (default: not_hop; size: 1)
            print "Configuring is_hot_tbl"
            matchspec0 = netbufferv4_is_hot_tbl_match_spec_t(\
                    meta_cm1_predicate = 2,
                    meta_cm2_predicate = 2,
                    meta_cm3_predicate = 2,
                    meta_cm4_predicate = 2)
            self.client.is_hot_tbl_table_add_with_is_hot(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_cache_frequency_tbl (default: nop; size: ?)
            print "Configuring access_cache_frequency_tbl"
            for tmpoptype in [GETREQ_INSWITCH]:
                matchspec0 = netbufferv4_access_cache_frequency_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        inswitch_hdr_is_sampled = 1,
                        inswitch_hdr_is_cached = 1)
                self.client.access_cache_frequency_table_add_with_update_cache_frequency(\
                        self.sess_hdl, self.dev_tgt, match_spec0)
            for is_sampled in sampled_list:
                for is_cached in cached_list:
                    matchspec0 = netbufferv4_access_cache_frequency_tbl_match_spec_t(\
                            op_hdr_optype = CACHE_POP_INSWITCH,
                            inswitch_hdr_is_sampled = is_sampled,
                            inswitch_hdr_is_cached = is_cached)
                    self.client.access_cache_frequency_table_add_with_reset_cache_frequency(\
                            self.sess_hdl, self.dev_tgt, match_spec0)

            # Table: access_valid_tbl (default: nop; size: ?)
            print "Configuring access_valid_tbl"
            for tmpoptype in [GETREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                matchspec0 = netbufferv4_access_valid_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        inswitch_hdr_is_cached = 1)
                self.client,access_valid_tbl_table_add_with_get_valid(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Stgae 2

            # Table: access_latest_tbl (default: nop; size: ?)
            print "Configuring access_latest_tbl"
            for valid in valid_list:
                matchspec0 = netbufferv4_access_latest_tbl_match_spec_t(\
                        op_hdr_optype = GETREQ_INSWITCH,
                        inswitch_hdr_is_cached = 1,
                        status_hdr_valid = valid)
                self.client.access_latest_tbl_table_add_with_get_latest(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
                for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                    if valid == 1:
                        matchspec0 = netbufferv4_access_latest_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                inswitch_hdr_is_cached = 1,
                                status_hdr_valid = valid)
                        self.client.access_latest_tbl_table_add_with_set_and_get_latest(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
            for is_cached in cached_list:
                for valid in valid_list:
                    matchspec0 = netbufferv4_access_latest_tbl_match_spec_t(\
                            op_hdr_optype = CACHE_POP_INSWITCH,
                            inswitch_hdr_is_cached = is_cached,
                            status_hdr_valid = valid)
                    self.client.access_latest_tbl_table_add_with_reset_and_get_latest(\
                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 3

            # Table: access_deleted_tbl (default: nop; size: ?)
            print "Configuring access_deleted_tbl"
            for valid in valid_list:
                for is_latest in latest_list:
                    matchspec0 = netbufferv4_access_deleted_tbl_match_spec_t(\
                            op_hdr_optype = GETREQ_INSWITCH,
                            inswitch_hdr_is_cached = 1,
                            status_hdr_valid = valid,
                            status_hdr_is_latest = is_latest)
                    self.client.access_deleted_tbl_table_add_with_get_deleted(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
                    if valid == 1 and is_latest == 0:
                        matchspec0 = netbufferv4_access_deleted_tbl_match_spec_t(\
                                op_hdr_optype = GETRES_LATEST_SEQ_INSWITCH,
                                inswitch_hdr_is_cached = 1,
                                status_hdr_valid = valid,
                                status_hdr_is_latest = is_latest)
                        self.client.access_deleted_tbl_table_add_with_reset_and_get_deleted(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = netbufferv4_access_deleted_tbl_match_spec_t(\
                                op_hdr_optype = GETRES_DELETED_SEQ_INSWITCH,
                                inswitch_hdr_is_cached = 1,
                                status_hdr_valid = valid,
                                status_hdr_is_latest = is_latest)
                        self.client.access_deleted_tbl_table_add_with_set_and_get_deleted(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
            for is_cached in cached_list:
                for valid in valid_list:
                    for is_latest in latest_list:
                        matchspec0 = netbufferv4_access_deleted_tbl_match_spec_t(\
                                op_hdr_optype = CACHE_POP_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                status_hdr_valid = valid,
                                status_hdr_is_latest = is_latest)
                        self.client.access_deleted_tbl_table_add_with_reset_and_get_deleted(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: update_vallen_tbl (default: nop; 24)
            print "Configuring update_vallen_tbl"
            for valid in valid_list:
                for is_latest in latest_list:
                    matchspec0 = netbufferv4_update_vallen_tbl_match_spec_t(\
                            op_hdr_optype = GETREQ_INSWITCH,
                            inswitch_hdr_is_cached = 1,
                            status_hdr_valid = valid,
                            status_hdr_is_latest = is_latest)
                    self.client.update_vallen_tbl_table_add_with_get_vallen(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
                    for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH]:
                        if valid == 1 and is_latest == 0:
                            matchspec0 = netbufferv4_update_vallen_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    inswitch_hdr_is_cached = 1,
                                    status_hdr_valid = valid,
                                    status_hdr_is_latest = is_latest)
                            self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
            for is_cached in cached_list:
                for valid in valid_list:
                    for is_latest in latest_list:
                        matchspec0 = netbufferv4_update_vallen_tbl_match_spec_t(\
                                op_hdr_optype = CACHE_POP_INSWITCH,
                                inswitch_hdr_is_cached = is_cached,
                                status_hdr_valid = valid,
                                status_hdr_is_latest = is_latest)
                        self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
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

            # Table: lastclone_tbl (default: not_lastclone_action; size: 1)
            print "Configuring lastclone_tbl"
            matchspec0 = netbufferv4_lastclone_tbl_match_spec_t(\
                    op_hdr_optype = CACHE_POP_INSWITCH_ACK,
                    meta_clonenum_for_pktloss = 0)
            self.client.lastclone_tbl_table_add_with_is_lastclone_action(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 10

            # Table: eg_port_forward_tbl (default: nop; size: ?)
            print "Configuring eg_port_forward_tbl")
            for is_cached in cached_list:
                for is_hot in hot_list:
                    for valid in valid_list:
                        for is_latest in latest_list:
                            for is_deleted in deleted_list:
                                for is_wrong_pipeline in pipeline_list:
                                    for is_lastclone_for_pktloss in lastclone_list:
                                        matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = GETREQ_INSWITCH,
                                            inswitch_hdr_is_cached = is_cached,
                                            meta_is_hot = is_hot,
                                            status_hdr_valid = valid,
                                            status_hdr_is_latest = is_latest,
                                            status_hdr_is_deleted = is_deleted,
                                            inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss)
                                        if is_cached == 0:
                                            if is_hot == 1:
                                                # Update GETREQ_INSWITCH as GETREQ_POP to server
                                                actnspec0 = netbufferv4_update_getreq_inswitch_to_getreq_pop_action_spec_t(self.devPorts[1])
                                                self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_pop(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update GETREQ_INSWITCH as GETREQ to server
                                                actnspec0 = netbufferv4_update_getreq_inswitch_to_getreq_action_spec_t(self.devPorts[1])
                                                self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            if valid == 0:
                                                # Update GETREQ_INSWITCH as GETREQ to server
                                                actnspec0 = netbufferv4_update_getreq_inswitch_to_getreq_action_spec_t(self.devPorts[1])
                                                self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            elif valid == 1:
                                                if is_latest == 0:
                                                    # Update GETREQ_INSWITCH as GETREQ_NLATEST to server
                                                    actnspec0 = netbufferv4_update_getreq_inswitch_to_getreq_nlatest_action_spec_t(self.devPorts[1])
                                                    self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq_nlatest(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                else:
                                                    if is_wrong_pipeline == 0:
                                                        if is_deleted == 1:
                                                            # Update GETREQ_INSWITCH as GETRES for deleted value to client
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_for_deleted(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        else:
                                                            # Update GETREQ_INSWITCH as GETRES to client
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                    elif is_wrong_pipeline == 1:
                                                        if is_deleted == 1:
                                                            # Update GETREQ_INSWITCH as GETRES for deleted value to client by mirroring
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_for_deleted_by_mirroring(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        else:
                                                            # Update GETREQ_INSWITCH as GETRES to client by mirroring
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                            elif valid == 3:
                                                if is_latest == 0:
                                                    # Update GETREQ_INSWITCH as GETREQ to server
                                                    actnspec0 = netbufferv4_update_getreq_inswitch_to_getreq_action_spec_t(self.devPorts[1])
                                                    self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getreq(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                                else:
                                                    if is_wrong_pipeline == 0:
                                                        if is_deleted == 1:
                                                            # Update GETREQ_INSWITCH as GETRES for deleted value to client
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_for_deleted(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        else:
                                                            # Update GETREQ_INSWITCH as GETRES to client
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                    elif is_wrong_pipeline == 1:
                                                        if is_deleted == 1:
                                                            # Update GETREQ_INSWITCH as GETRES for deleted value to client by mirroring
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_for_deleted_by_mirroring(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        else:
                                                            # Update GETREQ_INSWITCH as GETRES to client by mirroring
                                                            self.client.eg_port_forward_tbl_table_add_with_update_getreq_inswitch_to_getres_by_mirroring(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                        matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = GETRES_LATEST_SEQ,
                                            inswitch_hdr_is_cached = is_cached,
                                            meta_is_hot = is_hot,
                                            status_hdr_valid = valid,
                                            status_hdr_is_latest = is_latest,
                                            status_hdr_is_deleted = is_deleted,
                                            inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss)
                                        # TODO: check if we need to set egress port for packet cloned by clone_i2e
                                        # Update GETRES_LATEST_SEQ (by clone_i2e) as GETRES to client
                                        self.client.eg_port_forward_tbl_table_add_with_update_getres_latest_seq_to_getres(\
                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                        matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = GETRES_LATEST_SEQ_INSWITCH,
                                            inswitch_hdr_is_cached = is_cached,
                                            meta_is_hot = is_hot,
                                            status_hdr_valid = valid,
                                            status_hdr_is_latest = is_latest,
                                            status_hdr_is_deleted = is_deleted,
                                            inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss)
                                        # Drop GETRES_LATEST_SEQ_INSWITCH
                                        self.client.eg_port_forward_tbl_table_add_with_drop_getres_latest_seq_inswitch(\
                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                        matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = GETRES_DELETED_SEQ,
                                            inswitch_hdr_is_cached = is_cached,
                                            meta_is_hot = is_hot,
                                            status_hdr_valid = valid,
                                            status_hdr_is_latest = is_latest,
                                            status_hdr_is_deleted = is_deleted,
                                            inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss)
                                        # TODO: check if we need to set egress port for packet cloned by clone_i2e
                                        # Update GETRES_DELETED_SEQ (by clone_i2e) as GETRES to client
                                        self.client.eg_port_forward_tbl_table_add_with_update_getres_deleted_seq_to_getres(\
                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                        matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = GETRES_DELETED_SEQ_INSWITCH,
                                            inswitch_hdr_is_cached = is_cached,
                                            meta_is_hot = is_hot,
                                            status_hdr_valid = valid,
                                            status_hdr_is_latest = is_latest,
                                            status_hdr_is_deleted = is_deleted,
                                            inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss)
                                        # Drop GETRES_DELETED_SEQ_INSWITCH
                                        self.client.eg_port_forward_tbl_table_add_with_drop_getres_deleted_seq_inswitch(\
                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                        matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = CACHE_POP_INSWITCH,
                                            inswitch_hdr_is_cached = is_cached,
                                            meta_is_hot = is_hot,
                                            status_hdr_valid = valid,
                                            status_hdr_is_latest = is_latest,
                                            status_hdr_is_deleted = is_deleted,
                                            inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss)
                                        # Update CACHE_POP_INSWITCH as CACHE_POP_INSWITCH_ACK to reflector (w/ clone)
                                        actnspec0 = update_cache_pop_inswitch_to_cache_pop_inswitch_ack_action_spec_t(self.sids[1])
                                        self.client.eg_port_forward_tbl_table_add_with_update_cache_pop_inswitch_to_cache_pop_inswitch_ack_clone_for_pktloss(\
                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = CACHE_POP_INSWITCH_ACK,
                                            inswitch_hdr_is_cached = is_cached,
                                            meta_is_hot = is_hot,
                                            status_hdr_valid = valid,
                                            status_hdr_is_latest = is_latest,
                                            status_hdr_is_deleted = is_deleted,
                                            inswitch_hdr_is_wrong_pipeline = is_wrong_pipeline,
                                            meta_is_lastclone_for_pktloss = is_lastclone_for_pktloss)
                                        # TODO: check if we need to set egress port for packet cloned by clone_e2e
                                        if is_lastclone_for_pktloss == 0:
                                            # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector (w/ clone)
                                            actnspec0 = forward_cache_pop_inswitch_ack_clone_for_pktloss_action_spec_t(self.sids[1])
                                            self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack_clone_for_pktloss(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        elif is_lastclone_for_pktloss == 1:
                                            # Forward CACHE_POP_INSWITCH_ACK (by clone_e2e) to reflector
                                            self.client.eg_port_forward_tbl_table_add_with_forward_cache_pop_inswitch_ack(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)












            # Stage 1

            # Table assign_seq_tbl (default: nop; 3)
            # NOTE: PUTREQ_RECIR, DELREQ_RECIR, and PUTREQ_LARGE_RECIR do not need to assign new seq
            print "Configuring assign_seq_tbl"
            for tmpoptype in [PUTREQ_TYPE, DELREQ_TYPE]:
                matchspec0 = netbufferv4_assign_seq_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype)
                self.client.assign_seq_tbl_table_add_with_assign_seq(\ # Assign seq to seq_hdr.seq
                        self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_assign_seq_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_LARGE_TYPE)
            self.client.assign_seq_tbl_table_add_with_assign_seq_large(\ # Assign seq to meta.seq_large
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_assign_seq_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_LARGE_RECIR_TYPE)
            self.client.assign_seq_tbl_table_add_with_copy_seq_large(\ # Copy seq_hdr.seq to meta.seq_large (not assign new seq) 
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table update_iskeymatch_tbl (default: update_iskeymatch(0); 1)
            print "Configuring update_iskeymatch_tbl"
            matchspec0 = netbufferv4_update_iskeymatch_tbl_match_spec_t(\
                    meta_ismatch_keylolo=2,
                    meta_ismatch_keylohi=2,
                    meta_ismatch_keyhilo=2,
                    meta_ismatch_keyhihi=2)
            actnspec0 = netbufferv4_update_iskeymatch_action_spec_t(1)
            self.client.update_iskeymatch_tbl_table_add_with_update_iskeymatch(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Stage 2

            # Table: access_savedseq_tbl (default: nop; 12)
            print "Configuring access_savedseq_tbl"
            for tmpoptype in [PUTREQ_TYPE, PUTREQ_RECIR_TYPE, DELREQ_TYPE, DELREQ_RECIR_TYPE]:
                matchspec0 = netbufferv4_access_savedseq_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        other_hdr_isvalid = 1,
                        meta_iskeymatch = 1)
                self.client.access_savedseq_tbl_table_add_with_try_update_savedseq(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
            for isvalid in valid_list:
                for iskeymatch in  keymatch_list:
                    for tmpoptype in [GETRES_POP_TYPE, PUTREQ_POP_TYPE]:
                        matchspec0 = netbufferv4_access_savedseq_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                other_hdr_isvalid = isvalid,
                                meta_iskeymatch = iskeymatch)
                        #self.client.access_savedseq_tbl_table_add_with_reset_savedseq(\
                        #        self.sess_hdl, self.dev_tgt, matchspec0)
                        self.client.access_savedseq_tbl_table_add_with_set_and_get_savedseq(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
            for tmpoptype in [PUTREQ_LARGE_TYPE, PUTREQ_LARGE_RECIR_TYPE]:
                matchspec0 = netbufferv4_access_savedseq_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        other_hdr_isvalid = 1,
                        meta_iskeymatch = 1)
                self.client.access_savedseq_tbl_table_add_with_get_savedseq(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_lock_tbl (default: nop; 28)
            print "Configuring access_lock_tbl"
            for isvalid in valid_list:
                for zerovote in predicate_list:
                    for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE, PUTREQ_RECIR_TYPE]:
                        matchspec0 = netbufferv4_access_lock_tbl_match_spec_t(
                                op_hdr_optype=tmpoptype,
                                other_hdr_isvalid=isvalid,
                                meta_zerovote=zerovote)
                        if isvalid == 0 or zerovote == 2:
                            self.client.access_lock_tbl_table_add_with_try_lock(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        else:
                            self.client.access_lock_tbl_table_add_with_read_lock(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                    matchspec0 = netbufferv4_access_lock_tbl_match_spec_t(
                            op_hdr_optype=DELREQ_TYPE,
                            other_hdr_isvalid=isvalid,
                            meta_zerovote=zerovote)
                    self.client.access_lock_tbl_table_add_with_read_lock(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
            for isvalid in valid_list:
                for zerovote in predicate_list:
                    for tmpoptype in [GETRES_POP_TYPE, GETRES_NPOP_TYPE, GETRES_POP_LARGE_TYPE, PUTREQ_POP_TYPE]:
                        matchspec0 = netbufferv4_access_lock_tbl_match_spec_t(
                                op_hdr_optype=tmpoptype,
                                other_hdr_isvalid=isvalid,
                                meta_zerovote=zerovote)
                        self.client.access_lock_tbl_table_add_with_reset_lock(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

            # Start from stage 3

            # Table: access_case12_tbl (default: nop; 128)
            print "Configuring access_case12_tbl"
            for isvalid in valid_list:
                for iskeymatch in keymatch_list:
                    for canput in predicate_list:
                        for isbackup in backup_list:
                            # For case 1
                            for tmpoptype in [PUTREQ_TYPE, PUTREQ_RECIR_TYPE, DELREQ_TYPE, DELREQ_RECIR_TYPE]:
                                matchspec0 = netbufferv4_access_case12_tbl_match_spec_t(\
                                        op_hdr_optype=tmpoptype,
                                        other_hdr_isvalid=isvalid,
                                        meta_iskeymatch=iskeymatch,
                                        meta_canput=canput,
                                        meta_isbackup=isbackup)
                                if isvalid == 1 and iskeymatch == 1 and canput == 2 and isbackup == 1:
                                    self.client.access_case12_tbl_table_add_with_try_case12(\
                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                else:
                                    self.client.access_case12_tbl_table_add_with_read_case12(\
                                            self.sess_hdl, self.dev_tgt, matchspec0)
                            # For case 2
                            for tmpoptype in [GETRES_POP_TYPE, PUTREQ_POP_TYPE]:
                                matchspec0 = netbufferv4_access_case12_tbl_match_spec_t(\
                                        op_hdr_optype=tmpoptype,
                                        other_hdr_isvalid=isvalid,
                                        meta_iskeymatch=iskeymatch,
                                        meta_canput=canput,
                                        meta_isbackup=isbackup)
                                if isbackup == 1:
                                    self.client.access_case12_tbl_table_add_with_try_case12(\
                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                else:
                                    self.client.access_case12_tbl_table_add_with_read_case12(\
                                            self.sess_hdl, self.dev_tgt, matchspec0)
                            for tmpoptype in [PUTREQ_LARGE_TYPE, PUTREQ_LARGE_RECIR_TYPE]:
                                matchspec0 = netbufferv4_access_case12_tbl_match_spec_t(\
                                        op_hdr_optype=tmpoptype,
                                        other_hdr_isvalid=isvalid,
                                        meta_iskeymatch=iskeymatch,
                                        meta_canput=canput,
                                        meta_isbackup=isbackup)
                                if isbackup == 1 and isvalid == 1 and iskeymatch == 1:
                                    self.client.access_case12_tbl_table_add_with_try_case12(\
                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                else:
                                    self.client.access_case12_tbl_table_add_with_read_case12(\
                                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 11

            # Table access_case3_tbl (default: nop; 128)
            #print "Configuring access_case3_tbl"
            #for isvalid in valid_list:
            #    for zerovote in predicate_list:
            #        for iskeymatch in keymatch_list:
            #            for islock in lock_list:
            #                for isbackup in backup_list:
            #                    for tmpoptype in [PUTREQ_TYPE, PUTREQ_RECIR_TYPE]:
            #                        matchspec0 = netbufferv4_access_case3_tbl_match_spec_t(\
            #                                op_hdr_optype = tmpoptype,
            #                                other_hdr_isvalid = isvalid,
            #                                meta_zerovote = zerovote,
            #                                meta_iskeymatch = iskeymatch,
            #                                meta_islock = islock,
            #                                meta_isbackup = isbackup)
            #                        if islock == 0 and (isvalid == 0 or zerovote == 2):
            #                            continue
            #                        elif isvalid == 1 and iskeymatch == 1:
            #                            continue
            #                        elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
            #                            continue
            #                        else:
            #                            if isbackup == 0:
            #                                continue
            #                            else:
            #                                self.client.access_case3_tbl_table_add_with_try_case3(\
            #                                        self.sess_hdl, self.dev_tgt, matchspec0)
            #                    for tmpoptype in [DELREQ_TYPE, DELREQ_RECIR_TYPE]:
            #                        matchspec0 = netbufferv4_access_case3_tbl_match_spec_t(\
            #                                op_hdr_optype = tmpoptype,
            #                                other_hdr_isvalid = isvalid,
            #                                meta_zerovote = zerovote,
            #                                meta_iskeymatch = iskeymatch,
            #                                meta_islock = islock,
            #                                meta_isbackup = isbackup)
            #                        if isvalid == 1 and iskeymatch == 1:
            #                            continue
            #                        elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
            #                            continue
            #                        else:
            #                            if isbackup == 0:
            #                                continue
            #                            else:
            #                                self.client.access_case3_tbl_table_add_with_try_case3(\
            #                                        self.sess_hdl, self.dev_tgt, matchspec0)


            # Table: port_forward_tbl (default: nop; 1665)
            print "Configuring port_forward_tbl"
            for isvalid in valid_list:
                for zerovote in predicate_list:
                    for iskeymatch in keymatch_list:
                        for islock in lock_list:
                            for canput in  predicate_list:
                                for isbackup in backup_list:
                                    for iscase12 in case12_list:
                                        matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = GETREQ_TYPE,
                                                other_hdr_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if islock == 0 and (isvalid == 0 or zerovote == 2):
                                            # Update GETREQ as GETREQ_POP to server 
                                            actnspec0 = netbufferv4_update_getreq_to_getreq_pop_action_spec_t(self.devPorts[1])
                                            self.client.port_forward_tbl_table_add_with_update_getreq_to_getreq_pop(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        elif isvalid == 1 and iskeymatch == 1:
                                            # Sendback GETRES to client
                                            self.client.port_forward_tbl_table_add_with_update_getreq_to_getres(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                        elif islock == 1 and (isvalid == 0 or iskeymatch == 0):
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv4_recirculate_getreq_action_spec_t(self.recirPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_recirculate_getreq(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            # Forwrad GETREQ to server 
                                            actnspec0 = netbufferv4_port_forward_action_spec_t(self.devPorts[1]) 
                                            self.client.port_forward_tbl_table_add_with_port_forward(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = GETRES_POP_TYPE,
                                                other_hdr_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if isbackup == 1 and iscase12 == 0:
                                                # Update GETRES_POP as GETRES_POP_EVICT_CASE2 to server, clone original pkt with new kv to client port for GETRES to client
                                                actnspec0 = netbufferv4_update_getres_pop_to_case2_clone_for_getres_action_spec_t(\
                                                        self.sids[0])
                                                self.client.port_forward_tbl_table_add_with_update_getres_pop_to_case2_clone_for_getres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            if isvalid == 0: 
                                                # Drop GETRES_POP with old kv, clone original pkt with new kv to client port for GETRES to client
                                                actnspec0 = netbufferv4_drop_getres_pop_clone_for_getres_action_spec_t(\
                                                        self.sids[0])
                                                self.client.port_forward_tbl_table_add_with_drop_getres_pop_clone_for_getres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            elif isvalid == 1:
                                                # Update GETRES_POP as GETRES_POP_EVICT to server, clone original pkt with new kv to client port for GETRES to client
                                                actnspec0 = netbufferv4_update_getres_pop_to_evict_clone_for_getres_action_spec_t(\
                                                        self.sids[0])
                                                self.client.port_forward_tbl_table_add_with_update_getres_pop_to_evict_clone_for_getres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        # Update GETRES_NPOP as GETRES to client
                                        matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = GETRES_NPOP_TYPE,
                                                other_hdr_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        actnspec0 = netbufferv4_update_getres_npop_to_getres_action_spec_t(\
                                                self.devPorts[0])
                                        self.client.port_forward_tbl_table_add_with_update_getres_npop_to_getres(\
                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        # Update GETRES_POP_LARGE as GETRES to client
                                        matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = GETRES_POP_LARGE_TYPE,
                                                other_hdr_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        actnspec0 = netbufferv4_update_getres_pop_large_to_getres_action_spec_t(\
                                                self.devPorts[0])
                                        self.client.port_forward_tbl_table_add_with_update_getres_pop_large_to_getres(\
                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = PUTREQ_TYPE,
                                                other_hdr_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if islock == 0 and (isvalid == 0 or zerovote == 2):
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv4_update_putreq_to_putreq_pop_action_spec_t(self.recirPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_update_putreq_to_putreq_pop(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        elif isvalid == 1 and iskeymatch == 1:
                                            if canput == 2 and isbackup == 1 and iscase12 == 0:
                                                # Update PUTREQ as PUTREQ_CASE1 to server, clone original packet as PUTRES to client
                                                actnspec0 = netbufferv4_update_putreq_to_case1_clone_for_putres_action_spec_t(\
                                                        self.sids[0], self.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_to_case1_clone_for_putres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Sendback PUTRES to client
                                                self.client.port_forward_tbl_table_add_with_update_putreq_to_putres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                        elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv4_update_putreq_to_putreq_recir_action_spec_t(self.recirPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_update_putreq_to_putreq_recir(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            if isbackup == 0:
                                                # Update PUTREQ as PUTREQ_SEQ to server (forward with new seq)
                                                actnspec0 = netbufferv4_update_putreq_to_putreq_seq_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_putreq_to_putreq_seq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update PUTREQ as PUTREQ_MAY_CASE3 to server
                                                actnspec0 = netbufferv4_update_putreq_to_may_case3_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_putreq_to_may_case3(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = PUTREQ_POP_TYPE,
                                                other_hdr_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if isbackup == 1 and iscase12 == 0:
                                                # Update PUTREQ_POP as PUTREQ_POP_EVICT_CASE2 to server, clone original pkt with new kv to client port for PUTRES to client
                                                actnspec0 = netbufferv4_update_putreq_pop_to_case2_clone_for_putres_action_spec_t(\
                                                        self.sids[0], self.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_pop_to_case2_clone_for_putres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            if isvalid == 0: 
                                                # Drop PUTREQ_POP with old kv, clone original pkt with new kv to client port for PUTRES to client
                                                actnspec0 = netbufferv4_drop_putreq_pop_clone_for_putres_action_spec_t(\
                                                        self.sids[0])
                                                self.client.port_forward_tbl_table_add_with_drop_putreq_pop_clone_for_putres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            elif isvalid == 1:
                                                # Update PUTREQ_POP as PUTREQ_POP_EVICT to server, clone original pkt with new kv to client port for PUTRES to client
                                                actnspec0 = netbufferv4_update_putreq_pop_to_evict_clone_for_putres_action_spec_t(\
                                                        self.sids[0], self.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_pop_to_evict_clone_for_putres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = PUTREQ_RECIR_TYPE,
                                                other_hdr_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if islock == 0 and (isvalid == 0 or zerovote == 2):
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv4_update_putreq_recir_to_putreq_pop_action_spec_t(self.recirPorts[0])     
                                            self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_putreq_pop(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        elif isvalid == 1 and iskeymatch == 1:
                                            if canput == 2 and isbackup == 1 and iscase12 == 0:
                                                # Update PUTREQ_RECIR as PUTREQ_CASE1 to server, clone original packet as PUTRES to client
                                                actnspec0 = netbufferv4_update_putreq_recir_to_case1_clone_for_putres_action_spec_t(\
                                                        self.sids[0], self.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_case1_clone_for_putres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Sendback PUTRES to client
                                                self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_putres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                        elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv4_recirculate_putreq_recir_action_spec_t(self.recirPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_recirculate_putreq_recir(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            if isbackup == 0:
                                                # Update PUTREQ_RECIR as PUTREQ_SEQ to server (forward) 
                                                actnspec0 = netbufferv4_update_putreq_recir_to_putreq_seq_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_putreq_seq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update PUTREQ_RECIR as PUTREQ_MAY_CASE3 to server
                                                actnspec0 = netbufferv4_update_putreq_recir_to_may_case3_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_may_case3(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = PUTREQ_LARGE_TYPE,
                                                other_hdr_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if iskeymatch == 0 and islock == 1:
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv4_update_putreq_large_to_putreq_large_recir_action_spec_t(self.recirPorts[0])
                                            self.client.port_forward_tbl_table_add_with_update_putreq_large_to_putreq_large_recir(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        elif iskeymatch == 1 and isvalid == 1:
                                            if isbackup == 1 and iscase12 == 0:
                                                # Update PUTREQ_LARGE as PUTREQ_LARGE_EVICT_CASE2 to server, clone original packet as PUTREQ_LARGE to server
                                                actnspec0 = netbufferv4_update_putreq_large_to_case2_clone_for_putreq_large_seq_action_spec_t(\
                                                        self.sids[1], sefl.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_large_to_case2_clone_for_putreq_large_seq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update PUTREQ_LARGE as PUTREQ_LARGE_EVICT to server, clone original packet as PUTREQ_LARGE to server
                                                actnspec0 = netbufferv4_update_putreq_large_to_evict_clone_for_putreq_large_seq_action_spec_t(\
                                                        self.sids[1], sefl.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_large_to_evict_clone_for_putreq_large_seq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            # Update PUTREQ_LARGE as PUTREQ_LARGE_SEQ to server (forward)
                                            actnspec0 = netbufferv4_update_putreq_large_to_putreq_large_seq_action_spec_t(self.devPorts[1]) 
                                            self.client.port_forward_tbl_table_add_with_update_putreq_large_to_putreq_large_seq(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = PUTREQ_LARGE_RECIR_TYPE,
                                                other_hdr_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if iskeymatch == 0 and islock == 1:
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv4_recirculate_putreq_large_recir_action_spec_t(self.recirPorts[0])
                                            self.client.port_forward_tbl_table_add_with_recirculate_putreq_large_recir(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        elif iskeymatch == 1 and isvalid == 1:
                                            if isbackup == 1 and iscase12 == 0:
                                                # Update PUTREQ_LARGE_RECIR as PUTREQ_LARGE_EVICT_CASE2 to server, clone original packet as PUTREQ_LARGE to server
                                                actnspec0 = netbufferv4_update_putreq_large_recir_to_case2_clone_for_putreq_large_seq_action_spec_t(\
                                                        self.sids[1], self.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_large_recir_to_case2_clone_for_putreq_large_seq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update PUTREQ_LARGE_RECIR as PUTREQ_LARGE_EVICT to server, clone original packet as PUTREQ_LARGE to server
                                                actnspec0 = netbufferv4_update_putreq_large_recir_to_evict_clone_for_putreq_large_seq_action_spec_t(\
                                                        self.sids[1], sefl.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_large_recir_to_evict_clone_for_putreq_large_seq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            # Update PUTREQ_LARGE_RECIR as PUTREQ_LARGE_SEQ to server
                                            actnspec0 = netbufferv4_update_putreq_large_recir_to_putreq_large_seq_action_spec_t(self.devPorts[1]) 
                                            self.client.port_forward_tbl_table_add_with_update_putreq_large_recir_to_putreq_large_seq(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = DELREQ_TYPE,
                                                other_hdr_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if isvalid == 1 and iskeymatch == 1:
                                            if canput == 2 and isbackup == 1 and iscase12 == 0:
                                                # Update DELREQ as DELREQ_CASE1 to server, clone original packet as DELRES to client
                                                actnspec0 = netbufferv4_update_delreq_to_case1_clone_for_delres_action_spec_t(\
                                                        self.sids[0], self.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_delreq_to_case1_clone_for_delres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Sendback DELRES to client
                                                self.client.port_forward_tbl_table_add_with_update_delreq_to_delres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                        elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv4_update_delreq_to_delreq_recir_action_spec_t(self.recirPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_update_delreq_to_delreq_recir(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            if isbackup == 0:
                                                # Update DELREQ as DELREQ_SEQ to server (forward)
                                                actnspec0 = netbufferv4_update_delreq_to_delreq_seq_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_delerq_to_delreq_seq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update DELREQ as DELREQ_MAY_CASE3 to server 
                                                actnspec0 = netbufferv4_update_delreq_to_may_case3_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_delreq_to_may_case3(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = DELREQ_RECIR_TYPE,
                                                other_hdr_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if isvalid == 1 and iskeymatch == 1:
                                            if canput == 2 and isbackup == 1 and iscase12 == 0:
                                                # Update DELREQ_RECIR as DELREQ_CASE1 to server, clone original packet as DELRES to client
                                                actnspec0 = netbufferv4_update_delreq_recir_to_case1_clone_for_delres_action_spec_t(\
                                                        self.sids[0], self.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_delreq_recir_to_case1_clone_for_delres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Sendback DELRES to client
                                                self.client.port_forward_tbl_table_add_with_update_delreq_recir_to_delres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                        elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv4_recirculate_delreq_recir_action_spec_t(self.recirPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_recirculate_delreq_recir(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            if isbackup == 0:
                                                # Update DELREQ_RECIR as DELREQ_SEQ to server (forward)
                                                actnspec0 = netbufferv4_update_delreq_recir_to_delreq_seq_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_delreq_recir_to_delreq_seq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update DELREQ_RECIR as DELREQ_MAY_CASE3 to server 
                                                actnspec0 = netbufferv4_update_delreq_recir_to_may_case3_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_delreq_recir_to_may_case3(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        # Forward SCANREQ to server
                                        matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = SCANREQ_TYPE,
                                                other_hdr_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        actnspec0 = netbufferv4_port_forward_action_spec_t(self.devPorts[1]) 
                                        self.client.port_forward_tbl_table_add_with_port_forward(\
                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        # Forward RES to client
                                        for tmpoptype in [GETRES_TYPE, PUTRES_TYPE, DELRES_TYPE, SCANRES_TYPE]:
                                            matchspec0 = netbufferv4_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = tmpoptype,
                                                    other_hdr_isvalid = isvalid,
                                                    meta_zerovote = zerovote,
                                                    meta_iskeymatch = iskeymatch,
                                                    meta_islock = islock,
                                                    meta_canput = canput,
                                                    meta_isbackup = isbackup,
                                                    meta_iscase12 = iscase12)
                                            actnspec0 = netbufferv4_port_forward_action_spec_t(self.devPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_port_forward(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)



            ### Egress ###

            # Stage 0

            # Table: process_i2e_cloned_packet_tbl
            print "Configuring process_i2e_cloned_packet_tbl"
            matchspec0 = netbufferv4_process_i2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_TYPE)
            self.client.process_i2e_cloned_packet_tbl_table_add_with_update_cloned_putreq_to_putres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_process_i2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_RECIR_TYPE)
            self.client.process_i2e_cloned_packet_tbl_table_add_with_update_cloned_putreq_recir_to_putres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_process_i2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = DELREQ_TYPE)
            self.client.process_i2e_cloned_packet_tbl_table_add_with_update_cloned_delreq_to_delres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_process_i2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = DELREQ_RECIR_TYPE)
            self.client.process_i2e_cloned_packet_tbl_table_add_with_update_cloned_delreq_to_delres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_process_i2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_POP_TYPE)
            self.client.process_i2e_cloned_packet_tbl_table_add_with_update_cloned_getres_pop_to_getres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_process_i2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_POP_TYPE)
            self.client.process_i2e_cloned_packet_tbl_table_add_with_update_cloned_putreq_pop_to_putres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_process_i2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_LARGE_TYPE)
            self.client.process_i2e_cloned_packet_tbl_table_add_with_update_cloned_putreq_large_to_putreq_large_seq(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_process_i2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_LARGE_RECIR_TYPE)
            self.client.process_i2e_cloned_packet_tbl_table_add_with_update_cloned_putreq_large_recir_to_putreq_large_seq(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: process_e2e_cloned_packet_tbl 
            print "Configuring process_e2e_cloned_packet_tbl"
            matchspec0 = netbufferv4_process_e2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_POP_EVICT)
            self.client.process_e2e_cloned_packet_tbl_table_add_with_update_cloned_getres_pop_evict_to_switch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_process_e2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_POP_EVICT_CASE2)
            self.client.process_e2e_cloned_packet_tbl_table_add_with_update_cloned_getres_pop_evict_case2_to_switch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_process_e2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_POP_EVICT)
            self.client.process_e2e_cloned_packet_tbl_table_add_with_update_cloned_putreq_pop_evict_to_switch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_process_e2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_POP_EVICT_CASE2)
            self.client.process_e2e_cloned_packet_tbl_table_add_with_update_cloned_putreq_pop_evict_case2_to_switch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_process_e2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_LARGE_EVICT)
            self.client.process_e2e_cloned_packet_tbl_table_add_with_update_cloned_putreq_large_evict_to_switch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv4_process_e2e_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_LARGE_EVICT_CASE2)
            self.client.process_e2e_cloned_packet_tbl_table_add_with_update_cloned_putreq_large_evict_case2_to_switch(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: process_may_case3_tbl (default: nop; ?)
            #print "Configuring process_may_case3_tbl"
            #matchspec0 = netbufferv4_process_may_case3_tbl_match_spec_t(\
            #        op_hdr_optype = PUTREQ_MAY_CASE3_TYPE,
            #        other_hdr_iscase3 = 0)
            #self.client.process_may_case3_tbl_table_add_with_update_putreq_may_case3_to_case3(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec0 = netbufferv4_process_may_case3_tbl_match_spec_t(\
            #        op_hdr_optype = PUTREQ_MAY_CASE3_TYPE,
            #        other_hdr_iscase3 = 1)
            #self.client.process_may_case3_tbl_table_add_with_update_putreq_may_case3_to_putreq(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec0 = netbufferv4_process_may_case3_tbl_match_spec_t(\
            #        op_hdr_optype = DELREQ_MAY_CASE3_TYPE,
            #        other_hdr_iscase3 = 0)
            #self.client.process_may_case3_tbl_table_add_with_update_delreq_may_case3_to_case3(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec0 = netbufferv4_process_may_case3_tbl_match_spec_t(\
            #        op_hdr_optype = DELREQ_MAY_CASE3_TYPE,
            #        other_hdr_iscase3 = 1)
            #self.client.process_may_case3_tbl_table_add_with_update_delreq_may_case3_to_delreq(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 1

            # Table: hash_partition_tbl (default: nop; server_num <= 128)
            print "Configuring hash_partition_tbl"
            hash_start = 0
            hash_range_per_server = bucket_num / server_num
            for i in range(server_num):
                if i == server_num - 1:
                    hash_end = bucket_num - 1 # if end is not included, then it is just processed by port 1111
                else:
                    hash_end = hash_start + hash_range_per_server
                matchspec0 = netbufferv4_hash_partition_tbl_match_spec_t(\
                        udp_hdr_dstPort=server_port, \
                        eg_intr_md_egress_port=self.devPorts[1], \
                        meta_hashidx_start = hash_start, \
                        meta_hashidx_end = hash_end)
                actnspec0 = netbufferv4_update_dstport_action_spec_t(\
                        server_port + i, i)
                self.client.hash_partition_tbl_table_add_with_update_dstport(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                hash_start = hash_end

            # Table: hash_partition_reverse_tbl (default: nop; server_num <= 128)
            print "Configuring hash_partition_reverse_tbl"
            hash_start = 0
            hash_range_per_server = bucket_num / server_num
            for i in range(server_num):
                if i == server_num - 1:
                    hash_end = bucket_num - 1 # if end is not included, then it is just processed by port 1111
                else:
                    hash_end = hash_start + hash_range_per_server
                matchspec0 = netbufferv4_hash_partition_reverse_tbl_match_spec_t(\
                        udp_hdr_srcPort=server_port, \
                        eg_intr_md_egress_port=self.devPorts[1], \
                        meta_hashidx_start = hash_start, \
                        meta_hashidx_end = hash_end)
                actnspec0 = netbufferv4_update_dstport_reverse_action_spec_t(\
                        server_port + i, i)
                self.client.hash_partition_reverse_tbl_table_add_with_update_dstport_reverse(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                hash_start = hash_end

            # Stage 2
            # Table access_case3_tbl (default: nop; 128)
            print "Configuring access_case3_tbl"
            for tmpoptype in [PUTREQ_SEQ_TYPE, DELREQ_SEQ_TYPE, PUTREQ_LARGE_SEQ_TYPE]:
                matchspec0 = netbufferv4_access_case3_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_isbackup = 1)
                self.client.access_case3_tbl_table_add_with_read_case3(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
            for tmpoptype in [PUTRES_CASE3_TYPE, DELRES_CASE3_TYPE]:
                matchspec0 = netbufferv4_access_case3_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_isbackup = 1)
                self.client.access_case3_tbl_table_add_with_set_case3(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 3

            # Table: eg_port_forward_tbl (default: nop; 44)
            print "Configuring eg_port_forward_tbl"
            for iscase3 in case3_list:
                for isbackup in backup_list:
                    for tmpoptype in [GETRES_POP_EVICT_TYPE, GETRES_POP_EVICT_CASE2_TYPE, PUTREQ_POP_EVICT_TYPE, PUTREQ_POP_EVICT_CASE2_TYPE, PUTREQ_LARGE_EVICT_TYPE, PUTREQ_LARGE_EVICT_CASE2_TYPE]:
                        # Forward the original packet to server, clone a packet to switch os simulator for packet loss
                        matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                                op_hdr.optype=tmpoptype,
                                meta_iscase3=iscase3,
                                meta_isbackup=isbackup)
                        actnspec0 = netbufferv4_forward_to_server_clone_for_pktloss_action_spec_t(\
                                self.sids[1], self.devPorts[1])
                        self.client.eg_port_forward_tbl_table_add_with_forward_to_server_clone_for_pktloss(\
                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for iscase3 in case3_list:
                for isbackup in backup_list:
                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                            op_hdr.optype=PUTREQ_SEQ_TYPE,
                            meta_iscase3=iscase3,
                            meta_isbackup=isbackup)
                    if iscase3 == 0 and isbackup == 1:
                        # Update PUTREQ_SEQ to PUTREQ_CASE3 to server
                        actnspec0 = netbufferv4_update_putreq_seq_to_putreq_case3_action_spec_t(self.devPorts[1])
                        self.client.eg_port_forward_tbl_table_add_with_update_putreq_seq_to_putreq_case3(\
                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                    else:
                        # Forward PUTREQ_SEQ to server
                        actnspec0 = netbufferv4_eg_port_forward_action_spec_t(self.devPorts[1])
                        self.client.eg_port_forward_tbl_table_add_with_eg_port_forward(\
                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                            op_hdr.optype=DELREQ_SEQ_TYPE,
                            meta_iscase3=iscase3,
                            meta_isbackup=isbackup)
                    if iscase3 == 0 and isbackup == 1:
                        # Update DELREQ_SEQ to DELREQ_CASE3 to server
                        actnspec0 = netbufferv4_update_delreq_seq_to_delreq_case3_action_spec_t(self.devPorts[1])
                        self.client.eg_port_forward_tbl_table_add_with_update_delreq_seq_to_delreq_case3(\
                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                    else:
                        # Forward DELREQ_SEQ to server
                        actnspec0 = netbufferv4_eg_port_forward_action_spec_t(self.devPorts[1])
                        self.client.eg_port_forward_tbl_table_add_with_eg_port_forward(\
                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                            op_hdr.optype=PUTREQ_LARGE_SEQ_TYPE,
                            meta_iscase3=iscase3,
                            meta_isbackup=isbackup)
                    if iscase3 == 0 and isbackup == 1:
                        # Update PUTREQ_LARGE_SEQ to PUTREQ_LARGE_CASE3 to server
                        actnspec0 = netbufferv4_update_putreq_large_seq_to_putreq_large_case3_action_spec_t(self.devPorts[1])
                        self.client.eg_port_forward_tbl_table_add_with_update_putreq_large_seq_to_putreq_large_case3(\
                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                    else:
                        # Forward PUTREQ_LARGE_SEQ to server
                        actnspec0 = netbufferv4_eg_port_forward_action_spec_t(self.devPorts[1])
                        self.client.eg_port_forward_tbl_table_add_with_eg_port_forward(\
                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                    # Update PUTRES_CASE3 as PUTRES to client
                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                            op_hdr.optype=PUTRES_CASE3_TYPE,
                            meta_iscase3=iscase3,
                            meta_isbackup=isbackup)
                    actnspec0 = netbufferv4_update_putres_case3_to_putres_action_spec_t(\
                            self.devPorts[0])
                    self.client.eg_port_forward_tbl_table_add_with_update_putres_case3_to_putres(\
                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                    # Update DELRES_CASE3 as DELRES to client
                    matchspec0 = netbufferv4_eg_port_forward_tbl_match_spec_t(\
                            op_hdr.optype=DELRES_CASE3_TYPE,
                            meta_iscase3=iscase3,
                            meta_isbackup=isbackup)
                    actnspec0 = netbufferv4_update_delres_case3_to_delres_action_spec_t(\
                            self.devPorts[0])
                    self.client.eg_port_forward_tbl_table_add_with_update_delres_case3_to_delres(\
                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Stage 4

            # Table: update_udplen_tbl (default: nop; 145)
            print "Configuring update_udplen_tbl"
            for i in range(switch_max_vallen/8 + 1): # i from 0 to 16
                if i == 0:
                    vallen_start = 0
                    vallen_end = 0
                    aligned_vallen = 0
                else:
                    vallen_start = (i-1)*8+1 # 1, ..., 121
                    vallen_end = (i-1)*8+8 # 8, ..., 128
                    aligned_vallen = vallen_end # 8, ..., 128
                # NOTE: if vallen of GETRES > 128B, it must be issued by server which has already set correct udplen
                matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=GETRES_TYPE,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end)
                actnspec0 = netbufferv4_update_getres_udplen_action_spec_t(\
                        aligned_vallen)
                self.client.update_udplen_tbl_table_add_with_update_getres_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=GETRES_POP_EVICT_TYPE,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end)
                actnspec0 = netbufferv4_update_getres_pop_evict_udplen_action_spec_t(\
                        aligned_vallen)
                self.client.update_udplen_tbl_table_add_with_update_getres_pop_evict_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=GETRES_POP_EVICT_CASE2_TYPE,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end)
                actnspec0 = netbufferv4_update_getres_pop_evict_case2_udplen_action_spec_t(\
                        aligned_vallen)
                self.client.update_udplen_tbl_table_add_with_update_getres_pop_evict_case2_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=PUTREQ_POP_EVICT_TYPE,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end)
                actnspec0 = netbufferv4_update_putreq_pop_evict_udplen_action_spec_t(\
                        aligned_vallen)
                self.client.update_udplen_tbl_table_add_with_update_putreq_pop_evict_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=PUTREQ_POP_EVICT_CASE2_TYPE,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end)
                actnspec0 = netbufferv4_update_putreq_pop_evict_case2_udplen_action_spec_t(\
                        aligned_vallen)
                matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=PUTREQ_LARGE_EVICT_TYPE,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end)
                actnspec0 = netbufferv4_update_putreq_large_evict_udplen_action_spec_t(\
                        aligned_vallen)
                self.client.update_udplen_tbl_table_add_with_update_putreq_large_evict_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=PUTREQ_LARGE_EVICT_CASE2_TYPE,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end)
                actnspec0 = netbufferv4_update_putreq_large_evict_case2_udplen_action_spec_t(\
                        aligned_vallen)
                self.client.update_udplen_tbl_table_add_with_update_putreq_large_evict_case2_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=PUTREQ_CASE1_TYPE,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end)
                actnspec0 = netbufferv4_update_putreq_case1_udplen_action_spec_t(\
                        aligned_vallen)
                self.client.update_udplen_tbl_table_add_with_update_putreq_case1_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=DELREQ_CASE1_TYPE,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end)
                actnspec0 = netbufferv4_update_delreq_case1_udplen_action_spec_t(\
                        aligned_vallen)
                self.client.update_udplen_tbl_table_add_with_update_delreq_case1_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
            # NOTE: switch never directly responds PUTREQ_LARGE even if savedseq>seq, its PUTRES must be issued by server which has already set correct udplen
            matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                    op_hdr_optype=PUTRES_TYPE,
                    vallen_hdr_vallen_start=0,
                    vallen_hdr_vallen_end=switch_max_vallen) # [0, 128]
            self.client.update_udplen_tbl_table_add_with_update_putres_udplen(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            # NOTE: DELREQ does not have value -> vallen must be 0
            matchspec0 = netbufferv4_update_udplen_tbl_match_spec_t(\
                    op_hdr_optype=DELRES_TYPE,
                    vallen_hdr_vallen_start=0,
                    vallen_hdr_vallen_end=switch_max_vallen) # [0, 128]
            self.client.update_udplen_tbl_table_add_with_update_delres_udplen(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: update_macaddr_tbl (default: nop; 5)
            print "Configuring update_macaddr_tbl"
            actnspec0 = netbufferv4_update_macaddr_s2c_action_spec_t(\
                    macAddr_to_string(src_mac), \
                    macAddr_to_string(dst_mac))
            actnspec1 = netbufferv4_update_macaddr_c2s_action_spec_t(\
                    macAddr_to_string(src_mac), \
                    macAddr_to_string(dst_mac))
            matchspec0 = netbufferv4_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_TYPE)
            matchspec1 = netbufferv4_update_macaddr_tbl_match_spec_t(op_hdr_optype=PUTRES_TYPE)
            matchspec2 = netbufferv4_update_macaddr_tbl_match_spec_t(op_hdr_optype=DELRES_TYPE)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_s2c(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_s2c(\
                    self.sess_hdl, self.dev_tgt, matchspec1, actnspec0)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_s2c(\
                    self.sess_hdl, self.dev_tgt, matchspec2, actnspec0)
            #matchspec3 = netbufferv4_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_POP_TYPE)
            #self.client.update_macaddr_tbl_table_add_with_update_macaddr_c2s(\
            #        self.sess_hdl, self.dev_tgt, matchspec3, actnspec1)
            matchspec3 = netbufferv4_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_POP_EVICT_TYPE)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_c2s(\
                    self.sess_hdl, self.dev_tgt, matchspec3, actnspec1)
            matchspec4 = netbufferv4_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_POP_EVICT_CASE2_TYPE)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_c2s(\
                    self.sess_hdl, self.dev_tgt, matchspec4, actnspec1)

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
