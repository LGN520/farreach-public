import time
import signal, os
import socket
from threading import Thread, Lock
import redis

mutex = Lock()
running = True
cmd = "$SDE/run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/netreach-voting-v2/tofino/cache_update/ --target hw --setup"
phase1_cmd = "$SDE/run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/netreach-voting-v2/tofino/phase1/ --target hw --setup"
phase2_cmd = "$SDE/run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/netreach-voting-v2/tofino/phase2/ --target hw --setup"

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
for key in r.scan_iter("*"):
   r.delete(key)

def handler(signum, frame):
    global running
    print("Receive signal {}".format(signum))
    running = False
signal.signal(signal.SIGTERM, handler)

#s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#s.bind((controller_ip, controller_port))
#s.setblocking(0)

def cache_update():
    # Polling
    global running, mutex
    while running:
        #data, addr = s.recvfrom(1024)
        mutex.acquire()
        data = b"\x01\x02\x03"
        if len(data) > 0:
            rval = os.system("{} --data {}".format(cmd, data.decode("utf-8")))
            mutex.release()
            break; #TMP
        else:
            mutex.release()

def periodic_backup():
    global running, mutex
    while running:
        time.sleep(backup_interupt)
        mutex.acquire()
        rval1 = os.system(phase1_cmd)
        rval2 = os.system(phase2_cmd)
        mutex.release()

if __name__ == "__main__":
    cache_update_thread = Thread(target = cache_update)
    cache_update_thread.start()
    periodic_backup_thread = Thread(target = periodic_backup)
    periodic_backup.start()

    cache_update_thread.join()
    periodic_backup_thread.join()
    print("[controller] All threads finish!")
