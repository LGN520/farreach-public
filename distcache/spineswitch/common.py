import configparser
import os

hot_threshold = 20  # for 200 keys under 2 server threads for YCSB-based workload
# hot_threshold = 100 # for 200/500 keys under 16/2 server threads (around from 125) for NetCache synthetic workload
# hot_threshold = 200 # for 500 keys under 16/2 server threads w/o WAL or 1000 keys under 16/2 server threads

this_dir = os.path.dirname(os.path.abspath(__file__))

config = configparser.ConfigParser()
with open(os.path.join(os.path.dirname(this_dir), "config.ini"), "r") as f:
    config.readfp(f)

workload_name = config.get("global", "workload_name")
dynamic_ruleprefix = config.get("global", "dynamic_ruleprefix")

client_physical_num = int(config.get("global", "client_physical_num"))
server_physical_num = int(config.get("global", "server_physical_num"))
server_total_logical_num = int(config.get("global", "server_total_logical_num"))

server_worker_port_start = int(config.get("server", "server_worker_port_start"))
switchos_popserver_port = int(config.get("switch", "switchos_popserver_port"))

# kv_bucket_num
switch_kv_bucket_num = int(config.get("switch", "switch_kv_bucket_num"))
switch_partition_count = int(config.get("switch", "switch_partition_count"))
switch_max_vallen = int(config.get("switch", "switch_max_vallen"))
switchos_sample_cnt = int(config.get("switch", "switchos_sample_cnt"))
switchos_ptf_popserver_port = int(config.get("switch", "switchos_ptf_popserver_port"))
switchos_ptf_cachefrequencyserver_port = int(
    config.get("spineswitch0", "switchos_ptf_cachefrequencyserver_port")
)
switchos_ptf_snapshotserver_port = int(
    config.get("switch", "switchos_ptf_snapshotserver_port")
)

# reflector port
reflector_dp2cpserver_port = int(config.get("reflector", "reflector_dp2cpserver_port"))
reflector_ip_for_switchos = str(config.get("reflector", "reflector_ip_for_switchos"))
reflector_ips = []
for i in range(int(server_physical_num / 2)):
    reflector_ips.append(str(config.get(f"reflector{i}", "reflector_ip_for_switchos")))
# Front Panel Ports
#   List of front panel ports to use. Each front panel port has 4 channels.
#   Port 1 is broken to 1/0, 1/1, 1/2, 1/3. Test uses 2 ports.
#
#   ex: ["1/0", "1/1"]
#
client_ips = []
client_macs = []
client_fpports = []
client_pipeidxes = []
for i in range(client_physical_num):
    client_ips.append(str(config.get("client{}".format(i), "client_ip")))
    client_macs.append(str(config.get("client{}".format(i), "client_mac")))
    client_fpports.append(str(config.get("client{}".format(i), "client_fpport")))
    client_pipeidxes.append(int(config.get("client{}".format(i), "client_pipeidx")))
server_ips = []
server_macs = []
server_fpports = []
server_pipeidxes = []
server_logical_idxes_list = []
server_ip_for_controller_list = []
for i in range(server_physical_num):
    server_ips.append(str(config.get("server{}".format(i), "server_ip")))
    server_macs.append(str(config.get("server{}".format(i), "server_mac")))
    server_fpports.append(str(config.get("server{}".format(i), "server_fpport")))
    server_pipeidxes.append(int(config.get("server{}".format(i), "server_pipeidx")))
    server_ip_for_controller_list.append(
        str(config.get("server{}".format(i), "server_ip_for_controller"))
    )
    tmpstr = str(config.get("server{}".format(i), "server_logical_idxes"))
    server_logical_idxes = tmpstr.split(":")
    for j in range(len(server_logical_idxes)):
        server_logical_idxes[j] = int(server_logical_idxes[j])
    server_logical_idxes_list.append(server_logical_idxes)

control_config = configparser.ConfigParser()
with open(os.path.join(os.path.dirname(this_dir), "control_type.ini"), "r") as f:
    control_config.readfp(f)


SWITCHOS_ADD_CACHE_LOOKUP = int(
    control_config.get("switchos", "SWITCHOS_ADD_CACHE_LOOKUP")
)
SWITCHOS_ADD_CACHE_LOOKUP_ACK = int(
    control_config.get("switchos", "SWITCHOS_ADD_CACHE_LOOKUP_ACK")
)
SWITCHOS_REMOVE_CACHE_LOOKUP = int(
    control_config.get("switchos", "SWITCHOS_REMOVE_CACHE_LOOKUP")
)
SWITCHOS_REMOVE_CACHE_LOOKUP_ACK = int(
    control_config.get("switchos", "SWITCHOS_REMOVE_CACHE_LOOKUP_ACK")
)
SWITCHOS_CLEANUP = int(control_config.get("switchos", "SWITCHOS_CLEANUP"))
SWITCHOS_CLEANUP_ACK = int(control_config.get("switchos", "SWITCHOS_CLEANUP_ACK"))
SWITCHOS_ENABLE_SINGLEPATH = int(
    control_config.get("switchos", "SWITCHOS_ENABLE_SINGLEPATH")
)
SWITCHOS_ENABLE_SINGLEPATH_ACK = int(
    control_config.get("switchos", "SWITCHOS_ENABLE_SINGLEPATH_ACK")
)
SWITCHOS_SET_SNAPSHOT_FLAG = int(
    control_config.get("switchos", "SWITCHOS_SET_SNAPSHOT_FLAG")
)
SWITCHOS_SET_SNAPSHOT_FLAG_ACK = int(
    control_config.get("switchos", "SWITCHOS_SET_SNAPSHOT_FLAG_ACK")
)
SWITCHOS_DISABLE_SINGLEPATH = int(
    control_config.get("switchos", "SWITCHOS_DISABLE_SINGLEPATH")
)
SWITCHOS_DISABLE_SINGLEPATH_ACK = int(
    control_config.get("switchos", "SWITCHOS_DISABLE_SINGLEPATH_ACK")
)
SWITCHOS_LOAD_SNAPSHOT_DATA = int(
    control_config.get("switchos", "SWITCHOS_LOAD_SNAPSHOT_DATA")
)
SWITCHOS_LOAD_SNAPSHOT_DATA_ACK = int(
    control_config.get("switchos", "SWITCHOS_LOAD_SNAPSHOT_DATA_ACK")
)
SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG = int(
    control_config.get("switchos", "SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG")
)
SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG_ACK = int(
    control_config.get("switchos", "SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG_ACK")
)
SWITCHOS_PTF_POPSERVER_END = int(
    control_config.get("switchos", "SWITCHOS_PTF_POPSERVER_END")
)
SWITCHOS_PTF_SNAPSHOTSERVER_END = int(
    control_config.get("switchos", "SWITCHOS_PTF_SNAPSHOTSERVER_END")
)
SNAPSHOT_GETDATA_ACK = int(control_config.get("snapshot", "SNAPSHOT_GETDATA_ACK"))
CACHE_FREQUENCYREQ = int(control_config.get("snapshot", "CACHE_FREQUENCYREQ"))
CACHE_FREQUENCYACK = int(control_config.get("snapshot", "CACHE_FREQUENCYACK"))
SETDELETEDREQ = int(control_config.get("snapshot", "SETDELETEDREQ"))
SETDELETEDACK = int(control_config.get("snapshot", "SETDELETEDACK"))
# Set it as True if support range, or False otherwise
# NOTE: update RANGE_SUPPORT in main.p4 accordingly
RANGE_SUPPORT = False
# RANGE_SUPPORT = True

# 0b0001
PUTREQ = 0x0001
# WARMUPREQ = 0x0011
# LOADREQ = 0x0021
# 0b0011
PUTREQ_SEQ = 0x0003
PUTREQ_POP_SEQ = 0x0013
PUTREQ_SEQ_CASE3 = 0x0023
PUTREQ_POP_SEQ_CASE3 = 0x0033
NETCACHE_PUTREQ_SEQ_CACHED = 0x0043
PUTREQ_SEQ_BEINGEVICTED = 0x0053
PUTREQ_SEQ_CASE3_BEINGEVICTED = 0x0063
PUTREQ_SEQ_BEINGEVICTED_SPINE = 0x0073
PUTREQ_SEQ_CASE3_BEINGEVICTED_SPINE = 0x0083
# 0b0110
DELREQ_SEQ_INSWITCH = 0x0006
# For large value
PUTREQ_LARGEVALUE_SEQ_INSWITCH = 0x0016
# 0b0111
# 0b1111
GETRES_LATEST_SEQ_INSWITCH = 0x000F
GETRES_DELETED_SEQ_INSWITCH = 0x001F
GETRES_LATEST_SEQ_INSWITCH_CASE1 = 0x002F
GETRES_DELETED_SEQ_INSWITCH_CASE1 = 0x003F
PUTREQ_SEQ_INSWITCH_CASE1 = 0x004F
DELREQ_SEQ_INSWITCH_CASE1 = 0x005F
LOADSNAPSHOTDATA_INSWITCH_ACK = 0x006F
CACHE_POP_INSWITCH = 0x007F
CACHE_POP_INSWITCH_SPINE = 0x017F
NETCACHE_VALUEUPDATE_INSWITCH = 0x008F
# For large value
NETCACHE_CACHE_POP_INSWITCH_NLATEST = 0x015F
# 0b1011
GETRES_LATEST_SEQ = 0x000B
GETRES_DELETED_SEQ = 0x001B
CACHE_EVICT_LOADDATA_INSWITCH_ACK = 0x002B
NETCACHE_VALUEUPDATE = 0x003B
# 0b1001
GETRES = 0x0009
# 0b0101
PUTREQ_INSWITCH = 0x0005
# 0b0100
GETREQ_INSWITCH = 0x0004
DELREQ_INSWITCH = 0x0014
CACHE_EVICT_LOADFREQ_INSWITCH = 0x0024
CACHE_EVICT_LOADDATA_INSWITCH = 0x0034
LOADSNAPSHOTDATA_INSWITCH = 0x0044
SETVALID_INSWITCH = 0x0054
NETCACHE_WARMUPREQ_INSWITCH = 0x0064
NETCACHE_WARMUPREQ_INSWITCH_POP = 0x0074
# For large value
PUTREQ_LARGEVALUE_INSWITCH = 0x00A4
# 0b0010
DELREQ_SEQ = 0x0002
DELREQ_SEQ_CASE3 = 0x0012
NETCACHE_DELREQ_SEQ_CACHED = 0x0022
# For large value (PUTREQ_LARGEVALUE_SEQ_CACHED ONLY for netcache/distcache; PUTREQ_LARGEVALUE_SEQ_CASE3 ONLY for farreach/distfarreach)
PUTREQ_LARGEVALUE_SEQ = 0x0032
PUTREQ_LARGEVALUE_SEQ_CACHED = 0x0042
PUTREQ_LARGEVALUE_SEQ_CASE3 = 0x0052
# For read blocking under cache eviction rare case
DELREQ_SEQ_BEINGEVICTED = 0x0062
DELREQ_SEQ_CASE3_BEINGEVICTED = 0x0072
PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED = 0x0082
PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED = 0x0092
DELREQ_SEQ_BEINGEVICTED_SPINE = 0x00A2
DELREQ_SEQ_CASE3_BEINGEVICTED_SPINE = 0x00B3
PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED_SPINE = 0x00C2
PUTREQ_LARGEVALUE_SEQ_CASE3_BEINGEVICTED_SPINE = 0x00D2
# 0b1000
PUTRES = 0x0008
DELRES = 0x0018
# 0b1010
PUTRES_SEQ = 0x000A
DELRES_SEQ = 0x001A
# 0b0000
WARMUPREQ = 0x0000
SCANREQ = 0x0010
SCANREQ_SPLIT = 0x0020
GETREQ = 0x0030
DELREQ = 0x0040
GETREQ_POP = 0x0050
GETREQ_NLATEST = 0x0060
CACHE_POP_INSWITCH_ACK = 0x0070
SCANRES_SPLIT = 0x0080
CACHE_POP = 0x0090
CACHE_EVICT = 0x00A0
CACHE_EVICT_ACK = 0x00B0
CACHE_EVICT_CASE2 = 0x00C0
WARMUPACK = 0x00D0
LOADACK = 0x00E0
CACHE_POP_ACK = 0x00F0
CACHE_EVICT_LOADFREQ_INSWITCH_ACK = 0x0100
SETVALID_INSWITCH_ACK = 0x0110
NETCACHE_GETREQ_POP = 0x0120
NETCACHE_CACHE_POP = 0x0130
NETCACHE_CACHE_POP_ACK = 0x0140
NETCACHE_CACHE_POP_FINISH = 0x0150
NETCACHE_CACHE_POP_FINISH_ACK = 0x0160
NETCACHE_CACHE_EVICT = 0x0170
NETCACHE_CACHE_EVICT_ACK = 0x0180
NETCACHE_VALUEUPDATE_ACK = 0x0190
# For large value (NETCACHE_CACHE_POP_ACK_NLATEST is ONLY used by end-hosts)
PUTREQ_LARGEVALUE = 0x02D0
DISTNOCACHE_PUTREQ_LARGEVALUE_SPINE = 0x02E0
GETRES_LARGEVALUE_SERVER = 0x02F0
GETRES_LARGEVALUE = 0x0300
LOADREQ = 0x0310
LOADREQ_SPINE = 0x0320
NETCACHE_CACHE_POP_ACK_NLATEST = 0x0330
GETREQ_BEINGEVICTED = 0x0340
GETRES_SEQ = 0x006B
GETRES_LARGEVALUE_SEQ = 0x0350

GETREQ_SPINE = 0x0200
