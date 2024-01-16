#!/bin/python
import json
import numpy as np
import os

result_path = "/root/aeresults/exp_rack/1"

for root, dirs, files in os.walk(result_path):
    for filename in files:
        method_idx = 0
        split_filename = filename.split("-")
        client0_result = filename
        client1_result = filename.replace("client0", "client1")
        method = ""
        workload = ""
        if split_filename[-1] == "client1.out ":
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
        rack_num = int(int(split_filename[method_idx + 2]) / 2)

        # skip bug
        # if (
        #     # (workload == "workloadb" and method == "distreach" and rack_num == 3)
        #     # or (workload == "workloadc" and method == "distreach" and rack_num == 4)
        #     # (workload == "workloadc" and method == "distcache" and rack_num == 3) # x
        #     # or (workload == "workloada" and method == "distcache" and rack_num == 4)
        #     # (workload == "workloadd" and method == "distcache" and rack_num == 4) # x
        #     # or (workload == "workloadb" and method == "distreach" and rack_num == 4)
        #     # or (workload == "skewness-95" and method == "distreach" and rack_num == 3)
        #     # or (workload == "uniform" and method == "distreach" and rack_num == 3) 
        #     # or (workload == "uniform" and method == "distreach" and rack_num == 2) 
        #     (workload == "workloadd" and method == "distreach" and rack_num == 4) # x
        #     # or method != "distnocache"
        # ):
            # print("do it")
        # else:
            # continue
        # cac result
        server_num = rack_num * 16 * 2

        # print(result_path + "/" + client0_result)
        with open(result_path + "/" + client0_result) as f:
            python_dict = json.load(f)

        # perserverCachehits
        # perserverOpsdone
        # totalOpsdone
        sumCachehits = np.zeros(server_num)
        sumperserverCachehits = np.zeros(server_num)
        sumtotalOpsdone = 0
        serverOpsdone = np.zeros(server_num)
        for item in python_dict:
            # print(len(item["perserverCachehits"]),server_num,rack_num)
            sumperserverCachehits += np.array(item["perserverCachehits"])
            tmpcachesum = np.sum(np.array(item["perserverCachehits"]))
            sumCachehits += np.array(item["perserverCachehits"])
            sumtotalOpsdone += item["totalOpsdone"]
            serverOpsdone = serverOpsdone + np.array(item["perserverOpsdone"])
        # print(serverOpsdone)

        sum_of_squares = np.sum(np.square(serverOpsdone))
        sum_of_elements = np.sum(serverOpsdone)
        fairness_index = (sum_of_elements**2) / (len(serverOpsdone) * sum_of_squares)
        # print(result_path + "/" + client1_result)
        with open(result_path + "/" + client1_result) as f:
            python_dict = json.load(f)

        for item in python_dict:
            sumperserverCachehits += np.array(item["perserverCachehits"])
            tmpcachesum = np.sum(np.array(item["perserverCachehits"]))
            sumCachehits += np.array(item["perserverCachehits"])
            sumtotalOpsdone += item["totalOpsdone"]
            serverOpsdone = serverOpsdone + np.array(item["perserverOpsdone"])

        sum_of_squares = np.sum(np.square(serverOpsdone))
        sum_of_elements = np.sum(serverOpsdone)
        # Jain's fairness index
        fairness_index = (sum_of_elements**2) / (len(serverOpsdone) * sum_of_squares)
        cache_hit_rate = np.sum(sumCachehits) / sumtotalOpsdone
        # print("cache hit ratio", np.sum(sumCachehits) / sumtotalOpsdone)
        # print(f"Jain's fairness index: {fairness_index}")

        # print(np.argmax(serverOpsdone), serverOpsdone[np.argmax(serverOpsdone)])
        # print(
        #     np.argmax(sumperserverCachehits),
        #     sumperserverCachehits[np.argmax(sumperserverCachehits)],
        # )
        print(method, rack_num, workload, cache_hit_rate, fairness_index)
