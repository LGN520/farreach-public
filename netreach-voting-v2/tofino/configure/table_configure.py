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

from netbufferv2.p4_pd_rpc.ttypes import *
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
bucket_num = int(config.get("switch", "bucket_num"))
src_mac = str(config.get("client", "client_mac"))
dst_mac = str(config.get("server", "server_mac"))
src_ip = str(config.get("client", "client_ip"))
dst_ip = str(config.get("server", "server_ip"))
#gthreshold = int(config.get("switch", "gthreshold"))
#pthreshold = int(config.get("switch", "pthreshold"))

# Front Panel Ports
#   List of front panel ports to use. Each front panel port has 4 channels.
#   Port 1 is broken to 1/0, 1/1, 1/2, 1/3. Test uses 2 ports.
#
#   ex: ["1/0", "1/1"]
#
fp_ports = ["2/0", "3/0"]
#src_mac = "9c:69:b4:60:ef:a5"
#dst_mac = "9c:69:b4:60:ef:8d"
#src_ip = "10.0.0.31"
#dst_ip = "10.0.0.32"

GETREQ_TYPE = 0x00
PUTREQ_TYPE = 0x01
DELREQ_TYPE = 0x02
SCANREQ_TYPE = 0x03
GETRES_TYPE = 0x04
PUTRES_TYPE = 0x05
DELRES_TYPE = 0x06
SCANRES_TYPE = 0x07
GETREQ_POP_TYPE = 0x08
GETRES_NPOP_TYPE = 0x09
GETREQ_NLATEST_TYPE = 0x0a
GETRES_LATEST_TYPE = 0x0b
GETRES_NEXIST_TYPE = 0x0c

cached_list = [0, 1]
valid_list = [0, 1]
lock_list = [0, 1]
latest_list = [0, 1, 2]
being_evicted_list = [0, 1]
predicate_list = [1, 2]
backup_list = [0, 1]

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
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbufferv2"])

    def configure_update_val_tbl(self, valname):
        for iscached in cached_list:
            for isvalid in valid_list:
                for islatest in latest_list:
                    for being_evicted in being_evicted_list:
                        matchspec0 = netbufferv2_update_vallen_tbl_match_spec_t(\
                                op_hdr_optype = GETREQ_TYPE,
                                meta_iscached = iscached,
                                meta_isvalid = isvalid,
                                meta_islatest = islatest,
                                meta_being_evicted = being_evicted)
                        eval("self.client.update_val{}_tbl_table_add_with_get_val{}".format(valname, valname))(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                        if iscached == 1 and isvalid == 1 and islatest == 0 and being_evicted == 0:
                            matchspec0 = netbufferv2_update_vallen_tbl_match_spec_t(\
                                    op_hdr_optype = GETRES_LATEST_TYPE,
                                    meta_iscached = iscached,
                                    meta_isvalid = isvalid,
                                    meta_islatest = islatest,
                                    meta_being_evicted = being_evicted)
                            eval("self.client.update_val{}_tbl_table_add_with_set_val{}".format(valname, valname))(\
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

        self.recirPorts = [64, 192]

        # NOTE: in each pipeline, 64-67 are recir/cpu ports, 68-71 are recir/pktgen ports
        #self.cpuPorts = [64, 192] # CPU port is 100G

    ### MAIN ###

    def runTest(self):
        if test_param_get('cleanup') != True:
            print '\nTest'

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

            # Enable recirculation before add port
            for i in self.recirPorts:
                self.conn_mgr.recirculation_enable(self.sess_hdl, 0, i);

            # add and enable the platform ports
            for i in self.devPorts:
               self.pal.pal_port_add(0, i,
                                     pal_port_speed_t.BF_SPEED_40G,
                                     pal_fec_type_t.BF_FEC_TYP_NONE)
               self.pal.pal_port_enable(0, i)

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

            # Bind sid with port for packet mirror
            sidnum = 1
            sids = random.sample(xrange(BASE_SID_NORM, MAX_SID_NORM), sidnum)
            print "Binding sid {} with port {}".format(sids[0], self.devPorts[0]) # clone to client
            info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
                                  Direction_e.PD_DIR_INGRESS,
                                  sids[0],
                                  self.devPorts[0],
                                  True)
            #print "Binding sid {} with port {}".format(sids[0], self.devPorts[1]) # clone to server
            #info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
            #                      Direction_e.PD_DIR_INGRESS,
            #                      sids[1],
            #                      self.devPorts[1],
            #                      True)
            self.mirror.mirror_session_create(self.sess_hdl, self.dev_tgt, info)

            ### Stage 0 ###

            # Table: cache_lookup_tbl (default: cache_lookup_default)

            # Table: access_valid_tbl (default: nop; ?)
            print "Configuring access_valid_tbl"
            matchspec0 = netbufferv2_access_valid_tbl_match_spec_t(\
                    op_hdr_optype = GETREQ_TYPE)
            self.client.access_valid_tbl_table_add_with_get_valid(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_being_evicted_tbl (default: nop; ?)
            print "Configuring access_being_evicted_tbl"
            matchspec0 = netbufferv2_access_being_evicted_match_spec_t(\
                    op_hdr_optype = GETREQ_TYPE)
            self.client.access_being_evicted_tbl_table_add_with_get_being_evicted(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            ### Stage 1 ###

            # Table: access_vote_tbl (default: nop; ?)
            print "Configuring access_vote_tbl"
            matchspec0 = netbufferv2_access_vote_tbl_match_spec_t(\
                    op_hdr_optype = GETREQ_TYPE,
                    meta_iscached = 1,
                    meta_isvalid = 1,
                    meta_being_evicted = 0)
            self.client.access_vote_tbl_table_add_with_increase_vote(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv2_access_vote_tbl_match_spec_t(\
                    op_hdr_optype = GETREQ_TYPE,
                    meta_iscached = 0,
                    meta_isvalid = 1,
                    meta_being_evicted = 0)
            self.client.access_vote_tbl_table_add_with_decrease_vote(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_latest_tbl (default: nop; ?)
            print "Configuring access_latest_tbl"
            for iscached in cached_list:
                for isvalid in valid_list:
                    for being_evicted in being_evicted_list:
                        matchspec0 = netbufferv2_access_latest_tbl_match_spec_t(\
                                op_hdr_optype = GETREQ_TYPE,
                                meta_iscached = iscached,
                                meta_isvalid = isvalid,
                                meta_being_evicted = being_evicted)
                        self.client.access_latest_tbl_table_add_with_get_latest(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                        if iscached == 1 and isvalid == 1 and being_evicted == 0:
                            matchspec0 = netbufferv2_access_latest_tbl_match_spec_t(\
                                    op_hdr_optype = GETRES_LATEST_TYPE,
                                    meta_iscached = iscached,
                                    meta_isvalid = isvalid,
                                    meta_being_evicted = being_evicted)
                            self.client.access_latest_tbl_table_add_with_try_set_latest(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                            matchspec0 = netbufferv2_access_latest_tbl_match_spec_t(\
                                    op_hdr_optype = GETRES_NEXIST_TYPE,
                                    meta_iscached = iscached,
                                    meta_isvalid = isvalid,
                                    meta_being_evicted = being_evicted)
                            self.client.access_latest_tbl_table_add_with_try_clear_latest(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)

            ### Stage 2 ###

            # Table: access_lock_tbl (default: nop; ?)
            print "Configuring access_lock_tbl"
            for isvalid in valid_list:
                for iszerovote in predicate_list:
                    for being_evicted in being_evicted_list:
                        matchspec0 = netbufferv2_access_lock_tbl_match_spec_t(\
                                op_hdr_optype = GETREQ_TYPE,
                                meta_isvalid = isvalid,
                                meta_iszerovote = iszerovote,
                                meta_being_evicted = being_evicted)
                        if (isvalid == 0 and being_evicted == 0) or (iszerovote == 2 and being_evicted == 0):
                            slef.client.access_lock_tbl_table_add_with_try_lock(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                        else:
                            slef.client.access_lock_tbl_table_add_with_read_lock(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                        matchspec0 = netbufferv2_access_lock_tbl_match_spec_t(\
                                op_hdr_optype = GETRES_NPOP_TYPE,
                                meta_isvalid = isvalid,
                                meta_iszerovote = iszerovote,
                                meta_being_evicted = being_evicted)
                        if being_evicted == 0:
                            slef.client.access_lock_tbl_table_add_with_clear_lock(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: update_vallen_tbl (default: nop; ?)
            print "Configuring update_vallen_tbl"
            for iscached in cached_list:
                for isvalid in valid_list:
                    for islatest in latest_list:
                        for being_evicted in being_evicted_list:
                            matchspec0 = netbufferv2_update_vallen_tbl_match_spec_t(\
                                    op_hdr_optype = GETREQ_TYPE,
                                    meta_iscached = iscached,
                                    meta_isvalid = isvalid,
                                    meta_islatest = islatest,
                                    meta_being_evicted = being_evicted)
                            self.client.update_vallen_tbl_table_add_with_get_vallen(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                            if iscached == 1 and isvalid == 1 and islatest == 0 and being_evicted == 0:
                                matchspec0 = netbufferv2_update_vallen_tbl_match_spec_t(\
                                        op_hdr_optype = GETRES_LATEST_TYPE,
                                        meta_iscached = iscached,
                                        meta_isvalid = isvalid,
                                        meta_islatest = islatest,
                                        meta_being_evicted = being_evicted)
                                self.client.update_vallen_tbl_table_add_with_set_vallen(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: update_vallo1_tbl (default: nop; ?)
            print "Configuring update_vallo1_tbl"
            configure_update_val_tbl("lo1")

            # Table: update_valhi1_tbl (default: nop; ?)
            print "Configuring update_valhi1_tbl"
            configure_update_val_tbl("hi1")

            ### Stage 3-10 ###

            ### Stage 11 ###

            # Table: port_forward_tbl (default:nop; ?)
            print "Configuring port_forward_tbl"
            for iscached in cached_list:
                for isvalid in  valid_list:
                    for islatest in latest_list:
                        for iszerovote in predicate_list:
                            for islock in lock_list:
                                for being_evicted in being_evicted_list:
                                    matchspec0 = netbufferv2_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = GETREQ_TYPE,
                                            meta_iscached = iscached,
                                            meta_isvalid = isvalid,
                                            meta_islatest = islatest,
                                            meta_iszerovote = iszerovote,
                                            meta_islock = islock,
                                            meta_being_evicted = being_evicted)
                                    if iscached == 1 and isvalid == 1 and islatest == 1 and being_evicted == 0:
                                        self.client.port_forward_tbl_table_add_with_update_getreq_to_getres(\
                                            self.sess_hdl, self.dev_tgt, matchspec0) # Change GETREQ to GETRES -> client
                                    else if iscached == 1 and isvalid == 1 and islatest == 2 and being_evicted == 0:
                                        self.client.port_forward_tbl_table_add_with_update_getreq_to_getres_deleted(\
                                            self.sess_hdl, self.dev_tgt, matchspec0) # Change GETREQ to GETRES (deleted) -> client
                                    else if iscached == 1 and isvalid == 1 and islatest == 0 and being_evicted == 0:
                                        actnspec0 = netbufferv2_update_getreq_to_getreq_nlatest_action_spec_t(\
                                                self.devPorts[1]) # Forward GETREQ_NLATEST to server
                                        self.client.port_forward_tbl_table_add_with_update_getreq_to_getreq_nlatest(\
                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0) 
                                    else if (isvalid == 0 and islock == 0 and being_evicted == 0) or \
                                            (iszerovote == 2 and islock == 0 and being_evicted == 0):
                                        actnspec0 = netbufferv2_update_getreq_to_getreq_pop_action_spec_t(\
                                                self.devPorts[1]) # Forward GETREQ_POP to server
                                        self.client.port_forward_tbl_table_add_with_update_getreq_to_getreq_pop(\
                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                    else:
                                        actnspec0 = netbufferv2_port_forward_action_spec_t(\
                                                self.devPorts[1]) # Forward GETREQ to server
                                        self.client.port_forward_tbl_table_add_with_port_forward(\
                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                    matchspec0 = netbufferv2_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = GETRES_TYPE,
                                            meta_iscached = iscached,
                                            meta_isvalid = isvalid,
                                            meta_islatest = islatest,
                                            meta_iszerovote = iszerovote,
                                            meta_islock = islock,
                                            meta_being_evicted = being_evicted)
                                    actnspec0 = netbufferv2_port_forward_action_spec_t(\
                                            self.devPorts[0]) # Forward GETRES to client
                                    self.client.port_forward_tbl_table_add_with_port_forward(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                    matchspec0 = netbufferv2_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = GETRES_NPOP_TYPE,
                                            meta_iscached = iscached,
                                            meta_isvalid = isvalid,
                                            meta_islatest = islatest,
                                            meta_iszerovote = iszerovote,
                                            meta_islock = islock,
                                            meta_being_evicted = being_evicted)
                                    actnspec0 = netbufferv2_update_getres_npop_to_getres_action_spec_t(\
                                            self.devPorts[0]) # Change GETRES_NPOP to GETRES -> client
                                    self.client.port_forward_tbl_table_add_with_getres_npop_to_getres(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                    matchspec0 = netbufferv2_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = GETRES_LATEST_TYPE,
                                            meta_iscached = iscached,
                                            meta_isvalid = isvalid,
                                            meta_islatest = islatest,
                                            meta_iszerovote = iszerovote,
                                            meta_islock = islock,
                                            meta_being_evicted = being_evicted)
                                    actnspec0 = netbufferv2_update_getres_latest_to_getres_action_spec_t(\
                                            self.devPorts[0]) # Change GETRES_LATEST to GETRES -> client
                                    self.client.port_forward_tbl_table_add_with_getres_npop_to_getres(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                    matchspec0 = netbufferv2_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = GETRES_NEXIST_TYPE,
                                            meta_iscached = iscached,
                                            meta_isvalid = isvalid,
                                            meta_islatest = islatest,
                                            meta_iszerovote = iszerovote,
                                            meta_islock = islock,
                                            meta_being_evicted = being_evicted)
                                    actnspec0 = netbufferv2_update_getres_nexist_to_getres_action_spec_t(\
                                            self.devPorts[0]) # Change GETRES_NEXIST to GETRES -> client
                                    self.client.port_forward_tbl_table_add_with_getres_npop_to_getres(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            ### Egress ###

            # Table: hash_partition_tbl (default: nop; server_num <= 128)
            print "Configuring hash_partition_tbl"
            hash_start = 0
            hash_range_per_server = bucket_num / server_num
            for i in range(server_num):
                if i == server_num - 1:
                    hash_end = bucket_num - 1 # if end is not included, then it is just processed by port 1111
                else:
                    hash_end = hash_start + hash_range_per_server
                matchspec0 = netbufferv2_hash_partition_tbl_match_spec_t(\
                        udp_hdr_dstPort=server_port, \
                        eg_intr_md_egress_port=self.devPorts[1], \
                        meta_hashidx_start = hash_start, \
                        meta_hashidx_end = hash_end)
                actnspec0 = netbufferv2_update_dstport_action_spec_t(\
                        server_port + i)
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
                matchspec0 = netbufferv2_hash_partition_reverse_tbl_match_spec_t(\
                        udp_hdr_srcPort=server_port, \
                        eg_intr_md_egress_port=self.devPorts[1], \
                        meta_hashidx_start = hash_start, \
                        meta_hashidx_end = hash_end)
                actnspec0 = netbufferv2_update_dstport_reverse_action_spec_t(\
                        server_port + i)
                self.client.hash_partition_reverse_tbl_table_add_with_update_dstport_reverse(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                hash_start = hash_end

            # Table: drop_put_tbl (if key coherence is unnecessary)
            #print "Configuring drop_put_tbl"
            #matchspec0 = netbuffer_drop_put_tbl_match_spec_t(
            #        op_hdr_optype = PUTREQ_TYPE)
            #self.client.drop_put_tbl_table_add_with_ig_drop_unicast(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)

            # TMPDEBUG
            #print "Configuring forward_to_server_tbl"
            #matchspec0 = netbuffer_forward_to_server_tbl_match_spec_t(\
            #        op_hdr_optype=PUTREQ_TYPE,
            #        meta_islock=1)
            #matchspec1 = netbuffer_forward_to_server_tbl_match_spec_t(\
            #        op_hdr_optype=GETREQ_TYPE,
            #        meta_islock=1)
            #actnspec0 = netbuffer_forward_to_server_action_spec_t(\
            #        self.devPorts[1])
            #self.client.forward_to_server_tbl_table_add_with_forward_to_server(\
            #        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            #self.client.forward_to_server_tbl_table_add_with_forward_to_server(\
            #        self.sess_hdl, self.dev_tgt, matchspec1, actnspec0)

            # Table: update_macaddr_tbl (default: nop; 5)
            print "Configuring update_macaddr_tbl"
            actnspec0 = netbufferv2_update_macaddr_s2c_action_spec_t(\
                    macAddr_to_string(src_mac), \
                    macAddr_to_string(dst_mac))
            actnspec1 = netbufferv2_update_macaddr_c2s_action_spec_t(\
                    macAddr_to_string(src_mac), \
                    macAddr_to_string(dst_mac))
            matchspec0 = netbufferv2_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_TYPE)
            matchspec1 = netbufferv2_update_macaddr_tbl_match_spec_t(op_hdr_optype=PUTRES_TYPE)
            matchspec2 = netbufferv2_update_macaddr_tbl_match_spec_t(op_hdr_optype=DELRES_TYPE)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_s2c(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_s2c(\
                    self.sess_hdl, self.dev_tgt, matchspec1, actnspec0)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_s2c(\
                    self.sess_hdl, self.dev_tgt, matchspec2, actnspec0)
            matchspec3 = netbufferv2_update_macaddr_tbl_match_spec_t(op_hdr_optype=PUTREQ_GS_TYPE)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_c2s(\
                    self.sess_hdl, self.dev_tgt, matchspec3, actnspec1)
            matchspec4 = netbufferv2_update_macaddr_tbl_match_spec_t(op_hdr_optype=PUTREQ_GS_CASE2_TYPE)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_c2s(\
                    self.sess_hdl, self.dev_tgt, matchspec4, actnspec1)

            self.conn_mgr.complete_operations(self.sess_hdl)
            self.conn_mgr.client_cleanup(self.sess_hdl)

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
