from mininet.net import Mininet
from p4utils.mininetlib.node import *
from mininet.cli import CLI
from mininet.log import setLogLevel, info
import subprocess
import os


method = "nocache"
sw_path = subprocess.getstatusoutput("whereis simple_switch")[1].split(" ")[1]
p4_path = method + ".p4"
json_path = method + ".json"


def P4compile(p4_path, json_path):
    os.system("p4c-bm2-ss --p4v 16 " + p4_path + " -o  " + json_path)


def create_network():
    net = Mininet(cleanup=True, autoStaticArp=True)
    c2 = net.addController("c2")

    # Add switches
    s1 = net.addSwitch(
        "s1",
        cls=P4Switch,
        json_path=json_path,
        thrift_port=9090,
        pcap_dump=True,
        pcap_dir="./pcap",
        log_enabled=False,
        log_dir="./log",
        device_id=1,
    )

    s2 = net.addSwitch("s2", Controller=c2)

    # Add hosts
    h1 = net.addHost("h1", ip="10.0.1.1" + "/24", mac="00:00:0a:00:01:01")
    h2 = net.addHost("h2", ip="10.0.1.2" + "/24", mac="00:00:0a:00:01:02")
    h3 = net.addHost("h3", ip="10.0.1.3" + "/24", mac="00:00:0a:00:01:03")
    h4 = net.addHost("h4", ip="10.0.1.4" + "/24", mac="00:00:0a:00:01:04")
    switchos = net.addHost(
        "switchos", ip="192.168.1.6" + "/24", mac="00:00:0a:00:01:06"
    )
    # Add links between hosts and switches
    net.addLink(h1, s1)
    net.addLink(h2, s1)
    net.addLink(h3, s1)
    net.addLink(h4, s1)

    net.addLink(h1, s2)
    net.addLink(h2, s2)
    net.addLink(h3, s2)
    net.addLink(h4, s2)
    net.addLink(switchos, s2)
    h1.cmdPrint("ip addr add 192.168.1.1/24 broadcast 192.168.1.255 dev h1-eth1")
    h2.cmdPrint("ip addr add 192.168.1.2/24 broadcast 192.168.1.255 dev h2-eth1")
    h3.cmdPrint("ip addr add 192.168.1.3/24 broadcast 192.168.1.255 dev h3-eth1")
    h4.cmdPrint("ip addr add 192.168.1.4/24 broadcast 192.168.1.255 dev h4-eth1")

    # Add link between switches
    # net.addLink(s1, s2)

    # Add NAT node to the normal switch
    nat = net.addNAT(name="nat0", connect=s2, ip="192.168.1.5" + "/24").configDefault()

    # Start the network and run the CLI
    net.start()
    switchos.cmdPrint("ip route add default via 192.168.1.5")

    def handler(signum, frame):
        print("Signal handler called with signal", signum)
        time.sleep(1)
        print("Continuing execution...")
        net.stop()
        exit(0)

    signal.signal(signal.SIGTERM, handler)
    while True:
        # print("Waiting for SIGTERM signal...")
        time.sleep(3)


if __name__ == "__main__":
    os.system('mn -c')
    setLogLevel("info")
    P4compile(p4_path, json_path)
    create_network()
