import os
import time
import json
import math
from itertools import product

import logging
import ptf
import grpc
from ptf import config
import ptf.testutils as testutils

from bfruntime_client_base_tests import BfRuntimeTest
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as gc
import time
from ptf.thriftutils import *
from ptf.testutils import *
from ptf_port import *
this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from common import *
# import common
# print(this_dir,__file__)
cached_list = [0, 1]
hot_list = [0, 1]
validvalue_list = [0, 1, 3]
# validvalue_list = [0, 1, 2, 3] # If with PUTREQ_LARGE
latest_list = [0, 1]
stat_list = [0, 1]
deleted_list = [0, 1]
sampled_list = [0, 1]
lastclone_list = [0, 1]

access_val_mode_list = [0, 1, 2, 3]
is_largevalueblock_list = [0, 1]


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

# print("cmn",client_ips)
class TableConfigure(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "distreach"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        bfrt_info = self.interface.bfrt_info_get("distreach")
        
        self.target = gc.Target(device_id=0, pipe_id=0xffff)
        self.client_devports = []
        self.server_devports = []
        self.recir_devports = []
        # Initializing all tables
        self.access_cm1_tbl=bfrt_info.table_get('farreachEgress.access_cm1_tbl')
        self.access_cm2_tbl=bfrt_info.table_get('farreachEgress.access_cm2_tbl')
        self.access_cm3_tbl=bfrt_info.table_get('farreachEgress.access_cm3_tbl')
        self.access_cm4_tbl=bfrt_info.table_get('farreachEgress.access_cm4_tbl')
        self.l2l3_forward_tbl=bfrt_info.table_get('farreachIngress.l2l3_forward_tbl')
        self.access_cache_frequency_tbl=bfrt_info.table_get('farreachEgress.access_cache_frequency_tbl')
        self.access_seq_tbl=bfrt_info.table_get('farreachEgress.access_seq_tbl')
        self.access_validvalue_tbl=bfrt_info.table_get('farreachEgress.access_validvalue_tbl')
        self.is_hot_tbl=bfrt_info.table_get('farreachEgress.is_hot_tbl')
        self.hash_for_partition_tbl=bfrt_info.table_get('farreachIngress.hash_for_partition_tbl')

        self.access_latest_tbl=bfrt_info.table_get('farreachEgress.access_latest_tbl')
        self.save_client_udpport_tbl=bfrt_info.table_get('farreachEgress.save_client_udpport_tbl')
        self.hash_for_cm12_tbl=bfrt_info.table_get('farreachIngress.hash_for_cm12_tbl')
        self.hash_partition_tbl=bfrt_info.table_get('farreachIngress.hash_partition_tbl')
        
        self.access_deleted_tbl=bfrt_info.table_get('farreachEgress.access_deleted_tbl')
        self.access_savedseq_tbl=bfrt_info.table_get('farreachEgress.access_savedseq_tbl')
        self.update_vallen_tbl=bfrt_info.table_get('farreachEgress.update_vallen_tbl')
        self.cache_lookup_tbl=bfrt_info.table_get('farreachIngress.cache_lookup_tbl')
        self.update_valhi1_tbl=bfrt_info.table_get('farreachEgress.update_valhi1_tbl')
        self.update_valhi2_tbl=bfrt_info.table_get('farreachEgress.update_valhi2_tbl')
        self.update_vallo1_tbl=bfrt_info.table_get('farreachEgress.update_vallo1_tbl')
        self.update_vallo2_tbl=bfrt_info.table_get('farreachEgress.update_vallo2_tbl')
        self.hash_for_cm34_tbl=bfrt_info.table_get('farreachIngress.hash_for_cm34_tbl')
        self.hash_for_seq_tbl=bfrt_info.table_get('farreachIngress.hash_for_seq_tbl')
        self.special_ig_port_forward_tbl=bfrt_info.table_get('farreachIngress.special_ig_port_forward_tbl')
        self.update_valhi3_tbl=bfrt_info.table_get('farreachEgress.update_valhi3_tbl')
        self.update_valhi4_tbl=bfrt_info.table_get('farreachEgress.update_valhi4_tbl')
        self.update_vallo3_tbl=bfrt_info.table_get('farreachEgress.update_vallo3_tbl')
        self.update_vallo4_tbl=bfrt_info.table_get('farreachEgress.update_vallo4_tbl')
        self.update_valhi5_tbl=bfrt_info.table_get('farreachEgress.update_valhi5_tbl')
        self.update_valhi6_tbl=bfrt_info.table_get('farreachEgress.update_valhi6_tbl')
        self.update_vallo5_tbl=bfrt_info.table_get('farreachEgress.update_vallo5_tbl')
        self.update_vallo6_tbl=bfrt_info.table_get('farreachEgress.update_vallo6_tbl')
        self.lastclone_lastscansplit_tbl=bfrt_info.table_get('farreachEgress.lastclone_lastscansplit_tbl')
        self.recover_tbl = bfrt_info.table_get('farreachEgress.recover_tbl')
        self.update_valhi7_tbl=bfrt_info.table_get('farreachEgress.update_valhi7_tbl')
        self.update_valhi8_tbl=bfrt_info.table_get('farreachEgress.update_valhi8_tbl')
        self.update_vallo7_tbl=bfrt_info.table_get('farreachEgress.update_vallo7_tbl')
        self.update_vallo8_tbl=bfrt_info.table_get('farreachEgress.update_vallo8_tbl')
        self.another_eg_port_forward_tbl=bfrt_info.table_get('farreachEgress.another_eg_port_forward_tbl')
        self.update_valhi10_tbl=bfrt_info.table_get('farreachEgress.update_valhi10_tbl')
        self.update_valhi9_tbl=bfrt_info.table_get('farreachEgress.update_valhi9_tbl')
        self.update_vallo10_tbl=bfrt_info.table_get('farreachEgress.update_vallo10_tbl')
        self.update_vallo9_tbl=bfrt_info.table_get('farreachEgress.update_vallo9_tbl')
        self.eg_port_forward_tbl=bfrt_info.table_get('farreachEgress.eg_port_forward_tbl')
        self.update_valhi11_tbl=bfrt_info.table_get('farreachEgress.update_valhi11_tbl')
        self.update_valhi12_tbl=bfrt_info.table_get('farreachEgress.update_valhi12_tbl')
        self.update_vallo11_tbl=bfrt_info.table_get('farreachEgress.update_vallo11_tbl')
        self.update_vallo12_tbl=bfrt_info.table_get('farreachEgress.update_vallo12_tbl')
        self.update_ipmac_srcport_tbl=bfrt_info.table_get('farreachEgress.update_ipmac_srcport_tbl')
        self.update_valhi13_tbl=bfrt_info.table_get('farreachEgress.update_valhi13_tbl')
        self.update_valhi14_tbl=bfrt_info.table_get('farreachEgress.update_valhi14_tbl')
        self.update_vallo13_tbl=bfrt_info.table_get('farreachEgress.update_vallo13_tbl')
        self.update_vallo14_tbl=bfrt_info.table_get('farreachEgress.update_vallo14_tbl')
        self.ipv4_forward_tbl=bfrt_info.table_get('farreachIngress.ipv4_forward_tbl')
        self.prepare_for_cachehit_tbl=bfrt_info.table_get('farreachIngress.prepare_for_cachehit_tbl')
        self.add_and_remove_value_header_tbl=bfrt_info.table_get('farreachEgress.add_and_remove_value_header_tbl')
        self.forward_tbl=bfrt_info.table_get('farreachEgress.forward_tbl')
        self.update_pktlen_tbl=bfrt_info.table_get('farreachEgress.update_pktlen_tbl')
        self.update_valhi15_tbl=bfrt_info.table_get('farreachEgress.update_valhi15_tbl')
        self.update_valhi16_tbl=bfrt_info.table_get('farreachEgress.update_valhi16_tbl')
        self.update_vallo15_tbl=bfrt_info.table_get('farreachEgress.update_vallo15_tbl')
        self.update_vallo16_tbl=bfrt_info.table_get('farreachEgress.update_vallo16_tbl')
        self.ig_port_forward_tbl=bfrt_info.table_get('farreachIngress.ig_port_forward_tbl')
        self.set_hot_threshold_tbl=bfrt_info.table_get('farreachIngress.set_hot_threshold_tbl')
        self.sample_tbl=bfrt_info.table_get('farreachIngress.sample_tbl')
        self.access_largevalueseq_and_save_assignedseq_tbl=bfrt_info.table_get('farreachEgress.access_largevalueseq_and_save_assignedseq_tbl')
        self.is_largevalueblock_tbl =bfrt_info.table_get('farreachEgress.is_largevalueblock_tbl')
        self.l2l3_forward_tbl.info.key_field_annotation_add("hdr.ethernet_hdr.dstAddr", "mac")
        self.l2l3_forward_tbl.info.key_field_annotation_add("hdr.ipv4_hdr.dstAddr", "ipv4")
        self.ipv4_forward_tbl.info.key_field_annotation_add("hdr.ipv4_hdr.dstAddr", "ipv4")
        self.prepare_for_cachehit_tbl.info.key_field_annotation_add("hdr.ipv4_hdr.srcAddr","ipv4")


        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_mac", "farreachEgress.update_ipmac_srcport_server2client", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_mac", "farreachEgress.update_ipmac_srcport_server2client", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_ip", "farreachEgress.update_ipmac_srcport_server2client", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_ip", "farreachEgress.update_ipmac_srcport_server2client", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_mac", "farreachEgress.update_ipmac_srcport_switch2switchos", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("switch_mac", "farreachEgress.update_ipmac_srcport_switch2switchos", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_ip", "farreachEgress.update_ipmac_srcport_switch2switchos", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("switch_ip", "farreachEgress.update_ipmac_srcport_switch2switchos", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_mac", "farreachEgress.update_dstipmac_client2server", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_ip", "farreachEgress.update_dstipmac_client2server", "ipv4")
        
        self.special_ig_port_forward_tbl.info.data_field_annotation_add("client_mac", "farreachIngress.update_backup_to_backupack", "mac")
        self.special_ig_port_forward_tbl.info.data_field_annotation_add("reflector_mac", "farreachIngress.update_backup_to_backupack", "mac")
        self.special_ig_port_forward_tbl.info.data_field_annotation_add("client_ip", "farreachIngress.update_backup_to_backupack", "ipv4")
        self.special_ig_port_forward_tbl.info.data_field_annotation_add("reflector_ip", "farreachIngress.update_backup_to_backupack", "ipv4")
        # self.update_ipmac_srcport_tbl.info.data_field_annotation_add("switch_mac", "farreachEgress.update_dstipmac_switch2switchos", "mac")
        # self.update_ipmac_srcport_tbl.info.data_field_annotation_add("switch_ip", "farreachEgress.update_dstipmac_switch2switchos", "ipv4")
        # self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_mac", "farreachEgress.update_ipmac_srcport_client2server", "mac")
        # self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_mac", "farreachEgress.update_ipmac_srcport_client2server", "mac")
        # self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_ip", "farreachEgress.update_ipmac_srcport_client2server", "ipv4")
        # self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_ip", "farreachEgress.update_ipmac_srcport_client2server", "ipv4")
        
        # fetch port table 
        self.port_table = bfrt_info.table_get("$PORT")
        self.port_stat_table = bfrt_info.table_get("$PORT_STAT")
        self.port_hdl_info_table = bfrt_info.table_get("$PORT_HDL_INFO")
        self.port_fp_idx_info_table = bfrt_info.table_get("$PORT_FP_IDX_INFO")
        self.port_str_info_table = bfrt_info.table_get("$PORT_STR_INFO")
        # fetch mirror cfg
        mirror_cfg_table = bfrt_info.table_get("$mirror.cfg")

        for client_fpport in client_fpports:
            port, chnl = client_fpport.split("/")
            devport = int(port)
            self.port_table.entry_add(
                self.target,
                [self.port_table.make_key([gc.KeyTuple('$DEV_PORT', devport)])],
                [self.port_table.make_data([gc.DataTuple('$SPEED', str_val="BF_SPEED_40G"),
                                            gc.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                            gc.DataTuple('$PORT_ENABLE', bool_val=True)])])
            self.client_devports.append(devport)
        for server_fpport in server_fpports:
            port, chnl = server_fpport.split("/")
            devport = int(port) 
            self.port_table.entry_add(
                self.target,
                [self.port_table.make_key([gc.KeyTuple('$DEV_PORT', devport)])],
                [self.port_table.make_data([gc.DataTuple('$SPEED', str_val="BF_SPEED_40G"),
                                            gc.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                            gc.DataTuple('$PORT_ENABLE', bool_val=True)])])
            self.server_devports.append(devport)
                # get the device ports from pipeline_recirports_to/fromsingle

        # get the device ports from pipeline_recirports_to/fromsingle
        for recirport_tosingle in pipeline_recirports_tosingle:
            if recirport_tosingle is not None:
                port, chnl = recirport_tosingle.split("/")
                devport = int(port)
                self.port_table.entry_add(
                    self.target,
                    [self.port_table.make_key([gc.KeyTuple('$DEV_PORT', devport)])],
                    [self.port_table.make_data([gc.DataTuple('$SPEED', str_val="BF_SPEED_40G"),
                                                gc.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                                gc.DataTuple('$PORT_ENABLE', bool_val=True)])])
                self.recir_devports.append(devport)
        for recirport_fromsingle in pipeline_recirports_fromsingle:
            if recirport_fromsingle is not None:
                port, chnl = recirport_fromsingle.split("/")
                devport = int(port)
                self.port_table.entry_add(
                    self.target,
                    [self.port_table.make_key([gc.KeyTuple('$DEV_PORT', devport)])],
                    [self.port_table.make_data([gc.DataTuple('$SPEED', str_val="BF_SPEED_40G"),
                                                gc.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                                gc.DataTuple('$PORT_ENABLE', bool_val=True)])])
                self.recir_devports.append(devport)
        # prepare sid
        sidnum = len(self.client_devports) + len(self.server_devports)
        sids = random.sample(range(BASE_SID_NORM, MAX_SID_NORM), sidnum)
        self.client_sids = sids[0:len(self.client_devports)]
        self.server_sids = sids[len(self.client_devports):sidnum]
      
        for i in range(client_physical_num):
            print("Binding sid {} with client devport {} for both direction mirroring".format(self.client_sids[i], self.client_devports[i])) # clone to client
            key = mirror_cfg_table.make_key([gc.KeyTuple('$sid', self.client_sids[i])])
            data = mirror_cfg_table.make_data(
                [gc.DataTuple('$direction', str_val="BOTH"),
                gc.DataTuple('$ucast_egress_port', self.client_devports[i]),
                gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                gc.DataTuple('$session_enable', bool_val=True)],
                '$normal')
            mirror_cfg_table.entry_add(self.target,[key],[data])
        for i in range(server_physical_num):
            print("Binding sid {} with server devport {} for both direction mirroring".format(self.server_sids[i], self.server_devports[i])) # clone to server
            key = mirror_cfg_table.make_key([gc.KeyTuple('$sid', self.server_sids[i])])
            data = mirror_cfg_table.make_data(
                [gc.DataTuple('$direction', str_val="BOTH"),
                gc.DataTuple('$ucast_egress_port', self.server_devports[i]),
                gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                gc.DataTuple('$session_enable', bool_val=True)],
                '$normal')
            mirror_cfg_table.entry_add(self.target,[key],[data])
        # NOTE: data plane communicate with switchos by software-based reflector, which is deployed in one server machine
        isvalid = False
        for i in range(server_physical_num):
            if reflector_ip_for_switchos == server_ip_for_controller_list[i]:
                isvalid = True
                self.reflector_ip_for_switch = server_ips[i]
                self.reflector_mac_for_switch = server_macs[i]
                self.reflector_devport = self.server_devports[i]
                self.reflector_sid = self.server_sids[i] # clone to switchos (i.e., reflector at [the first] physical server)
        
        # Setting up PTF dataplane
        self.dataplane = ptf.dataplane_instance
        self.dataplane.flush()

    def configure_l2l3_forward_tbl(self):
        for i in range(client_physical_num):
            key = self.l2l3_forward_tbl.make_key([
                gc.KeyTuple('hdr.ethernet_hdr.dstAddr', client_macs[i]),
                gc.KeyTuple('hdr.ipv4_hdr.dstAddr', client_ips[i], prefix_len=32)])
            data = self.l2l3_forward_tbl.make_data([gc.DataTuple('eport', self.client_devports[i])],
                                         'farreachIngress.l2l3_forward')
            self.l2l3_forward_tbl.entry_add(self.target, [key], [data])
        for i in range(server_physical_num):
            key = self.l2l3_forward_tbl.make_key([gc.KeyTuple('hdr.ethernet_hdr.dstAddr', server_macs[i]),
                                         gc.KeyTuple('hdr.ipv4_hdr.dstAddr', server_ips[i], prefix_len=32)])
            data = self.l2l3_forward_tbl.make_data([gc.DataTuple('eport', self.server_devports[i])],
                                         'farreachIngress.l2l3_forward')
            self.l2l3_forward_tbl.entry_add(self.target, [key], [data])

    def configure_set_hot_threshold_tbl(self):
        data = self.set_hot_threshold_tbl.make_data(
            [gc.DataTuple('hot_threshold', hot_threshold)],
            'farreachIngress.set_hot_threshold')
        self.set_hot_threshold_tbl.default_entry_set(self.target, data)


    def configure_hash_for_partition_tbl(self):
        for tmpoptype in [
            GETREQ,
            CACHE_POP_INSWITCH,
            PUTREQ,
            DELREQ,
            WARMUPREQ,
            LOADREQ,
            CACHE_EVICT_LOADFREQ_INSWITCH,
            CACHE_EVICT_LOADDATA_INSWITCH,
            SETVALID_INSWITCH,
            GETRES_LATEST_SEQ,
            GETRES_DELETED_SEQ,
            PUTREQ_LARGEVALUE,
        ]:
            key = self.hash_for_partition_tbl.make_key([
                gc.KeyTuple('hdr.op_hdr.optype', tmpoptype)])
            data = self.hash_for_partition_tbl.make_data([],
                                            'farreachIngress.hash_for_partition')
            self.hash_for_partition_tbl.entry_add(self.target, [key], [data])
     
    def configure_hash_partition_tbl(self):
        hash_range_per_server = switch_partition_count / server_total_logical_num
        for tmpoptype in [
            GETREQ,
            CACHE_POP_INSWITCH,
            PUTREQ,
            DELREQ,
            WARMUPREQ,
            LOADREQ,
            CACHE_EVICT_LOADFREQ_INSWITCH,
            CACHE_EVICT_LOADDATA_INSWITCH,
            SETVALID_INSWITCH,
            GETRES_LATEST_SEQ,
            GETRES_DELETED_SEQ,
            PUTREQ_LARGEVALUE,
        ]:
            hash_start = 0 # [0, partition_count-1]
            for global_server_logical_idx in range(server_total_logical_num):
                if global_server_logical_idx == server_total_logical_num - 1:
                    hash_end = switch_partition_count - 1 # if end is not included, then it is just processed by port 1111
                else:
                    hash_end = hash_start + hash_range_per_server - 1
                # NOTE: both start and end are included
                key = self.hash_partition_tbl.make_key([
                    gc.KeyTuple('$MATCH_PRIORITY', 0),
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('meta.hashval_for_partition', low = hash_start, high = hash_end)
                ])

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
                    print("WARNING: no physical server covers global_server_logical_idx {} -> no corresponding MAT entries in hash_partition_tbl".format(global_server_logical_idx))
                else:
                    #udp_dstport = server_worker_port_start + global_server_logical_idx
                    udp_dstport = server_worker_port_start + local_server_logical_idx
                    eport = self.server_devports[server_physical_idx]
                    if tmpoptype == GETRES_LATEST_SEQ or tmpoptype == GETRES_DELETED_SEQ :
                        data = self.hash_partition_tbl.make_data([
                            gc.DataTuple('udpport', udp_dstport),
                            gc.DataTuple('eport', eport)],
                            'farreachIngress.hash_partition')
                        self.hash_partition_tbl.entry_add(self.target, [key], [data])
                    else:
                        data = self.hash_partition_tbl.make_data([
                            gc.DataTuple('eport', eport)],
                            'farreachIngress.hash_partition_for_special_response')
                        self.hash_partition_tbl.entry_add(self.target, [key], [data])
                hash_start = hash_end + 1

    def configure_hash_for_cm_tbl(self):
        for i in ["12", "34"]:
            print("Configuring hash_for_cm{}_tbl".format(i))
            for tmpoptype in [GETREQ,PUTREQ]:
                key = eval('self.hash_for_cm{}_tbl'.format(i)).make_key([gc.KeyTuple('hdr.op_hdr.optype',tmpoptype)])
                data = eval('self.hash_for_cm{}_tbl'.format(i)).make_data(
                    [],'farreachIngress.hash_for_cm{}'.format(i))
                eval('self.hash_for_cm{}_tbl'.format(i)).entry_add(self.target, [key], [data])
   
    def configure_hash_for_seq_tbl(self):
        for tmpoptype in [PUTREQ, DELREQ, PUTREQ_LARGEVALUE]:
            key = self.hash_for_seq_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype)])
            data = self.hash_for_seq_tbl.make_data([],'farreachIngress.hash_for_seq')
            self.hash_for_seq_tbl.entry_add(self.target, [key], [data])
    
    def configure_prepare_for_cachehit_tbl(self):
        for client_physical_idx in range(client_physical_num):
            tmp_clientsid = self.client_sids[client_physical_idx]
            for tmpoptype in [GETREQ, PUTREQ, DELREQ]:
                key = self.prepare_for_cachehit_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.ipv4_hdr.srcAddr', client_ips[client_physical_idx], prefix_len=32),
                    ])
                data = self.prepare_for_cachehit_tbl.make_data(
                    [gc.DataTuple('client_sid', tmp_clientsid)],
                    'farreachIngress.set_client_sid')
                self.prepare_for_cachehit_tbl.entry_add(self.target, [key], [data])

    def configure_ipv4_forward_tbl(self):
        
        for tmp_client_physical_idx in range(client_physical_num):
            eport = self.client_devports[tmp_client_physical_idx]
            tmpsid = self.client_sids[tmp_client_physical_idx]
            
            for tmpoptype in [
                GETRES_SEQ,
                PUTRES_SEQ,
                DELRES_SEQ,
                WARMUPACK,
                SCANRES_SPLIT,
                LOADACK,
                GETRES_LARGEVALUE_SEQ,
                # GETRES_LATEST_SEQ, 
                # GETRES_DELETED_SEQ
            ]:
                key = self.ipv4_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.ipv4_hdr.dstAddr', client_ips[tmp_client_physical_idx], prefix_len=32),
                    ])
                data = self.ipv4_forward_tbl.make_data(
                    [gc.DataTuple('eport', eport)],
                    'farreachIngress.forward_normal_response')
                self.ipv4_forward_tbl.entry_add(self.target, [key], [data])
            for tmpoptype in [GETRES_LATEST_SEQ, GETRES_DELETED_SEQ]:
                print(self.client_sids,tmpsid)
                key = self.ipv4_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.ipv4_hdr.dstAddr', client_ips[tmp_client_physical_idx], prefix_len=32)])
                data = self.ipv4_forward_tbl.make_data(
                    [gc.DataTuple('client_sid', tmpsid)],
                    'farreachIngress.forward_special_get_response')
                self.ipv4_forward_tbl.entry_add(self.target, [key], [data])
    def configure_sample_tbl(self):
        for tmpoptype in [GETREQ,PUTREQ]:
            key = self.sample_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    ])
            data = self.sample_tbl.make_data(
                [],
                'farreachIngress.sample')
            self.sample_tbl.entry_add(self.target, [key], [data])

    def configure_ig_port_forward_tbl(self):
        keys = []
        datas = []
        keys.append(self.ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', GETREQ),
                ]))
        datas.append(self.ig_port_forward_tbl.make_data(
            [],'farreachIngress.update_getreq_to_getreq_inswitch'))
        keys.append(self.ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', GETRES_LATEST_SEQ),
                ]))
        datas.append(self.ig_port_forward_tbl.make_data(
            [],'farreachIngress.update_getres_latest_seq_to_getres_latest_seq_inswitch'))
        keys.append(self.ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', GETRES_DELETED_SEQ),
                ]))
        datas.append(self.ig_port_forward_tbl.make_data(
            [],'farreachIngress.update_getres_deleted_seq_to_getres_deleted_seq_inswitch'))
        keys.append(self.ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', PUTREQ),
                ]))
        datas.append(self.ig_port_forward_tbl.make_data(
            [],'farreachIngress.update_putreq_to_putreq_inswitch'))
        keys.append(self.ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', DELREQ),
                ]))
        datas.append(self.ig_port_forward_tbl.make_data(
            [],'farreachIngress.update_delreq_to_delreq_inswitch'))

        keys.append(self.ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', PUTREQ_LARGEVALUE),
                ]))
        datas.append(self.ig_port_forward_tbl.make_data(
            [],'farreachIngress.update_putreq_largevalue_to_putreq_largevalue_inswitch'))
        self.ig_port_forward_tbl.entry_add(self.target, keys, datas)

    def configure_special_ig_port_forward_tbl(self):
        keys = []
        datas = []
        print(self.recir_devports)
        # update_cache_pop_inswitch_to_cache_pop_inswitch_forward;
		# update_cache_pop_inswitch_forward_to_cache_pop_inswitch;
		# update_setvalid_inswitch_to_setvalid_inswitch_forward;
		# update_setvalid_inswitch_forward_to_setvalid_inswitch;
		# update_cache_evict_inswitch_to_cache_evict_inswitch_forward;
		# update_cache_evict_inswitch_forward_to_cache_evict_inswitch;
		# forward_backup;
		# update_backup_to_backupack;
        # keys.append(self.special_ig_port_forward_tbl.make_key(
        #         [gc.KeyTuple('hdr.op_hdr.optype', BACKUP),
        #          gc.KeyTuple('ig_intr_md.ingress_port', self.reflector_devport),
        #         ]))
        # datas.append(self.special_ig_port_forward_tbl.make_data(
        #     [gc.DataTuple('eport', int(self.recir_devports[0]))],'farreachIngress.forward_backup'))
        keys.append(self.special_ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', BACKUP),
                 gc.KeyTuple('ig_intr_md.ingress_port', self.reflector_devport),
                ]))
        datas.append(self.special_ig_port_forward_tbl.make_data(
            [gc.DataTuple('eport', self.reflector_devport),
             gc.DataTuple('client_mac', client_macs[0]),
             gc.DataTuple('reflector_mac', self.reflector_mac_for_switch),
             gc.DataTuple('client_ip', client_ips[0]),
             gc.DataTuple('reflector_ip', self.reflector_ip_for_switch)],
            'farreachIngress.update_backup_to_backup'))
        keys.append(self.special_ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', BACKUPACK),
                 gc.KeyTuple('ig_intr_md.ingress_port', self.reflector_devport),
                ]))
        datas.append(self.special_ig_port_forward_tbl.make_data(
            [gc.DataTuple('eport', self.reflector_devport),
             gc.DataTuple('client_mac', client_macs[0]),
             gc.DataTuple('reflector_mac', self.reflector_mac_for_switch),
             gc.DataTuple('client_ip', client_ips[0]),
             gc.DataTuple('reflector_ip', self.reflector_ip_for_switch)],
            'farreachIngress.update_backup_to_backup'))
        self.special_ig_port_forward_tbl.entry_add(self.target, keys, datas)


    def configure_access_cm_tbl(self):
        cm_hashnum = 4
        for i in range(1, cm_hashnum + 1):
            print("Configuring access_cm{}_tbl".format(i))
            access_cm_tbl = eval('self.access_cm{}_tbl'.format(i))
            keys = []
            datas = []
            for tmpoptype in [GETREQ_INSWITCH, PUTREQ_INSWITCH]:
                keys.append(access_cm_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.inswitch_hdr.is_sampled',1),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', 0)]))
                datas.append(access_cm_tbl.make_data([],'farreachEgress.update_cm{}'.format(i)))
            access_cm_tbl.entry_add(self.target, keys, datas)

    def configure_access_cache_frequency_tbl(self):
        keys = []
        datas = []
        for tmpoptype in [GETREQ_INSWITCH, PUTREQ_INSWITCH]:
            keys.append(self.access_cache_frequency_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.inswitch_hdr.is_sampled',1),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached',1),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)]))
            datas.append(self.access_cache_frequency_tbl.make_data(
                [],
                'farreachEgress.update_cache_frequency'))
        for (is_sampled,is_cached) in product(sampled_list,cached_list):
            keys.append(self.access_cache_frequency_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', CACHE_POP_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_sampled',is_sampled),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)]))
            datas.append(self.access_cache_frequency_tbl.make_data(
                [],
                'farreachEgress.reset_cache_frequency'))
            keys.append(self.access_cache_frequency_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', CACHE_EVICT_LOADFREQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_sampled',is_sampled),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)]))
            datas.append(self.access_cache_frequency_tbl.make_data(
                [],
                'farreachEgress.get_cache_frequency'))
        self.access_cache_frequency_tbl.entry_add(self.target, keys, datas)

    def configure_access_validvalue_tbl(self):
        keys = []
        datas = []
        for tmpoptype in [
            GETREQ_INSWITCH,
            GETRES_LATEST_SEQ_INSWITCH,
            GETRES_DELETED_SEQ_INSWITCH,
            PUTREQ_INSWITCH,
            DELREQ_INSWITCH,
            PUTREQ_LARGEVALUE_INSWITCH,
        ]:
            keys.append(self.access_validvalue_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached',1),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)]))
            datas.append(self.access_validvalue_tbl.make_data(
                [],
                'farreachEgress.get_validvalue'))
        for is_cached in cached_list:
            # NOTE: set_validvalue does not change validvalue_hdr.validvalue
            keys.append(self.access_validvalue_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', SETVALID_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)]))
            datas.append(self.access_validvalue_tbl.make_data(
                [],
                'farreachEgress.set_validvalue'))
        
        key = self.access_validvalue_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', BACKUP),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])  
        keys.append(key)
        datas.append(self.access_validvalue_tbl.make_data([],'farreachEgress.get_validvalue'))
        key = self.access_validvalue_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', BACKUPACK),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',1)])  
        keys.append(key)
        datas.append(self.access_validvalue_tbl.make_data([],'farreachEgress.set_validvalue'))
        self.access_validvalue_tbl.entry_add(self.target,keys,datas)

    def configure_access_seq_tbl(self):
        keys = []
        datas = []
        for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH, PUTREQ_LARGEVALUE_INSWITCH]:
            keys.append(self.access_seq_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0)]))
            datas.append(self.access_seq_tbl.make_data([],'farreachEgress.assign_seq'))
        self.access_seq_tbl.entry_add(self.target, keys, datas)

    def configure_save_client_udpport_tbl(self):
        keys = []
        datas = []
        for tmpoptype in [GETREQ_INSWITCH, PUTREQ_INSWITCH, DELREQ_INSWITCH]:
            keys.append(self.save_client_udpport_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype)]))
            datas.append(self.save_client_udpport_tbl.make_data([],'farreachEgress.save_client_udpport'))
        self.save_client_udpport_tbl.entry_add(self.target, keys, datas)

    def configure_access_latest_tbl(self):
        keys = []
        datas = []
        for (is_cached,validvalue) in product(cached_list,validvalue_list):
            if is_cached == 1:
                keys.append(self.access_latest_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', GETREQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',0)]))
                datas.append(self.access_latest_tbl.make_data([],'farreachEgress.get_latest'))
            for tmpoptype in [GETRES_LATEST_SEQ_INSWITCH,GETRES_DELETED_SEQ_INSWITCH]:
                if is_cached == 1 and validvalue == 1:
                    keys.append(self.access_latest_tbl.make_key(
                        [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                        gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                        gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                        gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                        gc.KeyTuple('hdr.inswitch_hdr.is_found',0)]))
                    datas.append(self.access_latest_tbl.make_data([],'farreachEgress.set_and_get_latest'))
            for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH]:
                if is_cached == 1 and validvalue == 1:
                    keys.append(self.access_latest_tbl.make_key(
                        [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                        gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                        gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                        gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                        gc.KeyTuple('hdr.inswitch_hdr.is_found',0)]))
                    datas.append(self.access_latest_tbl.make_data([],'farreachEgress.set_and_get_latest'))
                elif is_cached == 1 and validvalue == 3:
                    keys.append(self.access_latest_tbl.make_key(
                        [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                        gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                        gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                        gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                        gc.KeyTuple('hdr.inswitch_hdr.is_found',0)]))
                    datas.append(self.access_latest_tbl.make_data([],'farreachEgress.reset_and_get_latest'))
            keys.append(self.access_latest_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', CACHE_POP_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)]))
            datas.append(self.access_latest_tbl.make_data([],'farreachEgress.reset_and_get_latest'))
            key = self.access_latest_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', PUTREQ_LARGEVALUE_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            if is_cached == 1 and validvalue == 1:
                keys.append(key)
                datas.append(self.access_latest_tbl.make_data([],'farreachEgress.reset_and_get_latest'))
            elif is_cached == 1 and validvalue == 3:
                keys.append(key)
                datas.append(self.access_latest_tbl.make_data([],'farreachEgress.reset_and_get_latest'))

        for validvalue in validvalue_list:
            key = self.access_latest_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', BACKUP),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])  
            keys.append(key)
            datas.append(self.access_latest_tbl.make_data([],'farreachEgress.get_latest'))
            key = self.access_latest_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', BACKUPACK),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',1)])  
            keys.append(key)
            datas.append(self.access_latest_tbl.make_data([],'farreachEgress.set_and_get_latest'))
        self.access_latest_tbl.entry_add(self.target, keys, datas)
    
    def configure_access_largevaluebseq_tbl(self):
        keys = []
        datas = []
        for (is_cached,validvalue) in product(cached_list,validvalue_list):
            key = self.access_largevalueseq_and_save_assignedseq_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', GETREQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            if is_cached == 1 and validvalue == 1:
                keys.append(key)
                datas.append(self.access_largevalueseq_and_save_assignedseq_tbl.make_data([],'farreachEgress.get_largevalueseq'))
            for tmpoptype in [
                GETRES_LATEST_SEQ_INSWITCH,
                GETRES_DELETED_SEQ_INSWITCH,
                PUTREQ_INSWITCH,
                DELREQ_INSWITCH,
            ]:
                key = self.access_largevalueseq_and_save_assignedseq_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
                if is_cached == 1 and validvalue == 1:
                    keys.append(key)
                    datas.append(self.access_largevalueseq_and_save_assignedseq_tbl.make_data([],'farreachEgress.reset_largevalueseq'))
            key = self.access_largevalueseq_and_save_assignedseq_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', CACHE_POP_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            keys.append(key)
            datas.append(self.access_largevalueseq_and_save_assignedseq_tbl.make_data([],'farreachEgress.reset_largevalueseq'))
            key = self.access_largevalueseq_and_save_assignedseq_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', PUTREQ_LARGEVALUE_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            # on-path in-switch invalidation for fragment 0 of PUTREQ_LARGEVALUE_INSWITCH
            if is_cached == 1 and validvalue == 1:
                keys.append(key)
                datas.append(self.access_largevalueseq_and_save_assignedseq_tbl.make_data([],'farreachEgress.set_largevalueseq'))
        
        for validvalue in validvalue_list:
            key = self.access_largevalueseq_and_save_assignedseq_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', BACKUP),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])  
            keys.append(key)
            datas.append(self.access_largevalueseq_and_save_assignedseq_tbl.make_data([],'farreachEgress.get_largevalueseq'))
            key = self.access_largevalueseq_and_save_assignedseq_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', BACKUPACK),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',1)])  
            keys.append(key)
            datas.append(self.access_largevalueseq_and_save_assignedseq_tbl.make_data([],'farreachEgress.set_largevalueseq'))
        self.access_largevalueseq_and_save_assignedseq_tbl.entry_add(self.target, keys, datas)

    def configure_access_largevalueseq_and_save_assignedseq_tbl(self):
        keys = []
        datas = []
        for (is_cached,validvalue) in product(cached_list,validvalue_list):
            key = self.access_largevalueseq_and_save_assignedseq_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', GETREQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            if is_cached == 1 and validvalue == 1:
                keys.append(key)
                datas.append(self.access_largevalueseq_and_save_assignedseq_tbl.make_data([],'farreachEgress.get_largevalueseq'))
            for tmpoptype in [
                GETRES_LATEST_SEQ_INSWITCH,
                GETRES_DELETED_SEQ_INSWITCH,
                PUTREQ_INSWITCH,
                DELREQ_INSWITCH,
            ]:
                key = self.access_largevalueseq_and_save_assignedseq_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
                if is_cached == 1 and validvalue == 1:
                    keys.append(key)
                    datas.append(self.access_largevalueseq_and_save_assignedseq_tbl.make_data([],'farreachEgress.reset_largevalueseq'))
            key = self.access_largevalueseq_and_save_assignedseq_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', CACHE_POP_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            keys.append(key)
            datas.append(self.access_largevalueseq_and_save_assignedseq_tbl.make_data([],'farreachEgress.reset_largevalueseq'))             
            # on-path in-switch invalidation for fragment 0 of PUTREQ_LARGEVALUE_INSWITCH
            key = self.access_largevalueseq_and_save_assignedseq_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', PUTREQ_LARGEVALUE_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            if is_cached == 1 and validvalue == 1:  
                keys.append(key)
                datas.append(self.access_largevalueseq_and_save_assignedseq_tbl.make_data([],'farreachEgress.set_largevalueseq'))
        self.access_largevalueseq_and_save_assignedseq_tbl.entry_add(self.target, keys, datas)
       
    def configure_is_largevalueblock_tbl(self):
        key = self.is_largevalueblock_tbl.make_key(
                [gc.KeyTuple('hdr.seq_hdr.largevalueseq', 0)])
        data = self.is_largevalueblock_tbl.make_data([],'NoAction')
        self.is_largevalueblock_tbl.entry_add(self.target, [key], [data])

    def configure_access_deleted_tbl(self):
        keys = []
        datas =[]
        for (is_cached,validvalue,is_latest,is_stat) in product(cached_list,validvalue_list,latest_list,stat_list):
            key = self.access_deleted_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', GETREQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.stat_hdr.stat',is_stat),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            if is_cached == 1:  
                keys.append(key)
                datas.append(self.access_deleted_tbl.make_data([],'farreachEgress.get_deleted'))
            key = self.access_deleted_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', GETRES_LATEST_SEQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.stat_hdr.stat',is_stat),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])    
            if is_cached == 1 and validvalue == 1 and is_latest == 0 and is_stat == 1:
                keys.append(key)
                datas.append(self.access_deleted_tbl.make_data([],'farreachEgress.reset_and_get_deleted'))
            key = self.access_deleted_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', GETRES_DELETED_SEQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.stat_hdr.stat',is_stat),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])  
            if is_cached == 1 and validvalue == 1 and is_latest == 0 and is_stat == 1:
                keys.append(key)
                datas.append(self.access_deleted_tbl.make_data([],'farreachEgress.set_and_get_deleted'))
            key = self.access_deleted_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', PUTREQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.stat_hdr.stat',is_stat),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])  
            if is_cached == 1 and validvalue == 1:
                keys.append(key)
                datas.append(self.access_deleted_tbl.make_data([],'farreachEgress.reset_and_get_deleted'))
            key = self.access_deleted_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', DELREQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.stat_hdr.stat',is_stat),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])  
            if is_cached == 1 and validvalue == 1:
                keys.append(key)
                datas.append(self.access_deleted_tbl.make_data([],'farreachEgress.set_and_get_deleted'))
            key = self.access_deleted_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', CACHE_POP_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.stat_hdr.stat',is_stat),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])  
            if is_stat == 1:
                keys.append(key)
                datas.append(self.access_deleted_tbl.make_data([],'farreachEgress.reset_and_get_deleted'))
            elif is_stat == 0:
                keys.append(key)
                datas.append(self.access_deleted_tbl.make_data([],'farreachEgress.set_and_get_deleted'))
            for tmpoptype in [CACHE_EVICT_LOADDATA_INSWITCH]:
                key = self.access_deleted_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    gc.KeyTuple('hdr.stat_hdr.stat',is_stat),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])  
                keys.append(key)
                datas.append(self.access_deleted_tbl.make_data([],'farreachEgress.get_deleted'))
        for (validvalue,is_latest,is_stat) in product(validvalue_list,latest_list,stat_list):
            key = self.access_deleted_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', BACKUP),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    gc.KeyTuple('hdr.stat_hdr.stat',is_stat),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])  
            keys.append(key)
            datas.append(self.access_deleted_tbl.make_data([],'farreachEgress.get_deleted'))
            key = self.access_deleted_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', BACKUPACK),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    gc.KeyTuple('hdr.stat_hdr.stat',is_stat),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',1)])  
            keys.append(key)
            datas.append(self.access_deleted_tbl.make_data([],'farreachEgress.set_and_get_deleted'))
        self.access_deleted_tbl.entry_add(self.target, keys, datas)
       
    def configure_update_vallen_tbl(self):
        keys = []
        datas =[]
        for (is_cached,validvalue,is_latest) in product(cached_list,validvalue_list,latest_list):
            key = self.update_vallen_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', GETREQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            if is_cached == 1:  
                keys.append(key)
                datas.append(self.update_vallen_tbl.make_data([],'farreachEgress.get_vallen'))
            for tmpoptype in [
                GETRES_LATEST_SEQ_INSWITCH,
                GETRES_DELETED_SEQ_INSWITCH,
            ]:
                key = self.update_vallen_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
                if is_cached == 1 and validvalue == 1 and is_latest == 0:
                    keys.append(key)
                    datas.append(self.update_vallen_tbl.make_data([],'farreachEgress.set_and_get_vallen'))
            key = self.update_vallen_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', PUTREQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            if is_cached == 1 and validvalue == 1:
                keys.append(key)
                datas.append(self.update_vallen_tbl.make_data([],'farreachEgress.set_and_get_vallen'))
            key = self.update_vallen_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', DELREQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            if is_cached == 1 and validvalue == 1:
                keys.append(key)
                datas.append(self.update_vallen_tbl.make_data([],'farreachEgress.reset_and_get_vallen'))
            key = self.update_vallen_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', CACHE_POP_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            keys.append(key)
            datas.append(self.update_vallen_tbl.make_data([],'farreachEgress.set_and_get_vallen'))
        for tmpoptype in [
            CACHE_EVICT_LOADDATA_INSWITCH,
        ]:
            key = self.update_vallen_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            keys.append(key)
            datas.append(self.update_vallen_tbl.make_data([],'farreachEgress.get_vallen'))
        for validvalue in validvalue_list:
            key = self.update_vallen_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', BACKUP),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',1),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            keys.append(key)
            datas.append(self.update_vallen_tbl.make_data([],'farreachEgress.get_vallen'))
            key = self.update_vallen_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', BACKUPACK),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',1),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',1)])
            keys.append(key)
            datas.append(self.update_vallen_tbl.make_data([],'farreachEgress.set_and_get_vallen'))
        self.update_vallen_tbl.entry_add(self.target, keys, datas)
       
    def configure_access_savedseq_tbl(self):
        keys = []
        datas =[]
        for (is_cached,validvalue,is_latest) in product(cached_list,validvalue_list,latest_list):
            key = self.access_savedseq_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', GETREQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            # For GETRES_SEQ
            # if is_cached == 1 and (validvalue == 1 or validvalue == 3) and is_latest == 1:
            # For GETRES_SEQ, GETREQ_BEINGEVICTED_RECORD, and GETREQ_LARGEVALUEBLOCK_RECORD
            if is_cached == 1:
                keys.append(key)
                datas.append(self.access_savedseq_tbl.make_data([],'farreachEgress.get_savedseq'))
            for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH]:
                key = self.access_savedseq_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
                if is_cached == 1 and validvalue == 1:
                    keys.append(key)
                    datas.append(self.access_savedseq_tbl.make_data([],'farreachEgress.set_and_get_savedseq'))
            for tmpoptype in [
                GETRES_LATEST_SEQ_INSWITCH,
                GETRES_DELETED_SEQ_INSWITCH,
            ]:
                key = self.access_savedseq_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
                if is_cached == 1 and validvalue == 1 and is_latest == 0:
                    keys.append(key)
                    datas.append(self.access_savedseq_tbl.make_data([],'farreachEgress.set_and_get_savedseq'))
            key = self.access_savedseq_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', CACHE_POP_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
            keys.append(key)
            datas.append(self.access_savedseq_tbl.make_data([],'farreachEgress.set_and_get_savedseq'))
            for tmpoptype in [
                CACHE_EVICT_LOADDATA_INSWITCH,
            ]:
                key = self.access_savedseq_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    gc.KeyTuple('hdr.inswitch_hdr.is_found',0)])
                keys.append(key)
                datas.append(self.access_savedseq_tbl.make_data([],'farreachEgress.get_savedseq'))
        self.access_savedseq_tbl.entry_add(self.target, keys, datas)

    def configure_recover_tbl(self):
        key = self.recover_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', BACKUP),
                 gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1)])
        data = self.recover_tbl.make_data([],'farreachEgress.set_is_found')
        self.recover_tbl.entry_add(self.target, [key], [data])

    def configure_another_eg_port_forward_tbl_largevalueblock(self):
        # Table: another_eg_port_forward_tbl (default: NoAction	; size = 296)
        keys =[]
        datas =[]
        tmp_client_sids = [0] + self.client_sids
        for(is_cached,is_hot,validvalue,is_latest,is_largevalueblock,is_deleted,tmp_client_sid,is_lastclone_for_pktloss) in product(cached_list,hot_list,validvalue_list,latest_list,is_largevalueblock_list,deleted_list,tmp_client_sids,lastclone_list):
            tmpstat = 0 if is_deleted == 1 else 1 # Use tmpstat as action data to reduce action number
            # NOTE: eg_intr_md.egress_port is read-only
            # for is_wrong_pipeline in pipeline_list:
            # for tmp_client_sid in self.sids:
            
            if (
                is_lastclone_for_pktloss == 0
                and tmp_client_sid != 0
            ):
                # size = 128*client_physical_num=256 < 128*8=1024
                key = self.another_eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',GETREQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ])
                if is_cached == 0:
                    if is_hot == 1:
                        # Update GETREQ_INSWITCH as GETREQ_POP to server
                        data = self.another_eg_port_forward_tbl.make_data(
                            [],
                            'farreachEgress.update_getreq_inswitch_to_getreq_pop')
                        keys.append(key)
                        datas.append(data)
                    else:
                        # Update GETREQ_INSWITCH as GETREQ to server
                        data = self.another_eg_port_forward_tbl.make_data(
                            [],
                            'farreachEgress.update_getreq_inswitch_to_getreq')
                        keys.append(key)
                        datas.append(data)
                else:
                    if validvalue == 0:
                        # Update GETREQ_INSWITCH as GETREQ to server
                        data = self.another_eg_port_forward_tbl.make_data(
                            [],
                            'farreachEgress.update_getreq_inswitch_to_getreq')
                        keys.append(key)
                        datas.append(data)
                    elif validvalue == 1:
                        if is_latest == 0:
                            if is_largevalueblock == 1:
                                ## Update GETREQ_INSWITCH as GETREQ_LARGEVALUEBLOCK_SEQ to server
                                # Update GETREQ_INSWITCH as GETREQ_LARGEVALUEBLOCK_RECORD to server
                                data = self.another_eg_port_forward_tbl.make_data(
                                    [gc.DataTuple('stat',tmpstat)],
                                    'farreachEgress.update_getreq_inswitch_to_getreq_largevalueblock_record')
                                keys.append(key)
                                datas.append(data)
                            elif is_largevalueblock == 0:
                                # Update GETREQ_INSWITCH as GETREQ_NLATEST to server
                                data = self.another_eg_port_forward_tbl.make_data(
                                    [],
                                    'farreachEgress.update_getreq_inswitch_to_getreq_nlatest')
                                keys.append(key)
                                datas.append(data)
                        else:
                            # Update GETREQ_INSWITCH as GETRES_SEQ to client by mirroring
                            data = self.another_eg_port_forward_tbl.make_data(
                                [gc.DataTuple('client_sid',tmp_client_sid),
                                gc.DataTuple('server_port',server_worker_port_start),
                                gc.DataTuple('stat',tmpstat)],
                            'farreachEgress.update_getreq_inswitch_to_getres_seq_by_mirroring')
                            keys.append(key)
                            datas.append(data)
                    elif validvalue == 3:
                        if is_latest == 0:
                            ## Update GETREQ_INSWITCH as GETREQ_BEINGEVICTED to server
                            # Update GETREQ_INSWITCH as GETREQ_BEINGEVICTED_RECORD to server
                            data = self.another_eg_port_forward_tbl.make_data(
                                [gc.DataTuple('stat',tmpstat)],
                                'farreachEgress.update_getreq_inswitch_to_getreq_beingevicted_record')
                            keys.append(key)
                            datas.append(data)
     
                        else:
                            # Update GETREQ_INSWITCH as GETRES_SEQ to client by mirroring
                            data = self.another_eg_port_forward_tbl.make_data(
                                [gc.DataTuple('client_sid',tmp_client_sid),
                                gc.DataTuple('server_port',server_worker_port_start),
                                gc.DataTuple('stat',tmpstat)],
                            'farreachEgress.update_getreq_inswitch_to_getres_seq_by_mirroring')
                            keys.append(key)
                            datas.append(data)
            
            if (
                is_cached == 0
                and is_hot == 0
                and validvalue == 0
                and is_latest == 0
                and is_largevalueblock == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
            ):
                key = self.another_eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',GETRES_LATEST_SEQ),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ])
                data = self.another_eg_port_forward_tbl.make_data(
                    [],
                    'farreachEgress.update_getres_latest_seq_to_getres_seq')
                keys.append(key)
                datas.append(data)


            if (
                is_cached == 0
                and is_hot == 0
                and validvalue == 0
                and is_latest == 0
                and is_largevalueblock == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
            ):
                key = self.another_eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',GETRES_DELETED_SEQ),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ])
                # TODO: check if we need to set egress port for packet cloned by clone_i2e
                # Update GETRES_DELETED_SEQ (by clone_i2e) as GETRES_SEQ to client
                data = self.another_eg_port_forward_tbl.make_data(
                    [],
                    'farreachEgress.update_getres_deleted_seq_to_getres_seq')
                keys.append(key)
                datas.append(data)

            # is_hot (cm_predicate=1), is_wrong_pipeline (not need range/hash partition), tmp_client_sid=0 (not need mirroring for res), is_lastclone_for_pktloss should be 0 for GETRES_DELETED_SEQ_INSWITCH
            # size = 128 -> 2
            # if is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
            if (
                is_hot == 0
                and is_largevalueblock == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
            ):
                key = self.another_eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',GETRES_DELETED_SEQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ])


            if (
                is_hot == 0
                and is_largevalueblock == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
            ):
                key = self.another_eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',PUTREQ_LARGEVALUE_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ]) 
                if (
                    is_cached == 1
                    and validvalue == 3
                ):
                    # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED to server
                    data = self.another_eg_port_forward_tbl.make_data(
                        [],
                        'farreachEgress.update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_beingevicted')
                    keys.append(key)
                    datas.append(data)

                else:
                    data = self.another_eg_port_forward_tbl.make_data(
                        [],
                        'farreachEgress.update_putreq_largevalue_inswitch_to_putreq_largevalue_seq')
                    keys.append(key)
                    datas.append(data)                        

        self.another_eg_port_forward_tbl.entry_add(self.target, keys, datas)

    def configure_eg_port_forward_tbl(self):
        # Table: eg_port_forward_tbl (default: NoAction	; size = 27+852*client_physical_num=27+852*2=1731 < 2048 < 27+852*8=6843 < 8192)
        keys = []
        datas = []
        tmp_client_sids = [0] + self.client_sids
        for(is_cached,is_hot,validvalue,is_latest,is_deleted,tmp_client_sid,is_lastclone_for_pktloss) in product(cached_list,hot_list,validvalue_list,latest_list,deleted_list,tmp_client_sids,lastclone_list):
            tmpstat = 0 if is_deleted == 1 else 1 # Use tmpstat as action data to reduce action number

            if (
                is_cached == 0
                and is_hot == 0
                and validvalue == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',CACHE_POP_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    # gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ])

                data = self.eg_port_forward_tbl.make_data(
                    [gc.DataTuple('switchos_sid',self.reflector_sid),
                    gc.DataTuple('reflector_port',reflector_dp2cpserver_port)],
                    'farreachEgress.update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone')
                keys.append(key)
                datas.append(data)
                # Update CACHE_POP_INSWITCH as CACHE_POP_INSWITCH_ACK to reflector (deprecated: w/ clone)

            if (
                is_lastclone_for_pktloss == 0
                and tmp_client_sid != 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',PUTREQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    # gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ])

                if is_cached == 0:
                    if is_hot == 1:
                        # Update PUTREQ_INSWITCH as PUTREQ_POP_SEQ to server
                        data = self.eg_port_forward_tbl.make_data(
                            [],
                            'farreachEgress.update_putreq_inswitch_to_putreq_pop_seq')
                        keys.append(key)
                        datas.append(data) 
                        
                    elif is_hot == 0:
                        # Update PUTREQ_INSWITCH as PUTREQ_SEQ to server
                        data = self.eg_port_forward_tbl.make_data(
                            [],
                            'farreachEgress.update_putreq_inswitch_to_putreq_seq')
                        keys.append(key)
                        datas.append(data) 
                elif is_cached == 1:
                    if validvalue == 0:
                        # Update PUTREQ_INSWITCH as PUTREQ_SEQ to server
                        data = self.eg_port_forward_tbl.make_data(
                            [],
                            'farreachEgress.update_putreq_inswitch_to_putreq_seq')
                        keys.append(key)
                        datas.append(data)
                    elif validvalue == 3:
                        # Update PUTREQ_INSWITCH as PUTREQ_SEQ_BEINGEVICTED to server
                        data = self.eg_port_forward_tbl.make_data(
                            [],
                            'farreachEgress.update_putreq_inswitch_to_putreq_seq_beingevicted')
                        keys.append(key)
                        datas.append(data)  
                    elif validvalue == 1:
                        data = self.eg_port_forward_tbl.make_data(
                            [gc.DataTuple('client_sid',tmp_client_sid),
                            gc.DataTuple('server_port',server_worker_port_start)],
                            'farreachEgress.update_putreq_inswitch_to_putres_seq_by_mirroring')
                        keys.append(key)
                        datas.append(data)                            
 
            if (
                is_hot == 0
                and is_lastclone_for_pktloss == 0
                and tmp_client_sid != 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',DELREQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    # gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ])
                if is_cached == 0:

                    # Update DELREQ_INSWITCH as DELREQ_SEQ to server
                    data = self.eg_port_forward_tbl.make_data(
                        [],
                        'farreachEgress.update_delreq_inswitch_to_delreq_seq')
                    keys.append(key)
                    datas.append(data)    
                elif is_cached == 1:
                    if validvalue == 0:

                        # Update DELREQ_INSWITCH as DELREQ_SEQ to server
                        data = self.eg_port_forward_tbl.make_data(
                            [],
                            'farreachEgress.update_delreq_inswitch_to_delreq_seq')
                        keys.append(key)
                        datas.append(data) 
                    elif validvalue == 3:

                        # Update DELREQ_INSWITCH as DELREQ_SEQ_BEINGEVICTED to server
                        data = self.eg_port_forward_tbl.make_data(
                            [],
                            'farreachEgress.update_delreq_inswitch_to_delreq_seq_beingevicted')
                        keys.append(key)
                        datas.append(data)

                    elif validvalue == 1:
                        # Update DELREQ_INSWITCH as DELRES_SEQ to client by mirroring
                        data = self.eg_port_forward_tbl.make_data(
                            [gc.DataTuple('client_sid',tmp_client_sid),
                            gc.DataTuple('server_port',server_worker_port_start)],
                            'farreachEgress.update_delreq_inswitch_to_delres_seq_by_mirroring')
                        keys.append(key)
                        datas.append(data)                            

            # size = 1
            if (
                is_cached == 1
                and is_hot == 0
                and validvalue == 0
                and is_latest == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',CACHE_EVICT_LOADFREQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    # gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ])            

                # Update CACHE_EVICT_LOADFREQ_INSWITCH as CACHE_EVICT_LOADFREQ_INSWITCH_ACK to reflector (w/ frequency)
                data = self.eg_port_forward_tbl.make_data(
                    [gc.DataTuple('switchos_sid',self.reflector_sid),
                    gc.DataTuple('reflector_port',reflector_dp2cpserver_port)],
                    'farreachEgress.update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone')
                keys.append(key)
                datas.append(data)        


            if (
                is_cached == 0
                and is_hot == 0
                and validvalue == 0
                and is_latest == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',CACHE_EVICT_LOADFREQ_INSWITCH_ACK),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    # gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ])       
                # Forward CACHE_EVICT_LOADFREQ_INSWITCH_ACK (by clone_e2e) to reflector
                # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                pass

            # size = 2
            if (
                is_cached == 1
                and is_hot == 0
                and validvalue == 0
                and is_latest == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',CACHE_EVICT_LOADDATA_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    # gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ])    

                # Update CACHE_EVICT_LOADDATA_INSWITCH as CACHE_EVICT_LOADDATA_INSWITCH_ACK to reflector
                data = self.eg_port_forward_tbl.make_data(
                    [gc.DataTuple('switchos_sid',self.reflector_sid),
                    gc.DataTuple('reflector_port',reflector_dp2cpserver_port),
                    gc.DataTuple('stat',tmpstat)],
                    'farreachEgress.update_cache_evict_loaddata_inswitch_to_cache_evict_loaddata_inswitch_ack_drop_and_clone')
                keys.append(key)
                datas.append(data)        

            if (
                is_cached == 0
                and is_hot == 0
                and validvalue == 0
                and is_latest == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',CACHE_EVICT_LOADDATA_INSWITCH_ACK),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    # gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ])    

                # Forward CACHE_EVICT_LOADDATA_INSWITCH_ACK (by clone_e2e) to reflector
                # NOTE: default action is NoAction	 -> forward the packet to sid set by clone_e2e
                pass

            if (
                is_hot == 0
                and is_latest == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',SETVALID_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('hdr.validvalue_hdr.validvalue',validvalue),
                    gc.KeyTuple('hdr.inswitch_hdr.is_latest',is_latest),
                    # gc.KeyTuple('meta.is_largevalueblock',is_largevalueblock),
                    gc.KeyTuple('hdr.inswitch_hdr.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    
                    ])   

                # Update SETVALID_INSWITCH as SETVALID_INSWITCH_ACK to reflector
                data = self.eg_port_forward_tbl.make_data(
                    [gc.DataTuple('switchos_sid',self.reflector_sid),
                    gc.DataTuple('reflector_port',reflector_dp2cpserver_port)],
                    'farreachEgress.update_setvalid_inswitch_to_setvalid_inswitch_ack_drop_and_clone')
                keys.append(key)
                datas.append(data)  


        self.eg_port_forward_tbl.entry_add(self.target, keys, datas)

    def configure_update_pktlen_tbl(self):
        keys = [] 
        datas = []
        for i in range(int(int(switch_max_vallen / 8 + 1))):  # i from 0 to 16
            if i == 0:
                vallen_start = 0
                vallen_end = 0
                aligned_vallen = 0
            else:
                vallen_start = (i - 1) * 8 + 1  # 1, 9, ..., 121
                vallen_end = (i - 1) * 8 + 8  # 8, 16, ..., 128
                aligned_vallen = vallen_end  # 8, 16, ..., 128
            val_stat_seq_udplen = aligned_vallen + 42
            val_stat_seq_iplen = aligned_vallen + 62
            val_seq_inswitch_stat_clone_udplen = aligned_vallen + 66
            val_seq_inswitch_stat_clone_iplen = aligned_vallen + 86
            val_seq_udplen = aligned_vallen + 38
            val_seq_iplen = aligned_vallen + 58
            val_seq_stat_udplen = aligned_vallen + 42
            val_seq_stat_iplen = aligned_vallen + 62
            val_seq_inswitch_stat_udplen = aligned_vallen + 62
            val_seq_inswitch_stat_iplen = aligned_vallen + 82
            backup_udplen = aligned_vallen + 63
            backup_iplen= aligned_vallen + 83
            for tmpoptype in [
                BACKUP,
                BACKUPACK
            ]:
                keys.append(self.update_pktlen_tbl.make_key([
                    gc.KeyTuple('$MATCH_PRIORITY', 0),
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.vallen_hdr.vallen', low = vallen_start, high = vallen_end)
                ]))
                datas.append(self.update_pktlen_tbl.make_data(
                    [gc.DataTuple('udplen', backup_udplen),
                    gc.DataTuple('iplen', backup_iplen)],
                    'farreachEgress.update_pktlen'))
                
            for tmpoptype in [
                GETRES_SEQ,
                GETREQ_BEINGEVICTED_RECORD,
                GETREQ_LARGEVALUEBLOCK_RECORD,
            ]:
                keys.append(self.update_pktlen_tbl.make_key([
                    gc.KeyTuple('$MATCH_PRIORITY', 0),
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.vallen_hdr.vallen', low = vallen_start, high = vallen_end)
                ]))
                datas.append(self.update_pktlen_tbl.make_data(
                    [gc.DataTuple('udplen', val_stat_seq_udplen),
                    gc.DataTuple('iplen', val_stat_seq_iplen)],
                    'farreachEgress.update_pktlen'))


            for tmpoptype in [
                PUTREQ_SEQ,
                PUTREQ_POP_SEQ,
                PUTREQ_SEQ_BEINGEVICTED,
            ]:
                keys.append(self.update_pktlen_tbl.make_key([
                    gc.KeyTuple('$MATCH_PRIORITY', 0),
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.vallen_hdr.vallen', low = vallen_start, high = vallen_end)
                ]))
                datas.append(self.update_pktlen_tbl.make_data(
                    [gc.DataTuple('udplen', val_seq_udplen),
                    gc.DataTuple('iplen', val_seq_iplen)],
                    'farreachEgress.update_pktlen'))
            keys.append(self.update_pktlen_tbl.make_key([
                gc.KeyTuple('$MATCH_PRIORITY', 0),
                gc.KeyTuple('hdr.op_hdr.optype', CACHE_EVICT_LOADDATA_INSWITCH_ACK),
                gc.KeyTuple('hdr.vallen_hdr.vallen', low = vallen_start, high = vallen_end)
            ]))
            datas.append(self.update_pktlen_tbl.make_data(
                [gc.DataTuple('udplen', val_seq_stat_udplen),
                gc.DataTuple('iplen', val_seq_stat_iplen)],
                'farreachEgress.update_pktlen'))


        onlyop_udplen = 26
        onlyop_iplen = 46
        seq_stat_udplen = 40
        seq_stat_iplen = 60
        seq_udplen = 36
        seq_iplen = 56
        scanreqsplit_udplen = 49
        scanreqsplit_iplen = 69
        frequency_udplen = 30
        frequency_iplen = 50
        keys.append(self.update_pktlen_tbl.make_key([
            gc.KeyTuple('$MATCH_PRIORITY', 0),
            gc.KeyTuple('hdr.op_hdr.optype', CACHE_POP_INSWITCH_ACK),
            gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = switch_max_vallen)
        ]))
        datas.append(self.update_pktlen_tbl.make_data(
            [gc.DataTuple('udplen', onlyop_udplen),
            gc.DataTuple('iplen', onlyop_iplen)],
            'farreachEgress.update_pktlen'))

        for tmpoptype in [PUTRES_SEQ, DELRES_SEQ]:
            keys.append(self.update_pktlen_tbl.make_key([
                gc.KeyTuple('$MATCH_PRIORITY', 0),
                gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = switch_max_vallen)
            ]))
            datas.append(self.update_pktlen_tbl.make_data(
                [gc.DataTuple('udplen', seq_stat_udplen),
                gc.DataTuple('iplen', seq_stat_iplen)],
                'farreachEgress.update_pktlen'))
        # , GETREQ_LARGEVALUEBLOCK_SEQ
        for tmpoptype in [
            DELREQ_SEQ,
            DELREQ_SEQ_CASE3,
            DELREQ_SEQ_BEINGEVICTED,
            DELREQ_SEQ_CASE3_BEINGEVICTED,
        ]:
            keys.append(self.update_pktlen_tbl.make_key([
                gc.KeyTuple('$MATCH_PRIORITY', 0),
                gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = switch_max_vallen)
            ]))
            datas.append(self.update_pktlen_tbl.make_data(
                [gc.DataTuple('udplen', seq_udplen),
                gc.DataTuple('iplen', seq_iplen)],
                'farreachEgress.update_pktlen'))
        keys.append(self.update_pktlen_tbl.make_key([
            gc.KeyTuple('$MATCH_PRIORITY', 0),
            gc.KeyTuple('hdr.op_hdr.optype', SCANREQ_SPLIT),
            gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = switch_max_vallen)
        ]))
        datas.append(self.update_pktlen_tbl.make_data(
            [gc.DataTuple('udplen', scanreqsplit_udplen),
            gc.DataTuple('iplen', scanreqsplit_iplen)],
            'farreachEgress.update_pktlen'))
        keys.append(self.update_pktlen_tbl.make_key([
            gc.KeyTuple('$MATCH_PRIORITY', 0),
            gc.KeyTuple('hdr.op_hdr.optype', CACHE_EVICT_LOADFREQ_INSWITCH_ACK),
            gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = switch_max_vallen)
        ]))
        datas.append(self.update_pktlen_tbl.make_data(
            [gc.DataTuple('udplen', frequency_udplen),
            gc.DataTuple('iplen', frequency_iplen)],
            'farreachEgress.update_pktlen'))
        keys.append(self.update_pktlen_tbl.make_key([
            gc.KeyTuple('$MATCH_PRIORITY', 0),
            gc.KeyTuple('hdr.op_hdr.optype', SETVALID_INSWITCH_ACK),
            gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = switch_max_vallen)
        ]))
        datas.append(self.update_pktlen_tbl.make_data(
            [gc.DataTuple('udplen', onlyop_udplen),
            gc.DataTuple('iplen', onlyop_iplen)],
            'farreachEgress.update_pktlen'))

        # For large value
        shadowtype_seq_udp_delta = 10
        shadowtype_seq_ip_delta = 10
        for tmpoptype in [
            PUTREQ_LARGEVALUE_SEQ,
            PUTREQ_LARGEVALUE_SEQ_CASE3,
            PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED,
            PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED,
        ]:
            keys.append(self.update_pktlen_tbl.make_key([
                gc.KeyTuple('$MATCH_PRIORITY', 0),
                gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = 65535)
            ])) # [0, 128]
            datas.append(self.update_pktlen_tbl.make_data(
                [gc.DataTuple('udplen_delta', shadowtype_seq_udp_delta),
                gc.DataTuple('iplen_delta', shadowtype_seq_ip_delta)],
                'farreachEgress.add_pktlen'))# 0 is priority (range may be overlapping)
        self.update_pktlen_tbl.entry_add(self.target, keys, datas)

    def configure_update_ipmac_srcport_tbl(self):
        keys = []
        datas = []
        # (1) for response from server to client, egress port has been set based on ip.dstaddr (or by clone_i2e) in ingress pipeline
        # (2) for response from switch to client, egress port has been set by clone_e2e in egress pipeline
        for tmp_client_physical_idx in range(client_physical_num):
            tmp_devport = self.client_devports[tmp_client_physical_idx]
            tmp_client_mac = client_macs[tmp_client_physical_idx]
            tmp_client_ip = client_ips[tmp_client_physical_idx]
            tmp_server_mac = server_macs[0]
            tmp_server_ip = server_ips[0]
            data_without_action = [gc.DataTuple('client_mac', tmp_client_mac),
                gc.DataTuple('server_mac', tmp_server_mac),
                gc.DataTuple('client_ip', tmp_client_ip),
                gc.DataTuple('server_ip', tmp_server_ip),
                gc.DataTuple('server_port', server_worker_port_start)]

            # GETRES, GETRES_LARGEVALUE, PUTRES, DELRES
            for tmpoptype in [
                GETRES_SEQ,
                PUTRES_SEQ,
                DELRES_SEQ,
                SCANRES_SPLIT,
                WARMUPACK,
                LOADACK,
                GETRES_LARGEVALUE_SEQ,
            ]:
                keys.append(self.update_ipmac_srcport_tbl.make_key([
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('eg_intr_md.egress_port', tmp_devport)
                ]))
                datas.append(self.update_ipmac_srcport_tbl.make_data(
                    data_without_action,
                    'farreachEgress.update_ipmac_srcport_server2client'))

        # for request from client to server, egress port has been set by partition_tbl in ingress pipeline
        for tmp_server_physical_idx in range(server_physical_num):
            tmp_devport = self.server_devports[tmp_server_physical_idx]
            tmp_server_mac = server_macs[tmp_server_physical_idx]
            tmp_server_ip = server_ips[tmp_server_physical_idx]
            data_without_action =  [gc.DataTuple('server_mac', tmp_server_mac),
                gc.DataTuple('server_ip', tmp_server_ip)]
            # , GETREQ_BEINGEVICTED, GETREQ_LARGEVALUEBLOCK_SEQ
            for tmpoptype in [
                GETREQ,
                GETREQ_NLATEST,
                PUTREQ_SEQ,
                DELREQ_SEQ,
                SCANREQ_SPLIT,
                GETREQ_POP,
                PUTREQ_POP_SEQ,
                PUTREQ_SEQ_CASE3,
                PUTREQ_POP_SEQ_CASE3,
                DELREQ_SEQ_CASE3,
                WARMUPREQ,
                LOADREQ,
                PUTREQ_LARGEVALUE_SEQ,
                PUTREQ_LARGEVALUE_SEQ_CASE3,
                PUTREQ_SEQ_BEINGEVICTED,
                PUTREQ_SEQ_CASE3_BEINGEVICTED,
                DELREQ_SEQ_BEINGEVICTED,
                DELREQ_SEQ_CASE3_BEINGEVICTED,
                PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED,
                PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED,
                GETREQ_BEINGEVICTED_RECORD,
                GETREQ_LARGEVALUEBLOCK_RECORD,
            ]:
                keys.append(self.update_ipmac_srcport_tbl.make_key([
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('eg_intr_md.egress_port', tmp_devport)
                ]))
                datas.append(self.update_ipmac_srcport_tbl.make_data(
                    data_without_action,
                    'farreachEgress.update_dstipmac_client2server'))
        # Here we use server_mac/ip to simulate reflector_mac/ip = switchos_mac/ip
 # (3) eg_intr_md.egress_port of the first ACK for cache population/eviction is set by partition_tbl in ingress pipeline, which will be finally dropped -> update ip/mac/srcport or not is not important
        # (4) eg_intr_md.egress_port of the cloned ACK for cache population/eviction is set by clone_e2e, which must be the devport towards switchos (aka reflector)
        tmp_devport = self.reflector_devport
        tmp_client_ip = client_ips[0]
        tmp_client_mac = client_macs[0]
        tmp_client_port = 123  # not cared by servers
        data_without_action = [gc.DataTuple('client_mac', tmp_client_mac),
            gc.DataTuple('switch_mac', self.reflector_mac_for_switch),
            gc.DataTuple('client_ip', tmp_client_ip),
            gc.DataTuple('switch_ip', self.reflector_ip_for_switch),
            gc.DataTuple('client_port', tmp_client_port)]
        for tmpoptype in [
            CACHE_POP_INSWITCH_ACK,
            CACHE_EVICT_LOADFREQ_INSWITCH_ACK,
            CACHE_EVICT_LOADDATA_INSWITCH_ACK,
            SETVALID_INSWITCH_ACK,
        ]:
            keys.append(self.update_ipmac_srcport_tbl.make_key([
                gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('eg_intr_md.egress_port', tmp_devport)
            ]))
            datas.append(self.update_ipmac_srcport_tbl.make_data(
                data_without_action,
                'farreachEgress.update_ipmac_srcport_switch2switchos'))
        self.update_ipmac_srcport_tbl.entry_add(self.target, keys, datas)

    def configure_add_and_remove_value_header_tbl(self):
        keys = []
        datas = []
        # NOTE: egress pipeline must not output PUTREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, CACHE_POP_INSWITCH, and PUTREQ_INSWITCH
        # NOTE: even for future PUTREQ_LARGE/GETRES_LARGE, as their values should be in payload, we should invoke add_only_vallen() for vallen in [0, global_max_vallen]
        # LOADREQ, GETRES
        for tmpoptype in [
            PUTREQ_SEQ,
            PUTREQ_POP_SEQ,
            PUTREQ_SEQ_CASE3,
            PUTREQ_POP_SEQ_CASE3,
            GETRES_SEQ,
            CACHE_EVICT_LOADDATA_INSWITCH_ACK,
            PUTREQ_SEQ_BEINGEVICTED,
            PUTREQ_SEQ_CASE3_BEINGEVICTED,
            GETREQ_BEINGEVICTED_RECORD,
            GETREQ_LARGEVALUEBLOCK_RECORD,
            BACKUP,
            BACKUPACK
        ]:
            for i in range(int(switch_max_vallen / 8 + 1)):  # i from 0 to 16
                if i == 0:
                    vallen_start = 0
                    vallen_end = 0
                else:
                    vallen_start = (i - 1) * 8 + 1  # 1, 9, ..., 121
                    vallen_end = (i - 1) * 8 + 8  # 8, 16, ..., 128
                keys.append(self.add_and_remove_value_header_tbl.make_key([
                    gc.KeyTuple('$MATCH_PRIORITY', 0),
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.vallen_hdr.vallen', low = vallen_start, high = vallen_end)]))
                if i == 0:
                    datas.append(self.add_and_remove_value_header_tbl.make_data([],'farreachEgress.add_only_vallen'))
                else:
                    datas.append(self.add_and_remove_value_header_tbl.make_data([],'farreachEgress.add_to_val{}'.format(i)))

        self.add_and_remove_value_header_tbl.entry_add(self.target, keys, datas)

    # def configure_drop_tbl(self):
    #     keys =[]
    #     datas =[]
    #     keys.append(self.drop_tbl.make_key([gc.KeyTuple('hdr.op_hdr.optype', GETRES_LATEST_SEQ_INSWITCH)]))
    #     datas.append(self.drop_tbl.make_data([],'farreachEgress.drop_getres_latest_seq_inswitch'))
    #     keys.append(self.drop_tbl.make_key([gc.KeyTuple('hdr.op_hdr.optype', GETRES_DELETED_SEQ_INSWITCH)]))
    #     datas.append(self.drop_tbl.make_data([],'farreachEgress.drop_getres_deleted_seq_inswitch'))
    #     self.drop_tbl.entry_add(self.target, keys, datas)

    def configure_update_val_tbl(self, valname):
        keys = []
        datas = []
        update_val_tbl = eval("self.update_val{}_tbl".format(valname))
        # NOTE: not access val_reg if access_val_mode == 0
        for access_val_mode in access_val_mode_list:
            key = update_val_tbl.make_key(
                [gc.KeyTuple('meta.access_val_mode', access_val_mode)])
            if access_val_mode == 1:
                keys.append(key)
                datas.append(update_val_tbl.make_data([],'farreachEgress.get_val{}'.format(valname)))
            elif access_val_mode == 2:
                keys.append(key)
                datas.append(update_val_tbl.make_data([],'farreachEgress.set_and_get_val{}'.format(valname)))
            elif access_val_mode == 3:
                keys.append(key)
                datas.append(update_val_tbl.make_data([],'farreachEgress.reset_and_get_val{}'.format(valname)))
        update_val_tbl.entry_add(self.target, keys, datas)
    ### MAIN ###

    # def configure_forward_tbl(self):

    def runTest(self):
        print("\nTest")
        ################################
        ### Normal MAT Configuration ###
        ################################

        # Ingress pipeline

        # Stage 0
        # Table: l2l3_forward_tbl (default: NoAction	; size = client_physical_num+server_physical_num = 4 < 16)
        print("Configuring l2l3_forward_tbl")
        self.configure_l2l3_forward_tbl()

        # Table: set_hot_threshold_tbl (default: set_hot_threshold; size = 1)
        print("Configuring set_hot_threshold_tbl")
        self.configure_set_hot_threshold_tbl()



        print("Configuring hash_for_partition_tbl")
        self.configure_hash_for_partition_tbl()
        print("Configuring hash_partition_tbl")
        self.configure_hash_partition_tbl()
        # Stage 3
        # Table: cache_lookup_tbl (default: uncached_action; size = 32K/64K)
        print("Leave cache_lookup_tbl managed by controller in runtime")

        # Table: hash_for_cm12/34_tbl (default: nop; size: 1)
        self.configure_hash_for_cm_tbl()
        
        # Table: hash_for_seq_tbl (default: NoAction	; size = 3)
        print("Configuring hash_for_seq_tbl")
        self.configure_hash_for_seq_tbl()

        # Stage 3
        # Table: prepare_for_cachehit_tbl (default: set_client_sid(0); size = 3*client_physical_num=6 < 3*8=24 < 32)
        print("Configuring prepare_for_cachehit_tbl")
        self.configure_prepare_for_cachehit_tbl()

        # Table: ipv4_forward_tbl (default: NoAction	; size = 9*client_physical_num=18 < 9*8=72)
        print("Configuring ipv4_forward_tbl")
        self.configure_ipv4_forward_tbl()
        # Stage 4

        # Table: sample_tbl (default: NoAction	; size = 2)
        print("Configuring sample_tbl")
        self.configure_sample_tbl()

        # Table: ig_port_forward_tbl (default: NoAction	; size = 7)
        print("Configuring ig_port_forward_tbl")
        self.configure_ig_port_forward_tbl()
        self.configure_special_ig_port_forward_tbl()
        # Egress pipeline
        # Stage 0
        # Table: access_cmi_tbl (default: initialize_cmi_predicate; size = 2)
        self.configure_access_cm_tbl()
        # Stgae 1
        # Table: is_hot_tbl (default: reset_is_hot; size = 1)
        print("Configuring is_hot_tbl")
        key = self.is_hot_tbl.make_key(
            [gc.KeyTuple('meta.cm1_predicate',2),
			gc.KeyTuple('meta.cm2_predicate',2),
			gc.KeyTuple('meta.cm3_predicate',2),
            gc.KeyTuple('meta.cm4_predicate',2)])
        data = self.is_hot_tbl.make_data([],'farreachEgress.set_is_hot')
        self.is_hot_tbl.entry_add(self.target, [key], [data])

        # Table: access_cache_frequency_tbl (default: nop; size: 10)
        print("Configuring access_cache_frequency_tbl")
        self.configure_access_cache_frequency_tbl()

        # Table: access_validvalue_tbl (default: reset_meta_validvalue; size = 8)
        print("Configuring access_validvalue_tbl")
        self.configure_access_validvalue_tbl()
        
        # Table: access_seq_tbl (default: NoAction	; size = 3)
        # NOTE: PUT/DELREQ_INSWITCH do NOT have fraginfo_hdr, while we ONLY assign seq for fragment 0 of PUTREQ_LARGEVALUE_INSWITCH
        print("Configuring access_seq_tbl")
        self.configure_access_seq_tbl()

        # Stgae 2
        # Table: save_client_udpport_tbl (default: NoAction	; size = 4)
        print("Configuring save_client_udpport_tbl")
        self.configure_save_client_udpport_tbl()

        # Table: access_latest_tbl (default: reset_is_latest; size = 20)
        print("Configuring access_latest_tbl")
        self.configure_access_latest_tbl()
        
        # Table: access_largevalueseq_and_save_assignedseq_tbl (default: reset_meta_largevalueseq; size = TODO)
        # if ENABLE_LARGEVALUEBLOCK:
        print("Configuring access_largevaluebseq_tbl")
        self.configure_access_largevaluebseq_tbl()
        
        print("Configuring is_largevalueblock_tbl")
        self.configure_is_largevalueblock_tbl()
        # Stage 3
        # Table: access_deleted_tbl (default: reset_is_deleted; size = 122)
        print("Configuring access_deleted_tbl")
        self.configure_access_deleted_tbl()

        # Table: update_vallen_tbl (default: reset_access_val_mode; 62)
        print("Configuring update_vallen_tbl")
        self.configure_update_vallen_tbl()

        # Table: access_savedseq_tbl (default: NoAction	; size = 56-2+8=62)
        print("Configuring access_savedseq_tbl")
        self.configure_access_savedseq_tbl()


        # Stage 4-11
        # Table: update_vallo1_tbl (default: nop; 14)
        for i in range(1, 17):
            print("Configuring update_vallo{}_tbl".format(i))
            self.configure_update_val_tbl("lo{}".format(i))
            print("Configuring update_valhi{}_tbl".format(i))
            self.configure_update_val_tbl("hi{}".format(i))

        # Stage 9
        # Table: lastclone_lastscansplit_tbl (default: reset_is_lastclone_lastscansplit; size = 4)
        # print("Configuring lastclone_lastscansplit_tbl")
        # self.configure_lastclone_lastscansplit_tbl()
        print("Configuring recover_tbl")
        self.configure_recover_tbl()
        # Stage 8
        # Table; another_eg_port_forward_tbl (default: NoAction	; size = < 4096)
        print("Configuring another_eg_port_forward_tbl")
        self.configure_another_eg_port_forward_tbl_largevalueblock()

        # Stage 9
        # Table: eg_port_forward_tbl (default: NoAction	; size = < 2048 < 8192)
        print("Configuring eg_port_forward_tbl")
        self.configure_eg_port_forward_tbl()

        # Table: update_pktlen_tbl (default: NoAction	; 15*17+14=269)
        print("Configuring update_pktlen_tbl")
        self.configure_update_pktlen_tbl()

        # Table: update_ipmac_srcport_tbl (default: NoAction	; 6*client_physical_num+20*server_physical_num+7=59 < 26*8+7=215 < 256)
        # NOTE: udp.dstport is updated by eg_port_forward_tbl (only required by switch2switchos)
        # NOTE: update_ipmac_srcport_tbl focues on src/dst ip/mac and udp.srcport
        print("Configuring update_ipmac_srcport_tbl")
        self.configure_update_ipmac_srcport_tbl()

        # Table: add_and_remove_value_header_tbl (default: remove_all; 17*16=272)
        print("Configuring add_and_remove_value_header_tbl")
        self.configure_add_and_remove_value_header_tbl()

        # Table: drop_tbl (default: NoAction	; size = 2)
        print("Configuring forward_tbl")
        # self.configure_forward_tbl()
