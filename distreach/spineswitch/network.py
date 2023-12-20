from mininet.net import Mininet
from p4utils.mininetlib.node import *
from mininet.cli import CLI
from mininet.log import setLogLevel, info
import subprocess
import os
from common import *

rack_physical_num = int(server_physical_num / 2)
current_dir = os.path.dirname(os.path.abspath(__file__))
method = "netbufferv4"
sw_path = subprocess.getstatusoutput("whereis simple_switch")[1].split(" ")[1]
p4_path = current_dir+"/"+method + ".p4"
json_path = current_dir+"/"+method + ".json"
partition_json_path = current_dir+"/"+"../spineswitch/partitionswitch.json"
print(partition_json_path)
def P4compile(p4_path, json_path):
    os.system("p4c-bm2-ss --p4v 16 " + p4_path + " -o  " + json_path)


host = []
rackswitchs = []
switchoses = []

debug=True

def create_network():
    net = Mininet(cleanup=True, autoStaticArp=True)
    c1 = net.addController("c1")
    # c2 = net.addController("c2")

    # Add switches
    for i in range(rack_physical_num):
        rackswitchs.append(
            net.addSwitch(
                "leaf_s{}".format(i + 1),
                cls=P4Switch,
                json_path=json_path,
                thrift_port=9090 + i + 1,
                pcap_dump=debug,
                pcap_dir="./pcap",
                log_enabled=debug,
                log_dir="./log",
                device_id=1 + i + 1,
            )
        )
    # 0 1 2 3 4
    client_s1 = net.addSwitch(
        "spine_s{}".format(0),
        cls=P4Switch,
        json_path=partition_json_path,
        thrift_port=9090 + 0,
        pcap_dump=debug,
        pcap_dir="./pcap",
        log_enabled=debug,
        log_dir="./log",
        device_id=1 + 0,
    )

    nat_s1 = net.addSwitch("nat_s1", Controller=c1)

    # Add hosts
    for i in range(client_physical_num + server_physical_num):
        host.append(
            net.addHost(
                "h{}".format(i + 1),
                ip="10.0.1.{}/24".format(i + 1),
                mac="00:00:0a:00:01:0{}".format(hex(i + 1)[2:]),
            )
        )

    for i in range(rack_physical_num):
        switchoses.append(
            net.addHost(
                "switchos{}".format(1 + i),
                ip="192.168.1.{}/24".format(200 + i + 1),
                mac="00:00:0a:00:01:{}".format(hex(200 + i + 1)[2:]),
            )
        )

    # Add links between hosts and switches
    for i in range(client_physical_num):
        net.addLink(host[i], client_s1)
    for i in range(rack_physical_num):
        # rack
        net.addLink(rackswitchs[i], host[2 + i * 2])
        net.addLink(rackswitchs[i], host[2 + i * 2 + 1])
        net.addLink(client_s1, rackswitchs[i])

    for i in range(client_physical_num + server_physical_num):
        net.addLink(host[i], nat_s1)
    # distnocachehas no switchos
    for i in range(rack_physical_num):
        net.addLink(switchoses[i], nat_s1)
    # host[i]
    for i in range(client_physical_num + server_physical_num):
        host[i].cmdPrint(
            "ip addr add 192.168.1.{}/24 broadcast 192.168.1.255 dev h{}-eth1".format(
                i + 1, i + 1
            )
        )

    # Add link between switches
    # net.addLink(s1, s2)

    # Add NAT node to the normal switch
    nat = net.addNAT(
        name="nat0", connect=nat_s1, ip="192.168.1.200" + "/24"
    ).configDefault()

    # Start the network and run the CLI
    net.start()
    for i in range(rack_physical_num):
        switchoses[i].cmdPrint("ip route add default via 192.168.1.200")
    CLI(net)
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
    os.system("mn -c")
    setLogLevel("info")
    # P4compile(p4_path, json_path)
    create_network()
