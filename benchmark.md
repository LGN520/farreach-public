# README of FarReach

Table of Contents
====================================

0. [Overview](#0-overview)
1. System Preperation
   1. [Dependency Installation](#11-dependency-installation)
   2. [Configuration settings](#12-confiugration-settings)
   3. [Code Compilation](#13-code-compilation)
   4. [Testbed Building](#14-testbed-building)
2. Data Preperation
   1. [Loading Phase](#21-loading-phase)
   2. [Workload Analysis & Dump Keys](#22-workload-analysis--dump-keys)
3. Running Experiments (Automatic Evaluation)
   1. [Regular Experiments](#31-regular-experiments)
   2. [Perform Static Single Rotation](#32-perform-static-single-rotation)
4. Running Workloads (Manual Evaluation)
   1. [Dynamic Workload](#41-evaluate-dynamic-workload)
   2. [Static Workload](#42-evaluate-static-workload-server-rotation)
5. Aggregate Statistics
   1. [Aggregate Scripts](#51-aggregate-scripts)
   2. [Scripts Usage and Example](#52-scripts-usage-and-example)

Contents
====================================

## 0 Overview

### Methods

- farreach: our in-switch write-back caching
- nocache: a baseline without in-switch caching
- netcache: a baseline with in-switch write-through caching

### Our Testbed

- You can follow our settings below to build your own testbed

</br>

- Machine requirements
	+ Four physical machines with Ubuntu 16.04/18.04
		* One main client (hostname: dl11)
		* One secondary client (hostname: dl20)
		* Two servers (hostnames: dl21 and dl30)
			- Note: controller is co-located in the first server (dl21)
	+ One 2-pipeline Tofino switch with SDE 8.9.1 (hostname: bf3)
		* **Note: larger version (e.g., 9.0.1) cannot support P4\_14 correctly due to compiler its own bugs**

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

# System Preperation

## 1.1 Dependency Installation

- Install libboost 1.81.0 into project directory
	+ Under project directory (e.g., /home/ssy/projects/farreach-public)
	+ `wget https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.tar.gz`
	+ `tar -xzvf boost_1_81_0.tar.gz`
	+ `cd boost_1_81_0; ./bootstrap.sh --with-libraries=system,thread --prefix=./install && sudo ./b2 install`
	<!-- + `sudo apt-get install libboost-all-dev` -->
- Install Maven 3.3.9 if not
- Install Java OpenJDK-8 if not
	+ **Configure JAVA_HOME**: add sth like `export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64/` (based on your own JAVA path) to ~/.bashrc
- Install Tofino SDE 8.9.1 if not
	+ **Note: larger version (e.g., 9.0.1) cannot support P4\_14 correctly due to compiler its own bugs**
- Install gcc-7.5.0 and g++-7.5.0 if not
- Use RocksDB 6.22.1 (already embedded in the project; not need extra installation)

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
	+ SWITCH\_PRIVATEKEY: the passwd-free ssh key used by {main client} to connect with {switch} as **root user** (e.g., .ssh/switch-private-key)
	+ CONNECTION\_PRIVATEKEY: the passwd-free ssh key used by {switch} to connect with two servers (for maxseq and in-switch snapshot), by two servers to connect with two clients (for client-side backups), and by {second server} to connect with {first server} (controller) (for in-switch snapshot) as **non-root user** (e.g., .ssh/connection-private-key)
	+ MAIN\_CLIENT: hostname of {main client} (e.g., dl11)
	+ SECONDARY\_CLIENT: hostname of {secondary client} (e.g., dl20)
	+ SERVER0: hostname of {first server} co-located with controller (e.g., dl21)
	+ SERVER1: hostname of {second server} (e.g., dl30)
	+ LEAFSWITCH: hostname of the Tofino {switch} (e.g., bf3)
	+ CLIENT/SWITCH/SERVER\_ROOTPATH: project directory path in clients/switch/servers (e.g., /home/ssy/projects/farreach-public)
    + EVALUATION\_OUTPUT\_PREFIX: path to store the raw statistics of each experiment for aggregate analysis
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
			- CONTROLLER_LOCAL_IP: the IP address of NIC for {controller} connecting to {switch} (same as SERVER0_TOSWITCH_IP in our testbed)
		* Switch
			- SWITCHOS_LOCAL_IP: the IP address of NIC for {switch} OS connecting to the local area network (NOT bypass {switch} data plane)
			- SWITCH_RECIRPORT_PIPELINE1TO0: the front panel port in {switch} data plane for pipeline 1 to connect with pipeline 0 for in-switch recirculation
			- SWITCH_RECIRPORT_PIPELINE0TO1: the front panel port in {switch} data plane for pipeline 0 to connect with pipeline 1 for in-switch recirculation
	+ CPU settings
		* First server
			- SERVER0_WORKER_CORENUM: the number of CPU cores specifically used for processing requests in first server
			- SERVER0_TOTAL_CORENUM: the total number of CPU cores in first server
		* Second server
			- SERVER1_WORKER_CORENUM: the number of CPU cores specifically used for processing requests in second server
			- SERVER1_TOTAL_CORENUM: the total number of CPU cores in second server

</br>

- Run `bash scripts/local/update_config_files.sh` to update ini configuration files based on network and CPU settings

</br>

- Update SSH configuration settings
	+ In any of above machines (2 clients, 2 servers, and 1 switch), if you need to manually type yes/no to check the hostname when using ssh command to connect other machines, add the following content to `~/.ssh/config` under the machine:
	```
	Host *
		StrictHostKeyChecking no
	```
	+ Generate SWITCH_PRIVATEKEY
		* Under {main client}, if the ssh key has not been created, run `sudo ssh-keygen -t rsa -f ~/.ssh/switch-private-key` (**use empty passwd for no passphrase**)
			- Also run `sudo chown {USER}:{USER} ~/.ssh/switch-private-key` (use your Linux username) to change the owner
		* Append the content of ~/.ssh/switch-private-key.pub of {main client}, into /home/root/.ssh/authorized_keys of {switch}
	+ Generate CONNECTION_PRIVATEKEY
		* Consider the following source-connectwith-destination pairs:
			- {switch}-connectwith-{first server} and {switch}-connectwith-{second server}
			- {first server}-connectwith-{main client}, {first server}-connectwith-{secondary client}, {second server}-connectwith-{main client}, and {second server}-to-{secondary client}
			- {second server}-connectwith-{first server} (controller is co-located with {first server})
		* For each pair of source connecting with destination
			- Under {source}, if the ssh key has not been created, run `ssh-keygen -t rsa -f ~/.ssh/connection-private-key` (**use empty passwd for no passphrase**)
			- Append the content of ~/.ssh/connection-private-key.pub of {source}, into ~/.ssh/authorized_keys of {destination}

## 1.3 Code Compilation

- Sync and compile (**TIME: around 3 hours**) RocksDB (ONLY need to perform once)
	+ Under {main client}, run `bash scripts/sync_kvs.sh` to sync rocksdb-6.22.1/ to each server
	+ Under {first server} and {second server}, compile RocksDB
		+ Run `sudo apt-get install libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev libjemalloc-dev libsnappy-dev` to install necessary dependencies for RocksDB
		+ `cd rocksdb-6.22.1`
		+ `PORTABLE=1 make static_lib`
			* We already comment -Wstrict-prototype and -Werror in Makefile to fix compilation failures due to strict-alias warning
			* We use PORTABLE=1 to fix runtime error of illegal instruction when open()
		+ For each method (farreach or nocache or netcache)
			* Run `mkdir /tmp/{method}` to prepare the directory (e.g., /tmp/farreach) for database in advance

</br>

- For each {method}
	+ Options of method name: farreach, nocache, or netcache
	+ Under {main client}
		* Set DIRNAME as {method} in scripts/common.sh
		* `bash scripts/remote/sync.sh` to sync code of the method to all machines
	+ Compile software code
		* Under each client, run `bash scripts/local/makeclient.sh` (TIME: around 10 minutes especially for the first time with Maven downloading time)
		* Under each switch
			- Run `bash scripts/local/makeswitchos.sh` to make switch OS (TIME: around 1 minute)
			- Run `su` to enter root mode, and run `cd {method}/tofino; bash compile.sh` to make P4 code (**TIME: around 3 hours**)
				+ Note: if you have compiled P4 code of {method} before, you do NOT need to re-compile it agina. If you really want to re-compile it (maybe due to P4 code modification), you should delete the corresponding directory (netbufferv4, nocache, or netcache) under $SDE/pkgsrc/p4-build/tofino/ before re-compilation.
		* Under each server, run `bash scripts/local/makeserver.sh` (TIME: around 5 minutes)
		* NOTE: if with "make: warning:  Clock skew detected.  Your build may be incomplete" during make, run `find . -type f | xargs touch` and then re-make files

<!--
* Automatic way (NOT used now due to very slow sequential compialtion) vs. makeclient/server/switchos.sh
	- Under the main client, `bash scripts/remote/makeremotefiles.sh` to make C/C++ and java code including client, switchos, controller, and server
-->

## 1.4 Testbed Building

- Building your testbed basedon network settings provided by scripts/global.sh
	+ In {main client}, run `bash scripts/local/confiugre_client.sh 0`
	+ In {secondary client}, run `bash scripts/local/configure_client.sh 1`
	+ In {first server}, run `bash scripts/local/confiugre_server.sh 0`
	+ In {second server}, run `bash scripts/local/confiugre_server.sh 1`
	+ In {switch} OS, run `bash scripts/local/confiugre_switchos.sh`

# Data Preperation

## 2.1 Loading Phase

- Perform the loading phase and backup for evaluation time reduction (ONLY need to perform once)
	+ Under {main client}
		* Set DIRNAME as nocache in scripts/common.sh and run `bash scripts/remote/sync_file.sh scripts common.sh` to sync scripts/common.sh
		* Run `bash scripts/remote/prepare_load.sh` to copy recordload/config.ini to overwrite nocache/config.ini in all clients, servers, and switch
	+ Under {switch}
		* Run `bash nocache/tofino/start_switch.sh` to launch nocache switch, which will open a CLI terminal
		* Run `cd nocache; bash localscripts/launchswitchtestbed.sh` to configure nocache switch
	+ Under {main client} (**TIME: around 30 minutes**)
		* Run `bash scripts/remote/load_and_backup.sh` to launch and kill servers and clients automatically for loading and backup
			- Note: scripts/remote/load_and_backup.sh will resume the original nocache/config.ini in all clients, server, and switch after all steps
	+ Under {switch}
		* Type Ctrl+C in the CLI terminal to stop switch
		* Run `cd nocache; bash localscripts/stopswitchtestbed.sh` to clear switch testbed

## 2.2 Workload Analysis & Dump Keys

- Analyze workloads to dump hot keys, dynamic patterns, and bottleneck partition (ONLY need to perform once)
	- For each {workload} (TIME: around 5 minutes per workload)
		- Options of workload name: workloada, workloadb, workloadc, workloadd, workloadf, workload-load, synthetic, synthetic-25, synthetic-75, skewness-90, skewness-95, uniform, valuesize-16, valuesize-32, valuesize-64
		- Under {main client}
			- Update workload\_name as {workload} in keydump/config.ini
			- Run `bash scripts/remote/keydump_and_sync.sh` to dump workload-related information and sync to all clients, servers, and switch
  
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

# Running Experiments (Automatic Evaluation)

## 3.1 Regular Experiments

- To carry out experiments according to paper, we have set up the scripts for running specific tasks. The scripts are placed under `scripts/exps/`.
	+ **Note that due to server rotation to cope with limited machines, each number may take 1-8 hour(s), and each experiment will test multiple numbers.**

</br>

- Scripts for different experiments

   | Exp # |                                 Scripts                                 | Description |
   | :---: | :---------------------------------------------------------------------: | --- |
   |   1   |             [run_exp_ycsb.sh](scripts/exps/run_exp_ycsb.sh)             | Throughput analysis for different YCSB core workloads |
   |   2   |          [run_exp_latency.sh](scripts/exps/run_exp_latency.sh)          | Latency analysis for different target throughputs |
   |   3   |      [run_exp_scalability.sh](scripts/exps/run_exp_scalability.sh)      | Scalability for different # of simulated servers |
   |   4   |      [run_exp_write_ratio.sh](scripts/exps/run_exp_write_ratio.sh)      | Synthetic workloads with different write ratios |
   |   5   | [run_exp_key_distribution.sh](scripts/exps/run_exp_key_distribution.sh) | Synthetic workloads with different key distributions |
   |   6   |       [run_exp_value_size.sh](scripts/exps/run_exp_value_size.sh)       | Synthetic workloads with different value sizes |
   |   7   |          [run_exp_dynamic.sh](scripts/exps/run_exp_dynamic.sh)          | Synthetic workloads with different dynamic workload patterns |
   |   8   |         [run_exp_snapshot.sh](scripts/exps/run_exp_snapshot.sh)         | Performance and control-plane bandwidth overhead of snapshot generation |
   |   9   |         [run_exp_recovery.sh](scripts/exps/run_exp_recovery.sh)         | Crash recovery time |

</br>

- Other useful scripts
	+ scripts/remote/stopall.sh: forcely to stop and kill clients, servers (including controller and simulated reflector), and switch (including data plane and switch OS).
		- Note: you can run this script to clear remaining threads, if the previous experiment fails due to incorrect configurations

</br>

- Run each {experiment} except exp_dynamic
	- Options: exp_ycsb, exp_latency, exp_scalability, exp_write_ratio, exp_key_distribution, exp_value_size, exp_snapshot, and exp_recovery.
	- Under {main client}, take exp_ycsb as an example
		- Run `bash scripts/exps/run_exp_ycsb.sh <roundnumber>`
		- Note: we run each experiment for multiple rounds to eliminate the effect of runtime variation (e.g., RocksDB fluctuation), so we need to specify <roundnumber> to indicate the index of current round.

</br>

- Run exp_dynamic (NOT use server rotation)
	- Under {main client}
		- Comment line 82 (#define SERVER_ROTATION) in common/helper.h to diable server rotation
		- `bash scripts/remote/sync_file.sh common helper.h`
	- Re-compile software code (NO need for P4 code)
		- Under {main client} and {secondary client}, run `bash scripts/local/makeclient.sh`
		- Under {first server} and {second server}, run `bash scripts/local/makeserver.sh`
		- Under {switch}, run `bash scripts/local/makeswithos.sh`
	- Run `bash scripts/exps/run_exp_dynamic.sh <roundnumber>`
	- After running all rounds of exp_dynamic, as most experiments use server rotation for static workload pattern instead of dynamic workload patterns
		- Under {main client}
			- Uncomment line 82 (#define SERVER_ROTATION) in common/helper.h to enable server rotation
			- `bash scripts/remote/sync_file.sh common helper.h`
		- Re-compile software code (NO need for P4 code)
			- Under {main client} and {secondary client}, run `bash scripts/local/makeclient.sh`
			- Under {first server} and {second server}, run `bash scripts/local/makeserver.sh`
			- Under {switch}, run `bash scripts/local/makeswithos.sh`

</br>

- Possible errors for exp_recovery due to testbed mis-configurations
	- Error messages for scp in farreach/localscripts/fetch\*.sh
		- If you have an error message of `hot key verification failed`, check the ssh connectivity between {switch} and all clients and servers
		- If you have an error message of `permission denied`, check the correctness of ownership for /tmp/farreach in {switch} and two servers
		- If you have an error message of `permission denied (public key)`, check whether you spefcify the correct private key in the switch such that it can copy files from clients/servers
	- If you want to test recovery time based on existing recovery-based data instead of running server-rotation-based experiments again, in scripts/global.sh and scripts/exps/run_exp_recovery.sh:
    	- Step 1: make sure `EVALUATION_OUTPUT_PREFIX` points to the path of recovery-related data (in-switch snapshot, client-side backups, and maxseq) generated by previous server-rotation-based experiments
		- Step 2: run `scripts/exps/run_exp_recovery.sh <workloadmode> 1`, where we set the argument <recoveryonly> as 1 to skip the step of server rotations

## 3.2 Perform Static Single Rotation

- During experiments with static pattern, single rotation failure may happen due to database performance fluctuate or unstable network. We also provide scripts to run one single rotation from server rotations experiements to avoid re-run the whole experiment.

</br>

- Under {main client}
	- Launch a single rotation with command: `bash scripts/exps/run_makeup_rotation_exp.sh <expname> <roundnumber> <methodname> <workloadname> <serverscale> <bottleneckidx> <targetrotation> [targetthpt]`
		- `expname`: experiment name (eg.: exp1)
			- Note: expname only determines the path to store generated statistics, yet not affect experiment results
		- `roundnumber`: experiment round number (eg.: 1)
		- `methodname`: experiment method (eg.: farreach)
		- `workloadname`: workload name (eg.: workloada)
		- `serverscale`: number of servers for rotation (eg.: 16)
		- `bottleneckidx`: bottleneck server index for rotation with this workload (eg.: 14)
		- `targetrotation`: makeup rotation index (eg.: 10)
		- `targetthpt`: throughput target of this rotation, only applicable for exp2.
	- Note: the arguments of scripts/exps/run_makeup_rotation_exp.sh are determined by each specific experiment
		- For example, for exp_ycsb, you may pass arguments with expname=exp1, roundnumber=0, methodname=farreach, workloadname=workloada, serverscale=16, bottleneckidx=14, targetrotation=10
	- Note: scripts/exps/run_makeup_rotation_exp.sh does NOT support exp_dynamic, as the experiment of dynamic workload patterns does NOT use server rotation
		- Therefore, this script ONLY works for experiments with server rotation: exp_key_distribution, exp_latency, exp_scalability, exp_value_size, exp_write_ratio, and exp_ycsb

# Running Workloads (Manual Evaluation)

## 4.1 Evaluate Dynamic Workload
Decide {workload} and {method} to use. E.g.: *farreach* and *workloada*.

1.  <u>[Under Main Client]</u> Prepare config files
    - Set file {method}/config.ini:
      - `workload_mode`=1
      - `workload_name`={workload} 
	  - `dynaic_ruleprefix`=hotin/hotout/random
      - `server_physical_num`=2
      - `server0::server_logical_idxes`=0
      - `server1::server_logical_idxes`=1
    - Set file scripts/global.sh accordingly


2. <u>[Under Main Client]</u> Check SERVER_ROTATION option
   - In common/helper.h, disable `SERVER_ROTATION` 
   - Sync code to all machines: `bash scripts/remote/sync.sh`
   - Re-compile clients/servers/switchos as mentioned before


3. <u>[Under Physical Switch]</u> Start switch services
   - Create two terminals in switch machine
   - Start switch on terminal 1 by: 
     - `cd {method}/tofino` 
     - `su`
     - `bash start_switch.sh`
   - Launch switch testbed in terminal 2 by:
     - `cd {method}`
     - `su`
     - `bash localscripts/launchswitchtestbed.sh`

	*NOTE: if encounter any problem, check tmp_\*.out and tofino/tmp_\*.out in switch first*


4. <u>[Under Main Client]</u> Start dynamic evaluation
   - Evaluate by: `bash scripts/remote/test_dynamic.sh` </br>


5. <u>[Under Main Client & Physical Switch]</u> Evaluation cleanup
   - Under physcial switch, stop service by:
     - `cd method`
     - `su`
     - `bash localscripts/stopswitchtestbed.sh`
   - Under main client, run `bash scripts/remote/stopservertestbed.sh`


6. <u>[Under Main Client]</u> Aggregate statistics
   - Calculate aggregated statistics by `bash scripts/remote/calculate_statistics.sh` 

</br></br>

## 4.2 Evaluate Static Workload (Server Rotation)
Decide {workload} and {method} to use. E.g.: *farreach* and *workloada*.

1. <u>[Under Main Client]</u> Prepare Config Files
   - Set file {method}/config.ini:
     - `workload_name`={workload}
     - `workload_mode`=0
     - `bottleneck_serveridx_for_rotation`
     - `server_total_logical_num_for_rotation`

	*NOTE: config.ini must have the correct bottleneckidx and rotation scale, otherwise PregeneratedWorkload will choose the incorrect requests of non-running servers and hence timeout*
   - Set scripts/common.sh accordingly

2. <u>[Under Main Client]</u> Check SERVER_ROTATION option
   - In common/helper.h, disable `SERVER_ROTATION` 
   - Sync code to all machines: `bash scripts/remote/sync.sh`
   - Re-compile clients/servers/switchos as mentioned before

3. <u>[Under Main Client]</u> Setup hot keys and forwarding rules into switch
   - Generate and sync config.ini for setup phase by: `bash scripts/remote/prepare_server_rotation.sh`

4. <u>[Under Physical Switch]</u> Start switch services
   - Create two terminals to switch machine
   - Start switch in terminal 1 by: 
     - `cd {method}/tofino` 
     - `su`
     - `bash start_switch.sh`
   - Launch switch testbed in terminal 2 by:
     - `cd {method}`
     - `su`
     - `bash localscripts/launchswitchtestbed.sh`

	*NOTE: if encounter any problem, check tmp_\*.out and tofino/tmp_\*.out in switch first*

5. <u>[Under Main Client]</u> Start static evaluation by server rotation
   - Start evaluation by `bash scripts/remote/test_server_rotation.sh`
      - Phase 1: the first rotation (physical server 0 runs the bottleneck server)
      - Phase 2: each subsequent rotation (physical server 0 runs the bottleneck server; physical server 1 runs each non-bottleneck server) </br>
   - Make up failed iteration
     - If strid=server-x is missed, run: `bash scripts/remote/test_server_rotation_p1.sh 1`
     - If strid=server-x-y is missed, run: `bash scripts/remote/test_server_rotation_p2.sh 1 y`

6. <u>[Under Main Client & Physical Switch]</u> Evaluation cleanup
   - Under physcial switch, stop service by:
     - `cd method`
     - `su`
     - `bash localscripts/stopswitchtestbed.sh`
   - Under main client, run `bash scripts/remote/stop_server_rotation.sh`

7. <u>[Under Main Client]</u> Aggregate statistics
   - Calculate aggregated statistics by `bash scripts/remote/calculate_statistics.sh` 
  
</br></br>

## 5.1 Aggregate Scripts 
- [calculate_statistics.sh](scripts/remote/calculate_statistics.sh): calculate throughput and latency under static or dynamic workload
- [calculate_bwcost.sh](scripts/remote/calculate_bwcost.sh): calculate bandwidth cost
- [calculate_recovery_time.sh](scripts/remote/calculate_recovery_time.sh): calculate recovery time
- NOTEs
	- If you use [automatic way (3.1 and 3.2)](#31-regular-experiments) to perform evaluation
		- As the exp scripts have aggregated the statistics automatically, you can redirect scripts' output and find aggregate statistics in the output file
		- For example, after `nohup bash scripts/exps/run_exp_ycsb.sh >tmp.out 2>&1 &`, you can find statistics in tmp.out
	- If you use [manual way (4.1 and 4.2)](#41-evaluate-dynamic-workload) to perform evaluation
		- You can run the aggregate scripts (e.g., calculate_statistics.sh) guided in [5.2](#52-scripts-usage-and-example) to get the aggregate statistics
		- The aggregate scripts will calculate the statistics based on the setting in global.sh, common.sh, and {method}/config.ini

</br></br>

## 5.2 Scripts Usage and Example
- Calculate throughput and latency of static workload with no target throughput specified
  - `bash scripts/remote/calculate_statistics.sh 0`
  - Applicable experiments: exp1, exp3, exp4, exp5, exp6, exp9.
  - Output example:
   ```bash
      ...
      [STATIC] average bottleneck totalthpt: 0.092875 MOPS; switchthpt: 0.0245 MOPS; serverthpt: 0.0675625 MOPS
      [STATIC] aggregate throughput: 1.31126577666 MOPS; normalized throughput: 19.4081891088, imbalanceratio: 1.01388888889
      [STATIC] average latency 286.901026111 us, medium latency 85 us, 90P latency 584 us, 95P latency 1717 us, 99P latency 2597 us
   ```
- Calculate throughput and latency of static workload with target aggregate throughput specified
  - `bash scripts/remote/calculate_statistics.sh 1`
  - Applicable experiments: exp2.
  - Output example:
   ```bash
      ...
      [STATIC] average bottleneck totalthpt: 0.0173125 MOPS; switchthpt: 0.00975 MOPS; serverthpt: 0.006875 MOPS
      [STATIC] aggregate throughput: 0.190073026316 MOPS; normalized throughput: 27.6469856459, imbalanceratio: 1.0
      [STATIC] average latency 94.8354988254 us, medium latency 57 us, 90P latency 90 us, 95P latency 123 us, 99P latency 1123 us
   ```

- Calculate throughput and latency of dynamic workload
   - `bash scripts/remote/calculate_statistics.sh 0`
   - Applicable experiments: exp7, exp8.
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
   ```
- Calculate system bandwidth cost 
  - `bash scripts/remote/calculate_bwcost.sh`
  - Applicable experiment: exp8.
  - Output example:
   ```bash
      perserver avglocalbwcost: [0.18816512500000002, 0.18981975, ... s, 0.19562]
      average bwcost of entire control plane: 4.18830950595 MiB/s
   ```

- Calculate system recovery time
  - `bash scripts/remote/calculate_recovery_time.sh <roundnumber>`
  - Applicable experiment: exp9.
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

</br>
</br>

Appendix
====================================

## Static Workload Server Bottleneck Index
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

</br>

## Other NOTEs

- Change parameters of workload profiles in benchmark/ycsb/workloads/
	+ For write ratio, change read/updateproportion in workloads/synthetic as workloadssynthetic-XXX
	+ For value size, change fieldlength in workloads/synthetic as workloads/valuesize-XXX
	+ For skewness, change requestdistribution/zipfianconstant in workloads/synthetic as workloads/skewness-XXX and workloads/uniform
- Dump and aggregate statistics
	- Under static pattern, each physical client should dump statistics into benchmark/output/{workloadname}-statistics/{method}-static{staticscale}-client{physicalidx}.out (e.g., benchmark/output/workloada-statistics/farreach-static16-client0.out) without parameter info
	- NOTE: before changing parameter for the next time of experiment
		+ Run `bash scripts/remote/calculate_statistics.sh` to get results of the current parameter
		+ Backup the statistics files of the current parameter if necessary, which will be overwritten next time

</br>
