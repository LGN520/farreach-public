# UDP-based NoCache + LSM-based KVS (rocskdb) + write-through in-switch cache

- Copy from netreach-v4-lsm
- See [important notes](./netreach-v4-lsm.md) in netreach-v4-lsm.md
+ IMPORTANT NOTE for optype enumeration
	- We keep the same optype enumeration fo FarReach, NoCache, and NetCache (files: packet_format.h, tofino/main.p4, tofino/common.py, tofino/p4src/parser.p4)
	- When adding a new optype
		+ If it is only processed by end-hosts, you need to update optype enumeration and packet format implementation (files: packet_format*, tofino/main.p4, tofino/common.py, tofino/p4src/parser.p4)
		+ If it is processed by switch, besides previous files, you also need to consider ingress partition_tbl + ig_port_forward_tbl, and egress eg_port_forward_tbl + update_pktlen_tbl + update_ipmac_srcport_tbl
	- NOTE: packet implemention is different (e.g., NetCache has inswitch_hdr.hashval_for_bfX and clone_hdr.server_sid/port yet FarReach NOT need)
+ Important NOTE for NetCache
	* [IMPORTANT] Issue not pointed out by NetCache: What if server.valueupdateserver validates in-switch cache after the subsequent PUTREQ invalidates in-switch cache, which may incur temporary unavailability/unserializability/inconsistency?
	* NOTE: for fair comparison
		* We perform in-switch key-based partition in each method
		* We use UDP with timeout-and-retry mechanism in each method
		* We keep the same seq mechanism in NetCache, which uses the sequence number as value version (it can get seq for GETREQ)
		* We do not load HHs of reported keys to choose more popular keys, as cache eviction is not frequent and we can evict all reported keys for high cache hit ratio -> actually, setting a larger hot threshold equals with filtering reported keys in controller under a smaller hot threshold
		* We do not use bitmap for fine-grained value control due to insufficient stages and no influence on evaluation results
		* We use a fixed sampling rate for both FarReach and NetCache
	* NOTE for validvalue_reg and latest_reg
		* validvalue_reg is used for atomicity of cache population/eviction in FarReach. i.e, validity of inswitch key and value
		* latest_reg is used for judging whether inswitch value is up-to-date in FarReach/NetCache, i.e., validity of inswitch value
		* FarReach requires validvalue_reg for atomic cache update especially for updaing cache_look_tbl of all ingress pipelines
			- During cache population, if w/o atomicity, some PUTREQs hit inswitch cache and mark latest=1, while others miss inswitch cache and update server-side value -> GETREQ may get out-of-date value from inswitch cache
			- During cache eviction, if w/o atomicity, CACHE_EVICT_LOADDATA_INSWITCH may load inconsistent data from inswitch cache
		* NetCache does not need validvalue_reg as it uses write-through instead of write-back policy
			- During cache population, even if a PUTREQ is not changed into PUTREQ_CACHED due to incorrect cached=0, it still waits for cache population and triggers NETCACHE_VALUEUPDATE as the key is in beingcached/cached keyset in server
			- During cache eviction, even if a PUTREQ is changed into PUTREQ_CACHED due to incorrect cached=1
				+ If key is not in beingcached/cached keyset due to NETCACHE_CACHE_EVICT, it process as usual PUTREQ
				+ If key is in beingcached/cached keyset and the PUTREQ_CACHED triggers a NETCACHE_VALUEUPDATE, it will only update inswitch value if cached=1 (i.e., victim key is not removed from all ingress pipelines); however, controller will store value of new key into switch only after victim key is removed from all ingress pipelines -> it will never overwrite the value of newly-populated key

## Overview

+ Basically follow algorithm 1 to design NetCache implementation
	* NOTE: NetCache controller is only responsible for cache population/eviction without snapshot
	- Tricky details
		+ Both NETCACHE_PUTREQ_SEQ_CACHED and NETCACHE_CACHE_POP_FINISH can move key from beingcached to cached keyset
		+ Removing key from beingupdated keyset by NETCACHE_CACHE_EVICT can affect NETCACHE_VALUEUPDATE_ACK and normal requests blocked for beingupdated keyset
	- Design for cache population
		+ switchos needs to fix duplicate reported keys (which may be already cached) due to either reliable report mechanism by duplication or periodic cleanup of bloom filter
		+ switchos sends NETCACHE_CACHE_POP to controller, which gets latest value from corresponding server -> server adds the key into beingcached keyset
			* NOTE: NETCACHE_CACHE_POP's key must not in beingcached/cached/beingupdated keyset
		+ switchos updates inswitch value -> adds key into MAT -> sends NETCACHE_CACHE_POP_FINISH to server -> server moves the key from beingcached keyset to cached keyset
			* NOTE: NETCACHE_CACHE_POP_FINISH's key must be in beingcached keyset or cached keyset
			* NOTE: NETCACHE_CACHE_POP_FINISH's key may already in cached keyset due to NETCACHE_PUTREQ_SEQ_CACHED
	- Design for cache eviction
		+ Controller collects frequency counters from data plane and finds victim
		+ Controller removes victim key from MAT -> sends NETCACHE_CACHE_EVICT to server, which removes key from beingcached/cached/beingupdated keysets atomically -> performs cache population
			* NOTE: NETCACHE_CACHE_EVICT's key must in beingcached keyset or cached keyset
			* NOTE: NETCACHE_CACHE_EVICT does not need to block for beingupdated keyset of previous write queries
	- Design for GETREQ
		+ If cached=1, latest=1, switch sends back GETRES directly
		+ Otherwise
			* If is_hot=1 and is_report=0, switch clones duplicate NETCACHE_GETREQ_POP to switchos, and forwards GETREQ to server finally
				- NOTE: we cannot forward GETREQ to server for the first cloned NETCACHE_GETREQ_POP to switchos, as optype has been changed into NETCACHE_GETREQ_POP
				- NOTE: although GETREQ and NETCACHE_GETREQ_POP are the same in essence, we cannot directly forward NETCACHE_GETREQ_POP to server at the same time of the first cloned NETCACHE_GETREQ_POP, as udp.dstport has been changed into reflector.port
				- IMPORTANT: as we cannot forward pkt to server at the same time of the first cloned NETCACHE_GETREQ_POP, we have to mirror the last cloned NETCACHE_GETREQ_POP for GETREQ to server
					+ NOTE: as we do not have partition_tbl in egress pipeline, we have to match eg_intr_md.egress_port and set clone_hdr.server_sid (we can merge meta.server_sid for SCANREQ_SPLIT and clone_hdr.server_sid for GETREQ_INSWITCH as one field to reduce PHV) -> then we match clone_hdr.server_sid in eg_port_forward_tbl to clone the last NETCACHE_GETREQ_POP as GETREQ to the corresponding server
			* Otherwise, switch forwards GETREQ to server
	- Design for PUTREQ (DELREQ is similar)
		+ NOTE: we allow temporary incoherence as NetCache paper
		+ NOTE: NetCache uses blocking cache update, yet FarReach uses non-blocking cache update
		+ For PUTREQ in switch
			+ If cached=1, switch sets latest=0 and forwards NETCACHE_PUTREQ_SEQ_CACHED to server
			+ Otherwise, switch forwards PUTREQ_SEQ to server
		+ For PUTREQ_SEQ/NETCACHE_PUTREQ_SEQ_CACHED in server
			* (1) If no key in beingcached/cached keyset, process it as usual
			* (2) If key in beingcached keyset not in cached keyset, block until key is in cached keyset, and go to step (3)
			* (3) If key in cached keyset
				- (3-1) If key not in beingupdated keyset, server sends PUTRES to client -> marks key as being updated and notifies server.valueupdateserver to update in-switch value by NETCACHE_VALUEUPDATE
					+ NOTE: NETCACHE_VALUEUPDATE_ACK's key may not in beingupdated keyset due to NETCACHE_CACHE_EVICT -> key must not in beingcached/cached keyset
				- (3-2) If key in beingupdated keyset, server blocks until the key is not in beingupdated keyset
					+ After the key is removed from beingupdated keyset, server needs to double-check cached keyset
						* NOTE: the key must not in beingcached keyset
						* If key is in cached keyset, it must be removed from beingupdated keyset by server.valueupdateserver (normal case) -> go to step (3-1)
						* If key is not in cached keyset due to NETCACHE_CAHCE_EVICT, it can directly go to step (1)
			* For PUTREQ, step (1) is likely
			* For NETCACHE_PUTREQ_SEQ_CACHED, step (3) is likely
				- If key is in beingcached keyset, i.e., step (2), NETCACHE_PUTREQ_SEQ_CACHED does not need to block, which directly moves key from beingcached keyset into cached keyset and go to step (3)
			* NOTE: NETCACHE_PUTREQ_CACHED may perform step (1) due to cache eviction

## Implementation log

+ Important code change of FarReach/NetCache to populate deleted keys
	* Support deleted keys for cache population
		- FarReach: CACHE_POP w/ stat (not stat_hdr) (files: packet_format*, server_impl.h)
			+ Sync to nocache and netcache (files: packet_foramt*)
		- NetCache: NETCACHE_CACHE_POP_ACK w/ stat (not stat_hdr) (files: packet_format*, server_impl.h, switchos.c)
		- Both: CACHE_POP_INSWITCH w/ stat_hdr (0x0007 -> 0x007f) to set deleted_reg (files: packet_format*, tofino/p4src/parser.p4, tofino/netbufferv4.p4, tofino/netcache.p4, tofino/common.py, tofino/p4src/regs/deleted.p4, configure/table_configure.py, switchos.c)
			+ sync to nocache (files: packet_format*)
	* Update for WARMUPREQ
		- Comment val_hdr from WARMUPREQ (0x0011 -> 0x0000), and remove WARMUPREQ from add_and_remove_value_header_tbl (files: packet_format.*, netbufferv4/nocache/netcache.p4, tofino/common.py, tofino/p4src/parser.p4, warmup_client.c) -> sync to nocache and netcache
		- FarReach: server gets value for WARMUPREQ and sends CACHE_POP w/ stat (not stat_hdr) for cache population (files: server_impl.h)
		- NetCache
			+ Add NETCACHE_WARMUPREQ_INSWITCH (only used by switch) and NETCACHE_WARMUPREQ_INSWITCH_POP (files: packet_format.*, tofino/main.p4, tofino/common.py, p4src/parser.p4. common_impl.h) -> sync optype enumeration to nocache and farreach
			+ Implement WARMUPREQ in switch
				* Add WARMUPREQ in prepare_for_cachehit_tbl, and convert it as NETCACHE_WARMUPREQ_INSWITCH in ig_port_forward_tbl (files: p4src/ingress_mat.p4, configure/table_configure.py)
				* Add NETCACHE_WARMUPREQ_INSWITCH in save_client_udpport_tbl (configure/table_configure.py)
				* Add NETCACHE_WARMUPREQ_INSWITCH_POP in lastclone_lastscansplit_tbl (configure/table_configure.py)
				* Update NETCACHE_WARMUPREQ_INSWITCH as NETCACHE_WARMUPREQ_INSWITCH_POP to switchos and send WARMUPACK to client by mirroring (files: p4src/egress_mat.p4, configure/table_configure.py)
				* Add NETCACHE_WARMUPREQ_INSWITCH_POP and WARMUPACK in update_pktlen_tbl and update_ipmac_srcport_tbl (files: p4src/egress_mat.p4, configure/table_configure.py)
				* Remove WARMUPREQ from update_ipmac_srcport_tbl (configure/table_configure.py)
			+ switchos processes NETCACHE_WARMUPREQ_INSWITCH_POP as NETCACHE_GETREQ_POP for cache population (files: reflector_impl.h, switchos.c)
			+ Remove WARMUPREQ from server (files: server_impl.h)
	* Compile FarReach under range partition -> try dynamic workload -> OK
	* Compile FarReach under hash partition -> try dynamic workload
	* Compile NoCache under range partition -> try dynamic workload
	* TODO: Compile NoCache under hash partition -> try dynamic workload

+ Implement NetCache
	* Client: not change
	* Switch: write-back -> write-through policy
		- Remove ptf_snapshotserver* and recover_switch*
		- Replace "netbufferv4" and "netreach-v4-lsm" as "netcache"
		- Ingress
			+ Remove need_recirculate_tbl, recirculate_tbl, and meta.need_recirculate (files: netcache.p4, p4src/ingress_mat.p4, p4src/header.p4, configure/table_configure.py)
			+ Remove snapshot_flag_tbl and inswitch_hdr.snapshot_flag (files: netcache.p4, p4src/ingress_mat.p4, p4src/header.p4, p4src/egress_mat.p4, configure/table_onfigure.p4, p4src/regs/case1.p4)
			+ Remove forward_special_get_response() from ipv4_forward_tbl (files: p4src/ingress_mat.p4, configure/table_configure,py)
				* NOTE: NetCache does not have conservative read
		- Egress
			+ Remove access_case1_tbl, case1_reg, meta.is_case1 (files: netcache.p4, p4src/egress_mat.p4, p4src/regs/case1.p4, p4src/header.p4, configure/table_configure.py)
			+ Remove drop_tbl (files: netcache.p4, p4src/egress_mat.p4, configure/table_configure.py)
			+ Remove validvalue_tbl, validvalue_hdr, and parse_validvalue (files: netcache.p4, p4src/regs/validvalue.p4, p4src/header.p4, p4src/parser.p4, p4src/egress_mat.md, p4src/regs/seq.p4, p4src/regs/val.p4, p4src/regs/latest.p4, p4src/regs/deleted.p4, configure/table_configure.py)
		- Remove unnecessary ingress MAT actions and entries
			+ Remove CACHE_EVICT_LOADDATA_INSWITCH and LOADSNAPSHOTDATA_INSWITCH from hash_for_partition_tbl, range_partition_tbl, and hash_partition_tbl (files: configure/table_configure.py, netcache.p4)
			+ Remove PUTREQ from hash_for_cmX_tbl, sample_tbl (files: configure/table_configure.py)
				* NOTE: NetCache only updates frequency counter or CM sketch for GETREQ
			+ Remove PUTREQ and DELREQ from prepare_for_cachehit_tbl (files: configure/table_configure.py)
				* NOTE: NetCache only supports cache hit for GETREQ
			+ Remove GETRES_LATEST/DELETED_SEQ and corresponding actions from ig_port_forward_tbl (files: configure/table_configure.py, p4src/ingress_mat.p4)
				* NOTE: GETREQ needs inswitch_hdr for cache hit; PUTREQ/DELREQ needs inswitch_hdr for invalidation; SCANREQ needs split_hdr for SCANREQ_SPLIT
				* NOTE: NetCache does not have design of conservative read
		- IMPORTANT: Tune MAT placement in egress pipeline as in Algorithm 1 (files: netcache.p4, p4src/regs/latest.p4, p4src/regs/seq.p4, p4src/egress_mat.p4, p4src/regs/cm.p4, p4src/regs/cache_frequency.p4, p4src/regs/deleted.p4)
			* Move access_latest_tbl, access_seq_tbl, save_client_udpport_tbl, process_scanreq_split_tbl in stage 0 (not related with latest)
			* Move access_cmX_tbl in stage 1 (update only if sampled=1 and (cached=0 or latest=0) for GETREQ)
			* Move is_hot_tbl (rely on CM), access_cache_frequency_tbl (update only if sampled=1 and cached=1 and latest=1 for GETREQ) in stage 2
			* Move access_deleted_tbl and access_savedseq_tbl in stage 2 to make room for bloom filter (rely on ishot) in stage 3
		- Remove unnecessary egress MAT actions and entries
			+ Remove GETRES_LATEST/DELETED_SEQ_INSWITCH, invoke reset_and_get_latest for PUTREQ/DELREQ_INSWITCH, and invoke set_and_get_latest for CACHE_POP_INSWITCH in access_latest_tbl (files: configure/table_configure.py)
				* NOTE: NetCache does not have design of conservative read
				* NOTE: NetCache writes invalidate inswitch value; cache population directly sets latest=1 due to blocking-based cache update
			+ Remove PUTREQ/DELREQ_INSWITCH from save_client_udpport_tbl (files: configure/table_configure.py)
				* NOTE: NetCache only supports cache hit for GETREQ
			+ Remove PUTREQ_INSWITCH and match meta.is_latest in access_cmX_tbl and access_cache_frequency_tbl (files: configure/table_configure.py, p4src/regs/cm.p4, p4src/regs/cache_frequency.p4)
				* NOTE: NetCache only updates frequency counter or CM sketch for GETREQ
			+ Remove GETRES_LATEST/DELETED_SEQ_INSWTICH, PUTREQ/DELREQ_INSWITCH, CACHE_EVICT_LOADDATA/LOADSNAPSHOTDATA_INSWITCH in access_deleted_tbl, access_savedseq_tbl, and access_vallen_tbl (files: configure/table_configure.py, p4src/regs/deleted.p4, p4src/regs/seq.p4, p4src/regs/val.p4)
				* NOTE: NetCache does not have design of conservative read
				* NOTE: NetCache does not support write-back policy
				* NOTE: NetCache does not have dirty data in switch and does not need inswitch snapshot
			+ Remove GETRES_LATEST/DELETED_SEQ_INSWITCH_CASE1 and PUTREQ/DELREQ_SEQ_INSWITCH_CASE1 from lastclone_lastscansplit_tbl (files: configure/table_configure.py)
				* NOTE: NetCache does not need inswitch snapshot and hence NO speical cases
			+ For eg_port_forward_tbl (files: configure/table_configure.py, p4src/egress_mat.md)
				* Remove validvalue-related actions for GETREQ_INSWITCH
				* Remove all actions for GETRES_LATEST/DELETED_SEQ, GETRES_LATEST/DELETED_SEQ_INSWITCH, and GETRES_LATEST/DELETED_SEQ_INSWITCH_CASE1
				* Keep only two actions, XXXreq_inswitch_to_XXXreq_seq and XXXreq_inswitch_to_netcache_XXXreq_seq_cached, for PUT/DEL
				* Remove all actions for PUTREQ_SEQ_INSWITCH_CASE1 and DELREQ_SEQ_INSWITCH_CASE1
				* Remove all actions for CACHE_EVICT_LOADDATA_INSWITCH/_ACK, LOADSNAPSHOTDATA_INSWITCH/_ACK, and SETVALID_INSWITCH/_ACK
			+ Remove XXX_CASE1, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ/_CASE3, CACHE_EVICT_LOADDATA_INSWITCH_ACK, LOADSNAPSHOTDATA_INSWITCH_ACK, PUTRES/DELRES, DELREQ_SEQ_CASE3, SETVALID_INSWITCH_ACK from update_pktlen_tbl
			+ Remove GETREQ_NLATEST, GETREQ_POP, PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ/_CASE3, DELREQ_SEQ_CASE3; GETRES_LATEST/DELETED_CASE1, CACHE_EVICT_LOADDATA_INSWITCH_ACK, LOADSNAPSHOTDATA_INSWITCH_ACK, SETVALID_INSWITCH_ACK from update_ipmac_srcport_tbl
			+ Remove PUTREQ_SEQ_CASE3, PUTREQ_POP_SEQ/_CASE3, XXX_CASE1, CACHE_EVICT_LOADDATA_INSWITCH_ACK, LOADSNAPSHOTDATA_INSWITCH_ACK from add_and_remove_value_header_tbl
		- Introduce bloom filter to reduce duplicate reports to controller
			+ NOTE: FarReach resorts server to report cache population to controller, which brings reliability for pktloss and reduces in-switch hardware usage for bloom filter
			+ Add hash_for_bfX_tbl in stage 3/4/5 of ingress pipeline (files: p4src/header.p4, packet_format.h, p4src/ingress_mat.p4, netcache.p4)
			+ Create 3 register arrays each with 256K 1-bit entries in egress pipeline for BF as in NetCache paper, and prepare meta.is_report1/2/3 (files: p4src/regs/bf.p4, p4src/header.p4)
			+ Add access_bfX_tbl in stage 3 of egress pipeline for GETREQ_INSWITCH (files: netcache.p4, configure/table_configure.py)
				* If is_hot=1, try to update bloom filter and get original bit into meta.is_reportX
				* Otherwise, reset meta.is_reportX = 0
			+ Add is_report_tbl in stage 4: only if is_report1/2/3 = 1, set meta.is_report=1; otherwise, set meta.is_report=0 (files: netcache.p4, p4src/ingress_mat.p4, configure/table_configure.py)
			+ Match meta.is_report in eg_port_forward_tbl for GETREQ_INSWITCH (files: p4src/egress_mat.p4, configure/table_configure.py)
				* Only if is_hot=1 and is_report=0, switch clones duplicate NETCACHE_GETREQ_POP to switchos, and forwards GETREQ to server finally
			+ Clear bloom filter periodically in ptf_cleaner (files: ptf_cleaner/table_configure.py)
		- Implement hot key report mechanism
			- Replace meta.server_sid with clone_hdr.server_sid, and add clone_hdr.server_udpport (files: p4src/header.p4, p4src/egress_mat.p4, configure/table_configure.py, packet_format.h)
				+ NOTE: not affect update_pktlen_tbl by now (NetCache does not need XXX_CASE1)
			- Add prepare_for_cachepop_tbl in stage 1 for GETREQ_INSWITCH (files: netcache.p4, p4src/ingress_mat.p4, configure/table_configure.py)
				+ (1) Match eg_intr_md.egress_port into clone_hdr.server_sid 
				+ (2) Save udp.dstport into clone_hdr.server_port
				+ NOTE: prepare_for_cachepop_tbl cannot in the same stage as process_scanreq_split_tbl due to clone_hdr.server_sid
				+ NOTE: FarReach does not need such a MAT as we resort server to trigger cache population
			- Match clone_hdr.server_sid in eg_port_forward_tbl even under hash partition (files: p4src/egress_mat.p4, configure/table_configure.py)
				+ NOTE: only NETCACHE_GETREQ_POP and SCANREQ_SPLIT will try to match clone_hdr.server_sid
			- Implement GETREQ_INSWITCH (files: netcache.p4, tofino/common.py, p4src/parser.p4, p4src/egress_mat.p4, configure/table_configure.py, packet_format*, common_impl.h)
				+ Add packet type of NETCACHE_GETREQ_POP
				+ Remove update_getreq_inswitch_to_getreq_pop() in eg_port_forward_tbl
				+ Implement update_getreq_inswitch_to_netcache_getreq_pop_clone_for_pktloss_and_getreq() in eg_port_forward_tbl
			- Implement NETCACHE_GETREQ_POP (files: p4src/egress_mat.p4, configure/table_configure.py)
				+ Implement forward_netcache_getreq_pop_clone_for_pktloss_and_getreq() and update_netcache_getreq_pop_to_getreq_by_mirroring() in eg_port_forward_tbl
				+ Add NETCACHE_GETREQ_POP in lastclone_lastscansplit_tbl, update_pktlen_tbl, and update_ipmac_srcport_tbl
					* NOTE: replace update_ipmac_srcport_switch2switchos as update_dstipmac_switch2switchps in update_ipmac_srcport_tbl
			- Implement GETREQ (files: configure/table_configure.py)
				+ NOTE: we can leverage the default action nop() in eg_port_forward_tbl to forward GETREQ to server
				+ Add GETREQ in update_pktlen_tbl and update_ipmac_srcport_tbl (already exist)
				+ NOTE: we do not need to update pktlen for GETREQ from GETREQ_INSWITCH, but need for GETREQ from NETCACHE_GETREQ_POP
			- Implement WARMUPREQ, NETCACHE_WARMUPREQ_INSWITCH, NETCACHE_WARMUPREQ_INSWITCH_POP, and NETCACHE_WARMUPACK (see details in the previous IMPORTANT code change to support deleted keys)
	* Reflector
		- Remove CACHE_EVICT_LOADDATA_INSWITCH, SETVALID_INSWITCH, and LOADSNAPSHOTDATA_INSWITCH from reflector.cp2dpserver; remove CACHE_EVICT_LOADDATA_INSWITCH_ACK, SETVALID_INSWITCH_ACK, LOADSNAPSHOTDATA_INSWITCH_ACK, and XXX_CASE1 from reflector.dp2cpserver (files: reflector_impl.h)
			+ NOTE: NetCache uses write-through policy, which does not have dirty data and not need in-switch snapshot
		- Implement cache population/eviction
			+ reflector.dp2cpserver forwards NETCACHE_GETREQ_POP/NETCACHE_WARMUPREQ_INSWITCH_POP to switchos.popserver (files: reflector_impl.h)
	* Switchos
		- Remove code related with in-switch snapshot, special cases, and recover from switchos (files: switchos.c)
			+ NOTE: NetCache uses write-through policy, which does not have dirty data and not need in-switch snapshot
		- Implement cache population
			* Implement switchos.popserver (files: switchos.c)
				* Receive NETCACHE_GETREQ_POP/NETCACHE_WARMUPREQ_INSWITCH_POP from reflector.dp2cpserver (remove CACHE_POP)
				* After fixing duplicate reported keys, notify switchos.popworker for cache population/eviction
			* Implement switchos.popworker (files: packet_format*, tofino/netcache.p4, tofino/common.py, common_impl.h, switchos.c)
				* Add packet type of NETCACHE_CACHE_POP, NETCACHE_CACHE_POP_ACK, NETCACHE_CACHE_POP_FINISH, and NETCACHE_CACHE_POP_FINISH_ACK
				* Calculate global logical serveridx based on key of NETCACHE_GETREQ_POP
				* switchos.popworker.popclient_for_controller sends NETCACHE_CACHE_POP to controller.popserver with time-out-and-retry to fetch value from corresponding server.popserver
				* After receiving NETCACHE_CACHE_POP_ACK, send CACHE_POP_INSWITCH to set in-switch value
				* Add key into cache_lookup_tbl
				* Send NETCACHE_CACHE_POP_FINISH to controller and wait for NETCACHE_CACHE_POP_NFINISH_ACK with timeout-and-retry
		- Implement cache eviciton (switchos.popworker.evictclient) (switchos.c, tofino/main.p4, tofino/common.py, tofino/p4src/parser.p4, packet_format.*, common_impl.h)
			* Add packet type of NETCACHE_CACHE_EVICT and NETCACHE_CACHE_EVICT_ACK -> sync optype to farreach/nocache
			* Load frequency counters of sampled kv idxes and find victim
			* Directly remove victim.key from cache_lookup_tbl (NOT need to setvalid 3 and load evicted data)
			* Send NETCACHE_CACHE_EVICT to controller.evictserver and wait for NETCACHE_CACHE_EVICT_ACK
			* Remove victim.key from cache metadata and popserver.cached_keyset
			* Perform cache population (fetch value -> update in-switch value -> add key into MAT -> send NETCACHE_CACHE_POP_FINISH)
		- Implement PUTREQ/DELREQ
			+ Assign seq for PUTREQ/DELREQ_INSWITCH as in FarReach
			+ Add update_put/delreq_inswitch_to_put/delreq_seq() and update_put/delreq_inswitch_to_netcache_put/delreq_seq_cached() in eg_port_forward_tbl (files: p4src/egress_mat.p4, configure/table_configure.p4)
				* If cached=1, switch sets latest=0 and forwards NETCACHE_PUTREQ_SEQ_CACHED to server
				* Otherwise, switch forwards PUTREQ_SEQ to server
			+ Add packet type of NETCACHE_PUT/DELREQ_SEQ_CACHED -> sync optype enumeration to farreach/nocache (files: tofino/main.p4, tofino/common.py, p4src/parser.p4, packet_format.h, packet_format_impl.h)
			+ Add NETCACHE_PUT/DELREQ_SEQ_CACHED into update_pktlen_tbl and update_ipmac_srcport_tbl (files: p4src/egress_mat.p4, configure/table_configure.py)
		- Implement NETCACHE_VALUEUPDATE/_ACK
			+ NOTE: both NETCACHE_VALUEDUPATE and CACHE_POP_INSWITCH are used to update inswitch value; however
				* CACHE_POP_INSWITCH: (1) as sent by switchos, it needs to perform partition_tbl to enter corresponding egress pipeline, and output CACHE_POP_INSWITCH_ACK to switchos by drop_and_clone; (2) as the key is not cached, inswitch_hdr.kvidx is given by switchos, which directly updates registers
				* NETCACHE_VALUEUPDATE: (1) as sent by server, it does NOT need partition_tbl and directly output NETCACHE_VALUEUPDATE_ACK to server by setting eport as iport; (2) as the key should be cached, inswitch_hdr is added by switch w/ kvidx from cache_lookup_tbl, which updates registers only if is_cached = 1
			+ Add packet type of NETCACHE_VALUEUPDATE/_ACK, and NETCACHE_VALUEUPDATE_INSWITCH (only optype enumeration) (files: tofino/main.p4, tofino/common.py, tofino/p4src/parser.p4, packet_format.*)
			+ Add update_netcache_valueupdate_to_netcache_valueupdate_inswitch_and_sendback() in ig_port_forward_tbl (files: p4src/ingress_mat.p4, configure/table_configure.py)
				* Add inswitch_hdr; update optype and shadowtype
				* Set egress port as ingress port; swap udp.srcport and udp.dstport (dstport becomes valueupdateserver's port)
			+ Update registers for NETCACHE_VALUEUPDATE_INSWITCH if is_cached = 1 (files: configure/table_configure.py)
				* Set latest = 1 to validate in-switch value
				* Set deleted based on stat_hdr.stat
				* Update vallen, val, and savedseq accordingly
			* Add update_netcache_valueupdate_inswitch_to_netcache_valueupdate_ack() in eg_port_forward_tbl (files: p4src/egress_mat.p4, configure/table_configure.py)
				* Add NETCACHE_VALUEUPDATE_ACK in update_pktlen_tbl and update_ipmac_srcport_tbl
		- Sync config.ini to configs/* for NetCache
	* Controller
		- Remove code related with in-switch snapshot (files: controller.c)
		- Implement cache population
			+ Implement controller.popserver (files: controller.c, config.ini, iniparser/iniparser_wrapper.*, common_impl.h)
				* Forward NETCACHE_GETREQ_POP from switchos.popworker to corresponding server (popserver_port_start + global server logical idx)
				* Wait for NETCACHE_GETREQ_POP_ACK from server with a timeout and forward it to switchos.popworker
					- NOTE: if timeout, controller does not need to retry, which is covered by switchos.popoworker
				* Forward NETCACHE_CACHE_POP_FINISH from switchos.popworker to corresponding server (popserver_port_start + global server logical idx)
				* Wait for NETCACHE_CACHE_POP_FINISH_ACK from server with a timeout and forward it to switchos.popworker
					- NOTE: if timeout, controller does not need to retry, which is covered by switchos.popoworker
		- Implement cache eviction (controller.evictserver) (files: controller.c)
			* Forward NETCACHE_CACHE_EVICT from switchos.popworker.evictclient to server.evictserver
			* Forward NETCACHE_CACHE_EVICT_ACK from server.evictserver to switchos.popworker.evictclient
	* Server
		- Remove code related with in-switch snapshot from server (files: server_impl.h, server.c)
		- Remove code related with GETREQ_POP, PUTREQ_SEQ_POP (files: server.c)
		- Implement cache population
			+ Implement server.popserver (files: server_impl.h, server.c)
				* Receive NETCACHE_CACHE_POP from controller.popserver.popclient
					* Add key into beingcached keyset, and get latest value
					* Send NETCACHE_CACHE_POP_ACK to controller.popserver.popclient
				* Receive NETCACHE_CACHE_POP_FINISH from controller.popserver.popclient
					* Move key from beingcached keyset if any into cached keyset
					* Send NETCACHE_CACHE_POP_FINISH_ACK to controller.popserver.popclient
		- Implement cache eviction (server.evictserver) (files: server_impl.h)
			+ Receive NETCACHE_CACHE_EVICT from controller.evictserver.evictclient
				* Remove key from beingcached/cached/beingupdated keyset atomically
				* Send NETCACHE_CACHE_EVICT_ACK to controller.evictserver.evictclient
		- Implement PUTREQ/DELREQ (files: server_impl.h, config,ini, iniparser/iniparser_wrapper.*, common_impl.h, tofino/p4src/parser.p4, server.c)
			+ Remove GETREQ_NLATEST from server
			+ Follow design of PUTREQ/DELREQ to finish server-side implementation
				* Implement process of PUT/DELREQ_SEQ
				* Implement process of NETCACHE_PUT/DELREQ_SEQ_CACHED
				* Implement server.valueupdateserver
					- Reserver udp ports for valueupdateserver
					- Send NETCACHE_VALUEUPDATE and wait for NETCACHE_VALUEUPDATE_ACK
					- Remove key from beingupdated keyset

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
	+ `./split_workload load linenum` -> workloada-load-{split_num}/*-*.out
	+ `./split_workload run linenum` -> workloada-run-{server_num}/*.out
- Change partition method (hash/range partition)
	+ RANGE_SUPPORT in tofino/netbufferv4.p4
	+ USE_HASH/RANGE in helper.h
	+ RANGE_SUPPORT in tofino/common.py
- Local loading phase
	- `./loader` to launch loaders in end host
	- If using ycsb, after ycsb loading phase, run `./loadfinish_client` in each physical server
- Warmup/Transaciton phase
	- Switch
		- Run `cd tofino`
		+ Run `su` to enter root account
		+ Run `bash compile.sh` to compile p4 into binary code
		+ Run `bash start_switch.sh` to launch Tofino
	- Launch switchos in local control plane of Tofino
		+ Create a new terminal and run `./switchos`
		+ Create a new terminal and run `bash ptf_popserver.sh`
		+ Create a new terminal and run `bash ptf_snapshotserver.sh`
		+ Create a new terminal and run `bash configure.sh` to configure data plane, then run `bash ptf_cleaner.sh`
	- Launch controller in end host
		+ `./controller`
	- Launch servers in end host
		+ `./server server_physical_idx`
		+ NOTE: to close server, use `sudo kill -15` to send SIGKILL
	- Launch clients in end host
		- Warmup phase: `./warmup_client`
		- Before transaction phase: `./preparefinish_client`
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

## Fixed issues

## Future work
