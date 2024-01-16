#!/usr/bin/env python3
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

workload_list = [
    "uniform",
    "skewness-95",
    "skewness-90",
    "synthetic",
    "synthetic-25",
    "synthetic-75",
    "workloada",
    "workloadb",
    "workloadc",
    "workloadd",
    "workloadf",
    "workload-load",
]

for workload in workload_list:

    file_path = f'{CLIENT_ROOTPATH}/keydump/config.ini'
    config = configparser.ConfigParser()
    config.read(file_path)
    config.set('global', 'workload_name', workload)
    with open(file_path, 'w') as config_file:
        config.write(config_file)

    cmd = ''
    if workload == "workload-load":
        cmd = f'cd {CLIENT_ROOTPATH}/benchmarkdist/ycsb; python2 ./bin/ycsb load keydump'
    else:
        cmd = f'cd {CLIENT_ROOTPATH}/benchmarkdist/ycsb; python2 ./bin/ycsb run keydump'
        
    os.system(cmd)
# gen rule
    cmd = f'cd {CLIENT_ROOTPATH}/benchmarkdist/ycsb; python2 generate_dynamicrules.py {workload}'
    os.system(cmd)

