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

## Overview

- Packet format
	+ Ethernet + IP + UDP
	+ Operation header: 1B optype, 16B key
	+ Value header: 4B vallen, variable length value (8B padding if vallen <= 128B)
	+ Result header: 1B result
		* NOTE: success flag for PUTRES/DELRES; deleted flag for GETRES; SCANRES does not need it
	+ Inswitch header: 1b is_cached, 1b is_sampled, 1b is_wrong_pipeline, 9 bit eport_for_res, 9b sid, 3b padding, 2B hashval, 2B idx
- Client
	+ Send GETREQ and wait for GETRES
- Switch
	+ Ingress pipeline
		* Stage 0
			- sid_tbl (optype, ingress port -> eport_for_res, sid)
				+ optype: GETREQ, PUTREQ, DELREQ
				+ sid: used by egress pipeline to generate response by packet mirroring
			- cache_lookup_tbl (key -> idx, is_cached)
				+ idx: index for in-switch cache (assigned by controller)
				+ NOTE: even if the packet is not FarReach packet, set inswitch_hdr does not affect their PHVs
			- hash_tbl (optype, key -> hashval)
				+ optype: GETREQ, PUTREQ, DELREQ
				- hash_for_partition_tbl (optype, key -> hash_partitionidx)
					+ Hash_partitionidx: partition across servers
				- hash_for_CM_tbl (optype, key -> hash_cmidx)
					+ hash_cmidx: index for CM
				- In implementation, we use hashval = hash_partitionidx = hash_cmidx to save PHV
			- sample_tbl (optype, key -> is_sampled)
				+ optype: GETREQ, PUTREQ, DELREQ
			- TODO: In distributed extension, spine/leaf switch uses idx to access the bucket in the corresponding virtual switch (controller has considered partition across virtual spine/leaf switches when assigning idx)
				+ TODO: Only leaf switch needs to perform hash_for_partition_tbl for partition across servers
				+ TODO; Only spine switch needs to assign seq
				+ NOTE: we enforce spine-leaf path for each key to ensure serializability and availability, which can be statically configured into switches (no hash operation in client); however, DistCache uses dynamic mechanism (power-of-two-choices) for load balance, client needs to perform hash-based sampling
					* TODO: In DistCache, client calculates a hash value to introduce sampling overhead
					* TODO: But not need to embed the sampling result; instead, we can use in-switch sample to choose spine or leaf;
			- NOTE: SCANREQ does not access all above tables
		* Stage 1
			- hash_partition_tbl or range_partition_tbl
				- Hash partition: (optype, hashval range, ingress port -> udp.dstPort, egress port, is_wrong_pipeline)
					+ optype: GETREQ, PUTREQ, DELREQ
					+ TODO: Check why we need the param of 0 for MAT with range matching
				- TODO: range partition: (optype, key range, ingress port -> udp.dstPort, egress port)
					+ optype: GETREQ, PUTREQ, DELREQ, SCANREQ
					+ TODO: We treat the most significant 4B of key as int32_t for range matching -> need conversion between little-endian and small-endian for the 4B of each key
				- NOTE: we use different udp.dstPort under the same egress port to simulate different egress ports
		* Stage 2: ig_port_forward_tbl (op_hdr.optype -> op_hdr.optype)
			- optype: GETREQ, PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ
			- Add inswitch_hdr to FarReach packet
		* Stage 3: ipv4_forward_tbl (optype, ingress port, dstip -> egress port)
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
		* Stage 1 (4 ALU)
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
				* TODOTODO: Snapshot
					- TODO: Controller atomically loads cached key
					- TODO: Controller only stores records with valid=1 and latest=1 into snapshot
					- TODO: For valid=0/3, the record is managed by controller
						+ valid=0: if being snapshoted, remove the newly populated record from snapshot if any
						+ valid=3: if being evicted, keep the original cached record in snapshot and remove the newly populated record from snapshot if any
					- TODO; GETRES_LATEST_SEQ and GETRES_DELETED_SEQ can also trigger case1
				+ TODO: PUTREQ_CASE1/DELREQ_CASE1 need to embed status_hdr.valid as valid could be 0
			- TODO: seq (optype -> seq)
				+ optype: PUTREQ_INSIWTCH, DELREQ_INSWITCH
				+ NOTE: SEQ_BUCKET_COUNT is independent with KV_BUCKET_COUNT
				+ NOTE: we need seq mechaism to avoid unnecessary overwrite caused by cache eviction
			- TODO: case1 (optype, is_cached, idx, is_snapshot -> is_case1)
		* Stage 2 (1 ALU)
			- latest_tbl (optype, is_cached, idx, valid -> is_latest)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH
				+ NOTE: latest=0 means no PUTREQ/DELREQ/GETRES_LATEST_SEQ/GETRES_DELETED_SEQ arrive at switch
				+ NOTE: even with seq mechanism, we still need latest_tbl for GETRES_LATEST_SEQ/GETRES_DELETED_SEQ 
		* Stage 3 (3 ALU)
			- deleted_tbl (optype, is_cached, idx, valid, is_latest -> is_deleted)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH
			- vallen (optype, is_cached, idx, valid, is_latest -> vallen_hdr)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH
				+ TODO: vallen and value can be placed from stage 1 to save # of stages if necessary
			- TODO: savedseq (optype, is_cached, idx, seq, valid, is_latest -> savedseq)
				+ TODO: optype: PUTREQ_INSIWTCH, DELREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH
				+ NOTE: we assume that there is no reordering between spine and leaf switch due to a single FIFO channel
		* Stage 4-9 (4 ALU)
			- From vallo1 to valhi12 (optype, is_cached, idx, valid, is_latest -> val_hdr)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH
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
	+ Each server maintains a set of keys being cached to avoid duplicate population
		* If GETREQ_POP triggers a cache population (i.e., key exists), server adds the key into cached key set
		* TODO: If server receives CACHE_EVICT, it removes the evicted key from cached key set
- NOTE: In the host colocated with server
	+ TODO: We use controller thread to simulate controller for cache management
	+ TODO: We use reflector thread to simulate the extra link for connection between data plane and switch OS
- Controller
	+ Cache population/eviction
		* Receive CACHE_POP <key, value, stat, seq, serveridx> from server by tcp channel
			- Per-server popclient -> one controller.popserver (with multiple subthreads = # of servers)
		* CANCELED: Add key into per-server cached key set (comment it if server.cached_keyset works well)
		* CANCELED: Add key into key-server map (comment it as CACHE_EVICT embeds serveridx towards corresponding server)
		* Maintain a map between serveridx and controller.popserver.subthreadidx
		* Send CACHE_POP to corresponding switchOS
	+ Eviction handler
		* TODO: Receive CACHE_EVICT <victim.key, vicktim.value, victim.result, victim.seq, victim.serveridx>
		* TODO: Check per-server cached key set to find the corresponding server
		* TODO: Send CACHE_EVICT to the correpsonding server, and wait for CACHE_EVICT_ACK <victim.key>
		* TODO: Send CACHE_EVICT_ACK to the switch OS
- Switch OS
	+ Cache population/eviction
		* Cache update thread (switchos.popworker): perform cache population/eviction
		* Maintain in-memory multi-level array: switch (TODO: different switches under distributed extension) -> egress pipeline (fixed due to testbed limitation) -> <idx, key>
		* TODO: Maintain a paramserver to pass parameters to ptf framework
		* Receive a CACHE_POP from controller -> check whether there exists free idx to assign
			- One controller.popclient -> one switchos.popserver
			- CANCELED: Add key into cached key set (comment it if server.cached_keyset works well)
			- If with free idx (cache population)
				+ Set valid[idx] = 0 for atomicity
				+ TODO: Send CACHE_POP_INSWITCH <key, value, seq, inswitch_hdr.idx> to data plane, and wait for CACHE_POP_INSWITCH_ACK
				+ Data plane (for the given idx)
					* TODO: Reset cache_frequency=0, latest=0, deleted=0
					* TODO: Update vallen, savedseq, value
					* TODO: Send CACHE_POP_INSWITCH_ACK <key> to switch OS
					* NOTE: valid is reset by switch OS; case1 is reset by snapshot thread;
				+ TODO: Add a new entry <key, idx> into cache_lookup_tbl of all ingress pipelines (must by ptf)
				+ TODO: Set valid[idx] = 1 to enable the cache entry
			- Otherwise (cache eviction)
				+ TODO: Sample idxes and load corresponding cache_frequency counters
				+ TODO: Choose the idx with the minimum frequency as the victim (approximate LRF)	
				+ TODO: Set valid[victim.idx] = 3 for atomicity (then only latest can be changed by data plane)
				+ TODO: Load deleted, vallen, val, and savedseq of victim
				+ TODO: Report CACHE_EVICT <victim.key, vicktim.value, victim.result, victim.seq, victim.serveridx> to controller, and wait for CACHE_EVICT_ACK
					* NOTE: we do not need to load latest
						- Even if latest=0, the value could still be latest <- PUT/DEL (lost later) w/ valid=3 resets latest from 1 to 0
						- No matter value is latest or not, we can always compare savedseq with server.seq for availability
					* NOTE: CACHE_EVICT reported by switch OS instead of data plane -> no need <key, value, seq, inswitch_hdr.is_deleted>
				+ TODO: Remove existing entry <victim.key, victim.idx> from cache_lookup_tbl of all ingress pipelines
				+ TODO: Invoke cache population for new CACHE_POP
	+ Snapshot
		* TODO: Snapshot thread: perform crash-consistent snapshot
		* TODO: Maintain an in-memory snapshot flag
		* TODO: If with ptf session limitation, we can place snapshot flag in SRAM; load values and reset registers by data plane;
	+ TODO: Periodically reset CM
		* TODO: If with ptf session limitation, we can reset it in data plane

## Details 

- NOTE: For each RES directly answered by switch
	+ If is_wrong_pipeline=0, set eg_intr_md.egress_port as inswitch_hdr.eport_for_res
	+ Otherwise, use packet mirroring (drop original packet + clone_e2e to corresponding egress port by inswitch_hdr.sid)
- NOTE: For each RES from server
	+ See ipv4_forward_tbl for details
- GETREQ
	+ Client sends GETREQ
	+ Ingress: GETREQ -> GETREQ_INSWITCH (is_sampled, is_cached, is_wrong_pipeline, sid, hashval, idx)
	+ Egress
		* Stage 0: update CM if inswitch_hdr.is_sampled=1 and inswitch_hdr.is_cached=0; update CM
		* Stage 1: update is_hot; update cache_frequency if inswitch_hdr.is_sampled=1 and inswitch_hdr.is_cached=1; get valid
		* Stage 2: get latest and deleted
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
		* If inswitch_hdr.is_cached=1 and inswitch_hdr.valid=1 and latest=0
			+ Set latest=1, delete=0, update savedseq (TODO), update vallen and value
		* Drop original packet, forwrad cloned packet (i2e) to client
			+ TODO: Check if we need to set eg_intr_md.egress_port for cloned packet (i2e)
- GETRES_DELETED_SEQ
	+ Server sends GETRES_DELETED_SEQ
	+ Ingress: 
		* GETRES_DELETED_SEQ -> GETRES_DELETED_SEQ_INSWITCH (is_cached, idx)
		* ipv4_forward_tbl -> forward GETRES_DELETED_SEQ_INSWITCH to pipeline of ingress port; clone GETRES_DELETED_SEQ to pipeline of client for GETRES 
	+ Egress
		* If inswitch_hdr.is_cached=1 and inswitch_hdr.valid=1 and latest=0
			+ Set latest=1, deleted=1, update savedseq (TODO), update vallen and value
		* Drop original packet, forwrad cloned packet (i2e) to client
			+ TODO: Check if we need to set eg_intr_md.egress_port for cloned packet (i2e)
- TODO: Cache population also updates savedseq
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
	+ Support CACHE_POP in server, controller, TODO: switch OS
		* TODO: controller.popclient
		* TODO: server.reflector
		* TODO: switchos.popworker

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
				+ FUTURE: PUTREQ_LARGE_EVICT_CASE1 does not need to embed status_hdr.valid as valid must be 1 (w/o eviction if invalid); but embed is also ok if limited by parser
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
