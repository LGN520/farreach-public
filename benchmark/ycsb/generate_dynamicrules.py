import sys
import os
import shutil
import copy
import numpy as np

periodnum = 8 # the first 10s w/o rulemap + subsequent seconds w/ rulemap every 10s
evictnum = 200 # replace 200 keys every 10s
rulenum = 40000 # the number of mapping rules

np.random.seed(0)

def loadkeys(filename):
    print "Load keys from {}".format(filename)
    fd = open(filename, "r")
    line = fd.readline()
    keys = []
    while line:
        elements = line.split(" ")
        for i in range(len(elements)):
            beginidx = elements[i].find("user")
            if beginidx != -1:
                #beginidx += 4
                #keystr = elements[i][beginidx:]
                keystr = elements[i] # NOTE: NOT skip "user" for RemoteDBKey::fromString()
                keys.append(keystr)
                break
        line = fd.readline()
    return keys

# Map oldkeys into newkeys
def dumprules(oldkeys, newkeys, dirname, periodidx):
    rulefilepath = os.path.join(dirname, "{}.out".format(periodidx))
    print "Dump rules into {}".format(rulefilepath)
    fd = open(rulefilepath, "w")
    for i in range(len(oldkeys)): # 0, 1, ..., 9999
        fd.write("{} {}\n".format(oldkeys[i], newkeys[i]))
    fd.flush()
    fd.close()

if len(sys.argv) != 2:
    print "Usage: python generate_dynamicrules workloadname"
    print "Example: python generate_dynamicrules workloada"
    exit(-1)

workloadname = sys.argv[1]
print "workloadname: {}".format(workloadname)

hotfilename = "../output/{}-hotest.out".format(workloadname)
hotestkeys = loadkeys(hotfilename)

coldfilename = "../output/{}-coldest.out".format(workloadname)
coldestkeys = loadkeys(coldfilename)

nearhotfilename = "../output/{}-nearhot.out".format(workloadname)
nearhotkeys = loadkeys(nearhotfilename)

if len(hotestkeys) < rulenum:
    print "[WARNING] the number of hot keys {} should >= rulenum {}".format(len(hotestkeys), rulenum)
    rulenum = len(hotestkeys)

# Hotin rules
hotin_dir = "../output/{}-hotinrules".format(workloadname)
if os.path.exists(hotin_dir):
    shutil.rmtree(hotin_dir)
os.mkdir(hotin_dir)
current_hotkeys = copy.deepcopy(hotestkeys)
for periodidx in range(0, periodnum): # 0, 2, ..., 5
    if periodidx != 0: # NOTE: we do NOT change key popularity in the first 10s
        cold_startidx = (periodidx - 1) * evictnum
        if cold_startidx + evictnum >= len(coldestkeys):
            print "[WARNING] coldidx {} exceeds # of coldkeys {}".format(cold_startidx + evictnum, len(coldestkeys))
            break
        for ruleidx in range(0, rulenum): # 0, 1, ..., 9999
            if ruleidx < evictnum: # 0, 1, ..., 199
                current_hotkeys[ruleidx] = coldestkeys[cold_startidx + ruleidx]
            else:
                current_hotkeys[ruleidx] = prev_hotkeys[ruleidx - evictnum]
    dumprules(hotestkeys, current_hotkeys, hotin_dir, periodidx)
    prev_hotkeys = copy.deepcopy(current_hotkeys)

# Random rules
random_dir = "../output/{}-randomrules".format(workloadname)
if os.path.exists(random_dir):
    shutil.rmtree(random_dir)
os.mkdir(random_dir)
coldidxspace = np.array(range(len(coldestkeys)))
hotidxspace = np.array(range(len(hotestkeys)))
current_hotkeys = copy.deepcopy(hotestkeys)
for periodidx in range(0, periodnum): # 0, 2, ..., 5
    if periodidx != 0: # NOTE: we do NOT change key popularity in the first 10s
        coldidxes = np.random.choice(coldidxspace, evictnum, replace=False)
        hotidxes = np.random.choice(hotidxspace, evictnum, replace=False)
        for ruleidx in range(0, rulenum): # 0, 1, ..., 9999
            evictidx = -1
            for i in range(len(hotidxes)):
                if ruleidx == hotidxes[i]:
                    evictidx = i
                    break
            if evictidx != -1:
                current_hotkeys[ruleidx] = coldestkeys[coldidxes[i]]
            else:
                current_hotkeys[ruleidx] = prev_hotkeys[ruleidx]
    dumprules(hotestkeys, current_hotkeys, random_dir, periodidx)
    prev_hotkeys = copy.deepcopy(current_hotkeys)

# Hotout rules
hotout_dir = "../output/{}-hotoutrules".format(workloadname)
if os.path.exists(hotout_dir):
    shutil.rmtree(hotout_dir)
os.mkdir(hotout_dir)
current_hotkeys = copy.deepcopy(hotestkeys)
for periodidx in range(0, periodnum): # 0, 2, ..., 5
    if periodidx != 0: # NOTE: we do NOT change key popularity in the first 10s
        nearhot_startidx = (periodidx - 1) * evictnum
        if nearhot_startidx + evictnum >= len(nearhotkeys):
            print "[WARNING] nearhotidx {} exceeds # of nearhotkeys {}".format(nearhot_startidx + evictnum, len(nearhotkeys))
            break
        for ruleidx in range(0, rulenum): # 0, 1, ..., 9999
            if ruleidx < len(current_hotkeys) - evictnum: # 0, 1, ..., 9999 - 200
                current_hotkeys[ruleidx] = prev_hotkeys[ruleidx + evictnum]
            else: # 9999 - 200 + 1, ..., 9999
                current_hotkeys[ruleidx] = nearhotkeys[nearhot_startidx + (ruleidx - (len(current_hotkeys) - evictnum))]
    dumprules(hotestkeys, current_hotkeys, hotout_dir, periodidx)
    prev_hotkeys = copy.deepcopy(current_hotkeys)
