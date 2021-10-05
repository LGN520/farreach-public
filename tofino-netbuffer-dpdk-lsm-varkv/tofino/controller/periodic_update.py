import time
import signal, os

running = True
cmd = "$SDE/run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/tofino-netbuffer-dpdk-lsm-varkv/tofino/reportkv/ --target hw --setup"

def handler(signum, frame):
    global running
    print("Receive signal {}".format(signum))
    running = False


backup_interupt = 5 # 5s

while running:
    time.sleep(backup_interupt)
    rval = os.system(cmd)

