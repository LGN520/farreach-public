# README of FarReach(BMV2 implementation)

- **Note: please refer to [ae_instructions.md](./ae_instructions.md) for getting started instructions and detailed instructions.**

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
3. [Running Experiments (Automatic Evaluation)](#3-running-experiments-automatic-evaluation)
	1. [Normal Script Usage](#31-normal-script-usage)
	2. [Perform Single Iteration](#32-perform-single-iteration)
4. [Running Workloads (Manual Evaluation)](#4-running-workloads-manual-evaluation)
	1. [Dynamic Workload (No Server Rotation)](#41-dynamic-workload-no-server-rotation)
	2. [Static Workload (Server Rotation)](#42-static-workload-server-rotation)
5. [Aggregate Statistics (Manual Evaluation)](#5-aggregate-statistics-manual-evaluation)
	1. [Scripts](#51-scripts)
	2. [Usage and Example](#52-usage-and-example)


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
	+ All clients/servers/switch are in the same local area network (NOT bypass {switch} data plane)
		* Main client: 192.168.1.1
		* Secondary client: 192.168.1.2
		* First server (co-located with controller): 192.168.1.3
		* Second server: 192.168.1.4
		* Nat host: 192.168.1.5
		* Bmv2 switch OS: 192.168.1.6(as Bmv2 only simulate the data plane, we need to use an extra virtual node to play the role of switchos)
	+ bypass {switch} data plane
		* Main client: 10.0.1.1
		* Secondary client: 10.0.1.2
		* First server: 10.0.1.3
		* Second server: 10.0.1.4
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

- Update the following configuration settings in scriptsbmv2/global.sh based on your own testbed
	+ Notes: Some variables will be used in tofino experiments, but not in bmv2 experiments, so you only need to change these following variables
	+ `CLIENT/SWITCH/SERVER\_ROOTPATH`: project directory path in clients/switch/servers (e.g., /root/farreach-public)
    + `EVALUATION\_OUTPUT\_PREFIX`: path to store the raw statistics of each experiment for aggregating analysis
	+ Network settings
		* Main client
			- `MAIN_CLIENT_TOSWITCH_IP`: the IP address of NIC for {main client} connecting to {switch}
			- `MAIN_CLIENT_TOSWITCH_MAC`: the MAC address of NIC for {main client} connecting to {switch}
			- `MAIN_CLIENT_TOSWITCH_FPPORT`: the front panel port in {switch} data plane corresponding to {main client}
			- `MAIN_CLIENT_LOCAL_IP`: the local IP address of NIC for {main client} connecting to the local area network (NOT bypass {switch} data plane)
		* Secondary client
			- `SECONDARY_CLIENT_TOSWITCH_IP`: the IP address of NIC for {secondary client} connecting to {switch}
			- `SECONDARY_CLIENT_TOSWITCH_MAC`: the MAC address of NIC for {secondary client} connecting to {switch}
			- `SECONDARY_CLIENT_TOSWITCH_FPPORT`: the front panel port in {switch} data plane corresponding to {secondary client}
			- `SECONDARY_CLIENT_LOCAL_IP`: the local IP address of NIC for {secondary client} connecting to the local area network (NOT bypass {switch} data plane)
		* First server
			- `SERVER0_TOSWITCH_IP`: the IP address of NIC for {first server} connecting to {switch}
			- `SERVER0_TOSWITCH_MAC`: the MAC address of NIC for {first server} connecting to {switch}
			- `SERVER0_TOSWITCH_FPPORT`: the front panel port in {switch} data plane corresponding to {first server}
			- `SERVER0_LOCAL_IP`: the local IP address of NIC for {first server} connecting to the local area network (NOT bypass {switch} data plane)
		* Secondary server
			- `SERVER1_TOSWITCH_IP`: the IP address of NIC for {second server} connecting to {switch}
			- `SERVER1_TOSWITCH_MAC`: the MAC address of NIC for {second server} connecting to {switch}
			- `SERVER1_TOSWITCH_FPPORT`: the front panel port in {switch} data plane corresponding to {second server}
			- `SERVER1_LOCAL_IP`: the local IP address of NIC for {second server} connecting to the local area network (NOT bypass {switch} data plane)
		* Controller (co-located with first server)
			- `CONTROLLER_LOCAL_IP`: the IP address of NIC for {controller} connecting to {switch} (the same as SERVER0_TOSWITCH_IP in our testbed)
		* Switch
			- `SWITCHOS_LOCAL_IP`: the IP address of NIC for {switch} OS connecting to the local area network (NOT bypass {switch} data plane)
	+ CPU settings
		* First server
			- `SERVER0_WORKER_CORENUM`: the number of CPU cores specifically used for processing requests in {first server} (e.g. 1)
			- `SERVER0_TOTAL_CORENUM`: the total number of CPU cores in {first server} (MUST larger than SERVER0_WORKER_CORENUM; e.g., 2)
		* Second server
			- `SERVER1_WORKER_CORENUM`: the number of CPU cores specifically used for processing requests in {second server} (e.g., 1)
			- `SERVER1_TOTAL_CORENUM`: the total number of CPU cores in {second server} (MUST larger than SERVER1_TOTAL_CORENUM; e.g., 2)


- Run `bash scriptsbmv2/local/update_config_files.sh` to update ini configuration files based on the above network and CPU settings in scriptsbmv2/global.sh


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

</br>

- Compile source code for all methods
	- Under host machine: `bash scriptsbmv2/remote/firstcompile.sh` to compile software code for all methods in clients, servers, and switch(**TIME: around 1 hour**).
	- For each {method}, under {switch}
		- Run `su` to enter root mode
		- Run `cd {method}/bmv2; bash compile.sh` to make P4 code 
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
			- Run `bash scriptsbmv2/remote/keydump_and_sync.sh` to dump workload-related information (e.g., hot keys and bottleneck serveridx)
  

# 3 Running Experiments (Automatic Evaluation)

- **Note: you can refer to [AE instructions](./ae_instructions.md) for getting started instructions and detailsed instructions.**

- **Note: if you want to add any new experiment script by youself in `scriptsbmv2/exps/`, the script file name should NOT include the reserved strings (ycsb, server, controller, reflector, and server_rotation); otherwise, the new experiment script may be killed by itself during evaluation.**

## 3.1 Normal Script Usage

- To reproduce experiments in our evaluation, we provide the following scripts under `scriptsbmv2/exps/`
	+ We use server rotation to cope with limited machines, which requires relatively long time
		- Each iteration fixes the bottleneck partition (and deploys a non-bottleneck partition) (**TIME: around 5 minutes**)
		- Each server rotation comprises tens of iterations to simulate multiple machines (**TIME: around 1-8 hour(s)**)
		- Each experiment needs multiple server rotations for different methods and parameter settings (**TIME: around 1-2 day(s)**)
		- Each round includes multiple experiments to evaluate from different perspectives (**TIME: around 1 week**)
		- We need multiple rounds to reduce runtime variation (**TIME: around 1-2 month(s)**)
		- Note: as the time for evaluation is relatively long, you may want to run scripts in the background
			- **Make sure that you use `screen` or `nohup` to run each script in the background, otherwise the script will be killed by OS after you log out from the ssh connection**
		+ Note: **before and after each experiment, run `killall python server controller switchos simple_switch ovs-testcontroller` to forcely kill all processes to avoid confliction**

</br>

- Note: **if you encouter many timeouts when issuing in main client**, your servers may fail to be launched (e.g., due to code compilation with incorrect serve rotation enabling/disabling) or may be launched with a large delay (e.g., due to limited server-side power in your testbed to load database files of 100M records pre-loaded before), therefore the client-issued requests are not answered and hence have timeouts.
	- You can increase the sleep time after launching servers yet before launching clients in `scriptsbmv2/remote/test_dynamic.sh` (now is 240s), `scriptsbmv2/remote/test_server_rotation.sh` (now is 60s), `scriptsbmv2/remote/test_server_rotation_p1.sh` (now is 120s), and `scriptsbmv2/remote/test_server_rotation_p2.sh` (now is 120s).

</br>

- Note: if you encouter any other problem, you can **keep the critical information and contact us if available (sysheng21@cse.cuhk.edu.hk) for help**
	- The causes of problem may be testbed mis-configuration, script mis-usage, resource confliction, bmv2 hardware bugs, and our code bugs
	- The critical information should include: command history of terminal, dumped information of scripts, log files generated by scripts(e.g., `{method}/tmp\_\*.out` in servers and switch, `benchmark/ycsb/tmp\_\*.out` in clients, and `{method}/Bmv2/tmp\_\*.out` in switch), and raw statistics generated by YCSB clients (e.g., `benchmark/ycsb/{method}-statistics/`)

</br>

- Scripts for different experiments

   | Exp # |                                 Scripts                                 | Description |
   | :---: | :---------------------------------------------------------------------: | --- |
   |   1   |             [run_exp_throughput.sh](scriptsbmv2/exps/run_exp_throughput.sh)             | Throughput analysis for different YCSB core workloads under static workload pattern (with server rotation) |
   |   2   |          [run_exp_latency.sh](scriptsbmv2/exps/run_exp_latency.sh)          | Latency analysis for different target throughputs under static workload pattern (with server rotation) |
   |   3   |      [run_exp_scalability.sh](scriptsbmv2/exps/run_exp_scalability.sh)      | Scalability for different # of simulated servers  under static workload pattern (with server rotation)|
   |   4   |      [run_exp_write_ratio.sh](scriptsbmv2/exps/run_exp_write_ratio.sh)      | Synthetic workloads with different write ratios under static workload pattern (with server rotation) |
   |   5   | [run_exp_key_distribution.sh](scriptsbmv2/exps/run_exp_key_distribution.sh) | Synthetic workloads with different key distributions under static workload pattern (with server rotation) |
   |   6   |       [run_exp_value_size.sh](scriptsbmv2/exps/run_exp_value_size.sh)       | Synthetic workloads with different value sizes under static workload pattern (with server rotation) |
   |   7   |          [run_exp_dynamic.sh](scriptsbmv2/exps/run_exp_dynamic.sh)          | Synthetic workloads with different dynamic workload patterns (NO server rotation) |
   |   8   |         [run_exp_snapshot.sh](scriptsbmv2/exps/run_exp_snapshot.sh)         | Performance and control-plane bandwidth overhead of snapshot generation under different dynamic workload patterns (NO server rotation) |
   |   9   |         [run_exp_recovery.sh](scriptsbmv2/exps/run_exp_recovery.sh)         | Crash recovery time under static workload pattern (with server rotation) |


</br>

- Other useful scripts
	+ scriptsbmv2/remote/stopall.sh: forcely to stop and kill ycsb clients, server rotation scripts, dynamic scripts, servers (including controller and simulated reflector), and switch (including data plane and switch OS).
		- Note: you can run this script to kill all involved processes, if the previous experiment fails (e.g., due to testbed mis-configuration)
	+ scriptsbmv2/remote/enable_server_rotation.sh: update common/helper.h to enable server rotation and re-compile software code of all methods
	+ scriptsbmv2/remote/disable_server_rotation.sh: update common/helper.h to disable server rotation and re-compile software code of all methods
	+ scriptsbmv2/results/\*.sh: parse raw output file of run_exp\_\*.sh to get results

</br>

- With server rotation: run each {experiment} except exp_dynamic and exp_snapshot
	- Options of {experiment}: exp_throughput, exp_latency, exp_scalability, exp_write_ratio, exp_key_distribution, exp_value_size, and exp_recovery
	- Under host machine
		- Re-compile all methods to enable server rotation if NOT: `bash scriptsbmv2/remote/enable_server_rotation.sh`
		- Run `nohup bash scriptsbmv2/exps/run_{experiment}.sh <roundnumber> >tmp_{experiment}.out 2>&1 &`
		- Note: we run each experiment for multiple rounds to eliminate the effect of runtime variation (e.g., RocksDB fluctuation), so we need to specify <roundnumber> to indicate the index of the current round for running an experiment
	- After experiment, Under host machine
		- `killall python server controller switchos simple_switch ovs-testcontroller` to kill all involved processes
		- `bash scriptsbmv2/results/parse_{experiment}.sh tmp_{experiment}.out` to get results

</br>

- Without server rotation: run {experiment} of exp_dynamic or exp_snapshot
	- Under host machine
		- Re-compile code to disable server rotation if NOT: `bash scriptsbmv2/remote/disable_server_rotation.sh`
		- Run `bash scriptsbmv2/exps/run_{experiment} <roundnumber>`
		- Note: we run each experiment for multiple rounds to eliminate the effect of runtime variation (e.g., RocksDB fluctuation), so we need to specify <roundnumber> to indicate the index of the current round
	- After experiment, Under host machine
		- `killall python server controller switchos simple_switch ovs-testcontroller` to kill all involved processes
		- `bash scriptsbmv2/results/parse_{experiment}.sh tmp_{experiment}.out` to get results
	- As most experiments use server rotation for static pattern instead of dynamic patterns, you may want to re-compile your code to enable server rotation again
		- Under host machine, enable server rotation: `bash scriptsbmv2/remote/enable_server_rotation.sh`


</br>

- Notes for exp_recovery
	- Possible errors for scp in `farreach/localscriptsbmv2/fetch\*.sh` (maybe due to testbed mis-configurations)
	- If you want to test recovery time based on previous raw statistics of exp_recovery instead of running server-rotation-based experiment for exp_recovery again
    	- Step 1: check `scriptsbmv2/global.sh` Under host machine
			- Make sure `EVALUATION_OUTPUT_PREFIX` points to the path of previous raw statistics of exp-recovery (including in-switch snapshot, client-side backups, and maxseq) generated by previous server-rotation-based experiments
		- Step 2: Under host machine
			- Set `exp9_recoveryonly` as 1 in `scriptsexps/run_exp_recovery.sh`
			- Run `scriptsbmv2/exps/run_exp_recovery.sh`, which will skip the step of running a new server-rotation-based experiment for exp_recovery
	- Note: **crash recovery time is strongly related with network settings in each specific testbed**. which may have differences in units of 0.1 seconds

## 3.2 Perform Single Iteration

- During one server rotation composed of tens of iterations, some iterations may fail due to database performance fluctuation or unstable testbed
	- To fix this issue, we provide a script to run a single iteration (**TIME: around 5 minutes**) for each failed iteration instead of re-running all iterations of the server rotation (**TIME: around 1-8 hour(s)**), which is time-consuming

</br>

- If scripts (e.g., `scriptsbmv2/local/calculate_statistics.sh`) say that you need to perform a single iteration for each missing iteration number of an incomplete server rotation of the experiment
	- Under host machine, run `bash scriptsbmv2/exps/run_makeup_rotation_exp.sh <expname> <roundnumber> <methodname> <workloadname> <serverscale> <bottleneckidx> <targetrotation> [targetthpt]` to launch a single iteration
		- `expname`: experiment name (eg.: "exp1" for throughput analysis)
			- Note: `expname` only indicates the path to store rawstatistics, yet NOT affect experiment results
		- `roundnumber`: the index of the current round for running the experiment (eg.: 0)
		- `methodname`: experiment method (eg.: farreach)
		- `workloadname`: workload name (eg.: workloada)
		- `serverscale`: number of simulated servers (eg.: 16)
		- `bottleneckidx`: bottleneck server index of server rotation related with the workload and scale (eg.: 14)
		- `targetrotation`: the non-bottleneck server index in the missing iteration (eg.: 10)
		- `targetthpt`: the target throughput of the server rotation for the missing iteration, only applicable for exp2 (latency analysis)
		- The above arguments of `scriptsbmv2/exps/run_makeup_rotation_exp.sh` are determined by the missing iteration of the server rotation for the specific experiment
			- For example, for exp_throughput, you may pass arguments with `expname=exp1, roundnumber=0, methodname=farreach, workloadname=workloada, serverscale=16, bottleneckidx=14, targetrotation=10` to execute the 11th iteration
				- The script will deploy the bottleneck serveridx 14 in {first server} and the non-bottleneck serveridx 10 in {second server} for the single iteration, and update the raw statistics in place
	- Note: `scriptsbmv2/exps/run_makeup_rotation_exp.sh` **should NOT support exp_dynamic or exp_snapshot**, as the experiments of dynamic workload patterns do NOT use server rotation
		- Therefore, this script ONLY works for experiments with server rotation: exp_key_distribution, exp_latency, exp_scalability, exp_value_size, exp_write_ratio, and exp_throughput
	- Note: `scriptsbmv2/exps/run_makeup_rotation_exp.sh` **now does NOT support the failure of the first iteration** (i.e., ONLY the bottleneck partition is deployed in {first server})
		- You may refer to [Section 4.2](#42-static-workload-server-rotation) (especially for Step 4) to use `scriptsbmv2/remote/test_server_rotation_p1.sh`
		- Make sure `scriptsbmv2/global.sh`, `scriptsbmv2/common.sh`, and `${method}/config.ini` are correctly configured before running `scriptsbmv2/remote/test_server_rotation_p1.sh`
		- (TODO) We may update `scriptsbmv2/exps/run_makeup_rotation_exp.sh` to support the first iteration in the future

# 4 Running Workloads (Manual Evaluation)

## 4.1 Dynamic Workload (No Server Rotation)

- Decide {workload} (e.g., workloada), {method} (e.g., farreach), and {dynamic pattern} (e.g., hotin) to use
	- Options of {dynamic pattern}: hotin, hotout, and random
	- Note: `scriptsbmv2/exps/run_exp_dynamic.sh` or `scriptsbmv2/exps/run_exp_snapshot.sh` include all the following steps (except step 2, as the two scripts assume that you have already disabled server rotation in advance)

</br>

- Step 1: prepare ini config files
	- Update settings in the config file `{method}/config.ini` (e.g., farreach/config.ini):
		- Set `global::workload_mode`=1
		- Set `global::workload_name`={workload} 
		- Set `global::dynamic_ruleprefix`={dynamic pattern}
		- Set `global::server_physical_num`=2
		- Set `global::server_total_logical_num`=2
		- Set `server0::server_logical_idxes`=0
		- Set `server1::server_logical_idxes`=1
	- Set DIRNAME as {method} in `scriptsbmv2/common.sh`
    - Double-check the global testbed settings in scriptsbmv2/global.sh based on your testbed

</br>

- Step 2: if server rotation is enabled (default setting), re-compile code to disable server rotation
	- Disable server rotation
		- Comment line 82 (#define SERVER_ROTATION) in `common/helper.h` to diable server rotation
	- For the current {method}, re-compile software code (NO need for P4 code)
		- Set DIRNAME as {method} in `scriptsbmv2/common.sh`
		- `make all`

</br>

- Step 3: launch switch data plane and switch OS
	- Create two terminals in {switch}
	- Launch switch data plane in the first terminal
		- `cd {method}/bmv2` 
		- `su`
		- Run `bash start_switch.sh`
	- Launch switch OS and other daemon processes (for cache management and snapshot generation) in the second terminal
		- `cd {method}`
		- `su`
		- `bash localscriptsbmv2/launchswitchostestbed.sh`
	- Note: if you encounter any problem, you can check the log files of `{method}/tmp_\*.out` and `{method}/Bmv2/tmp_\*.out` in {switch}

</br>

- Step 4: launch servers and clients without server rotation
	- `bash scriptsbmv2/remote/test_dynamic.sh`
	- Note: if you encouter any problem
		- You can check the output of {main client}
		- You can check the log files of `benchmark/ycsb/tmp_\*.out` 
		- You can check the log files of `{method}/tmp_\*.out` 

</br>

- Step 5: cleanup testbed
	- `killall python server controller switchos simple_switch ovs-testcontroller`

</br>

- Step 6: aggregate statistics
   - Run `bash scriptsbmv2/remote/calculate_statistics.sh`

</br>

- Step 7: if you do NOT run dynamic workload patterns, you should re-compile code to enable server rotation
	- Enable server rotation
		- Uncomment line 82 (#define SERVER_ROTATION) in `common/helper.h` to enable server rotation
		
	- For the current {method}, re-compile software code (NO need for P4 code)
		- Set DIRNAME as {method} in `scriptsbmv2/common.sh`
		- `make all`

## 4.2 Static Workload (Server Rotation)

- Decide {workload} (e.g., workloada) and {method} (e.g., farreach) to use
	- Note: we assmue that you have analyzed {workload} to get {bottleneck serveridx} for your {server rotation scale}
		- If not, please refer to [Section 2.2](#22-workload-analysis--dump-keys) for workload analysis
		- As bottleneck server index is stable for a given <{workload}, {server rotation scale}>, you can also directly refer to the appendix table in [Section 6.1](#61-bottleneck-index-for-server-rotation)
	- Note: we assume that you have already compiled code with server rotation enabled
		- If not, please refer to step 7 in [Section 4.1](#41-dynamic-workload-no-server-rotation) to re-compile code for enabling server rotation
	- Note: the scripts in `scriptsbmv2/exps/` (except run_exp_dynamic.sh and run_exp_snapshot.sh) include all the following steps

</br>

- Step 1: prepare ini config files 
	- Update settings in the config file `{method}/config.ini` (e.g., farreach/config.ini):
		- Set `workload_name`={workload}
		- Set `workload_mode`=0
		- Set `bottleneck_serveridx_for_rotation`={bottleneck serveridx}
		- Set `server_total_logical_num_for_rotation`={server rotation scale}
		- Note: `{method}/config.ini` must have the correct {bottleneck serveridx} and {server rotation scale}
			- Otherwise, client-side PregeneratedWorkload will issue the requests of incorrect partitions (corresponding to non-running servers) and hence timeout
	- Set DIRNAME as {method} in `scriptsbmv2/common.sh`
    - Double-check the global testbed settings in `scriptsbmv2/global.sh` based on your testbed

</br>

- Step 2: prepare for launching switch data plane and switch OS
	- Under host machine, run `bash scriptsbmv2/remote/prepare_server_rotation.sh`
		- This script can generate and sync a new {method}/config.ini based on the existing `{method}/config.ini` with the configurations you set in step 1
			- The main changes in the new `{method}/config.ini` is that it sets `server0::server_logical_idxes` as ${bottleneck serveridx} (e.g., 14), and sets `server1::server_logical_idxes` as all other serveridxes except ${bottleneck serveridx} (e.g., 0:1:2:3:4:5:6:7:8:9:10:11:12:13:15)
		- The goal is that {switch} can use the new `${method}/config.ini` to configure packet forwarding rules, such that we do NOT need to re-launch switch during server rotation

</br>

- Step 3: launch switch data plane and switch OS
	- Create two terminals in {switch}
	- Launch switch data plane in the first terminal
		- `cd {method}/bmv2` 
		- Run `bash start_switch.sh`
	- Launch switch OS and other daemon processes (for cache management and snapshot generation) in the second terminal
		- `cd {method}`
		- `su`
		- `bash localscriptsbmv2/launchswitchostestbed.sh`
	- Note: if you encounter any problem, you can check the log files of `{method}/tmp_\*.out` and {method}/Bmv2/tmp_\*.out in {switch}

</br>

- Step 4: launch servers and clients with server rotation
	- Under host machine run `bash scriptsbmv2/remote/test_server_rotation.sh`
		- Phase 1: test_server_rotation.sh invokes `scriptsbmv2/remote/test_server_rotation_p1.sh` to run the first iteration (the bottleneck partition is deployed into {first server})
		- Phase 2: test_server_rotation.sh invokes `scriptsbmv2/remote/test_server_rotation_p2.sh` to run the ith iteration (the bottleneck partition is deployed into {first server}, and the ith non-bottleneck partition is deployed into {second server}), where 1 <= i <= {server rotation scale}-1
	-Under host machine, perform a single iteration for each failed iteration if any
		- If strid=server-x is missed, run `bash scriptsbmv2/remote/test_server_rotation_p1.sh 1`
		- If strid=server-x-y is missed, run: `bash scriptsbmv2/remote/test_server_rotation_p2.sh 1 y`


</br>

- Step 5: cleanup testbed
	- `killall python server controller switchos simple_switch ovs-testcontroller`


</br>

- Step 6: aggregate statistics
   - Under host machine, run `bash scriptsbmv2/remote/calculate_statistics.sh`

