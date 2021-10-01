import socket

ETH_P_ALL = 3

def dumpObject(obj):
    for attr in dir(obj):
        if hasattr(obj, attr):
            print("obj.%s = %s" % (attr, getattr(obj, attr)))

def open_packet_socket(hostif_name):
    s = socket.socket(socket.AF_PACKET, socket.SOCK_RAW,
                      socket.htons(ETH_P_ALL))
    s.bind((hostif_name, ETH_P_ALL))
    s.setblocking(True)
    return s

s = open_packet_socket("veth251")
#s = open_packet_socket("test_hostif1")

resp = s.recv(4096)
dumpObject(resp)
close(s)
