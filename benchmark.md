# README for benchmark (running guide)

### Method
farreach / nocache / netcache / distfarreach / distnocache / distcache

### Machine Requirements:</br>
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

3. Running Workloads
   1. [Dynamic Workload](#31-evaluate-dynamic-workloads)
   2. [Static Workload](#32-evaluate-static-workload-single-switch)


</br></br>

Contents
====================================
## 1.1 Environment Installation

- Install boost
	+ `sudo apt-get install libboost-all-dev`
- Install Maven and Java
	+ Please use openjdk-8/11 to avoid version-related issues
- Tofino compiler version: 8.9.1
- ONLY for multi-switch setting: Install bpftools for spine reflector
	+ `git clone https://github.com/cloudflare/bpftools.git`
	+ `cd bpftools`
	+ `make`
- DEPRECATED: Disable THP to reduce write_memtable_time in rocksdb
	+ `sudo apt-get install hugepages`
	+ `sudo hugeadm --thp-never`
	+ `cat /sys/kernel/mm/transparent_hugepage/enabled` to check

</br>

## 1.2 Code compilation

- Update scripts/common.sh for your testbed accordingly
	+ Set specific USER (username), SECONDARY_CLIENT, and XXX_PATH (directory path)
- For each method (e.g., farreach)
	+ Under the main client
		* Set DIRNAME as the method in scripts/common.sh
		* `bash scripts/remote/sync.sh` to sync code of the method to all machines
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
		* Under each Tofino OS, enter directory of method/tofino/ (or method/tofino-spine/ and method/tofino-leaf/)
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
	+ In switch, launch and configure nocache switch by start_switch.sh and launchswitchtestbed.sh in nocache/
	+ Run `bash scripts/remote/load_and_backup.sh`, which will launch and kill servers automatically
    	+ NOTE: at the end of load_and_backup.sh, we copy nocache/config.ini to resume switch/server::nocache/config.ini

</br>

## 2.2 Workload Analysis & Dump Keys
- Under the main client
  - Update `<workload_name>` with the workload in file `keydump/config.ini`
  - (Recommend)Automatic way
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

## 3.1 Evaluate Dynamic Workloads
Decide {workload} and {method} to use. E.g.: *farreach* and *workloada*.

1.  <u>[Under Main Client]</u> Prepare config files
    - Set file {method}/config.ini:
      - `workload_mode`=1
      - `workload_name`={workload} 
      - `server_physical_num`=2
      - `server0::server_logical_idxes`=0
      - `server1::server_logical_idxes`=1
    - Set file scripts/common.sh accordingly


2. <u>[Under Main Client]</u> Check SERVER_ROTATION option
   - In common/helper.h, disable `SERVER_ROTATION` 
   - Sync code to all machines: `bash scripts/remote/sync.sh`
   - Re-compile clients/servers/switchos as mentioned before


3. <u>[Under Physical Switch]</u> Start switch services
   - Create two connection to switch machine
   - Start switch on connection 1 by: 
     - `cd {method}/tofino` 
     - `su`
     - `bash start_switch.sh`
   - Launch switch testbed on connection 2 by:
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

## 3.2 Evaluate Static Workload (Single-Switch)
Decide {workload} and {method} to use. E.g.: *farreach* and *workloada*.

1. <u>[Under Main Client]</u> Prepare Config Files
   - Set file {method}/config.ini:
     - `workload_name`={workload}
     - `workload_mode`=0
     - `bottleneck_serveridx_for_rotation`
     - `server_total_logical_num_for_rotation`

		*NOTE: config.ini must have the correct bottleneckidx and rotationscal, otherwise PregeneratedWorkload will choose the incorrect requests of non-running servers and hence timeout*
   - Set scripts/common.sh accordingly

2. <u>[Under Main Client]</u> Check SERVER_ROTATION option
   - In common/helper.h, disable `SERVER_ROTATION` 
   - Sync code to all machines: `bash scripts/remote/sync.sh`
   - Re-compile clients/servers/switchos as mentioned before

3. <u>[Under Main Client]</u> Setup hot keys and forwarding rules into switch
   - Generate and sync config.ini for setup phase by: `bash scripts/remote/prepare_server_rotation.sh`

4. <u>[Under Physical Switch]</u> Start switch services
   - Create two connection to switch machine
   - Start switch on connection 1 by: 
     - `cd {method}/tofino` 
     - `su`
     - `bash start_switch.sh`
   - Launch switch testbed on connection 2 by:
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

## Static server idx for different workloads and scale

| Workload Name | Scale | Bottleneck Serveridx |
| --- | --- | --- |
| workloada | 16 | 14 |
| workloada | 32 | 29 |
| workloada | 64 | 59 |
| workloada | 128 | 118 |

</br>

## Parameter sensitivity

- Change parameters
	+ For write ratio, change read/updateproportion in benchmark/ycsb/workloads/synthetic
	+ For value size, change fieldlength in benchmark/ycsb/workloads/synthetic
	+ For skewness, change ZIPFIAN_CONSTANT in benchmark/ycsb/core/generator/ZipfianGenerator + sync_file.sh + re-compile ycsb
- IMPORTANT NOTE
	- Under static pattern, each physical client should dump statistics into benchmark/output/{workloadname}-statistics/{method}-static{staticscale}-client{physicalidx}.out (e.g., benchmark/output/workloada-statistics/farreach-static16-client0.out) without parameter info
	- NOTE: before changing parameter for the next time of experiment
		+ Run `bash scripts/remote/calculate_statistics.sh` to get results of the current parameter
		+ Backup the statistics files of the current parameter if necessary, which will be overwritten next time

</br>

## Trial of in-memory KVS to reproduce NetCache

- Prepare TommyDS
	+ Run `bash scripts-inmemory/local/changeto_inmemory_testbed.sh` to perform the following steps
		+ In common/helper.h: uncomment `USE_TOMMYDS_KVS`
		+ In farreach/Makefile: use `TOMMYDS_LDLIBS` instead of `ROCKSDB_LDLIBS` for `server`
	+ Run `bash scripts-inmemory/remote/sync.sh` to sync required files including tommyds-2.2/ to all machines
	+ In each server, enter tommyds-2.2/ and run `make staticlib` to compile TommyDS before `bash scripts/remote/makeserver.sh`
- NOTE: use `git checkout -- <filename>` to cancel the changes of the above files
