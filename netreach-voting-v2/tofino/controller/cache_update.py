import time
import signal, os
import socket
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

r = redis.Redis(host=redis_ip, port=redis_port, decode_responses=True)
for key in r.scan_iter("prefix:*"):
   r.delete(key)

def handler(signum, frame):
    global running
    print("Receive signal {}".format(signum))
    running = False
signal.signal(signal.SIGTERM, handler)

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((controller_ip, controller_port))
s.setblocking(0)

# Polling
while running:
    try:
        data, addr = s.recvfrom(1024)
        if len(data) > 0:
            rval = os.system("{} --data {}".format(cmd, data.decode("utf-8")))
    except Exception e:
        print(e)
        continue

