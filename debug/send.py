#!/usr/bin/env python3
import random
import socket
import sys

from scapy.all import IP, UDP, Ether, get_if_hwaddr, get_if_list, sendp, Raw


def get_if():
    ifs = get_if_list()
    iface = None  # "h1-eth0"
    for i in get_if_list():
        if "eth0" in i:
            iface = i
            break
    if not iface:
        print("Cannot find eth0 interface")
        exit(1)
    return iface


# GET
# GETpayload = b'\x00\x30'+ \
#             b'\x00\x00\x00\x00\x00\x00\x00\x00\x40\xc2\x6e\xd1\x62\x5a\x1a\x66'
# PUT
# PUTpayload = b'\x00\x01'+b'\x00\x00\x00\x00\x00\x00\x00\x00\x40\xc2\x6e\xd1\x62\x5a\x1a\x66'+ \
#             b'\x00\x80'+ \
#             b'\x3a\x20\x64\x2f\x2b\x68\x3a\x4f'+ \
#             b'\x61\x22\x3e\x76\x2b\x5f\x33\x3c'+ \
#             b'\x39\x7c\x23\x20\x2e\x3c\x3d\x3a'+ \
#             b'\x2d\x35\x3a\x20\x43\x73\x3e\x54'+ \
#             b'\x2f\x25\x2b\x24\x2f\x5d\x37\x30'+ \
#             b'\x5d\x67\x29\x4f\x61\x32\x5b\x75'+ \
#             b'\x3a\x4d\x6b\x22\x2c\x30\x21\x29'+ \
#             b'\x60\x36\x45\x2f\x37\x58\x75\x32'+ \
#             b'\x26\x60\x36\x45\x75\x23\x47\x69'+ \
#             b'\x31\x3a\x6e\x3e\x59\x6d\x21\x5d'+ \
#             b'\x71\x26\x2a\x3a\x31\x25\x2c\x31'+ \
#             b'\x52\x7b\x38\x3f\x7a\x2e\x3c\x36'+ \
#             b'\x24\x29\x2c\x31\x45\x77\x2e\x26'+ \
#             b'\x20\x24\x4a\x73\x34\x26\x72\x37'+ \
#             b'\x22\x6e\x23\x34\x70\x2e\x50\x79'+ \
#             b'\x21\x56\x35\x25\x28\x2c\x2c\x37'+ \
#             b'\x00\x01'
GETpayload = bytes.fromhex("0030" + "00000000000000006d14352c081723ae")
WARMUPpayload = bytes.fromhex("0000" + "00000000000000006d14352c081723ae")
PUTpayload = bytes.fromhex(
    "0001"
    + "00000000000000006d14352c081723ae"
    + "00802f377c3e57232c373e2051392024303d4627315b2f202a6c3f2b66324529292066373c7c3746672a5b332d5a312935662c382625573f23576327252828227633587127536d3b253a355439385e77322c34202970235c2938462b2e486722557d3c46753238222250733f"
    + "6a6a6a3f7c222066345475372a6e2028383c3e382f30"
)


def main():
    addr = socket.gethostbyname("10.0.1.5")
    iface = get_if()

    print("sending on interface %s to %s" % (iface, str(addr)))
    pkt = Ether(src=get_if_hwaddr(iface), dst="00:00:0a:00:01:03")
    # print(GETpayload.hex())
    pkt = (
        pkt
        / IP(dst=addr)
        / UDP(dport=1152, sport=random.randint(49152, 65535))
        / Raw(load=GETpayload)
    )
    pkt.show2()
    # 计算校验和
    # pkt[UDP].chksum = None
    # pkt[UDP].chksum = UDP(str(pkt[UDP]).encode()).chksum
    # # 验证校验和是否正确
    # if pkt[UDP].chksum == 0:
    # print('Checksum',hex(UDP(str(pkt[UDP]).encode()).chksum))
    sendp(pkt, iface=iface, verbose=False)


if __name__ == "__main__":
    main()
