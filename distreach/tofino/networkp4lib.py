from p4utils.mininetlib.network_API import NetworkAPI
import argparse

net = NetworkAPI()
class StartP4Network:
    def run(self):
        net = NetworkAPI()
        # python2 ./bin/ycsb run keydump -P workloads/workloada -pi 0 -df <workloadname>

        # Network general options
        net.setLogLevel('info')
        # net.disableCli()

        # Network definition
        net.addP4Switch('s1')
        net.setP4SourceAll('netbufferv4.p4')
        
        # net.addSwitch('s2')

        net.addHost('h1')
        net.addHost('h2')
        net.addHost('h3')
        net.addHost('h4')
        # net.addHost('h5')

        net.addLink('h1','s1')
        net.addLink('h2','s1')
        # net.addLink('h1','s2')
        # net.addLink('h2','s2')
        net.addLink('h3','s1')
        net.addLink('h4','s1')

        # net.addLink('h5','s1')
        # Assignment strategy
        net.mixed()
        # # Nodes general options
        if args.disable_debug:
            net.enablePcapDumpAll()
            net.enableLogAll()
        else:
            pass
        
        # Start network
        net.startNetwork()



parser = argparse.ArgumentParser()
parser.add_argument('--disable-debug', dest='disable_debug', action='store_false',
                    help='disable debug mode')
args = parser.parse_args()
# print(args.disable_debug)
startp4network = StartP4Network()
startp4network.run()
