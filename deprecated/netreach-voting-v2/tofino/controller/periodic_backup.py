import time
import signal, os

running = True
phase1_cmd = "$SDE/run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/netreach-voting-v2/tofino/phase1/ --target hw --setup"
phase2_cmd = "$SDE/run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/netreach-voting-v2/tofino/phase2/ --target hw --setup"

def handler(signum, frame):
    global running
    print("Receive signal {}".format(signum))
    running = False
signal.signal(signal.SIGTERM, handler)

# Get parameters
this_dir = os.path.dirname(os.path.abspath(__file__))
import ConfigParser
config = ConfigParser.ConfigParser()
with open(os.path.join(os.path.dirname(os.path.dirname(this_dir)), "config.ini"), "r") as f:
    config.readfp(f)
backup_interupt = int(config.get("controller", "backup_interupt"))

while running:
    time.sleep(backup_interupt)
    rval1 = os.system(phase1_cmd)
    rval2 = os.system(phase2_cmd)

