# Tofino-based NetREACH + DPDK-based XIndex (in-memory KVS) + variable-length key-value pair w/ snapshot + control-plane-based cache update (netreach-v4)

- Copy from netreach-voting-v3-memory, refer to netreach-voting-v2
- Ptf will configure MAT in all pipelines
	+ To configure a specific pipeline, inject packet into data plane
- Althought eviction or snapshot focus on latest=1, which may come from response of GETREQ_NLATEST
	+ As savedseq of such a record must <= seq in server, no unnecessary overwrite for eviction and no incorrect result for range query
	+ For the results with latest=1 from PUTREQ/DELREQ, savedseq can be either larger or smaller than seq in server -> seq comparison
- For crash-consistent snapshot
	+ NOTE: we need case3 packet but not need case3 reg, as it does not save bandwidth, while explict notification for server-side snapshot is acceptable as snapshot is not frequent and dominated by register loading
	+ TODO: Controller
		* Prologue: send prepare messages to servers to enable current server-side snapshot; notify each switch OS to reset case1
		* Execution: set backup flag as true -> load in-switch cache -> set backup flag as false
		* Epilogue: notify servers for server-side snapshot -> receive all speical case reports -> send rollbacked snapshot to servers
		* NOTE: each switch OS can check # of case1=1 to decide the # of speical case reports; as the channel of data plane -> switch OS -> controller is reliable, all reports must eventually arrive at controller
	+ TODO: Server
		* Set snapshot flag = false -> initial snapshot -> set snapshot flag = true
		* Set snapshot flag = false only if receiving a prepare message of current period
		* When receiving case3 packet from data plane or explicit notification from controller, make server-side snapshot only if snapshot flag = false -> set snapshot flag = true
	+ NOTE: it guarantees that each server must make a server-side snapshot for each snapshot period exactly once!
- NOTE: one register can provide at most 3 stateful APIs -> we cannot aggregate valid and latest/deleted as a whole
- NOTE
	+ Egress pipeline recirculation issue is ok
	+ We provide weak-form durability -> data is durable before a certain snapshot timepoint -> bounded-error reliability
	+ Limited recirculation for atomicity in snapshot is ok

## Overview

- Packet format
	+ Ethernet + IP + UDP
	+ Operation header: 1B optype, 16B key
	+ Value header: 4B vallen, variable length value (8B padding if vallen <= 128B)
	+ Result header: 1B result
		* NOTE: success flag for PUTRES/DELRES; deleted flag for GETRES; SCANRES does not need it
	+ Inswitch header: 1b is_cached, 1b is_sampled, 1b is_wrong_pipeline, 9 bit eport_for_res, 9b sid, 3b padding, 2B hashval, 2B idx
	+ CACHE_POP: key, value, seq, serveridx
	+ CACHE_POP_INSWITCH: key, value, seq, inswitch_hdr.freeidx
	+ CACHE_EVICT: key, value, result, seq, serveridx
	+ CACHE_EVICT_ACK: key
- Client
	+ Send GETREQ and wait for GETRES
- Switch
	+ Ingress pipeline
		* Stage 0
			- need_recirculate_tbl (optype, ingress port -> need_recirculate; for atomic snapshot)
				+ optype: GETREQ, PUTREQ, DELREQ
		* Stage 1 (need_recirculate == 1)
			- recirculate_tbl (optype, need_recirculate; recirculate to the same ingress pipeline)
				+ optype: PUTREQ, DELREQ
		* Stage 1 (need_recirculate == 0)
			- snapshot_flag_Tbl (optype -> snapshot_flag)
			- sid_tbl (optype, ingress port -> eport_for_res, sid)
				+ optype: GETREQ, PUTREQ, DELREQ
				+ sid: used by egress pipeline to generate response by packet mirroring
			- cache_lookup_tbl (key -> idx, is_cached)
				+ idx: index for in-switch cache (assigned by controller)
				+ NOTE: even if the packet is not FarReach packet, set inswitch_hdr does not affect their PHVs
			- hash_for_partition_tbl (optype, key -> hashval_for_partition)
				+ optype: GETREQ, PUTREQ, DELREQ
			- hash_for_cm_tbl (optype, key -> hashval_for_cm)
				+ optype: GETREQ, PUTREQ
			- hash_for_seq_tbl (optype, key -> hashval_for_seq)
				+ optype: PUTREQ, DELREQ
			- sample_tbl (optype, key -> is_sampled)
				+ optype: GETREQ, PUTREQ
			- TODO: In distributed extension, spine/leaf switch uses idx to access the bucket in the corresponding virtual switch (controller has considered partition across virtual spine/leaf switches when assigning idx)
				+ TODO: Only leaf switch needs to perform hash_for_partition_tbl for partition across servers
				+ TODO; Only spine switch needs to assign seq
				+ NOTE: we enforce spine-leaf path for each key to ensure serializability and availability, which can be statically configured into switches (no hash operation in client); however, DistCache uses dynamic mechanism (power-of-two-choices) for load balance, client needs to perform hash-based sampling
					* TODO: In DistCache, client calculates a hash value to introduce sampling overhead
					* TODO: But not need to embed the sampling result; instead, we can use in-switch sample to choose spine or leaf;
			- NOTE: SCANREQ does not access all above tables
		* Stage 2 (need_recirculate == 0)
			- hash_partition_tbl or range_partition_tbl
				- Hash partition: (optype, hashval range, ingress port -> udp.dstPort, egress port, is_wrong_pipeline)
					+ optype: GETREQ, PUTREQ, DELREQ
					+ NOTE: PARTITION_COUNT (for consistent hashing) is independent with KV_BUCKET_COUNT
					+ TODO: Check why we need the param of 0 for MAT with range matching
				- TODO: range partition: (optype, key range, ingress port -> udp.dstPort, egress port)
					+ optype: GETREQ, PUTREQ, DELREQ, SCANREQ
					+ TODO: We treat the most significant 4B of key as int32_t for range matching -> need conversion between little-endian and small-endian for the 4B of each key
				- NOTE: we use different udp.dstPort under the same egress port to simulate different egress ports
		* Stage 3 (need_recirculate == 0)
			- ig_port_forward_tbl (op_hdr.optype -> op_hdr.optype)
				+ optype: GETREQ, PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ
				+ Add inswitch_hdr to FarReach packet
		* Stage 4 (need_recirculate == 0)
			- ipv4_forward_tbl (optype, ingress port, dstip -> egress port)
				- optype: GETRES, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH
				- For normal responses from server: set egress port to corresponding client
				- For GETRES_LATEST_SEQ/GETRES_DELETED_SEQ + INSWITCH from server	
					- Set egress port as ingress port to forward original packet to the same egress pipeline
					- Clone the original packet to corresponding client by clone_i2e for normal GETRES
				- NOTE: responses in ingress pipeline must come from server; switch-issues responses must in egress pipeline
	+ Egress pipeline
		* TODO: If with stage limitation, CM and cache_frequency can be placed into any stage
		* Stage 0 (4 ALU)
			- CM: 4 register arrays of 16b (optype, is_sampled, is_cached, hashval -> cm_predicates)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH
				+ NOTE: CM_BUCKET_COUNT is independent with KV_BUCKET_COUNT
				+ NOTE: we resort to server to notify controller to avoid in-switch BF; NetCache directly reports to server yet with BF
			- TODO: is_snapshot_tbl (optype -> is_snapshot)
		* Stage 1 (3 ALU)
			- is_hot_tbl (cm_predicates -> meta.is_hot)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH
				+ Reduce 4 cm_predicates into 1 meta.is_hot to reduce TCAM usage
			- cache_frequency_tbl (optype, is_sampled, is_cached, idx -> none)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH
			- valid_tbl (optype, is_cached, idx -> is_valid)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH
				+ NOTE: valid for cache population/eviction atomicity
					* TODO: valid=0 (invalid: newly-populated entry yet not all lookup tables are updated)
					* TODO: valid=1 (valid: cached entry with all lookup tables being updated)
					* TODO: valid=3 (being evicted: cached entry being evicted to server)
					* NOTE: Data plane does not change valid, while control plane changes valid: 0->1 or 0/1->3 or 3->0
				+ For data plane
					- TODO: valid=0: ignore the entry (treat it as uncached)
					- TODO: valid=1: conservative get, cached get, put/del, evict, snapshot
					- TODO: valid=3: get cache only if latest=1, put/del set latest=0 and forward to server (maybe case3)
			- seq (optype -> seq)
				+ optype: PUTREQ_INSIWTCH, DELREQ_INSWITCH
				+ NOTE: SEQ_BUCKET_COUNT is independent with KV_BUCKET_COUNT
				+ NOTE: we need seq mechaism to avoid unnecessary overwrite caused by cache eviction
		* Stage 2 (1 ALU)
			- latest_tbl (optype, is_cached, idx, valid -> is_latest)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH
				+ NOTE: latest=0 means no PUTREQ/DELREQ/GETRES_LATEST_SEQ/GETRES_DELETED_SEQ arrive at switch
				+ NOTE: even with seq mechanism, we still need latest_tbl for GETRES_LATEST_SEQ/GETRES_DELETED_SEQ 
		* Stage 3 (4 ALU)
			- deleted_tbl (optype, is_cached, idx, valid, is_latest -> is_deleted)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH
			- vallen (optype, is_cached, idx, valid, is_latest -> vallen_hdr)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH
				+ TODO: vallen and value can be placed from stage 1 to save # of stages if necessary
			- savedseq (optype, is_cached, idx, seq, valid, is_latest -> savedseq)
				+ optype: PUTREQ_INSIWTCH, DELREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH, CACHE_POP_INSWITCH
				+ NOTE: we assume that there is no reordering between spine and leaf switch due to a single FIFO channel
			- TODO: case1 (optype, is_cached, valid, is_latest, snapshot_flag, idx -> is_case1)
				+ optype: PUTREQ_INSIWTCH, DELREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH, GETRES_DELETED_SEQ_INSWITCH
		* Stage 4-9 (4 ALU)
			- From vallo1 to valhi12 (optype, is_cached, idx, valid, is_latest -> val_hdr)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH
			- lastclone_tbl (optype, clonenum_for_pktloss -> is_lastclone_for_pktloss)
				+ optype: CACHE_POP_INSWITCH_ACK
		* Stage 10 (4 ALU)
			- vallo13, valhi13, vallo14, valhi14 (optype, is_cached, idx, valid, is_latest -> val_hdr)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH
			- eg_port_forward_tbl (optype, is_cached, is_hot, is_valid, is_latest, is_deleted, is_wrong_pipeline -> optype)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH, GETRES_LATEST_SEQ
				+ TODO: If TCAM of single stage is not enough, we can assume the value of each matched key field is initialized
		* Stage 11 (4 ALU)
			- vallo15, valhi15, vallo16, valhi16 (optype, is_cached, idx, valid, is_latest -> val_hdr)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH
			- TODO: udphdr.hdrlen, ehterhdr.macaddr
- Server
	+ For GET: process GETREQ, GETREQ_POP, GETREQ_NLATEST -> GETRES, GETRES w/ cache population, GETRES_LATEST/DELETED_SEQ
	+ Cache population: each server maintains a set of keys being cached to avoid duplicate population
		* If GETREQ_POP triggers a cache population (i.e., key exists), server adds the key into cached key set
		* If server receives CACHE_EVICT, it removes the evicted key from cached key set
		* NOTE: we reduce server.evictservers to a single thread (server.evictserver) now
	+ TODO: Snapshot
		* Make server-side snapshot if necessary
			- optype: PUTREQ_SEQ_CASE3, DELREQ_SEQ_CASE3, CACHE_EVICT_CASE2, and SNAPSHOT_SERVERSIDE from controller.snapshotclient
		* TODO: Server.consnapshotserver receives snapshot data -> distribute data among servers with RCU mechanism for range query
			* TODO: deduplicate during range query
			* TODO: Treat DELREQ as a special write request -> do not ignore and free deleted atomic value, and remove deleted set (extended_xindex_dynamic_seq_del)
			* TODO: Provide getseq API for seq comparison
	+ TODOTODO: We can use multiple threads for controller.snapshotclient.consnapshotclients and server.consnapshotservers/evictservers if necessary -> controller needs to use perserver_bytes in snapshot data
- Controller
	+ Cache population
		* Receive CACHE_POP from server by tcp channel
			- Per-server popclient -> one controller.popserver (with multiple subthreads = # of servers)
		* CANCELED: Add key into per-server cached key set (comment it if server.cached_keyset works well)
		* CANCELED: Add key into key-server map (comment it as CACHE_EVICT embeds serveridx towards corresponding server)
		* Maintain a map between serveridx and controller.popserver.subthreadidx
		* Send CACHE_POP to corresponding switchOS
	+ Cache eviction
		* Receive CACHE_EVICT <victim.key, vicktim.value, victim.result, victim.seq, victim.serveridx>
		* CANCELED: Check per-server cached key set to find the corresponding server
		* Send CACHE_EVICT to the correpsonding server, and wait for CACHE_EVICT_ACK <victim.key>
		* Send CACHE_EVICT_ACK to the switch OS
	+ Snapshot
		* TODOTODO: If CASE3 with significant delay (impossible?), controller.snapshotclient sends SNAPSHOT_START to servers such that they can start to process CASE3/controller.notification for server-side snapshot (from receiving SNAPSHOT_START to receiving backup data)
		* controller.snapshotclient sends SNAPSHOT_START to ToR switchos periodically
			- NOTE: hybrid consistency and causal consistency are not accepted by caching which requires point-in-time consistency
			- For distributed snapshot
				+ TODO: controller enforces other spine switches echo each packet by the same spine switch A
				+ TODO: controller notifies A to start snapshot
					* TODO: The echoed packets also use A's flag to decide whether to report special cases
				+ TODO: controller notifies all other spines switches to start snapshot, and disables enforced echo mechanism
				+ NOTE: provide point-in-time consistency yet degrade throughput -> the degradation time should be limited compared with snapshot period!
		* Receive SNAPSHOT_SERVERSIDE from switchos, and notify servers for in-memory snapshot
		* Wait for SNAPSHOT_SERVERSIDE_ACK from servers, and send it to switchos
		* Receive per-switch snapshot (as final ACK of SNAPSHOT_START) -> send key-value records from controller.snapshotclient.consnapshotclient to server.consnapshotserver
		* Sleep until next snapshot period
- Switch OS
	+ Cache population/eviction
		* IMPORTANT: Workflow
			- Cache population: servers.popclients -> controller.popserver.subthreads -> controller.popclient -> switchos.popserver.connfd -> switchos.popworker -> ptf <-> switchos.paramserver
				+ NOTE: controller.popserver.subthreadidx != server.serveridx
			- Cache eviction: switchos.popworker notify -> ptf -> switchos.paramserver -> switchos.popworker.evictclient <-> controller.evictserver <-> controller.evictserver.evictclient <-> server.evictserver <-> server.normal_worker
			- NOTE: switchos.popworker performs cache population/eviction; switchos.paramserver communicates with ptf for parameters
		* Data structure
			- Maintain in-memory multi-level array: switch (TODO: different switches under distributed extension) -> egress pipeline (fixed due to testbed limitation) -> <idx, key and serveridx>
		* Detail: after receiving a CACHE_POP from controller -> check whether there exists free idx to assign
			- CANCELED: Add key into cached key set (comment it if server.cached_keyset works well)
			- If with free idx (cache population)
				+ Ptf: set valid[idx] = 0 for atomicity
				+ Switch os: send CACHE_POP_INSWITCH to data plane
					* NOTE: we use reflector to simulate extra link, which is stateless and does not deduplicate ACKs
					* TODO: try internal pcie port to inject pkt into specific pipeline
				+ Switch os: wait for CACHE_POP_INSWITCH_ACK <key>, where switchos.popworker deduplicates ACKs
				+ Ptf: (1) add a new entry <key, idx> into cache_lookup_tbl of all ingress pipelines; (2) and set valid[idx] = 1 to enable the cache entry
			- Otherwise (cache eviction)
				+ Switchos gets evictidx -> ptf:
					+ Sample idxes and load corresponding cache_frequency counters
					+ Choose the idx with the minimum frequency as the victim (approximate LRF)	
					+ Set valid[victim.idx] = 3 for atomicity (then only latest can be changed by data plane)
					+ Load deleted, vallen, val, and savedseq of victim
					+ Report evictidx, vallen, valbytes, deleted, and savedseq to switchos.paramserver
					+ NOTE: we do not consider latest here as latest=0 can still imply new value under valid=3
					+ FUTURE: consider valid if with PUTREQ_LARGE
				+ Switch os: get serveridx and key according to evictidx
				+ Switch os: report CACHE_EVICT <victim.key, vicktim.value, victim.result, victim.seq, victim.serveridx> to controller
					* NOTE: we do not need to load latest
						- Even if latest=0, the value could still be latest <- PUT/DEL (lost later) w/ valid=3 resets latest from 1 to 0
						- No matter value is latest or not, we can always compare savedseq with server.seq for availability
					* NOTE: CACHE_EVICT reported by switch OS instead of data plane -> no need <key, value, seq, inswitch_hdr.is_deleted>
				+ Switch os: wait for CACHE_EVICT_ACK
				+ Remove existing entry <victim.key, victim.idx> from cache_lookup_tbl of all ingress pipelines
				+ Invoke cache population for new CACHE_POP
		* Switch os: popworker resets intermediate data of paramserver after population/eviction
	+ Snapshot
		* NOTE
			- We need to finish current cache population/eviction before backuping cache metadata -> popworker_know_snapshot_prepare
			- We need to finish case1 & case2 before rollback -> popworker_know_snapshot_end & specialcaseserver_know_snapshot_end
				+ case1 between is_snapshot_prepare=true and is_snapshot=false are cached into udp recv buffer
		* Receive SNAPSHOT_START from controller 
		* snapshotserver stops cache population/eviction
			* snapshotserver sets is_snapshot_prepare=true such that popworker will be stopped temporarily
			* popworker sets popworker_know_snapshot_prepare=true if is_snapshot_prepare=true and popworker_know_snapshot_prepare=false
				- If is_snapshot_prepare=false w/ cache population from controller.popclient, popworker performs cache population
			* snapshotserver waits until popworker_know_snapshot_prepare=true
			* NOTE: all cached records at the snapshot timepoint should have valid=1
		* snapshotserver.ptf sets snapshot_flag=true w/ atomicity
			* snapshotserver.ptf sets need_recirculate=true to enforce all traffic of other pipelines to enter the same ingress pipeline
			* snapshotserver.ptf sets snapshot_flag=true to notify data plane the beginning of snapshot
			* snapshotserver.ptf sets need_recirculate=false to disable recirculation asap to mitigate thpt degradation
				- As the duration for atomicity is very small compared with snapshot period, it should be acceptable
				- TODO: Test the duration based on time difference between comment and uncomment
				- NOTE: snapshot timepoint is that of setting the snapshot_flag of the enforced ingress pipeline as true
			* Data plane reports case1 to switchos for in-switch value update (w/ inswitch_hdr.idx)
				* GETRES_LATEST_SEQ_CASE1, GETRES_DELETED_SEQ_CASE1, PUTREQ_CASE1, DELREQ_CASE1
				* FUTURE: PUTREQ_CASE1/DELREQ_CASE1 need to embed meta.valid as valid could be 2 (temporarilly invalid for PUTREQ_LARGE); GETRES_LATEST/DELETED_SEQ_CASE1 do not need as valid must be 1 if with value update
				* NOTE: only if case1.key = cached_keyarray_backup[idx].key, specialcaseserver stores case1 after seq comparison (use smaller one)
			* Data plane reports case3 to server for in-memory snapshot
				* PUTREQ_SEQ_CASE3, DELREQ_SEQ_CASE3
				* FUTURE: PUTREQ_LARGE_SEQ_CASE3
		* snapshotserver backups metadata, i.e., key and serveridx for each cache entry
			- It is atomic as cache population/eviction is temporarily stopped now
			- NOTE: each entry should appear in the final crash-consistent snapshot with the correct key
		* snapshotserver resumes cache population/eviction
			- snapshotserver sets is_snapshot=true to remember cache eviction as case2 for snapshot
			- snapshotserver sets is_snapshot_prepare=false to resume cache population and eviction
				+ NOTE: now popworker knows is_snapshot=true, which will report case2 for snapshot
			- snapshotserver resets popworker_know_snapshot_prepare=false to reset for next snapshot
			- Cache population will insert a new key-value record into an empty entry (valid=0/1)
				+ NOTE: snapshotserver only loads values from 0 to empty_index-1; new key-value records from empty_index to kv_bucketnum-1 do not affect the snapshot -> no special case for cache population
			- Cache eviction will evict an old key-value record (valid=1/3/0/1)
				+ NOTE: only if victim.key = cached_keyarray_backup[idx].key, popworker stores case2 after seq comparison (use smaller one)
				+ popworker sends CACHE_EVICT_CASE2 instead of CACHE_EVICT to server for server-side snapshot
		* If empty_index > 0, snapshotserver.ptf loads idx (0 ~ empty_index_backup-1), vallen, value, deleted, and savedseq
			- NOTE: ptf does not need to care about the records with valid != 1, as snapshotserver will rollback them from the loaded snapshot with special cases if any
			- TODOTODO: For records with latest=0, we also store them into snapshow now, which does not break point-in-time consistency; it is just a duplication which can be solved by seq comparison
		* switchos.ptf reports loaded snapshot data to switchos.snapshotdataserver by tcp
		* snapshotserver sends SNAPSHOT_SERVERSIDE to controller and hence servers to make server-side snapshot, and waits for ACK
		* snapshotserver.ptf resets snapshot_flag=false which does not need atomicity, and reset case1_reg
			- Some ingress pipelines see snapshot_flag=true and report special cases, which must be the same as loaded data
			- Others see snapshot_flag=false and directly update in-switch value w/o special cases, which does not change loaded data
		* snapshotserver ensures that popworker knows the end of snapshot (all case2 have been collected)
			- snapshotserver resets is_snapshot=false such that no subsequent evicted data as case2
			- snapshotserver sets is_snapshot_end=true
			- popworker sets popworker_know_snapshot_end=true, if is_snapshot_end=true and popworker_know_snapshot_end=false (at the end of loop)
			- specialcaseserver sets specialcaseserver_know_snapshot_end=true, if is_snapshot_end=true and specialcaseserver_know_snapshot_end=false (when timeout occurs)
			- snapshotserver waits until popworker_know_snapshot_end=true and specialcasesever_know_snapshot_end=true
		* snapshotserver performs rollback to get a crash-consistent snapshot
		* snapshotserver acknowledges controller.snapshotclient with crash-consistent snapshot data
		* snapshotserver resets metadata
			- is_snapshot_end=false -> popworker_know_snapshot_end=false and specialcaseserver_know_snapshot_end=false
			- is_snapshot_prepare=false -> popworker_know_snapshot_prepare=false; is_snapshot=false
			- free/reset backuped cache metadata
			- clear specialcases
			- free/reset loaded snapshot data
		* TODOTODO: If with ptf session limitation, we can place snapshot flag in SRAM; load values and reset registers by data plane;
	+ TODO: Periodically reset CM
		* TODO: If with ptf session limitation, we can reset it in data plane
- NOTE: In the host colocated with server
	+ TODO: We use controller thread to simulate controller for cache management
	+ TODO: We use reflector thread to simulate the extra link for connection between data plane and switch OS

## Details 

- NOTE: For each RES directly answered by switch
	+ If is_wrong_pipeline=0, set eg_intr_md.egress_port as inswitch_hdr.eport_for_res
	+ Otherwise, use packet mirroring (drop original packet + clone_e2e to corresponding egress port by inswitch_hdr.sid)
- NOTE: For each RES from server
	+ See ipv4_forward_tbl for details
- GETREQ
	+ Client sends GETREQ
	+ Ingress: GETREQ -> GETREQ_INSWITCH (is_sampled, is_cached, is_wrong_pipeline, sid, eport_for_res, hashval_for_partition, hashval_for_cm, idx)
	+ Egress
		* Stage 0: update CM if inswitch_hdr.is_sampled=1 and inswitch_hdr.is_cached=0;
		* Stage 1: update is_hot; update cache_frequency if inswitch_hdr.is_sampled=1 and inswitch_hdr.is_cached=1; get valid
		* Stage 2: get latest
		* Stage 3: get deleted
		* Intermediate stages: get vallen and value
		* Stage 10: eg_port_forward_tbl
			* If inswitch_hdr.is_cached=0
				- If is_hot=1, forward GETREQ_POP to server
				- Otherwise, forward GETREQ to server
			* If inswitch_hdr.is_cached=1
				- If valid=0, forward GETREQ to server
				- If valid=1
					+ If latest=0, forward GETREQ_NLATEST to server
					+ If latest=1 and deleted=1, set result_hdr.result=0 as deleted and send back GETRES directly/mirrorly
					+ If latest=1 and deleted=0, set result_hdr.result=1, and send back GETRES directly/mirrorly
				- If valid=3
					+ If latest=0, forward GETREQ to server
					+ If latest=1 and deleted=1, set result_hdr.result=0 as deleted and send back GETRES directly/mirrorly
					+ If latest=1 and deleted=0, set result_hdr.result=1, and send back GETRES directly/mirrorly
	+ Server
		* GETREQ -> GETRES
		* GETREQ_POP
			- If key not exist, send back GETRES and ignore cache population
			- Otherwise, send back GETRES and notify controller for cache population
		* GETREQ_NLATEST
			- If key exists, send back GETRES_LATEST_SEQ
			- If key not exist, send back GETRES_DELETED_SEQ
- GETRES_LATEST_SEQ
	+ Server sends GETRES_LATEST_SEQ
	+ Ingress
		* GETRES_LATEST_SEQ -> GETRES_LATEST_SEQ_INSWITCH (is_cached, idx)
		* ipv4_forward_tbl -> forward GETRES_LATEST_SEQ_INSWITCH to pipeline of ingress port; clone GETRES_LATEST_SEQ to pipeline of client for GETRES 
	+ Egress
		* If inswitch_hdr.is_cached=1 and meta.valid=1 and meta.is_latest=0
			+ Set latest=1, delete=0, update savedseq, update vallen and value
		* access_case1_tbl: if is_cached=1, valid=1, is_latest=0, and snapshot_flag=1, set and get case1
		* For original packet
			+ If is_cached=1, valid=1, is_latest=0, snapshot_flag=1, and is_case1=0, send duplicate GETRES_LATEST_SEQ_CASE1 to switch os by reflector (clone_field_list: clonenum_for_pktloss)
			+ Otherwise, drop the original packet
		* For cloned packet (i2e), forward it to client
			+ TODO: Check if we need to set eg_intr_md.egress_port for cloned packet (i2e)
- GETRES_LATEST_SEQ_CASE1
	+ Egress pipeline
		* Access lastclone_tbl to update is_lastclone_for_pktloss
		* If is_lastclone_for_pktloss = 0, decrease clonenum_for_pktloss, forward to reflector, and clone again
		* Otherwise, only forward to reflector
- GETRES_DELETED_SEQ
	+ Server sends GETRES_DELETED_SEQ
	+ Ingress: 
		* GETRES_DELETED_SEQ -> GETRES_DELETED_SEQ_INSWITCH (is_cached, idx)
		* ipv4_forward_tbl -> forward GETRES_DELETED_SEQ_INSWITCH to pipeline of ingress port; clone GETRES_DELETED_SEQ to pipeline of client for GETRES 
	+ Egress
		* If inswitch_hdr.is_cached=1 and inswitch_hdr.valid=1 and latest=0
			+ Set latest=1, deleted=1, update savedseq, update vallen and value
		* access_case1_tbl: if is_cached=1, valid=1, is_latest=0, and snapshot_flag=1, set and get case1
		* For original packet
			+ If is_cached=1, valid=1, is_latest=0, snapshot_flag=1, and is_case1=0, send duplicate GETRES_DELETED_SEQ_CASE1 to switch os by reflector (clone_field_list: clonenum_for_pktloss)
			+ Otherwise, drop the original packet
		* For cloned packet (i2e), forward it to client
			+ TODO: Check if we need to set eg_intr_md.egress_port for cloned packet (i2e)
- GETRES_DELETED_SEQ_CASE1
	+ Egress pipeline
		* Access lastclone_tbl to update is_lastclone_for_pktloss
		* If is_lastclone_for_pktloss = 0, decrease clonenum_for_pktloss, forward to reflector, and clone again
		* Otherwise, only forward to reflector
- CACHE_POP_INSWITCH
	+ Ingress pipeline (hashval_for_partition, idx)
		* Access
			* hash_tbl: access to get hashval for partition in case that switch os cannot specify ingress pipeline
				- TODO: If switch os can specify ingress pieline, we can directly set egress port as ingress port
			* hash_partition_tbl: access to set egress port (not care about udphdr.dstport and is_wrong_pipeline)
				- TODO: range_partition_tbl for range partition
		* Not access
			* sid_tbl: NOT access as not need to clone to ingress port for response
			* cache_lookup_tbl.uncached_action does NOT change inswitch_hdr.idx, only sets iscached=0
			* sample_tbl: NOT access as not need to update CM and always reset frequency counter
			* ig_port_forward_tbl: NOT access as we already have inswitch_hdr
			* ipv4_forward_tbl: NOT access as not need to route/clone according to ipv4_hdr.dstip
	+ Egress pipeline (for the given idx)
		* Reset cache_frequency=0, latest=0, deleted=0
			- NOTE: we reset valid=0 by switchos.ptf, as we do not have extra valid.ALU in data plane
				* At most 3 ALUs: get valid, set valid from 1 to 2 and from 2 to 1 (TODO: prepared for PUTREQ_LARGE w/ cache hit)
		* Update vallen, value, savedseq
		* Send sufficient duplicate CACHE_POP_INSWITCH_ACKs <key> to switch OS to avoid packet loss
		* NOTE: valid is reset by switch OS; case1 is reset by snapshot thread;
- CACHE_POP_INSWITCH_ACK
	+ Egress pipeline
		* Access lastclone_tbl to update is_lastclone_for_pktloss
		* If is_lastclone_for_pktloss = 0, decrease clonenum_for_pktloss, forward to reflector, and clone again
		* Otherwise, only forward to reflector
- PUTREQ
	+ Client sends PUTREQ
	+ Ingress: PUTREQ -> PUTREQ_INSWITCH (is_sampled, is_cached, is_wrong_pipeline, sid, eport_for_res, hashval_for_partition, hashval_for_cm, hashval_for_seq, idx)
	+ Egress
		* Stage 0: update CM if inswitch_hdr.is_sampled=1 and inswitch_hdr.is_cached=0;
		* Stage 1: 
			+ update is_hot; update cache_frequency if inswitch_hdr.is_sampled=1 and inswitch_hdr.is_cached=1; get valid
			+ assign seq
		* Stage 2: latest_tbl
			+ If valid=0, skip latest_tbl
			+ If iscached=1 and valid=1, set latest=1
			+ If iscached=1 and valid=3, set latest=0
		* Stage 3:
			+ deleted_tbl
				* If valid=0/3, skip deleted_tbl (keep the original deleted)
				* If iscached=1 and valid=1, set deleted=0
			+ savedseq_tbl
				* If valid=0/3, skip savedseq_tbl (keep the original savedseq)
				* If iscached=1 and valid=1, set savedseq=seq
			+ access_case1_tbl: if is_cached=1, valid=1, snapshot_flag=1, set and get case1
		* Intermediate stages: if iscached=1 and valid=1, set vallen and value
		* Stage 10: eg_port_forward_tbl
			* If inswitch_hdr.is_cached=0
				- If snapshot_flag=1
					+ If is_hot=1, forward PUTREQ_POP_SEQ_CASE3 to server
					+ Otherwise, forward PUTREQ_SEQ_CASE3 to server
				- If snapshot_flag=0
					+ If is_hot=1, forward PUTREQ_POP_SEQ to server
					+ Otherwise, forward PUTREQ_SEQ to server
			* If inswitch_hdr.is_cached=1
				- If valid=0
					+ If snapshot_flag=1, forward PUTREQ_SEQ_CASE3 to server
					+ If snapshot_flag=0, forward PUTREQ_SEQ to server
				- If valid=1
					+ If snapshot_flag=1, and is_case1=0, send duplicate PUTREQ_SEQ_CASE1 to switchos by reflector (clone_field_list: clonenum_for_pktloss, is_wrong_pipeline, sid, eport_for_res)
						* NOTE: the last clone of PUTREQ_SEQ_CASE1 will be updated as PUTRES to client instead of forwarding PUTREQ_SEQ_CACASE1, so we need to set clonenum_for_pktloss=2 if we need 3 duplicate packets (GETRES_LATEST/DELETED_SEQ_CASE1 set clonenum=1)
						* TODOTODO: If clone_field_list violates limitation, we should embed inswitch_hdr as PUTREQ_INSWITCH_SEQ_CASE1
					+ Otherwise, set result_hdr.result=1, and send back PUTRES directly/mirrorly
				- If valid=3
					+ If snapshot_flag=1, forward PUTREQ_SEQ_CASE3 to server
					+ If snapshot_flag=0, forward PUTREQ_SEQ to server
	+ Server
		* PUTREQ_SEQ -> update KVS, and sendback PUTRES
		* PUTREQ_POP_SEQ -> update KVS, sendback PUTRES, and trigger cache population
		* PUTREQ_CAES3 -> make in-memory snapshot, update KVS, and sendback PUTRES
		* PUTREQ_POP_CAES3 -> make in-memory snapshot, update KVS, sendback PUTRES, and trigger cache population
		* NOTE: as we do not maintain case3_reg in switch, we do not need to sendback PUTRES_CASE3 w/ serveridx
- PUTREQ_SEQ_CASE1
	+ Egress pipeline
		* Access lastclone_tbl to update is_lastclone_for_pktloss
		* If is_lastclone_for_pktloss = 0, decrease clonenum_for_pktloss, forward to reflector, and clone again
		* Otherwise, send back PUTRES directly/mirrorly
- DELREQ
	+ Client sends DELREQ
	+ Ingress: DELREQ -> DELREQ_INSWITCH (is_sampled, is_cached, is_wrong_pipeline, sid, eport_for_res, hashval_for_partition, hashval_for_seq, idx)
	+ Egress
		* Stage 0: not touch access_cm_tbl 
		* Stage 1: 
			+ not care about is_hot_tbl; not touch cache_frequency_tbl; get valid
			+ assign seq
		* Stage 2: latest_tbl
			+ If valid=0, skip latest_tbl
			+ If iscached=1 and valid=1, set latest=1
			+ If iscached=1 and valid=3, set latest=0
		* Stage 3:
			+ deleted_tbl
				* If valid=0/3, skip deleted_tbl (keep the original deleted)
				* If iscached=1 and valid=1, set deleted=1
			+ savedseq_tbl
				* If valid=0/3, skip savedseq_tbl (keep the original savedseq)
				* If iscached=1 and valid=1, set savedseq=seq
			+ access_case1_tbl: if is_cached=1, valid=1, snapshot_flag=1, set and get case1
		* Intermediate stages: reset vallen = 0 (save bandwidth), not access value 
		* Stage 10: eg_port_forward_tbl
			* If inswitch_hdr.is_cached=0
				- If snapshot_flag=1, forward DELREQ_SEQ_CASE3 to server
				- If snapshot_flag=0, forward DELREQ_SEQ to server
			* If inswitch_hdr.is_cached=1
				- If valid=0
					+ If snapshot_flag=1, forward DELREQ_SEQ_CASE3 to server
					+ If snapshot_flag=0, forward DELREQ_SEQ to server
				- If valid=1
					+ If snapshot_flag=1, and is_case1=0, send duplicate DELREQ_SEQ_CASE1 to switchos by reflector (clone_field_list: clonenum_for_pktloss, is_wrong_pipeline, sid, eport_for_res)
						* NOTE: the last clone of DELREQ_SEQ_CASE1 will be updated as DELRES to client instead of forwarding DELREQ_SEQ_CACASE1, so we need to set clonenum_for_pktloss=2 if we need 3 duplicate packets (GETRES_LATEST/DELETED_SEQ_CASE1 set clonenum=1)
						* TODOTODO: If clone_field_list violates limitation, we should embed inswitch_hdr as DELREQ_INSWITCH_SEQ_CASE1
					+ Otherwise, set result_hdr.result=1, and send back DELRES directly/mirrorly
				- If valid=3
					+ If snapshot_flag=1, forward DELREQ_SEQ_CASE3 to server
					+ If snapshot_flag=0, forward DELREQ_SEQ to server
	+ Server
		* DELREQ_SEQ -> update KVS, and sendback DELRES
		* DELREQ_SEQ_CASE3 -> make in-memory snapshot, update KVS, and sendback DELRES
		* NOTE: as we do not maintain case3_reg in switch, we do not need to sendback DELRES_CASE3 w/ serveridx
- DELREQ_SEQ_CASE1
	+ Egress pipeline
		* Access lastclone_tbl to update is_lastclone_for_pktloss
		* If is_lastclone_for_pktloss = 0, decrease clonenum_for_pktloss, forward to reflector, and clone again
		* Otherwise, send back DELRES directly/mirrorly
- TODO: Treat DELREQ as a speical PUTREQ in server
	+ TODO: Check code of xindex_buffer that whether buffer directly frees atomic value instead of marking it as deleted
		* NOTE: If so, it may delete in-memory snapshot version for the key and hence give incorrect answer for SCANREQ
	+ TODO: Implement DELREQ by in-place update in xindex_util, xindex_group_impl (data), and xindex_buffer (buffer/temp_buffer) instead of being freed -> not need deleted_set
	+ TODO: For memory efficiency, during merge, if the latest status is deleted and the latest snapshot ID is older enough than the current snapshot ID (e.g., k-2 vs. k), we free the entry in the final merged data array
		* NOTE: original xindex can directly free the entry whose status is deleted during merge, while extended xindex cannot
			- If we find the entry whose status is deleted from old data/buffer, original xindex ignores the entry and processes request in temp_buffer; however, extended xindex directly processes request on the entry
			- If we find no corresponding entry from new merged data, both original and extended xindex process request in temp_buffer
			- Therefore, extended xindex could lose results of concurrent PUTREQ/DELREQ, which arrive at server before replacing data pointer as final merged one
		* TODO: We invoke an atomic primitive try_to_free (protected by mutex lock) for each entry in data/buffer
			- TODO: If latest status and snapshot ID suffice above requirement, set is_free = true
			- TODO: If is_free = true, the entry does not serve any request (including GETREQ, PUTREQ, DELREQ, SCANREQ)
				+ PUTREQ/DELREQ will be served by buffer/temp_buffer
				+ GETREQ/SCANREQ will read from buffer/temp_buffer
					* NOTE: if w/o new entry in buffer/temp_buffer, the result is the same as w/ an old entry whose status is deleted in data/buffer for both GETREQ (due to latest status = deleted) and SCANREQ (due to very small latest snapshot ID)
			- NOTE: if PUTREQ/DELREQ arrive before try_to_free, is_free must be false; otherwise, they must be served by buffer/temp_buffer
		* TODO: We free each entry in the final merged data array if try_to_free returns true (i.e., is_free = true)
- TODO: If cache crashes, server should reset all seq as zero after recovery such that switch-assigned seq (>=1) must be larger
- TODO: If with parser limitation, we can introduce hint of next header in the previous header
- TODO: Check APIs of register access 

## Implementation log

- Copy netreach-voting-v3-memory to netreach-v4
- Remove unnecessary files
	+ \*.bak and \*.custom files in tofino/
- Use int32_t instead of uint64_t for key, as switch needs key for cache lookup, and compare key for range partition (key.\*, parser.\*)
	+ NOTE: we convert each 4B from little-endian into big-endian when serializing, and convert back when deserializing; so when switch OS receives key from controller, ptf will automatically perform endian conversion without extra processing
	+ TODO: test to_model_key in xindex_model_impl.h
+ Use int8_t instead of uint8_t for optype (packet_format.\*, ycsb/parser.\*)
- Remove host-side hashidx (packet_format.\*)
- Implement GET (packet_format.\*, ycsb_remote_client.c, ycsb_server.c, \*.p4)
	+ Support GETREQ and GETRES in client, switch, and server
	+ Support GETREQ_NLATEST in switch and server 
	+ Support GETRES_LATEST_SEQ and GETRES_DELETED_SEQ in server and switch
	+ Add result header for GETERS, GETRES_LATEST_SEQ and GETRES_DELETED_SEQ
- Implement cache population
	+ Support GETREQ_POP in server
	+ Support CACHE_POP in server, controller, switch OS
		* controller.popclient
		* server.reflector
		* switchos.popworker
	+ Support CACHE_POP_INSWITCH in switchos, reflector, switch
	+ Support CACHE_POP_INSWITCH_ACK in switch, reflector, switchos 
- Implement cache eviction
	+ switchos: choose victim -> send CACHE_EVICT to controller -> wait for ACK -> remove cache_lookup_tbl -> cache population -> reset intermediate metadata for paramserver
	+ controller: forward CACHE_EVICT to corresponding server -> wait for ACK -> update metadata (no metadata now) -> send ACK to switchos
	+ server: store evicted data if necessary -> update cached keyset -> send back ACK
- Use int32_t for vallen (val.c, val.h)
- Reorganize socket API
- Implement for PUTREQ and DELREQ
- Implement crash-consistent snapshot
	+ Implement case1
		* Add inswitch_hdr to case1 in data plane for inswitch_hdr.idx
	+ Implement case3
	+ Implement case2

## Run

- Software
	+ Launch switchos in local control plane of Tofino
	+ Launch controller in end host
	+ Launch servers in end host

## Simple test

## Fixed issues

## Future work

- For PUTREQ_LARGE
	+ NOTE: PUTREQ_LARGE always reads savedseq, vallen1, and value1; while only evicts if valid=1 and latest=1
	+ NOTE: In-switch cache snapshot should only store the records with valid=1 and latest=1
		* If valid=0 or latest=0, server must have up-to-date record
		* Case 1 scenarios
			- valid=0 or latest=0 (not store) -> valid=1 and latest=1 (store): PUTREQ and DELREQ
			- valid=1 and latest=1 (store) -> valid=0 or latest=0 (not store): PUTREQ_LARGE_EVICT
			- valid=1 and latest=1 (w/ old deleted/value) -> valid=1 and latest=1 (w/ new deleted/value): PUTREQ and DELREQ
			- Therefore, PUTREQ and DELREQ always trigger case1, while PUTREQ_LARGE only triggers case1 if with eviction
	+ TODO: PUTREQ_LARGE_SEQ: op_hdr + seq + vallen + value (not parse vallen)
	+ TODO: PUTREQ_LARGE_EVICT_SEQ: op_hdr + vallen1 + value1 + status_hdr + savedseq + seq + vallen + value
		+ TODO: If with parser limitation, place seq and savedseq into status_hdr 
	+ TODO: PUTREQ_LARGE_EVICT_SEQ_CASE1: op_hdr + vallen1 + value1 + status_hdr + savedseq + seq + vallen + value
	+ TODO: We use clone_e2e to notify controller for evicted data or case 1
		+ TODO: PUTREQ_LARGE_EVICT_SEQ_CLONE: op_hdr + vallen1 + value1 + status_hdr + savedseq + seq + vallen + value
		+ TODO: PUTREQ_LARGE_EVICT_SEQ_CASE1_CLONE: op_hdr + vallen1 + value1 + status_hdr + savedseq + seq + vallen + value
	+ TODO: Correctly calculate udp_hdr.hdrlen
	+ TODO: Server-side processing
		+ TODO: For non-cloned packet, server only processes seq, vallen, and value
		+ TODO: For cloned packet (i.e., from controller), server only processes savedseq, vallen1, value1, and status_hdr
	+ TODO: Simulate a controller in one end-host (co-located with server yet not accessing server-side info)
		* TODO: For EVICT_CLONE, controller simply forwards it to server
		* TODO: For EVICT_CASE1_CLONE, controller remembers the special case (vallen1 + value1 + status_hdr) and forwards it to server
- Switch
	+ Egress pipeline
		* Stage 1 (4 ALU)
			- valid_tbl
				+ NOTE: eviction triggerred by PUTREQ_LARGE belongs to case 1 (w/o key change, w/ value change to an inalid status)
				+ NOTE: valid for both cache population/eviction atomicity and large PUT
					* TODO: valid=1 (valid: cached entry with all lookup tables being updated w/o large values in server)
					* TODO: valid=2 (temporarily invalid: cached entry with all lookup tables being updated w/ large values in server)
					* Data plane only changes valid between 1 and 2 and only considers valid=1/2/3
						- TODO: valid=2: put/del, snapshot only if packet loss
						- TODO: valid=3: cached get only if latest=1, put/del/largeput set latest=0 and forward to server (maybe case3)
					* Snapshot
						- TODO: For those with valid=2, controller remembers the records recently evicted by PUTREQ_LARGE; if server does not have a large value with larger seq (due to packet loss), place the records back into in-switch snapshot
				+ FUTURE: PUTREQ_LARGE_EVICT_CASE1 does not need to embed meta.valid as valid must be 1 (w/o eviction if invalid); but embed is also ok if limited by parser
			- seq (optype -> seq)
				+ TODO: we need seq mechaism to avoid unnecessary overwrite caused by eviction of PUTREQ_LARGE or cache eviction
		* Stage 2 (4 ALU)
			- latest_tbl, deleted_tbl (optype, is_cached, idx -> status)
				+ TODO: even with seq mechanism, we still need latest_tbl for response of GETRES_NLATEST/DELETED (and PUTREQ_LARGE_EVICT)
+ Server
	* GETREQ_POP
		- TODO: If key not exist or vallen > 128B, send back GETRES and ignore cache population
	* GETREQ_NLATEST
		- TODO: If key exists and vallen > 128B, send back GETRES
		- TODO: If key exists and vallen <= 128B, send back GETRES_LATEST
