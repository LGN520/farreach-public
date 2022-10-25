import os
import re
import sys
import json
#import demjson

STRID = "strid"
TOTAL_OPSDONE = "totalOpsdone" # including cache hits and cache misses of all servers
PERSERVER_OPSDONE = "perserverOpsdone" # including cache misses of each server, whose size = rotation scale (e.g., 128)
PERSERVER_CACHEHITS = "perserverCachehits" # include cache hits of each server, whose size = rotation scale (e.g., 128)
EXECUTION_MILLIS = "executionMillis"
#TOTAL_LATENCY = "totalLatency"
#TOTAL_LATENCYNUM = "totalLatencynum"
#TOTAL_HISTOGRAM = "totalHistogram"
#PERSERVER_TOTAL_LATENCY = "perservertotalLatency" # including total latency of each running server, whose size is 1 or 2
#PERSERVER_TOTAL_LATENCYNUM = "perservertotalLatencynum" # including total latencynum of each running server, whose size is 1 or 2
#PERSERVER_TOTAL_HISTOGRAM = "perservertotalHistogram" # including total latency histogram of each running server, whose size is 1/2 * 10000
PERSERVER_CACHEHIT_TOTAL_LATENCY = "perserverCachehitTotalLatency" # including total latency of each running server of cache hits, whose size is 1 or 2
PERSERVER_CACHEHIT_TOTAL_LATENCYNUM = "perserverCachehitTotalLatencynum" # including total latencynum of each running server of cache hits, whose size is 1 or 2
PERSERVER_CACHEHIT_TOTAL_HISTOGRAM = "perserverCachehitTotalHistogram" # including total latency histogram of each running server of cache hits, whose size is 1/2 * 10000
PERSERVER_CACHEMISS_TOTAL_LATENCY = "perserverCachemissTotalLatency" # including total latency of each running server of cache misses, whose size is 1 or 2
PERSERVER_CACHEMISS_TOTAL_LATENCYNUM = "perserverCachemissTotalLatencynum" # including total latencynum of each running server of cache misses, whose size is 1 or 2
PERSERVER_CACHEMISS_TOTAL_HISTOGRAM = "perserverCachemissTotalHistogram" # including total latency histogram of each running server of cache misses, whose size is 1/2 * 10000

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

def aggregate_2dlist(locallist, remotelist, objidx, role):
    result = []
    if len(locallist) != len(remotelist):
        print "[ERROR][aggregate] for {}th obj, localjsonobj[{}}] size {} != remotejsonobj[{}}] size {}".format(objidx, role, len(locallist), role, len(remotelist))
        exit(-1)
    for i in range(len(locallist)):
        tmplocalsublist = locallist[i]
        tmpremotesublist = remotelist[i]
        if len(tmplocalsublist) != len(tmpremotesublist):
            print "[ERROR][aggregate] for {}th obj {}th histogram, localsublist size {} != remotesublist size {}".format(objidx, i, len(tmplocalsublist), len(tmpremotesublist))
            print "localsublist: {}".format(tmplocalsublist)
            print "remotesublist: {}".format(tmpremotesublist)
            exit(-1)
        tmpaggsublist = []
        for j in range(len(tmplocalsublist)):
            tmpaggsublist.append(tmplocalsublist[j] + tmpremotesublist[j])
        result.append(tmpaggsublist)
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

        aggobj[PERSERVER_CACHEHIT_TOTAL_LATENCY] = aggregate_1dlist(localjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCY], remotejsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCY], i, PERSERVER_CACHEHIT_TOTAL_LATENCY)
        aggobj[PERSERVER_CACHEHIT_TOTAL_LATENCYNUM] = aggregate_1dlist(localjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCYNUM], remotejsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCYNUM], i, PERSERVER_CACHEHIT_TOTAL_LATENCYNUM)
        aggobj[PERSERVER_CACHEHIT_TOTAL_HISTOGRAM] = aggregate_2dlist(localjsonobj[PERSERVER_CACHEHIT_TOTAL_HISTOGRAM], remotejsonobj[PERSERVER_CACHEHIT_TOTAL_HISTOGRAM], i, PERSERVER_CACHEHIT_TOTAL_HISTOGRAM)

        aggobj[PERSERVER_CACHEMISS_TOTAL_LATENCY] = aggregate_1dlist(localjsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCY], remotejsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCY], i, PERSERVER_CACHEMISS_TOTAL_LATENCY)
        aggobj[PERSERVER_CACHEMISS_TOTAL_LATENCYNUM] = aggregate_1dlist(localjsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCYNUM], remotejsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCYNUM], i, PERSERVER_CACHEMISS_TOTAL_LATENCYNUM)
        aggobj[PERSERVER_CACHEMISS_TOTAL_HISTOGRAM] = aggregate_2dlist(localjsonobj[PERSERVER_CACHEMISS_TOTAL_HISTOGRAM], remotejsonobj[PERSERVER_CACHEMISS_TOTAL_HISTOGRAM], i, PERSERVER_CACHEMISS_TOTAL_HISTOGRAM)

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
    global workloadmode
    global bottleneckidx
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
            print "[{}] overall thpt {} MOPS; cache hit rate {}; cache miss rate {}; normalized thpt {}".format(tmpstrid, getmops(float(tmptotalops) / float(tmptotaltime)), float(tmp_cachehitcnt) / float(tmptotalops), float(tmp_cachemisscnt) / float(tmptotalops), tmp_normalizedthpt)

        for j in range(len(tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCY])):
            print "[idx {}]".format(j)
            if workloadmode == 0:
                if j == 0:
                    tmpserveridx = bottleneckidx
                else:
                    if i == 0:
                        print "[ERROR] first rotation cannot have {} > 1 latency number!".format(len(tmpjsonobj[PERSERVER_TOTAL_LATENCY]))
                        exit(-1)
                    if i <= bottleneckidx:
                        tmpserveridx = i - 1
                    else:
                        tmpserveridx = i
            else:
                tmpserveridx = j
            tmpthpt = getmops((tmpjsonobj[PERSERVER_CACHEHITS][tmpserveridx] + tmpjsonobj[PERSERVER_OPSDONE][tmpserveridx]) / tmptotaltime)
            tmpserverthpt = getmops(tmpjsonobj[PERSERVER_OPSDONE][tmpserveridx] / tmptotaltime)
            print "totalthpt {} MOPS, serverthpt {} MOPS".format(tmpthpt, tmpserverthpt)

            tmp_cachehit_totallatency = tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCY][j]
            tmp_cachehit_totallatencynum = tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCYNUM][j]
            tmp_cachehit_totallatencyhist = tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_HISTOGRAM][j]
            tmp_cachehit_avglatency, tmp_cachehit_latencymedium, tmp_cachehit_latency90p, tmp_cachehit_latency95p, tmp_cachehit_latency99p = calculatelatency(tmp_cachehit_totallatency, tmp_cachehit_totallatencynum, tmp_cachehit_totallatencyhist)
            print "cache hits: avglat {} us, midlat {} us, 90Plat {} us, 95Plat {} us, 99Plat {} us".format(tmp_cachehit_avglatency, tmp_cachehit_latencymedium, tmp_cachehit_latency90p, tmp_cachehit_latency95p, tmp_cachehit_latency99p)

            tmp_cachemiss_totallatency = tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCY][j]
            tmp_cachemiss_totallatencynum = tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCYNUM][j]
            tmp_cachemiss_totallatencyhist = tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_HISTOGRAM][j]
            tmp_cachemiss_avglatency, tmp_cachemiss_latencymedium, tmp_cachemiss_latency90p, tmp_cachemiss_latency95p, tmp_cachemiss_latency99p = calculatelatency(tmp_cachemiss_totallatency, tmp_cachemiss_totallatencynum, tmp_cachemiss_totallatencyhist)
            print "cache misses: avglat {} us, midlat {} us, 90Plat {} us, 95Plat {} us, 99Plat {} us".format(tmp_cachemiss_avglatency, tmp_cachemiss_latencymedium, tmp_cachemiss_latency90p, tmp_cachemiss_latency95p, tmp_cachemiss_latency99p)
    return

def staticprocess(localjsonarray, remotejsonarray, bottleneckidx):
    if (len(localjsonarray) != len(remotejsonarray)):
        print "[ERROR][STATIC] client 0 jsonarray size {} != client 1 jsonarray size {}".format(len(localjsonarray), len(remotejsonarray))
        exit(-1)

    aggjsonarray = aggregate(localjsonarray, remotejsonarray, len(localjsonarray))

    calculate_perobjstat(aggjsonarray)

    # (1) Remove runtime variance on thpt

    avgbottleneckthpt = getmops(aggjsonarray[0][TOTAL_OPSDONE] / aggjsonarray[0][EXECUTION_MILLIS])
    avgbottleneck_serverthpt = getmops(aggjsonarray[0][PERSERVER_OPSDONE][bottleneckidx] / aggjsonarray[0][EXECUTION_MILLIS])
    for i in range(1, len(aggjsonarray)):
        tmpjsonobj = aggjsonarray[i]
        tmpexectime = tmpjsonobj[EXECUTION_MILLIS]
        tmp_bottleneckthpt = getmops((tmpjsonobj[PERSERVER_CACHEHITS][bottleneckidx] + tmpjsonobj[PERSERVER_OPSDONE][bottleneckidx]) / tmpexectime)
        tmp_bottleneck_serverthpt = getmops(tmpjsonobj[PERSERVER_OPSDONE][bottleneckidx] / tmpexectime)

        avgbottleneckthpt += tmp_bottleneckthpt
        avgbottleneck_serverthpt += tmp_bottleneck_serverthpt
    avgbottleneckthpt /= float(len(aggjsonarray))
    avgbottleneck_serverthpt /= float(len(aggjsonarray))
    print "[STATIC] average bottleneck throughput: {} MOPS; average bottleneck server throughput: {} MOPS".format(avgbottleneckthpt, avgbottleneck_serverthpt)

    totalthpt = avgbottleneckthpt
    max_serverthpt = avgbottleneck_serverthpt
    total_serverthpt = avgbottleneck_serverthpt
    # prepare for latency statistics (as server rotation only has two servers which over-estimates server-side latency, we use a simulate ratio to reduce the simulation variance)
    latency_simulation_ratios = [avgbottleneck_serverthpt]

    tmpbottleneckthpt = getmops(aggjsonarray[0][TOTAL_OPSDONE] / aggjsonarray[0][EXECUTION_MILLIS])
    print "[0th statistics] for {}: bottleneck thpt {}, rotated thpt 0, total thpt {}".format(bottleneckidx, tmpbottleneckthpt, tmpbottleneckthpt)
    for i in range(1, len(aggjsonarray)):
        tmpjsonobj = aggjsonarray[i]
        tmpexectime = tmpjsonobj[EXECUTION_MILLIS]
        tmp_bottleneckthpt = getmops((tmpjsonobj[PERSERVER_CACHEHITS][bottleneckidx] + tmpjsonobj[PERSERVER_OPSDONE][bottleneckidx]) / tmpexectime)
        tmp_bottleneck_serverthpt = getmops(tmpjsonobj[PERSERVER_OPSDONE][bottleneckidx] / tmpexectime)

        if i <= bottleneckidx:
            tmp_rotateidx = i - 1
        else:
            tmp_rotateidx = i
        tmp_rotatethpt = getmops((tmpjsonobj[PERSERVER_CACHEHITS][tmp_rotateidx] + tmpjsonobj[PERSERVER_OPSDONE][tmp_rotateidx]) / tmpexectime)
        tmp_rotate_serverthpt = getmops(tmpjsonobj[PERSERVER_OPSDONE][tmp_rotateidx] / tmpexectime)

        tmp_rotatethpt *= (avgbottleneckthpt / tmp_bottleneckthpt)
        tmp_rotate_serverthpt *= (avgbottleneck_serverthpt / tmp_bottleneck_serverthpt)

        totalthpt += tmp_rotatethpt
        if tmp_rotate_serverthpt > max_serverthpt:
            max_serverthpt = tmp_rotate_serverthpt
        total_serverthpt += tmp_rotate_serverthpt
        latency_simulation_ratios.append(tmp_rotate_serverthpt)
    normalized_thpt = float(totalthpt) / float(max_serverthpt)
    print "[STATIC] aggregate throughput: {} MOPS; normalized throughput: {}".format(totalthpt, normalized_thpt)

    for i in range(len(latency_simulation_ratios)):
        latency_simulation_ratios[i] = float(latency_simulation_ratios[i]) / float(total_serverthpt)

    # (2) Reduce runtime variance on latency

    # Get bottleneck latency statistics

    avgcachehit_bottleneckhist = [] # latency histogram for bottleneck partition of cache hits
    avgcachemiss_bottleneckhist = [] # latency histogram for bottleneck partition of cache misses
    for i in range(len(aggjsonarray)):
        tmpjsonobj = aggjsonarray[i]
        tmpcachehit_bottleneckhist = tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_HISTOGRAM][0]
        tmpcachemiss_bottleneckhist = tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_HISTOGRAM][0]
        if len(avgcachehit_bottleneckhist) == 0:
            avgcachehit_bottleneckhist = [0.0] * len(tmpcachehit_bottleneckhist)
            avgcachemiss_bottleneckhist = [0.0] * len(tmpcachemiss_bottleneckhist)
        for j in range(len(avgcachehit_bottleneckhist)):
            avgcachehit_bottleneckhist[j] += tmpcachehit_bottleneckhist[j]
            avgcachemiss_bottleneckhist[j] += tmpcachemiss_bottleneckhist[j]
    bottleneck_simulationratio = latency_simulation_ratios[0] # latency simulation ratio of bottleneck partition
    for i in range(len(avgcachehit_bottleneckhist)):
        avgcachehit_bottleneckhist[i] = float(avgcachehit_bottleneckhist[i]) / float(len(aggjsonarray))
        avgcachemiss_bottleneckhist[i] = float(avgcachemiss_bottleneckhist[i]) / float(len(aggjsonarray)) * float(bottleneck_simulationratio)

    avgbottleneckhist = []
    for i in range(len(avgcachehit_bottleneckhist)):
        avgbottleneckhist.append(avgcachehit_bottleneckhist[i] + avgcachemiss_bottleneckhist[i])
    #avgbottlenecktotallatency = 0.0
    #avgbottlenecktotallatencynum = 0.0
    #for i in range(len(avgbottleneckhist)):
    #    avgbottlenecktotallatency += float(i) * avgbottleneckhist[i]
    #    avgbottlenecktotallatencynum += avgbottleneckhist[i]

    #totallatency = avgbottlenecktotallatency
    #totallatencynum = avgbottlenecktotallatencynum
    totallatencyhist = [0] * len(avgbottleneckhist)
    for i in range(len(avgbottleneckhist)):
        totallatencyhist[i] = avgbottleneckhist[i]

    # Get each rotated partition latency statistics

    #avgrotatehist = [] # latency histogram for rotate partition
    for i in range(1, len(aggjsonarray)):
        tmpjsonobj = aggjsonarray[i]
        tmpcachehit_rotatehist = tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_HISTOGRAM][1]
        #tmpcachehit_rotatetotallatency = tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCY][1]
        #tmpcachehit_rotatetotallatencynum = tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCYNUM][1]
        tmpcachemiss_rotatehist = tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_HISTOGRAM][1]
        #tmpcachemiss_rotatetotallatency = tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCY][1]
        #tmpcachemiss_rotatetotallatencynum = tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCYNUM][1]

        tmpsimulationratio = latency_simulation_ratios[i]
        #totallatency += (tmpcachehit_rotatehist + tmpcachemiss_rotatehist / tmpsimulationratio)
        #totallatencynum += (tmpcachehit_rotatetotallatencynum + tmpcachemiss_rotatetotallatencynum / tmpsimulationratio)
        for j in range(len(totallatencyhist)):
            totallatencyhist[j] += (tmpcachehit_rotatehist[j] + float(tmpcachemiss_rotatehist[j]) * tmpsimulationratio)

    totallatency = 0.0
    totallatencynum = 0.0
    for i in range(len(totallatencyhist)):
        totallatency += float(i) * totallatencyhist[i]
        totallatencynum += totallatencyhist[i]

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
            tmp_totallatency += (tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCY][j] + tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCY][j])
            tmp_totallatencynum += (tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCYNUM][j] + tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCYNUM][j])
            for k in range(len(tmp_totallatencyhist)):
                tmp_totallatencyhist[k] += (tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_HISTOGRAM][j][k] + tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_HISTOGRAM][j][k])
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
    totalhistogram = [0] * len(aggjsonarray[0][PERSERVER_CACHEHIT_TOTAL_HISTOGRAM][0])
    for i in range(len(aggjsonarray)):
        tmpjsonobj = aggjsonarray[i]
        for j in range(len(tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCY])):
            totallatency += (tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCY][j] + tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCY][j])
            totallatencynum += (tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCYNUM][j] + tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCYNUM][j])
            for k in range(len(tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCYNUM][j])):
                totalhistogram[k] += (tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_HISTOGRAM][j][k] + tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_HISTOGRAM][j][k])

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
