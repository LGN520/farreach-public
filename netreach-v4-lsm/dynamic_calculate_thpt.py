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
        secnum = len(line2_elements)
        #print("secnum: {}".format(secnum))
        persec_pktsentcnt_list = []
        for tmp_element in line2_elements:
            persec_pktsentcnt_list.append(int(tmp_element))
        #print(persec_pktsentcnt_list)

        persec_runtimethpt_list = []
        persec_normalizedthpt_list = []
        for j in range(secnum):
            tmpline = f.readline()
            tmpline_elements = tmpline.split(" ")
            servernum = len(tmpline_elements)

            server_thpt_list = []
            for tmp_server_thpt in tmpline_elements:
                server_thpt_list.append(int(tmp_server_thpt))
            #for i in range(servernum):
            #    print("server {} thpt: {}".format(i, server_thpt_list[i]))

            switch_thpt = persec_pktsentcnt_list[j]
            for tmp_server_thpt in server_thpt_list:
                switch_thpt -= tmp_server_thpt
            #print(switch_thpt, persec_pktsentcnt_list[j])
            if switch_thpt < 0:
                switch_thpt = 0
            cache_hit_rate = float(switch_thpt) / persec_pktsentcnt_list[j]

            max_server_thpt = -1
            for tmp_server_thpt in server_thpt_list:
                if max_server_thpt == -1 or max_server_thpt < tmp_server_thpt:
                    max_server_thpt = tmp_server_thpt
            normalized_thpt = float(persec_pktsentcnt_list[j]) / max_server_thpt

            persec_runtimethpt_list.append(persec_pktsentcnt_list[j] / 1024.0 / 1024.0)
            persec_normalizedthpt_list.append(normalized_thpt)
            print("sec[{}] cache_hit_rate: {}, runtime_thpt: {} MOPS, normalized_thpt: {}".format(\
                    j, cache_hit_rate, persec_pktsentcnt_list[j] / 1024.0 / 1024.0, normalized_thpt))

        print("per-sec runtime thpts: {}".format(persec_runtimethpt_list))
        print("per-sec normalized thpts: {}".format(persec_normalizedthpt_list))
