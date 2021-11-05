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

from netbuffer.p4_pd_rpc.ttypes import *
from pltfm_pm_rpc.ttypes import *
from mirror_pd_rpc.ttypes import *
from pal_rpc.ttypes import *
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from res_pd_rpc.ttypes import *

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
#CPU_PORT = 192
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
GETREQ_S_TYPE = 0x08
PUTREQ_GS_TYPE = 0x09
PUTREQ_PS_TYPE = 0x0a
DELREQ_S_TYPE = 0x0b
GETRES_S_TYPE = 0x0c
GETRES_NS_TYPE = 0x0d

PUTREQ_U_TYPE = 0x20

valid_list = [0, 1]
dirty_list = [0, 1]
lock_list = [0, 1]
cache_update_list = [0, 1]
predicate_list = [1, 2]

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
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["netbuffer"])

    def configure_update_val_tbl(self, valname):
        # 1026
        for canput in predicate_list:
            matchspec0 = eval("netbuffer_update_val{}_tbl_match_spec_t".format(valname))(
                    op_hdr_optype=GETREQ_TYPE, 
                    meta_canput=canput,
                    meta_ismatch_keylololo=2, 
                    meta_ismatch_keylolohi=2,
                    meta_ismatch_keylohilo=2, 
                    meta_ismatch_keylohihi=2,
                    meta_ismatch_keyhilolo=2, 
                    meta_ismatch_keyhilohi=2,
                    meta_ismatch_keyhihilo=2,
                    meta_ismatch_keyhihihi=2,
                    meta_is_putreq_ru=0)
            eval("self.client.update_val{}_tbl_table_add_with_get_val{}".format(valname, valname))(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
        matchspec1 = eval("netbuffer_update_val{}_tbl_match_spec_t".format(valname))(
                op_hdr_optype=PUTREQ_TYPE, 
                meta_canput=2,
                meta_ismatch_keylololo=2, 
                meta_ismatch_keylolohi=2,
                meta_ismatch_keylohilo=2, 
                meta_ismatch_keylohihi=2,
                meta_ismatch_keyhilolo=2, 
                meta_ismatch_keyhilohi=2,
                meta_ismatch_keyhihilo=2,
                meta_ismatch_keyhihihi=2,
                meta_is_putreq_ru=0)
        eval("self.client.update_val{}_tbl_table_add_with_put_val{}".format(valname, valname))(\
                self.sess_hdl, self.dev_tgt, matchspec1)
        for ismatch_keylololo in predicate_list:
            for ismatch_keylolohi in predicate_list:
                for ismatch_keylohilo in predicate_list:
                    for ismatch_keylohihi in predicate_list:
                        for ismatch_keyhilolo in predicate_list:
                            for ismatch_keyhilohi in predicate_list:
                                for ismatch_keyhihilo in predicate_list:
                                    for ismatch_keyhihihi in predicate_list:
                                        for canput in predicate_list:
                                            matchspec0 = eval("netbuffer_update_val{}_tbl_match_spec_t".format(valname))(\
                                                    op_hdr_optype=GETRES_S_TYPE, 
                                                    meta_canput=canput,
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                    meta_is_putreq_ru = 0)
                                            eval("self.client.update_val{}_tbl_table_add_with_put_val{}".format(valname, valname))(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                            matchspec0 = eval("netbuffer_update_val{}_tbl_match_spec_t".format(valname))(\
                                                    op_hdr_optype=PUTREQ_TYPE, 
                                                    meta_canput=canput,
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                    meta_is_putreq_ru = 1)
                                            eval("self.client.update_val{}_tbl_table_add_with_put_val{}".format(valname, valname))(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)

    def configure_access_key_tbl(self, keyname):
        # 5
        matchspec0 = eval("netbuffer_access_key{}_tbl_match_spec_t".format(keyname))(\
                op_hdr_optype=GETREQ_TYPE,
                meta_is_putreq_ru=0);
        matchspec1 = eval("netbuffer_access_key{}_tbl_match_spec_t".format(keyname))(\
                op_hdr_optype=PUTREQ_TYPE,
                meta_is_putreq_ru=0);
        matchspec2 = eval("netbuffer_access_key{}_tbl_match_spec_t".format(keyname))(\
                op_hdr_optype=DELREQ_TYPE,
                meta_is_putreq_ru=0);
        eval("self.client.access_key{}_tbl_table_add_with_match_key{}".format(keyname, keyname))(\
                self.sess_hdl, self.dev_tgt, matchspec0)
        eval("self.client.access_key{}_tbl_table_add_with_match_key{}".format(keyname, keyname))(\
                self.sess_hdl, self.dev_tgt, matchspec1)
        eval("self.client.access_key{}_tbl_table_add_with_match_key{}".format(keyname, keyname))(\
                self.sess_hdl, self.dev_tgt, matchspec2)
        matchspec0 = eval("netbuffer_access_key{}_tbl_match_spec_t".format(keyname))(\
                op_hdr_optype=GETRES_S_TYPE,
                meta_is_putreq_ru=0)
        eval("self.client.access_key{}_tbl_table_add_with_modify_key{}".format(keyname, keyname))(\
                self.sess_hdl, self.dev_tgt, matchspec0)
        matchspec1 = eval("netbuffer_access_key{}_tbl_match_spec_t".format(keyname))(\
                op_hdr_optype=PUTREQ_TYPE,
                meta_is_putreq_ru=1)
        eval("self.client.access_key{}_tbl_table_add_with_modify_key{}".format(keyname, keyname))(\
                self.sess_hdl, self.dev_tgt, matchspec1)

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
        #self.devPorts.append(CPU_PORT)

        # get recirculate port from device port (remove the first 2-bit pipe ID)
        self.recirPorts = []
        for i in range(len(self.devPorts)):
            self.recirPorts.append(self.devPorts[i] & 0x7F)

    ### MAIN ###

    def runTest(self):
        if test_param_get('cleanup') != True:
            print '\nTest'

            # add and enable the platform ports
            for i in self.devPorts:
               self.pal.pal_port_add(0, i,
                                     pal_port_speed_t.BF_SPEED_40G,
                                     pal_fec_type_t.BF_FEC_TYP_NONE)
               self.pal.pal_port_enable(0, i)

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

            # Table assign_seq_tbl (default: nop; 1)
            print "Configuring assign_seq_tbl"
            matchspec0 = netbuffer_assign_seq_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_TYPE, 
                    seq_hdr_is_assigned = 0)
            self.client.assign_seq_tbl_table_add_with_assign_seq(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_keylololo_tbl (default: nop; 5)
            print "Configuring match_keylololo_tbl"
            self.configure_access_key_tbl("lololo")

            # Table: access_keylolohi_tbl (default: nop)
            print "Configuring match_keylolohi_tbl"
            self.configure_access_key_tbl("lolohi")

            # Table: access_keylohilo_tbl (default: nop)
            print "Configuring match_keylohilo_tbl"
            self.configure_access_key_tbl("lohilo")

            # Table: access_keylohihi_tbl (default: nop)
            print "Configuring match_keylohihi_tbl"
            self.configure_access_key_tbl("lohihi")

            # Table: access_keyhilolo_tbl (default: nop)
            print "Configuring match_keyhilolo_tbl"
            self.configure_access_key_tbl("hilolo")

            # Table: access_keyhilohi_tbl (default: nop)
            print "Configuring match_keyhilohi_tbl"
            self.configure_access_key_tbl("hilohi")

            # Table: access_keyhihilo_tbl (default: nop)
            print "Configuring match_keyhihilo_tbl"
            self.configure_access_key_tbl("hihilo")

            # Table: access_keyhihihi_tbl (default: nop)
            print "Configuring match_keyhihihi_tbl"
            self.configure_access_key_tbl("hihihi")

            # Table: access_valid_tbl (default: nop; 1280)
            print "Configuring access_valid_tbl"
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE, DELREQ_TYPE]:
                                                matchspec0 = netbuffer_access_valid_tbl_match_spec_t(
                                                        op_hdr_optype=tmpoptype, 
                                                        meta_ismatch_keylololo=ismatch_keylololo, 
                                                        meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                        meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                        meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                        meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                        meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                        meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                        meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                        meta_is_putreq_ru=0)
                                                if (tmpoptype == DELREQ_TYPE and \
                                                ismatch_keylololo == 2 and ismatch_keylolohi == 2 and \
                                                ismatch_keylohilo == 2 and ismatch_keylohihi == 2 and \
                                                ismatch_keyhilolo == 2 and ismatch_keyhilohi == 2 and \
                                                ismatch_keyhihilo == 2 and ismatch_keyhihihi == 2):
                                                    self.client.access_valid_tbl_table_add_with_clear_valid(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                else: 
                                                    self.client.access_valid_tbl_table_add_with_get_valid(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                            # NOTE: GETRES_S/PUTREQ_U_S triggers modify_key which does not set ismatch_key but set origin_key
                                            matchspec0 = netbuffer_access_valid_tbl_match_spec_t(
                                                    op_hdr_optype=GETRES_S_TYPE, 
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                    meta_is_putreq_ru=0)
                                            self.client.access_valid_tbl_table_add_with_set_valid(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                            matchspec0 = netbuffer_access_valid_tbl_match_spec_t(
                                                    op_hdr_optype=PUTREQ_TYPE, 
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                    meta_is_putreq_ru=1)
                                            self.client.access_valid_tbl_table_add_with_set_valid(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_dirty_tbl (default: nop; 1024)
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            matchspec0 = netbuffer_access_dirty_tbl_match_spec_t(\
                                                    op_hdr_optype=GETRES_S_TYPE,
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                    meta_is_putreq_ru=0)
                                            self.client.access_dirty_tbl_table_add_with_clear_dirty(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                            matchspec1 = netbuffer_access_dirty_tbl_match_spec_t(\
                                                    op_hdr_optype=PUTREQ_TYPE,
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                    meta_is_putreq_ru=1)
                                            self.client.access_dirty_tbl_table_add_with_set_dirty(\
                                                    self.sess_hdl, self.dev_tgt, matchspec1)
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE]: # DELREQ does not to read dirty bit
                                                matchspec0 = netbuffer_access_dirty_tbl_match_spec_t(\
                                                        op_hdr_optype=tmpoptype,
                                                        meta_ismatch_keylololo=ismatch_keylololo, 
                                                        meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                        meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                        meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                        meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                        meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                        meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                        meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                        meta_is_putreq_ru=0)
                                                if (tmpoptype == PUTREQ_TYPE and \
                                                ismatch_keylololo == 2 and ismatch_keylolohi == 2 and \
                                                ismatch_keylohilo == 2 and ismatch_keylohihi == 2 and \
                                                ismatch_keyhilolo == 2 and ismatch_keyhilohi == 2 and \
                                                ismatch_keyhihilo == 2 and ismatch_keyhihihi == 2):
                                                    self.client.access_dirty_tbl_table_add_with_set_dirty(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0)
                                                else:
                                                    self.client.access_dirty_tbl_table_add_with_get_dirty(\
                                                            self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_savedseq_tbl (default: nop; 1026)
            print "Configuring access_savedseq_tbl"
            matchspec0 = netbuffer_access_savedseq_tbl_match_spec_t(\
                    op_hdr_optype = PUTREQ_TYPE,
                    seq_hdr_is_assigned = 1,
		    meta_ismatch_keylololo=2, 
		    meta_ismatch_keylolohi=2, 
		    meta_ismatch_keylohilo=2, 
		    meta_ismatch_keylohihi=2, 
		    meta_ismatch_keyhilolo=2,
		    meta_ismatch_keyhilohi=2,
		    meta_ismatch_keyhihilo=2,
		    meta_ismatch_keyhihihi=2,
                    meta_is_putreq_ru=0)
            self.client.access_savedseq_tbl_table_add_with_try_update_savedseq(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbuffer_access_savedseq_tbl_match_spec_t(\
                    op_hdr_optype = DELREQ_TYPE,
                    seq_hdr_is_assigned = 0,
		    meta_ismatch_keylololo=2, 
		    meta_ismatch_keylolohi=2, 
		    meta_ismatch_keylohilo=2, 
		    meta_ismatch_keylohihi=2, 
		    meta_ismatch_keyhilolo=2,
		    meta_ismatch_keyhilohi=2,
		    meta_ismatch_keyhihilo=2,
		    meta_ismatch_keyhihihi=2,
                    meta_is_putreq_ru=0)
            self.client.access_savedseq_tbl_table_add_with_reset_savedseq(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            for isassigned in [0, 1]:
                                                matchspec0 = netbuffer_access_savedseq_tbl_match_spec_t(\
                                                        op_hdr_optype = GETRES_S_TYPE,
                                                        seq_hdr_is_assigned = isassigned,
                                                        meta_ismatch_keylololo=ismatch_keylololo, 
                                                        meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                        meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                        meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                        meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                        meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                        meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                        meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                        meta_is_putreq_ru=0)
                                                self.client.access_savedseq_tbl_table_add_with_reset_savedseq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                matchspec0 = netbuffer_access_savedseq_tbl_match_spec_t(\
                                                        op_hdr_optype = PUTREQ_TYPE,
                                                        seq_hdr_is_assigned = isassigned,
                                                        meta_ismatch_keylololo=ismatch_keylololo, 
                                                        meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                        meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                        meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                        meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                        meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                        meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                        meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                        meta_is_putreq_ru=1)
                                                self.client.access_savedseq_tbl_table_add_with_reset_savedseq(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Start from stage 4 (after keys and savedseq)

            # Table: update_vallen_tbl (default: nop; 2048)
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            for canput in predicate_list:
                                                for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE]: # DELREQ does not need vallen
                                                    matchspec0 = netbuffer_update_vallen_tbl_match_spec_t(\
                                                            op_hdr_optype=tmpoptype, 
                                                            meta_canput=canput,
                                                            meta_ismatch_keylololo=ismatch_keylololo, 
                                                            meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                            meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                            meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                            meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                            meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                            meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                            meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                            meta_is_putreq_ru=0)
                                                    if (tmpoptype == PUTREQ_TYPE and canput == 2 and \
                                                    ismatch_keylololo == 2 and ismatch_keylolohi == 2 and \
                                                    ismatch_keylohilo == 2 and ismatch_keylohihi == 2 and \
                                                    ismatch_keyhilolo == 2 and ismatch_keyhilohi == 2 and \
                                                    ismatch_keyhihilo == 2 and ismatch_keyhihihi == 2):
                                                        self.client.update_vallen_tbl_table_add_with_put_vallen(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                    else:
                                                        self.client.update_vallen_tbl_table_add_with_get_vallen(\
                                                                self.sess_hdl, self.dev_tgt, matchspec0)
                                                matchspec0 = netbuffer_update_vallen_tbl_match_spec_t(\
                                                        op_hdr_optype=GETRES_S_TYPE, 
                                                        meta_canput=canput,
                                                        meta_ismatch_keylololo=ismatch_keylololo, 
                                                        meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                        meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                        meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                        meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                        meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                        meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                        meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                        meta_is_putreq_ru=0)
                                                self.client.update_vallen_tbl_table_add_with_put_vallen(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                matchspec0 = netbuffer_update_vallen_tbl_match_spec_t(\
                                                        op_hdr_optype=PUTREQ_TYPE, 
                                                        meta_canput=canput,
                                                        meta_ismatch_keylololo=ismatch_keylololo, 
                                                        meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                        meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                        meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                        meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                        meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                        meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                        meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                        meta_is_putreq_ru=1)
                                                self.client.update_vallen_tbl_table_add_with_put_vallen(\
                                                        self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: update_vallo1_tbl (default: nop; 1026)
            print "Configuring update_vallo1_tbl"
            self.configure_update_val_tbl("lo1")

            #print "Configuring update_vallo2_tbl"
            #matchspec0 = netbuffer_update_vallo2_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo2_tbl_table_add_with_put_vallo2(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo2_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo2_tbl_table_add_with_get_vallo2(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo3_tbl"
            #matchspec0 = netbuffer_update_vallo3_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo3_tbl_table_add_with_put_vallo3(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo3_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo3_tbl_table_add_with_get_vallo3(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo4_tbl"
            #matchspec0 = netbuffer_update_vallo4_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo4_tbl_table_add_with_put_vallo4(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo4_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo4_tbl_table_add_with_get_vallo4(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo5_tbl"
            #matchspec0 = netbuffer_update_vallo5_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo5_tbl_table_add_with_put_vallo5(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo5_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo5_tbl_table_add_with_get_vallo5(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo6_tbl"
            #matchspec0 = netbuffer_update_vallo6_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo6_tbl_table_add_with_put_vallo6(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo6_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo6_tbl_table_add_with_get_vallo6(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo7_tbl"
            #matchspec0 = netbuffer_update_vallo7_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo7_tbl_table_add_with_put_vallo7(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo7_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo7_tbl_table_add_with_get_vallo7(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo8_tbl"
            #matchspec0 = netbuffer_update_vallo8_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo8_tbl_table_add_with_put_vallo8(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo8_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo8_tbl_table_add_with_get_vallo8(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo9_tbl"
            #matchspec0 = netbuffer_update_vallo9_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo9_tbl_table_add_with_put_vallo9(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo9_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo9_tbl_table_add_with_get_vallo9(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo10_tbl"
            #matchspec0 = netbuffer_update_vallo10_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo10_tbl_table_add_with_put_vallo10(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo10_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo10_tbl_table_add_with_get_vallo10(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo11_tbl"
            #matchspec0 = netbuffer_update_vallo11_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo11_tbl_table_add_with_put_vallo11(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo11_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo11_tbl_table_add_with_get_vallo11(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo12_tbl"
            #matchspec0 = netbuffer_update_vallo12_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo12_tbl_table_add_with_put_vallo12(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo12_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo12_tbl_table_add_with_get_vallo12(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo13_tbl"
            #matchspec0 = netbuffer_update_vallo13_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo13_tbl_table_add_with_put_vallo13(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo13_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo13_tbl_table_add_with_get_vallo13(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo14_tbl"
            #matchspec0 = netbuffer_update_vallo14_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo14_tbl_table_add_with_put_vallo14(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo14_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo14_tbl_table_add_with_get_vallo14(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo15_tbl"
            #matchspec0 = netbuffer_update_vallo15_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo15_tbl_table_add_with_put_vallo15(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo15_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo15_tbl_table_add_with_get_vallo15(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_vallo16_tbl"
            #matchspec0 = netbuffer_update_vallo16_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_vallo16_tbl_table_add_with_put_vallo8(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo16_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_vallo16_tbl_table_add_with_get_vallo8(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            # Table: update_valhi1_tbl (default: nop)
            print "Configuring update_valhi1_tbl"
            self.configure_update_val_tbl("hi1")

            #print "Configuring update_valhi2_tbl"
            #matchspec0 = netbuffer_update_valhi2_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi2_tbl_table_add_with_put_valhi2(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi2_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi2_tbl_table_add_with_get_valhi2(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi3_tbl"
            #matchspec0 = netbuffer_update_valhi3_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi3_tbl_table_add_with_put_valhi3(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi3_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi3_tbl_table_add_with_get_valhi3(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi4_tbl"
            #matchspec0 = netbuffer_update_valhi4_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi4_tbl_table_add_with_put_valhi4(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi4_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi4_tbl_table_add_with_get_valhi4(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi5_tbl"
            #matchspec0 = netbuffer_update_valhi5_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi5_tbl_table_add_with_put_valhi5(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi5_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi5_tbl_table_add_with_get_valhi5(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi6_tbl"
            #matchspec0 = netbuffer_update_valhi6_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi6_tbl_table_add_with_put_valhi6(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi6_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi6_tbl_table_add_with_get_valhi6(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi7_tbl"
            #matchspec0 = netbuffer_update_valhi7_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi7_tbl_table_add_with_put_valhi7(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi7_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi7_tbl_table_add_with_get_valhi7(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi8_tbl"
            #matchspec0 = netbuffer_update_valhi8_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi8_tbl_table_add_with_put_valhi8(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi8_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi8_tbl_table_add_with_get_valhi8(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi9_tbl"
            #matchspec0 = netbuffer_update_valhi9_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi9_tbl_table_add_with_put_valhi9(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi9_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi9_tbl_table_add_with_get_valhi9(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi10_tbl"
            #matchspec0 = netbuffer_update_valhi10_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi10_tbl_table_add_with_put_valhi10(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi10_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi10_tbl_table_add_with_get_valhi10(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi11_tbl"
            #matchspec0 = netbuffer_update_valhi11_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi11_tbl_table_add_with_put_valhi11(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi11_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi11_tbl_table_add_with_get_valhi11(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi12_tbl"
            #matchspec0 = netbuffer_update_valhi12_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi12_tbl_table_add_with_put_valhi12(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi12_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi12_tbl_table_add_with_get_valhi12(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi13_tbl"
            #matchspec0 = netbuffer_update_valhi13_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi13_tbl_table_add_with_put_valhi13(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi13_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi13_tbl_table_add_with_get_valhi13(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi14_tbl"
            #matchspec0 = netbuffer_update_valhi14_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi14_tbl_table_add_with_put_valhi14(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi14_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi14_tbl_table_add_with_get_valhi14(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi15_tbl"
            #matchspec0 = netbuffer_update_valhi15_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi15_tbl_table_add_with_put_valhi15(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi15_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi15_tbl_table_add_with_get_valhi15(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            #print "Configuring update_valhi16_tbl"
            #matchspec0 = netbuffer_update_valhi16_tbl_match_spec_t(
            #        op_hdr_optype=PUTREQ_TYPE)
            #self.client.update_valhi16_tbl_table_add_with_put_valhi8(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi16_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE)
            #self.client.update_valhi16_tbl_table_add_with_get_valhi8(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            # Stage 4 + n

            # Table: access_vote_tbl (default: nop; 2049)
            print "Configuring access_vote_tbl"
            # GETREQ and PUTREQ
            for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE]:
                matchspec0 = netbuffer_access_vote_tbl_match_spec_t(
                        meta_isvalid=1,
                        op_hdr_optype=tmpoptype, 
                        meta_ismatch_keylololo=2, 
                        meta_ismatch_keylolohi=2, 
                        meta_ismatch_keylohilo=2, 
                        meta_ismatch_keylohihi=2, 
                        meta_ismatch_keyhilolo=2,
                        meta_ismatch_keyhilohi=2,
                        meta_ismatch_keyhihilo=2,
                        meta_ismatch_keyhihihi=2,
                        meta_is_putreq_ru=0)
                self.client.access_vote_tbl_table_add_with_increase_vote(
                        self.sess_hdl, self.dev_tgt, matchspec0)
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            if (ismatch_keylololo == 2 and ismatch_keylolohi == 2 and \
                                            ismatch_keylohilo == 2 and ismatch_keylohihi == 2 and \
                                            ismatch_keyhilolo == 2 and ismatch_keyhilohi == 2 and \
                                            ismatch_keyhihilo == 2 and ismatch_keyhihihi == 2):
                                                continue
                                            for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE]:
                                                matchspec0 = netbuffer_access_vote_tbl_match_spec_t(
                                                        meta_isvalid=1,
                                                        op_hdr_optype=tmpoptype, 
                                                        meta_ismatch_keylololo=ismatch_keylololo, 
                                                        meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                        meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                        meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                        meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                        meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                        meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                        meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                        meta_is_putreq_ru=0)
                                                self.client.access_vote_tbl_table_add_with_decrease_vote(
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE]:
                                                matchspec0 = netbuffer_access_vote_tbl_match_spec_t(
                                                        meta_isvalid=0,
                                                        op_hdr_optype=tmpoptype, 
                                                        meta_ismatch_keylololo=ismatch_keylololo, 
                                                        meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                        meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                        meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                        meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                        meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                        meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                        meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                        meta_is_putreq_ru=0)
                                                self.client.access_vote_tbl_table_add_with_decrease_vote(
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
            # DELREQ (set vote as 0)
            matchspec0 = netbuffer_access_vote_tbl_match_spec_t(
                    meta_isvalid=1,
                    op_hdr_optype=DELREQ_TYPE, 
                    meta_ismatch_keylololo=2, 
                    meta_ismatch_keylolohi=2, 
                    meta_ismatch_keylohilo=2, 
                    meta_ismatch_keylohihi=2, 
                    meta_ismatch_keyhilolo=2,
                    meta_ismatch_keyhilohi=2,
                    meta_ismatch_keyhihilo=2,
                    meta_ismatch_keyhihihi=2,
                    meta_is_putreq_ru=0)
            self.client.access_vote_tbl_table_add_with_reset_vote(
                    self.sess_hdl, self.dev_tgt, matchspec0)
            # Cache update: GETRES_S and PUTREQ_RU (set vote as 1) (GETRES_NS: keep original vote)
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            for isvalid in valid_list:
                                                matchspec0 = netbuffer_access_vote_tbl_match_spec_t(
                                                        meta_isvalid=isvalid,
                                                        op_hdr_optype=GETRES_S_TYPE, 
                                                        meta_ismatch_keylololo=ismatch_keylololo, 
                                                        meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                        meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                        meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                        meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                        meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                        meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                        meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                        meta_is_putreq_ru=0)
                                                self.client.access_vote_tbl_table_add_with_init_vote(
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
                                                matchspec0 = netbuffer_access_vote_tbl_match_spec_t(
                                                        meta_isvalid=isvalid,
                                                        op_hdr_optype=PUTREQ_TYPE, 
                                                        meta_ismatch_keylololo=ismatch_keylololo, 
                                                        meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                        meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                        meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                        meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                        meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                        meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                        meta_ismatch_keyhihihi=ismatch_keyhihihi,
                                                        meta_is_putreq_ru=1)
                                                self.client.access_vote_tbl_table_add_with_init_vote(
                                                        self.sess_hdl, self.dev_tgt, matchspec0)
            
            # Table: try_res_tbl (default: nop; 3)
            print "Configuring try_res_tbl"
            matchspec0 = netbuffer_try_res_tbl_match_spec_t(\
                    op_hdr_optype=GETREQ_TYPE,
                    meta_isvalid=1,
                    meta_ismatch_keylololo=2, 
                    meta_ismatch_keylolohi=2,
                    meta_ismatch_keylohilo=2, 
                    meta_ismatch_keylohihi=2,
                    meta_ismatch_keyhilolo=2, 
                    meta_ismatch_keyhilohi=2,
                    meta_ismatch_keyhihilo=2,
                    meta_ismatch_keyhihihi=2)
            self.client.try_res_tbl_table_add_with_update_getreq_to_getres(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_try_res_tbl_match_spec_t(\
                    op_hdr_optype=PUTREQ_TYPE,
                    meta_isvalid=1,
                    meta_ismatch_keylololo=2, 
                    meta_ismatch_keylolohi=2,
                    meta_ismatch_keylohilo=2, 
                    meta_ismatch_keylohihi=2,
                    meta_ismatch_keyhilolo=2, 
                    meta_ismatch_keyhilohi=2,
                    meta_ismatch_keyhihilo=2,
                    meta_ismatch_keyhihihi=2)
            self.client.try_res_tbl_table_add_with_update_putreq_to_putres(\
                    self.sess_hdl, self.dev_tgt, matchspec1)
            matchspec2 = netbuffer_try_res_tbl_match_spec_t(\
                    op_hdr_optype=DELREQ_TYPE,
                    meta_isvalid=1,
                    meta_ismatch_keylololo=2, 
                    meta_ismatch_keylolohi=2,
                    meta_ismatch_keylohilo=2, 
                    meta_ismatch_keylohihi=2,
                    meta_ismatch_keyhilolo=2, 
                    meta_ismatch_keyhilohi=2,
                    meta_ismatch_keyhihilo=2,
                    meta_ismatch_keyhihihi=2)
            actnspec2 = netbuffer_update_delreq_to_s_and_clone_action_spec_t(sids[0]) # Clone for DELRES to client
            self.client.try_res_tbl_table_add_with_update_delreq_to_s_and_clone(\
                    self.sess_hdl, self.dev_tgt, matchspec2, actnspec2)

            # Table: access_lock_tbl (default: nop; 8)
            print "Configuring access_lock_tbl"
            for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE]:
                matchspec0 = netbuffer_access_lock_tbl_match_spec_t(
                        op_hdr_optype=tmpoptype,
                        meta_isevict=2,
                        meta_is_putreq_ru=0)
                self.client.access_lock_tbl_table_add_with_try_lock(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
            for isevict in predicate_list:
                for tmpoptype in [GETRES_S_TYPE, GETRES_NS_TYPE]:
                    matchspec1 = netbuffer_access_lock_tbl_match_spec_t(
                            op_hdr_optype=tmpoptype,
                            meta_isevict=isevict,
                            meta_is_putreq_ru=0)
                    self.client.access_lock_tbl_table_add_with_clear_lock(\
                            self.sess_hdl, self.dev_tgt, matchspec1)
                matchspec2 = netbuffer_access_lock_tbl_match_spec_t(
                        op_hdr_optype=PUTREQ_TYPE,
                        meta_isevict=isevict,
                        meta_is_putreq_ru=1)
                self.client.access_lock_tbl_table_add_with_clear_lock(\
                        self.sess_hdl, self.dev_tgt, matchspec2)

            # Table: trigger_cache_update_tbl (default: nop; 2)
            print "Configuring trigger_cache_update_tbl"
            matchspec0 = netbuffer_trigger_cache_update_tbl_match_spec_t(\
                    meta_islock=0,
                    op_hdr_optype=GETREQ_TYPE,
                    meta_isevict=2)
            self.client.trigger_cache_update_tbl_table_add_with_update_getreq(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_trigger_cache_update_tbl_match_spec_t(\
                    meta_islock=0,
                    op_hdr_optype=PUTREQ_TYPE,
                    meta_isevict=2)
            self.client.trigger_cache_update_tbl_table_add_with_update_putreq(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            # Table: port_forward_tbl (default: nop; 112)
            print "Configuring port_forward_tbl"
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for islock in lock_list:
                        if isvalid == 1 and isdirty == 1:
                            matchspec0 = netbuffer_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = GETRES_S_TYPE,
                                    meta_isvalid = 1,
                                    meta_isdirty = 1,
                                    meta_islock = islock,
                                    meta_is_putreq_ru = 0)
                            actnspec0 = netbuffer_update_getres_s_to_putreq_gs_and_clone_action_spec_t(\
                                    sids[0], self.devPorts[1]) # Clone to client port, output to server port
                            self.client.port_forward_tbl_table_add_with_update_getres_s_to_putreq_gs_and_clone(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                        else:
                            matchspec0 = netbuffer_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = GETRES_S_TYPE,
                                    meta_isvalid = isvalid,
                                    meta_isdirty = isdirty,
                                    meta_islock = islock,
                                    meta_is_putreq_ru = 0)
                            actnspec0 = netbuffer_update_getres_s_to_getres_action_spec_t(\
                                    self.devPorts[0]) # Output to client port
                            self.client.port_forward_tbl_table_add_with_update_getres_s_to_getres(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for islock in lock_list:
                        matchspec0 = netbuffer_port_forward_tbl_match_spec_t(\
                                op_hdr_optype = GETRES_NS_TYPE,
                                meta_isvalid = isvalid,
                                meta_isdirty = isdirty,
                                meta_islock = islock,
                                meta_is_putreq_ru = 0)
                        actnspec0 = netbuffer_update_getres_ns_to_getres_action_spec_t(\
                                self.devPorts[0]) # Output to client port
                        self.client.port_forward_tbl_table_add_with_update_getres_ns_to_getres(\
                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for islock in lock_list:
                        matchspec0 = netbuffer_port_forward_tbl_match_spec_t(\
                                op_hdr_optype = PUTREQ_U_TYPE,
                                meta_isvalid = isvalid,
                                meta_isdirty = isdirty,
                                meta_islock = islock,
                                meta_is_putreq_ru = 0)
                        #actnspec0 = netbuffer_recirculate_putreq_u_action_spec_t(\
                        #        self.recirPorts[1]) # Server port should have less load?
                        self.client.port_forward_tbl_table_add_with_recirculate_putreq_u(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                                #self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for islock in lock_list:
                        if isvalid == 1 and isdirty == 1:
                            matchspec0 = netbuffer_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = PUTREQ_TYPE,
                                    meta_isvalid = 1,
                                    meta_isdirty = 1,
                                    meta_islock = islock,
                                    meta_is_putreq_ru = 1)
                            actnspec0 = netbuffer_update_putreq_ru_to_ps_and_clone_action_spec_t(\
                                    sids[0], self.devPorts[1]) # Clone to client, forward to server
                            self.client.port_forward_tbl_table_add_with_update_putreq_ru_to_ps_and_clone(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
                        else:
                            matchspec0 = netbuffer_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = PUTREQ_TYPE,
                                    meta_isvalid = isvalid,
                                    meta_isdirty = isdirty,
                                    meta_islock = islock,
                                    meta_is_putreq_ru = 1)
                            actnspec0 = netbuffer_update_putreq_ru_to_putres_action_spec_t(\
                                    self.devPorts[0]) # Forward to client
                            self.client.port_forward_tbl_table_add_with_update_putreq_ru_to_putres(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE, DELREQ_TYPE]:
                        matchspec0 = netbuffer_port_forward_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                meta_isvalid = isvalid,
                                meta_isdirty = isdirty,
                                meta_islock = 1,
                                meta_is_putreq_ru = 0)
                        #actnspec0 = netbuffer_recirculate_pkt_action_spec_t(\
                        #        self.recirPorts[1]) # Server port should have less load?
                        self.client.port_forward_tbl_table_add_with_recirculate_pkt(\
                                self.sess_hdl, self.dev_tgt, matchspec0)
                                #self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for tmpoptype in [GETREQ_TYPE, PUTREQ_TYPE, DELREQ_TYPE]:
                        matchspec0 = netbuffer_port_forward_tbl_match_spec_t(\
                                op_hdr_optype = tmpoptype,
                                meta_isvalid = isvalid,
                                meta_isdirty = isdirty,
                                meta_islock = 0,
                                meta_is_putreq_ru = 0)
                        actnspec0 = netbuffer_port_forward_action_spec_t(\
                                self.devPorts[1]) # Output to server
                        self.client.port_forward_tbl_table_add_with_port_forward(\
                                self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for islock in lock_list:
                        for tmpoptype in [SCANREQ_TYPE, GETREQ_S_TYPE, DELREQ_S_TYPE]:
                            matchspec0 = netbuffer_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    meta_isvalid = isvalid,
                                    meta_isdirty = isdirty,
                                    meta_islock = islock,
                                    meta_is_putreq_ru = 0)
                            actnspec0 = netbuffer_port_forward_action_spec_t(\
                                    self.devPorts[1]) # Output to server
                            self.client.port_forward_tbl_table_add_with_port_forward(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            for isvalid in valid_list:
                for isdirty in dirty_list:
                    for islock in lock_list:
                        for tmpoptype in [GETRES_TYPE, PUTRES_TYPE, DELRES_TYPE, SCANRES_TYPE]:
                            matchspec0 = netbuffer_port_forward_tbl_match_spec_t(\
                                    op_hdr_optype = tmpoptype,
                                    meta_isvalid = isvalid,
                                    meta_isdirty = isdirty,
                                    meta_islock = islock,
                                    meta_is_putreq_ru = 0)
                            actnspec0 = netbuffer_port_forward_action_spec_t(\
                                    self.devPorts[0]) # Output to client
                            self.client.port_forward_tbl_table_add_with_port_forward(\
                                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Table: hash_partition_tbl (default: nop; server_num <= 128)
            print "Configuring hash_partition_tbl"
            hash_start = 0
            hash_range_per_server = bucket_num / server_num
            for i in range(server_num):
                if i == server_num - 1:
                    hash_end = bucket_num - 1 # if end is not included, then it is just processed by port 1111
                else:
                    hash_end = hash_start + hash_range_per_server
                matchspec0 = netbuffer_hash_partition_tbl_match_spec_t(\
                        udp_hdr_dstPort=server_port, \
                        ig_intr_md_for_tm_ucast_egress_port=self.devPorts[1], \
                        meta_hashidx_start = hash_start, \
                        meta_hashidx_end = hash_end)
                actnspec0 = netbuffer_update_dstport_action_spec_t(\
                        server_port + i)
                self.client.hash_partition_tbl_table_add_with_update_dstport(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                hash_start = hash_end

            # Table: origin_hash_partition_tbl (default: nop; server_num <= 128)
            print "Configuring origin_hash_partition_tbl"
            hash_start = 0
            hash_range_per_server = bucket_num / server_num
            for i in range(server_num):
                if i == server_num - 1:
                    hash_end = bucket_num - 1 # if end is not included, then it is just processed by port 1111
                else:
                    hash_end = hash_start + hash_range_per_server
                matchspec0 = netbuffer_origin_hash_partition_tbl_match_spec_t(\
                        udp_hdr_dstPort=server_port, \
                        ig_intr_md_for_tm_ucast_egress_port=self.devPorts[1], \
                        meta_origin_hashidx_start = hash_start, \
                        meta_origin_hashidx_end = hash_end)
                actnspec0 = netbuffer_update_dstport_action_spec_t(\
                        server_port + i)
                self.client.origin_hash_partition_tbl_table_add_with_update_dstport(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                hash_start = hash_end

            # Table: origin_hash_partition_reverse_tbl (default: nop; server_num <= 128)
            print "Configuring origin_hash_partition_reverse_tbl"
            hash_start = 0
            hash_range_per_server = bucket_num / server_num
            for i in range(server_num):
                if i == server_num - 1:
                    hash_end = bucket_num - 1 # if end is not included, then it is just processed by port 1111
                else:
                    hash_end = hash_start + hash_range_per_server
                matchspec0 = netbuffer_origin_hash_partition_reverse_tbl_match_spec_t(\
                        udp_hdr_srcPort=server_port, \
                        ig_intr_md_for_tm_ucast_egress_port=self.devPorts[1], \
                        meta_origin_hashidx_start = hash_start, \
                        meta_origin_hashidx_end = hash_end)
                actnspec0 = netbuffer_update_dstport_reverse_action_spec_t(\
                        server_port + i)
                self.client.origin_hash_partition_reverse_tbl_table_add_with_update_dstport_reverse(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                hash_start = hash_end

            # Table: drop_put_tbl (if key coherence is unnecessary)
            #print "Configuring drop_put_tbl"
            #matchspec0 = netbuffer_drop_put_tbl_match_spec_t(
            #        op_hdr_optype = PUTREQ_TYPE)
            #self.client.drop_put_tbl_table_add_with_ig_drop_unicast(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)

            # TMPDEBUG
            print "Configuring forward_to_server_tbl"
            matchspec0 = netbuffer_forward_to_server_tbl_match_spec_t(\
                    ig_intr_md_ingress_port=self.devPorts[0])
            actnspec0 = netbuffer_forward_to_server_action_spec_t(\
                    self.devPorts[1])
            self.client.forward_to_server_tbl_table_add_with_forward_to_server(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Egress pipeline

            # Table: update_macaddr_tbl (default: nop; 3)
            print "Configuring update_macaddr_tbl"
            actnspec = netbuffer_update_macaddr_action_spec_t(\
                    macAddr_to_string(src_mac), \
                    macAddr_to_string(dst_mac))
            matchspec0 = netbuffer_update_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_TYPE)
            matchspec1 = netbuffer_update_macaddr_tbl_match_spec_t(op_hdr_optype=PUTRES_TYPE)
            matchspec2 = netbuffer_update_macaddr_tbl_match_spec_t(op_hdr_optype=DELRES_TYPE)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr(\
                    self.sess_hdl, self.dev_tgt, matchspec1, actnspec)
            self.client.update_macaddr_tbl_table_add_with_update_macaddr(\
                    self.sess_hdl, self.dev_tgt, matchspec2, actnspec)

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
