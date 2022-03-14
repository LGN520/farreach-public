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
GETRES_POP_LARGE_TYPE = 0x0b
GETRES_POP_EVICT_TYPE = 0x0c
PUTREQ_POP_TYPE = 0x0d
PUTREQ_RECIR_TYPE = 0x0e
PUTREQ_POP_EVICT_TYPE = 0x0f
PUTREQ_LARGE_TYPE = 0x10
PUTREQ_LARGE_RECIR_TYPE = 0x11
PUTREQ_LARGE_EVICT_TYPE = 0x12
DELREQ_RECIR_TYPE = 0x13
PUTREQ_CASE1_TYPE = 0x14
DELREQ_CASE1_TYPE = 0x15
GETRES_POP_EVICT_CASE2_TYPE = 0x16
PUTREQ_POP_EVICT_CASE2_TYPE = 0x17
PUTREQ_LARGE_EVICT_CASE2_TYPE = 0x18
PUTREQ_MAY_CASE3_TYPE = 0x19
PUTREQ_CASE3_TYPE = 0x1a
DELREQ_MAY_CASE3_TYPE = 0x1b
DELREQ_CASE3_TYPE = 0x1c

valid_list = [0, 1]
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
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbufferv3"])

    def configure_access_key_tbl(self, keyname):
        # 9
        for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE, PUTREQ_RECIR_TYPE, DELREQ_TYPE, DELREQ_RECIR_TYPE, PUTREQ_LARGE_TYPE, PUTREQ_LARGE_RECIR]:
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
        # 16
        # NOTE: we do not need isvalid here even for PUTREQ_LARGE/RECIR
        for canput in predicate_list:
            for tmpoptype in [GETREQ_TYPE, PUTREQ_LARGE_TYPE, PUTREQ_LARGE_RECIR_TYPE]:
                matchspec0 = eval("netbufferv3_update_val{}_tbl_match_spec_t".format(valname))(
                        op_hdr_optype=tmpoptype,
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
            sidnum = 2
            sids = random.sample(xrange(BASE_SID_NORM, MAX_SID_NORM), sidnum)
            print "Binding sid {} with port {}".format(sids[0], self.devPorts[0]) # clone to client
            info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
                                  Direction_e.PD_DIR_INGRESS,
                                  sids[0],
                                  self.devPorts[0],
                                  True)
            print "Binding sid {} with port {}".format(sids[1], self.devPorts[1]) # clone to server
            info = mirror_session(MirrorType_e.PD_MIRROR_TYPE_NORM,
                                  Direction_e.PD_DIR_INGRESS,
                                  sids[1],
                                  self.devPorts[1],
                                  True)
            self.mirror.mirror_session_create(self.sess_hdl, self.dev_tgt, info)

            # Stage 0

            # Table: access_keylolo_tbl (default: initialize_ismatch_keylolo; 7)
            print "Configuring match_keylolo_tbl"
            self.configure_access_key_tbl("lolo")

            # Table: access_keylohi_tbl (default: initialize_ismatch_keylohi; 7)
            print "Configuring match_keylohi_tbl"
            self.configure_access_key_tbl("lohi")

            # Table: access_keyhilo_tbl (default: initialize_ismatch_keyhilo; 7)
            print "Configuring match_keyhilo_tbl"
            self.configure_access_key_tbl("hilo")

            # Table: access_keyhihi_tbl (default: initialize_ismatch_keyhihi; 7)
            print "Configuring match_keyhihi_tbl"
            self.configure_access_key_tbl("hihi")

            # Stage 1

            # Table: access_valid_tbl (default: nop; 104)
            print "Configuring access_valid_tbl"
            for ismatch_keylolo in predicate_list:
                for ismatch_keylohi in predicate_list:
                    for ismatch_keyhilo in predicate_list:
                        for ismatch_keyhihi in predicate_list:
                            for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE, PUTREQ_RECIR_TYPE, DELREQ_TYPE, DELREQ_RECIR_TYPE]:
                                matchspec0 = netbufferv3_access_valid_tbl_match_spec_t(
                                        op_hdr_optype=tmpoptype,
                                        meta_ismatch_keylolo=ismatch_keylolo, 
                                        meta_ismatch_keylohi=ismatch_keylohi, 
                                        meta_ismatch_keyhilo=ismatch_keyhilo, 
                                        meta_ismatch_keyhihi=ismatch_keyhihi)
                                self.client.access_valid_tbl_table_add_with_get_valid(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
                            # NOTE: Although GETRES_POP/PUTREQ_POP trigger set_and_get_key in access_key_tbl to set op_hdr.key in blackbox_alu, they also reset ismatch_key as 1 (default value) in action
                            for tmpoptype in [GETRES_POP_TYPE, PUTREQ_POP_TYPE]:
                                matchspec0 = netbufferv3_access_valid_tbl_match_spec_t(
                                        op_hdr_optype=tmpoptype,
                                        meta_ismatch_keylolo=ismatch_keylolo, 
                                        meta_ismatch_keylohi=ismatch_keylohi, 
                                        meta_ismatch_keyhilo=ismatch_keyhilo, 
                                        meta_ismatch_keyhihi=ismatch_keyhihi)
                                self.client.access_valid_tbl_table_add_with_set_and_get_valid(\
                                        self.sess_hdl, self.dev_tgt, matchspec0)
            for tmpoptype in [PUTREQ_LARGE_TYPE, PUTREQ_LARGE_RECIR_TYPE]:
                matchspec0 = netbufferv3_access_valid_tbl_match_spec_t(
                        op_hdr_optype=tmpoptype,
                        meta_ismatch_keylolo=2,
                        meta_ismatch_keylohi=2,
                        meta_ismatch_keyhilo=2,
                        meta_ismatch_keyhihi=2)
                self.client.access_valid_tbl_table_add_with_reset_and_get_valid(\
                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_vote_tbl (default: nop; 80)
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
                                        ismatch_keyhilo == 2 and ismatch_keyhihi == 2):
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

            # Table assign_seq_tbl (default: nop; 2)
            # NOTE: PUTREQ_RECIR, DELREQ_RECIR, and PUTREQ_LARGE_RECIR do not need to assign seq
            print "Configuring assign_seq_tbl"
            for tmpoptype in [PUTREQ_TYPE, DELREQ_TYPE, PUTREQ_LARGE_TYPE]:
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

            # Table: access_savedseq_tbl (default: nop; 12)
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

            # Table: access_lock_tbl (default: nop; 28)
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
                            meta_isvalid=isvalid,
                            meta_zerovote=zerovote)
                    self.client.access_lock_tbl_table_add_with_read_lock(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
            for isvalid in valid_list:
                for zerovote in predicate_list:
                    for tmpoptype in [GETRES_POP_TYPE, GETRES_NPOP_TYPE, GETRES_POP_LARGE_TYPE, PUTREQ_POP_TYPE]:
                        matchspec0 = netbufferv3_access_lock_tbl_match_spec_t(
                                op_hdr_optype=tmpoptype,
                                meta_isvalid=isvalid,
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
                                matchspec0 = netbufferv3_access_case12_tbl_match_spec_t(\
                                        op_hdr_optype=tmpoptype,
                                        meta_isvalid=isvalid,
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
                                matchspec0 = netbufferv3_access_case12_tbl_match_spec_t(\
                                        op_hdr_optype=tmpoptype,
                                        meta_isvalid=isvalid,
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
                                matchspec0 = netbufferv3_access_case12_tbl_match_spec_t(\
                                        op_hdr_optype=tmpoptype,
                                        meta_isvalid=isvalid,
                                        meta_iskeymatch=iskeymatch,
                                        meta_canput=canput,
                                        meta_isbackup=isbackup)
                                if isbackup == 1 and isvalid == 1 and iskeymatch == 1:
                                    self.client.access_case12_tbl_table_add_with_try_case12(\
                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                else:
                                    self.client.access_case12_tbl_table_add_with_read_case12(\
                                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: update_vallen_tbl (default: nop; 24)
            print "Configuring update_vallen_tbl"
            for canput in predicate_list:
                for tmpoptype in [GETREQ_TYPE, PUTREQ_LARGE_TYPE, PUTREQ_LARGE_RECIR_TYPE]:
                    matchspec0 = netbufferv3_update_vallen_tbl_match_spec_t(\
                            op_hdr_optype=tmpoptype,
                            meta_canput=canput,
                            meta_iskeymatch=1,
                            meta_isvalid=1)
                    self.client.update_vallen_tbl_table_add_with_get_vallen(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
            for tmpoptype in [PUTREQ_TYPE, PUTREQ_RECIR_TYPE, DELREQ_TYPE, DELREQ_RECIR_TYPE]:
                matchspec0 = netbufferv3_update_vallen_tbl_match_spec_t(\
                        op_hdr_optype=tmpoptype, 
                        meta_canput=2, # canput means valid=1, iskeymatch=1, and seq>savedseq
                        meta_iskeymatch=1,
                        meta_isvalid=1)
                if (tmpoptype == PUTREQ_TYPE or tmpoptype == PUTREQ_RECIR_TYPE):
                    self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
                else: # DELREQ resets and gets vallen for CASE1
                    self.client.update_vallen_tbl_table_add_with_reset_and_get_vallen(\
                            self.sess_hdl, self.dev_tgt, matchspec0)
            for iskeymatch in keymatch_list:
                for canput in predicate_list:
                    for isvalid in valid_list:
                        for tmpoptype in [GETRES_POP_TYPE, PUTREQ_POP_TYPE]:
                            matchspec0 = netbufferv3_update_vallen_tbl_match_spec_t(\
                                    op_hdr_optype=tmpoptype, 
                                    meta_canput=canput,
                                    meta_iskeymatch=iskeymatch,
                                    meta_isvalid=isvalid)
                            self.client.update_vallen_tbl_table_add_with_set_and_get_vallen(\
                                    self.sess_hdl, self.dev_tgt, matchspec0)

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

            # Stage 11

            # Table access_case3_tbl (default: nop; 128)
            print "Configuring access_case3_tbl"
            for isvalid in valid_list:
                for zerovote in predicate_list:
                    for iskeymatch in keymatch_list:
                        for islock in lock_list:
                            for isbackup in backup_list:
                                for tmpoptype in [PUTREQ_TYPE, PUTREQ_RECIR_TYPE]:
                                    matchspec0 = netbufferv3_access_case3_tbl_match_spec_t(\
                                            op_hdr_optype = tmpoptype,
                                            meta_isvalid = isvalid,
                                            meta_zerovote = zerovote,
                                            meta_iskeymatch = iskeymatch,
                                            meta_islock = islock,
                                            meta_isbackup = isbackup)
                                    if islock == 0 and (isvalid == 0 or zerovote == 2):
                                        continue
                                    elif isvalid == 1 and iskeymatch == 1:
                                        continue
                                    elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                        continue
                                    else:
                                        if isbackup == 0:
                                            continue
                                        else:
                                            self.client.access_case3_tbl_table_add_with_try_case3(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                for tmpoptype in [DELREQ_TYPE, DELREQ_RECIR_TYPE]:
                                    matchspec0 = netbufferv3_access_case3_tbl_match_spec_t(\
                                            op_hdr_optype = tmpoptype,
                                            meta_isvalid = isvalid,
                                            meta_zerovote = zerovote,
                                            meta_iskeymatch = iskeymatch,
                                            meta_islock = islock,
                                            meta_isbackup = isbackup)
                                    if isvalid == 1 and iskeymatch == 1:
                                        continue
                                    elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                        continue
                                    else:
                                        if isbackup == 0:
                                            continue
                                        else:
                                            self.client.access_case3_tbl_table_add_with_try_case3(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)


            # Table: port_forward_tbl (default: nop; 1665)
            print "Configuring port_forward_tbl"
            for isvalid in valid_list:
                for zerovote in predicate_list:
                    for iskeymatch in keymatch_list:
                        for islock in lock_list:
                            for canput in  predicate_list:
                                for isbackup in backup_list:
                                    for iscase12 in case12_list:
                                        matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = GETREQ_TYPE,
                                                meta_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
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
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if isbackup == 1 and iscase12 == 0:
                                                # Update GETRES_POP as GETRES_POP_EVICT_CASE2 to server, clone original pkt with new kv to client port for GETRES to client
                                                actnspec0 = netbufferv3_update_getres_pop_to_case2_clone_for_getres_action_spec_t(\
                                                        sids[0])
                                                self.client.port_forward_tbl_table_add_with_update_getres_pop_to_case2_clone_for_getres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
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
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        actnspec0 = netbufferv3_update_getres_npop_to_getres_action_spec_t(\
                                                self.devPorts[0])
                                        self.client.port_forward_tbl_table_add_with_update_getres_npop_to_getres(\
                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        # Update GETRES_POP_LARGE as GETRES to client
                                        matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = GETRES_POP_LARGE_TYPE,
                                                meta_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        actnspec0 = netbufferv3_update_getres_pop_large_to_getres_action_spec_t(\
                                                self.devPorts[0])
                                        self.client.port_forward_tbl_table_add_with_update_getres_pop_large_to_getres(\
                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = PUTREQ_TYPE,
                                                meta_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if islock == 0 and (isvalid == 0 or zerovote == 2):
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv3_update_putreq_to_putreq_pop_action_spec_t(self.recirPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_update_putreq_to_putreq_pop(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        elif isvalid == 1 and iskeymatch == 1:
                                            if canput == 2 and isbackup == 1 and iscase12 == 0:
                                                # Update PUTREQ as PUTREQ_CASE1 to server, clone original packet as PUTRES to client
                                                actnspec0 = netbufferv3_update_putreq_to_case1_clone_for_putres_action_spec_t(\
                                                        sids[0], self.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_to_case1_clone_for_putres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Sendback PUTRES to client
                                                self.client.port_forward_tbl_table_add_with_update_putreq_to_putres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                        elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv3_update_putreq_to_putreq_recir_action_spec_t(self.recirPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_update_putreq_to_putreq_recir(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            if isbackup == 0:
                                                # Forwrad PUTREQ to server 
                                                actnspec0 = netbufferv3_port_forward_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_port_forward(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update PUTREQ as PUTREQ_MAY_CASE3 to server
                                                actnspec0 = netbufferv3_update_putreq_to_may_case3_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_putreq_to_may_case3(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = PUTREQ_POP_TYPE,
                                                meta_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if isbackup == 1 and iscase12 == 0:
                                                # Update PUTREQ_POP as PUTREQ_POP_EVICT_CASE2 to server, clone original pkt with new kv to client port for PUTRES to client
                                                actnspec0 = netbufferv3_update_putreq_pop_to_case2_clone_for_putres_action_spec_t(\
                                                        sids[0], self.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_pop_to_case2_clone_for_putres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
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
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if islock == 0 and (isvalid == 0 or zerovote == 2):
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv3_update_putreq_recir_to_putreq_pop_action_spec_t(self.recirPorts[0])     
                                            self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_putreq_pop(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        elif isvalid == 1 and iskeymatch == 1:
                                            if canput == 2 and isbackup == 1 and iscase12 == 0:
                                                # Update PUTREQ_RECIR as PUTREQ_CASE1 to server, clone original packet as PUTRES to client
                                                actnspec0 = netbufferv3_update_putreq_recir_to_case1_clone_for_putres_action_spec_t(\
                                                        sids[0], self.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_case1_clone_for_putres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Sendback PUTRES to client
                                                self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_putres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                        elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv3_recirculate_putreq_recir_action_spec_t(self.recirPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_recirculate_putreq_recir(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            if isbackup == 0:
                                                # Forwrad PUTREQ to server 
                                                actnspec0 = netbufferv3_update_putreq_recir_to_putreq_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_putreq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update PUTREQ_RECIR as PUTREQ_MAY_CASE3 to server
                                                actnspec0 = netbufferv3_update_putreq_recir_to_may_case3_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_putreq_recir_to_may_case3(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = PUTREQ_LARGE_TYPE,
                                                meta_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if iskeymatch == 0 and islock == 1:
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv3_update_putreq_large_to_putreq_large_recir_action_spec_t(self.recirPorts[0])
                                            self.client.port_forward_tbl_table_add_with_update_putreq_large_to_putreq_large_recir(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        elif iskeymatch == 1 and isvalid == 1:
                                            if isbackup == 1 and iscase12 == 0:
                                                # Update PUTREQ_LARGE as PUTREQ_LARGE_EVICT_CASE2 to server, clone original packet as PUTREQ_LARGE to server
                                                actnspec0 = netbufferv3_update_putreq_large_to_case2_clone_for_putreq_large_action_spec_t(\
                                                        sids[1], sefl.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_large_to_case2_clone_for_putreq_large(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update PUTREQ_LARGE as PUTREQ_LARGE_EVICT to server, clone original packet as PUTREQ_LARGE to server
                                                actnspec0 = netbufferv3_update_putreq_large_to_evict_clone_for_putreq_large_action_spec_t(\
                                                        sids[1], sefl.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_large_to_evict_clone_for_putreq_large(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            # Forward PUTREQ_LARGE to server
                                            actnspec0 = netbufferv3_port_forward_action_spec_t(self.devPorts[1]) 
                                            self.client.port_forward_tbl_table_add_with_port_forward(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = PUTREQ_LARGE_RECIR_TYPE,
                                                meta_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if iskeymatch == 0 and islock == 1:
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv3_recirculate_putreq_large_recir_action_spec_t(self.recirPorts[0])
                                            self.client.port_forward_tbl_table_add_with_recirculate_putreq_large_recir(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        elif iskeymatch == 1 and isvalid == 1:
                                            if isbackup == 1 and iscase12 == 0:
                                                # Update PUTREQ_LARGE_RECIR as PUTREQ_LARGE_EVICT_CASE2 to server, clone original packet as PUTREQ_LARGE to server
                                                actnspec0 = netbufferv3_update_putreq_large_recir_to_case2_clone_for_putreq_large_action_spec_t(\
                                                        sids[1], sefl.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_large_recir_to_case2_clone_for_putreq_large(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update PUTREQ_LARGE_RECIR as PUTREQ_LARGE_EVICT to server, clone original packet as PUTREQ_LARGE to server
                                                actnspec0 = netbufferv3_update_putreq_large_recir_to_evict_clone_for_putreq_large_action_spec_t(\
                                                        sids[1], sefl.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_putreq_large_recir_to_evict_clone_for_putreq_large(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            # Update PUTREQ_LARGE_RECIR as PUTREQ_LARGE to server
                                            actnspec0 = netbufferv3_update_putreq_large_recir_to_putreq_large_action_spec_t(self.devPorts[1]) 
                                            self.client.port_forward_tbl_table_add_with_update_putreq_large_recir_to_putreq_large(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = DELREQ_TYPE,
                                                meta_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if isvalid == 1 and iskeymatch == 1:
                                            if canput == 2 and isbackup == 1 and iscase12 == 0:
                                                # Update DELREQ as DELREQ_CASE1 to server, clone original packet as DELRES to client
                                                actnspec0 = netbufferv3_update_delreq_to_case1_clone_for_delres_action_spec_t(\
                                                        sids[0], self.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_delreq_to_case1_clone_for_delres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Sendback DELRES to client
                                                self.client.port_forward_tbl_table_add_with_update_delreq_to_delres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                        elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv3_update_delreq_to_delreq_recir_action_spec_t(self.recirPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_update_delreq_to_delreq_recir(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            if isbackup == 0:
                                                # Forwrad DELREQ to server 
                                                actnspec0 = netbufferv3_port_forward_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_port_forward(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update DELREQ as DELREQ_MAY_CASE3 to server 
                                                actnspec0 = netbufferv3_update_delreq_to_may_case3_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_delreq_to_may_case3(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = DELREQ_RECIR_TYPE,
                                                meta_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        if isvalid == 1 and iskeymatch == 1:
                                            if canput == 2 and isbackup == 1 and iscase12 == 0:
                                                # Update DELREQ_RECIR as DELREQ_CASE1 to server, clone original packet as DELRES to client
                                                actnspec0 = netbufferv3_update_delreq_recir_to_case1_clone_for_delres_action_spec_t(\
                                                        sids[0], self.devPorts[1])
                                                self.client.port_forward_tbl_table_add_with_update_delreq_recir_to_case1_clone_for_delres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Sendback DELRES to client
                                                self.client.port_forward_tbl_table_add_with_update_delreq_recir_to_delres(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                        elif (isvalid == 0 or iskeymatch == 0) and islock == 1:
                                            # Use recirculate port 64 + pipe ID of ingress port
                                            actnspec0 = netbufferv3_recirculate_delreq_recir_action_spec_t(self.recirPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_recirculate_delreq_recir(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        else:
                                            if isbackup == 0:
                                                # Forwrad DELREQ to server 
                                                actnspec0 = netbufferv3_update_delreq_recir_to_delreq_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_delreq_recir_to_delreq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                            else:
                                                # Update DELREQ_RECIR as DELREQ_MAY_CASE3 to server 
                                                actnspec0 = netbufferv3_update_delreq_recir_to_may_case3_action_spec_t(self.devPorts[1]) 
                                                self.client.port_forward_tbl_table_add_with_update_delreq_recir_to_may_case3(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        # Forward SCANREQ to server
                                        matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                                op_hdr_optype = SCANREQ_TYPE,
                                                meta_isvalid = isvalid,
                                                meta_zerovote = zerovote,
                                                meta_iskeymatch = iskeymatch,
                                                meta_islock = islock,
                                                meta_canput = canput,
                                                meta_isbackup = isbackup,
                                                meta_iscase12 = iscase12)
                                        actnspec0 = netbufferv3_port_forward_action_spec_t(self.devPorts[1]) 
                                        self.client.port_forward_tbl_table_add_with_port_forward(\
                                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                                        # Forward RES to client
                                        for tmpoptype in [GETRES_TYPE, PUTRES_TYPE, DELRES_TYPE, SCANRES_TYPE]:
                                            matchspec0 = netbufferv3_port_forward_tbl_match_spec_t(\
                                                    op_hdr_optype = tmpoptype,
                                                    meta_isvalid = isvalid,
                                                    meta_zerovote = zerovote,
                                                    meta_iskeymatch = iskeymatch,
                                                    meta_islock = islock,
                                                    meta_canput = canput,
                                                    meta_isbackup = isbackup,
                                                    meta_iscase12 = iscase12)
                                            actnspec0 = netbufferv3_port_forward_action_spec_t(self.devPorts[0]) 
                                            self.client.port_forward_tbl_table_add_with_port_forward(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)



            ### Egress ###

            # Table: process_cloned_packet_tbl
            print "Configuring process_cloned_packet_tbl"
            matchspec0 = netbufferv3_process_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_TYPE)
            self.client.process_cloned_packet_tbl_table_add_with_update_cloned_putreq_to_putres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv3_process_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_RECIR_TYPE)
            self.client.process_cloned_packet_tbl_table_add_with_update_cloned_putreq_recir_to_putres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv3_process_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = DELREQ_TYPE)
            self.client.process_cloned_packet_tbl_table_add_with_update_cloned_delreq_to_delres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv3_process_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = DELREQ_RECIR_TYPE)
            self.client.process_cloned_packet_tbl_table_add_with_update_cloned_delreq_to_delres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv3_process_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = GETRES_POP_TYPE)
            self.client.process_cloned_packet_tbl_table_add_with_update_cloned_getres_pop_to_getres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv3_process_cloned_packet_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_POP_TYPE)
            self.client.process_cloned_packet_tbl_table_add_with_update_cloned_putreq_pop_to_putres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table process_may_case3_tbl (default: nop; ?)
            print "Configuring process_may_case3_tbl"
            matchspec0 = netbufferv3_process_may_case3_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_MAY_CASE3_TYPE,
                    other_hdr_iscase3 = 0)
            self.client.process_may_case3_tbl_table_add_with_update_putreq_may_case3_to_case3(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv3_process_may_case3_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_MAY_CASE3_TYPE,
                    other_hdr_iscase3 = 1)
            self.client.process_may_case3_tbl_table_add_with_update_putreq_may_case3_to_putreq(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv3_process_may_case3_tbl_match_spec_t(\
                    op_hdr_optype = DELREQ_MAY_CASE3_TYPE,
                    other_hdr_iscase3 = 0)
            self.client.process_may_case3_tbl_table_add_with_update_delreq_may_case3_to_case3(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbufferv3_process_may_case3_tbl_match_spec_t(\
                    op_hdr_optype = DELREQ_MAY_CASE3_TYPE,
                    other_hdr_iscase3 = 1)
            self.client.process_may_case3_tbl_table_add_with_update_delreq_may_case3_to_delreq(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

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

            # Table: update_udplen_tbl (default: nop; 120)
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
                matchspec0 = netbufferv3_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=GETRES_TYPE,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end)
                actnspec0 = netbufferv3_update_getres_udplen_action_spec_t(\
                        aligned_vallen)
                self.client.update_udplen_tbl_table_add_with_update_getres_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                for tmpoptype in [GETRES_POP_EVICT_TYPE, GETRES_POP_EVICT_CASE2_TYPE]:
                    matchspec0 = netbufferv3_update_udplen_tbl_match_spec_t(\
                            op_hdr_optype=tmpoptype,
                            vallen_hdr_vallen_start=vallen_start,
                            vallen_hdr_vallen_end=vallen_end)
                    actnspec0 = netbufferv3_update_getres_pop_evict_udplen_action_spec_t(\
                            aligned_vallen)
                    self.client.update_udplen_tbl_table_add_with_update_getres_pop_evict_udplen(\
                            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                for tmpoptype in [PUTREQ_POP_EVICT_TYPE, PUTREQ_POP_EVICT_CASE2_TYPE]:
                    matchspec0 = netbufferv3_update_udplen_tbl_match_spec_t(\
                            op_hdr_optype=tmpoptype,
                            vallen_hdr_vallen_start=vallen_start,
                            vallen_hdr_vallen_end=vallen_end)
                    actnspec0 = netbufferv3_update_putreq_pop_evict_udplen_action_spec_t(\
                            aligned_vallen)
                    self.client.update_udplen_tbl_table_add_with_update_putreq_pop_evict_udplen(\
                            self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                matchspec0 = netbufferv3_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=PUTREQ_CASE1_TYPE,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end)
                actnspec0 = netbufferv3_update_putreq_case1_udplen_action_spec_t(\
                        aligned_vallen)
                self.client.update_udplen_tbl_table_add_with_update_putreq_case1_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                matchspec0 = netbufferv3_update_udplen_tbl_match_spec_t(\
                        op_hdr_optype=DELREQ_CASE1_TYPE,
                        vallen_hdr_vallen_start=vallen_start,
                        vallen_hdr_vallen_end=vallen_end)
                actnspec0 = netbufferv3_update_delreq_case1_udplen_action_spec_t(\
                        aligned_vallen)
                self.client.update_udplen_tbl_table_add_with_update_delreq_case1_udplen(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
            # NOTE: if vallen of PUTREQ > 128B, its PUTRES must be issued by server which has already set correct udplen
            matchspec0 = netbufferv3_update_udplen_tbl_match_spec_t(\
                    op_hdr_optype=PUTRES_TYPE,
                    vallen_hdr_vallen_start=0,
                    vallen_hdr_vallen_end=switch_max_vallen) # [0, 128]
            self.client.update_udplen_tbl_table_add_with_update_putres_udplen(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

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
            #matchspec3 = netbufferv3_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_POP_TYPE)
            #self.client.update_macaddr_tbl_table_add_with_update_macaddr_c2s(\
            #        self.sess_hdl, self.dev_tgt, matchspec3, actnspec1)
            matchspec3 = netbufferv3_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_POP_EVICT_TYPE)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr_c2s(\
                    self.sess_hdl, self.dev_tgt, matchspec3, actnspec1)
            matchspec4 = netbufferv3_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_POP_EVICT_CASE2_TYPE)
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
