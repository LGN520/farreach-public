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

# "distnocache", "distreach", 
exp1_method_list=["distcache"]
exp1_method_dic = {
    "distreach": "farreach",
    "distcache": "netcache",
    "distnocache": "nocache",
}
exp1_core_workload_list=workload_list = [
    # "workloadf",
    # "uniform",
    # "skewness-95",
    # "skewness-90",
    # "synthetic",
    # "synthetic-25",
    # "synthetic-75",
    "workloada",
    "workloadb",
    "workloadc",
    "workloadd",
    "workload-load",
]
# exp1_physical_server_scale=8
exp1_server_scale_total = 128 
exp1_server_scale=16
exp1_output_path=f"{EVALUATION_OUTPUT_PREFIX}/exp_rack/{args.roundidx}"

### Create json backup directory
os.makedirs(exp1_output_path, exist_ok=True)

for exp1_method in exp1_method_list:
    for exp1_workload in exp1_core_workload_list:
        for exp1_physical_server_scale in [2,4,6,8]:
            exp1_server_scale = int(exp1_server_scale_total / exp1_physical_server_scale)
            # if (
            #     # (exp1_workload == "workloadb" and exp1_method== "distreach" and exp1_physical_server_scale== 6)
            #     # or (exp1_workload == "workloadc" and exp1_method== "distreach" and exp1_physical_server_scale== 8)
            #     # (exp1_workload == "workloadc" and exp1_method== "distcache" and exp1_physical_server_scale== 6)
            #     # or (exp1_workload == "workloada" and exp1_method== "distcache" and exp1_physical_server_scale== 8)
            #     # or (exp1_workload == "workloadd" and exp1_method== "distcache" and exp1_physical_server_scale== 8)
            #     # or (exp1_workload == "workloadb" and exp1_method== "distreach" and exp1_physical_server_scale== 8)
            #     # or (exp1_workload == "skewness-95" and exp1_method== "distreach" and exp1_physical_server_scale== 6)
            #     # or (exp1_workload == "uniform" and exp1_method== "distreach" and exp1_physical_server_scale== 6)
            #     # or (exp1_workload == "uniform" and exp1_method== "distreach" and exp1_physical_server_scale== 4)
            #     (exp1_workload == "workloadd" and exp1_method== "distreach" and exp1_physical_server_scale== 8)
            #     # or (exp1_method == "distnocache" and exp1_workload != 'uniform' and exp1_workload != 'skewness-95')
            # ):
            #     # do it
            #     print('lets do it')
            # else:
            #     continue
            
            print(f"[exp1][{exp1_method}][{exp1_workload}]")
            print(f"[exp1][{exp1_method}][{exp1_workload}] run workload with {exp1_workload}servers")
            ### Preparation
            os.system(f"bash {SWITCH_ROOTPATH}/{exp1_method}/localscriptsbmv2/stopswitchtestbed.sh")
            print(f"[exp1][{exp1_method}][{exp1_workload}] update {exp1_method} config with {exp1_workload}")

            rules = [
                ('global','workload_name',str(exp1_workload)),
                ('global','server_total_logical_num',str(exp1_server_scale)),
                ('global','server_total_logical_num_for_rotation',str(exp1_physical_server_scale * exp1_server_scale)),
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

            rules = [
                ('global','workload_name',str(exp1_workload)),
                ('global','server_total_logical_num',str(exp1_physical_server_scale * exp1_server_scale)),
                ('global','server_physical_num',str(exp1_physical_server_scale)),
                ('global','server_total_logical_num_for_rotation',str(exp1_physical_server_scale * exp1_server_scale)),
                ('global','bottleneck_serveridx_for_rotation','0'),
                ('global','workload_mode','0'),
                ('global','dynamic_periodinterval','60'),
                # ('global','client_physical_num','1'),
                # ('global','client_total_logical_num','2'),
            ]
            for i in range(exp1_physical_server_scale):
                rules.append((f'server{i}','server_logical_idxes',rotated_servers_list[i]))
            
            rules.append((f'switch','switch_kv_bucket_num','10000'))
            if exp1_method == "distreach" or exp1_method == "distcache" :
                for i in range(4):
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
            sleep(5)
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
                ('global','client_total_logical_num','2'),
                ('client0','client_logical_num','2'),
                ('client1','client_logical_num','2')
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
            ### Cleanup
            shutil.move(f"{CLIENT_ROOTPATH}/benchmarkdist/output/{exp1_workload}-statistics/{exp1_method_dic[exp1_method]}-static{exp1_physical_server_scale * exp1_server_scale}-client0.out",
                            f"{exp1_output_path}/{exp1_workload}-{exp1_method}-static{exp1_physical_server_scale * exp1_server_scale}-{exp1_physical_server_scale}-client0.out")
            # shutil.move(f"{CLIENT_ROOTPATH}/benchmarkdist/output/{exp1_workload}-statistics/{exp1_method_dic[exp1_method]}-static{exp1_physical_server_scale * exp1_server_scale}-client1.out",
            #                 f"{exp1_output_path}/{exp1_workload}-{exp1_method}-static{exp1_physical_server_scale * exp1_server_scale}-{exp1_physical_server_scale}-client1.out")

            


            shutil.move(f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini.bak",f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini")
                    
            print(f"stop storage servers of {exp1_method}")
            os.system(f"cd {SWITCH_ROOTPATH}; bash scriptsdist/remote/stopservertestbed.sh")
            print(f"[exp1][{exp1_method}] stop switchos")
            os.system(f"cd {SWITCH_ROOTPATH}/{exp1_method}; bash localscriptsbmv2/stopswitchtestbed.sh")
            
            network_process.kill()
