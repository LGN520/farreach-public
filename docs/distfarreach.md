# Distributed FarReach

- Copy from netreach-v4-lsm
- See [important notes](./netreach-v4-lsm.md) in netreach-v4-lsm.md
+ Important NOTEs for coding
	- NOTE: we start from netreach-v4-lsm to distributed FarReach, from nocache to distributed NoCache, and also from netcache to distcache at the same time -> keep to SYNC distributed extensions for the three directories!!!
	- NOTE: keep to compile P4 code step by step to quickly detect tricky Tofino limitations!!!
+ Important differences compared with single-switch implementation
	- NOTEs for packet type
		+ For leaf switch, GET/PUT/DEL/SCAN/WARMUP/LOADREQ is sent by client; GET/WARMUP/LOADREQ_SPINE, PUT/DELREQ_SEQ and SCANREQ_SPLIT is sent by spine switch; GET/PUT/DELRES/WARMUP/LOADACK_SERVER and SCANRES_SPLIT_SERVER is sent by server; GET/PUT/DELRES/WARMUP/LOADACK and SCANRES_SPLIT is sent by spine switch
		+ For spine switch, GET/PUT/DEL/SCAN/WARMUP/LOADREQ is sent by leaf switch; GET/PUT/DELRES/WARMUP/LOADACK and SCANRES_SPLIT is sent by leaf switch
	- NOTE: cache_lookup_tbl matches both op_hdr.key and op_hdr.globalswitchidx into inswitch_hdr.is_cached/idx
		+ Due to switch simulation, inswitch_hdr.idx must be different; however, if w/ individual physical switches w/o simulation, inswitch_hdr.idx can be the same for different op_hdr.globalswitchidx
+ NOTE: we avoid dependency on ingress port; see details in [implementation log after/during DistFarReach](netreach-v4-lsm.md)

- [IMPORTANT] NOTE for compilation error of pragma stage limitation
	+ Reason 1: too many entries which exceeds one-stage SRAM due to exact matching (e.g., cache_lookup_tbl) or TCAM due to non-exact matching (e.g., range/ternary/lpm matching; range_partition_tbl)
	+ Solution 1: use multiple pragmas to split table entries into different stages
	+ Reason 2: too many actions and action parameters which exceeds one-stage VLIW
		* NOTE: even if we split the table into multiple stages, each stage still needs enough VLIW for all possible actions as Tofino does NOT know which MAT entries you will configure into the current stage
	+ Solution 2: split the MAT table into multiple tables such that each table is responsible for a part of actions

- [IMPORTANT] NOTE for exceeding power budget for the worst case of MAT access flow
	+ Reason: access too much SRAM/TCAM in the worst case
	+ Possible solutions
		- For MAT: **reduce MAT number**; reduce MAT size; add control flow to avoid unnecessary MAT access; split large MAT into small MATs if total size decreases
		- For reg: **reduce register size**
		- NOTE: distributing MAT entries into multiple stages does NOT work, as you need to access the same amount of TCAM/SRAM as in single stage

- [IMPORTANT] NOTEs for methodology change
	+ For hash/range partition strategy
		* Change USE_HASH/RANGE in helper.h
		* Change RANGE_SUPPORT in tofino-*/main.p4 and recompile
		* Change RANGE_SUPPORT in tofino-*/common.py
	+ For server rotation or not
		* Change bottleneck_partitionidx in config.ini and test_server_rotation.sh
			- NOTE: for different partition strategy (hash/range) and different workloads, bottleneck partition is different
		* Change workload_mode as 0 in config.ini
		* Change SERVER_ROTATION in helper.h

## Overview

+ Design for distributed extension of FarReach
	* Normal request/response + cache hit/miss
		- client.GETREQ -> client-leaf.GETREQ -> spine.GETREQ
			+ client-leaf processes GETREQ from client
				* spineselect_tbl sets eport and globalswitchidx to spine switch, while do NOT perform any MAT in following stages of both ingress and egress pipeline
				* NOTE: we focus on data center scenario; if client-side ToR switch is not programmable switch, we can resort client library to find corresponding spine switch or deploy such a partition in gateway switch of data center
				* NOTE: to avoid cache incohernce, distributed farreach only keeps at most one in-switch copy for each key in spine and leaf switches
			+ spine processes GETREQ from client-leaf
				+ cache_lookup_tbl matches op_hdr.key and op_hdr.globalswitchidx (set by leaf switch) into inswitch_hdr.is_cached/idx
				+ partition_tbl sets eport and globalswitchidx to leaf switch based on key
				+ prepare_for_cachehit_tbl saves client_sid for GETREQ (sid of client-leaf switch)
				+ ig_port_forward_tbl: convert GETREQ into GETREQ_INSWITCH
				+ Basically similar as single-switch egress processing; except that
					* NO CM-related MATs
					* Change save_client_udpport_tbl as save_client_info_tbl to save client_ip/mac/udpport (aka srcip/mac/udpport) into clone_hdr to prepare for cache hit
					* For cache hit, clone GETRES to client_sid (corresponding to client-leaf switch), and update dstip/mac/udppor as clientip/mac/udpport in clone_hdr
						- NOTE: update_ipmac_srcport_tbl does NOT change dstip/mac for server2client packet
					* For cache miss, forward GETREQ_SPINE to server-leaf switch
					* update_ipmac_srcport_tbl does NOT change dstip/mac for server2client (only change srcip/mac/port); change src/dst ip/mac and srcport for switch2switchos as in single-switch
						- NOTE: spine switch does NOT need client2server, as we do NOT need to change ip/mac from spine to leaf
						- NOTE: dstip/mac/port has been set as clone_hdr.client_ip/mac/port (saved by egress save_client_info_tbl) for normal response of cache hit
				+ NOTE: access snapshot_flag_tbl, hash_for_seq_tbl, sample_tbl, prepare_for_cachehit_tbl similar as single-switch
			+ If cache hit in spine: -> spine.GETRES -> client-leaf.GETRES -> client.GETRES
				+ client-leaf processes GETRES from spine
					* Forward GETRES to client based on IP in ipv4_forward_tbl
			+ If cache miss in spine: -> spine.GETREQ_SPINE -> server-leaf.GETREQ_SPINE
				+ server-leaf processes GETREQ_SPINE from spine
					* NOTE: NOT access hash_for_spineselect_tbl and spineselect_tbl
					* partition_tbl sets eport and udp.dstport to server based on key
					* cache_lookup_tbl matches op_hdr.key and op_hdr.globalswitchidx (set by spine switch) into inswitch_hdr.is_cached/idx
					* prepare_for_cachehit_tbl saves client_sid for GETREQ_SPINE (sid of spine switch)
					+ ig_port_forward_tbl: convert GETREQ_SPINE into GETREQ_INSWITCH
					* Basically similar as single-switch egress processing; except that
						* NO seq-related MATs
						* Change save_client_udpport_tbl as save_client_info_tbl to save client_ip/mac/udpport (aka srcip/mac/udpport) into clone_hdr to prepare for cache hit
						* For cache hit, clone GETRES to client_sid (corresponding to spine switch), and update dstip/mac/udppor as clientip/mac/udpport in clone_hdr
							- NOTE: update_ipmac_srcport_tbl does NOT change dstip/mac for server2client packet
						* For cache miss, convert GETREQ_INSWITCH as GETREQ to server as in single-switch
						* update_ipmac_srcport_tbl does NOT change dstip/mac for server2client (only change srcip/mac/port); change src/dst ip/mac and srcport for switch2switchos as in single-switch; change dstip/mac for client2server
							- NOTE: dstip/mac/port has been set as clone_hdr.client_ip/mac/port (saved by egress save_client_info_tbl) for normal response of cache hit
				* If cache hit in leaf: -> server-leaf.GETRES -> spine.GETRES -> client-leaf.GETRES -> client.GETRES
					* spine processes GETRES from server-leaf
						* Forward GETRES to client-leaf based on IP in ipv4_forward_tbl
					* client-leaf processes GETRES from spine
						* Forward GETRES to client based on IP in ipv4_forward_tbl
				* If cache miss in leaf: -> server-leaf.GETREQ -> server.GETREQ -> server.GETRES_SERVER -> server-leaf.GETRES_SERVER -> server-leaf.GETRES -> spine.GETRES -> client-leaf.GETRES -> client.GETRES
					* server-leaf processes GETRES_SERVER from server
						* Convert it as GETRES to spine switch based on IP in ipv4_forward_tbl and ig_port_forward_tbl
						* NOTE: spine/client-leaf process GETRES as mentioned before
		- client.SCANREQ -> client-leaf.SCANREQ -> spine.SCANREQ -> spine.SCANREQ_SPLIT -> server-leaf.SCANREQ_SPLIT -> server.SCANREQ_SPLIT -> server.SCANRES_SPLIT_SERVER -> server-leaf.SCANRES_SPLIT_SERVER -> server-leaf.SCANRES_SPLIT -> spine.SCANRES_SPLIT -> client-leaf.SCANRES_SPLIT -> client.SCANRES_SPLIT
			+ spine processes SCANREQ
				* partition_tbl sets op_hdr.globalswitchidx as the idx of first switch and resets split_hdr.cur_scanswitchidx as 0; partition_for_scan_endkey_tbl calculates split_hdr.max_scanswitchnum = last_scanswitchidx + 1 - op_hdr.globalswitchidx
				* process_scanreq_split_tbl calculates meta.remain_scannum based on cur_scanswitchidx and max_scanswitchnum, sets server_sid based on globalswitchidx + 1/2, and increases globalswitchidx for cloned SCANREQ_SPLIT
					- NOTE: we can increase cur_scanswitchidx in eg_port_forward_tbl, but CANNOT increase op_hdr.globalswitchidx, which will be used by leaf switch for cache lookup
			+ server-leaf processes SCANREQ_SPLIT
				* partition_tbl sets split_hdr.globalserveridx as the idx of first server based on both key and op_hdr.globalswitchidx (set by spine switch)
				* partition_for_scan_endkey_tbl gets last_scanidx based on both key and op_hdr.globalswitchidx, and calculates split_hdr.max_scannum = last_scanidx + 1 - globalserveridx
				* Remove update_scanreq_to_scanreq_split() in ig_port_forward_tbl
				* process_scanreq_split_tbl calculates meta.remain_scannum based on cur_scanidx and max_scannum, sets server_sid (and udp.dstport) based on globalserveridx
					- NOTE: we can increase globalserveridx in eg_port_forward_tbl
					- NOTE: we set pktlen for SCANREQ_SPLIT in both spine and server-leaf switch
			+ server-leaf forwards SCANRES_SPLIT_SERVER from server as SCANRES_SPLIT to spine switch
			+ spine and client-leaf switch forward SCANRES_SPLIT to client-leaf and client respectively
		- client.PUT/DELREQ -> client-leaf.PUT/DELREQ -> spine.PUT/DELREQ -> spine.PUT/DELREQ_INSWITCH
			+ If cache hit in spine, spine.PUT/DELRES -> client-leaf.PUT/DELRES -> client.PUT/DELRES
			+ If cache miss in spine, spine.PUT/DELREQ_SEQ -> server-leaf.PUT/DELREQ_SEQ -> server-leaf.PUT/DELREQ_SEQ_INSWITCH
				* If cache hit in server-leaf, server-leaf.PUT/DELRES -> spine.PUT/DELRES -> client-leaf.PUT/DELRES -> client.PUT/DELRES
				* If cache miss in server-leaf, server-leaf.PUT/DELREQ_SEQ -> server.PUT/DELREQ_SEQ -> server.PUT/DELRES_SERVER -> server-leaf.PUT/DELRES_SERVER -> server-leaf.PUT/DELRES -> spine.PUT/DELRES -> client-leaf.PUT/DELRES -> client.PUT/DELRES
		- client.WARMUP/LOADREQ -> client-leaf.WARMUP/LOADREQ -> spine.WARMUP/LOADREQ -> spine.WARMUP/LOADREQ_SPINE -> server-leaf.WARMUP/LOADREQ_SPINE -> server-leaf.WARMUP/LOADREQ -> server.WARMUP/LOADREQ -> server.WARMUP/LOADACK_SERVER -> server-leaf.WARMUP/LOADACK_SERVER -> server-leaf.WARMUP/LOADACK -> spine.WARMUP/LOADACK -> client-leaf.WARMUP/LOADACK -> client.WARMUP/LOADACK
	* Cache population/eviction
		- For cache population triggered by GET/PUTREQ in transaction phase
			+ NOTE: spine switch does NOT have CM registers for cache population, which is covered by server-leaf switch
			+ In FarReach, server-leaf.GETREQ_INSWITCH/PUTREQ_SEQ_INSWITCH -> server-leaf.GETREQ_POP/PUTREQ_POP_SEQ -> server.GETREQ_POP/PUTREQ_POP_SEQ -> cache population in control plane
			+ In DistCache, server-leaf.GETREQ_INSWITCH -> server-leaf.NETCACHE_GETREQ_POP and server-leaf.GETREQ -> reflector/switchos.NETCACHE_GETREQ_POP and server.GETREQ -> cache population in control plane and server.GETRES_SERVER as in single-switch
		- For cache population triggered by WARMUPREQ in warmup phase
			+ NOTE: in FarReach, server receives WARMUPREQ as mentioned before, and processes it for cache population
			+ In DistCache, spine.WARMUPREQ -> spine.NETCACHE_WARMUPREQ_INSWITCH -> spine.NETCACHE_WARMUPREQ_INSWITCH_POP and spine.WARMUPACK -> reflector/switchos.NETCACHE_WARMUPREQ_INSWITCH_POP and client-leaf.WARMUPACK -> cache population in control plane and client.WARMUPACK
		- For cache population/eviction in control plane
			+ Launch switchos with physical switch role (spine or leaf)
				* NOTE: each switchos can only read/write inswitch cache of physical spine or leaf switch
			+ For DistFarReach
				* NOTE: DistFarReach triggers cache population by servers -> key of CACHE_POP must NOT be cached -> key MUST be cached at most ONCE and hence in at most ONE switch
				* NOTE: to keep stateless controller, controller sends CACHE_POP_ACK to server only if receive CACHE_POP_ACK from spine/leaf, which means that some switchos.popworker has performed cache population/eviction for the key
					- TODOTODO: server sends CACNE_POP to controller by server.popclient
						+ NOTE: wait time for CACHE_POP_ACK in distributed scenario becomes larger than single-switch -> using server.popclient can avoid from affecting server.worker performance for normal requests
						+ TODOTODO: NOTE: now we still use server.worker.popclient instead of server.popclient as cache eviction is rare compared with normal requests
				* controller.popserver receives CACHE_POP from server.popclient
					- controller.popserver calculates logical spine/leafswitchidx based on the key to find physical spine/leaf switch
						+ NOTE: as we only have one physical spine/leaf switch, we validate the logical spine/leafswitchidx
					- controller.popserver randomly chooses spine/leaf switch to populate the key
					- controller.popserver.popclient_for_spine/leaf forwards CACHE_POP to switchos.popserver of sampled switch
					- controller.popserver.popclient_for_spine/leaf waits for CACHE_POP_ACK
					- controller.popserver forwards CACHE_POP_ACK to server.popclient, and waits for next CACHE_POP
				* For each physical spine/leaf switchos, after receiving CACHE_POP from controller.popserver.popclient
					- spine/leafswitchos.popserver sends CACHE_POP_ACK to controller.popserver.popclient_for_spine/leaf
					- spine/leafswitchos.popserver passes CACHE_POP to spine/leafswitchos.popworker
					- spine/leafswitchos.popworker calculates pipeidx and switchidx based on switchos role to populcate/evict key on cache_lookup_tbl
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
	* Conservative read / special type for cached keys
		- For GETRES_LATEST/DELETED_SEQ in DistFarReach
			+ Server sends GETRES_LATEST/DELETED_SEQ to server-leaf as in single-switch
			+ Server-leaf ipv4_forward_tbl matches inswitch_hdr.is_cached
				* If is_cached=1, clone GETRES_LATEST/DELETED_SEQ to spine switch by clone_i2e, and convert original pkt as GETRES_LATEST/DELETED_SEQ_INSWITCH to pipeline of ingress port (aka server pipeline)
				* If is_cached=0, convert original pkt as GETRES_LATEST/DELETED_SEQ_INSWITCH to pipeline of spine switch
				* NOTE: packet type conversion is performed by ig_port_forward_tbl
			+ For GETRES_LATEST/DELETED_SEQ (MUST cloned to spine switch by clone_i2e), server-leaf eg_port_forward_tbl converts the pkt into GETRES as in single-switch
				* NOTE: GETRES is processed by spine/client-leaf switch as mentioned before
			+ For GETRES_LATEST/DELETED_SEQ_INSWITCH in server-leaf switch
				* If is_cached=1 (eport MUST be server devport), try to update egress registers and finally be dropped by drop_tbl as in single-switch
				* If is_cached=0 (eport MUST be spine switch devport), eg_port_forward_tbl converts it back to GETRES_LATEST/DELETED_SEQ, which will NOT be dropped by drop_tbl
					- NOTE: we do NOT need to access update_pktlen_tbl and update_ipmac_srcport_tbl for GETRES_LATEST/DELETED_SEQ in server-leaf switch
					- NOTE: due to is_cached=0 for GETRES_LATEST/DELETED_SEQ_INSWITCH entering pipeline of spine switch, we must NOT change any egress registers
					- NOTE: we NEED to access add_and_remove_value_header_tbl for GETRES_LATEST/DELETED_SEQ to hold vallen and value headers
			+ Spine switch clones GETRES_LATEST/DELETED_SEQ to client-leaf switch by clone_i2e, and convert original pkt as GETRES_LATEST/DELETED_SEQ_INSWITCH to pipeline of ingress port (aka server-leaf pipeline) as in single-switch
			+ Spine switch converts GETRES_LATEST/DELETED_SEQ (MUST cloned to client) as GETRES as in single-switch
			+ Spine switch tries to update registers for GETRES_LATEST/DELETED_SEQ_INSWITCH, and dropped by drop_tbl as in single-switch
		- In DistCache, NETCACHE_PUT/DELREQ_SEQ_CACHED
			+ client.PUT/DELREQ -> client-leaf.PUT/DELREQ -> spine.PUT/DELREQ -> spine.NETCACHE_PUT/DELREQ_SEQ_CACHED
			+ NOTE: it means that the key is cached in spine switch -> the key MUST be cached in corresponding server-leaf switch
			+ server-leaf switch processes NETCACHE_PUT/DELREQ_SEQ_CACHED as PUT/DELREQ_SEQ from spine switch
				* server-leaf.NETCACHE_PUT/DELREQ_SEQ_CACHED -> server-leaf.PUT/DELREQ_SEQ_INSWITCH -> server-leaf.NETCACHE_PUT/DELREQ_SEQ_CACHED
				* NOTE: as the key MUST be cached in server-leaf switch here, the pkt must become NETCACHE_PUT/DELREQ_SEQ_CACHED to server
				* NOTE: even if the key is NOT cached in server-leaf due to rare reason (e.g., nearly impossible fast cache eviction) and the pkt is not marked as XXX_CACHED to server, server can still perform correctly based on its inswitch cache metadata (i.e., beingcached/cached/beingupdated keyset)
	* Crash-consistent snapshot and special cases
		- NOTE: NOT affect read queries and the queries outside transaction phase including GET/WARMUP/LOADREQ/_SPINE and SCANREQ/_SPLIT
			+ NOTE: by doing this, we can recirculate/forward pkt for atomic snapshot yet not rely on ingress/egress port
		- spine.enable_singlepath
			+ For need_recirculate_tbl (matching op_hdr.optype, not affect other pkts) and recirculate_tbl, spine switch simply recirculates write traffic (including PUT/DELREQ/_SEQ and GETRES_LATEST/DELETED_SEQ/_SERVER) to the pipeline of the first client-side leaf switch
				* TODOTODO: NOTE: if we have multiple physical spine switches, non-first physical spine switches need to forward their write traffic to the first physical spine switch, similar as server-leaf switches
					- For example, client-leaf sends PUTREQ to 2nd spine switch -> 2nd spine switch converts PUTREQ as PUTREQ_SINGLEPATH to server-leaf switch -> server-leaf switch forwards PUTREQ_SINGLEPATH to the first physical spine switch, which read snapshot flag and sends PUTREQ_SINGLEPATH_INSWITCH back to server-leaf switch -> server-leaf switch sends PUTREQ_SINGLEPATH_INSWITCH to 2nd spine switch based on key by spineselect_tbl (as in client-leaf switch)
					- NOTE: although ingress port may NOT be the corresponding client-leaf switch for cache hit, we can directly clone a PUT/DELRES to server-leaf switch, which will forward the response to corresponding client based on IP
				* NOTE: now we only have one physical spine switch, so we only need recirculation for unmatched ingress ports instead of forwarding to other spine switches; as we only have one physical leaf switch, we do NOT have unmatched ingress ports
			+ For eg_port_forward_tbl, spine switch only needs to consider CASE1 (inswitch value change) as in single-switch -> remove CASE3-related MAT entries
				- NOTE: CASE2 (inswitch key change) is covered by controller/switchos; CASE3 (inserver value change) is covered by leaf switch
		- server-leaf.enable_singlepath
			+ NOTE: server-leaf write traffic from client (e.g., PUT/DELREQ) is NOT affected by singlepath, which is still directly forwarded to correpsonding spine switch as usual
			+ For PUT/DELREQ_SEQ
				* server-leaf.PUT/DELREQ_SEQ -> firstspine.PUT/DELREQ_SEQ
					- NOTE: as need_recirculate=1, server-leaf will not change it as PUT/DELREQ_SEQ_INSWITCH to egress of server
					- NOTE  NOT access update_pktlen_tbl as NOT change packet header; NOT access update_ipmac_srcport_tbl as spine switch does NOT concern; access add_and_remove_value_header_tbl for PUTREQ_SEQ
				* firstspine.PUT/DELREQ_SEQ -> firstspine.PUT/DELREQ_SEQ_INSWITCH w/ snapshot_flag
					- NOTE: firstspine set eport based on key to server-leaf
					- NOTE  NOT access update_pktlen_tbl as server-leaf will remove inswitch_hdr before sending to server; NOT access update_ipmac_srcport_tbl as it is covered by server-leaf; access add_and_remove_value_header_tbl for PUTREQ_SEQ_INSWITCH
				* server-leaf processes PUT/DELREQ_SEQ_INSWITCH as PUT/DELREQSEQ from spine switch
					- NOTE: NOT access need_recirculate_tbl and snapshot_flag_tbl for PUT/DELREQ_SEQ_INSWITCH
					- NOTE: although ingress port may NOT be the corresponding spine switch for cache hit, we can directly clone a PUT/DELRES to the first spine switch, which will forward the response to corresponding client based on IP
			+ For GETRES_LATEST/DELETED_SEQ
				* server-leaf.GETRES_LATEST/DELETED_SEQ_SERVER -> firstspine.GETRES_LATEST/DELETED_SEQ_SERVER
					- NOTE: as need_recirculate=1, ig_port_forward_tbl will NOT change it as GETRES_LATEST/DELETED_SEQ_INSWITCH (and clone GETRES to spine), and partition_tbl/ipv4_forward_tbl will NOT set eport again after recirculate_tbl
					- NOTE: NOT access update_pktlen_tbl as NOT change packet header; NOT access update_ipmac_srcport_tbl as spine switch does NOT concern; access add_and_remove_value_header_tbl for GETRES_LATEST/DELETED_SEQ_SERVER
				* firstspine.GETRES_LATEST/DELETED_SEQ_SERVER -> firstspine.GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH w/ snapshot_flag
					- firstspine set eport based on key to server-leaf
					- NOTE  NOT access update_pktlen_tbl as server-leaf will remove inswitch_hdr before sending to spine again; NOT access update_ipmac_srcport_tbl as it is covered by server-leaf; access add_and_remove_value_header_tbl for GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH
				* server-leaf processes GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH as GETRES_LATEST/DELETED_SEQ_SERVER from spine switch
					- NOTE: NOT access need_recirculate_tbl and snapshot_flag_tbl for GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH
					- NOTE: ingress port change does NOT affect speical response forwarding, which is based on dstip
			+ For eg_port_forward_tbl, server-leaf switch needs to consider both CASE1 (inswitch value change) and CASE3 (inserver value change) as in single-switch
				- NOTE: CASE2 (inswitch key change) is covered by controller/switchos

## Implementation log

+ Implement DistFarReach
	* Replace "netreach-v4-lsm" or "netbufferv4" into "distfarreach" -> sync to distnocache and distcache
		- Software: io_helper.c, test_server_rotation*.sh, sync_file.sh
		- Hardware: ptf_popserver.sh (and table_configure.py), ptf_snapshotserver.sh (and table_configure.py), recover_switch.sh (and table_configure.py), ptf_cleaner.sh (and table_configure.py), configure.sh (and table_configure.py), compile.sh, start_switch.sh, main.p4
	* Create tofino-spine for spine switch and tofino-leaf for leaf switch -> sync to distnocache and distcache
		- Replace "tofino/" as "tofino-spine/" or "tofino-leaf/" (files: ptf_popserver.sh, ptf_snapshotserver.sh, recover_switch.sh, , compile.sh, ptf_cleaner.sh, configure.sh)
		- Replace "distfarreach" as "distfarreachspine" or "distfarreachleaf" for P4 program name and main file name (files: ptf_popserver.sh (and table_configure.py), ptf_snapshotserver.sh (and table_configure.py), recover_switch.sh (and table_configure.py), ptf_cleaner.sh (and table_configure.py), configure.sh (and table_configure.py), compile.sh, start_switch.sh, main.p4)
	* NOTE: client is NOT changed -> sync to distnocache and distcache
	* Normal request/response + cache hit/miss
		- For GETREQ -> sync to distnocache and distcache
			+ Add spine switch and leaf switch configuration (files: config.ini, iniparser/iniparser_wrapper.*, common_impl.h, tofino-*/common.py)
			+ Add globalswitchidx into op_hdr (files: tofino-*/p4src/header.p4, packet_format*)
				* Update update_pktlen_tbl accordingly (files: tofino-*/p4src/egress_mat.p4, tofino-*/configure/table_configure.py)
			+ Client-leaf switch forwards GETREQ from client to spine switch
				* NOTE: for client-leaf switch transferring pkts from client to spine switch, it does NOT belong to KVS system (NO inswitch cache) and NOT need to know range/hash partition strategy of server-side
					- Client-side leaf switch simply selects spine by hashing (FarReach: fixed; DistCache: power-of-two choices)
					- Server-side spine/leaf switches need to know range/hash partition strategy to choose leaf switch / server
					- Actually, we use a single physical leaf switch to simulate both client-side and server-side leaf switch
				* Add hash_for_spineselect_tbl in stage 0 of ingress pipeline (NOT match meta.need_recirculate; NOT concern RANGE_SUPPORT) and meta.hashval_for_spineselect (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/p4src/header.p4, tofino-leaf/main.p4, tofino-leaf/configure/table_configure.py)
				* Add spineselect_tbl in stage 1 to set eport and globalswitchidx based on meta.hashval_for_spine (files: tofino-leaf/p4src/ingres_mat.p4, tofino-leaf/main.p4, tofino-leaf/configure/table_configure.py)
					+ NOTE: it cannot in the same stage as hash/range_partition_tbl
				* Match GETREQ_SPINE from spine instead of GETREQ from client in hash_for_partition_tbl and range/hash_partition_tbl in leaf switch (files: tofino-leaf/configure/table_configure.py)
					+ NOTE: leaf.hash/range_partition_tbl do not process GETREQ from client, while spine.hash/range_partition_tbl do
			+ Spine switch processes GETREQ from clinet-leaf switch
				* Remove client_devports and server_devports from spine switch; add reflector_devport (see details later about adding spine/leaf reflector for cache population/eviction)
				* Match op_hdr.globalswitchidx in cache_lookup_tbl and move it from stage 2 to stage 1 (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/main.p4, tofino-spine/ptf_popserver/table_configure.py)
				* Forward GETREQ to leaf switch by range/hash partition_tbl (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configure/table_configure.py)
					- NOTE: spine switch does not need to change udp.dstport in partition_tbl as leaf switch NOT concern
				* Bind leafswitch.sid to leafswitch.devport in spine switch; save clientleafswitch.sid as client_sid in prepare_for_cachehit_tbl (files: tofino-spine/configure/table_configure.py)
				* Convert GETREQ as GETREQ_INSWITCH in ig_port_forward_tbl (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configure/table_configure.py)
				* Remove CM-related MATs including hash_for_cmX_tbl, access_cmX_tbl, is_hot_tbl; remove meta.cmX_predicate and meta.is_hot (files: tofino-spine/main.p4, tofino-spine/p4src/regs/cm.p4, tofino-spine/p4src/ingress_mat.p4, tofino-spine/p4src/egress_mat.p4, tofino-spine/configure/table_configure.py)
					- Remove meta.is_hot and population-related actions from eg_port_forward_tbl in spine switch (files: tofino-spine/configure/table_configure.py, tofino-spine/p4src/egress_mat.p4)
				* Add clone_hdr.client_ip and clone_hdr.client_mac (files: tofino-*/p4src/header.p4, packet_format.h)
				* Replace save_client_udpport_tbl as save_client_info_tbl to store client.ip/mac/udpport into clone_hdr to prepare for cache hit (files: tofino-*/main.p4, tofino-*/p4src/egress_mat.p4, tofino-*/configure/table_configure.py)
				* For cache hit, set dst.ip/mac/udpport as clone_hdr.client.ip/mac/udpport in update_getreq_inswitch_to_getres_by_mirroring() of eg_port_forward_tbl (files: tofino-*/p4src/egress_mat.p4, tofino-*/configure/table_configure.py)
				* For cache miss, replace update_getreq_inswitch_to_getreq() as update_getreq_inswitch_to_getreq_spine() in eg_port_forward_tbl (files: tofino-spine/p4src/egress_mat.p4, tofino-spine/configure/table_configure.py)
					- Add GETREQ_SPINE (only optype enumeration, not parsed by end-hosts) (files: tofino-*/main.p4, tofino-*/common.py, tofino-*/p4src/parser.py, packet_format.h)
				* For update_ipmac_srcport_tbl, remove client2server; keep switch2switchos; NOT update dst.ip/mac in server2client (files: tofino-spine/p4src/egress_mat.p4, tofino-spine/configure/table_configure.py)
					- NOTE: fix missing PUT/DELREQ_SEQ_INSWITCH_CASE1 of switch2switchos in update_ipmac_srcport_tbl (files: netreach-v4-lsm/tofino/configure/table_configure.py) -> fix an issue of clone_hdr.clonenum_for_pktloss -> test correctness and effect on system performance w/ snapshot (see details in [farreach document](./netreach-v4-lsm.md))
			+ Client-leaf switch processes GETRES from spine switch (if cache hit in spine)
				* ipv4_forward_tbl forwards GETRES from spine to client based on dstip
					- NOTE: FarReach needs to recirculate for PUT/DELREQ and GETRES_LATEST/DELETED_SEQ if necessary (files: distfarreach/tofino-*/configure/table_configure.py, distfarreach/tofino-*/p4src/ingress_mat.p4, netreach-v4-lsm/tofino/configure/table_configure.py, netreach-v4-lsm/tofino/p4src/ingress_mat.p4)
					- Re-compile farreach -> test dynamic workload (especially for configure.sh) -> update visulization/netreachv4-nodebug-hashpartition
				* NOTE: client-leaf does NOT change dst.ip/mac/udpport for server2client in update_ipmac_srcport_tbl as it must have been set by spine/server-leaf switch or server (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/configure/table_configure.py)
			+ Server-leaf switch processes GETREQ_SPINE from spine switch (if cache miss in spine)
				* Access hash_for_partition_tbl and hash/range_partition_tbl to set eport and udp.dstport for GETREQ_SPINE (files: tofino-leaf/configure/table_configure.py)
				* Implement update_getreq_spine_to_getreq_inswitch() in ig_port_forward_tbl (files: tofino-leaf/p4src/ingress_mat.p4)
				* Match op_hdr.globalswitchidx in cache_lookup_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/ptf_popserver/table_configure.py)
					- NOTE: NOT need to move cache_lookup_tbl one stage before, as range/hash_partition_tbl does NOT change op_hdr.globalswitchidx in leaf switch
				* Access hash_for_cmX_tbl, prepare_for_cachehit_tbl (bind spineswitch.sid to spineswitch.devport; set client_sid as spineswitch.sid), sample_tbl for GETREQ_SPINE (files: tofino-leaf/configure/table_configure.py)
				* Remove seq-related MATs including hash_for_seq_tbl, and access_seq_tbl (files: tofino-leaf/main.p4, tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py, tofino-leaf/p4src/regs/seq.p4)
				* Access save_client_info_tbl for GETREQ_INSWITCH as in spine switch (files: tofino-leaf/configure/table_configure.py)
				* For cache hit, convert GETREQ_INSWITCH as GETERS and clone it to spineswitch.sid w/ clone_hdr.client_ip/mac/udpport (files: tofino-leaf/configure/table_configure.py)
					- NOTE: for cache miss, use update_getreq_inswitch_to_getreq() as in single-switch
				* NOTE: for request pkt from client to spine switch, due to eport = spineswitch.devport instead of server.devport, it will NOT change dstip/mac as client2server in update_ipmac_srcport_tbl
			+ Spine switch and client-leaf process GETRES from server-leaf switch (if cache hit in server-leaf)
				* Spine switch forwards GETRES to client-leaf switch based on dstip in ipv4_forward_tbl (files: tofino-spine/configure/table_configure.py)
				* Client-leaf switch forwards GETRES to client based on dstip in ipv4_forward_tbl as in single-switch
			+ Server-leaf processes GETRES_SERVER from server (if cache miss in server-leaf)
				* Add GETRES_SERVER (both optype enumeration and implementation) (files: tofino-*/main.p4, tofino-*/common.py, tofino-*/p4src/parser.p4, packet_foramt.*, common_impl.h)
				* Server generates GETRES_SERVER to server-leaf switch (files: server_impl.h)
				* Server-leaf switch forwards GETRES_SERVER to spine switch in ipv4_forward_tbl, and converts it as GETRES in ig_port_forward_tbl (tofino-leaf/configure/table_configure.py, tofino-leaf/p4src/ingress_mat.p4)
				* NOTE: spine/client-leaf switch process GETRES as mentioned before
		- For SCANREQ -> sync to distnocache/distcache
			+ Client-leaf switch forwards SCANREQ to spine switch by spineselect_tbl as GETREQ
			+ Spine switch processes SCANREQ from client-leaf switch
				* Add cur_scanswitchidx and max_scanswitchnum in split_hdr (files: tofino-*/p4src/header.p4, packet_format.*)
				* partition_tbl sets eport and op_hdr.globalswitchidx as GETREQ
				* range_partition_for_scan_endkey_tbl calculates max_scanswitchnum = last_scanswitchidx + 1 - globalswitchidx (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configure/table_configure.py)
				* process_scanreq_tbl sets server_sid based on globalswitchidx + 1/2, calculates remain_scannum = max_scanswitchnum - cur_scanswitchidx; and increases globalswitchidx if is_clone=1 (NOTE: not in eg_forward_tbl) (files: tofino-spine/p4src/egress_mat.p4, tofino-spine/configure/table_configure.py)
					- NOTE: spine switch does NOT set udp.dstport for SCANREQ_SPLIT, which is covered by partition_tbl and process_scanreq_split_tbl in server-leaf switch
				* forward_scanreq_split_and_clone() and forward_scanreq_split() update split_hdr.cur_scanswitchidx yet NOT update op_hdr.globalswitchidx (also NOT update split_hdr.globalserveridx) in eg_port_forward_tbl (files: tofino-spine/p4src/egress_mat.p4)
				* Update udprecvlarge_multisrc to consider multiple cur_scanswitchidx (different leaf switches) and cur_scanidx (diferent servers) (files: socket_helper.*, packet_format.*, server_impl.h, remote_client.c)
			+ Server-leaf switch processes SCANREQ_SPLIT from spine switch
				* range_partition_tbl and range_partition_for_scan_endkey_tbl match both op_hdr.key and op_hdr.globalswitchidx to set udp.dstport, eport, clone_hdr.globalserveridx, cur_scanidx, and max_scannum (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
					- For example, we simulate 16 logical leaf switches, each corresponds to 8 simulated servers
					- Given a globalswitchidx, it corresponds to a key range [keystart, keyend], which covers 8 simulated servers
					- For key in [min, keystart + key_range_per_server], we always assign the smallest serveridx
					- For key in [keyend - key_range_per_server, max], we always assign the largest serveridx
					- Therefore, we can always assign a valid serveridx (8 servers covered by the leaf switch of globalswitchidx) for startkey / endkey to set globalserveridx and max_scannum (must <= 8)
				* Remove update_scanreq_to_scanreq_split() in ig_port_forward_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
				* As in single-switch, process_scanreq_split_tbl updates server_sid for next SCANREQ_SPLIT based on globalserveridx + 1; calculates meta.remain_scannum based on cur_scanidx and max_scannum; updates udp.dstport for cloned pkt based on globalserveridx (NOTE: udp.dstport of the first SCANREQ_SPLIT has been set by range_partition_tbl in ingress pipeline)
					- NOTE: eg_port_forward_tbl increases globalserveridx as cur_scanidx
			+ Server-leaf processes SCANRES_SPLIT_SERVER from server
				* Add packet type of SCANRES_SPLIT_SERVER (files: tofino-*/main.p4, tofino-*/common.py, tofino-*/p4src/parser.p4, packet_format.*, common_impl.h)
				* Server answers SCANRES_SPLIT_SERVER to server-leaf (files: server_impl.h)
				* Server-leaf forwards SCANRES_SPLIT_SERVER to spine switch in ipv4_forward_tbl (files: tofino-leaf/configure/table_configure.py)
				* Server-leaf converts SCANRES_SPLIT_SERVER as SCANRES_SPLIT in ig_port_forward_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
				* NOTE: spine/client-leaf switch forward SCANRES_SPLIT based on dstip in ipv4_forward_tbl like GETRES as mentioned before
		- For PUT/DELREQ -> sync to distnocache/distcache
			+ Client-leaf forwards PUT/DELREQ to spine switch by spineselect_tbl (files: tofino-leaf/configure/table_configure.py)
			+ Spine switch processes PUT/DELREQ from client-leaf as in single-switch
				* Ingress forwards PUT/DELREQ to corresponding server-leaf switch in partition_tbl, and converts PUT/DELREQ into PUT/DELREQ_INSWITCH in ig_port_forward_tbl as in single-switch
				* Egress accesses cache_frequency_reg, validvalue_reg, seq_reg, save_client_info_tbl, latest_reg, deleted_reg, vallen_reg, savedseq_reg, case1_reg, valX_reg as in single-switch
				* For cache hit, set dst.ip/mac/udpport as clone_hdr.client.ip/mac/udpport in update_put/delreq_inswitch_to_put/delres_by_mirroring() of eg_port_forward_tbl (files: tofino-spine/p4src/egress_mat.p4, tofino-spine/configure/table_configure.py)
					- NOTE: server2client in update_ipmac_srcport_tbl does NOT change dstip/mac; update pktlen for PUT/DELRES
				* For cache miss, invoke update_put/delreq_inswitch_to_put/delreq_seq() in eg_port_forward_tbl as in single-switch
					- NOTE: spine switch does NOT have client2server in update_ipmac_srcport_tbl; yet update pktlen for PUT/DELREQ_SEQ
			+ Client-leaf switch forwards PUT/DELRES to client based on IP in ipv4_forward_tbl as in single-switch
			+ Server-leaf switch processes PUT/DELREQ_SEQ
				* Ingress forwards PUT/DELREQ_SEQ to server in partition_tbl (files: tofino-leaf/configure/table_configure.py)
				* Ingress converts PUT/DELREQ_SEQ into PUT/DELREQ_SEQ_INSWITCH in ig_port_forward_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
					- Add packet type of PUT/DELREQ_SEQ_INSWITCH (only optype enumeration) (files: tofino-*/main.p4, tofino-*/common.py, tofino-*/p4src/parser.p4, packet_format.h)
				* Egress accesses cmX_reg, cache_frequency_reg, validvalue_reg, save_client_info_tbl, latest_reg, deleted_reg, vallen_reg, savedseq_reg, case1_reg for PUT/DELREQ_SEQ_INSWITCH (files: tofino-leaf/configure/table_configure.py)
					- TODOTODO: Now we do NOT update cmX_reg and cache_frequency_reg for DELREQ_INSWITCH in FarReach (netreach-v4-lsm)
				* For cache hit, set dst.ip/mac/udpport as clone_hdr.client.ip/mac/udpport in update_put/delreq_seq_inswitch_to_put/delres_by_mirroring() of eg_port_forward_tbl (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/configure/table_configure.py)
					- NOTE: server2client in update_ipmac_srcport_tbl does NOT change dstip/mac; update pktlen for PUT/DELRES
				* For cache miss, replace update_put/delreq_inswitch_to_put/delreq_seq() as update_put/delreq_seq_inswitch_to_put/delreq_seq() in eg_port_forward_tbl (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/configure/table_configure.py)
					- NOTE: server-leaf switch invokes client2server in update_ipmac_srcport_tbl; and update pktlen for PUT/DELREQ_SEQ
			+ Server-leaf processes PUT/DELRES_SERVER from server
				* Add packet type of PUT/DELRES_SERVER (files: tofino-*/main.p4, tofino-*/common.py, tofino-*/p4src/parser.p4, packet_format.*, common_impl.h)
				* Server sends PUT/DELRES_SERVER to server-leaf switch (files: server_impl.h)
				* Server-leaf forwards PUT/DELRES_SERVER to spine switch in ipv4_forward_tbl (files: tofino-leaf/configure/table_configure.py)
				* Server-leaf converts PUT/DELRES_SERVER as PUT/DELRES in ig_port_forward_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
				* Spine and client-leaf switch forward PUT/DELRESR to client-leaf and client respectively in ipv4_forward_tbl like SCANRES_SPLIT/GETRES as mentioned before
		- For WARMUP/LOADREQ -> sync to distnocach/distcache
			+ Client-leaf switch forwards WARMUP/LOADREQ to spine switch by spineselect_tbl (files: tofino-leaf/configure/table_configure.py)
			+ Spine switch forwards WARMUP/LOADREQ to server-leaf switch by partition_tbl (files: tofino-spine/configure/table_configure.py)
			+ Spine switch converts WARMUP/LOADREQ as WARMUP/LOADREQ_SPINE at ig_port_forward_tbl, which ignore most MATs in egress pipeline (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configure/table_configure.py)
				* Add packet type of WARMUP/LOADREQ_SPINE (only optype enumeration) (files: tofino-*/main.p4, tofino-*/common.py, tofino-*/p4src/parser.p4, packet_format.h)
				* Access add_and_remove_value_header_tbl for LOADREQ_SPINE (files: tofino-spine/configure/table_configure.py)
			+ Server-leaf switch forwards WARMUP/LOADREQ_SPINE to server by partition_tbl (files: tofino-leaf/configure/table_configure.py)
			+ Server-leaf switch converts WARMUP/LOADREQ_SPINE into WARMUP/LOADREQ in ig_port_forward_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
			+ Server sends WARMUP/LOADACK_SERVER to server-leaf (files: server_impl.h)
				* Add packet type of WARMUP/LOADACK_SERVER (optype enumeration + implementation) (files: tofino-*/main.p4, tofino-*/common.py, tofino-*/p4src/parser.p4, packet_format.*, common_impl.h)
			+ Server-leaf switch forwards WARMUP/LOADACK_SERVER as WARMUP/LOADACK to spine by ipv4_forward_tbl and ig_port_forward_tbl (files: tofino-leaf/configure/table_configure.py, tofino-leaf/p4src/ingress_mat.p4)
			+ Spine and client-leaf switch forward WARMUP/LOADACK to client-leaf and client respectively by ipv4_forward_tbl as in single-switch (files: tofino-*/configure/table_configure.py)
	* Cache population/eviction
		- For GETREQ_POP/PUTREQ_POP_SEQ/WARMUPREQ of cache population
			+ Server-leaf switch converts GETREQ_INSWITCH as GETREQ_POP to server as in single-switch
			+ Server-leaf switch converts PUTREQ_SEQ_INSWITCH as PUTREQ_POP_SEQ to server (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/configure/table_configure.py)
				* Server sends PUTRES_SERVER to server-leaf for PUTREQ_POP_SEQ, and triggers cache population (files: server_impl.h)
			+ Server-leaf coverts WARMUPREQ_SPINE as WARMUPREQ to server for cache population as mentioned before
		- Add spine/leaf.reflector -> sync to distnoachce/distcache
			+ Change original reflector configuration as relfector_for_leaf and reflector_for_spine (files: config.ini, configs/\*, common_impl.h, iniparser/iniparser_wrapper.*, tofino-*/common.py)
				* NOTE: we use different udp ports for reflector_for_leaf/spine as they may be launched in the same physical machine -> reserve the udp ports for link simulation (files: tofino-spine/p4src/parser.p4)
			+ Add reflector_for_spine.reflector_ip/mac/fpport_for_switch for dp2cp; and configure ip addrs of the NICs in physical client/server (config.ini, configs/\*, common_impl.h, iniparser/iniparser_wrapper.*, tofino-*/common.py, configure_client/server.sh)
				* NOTE: reflector_ip_for_switchos is used for cp2dp; reflector_ip_for_switch is used for dp2cp 
				* NOTE: reflector_for_leaf can directly use client/server.ip/mac/fpport as reflector_ip/mac/fpport_for_switch, yet reflector_for_spine CANNOT due to different NICs
			+ Launch reflector individually instead of as a module in server.c (files: reflector_impl.h, reflector.c, server.c, Makefile)
				* NOTE: we need to launch reflector_for_spine/leaf explicitly (files: docs/distfarreach.md)
				* NOTE: we check reflector_for_spine/leaf.reflector_ip_for_switchos (files: reflector.c)
				* NOTE: reflector/switchos use different switchos.ip and reflector.ip/port based on role (files: reflector.c, switchos.c)
			+ switchos uses reflector_for_leaf/spine based on phyiscal switchos role (switchos.c)
			+ spine/leaf switch uses reflector_for_leaf/spine respectively
				* NOTE: reflector_for_leaf.ip/mac/devport/sid_for_switch must be a client/server.devport/sid (tofino-leaf/configure/table_configure.py)
				* NOTE: reflector_for_spine.ip/mac/devport/sid_for_switch must be an individual ip/mac/devport/sid (tofino-spine/configure/table_configure.py)
		- For cache population/eviction in control plane -> sync to distnocache/distcache
			+ Add reflector_for_spine/leaf.cp2dp_dstip in configuration such that reflector.cp2dpserver can send pkt to spine/leaf switch (files: config,ini, configs/\*, iniparser/iniparser_wrapper.\*, common_impl.h, reflector_impl.h, reflector.c)
			+ controller.popserver validates logical spine/leafswitchidx based on the key (files: controller.c)
				* Implement Key::get_spine/leafswitch_idx() (files: key.\*)
			+ controller.popserver samples spine/leaf for cache population (files: controller.c)
			+ controller.popserver.popclient_for_spine/leaf sends CACHE_POP to sampled switchos.popserver, and waits for CACHE_POP_ACK (files: controller.c)
				* NOTE: controller needs to communiate with both spine/leaf switchos -> need two sets of global variables; while reflector/switchos can update exclusive global variables based on its role (spine or leaf)
			+ controller.popserver forwards CACHE_POP_ACK to corresponding server (files: controller.c)
			+ spine/leafswitchos.popserver processes CACHE_POP from controller.popserver as in single-switch
			+ spine/leafswitchos.popworker calculates pipeidx based on role (spine -> leafswitch.pipeidx; leaf -> server.pipeidx) (files: switchos.c)
			+ spine/leafswitchos.popworker calculates switchidx based on role and key (spine -> spineswitchidx; leaf -> leafswitchidx) (files: switchos.c, tofino-*/ptf_popserver/table_configure.py)
				* NOTE: we check spineswitchnum <= leafswitchnum <= servernum; and servernum % leafswitchnum = 0 in common module (files: common_impl.h)
	* Conservative read w/ special responses
		- Server-leaf processes GETRES_LATEST/DELETED_SEQ from server
			+ Add inswitch_hdr.is_cached into ipv4_forward_tbl match fields (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
			+ For GETRES_LATEST/DELETED_SEQ, we implement forward_special_get_response_to_server_and_clone_to_spine() for is_cached=1 and forward_special_get_response_to_spine() for is_cached=0 in ipv4_forward_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
				* NOTE: we convert GETRES_LATEST/DELETED_SEQ as GETRES_LATEST/DELETED_SEQ_INSWITCH in ig_port_forward_tbl as in single-switch
			+ eg_port_forward_tbl converts GETRES_LATEST/DELETED_SEQ into GETRES as in single-switch
			+ eg_port_forward_tbl converts GETRES_LATEST/DELETED_SEQ_INSWITCH as GETRES_LATEST/DELETED_SEQ if is_cached=0; or do NOT change optype if is_cached=1, which will be dropped by drop_tbl later (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/configure/table_configure.py)
			+ access add_and_remove_value_header_tbl for GETRES_LATEST/DELETED_SEQ to avoid remove_all() (files: tofino-leaf/configure/table_configure.py)
		- Spine switch processes GETRES_LATEST/DELETED_SEQ from server-leaf as in single-switch
	* Crash-consistent snapshot and special cases
		- Enable single path in spine switch
			+ NOTE: we consider PUT/DELREQ from client-leaf; GETRES_LATEST/DELETED_SEQ from server-leaf
				* NOTE: for PUT/DELREQ_SEQ and GETRES_LATEST/DELETED_SEQ_SERVER from server-leaf, we should process them in server-leaf.egress instead of spine.egress (see details below, i.e., enable singlepath in server-leaf switch)
				* Add packet type of GETRES_LATEST/DELETED_SEQ_SERVER (files: tofino-*/main.p4, tofino-*/common.py, tofino-*/p4src/parser.p4, packet_format,*, common_impl.h)
				* Server sends GETRES_LATEST/DELETED_SEQ_SERVER to server-leaf (files: server_impl.h)
				* server-leaf ingress processes GETRES_LATEST/DELETED_SEQ_SERVER as mentioned before, which converts original pkt as GETRES_LATEST/DELETED_SEQ_INSWITCH to egress based on is_cached (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py, ptf_snapshotserver/table_configure.py)
			+ Access need_recirculate_tbl for the ingress ports which are NOT in the same pipeline of the first client-leaf switch (files: tofino-spine/ptf_snapshotserver/table_configure.py)
			+ recirculate_tbl recirculates write traffic to the pipeline of the first client-leaf switch (files: tofino-spine/configure/table_configure.py)
			+ eg_port_forward_tbl generates GETRES_LATEST/DELETED_SEQ_INSWITCH_CASE1 and PUT/DELREQ_SEQ_INSWITCH_CASE1 as in single-switch
			+ For special cases
				* Generate CASE1 for PUT/DELREQ_INSWITCH and GETRES_LATEST/DELETED_SEQ_INSWITCH as in single-switch
				* Remove CASE3-related actions and MAT entries (files: tofino-spine/p4src/egress_mat.p4, tofino-spine/configure/table_configure.py)
		- Enable single path in server-leaf switch
			+ server-leaf processes PUT/DELREQ_SEQ from spine switch
				* Access need_recirculate_tbl and snapshot_flag_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py, tofino-leaf/ptf_snapshotserver/table_configure.py)
					- NOTE: need_recirculate_tbl does NOT need to match iport in server-leaf switch
					- TODOTODO: NOTE: need_recirculate_tbl does NOT need to match iport in non-first spine switch
				* recirculate_tbl forwards PUT/DELREQ_SEQ to the first spine switch (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf.configure/table_configure.py)
			+ spine processes PUT/DELREQ_SEQ from server-leaf switch
				* Access need_recirculate_tbl and recirculate_tbl to recirculate PUT/DELREQ_SEQ if necessary (files: tofino-spine/configure/table_configure.py, tofino-spine/ptf_snapshotserver/table_configure.py)
				* If need_recirculate=0, partition_tbl sets eport to corresponding server-leaf switch based on key (files: tofino-spine/configure/table_configure.py)
				* If need_recirculate=0, snapshot_flag_tbl sets inswitch_hdr.snapshot_flag (files: tofino-spine/ptf_snapshotserver/table_configure.py)
				* If need_recirculate=0, ig_port_forward_tbl converts it as PUT/DELREQ_SEQ_INSWITCH (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configura/table_configure.py)
					- NOTE: spine switch processes PUT/DELREQ_INSWITCH in egress pipeline, yet NOT process PUT/DELREQ_SEQ_INSWITCH, which is processed by server-leaf switch
				* NOTE: NOT access prepare_for_cache_hit_tbl, ipv4_forward_tbl, sample_tbl, and egress MATs (except add_and_remove_value_header_tbl for PUTREQ_SEQ_INSWITCH -> files: tofino-spine/configure/table_configure.py)
			+ server-leaf switch processes PUT/DELREQ_SEQ_INSWITCH from spine switch
				* Access partition_tbl, hash_for_cmX_tbl, prepare_for_cachehit_tbl, and sample_tbl for PUT/DELREQ_SEQ_INSWITCH as PUT/DELREQ_SEQ (files: tofino-leaf/configure/table_configure.py)
					- NOTE: NOT access need_recirculate_tbl, recirculate_tbl, hash_for_spineselect_tbl, spineselect_tbl, snapshot_flag_tbl, ipv4_forward_tbl, and ig_port_forward_tbl
			+ server-leaf processes GETRES_LATEST/DELETED_SEQ_SERVER from server
				* Access need_recirculate_tbl and snapshot_flag_tbl as PUT/DELREQ_SEQ (files: tofino-leaf/configure/table_configure.py, tofino-leaf/ptf_snapshotserver/table_configure.py)
				* recirculate_tbl forwards GETRES_LATEST/DELETED_SEQ_SERVER to the first spine switch (files: tofino-leaf/configure/table_configure.py)
				* Access add_and_remove_value_header_tbl for GETRES_LATEST/DELETED_SEQ_SERVER (files: tofino-leaf/configure/table_configure.py)
			+ spine processes GETRES_LATEST/DELETED_SEQ_SERVER from server-leaf switch
				* Access need_recirculate_tbl and recirculate_tbl to recirculate pkt if necessary (files: tofino-spine/configure/table_configure.py, tofino-spine/ptf_snapshotserver/table_configure.py)
				* If need_recirculate=0, partition_tbl sets eport to corresponding server-leaf switch based on key (files: tofino-spine/configure/table_configure.py)
				* If need_recirculate=0, snapshot_flag_tbl sets inswitch_hdr.snapshot_flag (files: tofino-spine/ptf_snapshotserver/table_configure.py)
				* If need_recirculate=0, ig_port_forward_tbl converts it as GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configura/table_configure.py)
					- Add packet type of GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH (optype enumeration only) (files: tofino-*/main.p4, tofino-*/common.py, tofino-*/p4src/parser.p4, packet_format.h)
					- NOTE: spine switch processes GETRES_LATEST/DELETED_SEQ_INSWITCH in egress pipeline, yet NOT process GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH, which is processed by server-leaf switch in ingress pipeline
				* NOTE: NOT access prepare_for_cache_hit_tbl, ipv4_forward_tbl, sample_tbl, and egress MATs (except add_and_remove_value_header_tbl for GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH -> files: tofino-spine/configure/table_configure.py)
			+ server-leaf switch processes GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH from spine switch
				* Access ipv4_forward_tbl to clone/forward to corresponding spine switch based on is_cached; access ig_port_forward_tbl to convert GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH as GETRES_LATEST/DELETED_SEQ_INSWITCH (files: tofino-leaf/configure/table_configure.py, tofino-leaf/p4src/ingress_mat.p4)
					- NOTE: NOT access need_recirculate_tbl, recirculate_tbl, hash_for_spineselect_tbl, spineselect_tbl, snapshot_flag_tbl, partition_tbl, hash_for_cmX_tbl, prepare_for_cachehit_tbl, and sample_tbl
			+ For special cases
				* Generate CASE1 for GETRES_LATEST/DELETED_SEQ_INSWITCH as in single-switch
				* Generate CASE1 and CASE3 for PUT/DELREQ_SEQ_INSWITCH instead of PUT/DELREQ_INSWITCH (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/configure/table_configure.py)

## Implementation log after finishing most code

+ Other implementation details
	* Update update_pktlen_tbl accordingly (files: tofino-*/p4src/egress_mat.p4, tofino-*/configure/table_configure.py)
		- DistFarReach/DistNoCache: op_hdr.globalswitchidx, split_hdr.cur_scanswitchidx, split_hdr.max_scanswitchnum
		- DistCache: op_hdr.globalswitchidx, split_hdr.cur_scanswitchidx, split_hdr.max_scanswitchnum, clone_hdr.server_sid, clone_hdr.server_udpport, inswitch_hdr.hashval_for_bfX
		- TODO: Update ycsb client library accordingly
	* Retrieve setting udp.srcport = server_worker_iort_start in eg_port_forward_tbl for DistFarReach/DistCache (files: tofino-*/p4src/egress_mat.p4, tofino-*/configure/table_configure.py)
		- NOTE: udp.dstport has been set as client.udpport, if NOT set srcport as server.worker.udpport, pkt will be dropped by parser/deparser due to NO reserved udp ports
		- NOTE: although pkt type has been changed (e.g., GETREQ_INSWITCH -> GETRES, PUTREQ_SEQ_INSWITCH_CASE1 -> PUTRES), as the current devport is server/reflector.devport instead of client.devport, it will NOT access update_ipmac_srcport_tbl for server2client to change udp.srcport
		- DistFarReach spine/leaf: GET/PUT/DELREQ -> GET/PUT/DELRES; PUT/DELREQ_SEQ_INSWITCH_CASE1 -> PUT/DELRES
		- DistCache spine/leaf: GETREQ -> GETRES, NETCACHE_WARAMUPREQ_INSWITCH_POP -> WARMUPACK
	* Update_ipmac_srcport_tbl
		- DistFarReach: add PUT/DELREQ_SEQ_INSWITCH_CASE1 for switch2switchos as in FarReach
	* Update add_and_remove_value_header_tbl
		- DistFarReach/DistCache leaf: add PUTREQ for client-leaf
	* Check all possible packet types from spine switch to server-leaf switch (e.g., GETREQ_NLATEST)
		- For GETREQ_NLATEST from spine switch in DistFarReach, server-leaf directly forwards it to server by accessing partition_tbl and update_ipmac_srcport_tbl as client2server (files: tofino-leaf/configure/table_configure.py)
		- NOTE: for NETCACHE_PUT/DELREQ_SEQ_CACHED from spine switch in DistCache, server-leaf ingress processes it as PUT/DELREQ_SEQ, i.e., converting it into PUT/DELREQ_SEQ_INSWITCH, which will be converted as NETCACHE_PUT/DELREQ_SEQ_CACHED to server (NOTE: if spine caches the key, server-leaf must also cache the key)

## Implementation log after debug and sync from distnocache

+ Compile and test DistFarReach
	* Range partition w/ debug mode
		- Issue: we CANNOT use pragma stage X to specify stage ID for range/hash_partition_tbl -> SYNC to distcache
			+ Reason: one stage TCAM CANNOT accommodate 4096 entries for range/hash_partition_tbl due to range matching
			+ Solution: NOT use pragma stage X; move hash/range_partition_tbl as late as possible, and reserve two stages for them (files: tofino-*/main.p4, tofino-*/p4src/ingress_mat.p4)
			+ Observation: based on visualization, range/hash_paritition_tbl needs TCAM of two stages to support range matching
		- Pass correctness test of normal requests/responses
			+ Issue: spine switch sets cur_scanswithidx = 1&1 instead of 1&2 for two SCANREQ_SPLITs -> error in socket_helper.recvlarge_multisrc; yet server-leaf switch sets cur_scanidx = 1&2 correctly -> SYNC to distcache
				* Observation: the last cloned SCANREQ_SPLIT hits correct MAT entry, i.e., forward_scanreq_split(), in eg_port_forward_tbl to increase split_hdr.cur_scanswitchidx by 1
				* Possible reason: Tofino compiler's PHV allocation bug
				* [DEPRECATED] Solutions
					- Re-compile distfarreach.spine -> FAIL
					- Use 32b for split_hdr.cur_scanswitchidx, split_hdr.max_scanswitchnum, and meta.remain_scannum; update pktlen accordingly (files: tofino-*/p4src/header.p4, packet_format.*; tofino-*/p4src/egress_mat.p4, tofino-*/configure/table_configure.py) -> FAIL: CANNOT pass compilation due to PHV limitation
					- Introduce increase_globalswitchidx_for_cloned_scanreq_split_tbl after process_scanreq_split_tbl in spine switch such that split_hdr.cur_scanswitchidx/max_scanswitchnum do NOT need to be placed in the same group as op_hdr.globalswitchidx (files: tofino-spine/main.p4, tofino-spine/p4src/egress_mat.p4, tofino-spine/configure/table_configure.py) -> FAIL
					- Extract split_hdr.cur_scanswitchidx/max_scanswitchnum as splitswitch_hdr to change spine-switch packet format and hence PHV allocation (files: tofino-spine/p4src/header.p4, tofino-spine/p4src/parser.py, tofino-spine/p4src/ingress_mat.p4, tofino-spine/p4src/egress_mat.p4) -> FAIL
					- Use 32b for splitswitch_hdr.cur_scanswitchidx, splitswitch_hdr.max_scanswitchnum, and meta.remain_scannum; update pktlen accordingly (files: tofino-*/p4src/header.p4, packet_format.*; tofino-*/p4src/egress_mat.p4, tofino-*/configure/table_configure.py) -> FAIL due to PHV limitation
					- Swap cur_scanidx/max_scannum and cur_scanswitchidx/max_scanswitchnum (files: tofino-*/header.p4, packet_format.*) -> FAIL
					- Use 8b split_hdr.is_clone; update pktlen accordingly (files: tofino-*/p4src/header.p4, packet_format.*; tofino-*/p4src/egress_mat.p4, tofino-*/configure/table_configure.py) -> FAIL: split_hdr.cur_scanswitchidx = 1&0 now
					- Add one/two 16b padding into split_hdr (files: tofino-*/p4src/header.p4, packet_format.*) -> FAIL
				* [CORRECT] Solution: use pragma no_overlay to disable PHV overlay of split_hdr.cur_scanswitchidx (files: tofino-spine/p4src/header.p4) -> OK for cur_scanswitchidx, yet max_scanswitchnum becomes 0 -> WORK -> sync to distcache
					- Also use pragma no_overlay for split_hdr.max_scanswitchnum
		- Code change
			+ Add local/remotescripts for static/dynamic workload -> sync to farreach/nocache/netcache/distnocache/distcache
			+ Update docs -> sync to farreach/nocache/netcache/distnocache/distcache
		- Pass correctness of cache population/eviction
			+ Issue: no SETVALID_INSWITCH_ACK to leaf reflector -> SYNC to distnocache/distcache
				* Reason: server-leaf.range_partition_tbl matches op_hdr.globalswitchidx -> NOT set leafswitchidx for SETVALID_INSWITCH means no matching entry in range_partition_tbl -> pkt is dropped by ingress.deparser
				* Solution: set globalswitchidx for SETVALID_INSWITCH, CACHE_POP_INSWITCH, CACHE_EVICT_LOADFREQ_INSWITCH, CACHE_EVICT_LOADDATA_INSWITCH, LOADSNAPSHOTDATA_INSWITCH, GETRES_LATEST/DELETED_SEQ_SERVER (files: packet_format.*, switchos.c) -> sync to distnocache/distcache
			+ Issue: no GETRES to spine switch and client for GETRES_DELETED_SEQ_SERVER hit in leaf switch -> ONLY for distfarreach
				* Reason: pkt cloned by clone_i2e is GETRES_LATEST/DELETED_SEQ_SERVER/_INSWITCH instead of GETRES_LATEST/DELETED_SEQ
				* Solution: leaf switch should convert GETRES_LATEST/DELETED_SEQ_SERVER/_INSWITCH as GETRES to spine switch (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/configure/table_configure.py)
			+ Issue: w/ SETVALID_INSWITCH_ACK to spine reflector NIC, yet NOT received by dp2cpserver -> SYNC to distnocache/distcache
				* Reason 1: spine switch uses 10.0.1.11 as srcip which is the local IP of client 0, where spine reflector is deployed
				* Solution 1: use reflector_dp2cp_dstip as srcip for switch2switchos (files: config.ini, configs/*, tofino-*/comon.py, tofino-*/configure/table_configure.py) -> sync to distnocache/distcache
				* [DEPRECATED]
					* Reason 2: due to dp2cpserver.udpport of 5018?
					* Solution 2: try udpport of 5028 -> FAIL
					* Reason 3: port collision?
					* Solution 3: netstat -tunlp -> sudo lsof -i:123 -> kill ntpd -> FAIL
					* Reason 4: ip configuration error?
					* Solution 4: change ip from 10.0.2.X into 10.2.2.X
					* Reason 5: firewall; route/iptable rules; rpfilter rule?
					* Solution 5: sudo ufw disable; check by route -n and iptables -L; sysctl net.ipv4.conf.all.rp_filter=0 + sysctl net.ipv4.conf.default.rp_filter=0 -> FAIL
					* Reason 6: kernel packet drop?
					* Solution 6: install dropwatch; run `sudo dropwatch -l kas` to use /proc/kallsyms to find corresponding kernel function -> FAIL
					* Solution 7: reboot -> FAIL
					* Solution 8: use `sudo perf record -g -a -e skb:kfree_skb` + `sudo perf script` to find pkt drop -> FAIL
					* Solution 9: use kernel dynamic debug mechanism to find pkt drop -> FAIL (no error message)
						- `sudo dmesg -C`
						- `echo "file net/ipv4/udp.c +p" > /sys/kernel/debug/dynamic_debug/control`
						- `cat /sys/kernel/debug/dynamic_debug/control | grep "udp.c"`
						- Run testbed to receive SETVALID_INSWITCH_ACK from NIC, and run `sudo dmesg` to see kernel debug messages
						- `echo "file net/ipv4/udp.c -p" > /sys/kernel/debug/dynamic_debug/control`
				* Reason: unclear
				* Solution: use AF_PACKET raw socket for spine switch reflector; launch/stop spine reflector w/ sudo permission (files: socket_helper.*, reflector_impl.h, remotescripts/launchservertestbed.sh, remotescripts/stopservertestbed.sh)
			+ Pass WARMUPREQ/GETREQ_POP/PUTREQ_POP in leaf switch
			+ Pass GET/PUT/DELREQ of cache hit and GETRES_LATEST/DELETED_SEQ_SERVER of conservative read in leaf switch
			+ Pass WARMUPREQ in spine switch
			+ Pass GET/PUT/DELREQ of cache hit and GETRES_LATEST/DELETED_SEQ_SERVER of conservative read in spine switch
		- Pass correctness of crash-consistent snapshot and special cases
			+ Issue: no DELREQ_SEQ_CASE3 for DELREQ_SEQ_INSWITCH under snapshot -> ONLY for distfarreach
				* Reason: snapshot_flag is reset by default action of snapshot_flag_tbl for DELREQ_SEQ_INSWITCH
				* Solution: explictily invoke nop() in snapshot_flag_tbl for PUT/DELREQ_SEQ_INSWITCH and GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH (tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
			+ Issue: no GETRES_DELETED_SEQ_SERVER_INSWITCH during single path -> ONLY for distfarreach
				* Reason: GETRES_DELETED_SEQ_SERVER being forwarded to spine switch is converted as GETRES in egress pipeline
				* Solution: use bypass_egress for recirculated PUT/DELREQ_SEQ and GETRES_LATEST/DELTED_SEQ_SERVER (files: tofino-leaf/p4src/ingress_mat.p4)
			+ Strage observation: the counter of entry 8 of ipv4_forward_tbl in both spine and leaf switch is continuously increasing, which may be caused by Tofino bug
				* In leaf switch. entry 8 is to forward GETRES_DELETED_SEQ_SERVER to spine switch, however the entry of ig_port_forward_tbl to convert GETRES_DELETED_SEQ_SERVER into GETRES_DELETED_SEQ_INSWITCH does NOT have corresponding counter
				* In spine switch, entry 8 is to clone GETRES_DELETED_SEQ for GETRES and forward it to update registers, however both the entry of ig_port_forward_tbl to convert GETRES_DELETED_SEQ into GETRES_DELETED_SEQ_INSWITCH and the entry of update_ipmac_srcport_tbl for GETRES as server2client do NOT have corresponding counter
				* Server does NOT send repeat GETRES_DELETED_SEQ_SERVER and client does NOT receive repeat GETRES
			+ Issue: no CACHE_EVICT_LOADFREQ_INSWITCH_ACK to spine reflector -> SYNC to distnocache/distcache
				* Reason: spine.cache_lookup_tbl matches op_hdr.globalswitchidx, while set leafswitchidx for CACHE_EVICT_LOADFREQ_INSWITCH -> is_cached = 0 and hence not converted into ACK in eg_port_forward_tbl
				* [IMPORTANT]: we need to set op_hdr.globalswitchidx for GETRES_LATEST/DELETED_SEQ_SERVER and all pkts from control plane to data plane including SET_VALID_INSWITCH, CACHE_POP_INSWITCH, CACHE_EVICT_LOADFREQ/DATA_INSWITCH, LOADSNAPSHOTDATA_INSWITCH
					- In server-leaf switch, we use globalswitchidx for both range_partition_tbl and cache_lookup_tbl
					- In spine switch, we use globalswitchidx for cache_lookup_tbl
				* Solution: set globalswitchidx based on role (i.e., spine/leaf) for SETVALID_INSWITCH, CACHE_POP_INSWITCH, CACHE_EVICT_LOADFREQ_INSWITCH, CACHE_EVICT_LOADDATA_INSWITCH, LOADSNAPSHOTDATA_INSWITCH, GETRES_LATEST/DELETED_SEQ_SERVER (files: packet_format.*, switchos.c) -> sync to distnocache/distcache
			+ Pass PUT/DELREQ_SEQ_INSWITCH during single path
			+ Pass PUT/DELEQ_SEQ_CASE3
			+ Populate a key in leaf switch
				+ Pass GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH during single path
				+ Pass PUT/DELREQ_SEQ_INSWITCH_CASE1
				+ Pass GETRES_LATEST/DELETED_SEQ_INSWITCH_CASE1
				+ Pass CACHE_EVICT_CASE2
				+ Pass LOADSNAPSHOTDATA_INSWITCH
			+ Populate a key in spine switch
				+ Pass GETRES_LATEST/DELETED_SEQ_SERVER_INSWITCH during single path
				+ Pass PUT/DELREQ_SEQ_INSWITCH_CASE1
				+ Pass GETRES_LATEST/DELETED_SEQ_INSWITCH_CASE1
				+ Pass CACHE_EVICT_CASE2
				+ Pass LOADSNAPSHOTDATA_INSWITCH
		- Pass correctness test of multiple clients + multiple servers
	* Code change after debugging range partition
		- Code for controller.snapshotclient to get snapshot from both spine and leaf switchos
		- Use identity for spineselect_tbl (i.e., using two different hash functions to simulate independent hash functions) (files: tofino-leaf/p4src/ingress_mat.p4, key.c) -> sync to distnocache/distcache
			+ We can confirm whether Tofino-based identity = CPU-based identity during correctness test of cache population (CPU-based hash) and cache hit (Tofino-based hash)
			+ TODO: Recompile distnocache under hash partition
	* Code change for issues
		- Under server rotation, rotated server thread should use the same udp port for worker in transaction phase as in loading and warmup phase -> SYNC to farreach/nocache/netcache/distnocache/distcache
			+ Add bottleneck_partitionidx in configuration (files: config.ini, configs/*, common_impl.h, iniparser/iniparser_wrapper.*)
			+ Update config file line number in script (files: remotescripts/test_server_rotation.sh)
			+ Non-bottleneck server worker calculates port based on its global logical idx and bottleneck serveridx (files: server_impl.h)
		- Split cache_lookup_tbl and range/hash_partition_tbl into multiple stages to fix TCAM/SRAM issues in spine/leaf switch (files: tofino-*/p4src/ingress_mat.p4, tofino-*/main.p4) -> SYNC to distcache
		- Split part of eg_port_forward_tbl into another_eg_port_forward_tbl to fix VLIW issue in leaf switch (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/main.p4, tofino-leaf/configure/table_configure.py) -> ONLY for distfarreach
			+ NOTE: as DistCache does NOT use too much VLIW for eg_port_forward_tbl, we do NOT need to split it
		- Reduce power budget cost by: (1) introduce hash_for_cm12_tbl and hash_for_cm34_tbl; (2) reduce SEQ_BUCKET_COUNT to 4096; (3) reduce eg_port_forward_tbl to 1024; (4) use 16K instead of 32K cache entries in leaf switch -> SYNC to distcache
	* Range partition w/o debug mode
		- Test write-only dynamic workload -> from 0.34 to 0.51 MOPS for 1024/2 clients + 8/2 servers
		- Try server rotation scripts -> OK (NOTE: bottlenect partition is NOT 123 under range partition)
		- Update distfarreac visualization under range partition
	* Code change
		- Use bpf_filter to avoid long latency issue of raw socket due to receiving many unmatched packets (files: socket_helper.*) -> sync to distnocache/distcache
		- Fix duplicate SNAPSHOT_GETDATA by re-sending snapshot data from switchos to controller; reduce controller.snapshotclient timeout value (files: switchos.c, controller.c, socket_helper.h) -> ONLY for distfarreach
			* TODO: Use subthreads to get snapshot data from spine/leaf switch
	* Hash partition w/o debug mode
		- Test dynamic workload -> from 1.1 to 1.2 MOPS MOPS for 1024/2 clients + 8/2 servers
		- Update distfarreach visualization under hash partition

## Implementation log after coding DistCache

+ NOTE: ipv4_forward_tbl must follow range/hash_partition_tbl as it needs to reset eport for special get responses (GETRES_LATEST/DELETED_SEQ_SERVER/\_INSWITCH) if cache miss in server-leaf -> ONLY for distfarreach (files: tofino-\*/p4src/ingress_mat.p4, tofino-\*/main.p4)
	+ Re-compile and re-test GETRES_LATEST/DELETED_SEQ_SERVER w/ cache hit in spine or server-leaf switch
	+ Update visualization under range and hash partition

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
	- NOTEs
		+ Enable SERVER_ROTATION in helper.h
		+ Update bottleneck partition in configs/config.ini.rotation-switch and remotescripts/test_server_rotation.sh
			* global::bottleneck_serveridx_for_rotation
			* server0::server_logical_idxes
			* server1::server_logical_idxes
			* bottleneck_serveridx in shell
	- Use config.ini.rotation-switch-switchos-loading-warmupclient
		+ Start switch by `bash start_switch.sh`
		+ Start switchos and ptf.servers, and configure data plane by `bash localscripts/launchswitchostestbed`
		+ Start servers in client 0 (dl11) by `bash remotescripts/launchservertestbed`
		+ Start clients to perform loading phase and warmup phase
		+ Deprecated manual way
			+ Start switch and configure data plane
			+ Start switchos and ptf.pop/snapshotserver
			+ Start clients and servers for loading phase
			+ Start clients and servers for warmup phase
	- Use config.ini.rotation-transaction
		+ Run `bash test_server_rotation.sh` directly, which will stop-and-start clients and servers repeatedly with different config files
			+ NOTE: switch, switchos, ptf.servers, controller, and reflectors are still running
		+ Deprecated manual way
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
