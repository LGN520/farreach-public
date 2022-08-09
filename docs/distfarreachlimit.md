# Distributed FarReach w/ counter-based rate limit (DEPRECATED due to NO sufficient power budget)

- Copy from distfarreach
- See [important notes](./netreach-v4-lsm.md) in netreach-v4-lsm.md
- NOTE: we avoid dependency on ingress port; see details in [implementation log after/during DistFarReach](netreach-v4-lsm.md)
- See [other important notes and details](./distfarreach.md) in distfarreach.md

## Overview

- Support counter-based rate limit for switch simulation
	+ Maintain per-logicalswitch load counter in spine/leaf egress
	+ Periodically reset load counter
	+ If counter > threshold, drop each new request

## Implementation log

- Replace distfarreach as distfarreachlimit (files: io_helper.c)
	+ Run `sed -i s/distfarreach/distfarreachlimit/g `cat replace.out``
	+ Rename tofino-*/main.py
- Add in leaf switch
	+ Access ratelimit_threshold_tbl to set ratelimit_threshold by default action (files: tofino-leaf/main.p4, tofino-leaf/p4srg/egress_mat.p4, tofino-leaf/p4src/header.p4, TODO: tofino-leaf/configure/table_configure.py)
	+ Access ratelimitload_reg in egress pipeline based on op_hdr.globalswitchidx (files: tofino-leaf/p4srg/regs/ratelimitload.p4, tofino-leaf/main.p4, tofino-leaf/p4src/header.p4)
		* Increase ratelimitload_reg for GETREQ_INSWITCH, PUT/DELREQ_SEQ_INSWITCH, SCANREQ_SPLIT (files: TODO: tofino-leaf/configure/table_configure.py)
		* Update/reset meta.ratelimitload_predicate by comparing counter with ratelimit_threshold
	+ TODO: Match meta.ratelimitload_predicate in drop_for_ratelimit_tbl (files: tofino-leaf/main.p4. tofino-leaf/p4src/egress_mat.p4, TODO: tofino-leaf/configure/table_configure.py)
		* NOTE: we must drop for ratelimit before another_eg_port_forward_tbl and eg_port_forward_tbl such that optype has not been changed
		* Drop pkt for GETREQ_INSWITCH, PUT/DELREQ_SEQ_INSWITCH, and SCANREQ_SPLIT ONLY if ratelimitload_predicate = 2
- TODO: Add in spine switch
- TODO: Reset ratelimitload_reg periodically

## Run

- Hardware configure
	+ Deprecated: DPDK
		* Follow [tech_report](./tech_report.md) to confiure dpdk
	+ Configure after each login
		* `source configure_client.sh`: configure NIC ipv4 address, arp table, UDP rcvbuf size, and openfd limitation
		* `source configure_server.sh`: configure NIC ipv4 address, arp table, UDP rcvbuf size, and openfd limitation
		* `sudo bash configure_switchos.sh`: configure UDP rcvbuf size
	+ Max # of open files
		* `sudo vim /etc/security/limits.conf` to set hard and soft limits on maximum # of open files
		* logout and re-login
		* `ulimit -n number` to set soft # of open files
- Prepare synthetic Zipf workload for loading, warmup, and transaction phase
	+ Modify synthetic-generator/common.py to configure max_key, max_hotkey, and query_num
		+ Update config.ini to configure correct workload name
	+ `python gen_kv.sh` to generate workload for loading phase
		+ `./split_workload load linenum` -> workloada-load-{split_num}/*-*.out
	+ `python gen_warmupkv.sh` to generate workload for warmup phase
	+ `python gen_queries_zipf.sh` to generate Zipf workload for transaction phase
		+ `./split_workload run linenum` -> workloada-run-{server_num}/*.out
- Prepare YCSB workload for loading or transaction phase
	+ For example:
	+ `./bin/ycsb.sh load basic -P workloads/workloada -P netbuffer.dat > workloada-load.out`
	+ `./bin/ycsb.sh run basic -P workloads/workloada -P netbuffer.dat > workloada-run.out`
	+ `./split_workload load linenum` -> workloada-load-{split_num}/\*-\*.out
	+ `./split_workload run linenum` -> workloada-run-{server_num}/\*.out
- Change partition method (hash/range partition)
	+ RANGE_SUPPORT in tofino-*/netbufferv4.p4
	+ USE_HASH/RANGE in helper.h
	+ RANGE_SUPPORT in tofino-*/common.py
- Local loading phase
	- `./loader` to launch loaders in end host
	- If using ycsb, after ycsb loading phase, run `./loadfinish_client` in each physical server
- Warmup/Transaciton phase
	- Spine switch (bf3/bf2; we use bf3 now) and leaf switch (bf1)
		- Run `cd tofino`
		+ Run `su` to enter root account
		+ Run `bash compile.sh` to compile p4 into binary code (IMPORTANT: NOT execute unless you change p4 code)
		+ Run `bash start_switch.sh` to launch Tofino
	- Maunual way to launch testbed (out-of-date)
		- Launch two switchoses (in spine or leaf switch) in local control plane of Tofino
			+ Create a new terminal and run `./switchos spine/leaf`
			+ Create a new terminal and run `bash ptf_popserver.sh`
			+ Create a new terminal and run `bash ptf_snapshotserver.sh`
			+ Create a new terminal and run `bash configure.sh` to configure data plane, then run `bash ptf_cleaner.sh`
		- Launch controller in end host
			+ `./controller`
		- Launch two reflectors (for spine or leaf switch) in corresponding end hosts
			+ `./reflector spine/leaf`
		- Launch servers in end host
			+ `./server server_physical_idx`
			+ NOTE: to close server, use `sudo kill -15` to send SIGKILL
	- Automatic way to launch testbed (latest)
		+ In each switch
			* Run `su` to enter root account
			* Run `bash localscripts/launchswitchostestbed.sh spine/leaf` to configure switch, launch switchos and ptf_pop/snapshotserver/cleaner
				- Run `bash localscripts/stopswitchostestbed.sh` to stop switch, switchos, and ptf_XXX
		+ In client 0 (dl11)
			* Run `bash remotescripts/launchservertestbed.sh` to launch controller, server, and reflector if any
				- Run `bash remotescripts/stopservertestbed.sh` to stop contoller, server, and reflector if any
	- Launch clients in end host
		- Warmup phase: `./warmup_client`
		- Before transaction phase: `./preparefinish_client` to make server-side snapshot and start in-switch snapshot
			+ NOTE: NOT manual flush now
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

- Normal requests/responses
	* Single READREQ
	* Single PUTREQ
	* Single DELREQ
	* Single SCANREQ
	* Single LOADREQ
- Cache population/eviction
	* WARMUPREQ/PUTREQ_POP/DELREQ_POP in leaf switch
	* WARMUPREQ/PUTREQ_POP/DELREQ_POP in spine switch
- Cache hit
	* GET/PUT/DELREQ hit in leaf switch
	* GET/PUT/DELREQ hit in spine switch
- Conservative read
	* GETRES_LATEST_SEQ_SERVER in leaf switch
	* GETRES_DELETED_SEQ_SERVER in leaf switch
	* GETRES_LATEST_SEQ in spine switch
	* GETRES_DELETED_SEQ in spine switch
- Crash-consistent snapshot
	* Snapshot loading by LOADSNAPSHOTDATA_INSWITCH
	* Single path
		- PUTREQ_SEQ_INSWITCH
		- DELREQ_SEQ_INSWITCH
		- GETRES_LATEST_SEQ_SERVER_INSWITCH
		- GETRES_DELETED_SEQ_SERVER_INSWITCH
	* Special cases
		- PUTREQ_SEQ_INSWITCH_CASE1
		- DELREQ_SEQ_INSWITCH_CASE1
		- GETRES_LATEST_SEQ_INSWITCH_CASE1
		- GETRES_DELETED_SEQ_INSWITCH_CASE1
		- CACHE_EVICT_CASE2
		- PUTREQ_SEQ_CASE3
		- DELREQ_SEQ_CASE3
