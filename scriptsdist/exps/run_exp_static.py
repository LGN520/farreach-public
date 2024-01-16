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

from utils.serverlist import find_server_index, genserverlist

current_path = os.path.dirname(os.path.abspath(__file__))
root_path = os.path.dirname(current_path)
sys.path.append(root_path)

from config.common import *
from utils.genconfig import genconfig, gennewconfig

parser = argparse.ArgumentParser(description="exp_dynamic")
parser.add_argument("roundidx", type=int, help="an integer for the roundidx")
args = parser.parse_args()

is_bmv2 = 1

# "netcache" "nocache" 
exp1_method_list=["distreach", "distcache", "distnocache"]
exp1_method_dic = {
    "distreach": "farreach",
    "distcache": "netcache",
    "distnocache": "nocache",
}
exp1_core_workload_list=["workloada","workloadb", "workloadc", "workloadd", "workloadf", "workload-load"]
exp1_physical_server_scale=8
exp1_server_scale=16
exp1_output_path=f"{EVALUATION_OUTPUT_PREFIX}/exp1/{args.roundidx}"

### Create json backup directory
os.makedirs(exp1_output_path, exist_ok=True)


for exp1_method in exp1_method_list:
    for exp1_workload in exp1_core_workload_list:
        print(f"[exp1][{exp1_method}][{exp1_workload}]")
        print(f"[exp1][{exp1_method}][{exp1_workload}] run workload with {exp1_workload}servers")
        ### Preparation
        os.system(f"bash {SWITCH_ROOTPATH}/{exp1_method}/localscriptsbmv2/stopswitchtestbed.sh")
        print(f"[exp1][{exp1_method}][{exp1_workload}] update {exp1_method} config with {exp1_workload}")

        rules = [
            (r"workload_name=.*", f"workload_name={exp1_workload}"),
            (
                r"server_total_logical_num=.*",
                f"server_total_logical_num={exp1_server_scale}",
            ),
            (
                r"server_total_logical_num_for_rotation=.*",
                f"server_total_logical_num_for_rotation={exp1_server_scale}",
            ),
            (r"controller_snapshot_period=.*", f"controller_snapshot_period=10000"),
            (r"switch_kv_bucket_num=.*", f"switch_kv_bucket_num=10000"),
        ]
        if exp1_workload == "workload-load":
            rules.append((r"bottleneck_serveridx_for_rotation=.*",f"server_total_logical_num_for_rotation=0"))
        elif exp1_workload == "workloadd":
            rules.append((r"bottleneck_serveridx_for_rotation=.*",f"server_total_logical_num_for_rotation=0"))
        else:
            rules.append((r"bottleneck_serveridx_for_rotation=.*",f"server_total_logical_num_for_rotation=0"))
        genconfig(
            f"{CLIENT_ROOTPATH}/{exp1_method}/configs/config.ini.static",
            f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini",
            rules,
        )
        commonconfig = CommonConfig(exp1_method)
        
        print(f"[exp1][{exp1_method}][{exp1_workload}] prepare server rotation")

        print("prepare server rotation")
        rotated_servers_list = genserverlist(commonconfig.server_total_logical_num_for_rotation)
        print(f"Rotated servers: {rotated_servers_list}")

        # backup config.ini 
        shutil.move(f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini", f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini.bak")
        print(f"Backup {CLIENT_ROOTPATH}/{exp1_method}/config.ini into {CLIENT_ROOTPATH}/{exp1_method}/config.ini.bak, which will be resumed by test/stop_server_rotation.sh")

        # Generate new config.ini 
        shutil.copyfile(f"{CLIENT_ROOTPATH}/{exp1_method}/configs/config.ini.static.setup", f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini")
        print(f"Generate new {CLIENT_ROOTPATH}/{exp1_method}/config.ini based on {CLIENT_ROOTPATH}/{exp1_method}/configs/config.ini.static.setup to prepare for server rotation")

        rules = [
            ('global','workload_name',exp1_workload),
            ('global','server_total_logical_num',exp1_server_scale),
            ('global','server_total_logical_num_for_rotation',commonconfig.server_total_logical_num_for_rotation),
            ('global','bottleneck_serveridx_for_rotation',commonconfig.bottleneck_serveridx),
        ]
        for i in range(exp1_physical_server_scale):
            rules.append((f'server{i}','server_logical_idxes',rotated_servers_list[i]))

        if exp1_method == "distreach":
            rules.append(('controller','controller_snapshot_period','10000'))
            rules.append(('switch','switch_kv_bucket_num','10000'))
        gennewconfig(
            f"{CLIENT_ROOTPATH}/{exp1_method}/configs/config.ini.static.setup",
            f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini",
            rules,
        )

        print(f"[exp1][{exp1_method}][{exp1_workload}] start switchos")

        # start mininet
        network_process = subprocess.Popen(
            ["python", f"{SWITCH_ROOTPATH}/{exp1_method}/leafswitch/network.py"]
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
        print(f"stop storage servers of {exp1_method}")
        os.system(f"cd {SWITCH_ROOTPATH}; bash scriptsdist/remote/stopservertestbed.sh")
        
        ##### Part 1 #####
        print("[part 1] run single bottleneck server thread")
        if exp1_method == "distreach":
            os.system(f"cd {SWITCH_ROOTPATH}/{exp1_method}/leafswitch; bash cleanup_obselete_snapshottoken.sh >tmp_cleanup.out 2>&1")
        # test_server_rotation_p1 0 
        print("prepare and sync test_server_rotation_p1 phase's config.ini")
        rules=[
            ('global','workload_name',exp1_workload),
            ('global','server_total_logical_num_for_rotation',commonconfig.server_total_logical_num_for_rotation),
            ('global','bottleneck_serveridx_for_rotation',commonconfig.bottleneck_serveridx),
            ('server0','server_logical_idxes',commonconfig.bottleneck_serveridx),
        ]
        if exp1_method == "distreach":
            rules.append(('controller','controller_snapshot_period','10000'))
            rules.append(('switch','switch_kv_bucket_num','10000'))
        gennewconfig(
            f"{CLIENT_ROOTPATH}/{exp1_method}/configs/config.ini.static.1p.bmv2",
            f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini",
            rules,
        )
        print("start servers")
        command_server0=f"cd {SERVER_ROOTPATH}/{exp1_method}; mx h3  ./server 0 >tmp_serverrotation_part1_server0.out 2>&1"
        subprocess.Popen(command_server0, shell=True)
        sleep(30)
        if commonconfig.with_controller:
            print("start controller")
            command_controller0=f"cd {SERVER_ROOTPATH}/{exp1_method}; mx h3 ./controller 0> tmp_serverrotation_part1_controller0.out 2>&1"
            subprocess.Popen(command_controller0, shell=True)
            sleep(10)

        # Start client 
        print(f"launch clients of {exp1_method}")
        benchmark_dir = f"{SERVER_ROOTPATH}/benchmark/ycsb"
        # ycsb
        shutil.move(f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini",f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini.bak")
        shutil.copyfile(f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini",f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini")
        command_client1 = f"cd {benchmark_dir};mx h2 python2 bin/ycsb run {exp1_method_dic[exp1_method]} -pi 1 -sr 0 > {benchmark_dir}/tmp_client1.out 2>&1 &"
        subprocess.Popen(command_client1, shell=True)
        sleep(10)
        command_client0 = f"cd {benchmark_dir};mx h1 python2 bin/ycsb run {exp1_method_dic[exp1_method]} -pi 0 -sr 0 > {benchmark_dir}/tmp_client0.out 2>&1"
        subprocess.run(command_client0, shell=True)
        shutil.move(f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini.bak",f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini")
        
        # stop server
        print(f"stop storage servers of {exp1_method}")
        os.system(f"cd {SWITCH_ROOTPATH}; bash scriptsdist/remote/stopservertestbed.sh")
                
        ##### Part 2 #####
        print("[part 2] run bottleneck server thread + rotated server thread")
        for rotateidx in range(commonconfig.server_total_logical_num_for_rotation):
            physical_rotateidx = -1
            if rotateidx == commonconfig.bottleneck_serveridx:
                continue
            else:
                physical_rotateidx = find_server_index(rotated_servers_list, rotateidx)
            if exp1_method == "distreach":
                subprocess.run(
                    f"cd {SWITCH_ROOTPATH}/{exp1_method}/leafswitch; bash cleanup_obselete_snapshottoken.sh >> tmp_cleanup.out 2>&1",
                    shell=True
                )
            print("retrieve bottleneck partition back to the state after loading phase")
            os.system(f"rm -r /tmp/{exp1_method}/worker*snapshot*; rm -r /tmp/{exp1_method}/controller*snapshot*")
            # test_server_rotation_p2.sh 0 {rotateidx}"

            # Start client 
            print(f"launch clients of {exp1_method}")
            benchmark_dir = f"{SERVER_ROOTPATH}/benchmark/ycsb"
            # ycsb
            shutil.move(f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini",f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini.bak")
            shutil.copyfile(f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini",f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini")
            command_client1 = f"cd {benchmark_dir};mx h2 python2 bin/ycsb run {exp1_method_dic[exp1_method]} -pi 1 -sr 0 > {benchmark_dir}/tmp_client1.out 2>&1 &"
            subprocess.Popen(command_client1, shell=True)
            sleep(10)
            command_client0 = f"cd {benchmark_dir};mx h1 python2 bin/ycsb run {exp1_method_dic[exp1_method]} -pi 0 -sr 0 > {benchmark_dir}/tmp_client0.out 2>&1"
            subprocess.run(command_client0, shell=True)
            shutil.move(f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini.bak",f"{CLIENT_ROOTPATH}/{exp1_method_dic[exp1_method]}/config.ini")
        
            print(f"Total rotations: {rotateidx}")

            print(f"stop storage servers of {exp1_method}")
            os.system(f"cd {SWITCH_ROOTPATH}; bash scriptsdist/remote/stopservertestbed.sh")

        print(f"Resume ${exp1_method}/config.ini with ${exp1_method}/config.ini.bak if any")
        shutil.move(f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini.bak", f"{CLIENT_ROOTPATH}/{exp1_method}/config.ini")
        
        # stop server rotation
        print(f"stop storage servers of {exp1_method}")
        os.system(f"cd {SWITCH_ROOTPATH}; bash scriptsdist/remote/stopservertestbed.sh")
        print(f"[exp1][{exp1_method}] stop switchos")
        os.system(f"cd {SWITCH_ROOTPATH}/{exp1_method}; bash localscriptsbmv2/stopswitchtestbed.sh")
        

        ### Cleanup
        shutil.copyfile(f"{CLIENT_ROOTPATH}/benchmark/output/{exp1_workload}-statistics/{exp1_method}-static{exp1_server_scale}-client0.out",
                        f"{exp1_output_path}/{exp1_workload}-{exp1_method}-static{exp1_server_scale}-client0.out ")
        shutil.copyfile(f"{CLIENT_ROOTPATH}/benchmark/output/{exp1_workload}-statistics/{exp1_method}-static{exp1_server_scale}-client1.out",
                        f"{exp1_output_path}/{exp1_workload}-{exp1_method}-static{exp1_server_scale}-client1.out ")
    
        
        network_process.kill()



