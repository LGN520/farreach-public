#!/bin/python
import json
import numpy as np
import os
import re
def extract_throughput(filename):
    # 
    pattern = r"\[OVERALL\], Throughput\(ops/sec\), (\d+\.\d+)"
    # 
    with open(filename, "r") as file:
        for line in file:
            # 
            match = re.search(pattern, line)
            if match:
                # 
                throughput = match.group(1)
                return float(throughput)

    # 
    return None

def extract_cachehit(filename):

    pattern = r"\[INFO\]\[InSwitchCacheClient\] cacheHitCount: (\d+)"

    with open(filename, "r") as file:
        for line in file:

            match = re.search(pattern, line)
            if match:

                throughput = match.group(1)
                return float(throughput)

    return -1

exp_round= 30000
result_path = f"/root/aeresults/exp_rack/{exp_round}"
print(result_path)
# print(len(os.walk(result_path)))
for root, dirs, files in os.walk(result_path):
    for filename in files:
        method_idx = 0
        split_filename = filename.split("-")
        client0_result = filename
        client1_result = filename.replace("client0", "client1")
        method = ""
        workload = ""
        if split_filename[-1] == "client0.out":
            continue
        if (
            split_filename[1] == "distcache"
            or split_filename[1] == "distnocache"
            or split_filename[1] == "distreach"
        ):
            method_idx = 1
            workload = split_filename[0]
        else:
            method_idx = 2
            workload = split_filename[0] + "-" + split_filename[1]
        method = split_filename[method_idx]
        # print(split_filename)
        rack_num = int(int(split_filename[method_idx + 2]) / 2)
        print(method, rack_num, workload, extract_throughput(root+"/"+filename),extract_cachehit(root+"/"+filename)/999936)
        # print(method, rack_num, workload, cache_hit_rate, fairness_index,(sum_of_elements+(np.sum(sumCachehits)))/exectime)#,serverOpsdone.max())
