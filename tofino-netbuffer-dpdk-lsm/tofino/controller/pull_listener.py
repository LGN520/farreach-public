import time
import signal, os
import socket
#from io import BlockingIOError

running = True
cmd = "bash backup.sh"

listener_ip = "172.16.112.19"
listener_port = 3334

def handler(signum, frame):
    global running
    print("Receive signal {}".format(signum))
    running = False

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((listener_ip, listener_port))
s.setblocking(0)

# Polling
while running:
    try:
        data, addr = s.recvfrom(1024)
        if len(data) > 0:
            rval = os.system(cmd)
    #except BlockingIOError as e:
    except:
        continue

