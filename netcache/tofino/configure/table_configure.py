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

# remember to install it to


cached_list = [0, 1]
hot_list = [0, 1]
report_list = [0, 1]
latest_list = [0, 1]
stat_list = [0, 1]
deleted_list = [0, 1]
sampled_list = [0, 1]
lastclone_list = [0, 1]
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

class TableConfigure(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "netcache"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        bfrt_info = self.interface.bfrt_info_get("netcache")
        
        self.target = gc.Target(device_id=0, pipe_id=0xffff)
        self.client_devports = []
        self.server_devports = []
        # Initializing all tables

        self.access_latest_tbl = bfrt_info.table_get("netcacheEgress.access_latest_tbl")
        self.access_seq_tbl = bfrt_info.table_get("netcacheEgress.access_seq_tbl")
        self.save_client_udpport_tbl = bfrt_info.table_get("netcacheEgress.save_client_udpport_tbl")
        self.cache_lookup_tbl = bfrt_info.table_get("netcacheIngress.cache_lookup_tbl")
        self.l2l3_forward_tbl = bfrt_info.table_get("netcacheIngress.l2l3_forward_tbl")
        self.ipv4_forward_tbl = bfrt_info.table_get("netcacheIngress.ipv4_forward_tbl")
        self.ig_port_forward_tbl = bfrt_info.table_get("netcacheIngress.ig_port_forward_tbl")
        self.set_hot_threshold_tbl = bfrt_info.table_get("netcacheIngress.set_hot_threshold_tbl")
        self.access_cm1_tbl = bfrt_info.table_get("netcacheEgress.access_cm1_tbl")
        self.access_cm2_tbl = bfrt_info.table_get("netcacheEgress.access_cm2_tbl")
        self.access_cm3_tbl = bfrt_info.table_get("netcacheEgress.access_cm3_tbl")
        self.access_cm4_tbl = bfrt_info.table_get("netcacheEgress.access_cm4_tbl")
        self.prepare_for_cachepop_tbl = bfrt_info.table_get("netcacheEgress.prepare_for_cachepop_tbl")
        self.sample_tbl = bfrt_info.table_get("netcacheIngress.sample_tbl")
        self.access_cache_frequency_tbl = bfrt_info.table_get("netcacheEgress.access_cache_frequency_tbl")
        self.access_deleted_tbl = bfrt_info.table_get("netcacheEgress.access_deleted_tbl")
        self.access_savedseq_tbl = bfrt_info.table_get("netcacheEgress.access_savedseq_tbl")
        self.is_hot_tbl = bfrt_info.table_get("netcacheEgress.is_hot_tbl")
        self.prepare_for_cachehit_tbl = bfrt_info.table_get("netcacheIngress.prepare_for_cachehit_tbl")
        self.access_bf1_tbl = bfrt_info.table_get("netcacheEgress.access_bf1_tbl")
        self.access_bf2_tbl = bfrt_info.table_get("netcacheEgress.access_bf2_tbl")
        self.access_bf3_tbl = bfrt_info.table_get("netcacheEgress.access_bf3_tbl")
        self.update_vallen_tbl = bfrt_info.table_get("netcacheEgress.update_vallen_tbl")
        self.update_valhi1_tbl = bfrt_info.table_get("netcacheEgress.update_valhi1_tbl")
        self.update_valhi2_tbl = bfrt_info.table_get("netcacheEgress.update_valhi2_tbl")
        self.update_vallo1_tbl = bfrt_info.table_get("netcacheEgress.update_vallo1_tbl")
        self.update_vallo2_tbl = bfrt_info.table_get("netcacheEgress.update_vallo2_tbl")
        self.update_valhi3_tbl = bfrt_info.table_get("netcacheEgress.update_valhi3_tbl")
        self.update_valhi4_tbl = bfrt_info.table_get("netcacheEgress.update_valhi4_tbl")
        self.update_vallo3_tbl = bfrt_info.table_get("netcacheEgress.update_vallo3_tbl")
        self.update_vallo4_tbl = bfrt_info.table_get("netcacheEgress.update_vallo4_tbl")
        self.hash_for_partition_tbl = bfrt_info.table_get("netcacheIngress.hash_for_partition_tbl")
        self.update_valhi5_tbl = bfrt_info.table_get("netcacheEgress.update_valhi5_tbl")
        self.update_valhi6_tbl = bfrt_info.table_get("netcacheEgress.update_valhi6_tbl")
        self.update_vallo5_tbl = bfrt_info.table_get("netcacheEgress.update_vallo5_tbl")
        self.update_vallo6_tbl = bfrt_info.table_get("netcacheEgress.update_vallo6_tbl")
        self.hash_for_cm12_tbl = bfrt_info.table_get("netcacheIngress.hash_for_cm12_tbl")
        self.hash_partition_tbl = bfrt_info.table_get("netcacheIngress.hash_partition_tbl")
        self.is_report_tbl = bfrt_info.table_get("netcacheEgress.is_report_tbl")
        self.update_valhi7_tbl = bfrt_info.table_get("netcacheEgress.update_valhi7_tbl")
        self.update_valhi8_tbl = bfrt_info.table_get("netcacheEgress.update_valhi8_tbl")
        self.update_vallo7_tbl = bfrt_info.table_get("netcacheEgress.update_vallo7_tbl")
        self.update_vallo8_tbl = bfrt_info.table_get("netcacheEgress.update_vallo8_tbl")
        self.hash_for_seq_tbl = bfrt_info.table_get("netcacheIngress.hash_for_seq_tbl")
        self.lastclone_lastscansplit_tbl = bfrt_info.table_get("netcacheEgress.lastclone_lastscansplit_tbl")
        self.update_valhi10_tbl = bfrt_info.table_get("netcacheEgress.update_valhi10_tbl")
        self.update_valhi9_tbl = bfrt_info.table_get("netcacheEgress.update_valhi9_tbl")
        self.update_vallo10_tbl = bfrt_info.table_get("netcacheEgress.update_vallo10_tbl")
        self.update_vallo9_tbl = bfrt_info.table_get("netcacheEgress.update_vallo9_tbl")
        self.hash_for_cm34_tbl = bfrt_info.table_get("netcacheIngress.hash_for_cm34_tbl")
        self.eg_port_forward_tbl = bfrt_info.table_get("netcacheEgress.eg_port_forward_tbl")
        self.update_valhi11_tbl = bfrt_info.table_get("netcacheEgress.update_valhi11_tbl")
        self.update_valhi12_tbl = bfrt_info.table_get("netcacheEgress.update_valhi12_tbl")
        self.update_vallo11_tbl = bfrt_info.table_get("netcacheEgress.update_vallo11_tbl")
        self.update_vallo12_tbl = bfrt_info.table_get("netcacheEgress.update_vallo12_tbl")
        self.hash_for_bf1_tbl = bfrt_info.table_get("netcacheIngress.hash_for_bf1_tbl")
        self.update_ipmac_srcport_tbl = bfrt_info.table_get("netcacheEgress.update_ipmac_srcport_tbl")
        self.update_valhi13_tbl = bfrt_info.table_get("netcacheEgress.update_valhi13_tbl")
        self.update_valhi14_tbl = bfrt_info.table_get("netcacheEgress.update_valhi14_tbl")
        self.update_vallo13_tbl = bfrt_info.table_get("netcacheEgress.update_vallo13_tbl")
        self.update_vallo14_tbl = bfrt_info.table_get("netcacheEgress.update_vallo14_tbl")
        self.hash_for_bf2_tbl = bfrt_info.table_get("netcacheIngress.hash_for_bf2_tbl")
        self.add_and_remove_value_header_tbl = bfrt_info.table_get("netcacheEgress.add_and_remove_value_header_tbl")
        self.update_pktlen_tbl = bfrt_info.table_get("netcacheEgress.update_pktlen_tbl")
        self.update_valhi15_tbl = bfrt_info.table_get("netcacheEgress.update_valhi15_tbl")
        self.update_valhi16_tbl = bfrt_info.table_get("netcacheEgress.update_valhi16_tbl")
        self.update_vallo15_tbl = bfrt_info.table_get("netcacheEgress.update_vallo15_tbl")
        self.update_vallo16_tbl = bfrt_info.table_get("netcacheEgress.update_vallo16_tbl")
        self.hash_for_bf3_tbl = bfrt_info.table_get("netcacheIngress.hash_for_bf3_tbl")

        self.l2l3_forward_tbl.info.key_field_annotation_add("hdr.ethernet_hdr.dstAddr", "mac")
        self.l2l3_forward_tbl.info.key_field_annotation_add("hdr.ipv4_hdr.dstAddr", "ipv4")
        self.ipv4_forward_tbl.info.key_field_annotation_add("hdr.ipv4_hdr.dstAddr", "ipv4")
        self.prepare_for_cachehit_tbl.info.key_field_annotation_add("hdr.ipv4_hdr.srcAddr","ipv4")

        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_mac", "netcacheEgress.update_ipmac_srcport_server2client", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_mac", "netcacheEgress.update_ipmac_srcport_server2client", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_ip", "netcacheEgress.update_ipmac_srcport_server2client", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_ip", "netcacheEgress.update_ipmac_srcport_server2client", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_mac", "netcacheEgress.update_ipmac_srcport_switch2switchos", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("switch_mac", "netcacheEgress.update_ipmac_srcport_switch2switchos", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_ip", "netcacheEgress.update_ipmac_srcport_switch2switchos", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("switch_ip", "netcacheEgress.update_ipmac_srcport_switch2switchos", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_mac", "netcacheEgress.update_dstipmac_client2server", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_ip", "netcacheEgress.update_dstipmac_client2server", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("switch_mac", "netcacheEgress.update_dstipmac_switch2switchos", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("switch_ip", "netcacheEgress.update_dstipmac_switch2switchos", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_mac", "netcacheEgress.update_ipmac_srcport_client2server", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_mac", "netcacheEgress.update_ipmac_srcport_client2server", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_ip", "netcacheEgress.update_ipmac_srcport_client2server", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_ip", "netcacheEgress.update_ipmac_srcport_client2server", "ipv4")
        
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
            devport = 136
            self.port_table.entry_add(
                self.target,
                [self.port_table.make_key([gc.KeyTuple('$DEV_PORT', devport)])],
                [self.port_table.make_data([gc.DataTuple('$SPEED', str_val="BF_SPEED_40G"),
                                            gc.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                            gc.DataTuple('$PORT_ENABLE', bool_val=True)])])
            self.client_devports.append(devport)
        for server_fpport in server_fpports:
            port, chnl = server_fpport.split("/")
            devport = 36 
            self.port_table.entry_add(
                self.target,
                [self.port_table.make_key([gc.KeyTuple('$DEV_PORT', devport)])],
                [self.port_table.make_data([gc.DataTuple('$SPEED', str_val="BF_SPEED_40G"),
                                            gc.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                            gc.DataTuple('$PORT_ENABLE', bool_val=True)])])
            self.server_devports.append(devport)

        # prepare sid
        sidnum = len(self.client_devports) + len(self.server_devports)
        sids = random.sample(range(BASE_SID_NORM, MAX_SID_NORM), sidnum)
        self.client_sids = sids[0:len(self.client_devports)]
        self.server_sids = sids[len(self.client_devports):sidnum]
      
        for i in range(client_physical_num):
            print("Binding sid {} with client devport {} for both direction mirroring".format(self.client_sids[i], self.client_devports[i])) # clone to client
            key = mirror_cfg_table.make_key([gc.KeyTuple('$sid', self.client_sids[i])])
            data = mirror_cfg_table.make_data(
                [gc.DataTuple('$direction', str_val="EGRESS"),
                gc.DataTuple('$ucast_egress_port', self.client_devports[i]),
                gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                gc.DataTuple('$session_enable', bool_val=True)],
                '$normal')
            mirror_cfg_table.entry_add(self.target,[key],[data])
        for i in range(server_physical_num):
            print("Binding sid {} with server devport {} for both direction mirroring".format(self.server_sids[i], self.server_devports[i])) # clone to server
            key = mirror_cfg_table.make_key([gc.KeyTuple('$sid', self.server_sids[i])])
            data = mirror_cfg_table.make_data(
                [gc.DataTuple('$direction', str_val="EGRESS"),
                gc.DataTuple('$ucast_egress_port', self.server_devports[i]),
                gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                gc.DataTuple('$session_enable', bool_val=True)],
                '$normal')
            mirror_cfg_table.entry_add(self.target,[key],[data])
        # NOTE: data plane communicate with switchos by software-based reflector, which is deployed in one server machine
        for i in sids:
            resp = mirror_cfg_table.entry_get(
                self.target,
                [mirror_cfg_table.make_key([gc.KeyTuple('$sid', i)])],
                {"from_hw": True})
            for data ,_ in resp:
                print(data.to_dict())
        isvalid = False
        for i in range(server_physical_num):
            if reflector_ip_for_switchos == server_ip_for_controller_list[i]:
                isvalid = True
                self.reflector_ip_for_switch = server_ips[i]
                self.reflector_mac_for_switch = server_macs[i]
                self.reflector_devport = self.server_devports[i]
                self.reflector_sid = self.server_sids[i] # clone to switchos (i.e., reflector at [the first] physical server)
        print("self.reflector_sid",self.reflector_sid)
        # # Setting up PTF dataplane
        self.dataplane = ptf.dataplane_instance
        self.dataplane.flush()

    def configure_l2l3_forward_tbl(self):
        for i in range(client_physical_num):
            key = self.l2l3_forward_tbl.make_key([
                gc.KeyTuple('hdr.ethernet_hdr.dstAddr', client_macs[i]),
                gc.KeyTuple('hdr.ipv4_hdr.dstAddr', client_ips[i], prefix_len=32)])
            data = self.l2l3_forward_tbl.make_data([gc.DataTuple('eport', self.client_devports[i])],
                                         'netcacheIngress.l2l3_forward')
            self.l2l3_forward_tbl.entry_add(self.target, [key], [data])
        for i in range(server_physical_num):
            key = self.l2l3_forward_tbl.make_key([gc.KeyTuple('hdr.ethernet_hdr.dstAddr', server_macs[i]),
                                         gc.KeyTuple('hdr.ipv4_hdr.dstAddr', server_ips[i], prefix_len=32)])
            data = self.l2l3_forward_tbl.make_data([gc.DataTuple('eport', self.server_devports[i])],
                                         'netcacheIngress.l2l3_forward')
            self.l2l3_forward_tbl.entry_add(self.target, [key], [data])

    def configure_set_hot_threshold_tbl(self):
        # hot_threshold = 0
        data = self.set_hot_threshold_tbl.make_data(
            [gc.DataTuple('hot_threshold', hot_threshold)],
            'netcacheIngress.set_hot_threshold')
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
            SETVALID_INSWITCH,
            PUTREQ_LARGEVALUE,
            NETCACHE_CACHE_POP_INSWITCH_NLATEST,
        ]:
            key = self.hash_for_partition_tbl.make_key([
                gc.KeyTuple('hdr.op_hdr.optype', tmpoptype)])
            data = self.hash_for_partition_tbl.make_data([],
                                            'netcacheIngress.hash_for_partition')
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
            SETVALID_INSWITCH,
            PUTREQ_LARGEVALUE,
            NETCACHE_CACHE_POP_INSWITCH_NLATEST,
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
                    
                    data = self.hash_partition_tbl.make_data([
                        gc.DataTuple('udpport', udp_dstport),
                        gc.DataTuple('eport', eport)],
                        'netcacheIngress.hash_partition')
                    self.hash_partition_tbl.entry_add(self.target, [key], [data])
                hash_start = hash_end + 1
    
    def configure_hash_for_cm_tbl(self):
        for i in ["12", "34"]:
            print("Configuring hash_for_cm{}_tbl".format(i))
            for tmpoptype in [GETREQ]:
                key = eval('self.hash_for_cm{}_tbl'.format(i)).make_key([gc.KeyTuple('hdr.op_hdr.optype',tmpoptype)])
                data = eval('self.hash_for_cm{}_tbl'.format(i)).make_data(
                    [],'netcacheIngress.hash_for_cm{}'.format(i))
                eval('self.hash_for_cm{}_tbl'.format(i)).entry_add(self.target, [key], [data])

    def configure_hash_for_bf_tbl(self):
        for i in range(1, 4):
            print("Configuring hash_for_bf{}_tbl".format(i))
            for tmpoptype in [GETREQ]:
                key = eval('self.hash_for_bf{}_tbl'.format(i)).make_key([gc.KeyTuple('hdr.op_hdr.optype',tmpoptype)])
                data = eval('self.hash_for_bf{}_tbl'.format(i)).make_data(
                    [],'netcacheIngress.hash_for_bf{}'.format(i))
                eval('self.hash_for_bf{}_tbl'.format(i)).entry_add(self.target, [key], [data])

    def configure_hash_for_seq_tbl(self):
        for tmpoptype in [PUTREQ, DELREQ, PUTREQ_LARGEVALUE]:
            key = self.hash_for_seq_tbl.make_key([gc.KeyTuple('hdr.op_hdr.optype', tmpoptype)])
            data = self.hash_for_seq_tbl.make_data([],'netcacheIngress.hash_for_seq')
            self.hash_for_seq_tbl.entry_add(self.target, [key], [data])
    
    def configure_prepare_for_cachehit_tbl(self):
        for client_physical_idx in range(client_physical_num):
            tmp_clientsid = self.client_sids[client_physical_idx]
            for tmpoptype in [GETREQ, WARMUPREQ]:
                key = self.prepare_for_cachehit_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.ipv4_hdr.srcAddr', client_ips[client_physical_idx], prefix_len=32)])
                data = self.prepare_for_cachehit_tbl.make_data(
                    [gc.DataTuple('client_sid', tmp_clientsid)],
                    'netcacheIngress.set_client_sid')
                self.prepare_for_cachehit_tbl.entry_add(self.target, [key], [data])

    def configure_ipv4_forward_tbl(self):
        for tmp_client_physical_idx in range(client_physical_num):
            eport = self.client_devports[tmp_client_physical_idx]
            for tmpoptype in [
                GETRES,
                PUTRES,
                DELRES,
                WARMUPACK,
                SCANRES_SPLIT,
                LOADACK,
                GETRES_LARGEVALUE,
            ]:
                key = self.ipv4_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.ipv4_hdr.dstAddr', client_ips[tmp_client_physical_idx], prefix_len=32)])
                data = self.ipv4_forward_tbl.make_data(
                    [gc.DataTuple('eport', eport)],
                    'netcacheIngress.forward_normal_response')
                self.ipv4_forward_tbl.entry_add(self.target, [key], [data])
    
    def configure_sample_tbl(self):
        for tmpoptype in [GETREQ]:
            key = self.sample_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype)])
            data = self.sample_tbl.make_data(
                [],
                'netcacheIngress.sample')
            self.sample_tbl.entry_add(self.target, [key], [data])
    
    def configure_ig_port_forward_tbl(self):
        keys = []
        datas = []
        keys.append(self.ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', GETREQ)]))
        datas.append(self.ig_port_forward_tbl.make_data(
            [],'netcacheIngress.update_getreq_to_getreq_inswitch'))
        keys.append(self.ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', PUTREQ)]))
        datas.append(self.ig_port_forward_tbl.make_data(
            [],'netcacheIngress.update_putreq_to_putreq_inswitch'))
        keys.append(self.ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', DELREQ)]))
        datas.append(self.ig_port_forward_tbl.make_data(
            [],'netcacheIngress.update_delreq_to_delreq_inswitch'))
        keys.append(self.ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', WARMUPREQ)]))
        datas.append(self.ig_port_forward_tbl.make_data(
            [],'netcacheIngress.update_warmupreq_to_netcache_warmupreq_inswitch'))
        keys.append(self.ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', NETCACHE_VALUEUPDATE)]))
        datas.append(self.ig_port_forward_tbl.make_data(
            [],'netcacheIngress.update_netcache_valueupdate_to_netcache_valueupdate_inswitch'))
        keys.append(self.ig_port_forward_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', PUTREQ_LARGEVALUE)]))
        datas.append(self.ig_port_forward_tbl.make_data(
            [],'netcacheIngress.update_putreq_largevalue_to_putreq_largevalue_inswitch'))
        self.ig_port_forward_tbl.entry_add(self.target, keys, datas)

    def configure_access_latest_tbl(self):
        keys = []
        datas = []
        # is_cached == 1
        keys.append(self.access_latest_tbl.make_key(
            [gc.KeyTuple('hdr.op_hdr.optype', GETREQ_INSWITCH),
            gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
            gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0)]))
        datas.append(self.access_latest_tbl.make_data([],'netcacheEgress.get_latest'))
        # NOTE: write queries of NetCache "invalidates" in-switch value by setting latest=0
        for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH]:
            keys.append(self.access_latest_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0)]))
            datas.append(self.access_latest_tbl.make_data([],'netcacheEgress.reset_and_get_latest'))
        
        keys.append(self.access_latest_tbl.make_key(
            [gc.KeyTuple('hdr.op_hdr.optype', NETCACHE_VALUEUPDATE_INSWITCH),
            gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
            gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0)]))
        datas.append(self.access_latest_tbl.make_data([],'netcacheEgress.set_and_get_latest'))
        # on-path in-switch invalidation for fragment 0 of PUTREQ_LARGEVALUE_INSWITCH
        keys.append(self.access_latest_tbl.make_key(
            [gc.KeyTuple('hdr.op_hdr.optype', PUTREQ_LARGEVALUE_INSWITCH),
            gc.KeyTuple('hdr.inswitch_hdr.is_cached', 1),
            gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0)]))
        datas.append(self.access_latest_tbl.make_data([],'netcacheEgress.reset_and_get_latest')) 

        # is_cached == 0 or 1
        for is_cached in cached_list:
            # NOTE: cache population of NetCache directly sets latest=1 due to blocking-based cache update
            keys.append(self.access_latest_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', CACHE_POP_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0)]))
            datas.append(self.access_latest_tbl.make_data([],'netcacheEgress.set_and_get_latest'))

            # NOTE: cache population of NetCache directly resets latest=0 for large value
            keys.append(self.access_latest_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', NETCACHE_CACHE_POP_INSWITCH_NLATEST),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0)]))
            datas.append(self.access_latest_tbl.make_data([],'netcacheEgress.reset_and_get_latest'))

        self.access_latest_tbl.entry_add(self.target, keys, datas)

    def configure_access_deleted_tbl(self):
        keys = []
        datas = []
        for (is_cached,is_latest,is_stat) in product(cached_list,latest_list,stat_list):
            if is_cached == 1:
                keys.append(self.access_deleted_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', GETREQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('meta.is_latest', is_latest),
                    gc.KeyTuple('hdr.stat_hdr.stat',is_stat)]))
                datas.append(self.access_deleted_tbl.make_data([],'netcacheEgress.get_deleted'))
            for tmpoptype in [
                CACHE_POP_INSWITCH,
                NETCACHE_CACHE_POP_INSWITCH_NLATEST,
            ]:
                key = self.access_deleted_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('meta.is_latest', is_latest),
                    gc.KeyTuple('hdr.stat_hdr.stat',is_stat)])
                if is_stat == 1:
                    keys.append(key)
                    datas.append(self.access_deleted_tbl.make_data([],'netcacheEgress.reset_and_get_deleted'))
                elif is_stat == 0:
                    keys.append(key)
                    datas.append(self.access_deleted_tbl.make_data([],'netcacheEgress.set_and_get_deleted'))
                
            key = self.access_deleted_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', NETCACHE_VALUEUPDATE_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                gc.KeyTuple('meta.is_latest', is_latest),
                gc.KeyTuple('hdr.stat_hdr.stat',is_stat)])
            if is_cached == 1:
                if is_stat == 1:
                    keys.append(key)
                    datas.append(self.access_deleted_tbl.make_data([],'netcacheEgress.reset_and_get_deleted'))
                elif is_stat == 0:
                    keys.append(key)
                    datas.append(self.access_deleted_tbl.make_data([],'netcacheEgress.set_and_get_deleted'))
        self.access_deleted_tbl.entry_add(self.target, keys, datas)

    def configure_access_seq_tbl(self):
        keys = []
        datas = []
        for tmpoptype in [PUTREQ_INSWITCH, DELREQ_INSWITCH, PUTREQ_LARGEVALUE_INSWITCH]:
            keys.append(self.access_seq_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.fraginfo_hdr.cur_fragidx', 0)]))
            datas.append(self.access_seq_tbl.make_data([],'netcacheEgress.assign_seq'))
        self.access_seq_tbl.entry_add(self.target, keys, datas)

    def configure_save_client_udpport_tbl(self):
        keys = []
        datas = []
        for tmpoptype in [GETREQ_INSWITCH, NETCACHE_WARMUPREQ_INSWITCH]:
            keys.append(self.save_client_udpport_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype)]))
            datas.append(self.save_client_udpport_tbl.make_data([],'netcacheEgress.save_client_udpport'))
        self.save_client_udpport_tbl.entry_add(self.target, keys, datas)

    def configure_prepare_for_cachepop_tbl(self):
        keys = []
        datas = []
        for tmp_server_physical_idx in range(server_physical_num):
            tmp_devport = self.server_devports[tmp_server_physical_idx]
            tmp_server_sid = self.server_sids[tmp_server_physical_idx]
            keys.append(self.prepare_for_cachepop_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', GETREQ_INSWITCH),
                gc.KeyTuple('eg_intr_md.egress_port', tmp_devport)]))
            datas.append(self.prepare_for_cachepop_tbl.make_data(
                [gc.DataTuple('server_sid', tmp_server_sid)],
                'netcacheEgress.set_server_sid_and_port'))
        keys.append(self.prepare_for_cachepop_tbl.make_key(
            [gc.KeyTuple('hdr.op_hdr.optype', NETCACHE_GETREQ_POP),
            gc.KeyTuple('eg_intr_md.egress_port', self.reflector_devport)]))
        datas.append(self.prepare_for_cachepop_tbl.make_data(
            [],
            'netcacheEgress.NoAction'))
        self.prepare_for_cachepop_tbl.entry_add(self.target, keys, datas)

    def configure_access_cache_frequency_tbl(self):
        keys = []
        datas = []
        keys.append(self.access_cache_frequency_tbl.make_key(
            [gc.KeyTuple('hdr.op_hdr.optype', GETREQ_INSWITCH),
            gc.KeyTuple('hdr.inswitch_hdr.is_sampled',1),
            gc.KeyTuple('hdr.inswitch_hdr.is_cached',1),
            gc.KeyTuple('meta.is_latest',1)]))
        datas.append(self.access_cache_frequency_tbl.make_data(
            [],
            'netcacheEgress.update_cache_frequency'))
        for (is_sampled,is_cached,is_latest) in product(sampled_list,cached_list,latest_list):
            for tmpoptype in [
                CACHE_POP_INSWITCH,
                NETCACHE_CACHE_POP_INSWITCH_NLATEST,
            ]:
                keys.append(self.access_cache_frequency_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.inswitch_hdr.is_sampled',is_sampled),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_latest',is_latest)]))
                datas.append(self.access_cache_frequency_tbl.make_data(
                    [],
                    'netcacheEgress.reset_cache_frequency'))
            keys.append(self.access_cache_frequency_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', CACHE_EVICT_LOADFREQ_INSWITCH),
                gc.KeyTuple('hdr.inswitch_hdr.is_sampled',is_sampled),
                gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                gc.KeyTuple('meta.is_latest',is_latest)]))
            datas.append(self.access_cache_frequency_tbl.make_data(
                [],
                'netcacheEgress.get_cache_frequency'))
        self.access_cache_frequency_tbl.entry_add(self.target, keys, datas)

    def configure_access_cm_tbl(self):
        cm_hashnum = 4
        for i in range(1, cm_hashnum + 1):
            print("Configuring access_cm{}_tbl".format(i))
            access_cm_tbl = eval('self.access_cm{}_tbl'.format(i))
            keys = []
            datas = []
            for is_cached in cached_list:
                for is_latest in latest_list:
                    if is_cached == 1 and is_latest == 1:  # follow algorithm 1 in NetCache paper to update CM
                        continue
                    else:
                        keys.append(access_cm_tbl.make_key(
                            [gc.KeyTuple('hdr.op_hdr.optype', GETREQ_INSWITCH),
                            gc.KeyTuple('hdr.inswitch_hdr.is_sampled', 1),
                            gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                            gc.KeyTuple('meta.is_latest',is_latest)]))
                        datas.append(access_cm_tbl.make_data([],'netcacheEgress.update_cm{}'.format(i)))
            access_cm_tbl.entry_add(self.target, keys, datas)

    def configure_access_savedseq_tbl(self): 
        keys = []
        datas = []      
        for is_cached in cached_list:
            for is_latest in latest_list:
                for tmpoptype in [
                    CACHE_POP_INSWITCH,
                    NETCACHE_CACHE_POP_INSWITCH_NLATEST,
                ]:
                    keys.append(self.access_savedseq_tbl.make_key(
                        [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                        gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                        gc.KeyTuple('meta.is_latest', is_latest)]))
                    datas.append(self.access_savedseq_tbl.make_data([],'netcacheEgress.set_and_get_savedseq'))
                if is_cached == 1:
                    keys.append(self.access_savedseq_tbl.make_key(
                        [gc.KeyTuple('hdr.op_hdr.optype', NETCACHE_VALUEUPDATE_INSWITCH),
                        gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                        gc.KeyTuple('meta.is_latest', is_latest)]))
                    datas.append(self.access_savedseq_tbl.make_data([],'netcacheEgress.set_and_get_savedseq'))
        self.access_savedseq_tbl.entry_add(self.target, keys, datas)

    def configure_access_bf_tbl(self):
        bf_hashnum = 3
        for i in range(1, bf_hashnum + 1):
            print("Configuring access_bf{}_tbl".format(i))
            keys = []
            datas = []
            access_bf_tbl = eval('self.access_bf{}_tbl'.format(i))
            for tmpoptype in [GETREQ_INSWITCH]:
                keys.append(access_bf_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('meta.is_hot', 1)]))
                datas.append(access_bf_tbl.make_data([],'netcacheEgress.update_bf{}'.format(i)))
            access_bf_tbl.entry_add(self.target, keys, datas)
           
    def configure_lastclone_lastscansplit_tbl(self):
        keys = []
        datas = []
        for tmpoptype in [NETCACHE_GETREQ_POP, NETCACHE_WARMUPREQ_INSWITCH_POP]:
            keys.append(self.lastclone_lastscansplit_tbl.make_key(
                [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.clone_hdr.clonenum_for_pktloss', 0)]))
            datas.append(self.lastclone_lastscansplit_tbl.make_data([],'netcacheEgress.set_is_lastclone'))
        self.lastclone_lastscansplit_tbl.entry_add(self.target, keys, datas)
        # Stage 10

    def configure_eg_port_forward_tbl(self):
        keys = []
        datas = []
        # Table: eg_port_forward_tbl (default: nop; size: 27+852*client_physical_num=27+852*2=1731 < 2048 < 27+852*8=6843 < 8192)
        tmp_client_sids = [0] + self.client_sids
        tmp_server_sids = [0] + self.server_sids
        for(is_cached,is_hot,is_report,is_latest,is_deleted,tmp_client_sid,is_lastclone_for_pktloss,tmp_server_sid) in product(cached_list,hot_list,report_list,latest_list,deleted_list,tmp_client_sids,lastclone_list,tmp_server_sids):
            # Use tmpstat as action data to reduce action number
            tmpstat = 0 if is_deleted == 1 else 1
            # NOTE: eg_intr_md.egress_port is read-only
            # tmp_server_sids Only work for NETCACHE_GETREQ_POP
            # is_hot=0, is_report=0, is_latest=0, is_deleted=0, is_lastclone_for_pktloss=0, tmp_server_sid=0 for NETCACHE_WARMUPREQ_INSWITCH
            # NOTE: tmp_server_sid must be 0 as the last NETCACHE_WARMUPREQ_INSWITCH_POP is cloned as WARMUPACK to client instead of server
            if (
                is_hot == 0
                and is_report == 0
                and is_latest == 0
                and is_deleted == 0
                and is_lastclone_for_pktloss == 0
                and tmp_server_sid == 0
                and tmp_client_sid != 0
            ):
                # size: 2*client_physical_num=4 < 2*8=16
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',NETCACHE_WARMUPREQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('meta.is_report',is_report) ,
                    gc.KeyTuple('meta.is_latest',is_latest),
                    gc.KeyTuple('meta.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                # Update NETCACHE_WARMUP_INSWITCH as NETCACHE_WARMUP_INSWITCH_POP to switchos by cloning
                data = self.eg_port_forward_tbl.make_data(
                    [gc.DataTuple('switchos_sid',self.reflector_sid),
                    gc.DataTuple('reflector_port',reflector_dp2cpserver_port)],
                    'netcacheEgress.update_netcache_warmupreq_inswitch_to_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack')
                keys.append(key)
                datas.append(data)
            # is_hot=0, is_report=0, is_latest=0, is_deleted=0, tmp_server_sid=0 for NETCACHE_WARMUPREQ_INSWITCH_POP
            if (
                is_hot == 0
                and is_report == 0
                and is_latest == 0
                and is_deleted == 0
                and tmp_server_sid == 0
                and tmp_client_sid != 0
            ):
                # size: 4*client_physical_num=8 < 4*8=32
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',NETCACHE_WARMUPREQ_INSWITCH_POP),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('meta.is_report',is_report) ,
                    gc.KeyTuple('meta.is_latest',is_latest),
                    gc.KeyTuple('meta.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                if is_lastclone_for_pktloss == 0:
                    # Forward NETCACHE_WARMUP_INSWITCH_POP to switchos and clone
                    data = self.eg_port_forward_tbl.make_data(
                        [gc.DataTuple('switchos_sid',self.reflector_sid)],
                        'netcacheEgress.forward_netcache_warmupreq_inswitch_pop_clone_for_pktloss_and_warmupack')
                    keys.append(key)
                    datas.append(data)
                elif is_lastclone_for_pktloss == 1:
                    # Update NETCACHE_WARMUP_INSWITCH_POP as WARMUPACK to client by mirroring
                    # NOTE: WARMUPACK performs default action nop() to be forwarded to client
                    data = self.eg_port_forward_tbl.make_data(
                        [gc.DataTuple('client_sid',tmp_client_sid),
                        gc.DataTuple('server_port',server_worker_port_start)],
                        'netcacheEgress.update_netcache_warmupreq_inswitch_pop_to_warmupack_by_mirroring')
                    keys.append(key)
                    datas.append(data)
            # is_lastclone_for_pktloss should be 0 for GETREQ_INSWITCH
            if (
                is_lastclone_for_pktloss == 0
                and tmp_client_sid != 0
                and tmp_server_sid != 0
            ):
                # size: 32*client_physical_num*server_physical_num=128 < 32*8*8=2048
                # NOTE: tmp_client_sid != 0 to prepare for cache hit; tmp_server_sid != 0 to prepare for cache pop (clone last NETCACHE_GETREQ_POP as GETREQ to server)
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',GETREQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('meta.is_report',is_report) ,
                    gc.KeyTuple('meta.is_latest',is_latest),
                    gc.KeyTuple('meta.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                # data = self.eg_port_forward_tbl.make_data(
                #         [],
                #         'netcacheEgress.update_getreq_inswitch_to_getreq')
                # keys.append(key)
                # datas.append(data)
                if is_cached == 0:
                    if is_hot == 1 and is_report == 0:
                        # Update GETREQ_INSWITCH as NETCACHE_GETREQ_POP to switchos by cloning
                        data = self.eg_port_forward_tbl.make_data(
                            [gc.DataTuple('switchos_sid',self.reflector_sid),
                            gc.DataTuple('reflector_port',reflector_dp2cpserver_port)],
                            'netcacheEgress.update_getreq_inswitch_to_netcache_getreq_pop_clone_for_pktloss_and_getreq')
                        keys.append(key)
                        datas.append(data)
                    else:
                        # Update GETREQ_INSWITCH as GETREQ to server
                        data = self.eg_port_forward_tbl.make_data(
                            [],
                            'netcacheEgress.update_getreq_inswitch_to_getreq')
                        keys.append(key)
                        datas.append(data)
                else:  # is_cached == 1
                    if is_latest == 0:  # follow algorithm 1 in NetCache paper to report hot key if necessary
                        if is_hot == 1 and is_report == 0:
                            # Update GETREQ_INSWITCH as NETCACHE_GETREQ_POP to switchos by cloning
                            data = self.eg_port_forward_tbl.make_data(
                                [gc.DataTuple('switchos_sid',self.reflector_sid),
                                gc.DataTuple('reflector_port',reflector_dp2cpserver_port)],
                                'netcacheEgress.update_getreq_inswitch_to_netcache_getreq_pop_clone_for_pktloss_and_getreq')
                            keys.append(key)
                            datas.append(data)
                        else:
                            # Update GETREQ_INSWITCH as GETREQ to server
                            data = self.eg_port_forward_tbl.make_data(
                                [],
                                'netcacheEgress.update_getreq_inswitch_to_getreq')
                            keys.append(key)
                            datas.append(data)
                    else:  # is_cached == 1 and is_latest == 1
                        # Update GETREQ_INSWITCH as GETRES to client by mirroring
                        # (bit<10> client_sid,bit<16> server_port,bit<8> stat) 
                        data = self.eg_port_forward_tbl.make_data(
                            [gc.DataTuple('client_sid',tmp_client_sid),
                            gc.DataTuple('server_port',server_worker_port_start),
                            gc.DataTuple('stat',tmpstat)],
                            'netcacheEgress.update_getreq_inswitch_to_getres_by_mirroring')
                        keys.append(key)
                        datas.append(data)

            # is_cached=0 (no inswitch_hdr), is_hot=0 (not access CM), is_report=0 (not access BF), is_latest=0, is_deleted=0, tmp_client_sid=0 (no inswitch_hdr), tmp_server_sid!=0 for NETCACHE_GETREQ_POP
            if (
                is_cached == 0
                and is_hot == 0
                and is_report == 0
                and is_latest == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and tmp_server_sid != 0
            ):
                # size: 2*server_physical_num = 4 < 16
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',NETCACHE_GETREQ_POP),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('meta.is_report',is_report) ,
                    gc.KeyTuple('meta.is_latest',is_latest),
                    gc.KeyTuple('meta.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                if is_lastclone_for_pktloss == 0:
                    data = self.eg_port_forward_tbl.make_data(
                        [gc.DataTuple('switchos_sid',self.reflector_sid)],
                        'netcacheEgress.forward_netcache_getreq_pop_clone_for_pktloss_and_getreq')
                    keys.append(key)
                    datas.append(data)
                else:
                    data = self.eg_port_forward_tbl.make_data(
                        [gc.DataTuple('server_sid',tmp_server_sid)],
                        'netcacheEgress.update_netcache_getreq_pop_to_getreq_by_mirroring')
                    keys.append(key)
                    datas.append(data)

            # is_cached=0 (memset inswitch_hdr by end-host, and key must not be cached in cache_lookup_tbl for CACHE_POP_INSWITCH), is_hot (cm_predicate=1), is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss should be 0 for CACHE_POP_INSWITCH
            # size: 4
            # if is_cached == 0 and is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
            if (
                is_cached == 0
                and is_hot == 0
                and is_report == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
                and tmp_server_sid == 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',CACHE_POP_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('meta.is_report',is_report) ,
                    gc.KeyTuple('meta.is_latest',is_latest),
                    gc.KeyTuple('meta.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                data = self.eg_port_forward_tbl.make_data(
                    [gc.DataTuple('switchos_sid',self.reflector_sid),
                    gc.DataTuple('reflector_port',reflector_dp2cpserver_port)],
                    'netcacheEgress.update_cache_pop_inswitch_to_cache_pop_inswitch_ack_drop_and_clone')
                keys.append(key)
                datas.append(data)

            # is_cached=0 (memset inswitch_hdr by end-host, and key must not be cached in cache_lookup_tbl for NETCACHE_CACHE_POP_INSWITCH_NLATEST), is_hot (cm_predicate=1), is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss should be 0 for NETCACHE_CACHE_POP_INSWITCH_NLATEST
            # size: 4
            # if is_cached == 0 and is_hot == 0 and is_wrong_pipeline == 0 and is_lastclone_for_pktloss == 0:
            if (
                is_cached == 0
                and is_hot == 0
                and is_report == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
                and tmp_server_sid == 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',NETCACHE_CACHE_POP_INSWITCH_NLATEST),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('meta.is_report',is_report) ,
                    gc.KeyTuple('meta.is_latest',is_latest),
                    gc.KeyTuple('meta.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                # Update NETCACHE_CACHE_POP_INSWITCH_NLATEST as CACHE_POP_INSWITCH_ACK to reflector (deprecated: w/ clone)
                data = self.eg_port_forward_tbl.make_data(
                    [gc.DataTuple('switchos_sid',self.reflector_sid),
                    gc.DataTuple('reflector_port',reflector_dp2cpserver_port)],
                    'netcacheEgress.update_netcache_cache_pop_inswitch_nlatest_to_cache_pop_inswitch_ack_drop_and_clone')
                keys.append(key)
                datas.append(data)
            # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktloss should be 0 for CACHE_POP_INSWITCH_ACK
            # size: 0
            # if is_cached == 0 and is_hot == 0 and is_latest == 0 and is_deleted == 0 and is_wrong_pipeline == 0:
            if (
                is_cached == 0
                and is_hot == 0
                and is_report == 0
                and is_latest == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
                and tmp_server_sid == 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',CACHE_POP_INSWITCH_ACK),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('meta.is_report',is_report) ,
                    gc.KeyTuple('meta.is_latest',is_latest),
                    gc.KeyTuple('meta.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                pass
            # is_hot, is_deleted, tmp_client_sid, is_lastclone_for_pktloss should be 0 for PUTREQ_INSWITCH
            # size: 4
            if (
                is_hot == 0
                and is_report == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
                and tmp_server_sid == 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',PUTREQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('meta.is_report',is_report) ,
                    gc.KeyTuple('meta.is_latest',is_latest),
                    gc.KeyTuple('meta.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                if is_cached == 0:
                    data = self.eg_port_forward_tbl.make_data(
                        [],
                        'netcacheEgress.update_putreq_inswitch_to_putreq_seq')
                    keys.append(key)
                    datas.append(data)
                    # Update PUTREQ_INSWITCH as PUTREQ_SEQ to server
                elif is_cached == 1:
                    data = self.eg_port_forward_tbl.make_data(
                        [],
                        'netcacheEgress.update_putreq_inswitch_to_netcache_putreq_seq_cached')
                    keys.append(key)
                    datas.append(data)
                    # Update PUTREQ_INSWITCH as NETCACHE_PUTREQ_SEQ_CACHED to server

            # is_hot, (cm_predicate=1), is_deleted, tmp_client_sid, is_lastclone_for_pktloss should be 0 for DELREQ_INSWITCH
            # size: 4
            if (
                is_hot == 0
                and is_report == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
                and tmp_server_sid == 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',DELREQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('meta.is_report',is_report) ,
                    gc.KeyTuple('meta.is_latest',is_latest),
                    gc.KeyTuple('meta.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                if is_cached == 0:
                    # Update DELREQ_INSWITCH as DELREQ_SEQ to server
                    data = self.eg_port_forward_tbl.make_data(
                        [],
                        'netcacheEgress.update_delreq_inswitch_to_delreq_seq')
                    keys.append(key)
                    datas.append(data)
                elif is_cached == 1:
                    # Update DELREQ_INSWITCH as NETCACHE_DELREQ_SEQ_CACHED to server
                    data = self.eg_port_forward_tbl.make_data(
                        [],
                        'netcacheEgress.update_delreq_inswitch_to_netcache_delreq_seq_cached')
                    keys.append(key)
                    datas.append(data)
            # is_cached=1 (key must be cached in cache_lookup_tbl for CACHE_EVICT_LOADFREQ_INSWITCH), is_hot (cm_predicate=1), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0, is_lastclone_for_pktloss should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH
            # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
            # size: 1
            if (
                is_cached == 1
                and is_report == 0
                and is_hot == 0
                and is_latest == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
                and tmp_server_sid == 0
            ):
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',CACHE_EVICT_LOADFREQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('meta.is_report',is_report) ,
                    gc.KeyTuple('meta.is_latest',is_latest),
                    gc.KeyTuple('meta.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                # Update CACHE_EVICT_LOADFREQ_INSWITCH as CACHE_EVICT_LOADFREQ_INSWITCH_ACK to reflector (w/ frequency)
                data = self.eg_port_forward_tbl.make_data(
                    [gc.DataTuple('switchos_sid',self.reflector_sid),
                    gc.DataTuple('reflector_port',reflector_dp2cpserver_port)],
                    'netcacheEgress.update_cache_evict_loadfreq_inswitch_to_cache_evict_loadfreq_inswitch_ack_drop_and_clone')
                keys.append(key)
                datas.append(data)
            # is_cached=0 (no inswitch_hdr after clone_e2e), is_hot (cm_predicate=1), is_latest, is_deleted, is_wrong_pipeline, tmp_client_sid=0 (no inswitch_hdr), is_lastclone_for_pktlos should be 0 for CACHE_EVICT_LOADFREQ_INSWITCH_ACK
            # NOTE: is_cached must be 1 (CACHE_EVCIT must match an entry in cache_lookup_tbl)
            # size: 0
            if (
                is_cached == 0
                and is_report == 0
                and is_hot == 0
                and is_latest == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
                and tmp_server_sid == 0
            ):
                # key = self.eg_port_forward_tbl.make_key(
                #     [gc.KeyTuple('hdr.op_hdr.optype',CACHE_EVICT_LOADFREQ_INSWITCH_ACK),
                #     gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                #     gc.KeyTuple('meta.is_hot',is_hot),
                #     gc.KeyTuple('meta.is_report',is_report) ,
                #     gc.KeyTuple('meta.is_latest',is_latest),
                #     gc.KeyTuple('meta.is_deleted',is_deleted),
                #     gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                #     gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                #     gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                # Forward CACHE_EVICT_LOADFREQ_INSWITCH_ACK (by clone_e2e) to reflector
                # NOTE: default action is nop -> forward the packet to sid set by clone_e2e
                pass
            # is_hot=0, is_report=0, tmp_client_sid=0, is_lastclone_for_pktloss=0, tmp_server_sid=0 for NETCACHE_VALUEUPDATE_INSWITCH
            if (
                is_hot == 0
                and is_report == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
                and tmp_server_sid == 0
            ):
                # size: 8
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',NETCACHE_VALUEUPDATE_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('meta.is_report',is_report) ,
                    gc.KeyTuple('meta.is_latest',is_latest),
                    gc.KeyTuple('meta.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                # Update NETCACHE_VALUEUPDATE_INSWITCH as NETCACHE_VALUEUPDATE_ACK to server
                data = self.eg_port_forward_tbl.make_data(
                    [],
                    'netcacheEgress.update_netcache_valueupdate_inswitch_to_netcache_valueupdate_ack')
                keys.append(key)
                datas.append(data)
            # is_hot=0, is_report=0, is_deleted=0, tmp_client_sid=0, is_lastclone_for_pktloss=0, tmp_server_sid=0 for PUTREQ_LARGEVALUE_INSWITCH
            if (
                is_hot == 0
                and is_report == 0
                and is_deleted == 0
                and tmp_client_sid == 0
                and is_lastclone_for_pktloss == 0
                and tmp_server_sid == 0
            ):
                # size: 4
                key = self.eg_port_forward_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype',PUTREQ_LARGEVALUE_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached',is_cached),
                    gc.KeyTuple('meta.is_hot',is_hot),
                    gc.KeyTuple('meta.is_report',is_report) ,
                    gc.KeyTuple('meta.is_latest',is_latest),
                    gc.KeyTuple('meta.is_deleted',is_deleted),
                    gc.KeyTuple('hdr.inswitch_hdr.client_sid',tmp_client_sid),
                    gc.KeyTuple('meta.is_lastclone_for_pktloss',is_lastclone_for_pktloss),
                    gc.KeyTuple('hdr.clone_hdr.server_sid',tmp_server_sid),])
                if is_cached == 0:
                    # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ to server
                    data = self.eg_port_forward_tbl.make_data(
                        [],
                        'netcacheEgress.update_putreq_largevalue_inswitch_to_putreq_largevalue_seq')
                    keys.append(key)
                    datas.append(data)
                elif is_cached == 1:
                    # Update PUTREQ_LARGEVALUE_INSWITCH as PUTREQ_LARGEVALUE_SEQ_CACHED to server
                    data = self.eg_port_forward_tbl.make_data(
                        [],
                        'netcacheEgress.update_putreq_largevalue_inswitch_to_putreq_largevalue_seq_cached')
                    keys.append(key)
                    datas.append(data)
        self.eg_port_forward_tbl.entry_add(self.target, keys, datas)
   
    def configure_update_pktlen_tbl(self):
        keys = [] 
        datas = []
        for i in range(int(switch_max_vallen / 8 + 1)):  # i from 0 to 16
            if i == 0:
                vallen_start = 0
                vallen_end = 0
                aligned_vallen = 0
            else:
                vallen_start = (i - 1) * 8 + 1  # 1, 9, ..., 121
                vallen_end = (i - 1) * 8 + 8  # 8, 16, ..., 128
                aligned_vallen = vallen_end  # 8, 16, ..., 128
            val_stat_udplen = aligned_vallen + 34
            val_stat_iplen = aligned_vallen + 54
            val_seq_udplen = aligned_vallen + 34
            val_seq_iplen = aligned_vallen + 54
            keys.append(self.update_pktlen_tbl.make_key([
                gc.KeyTuple('$MATCH_PRIORITY', 0),
                gc.KeyTuple('hdr.op_hdr.optype', GETRES),
                gc.KeyTuple('hdr.vallen_hdr.vallen', low = vallen_start, high = vallen_end)
            ]))
            datas.append(self.update_pktlen_tbl.make_data(
                [gc.DataTuple('udplen', val_stat_udplen),
                gc.DataTuple('iplen', val_stat_iplen)],
                'netcacheEgress.update_pktlen'))

            for tmpoptype in [PUTREQ_SEQ, NETCACHE_PUTREQ_SEQ_CACHED]:
                keys.append(self.update_pktlen_tbl.make_key([
                    gc.KeyTuple('$MATCH_PRIORITY', 0),
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.vallen_hdr.vallen', low = vallen_start, high = vallen_end)
                ]))
                datas.append(self.update_pktlen_tbl.make_data(
                    [gc.DataTuple('udplen', val_seq_udplen),
                    gc.DataTuple('iplen', val_seq_iplen)],
                    'netcacheEgress.update_pktlen'))

        onlyop_udplen = 26
        onlyop_iplen = 46
        # stat_udplen = 32
        # stat_iplen = 52
        seq_udplen = 32
        seq_iplen = 52
        scanreqsplit_udplen = 49
        scanreqsplit_iplen = 69
        frequency_udplen = 30
        frequency_iplen = 50
        op_clone_udplen = 34
        op_clone_iplen = 54
        op_inswitch_clone_udplen = 64
        op_inswitch_clone_iplen = 84
        for tmpoptype in [
            CACHE_POP_INSWITCH_ACK,
            GETREQ,
            WARMUPACK,
            NETCACHE_VALUEUPDATE_ACK,
        ]:
            keys.append(self.update_pktlen_tbl.make_key([
                gc.KeyTuple('$MATCH_PRIORITY', 0),
                gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = switch_max_vallen)
            ])) # [0, 128]
            datas.append(self.update_pktlen_tbl.make_data(
                [gc.DataTuple('udplen', onlyop_udplen),
                gc.DataTuple('iplen', onlyop_iplen)],
                'netcacheEgress.update_pktlen'))# 0 is priority (range may be overlapping)
        for tmpoptype in [DELREQ_SEQ, NETCACHE_DELREQ_SEQ_CACHED]:
            keys.append(self.update_pktlen_tbl.make_key([
                gc.KeyTuple('$MATCH_PRIORITY', 0),
                gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = switch_max_vallen)
            ])) # [0, 128]
            datas.append(self.update_pktlen_tbl.make_data(
                [gc.DataTuple('udplen', seq_udplen),
                gc.DataTuple('iplen', seq_iplen)],
                'netcacheEgress.update_pktlen'))# 0 is priority (range may be overlapping)
        keys.append(self.update_pktlen_tbl.make_key([
            gc.KeyTuple('$MATCH_PRIORITY', 0),
            gc.KeyTuple('hdr.op_hdr.optype', SCANREQ_SPLIT),
            gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = switch_max_vallen)
        ])) # [0, 128]
        datas.append(self.update_pktlen_tbl.make_data(
            [gc.DataTuple('udplen', scanreqsplit_udplen),
            gc.DataTuple('iplen', scanreqsplit_iplen)],
            'netcacheEgress.update_pktlen'))# 0 is priority (range may be overlapping)
        keys.append(self.update_pktlen_tbl.make_key([
            gc.KeyTuple('$MATCH_PRIORITY', 0),
            gc.KeyTuple('hdr.op_hdr.optype', CACHE_EVICT_LOADFREQ_INSWITCH_ACK),
            gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = switch_max_vallen)
        ])) # [0, 128]
        datas.append(self.update_pktlen_tbl.make_data(
            [gc.DataTuple('udplen', frequency_udplen),
            gc.DataTuple('iplen', frequency_iplen)],
            'netcacheEgress.update_pktlen'))# 0 is priority (range may be overlapping)
        keys.append(self.update_pktlen_tbl.make_key([
            gc.KeyTuple('$MATCH_PRIORITY', 0),
            gc.KeyTuple('hdr.op_hdr.optype', NETCACHE_GETREQ_POP),
            gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = switch_max_vallen)
        ])) # [0, 128]
        datas.append(self.update_pktlen_tbl.make_data(
            [gc.DataTuple('udplen', op_clone_udplen),
            gc.DataTuple('iplen', op_clone_iplen)],
            'netcacheEgress.update_pktlen'))# 0 is priority (range may be overlapping)
        keys.append(self.update_pktlen_tbl.make_key([
            gc.KeyTuple('$MATCH_PRIORITY', 0),
            gc.KeyTuple('hdr.op_hdr.optype', NETCACHE_WARMUPREQ_INSWITCH_POP),
            gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = switch_max_vallen)
        ])) # [0, 128]
        datas.append(self.update_pktlen_tbl.make_data(
            [gc.DataTuple('udplen', op_inswitch_clone_udplen),
            gc.DataTuple('iplen', op_inswitch_clone_iplen)],
            'netcacheEgress.update_pktlen'))# 0 is priority (range may be overlapping)
        # For large value
        shadowtype_seq_udp_delta = 6
        shadowtype_seq_ip_delta = 6
        for tmpoptype in [PUTREQ_LARGEVALUE_SEQ, PUTREQ_LARGEVALUE_SEQ_CACHED]:
            keys.append(self.update_pktlen_tbl.make_key([
                gc.KeyTuple('$MATCH_PRIORITY', 0),
                gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('hdr.vallen_hdr.vallen', low = 0, high = 65535)
            ])) # [0, 128]
            datas.append(self.update_pktlen_tbl.make_data(
                [gc.DataTuple('udplen_delta', shadowtype_seq_udp_delta),
                gc.DataTuple('iplen_delta', shadowtype_seq_ip_delta)],
                'netcacheEgress.add_pktlen'))# 0 is priority (range may be overlapping)
        self.update_pktlen_tbl.entry_add(self.target, keys, datas)

    def configure_update_vallen_tbl(self):
        keys = []
        datas = []
        for (is_cached,is_latest) in product(cached_list,latest_list):
            if is_cached == 1:
                keys.append(self.update_vallen_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', GETREQ_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('meta.is_latest', is_latest)]))
                datas.append(self.update_vallen_tbl.make_data([],'netcacheEgress.get_vallen'))
                keys.append(self.update_vallen_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', NETCACHE_VALUEUPDATE_INSWITCH),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('meta.is_latest', is_latest)]))
                datas.append(self.update_vallen_tbl.make_data([],'netcacheEgress.set_and_get_vallen'))
            for tmpoptype in [
                CACHE_POP_INSWITCH,
                NETCACHE_CACHE_POP_INSWITCH_NLATEST,
            ]:
                keys.append(self.update_vallen_tbl.make_key(
                    [gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.inswitch_hdr.is_cached', is_cached),
                    gc.KeyTuple('meta.is_latest', is_latest)]))
                datas.append(self.update_vallen_tbl.make_data([],'netcacheEgress.set_and_get_vallen'))
        self.update_vallen_tbl.entry_add(self.target, keys, datas)

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
                datas.append(update_val_tbl.make_data([],'netcacheEgress.get_val{}'.format(valname)))
            elif access_val_mode == 2:
                keys.append(key)
                datas.append(update_val_tbl.make_data([],'netcacheEgress.set_and_get_val{}'.format(valname)))
            elif access_val_mode == 3:
                keys.append(key)
                datas.append(update_val_tbl.make_data([],'netcacheEgress.reset_and_get_val{}'.format(valname)))
        update_val_tbl.entry_add(self.target, keys, datas)

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
            for tmpoptype in [
                GETRES,
                PUTRES,
                DELRES,
                SCANRES_SPLIT,
                WARMUPACK,
                LOADACK,
                GETRES_LARGEVALUE,
            ]:
                keys.append(self.update_ipmac_srcport_tbl.make_key([
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('eg_intr_md.egress_port', tmp_devport)
                ]))
                datas.append(self.update_ipmac_srcport_tbl.make_data(
                    data_without_action,
                    'netcacheEgress.update_ipmac_srcport_server2client'))

        # for request from client to server, egress port has been set by partition_tbl in ingress pipeline
        for tmp_server_physical_idx in range(server_physical_num):
            tmp_devport = self.server_devports[tmp_server_physical_idx]
            tmp_server_mac = server_macs[tmp_server_physical_idx]
            tmp_server_ip = server_ips[tmp_server_physical_idx]
            data_without_action =  [gc.DataTuple('server_mac', tmp_server_mac),
                gc.DataTuple('server_ip', tmp_server_ip)]
            for tmpoptype in [
                GETREQ,
                PUTREQ_SEQ,
                NETCACHE_PUTREQ_SEQ_CACHED,
                DELREQ_SEQ,
                NETCACHE_DELREQ_SEQ_CACHED,
                SCANREQ_SPLIT,
                LOADREQ,
                PUTREQ_LARGEVALUE_SEQ,
                PUTREQ_LARGEVALUE_SEQ_CACHED,
            ]:
                keys.append(self.update_ipmac_srcport_tbl.make_key([
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('eg_intr_md.egress_port', tmp_devport)
                ]))
                datas.append(self.update_ipmac_srcport_tbl.make_data(
                    data_without_action,
                    'netcacheEgress.update_dstipmac_client2server'))

            tmp_client_mac = client_macs[0]
            tmp_client_ip = client_ips[0]
            tmp_client_port = 123  # not cared by switchos
            data_without_action = [gc.DataTuple('client_mac', tmp_client_mac),
                gc.DataTuple('server_mac', tmp_server_mac),
                gc.DataTuple('client_ip', tmp_client_ip),
                gc.DataTuple('server_ip', tmp_server_ip),
                gc.DataTuple('client_port', tmp_client_port)]
            for tmpoptype in [NETCACHE_VALUEUPDATE_ACK]:  # simulate client -> server
                keys.append(self.update_ipmac_srcport_tbl.make_key([
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('eg_intr_md.egress_port', tmp_devport)
                ]))
                datas.append(self.update_ipmac_srcport_tbl.make_data(
                    data_without_action,
                    'netcacheEgress.update_ipmac_srcport_client2server'))

        # Here we use server_mac/ip to simulate reflector_mac/ip = switchos_mac/ip
        # (1) eg_intr_md.egress_port of the first GETRES_CASE1 is set by ipv4_forward_tbl (as ingress port), which will be finally dropped -> update ip/mac/srcport or not is not important
        # (2) eg_intr_md.egress_port of cloned GETRES_CASE1s is set by clone_e2e, which must be the devport towards switchos (aka reflector)
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
        # simulate client/switch -> switchos
        for tmpoptype in [CACHE_POP_INSWITCH_ACK, CACHE_EVICT_LOADFREQ_INSWITCH_ACK]:
            keys.append(self.update_ipmac_srcport_tbl.make_key([
                gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('eg_intr_md.egress_port', tmp_devport)
            ]))
            datas.append(self.update_ipmac_srcport_tbl.make_data(
                data_without_action,
                'netcacheEgress.update_ipmac_srcport_switch2switchos'))

        data_without_action =  [gc.DataTuple('switch_mac', self.reflector_mac_for_switch),
                gc.DataTuple('switch_ip', self.reflector_ip_for_switch)]
        for tmpoptype in [NETCACHE_GETREQ_POP, NETCACHE_WARMUPREQ_INSWITCH_POP]:
            keys.append(self.update_ipmac_srcport_tbl.make_key([
                gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                gc.KeyTuple('eg_intr_md.egress_port', tmp_devport)
            ]))
            datas.append(self.update_ipmac_srcport_tbl.make_data(
                data_without_action,
                'netcacheEgress.update_dstipmac_switch2switchos'))

        self.update_ipmac_srcport_tbl.entry_add(self.target, keys, datas)

    def configure_add_and_remove_value_header_tbl(self):
        # NOTE: egress pipeline must not output PUTREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, CACHE_POP_INSWITCH, and PUTREQ_INSWITCH
        # NOTE: even for future PUTREQ_LARGE/GETRES_LARGE, as their values should be in payload, we should invoke add_only_vallen() for vallen in [0, global_max_vallen]
        # , LOADREQ
        keys = []
        datas = []
        for tmpoptype in [PUTREQ_SEQ, NETCACHE_PUTREQ_SEQ_CACHED, GETRES]:
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
                    datas.append(self.add_and_remove_value_header_tbl.make_data([],'netcacheEgress.add_only_vallen'))
                else:
                    datas.append(self.add_and_remove_value_header_tbl.make_data([],'netcacheEgress.add_to_val{}'.format(i)))

        self.add_and_remove_value_header_tbl.entry_add(self.target, keys, datas)

    ### MAIN ###

    def runTest(self):
        print("Configuring start")
        
        ################################
        ### Normal MAT Configuration ###
        ################################

        # Ingress pipeline
        # Table: l2l3_forward_tbl (default: nop; size: client_physical_num+server_physical_num = 4 < 16)
        print("Configuring l2l3_forward_tbl")
        self.configure_l2l3_forward_tbl()

        # Table: set_hot_threshold_tbl (default: set_hot_threshold; size: 1)
        print("Configuring set_hot_threshold_tbl")
        self.configure_set_hot_threshold_tbl()
        # # Table: hash_for_partition_tbl (default: nop; size: 5)
        print("Configuring hash_for_partition_tbl")
        self.configure_hash_for_partition_tbl()

        print("Configuring hash_partition_tbl")
        self.configure_hash_partition_tbl()

        # # Table: cache_lookup_tbl (default: uncached_action; size: 32K/64K)
        print("Leave cache_lookup_tbl managed by controller in runtime")
        
        # Table: hash_for_cm12/34_tbl (default: nop; size: 1)
        self.configure_hash_for_cm_tbl()
        
        # Table: hash_for_bf1/2/3_tbl (default: nop; size: 1)
        self.configure_hash_for_bf_tbl()

        # Table: hash_for_seq_tbl (default: nop; size: 3)
        print("Configuring hash_for_seq_tbl")
        self.configure_hash_for_seq_tbl()

        # Table: prepare_for_cachehit_tbl (default: set_client_sid(0); size: 2*client_physical_num=4 < 2*8=16 < 32)
        print("Configuring prepare_for_cachehit_tbl")
        self.configure_prepare_for_cachehit_tbl()

        # Table: ipv4_forward_tbl (default: nop; size: 7*client_physical_num=14 < 7*8=56)
        print("Configuring ipv4_forward_tbl")
        self.configure_ipv4_forward_tbl()
        # Stage 4
        # Table: sample_tbl (default: nop; size: 1)
        print("Configuring sample_tbl")
        self.configure_sample_tbl()
        # Table: ig_port_forward_tbl (default: nop; size: 7)
        print("Configuring ig_port_forward_tbl")
        self.configure_ig_port_forward_tbl()
  
        # Egress pipeline

        # Stage 0
        # Table: access_latest_tbl (default: reset_is_latest; size: 9)
        print("Configuring access_latest_tbl")
        self.configure_access_latest_tbl()

        # Table: access_seq_tbl (default: nop; size: 3)
        # NOTE: PUT/DELREQ_INSWITCH do NOT have fraginfo_hdr, while we ONLY assign seq for fragment 0 of PUTREQ_LARGEVALUE_INSWITCH
        print("Configuring access_seq_tbl")
        self.configure_access_seq_tbl()
        # Table: save_client_udpport_tbl (default: nop; size: 4)
        print("Configuring save_client_udpport_tbl")
        self.configure_save_client_udpport_tbl()

        # Stage 1
        # Table: prepare_for_cachepop_tbl (default: reset_server_sid(); size: 2*server_physical_num+1=5 < 17)
        print("Configuring prepare_for_cachepop_tbl")
        self.configure_prepare_for_cachepop_tbl()

        # Table: access_cmi_tbl (default: initialize_cmi_predicate; size: 3)
        self.configure_access_cm_tbl()
        # Stgae 2

        # Table: is_hot_tbl (default: reset_is_hot; size: 1)
        print("Configuring is_hot_tbl")
        key = self.is_hot_tbl.make_key(
            [gc.KeyTuple('meta.cm1_predicate',2),
			gc.KeyTuple('meta.cm2_predicate',2),
			gc.KeyTuple('meta.cm3_predicate',2),
            gc.KeyTuple('meta.cm4_predicate',2)])
        data = self.is_hot_tbl.make_data([],'netcacheEgress.set_is_hot')
        self.is_hot_tbl.entry_add(self.target, [key], [data])

        # Table: access_cache_frequency_tbl (default: nop; size: 25)
        print("Configuring access_cache_frequency_tbl")
        self.configure_access_cache_frequency_tbl()

        # Table: access_deleted_tbl (default: reset_is_deleted; size: 16)
        print("Configuring access_deleted_tbl")
        self.configure_access_deleted_tbl()

        # Table: access_savedseq_tbl (default: nop; size: 10)
        print("Configuring access_savedseq_tbl")
        self.configure_access_savedseq_tbl()
        # Stage 3

        # Table: update_vallen_tbl (default: reset_access_val_mode; 12)
        print("Configuring update_vallen_tbl")
        self.configure_update_vallen_tbl()

        # Table: access_bfX_tbl (default: reset_is_reportX; size: 3)s
        self.configure_access_bf_tbl()
        # Stage 4
        # Table: is_report_tbl (default: reset_is_report; size: 1)
        # print("Configuring is_report_tbl")
        # key = self.is_report_tbl.make_key(
        #     [gc.KeyTuple('meta.is_report1',1),
		# 	gc.KeyTuple('meta.is_report2',1),
		# 	gc.KeyTuple('meta.is_report3',1)])
        # data = self.is_report_tbl.make_data([],'netcacheEgress.set_is_report')
        # self.is_report_tbl.entry_add(self.target, [key], [data])
        # Stage 4-11

        # Table: update_vallo1_tbl (default: nop; 14)
        for i in range(1, 17):
            print("Configuring update_vallo{}_tbl".format(i))
            self.configure_update_val_tbl("lo{}".format(i))
            print("Configuring update_valhi{}_tbl".format(i))
            self.configure_update_val_tbl("hi{}".format(i))

        # Stage 9

        # Table: lastclone_lastscansplit_tbl (default: reset_is_lastclone_lastscansplit; size: 2/3)
        print("Configuring lastclone_lastscansplit_tbl")
        self.configure_lastclone_lastscansplit_tbl()
        # Stage 10
        # Table: eg_port_forward_tbl (default: nop; size: < 2048 < 8192)
        print("Configuring eg_port_forward_tbl")
        self.configure_eg_port_forward_tbl()
        # Table: update_pktlen_tbl (default: nop; 3*17+10 = 61)
        print("Configuring update_pktlen_tbl")
        self.configure_update_pktlen_tbl()

        # Table: update_ipmac_srcport_tbl (default: nop; 7*client_physical_num+12*server_physical_num+9=47 < 19*8+9=161 < 256)
        # NOTE: udp.dstport is updated by eg_port_forward_tbl (only required by switch2switchos)
        # NOTE: update_ipmac_srcport_tbl focues on src/dst ip/mac and udp.srcport
        print("Configuring update_ipmac_srcport_tbl")
        self.configure_update_ipmac_srcport_tbl()

        # Table: add_and_remove_value_header_tbl (default: remove_all; 17*4=68)
        print("Configuring add_and_remove_value_header_tbl")
        self.configure_add_and_remove_value_header_tbl()


