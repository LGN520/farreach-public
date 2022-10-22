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
TOTAL_LATENCY = "totalLatency"
TOTAL_LATENCYNUM = "totalLatencynum"
TOTAL_HISTOGRAM = "totalHistogram"

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
        aggobj[PERSERVER_OPSDONE] = []
        if (len(localjsonobj[PERSERVER_OPSDONE]) != len(remotejsonobj[PERSERVER_OPSDONE])):
            print "[ERROR][aggregate] for {}th obj, localjsonobj[PERSERVER_OPSDONE] size {} != remotejsonobj[PERSERVER_OPSDONE] size {}".format(i, len(localjsonobj[PERSERVER_OPSDONE]), len(remotejsonobj[PERSERVER_OPSDONE]))
            print "localjsonobj[PERSERVER_OPSDONE]: {}".format(localjsonobj[PERSERVER_OPSDONE])
            print "remotejsonobj[PERSERVER_OPSDONE]: {}".format(remotejsonobj[PERSERVER_OPSDONE])
            exit(-1)
        for j in range(len(localjsonobj[PERSERVER_OPSDONE])):
            aggobj[PERSERVER_OPSDONE].append(localjsonobj[PERSERVER_OPSDONE][j] + remotejsonobj[PERSERVER_OPSDONE][j])
        aggobj[PERSERVER_CACHEHITS] = []
        if (len(localjsonobj[PERSERVER_CACHEHITS]) != len(remotejsonobj[PERSERVER_CACHEHITS])):
            print "[ERROR][aggregate] for {}th obj, localjsonobj[PERSERVER_CACHEHITS] size {} != remotejsonobj[PERSERVER_CACHEHITS] size {}".format(i, len(localjsonobj[PERSERVER_CACHEHITS]), len(remotejsonobj[PERSERVER_CACHEHITS]))
            print "localjsonobj[PERSERVER_CACHEHITS]: {}".format(localjsonobj[PERSERVER_CACHEHITS])
            print "remotejsonobj[PERSERVER_CACHEHITS]: {}".format(remotejsonobj[PERSERVER_CACHEHITS])
            exit(-1)
        for j in range(len(localjsonobj[PERSERVER_CACHEHITS])):
            aggobj[PERSERVER_CACHEHITS].append(localjsonobj[PERSERVER_CACHEHITS][j] + remotejsonobj[PERSERVER_CACHEHITS][j])
        aggobj[EXECUTION_MILLIS] = (localjsonobj[EXECUTION_MILLIS] + remotejsonobj[EXECUTION_MILLIS]) / 2
        #aggobj[EXECUTION_MILLIS] = GLOBAL_PEROBJ_EXECUTION_MILLIS
        aggobj[TOTAL_LATENCY] = localjsonobj[TOTAL_LATENCY] + remotejsonobj[TOTAL_LATENCY]
        aggobj[TOTAL_LATENCYNUM] = localjsonobj[TOTAL_LATENCYNUM] + remotejsonobj[TOTAL_LATENCYNUM]
        aggobj[TOTAL_HISTOGRAM] = []
        if (len(localjsonobj[TOTAL_HISTOGRAM]) != len(remotejsonobj[TOTAL_HISTOGRAM])):
            print "[ERROR][aggregate] for {}th obj, localjsonobj[TOTAL_HISTOGRAM] size {} != remotejsonobj[TOTAL_HISTOGRAM] size {}".format(i, len(localjsonobj[TOTAL_HISTOGRAM]), len(remotejsonobj[TOTAL_HISTOGRAM]))
            print "localjsonobj[TOTAL_HISTOGRAM]: {}".format(localjsonobj[TOTAL_HISTOGRAM])
            print "remotejsonobj[TOTAL_HISTOGRAM]: {}".format(remotejsonobj[TOTAL_HISTOGRAM])
            exit(-1)
        for j in range(len(localjsonobj[TOTAL_HISTOGRAM])):
            aggobj[TOTAL_HISTOGRAM].append(localjsonobj[TOTAL_HISTOGRAM][j] + remotejsonobj[TOTAL_HISTOGRAM][j])
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
            print "[{}] thpt {}; cache hit rate {}; cache miss rate {}; normalized thpt {}".format(tmpstrid, getmops(float(tmptotalops) / float(tmptotaltime)), float(tmp_cachehitcnt) / float(tmptotalops), float(tmp_cachemisscnt) / float(tmptotalops), tmp_normalizedthpt)

def staticprocess(localjsonarray, remotejsonarray, bottleneckidx):
    if (len(localjsonarray) != len(remotejsonarray)):
        print "[ERROR][STATIC] client 0 jsonarray size {} != client 1 jsonarray size {}".format(len(localjsonarray), len(remotejsonarray))
        exit(-1)

    aggjsonarray = aggregate(localjsonarray, remotejsonarray, len(localjsonarray))

    calculate_perobjstat(aggjsonarray)

    # Remove runtime variance

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

    totallatency = 0
    totallatencynum = 0
    totalhistogram = [0] * len(aggjsonarray[0][TOTAL_HISTOGRAM])
    for i in range(len(aggjsonarray)):
        totallatency += aggjsonarray[i][TOTAL_LATENCY]
        totallatencynum += aggjsonarray[i][TOTAL_LATENCYNUM]
        for j in range(len(aggjsonarray[i][TOTAL_HISTOGRAM])):
            totalhistogram[j] += aggjsonarray[i][TOTAL_HISTOGRAM][j]

    avglatency, latencymedium, latency90p, latency95p, latency99p = calculatelatency(totallatency, totallatencynum, totalhistogram)
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
        persecthpt[i] = getmops(aggjsonarray[i][TOTAL_OPSDONE] / aggjsonarray[i][EXECUTION_MILLIS])
        perseclatencyavg[i], perseclatencymedium[i], perseclatency90p[i], perseclatency95p[i], perseclatency99p[i] = calculatelatency(\
                aggjsonarray[i][TOTAL_LATENCY], aggjsonarray[i][TOTAL_LATENCYNUM], aggjsonarray[i][TOTAL_HISTOGRAM])
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
        totallatency += aggjsonarray[i][TOTAL_LATENCY]
        totallatencynum += aggjsonarray[i][TOTAL_LATENCYNUM]
        for j in range(len(aggjsonarray[i][TOTAL_HISTOGRAM])):
            totalhistogram[j] += aggjsonarray[i][TOTAL_HISTOGRAM][j]

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
