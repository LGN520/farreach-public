# README of DistReach

- If you have any question, please contact us if available (sysheng21@cse.cuhk.edu.hk).

# Table of Contents

0. [Overview](#0-overview)
	1. [Methods](#01-methods)
	2. [Our Testbed](#02-our-testbed)
1. [System Preparation](#1-system-preparation)
	1. [Dependency Installation](#11-dependency-installation)
	2. [Configuration settings](#12-configuration-settings)
	3. [Code Compilation](#13-code-compilation)
	4. [Testbed Building](#14-testbed-building)
2. [Data Preparation](#2-data-preparation)
	1. [Loading Phase](#21-loading-phase)
	2. [Workload Analysis & Dump Keys](#22-workload-analysis--dump-keys)
3. [Running Experiments](#3-running-experiments)
	1. [Script Usage](#31-script-usage)




# Contents

## 0 Overview

### 0.1 Methods

- farreach: our in-switch write-back caching
- nocache: a baseline without in-switch caching
- netcache: a baseline with in-switch write-through caching

### 0.2 Our Testbed

- You can follow our settings below to build your own testbed

</br>

- Machine requirements
	+ One physical machines with Ubuntu 20.04

</br>

- Network configuration and topology(in mininet)
	+ you can refer to {method}/configs/config.ini.dynamic for more details. If you want to conduct a larger scale (more than 8 racks), you need to modify these files first.
	+ All clients/servers/switch are in the same local area network (NOT bypass {switch} data plane)
		* Main client: 192.168.1.1
		* Secondary client: 192.168.1.2
		* First server (co-located with controller): 192.168.1.3
		* Second server: 192.168.1.4
		* ...... (multiple servers and controllers)
		* Bmv2 switch OS: 192.168.1.201(as Bmv2 only simulate the data plane, we need to use an extra virtual node to play the role of switchos)
		* ......(multiple switch OS)
	+ bypass {switch} data plane
		* Main client: 10.0.1.1
		* Secondary client: 10.0.1.2
		* First server: 10.0.1.3
		* Second server: 10.0.1.4
		* ......(multiple servers)
	+ Testbed topology of programmable-switch-based network (bypass {switch} data plane)
		* Main client (NIC: h1-eth0; MAC: 00:00:0a:00:01:01) <-> Bmv2 switch (front panel port: 1/0)
		* Secondary client (NIC: h2-eth0; MAC: 00:00:0a:00:01:02) <-> Bmv2 switch (front panel port: 2/0)
		* First server (NIC: h3-eth0; MAC: 00:00:0a:00:01:03) <-> Bmv2 switch (front panel port: 3/0)
		* Second server (NIC: h4-eth0; MAC: 00:00:0a:00:01:04) <-> Bmv2 switch (front panel port: 4/0)
		* Bmv2 doesn't need in-switch cross-pipeline recirculation

# 1 System Preparation

- **Note: system preparation has already been done in our AE testbed, so AEC members do NOT need to re-execute the following steps.**

## 1.1 Dependency Installation
+ Python 3.8.10
- Install python libraries for python 2.7.12 if not
	+ `pip install -r requirements.txt`
	+ Make sure your 'python' alias to 'python3', if you want to use python2, please enter 'python2'
- Install libboost 1.81.0 if not
	+ Under project directory (e.g., /home/ssy/projects/farreach-public)
	```
	wget https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.tar.gz
	tar -xzvf boost_1_81_0.tar.gz
	cd boost_1_81_0; ./bootstrap.sh --with-libraries=system,thread --prefix=./install && sudo ./b2 install
	```
	<!-- + `sudo apt-get install libboost-all-dev` -->
- Install Maven 3.3.9 in {main client} and {secondary client} if not 
- Install Java OpenJDK-8 in {main client} and {secondary client} if not
	+ **Configure JAVA_HOME**: add sth like `export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64/` (based on your own JAVA path) to ~/.bashrc
- Install gcc-7.5.0 and g++-7.5.0 in two clients and two servers if not (NO need for {switch})
- Use RocksDB 6.22.1 (already embedded in the project; NO need for extra installation)

- Prepare the P4 enviroment
	+ Dependencies

	+ Before installing bmv2 and PI, you also need the following modules as dependencies `bmv2_deps`

	+ To run our code correctly, you need to make a tiny change to bmv2's source code before compiling it.
		+ At targets/simple_switch/primitives.cpp#L231, comment `hdr.reset();`

| Dependencies repo  | Version |
| --- | --- |
| [protobuf](https://github.com/protocolbuffers/protobuf) | 3.18 |
| [grpc](https://github.com/grpc/grpc) |  1.43 |
| [PI](https://github.com/p4lang/PI) | v0.1 |
| [bmv2](https://github.com/p4lang/behavioral-model) | 1.15 |
| [p4c](https://github.com/p4lang/p4c) | Latest |
| [mininet](https://github.com/mininet/mininet)  | Latest |

| bmv2_deps repo | Version |
| --- | --- | 
| nnpy | pip install nnpy |
| [nanomsg](https://github.com/nanomsg/nanomsg/releases/tag/1.) | 1.0.0 |
| [thrift](https://github.com/apache/thrift/releases/tag/0.11.0) | 0.11.0 |

## 1.2 Configuration Settings
- It is best not to change our configuration files at will. The experimental script will automatically modify part of them at runtime.
	+ **note: if you want to change the ip in mininet, you need to change {method}/configs/config.ini.dynamic and {method}/leafswitch/network.py**
	+ **if you want to change the port used in softwares, you need to change {method}/configs/config.ini.dynamic**
	+ **if you want to change some parameters like thread nums, you need to change {method}/configs/config.ini.dynamic and the experiments scripts**

## 1.3 Code Compilation

- Compile RocksDB (**ONLY need to perform once**)
	+ compile RocksDB
		+ Run `sudo apt-get install libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev libjemalloc-dev libsnappy-dev` to install necessary dependencies for RocksDB
		+ `cd rocksdb-6.22.1`
		+ `PORTABLE=1 make static_lib`
			* We already comment -Wstrict-prototype and -Werror in RocksDB's Makefile to fix compilation errors due to strict-alias warning
			* We use PORTABLE=1 to fix runtime error of illegal instruction when open()
		+ For each method (farreach or nocache or netcache)
			* Run `mkdir /tmp/{method}` to prepare the directory (e.g., /tmp/farreach) for database in advance
			* use /tmp/farreach for distreach, /tmp/netcache for distcache, /tmp/nocache for distnocache
</br>

- Compile source code for all methods
	- For each {method}, under {switch}
		- Run `su` to enter root mode
		- Run `cd {method}; bash compile.sh` to make P4 code 
			+ If you have compiled P4 code of {method} before, you do **NOT need to re-compile it again**


# 2 Data Preparation

- **Note: data preparation has already been done in our AE testbed, so AEC members do NOT need to re-execute the following steps.**

## 2.1 Loading Phase

- Perform the loading phase and backup for evaluation time reduction (**TIME: around 40 minutes**; **ONLY need to perform once**)
	+ Dataset(100M records) is approximately 16GB
	+ Modify these options in `recordload/config.ini`
		- \[server0\]: server_ip:127.0.0.1
		- \[client0\]: client_ip:127.0.0.1
	+ load data
		- `mkdir /tmp/nocache`
		- `cd nocache`
		- `./server 0 &`
		- `cd ../benchmark/ycsb`
		- `python2 ./bin/ycsb run recordload`
		- `mkdir /tmp/rocksdbbackups`
		- `cp -r /tmp/nocache/* /tmp/rocksdbbackups`
	+ Stop everything
		- `killall python server controller switchos simple_switch ovs-testcontroller`

## 2.2 Workload Analysis & Dump Keys

- Analyze workloads to generate workload-related information before evaluation (**ONLY need to perform once**)
	- Workload-related information includes:
		- Dump hot keys and per-client-thread pregenerated workloads (independent with server rotation scale (i.e., # of simulated servers))
		- Generate key poularity change rules for dynamic patterns (independent with server rotation scale)
		- Calculate bottleneck partitions for different server rotation scales (i.e., 16, 32, 64, and 128)
	- For each {workload}
		- Options of {workload}
			- YCSB core workloads: workloada, workloadb, workloadc, workloadd, workloadf, workload-load
			- Synthetic workloads: synthetic (i.e., 100% writes, 0.99 skewness, 128B valuesize), synthetic-25, synthetic-75, skewness-90, skewness-95, uniform, valuesize-16, valuesize-32, valuesize-64
			- Note: NO need to analyze the following workloads due to duplication
				- synthetic-0 is the same as workloadc
				- synthetic-50 is the same as workloada
				- synthetic-100, skewness-99, and valuesize-128 are the same as synthetic
		- TIME cost of workload analysis
			- **Each of most workloads needs around 5 minutes** (including workloada, workloadb, workloadc, workloadd, workloadf, workload-load, synthetic, valuesize-16, valuesize-32, and valuesize-64)
			- **Each of other workloads needs 20 minutes to 40 minutes** (including skewness-90 of 20m, skewness-95 of 40m, and uniform of 30m)
				- The reason is that YCSB uses Zipfian key generator with small skewness and uniform key generator for these workloads, which incurs large computation overhead
				- Although workload-load is also uniform distribution, YCSB uses counter generator which has small computation overhead
		- Under host machine
			- To complete this step quickly, you do not need to perform it on the mininet virtual node. You can perform it directly on the host machine.
			- Update workload\_name as {workload} in `keydump/config.ini`
				- Note: you do NOT need to change configurations related with server rotation scales (i.e., server\_total\_logical\_num and server\_total\_logical\_num\_for_rotation) in `keydump/config.ini`, which is NOT used here
			- Update {max_load_batch_size} and {load_batch_size} in `keydump/config.ini`. (e.g 40000 for 4 per-layer switches * 10000 records in cache) 
			- Run `python scriptsdist/keydump_for_all.py` to dump workload-related information (e.g., hot keys and bottleneck serveridx)
  

# 3 Running Experiments 

- **Note: if you want to add any new experiment script by youself in `scriptsdist/exps/`, the script file name should NOT include the reserved strings (ycsb, server, controller, reflector, and server_rotation); otherwise, the new experiment script may be killed by itself during evaluation.**

## 3.1 Script Usage

- To reproduce experiments in our evaluation, we provide the following scripts under `scriptsdist/exps/`
	+ we do not use server rotation for dist-method
	+ A workload for a method in a round takes about 40-60 min.
	+ **Make sure that you use `screen` or `nohup` to run each script in the background, otherwise the script will be killed by OS after you log out from the ssh connection**
	+ Note: **before and after each experiment, run `killall python server controller switchos simple_switch ovs-testcontroller` to forcely kill all processes to avoid confliction**

</br>

- Note: **if you encouter many timeouts when issuing in main client**, your servers may fail to be launched (e.g., due to code compilation with incorrect serve rotation enabling/disabling) or may be launched with a large delay (e.g., due to limited server-side power in your testbed to load database files of 100M records pre-loaded before), therefore the client-issued requests are not answered and hence have timeouts.

</br>
- Note: if you encouter any other problem, you can **keep the critical information and contact us if available (sysheng21@cse.cuhk.edu.hk) for help**
	- The causes of problem may be testbed mis-configuration, script mis-usage, resource confliction, bmv2 hardware bugs, and our code bugs
	- The critical information should include: command history of terminal, dumped information of scripts, log files generated by scripts(e.g., `{method}/tmp\_\*.out` in servers and switch, `benchmark/ycsb/tmp\_\*.out` in clients, and `{method}/Bmv2/tmp\_\*.out` in switch), and raw statistics generated by YCSB clients (e.g., `benchmark/ycsb/{method}-statistics/`)

</br>

- Scripts for different experiments
- you could use scriptsdist/exps/run_exp_rack_num.py to run static pattern experiments and scriptsdist/exps/run_exp_rack_num_dynamic.py to run dynamic pattern experiments
	- you could modify variable in the 2 py scripts to run what you want

	- scriptsdist/exps/run_exp_rack_num.py
   | Variable  | Description |
   | :---:  | --- 					|
	|exp1_core_workload_list| the workload you want to run		|
	|exp1_server_scale_total| the sum of servers		| 
	|client_logical_num| the number of logical clients			|
	|scale_list|	e.g.[16,8,2] the number of server nodes (and it also determines racks, 16 server nodes need 8 racks)|
	|dynamic_periodintervals|e.g. [10,5000]	clients' running time	| 

|#Exp|exp1_core_workload_list|scale_list|client_logical_num|exp1_server_scale_total|
|---|---|---|---|---|
|10 Performance analysis under multiple switches.|["workloada", "workloadb","workloadc", "workloadf", "workloadd","workload-load",]|[4]|512|16|
|11 Impact of write ratio under multiple switches.|["skewness-95", "skewness-90","synthetic","uniform"]|[4]|512|16|
|12 Impact of key distribution under multiple switches|["synthetic","synthetic-25","synthetic-75","workloada", "workloadc"]|[4]|512|16|
|13 Impact of per-layer switch number|["synthetic"]|[2,4,8,16]|512|16|

	- scriptsdist/exps/run_exp_rack_num_dynamic.py
		- basicly the same as scriptsdist/exps/run_exp_rack_num.py
		- different variable exp1_dynamic_rule_list ["hotin","hotout","random"]