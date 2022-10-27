STRID = "strid"
TOTAL_OPSDONE = "totalOpsdone" # including cache hits and cache misses of all logical servers
PERSERVER_OPSDONE = "perserverOpsdone" # including cache misses of each logical server, whose size = rotation scale (e.g., 128)
PERSERVER_CACHEHITS = "perserverCachehits" # include cache hits of each logical server, whose size = rotation scale (e.g., 128)
EXECUTION_MILLIS = "executionMillis"
#TOTAL_LATENCY = "totalLatency"
#TOTAL_LATENCYNUM = "totalLatencynum"
#TOTAL_HISTOGRAM = "totalHistogram"
#PERSERVER_TOTAL_LATENCY = "perservertotalLatency" # including total latency of each physical server, whose size is 1 or 2
#PERSERVER_TOTAL_LATENCYNUM = "perservertotalLatencynum" # including total latencynum of each physical server, whose size is 1 or 2
#PERSERVER_TOTAL_HISTOGRAM = "perservertotalHistogram" # including total latency histogram of each physical server, whose size is 1/2 * 10000
PERSERVER_CACHEHIT_TOTAL_LATENCY = "perserverCachehitTotalLatency" # including total latency of each physical server of cache hits, whose size is 1 or 2
PERSERVER_CACHEHIT_TOTAL_LATENCYNUM = "perserverCachehitTotalLatencynum" # including total latencynum of each physical server of cache hits, whose size is 1 or 2
PERSERVER_CACHEHIT_TOTAL_HISTOGRAM = "perserverCachehitTotalHistogram" # including total latency histogram of each physical server of cache hits, whose size is 1/2 * 10000
PERSERVER_CACHEMISS_TOTAL_LATENCY = "perserverCachemissTotalLatency" # including total latency of each physical server of cache misses, whose size is 1 or 2
PERSERVER_CACHEMISS_TOTAL_LATENCYNUM = "perserverCachemissTotalLatencynum" # including total latencynum of each physical server of cache misses, whose size is 1 or 2
PERSERVER_CACHEMISS_TOTAL_HISTOGRAM = "perserverCachemissTotalHistogram" # including total latency histogram of each physical server of cache misses, whose size is 1/2 * 10000

STATIC_PEROBJ_EXECUTION_MILLIS = 10 * 1000
DYNAMIC_PEROBJ_EXECUTION_MILLIS = 1 * 1000
GLOBAL_PEROBJ_EXECUTION_MILLIS = -1

# Utils

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

def calculatelatency(totalhistogram):
    totallatency, totallatencynum = get_total_latency_and_latencynum(totalhistogram)
    if totallatencynum == 0:
        avglatency = 0
    else:
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
    if totallatencynum > 0:
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
    minlatency = 0
    maxlatency = 0
    for i in range(len(totalhistogram)):
        if minlatency == 0 and totalhistogram[i] != 0:
            minlatency = i
        if maxlatency == 0 and totalhistogram[len(totalhistogram) - 1 - i] != 0:
            maxlatency = len(totalhistogram) - 1 - i
    return avglatency, latencymedium, latency90p, latency95p, latency99p, minlatency, maxlatency

def get_logical_serveridx(rotationidx, physicalidx, workloadmode, bottleneckidx):
    if workloadmode == 0:
        if physicalidx == 0:
            tmp_logical_serveridx = bottleneckidx
        else:
            if rotationidx == 0:
                print "[ERROR] first rotation cannot have > 1 latency number!"
                exit(-1)
            if rotationidx <= bottleneckidx:
                tmp_logical_serveridx = rotationidx - 1
            else:
                tmp_logical_serveridx = rotationidx
    else:
        tmp_logical_serveridx = physicalidx
    return tmp_logical_serveridx

def get_total_switch_server_thpts(jsonarray, rotationidx, physicalidx, workloadmode, bottleneckidx):
    tmp_logical_serveridx = get_logical_serveridx(rotationidx, physicalidx, workloadmode, bottleneckidx)

    tmpjsonobj = jsonarray[rotationidx]
    tmptotaltime = tmpjsonobj[EXECUTION_MILLIS]
    tmptotalthpt = getmops((tmpjsonobj[PERSERVER_CACHEHITS][tmp_logical_serveridx] + tmpjsonobj[PERSERVER_OPSDONE][tmp_logical_serveridx]) / tmptotaltime)
    tmpswitchthpt = getmops(tmpjsonobj[PERSERVER_CACHEHITS][tmp_logical_serveridx] / tmptotaltime)
    tmpserverthpt = getmops(tmpjsonobj[PERSERVER_OPSDONE][tmp_logical_serveridx] / tmptotaltime)
    return tmptotalthpt, tmpswitchthpt, tmpserverthpt

def get_total_cachehit_cachemiss_latencyhist(jsonarray, rotationidx, physicalidx):
    tmp_cachehit_totallatencyhist = jsonarray[rotationidx][PERSERVER_CACHEHIT_TOTAL_HISTOGRAM][physicalidx]
    tmp_cachemiss_totallatencyhist = jsonarray[rotationidx][PERSERVER_CACHEMISS_TOTAL_HISTOGRAM][physicalidx]
    tmp_totallatencyhist = [0] * len(tmp_cachehit_totallatencyhist)
    for i in range(len(tmp_totallatencyhist)):
        tmp_totallatencyhist[i] = tmp_cachehit_totallatencyhist[i] + tmp_cachemiss_totallatencyhist[i]
    return tmp_totallatencyhist, tmp_cachehit_totallatencyhist, tmp_cachemiss_totallatencyhist

def get_total_latency_and_latencynum(latencyhist):
    totallatency = 0.0
    totallatencynum = 0.0
    for i in range(len(latencyhist)):
        totallatency += float(i) * latencyhist[i]
        totallatencynum += latencyhist[i]
    return totallatency, totallatencynum

def calculate_perobjstat(aggjsonarray, workloadmode, bottleneckidx):
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

            #tmp_cachehit_totallatency = tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCY][j]
            #tmp_cachehit_totallatencynum = tmpjsonobj[PERSERVER_CACHEHIT_TOTAL_LATENCYNUM][j]
            #tmp_cachemiss_totallatency = tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCY][j]
            #tmp_cachemiss_totallatencynum = tmpjsonobj[PERSERVER_CACHEMISS_TOTAL_LATENCYNUM][j]
            #tmp_totallatency = tmp_cachehit_totallatency + tmp_cachemiss_totallatency
            #tmp_totallatencynum = tmp_cachehit_totallatencynum + tmp_cachemiss_totallatencynum

            tmp_totallatencyhist, tmp_cachehit_totallatencyhist, tmp_cachemiss_totallatencyhist = get_total_cachehit_cachemiss_latencyhist(aggjsonarray, i, j)

            tmp_cachehit_avglatency, tmp_cachehit_latencymedium, tmp_cachehit_latency90p, tmp_cachehit_latency95p, tmp_cachehit_latency99p, tmp_cachehit_minlatency, tmp_cachehit_maxlatency = calculatelatency(tmp_cachehit_totallatencyhist)
            print "cache hits: avglat {} us, midlat {} us, 90Plat {} us, 95Plat {} us, 99Plat {} us, minlat {} us, maxlat {} us".format(tmp_cachehit_avglatency, tmp_cachehit_latencymedium, tmp_cachehit_latency90p, tmp_cachehit_latency95p, tmp_cachehit_latency99p, tmp_cachehit_minlatency, tmp_cachehit_maxlatency)
            tmp_cachemiss_avglatency, tmp_cachemiss_latencymedium, tmp_cachemiss_latency90p, tmp_cachemiss_latency95p, tmp_cachemiss_latency99p, tmp_cachemiss_minlatency, tmp_cachemiss_maxlatency = calculatelatency(tmp_cachemiss_totallatencyhist)
            print "cache misses: avglat {} us, midlat {} us, 90Plat {} us, 95Plat {} us, 99Plat {} us, minlat {} us, maxlat {} us".format(tmp_cachemiss_avglatency, tmp_cachemiss_latencymedium, tmp_cachemiss_latency90p, tmp_cachemiss_latency95p, tmp_cachemiss_latency99p, tmp_cachemiss_minlatency, tmp_cachemiss_maxlatency)
            tmp_avglatency, tmp_latencymedium, tmp_latency90p, tmp_latency95p, tmp_latency99p, tmp_minlatency, tmp_maxlatency = calculatelatency(tmp_totallatencyhist)
            print "cache hits+misses: avglat {} us, midlat {} us, 90Plat {} us, 95Plat {} us, 99Plat {} us, minlat {} us, maxlat {} us".format(tmp_avglatency, tmp_latencymedium, tmp_latency90p, tmp_latency95p, tmp_latency99p, tmp_minlatency, tmp_maxlatency)

            tmptotalthpt, tmpswitchthpt, tmpserverthpt = get_total_switch_server_thpts(aggjsonarray, i, j, workloadmode, bottleneckidx)
            print "totalthpt {} MOPS, switchthpt {} MOPS, serverthpt {} MOPS, overflow latencynum: {}".format(tmptotalthpt, tmpswitchthpt, tmpserverthpt, tmp_totallatencyhist[-1])
    return
