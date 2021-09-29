import time
import signal, os
import socket

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
    data, addr = s.recvfrom(1024)
    if data > 0:
        print data
        print addr
        #rval = os.system(cmd)

