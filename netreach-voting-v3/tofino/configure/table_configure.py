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

from netbufferv3.p4_pd_rpc.ttypes import *
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
GETRES_POP_TYPE = 0x09
GETRES_NPOP_TYPE = 0x0a
GETRES_POP_EVICT_TYPE = 0x0b
PUTREQ_POP_TYPE = 0x0c
PUTREQ_RECIR_TYPE = 0x0d
PUTREQ_POP_EVICT_TYPE = 0x0e
DELREQ_RECIR_TYPE = 0x0f

valid_list = [0, 1]
keymatch_list = [0, 1]
lock_list = [0, 1]
predicate_list = [1, 2]
backup_list = [0, 1]
case1_list = [0, 1]
case2_list = [0, 1]
case3_list = [0, 1]
assigned_list = [0, 1]

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
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbufferv3"])

    def configure_access_key_tbl(self, keyname):
        # 5
        for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE, PUTREQ_RECIR_TYPE, DELREQ_TYPE, DELREQ_RECIR_TYPE]:
            matchspec0 = eval("netbufferv3_access_key{}_tbl_match_spec_t".format(keyname))(\
                    op_hdr_optype=tmpoptype)
            eval("self.client.access_key{}_tbl_table_add_with_match_key{}".format(keyname, keyname))(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
        matchspec0 = eval("netbufferv3_access_key{}_tbl_match_spec_t".format(keyname))(\
                op_hdr_optype=GETRES_POP_TYPE)
        eval("self.client.access_key{}_tbl_table_add_with_set_and_get_key{}".format(keyname, keyname))(\
                self.sess_hdl, self.dev_tgt, matchspec0)
        matchspec0 = eval("netbufferv3_access_key{}_tbl_match_spec_t".format(keyname))(\
                op_hdr_optype=PUTREQ_POP_TYPE)
        eval("self.client.access_key{}_tbl_table_add_with_set_and_get_key{}".format(keyname, keyname))(\
                self.sess_hdl, self.dev_tgt, matchspec0)

    def configure_update_val_tbl(self, valname):
        # 1028
        for canput in predicate_list:
            matchspec0 = eval("netbufferv3_update_val{}_tbl_match_spec_t".format(valname))(
                    op_hdr_optype=GETREQ_TYPE,
                    meta_canput=canput,
                    meta_iskeymatch=1)
            eval("self.client.update_val{}_tbl_table_add_with_get_val{}".format(valname, valname))(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
        for tmpoptype in [PUTREQ_TYPE, PUTREQ_RECIR_TYPE, DELREQ_TYPE, DELREQ_RECIR_TYPE]:
            matchspec1 = eval("netbufferv3_update_val{}_tbl_match_spec_t".format(valname))(
                    op_hdr_optype=tmpoptype, 
                    meta_canput=2,
                    meta_iskeymatch=1)
            if tmpoptype == PUTREQ_TYPE or tmpoptype == PUTREQ_RECIR_TYPE:
                eval("self.client.update_val{}_tbl_table_add_with_set_and_get_val{}".format(valname, valname))(\
                        self.sess_hdl, self.dev_tgt, matchspec1)
            else: # DELREQ gets value for CASE1
                eval("self.client.update_val{}_tbl_table_add_with_get_val{}".format(valname, valname))(\
                        self.sess_hdl, self.dev_tgt, matchspec1)
        for iskeymatch in keymatch_list:
            for canput in predicate_list:
                for tmpoptype in [GETRES_POP_TYPE, PUTREQ_POP_TYPE]:
                    matchspec0 = eval("netbufferv3_update_val{}_tbl_match_spec_t".format(valname))(\
                            op_hdr_optype= tmpoptype,
                            meta_canput=canput,
                            meta_iskeymatch=iskeymatch)
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

            # Stage 0

            # Table: access_keylolo_tbl (default: nop; 5)
            print "Configuring match_keylolo_tbl"
            self.configure_access_key_tbl("lolo")

            # Table: access_keylohi_tbl (default: nop; 5)
            print "Configuring match_keylohi_tbl"
            self.configure_access_key_tbl("lohi")

            # Table: access_keyhilo_tbl (default: nop; 5)
            print "Configuring match_keyhilo_tbl"
            self.configure_access_key_tbl("hilo")

            # Table: access_keyhihi_tbl (default: nop; 5)
            print "Configuring match_keyhihi_tbl"
            self.configure_access_key_tbl("hihi")

            # Stage 1

            # Table: access_valid_tbl (default: nop; 1280)
            print "Configuring access_valid_tbl"
            for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE, PUTREQ_RECIR_TYPE, DELREQ_TYPE, DELREQ_RECIR_TYPE]:
                matchspec0 = netbufferv3_access_valid_tbl_match_spec_t(
                        op_hdr_optype=tmpoptype)
                self.client.access_valid_tbl_table_add_with_get_valid(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
            # NOTE: GETRES_POP/PUTREQ_U_S triggers set_and_get_key in access_key_tbl which does not 
            # set ismatch_key but set op_hdr.key 
            for tmpoptype in [GETRES_POP_TYPE, PUTREQ_POP_TYPE]:
                matchspec0 = netbufferv3_access_valid_tbl_match_spec_t(
                        op_hdr_optype=tmpoptype)
                self.client.access_valid_tbl_table_add_with_set_valid(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_vote_tbl (default: nop; 2049)
            print "Configuring access_vote_tbl"
            for ismatch_keylolo in predicate_list:
                for ismatch_keylohi in predicate_list:
                    for ismatch_keyhilo in predicate_list:
                        for ismatch_keyhihi in predicate_list:
                            for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE, PUTREQ_RECIR_TYPE]:
                                matchspec0 = netbufferv3_access_vote_tbl_match_spec_t(
                                        op_hdr_optype=tmpoptype, 
                                        meta_ismatch_keylolo=ismatch_keylolo, 
                                        meta_ismatch_keylohi=ismatch_keylohi, 
                                        meta_ismatch_keyhilo=ismatch_keyhilo, 
                                        meta_ismatch_keyhihi=ismatch_keyhihi)
                                if (ismatch_keylolo == 2 and ismatch_keylohi == 2 and \
                                ismatch_keyhilo == 2 and ismatch_keyhihi == 2)
                                    self.client.access_vote_tbl_table_add_with_increase_vote(
                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                else:
                                    self.client.access_vote_tbl_table_add_with_decrease_vote(
                                            self.sess_hdl, self.dev_tgt, matchspec0)
                            for tmpoptype in [GETRES_POP_TYPE, PUTREQ_POP_TYPE]:
                                matchspec0 = netbufferv3_access_vote_tbl_match_spec_t(
                                        op_hdr_optype=tmpoptype,
                                        meta_ismatch_keylolo=ismatch_keylolo, 
                                        meta_ismatch_keylohi=ismatch_keylohi, 
                                        meta_ismatch_keyhilo=ismatch_keyhilo, 
                                        meta_ismatch_keyhihi=ismatch_keyhihi)
                                self.client.access_vote_tbl_table_add_with_init_vote(
                                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table assign_seq_tbl (default: nop; 1)
            # NOTE: PUTREQ_RECIR does not need to assign seq
            print "Configuring assign_seq_tbl"
            for tmpoptype in [PUTREQ_TYPE, DELREQ_TYPE]:
                matchspec0 = netbufferv3_assign_seq_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype)
                self.client.assign_seq_tbl_table_add_with_assign_seq(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table update_iskeymatch_tbl (default: update_iskeymatch(0); 1)
            print "Configuring update_iskeymatch_tbl"
            matchspec0 = netbufferv3_update_iskeymatch_tbl_match_spec_t(\
                    meta_ismatch_keylolo=2,
                    meta_ismatch_keylohi=2,
                    meta_ismatch_keyhilo=2,
                    meta_ismatch_keyhihi=2)
            actnspec0 = netbufferv3_update_iskeymatch_action_spec_t(1)
            self.client.update_iskeymatch_tbl_table_add_with_update_iskeymatch(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Stage 2

            # Table: access_savedseq_tbl (default: nop; 1026)
            print "Configuring access_savedseq_tbl"
            for tmpoptype in [PUTREQ_TYPE, PUTREQ_RECIR_TYPE, DELREQ_TYPE, DELREQ_RECIR_TYPE]:
                matchspec0 = netbufferv3_access_savedseq_tbl_match_spec_t(\
                        op_hdr_optype = tmpoptype,
                        meta_isvalid = 1,
                        meta_iskeymatch = 1)
                self.client.access_savedseq_tbl_table_add_with_try_update_savedseq(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
            for isvalid in valid_list:
                for iskeymatch in  keymatch_list:
                    for tmpoptype in [GETRES_POP_TYPE, PUTREQ_POP_TYPE]:
                        matchspec0 = netbufferv3_access_savedseq_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                meta_isvalid = isvalid,
                                meta_iskeymatch = iskeymatch)
                        self.client.access_savedseq_tbl_table_add_with_reset_savedseq(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_lock_tbl (default: nop; 12)
            print "Configuring access_lock_tbl"
            for isvalid in valid_list:
                for zerovote in predicate_list:
                    for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE, PUTREQ_RECIR_TYPE]:
                        matchspec0 = netbufferv3_access_lock_tbl_match_spec_t(
                                op_hdr_optype=tmpoptype,
                                meta_isvalid=isvalid,
                                meta_zerovote=zerovote)
                        if isvalid == 0 or zerovote == 2:
                            self.client.access_lock_tbl_table_add_with_try_lock(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                        else:
                            self.client.access_lock_tbl_table_add_with_read_lock(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)
                    matchspec0 = netbufferv3_access_lock_tbl_match_spec_t(
                            op_hdr_optype=DELREQ_TYPE,
                            meta_zerovote=zerovote)
                    self.client.access_lock_tbl_table_add_with_read_lock(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
            for isvalid in valid_list:
                for zerovote in predicate_list:
                    for tmpoptype in [GETRES_POP_TYPE, GETRES_NPOP_TYPE, PUTREQ_POP_TYPE]:
                        matchspec0 = netbufferv3_access_lock_tbl_match_spec_t(
                                op_hdr_optype=tmpoptype,
                                meta_isvalid=isvalid,
                                meta_zerovote=zerovote)
                        self.client.access_lock_tbl_table_add_with_reset_lock(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

            # Start from stage 3

            # Table: update_vallen_tbl (default: nop; 2560)
            for canput in predicate_list:
                matchspec0 = netbufferv3_update_vallen_tbl_match_spec_t(\
                        op_hdr_optype=GETREQ_TYPE,
                        meta_canput=canput,
                        meta_iskeymatch=1)
                self.client.update_vallen_tbl_table_add_with_get_vallen(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
            for tmpoptype in [PUTREQ_TYPE, PUTREQ_RECIR_TYPE, DELREQ_TYPE, DELREQ_RECIR_TYPE]:
                matchspec0 = netbufferv3_update_vallen_tbl_match_spec_t(\
                        op_hdr_optype=tmpoptype, 
                        meta_canput=2, # canput means valid=1, iskeymatch=1, and seq>savedseq
                        meta_iskeymatch=1)
                if (tmpoptype == PUTREQ_TYPE or tmpoptype == PUTREQ_RECIR_TYPE):
                    self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
                else: # DELREQ resets and gets vallen for CASE1
                    self.client.update_vallen_tbl_table_add_with_reset_and_get_vallen(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
            for iskeymatch in keymatch_list:
                for canput in predicate_list:
                    for tmpoptype in [GETRES_POP_TYPE, PUTREQ_POP_TYPE]:
                        matchspec0 = netbufferv3_update_vallen_tbl_match_spec_t(\
                                op_hdr_optype=tmpoptype, 
                                meta_canput=canput,
                                meta_iskeymatch=iskeymatch)
                        self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: update_vallo1_tbl (default: nop; 1028)
            print "Configuring update_vallo1_tbl"
            self.configure_update_val_tbl("lo1")

            #print "Configuring update_vallo2_tbl"
            #matchspec0 = netbufferv3_update_vallo2_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo2_tbl_table_add_with_put_vallo2(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo2_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo2_tbl_table_add_with_get_vallo2(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo3_tbl"
            #matchspec0 = netbufferv3_update_vallo3_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo3_tbl_table_add_with_put_vallo3(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo3_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo3_tbl_table_add_with_get_vallo3(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo4_tbl"
            #matchspec0 = netbufferv3_update_vallo4_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo4_tbl_table_add_with_put_vallo4(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo4_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo4_tbl_table_add_with_get_vallo4(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo5_tbl"
            #matchspec0 = netbufferv3_update_vallo5_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo5_tbl_table_add_with_put_vallo5(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo5_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo5_tbl_table_add_with_get_vallo5(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo6_tbl"
            #matchspec0 = netbufferv3_update_vallo6_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo6_tbl_table_add_with_put_vallo6(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo6_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo6_tbl_table_add_with_get_vallo6(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo7_tbl"
            #matchspec0 = netbufferv3_update_vallo7_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo7_tbl_table_add_with_put_vallo7(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo7_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo7_tbl_table_add_with_get_vallo7(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo8_tbl"
            #matchspec0 = netbufferv3_update_vallo8_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo8_tbl_table_add_with_put_vallo8(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo8_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo8_tbl_table_add_with_get_vallo8(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo9_tbl"
            #matchspec0 = netbufferv3_update_vallo9_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo9_tbl_table_add_with_put_vallo9(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo9_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo9_tbl_table_add_with_get_vallo9(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo10_tbl"
            #matchspec0 = netbufferv3_update_vallo10_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo10_tbl_table_add_with_put_vallo10(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo10_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo10_tbl_table_add_with_get_vallo10(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo11_tbl"
            #matchspec0 = netbufferv3_update_vallo11_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo11_tbl_table_add_with_put_vallo11(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo11_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo11_tbl_table_add_with_get_vallo11(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo12_tbl"
            #matchspec0 = netbufferv3_update_vallo12_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo12_tbl_table_add_with_put_vallo12(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo12_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo12_tbl_table_add_with_get_vallo12(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo13_tbl"
            #matchspec0 = netbufferv3_update_vallo13_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo13_tbl_table_add_with_put_vallo13(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo13_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo13_tbl_table_add_with_get_vallo13(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo14_tbl"
            #matchspec0 = netbufferv3_update_vallo14_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo14_tbl_table_add_with_put_vallo14(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo14_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo14_tbl_table_add_with_get_vallo14(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo15_tbl"
            #matchspec0 = netbufferv3_update_vallo15_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo15_tbl_table_add_with_put_vallo15(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo15_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo15_tbl_table_add_with_get_vallo15(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo16_tbl"
            #matchspec0 = netbufferv3_update_vallo16_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo16_tbl_table_add_with_put_vallo8(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_vallo16_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo16_tbl_table_add_with_get_vallo8(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            # Table: update_valhi1_tbl (default: nop; 1028)
            print "Configuring update_valhi1_tbl"
            self.configure_update_val_tbl("hi1")

            #print "Configuring update_valhi2_tbl"
            #matchspec0 = netbufferv3_update_valhi2_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi2_tbl_table_add_with_put_valhi2(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi2_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi2_tbl_table_add_with_get_valhi2(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi3_tbl"
            #matchspec0 = netbufferv3_update_valhi3_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi3_tbl_table_add_with_put_valhi3(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi3_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi3_tbl_table_add_with_get_valhi3(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi4_tbl"
            #matchspec0 = netbufferv3_update_valhi4_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi4_tbl_table_add_with_put_valhi4(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi4_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi4_tbl_table_add_with_get_valhi4(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi5_tbl"
            #matchspec0 = netbufferv3_update_valhi5_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi5_tbl_table_add_with_put_valhi5(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi5_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi5_tbl_table_add_with_get_valhi5(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi6_tbl"
            #matchspec0 = netbufferv3_update_valhi6_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi6_tbl_table_add_with_put_valhi6(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi6_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi6_tbl_table_add_with_get_valhi6(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi7_tbl"
            #matchspec0 = netbufferv3_update_valhi7_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi7_tbl_table_add_with_put_valhi7(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi7_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi7_tbl_table_add_with_get_valhi7(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi8_tbl"
            #matchspec0 = netbufferv3_update_valhi8_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi8_tbl_table_add_with_put_valhi8(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi8_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi8_tbl_table_add_with_get_valhi8(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi9_tbl"
            #matchspec0 = netbufferv3_update_valhi9_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi9_tbl_table_add_with_put_valhi9(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi9_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi9_tbl_table_add_with_get_valhi9(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi10_tbl"
            #matchspec0 = netbufferv3_update_valhi10_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi10_tbl_table_add_with_put_valhi10(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi10_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi10_tbl_table_add_with_get_valhi10(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi11_tbl"
            #matchspec0 = netbufferv3_update_valhi11_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi11_tbl_table_add_with_put_valhi11(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi11_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi11_tbl_table_add_with_get_valhi11(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi12_tbl"
            #matchspec0 = netbufferv3_update_valhi12_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi12_tbl_table_add_with_put_valhi12(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi12_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi12_tbl_table_add_with_get_valhi12(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi13_tbl"
            #matchspec0 = netbufferv3_update_valhi13_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi13_tbl_table_add_with_put_valhi13(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi13_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi13_tbl_table_add_with_get_valhi13(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi14_tbl"
            #matchspec0 = netbufferv3_update_valhi14_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi14_tbl_table_add_with_put_valhi14(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi14_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi14_tbl_table_add_with_get_valhi14(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi15_tbl"
            #matchspec0 = netbufferv3_update_valhi15_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi15_tbl_table_add_with_put_valhi15(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi15_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi15_tbl_table_add_with_get_valhi15(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi16_tbl"
            #matchspec0 = netbufferv3_update_valhi16_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi16_tbl_table_add_with_put_valhi8(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbufferv3_update_valhi16_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi16_tbl_table_add_with_get_valhi8(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            # Stage 4 + n

            # Table: access_case1_tbl (default: nop; 2048)
            print "Configuring access_case1_tbl"
            for ismatch_keylolo in predicate_list:
                for ismatch_keylohi in predicate_list:
                    for ismatch_keyhilo in predicate_list:
                        for ismatch_keyhihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            for isvalid in valid_list:
                                                for isbackup in backup_list:
                                                    for tmpoptype in [PUTREQ_TYPE, DELREQ_TYPE]:
                                                        matchspec0 = netbufferv3_access_case1_tbl_match_spec_t(
                                                                op_hdr_optype=tmpoptype, 
                                                                meta_isvalid = isvalid,
                                                                meta_ismatch_keylolo=ismatch_keylolo, 
                                                                meta_ismatch_keylohi=ismatch_keylohi, 
                                                                meta_ismatch_keyhilo=ismatch_keyhilo, 
                                                                meta_ismatch_keyhihi=ismatch_keyhihi, 
                                                                meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                                meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                                meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                                meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                                meta_isbackup=isbackup)
                                                        if (ismatch_keylolo == 2 and ismatch_keylohi == 2 and \
                                                        ismatch_keyhilo == 2 and ismatch_keyhihi == 2 and \
                                                        ismatch_keyhilolo == 2 and ismatch_keyhilohi == 2 and \
                                                        ismatch_keyhihilo == 2 and ismatch_keyhihihi == 2 and \
                                                        isvalid == 1 and isbackup == 1):
                                                            self.client.access_case1_tbl_table_add_with_try_case1(\
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                                        else:
                                                            self.client.access_case1_tbl_table_add_with_read_case1(
                                                                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_case2_tbl (default: nop; 4)
            print "Configuring access_case2_tbl"
            for isbackup in backup_list:
                for tmpoptype in [GETRES_S_TYPE, PUTREQ_RU_TYPE]:
                    matchspec0 = netbufferv3_access_case2_tbl_match_spec_t(\
                            op_hdr_optype=tmpoptype,
                            meta_isbackup=isbackup)
                    if isbackup == 1:
                        self.client.access_case2_tbl_table_add_with_try_case2(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                    else:
                        self.client.access_case2_tbl_table_add_with_read_case2(\
                                self.sess_hdl, self.dev_tgt, matchspec0)

            # Stage 11

            # Table: port_forward_tbl (default: nop; 544)
            print "Configuring port_forward_tbl"
            for isvalid in valid_list:
                for zerovote in predicate_list:
                    for iskeymatch in keymatch_list:
                        for islock in lock_list:
                            matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = GETREQ_TYPE,
                                    meta_isvalid = isvalid,
                                    meta_zerovote = zerovote,
                                    meta_iskeymatch = iskeymatch,
                                    meta_islock = islock)
                            if islock == 0 and (isvalid == 0 or zerovote == 2):
                                # Update GETREQ as GETREQ_POP to server 
                                actnspec0 = netbufferv3_update_getreq_to_getreq_pop_action_spec_t(self.devPorts[1])
                                self.client.port_forward_tbl_table_add_with_update_getreq_to_getreq_pop(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            elif isvalid == 1 and iskeymatch == 1:
                                # Sendback GETRES to client
                                self.client.port_forward_tbl_table_add_with_update_getreq_to_getres(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            elif islock == 1 and (isvalid == 0 or iskeymatch == 0):
                                # Use recirculate port 64 + pipe ID of ingress port
                                actnspec0 = netbufferv3_recirculate_getreq_action_spec_t(self.recirPorts[0]) 
                                self.client.port_forward_tbl_table_add_with_recirculate_getreq(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            else:
                                # Forwrad GETREQ to server 
                                actnspec0 = netbufferv3_port_forward_action_spec_t(self.devPorts[1]) 
                                self.client.port_forward_tbl_table_add_with_port_forward(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = GETRES_POP_TYPE,
                                    meta_isvalid = isvalid,
                                    meta_zerovote = zerovote,
                                    meta_iskeymatch = iskeymatch,
                                    meta_islock = islock)
                            if isvalid == 0: 
                                # Drop GETRES_POP with old kv, clone original pkt with new kv to client port for GETRES to client
                                actnspec0 = netbufferv3_drop_getres_pop_clone_for_getres_action_spec_t(\
                                        sids[0])
                                self.client.port_forward_tbl_table_add_with_drop_getres_pop_clone_for_getres(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            elif isvalid == 1:
                                # Update GETRES_POP as GETRES_POP_EVICT to server, clone original pkt with new kv to client port for GETRES to client
                                actnspec0 = netbufferv3_update_getres_pop_to_evict_clone_for_getres_action_spec_t(\
                                        sids[0])
                                self.client.port_forward_tbl_table_add_with_update_getres_pop_to_evict_clone_for_getres(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            # Update GETRES_NPOP as GETRES to client
                            matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = GETRES_NPOP_TYPE,
                                    meta_isvalid = isvalid,
                                    meta_zerovote = zerovote,
                                    meta_iskeymatch = iskeymatch,
                                    meta_islock = islock)
                            actnspec0 = netbufferv3_update_getres_npop_to_getres_action_spec_t(\
                                    self.devPorts[0])
                            self.client.port_forward_tbl_table_add_with_update_getres_npop_to_getres(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = PUTREQ_TYPE,
                                    meta_isvalid = isvalid,
                                    meta_zerovote = zerovote,
                                    meta_iskeymatch = iskeymatch,
                                    meta_islock = islock)
                            if islock == 0 and (isvalid == 0 or zerovote == 0):
                                # Use recirculate port 64 + pipe ID of ingress port
                                actnspec0 = netbufferv3_update_putreq_to_putreq_pop_action_spec_t(self.recirPorts[0]) 
                                self.client.port_forward_tbl_table_add_with_update_putreq_to_putreq_pop(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            elif isvalid == 1 and iskeymatch == 1:
                                # Sendback PUTRES to client
                                self.client.port_forward_tbl_table_add_with_update_putreq_to_putres(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                # Use recirculate port 64 + pipe ID of ingress port
                                actnspec0 = netbufferv3_update_putreq_to_putreq_recir_action_spec_t(self.recirPorts[0]) 
                                self.client.port_forward_tbl_table_add_with_update_putreq_to_putreq_recir(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            else:
                                # Forwrad PUTREQ to server 
                                actnspec0 = netbufferv3_port_forward_action_spec_t(self.devPorts[1]) 
                                self.client.port_forward_tbl_table_add_with_port_forward(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = PUTREQ_POP_TYPE,
                                    meta_isvalid = isvalid,
                                    meta_zerovote = zerovote,
                                    meta_iskeymatch = iskeymatch,
                                    meta_islock = islock)
                            if isvalid == 0: 
                                # Drop PUTREQ_POP with old kv, clone original pkt with new kv to client port for PUTRES to client
                                actnspec0 = netbufferv3_drop_putreq_pop_clone_for_putres_action_spec_t(\
                                        sids[0])
                                self.client.port_forward_tbl_table_add_with_drop_putreq_pop_clone_for_putres(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            elif isvalid == 1:
                                # Update PUTREQ_POP as PUTREQ_POP_EVICT to server, clone original pkt with new kv to client port for PUTRES to client
                                actnspec0 = netbufferv3_update_putreq_pop_to_evict_clone_for_putres_action_spec_t(\
                                        sids[0], self.devPorts[1])
                                self.client.port_forward_tbl_table_add_with_update_putreq_pop_to_evict_clone_for_putres(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = PUTREQ_RECIR_TYPE,
                                    meta_isvalid = isvalid,
                                    meta_zerovote = zerovote,
                                    meta_iskeymatch = iskeymatch,
                                    meta_islock = islock)
                            if islock == 0 and (isvalid == 0 or zerovote == 0):
                                # Use recirculate port 64 + pipe ID of ingress port
                                actnspec0 = netbufferv3_update_putreq_recir_to_putreq_pop_action_spec_t(self.recirPorts[0])     
                                self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_putreq_pop(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            elif isvalid == 1 and iskeymatch == 1:
                                # Sendback PUTRES to client
                                self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_putres(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                # Use recirculate port 64 + pipe ID of ingress port
                                actnspec0 = netbufferv3_recirculate_putreq_recir_action_spec_t(self.recirPorts[0]) 
                                self.client.port_forward_tbl_table_add_with_recirculate_putreq_recir(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            else:
                                # Forwrad PUTREQ to server 
                                actnspec0 = netbufferv3_update_putreq_recir_to_putreq_action_spec_t(self.devPorts[1]) 
                                self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_putreq(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = DELREQ_TYPE,
                                    meta_isvalid = isvalid,
                                    meta_zerovote = zerovote,
                                    meta_iskeymatch = iskeymatch,
                                    meta_islock = islock)
                            if isvalid == 1 and iskeymatch == 1:
                                # Sendback DELRES to client
                                self.client.port_forward_tbl_table_add_with_update_delreq_to_delres(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                # Use recirculate port 64 + pipe ID of ingress port
                                actnspec0 = netbufferv3_update_delreq_to_delreq_recir_action_spec_t(self.recirPorts[0]) 
                                self.client.port_forward_tbl_table_add_with_update_delreq_to_delreq_recir(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            else:
                                # Forwrad DELREQ to server 
                                actnspec0 = netbufferv3_port_forward_action_spec_t(self.devPorts[1]) 
                                self.client.port_forward_tbl_table_add_with_port_forward(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = DELREQ_RECIR_TYPE,
                                    meta_isvalid = isvalid,
                                    meta_zerovote = zerovote,
                                    meta_iskeymatch = iskeymatch,
                                    meta_islock = islock)
                            if isvalid == 1 and iskeymatch == 1:
                                # Sendback DELRES to client
                                self.client.port_forward_tbl_table_add_with_update_delreq_recir_to_delres(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                # Use recirculate port 64 + pipe ID of ingress port
                                actnspec0 = netbufferv3_recirculate_delreq_recir_action_spec_t(self.recirPorts[0]) 
                                self.client.port_forward_tbl_table_add_with_recirculate_delreq_recir(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            else:
                                # Forwrad DELREQ to server 
                                actnspec0 = netbufferv3_update_delreq_recir_to_delreq_action_spec_t(self.devPorts[1]) 
                                self.client.port_forward_tbl_table_add_with_update_delreq_recir_to_delreq(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)


                            # Deprecated
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for isbackup in backup_list:
                        for iscase3 in case3_list:
                            for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE, DELREQ_TYPE]:
                                matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                        op_hdr_optype = tmpoptype,
                                        meta_isvalid = isvalid,
                                        meta_isdirty = isdirty,
                                        meta_islock = 0,
                                        meta_isbackup = isbackup,
                                        meta_iscase3 = iscase3)
                                if tmpoptype == PUTREQ_TYPE and isbackup == 1 and iscase3 == 0:
                                    actnspec0 = netbufferv3_update_putreq_to_case3_action_spec_t(\
                                            self.devPorts[1]) # Output to server
                                    self.client.port_forward_tbl_table_add_with_update_putreq_to_case3(\
                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                elif tmpoptype == DELREQ_TYPE and isbackup == 1 and iscase3 == 0:
                                    actnspec0 = netbufferv3_update_delreq_to_case3_action_spec_t(\
                                            self.devPorts[1]) # Output to server
                                    self.client.port_forward_tbl_table_add_with_update_delreq_to_case3(\
                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                else:
                                    actnspec0 = netbufferv3_port_forward_action_spec_t(\
                                            self.devPorts[1]) # Output to server
                                    self.client.port_forward_tbl_table_add_with_port_forward(\
                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for islock in lock_list:
                        for isbackup in backup_list:
                            for iscase3 in case3_list:
                                for tmpoptype in [SCANREQ_TYPE, GETREQ_S_TYPE, DELREQ_S_TYPE]:
                                    matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = tmpoptype,
                                            meta_isvalid = isvalid,
                                            meta_isdirty = isdirty,
                                            meta_islock = islock,
                                            meta_isbackup = isbackup,
                                            meta_iscase3 = iscase3)
                                    actnspec0 = netbufferv3_port_forward_action_spec_t(\
                                            self.devPorts[1]) # Output to server
                                    self.client.port_forward_tbl_table_add_with_port_forward(\
                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for islock in lock_list:
                        for isbackup in backup_list:
                            for iscase3 in case3_list:
                                for tmpoptype in [GETRES_TYPE, PUTRES_TYPE, DELRES_TYPE, SCANRES_TYPE]:
                                    matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                            op_hdr_optype = tmpoptype,
                                            meta_isvalid = isvalid,
                                            meta_isdirty = isdirty,
                                            meta_islock = islock,
                                            meta_isbackup = isbackup,
                                            meta_iscase3 = iscase3)
                                    actnspec0 = netbufferv3_port_forward_action_spec_t(\
                                            self.devPorts[0]) # Output to client
                                    self.client.port_forward_tbl_table_add_with_port_forward(\
                                            self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for isdirty in dirty_list:
                for islock in lock_list:
                    for isbackup in backup_list:
                        for iscase3 in case3_list:
                            matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = PUTREQ_CASE1_TYPE,
                                    meta_isvalid = 1,
                                    meta_isdirty = isdirty,
                                    meta_islock = islock,
                                    meta_isbackup = isbackup,
                                    meta_iscase3 = iscase3)
                            actnspec0 = netbufferv3_forward_putreq_case1_and_clone_action_spec_t(\
                                    sids[0], self.devPorts[1]) # Clone to client, forward to server
                            self.client.port_forward_tbl_table_add_with_forward_putreq_case1_and_clone(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                            matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = DELREQ_CASE1_TYPE,
                                    meta_isvalid = 1,
                                    meta_isdirty = isdirty,
                                    meta_islock = islock,
                                    meta_isbackup = isbackup,
                                    meta_iscase3 = iscase3)
                            actnspec0 = netbufferv3_forward_delreq_case1_and_clone_action_spec_t(\
                                    sids[0], self.devPorts[1]) # Clone to client, forward to server
                            self.client.port_forward_tbl_table_add_with_forward_delreq_case1_and_clone(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for islock in lock_list:
                        for isbackup in backup_list:
                            for iscase3 in case3_list:
                                matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                        op_hdr_optype = GETRES_S_CASE2_TYPE,
                                        meta_isvalid = isvalid,
                                        meta_isdirty = isdirty,
                                        meta_islock = islock,
                                        meta_isbackup = isbackup,
                                        meta_iscase3 = iscase3)
                                actnspec0 = netbufferv3_update_getres_s_case2_to_putreq_gs_case2_and_clone_action_spec_t(\
                                        sids[0], self.devPorts[1]) # Clone to client port, output to server port
                                self.client.port_forward_tbl_table_add_with_update_getres_s_case2_to_putreq_gs_case2_and_clone(\
                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for islock in lock_list:
                        for isbackup in backup_list:
                            for iscase3 in case3_list:
                                matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                        op_hdr_optype = PUTREQ_RU_CASE2_TYPE,
                                        meta_isvalid = isvalid,
                                        meta_isdirty = isdirty,
                                        meta_islock = islock,
                                        meta_isbackup = isbackup,
                                        meta_iscase3 = iscase3)
                                actnspec0 = netbufferv3_update_putreq_ru_case2_to_putreq_ps_case2_and_clone_action_spec_t(\
                                        sids[0], self.devPorts[1]) # Clone to client port, output to server port
                                self.client.port_forward_tbl_table_add_with_update_putreq_ru_case2_to_putreq_ps_case2_and_clone(\
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
                matchspec0 = netbufferv3_hash_partition_tbl_match_spec_t(\
                        udp_hdr_dstPort=server_port, \
                        eg_intr_md_egress_port=self.devPorts[1], \
                        meta_hashidx_start = hash_start, \
                        meta_hashidx_end = hash_end)
                actnspec0 = netbufferv3_update_dstport_action_spec_t(\
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
                matchspec0 = netbufferv3_hash_partition_reverse_tbl_match_spec_t(\
                        udp_hdr_srcPort=server_port, \
                        eg_intr_md_egress_port=self.devPorts[1], \
                        meta_hashidx_start = hash_start, \
                        meta_hashidx_end = hash_end)
                actnspec0 = netbufferv3_update_dstport_reverse_action_spec_t(\
                        server_port + i)
                self.client.hash_partition_reverse_tbl_table_add_with_update_dstport_reverse(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                hash_start = hash_end

            # Table: update_macaddr_tbl (default: nop; 5)
            print "Configuring update_macaddr_tbl"
            actnspec0 = netbufferv3_update_macaddr_s2c_action_spec_t(\
                    macAddr_to_string(src_mac), \
                    macAddr_to_string(dst_mac))
            actnspec1 = netbufferv3_update_macaddr_c2s_action_spec_t(\
                    macAddr_to_string(src_mac), \
                    macAddr_to_string(dst_mac))
            matchspec0 = netbufferv3_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_TYPE)
            matchspec1 = netbufferv3_update_macaddr_tbl_match_spec_t(op_hdr_optype=PUTRES_TYPE)
            matchspec2 = netbufferv3_update_macaddr_tbl_match_spec_t(op_hdr_optype=DELRES_TYPE)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_s2c(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_s2c(\
                    self.sess_hdl, self.dev_tgt, matchspec1, actnspec0)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_s2c(\
                    self.sess_hdl, self.dev_tgt, matchspec2, actnspec0)
            matchspec3 = netbufferv3_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_POP_TYPE)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_c2s(\
                    self.sess_hdl, self.dev_tgt, matchspec3, actnspec1)
            matchspec4 = netbufferv3_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_POP_EVICT_TYPE)
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
