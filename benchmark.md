# README for benchmark (running guide)

- method: farreach / nocache / netcache / distfarreach / distnocache / distcache

## Environment installation

- Install boost
	+ `sudo apt-get install libboost-all-dev`
- Install bpftools for spine reflector
	+ `git clone https://github.com/cloudflare/bpftools.git`
	+ `cd bpftools`
	+ `make`
- Disable THP to reduce write_memtable_time in rocksdb
	+ `sudo apt-get install hugepages`
	+ `sudo hugeadm --thp-never`
	+ `cat /sys/kernel/mm/transparent_hugepage/enabled` to check
- Install Maven and Java
	+ Please use openjdk-8/11 to avoid version-related issues

## Code compilation

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
	+ Compile hardware code (NOT need to run now as we have compiled hardware code)
		* Under each Tofino OS, enter directory of method/tofino/ (or method/tofino-spine/ and method/tofino-leaf/)
			- `bash compile.sh` (NOTE: if we have already compiled for all methods, we do NOT need to run this command unless we change in-switch implementation)

## Prepartion phase

- Loading phase for each server scale (e.g., 2/16/32/64/128 servers)
	+ In nocache/config.ini, set workloadmode, server_physical_num, total_server_logical_num, and per-server logical_server_idxes accordingly
		* If workloadmode=0, also set bottleneck_serveridx_for_rotation and total_server_logical_num_for_rotation accordingly (NOTE: total_server_logical_num_for_rotation must = total_server_logical_num here)
	+ Under the main client, perform the loading phase and backup for evaluation time reduction
		* Disable server_rotation in common/helper.h
		* Sync nocache/ including the new nocache/config.ini to all machines and re-compile
		* In switch, launch and configure nocache switch by start_switch.sh and launchswitchtestbed.sh in nocache/
		* Run `bash scripts/remote/load_and_backup.sh`, which will launch and kill servers automatically
- Workload analysis for each workload (e.g., workloada)
	+ Under the main client
		* Update workload_name with the workload in keydump/config.ini
		* Automatic way
			- Run `bash scripts/remote/keydump_and_sync.sh`, which will sync required files to clients/server after keydump
		* ~~Manual way (DEPRECATED)~~
			- ~~`cd benchmark/ycsb; ./bin/ycsb run keydump; cd ../../`~~
				+ Dump hottest/nearhot/coldest keys into benchmark/output/<workloadname>-\*.out for warmup phase later
				+ Dump bottleneckt serveridx for different server rotation scales for server rotation later
				+ Pre-generate workload files in benchmark/output/<workloadname>-pregeneration/ for server rotation later
			- ~~`cd benchmark/ycsb; python generate_dynamicrules.py <workloadname>; cd ../../`~~
				+ Generates key popularity changing rules in benchmark/output/<workloadname>-\*rules/ for dynamic pattern later
			- ~~`bash scripts/remote/synckeydump.sh <workloadname>` to sync the above files of the workload to another client~~

## Dynamic running

- Given a method (e.g., farreach) and a workload (e.g., workloada)
	* Set workload_mode = 1 and workload_name = workload in method/config.ini
		- Keep server_physical_num=2, server0::server_logical_idxes=0, and server1:;server_logical_idxes=1
	* Set scripts/common.sh accordingly
- Re-compile code for server rotation if necessary
	+ Disable server_rotation in common/helper.h
	+ `bash scripts/remote/sync.sh` to sync code to all machines
	+ Re-compile clients/servers/switchos as mentioned before
- Under each physical switch
	+ Create a terminal and run `cd method/tofino; su; bash start_switch.sh`
	+ Create a terminal and run `cd method; su; bash localscripts/launchswitchtestbed.sh`
	+ NOTE: if encounter any problem, check tmp_\*.out and tofino/tmp_\*.out in switch first
		* Run `cd method; su; bash localscripts/stopswitchtestbed.sh` before next time of running
- Under the main client
	+ Run `bash scripts/remote/test_dynamic.sh` to evaluate under dynamic pattern
	+ ~~Manual way (DEPRECATED)~~
		+ ~~Run `bash scripts/remotes/launchservertestbed.sh`~~
			* ~~Run `cd method; ./warmup_client; cd ..` to pre-admit hot keys~~
		+ ~~NOTE: if encounter nay problem, check tmp_\*.out in each server first~~
			* ~~Run `bash scripts/remote/stopservertestbed.sh` before next time of running~~
		+ ~~Launch clients~~
			- ~~Under client 1~~
				+ ~~Run `cd benchmark/ycsb; ./bin/ycsb run method -pi 1`~~
			- ~~Under client 0~~
				+ ~~Run `cd benchmark/ycsb; ./bin/ycsb run method -pi 0`~~
	- NOTE: each physical client should dump statistics into benchmark/output/<workloadname>-statistics/<method>-<dynamicpattern>-client<physicalidx>.out (e.g., benchmark/output/workloada-statistics/farreach-hotin-client0.out)
- After running
	+ Under each physical switch, run `cd method; su; bash localscripts/stopswitchtestbed.sh`
	+ Under main client, run `bash scripts/remote/stopservertestbed.sh`
- Aggregate statistics
	+ Run `bash scripts/remote/calculate_statistics.sh` to get aggregated statistics

## Static running of single-switch method (e.g., farreach/nocache/netcache)

- Given a method (e.g., farreach) and a workload (e.g., workloada)
	+ In method/config.ini
		* Set workload_name = workload, workload_mode = 0
		* Set bottleneck_serveridx_for_rotation and server_total_logical_num_for_rotation accordingly
			- NOTE: config.ini must have the correct bottleneckidx and rotationscal, otherwise PregeneratedWorkload will choose the incorrect requests of non-running servers and hence timeout
	+ Set scripts/common.sh accordingly
- Re-compile code for server rotation if necessary
	+ Enable server_rotation in common/helper.h
	+ `bash scripts/remote/sync.sh` to sync code to all machines
	+ Re-compile clients/servers/switchos as mentioned before
- Phase 0: setup hot keys and forwarding rules into switch
	+ `bash scripts/remote/prepare_server_rotation.sh` to generate and sync config.ini for setup phase
	+ Launch switch based on the new config.ini as mentioned in dynamic running
	+ ~~Manual way (DEPRECATED)~~
		+ ~~Launch server and controller based on the new config.ini as mentioned in dynamic running~~
		+ ~~If the method is not nocache/distnocache, `cd method; ./warmup_client` to pre-admit hot keys into switch~~
		+ ~~`bash scripts/remote/stopservertestbed.sh` to stop server and controller~~
- Server rotation
	+ `bash scripts/remote/test_server_rotation.sh`
		* Phase 1: the first rotation (physical server 0 runs the bottleneck server)
		* Phase 2: each subsequent rotation (physical server 0 runs the bottleneck server; physical server 1 runs each non-bottleneck server)
		* NOTE: each physical client should dump statistics into benchmark/output/<workloadname>-statistics/<method>-static<staticscale>-client<physicalidx>.out (e.g., benchmark/output/workloada-statistics/farreach-static16-client0.out)
			- NOTE: you can check the statistics during server rotation to see whether the result is reasonable
	+ `bash scripts/remote/stop_server_rotation.sh`
	+ NOTE: if some rotation is failed, you can use scripts/remote/test_server_rotation_\<p1/p2\>.sh to get the result of the rotation
		* [IMPORTANT] backup the existing statistics file first to avoid mis-overwriting
		* For example, if strid=server-x is missed, run `bash scripts/remote/test_server_rotation_p1.sh 1`
		* For example, if strid=server-x-y is missed, run `bash scripts/remote/test_server_rotation_p2.sh 1 y`
	+ NOTE: we limit the execution time of each rotation as 10 seconds in all methods for fair comparisons
		* Involved files: ycsb/core/Client.java
		* DEPRECATED: ycsb/core/TerminatorThread.java, scripts/local/calculate_statistics_helper.py
- Aggregate statistics
	+ Run `bash scripts/remote/calculate_statistics.sh` to get aggregated statistics

## Parameter sensitivity

- Change parameters
	+ For write ratio, change read/updateproportion in benchmark/ycsb/workloads/synthetic
	+ For value size, change fieldlength in benchmark/ycsb/workloads/synthetic
	+ For skewness, change ZIPFIAN_CONSTANT in benchmark/ycsb/core/generator/ZipfianGenerator + sync_file.sh + re-compile ycsb
- IMPORTANT NOTE
	- Under static pattern, each physical client should dump statistics into benchmark/output/<workloadname>-statistics/<method>-static<staticscale>-client<physicalidx>.out (e.g., benchmark/output/workloada-statistics/farreach-static16-client0.out) without parameter info
	- NOTE: before changing parameter for the next time of experiment
		+ Run `bash scripts/remote/calculate_statistics.sh` to get results of the current parameter
		+ Backup the statistics files of the current parameter if necessary, which will be overwritten next time
	- ~~(NOT run dynamic pattern here) under dynamic pattern, each physical client should dump statistics into benchmark/output/<workloadname>-statistics/<method>-<dynamicpattern>-client<physicalidx>.out (e.g., benchmark/output/workloada-statistics/farreach-hotin-client0.out)~~

## Appendix

- Static server idx for different workloads and scale

| Workload Name | Scale | Bottleneck Serveridx |
| --- | --- | --- |
| workloada | 16 | 14 |
| workloada | 32 | 29 |
| workloada | 64 | 59 |
| workloada | 128 | 118 |

## Trial of in-memory KVS to reproduce NetCache

- Prepare TommyDS
	+ Run `bash scripts-inmemory/local/changeto_inmemory_testbed.sh` to perform the following steps
		+ In common/helper.h: uncomment `USE_TOMMYDS_KVS`
		+ In farreach/Makefile: use `TOMMYDS_LDLIBS` instead of `ROCKSDB_LDLIBS` for `server`
		+ Replace farreach/config.ini with farreach/configs/config.ini.inmemory
		+ In scripts-inmemory/remote/sync.sh, uncomment `syncfiles_toall tommyds-2.2 \*`
		+ In farreach/tofino/netbufferv4.p4, uncomment `USE_BFSDE920`
	+ Run `bash scripts-inmemory/remote/sync.sh` to sync required files including tommyds-2.2/ to all machines
	+ In each server, enter tommyds-2.2/ and run `make staticlib` to compile TommyDS before `bash scripts/remote/makeserver.sh`
	+ In each switch, enter farreach/tofino and run `su; bash compile.sh` to compile switch code
- NOTE: use `git checkout -- <filename>` to cancel the changes of the above files
