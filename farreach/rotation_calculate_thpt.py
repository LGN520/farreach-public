import os
import sys

bottleneck_idx = 123

if len(sys.argv) != 2:
    print("Usage: python rotation_calculate_thpt.py result.out")
    exit(-1)

resultpath = sys.argv[1]
print("Result file: {}".format(resultpath))

with open(resultpath, "r") as f:
    line0 = f.readline()
    linenum = int(line0.split(" ")[0])
    print("linenum: {}".format(linenum))
    print("")

    perserver_thpt_list = [0]*linenum

    line1 = f.readline()
    line1_elements = line1.split(" ")
    bottleneck_pktcnt = int(line1_elements[0]) # cache miss cnt of bottleneck server thread
    totalpktcnt = int(line1_elements[1]) # total pkt cnt of bottleneck server thread
    bottleneck_thpt = float(line1_elements[2])
    bottleneck_latency = float(line1_elements[3])
    bottleneck_time = totalpktcnt / bottleneck_thpt
    perserver_thpt_list[bottleneck_idx] = bottleneck_pktcnt / bottleneck_time

    withrotate_thpt_list = []
    withrotate_latency_list = []
    for i in range(linenum):
        if i == bottleneck_idx:
            continue
        tmpline = f.readline()
        tmpline_elements = tmpline.split(" ")
        tmp_bottleneck_pktcnt = int(tmpline_elements[0])
        tmp_rotate_pktcnt = int(tmpline_elements[1]) # cache miss cnt of rotated server thread
        tmp_totalpktcnt = int(tmpline_elements[2]) # total pkt cnt of bottleneck server thread + rotated server thread
        tmp_thpt = float(tmpline_elements[3])
        tmp_latency = float(tmpline_elements[4])
        withrotate_thpt_list.append(tmp_thpt)
        withrotate_latency_list.append(tmp_latency)

        tmp_time = tmp_totalpktcnt / tmp_thpt
        perserver_thpt_list[i] = tmp_rotate_pktcnt / tmp_time

    system_totalthpt = bottleneck_thpt
    system_avglatency = bottleneck_latency
    for i in range(len(withrotate_thpt_list)):
        if withrotate_thpt_list[i] - bottleneck_thpt > 0: # avoid abnormal trials due to hardware issue
            system_totalthpt += (withrotate_thpt_list[i] - bottleneck_thpt)
        system_avglatency += (withrotate_latency_list[i])
    system_avglatency /= linenum
    print("system thpt: {}; avg latency: {}".format(system_totalthpt, system_avglatency))

    print("per-server throughput: {}".format(perserver_thpt_list))
    min_perserver_thpt = -1
    max_perserver_thpt = -1
    avg_perserver_thpt = 0
    for i in range(len(perserver_thpt_list)):
        if min_perserver_thpt == -1 or min_perserver_thpt > perserver_thpt_list[i]:
            min_perserver_thpt = perserver_thpt_list[i]
        if max_perserver_thpt == -1 or max_perserver_thpt < perserver_thpt_list[i]:
            max_perserver_thpt = perserver_thpt_list[i]
        avg_perserver_thpt += perserver_thpt_list[i]
    avg_perserver_thpt /= len(perserver_thpt_list)
    print("min per-server thpt: {}, max per-server thpt: {}, avg per-server thpt: {}".format(min_perserver_thpt, max_perserver_thpt, avg_perserver_thpt))
