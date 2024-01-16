import os
import re
import sys
import json

# import demjson

from helper import *

# PER-LINE FORMAT: "totalbwcost: <totalbwcost> MiB/s; localbwcost: <localbwcost> MiB/s"

if len(sys.argv) != 2:
    print("Invalid usage of calculate_bwcost_helper.py")
    print("Arguments: {}".format(sys.argv))
    exit(-1)

filepath = sys.argv[1]
# snapshot_period = int(sys.argv[2]) # in units of ms

if not os.path.exists(filepath):
    print("No such file {}".format(filepath))
    exit(-1)

with open(filepath, "r") as f:
    totalbwcost_list = []
    workloadmode = -1

    # for static pattern
    staticscale = -1
    staticbottleneckidx = -1
    perserver_localbwcost = []
    perserver_localbwcostcnt = []

    while True:
        tmpline = f.readline()
        if not tmpline:
            break

        tmpelements = tmpline.split(" ")

        # Parse STRID
        tmpstrid = tmpelements[
            0
        ]  # dynamic / static<scale>-<bottleneckidx>[-<rotateidx>]
        if workloadmode == -1:
            if tmpstrid.find("dynamic") != -1:  # dynamic
                workloadmode = 1
            else:  # static<scale>-<bottleneckidx>[-<rotateidx>]
                workloadmode = 0
        if workloadmode == 0:
            tmpstrid_elements = tmpstrid.split("-")
            if staticscale == -1:
                staticscale = int(tmpstrid_elements[0][6:])  # static<scale>
                staticbottleneckidx = int(tmpstrid_elements[1])  # <bottleneckidx>
                perserver_localbwcost = [0] * staticscale
                perserver_localbwcostcnt = [0] * staticscale
            if len(tmpstrid_elements) == 3:
                tmprotateidx = int(tmpstrid_elements[2])  # <rotateidx>
            else:
                tmprotateidx = -1

        # Parse totalbwcost
        tmptotalbwcost = float(tmpelements[2])

        tmpline = f.readline()  # localbwcost line
        if not tmpline:
            print("[ERROR] no localbwcost line after globalbwcost line")
            break

        if workloadmode == 1:  # dynamic pattern
            totalbwcost_list.append(tmptotalbwcost)  # skip localbwcost line
        else:  # static pattern
            # Parse localbwcost list
            tmpelements = tmpline.split(" ")
            tmp_localbwcost_list = tmpelements[2:]
            if len(tmp_localbwcost_list) != staticscale:
                print(
                    "[ERROR] tmp_localbwcost_list.length is {} != {}".format(
                        len(tmp_localbwcost_list), staticscale
                    )
                )
                break
            for i in range(len(tmp_localbwcost_list)):
                tmp_localbwcost_list[i] = float(tmp_localbwcost_list[i])

            # Update perserver localbwcost
            perserver_localbwcost[staticbottleneckidx] += tmp_localbwcost_list[
                staticbottleneckidx
            ]
            perserver_localbwcostcnt[staticbottleneckidx] += 1
            tmptotalbwcost -= tmp_localbwcost_list[staticbottleneckidx]
            if tmprotateidx != -1:
                perserver_localbwcost[tmprotateidx] += tmp_localbwcost_list[
                    tmprotateidx
                ]
                perserver_localbwcostcnt[tmprotateidx] += 1
                tmptotalbwcost -= tmp_localbwcost_list[tmprotateidx]

            totalbwcost_list.append(tmptotalbwcost)  # totalbwcost - localbwcosts

    avgtotalbwcost = 0.0
    for i in range(len(totalbwcost_list)):
        avgtotalbwcost += totalbwcost_list[i]
    avgtotalbwcost /= float(len(totalbwcost_list))

    perserver_avglocalbwcost = [0.0] * staticscale
    if workloadmode == 0:  # static pattern
        for i in range(len(perserver_avglocalbwcost)):
            perserver_avglocalbwcost[i] = perserver_localbwcost[i] / float(
                perserver_localbwcostcnt[i]
            )
            avgtotalbwcost += perserver_avglocalbwcost[i]
        print("perserver avglocalbwcost: {}".format(perserver_avglocalbwcost))

    # print("snapshot period: {}s".format(snapshot_period / 1000))
    print("average bwcost of entire control plane: {} MiB/s".format(avgtotalbwcost))
