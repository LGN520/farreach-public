#!/usr/bin/env python3
# run this scripts on host
# exp_dynamic
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
from utils.genconfig import genconfig

parser = argparse.ArgumentParser(description="exp_dynamic")
parser.add_argument("roundidx", type=int, help="an integer for the roundidx")
args = parser.parse_args()

is_bmv2 = 1

exp7_workload = "synthetic"
exp7_server_scale = 8
exp7_server_scale_for_rotation = 16
exp7_server_scale_bottleneck = 3
# "distnocache", , "distcache"
exp7_method_list = ["distreach"]
exp7_method_dic = {
    "distreach": "farreach",
    "distcache": "netcache",
    "distnocache": "nocache",
}
exp7_dynamic_rule_list = ["hotin","hotout","random"]
exp7_output_path = f"{EVALUATION_OUTPUT_PREFIX}/dist_exp7/{args.roundidx}"

os.makedirs(exp7_output_path, exist_ok=True)

for exp7_method in exp7_method_list:
    for exp7_rule in exp7_dynamic_rule_list:
        print(f"[exp7][{exp7_method}][{exp7_rule}]")
        print(f"[exp7][{exp7_method}][{exp7_rule}] run rulemap with {exp7_rule}")
        os.system(f"bash {SWITCH_ROOTPATH}/{exp7_method}/localscriptsbmv2/stopswitchtestbed.sh")
        print(f"[exp7][{exp7_method}][{exp7_rule}] update {exp7_method} config")

        rules = [
            (r"workload_name=.*", f"workload_name={exp7_workload}"),
            (r"workload_mode=.*", f"workload_mode=1"),
            (r"dynamic_ruleprefix=.*", f"dynamic_ruleprefix={exp7_rule}"),
            (
                r"server_total_logical_num=.*",
                f"server_total_logical_num={exp7_server_scale}",
            ),
            (
                r"server_total_logical_num_for_rotation=.*",
                f"server_total_logical_num_for_rotation={exp7_server_scale_for_rotation}",
            ),
            (
                r"bottleneck_serveridx_for_rotation=.*",
                f"bottleneck_serveridx_for_rotation={exp7_server_scale_bottleneck}",
            ),
            (r"controller_snapshot_period=.*", f"controller_snapshot_period=10000"),
            (r"switch_kv_bucket_num=.*", f"switch_kv_bucket_num=10000"),
            (r"server_logical_idxes=TODO0", f"server_logical_idxes=0"),
            (r"server_logical_idxes=TODO1", f"server_logical_idxes=1"),
        ]
        genconfig(
            f"{CLIENT_ROOTPATH}/{exp7_method}/configs/config.ini.dynamic",
            f"{CLIENT_ROOTPATH}/{exp7_method}/config.ini",
            rules,
        )
        commonconfig = CommonConfig(exp7_method)

        print(f"[exp7][{exp7_method}][{exp7_rule}] start switchos")

        # start mininet
        network_process = subprocess.Popen(
            ["python", f"{SWITCH_ROOTPATH}/{exp7_method}/leafswitch/network.py"]
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
        
        os.system(f"cd {SWITCH_ROOTPATH}/{exp7_method}; bash localscriptsbmv2/launchswitchostestbed.sh")
        
        ### Evaluation
        print(f"[exp7][{exp7_method}][{exp7_rule}] test dynamic workload pattern without server rotation")
        # launchservertestbed
        print("clear tmp files in remote servers and controller if any")
        os.system(f"cd {SERVER_ROOTPATH}/{exp7_method}; rm tmp_server*.out;rm tmp_controller*.out;")
        print("launch servers")
        os.system(f"cd {SERVER_ROOTPATH}/{exp7_method}; bash startserver.sh; sleep 10s;")
        # monitor server start
        file_size = -1
        while True:
            sleep(5)
            current_size = os.path.getsize(f"{SERVER_ROOTPATH}/{exp7_method}/tmp_server{exp7_server_scale - 1}.out")
            if current_size != file_size:
                file_size = current_size
            else:
                sleep(5)
                break
        # Use the file size to monitor the end of the warm up phase
        if commonconfig.with_controller == True:
            print("pre-admit hot keys")
            os.system(f"cd {SERVER_ROOTPATH}/{exp7_method}; mx h1 ./warmup_client; sleep 10s;")
            file_size = -1
            while True:
                sleep(10)
                current_size = os.path.getsize(f"{SERVER_ROOTPATH}/{exp7_method}/leafswitch/ptf_popserver0.out")
                if current_size != file_size:
                    file_size = current_size
                else:
                    sleep(5)
                    break

        print(f"launch clients of {exp7_method}")
        benchmark_dir = f"{SERVER_ROOTPATH}/benchmark/ycsb"

        # ycsb
        shutil.move(f"{CLIENT_ROOTPATH}/{exp7_method_dic[exp7_method]}/config.ini",f"{CLIENT_ROOTPATH}/{exp7_method_dic[exp7_method]}/config.ini.bak")
        shutil.copyfile(f"{CLIENT_ROOTPATH}/{exp7_method}/config.ini",f"{CLIENT_ROOTPATH}/{exp7_method_dic[exp7_method]}/config.ini")
        command_client1 = f"cd {benchmark_dir};mx h2 python2 bin/ycsb run {exp7_method_dic[exp7_method]} -pi 1 > {benchmark_dir}/tmp_client1.out 2>&1 &"
        subprocess.Popen(command_client1, shell=True)
        sleep(10)
        command_client0 = f"cd {benchmark_dir};mx h1 python2 bin/ycsb run {exp7_method_dic[exp7_method]} -pi 0 > {benchmark_dir}/tmp_client0.out 2>&1"
        subprocess.run(command_client0, shell=True)

        ### Cleanup
        shutil.copyfile(f"{CLIENT_ROOTPATH}/benchmark/output/{exp7_workload}-statistics/{exp7_method_dic[exp7_method]}-{exp7_rule}-client0.out",
                        f"{exp7_output_path}/{exp7_workload}-{exp7_method}-{exp7_rule}-client0.out")
        shutil.copyfile(f"{CLIENT_ROOTPATH}/benchmark/output/{exp7_workload}-statistics/{exp7_method_dic[exp7_method]}-{exp7_rule}-client1.out",
                        f"{exp7_output_path}/{exp7_workload}-{exp7_method}-{exp7_rule}-client1.out")


        shutil.move(f"{CLIENT_ROOTPATH}/{exp7_method_dic[exp7_method]}/config.ini.bak",f"{CLIENT_ROOTPATH}/{exp7_method_dic[exp7_method]}/config.ini")
                
        print(f"stop storage servers of {exp7_method}")
        os.system(f"cd {SWITCH_ROOTPATH}; bash scriptsdist/remote/stopservertestbed.sh")
        print(f"[exp7][{exp7_method}] stop switchos")
        os.system(f"cd {SWITCH_ROOTPATH}/{exp7_method}; bash localscriptsbmv2/stopswitchtestbed.sh")
        
        network_process.kill()
