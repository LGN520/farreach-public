import time
import signal, os
import errno
import socket
import struct
import redis

running = True
cmd = "$SDE/run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/netreach-voting-v2/tofino/cache_update/ --target hw --setup"

# Get parameters
this_dir = os.path.dirname(os.path.abspath(__file__))
import ConfigParser
config = ConfigParser.ConfigParser()
with open(os.path.join(os.path.dirname(os.path.dirname(this_dir)), "config.ini"), "r") as f:
    config.readfp(f)
controller_ip = str(config.get("controller", "controller_ip"))
controller_port = int(config.get("controller", "controller_port"))
redis_ip = str(config.get("controller", "redis_ip"))
redis_port = int(config.get("controller", "redis_port"))
pop_prefix = str(config.get("other", "pop_prefix"))

def encode_popreq(data):
    # Parse key, vallen, value, hashidx, thread_id
    remainlen = len(data)
    remainlen -= 17 # Minus key and vallen
    keylo, keyhi, vallen, data = struct.unpack("=QQB{}s".format(remainlen), data)
    value_list = []
    for i in range(vallen):
        remainlen -= 8
        tmpval, data = struct.unpack("=Q{}s".format(remainlen), data)
        value_list.append(tmpval)
    hashidx, thread_id = struct.unpack("=HB", data)

    # Encode into a string
    res = "{}-{}-{}".format(keylo, keyhi, vallen)
    for i in range(vallen):
        res += "-{}".format(value_list[i])
    res += "-{}-{}".format(hashidx, thread_id)
    return res

def handler(signum, frame):
    global running
    print("Receive signal {}".format(signum))
    running = False
signal.signal(signal.SIGTERM, handler)

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((controller_ip, controller_port))
s.setblocking(0)
r = redis.Redis(host=redis_ip, port=redis_port, decode_responses=True)

# Polling
while running:
    try:
        data, addr = s.recvfrom(1024)
        r.set(pop_prefix, encode_popreq(data))
        if len(data) > 0:
            rval = os.system(cmd)
    except IOError as e:
        if e.errno == errno.EWOULDBLOCK:
            continue
        else:
            print(e)
            exit()
