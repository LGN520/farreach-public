import time
import signal, os
import socket

running = True
cmd = "$SDE/run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/netreach-voting-v2/tofino/cache_update/ --target hw --setup"

this_dir = os.path.dirname(os.path.abspath(__file__))

import ConfigParser
config = ConfigParser.ConfigParser()
with open(os.path.join(os.path.dirname(os.path.dirname(this_dir)), "config.ini"), "r") as f:
    config.readfp(f)

controller_ip = str(config.get("controller", "controller_ip"))
controller_port = int(config.get("controller", "controller_port"))

def handler(signum, frame):
    global running
    print("Receive signal {}".format(signum))
    running = False
signal.signal(signal.SIGTERM, handler)

#s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#s.bind((controller_ip, controller_port))
#s.setblocking(0)

# Polling
while running:
    try:
        #data, addr = s.recvfrom(1024)
        data = b"\x01\x02\x03"
        if len(data) > 0:
            rval = os.system("{} --data {}".format(cmd, data.decode("utf-8")))
            break; #TMP
    except Exception e:
        print(e)
        continue

