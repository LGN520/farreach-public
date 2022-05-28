import os
import sys

if len(sys.argv) != 2:
    print("Usage: python calculate_thpt.py result.out")
    exit(-1)

resultpath = sys.argv[1]
print("Result file: {}".format(resultpath))

SWITCH_MAX_THPT = 1 * 1000 * 1000 * 1000 # 1 GOPS
PERSERVER_MAX_THPT = 1 * 1000 * 1000 # 1 MOPS

with open(resultpath, "r") as f:
    line1 = f.readline()
    line1_elements = line1.split(" ")
    total_pktcnt = int(line1_elements[0])
    total_time = float(line1_elements[1])
    print("total pktcnt: {}, total time: {}".format(total_pktcnt, total_time))

    line2 = f.readline()
    line2_elements = line2.split(" ")
    servernum = len(line2_elements)
    print("servernum: {}".format(servernum))
    server_pktcnt_list = []
    for i in range(servernum):
        server_pktcnt_list.append(int(line2_elements[i]))
    for i in range(servernum):
        print("server {} pktcnt: {}".format(i, server_pktcnt_list[i]))

    total_thpt = float(total_pktcnt) / total_time
    print("total thpt: {}".format(total_thpt))
    server_thpt_list = []
    for i in range(servernum):
        server_thpt_list.append(float(server_pktcnt_list[i]) / total_time)
    for i in range(servernum):
        print("server {} thpt: {}".format(i, server_thpt_list[i]))
    switch_thpt = total_thpt
    for i in range(servernum):
        switch_thpt -= server_thpt_list[i]
    print("switch thpt: {}".format(switch_thpt))

    factor_list = []
    if switch_thpt != 0:
        factor_list.append(float(SWITCH_MAX_THPT) / switch_thpt)
    for i in range(servernum):
        if server_thpt_list[i] != 0:
            factor_list.append(float(PERSERVER_MAX_THPT) / server_thpt_list[i])

    final_factor = -1
    for i in range(len(factor_list)):
        if final_factor == -1 or final_factor > factor_list[i]:
            final_factor = factor_list[i]
    final_thpt = total_thpt * final_factor
    print("final factor: {}, final thpt: {} MOPS".format(final_factor, final_thpt/1000.0/1000.0))
