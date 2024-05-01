#!/bin/python
import json
import numpy as np
import os
# distnocache 2 synthetic-25 
# distcache 2 synthetic-25
# distnocache 2 workloadd
# distcache 2 synthetic
# distnocache 2 synthetic-75
# distcache 2 synthetic-75
# distcache 2 synthetic
# distcache 2 workloada
# distnocache 2 workloada
exp_round= 515
result_path = f"/root/aeresults/exp_rack/{exp_round}"
print(result_path)
# print(len(os.walk(result_path)))
for root, dirs, files in os.walk(result_path):
    for filename in files:
        if filename.endswith("txt"):
            continue
        method_idx = 0
        split_filename = filename.split("-")
        client0_result = filename
        client1_result = filename.replace("client0", "client1")
        method = ""
        workload = ""
        if split_filename[-1] == "client1.out " :
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
        # server_num = rack_num * 16 * 2
        
        # print(result_path + "/" + client0_result)
        with open(result_path + "/" + client0_result) as f:
            python_dict = json.load(f)
        server_num = len(np.array(python_dict[0]["perserverCachehits"]))
        # perserverCachehits
        # perserverOpsdone
        # totalOpsdone
        sumCachehits = np.zeros(server_num)
        sumperserverCachehits = np.zeros(server_num)
        sumtotalOpsdone = 0
        serverOpsdone = np.zeros(server_num)
        # print(len(python_dict))
        exectime=0
        for item in python_dict:
            exectime = item["executionMillis"]/1000
            # print(len(item["perserverCachehits"]),server_num,rack_num)
            sumperserverCachehits += np.array(item["perserverCachehits"])
            tmpcachesum = np.sum(np.array(item["perserverCachehits"]))
            sumCachehits += np.array(item["perserverCachehits"])
            sumtotalOpsdone += item["totalOpsdone"]
            serverOpsdone = serverOpsdone + np.array(item["perserverOpsdone"])
        # print(exectime)

        # sum_of_squares = np.sum(np.square(serverOpsdone))
        # sum_of_elements = np.sum(serverOpsdone)
        # fairness_index = (sum_of_elements**2) / (len(serverOpsdone) * sum_of_squares)
        # print(result_path + "/" + client1_result)
        if os.path.exists(result_path + "/" + client1_result):
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
        # print(serverOpsdone.max(),serverOpsdone.min())
        # print(sum_of_elements/serverOpsdone.max(),(np.sum(sumCachehits))/serverOpsdone.max())
        # print(np.argmax(serverOpsdone), serverOpsdone[np.argmax(serverOpsdone)])
        # print(
        #     np.argmax(sumperserverCachehits),
        #     sumperserverCachehits[np.argmax(sumperserverCachehits)],
        # )
        # print(serverOpsdone.max())
        # if workload =="skewness-90" or  workload =="skewness-95" or  workload =="synthetic" or  workload =="uniform": 
        print('%12s'%method, rack_num, '%15s'%workload, '%.5f'%cache_hit_rate, '%.5f'%((sum_of_elements+(np.sum(sumCachehits)))/exectime))#,serverOpsdone.max())
