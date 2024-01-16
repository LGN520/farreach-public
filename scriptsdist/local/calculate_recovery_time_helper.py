import os
import re
import sys
import json

from helper import *

if len(sys.argv) != 3:
    print("Invalid usage of calculate_recovery_time_helper.py")
    print("Arguments: {}".format(sys.argv))
    exit(-1)

filepath = sys.argv[1]
filelist = [
    "tmp_launchswitchostestbed.out",
    "tmp_server_0.out",
    "tmp_server_1.out",
    "tmp_switchos.out",
    "tmp_test_recovery_time.out",
]
server_scale = float(sys.argv[2])

server_collect_list = []
server_preprocess_list = []
server_replay_list = []
switch_collect_list = []
switch_replay_list = []

for filename in filelist:
    outputfile = filepath + "/" + filename
    if not os.path.exists(outputfile):
        print("No such file {}".format(outputfile))
        exit(-1)

    with open(outputfile, "r") as f:
        for line in f:
            if len(re.findall(r"collect time server", line)) != 0:
                if len(re.findall(r"\d*.\d+", line)) == 1:
                    server_collect_list.append(float(re.findall(r"\d*.\d+", line)[0]))
                else:
                    server_collect_list.append(float(re.findall(r"\d.\d+", line)[0]))

            if len(re.findall(r"Preprocessing time of client-side", line)) != 0:
                server_preprocess_list.append(float(re.findall(r"\d.\d+", line)[0]))

            if len(re.findall(r"Replay time of server", line)) != 0:
                server_replay_list.append(float(re.findall(r"\d.\d+", line)[0]))

            if len(re.findall(r"Replay time of switch&switchos", line)) != 0:
                switch_replay_list.append(float(re.findall(r"\d.\d+", line)[0]))

            if len(re.findall(r"collect time switchos", line)) != 0:
                if len(re.findall(r"\d*.\d+", line)) == 1:
                    switch_collect_list.append(float(re.findall(r"\d*.\d+", line)[0]))
                else:
                    switch_collect_list.append(float(re.findall(r"\d.\d+", line)[0]))

print("Server collect time: {} s".format(server_collect_list[0]))
server_preprocess_time = 0
for time in server_preprocess_list:
    server_preprocess_time += time
print("Server preprocess time: {} s".format(server_preprocess_time))
server_replay_time = 0
for time in server_replay_list:
    server_replay_time += time
print("Server replay time: {} s".format(server_replay_time / server_scale))
print(
    "Server total recovery time: {} s".format(
        server_replay_time / server_scale
        + server_preprocess_time
        + server_collect_list[0]
    )
)

print("Switch collect time: {} s".format(switch_collect_list[0]))
print("Switch replay time: {} s".format(switch_replay_list[0]))
print(
    "Switch total recovery time: {} s".format(
        switch_collect_list[0] + switch_replay_list[0]
    )
)
