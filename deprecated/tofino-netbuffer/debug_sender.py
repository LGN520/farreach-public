#!/usr/bin/env python

# Execute on h1

from scapy.fields import IPField, BitField, ByteField, ShortField, ByteEnumField, \
        IntField, ShortEnumField, StrField, XByteField, XShortField
from scapy.layers.inet import IP, UDP, TCP
from scapy.layers.l2 import Ether
from scapy.packet import Packet, bind_layers
from scapy.all import *

from time import sleep

ETHERTYPE_IPV4 = 0x0800
PROTOTYPE_TCP = 0x06
PROTOTYPE_UDP = 0x11
OP_PORT = 1111

src_mac = "9c:69:b4:60:ef:a4"
dst_mac = "9c:69:b4:60:ef:8d"
src_ip = "10.0.0.31"
dst_ip = "10.0.0.32"
src_if = "ens3f0"
dst_if = "ens3f1"

class Custom(Packet):
    name = "OP"
    fields_desc = [
        # TODO: your fields
        IntField("optype", 0), \
        IntField("threadid", 0), \
        BitField("key", 0, 64)
    ]

bind_layers(UDP, Custom, dport=OP_PORT)

def main():
    print("Send custom packet to query...")
    pkt = Ether(src=src_mac, dst=dst_mac, type=ETHERTYPE_IPV4) / \
            IP(src=src_ip, dst=dst_ip, proto=PROTOTYPE_UDP) / UDP(dport=OP_PORT) / Custom() #TODO
    sendp(pkt, iface=src_if, verbose=0)

if __name__ == "__main__":
    main()

