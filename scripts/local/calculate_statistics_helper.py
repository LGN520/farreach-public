import os
import re
import sys
import json
#import demjson

STRID = "strid"
TOTAL_OPSDONE = "totalOpsdone"
PERSERVER_OPSDONE = "perserverOpsdone"
EXECUTION_MILLIS = "executionMillis"
TOTAL_LATENCY = "totalLatency"
TOTAL_LATENCYNUM = "totalLatencynum"
TOTAL_HISTOGRAM = "totalHistogram"

STATIC_PEROBJ_EXECUTION_MILLIS = 10 * 1000
DYNAMIC_PEROBJ_EXECUTION_MILLIS = 1 * 1000
GLOBAL_PEROBJ_EXECUTION_MILLIS = -1

def aggregate(localjsonarray, remotejsonarray, length):
    global GLOBAL_PEROBJ_EXECUTION_MILLIS

    aggjsonarray = []
    for i in range(length):
        localjsonobj = localjsonarray[i]
        remotejsonobj = remotejsonarray[i]
        aggobj = {}
        aggobj[TOTAL_OPSDONE] = localjsonobj[TOTAL_OPSDONE] + remotejsonobj[TOTAL_OPSDONE]
        aggobj[PERSERVER_OPSDONE] = []
        if (len(localjsonobj[PERSERVER_OPSDONE]) != len(remotejsonobj[PERSERVER_OPSDONE])):
            print "[ERROR][aggregate] for {}th obj, localjsonobj[PERSERVER_OPSDONE] size {} != remotejsonobj[PERSERVER_OPSDONE] size {}".format(i, len(localjsonobj[PERSERVER_OPSDONE]), len(remotejsonobj[PERSERVER_OPSDONE]))
            print "localjsonobj[PERSERVER_OPSDONE]: {}".format(localjsonobj[PERSERVER_OPSDONE])
            print "remotejsonobj[PERSERVER_OPSDONE]: {}".format(remotejsonobj[PERSERVER_OPSDONE])
            exit(-1)
        for j in range(len(localjsonobj[PERSERVER_OPSDONE])):
            aggobj[PERSERVER_OPSDONE].append(localjsonobj[PERSERVER_OPSDONE][j] + remotejsonobj[PERSERVER_OPSDONE][j])
        localjsonobj[PERSERVER_OPSDONE] + remotejsonobj[PERSERVER_OPSDONE]
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

        if STRID in tmpjsonobj.keys():
            tmpstrid = tmpjsonobj[STRID]
        else:
            tmpstrid = "tmpstrid"
        tmptotalops = tmpjsonobj[TOTAL_OPSDONE]
        tmp_perserverops = tmpjsonobj[PERSERVER_OPSDONE]
        tmptotaltime = tmpjsonobj[EXECUTION_MILLIS]

        tmp_cachemisscnt = 0
        tmp_maxserverops = 0
        for j in range(len(tmp_perserverops)):
            tmp_cachemisscnt += tmp_perserverops[j]
            if tmp_perserverops[j] > tmp_maxserverops:
                tmp_maxserverops = tmp_perserverops[j]
        
        tmp_cachehitcnt = tmptotalops - tmp_cachemisscnt
        tmp_normalizedthpt = float(tmptotalops) / float(tmp_maxserverops)
        print "[{}] thpt {}; cache hit rate {}; normalized thpt {}".format(tmpstrid, float(tmptotalops) / float(tmptotaltime), float(tmp_cachehitcnt) / float(tmptotalops), tmp_normalizedthpt)

def staticprocess(localjsonarray, remotejsonarray):
    if (len(localjsonarray) != len(remotejsonarray)):
        print "[ERROR][STATIC] client 0 jsonarray size {} != client 1 jsonarray size {}".format(len(localjsonarray), len(remotejsonarray))
        exit(-1)

    aggjsonarray = aggregate(localjsonarray, remotejsonarray, len(localjsonarray))

    calculate_perobjstat(aggjsonarray)

    bottleneckthpt = aggjsonarray[0][TOTAL_OPSDONE] / aggjsonarray[0][EXECUTION_MILLIS] * 1000 / 1000.0 / 1000.0
    totalthpt = bottleneckthpt
    for i in range(1, len(aggjsonarray)):
        boththpt = aggjsonarray[i][TOTAL_OPSDONE] / aggjsonarray[i][EXECUTION_MILLIS] * 1000 / 1000.0 / 1000.0
        rotatethpt = boththpt - bottleneckthpt
        if rotatethpt > 0:
            totalthpt += rotatethpt
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
        persecthpt[i] = aggjsonarray[i][TOTAL_OPSDONE] / aggjsonarray[i][EXECUTION_MILLIS] * 1000 / 1000.0 / 1000.0
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

if len(sys.argv) != 4:
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
    staticprocess(localjsonarray, remotejsonarray)
else:
    GLOBAL_PEROBJ_EXECUTION_MILLIS = DYNAMIC_PEROBJ_EXECUTION_MILLIS
    dynamicprocess(localjsonarray, remotejsonarray)
