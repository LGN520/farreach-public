import ConfigParser

this_dir = os.path.dirname(os.path.abspath(__file__))

config = ConfigParser.ConfigParser()
with open(os.path.join(os.path.dirname(this_dir), "config.ini"), "r") as f:
    config.readfp(f)

server_num = int(config.get("server", "server_num"))
server_port = int(config.get("server", "server_port"))
kv_bucket_num = int(config.get("switch", "switch_kv_bucket_num"))
partition_count = int(config.get("switch", "partition_count"))
src_mac = str(config.get("client", "client_mac"))
dst_mac = str(config.get("server", "server_mac"))
src_ip = str(config.get("client", "client_ip"))
dst_ip = str(config.get("server", "server_ip"))
switch_max_vallen = int(config.get("switch", "switch_max_vallen"))
ingress_pipeidx = int(config.get("hardware", "ingress_pipeidx"))
egress_pipeidx = int(config.get("hardware", "egress_pipeidx"))
switchos_paramserver_port = int(config.get("switch", "switchos_paramserver_port"))
switchos_sample_cnt = int(config.get("switch", "switchos_sample_cnt"))
switchos_snapshotdataserver_port = int(config.get("switch", "switchos_snapshotdataserver_port"))
reflector_port = int(config.get("reflector", "reflector_port"))

control_config = ConfigParser.ConfigParser()
with open(os.path.join(os.path.dirname(this_dir), "control_type.ini"), "r") as f:
    control_config.readfp(f)

SWITCHOS_GET_FREEIDX = int(control_config.get("switchos", "switchos_get_freeidx"))
SWITCHOS_GET_KEY_FREEIDX = int(control_config.get("switchos", "switchos_get_key_freeidx"))
SWITCHOS_SET_EVICTDATA = int(control_config.get("switchos", "switchos_set_evictdata"))
SWITCHOS_GETCACHEDEMPTYINDEX = int(control_config.get("switchos", "switchos_get_cachedemptyindex"))
SWITCHOS_GET_EVICTKEY = int(control_config.get("switchos", "switchos_get_evictkey"))

# Front Panel Ports
#   List of front panel ports to use. Each front panel port has 4 channels.
#   Port 1 is broken to 1/0, 1/1, 1/2, 1/3. Test uses 2 ports.
#
#   ex: ["1/0", "1/1"]
#
fp_ports = []
src_fpport = str(config.get("hardware", "src_fpport"))
fp_ports.append(src_fpport)
dst_fpport = str(config.get("hardware", "dst_fpport"))
fp_ports.append(dst_fpport)
#fp_ports = ["2/0", "3/0"]

# Set it as True if support range, or False otherwise
RANGE_SUPPORT = False

# 0b0001
PUTREQ = 0x01
# 0b0011
GETRES_LATEST_SEQ = 0x03
GETRES_DELETED_SEQ = 0x13
PUTREQ_SEQ = 0x23
PUTREQ_POP_SEQ = 0x33
PUTREQ_SEQ_CASE3 = 0x43
PUTREQ_POP_SEQ_CASE3 = 0x53
# 0b0111
GETRES_LATEST_SEQ_INSWITCH = 0x07
GETRES_DELETED_SEQ_INSWITCH = 0x17
CACHE_POP_INSWITCH = 0x27
# 0b1111
GETRES_LATEST_SEQ_INSWITCH_CASE1 = 0x0f
GETRES_DELETED_SEQ_INSWITCH_CASE1 = 0x1f
PUTREQ_SEQ_INSWITCH_CASE1 = 0x2f
DELREQ_SEQ_INSWITCH_CASE1 = 0x3f
# 0b1001
GETRES = 0x09
# 0b0101
PUTREQ_INSWITCH = 0x05
# 0b0100
GETREQ_INSWITCH = 0x04
DELREQ_INSWITCH = 0x14
# 0b0010
DELREQ_SEQ = 0x02
DELREQ_SEQ_CASE3 = 0x12
# 0b1000
PUTRES = 0x08
DELRES = 0x18
# 0b0000
SCANREQ = 0x10
SCANREQ_SPLIT = 0x20
GETREQ = 0x30
DELREQ = 0x40
GETREQ_POP = 0x50
GETREQ_NLATEST = 0x60
CACHE_POP_INSWITCH_ACK = 0x70
SCANRES_SPLIT = 0x80
CACHE_POP = 0x90
CACHE_EVICT = 0xa0
CACHE_EVICT_ACK = 0xb0
CACHE_EVICT_CASE2 = 0xc0

#GETREQ = 0x00
#PUTREQ = 0x01
#DELREQ = 0x02
#SCANREQ = 0x03
#GETRES = 0x04
#PUTRES = 0x05
#DELRES = 0x06
#SCANRES_SPLIT = 0x07
#GETREQ_INSWITCH = 0x08
#GETREQ_POP = 0x09
#GETREQ_NLATEST = 0x0a
#GETRES_LATEST_SEQ = 0x0b
#GETRES_LATEST_SEQ_INSWITCH = 0x0c
#GETRES_LATEST_SEQ_INSWITCH_CASE1 = 0x0d
#GETRES_DELETED_SEQ = 0x0e
#GETRES_DELETED_SEQ_INSWITCH = 0x0f
#GETRES_DELETED_SEQ_INSWITCH_CASE1 = 0x10
#PUTREQ_INSWITCH = 0x11
#PUTREQ_SEQ = 0x12
#PUTREQ_POP_SEQ = 0x13
#PUTREQ_SEQ_INSWITCH_CASE1 = 0x14
#PUTREQ_SEQ_CASE3 = 0x15
#PUTREQ_POP_SEQ_CASE3 = 0x16
#DELREQ_INSWITCH = 0x17
#DELREQ_SEQ = 0x18
#DELREQ_SEQ_INSWITCH_CASE1 = 0x19
#DELREQ_SEQ_CASE3 = 0x1a
#SCANREQ_SPLIT = 0x1b
#CACHE_POP = 0x1c
#CACHE_POP_INSWITCH = 0x1d
#CACHE_POP_INSWITCH_ACK = 0x1e
#CACHE_EVICT = 0x1f
#CACHE_EVICT_ACK = 0x20
#CACHE_EVICT_CASE2 = 0x21

# u16val in [0, 2^16-1] -> i16val in [-2^15, 2^15-1]
def convert_u16_to_i16(u16val):
    if u16val < 0 or u16val >= pow(2, 16):
        print "Invalid u16val: {}".format(u16val)
    if u16val >= pow(2, 15): # [2^15, 2^16-1] -> [-2^15, -1]
        i16val = u16val - pow(2, 16)
    else: # [0, 2^15-1] -> [0, 2^15-1]
        i16val = u16val
    return i16val

# i16val in [-2^15, 2^15-1] -> u16val in [0, 2^16-1]
def convert_i16_to_u16(i16val):
    if i16val < -pow(2, 15) or i16val >= pow(2, 15):
        print "Invalid i16val: {}".format(i16val)
    if i16val < 0: # [-2^15, -1] -> [2^15, 2^16-1]
        u16val = i16val + pow(2, 16)
    else: # [0, 2^15-1] -> [0, 2^15-1]
        u16val = i16val
    return u16val

# u32val in [0, 2^32-1] -> i32val in [-2^31, 2^31-1]
def convert_u32_to_i32(u32val):
    if u32val < 0 or u32val >= pow(2, 32):
        print "Invalid u32val: {}".format(u32val)
    if u32val >= pow(2, 31): # [2^31, 2^32-1] -> [-2^31, -1]
        i32val = u32val - pow(2, 32)
    else: # [0, 2^31-1] -> [0, 2^31-1]
        i32val = u32val
    return i32val

# i32val in [-2^31, 2^31-1] -> u32val in [0, 2^32-1]
def convert_i32_to_u32(i32val):
    if i32val < -pow(2, 31) or i32val >= pow(2, 31):
        print "Invalid i32val: {}".format(i32val)
    if i32val < 0: # [-2^31, -1] -> [2^31, 2^32-1]
        u32val = i32val + pow(2, 32)
    else: # [0, 2^31-1] -> [0, 2^31-1]
        u32val = i32val
    return u32val
