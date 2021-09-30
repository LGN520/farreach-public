import time
import signal, os

running = True
cmd = "bash backup.sh"

def handler(signum, frame):
    global running
    print("Receive signal {}".format(signum))
    running = False


backup_interupt = 5 # 5s

while running:
    time.sleep(backup_interupt)
    rval = os.system(cmd)

