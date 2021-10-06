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

# Front Panel Ports
#   List of front panel ports to use. Each front panel port has 4 channels.
#   Port 1 is broken to 1/0, 1/1, 1/2, 1/3. Test uses 2 ports.
#
#   ex: ["1/0", "1/1"]
#
fp_ports = ["2/0", "3/0"]
#CPU_PORT = 192
src_mac = "9c:69:b4:60:ef:a5"
dst_mac = "9c:69:b4:60:ef:8d"
src_ip = "10.0.0.31"
dst_ip = "10.0.0.32"

GETREQ_TYPE = 0x00
PUTREQ_TYPE = 0x01
DELREQ_TYPE = 0x02
SCANREQ_TYPE = 0x03
GETRES_TYPE = 0x04
PUTRES_TYPE = 0x05
DELRES_TYPE = 0x06
SCANRES_TYPE = 0x07
PUTREQ_S_TYPE = 0x08
DELREQ_S_TYPE = 0x09

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

            # Table: ipv4_lpm
            #print "Configuring ipv4_lpm"
            #ipv4addr0 = ipv4Addr_to_i32(src_ip)
            #matchspec0 = netbuffer_ipv4_lpm_match_spec_t(ipv4_hdr_dstAddr=ipv4addr0, ipv4_hdr_dstAddr_prefix_length=32)
            #actnspec0 = netbuffer_ipv4_forward_action_spec_t(self.devPorts[0])
            #ipv4addr1 = ipv4Addr_to_i32(dst_ip)
            #matchspec1 = netbuffer_ipv4_lpm_match_spec_t(ipv4_hdr_dstAddr=ipv4addr1, ipv4_hdr_dstAddr_prefix_length=32)
            #actnspec1 = netbuffer_ipv4_forward_action_spec_t(self.devPorts[1])
            #self.client.ipv4_lpm_table_add_with_ipv4_forward(\
            #        self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            #self.client.ipv4_lpm_table_add_with_ipv4_forward(\
            #        self.sess_hdl, self.dev_tgt, matchspec1, actnspec1)

            # Table: port_forward_tbl
            print "Configuring port_forward_tbl"
            matchspec0 = netbuffer_port_forward_tbl_match_spec_t(ig_intr_md_ingress_port=self.devPorts[0])
            actnspec0 = netbuffer_port_forward_action_spec_t(self.devPorts[1])
            self.client.port_forward_tbl_table_add_with_port_forward(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)
            matchspec1 = netbuffer_port_forward_tbl_match_spec_t(ig_intr_md_ingress_port=self.devPorts[1])
            actnspec1 = netbuffer_port_forward_action_spec_t(self.devPorts[0])
            self.client.port_forward_tbl_table_add_with_port_forward(\
                    self.sess_hdl, self.dev_tgt, matchspec1, actnspec1)

            # Table: put_port_forward_tbl
            print "Configuring put_port_forward_tbl"
            for i in range(len(self.devPorts)):
                matchspec0 = netbuffer_put_port_forward_tbl_match_spec_t(
                        ig_intr_md_ingress_port = self.devPorts[i],
                        op_hdr_optype = PUTREQ_TYPE,
                        meta_isvalid = 0)
                self.client.put_port_forward_tbl_table_add_with_droppkt(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
                matchspec0 = netbuffer_put_port_forward_tbl_match_spec_t(
                        ig_intr_md_ingress_port = self.devPorts[i],
                        op_hdr_optype = PUTREQ_S_TYPE,
                        meta_isvalid = 0)
                self.client.put_port_forward_tbl_table_add_with_droppkt(\
                        self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_put_port_forward_tbl_match_spec_t(
                    ig_intr_md_ingress_port = self.devPorts[0],
                    op_hdr_optype = PUTREQ_TYPE,
                    meta_isvalid = 1)
            actnspec1 = netbuffer_port_forward_action_spec_t(self.devPorts[1])
            self.client.put_port_forward_tbl_table_add_with_port_forward(\
                    self.sess_hdl, self.dev_tgt, matchspec1, actnspec1)
            matchspec2 = netbuffer_put_port_forward_tbl_match_spec_t(
                    ig_intr_md_ingress_port = self.devPorts[1],
                    op_hdr_optype = PUTREQ_TYPE,
                    meta_isvalid = 1)
            actnspec2 = netbuffer_port_forward_action_spec_t(self.devPorts[0])
            self.client.put_port_forward_tbl_table_add_with_port_forward(\
                    self.sess_hdl, self.dev_tgt, matchspec2, actnspec2)
            matchspec1 = netbuffer_put_port_forward_tbl_match_spec_t(
                    ig_intr_md_ingress_port = self.devPorts[0],
                    op_hdr_optype = PUTREQ_S_TYPE,
                    meta_isvalid = 1)
            actnspec1 = netbuffer_port_forward_action_spec_t(self.devPorts[1])
            self.client.put_port_forward_tbl_table_add_with_port_forward(\
                    self.sess_hdl, self.dev_tgt, matchspec1, actnspec1)
            matchspec2 = netbuffer_put_port_forward_tbl_match_spec_t(
                    ig_intr_md_ingress_port = self.devPorts[1],
                    op_hdr_optype = PUTREQ_S_TYPE,
                    meta_isvalid = 1)
            actnspec2 = netbuffer_port_forward_action_spec_t(self.devPorts[0])
            self.client.put_port_forward_tbl_table_add_with_port_forward(\
                    self.sess_hdl, self.dev_tgt, matchspec2, actnspec2)

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

            # Table: match_keylololo_tbl
            print "Configuring match_keylololo_tbl"
            matchspec0 = netbuffer_match_keylololo_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            matchspec1 = netbuffer_match_keylololo_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            matchspec2 = netbuffer_match_keylololo_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            self.client.match_keylololo_tbl_table_add_with_get_match_keylololo(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.match_keylololo_tbl_table_add_with_put_match_keylololo(\
                    self.sess_hdl, self.dev_tgt, matchspec1)
            self.client.match_keylololo_tbl_table_add_with_get_match_keylololo(\
                    self.sess_hdl, self.dev_tgt, matchspec2)

            # Table: match_keylolohi_tbl
            print "Configuring match_keylolohi_tbl"
            matchspec0 = netbuffer_match_keylolohi_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            matchspec1 = netbuffer_match_keylolohi_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            matchspec2 = netbuffer_match_keylolohi_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            self.client.match_keylolohi_tbl_table_add_with_get_match_keylolohi(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.match_keylolohi_tbl_table_add_with_put_match_keylolohi(\
                    self.sess_hdl, self.dev_tgt, matchspec1)
            self.client.match_keylolohi_tbl_table_add_with_get_match_keylolohi(\
                    self.sess_hdl, self.dev_tgt, matchspec2)

            # Table: match_keylohilo_tbl
            print "Configuring match_keylohilo_tbl"
            matchspec0 = netbuffer_match_keylohilo_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            matchspec1 = netbuffer_match_keylohilo_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            matchspec2 = netbuffer_match_keylohilo_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            self.client.match_keylohilo_tbl_table_add_with_get_match_keylohilo(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.match_keylohilo_tbl_table_add_with_put_match_keylohilo(\
                    self.sess_hdl, self.dev_tgt, matchspec1)
            self.client.match_keylohilo_tbl_table_add_with_get_match_keylohilo(\
                    self.sess_hdl, self.dev_tgt, matchspec2)

            # Table: match_keylohihi_tbl
            print "Configuring match_keylohihi_tbl"
            matchspec0 = netbuffer_match_keylohihi_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            matchspec1 = netbuffer_match_keylohihi_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            matchspec2 = netbuffer_match_keylohihi_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            self.client.match_keylohihi_tbl_table_add_with_get_match_keylohihi(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.match_keylohihi_tbl_table_add_with_put_match_keylohihi(\
                    self.sess_hdl, self.dev_tgt, matchspec1)
            self.client.match_keylohihi_tbl_table_add_with_get_match_keylohihi(\
                    self.sess_hdl, self.dev_tgt, matchspec2)

            # Table: match_keyhilolo_tbl
            print "Configuring match_keyhilolo_tbl"
            matchspec0 = netbuffer_match_keyhilolo_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            matchspec1 = netbuffer_match_keyhilolo_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            matchspec2 = netbuffer_match_keyhilolo_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            self.client.match_keyhilolo_tbl_table_add_with_get_match_keyhilolo(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.match_keyhilolo_tbl_table_add_with_put_match_keyhilolo(\
                    self.sess_hdl, self.dev_tgt, matchspec1)
            self.client.match_keyhilolo_tbl_table_add_with_get_match_keyhilolo(\
                    self.sess_hdl, self.dev_tgt, matchspec2)

            # Table: match_keyhilohi_tbl
            print "Configuring match_keyhilohi_tbl"
            matchspec0 = netbuffer_match_keyhilohi_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            matchspec1 = netbuffer_match_keyhilohi_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            matchspec2 = netbuffer_match_keyhilohi_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            self.client.match_keyhilohi_tbl_table_add_with_get_match_keyhilohi(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.match_keyhilohi_tbl_table_add_with_put_match_keyhilohi(\
                    self.sess_hdl, self.dev_tgt, matchspec1)
            self.client.match_keyhilohi_tbl_table_add_with_get_match_keyhilohi(\
                    self.sess_hdl, self.dev_tgt, matchspec2)

            # Table: match_keyhihilo_tbl
            print "Configuring match_keyhihilo_tbl"
            matchspec0 = netbuffer_match_keyhihilo_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            matchspec1 = netbuffer_match_keyhihilo_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            matchspec2 = netbuffer_match_keyhihilo_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            self.client.match_keyhihilo_tbl_table_add_with_get_match_keyhihilo(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.match_keyhihilo_tbl_table_add_with_put_match_keyhihilo(\
                    self.sess_hdl, self.dev_tgt, matchspec1)
            self.client.match_keyhihilo_tbl_table_add_with_get_match_keyhihilo(\
                    self.sess_hdl, self.dev_tgt, matchspec2)

            # Table: match_keyhihihi_tbl
            print "Configuring match_keyhihihi_tbl"
            matchspec0 = netbuffer_match_keyhihihi_tbl_match_spec_t(op_hdr_optype=GETREQ_TYPE);
            matchspec1 = netbuffer_match_keyhihihi_tbl_match_spec_t(op_hdr_optype=PUTREQ_TYPE);
            matchspec2 = netbuffer_match_keyhihihi_tbl_match_spec_t(op_hdr_optype=DELREQ_TYPE);
            self.client.match_keyhihihi_tbl_table_add_with_get_match_keyhihihi(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            self.client.match_keyhihihi_tbl_table_add_with_put_match_keyhihihi(\
                    self.sess_hdl, self.dev_tgt, matchspec1)
            self.client.match_keyhihihi_tbl_table_add_with_get_match_keyhihihi(\
                    self.sess_hdl, self.dev_tgt, matchspec2)

            # Table: access_valid_tbl
            print "Configuring access_valid_tbl"
            predicate_list = [0, 2]
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            matchspec0 = netbuffer_access_valid_tbl_match_spec_t(
                                                    op_hdr_optype=GETREQ_TYPE, 
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi)
                                            self.client.access_valid_tbl_table_add_with_get_valid(\
                                                    self.sess_hdl, self.dev_tgt, matchspec0)
            for ismatch_keylololo in predicate_list:
                for ismatch_keylolohi in predicate_list:
                    for ismatch_keylohilo in predicate_list:
                        for ismatch_keylohihi in predicate_list:
                            for ismatch_keyhilolo in predicate_list:
                                for ismatch_keyhilohi in predicate_list:
                                    for ismatch_keyhihilo in predicate_list:
                                        for ismatch_keyhihihi in predicate_list:
                                            matchspec1 = netbuffer_access_valid_tbl_match_spec_t(
                                                    op_hdr_optype=PUTREQ_TYPE, 
                                                    meta_ismatch_keylololo=ismatch_keylololo, 
                                                    meta_ismatch_keylolohi=ismatch_keylolohi, 
                                                    meta_ismatch_keylohilo=ismatch_keylohilo, 
                                                    meta_ismatch_keylohihi=ismatch_keylohihi, 
                                                    meta_ismatch_keyhilolo=ismatch_keyhilolo,
                                                    meta_ismatch_keyhilohi=ismatch_keyhilohi,
                                                    meta_ismatch_keyhihilo=ismatch_keyhihilo,
                                                    meta_ismatch_keyhihihi=ismatch_keyhihihi)
                                            self.client.access_valid_tbl_table_add_with_set_valid(\
                                                    self.sess_hdl, self.dev_tgt, matchspec1)
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

            # Table: update_vallen_tbl
            matchspec0 = netbuffer_update_vallen_tbl_match_spec_t(
                            op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallen_tbl_table_add_with_get_vallen(
                            self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec0 = netbuffer_update_vallen_tbl_match_spec_t(
                            op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallen_tbl_table_add_with_put_vallen(
                            self.sess_hdl, self.dev_tgt, matchspec0)

            #valid_list = [0, 1]
            #predicate_list = [0, 1, 2, 4, 8]
            #for isvalid in valid_list:
            #    for ismatch_keylolo in predicate_list:
            #        for ismatch_keylohi in predicate_list:
            #            for ismatch_keyhilo in predicate_list:
            #                for ismatch_keyhihi in predicate_list:
            #                    matchspec0 = netbuffer_update_vallo_tbl_match_spec_t(
            #                            op_hdr_optype=PUTREQ_TYPE, 
            #                            meta_isvalid=isvalid, 
            #                            meta_ismatch_keylolo=ismatch_keylolo, 
            #                            meta_ismatch_keylohi=ismatch_keylohi, 
            #                            meta_ismatch_keyhilo=ismatch_keyhilo,
            #                            meta_ismatch_keyhihi=ismatch_keyhihi)
            #                    self.client.update_vallo_tbl_table_add_with_put_vallo(\
            #                            self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_vallo_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE, 
            #        meta_isvalid=1, 
            #        meta_ismatch_keylolo=2, 
            #        meta_ismatch_keylohi=2, 
            #        meta_ismatch_keyhilo=2, 
            #        meta_ismatch_keyhihi=2)
            #self.client.update_vallo_tbl_table_add_with_get_vallo(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            # Table: update_vallo_tbl
            print "Configuring update_vallo1_tbl"
            matchspec0 = netbuffer_update_vallo1_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallo1_tbl_table_add_with_put_vallo1(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo1_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallo1_tbl_table_add_with_get_vallo1(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_vallo2_tbl"
            matchspec0 = netbuffer_update_vallo2_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallo2_tbl_table_add_with_put_vallo2(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo2_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallo2_tbl_table_add_with_get_vallo2(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_vallo3_tbl"
            matchspec0 = netbuffer_update_vallo3_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallo3_tbl_table_add_with_put_vallo3(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo3_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallo3_tbl_table_add_with_get_vallo3(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_vallo4_tbl"
            matchspec0 = netbuffer_update_vallo4_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallo4_tbl_table_add_with_put_vallo4(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo4_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallo4_tbl_table_add_with_get_vallo4(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_vallo5_tbl"
            matchspec0 = netbuffer_update_vallo5_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallo5_tbl_table_add_with_put_vallo5(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo5_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallo5_tbl_table_add_with_get_vallo5(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_vallo6_tbl"
            matchspec0 = netbuffer_update_vallo6_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallo6_tbl_table_add_with_put_vallo6(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo6_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallo6_tbl_table_add_with_get_vallo6(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_vallo7_tbl"
            matchspec0 = netbuffer_update_vallo7_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallo7_tbl_table_add_with_put_vallo7(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo7_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallo7_tbl_table_add_with_get_vallo7(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_vallo8_tbl"
            matchspec0 = netbuffer_update_vallo8_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallo8_tbl_table_add_with_put_vallo8(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo8_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallo8_tbl_table_add_with_get_vallo8(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_vallo9_tbl"
            matchspec0 = netbuffer_update_vallo9_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallo9_tbl_table_add_with_put_vallo9(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo9_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallo9_tbl_table_add_with_get_vallo9(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_vallo10_tbl"
            matchspec0 = netbuffer_update_vallo10_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallo10_tbl_table_add_with_put_vallo10(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo10_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallo10_tbl_table_add_with_get_vallo10(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_vallo11_tbl"
            matchspec0 = netbuffer_update_vallo11_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallo11_tbl_table_add_with_put_vallo11(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo11_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallo11_tbl_table_add_with_get_vallo11(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_vallo12_tbl"
            matchspec0 = netbuffer_update_vallo12_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_vallo12_tbl_table_add_with_put_vallo12(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_vallo12_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_vallo12_tbl_table_add_with_get_vallo12(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

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

            #valid_list = [0, 1]
            #predicate_list = [0, 1, 2, 4, 8]
            #for isvalid in valid_list:
            #    for ismatch_keylolo in predicate_list:
            #        for ismatch_keylohi in predicate_list:
            #            for ismatch_keyhilo in predicate_list:
            #                for ismatch_keyhihi in predicate_list:
            #                    matchspec0 = netbuffer_update_valhi_tbl_match_spec_t(
            #                            op_hdr_optype=PUTREQ_TYPE, 
            #                            meta_isvalid=isvalid, 
            #                            meta_ismatch_keylolo=ismatch_keylolo, 
            #                            meta_ismatch_keylohi=ismatch_keylohi, 
            #                            meta_ismatch_keyhilo=ismatch_keyhilo,
            #                            meta_ismatch_keyhihi=ismatch_keyhihi)
            #                    self.client.update_valhi_tbl_table_add_with_put_valhi(\
            #                            self.sess_hdl, self.dev_tgt, matchspec0)
            #matchspec1 = netbuffer_update_valhi_tbl_match_spec_t(
            #        op_hdr_optype=GETREQ_TYPE, 
            #        meta_isvalid=1, 
            #        meta_ismatch_keylolo=2, 
            #        meta_ismatch_keylohi=2, 
            #        meta_ismatch_keyhilo=2, 
            #        meta_ismatch_keyhihi=2)
            #self.client.update_valhi_tbl_table_add_with_get_valhi(\
            #        self.sess_hdl, self.dev_tgt, matchspec1)

            # Table: update_valhi_tbl
            print "Configuring update_valhi1_tbl"
            matchspec0 = netbuffer_update_valhi1_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_valhi1_tbl_table_add_with_put_valhi1(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_valhi1_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_valhi1_tbl_table_add_with_get_valhi1(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_valhi2_tbl"
            matchspec0 = netbuffer_update_valhi2_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_valhi2_tbl_table_add_with_put_valhi2(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_valhi2_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_valhi2_tbl_table_add_with_get_valhi2(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_valhi3_tbl"
            matchspec0 = netbuffer_update_valhi3_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_valhi3_tbl_table_add_with_put_valhi3(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_valhi3_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_valhi3_tbl_table_add_with_get_valhi3(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_valhi4_tbl"
            matchspec0 = netbuffer_update_valhi4_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_valhi4_tbl_table_add_with_put_valhi4(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_valhi4_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_valhi4_tbl_table_add_with_get_valhi4(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_valhi5_tbl"
            matchspec0 = netbuffer_update_valhi5_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_valhi5_tbl_table_add_with_put_valhi5(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_valhi5_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_valhi5_tbl_table_add_with_get_valhi5(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_valhi6_tbl"
            matchspec0 = netbuffer_update_valhi6_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_valhi6_tbl_table_add_with_put_valhi6(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_valhi6_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_valhi6_tbl_table_add_with_get_valhi6(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_valhi7_tbl"
            matchspec0 = netbuffer_update_valhi7_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_valhi7_tbl_table_add_with_put_valhi7(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_valhi7_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_valhi7_tbl_table_add_with_get_valhi7(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_valhi8_tbl"
            matchspec0 = netbuffer_update_valhi8_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_valhi8_tbl_table_add_with_put_valhi8(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_valhi8_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_valhi8_tbl_table_add_with_get_valhi8(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_valhi9_tbl"
            matchspec0 = netbuffer_update_valhi9_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_valhi9_tbl_table_add_with_put_valhi9(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_valhi9_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_valhi9_tbl_table_add_with_get_valhi9(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

            print "Configuring update_valhi10_tbl"
            matchspec0 = netbuffer_update_valhi10_tbl_match_spec_t(
                    op_hdr_optype=PUTREQ_TYPE)
            self.client.update_valhi10_tbl_table_add_with_put_valhi10(\
                    self.sess_hdl, self.dev_tgt, matchspec0)
            matchspec1 = netbuffer_update_valhi10_tbl_match_spec_t(
                    op_hdr_optype=GETREQ_TYPE)
            self.client.update_valhi10_tbl_table_add_with_get_valhi10(\
                    self.sess_hdl, self.dev_tgt, matchspec1)

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

            # Table: clone_putpkt_tbl
            print "Configuring clone_putpkt_tbl"
            matchspec0 = netbuffer_clone_putpkt_tbl_match_spec_t(ig_intr_md_ingress_port=self.devPorts[0])
            actnspec0 = netbuffer_clone_putpkt_action_spec_t(sids[0])
            self.client.clone_putpkt_tbl_table_add_with_clone_putpkt(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

            # Table: clone_delpkt_tbl
            print "Configuring clone_delpkt_tbl"
            matchspec0 = netbuffer_clone_delpkt_tbl_match_spec_t(ig_intr_md_ingress_port=self.devPorts[0])
            actnspec0 = netbuffer_clone_delpkt_action_spec_t(sids[0])
            self.client.clone_delpkt_tbl_table_add_with_clone_delpkt(\
                    self.sess_hdl, self.dev_tgt, matchspec0, actnspec0)

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
