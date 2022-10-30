import os
import re
import sys
import json
#import demjson

from helper import *

# Process static/dynamic statistics

def staticprocess(localjsonarray, remotejsonarray, bottleneckidx):
    if (len(localjsonarray) != len(remotejsonarray)):
        print "[ERROR][STATIC] client 0 jsonarray size {} != client 1 jsonarray size {}".format(len(localjsonarray), len(remotejsonarray))
        exit(-1)

    workloadmode = 0

    aggjsonarray = aggregate(localjsonarray, remotejsonarray, len(localjsonarray))

    calculate_perobjstat(aggjsonarray, workloadmode, bottleneckidx)

    # (1) Remove runtime variance on thpt

    avgbottleneck_totalthpt, avgbottleneck_switchthpt, avgbottleneck_serverthpt = get_total_switch_server_thpts(aggjsonarray, 0, 0, workloadmode, bottleneckidx)
    for i in range(1, len(aggjsonarray)):
        tmp_bottleneck_totalthpt, tmp_bottleneck_switchthpt, tmp_bottleneck_serverthpt = get_total_switch_server_thpts(aggjsonarray, i, 0, workloadmode, bottleneckidx)

        avgbottleneck_totalthpt += tmp_bottleneck_totalthpt
        avgbottleneck_switchthpt += tmp_bottleneck_switchthpt
        avgbottleneck_serverthpt += tmp_bottleneck_serverthpt
    avgbottleneck_totalthpt /= float(len(aggjsonarray))
    avgbottleneck_switchthpt /= float(len(aggjsonarray))
    avgbottleneck_serverthpt /= float(len(aggjsonarray))
    print "[STATIC] average bottleneck totalthpt: {} MOPS; switchthpt: {} MOPS; serverthpt: {} MOPS".format(avgbottleneck_totalthpt, avgbottleneck_switchthpt, avgbottleneck_serverthpt)

    totalthpt = avgbottleneck_totalthpt
    max_serverthpt = avgbottleneck_serverthpt
    total_serverthpt = avgbottleneck_serverthpt

    for i in range(1, len(aggjsonarray)):
        tmp_bottleneck_totalthpt, tmp_bottleneck_switchthpt, tmp_bottleneck_serverthpt = get_total_switch_server_thpts(aggjsonarray, i, 0, workloadmode, bottleneckidx)
        tmp_rotate_totalthpt, tmp_rotate_switchthpt, tmp_rotate_serverthpt = get_total_switch_server_thpts(aggjsonarray, i, 1, workloadmode, bottleneckidx)

        tmp_rotate_totalthpt *= (avgbottleneck_totalthpt / tmp_bottleneck_totalthpt)
        if tmp_bottleneck_switchthpt != 0:
            tmp_rotate_switchthpt *= (avgbottleneck_switchthpt / tmp_bottleneck_switchthpt)
        tmp_rotate_serverthpt *= (avgbottleneck_serverthpt / tmp_bottleneck_serverthpt)

        totalthpt += tmp_rotate_totalthpt
        if tmp_rotate_serverthpt > max_serverthpt:
            max_serverthpt = tmp_rotate_serverthpt
        total_serverthpt += tmp_rotate_serverthpt
    normalized_thpt = float(totalthpt) / float(max_serverthpt)
    normalized_serverthpt = float(total_serverthpt) / float(max_serverthpt)
    print "[STATIC] aggregate throughput: {} MOPS; normalized throughput: {}, normalized serverthpt: {}".format(totalthpt, normalized_thpt, normalized_serverthpt)

    # (2) Reduce runtime variance on latency

    # Get bottleneck latency statistics

    avgbottleneckhist = [] # latency histogram for bottleneck partition
    for i in range(len(aggjsonarray)):
        tmpjsonobj = aggjsonarray[i]
        tmp_bottleneckhist, _, _ = get_total_cachehit_cachemiss_latencyhist(aggjsonarray, i, 0)
        if len(avgbottleneckhist) == 0:
            avgbottleneckhist = [0.0] * len(tmp_bottleneckhist)
        for j in range(len(avgbottleneckhist)):
            avgbottleneckhist[j] += float(tmp_bottleneckhist[j])
    for i in range(len(avgbottleneckhist)):
        avgbottleneckhist[i] = float(avgbottleneckhist[i]) / float(len(aggjsonarray))

    totallatencyhist = [0] * len(avgbottleneckhist)
    for i in range(len(avgbottleneckhist)):
        totallatencyhist[i] = avgbottleneckhist[i]

    # Get each rotated partition latency statistics

    for i in range(1, len(aggjsonarray)):
        tmp_rotatehist, _, _ = get_total_cachehit_cachemiss_latencyhist(aggjsonarray, i, 1)

        for j in range(len(totallatencyhist)):
            totallatencyhist[j] += float(tmp_rotatehist[j])

    # Get aggregated latency results

    avglatency, latencymedium, latency90p, latency95p, latency99p, _, _ = calculatelatency(totallatencyhist)
    print "[STATIC] average latency {} us, medium latency {} us, 90P latency {} us, 95P latency {} us, 99P latency {} us".format(avglatency, latencymedium, latency90p, latency95p, latency99p)

def dynamicprocess(localjsonarray, remotejsonarray):
    seconds = len(localjsonarray)
    if seconds > len(remotejsonarray):
        seconds = len(remotejsonarray)
    print "[DYNAMIC] common seconds: {}".format(seconds)

    workloadmode = 1

    aggjsonarray = aggregate(localjsonarray, remotejsonarray, seconds)

    calculate_perobjstat(aggjsonarray, workloadmode, -1)

    persecthpt = [0] * seconds
    persec_normalizedthpt = [0] * seconds
    persec_normalizedserverthpt = [0] * seconds
    perseclatencyavg = [0] * seconds
    perseclatencymedium = [0] * seconds
    perseclatency90p = [0] * seconds
    perseclatency95p = [0] * seconds
    perseclatency99p = [0] * seconds
    for i in range(seconds):
        tmpjsonobj = aggjsonarray[i]
        persecthpt[i] = getmops(tmpjsonobj[TOTAL_OPSDONE] / tmpjsonobj[EXECUTION_MILLIS])

        tmp_maxserverthpt = 0
        tmp_totalserverthpt = 0
        for j in range(len(tmpjsonobj[PERSERVER_OPSDONE])):
            tmp_serverthpt = getmops(tmpjsonobj[PERSERVER_OPSDONE][j] / tmpjsonobj[EXECUTION_MILLIS])
            if tmp_serverthpt > tmp_maxserverthpt:
                tmp_maxserverthpt = tmp_serverthpt
            tmp_totalserverthpt += tmp_serverthpt
        if tmp_maxserverthpt != 0:
            persec_normalizedthpt[i] = persecthpt[i] / float(tmp_maxserverthpt)
            persec_normalizedserverthpt[i] = tmp_totalserverthpt / float(tmp_maxserverthpt)
        else:
            persec_normalizedthpt[i] = 0
            persec_normalizedserverthpt[i] = 0

        tmp_server0_totallatencyhist, _, _ = get_total_cachehit_cachemiss_latencyhist(aggjsonarray, i, 0)
        tmp_server1_totallatencyhist, _, _ = get_total_cachehit_cachemiss_latencyhist(aggjsonarray, i, 1)
        tmp_totallatencyhist = [0] * len(tmp_server0_totallatencyhist)
        for j in range(len(tmp_server0_totallatencyhist)):
            tmp_totallatencyhist[j] = tmp_server0_totallatencyhist[j] + tmp_server1_totallatencyhist[j]
        perseclatencyavg[i], perseclatencymedium[i], perseclatency90p[i], perseclatency95p[i], perseclatency99p[i], _, _ = calculatelatency(tmp_totallatencyhist)
    print "[DYNAMIC] per-second statistics:"
    print "thpt (MOPS): {}".format(persecthpt)
    print "normalized thpt: {}".format(persec_normalizedthpt)
    print "normalized serverthpt: {}".format(persec_normalizedserverthpt)
    print "avg latency (us): {}".format(perseclatencyavg)
    print "medium latency (us): {}".format(perseclatencymedium)
    print "90P latency (us): {}".format(perseclatency90p)
    print "95P latency (us): {}".format(perseclatency95p)
    print "99P latency (us): {}".format(perseclatency99p)

    avgthpt = 0.0
    for i in range(len(persecthpt)):
        avgthpt += persecthpt[i]
    avgthpt /= float(len(persecthpt))

    totalhistogram = [0] * len(aggjsonarray[0][PERSERVER_CACHEHIT_TOTAL_HISTOGRAM][0])
    for i in range(len(aggjsonarray)):
        tmp_server0_totallatencyhist, _, _ = get_total_cachehit_cachemiss_latencyhist(aggjsonarray, i, 0)
        tmp_server1_totallatencyhist, _, _ = get_total_cachehit_cachemiss_latencyhist(aggjsonarray, i, 1)
        for j in range(len(tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCY])):
            totalhistogram[j] += (tmp_server0_totallatencyhist[j] + tmp_server1_totallatencyhist[j])

    avglatency, latencymedium, latency90p, latency95p, latency99p, _, _ = calculatelatency(totalhistogram)
    print "[DYNAMIC][OVERALL] avgthpt {} MOPS, avelat {} us, medlat {} us, 90Plat {} us, 95Plat {} us, 99Plat {} us".format(avgthpt, avglatency, latencymedium, latency90p, latency95p, latency99p)

if len(sys.argv) != 5:
    print "Invalid usage of calculate_statistics_helper.py"
    print "Arguments: {}".format(sys.argv)
    exit(-1)

workloadmode = int(sys.argv[1])

if workloadmode != 1 and workloadmode != 0:
    print "Invalid workloadmode {}".format(workloadmode)
    exit(-1)

localfilepath = sys.argv[2]
remotefilepath = sys.argv[3]
print "Aggregate statistics between {} and {}".format(localfilepath, remotefilepath)

bottleneckidx = int(sys.argv[4])

if not os.path.exists(localfilepath):
    print "No such file {}".format(localfilepath)
    exit(-1)

if not os.path.exists(remotefilepath):
    print "No such file {}".format(remotefilepath)
    exit(-1)

print "Load {}...".format(localfilepath)
localjsonstr = open(localfilepath).read()
print "Load {}...".format(remotefilepath)
remotejsonstr = open(remotefilepath).read()

# DEPRECATED: demjson is too slow!!!
#print "Decode {}...".format(localfilepath)
#localjsonarray = demjson.decode(localjsonstr)
#print "{}".format(len(localjsonarray))
#print "Decode {}...".format(remotefilepath)
#remotejsonarray = demjson.decode(remotejsonstr)
#print "{}".format(len(remotejsonarray))

print "Decode {}...".format(localfilepath)
#localjsonstr = re.sub(r'\\', '', localjsonstr) # uncomment if with \" in json file
localjsonarray = json.loads(localjsonstr)

print "Decode {}...".format(remotefilepath)
#remotejsonstr = re.sub(r'\\', '', remotejsonstr) # uncomment if with \" in json file
remotejsonarray = json.loads(remotejsonstr)

if workloadmode == 0:
    GLOBAL_PEROBJ_EXECUTION_MILLIS = STATIC_PEROBJ_EXECUTION_MILLIS
    staticprocess(localjsonarray, remotejsonarray, bottleneckidx)
else:
    GLOBAL_PEROBJ_EXECUTION_MILLIS = DYNAMIC_PEROBJ_EXECUTION_MILLIS
    dynamicprocess(localjsonarray, remotejsonarray)
