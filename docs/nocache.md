# UDP-based NoCache + LSM-based KVS (rocskdb) + NO in-switch cache

- Copy from netreach-v4-lsm
- See [important notes](./netreach-v4-lsm.md) in netreach-v4-lsm.md

## Overview

## Implementation log

## Run

- Hardware configure
	+ Configure after each login
		* `source configure_client.sh`: configure NIC ipv4 address, arp table, UDP rcvbuf size, and openfd limitation
		* `source configure_server.sh`: configure NIC ipv4 address, arp table, UDP rcvbuf size, and openfd limitation
		* `sudo bash configure_switchos.sh`: configure UDP rcvbuf size
	+ Max # of open files
		* `sudo vim /etc/security/limits.conf` to set hard and soft limits on maximum # of open files
		* logout and re-login
		* `ulimit -n number` to set soft # of open files
- Prepare synthetic Zipf workload for loading and transaction phase
	+ Modify synthetic-generator/common.py to configure max_key, max_hotkey, and query_num
		+ Update config.ini to configure correct workload name
	+ `python gen_kv.sh` to generate workload for loading phase
		+ `./split_workload load linenum` -> workloada-load-{split_num}/*-*.out
	+ `python gen_queries_zipf.sh` to generate Zipf workload for transaction phase
		+ `./split_workload run linenum` -> workloada-run-{server_num}/*.out
- Prepare YCSB workload for loading or transaction phase
	+ For example:
	+ `./bin/ycsb.sh load basic -P workloads/workloada -P netbuffer.dat > workloada-load.out`
	+ `./bin/ycsb.sh run basic -P workloads/workloada -P netbuffer.dat > workloada-run.out`
	+ `./split_workload load linenum` -> workloada-load-{split_num}/*-*.out
	+ `./split_workload run linenum` -> workloada-run-{server_num}/*.out
- Change partition method (hash/range partition)
	+ RANGE_SUPPORT in tofino/netbufferv4.p4
	+ USE_HASH/RANGE in helper.h
	+ RANGE_SUPPORT in tofino/common.py
- Local loading phase
	- `./loader` to launch loaders in end host
	- If using ycsb, after ycsb loading phase, run `./loadfinish_client` in each physical server
- Transaciton phase (NOTE: no warmup phase due to no in-switch cache)
	- Switch
		- Run `cd tofino`
		+ Run `su` to enter root account
		+ Run `bash compile.sh` to compile p4 into binary code
		+ Run `bash start_switch.sh` to launch Tofino
	- Configure switch (NOTE: not need switchos and ptf.popserver/snapshotserer due to no in-switch cache)
		+ Create a new terminal and run `bash configure.sh` to configure data plane, then run `bash ptf_cleaner.sh`
	- NOTE: not need controller due to no in-switch cache
	- Launch servers in end host
		+ `./server server_physical_idx`
		+ NOTE: to close server, use `sudo kill -15` to send SIGKILL
	- Launch clients in end host (NOTE: not need warmup_client due to no in-switch cache)
		- Transaction phase: `./remote_client client_physical_idx`
- Server rotation for static workload
	- Use config.ini.rotation-switch-switchos-loading-warmupclient
		+ Start switch and configure data plane
		+ Start switchos and ptf.pop/snapshotserver
		+ Start clients and servers for loading phase
		+ Start clients and servers for warmup phase
	- Use config.ini.rotation-transaction
		+ Start clients and servers for transaction phase (repeat 127 times)
		+ Example of 128 server threads
			* 1 server physical num + 1 server total logical num
				- (1) set server0.server_logical_idxes as bottleneck server logical index in config,ini
				- (2) launch bottleneck server thread in server0
				- (3) Run `./remote_client 1` in client 1
				- (4) Run `./remote_client 0` in client 0
				- (5) Record client0's server rotation data of all physical clients into result.out
			* 2 server physical num + 2 server total logical num
				- (1) set server0.server_logical_idxes as bottleneck server logical index in config.ini
				- (2) Change server1.server_logical_idxes (not equal to server0.server_logical_idxes), e.g., from 0 to 1, in config.ini
				- (3) Run `bash test_server_rotation.sh` in client0
				- (4) Record client0's server rotation data of all physical clients into result.out
				- (5) Go back to step (1) until repeating 127 times
	- IMPORTANT: try different # of client threads to sature servers
		+ NOTE: more client threads does not mean better throughput, as client threads have CPU contention overhead
		+ We MUST try different # of client threads to get the best runtime thpt improvement
- Utils scripts
	- Help to update config.ini
		+ gen_logical_idxes: generate server logical indexes from startidx to endidx
	- Help to generate throughput result files
		+ sum_tworows_for_bottleneckserver.py: sum over per-server pktcnts of two clients to find bottleneck partition
		+ sum_twofiles.py: sum over per-server results to get aggregate statistics (NOTE: the two files must have the same content format)
		+ Deprecated (covered by client0.rotationdataserver): gen_rotation_onerow_result.py: generate one row of rotation throughput result files by summing over per-client rotation result line 
	- Analyze throughput result files: dynamic/static/rotation_calculate_thpt.py
	- sync_file.sh: sync one file (filepath relateive to netreach-v4-lsm/) to all other machines
	- ../sync.sh: sync entire netreach-v4-lsm directory to other machines (NOTE: old directory of other machines will be deleted first)

## Simple test

- NOTE: set switch_kv_bucket_num, KV_BUCKET_COUNT, CM_BUCKET_COUNT, HH_THRESHOLD, SEQ_BUCKET_COUNT as 1 before test
- Test cases of normal operations: see directory of "testcases/normal"
	+ Case 1: single read (GETREQ arrives at server)
		* No key in cache_lookup_tbl, cm=1, {cache_frequency, vallen, val, seq, savedseq, valid, latest, deleted, case1}=0
	+ Case 2: single write (PUTREQ arrives at server)
		* No key in cache_lookup_tbl, cm=1, {cache_frequency, vallen, val, seq, savedseq, valid, latest, deleted, case1}=0
	+ Case 3: read(k1,v1)-write(k1,v2)-read(k1,v2) (GETREQ-PUTREQ_SEQ_POP-GETREQ_POP arrive at server; ignore cache population here)
		* No key in cache_lookup_tbl, cm=3, {cache_frequency, vallen, val, seq, savedseq, valid, latest, deleted, case1}=0
	+ Case 4: single delete (DELREQ arrives at server)
		* No key in cache_lookup_tbl, {cm, cache_frequency, vallen, val, seq, savedseq, valid, latest, deleted, case1}=0
	+ Case 5: read(k1,v1)-delete(k1)-read(k1,none) (GETREQ-DELREQ_SEQ-GETREQ_POP arrive at server; ignore cache population here)
		* No key in cache_lookup_tbl, cm=2, {cache_frequency, vallen, val, seq, savedseq, valid, latest, deleted, case1}=0
- NOTE: no testcases for cache population/eviction/hit and crash-consistent snapshot

## Fixed issues

## Future work
