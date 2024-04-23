#!/usr/bin/env python3
# run this scripts on host
# exp_static
import os
import sys
import argparse
import shutil
import subprocess
import multiprocessing
from time import sleep



current_path = os.path.dirname(os.path.abspath(__file__))
root_path = os.path.dirname(current_path)
sys.path.append(root_path)

from config.common import *
from utils.genconfig import genconfig, gennewconfig
from utils.serverlist import find_server_index, genserverlist
parser = argparse.ArgumentParser(description="exp_dynamic")
parser.add_argument("roundidx", type=int, help="an integer for the roundidx")
args = parser.parse_args()

is_bmv2 = 1

# "distnocache", "distreach", "distcache"
exp1_method_list=["distcache","distnocache", "distreach"]
exp1_method_dic = {
    "distreach": "farreach",
    "distcache": "netcache",
    "distnocache": "nocache",
}
exp1_core_workload_list=workload_list = [
    # "skewness-95",
    # "skewness-90",
    "synthetic",
    # "synthetic-25",
    # "synthetic-75",
    # "workloada",
    # "workloadb",
    # "workloadc",
    # "uniform",
    # "workloadf",
    # "workloadd",
    # "workload-load",
]
# exp1_physical_server_scale=8
exp1_server_scale_total = 16 
exp1_server_scale=16
exp1_output_path=f"{EVALUATION_OUTPUT_PREFIX}/exp_rack/{args.roundidx}"
scale_list = [16,8,2]
### Create json backup directory
os.makedirs(exp1_output_path, exist_ok=True)

# set dynamic_periodinterval to 5000 is enough to run 1.5M requests
# bottleneck idx is not needed in dist-method experiment, as we do not use server rotation
for dynamic_periodinterval in [5000]:
    for exp1_method in exp1_method_list:
        for exp1_workload in exp1_core_workload_list:
            for exp1_physical_server_scale in scale_list:
                exp1_server_scale = int(exp1_server_scale_total / exp1_physical_server_scale)
               
                print(f"[exp1][{exp1_method}][{exp1_workload}]")
                print(f"[exp1][{exp1_method}][{exp1_workload}] run workload with {exp1_workload}servers")
                ### Preparation
                os.system(f"bash {SWITCH_ROOTPATH}/{exp1_method}/localscriptsbmv2/stopswitchtestbed.sh")
                print(f"[exp1][{exp1_method}][{exp1_workload}] update {exp1_method} config with {exp1_workload}")

                rules = [
                    ('global','workload_name',str(exp1_workload)),
                    ('global','server_total_logical_num',str(exp1_server_scale_total)),
                    ('global','server_total_logical_num_for_rotation',str(exp1_server_scale_total)),
                    ('global','bottleneck_serveridx_for_rotation','0'),
                    ('global','workload_mode','0'),
                    # ('global','client_physical_num','1'),
                    # ('global','client_total_logical_num','2'),
                ]
                
                gennewconfig(
                    f"{CLIENT_ROOTPATH}/{exp1_method}/configs/config.ini.dynamic",
                    f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini",
                    rules,
                )
                commonconfig = CommonConfig(exp1_method)
                
                print(f"[exp1][{exp1_method}][{exp1_workload}] prepare server rotation")

                print("prepare server rotation")
                rotated_servers_list = genserverlist(commonconfig.server_total_logical_num_for_rotation,exp1_physical_server_scale)
                print(f"Rotated servers: {rotated_servers_list}")

                # backup config.ini 
                shutil.move(f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini", f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini.bak")
                print(f"Backup {CLIENT_ROOTPATH}/{exp1_method}/config.ini into {CLIENT_ROOTPATH}/{exp1_method}/config.ini.bak, which will be resumed by test/stop_server_rotation.sh")

                # Generate new config.ini 
                shutil.copyfile(f"{CLIENT_ROOTPATH}/{exp1_method}/configs/config.ini.dynamic", f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini")
                print(f"Generate new {CLIENT_ROOTPATH}/{exp1_method}/config.ini based on {CLIENT_ROOTPATH}/{exp1_method}/configs/config.ini.dynamic to prepare for server rotation")
                # dynamic_periodinterval = 10
                # if exp1_method == "distcache":
                #     dynamic_periodinterval =100 
                # elif exp1_method == "distnocache":
                #     dynamic_periodinterval = 80
                rules = [
                    ('global','workload_name',str(exp1_workload)),
                    ('global','server_total_logical_num',str(exp1_server_scale_total)),
                    ('global','server_physical_num',str(exp1_physical_server_scale)),
                    ('global','server_total_logical_num_for_rotation',str(exp1_server_scale_total)),
                    ('global','bottleneck_serveridx_for_rotation','0'),
                    ('global','workload_mode','0'),
                    ('global','dynamic_periodinterval',str(dynamic_periodinterval)),
                    # ('global','client_physical_num','1'),
                    # ('global','client_total_logical_num','2'),
                ]
                for i in range(exp1_physical_server_scale):
                    rules.append((f'server{i}','server_logical_idxes',rotated_servers_list[i]))
                
                rules.append((f'switch','switch_kv_bucket_num','10000'))
                if exp1_method == "distreach" or exp1_method == "distcache" :
                    for i in range(8):
                        rules.append((f'switch{i}','switch_kv_bucket_num','10000'))
                if exp1_method == "distreach":
                    rules.append(('controller','controller_snapshot_period','0'))
                    # 
                gennewconfig(
                    f"{CLIENT_ROOTPATH}/{exp1_method}/configs/config.ini.dynamic",
                    f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini",
                    rules,
                )

                print(f"[exp1][{exp1_method}][{exp1_workload}] start switchos")

                if exp1_physical_server_scale > 2 and exp1_method =="distreach":
                    network_creator = 'network_multi'
                else:
                    network_creator = 'network'
                # start mininet
                print(network_creator,exp1_physical_server_scale)
                network_process = subprocess.Popen(
                    ["python", f"{SWITCH_ROOTPATH}/{exp1_method}/leafswitch/{network_creator}.py"]
                )
                sleep(10)
                # Use the start of simple_switch to monitor the start of mininet
                while True:
                    # check if switch start
                    process = subprocess.Popen(
                        "ps -a | grep simple_switch",
                        shell=True,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE,
                    )
                    stdout, stderr = process.communicate()
                    sleep(2)
                    if stdout:
                        break
                sleep(10)
                os.system(f"cd {SWITCH_ROOTPATH}/{exp1_method}; bash localscriptsbmv2/launchswitchostestbed.sh")

                ### Evaluation
                # test server rotation
                ##### Part 0 #####
                print("launch servers")
                os.system(f"cd {SERVER_ROOTPATH}/{exp1_method}; bash startserver.sh; sleep 10s;")
                if commonconfig.with_controller == True:
                    print("pre-admit hot keys")
                    os.system(f"cd {SERVER_ROOTPATH}/{exp1_method}; mx h1 ./warmup_client; sleep 10s;")
                    # Use the file size to monitor the end of the warm up phase
                    file_size = -1
                    while True:
                        sleep(10)
                        current_size = os.path.getsize(f"{SERVER_ROOTPATH}/{exp1_method}/leafswitch/ptf_popserver0.out")
                        if current_size != file_size:
                            file_size = current_size
                        else:
                            sleep(5)
                            break


                print(f"launch clients of {exp1_method}")
                benchmark_dir = f"{SERVER_ROOTPATH}/benchmarkdist/ycsb"

                # ycsb
                shutil.move(f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini",f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini.bak")
                shutil.copyfile(f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini",f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini")
                rules=[
                    ('global','client_physical_num','1'),
                    ('global','client_total_logical_num','512'),
                    ('client0','client_logical_num','512'),
                    ('client1','client_logical_num','512')
                ]

                gennewconfig(
                    f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini",
                    f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini",
                    rules,
                )
                # command_client1 = f"cd {benchmark_dir};mx h2 python2 bin/ycsb run {exp1_method_dic[exp1_method]} -pi 1 > {benchmark_dir}/tmp_client1.out 2>&1 &"
                # subprocess.Popen(command_client1, shell=True)
                # sleep(10)
                command_client0 = f"cd {benchmark_dir};mx h1 python2 bin/ycsb run {exp1_method_dic[exp1_method]} -pi 0 > {benchmark_dir}/tmp_client0.out 2>&1"
                subprocess.run(command_client0, shell=True)

                os.system('sync')
                sleep(1)
                exp1_output_path=f"{EVALUATION_OUTPUT_PREFIX}/exp_rack/{args.roundidx}"
                ### Cleanup
                if os.path.exists(f"{CLIENT_ROOTPATH}/benchmarkdist/ycsb/tmp_client0.out"):
                    shutil.move(f"{CLIENT_ROOTPATH}/benchmarkdist/ycsb/tmp_client0.out",
                                f"{exp1_output_path}/{exp1_workload}-{exp1_method}-static{exp1_server_scale_total}-{exp1_physical_server_scale}-client0.out.txt")
                else:
                    print(f"[ERROR]{CLIENT_ROOTPATH}/benchmarkdist/ycsb/tmp_client0.out is not exist check tmp_client.out")
                if os.path.exists(f"{CLIENT_ROOTPATH}/benchmarkdist/output/{exp1_workload}-statistics/{exp1_method_dic[exp1_method]}-static{exp1_server_scale_total}-client0.out"):  
                    shutil.move(f"{CLIENT_ROOTPATH}/benchmarkdist/output/{exp1_workload}-statistics/{exp1_method_dic[exp1_method]}-static{exp1_server_scale_total}-client0.out",
                                f"{exp1_output_path}/{exp1_workload}-{exp1_method}-static{exp1_server_scale_total}-{exp1_physical_server_scale}-client0.out")
                else:
                    print(f"[ERROR]{CLIENT_ROOTPATH}/benchmarkdist/output/{exp1_workload}-statistics/{exp1_method_dic[exp1_method]}-static{exp1_server_scale_total}-client0.out is not exist check tmp_client.out")
                    
                # shutil.move(f"{CLIENT_ROOTPATH}/benchmarkdist/output/{exp1_workload}-statistics/{exp1_method_dic[exp1_method]}-static{exp1_server_scale_total}-client0.out",
                #                 f"{exp1_output_path}/{exp1_workload}-{exp1_method}-static{exp1_server_scale_total}-{exp1_physical_server_scale}-client0.out")
                # if exp1_method == "distreach":
                # sleep(60)
                # for idx in range(1,5):
                #     # print("idx = ",idx)
                    
                #     subprocess.run(command_client0, shell=True)
                #     os.system('sync')
                #     sleep(1)
                #     ### Cleanup
                #     exp1_output_path=f"{EVALUATION_OUTPUT_PREFIX}/exp_rack/{args.roundidx + idx}"
                #     os.makedirs(exp1_output_path, exist_ok=True)
                #     shutil.move(f"{CLIENT_ROOTPATH}/benchmarkdist/output/{exp1_workload}-statistics/{exp1_method_dic[exp1_method]}-static{exp1_server_scale_total}-client0.out",
                #                     f"{exp1_output_path}/{exp1_workload}-{exp1_method}-static{exp1_server_scale_total}-{exp1_physical_server_scale}-client0.out")
                #     print(f"{exp1_output_path}/{exp1_workload}-{exp1_method}-static{exp1_server_scale_total}-{exp1_physical_server_scale}-client0.out")
                #     sleep(60)

                shutil.move(f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini.bak",f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini")
                        
                print(f"stop storage servers of {exp1_method}")
                os.system(f"cd {SWITCH_ROOTPATH}; bash scriptsdist/remote/stopservertestbed.sh")
                print(f"[exp1][{exp1_method}] stop switchos")
                os.system(f"cd {SWITCH_ROOTPATH}/{exp1_method}; bash localscriptsbmv2/stopswitchtestbed.sh")
                
                network_process.kill()
