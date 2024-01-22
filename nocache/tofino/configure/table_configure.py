import os
import time
import json
import math
# from scapy.all import *
this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
# print(this_dir,sys.path) 
from common import *
# p
# from common import *

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
# RECIRCULATION_PORT = 68 + 128

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
from ptf_port import *
# from pal_rpc.ttypes import *
# print(json.dumps(config))

# fw = open("./test.txt", 'w') 
# fw.write(json.dumps(config))  
# fw.write("\n")    


swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)
    swports.sort()
print(swports)
swports_0 = []
swports_1 = []
swports_2 = []
swports_3 = []
# the following method categorizes the ports in ports.json file as belonging to either of the pipes (0, 1, 2, 3)
def port_to_pipe(port):
    local_port = port & 0x7F
    assert (local_port < 72)
    pipe = (port >> 7) & 0x3
    assert (port == ((pipe << 7) | local_port))
    return pipe
for port in swports:
    pipe = port_to_pipe(port)
    if pipe == 0:
        swports_0.append(port)
    elif pipe == 1:
        swports_1.append(port)
    elif pipe == 2:
        swports_2.append(port)
    elif pipe == 3:
        swports_3.append(port)

print(swports_0,swports_1,swports_2,swports_3)

logger = logging.getLogger('Test')
if not len(logger.handlers):
    logger.addHandler(logging.StreamHandler())

# print(config)

class TableConfigure(BfRuntimeTest):
    def setUp(self):
        client_id = 0
        p4_name = "nocache_16"
        BfRuntimeTest.setUp(self, client_id, p4_name)
        bfrt_info = self.interface.bfrt_info_get("nocache_16")
        
        self.target = gc.Target(device_id=0, pipe_id=0xffff)
        self.client_devports = []
        self.server_devports = []
        # Initializing all tables
        self.hash_for_partition_tbl = bfrt_info.table_get("nocahceIngress.hash_for_partition_tbl")
        self.l2l3_forward_tbl = bfrt_info.table_get("nocahceIngress.l2l3_forward_tbl")
        self.hash_partition_tbl = bfrt_info.table_get("nocahceIngress.hash_partition_tbl")
        self.ipv4_forward_tbl = bfrt_info.table_get("nocahceIngress.ipv4_forward_tbl")
        self.update_ipmac_srcport_tbl = bfrt_info.table_get("nocahceEgress.update_ipmac_srcport_tbl")
        self.update_pktlen_tbl = bfrt_info.table_get("nocahceEgress.update_pktlen_tbl")
        
        self.l2l3_forward_tbl.info.key_field_annotation_add("hdr.ethernet_hdr.dstAddr", "mac")
        self.l2l3_forward_tbl.info.key_field_annotation_add("hdr.ipv4_hdr.dstAddr", "ipv4")
        self.ipv4_forward_tbl.info.key_field_annotation_add("hdr.ipv4_hdr.dstAddr", "ipv4")
        
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_mac", "nocahceEgress.update_ipmac_srcport_server2client", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_mac", "nocahceEgress.update_ipmac_srcport_server2client", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("client_ip", "nocahceEgress.update_ipmac_srcport_server2client", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_ip", "nocahceEgress.update_ipmac_srcport_server2client", "ipv4")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_mac", "nocahceEgress.update_dstipmac_client2server", "mac")
        self.update_ipmac_srcport_tbl.info.data_field_annotation_add("server_ip", "nocahceEgress.update_dstipmac_client2server", "ipv4")
        
        # fetch port table 
        self.port_table = bfrt_info.table_get("$PORT")
        self.port_stat_table = bfrt_info.table_get("$PORT_STAT")
        self.port_hdl_info_table = bfrt_info.table_get("$PORT_HDL_INFO")
        self.port_fp_idx_info_table = bfrt_info.table_get("$PORT_FP_IDX_INFO")
        self.port_str_info_table = bfrt_info.table_get("$PORT_STR_INFO")
        # 

        # print(self.port_table.info.__dict__)
        for client_fpport in client_fpports:
            port, chnl = client_fpport.split("/")
            devport = 136
            self.port_table.entry_add(
                self.target,
                [self.port_table.make_key([gc.KeyTuple('$DEV_PORT', 136)])],
                [self.port_table.make_data([gc.DataTuple('$SPEED', str_val="BF_SPEED_40G"),
                                            gc.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                            gc.DataTuple('$PORT_ENABLE', bool_val=True)])])
            # print(port)
            # self.port_table.entry_add(
            #     self.target,
            #     [self.port_table.make_key([gc.KeyTuple('$DEV_PORT',int(port))])],
            #     [self.port_table.make_data([gc.DataTuple('$PORT_ENABLE', bool_val=False)])])
            # devport = testutils.pal.pal_port_front_panel_port_to_dev_port_get(0, int(port), int(chnl))
            # print(dev)
            self.client_devports.append(devport)
        for server_fpport in server_fpports:
            port, chnl = server_fpport.split("/")
            devport = 36 
            self.port_table.entry_add(
                self.target,
                [self.port_table.make_key([gc.KeyTuple('$DEV_PORT', 36)])],
                [self.port_table.make_data([gc.DataTuple('$SPEED', str_val="BF_SPEED_40G"),
                                            gc.DataTuple('$FEC', str_val="BF_FEC_TYP_NONE"),
                                            gc.DataTuple('$PORT_ENABLE', bool_val=True)])])
            self.server_devports.append(devport)

        # Setting up PTF dataplane
        self.dataplane = ptf.dataplane_instance
        self.dataplane.flush()

    def runTest(self):
        print('start configure')
        # resp = self.l2l3_forward_tbl.entry_get(self.target)
        # i=0
        # for data, key in resp:
        #     print(data.to_dict(),key.to_dict())
       
        print("Configuring l2l3_forward_tbl")
        for i in range(client_physical_num):
            key = self.l2l3_forward_tbl.make_key([
                gc.KeyTuple('hdr.ethernet_hdr.dstAddr', client_macs[i]),
                gc.KeyTuple('hdr.ipv4_hdr.dstAddr', client_ips[i], prefix_len=32)])
            data = self.l2l3_forward_tbl.make_data([gc.DataTuple('eport', self.client_devports[i])],
                                         'nocahceIngress.l2l3_forward')
            self.l2l3_forward_tbl.entry_add(self.target, [key], [data])
            # gc.KeyTuple('$MATCH_PRIORITY', 1),
        for i in range(server_physical_num):
            key = self.l2l3_forward_tbl.make_key([gc.KeyTuple('hdr.ethernet_hdr.dstAddr', server_macs[i]),
                                         gc.KeyTuple('hdr.ipv4_hdr.dstAddr', server_ips[i], prefix_len=32)])
            data = self.l2l3_forward_tbl.make_data([gc.DataTuple('eport', self.server_devports[i])],
                                         'nocahceIngress.l2l3_forward')
            self.l2l3_forward_tbl.entry_add(self.target, [key], [data])
        # Table: hash_for_partition_tbl (default: nop; size: 5)
        print("Configuring hash_for_partition_tbl")
        for tmpoptype in [GETREQ, PUTREQ, DELREQ, LOADREQ, PUTREQ_LARGEVALUE]:
            key = self.hash_for_partition_tbl.make_key([
                gc.KeyTuple('hdr.op_hdr.optype', tmpoptype)])
            data = self.hash_for_partition_tbl.make_data([],
                                         'nocahceIngress.hash_for_partition')
            self.hash_for_partition_tbl.entry_add(self.target, [key], [data])

        print("Configuring hash_partition_tbl")
        hash_range_per_server = switch_partition_count / server_total_logical_num
        for tmpoptype in [GETREQ, PUTREQ, DELREQ, LOADREQ, PUTREQ_LARGEVALUE]:
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
                        'nocahceIngress.hash_partition')
                    self.hash_partition_tbl.entry_add(self.target, [key], [data])
                hash_start = hash_end + 1
        
        print("Configuring ipv4_forward_tbl")
        for tmp_client_physical_idx in range(client_physical_num):
            ipv4addr = client_ips[tmp_client_physical_idx]
            eport = self.client_devports[tmp_client_physical_idx]
            # tmpsid = self.client_sids[tmp_client_physical_idx]
            for tmpoptype in [GETRES, PUTRES, DELRES, SCANRES_SPLIT, LOADACK, GETRES_LARGEVALUE]:
                key = self.ipv4_forward_tbl.make_key([
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('hdr.ipv4_hdr.dstAddr', ipv4addr, prefix_len=32)
                ])
                data = self.ipv4_forward_tbl.make_data(
                    [gc.DataTuple('eport', eport)],
                    'nocahceIngress.forward_normal_response')
                self.ipv4_forward_tbl.entry_add(self.target, [key], [data])

        # Table: update_ipmac_srcport_tbl (default: nop; 5*client_physical_num+5*server_physical_num=20 < 10*8=80 < 128)
        # NOTE: udp.dstport is updated by eg_port_forward_tbl (only required by switch2switchos)
        # NOTE: update_ipmac_srcport_tbl focues on src/dst ip/mac and udp.srcport
        print("Configuring update_ipmac_srcport_tbl")
        # (1) for response from server to client, egress port has been set based on ip.dstaddr (or by clone_i2e) in ingress pipeline
        # (2) for response from switch to client, egress port has been set by clone_e2e in egress pipeline
        for tmp_client_physical_idx in range(client_physical_num):
            tmp_devport = self.client_devports[tmp_client_physical_idx]
            tmp_client_mac = client_macs[tmp_client_physical_idx]
            tmp_client_ip = client_ips[tmp_client_physical_idx]
            tmp_server_mac = server_macs[0]
            tmp_server_ip = server_ips[0]
            data = self.update_ipmac_srcport_tbl.make_data(
                [gc.DataTuple('client_mac', tmp_client_mac),
                gc.DataTuple('server_mac', tmp_server_mac),
                gc.DataTuple('client_ip', tmp_client_ip),
                gc.DataTuple('server_ip', tmp_server_ip),
                gc.DataTuple('server_port', server_worker_port_start)],
                'nocahceEgress.update_ipmac_srcport_server2client')
            for tmpoptype in [GETRES, PUTRES, DELRES, SCANRES_SPLIT, LOADACK, GETRES_LARGEVALUE]:
                key = self.update_ipmac_srcport_tbl.make_key([
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('eg_intr_md.egress_port', tmp_devport)
                ])
                self.update_ipmac_srcport_tbl.entry_add(self.target, [key], [data])

        # for request from client to server, egress port has been set by partition_tbl in ingress pipeline
        for tmp_server_physical_idx in range(server_physical_num):
            tmp_devport = self.server_devports[tmp_server_physical_idx]
            tmp_server_mac = server_macs[tmp_server_physical_idx]
            tmp_server_ip = server_ips[tmp_server_physical_idx]
            data = self.update_ipmac_srcport_tbl.make_data(
                [gc.DataTuple('server_mac', tmp_server_mac),
                gc.DataTuple('server_ip', tmp_server_ip)],
                'nocahceEgress.update_dstipmac_client2server')
            for tmpoptype in [GETREQ, PUTREQ, DELREQ, SCANREQ_SPLIT, LOADREQ, PUTREQ_LARGEVALUE]:
                key = self.update_ipmac_srcport_tbl.make_key([
                    gc.KeyTuple('hdr.op_hdr.optype', tmpoptype),
                    gc.KeyTuple('eg_intr_md.egress_port', tmp_devport)
                ])
                self.update_ipmac_srcport_tbl.entry_add(self.target, [key], [data])

