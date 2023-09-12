import os
import sys

if len(sys.argv) != 2:
    print("Usage: python calculate_thpt.py result.out")
    exit(-1)

resultpath = sys.argv[1]
print("Result file: {}".format(resultpath))

with open(resultpath, "r") as f:
    line = f.readline()
    methodnum = int(line.split(" ")[0])
    print("methodnum: {}".format(methodnum))
    print("")

    for i in range(methodnum):
        line1 = f.readline()
        line1_elements = line1.split(" ")
        methodname = line1_elements[0]
        total_pktcnt = int(line1_elements[1])
        total_time = float(line1_elements[2])
        print("[{}]".format(methodname))
        #print("total pktcnt: {}, total time: {}".format(total_pktcnt, total_time))

        line2 = f.readline()
        line2_elements = line2.split(" ")
        servernum = len(line2_elements)
        #print("servernum: {}".format(servernum))
        server_pktcnt_list = []
        for i in range(servernum):
            server_pktcnt_list.append(int(line2_elements[i]))
        #for i in range(servernum):
        #    print("server {} pktcnt: {}; ".format(i, server_pktcnt_list[i])),
        #print("")

        total_thpt = float(total_pktcnt) / total_time
        #print("total thpt: {}".format(total_thpt))

        server_thpt_list = []
        for i in range(servernum):
            server_thpt_list.append(float(server_pktcnt_list[i]) / total_time)
        #for i in range(servernum):
        #    print("server {} thpt: {}".format(i, server_thpt_list[i]))

        switch_thpt = total_thpt
        for i in range(servernum):
            switch_thpt -= server_thpt_list[i]
        if switch_thpt < 0:
            switch_thpt = 0
        #print("switch thpt: {}".format(switch_thpt))
        print("cache hit rate: {}".format(switch_thpt / total_thpt))

        max_server_thpt = -1
        for i in range(servernum):
            if max_server_thpt == -1 or max_server_thpt < server_thpt_list[i]:
                max_server_thpt = server_thpt_list[i]
        print("normalized thpt: {}".format(total_thpt / max_server_thpt))
