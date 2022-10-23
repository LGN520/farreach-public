import os
import re
import sys
import json
#import demjson

STRID = "strid"
TOTAL_OPSDONE = "totalOpsdone"
PERSERVER_OPSDONE = "perserverOpsdone"
PERSERVER_CACHEHITS = "perserverCachehits"
EXECUTION_MILLIS = "executionMillis"
#TOTAL_LATENCY = "totalLatency"
#TOTAL_LATENCYNUM = "totalLatencynum"
#TOTAL_HISTOGRAM = "totalHistogram"
PERSERVER_TOTAL_LATENCY = "perservertotalLatency"
PERSERVER_TOTAL_LATENCYNUM = "perservertotalLatencynum"
PERSERVER_TOTAL_HISTOGRAM = "perservertotalHistogram"

STATIC_PEROBJ_EXECUTION_MILLIS = 10 * 1000
DYNAMIC_PEROBJ_EXECUTION_MILLIS = 1 * 1000
GLOBAL_PEROBJ_EXECUTION_MILLIS = -1

def getmops(opsperms):
    return opsperms * 1000 / 1000.0 / 1000.0

def getstrid(tmpjsonobj):
    if STRID in tmpjsonobj.keys():
        tmpstrid = tmpjsonobj[STRID]
    else:
        tmpstrid = "NOSTRID"
    return tmpstrid

def aggregate_1dlist(locallist, remotelist, objidx, role):
    result = []
    if (len(locallist) != len(remotelist)):
        print "[ERROR][aggregate] for {}th obj, localjsonobj[{}] size {} != remotejsonobj[{}] size {}".format(objidx, role, len(locallist), role, len(remotelist))
        print "localjsonobj[{}: {}".format(role, locallist)
        print "remotejsonobj[{}]: {}".format(role, remotelist)
        exit(-1)
    for i in range(len(locallist)):
        result.append(locallist[i] + remotelist[i])
    return result

def aggregate(localjsonarray, remotejsonarray, length):
    global GLOBAL_PEROBJ_EXECUTION_MILLIS

    aggjsonarray = []
    for i in range(length):
        localjsonobj = localjsonarray[i]
        remotejsonobj = remotejsonarray[i]

        aggobj = {}

        localstrid = getstrid(localjsonobj)
        remotestrid = getstrid(remotejsonobj)
        if localstrid == remotestrid:
            aggobj[STRID] = localstrid
        else:
            print "[ERROR][STRID] for STRID of the {}th JsonObject, {} != {}".format(i, localstrid, remotestrid)
            exit(-1)

        aggobj[TOTAL_OPSDONE] = localjsonobj[TOTAL_OPSDONE] + remotejsonobj[TOTAL_OPSDONE]
        aggobj[PERSERVER_OPSDONE] = aggregate_1dlist(localjsonobj[PERSERVER_OPSDONE], remotejsonobj[PERSERVER_OPSDONE], i, PERSERVER_OPSDONE)
        aggobj[PERSERVER_CACHEHITS] = aggregate_1dlist(localjsonobj[PERSERVER_CACHEHITS], remotejsonobj[PERSERVER_CACHEHITS], i, PERSERVER_CACHEHITS)

        aggobj[EXECUTION_MILLIS] = (localjsonobj[EXECUTION_MILLIS] + remotejsonobj[EXECUTION_MILLIS]) / 2
        #aggobj[EXECUTION_MILLIS] = GLOBAL_PEROBJ_EXECUTION_MILLIS

        aggobj[PERSERVER_TOTAL_LATENCY] = aggregate_1dlist(localjsonobj[PERSERVER_TOTAL_LATENCY], remotejsonobj[PERSERVER_TOTAL_LATENCY], i, PERSERVER_TOTAL_LATENCY)
        aggobj[PERSERVER_TOTAL_LATENCYNUM] = aggregate_1dlist(localjsonobj[PERSERVER_TOTAL_LATENCYNUM], remotejsonobj[PERSERVER_TOTAL_LATENCYNUM], i, PERSERVER_TOTAL_LATENCYNUM)

        aggobj[PERSERVER_TOTAL_HISTOGRAM] = []
        if len(localjsonobj[PERSERVER_TOTAL_HISTOGRAM]) != len(remotejsonobj[PERSERVER_TOTAL_HISTOGRAM]):
            print "[ERROR][aggregate] for {}th obj, localjsonobj[PERSERVER_TOTAL_HISTOGRAM] size {} != remotejsonobj[PERSERVER_TOTAL_HISTOGRAM] size {}".format(i, len(localjsonobj[PERSERVER_TOTAL_HISTOGRAM]), len(remotejsonobj[PERSERVER_TOTAL_HISTOGRAM]))
            exit(-1)
        for j in range(len(localjsonobj[PERSERVER_TOTAL_HISTOGRAM])):
            tmplocalhist = localjsonobj[PERSERVER_TOTAL_HISTOGRAM][j]
            tmpremotehist = remotejsonobj[PERSERVER_TOTAL_HISTOGRAM][j]
            if len(tmplocalhist) != len(tmpremotehist):
                print "[ERROR][aggregate] for {}th obj {}th histogram, local size {} != remote size {}".format(i, j, len(tmplocalhist), len(tmpremotehist))
                print "localhist: {}".format(localhist)
                print "remotehist: {}".format(remotehist)
                exit(-1)
            tmpagghist = []
            for k in range(len(tmplocalhist)):
                tmpagghist.append(tmplocalhist[k] + tmpremotehist[k])
            aggobj[PERSERVER_TOTAL_HISTOGRAM].append(tmpagghist)

        aggjsonarray.append(aggobj)
    return aggjsonarray

def calculatelatency(totallatency, totallatencynum, totalhistogram):
    avglatency = float(totallatency) / float(totallatencynum)
    donemedium = False
    latencymedium = 0
    done90p = False
    latency90p = 0
    done95p = False
    latency95p = 0
    #done99p = False
    latency99p = 0
    curlatencynum = 0
    for i in range(len(totalhistogram)):
        curlatencynum += totalhistogram[i]
        if donemedium == False and float(curlatencynum) / float(totallatencynum) >= 0.50:
            donemedium = True
            latencymedium = i
        if done90p == False and float(curlatencynum) / float(totallatencynum) >= 0.90:
            done90p = True
            latency90p = i
        if done95p == False and float(curlatencynum) / float(totallatencynum) >= 0.95:
            done95p = True
            latency95p = i
        if float(curlatencynum) / float(totallatencynum) >= 0.99:
            latency99p = i
            break
    return avglatency, latencymedium, latency90p, latency95p, latency99p

def calculate_perobjstat(aggjsonarray):
    for i in range(len(aggjsonarray)):
        tmpjsonobj = aggjsonarray[i]

        tmpstrid = getstrid(tmpjsonobj)
        tmptotalops = tmpjsonobj[TOTAL_OPSDONE]
        tmp_perserverops = tmpjsonobj[PERSERVER_OPSDONE]
        tmp_perservercachehits = tmpjsonobj[PERSERVER_CACHEHITS]
        tmptotaltime = tmpjsonobj[EXECUTION_MILLIS]

        tmp_cachemisscnt = 0
        tmp_cachehitcnt = 0
        tmp_maxserverops = 0
        for j in range(len(tmp_perserverops)):
            tmp_cachemisscnt += tmp_perserverops[j]
            if tmp_perserverops[j] > tmp_maxserverops:
                tmp_maxserverops = tmp_perserverops[j]
            tmp_cachehitcnt += tmp_perservercachehits[j]
        
        if tmp_maxserverops == 0:
            print "[{}] tmp_maxserverops is 0, tmp_perserverops: {}".format(tmpstrid, tmp_perserverops)
        else:
            tmp_normalizedthpt = float(tmptotalops) / float(tmp_maxserverops)
            print "[{}] thpt {} MOPS; cache hit rate {}; cache miss rate {}; normalized thpt {}".format(tmpstrid, getmops(float(tmptotalops) / float(tmptotaltime)), float(tmp_cachehitcnt) / float(tmptotalops), float(tmp_cachemisscnt) / float(tmptotalops), tmp_normalizedthpt)

        for j in range(len(tmpjsonobj[PERSERVER_TOTAL_LATENCY])):
            tmp_totallatency = tmpjsonobj[PERSERVER_TOTAL_LATENCY][j]
            tmp_totallatencynum = tmpjsonobj[PERSERVER_TOTAL_LATENCYNUM][j]
            tmp_totallatencyhist = tmpjsonobj[PERSERVER_TOTAL_HISTOGRAM][j]
            tmp_avglatency, tmp_latencymedium, tmp_latency90p, tmp_latency95p, tmp_latency99p = calculatelatency(tmp_totallatency, tmp_totallatencynum, tmp_totallatencyhist)
            print "[idx {}] average latency {} us, medium latency {} us, 90P latency {} us, 95P latency {} us, 99P latency {} us".format(j, tmp_avglatency, tmp_latencymedium, tmp_latency90p, tmp_latency95p, tmp_latency99p)
    return

def staticprocess(localjsonarray, remotejsonarray, bottleneckidx):
    if (len(localjsonarray) != len(remotejsonarray)):
        print "[ERROR][STATIC] client 0 jsonarray size {} != client 1 jsonarray size {}".format(len(localjsonarray), len(remotejsonarray))
        exit(-1)

    aggjsonarray = aggregate(localjsonarray, remotejsonarray, len(localjsonarray))

    calculate_perobjstat(aggjsonarray)

    # (1) Remove runtime variance on thpt

    avgbottleneckthpt = getmops(aggjsonarray[0][TOTAL_OPSDONE] / aggjsonarray[0][EXECUTION_MILLIS])
    for i in range(1, len(aggjsonarray)):
        tmpjsonobj = aggjsonarray[i]
        tmpexectime = tmpjsonobj[EXECUTION_MILLIS]
        tmp_bottleneckthpt = getmops((tmpjsonobj[PERSERVER_CACHEHITS][bottleneckidx] + tmpjsonobj[PERSERVER_OPSDONE][bottleneckidx]) / tmpexectime)

        avgbottleneckthpt += tmp_bottleneckthpt
    avgbottleneckthpt /= float(len(aggjsonarray))

    totalthpt = avgbottleneckthpt
    tmpbottleneckthpt = getmops(aggjsonarray[0][TOTAL_OPSDONE] / aggjsonarray[0][EXECUTION_MILLIS])
    print "[0th statistics] for {}: bottleneck thpt {}, rotated thpt 0, total thpt {}".format(bottleneckidx, tmpbottleneckthpt, tmpbottleneckthpt)
    for i in range(1, len(aggjsonarray)):
        tmpjsonobj = aggjsonarray[i]
        tmpexectime = tmpjsonobj[EXECUTION_MILLIS]
        tmp_bottleneckthpt = getmops((tmpjsonobj[PERSERVER_CACHEHITS][bottleneckidx] + tmpjsonobj[PERSERVER_OPSDONE][bottleneckidx]) / tmpexectime)

        if i <= bottleneckidx:
            tmp_rotateidx = i - 1
        else:
            tmp_rotateidx = i
        tmp_rotatethpt = getmops((tmpjsonobj[PERSERVER_CACHEHITS][tmp_rotateidx] + tmpjsonobj[PERSERVER_OPSDONE][tmp_rotateidx]) / tmpexectime)
        if tmp_rotatethpt <= 0:
            print "[ERROR] {}th statistics for {}-{}: thpt of rotated server is 0!".format(i, bottleneckidx, tmp_rotateidx)
            exit(-1)
        else:
            print "[{}th statistics] for {}-{}: bottleneck thpt {}, rotated thpt {}, total thpt {}".format(i, bottleneckidx, tmp_rotateidx, tmp_bottleneckthpt, tmp_rotatethpt, getmops(tmpjsonobj[TOTAL_OPSDONE] / tmpexectime))
        totalthpt += tmp_rotatethpt * (avgbottleneckthpt / tmp_bottleneckthpt)

    print "[STATIC] average bottleneck throughput: {} MOPS".format(avgbottleneckthpt)
    print "[STATIC] aggregate throughput: {} MOPS".format(totalthpt)

    # (2) Reduce runtime variance on latency

    totallatency = 0
    totallatencynum = 0
    totallatencyhist = []

    # Get bottleneck latency statistics

    avgbottleneckhist = [] # latency histogram for bottleneck partition
    for i in range(len(aggjsonarray)):
        tmpjsonobj = aggjsonarray[i]
        tmpbottleneckhist = tmpjsonobj[PERSERVER_TOTAL_HISTOGRAM][0]
        if len(avgbottleneckhist) == 0:
            avgbottleneckhist = [0] * len(tmpbottleneckhist)
        for j in range(len(avgbottleneckhist)):
            avgbottleneckhist[j] += tmpbottleneckhist[j]
    for i in range(len(avgbottleneckhist)):
        avgbottleneckhist[i] /= len(aggjsonarray)

    avgbottlenecktotallatency = 0
    avgbottlenecktotallatencynum = 0
    for i in range(len(avgbottleneckhist)):
        avgbottlenecktotallatency += i * avgbottleneckhist[i]
        avgbottlenecktotallatencynum += avgbottleneckhist[i]
    #avglatency, latencymedium, latency90p, latency95p, latency99p = calculatelatency(avgbottlenecktotallatency, avgbottlenecktotallatencynum, avgbottleneckhist)
    #print "[average bottleneck latency] average latency {} us, medium latency {} us, 90P latency {} us, 95P latency {} us, 99P latency {} us".format(avglatency, latencymedium, latency90p, latency95p, latency99p)

    totallatency = avgbottlenecktotallatency
    totallatencynum = avgbottlenecktotallatencynum
    totallatencyhist = [0] * len(avgbottleneckhist)
    for i in range(len(avgbottleneckhist)):
        totallatencyhist[i] = avgbottleneckhist[i]

    # Get each rotated partition latency statistics

    for i in range(1, len(aggjsonarray)):
        tmpjsonobj = aggjsonarray[i]
        tmprotatehist = tmpjsonobj[PERSERVER_TOTAL_HISTOGRAM][1]
        tmprotatetotallatency = tmpjsonobj[PERSERVER_TOTAL_LATENCY][1]
        tmprotatetotallatencynum = tmpjsonobj[PERSERVER_TOTAL_LATENCYNUM][1]

        totallatency += tmprotatetotallatency
        totallatencynum += tmprotatetotallatencynum
        for j in range(len(totallatencyhist)):
            totallatencyhist[j] += tmprotatehist[j]

    # Get aggregated latency results

    avglatency, latencymedium, latency90p, latency95p, latency99p = calculatelatency(totallatency, totallatencynum, totallatencyhist)
    print "[STATIC] average latency {} us, medium latency {} us, 90P latency {} us, 95P latency {} us, 99P latency {} us".format(avglatency, latencymedium, latency90p, latency95p, latency99p)

def dynamicprocess(localjsonarray, remotejsonarray):
    seconds = len(localjsonarray)
    if seconds > len(remotejsonarray):
        seconds = len(remotejsonarray)
    print "[DYNAMIC] common seconds: {}".format(seconds)

    aggjsonarray = aggregate(localjsonarray, remotejsonarray, seconds)

    calculate_perobjstat(aggjsonarray)

    persecthpt = [0] * seconds
    perseclatencyavg = [0] * seconds
    perseclatencymedium = [0] * seconds
    perseclatency90p = [0] * seconds
    perseclatency95p = [0] * seconds
    perseclatency99p = [0] * seconds
    for i in range(seconds):
        tmpjsonobj = aggjsonarray[i]
        persecthpt[i] = getmops(tmpjsonobj[TOTAL_OPSDONE] / tmpjsonobj[EXECUTION_MILLIS])

        tmp_totallatency = 0
        tmp_totallatencynum = 0
        tmp_totallatencyhist = [0] * len(tmpjsonobj[PERSERVER_TOTAL_HISTOGRAM][0])
        for j in range(len(tmpjsonobj[PERSERVER_TOTAL_LATENCY])):
            tmp_totallatency += tmpjsonobj[PERSERVER_TOTAL_LATENCY][j]
            tmp_totallatencynum += tmpjsonobj[PERSERVER_TOTAL_LATENCYNUM][j]
            for k in range(len(tmp_totallatencyhist)):
                tmp_totallatencyhist[k] += tmpjsonobj[PERSERVER_TOTAL_HISTOGRAM][j][k]
        perseclatencyavg[i], perseclatencymedium[i], perseclatency90p[i], perseclatency95p[i], perseclatency99p[i] = calculatelatency(\
                tmp_totallatency, tmp_totallatencynum, tmp_totallatencyhist)
    print "[DYNAMIC] per-second statistics:"
    print "thpt (MOPS): {}".format(persecthpt)
    print "avg latency (us): {}".format(perseclatencyavg)
    print "medium latency (us): {}".format(perseclatencymedium)
    print "90P latency (us): {}".format(perseclatency90p)
    print "95P latency (us): {}".format(perseclatency95p)
    print "99P latency (us): {}".format(perseclatency99p)

    totallatency = 0
    totallatencynum = 0
    totalhistogram = [0] * len(aggjsonarray[0][TOTAL_HISTOGRAM])
    for i in range(len(aggjsonarray)):
        tmpjsonobj = aggjsonarray[i]
        for j in range(len(tmpjsonobj[PERSERVER_TOTAL_LATENCY])):
            totallatency += tmpjsonobj[PERSERVER_TOTAL_LATENCY][j]
            totallatencynum += tmpjsonobj[PERSERVER_TOTAL_LATENCYNUM][j]
            for k in range(len(tmpjsonobj[PERSERVER_TOTAL_HISTOGRAM][j])):
                totalhistogram[k] += tmpjsonobj[PERSERVER_TOTAL_HISTOGRAM][j][k]

    avglatency, latencymedium, latency90p, latency95p, latency99p = calculatelatency(totallatency, totallatencynum, totalhistogram)
    print "[DYNAMIC][OVERALL] average latency {} us, medium latency {} us, 90P latency {} us, 95P latency {} us, 99P latency {} us".format(avglatency, latencymedium, latency90p, latency95p, latency99p)

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
