# Running Guide

- method: netreach-v4-lsm / nocache / netcache / distfarreach / distnocache / distcache

## Environment Installation

- NOTEs
	+ In our testbed, we have already installed the following environment (NOT need to install them once again)
	+ We have provided RocksDB 6.22.1 and YCSB 0.17.0 (TODO) in our source code
- We use gcc-5.4.0 and g++-5.4.0
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
- Compile code
	+ Under main client (i.e., the first physical client dl11), enter directory of method/
		* `bash remotescripts/makeremotefiles` to make C/C++ code including switchos, controller, and server
	+ Under each Tofino OS, enter directory of method/tofino/ (or method/tofino-spine/ and method/tofino-leaf/)
		* `bash compile.sh` (NOTE: if we have already compiled for all methods, we do NOT need to run this command unless we change in-switch implementation)
	+ Under each physical client, enter directory of ycsb/
		* For each method, `mvn -pl site.ycsb:method-binding -am clean package`

## Normal running

- Launch switch and switchos
	+ Single-switch testbed (i.e., bf1)
		+ (1) Create a terminal to login a Tofino OS, and enter directory of method/tofino/
			* `bash start_switch.sh`
		+ (2) Create another terminal to login the same Tofino OS, and enter directory of method/
			* `bash localscripts/launchswitchostestbed.sh` to configure data plane and launch corresponding switchos processes
				- NOTE: you can check tmp_switchos.out, tmp_popserver.out, tmp_snapshotserver.out, and tmp_cleaner.out if any (some method may NOT have all the above files) to see whether switchos processes are launched correctly
	+ Distributed testbed (i.e., bf3 as spine and bf1 as client/server leaf)
		+ You need to run the above two steps in both bf3 and bf1
			* NOTE: step (1) is under method/tofino-spine/ for bf3 and method/tofino-leaf/ for bf1
			* NOTE: command in step (2) is `bash localscripts/launchswitchostestbed.sh spine/leaf` for bf3/bf1 respectively
- Launch server
	+ Under main client (i.e., the first physical client dl11), enter directory of method/
		+ `bash remotescripts/launchservertestbed.sh` to launch server(s), controller, and reflectors (simulated link between switch and switchos)
			* NOTE: you can check tmp_server.out, tmp_controller.out, and tmp_reflector.out if any (tmp_reflector.out exist in dl11 and dl16 ONLY for distributed testbed) to see whether those processes are launched correctly
- Loading phase
	+ TODO: Use YCSB loading client to store a fixed amount of records into servers
		* TODO: `./bin/ycsb load method -P workload`
	+ TODO: Backup RocksDB files to avoid repeat loading phase in each experiment
		* NOTE: if you have backuped RocksDB files, you can directly overwrite RocksDB files w/ backuped files before launching servers
- Warmup phase ONLY for Farreach/DistFarreach/Netcache/Distcache
	+ Under main client (i.e., the first physical client dl11), enter directory of method/
		+ `./warmup_client.c` to pre-populate 10000 hot keys into switch(es) based on XXX-warmup.out
		+ Under the terminal running `bash start_switch.sh` in each Tofino OS, use `pd-method` -> `pd cache_lookup_tbl get_entry_count` to check whether 10000 hot keys have already been populated into switch(es)
- Extra phase ONLY for Distcache
	+ DEPRECATED: For Distcache, under each Tofino OS, enter directory of method/tofino/ (or method/tofino-spine/ and method/tofino-leaf/)
		+ DEPRECATED: `bash set_all_latest.sh` to set all latest_reg = 1 to ensure the correctness of warmup phase (to fix Tofino hardware bug)
		+ NOTE: now we use a trick to directly set latest=1 for CACHE_POP_INSWITCH to fix the Tofino hardware bug without undermining perf
	+ DEPRECATED: for Farreach/DistFarreach, under main client, enter directory of method/
		+ DEPRECATED: `./preparefinish_client.sh` to notify controller to start periodic snapshot, and servers to make an initial snapshot for range query
		+ NOTE: now we automatically execute preparefinish_client to trigger snapshot in physical client 0
- Transaction phase
	+ TODO: Use YCSB transaction client to send different types of requests and receive corresponding responses based on specified workload
		* TODO: `./bin/ycsb run method -P workload`

## Evaluation for static workload

- Prepare phase
	- NOTEs
		+ Enable SERVER_ROTATION in helper.h and compile all
		+ Update bottleneck partition in configs/config.ini.rotation-switch and remotescripts/test_server_rotation.sh
			* global::bottleneck_serveridx_for_rotation
			* server0::server_logical_idxes
			* server1::server_logical_idxes
			* bottleneck_serveridx in shell
	+ Under main client (i.e., the first physical client dl11), enter directory of method/
		+ `bash remotescripts/prepare_for_static.sh` to sync necessary files to another client and servers
- Test phase
	+ Under main client (i.e., the first physical client dl11), enter directory of method/
		* Copy method/configs/config.ini.rotation-switch-switchos-loading-warmupclient.dl16dl13 as method/config.ini
		* `bash sync_file.sh config.ini` to sync config.ini to another client. switch(es), and servers
	+ Perform all steps before transaction phase as in normal running
		* NOTE: for all methods, you do NOT need to perform loading phase, if you have already backuped RocksDB files to avoid repeat loading phase
			- Instead, you NEED to overwrite RocksDB files w/ backuped files before launching servers
		* NOTE: for Farreach/DistFarreach, remote_client will automtically execute `./preparefinish_client.sh`
		* NOTE: for nocache/distnocache, as they do NOT need warmup/extra phase, you can skip this step and directly perform automatic transaction phase
		* NOTE: for DistCache, NOT need to run `bash set_all_latest.sh` now
	+ Automatic transaction phase
		* `nohup bash remotescripts/test_server_rotation.sh >tmp.out 2>&1 &` to run 128 experiments under server rotation automatically
			- TODO: For YCSB transaction clients of all methods, consider how to run with a fixed time to get average throughput
			- NOTE: we close-and-restart servers in each experiment
				+ TODO: Before launching servers in each experiment, we have to resume server-side RocksDB state to that after loading phase by overwriting RocksDB files
			- NOTE: for Farreach/DistFarreach, we have to close-and-restart controller in each experiment
				+ TODO: For Farreach/DistFarreach, consider how to introduce snapshot during server rotation to demonstrate its limited effect on write performance
		* NOTE: you can check tmp.out (in main client), tmp_serverrotation_part1*.out, and tmp_serverrotation_part2*.out in clients and servers to see whether they are running correctly
		* NOTE: you can check tmp_rotation.out in main client (i.e., the first physical client dl11) to see the aggregated server rotation result
- Evaluation
	+ DEPRECATED: Add the number of experiments (e.g., 128) as the first line of tmp_rotation.out (refer to results/rotation-result.template)
	+ Run `python rotation_calculate_thpt.py tmp_rotation.out` to calculate server rotation statistics

## Evaluation for dynamic workload

- Prepare phase
	+ Under main client (i.e., the first physical client dl11), enter directory of method/
		+ `bash remotescripts/prepare_for_dynamic.sh` to sync necessary files to another client and servers
- Test phase
	+ Under main client (i.e., the first physical client dl11), enter directory of method/
		* Copy method/configs/config.ini.dl16dl13 as method/config.ini
		* `bash sync_file.sh config.ini` to sync config.ini to another client. switch(es), and servers
	+ Perform all steps including transaction phase as in normal running
		* NOTE: for all methods, you do NOT need to perform loading phase, if you have already backuped RocksDB files to avoid repeat loading phase
			- Instead, you NEED to overwrite RocksDB files w/ backuped files before launching servers
		* DEPRECATED: for Farreach/DistFarreach, you NEED to perform the extra phase by `./preparefinish_client.sh` to notify controller and server to make snapshot for range query
- Evaluation	
	+ Create two temporary files tmpa.out/tmpb.out for client0/client1 respectively
	+ From the dumped statistics of each client, copy client-side total pktcnt, per-sec total throughput, and per-sec per-server throughput into each tmp file (refer to results/dynamic-result.template)
	+ Run `python sum_twofiles.py tmpa.out tmpb.out tmpc.out` to aggregate dynamic results of two clients into tmpc.out
	+ Add/update the number of methods (e.g., 1), each method name (e.g., farreach), and total time (i.e., 70s) in tmpc.out
	+ Run `python dynamic_calculate_thpt.py tmpc.out` to calculate dynamic worload statistics

## Others

- Utils scripts
	- Help to update config.ini
		+ gen_logical_idxes: generate server logical indexes from startidx to endidx
	- Help to generate throughput result files
		+ sum_tworows_for_bottleneckserver.py: sum over per-server pktcnts of two clients to find bottleneck partition
		+ sum_twofiles.py: sum over per-server results to get aggregate statistics (NOTE: the two files must have the same content format)
		+ Deprecated (covered by client0.rotationdataserver): gen_rotation_onerow_result.py: generate one row of rotation throughput result files by summing over per-client rotation result line 
	- Analyze throughput result files: dynamic/static/rotation_calculate_thpt.py
		+ static: used to calculate normalized throughput for normal runing
		+ dynamic: used to calculate per-sec runtime throughput and normalized throughput for dynamic workload
		+ rotation: used to calculate avg runtime throughput and latency for static workload with server rotation
	- sync_file.sh: sync one file (filepath relateive to netreach-v4-lsm/) to all other machines
	- ../sync.sh: sync entire netreach-v4-lsm directory to other machines (NOTE: old directory of other machines will be deleted first)
