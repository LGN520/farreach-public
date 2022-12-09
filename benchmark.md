# README for benchmark (running guide)

### Method
farreach / nocache / netcache

### Machine Requirements in Our Testbed:</br>
- 1 Main Client </br>
- 1 Secondary Client </br>
- 2 Servers </br>
- 1 Physical Switch </br>

</br>

Table of Contents
====================================
1. System Preperation
   1. [Environment Installation](#11-environment-installation)
   2. [Code Compilation](#12-code-compilation)
   3. [Testbed Configuration](#13-testbed-configuration)

</br>

2. Data Preperation
   1. [Loading Procedure](#21-loading-phase)
   2. [Keydump Procedure](#22-workload-analysis--dump-keys)

</br>

3. Running Experiments (Automatic Evaluation)
   1. [Regular Experiments](#31-regular-experiments)
   2. [Makeup Static Single Rotation](#32-makeup-static-single-rotation)

</br>

4. Running Workloads (Manual Evaluation)
   1. [Dynamic Workload](#41-evaluate-dynamic-workload)
   2. [Static Workload](#42-evaluate-static-workload-server-rotation)

</br></br>

Contents
====================================
## 1.1 Environment Installation

- Install boost
	+ `sudo apt-get install libboost-all-dev`
- Install Maven and Java
	+ Please use openjdk-8/11 to avoid version-related issues
- Tofino compiler version: 8.9.1
- Use gcc-7.5.0 and g++-7.5.0
- Use RocksDB 6.22.1
<!-- - ONLY for multi-switch setting: Install bpftools for spine reflector -->
<!--	+ `git clone https://github.com/cloudflare/bpftools.git` -->
<!--	+ `cd bpftools` -->
<!--	+ `make` -->
<!-- - DEPRECATED: Disable THP to reduce write_memtable_time in rocksdb -->
<!--	+ `sudo apt-get install hugepages` -->
<!--	+ `sudo hugeadm \-\-thp-never` -->
<!--	+ `cat /sys/kernel/mm/transparent_hugepage/enabled` to check -->

</br>

## 1.2 Code compilation

- Update scripts/global.sh for your testbed accordingly
	+ Set specific USER (username), XXX_PRIVATEKEY, MAIN/SECONDARY_CLIENT, and XXX_ROOTPATH (directory path)
- For each {method} (e.g., farreach)
	+ Under the main client
		* Set DIRNAME as the {method} in scripts/common.sh
		* `bash scripts/remote/sync.sh` to sync code of the method to all machines (NOTE: rocksdb-6.22.1/ only needs to be synced at most once)
	+ Under each server (NOT including client/switch), compile RocksDB (NOTE: only need to compile once)
		+ `sudo apt-get install libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev`
		+ `cd rocksdb-6.22.1`
		+ `PORTABLE=1 make static_lib`
			* Comment -Wstrict-prototype and -Werror if compilation fails due to strict-alias warning
			* Add PORTABLE=1 if with runtime error of illegal instruction when open()
			* Possible error: compression type Snappy is not linked with the binary
				- Install libsnappy and remake static lib of rocksdb
				- Prepare the directory for database in advance
	+ Compile software code
		* Manual way
			- Under each client, run `bash scripts/local/makeclient.sh`
			- Under each switch, run `bash scripts/local/makeswitch.sh`
			- Under each server, run `bash scripts/local/makeserver.sh`
		* Automatic way (NOT used now due to slow sequential compialtion)
			- Under the main client, `bash scripts/remote/makeremotefiles.sh` to make C/C++ and java code including client, switchos, controller, and server
		* NOTE: if with "make: warning:  Clock skew detected.  Your build may be incomplete" during make, run `find . -type f | xargs touch` and then re-make files
	+ Compile hardware code (NOT need to run now as we have compiled hardware code)
		* Under each Tofino OS, enter directory of {method}/tofino/
			- `bash compile.sh` (NOTE: if we have already compiled for all methods, we do NOT need to run this command unless we change in-switch implementation)

</br>

## 1.3 Testbed Configuration
- Building your testbed
	+ Modify scripts/local/configure_\<client/server/switchos\>.sh based on your own testbed settings
	+ In each physical client, run `bash scripts/local/confiugre_client.sh`
	+ In each physical server, run `bash scripts/local/confiugre_server.sh`
	+ In each physical switch, run `bash scripts/local/confiugre_switchos.sh`

</br></br>

## 2.1 Loading Phase
- Under the main client, perform the loading phase and backup for evaluation time reduction
	+ Run `bash scripts/remote/prepare_load.sh`, which will copy recordload/config.ini to switch/server::nocache/config.ini
	+ In switch, launch and configure nocache switch by nocache/tofino/start_switch.sh and nocache/localscripts/launchswitchtestbed.sh
	+ Run `bash scripts/remote/load_and_backup.sh`, which will launch and kill servers automatically
    	+ NOTE: at the end of load_and_backup.sh, we copy nocache/config.ini to resume switch/server::nocache/config.ini

</br>

## 2.2 Workload Analysis & Dump Keys
- Under the main client
  - Update `<workload_name>` with the workload in file `keydump/config.ini`
  - (Recommend) Automatic way
    - Run `bash scripts/remote/keydump_and_sync.sh`, which will sync required files to clients/server after keydump
  - (DEPRECATED) Manual way 
    - `cd benchmark/ycsb; ./bin/ycsb run keydump; cd ../../`
  		+ Dump hottest/nearhot/coldest keys into `benchmark/output/<workloadname>-\*.out` for warmup phase later
  		+ Dump bottleneckt serveridx for different server rotation scales for server rotation later
  		+ Pre-generate workload files in `benchmark/output/<workloadname>-pregeneration/` for server rotation later
  	- `cd benchmark/ycsb; python generate_dynamicrules.py <workloadname>; cd ../../`
  		+ Generates key popularity changing rules in `benchmark/output/<workloadname>-\*rules/` for dynamic pattern later
  	- `bash scripts/remote/deprecated/synckeydump.sh <workloadname>` to sync the above files of the workload to another client

</br></br>

## 3.1 Regular Experiments
To carry out experiments according to paper, we have set up the scripts for running specific tasks. The scripts are placed under `scripts/exps/`.
1. <u>[Under Main Client]</u> Prepare Configuration
   - Under `scripts/global.sh`, check on configurations:
     - `SWITCH_PRIVATEKEY`: path to private key which will be used for remote connection to switch machine with root access;
     - `CONNECTION_PRIVATEKEY`: path to private key which will be used for remote connection to other server/client machines;
     - `EVALUATION_SCRIPTS_PATH`: path to these experiment scripts;
     - `EVALUATION_OUTPUT_PREFIX`: path to the generated raw output
2. <u>[Under Main Client]</u> Run Experiments
   - To eliminate influence by rocksdb performance, the scripts target to run experiments for multiple round. Every time running the experiments, we need to define the specific round number `<roundnumber>` for running these experiment.
   - For a specific experiment (exp_ycsb for example), run its corresponding scripts by `bash scripts/exps/run_exp_ycsb.sh<roundnumber>`.
3. Experiments List and Scripts
   | Exp # | Scripts |
   | :---: | :---: |
   | 1 | [run_exp_ycsb.sh](scripts/exps/run_exp_ycsb.sh) |
   | 2 | [run_exp_latency.sh](scripts/exps/run_exp_latency.sh) | 
   | 3 | [run_exp_scalability.sh](scripts/exps/run_exp_scalability.sh) | 
   | 4 | [run_exp_write_ratio.sh](scripts/exps/run_exp_write_ratio.sh) |
   | 5 | [run_exp_key_distribution.sh](scripts/exps/run_exp_key_distribution.sh) |
   | 6 | [run_exp_value_size.sh](scripts/exps/run_exp_value_size.sh) |
   | 7 | [run_exp_dynamic.sh](scripts/exps/run_exp_dynamic.sh) |
   | 8 | [run_exp_snapshot.sh](scripts/exps/run_exp_snapshot.sh) |
   | 9 | [run_exp_recovery.sh](scripts/exps/run_exp_recovery.sh) |

- NOTEs for exp9
	- Error messages for scp in farreach/localscripts/fetch\*.sh
		- If you have an error message of `hot key verification failed`, check the ssh connectivity between the switch and all clients/servers
		- If you have an error message of `permission denied`, check the correctness of ownership for /tmp/farreach in the switch and servers
		- If you have an error message of `permission denied (public key)`, check whether you spefcify the correct private key in the switch such that it can copy files from clients/servers
	- If you want to test recovery time based on existing recovery information instead of running server rotations again
		- TODO

4. Aggregate statistics
- Scripts
	- [calculate_statistics.sh](scripts/remote/calculate_statistics.sh): calculate throughput and latency
	- [calculate_bwcost.sh](scripts/remote/calculate_bwcost.sh): calculate bandwidth cost
	- [calculate_recovery_time_helper.py](scripts/local/calculate_recovery_time_helper.py): calculate recovery time
- Usage
	- TODO

</br></br>

## 3.2 Makeup Static Single Rotation
During experiments with static pattern, single rotation failure may happen due to database performance fluctuate or unstable network. We also provide scripts to run one single rotation from server rotations experiements to avoid re-run the whole experiment.
1. <u>[Under Main Client]</u> Applicable Experiments
   - All clients, servers and switchs are configured to be running static workload. The script applies for experiments:
     - exp_key_distribution
     - exp_latency
     - exp_scalability
     - exp_value_size
     - exp_write_ratio
     - exp_ycsb
2. <u>[Under Main Client]</u> Run makeup single rotation
   - Run experiment with command: `bash scripts/exps/run_makeup_rotation_exp.sh <expname> <roundnumber> <methodname> <workloadname> <serverscale> <bottleneckidx> <targetrotation> [targetthpt]`
      - `expname`: experiment name (eg.: exp1)
      - `roundnumber`: experiment round number (eg.: 1)
      - `methodname`: experiment method (eg.: farreach)
      - `workloadname`: workload name (eg.: workloada)
      - `serverscale`: number of servers for rotation (eg.: 16)
      - `bottleneckidx`: bottleneck server index for rotation with this workload (eg.: 14)
      - `targetrotation`: makeup rotation index (eg.: 10)
      - `targetthpt`: throughput target of this rotation, only applicable for exp2.

</br></br>

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
  

</br>
</br>

Appendix
====================================

## Static Workload Server Bottleneck Index
| Workload Name | Scale | Bottleneck Serveridx |
| :---: | :---: | :---: |
| workload-load | 16 | 13 |
| workloada | 16 | 14 |
| workloadb | 16 | 14 |
| workloadc | 16 | 14 |
| workloadd | 16 | 15 |
| workloadf | 16 | 14 |
| synthetic | 16 | 14 |
| synthetic-\* | 16 | 14 |
| valuesize-\* | 16 | 14 |
| skewness-90 | 16 | 14 |
| skewness-95 | 16 | 14 |
| uniform | 16 | 8 |
| workloada | 32 | 29 |
| workloada | 64 | 59 |
| workloada | 128 | 118 |

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
