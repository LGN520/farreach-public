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
gthreshold = int(config.get("switch", "gthreshold"))
pthreshold = int(config.get("switch", "pthreshold"))

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
PUTREQ_S_TYPE = 0x09
DELREQ_S_TYPE = 0x0a
GETRES_S_TYPE = 0x0b

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
            #                      sids[0],
            #                      self.devPorts[1],
            #                      True)
            self.mirror.mirror_session_create(self.sess_hdl, self.dev_tgt, info)

	    # Table: load_gthreshold_tbl
	    # TODO
	    #self.client.load_gthreshold_tbl_table_set_default_action_load_gthreshold(\
	    #	    self.sess_hdl, self.dev_tgt, gthreshold)
	    #self.client.load_pthreshold_tbl_table_set_default_action_load_pthreshold(\
	    #	    self.sess_hdl, self.dev_tgt, pthreshold)

            # Table: access_keylololo_tbl (default: match_keylololo)
            print "Configuring match_keylololo_tbl"
            #matchspec0 = netbuffer_access_keylololo_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            #matchspec1 = netbuffer_access_keylololo_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            #matchspec2 = netbuffer_access_keylololo_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            #self.client.access_keylololo_tbl_table_add_with_match_keylololo(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #self.client.access_keylololo_tbl_table_add_with_match_keylololo(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)
            #self.client.access_keylololo_tbl_table_add_with_match_keylololo(\
            #        self.sess_hdl, self.dev_tgt, matchspec2)
            matchspec0 = netbuffer_access_keylololo_tbl_match_spec_t(op_hdr_optype=GETRES_S_TYPE)
            self.client.access_keylololo_tbl_table_add_with_modify_keylololo(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_keylolohi_tbl (default: match_keylolohi)
            print "Configuring match_keylolohi_tbl"
            #matchspec0 = netbuffer_access_keylolohi_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            #matchspec1 = netbuffer_access_keylolohi_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            #matchspec2 = netbuffer_access_keylolohi_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            #self.client.access_keylolohi_tbl_table_add_with_match_keylolohi(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #self.client.access_keylolohi_tbl_table_add_with_match_keylolohi(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)
            #self.client.access_keylolohi_tbl_table_add_with_match_keylolohi(\
            #        self.sess_hdl, self.dev_tgt, matchspec2)
            matchspec0 = netbuffer_access_keylolohi_tbl_match_spec_t(op_hdr_optype=GETRES_S_TYPE)
            self.client.access_keylolohi_tbl_table_add_with_modify_keylolohi(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_keylohilo_tbl (default: match_keylohilo)
            print "Configuring match_keylohilo_tbl"
            #matchspec0 = netbuffer_access_keylohilo_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            #matchspec1 = netbuffer_access_keylohilo_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            #matchspec2 = netbuffer_access_keylohilo_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            #self.client.access_keylohilo_tbl_table_add_with_match_keylohilo(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #self.client.access_keylohilo_tbl_table_add_with_match_keylohilo(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)
            #self.client.access_keylohilo_tbl_table_add_with_match_keylohilo(\
            #        self.sess_hdl, self.dev_tgt, matchspec2)
            matchspec0 = netbuffer_access_keylohilo_tbl_match_spec_t(op_hdr_optype=GETRES_S_TYPE)
            self.client.access_keylohilo_tbl_table_add_with_modify_keylohilo(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_keylohihi_tbl (default: match_keylohihi)
            print "Configuring match_keylohihi_tbl"
            #matchspec0 = netbuffer_access_keylohihi_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            #matchspec1 = netbuffer_access_keylohihi_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            #matchspec2 = netbuffer_access_keylohihi_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            #self.client.access_keylohihi_tbl_table_add_with_match_keylohihi(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #self.client.access_keylohihi_tbl_table_add_with_match_keylohihi(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)
            #self.client.access_keylohihi_tbl_table_add_with_match_keylohihi(\
            #        self.sess_hdl, self.dev_tgt, matchspec2)
            matchspec0 = netbuffer_access_keylohihi_tbl_match_spec_t(op_hdr_optype=GETRES_S_TYPE)
            self.client.access_keylohihi_tbl_table_add_with_modify_keylohihi(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_keyhilolo_tbl (default: match_keyhilolo)
            print "Configuring match_keyhilolo_tbl"
            #matchspec0 = netbuffer_access_keyhilolo_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            #matchspec1 = netbuffer_access_keyhilolo_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            #matchspec2 = netbuffer_access_keyhilolo_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            #self.client.access_keyhilolo_tbl_table_add_with_match_keyhilolo(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #self.client.access_keyhilolo_tbl_table_add_with_match_keyhilolo(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)
            #self.client.access_keyhilolo_tbl_table_add_with_match_keyhilolo(\
            #        self.sess_hdl, self.dev_tgt, matchspec2)
            matchspec0 = netbuffer_access_keyhilolo_tbl_match_spec_t(op_hdr_optype=GETRES_S_TYPE)
            self.client.access_keyhilolo_tbl_table_add_with_modify_keyhilolo(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_keyhilohi_tbl (default: match_keyhilohi)
            print "Configuring match_keyhilohi_tbl"
            #matchspec0 = netbuffer_access_keyhilohi_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            #matchspec1 = netbuffer_access_keyhilohi_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            #matchspec2 = netbuffer_access_keyhilohi_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            #self.client.access_keyhilohi_tbl_table_add_with_match_keyhilohi(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #self.client.access_keyhilohi_tbl_table_add_with_match_keyhilohi(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)
            #self.client.access_keyhilohi_tbl_table_add_with_match_keyhilohi(\
            #        self.sess_hdl, self.dev_tgt, matchspec2)
            matchspec0 = netbuffer_access_keyhilohi_tbl_match_spec_t(op_hdr_optype=GETRES_S_TYPE)
            self.client.access_keyhilohi_tbl_table_add_with_modify_keyhilohi(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_keyhihilo_tbl (default: match_keyhihilo)
            print "Configuring match_keyhihilo_tbl"
            #matchspec0 = netbuffer_access_keyhihilo_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            #matchspec1 = netbuffer_access_keyhihilo_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            #matchspec2 = netbuffer_access_keyhihilo_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            #self.client.access_keyhihilo_tbl_table_add_with_match_keyhihilo(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #self.client.access_keyhihilo_tbl_table_add_with_match_keyhihilo(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)
            #self.client.access_keyhihilo_tbl_table_add_with_match_keyhihilo(\
            #        self.sess_hdl, self.dev_tgt, matchspec2)
            matchspec0 = netbuffer_access_keyhihilo_tbl_match_spec_t(op_hdr_optype=GETRES_S_TYPE)
            self.client.access_keyhihilo_tbl_table_add_with_modify_keyhihilo(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_keyhihihi_tbl (default: match_keyhihihi)
            print "Configuring match_keyhihihi_tbl"
            #matchspec0 = netbuffer_access_keyhihihi_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            #matchspec1 = netbuffer_access_keyhihihi_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            #matchspec2 = netbuffer_access_keyhihihi_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            #self.client.access_keyhihihi_tbl_table_add_with_match_keyhihihi(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            #self.client.access_keyhihihi_tbl_table_add_with_match_keyhihihi(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)
            #self.client.access_keyhihihi_tbl_table_add_with_match_keyhihihi(\
            #        self.sess_hdl, self.dev_tgt, matchspec2)
            matchspec0 = netbuffer_access_keyhihihi_tbl_match_spec_t(op_hdr_optype=GETRES_S_TYPE)
            self.client.access_keyhihihi_tbl_table_add_with_modify_keyhihihi(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_valid_tbl (default: get_valid)
            print "Configuring access_valid_tbl"
            #predicate_list = [0, 2]
            #for ismatch_keylololo in predicate_list:
            #    for ismatch_keylolohi in predicate_list:
            #        for ismatch_keylohilo in predicate_list:
            #            for ismatch_keylohihi in predicate_list:
            #                for ismatch_keyhilolo in predicate_list:
            #                    for ismatch_keyhilohi in predicate_list:
            #                        for ismatch_keyhihilo in predicate_list:
            #                            for ismatch_keyhihihi in predicate_list:
            #                                matchspec0 = netbuffer_access_valid_tbl_match_spec_t(
            #                                        op_hdr_optype=GETREQ_TYPE, 
            #                                        meta_ismatch_keylololo=ismatch_keylololo, 
            #                                        meta_ismatch_keylolohi=ismatch_keylolohi, 
            #                                        meta_ismatch_keylohilo=ismatch_keylohilo, 
            #                                        meta_ismatch_keylohihi=ismatch_keylohihi, 
            #                                        meta_ismatch_keyhilolo=ismatch_keyhilolo,
            #                                        meta_ismatch_keyhilohi=ismatch_keyhilohi,
            #                                        meta_ismatch_keyhihilo=ismatch_keyhihilo,
            #                                        meta_ismatch_keyhihihi=ismatch_keyhihihi)
            #                                self.client.access_valid_tbl_table_add_with_get_valid(\
            #                                        self.sess_hdl, self.dev_tgt, matchspec0)
            #                                matchspec1 = netbuffer_access_valid_tbl_match_spec_t(
            #                                        op_hdr_optype=PUTREQ_TYPE, 
            #                                        meta_ismatch_keylololo=ismatch_keylololo, 
            #                                        meta_ismatch_keylolohi=ismatch_keylolohi, 
            #                                        meta_ismatch_keylohilo=ismatch_keylohilo, 
            #                                        meta_ismatch_keylohihi=ismatch_keylohihi, 
            #                                        meta_ismatch_keyhilolo=ismatch_keyhilolo,
            #                                        meta_ismatch_keyhilohi=ismatch_keyhilohi,
            #                                        meta_ismatch_keyhihilo=ismatch_keyhihilo,
            #                                        meta_ismatch_keyhihihi=ismatch_keyhihihi)
            #                                self.client.access_valid_tbl_table_add_with_get_valid(\
            #                                        self.sess_hdl, self.dev_tgt, matchspec1)
            matchspec2 = netbuffer_access_valid_tbl_match_spec_t(
                    op_hdr_optype=DELREQ_TYPE, 
                    meta_ismatch_keylololo=2, 
                    meta_ismatch_keylolohi=2, 
                    meta_ismatch_keylohilo=2, 
                    meta_ismatch_keylohihi=2, 
                    meta_ismatch_keyhilolo=2, 
                    meta_ismatch_keyhilohi=2, 
                    meta_ismatch_keyhihilo=2,
                    meta_ismatch_keyhihihi=2)
            self.client.access_valid_tbl_table_add_with_clear_valid(\
                    self.sess_hdl, self.dev_tgt, matchspec2)
            predicate_list = [0, 2]
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            # NOTE: GETRES_S triggers modify_key which does not set ismatch_key but set origin_key
                                            matchspec3 = netbuffer_access_valid_tbl_match_spec_t(
                                                    op_hdr_optype=GETRES_S_TYPE, 
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi)
                                            self.client.access_valid_tbl_table_add_with_set_valid(\
                                                    self.sess_hdl, self.dev_tgt, matchspec3)

            # Table: access_dirty_tbl (default: get_dirty)
            #matchspec0 = netbuffer_access_dirty_tbl_match_spec_t(
            #                op_hdr_optype=GETREQ_TYPE)
            #self.client.access_dirty_tbl_table_add_with_get_dirty(
            #                self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec0 = netbuffer_access_dirty_tbl_match_spec_t(
            #                op_hdr_optype=PUTREQ_TYPE)
            #self.client.access_dirty_tbl_table_add_with_get_dirty(
            #                self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec0 = netbuffer_access_dirty_tbl_match_spec_t(
            #                op_hdr_optype=DELREQ_TYPE)
            #self.client.access_dirty_tbl_table_add_with_get_dirty(
            #                self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbuffer_access_dirty_tbl_match_spec_t(\
                    op_hdr_optype=GETRES_S_TYPE)
            self.client.access_dirty_tbl_table_add_with_clear_dirty(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: update_vallen_tbl (default: get_vallen)
            matchspec0 = netbuffer_update_vallen_tbl_match_spec_t(\
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
            self.client.update_vallen_tbl_table_add_with_put_vallen(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_gposvote_tbl (default: get_gposvote)
            print "Configuring access_gposvote_tbl"
	    matchspec0 = netbuffer_access_gposvote_tbl_match_spec_t(
                    meta_isvalid=1,
		    op_hdr_optype=GETREQ_TYPE, 
		    meta_ismatch_keylololo=2, 
		    meta_ismatch_keylolohi=2, 
		    meta_ismatch_keylohilo=2, 
		    meta_ismatch_keylohihi=2, 
		    meta_ismatch_keyhilolo=2,
		    meta_ismatch_keyhilohi=2,
		    meta_ismatch_keyhihilo=2,
		    meta_ismatch_keyhihihi=2)
	    self.client.access_gposvote_tbl_table_add_with_increase_gposvote(\
		    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_gnegvote_tbl (default: get_gnegvote)
	    print "Configuring access_gnegvote_tbl"
            predicate_list = [0, 2]
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
                                            matchspec0 = netbuffer_access_gnegvote_tbl_match_spec_t(
                                                    meta_isvalid=1,
                                                    op_hdr_optype=GETREQ_TYPE, 
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi)
                                            self.client.access_gnegvote_tbl_table_add_with_increase_gnegvote(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            matchspec0 = netbuffer_access_gnegvote_tbl_match_spec_t(
                                                    meta_isvalid=0,
                                                    op_hdr_optype=GETREQ_TYPE, 
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi)
                                            self.client.access_gnegvote_tbl_table_add_with_increase_gnegvote(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_pposvote_tbl (default: get_pposvote)
            print "Configuring access_pposvote_tbl"
	    matchspec0 = netbuffer_access_pposvote_tbl_match_spec_t(
                    meta_isvalid=1,
		    op_hdr_optype=PUTREQ_TYPE, 
		    meta_ismatch_keylololo=2, 
		    meta_ismatch_keylolohi=2, 
		    meta_ismatch_keylohilo=2, 
		    meta_ismatch_keylohihi=2, 
		    meta_ismatch_keyhilolo=2,
		    meta_ismatch_keyhilohi=2,
		    meta_ismatch_keyhihilo=2,
		    meta_ismatch_keyhihihi=2)
	    self.client.access_pposvote_tbl_table_add_with_increase_pposvote(\
		    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: access_pnegvote_tbl (default: get_pnegvote)
	    print "Configuring access_pnegvote_tbl"
            predicate_list = [0, 2]
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
                                            matchspec0 = netbuffer_access_pnegvote_tbl_match_spec_t(
                                                    meta_isvalid=1,
                                                    op_hdr_optype=PUTREQ_TYPE, 
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi)
                                            self.client.access_pnegvote_tbl_table_add_with_increase_pnegvote(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            matchspec0 = netbuffer_access_pnegvote_tbl_match_spec_t(
                                                    meta_isvalid=0,
                                                    op_hdr_optype=PUTREQ_TYPE, 
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi)
                                            self.client.access_pnegvote_tbl_table_add_with_increase_pnegvote(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: hash_partition_tbl
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
                actnspec0 = netbuffer_hash_partition_action_spec_t(\
                        server_port + i)
                self.client.hash_partition_tbl_table_add_with_hash_partition(\
                        self.sess_hdl, self.dev_tgt, matchspec0, 0, actnspec0)
                hash_start = hash_end

            # Table: port_forward_tbl (default: nop)
            print "Configuring port_forward_tbl"
            matchspec0 = netbuffer_port_forward_tbl_match_spec_t(ig_intr_md_ingress_port=self.devPorts[0])
            actnspec0 = netbuffer_port_forward_action_spec_t(self.devPorts[1])
            self.client.port_forward_tbl_table_add_with_port_forward(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            matchspec1 = netbuffer_port_forward_tbl_match_spec_t(ig_intr_md_ingress_port=self.devPorts[1])
            actnspec1 = netbuffer_port_forward_action_spec_t(self.devPorts[0])
            self.client.port_forward_tbl_table_add_with_port_forward(\
                    self.sess_hdl, self.dev_tgt, matchspec1, actnspec1)

            # Table: drop_put_tbl
            #print "Configuring drop_put_tbl"
            matchspec0 = netbuffer_drop_put_tbl_match_spec_t(
                    op_hdr_optype = PUTREQ_TYPE)
            self.client.drop_put_tbl_table_add_with_ig_drop_unicast(\
                    self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: drop_put_tbl
            #print "Configuring drop_put_tbl"
            #matchspec0 = netbuffer_drop_put_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE)
            #self.client.drop_put_tbl_table_add_with_droppkt(self.sess_hdl, self.dev_tgt, matchspec0)

            # Table: sendback_getres_tbl
            #print "Configuring sendback_getres_tbl"
            #matchspec0 = netbuffer_sendback_getres_tbl_match_spec_t(ig_intr_md_ingress_port=self.devPorts[0])
            #actnspec0 = netbuffer_sendback_getres_action_spec_t(\
            #        macAddr_to_string(src_mac), \
            #        macAddr_to_string(dst_mac))
            #self.client.sendback_getres_tbl_table_add_with_sendback_getres(\
            #        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Table: sendback_putres_tbl
            #print "Configuring sendback_putres_tbl"
            #matchspec0 = netbuffer_sendback_putres_tbl_match_spec_t(ig_intr_md_ingress_port=self.devPorts[0])
            #actnspec0 = netbuffer_sendback_putres_action_spec_t(\
            #        macAddr_to_string(src_mac), \
            #        macAddr_to_string(dst_mac))
            #self.client.sendback_putres_tbl_table_add_with_sendback_putres(\
            #        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Table: sendback_delres_tbl
            #print "Configuring sendback_delres_tbl"
            #matchspec0 = netbuffer_sendback_delres_tbl_match_spec_t(ig_intr_md_ingress_port=self.devPorts[0])
            #actnspec0 = netbuffer_sendback_delres_action_spec_t(\
            #        macAddr_to_string(src_mac), \
            #        macAddr_to_string(dst_mac))
            #self.client.sendback_delres_tbl_table_add_with_sendback_delres(\
            #        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Table: swap_macaddr_tbl
            print "Configuring swap_macaddr_tbl"
            actnspec = netbuffer_swap_macaddr_action_spec_t(\
                    macAddr_to_string(src_mac), \
                    macAddr_to_string(dst_mac))
            matchspec0 = netbuffer_swap_macaddr_tbl_match_spec_t(op_hdr_optype=GETRES_TYPE)
            matchspec1 = netbuffer_swap_macaddr_tbl_match_spec_t(op_hdr_optype=PUTRES_TYPE)
            matchspec2 = netbuffer_swap_macaddr_tbl_match_spec_t(op_hdr_optype=DELRES_TYPE)
            self.client.swap_macaddr_tbl_table_add_with_swap_macaddr(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec)
            self.client.swap_macaddr_tbl_table_add_with_swap_macaddr(\
                    self.sess_hdl, self.dev_tgt, matchspec1, actnspec)
            self.client.swap_macaddr_tbl_table_add_with_swap_macaddr(\
                    self.sess_hdl, self.dev_tgt, matchspec2, actnspec)

            # Start from stage 5 (after keys, valid bits, and votes (occupying entire stage 4))

            # Table: update_vallo1_tbl (default: get_vallo1)
            print "Configuring update_vallo1_tbl"
            #matchspec0 = netbuffer_update_vallo1_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE, 
            #        meta_isvalid=1, 
            #        meta_ismatch_keylololo=2, 
            #        meta_ismatch_keylolohi=2,
            #        meta_ismatch_keylohilo=2, 
            #        meta_ismatch_keylohihi=2,
            #        meta_ismatch_keyhilolo=2, 
            #        meta_ismatch_keyhilohi=2,
            #        meta_ismatch_keyhihilo=2,
            #        meta_ismatch_keyhihihi=2)
            #self.client.update_vallo_tbl_table_add_with_get_vallo1(\
            #        self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo1_tbl_match_spec_t(
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
            self.client.update_vallo_tbl_table_add_with_put_vallo1(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

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

            # Table: update_valhi1_tbl (default: get_valhi1)
            print "Configuring update_valhi1_tbl"
            #matchspec0 = netbuffer_update_valhi1_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE, 
            #        meta_isvalid=1, 
            #        meta_ismatch_keylololo=2, 
            #        meta_ismatch_keylolohi=2,
            #        meta_ismatch_keylohilo=2, 
            #        meta_ismatch_keylohihi=2,
            #        meta_ismatch_keyhilolo=2, 
            #        meta_ismatch_keyhilohi=2,
            #        meta_ismatch_keyhihilo=2,
            #        meta_ismatch_keyhihihi=2)
            self.client.update_vallo_tbl_table_add_with_get_valhi1(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_valhi1_tbl_match_spec_t(
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
            self.client.update_vallo_tbl_table_add_with_put_valhi1(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

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
            
            # Table: try_res_tbl (default: nop)
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
            self.client.try_res_tbl_table_add_with_sendback_getres(\
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
            self.client.try_res_tbl_table_add_with_sendback_putres(\
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
            actnspec2 = netbuffer_update_delreq_and_clone_action_spec_t(sids[0]) # Clone for DELRES to client
            self.client.try_res_tbl_table_add_with_update_delreq_and_clone(\
                    self.sess_hdl, self.dev_tgt, matchspec2)

            # Table: calculate_diff_tbl (default: default_diff)
            print "Configuring calculate_diff_tbl"
	    matchspec0 = netbuffer_calculate_diff_tbl_match_spec_t(
                    meta_isdirty=1,
                    op_hdr_optype=GETREQ_TYPE)
	    self.client.calculate_diff_tbl_table_add_with_gneg_ppos_diff(\
		    self.sess_hdl, self.dev_tgt, matchspec0)
	    matchspec1 = netbuffer_calculate_diff_tbl_match_spec_t(
                    meta_isdirty=1,
                    op_hdr_optype=PUTREQ_TYPE)
	    self.client.calculate_diff_tbl_table_add_with_pneg_ppos_diff(\
		    self.sess_hdl, self.dev_tgt, matchspec1)
	    matchspec2 = netbuffer_calculate_diff_tbl_match_spec_t(
                    meta_isdirty=0,
                    op_hdr_optype=GETREQ_TYPE)
	    self.client.calculate_diff_tbl_table_add_with_gneg_gpos_diff(\
		    self.sess_hdl, self.dev_tgt, matchspec2)
	    matchspec3 = netbuffer_calculate_diff_tbl_match_spec_t(
                    meta_isdirty=0,
                    op_hdr_optype=PUTREQ_TYPE)
	    self.client.calculate_diff_tbl_table_add_with_pneg_gpos_diff(\
		    self.sess_hdl, self.dev_tgt, matchspec3)

            # Table: access_lock_tbl (default: nop)
            print "Configuring access_lock_tbl"
            predicate_list = [0, 2]
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
                                            matchspec0 = netbuffer_access_lock_tbl_match_spec_t(
                                                    op_hdr_optype=GETREQ_TYPE, 
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi)
                                            self.client.access_lock_tbl_table_add_with_try_glock(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)
                                            matchspec1 = netbuffer_access_lock_tbl_match_spec_t(
                                                    op_hdr_optype=PUTREQ_TYPE, 
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi)
                                            self.client.access_lock_tbl_table_add_with_try_plock(\
                                                    self.sess_hdl, self.dev_tgt, matchspec1)

            # Table: update_getreq_tbl (default: nop)
            print "Configuring update_getreq_tbl"
            predicate_list = [0, 2]
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
                                            matchspec0 = netbuffer_update_getreq_tbl_match_spec_t(
                                                    meta_islock=0,
                                                    op_hdr_optype=GETREQ_TYPE, 
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi)
                                            self.client.update_getreq_tbl_table_add_with_update_getreq(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)

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
