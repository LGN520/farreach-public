import os
import re
import sys
import json
#import demjson

from helper import *

# PER-LINE FORMAT: "totalbwcost: <totalbwcost> MiB/s; localbwcost: <localbwcost> MiB/s"

if len(sys.argv) != 3:
    print "Invalid usage of calculate_bwcost_helper.py"
    print "Arguments: {}".format(sys.argv)
    exit(-1)

filepath = sys.argv[1]
snapshot_period = int(sys.argv[2]) # in units of ms

if not os.path.exists(filepath):
    print "No such file {}".format(filepath)
    exit(-1)

with open(filepath, "r") as f:
    totalbwcost_list = []
    localbwcost_list = []
    while True:
        tmpline = f.readline()
        if not tmpline:
            break

        tmpelements = tmpline.split(" ")
        tmptotalbwcost = float(tmpelements[1])
        tmplocalbwcost = float(tmpelements[4])
        totalbwcost_list.append(tmptotalbwcost)
        localbwcost_list.append(tmplocalbwcost)

    avgtotalbwcost = 0.0
    avglocalbwcost = 0.0
    for i in range(len(totalbwcost_list)):
        avgtotalbwcost += totalbwcost_list[i]
        avglocalbwcost += localbwcost_list[i]
    avgtotalbwcost /= float(len(totalbwcost_list))
    avglocalbwcost /= float(len(localbwcost_list))

    print "snapshot period: {}s".format(snapshot_period / 1000)
    print "average bwcost of entire control plane: {} MiB/s".format(avgtotalbwcost)
    print "average bwcost of local control plane: {} MiB/s".format(avglocalbwcost)
