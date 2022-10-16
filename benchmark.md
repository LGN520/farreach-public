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
	+ Configuration
		* In nocache/config.ini, set workloadmode, total_server_logical_num, and per-server logical_server_idxes accordingly
			- If workloadmode=0, also set bottleneck_serveridx_for_rotation and total_server_logical_num_for_rotation accordingly (NOTE: total_server_logical_num_for_rotation must = total_server_logical_num here)
	+ Under the main client, perform the loading phase and backup for evaluation time reduction
		* Launch nocache/ in switch and servers
		* Run `bash scripts/remote/load_and_backup.sh`, which will kill servers after backup
- Workload analysis for each workload (e.g., workloada)
	+ Under the main client
		* Update workload_name with the workload in keydump/config.ini
		* `cd benchmark/ycsb; ./bin/ycsb run keydump; cd ../../`
			- Dump hottest/nearhot/coldest keys into benchmark/output/<workloadname>-\*.out for warmup phase later
			- Dump bottleneckt serveridx for different server rotation scales for server rotation later
			- Pre-generate workload files in benchmark/output/<workloadname>-pregeneration/ for server rotation later
		* `cd benchmark/ycsb; python generate_dynamicrules.py <workloadname>; cd ../../`
			- Generates key popularity changing rules in benchmark/output/<workloadname>-\*rules/ for dynamic pattern later
		* `bash scripts/remote/synckeydump.sh <workloadname>` to sync the above files of the workload to another client

## Dynamic running

- Given a method (e.g., farreach) and a workload (e.g., workloada)
	* Set workload_mode = 1 and workload_name = workload in method/config.ini
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
	+ Run `bash scripts/remotes/launchservertestbed.sh`
	+ NOTE: if encounter nay problem, check tmp_\*.out in each server first
		* Run `bash scripts/remote/stopservertestbed.sh` before next time of running
- Evaluation
	- Under client 1
		+ Run `cd benchmark/ycsb; ./bin/ycsb run method -pi 1`
	- Under client 0
		+ Run `cd benchmark/ycsb; ./bin/ycsb run method -pi 0`
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
	+ Launch switch, server, and controller based on the new config.ini as mentioned in dynamic running
	+ If the method is not nocache/distnocache, `cd method; ./warmup_client` to pre-admit hot keys into switch
	+ `bash scripts/remote/stopservertestbed.sh` to stop server and controller
- Server rotation
	+ `bash scripts/remote/test_server_rotation.sh`
		* Phase 1: the first rotation (physical server 0 runs the bottleneck server)
		* Phase 2: each subsequent rotation (physical server 0 runs the bottleneck server; physical server 1 runs each non-bottleneck server)
		* NOTE: each physical client should dump statistics into benchmark/output/<workloadname>-statistics/<method>-static<staticscale>-client<physicalidx>.out (e.g., benchmark/output/workloada-statistics/farreach-static16-client0.out)
			- NOTE: you can check the statistics during server rotation to see whether the result is reasonable
	+ `bash scripts/remote/stop_server_rotation.sh`
- Aggregate statistics
	+ Run `bash scripts/remote/calculate_statistics.sh` to get aggregated statistics

## Appendix

- Static server idx for different workloads and scale

| Workload Name | Scale | Bottleneck Serveridx |
| --- | --- | --- |
| workloada | 16 | 14 |
| workloada | 32 | 29 |
| workloada | 64 | 59 |
| workloada | 128 | 118 |
