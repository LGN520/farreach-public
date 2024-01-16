import os
import re
import sys
import json

# import demjson

from helper import *

TARGET_AGGTHPT = 1.0  # target aggregate thpt is 1MOPS

# Calculate targets


def calculate_targets(localjsonarray, remotejsonarray, bottleneckidx):
    if len(localjsonarray) != len(remotejsonarray):
        print(
            "[ERROR][STATIC] client 0 jsonarray size {} != client 1 jsonarray size {}".format(
                len(localjsonarray), len(remotejsonarray)
            )
        )
        exit(-1)

    workloadmode = 0

    aggjsonarray = aggregate(localjsonarray, remotejsonarray, len(localjsonarray))

    # (1) Get aggregate thpt and per-rotation thpt

    perrotation_thpts = []

    (
        avgbottleneck_totalthpt,
        avgbottleneck_switchthpt,
        avgbottleneck_serverthpt,
    ) = get_total_switch_server_thpts(aggjsonarray, 0, 0, workloadmode, bottleneckidx)
    perrotation_thpts.append(avgbottleneck_totalthpt)
    for i in range(1, len(aggjsonarray)):
        tmp_bottleneck_totalthpt, _, _ = get_total_switch_server_thpts(
            aggjsonarray, i, 0, workloadmode, bottleneckidx
        )
        avgbottleneck_totalthpt += tmp_bottleneck_totalthpt

    avgbottleneck_totalthpt /= float(len(aggjsonarray))

    totalthpt = avgbottleneck_totalthpt

    for i in range(1, len(aggjsonarray)):
        tmp_bottleneck_totalthpt, _, _ = get_total_switch_server_thpts(
            aggjsonarray, i, 0, workloadmode, bottleneckidx
        )
        tmp_rotate_totalthpt, _, _ = get_total_switch_server_thpts(
            aggjsonarray, i, 1, workloadmode, bottleneckidx
        )

        perrotation_thpts.append(tmp_bottleneck_totalthpt + tmp_rotate_totalthpt)

        tmp_rotate_totalthpt *= avgbottleneck_totalthpt / tmp_bottleneck_totalthpt
        totalthpt += tmp_rotate_totalthpt

    weight = TARGET_AGGTHPT / float(totalthpt)
    output = ""
    for i in range(len(perrotation_thpts)):
        perrotation_thpts[i] *= weight
        if i == 0:
            output = "{}".format(perrotation_thpts[i] * 1000000 / 2)
        else:
            output = "{} {}".format(output, perrotation_thpts[i] * 1000000 / 2)
    print(output)


if len(sys.argv) != 4:
    print("Invalid usage of calculate_statistics_helper.py")
    print("Arguments: {}".format(sys.argv))
    exit(-1)

workloadmode = 0

localfilepath = sys.argv[1]
remotefilepath = sys.argv[2]
bottleneckidx = int(sys.argv[3])

if not os.path.exists(localfilepath):
    print("No such file {}".format(localfilepath))
    exit(-1)

if not os.path.exists(remotefilepath):
    print("No such file {}".format(remotefilepath))
    exit(-1)

localjsonstr = open(localfilepath).read()
remotejsonstr = open(remotefilepath).read()

localjsonarray = json.loads(localjsonstr)
remotejsonarray = json.loads(remotejsonstr)

GLOBAL_PEROBJ_EXECUTION_MILLIS = STATIC_PEROBJ_EXECUTION_MILLIS
calculate_targets(localjsonarray, remotejsonarray, bottleneckidx)
