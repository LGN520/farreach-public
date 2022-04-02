import time
import signal, os

running = True
phase1_cmd = "$SDE/run_p4_tests.sh -p netbufferv4 -t /home/ssy/NetBuffer/netreach-voting-v3/tofino/phase1/ --target hw --setup"
phase2_cmd = "$SDE/run_p4_tests.sh -p netbufferv4 -t /home/ssy/NetBuffer/netreach-voting-v3/tofino/phase2/ --target hw --setup"

def handler(signum, frame):
    global running
    print("Receive signal {}".format(signum))
    running = False


backup_interupt = 5 # 5s

while running:
    time.sleep(backup_interupt)
    rval1 = os.system(phase1_cmd)
    rval2 = os.system(phase2_cmd)

