# DistCache

- Copy from netcache
- See [important notes](./netreach-v4-lsm.md) in netreach-v4-lsm.md
- See [common implementation details](./distfarreach.md) in distfarreach.md
+ NOTE: we avoid dependency on ingress port; see details in [implementation log after/during DistFarReach](netreach-v4-lsm.md)

## Overview

+ Design for DistCache
	* Normal request/response + cache hit/miss
		- See [overview](./distfarreach.md) in distfarreach.md for details
	* Cache population/eviction
		- See [overview](./distfarreach.md) in distfarreach.md for more details
		- For cache population triggered by GET/PUTREQ in transaction phase
			+ NOTE: spine switch does NOT have CM registers for cache population, which is covered by server-leaf switch
			+ In DistCache, server-leaf.GETREQ_INSWITCH -> server-leaf.NETCACHE_GETREQ_POP and server-leaf.GETREQ -> reflector/switchos.NETCACHE_GETREQ_POP and server.GETREQ -> cache population in control plane and server.GETRES_SERVER as in single-switch
		- For cache population triggered by WARMUPREQ in warmup phase
			+ In DistCache, spine.WARMUPREQ -> spine.NETCACHE_WARMUPREQ_INSWITCH -> spine.NETCACHE_WARMUPREQ_INSWITCH_POP and spine.WARMUPACK -> reflector/switchos.NETCACHE_WARMUPREQ_INSWITCH_POP and client-leaf.WARMUPACK -> cache population in control plane and client.WARMUPACK
		- For cache population/eviction in control plane
			+ Launch switchos with physical switch role (spine or leaf)
				* NOTE: each switchos can only read/write inswitch cache of physical spine or leaf switch
			+ For DistCache
				+ NOTE: phyiscal spine and leaf switchos in DistCache MUST have the SAME cached keyset
					- We fix duplicate cache population in spine/leaf switchos.dppopserver (duplicate NETCACHE_WARMUPREQ_INSWITCH_POP / NETCACHE_GETREQ_POP from data plane) -> similar as duplicate hot reports in FarReach, which is not rare and hence we CANNOT allow them entering message queue of switchos.popworker
						+ TODOTODO: NOTE: now we resort spine/leaf switchos.popworker to fix duplicate CACHE_POPs caused by timeout-and-retry of dppopserver, as it is rare and we can allow duplicate CACHE_POPs entering message queue of switchos.popworker
				+ spine/leaf switchos.dppopserver receives NETCACHE_WARMUPREQ_INSWITCH_POP/NETCACHE_GETREQ_POP from data plane
				+ spine/leaf switchos.dppopserver sends NETCACHE_CACHE_POP to controller.popserver and then server
					* NOTE: NETCACHE_CACHE_POP in DistCache aims for not only fetching value from server, but also passing the key to both spine/leaf switchos.cppopserver/popworker for cache population/eviction
				+ After controller.popserver receives NETCACHE_CACHE_POP_ACK w/ new key-value pair from server
					* controller.popserver validates spine/leafswitchidx based on the key
					* controller.popserver.popclient_for_spine/leaf send CACHE_POPs to spine/leaf.cppopserver, and wait for two CACHE_POP_ACKs
					* After receiving two CACHE_POP_ACKs, controller.popserver sends NETCACHE_CACHE_POP_ACK to original switchos.dppopserver to avoid timeout
					* spine/leaf switchos.cppopserver receives CACHE_POP, responds CACHE_POP_ACK, and passes CACHE_POP to spine/leaf switchos.popworker
					* To avoid inconsistent victim between spine/leaf, spine chooses victim by CACHE_EVICT_LOADFREQ_INSWITCH/_ACK -> spine.popworker.victimclient sends DISTCACHE_CACHE_EVICT_VICTIM w/ victim.key&idx to controller.victimserver, and waits for DISTCACHE_CACHE_EVICT_VICTIM_ACK -> controller.victimserver sends NETCACHE_CACH_EVICT_VICTIM to popworker.victimserver of corresponding leaf switch, and forwards DISTCACHE_CACHE_EVICT_VICTIM_ACK from leaf to spine
						- TODOTODO: NOTE: (now we still hold for DistCache) we do NOT need cache_frequency_reg in server-leaf switch, as spine switch chooses vicitm for consistent cache between spine and server-leaf
						- TODOTODO: NOTE: (now we do NOT use individual thread) we can launch leafswitchos.popworker.victimserver as an individual thread to fix duplicate DISTCACHE_CACHE_EVICT_VICTIMs
					* NOTE: both spine/leafswitchos.dppopserver/popworker can send NETCACHE_CACHE_POP / NETCACHE_CACHE_POP_FINISH / NETCACHE_CACHE_EVICT to server during cache pop/evict -> we need to fix duplicate NETCACHE_CACHE_POP / NETCACHE_CACHE_POP_FINISH / NETCACHE_CACHE_EVICT in server
	* Special types for cached key
		- See [overview](./distfarreach.md) in distfarreach.md for more details
		- In DistCache, NETCACHE_PUT/DELREQ_SEQ_CACHED
			+ client.PUT/DELREQ -> client-leaf.PUT/DELREQ -> spine.PUT/DELREQ -> spine.NETCACHE_PUT/DELREQ_SEQ_CACHED
			+ NOTE: it means that the key is cached in spine switch -> the key MUST be cached in corresponding server-leaf switch
			+ server-leaf switch processes NETCACHE_PUT/DELREQ_SEQ_CACHED as PUT/DELREQ_SEQ from spine switch
				* server-leaf.NETCACHE_PUT/DELREQ_SEQ_CACHED -> server-leaf.PUT/DELREQ_SEQ_INSWITCH -> server-leaf.NETCACHE_PUT/DELREQ_SEQ_CACHED
				* NOTE: as the key MUST be cached in server-leaf switch here, the pkt must become NETCACHE_PUT/DELREQ_SEQ_CACHED to server
				* NOTE: even if the key is NOT cached in server-leaf due to rare reason (e.g., nearly impossible fast cache eviction) and the pkt is not marked as XXX_CACHED to server, server can still perform correctly based on its inswitch cache metadata (i.e., beingcached/cached/beingupdated keyset)

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
		+ Run `bash compile.sh` to compile p4 into binary code
		+ Run `bash start_switch.sh` to launch Tofino
	- Launch two switchoses (in spine or leaf switch) in local control plane of Tofino
		+ Create a new terminal and run `./switchos spine/leaf`
		+ Create a new terminal and run `bash ptf_popserver.sh`
		+ Create a new terminal and run `bash configure.sh` to configure data plane, then run `bash ptf_cleaner.sh`
	- Launch controller in end host
		+ `./controller`
	- Launch two reflectors (for spine or leaf switch) in corresponding end hosts
		+ `./reflector spine/leaf`
	- Launch servers in end host
		+ `./server server_physical_idx`
		+ NOTE: to close server, use `sudo kill -15` to send SIGKILL
	- Launch clients in end host
		- Warmup phase: `./warmup_client`
		- Transaction phase: `./remote_client client_physical_idx`
- Server rotation for static workload
	- Use config.ini.rotation-switch-switchos-loading-warmupclient
		+ Start switch and configure data plane
		+ Start switchos and ptf.popserver
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

## Implementation log

+ Implement DistCache
	* Copy netcache to distcache
	* See [dist-farrech.md](./dist-farreach.md) for changes which are synced to DistCache
	* Normal request/response
		- Remove BF-related MATs in spine switch including access_bfX_tbl and hash_for_bfX_tbl (files: tofino-spine/main.p4, tofino-spine/p4src/regs/bf.p4, tofino-spine/configure/table_configure.py, tofino-spine/p4src/ingress_mat.p4)
			+ NOTE: meta.is_hot/is_report MUST be set as 0 by default actions in is_hot_tbl and is_report_tbl for eg_port_forward_tbl in spine switch (files: tofino-spine/configure/table_configure.py)
		- server-leaf processes NETCACHE_PUT/DELREQ_SEQ_CACHED as PUT/DELREQ_SEQ from spine switch (files: tofino-leaf/configure/table_configure.py, tofino-leaf/p4src/ingress_mat.p4)
	* Cache population/eviction
		* For GETREQ of cache population
			- Server-leaf converts GETREQ_INSWITCH as NETCACHE_GETREQ_POP for cache population as in single-switch
				+ Server-leaf clones NETCACHE_GETREQ_POP to reflector_for_leaf.sid/port (files: tofino-leaf/configure/table_configure.py)
		* For WARMUPREQ of cache population
			- Client-leaf switch forwards WARMUPREQ to spine switch
			- NOTE: DistCache spine switch converts WARMUPREQ (from client-leaf) into NETCACHE_WARMUPREQ_INSWITCH instead of WARMUPREQ_SPINE
			- Spine switch converts WARMUPREQ as NETCACHE_WARMUPREQ_INSWITCH/_POP for cache population, and clones WARMUPACK w/ client info to client-leaf switch (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configure/table_configure.py)
				+ Spine switch clones NETCACHE_WARMUPREQ_INSWITCH_POP to reflector_for_spine.sid/udpport/devport/ip/mac (files: tofino-spine/configure/table_configure.py)
				+ Remove update_warmupreq_to_warmupreq_spine() in ig_port_forward_tbl of spine switch
				+ Remove WARMUPREQ_SPINE-related MAT entries in egress pipeline
			- Client-leaf switch forwards WARMACK to client
			- NOTE: server-leaf switch will NOT process WARMUPREQ or WARMUPREQ_SPINE for cache population
				+ Remove update_warmupreq_to_netcache_warmupreq_inswitch() and update_warmupreq_spine_to_warmupreq() in ig_port_forward_tbl of server-leaf switch (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
				+ For server-leaf switch, remove WARMUPREQ_SPINE-related MAT entries in ingress pipeline; remove WARMUPREQ/NETCACHE_WARMUPREQ_INSWITCH-related MAT entires in egress pipeline (files: tofino-leaf/configure/table_configure.py)
		* For cache population/eviction in control plane -> sync to distfarreach/distcache if necessary
			- spine/leafswitchos.dppopserver processes NETCACHE_WARMUPREQ_INSWITCH_POP/NETCACHE_GETREQ_POP from data plane
				+ Add switchos.dp/cppopserver_port in configuration (ONLY used by DistCache) (files: config.ini, configs/*, iniparser/iniparser_wrapper.*, common_impl.h)
				+ spine/leafreflector.dp2cpserver sends pkt to spine/leafswitchos.dppopserver (ONLY used by DistCache) (files: reflector_impl.h)
				+ spine/leafswitchos.dppopserver fixes duplicate cache population from data plane as in single-switch
				+ If the key is not cached, spine/leafswitchos.dppopserver.popclient_for_controller sends NETCACHE_CACHE_POP to controller.popserver, which forwards the pkt to correspondings server further (files: switchos.c, controller.c)
				+ controller.popserver receives NETCACHE_CACHE_POP_ACK from server.popserver and validates spine/leaf switchidx (files: controller.c)
				+ controller.popserver.popclient_for_spine/leaf send CACHE_POPs to spine/leafswitchos.cppopserver; and wait for CACHEPOP_ACKs (files: controller.c)
				+ controller.popserver receives two CACHE_POP_ACKs, and sends NETCACHE_CACHE_POP_ACK to original switchos.dppopserver (files: controller.c)
				+ spine/leafsiwtchos.cppopserver receives CACHE_POP, sends CACHE_POP_ACK to controller.popserver.popclient_for_spine/leaf, and passes CACHE_POP to switchos.popworker (files: switchos.c)
				+ DEPRECATED: spine/leafswitchos.cppopserver fixes duplicate cache population from controller.popserver due to timeout-and-retry of switchos.dppopperver (files: switchos.c)
					* NOTE: we resort switchos.popworker to fix duplicate CACHE_POPs as in FarReach/DistFarReach
				+ To avoid inconsistent victim between spine/leaf for the same newly-populcated key
					* spine.popworker chooses victim by CACHE_EVICT_LOADFREQ_INSWITCH/_ACK (files: switchos.c)
					* leaf.popworker does NOT choose victim; instead leaf.popworker.victimserver waits for DISTCACHE_CACHE_EVICT_VICTIM (files: switchos.c)
						- Add switch:leafvictimserver_port and controller:victimserver_port in configuration (ONLY for DistCache) (files: config.ini, configs/*, iniparser/iniparser_wrapper.*, common_impl.h)
					* Add packet type of DISTCACHE_CACHE_EVICT_VICTIM/_ACK (files: tofino-*/main.p4, tofino-*/common.py, tofino-*/p4src/parser.p4, packet_format.\*, common_impl.h)
					* spine.popworker.victimclient_for_controlelr sends DISTCACHE_CACHE_EVICT_VICTIM (including newkey, victim.key, and victim.idx) to controller.victimserver, and waits for DISTCACHE_CACHE_EVICT_VICTIM_ACK (files: switchos.c)
					* controller.victimserver.victimclient_for_leaf validates spine/leafswitchidx, sends DISTCACHE_CACHE_EVICT_VICTIM to corresponding leaf.popworker.victimserver, waits for DISTCACHE_CACHE_EVICT_VICTIM_ACK (files: controller.c)
					* leaf.popworker.victimserver receives DISTCACHE_CACHE_EVICT_VICTIM, validates victim.key and victim.idx, and sends DISTCACHE_CACHE_EVICT_VICTIM_ACK to controller.victimserver.victimclient_for_leaf, which forwards ack to spine (files: switchos.c, controller.c)
			- Consider duplicate NETCACHE_CACHE_POP / NETCACHE_CACHE_POP_FINISH / NETCACHE_CACHE_EVICT in server (files: server_impl.h)

## Simple test

## Deprecated staff

+ DEPRECATED design for cache population/eviction in DistFarReach
	* NOTE: DistFarReach triggers cache population by servers -> key of CACHE_POP must NOT be cached -> key MUST be cached at most ONCE and hence in at most ONE switch
	* NOTE: to keep stateless controller, controller sends CACHE_POP_ACK to server only if receive CACHE_POP_ACK from spine/leaf, which means that some switchos.popworker has performed cache population/eviction for the key
		- TODO: server sends CACNE_POP to controller by server.popclient
			+ NOTE: wait time for CACHE_POP_ACK in distributed scenario becomes larger than single-switch -> using server.popclient can avoid from affecting server.worker performance for normal requests
			+ TODO: Now we still use server.worker.popclient instead of server.popclient as cache eviction is rare compared with normal requests
	* controller.popserver receives CACHE_POP from server.popclient
		- controller.popserver calculates logical spineswitchidx based on the key to find physical spine switch
			+ NOTE: as we only have one physical spine switch, we validate the logical spineswitchidx
		- TODO: controller.popserver.popclient_for_spine forwards CACHE_POP to switchos.popserver of corresponding spine switch
		- TODO: controller.popserver.popclient_for_spine waits for CACHE_POP_ACK / CACHE_POP_FULL_ACK
		- TODO: If receive CACHE_POP_ACK from spine switch, controller.popserver forwards CACHE_POP_ACK to server.popclient, and waits for next CACHE_POP from server.popclient
		- If receive CACHE_POP_FULL_ACK from spine switch
			+ controller.popserver calculates logical leafswitchidx based on the key to find physical server-leaf
				* NOTE: as we only have one physical leaf switch, we validate the logical leafswitchidx
			+ TODO: controller.popserver.popclient sends CACHE_POP to switchos.popserver of corresponding phyiscal leaf
			+ TODO: controller.popserver.popclient waits for CACHE_POP_ACK / CACHE_POP_FULL_ACK
			+ TODO: If receive CACHE_POP_ACK from leaf switch, controller.popserver forwards CACHE_POP_ACK to server.popclient, and waits for next CACHE_POP from server.popclient
			+ If receive CACHE_POP_FULL_ACK from leaf switch
				* NOTE: we do NOT need to find global victim by CACHE_POP_GETVICTIMs; instead, we still use sampling to reduce cache pop latency
				* TODO: controller.popserver.popclient samples one switch from spine/leaf and sends CACHE_POP_FULL w/ new key to sampled switch
				* TODO: controller receives CACHE_POP_ACK from sampled switch, and forwards it to server.popclient
	* For each physical spine/leaf switchos, after receiving CACHE_POP from controller.popserver.popclient
		- TODO: switchos.popserver checks inswitch cache size first
			+ TODO: Maintain a mutex lock for switchos_cached_empty_index
			+ TODO: If the current switch is not full, switchos.popserver sends CACHE_POP_ACK to controller.popserver.popclient, and pass CACHE_POP w/ new key to switchos.popworker for cache population
			+ TODO: If the current switch is full, switchos.popserver sends CACHE_POP_FULL_ACK to controller.popserver.popclient, yet NOT pass CACHE_POP w/ new key to switchos.popworker for cache population
	* For each physical spine/leaf switchos, after receiving CACHE_POP_FULL from controller.popserver.popclient
		- TODO: switchos.popserver directly sends CACHE_POP_ACK w/ new key to controller.popserver.popclient, and pass CACHE_POP_FULL w/ new key to switchos.popworker for cache eviction
		- TODO: NOTE: CACHE_POP_FULL is inherited from CACHE_POP
		- TODO: NOTE: switchos_cached_empty_index must be full
+ DEPRECATED implementation of cache population/eviction in DistFarReach
	+ TODO: controller.popsever.popclient_for_spine forwards CACHE_POP to spine.switchos.popserver and waits for CACHE_POP/_FULL_ACK (files: controller.c)
		* NOTE: controller needs to communiate with both spine/leaf switchos -> need two sets of global variables; while reflector/switchos can update exclusive global variables based on its role (spine or leaf)
		* TODO: Add packet type of CACHE_POP_FULL_ACK (files: tofino-*/main.p4, tofino-*/common,py, tofino-*/p4src/parser.p4, packet_format.*, common_impl.h)
	+ spine/leafswitchos.popserver processes CACHE_POP from controller.popserver.popclient_for_spine/leaf
		* TODO: Add mutex lock for inswitch cache metadata (writer: switchos.popworker; reader: switchos.popsever/snapshotserver) (files: switchos.c)
		* TODO: switchos.popserver gets pipeidx based on role (spineswitchos -> leafswitch.pipeidx; leafswitchos -> server.pipeidx)
		* TODO: If spine/leaf inswitch cache of the pipeidx is NOT full, switchos.popserver sends CACHE_POP_ACK to controller.popserver.popclient_for_spine/leaf, and passes CACHE_POP to switchos.popworker
		* TODO: If spine/leaf inswitch cache of the pipeidx is full, switchos.popserver sends CACHE_POP_FULL_ACK to controller.popserver.popclient_for_spine/leaf
