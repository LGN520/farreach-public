# README of FarReach

# Table of Contents

0. [Overview](#0-overview)
	1. [Methods](#01-methods)
	2. [Our Testbed](#02-our-testbed)
1. [System Preperation](#1-system-preperation)
	1. [Dependency Installation](#11-dependency-installation)
	2. [Configuration settings](#12-configuration-settings)
	3. [Code Compilation](#13-code-compilation)
	4. [Testbed Building](#14-testbed-building)
2. [Data Preperation](#2-data-preperation)
	1. [Loading Phase](#21-loading-phase)
	2. [Workload Analysis & Dump Keys](#22-workload-analysis--dump-keys)
3. [Running Experiments (Automatic Evaluation)](#3-running-experiments-automatic-evaluation)
	1. [Normal Script Usage](#31-normal-script-usage)
	2. [Perform Single Iteration](#32-perform-single-iteration)
4. [Running Workloads (Manual Evaluation)](#4-running-workloads-manual-evaluation)
	1. [Dynamic Workload (No Server Rotation)](#41-dynamic-workload-no-server-rotation)
	2. [Static Workload (Server Rotation)](#42-static-workload-server-rotation)
5. [Aggregate Statistics](#5-aggregate-statistics)
	1. [Scripts](#51-scripts)
	2. [Usage and Example](#52-usage-and-example)
6. [Appendix](#6-appendix)
	1. [Bottleneck Index for Server Rotation](#61-bottleneck-index-for-server-rotation)
	2. [Other Notes](#62-other-notes)

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
	+ Four physical machines with Ubuntu 16.04/18.04
		* One main client (hostname: dl11)
		* One secondary client (hostname: dl20)
		* Two servers (hostnames: dl21 and dl30)
			- Note: controller is co-located in the first server (dl21)
	+ One 2-pipeline Tofino switch with SDE 8.9.1 (hostname: bf3)
		* **Note: larger SDE version (e.g., 9.0.1) cannot support P4\_14 correctly due to compiler its own bugs**

</br>

- Network configuration and topology
	+ All clients/servers/switch are in the same local area network (IP mask: 172.16.255.255; NOT bypass {switch} data plane)
		* Main client: 172.16.112.11
		* Secondary client: 172.16.112.20
		* First server (co-located with controller): 172.16.112.21
		* Second server: 172.16.112.30
		* Tofino switch OS: 172.16.112.19
	+ Testbed topology of programmable-switch-based network (bypass {switch} data plane)
		* Main client (NIC: enp129s0f0; MAC: 3c:fd:fe:bb:ca:78) <-> Tofino switch (front panel port: 5/0)
		* Secondary client (NIC: ens2f0; MAC: 9c:69:b4:60:34:f4) <-> Tofino switch (front panel port: 21/0)
		* First server (NIC: ens2f1; MAC: 9c:69:b4:60:34:e1) <-> Tofino switch (front panel port: 6/0)
		* Second server (NIC: ens3f1; MAC: 9c:69:b4:60:ef:c1) <-> Tofino switch (front panel port: 3/0)
		* Tofino switch (front panel port: 7/0) <-> Tofino switch (front panel port: 12/0) (for in-switch cross-pipeline recirculation)

# 1 System Preperation

## 1.1 Dependency Installation

- Install libboost 1.81.0 in {first server} and {second server} if not
	+ Under project directory (e.g., /home/ssy/projects/farreach-public)
		+ `wget https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.tar.gz`
		+ `tar -xzvf boost_1_81_0.tar.gz`
		+ `cd boost_1_81_0; ./bootstrap.sh --with-libraries=system,thread --prefix=./install && sudo ./b2 install`
	<!-- + `sudo apt-get install libboost-all-dev` -->
- Install Maven 3.3.9 in {main client} and {secondary client} if not 
- Install Java OpenJDK-8 in {main client} and {secondary client} if not
	+ **Configure JAVA_HOME**: add sth like `export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64/` (based on your own JAVA path) to ~/.bashrc
- Install Tofino SDE 8.9.1 in {switch} if not
	+ **Note: larger version (e.g., 9.0.1) cannot support P4\_14 correctly due to compiler its own bugs**
- Install gcc-7.5.0 and g++-7.5.0 in two clients and two servers if not (NO need for {switch})
- Use RocksDB 6.22.1 (already embedded in the project; NO need for extra installation)

<!-- - ONLY for multi-switch setting: Install bpftools for spine reflector -->
<!--	+ `git clone https://github.com/cloudflare/bpftools.git` -->
<!--	+ `cd bpftools` -->
<!--	+ `make` -->
<!-- - DEPRECATED: Disable THP to reduce write_memtable_time in rocksdb -->
<!--	+ `sudo apt-get install hugepages` -->
<!--	+ `sudo hugeadm \-\-thp-never` -->
<!--	+ `cat /sys/kernel/mm/transparent_hugepage/enabled` to check -->

## 1.2 Configuration Settings

- Update the following configuration settings in scripts/global.sh based on your own testbed
	+ USER: Linux username (e.g., ssy)
	+ SWITCH\_PRIVATEKEY: the passwd-free ssh key used by {main client} to connect with {switch} as **root user** (e.g., ~/.ssh/switch-private-key)
		* See detailed steps in "Update SSH configuration settings" at the end of this Section 1.2
	+ CONNECTION\_PRIVATEKEY: the passwd-free ssh key used by {switch} to connect with two servers (for maxseq and in-switch snapshot), by two servers to connect with two clients (for client-side backups), and by {second server} to connect with {first server} (co-located with controller) (for in-switch snapshot) as **non-root user** (e.g., ~/.ssh/connection-private-key)
		* See detailed steps in "Update SSH configuration settings" at the end of this Section 1.2
	+ MAIN\_CLIENT: hostname of {main client} (e.g., dl11)
	+ SECONDARY\_CLIENT: hostname of {secondary client} (e.g., dl20)
	+ SERVER0: hostname of {first server} co-located with controller (e.g., dl21)
	+ SERVER1: hostname of {second server} (e.g., dl30)
	+ LEAFSWITCH: hostname of the Tofino {switch} (e.g., bf3)
	+ CLIENT/SWITCH/SERVER\_ROOTPATH: project directory path in clients/switch/servers (e.g., /home/ssy/projects/farreach-public)
    + EVALUATION\_OUTPUT\_PREFIX: path to store the raw statistics of each experiment for aggregating analysis
	+ Network settings
		* Main client
			- MAIN_CLIENT_TOSWITCH_IP: the IP address of NIC for {main client} connecting to {switch}
			- MAIN_CLIENT_TOSWITCH_MAC: the MAC address of NIC for {main client} connecting to {switch}
			- MAIN_CLIENT_TOSWITCH_FPPORT: the front panel port in {switch} data plane corresponding to {main client}
			- MAIN_CLIENT_TOSWITCH_PIPEIDX: the pipeline index (0 or 1) for MAIN_CLIENT_TOSWITCH_FPPORT
			- MAIN_CLIENT_LOCAL_IP: the local IP address of NIC for {main client} connecting to the local area network (NOT bypass {switch} data plane)
		* Secondary client
			- SECONDARY_CLIENT_TOSWITCH_IP: the IP address of NIC for {secondary client} connecting to {switch}
			- SECONDARY_CLIENT_TOSWITCH_MAC: the MAC address of NIC for {secondary client} connecting to {switch}
			- SECONDARY_CLIENT_TOSWITCH_FPPORT: the front panel port in {switch} data plane corresponding to {secondary client}
			- SECONDARY_CLIENT_TOSWITCH_PIPEIDX: the pipeline index (0 or 1) for SECONDARY_CLIENT_TOSWITCH_FPPORT
			- SECONDARY_CLIENT_LOCAL_IP: the local IP address of NIC for {secondary client} connecting to the local area network (NOT bypass {switch} data plane)
		* First server
			- SERVER0_TOSWITCH_IP: the IP address of NIC for {first server} connecting to {switch}
			- SERVER0_TOSWITCH_MAC: the MAC address of NIC for {first server} connecting to {switch}
			- SERVER0_TOSWITCH_FPPORT: the front panel port in {switch} data plane corresponding to {first server}
			- SERVER0_TOSWITCH_PIPEIDX: the pipeline index (0 or 1) for SERVER0_TOSWITCH_FPPORT
			- SERVER0_LOCAL_IP: the local IP address of NIC for {first server} connecting to the local area network (NOT bypass {switch} data plane)
		* Secondary server
			- SERVER1_TOSWITCH_IP: the IP address of NIC for {second server} connecting to {switch}
			- SERVER1_TOSWITCH_MAC: the MAC address of NIC for {second server} connecting to {switch}
			- SERVER1_TOSWITCH_FPPORT: the front panel port in {switch} data plane corresponding to {second server}
			- SERVER1_TOSWITCH_PIPEIDX: the pipeline index (0 or 1) for SERVER1_TOSWITCH_FPPORT
			- SERVER1_LOCAL_IP: the local IP address of NIC for {second server} connecting to the local area network (NOT bypass {switch} data plane)
		* Controller (co-located with first server)
			- CONTROLLER_LOCAL_IP: the IP address of NIC for {controller} connecting to {switch} (the same as SERVER0_TOSWITCH_IP in our testbed)
		* Switch
			- SWITCHOS_LOCAL_IP: the IP address of NIC for {switch} OS connecting to the local area network (NOT bypass {switch} data plane)
			- SWITCH_RECIRPORT_PIPELINE1TO0: the front panel port in {switch} data plane for pipeline 1 to connect with pipeline 0 for in-switch recirculation
			- SWITCH_RECIRPORT_PIPELINE0TO1: the front panel port in {switch} data plane for pipeline 0 to connect with pipeline 1 for in-switch recirculation
	+ CPU settings
		* First server
			- SERVER0_WORKER_CORENUM: the number of CPU cores specifically used for processing requests in {first server} (e.g. 16)
			- SERVER0_TOTAL_CORENUM: the total number of CPU cores in {first server} (MUST larger than SERVER0_WORKER_CORENUM; e.g., 48)
		* Second server
			- SERVER1_WORKER_CORENUM: the number of CPU cores specifically used for processing requests in {second server} (e.g., 16)
			- SERVER1_TOTAL_CORENUM: the total number of CPU cores in {second server} (MUST larger than SERVER1_TOTAL_CORENUM; e.g., 48)

</br>

- Run `bash scripts/local/update_config_files.sh` to update ini configuration files based on the above network and CPU settings in scripts/global.sh

</br>

- Update SSH configuration settings
	+ In any of above machines (2 clients, 2 servers, and 1 switch), if you need to manually type yes/no to check the hostname when using ssh command to connect other machines, add the following content to `~/.ssh/config` under the machine:
	```
	Host *
		StrictHostKeyChecking no
	```
	+ Generate SWITCH_PRIVATEKEY
		* Under {main client}, if the ssh key has not been created, run `sudo ssh-keygen -t rsa -f /home/{USER}/.ssh/switch-private-key` (**use empty passwd for no passphrase**)
			- Also run `sudo chown {USER}:{USER} /home/{USER}/.ssh/switch-private-key` (use your Linux username) to change the owner
		* Append the content of ~/.ssh/switch-private-key.pub of {main client}, into /root/.ssh/authorized_keys of {switch}
	+ Generate CONNECTION_PRIVATEKEY
		* Consider the following source-connectwith-destination pairs:
			- {switch}-connectwith-{first server} and {switch}-connectwith-{second server}
			- {first server}-connectwith-{main client}, {first server}-connectwith-{secondary client}, {second server}-connectwith-{main client}, and {second server}-to-{secondary client}
			- {second server}-connectwith-{first server} (controller is co-located with {first server})
		* For each pair of source connecting with destination
			- Under {source}, if the ssh key has not been created, run `ssh-keygen -t rsa -f ~/.ssh/connection-private-key` (**use empty passwd for no passphrase**)
			- Append the content of ~/.ssh/connection-private-key.pub of {source}, into ~/.ssh/authorized_keys of {destination}

## 1.3 Code Compilation

- Sync and compile RocksDB (**TIME: around 3 hours**; ONLY need to perform once)
	+ Under {main client}, run `bash scripts/remote/sync_kvs.sh` to sync rocksdb-6.22.1/ to {first server} and {second server}
	+ Under {first server} and {second server}, compile RocksDB
		+ Run `sudo apt-get install libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev libjemalloc-dev libsnappy-dev` to install necessary dependencies for RocksDB
		+ `cd rocksdb-6.22.1`
		+ `PORTABLE=1 make static_lib`
			* We already comment -Wstrict-prototype and -Werror in RocksDB's Makefile to fix compilation errors due to strict-alias warning
			* We use PORTABLE=1 to fix runtime error of illegal instruction when open()
		+ For each method (farreach or nocache or netcache)
			* Run `mkdir /tmp/{method}` to prepare the directory (e.g., /tmp/farreach) for database in advance

</br>

- For each {method}
	+ Options of method name: farreach, nocache, or netcache
	+ Under {main client}
		* Set DIRNAME as {method} in scripts/common.sh
		* `bash scripts/remote/sync.sh` to sync code of {method} to all machines
	+ Compile software code
		* Under {main client} and {secondary client}
			- Run `bash scripts/local/makeclient.sh` (TIME: around 10 minutes; especially for the first time with Maven downloading libs)
		* Under {switch}
			- Run `bash scripts/local/makeswitchos.sh` to make switch OS (TIME: around 1 minute)
			- Run `su` to enter root mode, and run `cd {method}/tofino; bash compile.sh` to make P4 code (**TIME: around 3 hours**)
				+ If you have compiled P4 code of {method} before, you do **NOT need to re-compile it again**
				+ If you really want to re-compile it (maybe due to P4 code modification), you should delete the corresponding directory (netbufferv4, nocache, or netcache) under $SDE/pkgsrc/p4-build/tofino/ before re-compilation
		* Under {first server} and {second server}
			- Run `bash scripts/local/makeserver.sh` (TIME: around 5 minutes)
		* NOTE: if with "make: warning:  Clock skew detected.  Your build may be incomplete" during make, run `cd {method}; find . -type f | xargs touch` and then re-make files

<!--
* Automatic way (NOT used now due to very slow sequential compialtion) vs. makeclient/server/switchos.sh
	- Under the main client, `bash scripts/remote/makeremotefiles.sh` to make C/C++ and java code including client, switchos, controller, and server
-->

## 1.4 Testbed Building

- Building your testbed based on network settings provided by scripts/global.sh
	+ In {main client}, run `bash scripts/local/configure_client.sh 0`
	+ In {secondary client}, run `bash scripts/local/configure_client.sh 1`
	+ In {first server}, run `bash scripts/local/configure_server.sh 0`
	+ In {second server}, run `bash scripts/local/configure_server.sh 1`
	+ In {switch} OS, run `su` to enter root mode and run `bash scripts/local/configure_switchos.sh`

# 2 Data Preperation

## 2.1 Loading Phase

- Perform the loading phase and backup for evaluation time reduction (**TIME: around 40 minutes**; ONLY need to perform once)
	+ Under {main client}
		* Set DIRNAME as nocache in scripts/common.sh
		* Run `bash scripts/remote/sync_file.sh scripts common.sh` to sync scripts/common.sh to all clients, servers, and switch
		* Run `bash scripts/remote/prepare_load.sh` to copy recordload/config.ini to overwrite nocache/config.ini in all clients, servers, and switch
	+ Under {switch}, create two terminals
		* In the first terminal
			- `cd nocache/tofino`
			- Run `su` to enter root mode
			- Run `bash start_switch.sh` to launch nocache switch data plane, which will open a CLI
		* In the second terminal
			- `cd nocache`
			- Run `su` to enter root mode
			- Run `bash localscripts/launchswitchostestbed.sh` to launch nocache switch OS and other daemon processes
	+ Under {main client}
		* Run `bash scripts/remote/load_and_backup.sh` to launch servers and clients automatically for loading and backup
			- Note: scripts/remote/load_and_backup.sh will kill servers and clients at the end automatically
				- You can also run `bash scripts/stopall.sh` manually under {main client} in case that some processses are NOT killed
			- Note: scripts/remote/load_and_backup.sh will resume the original nocache/config.ini in all clients, server, and switch after all steps
	+ Under {switch}
		- In the first terminal
			- Type exit and press enter to stop switch data plane
			- If CLI is still NOT closed, type Ctrl+C in the CLI to stop switch data plane
		- In the second terminal
			- `cd nocache`
			- Run `su` to enter root mode
			- Run `bash localscripts/stopswitchtestbed.sh` to stop switch OS and other daemon processes

## 2.2 Workload Analysis & Dump Keys

- Analyze workloads to dump hot keys, dynamic patterns, and bottleneck partition (ONLY need to perform once)
	- For each {workload} (TIME: around 5 minutes per workload)
		- Options of workload name: workloada, workloadb, workloadc, workloadd, workloadf, workload-load, synthetic, synthetic-25, synthetic-75, skewness-90, skewness-95, uniform, valuesize-16, valuesize-32, valuesize-64
		- Under {main client}
			- Update workload\_name as {workload} in keydump/config.ini
			- Run `bash scripts/remote/keydump_and_sync.sh` to dump workload-related information (e.g., hot keys and bottleneck serveridx), and sync them to all clients, servers, and switch
  
<!--
- (DEPRECATED) Manual way vs. keydump_and_sync.sh
    - `cd benchmark/ycsb; ./bin/ycsb run keydump; cd ../../`
  		+ Dump hottest/nearhot/coldest keys into `benchmark/output/<workloadname>-\*.out` for warmup phase later
  		+ Dump bottleneckt serveridx for different server rotation scales for server rotation later
  		+ Pre-generate workload files in `benchmark/output/<workloadname>-pregeneration/` for server rotation later
  	- `cd benchmark/ycsb; python generate_dynamicrules.py <workloadname>; cd ../../`
  		+ Generates key popularity changing rules in `benchmark/output/<workloadname>-\*rules/` for dynamic pattern later
  	- `bash scripts/remote/deprecated/synckeydump.sh <workloadname>` to sync the above files of the workload to another client
-->

# 3 Running Experiments (Automatic Evaluation)

## 3.1 Normal Script Usage

- To reproduce experiments in our evaluation, we provide the following scripts under `scripts/exps/`
	+ Note: we use server rotation to cope with limited machines, which requires relatively long time
		- Each iteration fixes the bottleneck partition (and deploys a non-bottleneck partition) (**TIME: around 5 minutes**)
		- Each server rotation comprises tens of iterations to simulate multiple machines (**TIME: around 1-8 hour(s)**)
		- Each experiment needs multiple server rotations for different methods and parameter settings (**TIME: around 1-2 day(s)**)
		- Each round includes multiple experiments to evaluate from different perspectives (**TIME: around 1 week**)
		- We need multiple rounds to reduce runtime variation (**TIME: around 1-2 month(s)**)
	+ Note: if you encouter any problem (maybe due to testbed mis-configuration, script mis-usage, resource confliction (e.g., Tofino switch data plane cannot support multiple P4 programs simultaneously), and our code bugs), you can keep the critical information and contact us (sysheng21@cse.cuhk.edu.hk) for help
		- The critical information could include: command history of terminal, dumped information of scripts, log files generated by scripts(e.g., {method}/tmp\_\*.out in servers and switch, benchmark/ycsb/tmp\_\*.out in clients, and {method}/tofino/tmp\_\*.out in switch), and raw statistics generated by YCSB clients (e.g., benchmark/ycsb/{method}-statistics/)

</br>

- Scripts for different experiments

   | Exp # |                                 Scripts                                 | Description |
   | :---: | :---------------------------------------------------------------------: | --- |
   |   1   |             [run_exp_ycsb.sh](scripts/exps/run_exp_ycsb.sh)             | Throughput analysis for different YCSB core workloads under static workload pattern (with server rotation) |
   |   2   |          [run_exp_latency.sh](scripts/exps/run_exp_latency.sh)          | Latency analysis for different target throughputs under static workload pattern (with server rotation) |
   |   3   |      [run_exp_scalability.sh](scripts/exps/run_exp_scalability.sh)      | Scalability for different # of simulated servers  under static workload pattern (with server rotation)|
   |   4   |      [run_exp_write_ratio.sh](scripts/exps/run_exp_write_ratio.sh)      | Synthetic workloads with different write ratios under static workload pattern (with server rotation) |
   |   5   | [run_exp_key_distribution.sh](scripts/exps/run_exp_key_distribution.sh) | Synthetic workloads with different key distributions under static workload pattern (with server rotation) |
   |   6   |       [run_exp_value_size.sh](scripts/exps/run_exp_value_size.sh)       | Synthetic workloads with different value sizes under static workload pattern (with server rotation) |
   |   7   |          [run_exp_dynamic.sh](scripts/exps/run_exp_dynamic.sh)          | Synthetic workloads with different dynamic workload patterns (NO server rotation) |
   |   8   |         [run_exp_snapshot.sh](scripts/exps/run_exp_snapshot.sh)         | Performance and control-plane bandwidth overhead of snapshot generation under different dynamic workload patterns (NO server rotation) |
   |   9   |         [run_exp_recovery.sh](scripts/exps/run_exp_recovery.sh)         | Crash recovery time under static workload pattern (with server rotation) |

</br>

- Other useful scripts
	+ scripts/remote/stopall.sh: forcely to stop and kill clients, servers (including controller and simulated reflector), and switch (including data plane and switch OS).
		- Note: you can run this script to kill all involved processes, if the previous experiment fails (e.g., due to testbed mis-configuration)

</br>

- With server rotation: run each {experiment} except exp_dynamic and exp_snapshot
	- Options: exp_ycsb, exp_latency, exp_scalability, exp_write_ratio, exp_key_distribution, exp_value_size, and exp_recovery
	- Under {main client}, take exp_ycsb as an example
		- Run `bash scripts/exps/run_exp_ycsb.sh <roundnumber>`
		- Note: we run each experiment for multiple rounds to eliminate the effect of runtime variation (e.g., RocksDB fluctuation), so we need to specify <roundnumber> to indicate the index of the current round

</br>

- Without server rotation: run exp_dynamic or exp_snapshot
	- Under {main client}, re-compile code to disable server rotation if not
		- Comment line 82 (#define SERVER_ROTATION) in common/helper.h to disable server rotation
		- `bash scripts/remote/sync_file.sh common helper.h` to sync code changes to all machines
	- Re-compile software code (NO need for P4 code of switch data plane)
		- Under {main client} and {secondary client}, run `bash scripts/local/makeclient.sh`
		- Under {first server} and {second server}, run `bash scripts/local/makeserver.sh`
		- Under {switch}, run `bash scripts/local/makeswithos.sh`
	- Under {main client}, take exp_dynamic as an example
		- Run `bash scripts/exps/run_exp_dynamic.sh <roundnumber>`
		- Note: we run each experiment for multiple rounds to eliminate the effect of runtime variation (e.g., RocksDB fluctuation), so we need to specify <roundnumber> to indicate the index of the current round
	- After running all rounds of exp_dynamic or exp_snapshot, as most experiments use server rotation for static workload pattern instead of dynamic workload patterns, you may want to re-compile your code to enable server rotation again
		- Under {main client}
			- Uncomment line 82 (#define SERVER_ROTATION) in common/helper.h to enable server rotation
			- `bash scripts/remote/sync_file.sh common helper.h` to sync code changes to all machines
		- Re-compile software code (NO need for P4 code of switch data plane)
			- Under {main client} and {secondary client}, run `bash scripts/local/makeclient.sh`
			- Under {first server} and {second server}, run `bash scripts/local/makeserver.sh`
			- Under {switch}, run `bash scripts/local/makeswithos.sh`

</br>

- NOTEs for exp_recovery
	- Possible error messages for scp in farreach/localscripts/fetch\*.sh (maybe due to testbed mis-configurations)
		- If you have an error message of `hot key verification failed`, check whether {switch} can connect with two servers, two servers can connect with two clients, and {second server} can connect with {first server}, by CONNECTION_PRIVATEKEY
		- If you have an error message of `permission denied` when transfering files, check the correctness of ownership for /tmp/farreach in {switch} and two servers
		- If you have an error message of `permission denied (public key)`, check whether you spefcify the correct CONNECTION_PRIVATEKEY in {switch} and two servers
	- If you want to test recovery time based on existing raw statistics instead of running server-rotation-based experiments again
    	- Step 1: check scripts/global.sh under {main client}
			- Make sure `EVALUATION_OUTPUT_PREFIX` points to the path of existing raw statistics (including in-switch snapshot, client-side backups, and maxseq) generated by previous server-rotation-based experiments
		- Step 2: under {main client}
			- Run `scripts/exps/run_exp_recovery.sh <workloadmode> 1`, where we set the argument <recoveryonly> as 1 to skip the step of running a new server-rotation-based experiment

## 3.2 Perform Single Iteration

- During one server rotation composed of tens of iterations, some iterations may fail due to database performance fluctuation or unstable testbed
	- To fix this issue, we provide a script to run a single iteration (**TIME: around 5 minutes**) for each failed iteration instead of re-running all iterations of the server rotation (**TIME: around 1-8 hour(s)**), which is time-consuming

</br>

- If scripts (e.g., scripts/local/calculate_statistics.sh) say that you need to perform a single iteration for each missing number
	- Under {main client}, run `bash scripts/exps/run_makeup_rotation_exp.sh <expname> <roundnumber> <methodname> <workloadname> <serverscale> <bottleneckidx> <targetrotation> [targetthpt]` to launch a single iteration
		- `expname`: experiment name (eg.: exp1)
			- Note: expname only determines the path to store generated statistics, yet not affect experiment results
		- `roundnumber`: experiment round number (eg.: 1)
		- `methodname`: experiment method (eg.: farreach)
		- `workloadname`: workload name (eg.: workloada)
		- `serverscale`: number of servers for rotation (eg.: 16)
		- `bottleneckidx`: bottleneck server index for rotation with this workload (eg.: 14)
		- `targetrotation`: the non-bottleneck server index in the iteration (eg.: 10)
		- `targetthpt`: throughput target of this rotation, only applicable for exp2
		- The above arguments of scripts/exps/run_makeup_rotation_exp.sh are determined by the missing iteration of the server rotation for the specific experiment
			- For example, for exp_ycsb, you may pass arguments with expname=exp1, roundnumber=0, methodname=farreach, workloadname=workloada, serverscale=16, bottleneckidx=14, targetrotation=10 -> the script will deploy the bottleneck serveridx 14 in {first server} and the non-bottleneck serveridx 10 in {second server} for the single iteration, and update the raw statistics in place
	- Note: scripts/exps/run_makeup_rotation_exp.sh **should NOT support exp_dynamic or exp_snapshot**, as the experiments of dynamic workload patterns do NOT use server rotation
		- Therefore, this script ONLY works for experiments with server rotation: exp_key_distribution, exp_latency, exp_scalability, exp_value_size, exp_write_ratio, and exp_ycsb
	- Note: scripts/exps/run_makeup_rotation_exp.sh **now does NOT support the failure of the first iteration** (i.e., ONLY the bottleneck partition is deployed in {first server})
		- You may refer to [Section 4.2](#42-static-workload-server-rotation) (especially for Step 4) to use scripts/remote/test_server_rotation_p1.sh
		- Make sure scripts/global.sh, scripts/common.sh, and ${method}/config.ini are correctly configured before running scripts/remote/test_server_rotation_p1.sh
		- (TODO) We may update scripts/exps/run_makeup_rotation_exp.sh to support the first iteration in the future

# 4 Running Workloads (Manual Evaluation)

## 4.1 Dynamic Workload (No Server Rotation)

- Decide {workload} (e.g., workloada), {method} (e.g., farreach), and {dynamic pattern} (e.g., hotin) to use
	- Options for dynamic pattern: hotin, hotout, and random
	- Note: scripts/exps/run_exp_dynamic.sh or scripts/exps/run_exp_snapshot.sh include all the following steps (except step 2, as the two scripts assume that you have already disabled server rotation in advance)

</br>

- Step 1: prepare ini config files (under {main client})
	- Update settings in the config file {method}/config.ini (e.g., farreach/config.ini):
		- Set global::workload_mode=1
		- Set global::workload_name={workload} 
		- Set global::dynaic_ruleprefix={dynamic pattern}
		- Set global::server_physical_num=2
		- Set global::server_total_logical_num=2
		- Set server0::server_logical_idxes=0
		- Set server1::server_logical_idxes=1
	- Set DIRNAME as {method} in scripts/common.sh
    - Double-check the global testbed settings in scripts/global.sh based on your testbed

</br>

- Step 2: if server rotation is enabled (default setting), re-compile code to disable server rotation
	- Comment line 82 (#define SERVER_ROTATION) in common/helper.h to diable server rotation
	- Run `bash scripts/remote/sync_file.sh common helper.h` to sync code changes to all machines
	- Re-compile software code (NO need for P4 code)
		- Under {main client} and {secondary client}, run `bash scripts/local/makeclient.sh`
		- Under {first server} and {second server}, run `bash scripts/local/makeserver.sh`
		- Under {switch}, run `bash scripts/local/makeswithos.sh`

</br>

- Step 3: launch switch data plane and switch OS
	- Create two terminals in {switch}
	- Launch switch data plane in the first terminal
		- `cd {method}/tofino` 
		- `su`
		- Run `bash start_switch.sh`, which will open a CLI
	- Launch switch OS and other daemon processes (for cache management and snapshot generation) in the second terminal
		- `cd {method}`
		- `su`
		- `bash localscripts/launchswitchostestbed.sh`
	- NOTE: if you encounter any problem, you can check the log files of {method}/tmp_\*.out and {method}/tofino/tmp_\*.out in {switch}

</br>

- Step 4: launch servers and clients without server rotation
	- Under {main client}: `bash scripts/remote/test_dynamic.sh`
	- NOTE: if you encouter any problem
		- You can check the output of {main client}
		- You can check the log files of benchmark/ycsb/tmp_\*.out in {secondary client}
		- You can check the log files of {method}/tmp_\*.out in {first server} and {second server}

</br>

- Step 5: cleanup testbed
	- Under {switch}
		- Stop switch data plane in the CLI of the first terminal
			- Type exit and press enter
			- If CLI is not closed, type Ctrl+C
		- Stop switch OS and other daemon processes in the second terminal
			- `cd {method}`
			- `su`
			- `bash localscripts/stopswitchtestbed.sh`
	- Under {main client}, run `bash scripts/remote/stopservertestbed.sh` to stop servers
	- Note: if some processes are still NOT stopped, under {main client}, you can run `bash scripts/remote/stopall.sh`

</br>

- Step 6: aggregate statistics
   - Under {main client}, run `bash scripts/remote/calculate_statistics.sh`

</br>

- Step 7: if you do NOT run dynamic workload patterns, you should re-compile code to enable server rotation
	- Under {main client}
		- Uncomment line 82 (#define SERVER_ROTATION) in common/helper.h to enable server rotation
		- `bash scripts/remote/sync_file.sh common helper.h` to sync code changes to all machines
	- Re-compile software code (NO need for P4 code)
		- Under {main client} and {secondary client}, run `bash scripts/local/makeclient.sh`
		- Under {first server} and {second server}, run `bash scripts/local/makeserver.sh`
		- Under {switch}, run `bash scripts/local/makeswithos.sh`

## 4.2 Static Workload (Server Rotation)

- Decide {workload} (e.g., workloada) and {method} (e.g., farreach) to use
	- Note: we assmue that you have analyzed {workload} to get {bottleneck serveridx} for your {server rotation scale}
		- If not, please refer to [Section 2.2](#22-workload-analysis--dump-keys)
		- As bottleneck server index is fixed for a given <{workload}, {server rotation scale}>, you can directly refer to the appendix table in [Section 6.1](#61-bottleneck-index-for-server-rotation)
	- Note: we assume that you have already compiled code enabling server rotation
		- If not, please refer to step 7 in [Section 4.1](#41-dynamic-workload-no-server-rotation) mentioned before
	- Note: the scripts in scripts/exps/ (except run_exp_dynamic.sh and run_exp_snapshot.sh) include all the following steps

</br>

- Step 1: prepare ini config files (under {main client})
	- Update settings in the config file {method}/config.ini (e.g., farreach/config.ini):
		- Set workload_name={workload}
		- Set workload_mode=0
		- Set bottleneck_serveridx_for_rotation={bottleneck serveridx}
		- Set server_total_logical_num_for_rotation={server rotation scale}
		- NOTE: {method}/config.ini must have the correct {bottleneck serveridx} and {server rotation scale}
			- Otherwise, client-side PregeneratedWorkload will issue the requests of incorrect partitions (corresponding to non-running servers) and hence timeout
	- Set DIRNAME as {method} in scripts/common.sh
    - Double-check the global testbed settings in scripts/global.sh based on your testbed

</br>

- Step 2: prepare for launching switch data plane and switch OS
	- Under {main client}, run `bash scripts/remote/prepare_server_rotation.sh`
		- This script can generate and sync a new {method}/config.ini based on the existing {method}/config.ini with the configurations you set in step 1
			- The main changes in the new {method}/config.ini is that it sets server0::server_logical_idxes as ${bottleneck serveridx} (e.g., 14), and sets server1::server_logical_idxes as all other serveridxes except ${bottleneck serveridx} (e.g., 0:1:2:3:4:5:6:7:8:9:10:11:12:13:15)
		- The goal is that {switch} can use the new ${method}/config.ini to configure packet forwarding rules, such that we do NOT need to re-launch switch during server rotation

</br>

- Step 3: launch switch data plane and switch OS
	- Create two terminals in {switch}
	- Launch switch data plane in the first terminal
		- `cd {method}/tofino` 
		- `su`
		- Run `bash start_switch.sh`, which will open a CLI
	- Launch switch OS and other daemon processes (for cache management and snapshot generation) in the second terminal
		- `cd {method}`
		- `su`
		- `bash localscripts/launchswitchostestbed.sh`
	- NOTE: if you encounter any problem, you can check the log files of {method}/tmp_\*.out and {method}/tofino/tmp_\*.out in {switch}

</br>

- Step 4: launch servers and clients with server rotation
	- Under {main client}, run `bash scripts/remote/test_server_rotation.sh`
		- Phase 1: test_server_rotation.sh invokes scripts/remote/test_server_rotation_p1.sh to run the first iteration (the bottleneck partition is deployed into {first server})
		- Phase 2: test_server_rotation.sh invokes scripts/remote/test_server_rotation_p2.sh to run the ith iteration (the bottleneck partition is deployed into {first server}, and the ith non-bottleneck partition is deployed into {second server}), where 1 <= i <= {server rotation scale}-1
	- Under {main client}, perform a single iteration for each failed iteration if any
		- If strid=server-x is missed, run `bash scripts/remote/test_server_rotation_p1.sh 1`
		- If strid=server-x-y is missed, run: `bash scripts/remote/test_server_rotation_p2.sh 1 y`


</br>

- Step 5: cleanup testbed
	- Under {switch}
		- Stop switch data plane in the CLI of the first terminal
			- Type exit and press enter
			- If CLI is not closed, type Ctrl+C
		- Stop switch OS and other daemon processes in the second terminal
			- `cd {method}`
			- `su`
			- `bash localscripts/stopswitchtestbed.sh`
	- Under {main client}, run `bash scripts/remote/stopservertestbed.sh` to stop servers
	- Note: if some processes are still NOT stopped, under {main client}, you can run `bash scripts/remote/stopall.sh`

</br>

- Step 6: aggregate statistics
   - Under {main client}, run `bash scripts/remote/calculate_statistics.sh`

# 5. Aggregate Statistics

## 5.1 Scripts 

- We provide the following scripts to help aggregate statistics
	- [calculate_statistics.sh](scripts/remote/calculate_statistics.sh): calculate throughput and latency with or without server rotation
	- [calculate_bwcost.sh](scripts/remote/calculate_bwcost.sh): calculate control-plane bandwidth cost
	- [calculate_recovery_time.sh](scripts/remote/calculate_recovery_time.sh): calculate crash recovery time
- NOTEs
	- If you use [automatic way in Section 3](#3-running-experiments-automatic-evaluation) for evaluation
		- As the run_exp_\* scripts have aggregated the statistics automatically, you can redirect stdout of the script into a file and find aggregated results in the file
		- For example, after `nohup bash scripts/exps/run_exp_ycsb.sh >tmp.out 2>&1 &`, you can find aggregated results in tmp.out
	- If you use [manual way in Section 4](#4-running-workloads-manual-evaluation) for evaluation
		- You can follow [Section 5.2](#52-usage-and-example) to run a script (e.g., calculate_statistics.sh) and get the corresponding gggregated statistics
		- The scripts will aggregate the statistics based on the settings in scripts/global.sh, scripts/common.sh, and {method}/config.ini

## 5.2 Usage and Example

- Calculate throughput and latency with server rotation yet without target throughput
	- Under {main client}, run `bash scripts/remote/calculate_statistics.sh 0`
	- Supported experiments: exp1, exp3, exp4, exp5, and exp6
	- Output example:
	```bash
		...
		[STATIC] average bottleneck totalthpt: 0.092875 MOPS; switchthpt: 0.0245 MOPS; serverthpt: 0.0675625 MOPS
		[STATIC] aggregate throughput: 1.31126577666 MOPS; normalized throughput: 19.4081891088, imbalanceratio: 1.01388888889
		[STATIC] average latency 286.901026111 us, medium latency 85 us, 90P latency 584 us, 95P latency 1717 us, 99P latency 2597 us
		...
	```

</br>

- Calculate throughput and latency with server rotation and with target throughput
	- Under {main client}, run `bash scripts/remote/calculate_statistics.sh 1`
	- Supported experiment: exp2
	- Output example:
	```bash
		...
		[STATIC] average bottleneck totalthpt: 0.0173125 MOPS; switchthpt: 0.00975 MOPS; serverthpt: 0.006875 MOPS
		[STATIC] aggregate throughput: 0.190073026316 MOPS; normalized throughput: 27.6469856459, imbalanceratio: 1.0
		[STATIC] average latency 94.8354988254 us, medium latency 57 us, 90P latency 90 us, 95P latency 123 us, 99P latency 1123 us
		...
	```

</br>

- Calculate throughput and latency of dynamic workload
	- Under {main client}, run `bash scripts/remote/calculate_statistics.sh 0`
	- Supported experiments: exp7, exp8
	- Output example:
	```bash
		...
		[DYNAMIC] per-second statistics:
		thpt (MOPS): [0.178, 0.245, ... , 0.215]
		normalized thpt: [3.2962962962962963, 3.310810810810811, ... , 3.2575757575757573]
		imbalanceratio: [1.0, 1.0, ... , 1.0153846153846153]
		avg latency (us): [497.21625182852614, 517.8129587343011, ... , 501.72249302450666]
		medium latency (us): [120, 95, ... , 132]
		90P latency (us): [1487, 1571, ... , 1579]
		95P latency (us): [1535, 1610, ... , 1642]
		99P latency (us): [1614, 1687, ... , 1729]
		[DYNAMIC][OVERALL] avgthpt 0.228106666667 MOPS, avelat 0 us, medlat 0 us, 90Plat 0 us, 95Plat 0 us, 99Plat 0 us
		...
	```

</br>

- Calculate control-plane bandwidth cost 
	- Under {main client}, run `bash scripts/remote/calculate_bwcost.sh`
	- Supported experiment: exp8
	- Output example:
	```bash
		perserver avglocalbwcost: [0.18816512500000002, 0.18981975, ... s, 0.19562]
		average bwcost of entire control plane: 4.18830950595 MiB/s
	```

</br>

- Calculate crash recovery time
	- Under {main client}, run `bash scripts/remote/calculate_recovery_time.sh <roundnumber>`
	- Supported experiment: exp9
	- Output example:
	```bash
		Server collect time: 1.0 s
		Server preprocess time: 0.016991 s
		Server replay time: 0.0106864375 s
		Server total recovery time: 1.0276774375 s
		Switch collect time: 0.9605 s
		Switch replay time: 0.338202 s
		Switch total recovery time: 1.298702 s
	```

# 6 Appendix

## 6.1 Bottleneck Index for Server Rotation

| Workload Name | Scale | Bottleneck Serveridx |
| :-----------: | :---: | :------------------: |
| workload-load |  16   |          13          |
|   workloada   |  16   |          14          |
|   workloadb   |  16   |          14          |
|   workloadc   |  16   |          14          |
|   workloadd   |  16   |          15          |
|   workloadf   |  16   |          14          |
|   synthetic   |  16   |          14          |
| synthetic-\*  |  16   |          14          |
| valuesize-\*  |  16   |          14          |
|  skewness-90  |  16   |          8           |
|  skewness-95  |  16   |          8           |
|    uniform    |  16   |          5           |
|   workloada   |  32   |          29          |
|   workloada   |  64   |          59          |
|   workloada   |  128  |         118          |

## 6.2 Other NOTEs

- We haved changed parameters of some workload profiles in benchmark/ycsb/workloads/ for sysnthetic workloads
	+ For write ratio (e.g., 25%), change readproportion and updateproportion in workloads/synthetic as workloads/synthetic-XXX (e.g., workloads/synthetic-25)
	+ For value size (e.g., 32), change fieldlength in workloads/synthetic as workloads/valuesize-XXX (e.g., workloads/valuesize-32)
	+ For skewness (e.g., 0.9), change requestdistribution and zipfianconstant in workloads/synthetic as workloads/skewness-XXX (e.g., workloads/skewness-0.9) and workloads/uniform

</br>

- Paths for raw statistics and aggregated results
	- Under static pattern with server rotation
		- {main client} and {secondary client} should dump raw statistics into benchmark/output/{workloadname}-statistics/{method}-static{server rotation scale}-client{physicalidx}.out (e.g., benchmark/output/workloada-statistics/farreach-static16-client0.out)
	- Under dynamic pattern without server rotation
		- {main client} and {secondary client} should dump raw statistics into benchmark/output/{workloadname}-statistics/{method}-{dynamic pattern}-client{physicalidx}.out (e.g., benchmark/output/synthetic-statistics/farreach-hotin-client0.out)
	- If you use manual way as in [Section 4](#4-running-workloads-manual-evaluation) for evaluation
		- As the paths of raw statistics do NOT have other parameter information (e.g., write ratio or skewness), **you need to aggregate the raw statistics before running the next experiment, which may overwrite them**
			- You can refer to [Section 5](#5-aggregate-statistics) to get aggregated results for the current experiment
			- You can also backup the raw statistics files for the current experiment if necessary, which will be overwritten next time
		- Note: the scripts of automatic way for evaluation in [Section 3](#3-running-experiments-automatic-evaluation) will automatically aggregate and backup raw statistics, so you do NOT need to do it manually
