# README for benchmark (running guide)

- method: farreach / nocache / netcache / distfarreach / distnocache / distcache

## Important NOTEs

- If code changes in ../common (including packet_format.\* and socket_helper.\*)
	+ SYNC to inswitchcache-java/jnisrc/com_inswitchcache_core_SocketHelper.c

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

- NOTE: update scripts based on your testbed accordingly
	+ Update scripts/common.sh with specific USER (username), DIRNAME (method), SECONDARY_CLIENT, and XXX_PATH (directory path)
	+ Update scripts/remote/makeremotefiles.sh and scripts/remote/sync.sh with specific hostnames of physical machines
- Under main client (i.e., the first physical client dl11)
	+ `cd scripts/remote; bash sync.sh` to sync source code to all machines
- Under each server (NOT including client/switch), compile RocksDB (NOTE: only need to compile once)
	+ `sudo apt-get install libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev`
	+ `cd rocksdb-6.22.1`
	+ `PORTABLE=1 make static_lib`
		* Comment -Wstrict-prototype and -Werror if compilation fails due to strict-alias warning
		* Add PORTABLE=1 if with runtime error of illegal instruction when open()
		* Possible error: compression type Snappy is not linked with the binary
			- Install libsnappy and remake static lib of rocksdb
			- Prepare the directory for database in advance
- Compile source code
	+ Compile software code
		* Manual way
			- Under each client, run `cd scripts/local; bash makeclient.sh`
			- Under each switch, run `cd scripts/local; bash makeswitch.sh`
			- Under each server, run `cd scripts/local; bash makeserver.sh`
		* Automatic way: under project root directory of main client (i.e., the first physical client dl11)
			- `cd scripts/remote; bash makeremotefiles.sh` to make C/C++ and java code including client, switchos, controller, and server
	+ Compile hardware code
		* (NOT need to run now) Under each Tofino OS, enter directory of method/tofino/ (or method/tofino-spine/ and method/tofino-leaf/)
			- `bash compile.sh` (NOTE: if we have already compiled for all methods, we do NOT need to run this command unless we change in-switch implementation)

## Loading phase

- TODO

## Dynamic running of single-switch method (e.g., farreach/nocache/netcache)

- Preparation
	+ Configuration
		* Make sure workload_mode = 1 in method/config.ini
		* Make sure the workloadname is consistent in keydump/config.ini and method/config.ini
	+ Under the main client, enter benchmark/ycsb/ to prepare dynamic rules if not prepared before
		* Get hot/cold keys for static/dynamic workloads
			- `./bin/ycsb run keydump`, which generates workload files in benchmark/output/<workloadname>-\*.out
		* Generate rulemap rule files for dynamic workload
			- `python generate_dynamicrules.py <workloadname>`, which generates rules in benchmark/output/<workloadname>-\*rules/
			- For example, `python generate_dynamicrules.py workloada`
- Under each physical switch
	+ Create a terminal and run `cd method/tofino; su; bash start_switch.sh`
	+ Create a terminal and run `cd method; su; bash localscripts/launchswitchtestbed.sh`
	+ NOTE: if encounter any problem, check tmp_\*.out in switch first
		* Run `cd method; su; bash localscripts/stopswitchtestbed.sh` in switch before next time of running
- Under the main client
	+ Run `cd method; bash remotescripts/launchservertestbed.sh`
	+ NOTE: if encounter nay problem, check tmp_\*.out in each server first
		* Run `cd method; bash remotescripts/stopservertestbed.sh` in main client before next time of running
- Under client 1
	+ Run `cd benchmark/ycsb; ./bin/ycsb run method -pi 1`
- Under client 0
	+ Run `cd benchmark/ycsb; ./bin/ycsb run method -pi 0`
- After running
	+ Under each physical switch, run `cd method; su; bash localscripts/stopswitchtestbed.sh`
	+ Under main client, run `cd method; bash remotescripts/stopservertestbed.sh`
- Aggregate statistics
	+ NOTE: each physical client should dump statistics into benchmark/output/<workloadname>-statistics/<method>-<dynamicpattern>-client<physicalidx>.out (e.g., benchmark/output/workloada-statistics/farreach-hotin-client0.out)
	+ Run `cd scripts/remote; bash calculate_statistics.sh <workloadname> dynamic <dynamicpattern>` to get aggregated statistics

## Static running of single-switch method (e.g., farreach/nocache/netcache)

- Preparation
	+ Configuration
		* Make sure workload_mode = 0 in method/config.ini and SERVER_ROTATION is defined in common/helper.h
		* Make sure the workloadname is consistent in keydump/config.ini and method/config.ini
	+ Under the main client, enter benchmark/ycsb/ to get bottleneck serveridx for each scale
		* Get bottleneck serveridx under different scales, and pre-generate per-logical-client workload to avoid invalid CPU cycles
			- `./bin/ycsb run keydump`
	+ Under client 1
		* Run `cd benchmark/ycsb; ./bin/ycsb run method -pi 1`
	+ Under client 0
		* Run `cd benchmark/ycsb; ./bin/ycsb run method -pi 0`
- TODO: Evaluation steps
- Aggregate statistics
	+ NOTE: each physical client should dump statistics into benchmark/output/<workloadname>-statistics/<method>-static<staticscale>-client<physicalidx>.out (e.g., benchmark/output/workloada-statistics/farreach-static16-client0.out)
	+ Run `cd scripts/remote; bash calculate_statistics.sh <workloadname> static <staticscale>` to get aggregated statistics
