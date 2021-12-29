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
backup_interupt = int(config.get("controller", "backup_interupt"))
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
for key in r.scan_iter("*"):
   r.delete(key)

def cache_update():
    # Polling
    global running, mutex
    while running:
        try:
            data, addr = s.recvfrom(1024)
            r.set(pop_prefix, encode_popreq(data))
            mutex.acquire()
            if len(data) > 0:
                rval = os.system(cmd)
            mutex.release()
        except IOError as e:
            if e.errno == errno.EWOULDBLOCK:
                mutex.release()
                continue
            else:
                print(e)
                exit()

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
    periodic_backup_thread.start()

    cache_update_thread.join()
    periodic_backup_thread.join()
    print("[controller] All threads finish!")
