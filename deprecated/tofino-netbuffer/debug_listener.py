#!/usr/bin/env python

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

OP_PORT = 111
dst_if = "ens3f0"

class Custom(Packet):
    name = "OP"
    fields_desc = [
        # TODO: your fields
        IntField("optype", 0), \
        IntField("threadid", 0), \
        BitField("key", 0, 64)
    ]

bind_layers(UDP, Custom, dport=OP_PORT)

cnt = 0

def handlePacket(packet):
    global cnt
    cnt = cnt + 1
    print("\nIndex of the packet: {}".format(cnt))

    if packet[Ether].type != ETHERTYPE_IPV4:
        print("Not IPV4 pkt!")
        return
    
    packet.show()

    #print("srcmac:{} dstmac: {}, srcip: {}, dstip: {}\n".format(\
    #        packet[Ether].src, packet[Ether].dst, packet[IP].src, packet[IP].dst))

    #if packet.haslayer("Custom"):
    #    print("[Custom info] srcip: {}, dstip: {}, srcport: {}, dstport: {}, protocol: {}\n".format(\
    #            packet[Custom].srcIP, packet[Custom].dstIP, packet[Custom].srcPort, packet[Custom].dstPort, packet[Custom].protocol))
    #    print("Result: {}\n".format(packet[Custom].result))

def main():
    print("Sniff custom packet to get result (listening)...")
    sniff(iface=dst_if, prn=lambda x: handlePacket(x), count=0)

if __name__ == "__main__":
    main()

