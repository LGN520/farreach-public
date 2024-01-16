#!/usr/bin/env python3

# Global information unchanged during experiments

CLIENT_ROOTPATH = "/root/P4/farreach-private"
SWITCH_ROOTPATH = "/root/P4/farreach-private"
SERVER_ROOTPATH = "/root/P4/farreach-private"

# backup rocksdb after loading phase
BACKUPS_ROOTPATH = "/tmp/rocksdbbackups"

# experiment
EVALUATION_SCRIPTS_PATH = "{CLIENT_ROOTPATH}/scriptsdist".format(
    CLIENT_ROOTPATH=CLIENT_ROOTPATH
)
EVALUATION_OUTPUT_PREFIX = "/root/aeresults"

# network settings
MAIN_CLIENT_TOSWITCH_IFNAME = "s1-eth1"
MAIN_CLIENT_TOSWITCH_IP = "10.0.1.1"
MAIN_CLIENT_TOSWITCH_MAC = "00:00:0a:00:01:01"
MAIN_CLIENT_TOSWITCH_FPPORT = "1/0"
MAIN_CLIENT_TOSWITCH_PIPEIDX = "1"
MAIN_CLIENT_LOCAL_IP = "10.0.1.1"


SECONDARY_CLIENT_TOSWITCH_IFNAME = "s1-eth2"
SECONDARY_CLIENT_TOSWITCH_IP = "10.0.1.2"
SECONDARY_CLIENT_TOSWITCH_MAC = "00:00:0a:00:01:02"
SECONDARY_CLIENT_TOSWITCH_FPPORT = "2/0"
SECONDARY_CLIENT_TOSWITCH_PIPEIDX = "0"
SECONDARY_CLIENT_LOCAL_IP = "10.0.1.2"

SERVER_TOSWITCH_IFNAME = []
SERVER_TOSWITCH_IP = []
SERVER_TOSWITCH_MAC = []
SERVER_TOSWITCH_FPPORT = []
SERVER_TOSWITCH_PIPEIDX = []
SERVER_LOCAL_IP = []

for i in range(8):
    SERVER_TOSWITCH_IFNAME.append("s{}-eth{}".format(i / 2 + 1, i % 2 + 1))
    SERVER_TOSWITCH_IP.append("10.0.1.{}".format(3 + i))
    SERVER_TOSWITCH_MAC.append("00:00:0a:00:01:0{}".format(hex(i + 3)[2:]))
    SERVER_TOSWITCH_FPPORT.append("{}/0".format(i % 2 + 1))
    SERVER_TOSWITCH_PIPEIDX.append("1")
    # SERVER_LOCAL_IP.append()

CONTROLLER_LOCAL_IP = []
SWITCHOS_LOCAL_IP = []

for i in range(4):
    CONTROLLER_LOCAL_IP.append(SERVER_TOSWITCH_IP[i * 2])
    # SWITCHOS_LOCAL_IP.append()
# CPU settings

SERVER_WORKER_CORENUM = 1
SERVER_TOTAL_CORENUM = 2

is_global_included = 1
