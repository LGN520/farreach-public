# DistCache

- Copy from netcache
- See [important notes](./netreach-v4-lsm.md) in netreach-v4-lsm.md
- See [common implementation details](./distfarreach.md) in distfarreach.md
+ NOTE: we avoid dependency on ingress port; see details in [implementation log after/during DistFarReach](netreach-v4-lsm.md)
+ [IMPORTANT] Discuss with Qun about DistCache
	+ Review DistCache to prepare for implementation; check counter-based rate limit and multiple queue in Tofino guidebook
	+ (Switch simulation) we use one physical switch to simulate multiple logical switches by splitting SRAM based on logical switch ID
		* DistCache simulates multiple logical switches by multiple queues + counter-based rate limit
			- Maintain per-switchidx counter in spine/leaf, and periodically reset it
			- If the counter > threshold, drop each new request
		* NOTE: we do NOT have sufficient power budget to support counter-based rate limit (exceed 0.77 W)
		* (1) switch simulation is not related with inswitch cache design
		* (2) Due to limited servers and LSM-based KVS, our system thpt is at most several MOPS, which is far smaller than switch thpt -> switch thpt will never become bottleneck -> we do NOT need to rate limit for different logical switches
			- For example, 2-pipeline switch thpt is 3.2 GOPS -> to simulate 100 switches, each switch has max thpt of 32 MOPS -> our system max thpt is just several MOPS
	+ (Routing mechanism) we use ECMP to choose least-loaded path in DistCache -> if too many MATs, simply set an incorrect switchidx to simulate any path to server-leaf switch
		* We do NOT have sufficient stateful ALUs to implement CONGA/HULA due to Tofino limitation
			- We have used 3 regs for power-of-two-choices: server-leaf traffic load + client-leaf spine/server-leaf traffic load
			- To support HULA, we need at least 2 more regs: client-leaf per-destination traffic load + best hop (aka devport)
				+ NOTE: we CANNOT reuse spine/server-leaf traffic load as per-destination traffic load -> for spine/server-leaf traffic load, we directly use load_hdr.spine/leafload to update; while for per-destination traffic load, ONLY if max(spineload, leafload) < per-destination load, we will update the load and best hop
				+ For complete HULA, we even need the 3rd reg: client-leaf per-destination time for aging mechanism
		* (1) DistCache paper has argued that this module is orthogonal with inswitch cache design
		* (2) As mentioned before, switch is NOT the bottleneck -> even if we forward all traffic to the same physical path (or set the same virtual switch index aka virtual path if w/ rate limit), the switch (virtual switch) will NOT be overloaded
	+ (Memory management) we still ignore bitmap-based memory management as in NetCache, as memory efficiency is NOT our focus and does NOT affect evaluation (we always pre-populate 10000 hot objects into switch)
	+ NOTE: our focus is to design power-of-two-choices and cache coherence mechanism
		* As the above tricks are NOT related with in-switch cache and switch is NOT the bottleneck in our scenario, our implementation is OK -> we ONLY need to mention what features we have implemented in README without detailed reasons (same for NetCache)

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

## Implementation log after reviewing DistCache paper

+ Exclusive design and implementation for DistCache (ONLY for DistCache by default)
	* NOTE: spine switch does NOT need WARMUPREQ_SPINE
		- Spine switch directly converts WARMUPREQ as NETCACHE_WARMUPREQ_INSWITCH, which generates NETCACHE_WARMUPREQ_INSWITCH_POP to switchos and sends WARMUPACK back to leaf switch w/ client info from clone_hdr
		- client-leaf forwards WARMUPREQ to spine, yet server-leaf switch does NOT need to process WARMUPREQ
	* Power-of-two-choices based on per-switch traffic load
		- Use op_hdr.spineswitchidx and op_hdr.leafswitchidx instead of op_hdr.globalswitchidx (files: tofino-*/p4src/header.p4, packet_format*) -> ONLY for DistCache
			+ Update pktlen accordingly (files: tofino-*/p4src/egress_mat.p4, tofino-*/configure/table_configure.py)
			+ Update matching fields accordingly (files: tofino-*/main.p4, tofino-*/p4src/ingress_mat.p4, tofino-*/ptf_popserver/table_configure.py, tofino-*/p4src/egress_mat.p4, tofino-*/configure/table_configure.py)
			+ Spine/leaf switchos set spine/leafswitchidx accordingly for CACHE_EVICT_LOADFREQ_INSWITCH, CACHE_POP_INSWITCH, and add/remove cache_lookup_tbl by calculate_switchidx/_by_role (files: switchos.c)
		- Per-switch traffic load
			+ spine switch maintains spineload_reg in stage 0 for GETREQ/GETRES based on spineswitchidx -> ONLY for DistCache (files: tofino-spine/main.p4, tofino-spine/p4src/regs/spineload.p4, tofino-spine/configure/table_configure.py)
				* Add switchload_hdr including spine/leafload for GETREQ_INSWITCH/GETRES/DISTCACHE_GETRES_SPINE (files: tofino-*/p4src/header.p4, tofino-*/p4src/parser.p4, packet_format.*)
					- Add ONLY optype of DISTCACHE_GETRES_SPINE (NO implementation -> SYNC to distfarreach/distnocache) (files: tofino-*/main.p4, tofino-*/p4src/parser.p4, tofino-*/common,py, packet_format.h)
					- Update pktlen accordingly (files: tofino-*/p4src/egress_mat.p4, tofino-*/configure/table_configure.py)
				* For GETREQ from client-leaf
					- NOTE: if w/ cache hit, client-leaf.GETREQ -> spine.GETREQ -> spine.GETREQ_INSWITCH -> spine.DISTCACHE_GETRES_SPINE -> client-leaf.GETRES -> client.GETRES
					- client-leaf calculates spineswitchidx for GETREQ by spineselect_tbl as in distfarreach/distnocache
					- GETREQ updates spineload_reg, and converts into GETREQ_INSWITCH w/ switchload_hdr (tofino-spine/p4src/ingress_mat.p4)
					- spine switch converts GETREQ_INSWITCH as DISTCACHE_GETRES_SPINE to client-leaf if cache hit (tofino-spine/p4src/egress_mat.p4, tofino-spine/configure/table_configure.py)
						+ Update ipmac&srcport accordingly (tofino-spine/configure/table_configure.py)
						+ client-leaf forwards DISTCACHE_GETRES_SPINE as GETRES to client (tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
					- spine switch removes switchload_hdr from GETREQ_INSWITCH as GETREQ_SPINE to server-leaf if cache miss (tofino-spine/p4src/egress_mat.p4)
				* For GETRES from server-leaf
					- NOTE: place ipv4_forward_tbl before range/hash_partition_tbl (files: tofino-*/main.p4, tofino-*/p4src/ingress_mat.p4)
					- server-leaf calculates spineswitchidx for GETRES_SERVER by spineselect_tbl yet NOT change eport (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
					- spine.GETRES reads spineload_reg into load_hdr.spineload based on spineswitchidx (files: tofino-spine/configure/table_configure.py)
			+ server-leaf switch maintains leafload_reg in stage 0 for GETREQ_SPINE/GETRES_SERVER based on leafswitchidx -> ONLY for DistCache (files: tofino-leaf/main.p4, tofino-leaf/p4src/leafload.p4, tofino-leaf/configure/table_configure.py)
				* Add switchload_hdr including spineload and leafload for GETREQ_INSWITCH/GETRES/GETRES_SERVER as in spine switch
					- Update pktlen accordingly as in spine switch
				* For GETREQ_SPINE from spine switch
					- NOTE: if w/ cache hit, spine.GETREQ_SPINE -> server-leaf.GETREQ_SPINE -> server-leaf.GETRES -> spine.GETRES -> client.GETRES
					- spine calculates leafswitchidx for GETREQ by range/hash_partition_tbl, and forwards GETREQ_INSWITCH as GETREQ_SPINE to server-leaf as usual
					- server-leaf updates leafload_reg for GETREQ_SPINE, and converts it into GETREQ_INSWITCH w/ switchload_hdr (files: tofino-leaf/confiugure/table_configure.py, tofino-leaf/p4src/ingress_mat.p4)
					- server-leaf converts GETREQ_INSWITCH as GETRES to spine if cache hit as usual
						+ spine forwards GETRES to client-leaf and client as usual
					- server-leaf removes switchload_hdr from GETREQ_INSWITCH as GETREQ/NETCACHE_GETREQ_POP to storage-server/leaf-reflector if cache miss (files: tofino-leaf/egress_mat.p4)
				* For GETRES_SERVER from storage server
					- storage server calculates leafswitchidx (and spineswitchidx) for GETRES_SERVER (files; packet_format.*, server_impl.h)
					- GETRES_SERVER reads leafload_reg into load_hdr.leafload based on leafswitchidx (files: tofino-leaf/configure/table_configure.py)
					- server-leaf converts GETRES_SERVER as GETRES to spine, client-leaf, and client as usual
			+ switchos periodically resets spine/leafload_reg per second (files: tofino-*/ptf_cleaner/table_configure.py)
		- client-leaf traffic load for query decision -> ONLY for distcache
			+ client-leaf maintains spineload_forclient_reg and leafload_forcient_reg in stage 0 and stage 2 for GETREQ/DISTCACHE_GETRES_SPINE/GETRES based on spine/leafswitchidx (files: tofino-leaf/main.p4, tofino-leaf/p4src/header.p4, tofino-leaf/p4src/regs/spineload_forclient.p4, tofino-leaf/p4src/regs/leafload_forclient.p4, tofino-leaf/configure/table_configure.py)
				* GETRES writes load_hdr.spine/leafload into spine/leafload_forclient_reg
				* DISTCACHE_GETRES_SPINE ONLY writes load_hdr,spineload into spineload_forclient_reg
				* GETREQ reads spine/leafload_forclient_reg into meta.spineload_forclient and meta.toleaf_predicate
			+ NOTE: client calculates spineswitchidx and leafswitchidx for GETREQ (NOTE: we CANNOT place it in client-leaf as we do NOT have sufficient stages and stateful ALUs due to Tofino limitation) (files: packet_format.*, remote_client.c)
		- Query routing for GETREQ
			+ NOTE: for optypes except GETREQ, client-leaf accesses spineselect_tbl as usual (files: tofino-leaf/main.p4, tofino-leaf/configure/table_configure.py)
			+ For GETREQ, client-leaf access spineselect_tbl w/ meta.toleaf_predicate = 1 or 2
				* Move hash_for_bf2_tbl from stage 5 to stage 2, and move spineselect_tbl and cache_lookup_tbl one stage backwards, such that we can place leafload_forclient_reg in stage 2 (files: tofino-leaf/main.p4, tofino-leaf/p4src/ingress_mat.p4)
				* client-leaf compares meta.spine/leafload_forclient in stateful ALU to set meta.toleaf_predicate when get_leafload_forclient for GETREQ (files: tofino-leaf/p4src/header.p4, tofino-leaf/p4src/regs/leafload_forclient.p4)
					- NOTE: we CANNOT use gateway table due to Tofino limitation (logical op between two PHVs must be == or !=; for logical op of > or <, can ONLY be used by one PHV and one constant, whose total bits <= 12 bits)
					- NOTE: we reset meta.toleaf_predicate = 1 in default action of access_leafload_forclient_tbl
				* spineselect_tbl matches meta.toleaf_predicate to set spineswitchidx and eport (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
					- NOTE: spineswitchidx of GETREQ has been set by client before spineselect_for_getreq_tbl
					- NOTE: meta.toleaf_predicate = 1 for all other optypes except GETREQ
					- If meta.spineload_forclient >= meta.leafload_forclient (aka toleaf_predicate = 2), client-leaf sets incorrect spineswitchidx and eport
					- Otherwise (aka toleaf_predicate = 1), client-leaf sets correct spineswitchidx and eport
				* For incorrect spineswitchidx of GETREQ if meta.toleaf_predicate=2
					- Method A: use (correct spineswitchidx + 1) % spineswitchnum
					- Method B: key-based ECMP (USED NOW)
						+ access hash_for_ecmp_tbl to get meta.hashval_for_ecmp in stage 0 or 1 (files: tofino-leaf/main.p4, tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/p4src/header.p4, tofino-leaf/configure/table_configure.py)
						+ access ecmp_for_getreq_tbl to set meta.toleaf_offset in stage 2 (files: tofino-leaf/main.p4, tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/p4src/header.p4, tofino-leaf/configure/table_configure.py)
							* NOTE: split switch_partition_count into spineswitchnum - 1, and set meta.toleaf_offset in the range of [1, spineswitchnum - 1]
						+ spineselect_tbl invokes spineselect_for_getreq_toleaf to get incorrect spineswitchidx = correct spineswitchidx + meta.toleaf_offset in stage 3 (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
						+ access cutoff_spineswitchidx_for_ecmp_tbl in stage 4 to decrease spineswitchidx by spineswitchnum if spineswitchidx is in the range of [spineswitchnum, 2*spineswitchnum - 2] (files: tofino-leaf/main.p4, tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
		- TODOTODO: Merge spine/leafload_forclient_reg as one register array to optimize resource usage
	* Cache coherence based on two-phase protocol
		- NOTE: both phase 1 and phase 2 are ONLY performed for cached keys
		- Phase 1: cache invalidation
			+ PUT/DELREQ_INSWITCH and PUT/DELREQ_SEQ_INSWITCH do NOT invalidate in-switch value on path (files: tofino-*/configure/table_configure.py)
			+ For each PUT/DELREQ_SEQ and NETCACHE_PUT/DELREQ_SEQ_CACHED **of cached key**, server covers in-switch value invalidation
				* Add optype and implementation of DISTCACHE_INVALIDATE/_ACK -> SYNC ONLY optype to distnocache/distfarreach (files: tofino-*/main.p4, tofino-*/common.py, tofino-*/p4src/parser.p4, packet_format.*, common_impl.h)
				* Add server_invalidateserver_port_start in configuration and reserve udp ports for worker.invalidateserver (files: config.ini, configs/*, iniparser/iniparser_wrapper.*. common_impl.h, tofino-*/p4src/parser.p4)
				* worker.invalidateserver reliably sends DISTCACHE_INVALIDATE to server-leaf and waits for DISTCACHE_INVALIDATE_ACK (files: server_impl.h)
				* In server-leaf, for DISTCACHE_INVALIDATE
					- For original DISTCACHE_INVALIDATE
						+ Set spineswitchidx in spineselect_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
						+ Set eport to corresponding server in range/hash_partition_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
						+ Convert original DISTCACHE_INVALIDATE into DISTCACHE_INVALIDATE_INSWITCH to egress in ig_port_forward_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
							* Add optype of DISTCACHE_INVALIDATE_INSWITCH -> SYNC to distnocache/distfarreach (files: tofino-*/main.p4, tofino-*/common.py, tofino-*/p4src/parser.p4, packet_format.h)
						+ Reset latest=0 if cached=1 to invalidate inswitch value in access_latest_tbl, and drop DISTCACHE_INVALIDATE_INSWITCH in drop_tbl (files: tofino-leaf/configure/table_configure.py, tofino-leaf/p4src/egress_mat.p4, tofino-leaf/main.p4)
					- Send cloned DISTCACHE_INVALIDATE to spine switch by clone_i2e in ipv4_forward_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
						+ TODOTODO: We can also clone DISTCACHE_INVALIDATE to spine switch in spineselect_tbl based on key
						+ NOTE: spineswitchidx has been set by storage server
				* In spine switch, for DISTCACHE_INVALIDATE
					- Set eport to corresponding server-leaf in range/hash_partition_tbl (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configure/table_configure.py)
					- Convert DISTCACHE_INVALIDATE into DISTCACHE_INVALIDATE_INSWITCH to egress, and swap udp.src/dstport (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configure/table_configure.py)
					- Reset latest=0 if cached=1 to invalidate inswitch value for DISTCACHE_INVALIDATE_INSWITCH (files: tofino-spine/configure/table_configure.py)
					- Convert DISTCACHE_INVALIDATE_INSWITCH as DISTCACHE_INVALIDATE_ACK to server (files: tofino-spine/p4src/egress_mat.p4, tofino-spine/configure/table_configure.py)
				* In server-leaf, for DISTCACHE_INVALIDATE_ACK
					- Set eport (NOT udp.dstport) to corresponding server in range/hash_partition_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
					- Update src/dst.ip/mac and srcport for DISTCACHE_INVALIDATE_ACK as client2server (files: tofino-leaf/configure/table_configure.py)
				* After receiving DISTCACHE_INVALIDATE_ACK, server can send PUTRES_SERVER to client if no packet is performing value update (aka phase 2) as in NetCache
					- NOTE: PUTREQ_SEQ does NOT need to wait for its own phase 2, but it needs to wait for the phase 2 of previous packets
					- NOTE: in most case, after waiting for its own phase 1, the phase 2 of previous packets has finished
		- Phase 2: cache value update
			+ Server reliably sends NETCACHE_VALUE_UPDATE to update in-switch value, and waits for NETCACHE_VALUE_UPDATE_ACK
				* In storage server
					- Add the key into beingupdated keyset to block subsequent write queries for consistency in server.worker (for PUT/DELREQ and NETCACHE_PUT/DELREQ_CACHED) and server.popserver (for NETCACHE_CACHE_POP_FINISH) (files: server_impl.h)
					- Set spineswitchidx and leafswitchidx for NETCACHE_VALUE_UPDATE (files: packet_foramt.*, server_impl.h)
				* In server-leaf, for NETCACHE_VALUEUPDATE
					- For original NETCACHE_VALUEUPDATE
						+ Set spineswitchidx in spineselect_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
						+ Set eport to corresponding server in range/hash_partition_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
						+ Convert original NETCACHE_VALUEUPDATE into NETCACHE_VALUEUPDATE_INSWITCH to egress in ig_port_forward_tbl (files: tofino-leaf/p4src/ingress_mat.p4)
						+ Update registers including latest, deleted, savedseq, vallen, and value if cached=1 as usual; and drop NETCACHE_VALUEUPDATE_INSWITCH in drop_tbl (files: tofino-leaf/configure/table_configure.py, tofino-leaf/p4src/egress_mat.p4)
					- Send cloned NETCACHE_VALUEUPDATE to spine switch by clone_i2e in ipv4_forward_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
						+ NOTE: we should access add_and_remove_value_header_tbl for cloned NETCACHE_VALUEUPDATE to hold value headers to spine switch (files: tofino-leaf/configure/table_configure.py)
						+ TODOTODO: We can also clone NETCACHE_VALUEUPDATE to spine switch in spineselect_tbl based on key
						+ NOTE: spineswitchidx has been set by storage server
				* In spine switch, for NETCACHE_VALUEUPDATE
					- Set eport to corresponding server-leaf in range/hash_partition_tbl (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configure/table_configure.py)
					- Convert NETCACHE_VALUEUPDATE into NETCACHE_VALUEUPDATE_INSWITCH to egress, and swap udp.src/dstport (files: tofino-spine/p4src/ingress_mat.p4)
					- Update registers including latest, deleted, savedseq, vallen, and value if cached=1 for NETCACHE_VALUEUPDATE_INSWITCH as usual
					- Convert NETCACHE_VALUEUPDATE_INSWITCH as NETCACHE_VALUEUPDATE_ACK to server in eg_port_forward_tbl (files: tofino-spine/p4src/egress_mat.p4, tofino-spine/configure/table_configure.py)
						+ NOTE: we do NOT need update_netcache_valueupdate_inswitch_to_netcache_valueupdate_ack in server-leaf (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/configure/table_configure.py)
				* In server-leaf, for NETCACHE_VALUEUPDATE_ACK
					- Set eport (NOT udp.dstport) to corresponding server in range/hash_partition_tbl (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
					- Update src/dst.ip/mac and srcport for NETCACHE_VALUEUPDATE_ACK as client2server; update pktlen accordingly (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/configure/table_configure.py)
				* After receiving NETCACHE_VALUEUPDATE_ACK, server can remove the key from beingupdated keyset as in NetCache
	* Cache population w/o blocking
		- NOTE: still w/ blocking during value update (aka phase 2 of cache coherence mechanism)
		- Each switch reports cache population to local control plane (aka switchos) as in NetCache
			+ NOTE: not sure how DistCache guarantees that each cache layer has one copy of cached object -> in our implementation, we will notify each cache population/eviction to corresponding cache switches by controller such that we can use power-of-two-choices for those cache copies
			+ The difference compared with NetCache is that: DistCache does NOT need to fetch value from storage server, which also does NOT need beingcached keyset to block write requests; while it needs NETCACHE_VALUEUPDATE to validate in-switch value after receiving NETCACHE_CACHE_POP_FINISH
		- For cache population caused by each NETCACHE_GETREQ_POP/NETCACHE_WARMUPREQ_INSWITCH_POP
			+ In spine/leaf-switchos.dppopserver
				* Send NETCACHE_CACHE_POP to controller as usual, yet wait for NETCACHE_CACHE_POP_ACK (w/ default value; NOT fetch value from storage server) from controller (files: switchos.c)
					- NOTE: NETCACHE_CACHE_POP and CACHE_POP_ACK w/o value; NETCACHE_CACHE_POP_ACK and CACHE_POP w/ value
			+ In controller
				* After receiving NETCACHE_CACHE_POP, directly send CACHE_POP w/ default value to spine&leaf-switchos.cppopserver instead of fetching value from storage server (files: controller.c)
				* After receiving two CACHE_POP_ACKs, send NETCACHE_CACHE_POP_ACK w/ default value to spine/leaf-switchos.dppopserver (files: controller.c)
			+ In spine&leaf-switchos.cppopserver
				* After receiving CACHE_POP, send CACHE_POP_ACK back to controller.popserver, and notify switchos.popworker as usual
			+ In spine&leaf-switchos.popworker
				* Directly send CACHE_POP_INSWITCH to server to set in-switch cache w/ default value: vallen = 0, latest = 0, deleted = 1 (aka stat = 0), and savedseq = 0 (files: tofino-*/configure/table_configure.py)
				* Send NETCACHE_CACHE_POP_FINISH to controller and storage server, and wait for NETCACHE_CACHE_POP_FINISH_ACK as usual
				* For cache eviction, spine sends DISTCACHE_CACHE_EVICT_VICTIM to leaf and waits for DISTCACHE_CACHE_EVICT_VICTIM_ACK to keep same copies in spine/leaf switch for power-of-two-choices as implemented before
					- Before cache population, spine/leaf sends NETCACHE_CACHE_EVICT to server to remove key from cached/beingupdated keyset and waits for NETCACHE_CACHE_EVICT_ACK as usual (files: server_impl.h)
			+ In storage server
				* NOTE: server.popserver does NOT need to process NETCACHE_CACHE_POP -> NOT need to add the key into beingcached keyset and NOT need to fetch value for cache population (files: server_impl.h)
				* After receiving NETCACHE_CACHE_POP_FINISH, send NETCACHE_CACHE_POP_FINISH_ACK to controller and spine/leaf switchos immediately to avoid timeout and retry (files: server_impl.h)
				* Add the key into cached keyset (files: server_impl.h)
					- NOTE: NOT need beingcached keyset as switchos does NOT fetch value from server during cache population (files: server_impl.h)
				* Perform phase 2 of cache coherence to update inswitch value in server.popserver (files: server_impl.h)

## Simple test

- Normal requests
	+ 3 GETREQs -> 3 GETRESs by server (power-of-two-choices under cache miss)
		* spineload_forclient of correct spineswitchidx = leafload_forclient = 0, toleaf_predicate = 2 -> spineload of incorrect spineswitchidx = 1, leafload = 1
		* spineload_forclient of correct spine switchidx=0 < leafload_forclient = 1, toleaf_predicate = 1 -> spineload of correct spineswitchidx = 1, leafload = 2
		* spineload_forclient of correct spine switchidx=1 < leafload_forclient = 2, toleaf_predicate = 1 -> spineload of correct spineswitchidx = 2, leafload = 3
	+ PUTREQ -> PUTRES by server
	+ DELREQ -> DELRES by server
	+ SCANREQ -> SCANRES by server
	+ LOADREQ -> LOADACK by server
- Cache population (cache coherence phase 2)
	+ WARMUPREQ -> NETCACHE_WARMUP_INSWITCH_POP and WARMUPACK by spine switch
		* Perform population: NETCACHE_CACHE_POP -> CACHE_POP_INSIWTCH (latest=1) -> NETCACHE_CACHE_FINISH/_ACK -> NETCACHE_VALUEUPDATE/_ACK
	+ Sufficient GETREQs under sampling -> GETREQ w/ is_hot=1 and is_report=0 -> NETCACHE_GETREQ_POP by server-leaf and GETRES by server -> perform population (latest=0 -> NETCACHE_VALUEUPDATE/_ACK -> latest=1)
		* Note latest=1 -> GETREQ w/ is_hot=0 and is_report=1 -> GETRES by switch (latest=1)
		* Set latest=0 manually -> GETREQ w/ is_hot=1 and is_report=1 -> GETRES by server w/o population (latest=0)
		* Set bf1=0 manullay -> GETREQ w/ is_hot=1 and is_report=0 -> NETCACHE_GETREQ_POP by server-leaf and GETRES by server -> no population (latest=0, bf1=1)
			- NOTE: due to sampling, not every GETREQ can trigger NETCACHE_GETREQ_POP and filtered by switchos
- Cache hit (cache coherence phase 1&2)
	+ GETREQ -> GETRES by switch (latest=1)
	+ PUT/DELREQ -> NETCACHE_PUT/DELREQ_CACHED -> DISTCACHE_INVALIDATE/_ACK by server to set latest=0 -> PUT/DELRES by server -> NETCACHE_VALUEUPDATE/_ACK (latest=1, deleted=0/1)
- Cache eviction
	+ WARMUPREQ of another key -> NETCACHE_WARMUPREQ_INSWITCH_POP and WARMUPACK by spine switch
		* Perform eviction: CACHE_EVICT_LOADFREQ_INSWITCH/_ACK by spine -> DISTCACHE_CACHE_EVICT_VICTIM/_ACK between spine and server-leaf -> both remove victim.key from cache_lookup_tbl -> both send/recv NETCACHE_CACHE_EVICT/_ACK -> server remove key from cached/beingupdated keyset
		* Perform population: NETCACHE_CACHE_POP -> CACHE_POP_INSIWTCH (latest=0) -> NETCACHE_CACHE_FINISH/_ACK
	+ Sufficient GETREQs of another key -> NETCACHE_GETREQ_POP by server-leaf and GETRES by server
		* Perform eviction and population as mentioned above

## Deprecated stuff

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
	- Maunual way to launch testbed (out-of-date)
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
	- Automatic way to launch testbed (latest)
		+ In each switch
			* Run `su` to enter root account
			* Run `bash localscripts/launchswitchostestbed.sh spine/leaf` to configure switch, launch switchos and ptf_popserver/cleaner
				- Run `bash localscripts/stopswitchostestbed.sh` to stop switch, switchos, and ptf_XXX
		+ In client 0 (dl11)
			* Run `bash remotescripts/launchservertestbed.sh` to launch controller, server, and reflector if any
				- Run `bash remotescripts/stopservertestbed.sh` to stop contoller, server, and reflector if any
	- Launch clients in end host
		- Warmup phase: `./warmup_client`
			* In each switch, run `bash set_all_latest.sh`
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
			+ Start switchos and ptf.popserver
			+ Start clients and servers for loading phase
			+ Start clients and servers for warmup phase
				* In each switch, run `bash set_all_latest.sh`
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
