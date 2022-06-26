import os
import sys

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

    line1 = f.readline()
    line1_elements = line1.split(" ")
    bottleneck_pktcnt = int(line1_elements[0]) # cache miss cnt of bottleneck server thread
    bottleneck_totalpktcnt = int(line1_elements[1]) # total pkt cnt of bottleneck server thread
    bottleneck_thpt = float(line1_elements[2])
    bottleneck_latency = float(line1_elements[3])

    rotate_thpt_list = []
    rotate_latency_list = []
    for i in range(linenum-1):
        tmpline = f.readline()
        tmpline_elements = tmpline.split(" ")
        tmpbottleneck_pktcnt = int(tmpline_elements[0])
        tmprotate_pktcnt = int(tmpline_elements[1]) # cache miss cnt of rotated server thread
        tmprotate_totalpktcnt = int(tmpline_elements[2]) # total pkt cnt of bottleneck server thread + rotated server thread
        tmprotate_thpt = float(tmpline_elements[3])
        tmprorate_latency = float(tmpline_elements[4])
        rotate_thpt_list.append(tmprotate_thpt)
        rotate_latency_list.append(tmprorate_latency)

    system_totalthpt = bottleneck_thpt
    system_avglatency = bottleneck_latency
    for i in range(len(rotate_thpt_list)):
        system_totalthpt += (rotate_thpt_list[i] - bottleneck_thpt)
        system_avglatency += (rotate_latency_list[i])
    system_avglatency /= linenum
    print("system thpt: {}; avg latency: {}".format(system_totalthpt, system_avglatency))
