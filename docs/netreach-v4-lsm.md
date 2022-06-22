# Tofino-based NetREACH + LSM-based KVS (rocksdb) + variable-length key-value pair w/ snapshot + control-plane-based cache update (netreach-v4-lsm)

- Copy from netreach-v4-xindex -> keep the same in-switch implementation, and only change server-side implementation
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
- NOTEs
	+ Egress pipeline recirculation issue is ok
	+ We provide weak-form durability -> data is durable before a certain snapshot timepoint -> bounded-error reliability
	+ Limited recirculation for atomicity in snapshot is ok
- NOTE for xindex
	+ For a record in data, PUT (in-place update) -> DEL (mark data.record as removed) -> PUT (insert buffer.record) -> DEL (mark buffer.record as removed)
		* If w/o compact: -> PUT (replace buffer.record w/ a new one)
		* If w/ compact: -> PUT (insert temp_buffer.record) -> DEL (mark temp_buffer.record as removed) -> PUT (replace temp_buffer.record w/ a new one)
	+ As xindex does not maintain entry of deleted record in merged data, it does not allow in-place update for DELREQ in data and buffer during compaction
	+ After treating DELREQ as a speical PUTREQ
		* TODO: If the record is not old enough. allow in-place update for removed record in data/buffer/temp_buffer
		* TODO: Otherwise, cope with removed record as in original xindex
		* TODO: Range query should consider removed records
- NOTE for multi-thread programming
	+ Cache coherence among CPU caches is fixed by MESI protocol -> we treat CPU caches and memory as a consistent whole
	+ For cache consistency between CPU register and whole memory
		* volatile: disable reordering; disable register cache (both LOAD/STORE) for the specified variables
		* memory fence: disable reordering; disable register cache by re-loading all registers before LOAD and flushing to memory after STORE 
	+ For atomicity/serializability, we can use locking. copy-on-write w/ RCU, and version check
- NOTE for PHV allocation
	+ Containers accessed by one ALU must be allocated in the same group
		* One container can accommodate arbitrary fields from the same packet header
		* One field can only be split into two containers	
	+ In each stage, one container can only be accessed once by an ALU (read-read does not incur confliction, yet write-write or read-write incurs)
		* Even if two fields do not have dependency, if they are located in a same container, they cannot be accessed by different ALUs in the same stage
	+ The above contraints are only for ALU instead of MAU -> MAU can match containers in different groups, each can be matched with multiple times
	+ Unextracted fields (e.g., metadata) do not need 8-bit alignment in PHV
- NOTE for parser
	+ The total select length of each packet header (not each single field) for select/switch expression after being extracted is limited (<=4)
		* select length: # of selects between the first select of the packet header and the last one; for example:
		* select(optype) -> extract(vallen) -> extract(value) -> select(optype) -> select(optype) -> select(optype): select length = 4 -> OK
		* select(optype) -> select(vallen) -> extract(value) -> select(optype) -> select(optype) -> select(optype): select length = 5 -> OK
	+ No limitation on # of selects and branches
- NOTE for signedness
	+ Tofino uses unsigned and non-saturating (wrap for overflow/underflow) for PHV and stateful regs
	+ We can set signed and saturating (fixed at maximum/minimum) if necessary; for example
		* For a field of packet header, op_hdr.optype: 8 (signed, saturating)
		* FOr a reg, attributes: signed, saturating
	+ Ptf uses Thrift to configure data plane
		* Thrift only supports int8 (<=8b), int16 (<=16b), int32 (<=32b), and string (>32b) -> we eventually focus on the bytes
		* The signedness of the bytes passed by Thrift is determined by the target (field/reg) in data plane
		* For example, ptf passes ipv4 address as int32 to data plane, which yet treats the bytes as uint32
- NOTE for python search path
	+ If use interative python shell -> current shell execution path
	+ If directly run the script -> script file path
	+ If use python -m package.module -> all package paths 
	+ Example: tmp/shell.py (shell path) -> tmp/a/helper.py -> tmp/a/b/main.py
		* To support ..helper in main.py, we must know a.helper and a.b.main (both helper and a&b relationship <-> path of tmp)
			- python a/b/main.py -> only know main -> not support
			- python a/b/main.py + manually append path of a into sys.path -> only know helper and b.main -> not support
			- (1) python a/b/main.py + manually append path of tmp into sys.path; (2) python shell.py; (3) python -m a.b.main
				+ -> know a.helper and a.b.main -> support!
- NOTE for socket timeout
	+ select() is used to set timeout for all output functions, e.g., connect and send
	+ setsockopt() for SO_RCVTIMEO is used to set timeout for all input functions, e.g., accept and receive
- NOTE for parser/deparser
	+ We must invoke add_header() explicitly for each new header even if the parser/deparser has indicated that we need the new header; otherwise, the new header will not be embedded into the packet
- NOTE for range match
	+ The maximum length of field for range matching is 20 bits
- NOTE for counter
	+ We can use direct/indirect counter for debugging, which does not occupy stateful ALU
- NOTE for mirroring
	+ We need to create mirror_session in ptf for both ingress direction and egress direction explicitly
	+ NOTE: EGRESS will replace INGRESS, so we need to use BOTH
- NOTE for MAU limitation
	+ # of MAUs in each physical stage is limited (aka same stage in ingress pipeline + egress pipeline)
		* NOTE: calculating hash value is MAU-consuming
		* For example, calculating 4 hash values in ingress stage 2 consumes most MAUs -> no MAU for some MAT in egress stage 2
	+ Similarly, # of stateful ALUs is also limited by physical stage
	+ Each physical stage has two PHV copies for ingress/egress -> same stage in ingress/egress does not have PHV confliction
- NOTE: there are only two ways to test system max throughput (especially for dynamic workload)
	+ Normalized throughput: system throughput / bottlenect server throughput
		* It assumes that as input traffic scales out, cache hit rate and server load ratio does not change, which should be correct and not related with queuing occupancy
	+ Emulated throughput: use multi-thread to simulate multiple servers
		* NOTE: server rotation/emulation in essence has no difference with normalized throughput
			- For server emulation, we need to saturate the bottleneck server thread, i.e., tune client sending rate to make the bottleneck server thread have the same throughput w/ cache as that w/o cache
			- For server rotation, we need to tune client sending rate to saturate the bottleneck physical server
			- All (server rotation/emulation and normalize throughput) are used to evaluate load balance ratio / system maximum throughput under large enough input traffic (different baselines can use different input traffic as long as they can saturate the system)
		* Cannot represent practical result even if we use sufficient client threads to saturate server threads?
			- TODO: need to be confirmed by experiment on emulated thpt
			- If we use multiple client/server threads to listen on the same NIC, linux socket overhead (100~200us) could be dominant -> even if we reduce server-side overhead, linux socket overhead in client is still large and our throughput improvement is limited
				+ In practical case, each physical client has an individual NIC -> no such socket overhead
			- If we use one thread to listen on the NIC and distribute received packets to different server threads by message queues (as in NetCache), we still have only one NIC to receive packets
				+ In practical case, all NICs of different servers can receive packets simultaneously; and w/o message queue overhead
- NOTE: even if we set NODELAY for tcp socket, it will not send packet immediately before send() returns
	+ Linux socket uses shallow copy for send(), as tcp socket does not send packet immediately, if we change userspace buf before it sends out the packet, it will trigger misbehavior
	+ For example, we send key A for cache population, and then send key B; as A is not sent out before we serialize B into buf, we see two keys of B in switchos for cache population -> duplicate cache population issue
- NOTE: for system throughput
	+ Max system throughput is limited by the min component throughput
		* For each component, throughput is related with its latency
		* Multi-thread increases CPU throughput, which improves system throughput only if CPU is the bottleneck of single thread
	+ Load balance affects system throughput only if each server has the same max throughput
		* For example, given a thread to simulate each server, if CPU is the bottleneck componenet, each thread with a new CPU core has the same max thpt, which can reflect load ratio
			+ NOTE: If # of threads exceeds CPU core number, adding thread will not increase system throughput
			+ NOTE: If bottleneck componenet changes from CPU to others such as disk after launching sufficient threads, adding thread will not increase system throughput
			+ Under the above two cases, each thread does not have the same max throughput -> load ratio does not affect system throughput
	+ Two ways to test system max throughput
		* Like ycsb: each client follows request-response-request + sufficient client threads
		* Like pkt generator: each client continuously send large # of requests -> pkt loss due to queue overflow

## Overview

- Packet format
	+ Ethernet + IP + UDP
	+ Operation header: 1B optype, 16B key
	+ Value header: 4B vallen, variable length value (8B padding if vallen <= 128B)
	+ Result header: 1B result
		* NOTE: success flag for PUTRES/DELRES; deleted flag for GETRES; SCANRES does not need it
	+ Inswitch header: 1b is_cached, 1b is_sampled, 1b is_wrong_pipeline, 9 bit eport_for_res, 9b sid, 3b padding, 2B hashval, 2B idx
	+ CACHE_POP: op_hdr(optype+key), value, seq, serveridx
	+ CACHE_POP_INSWITCH: op_hdr(optype+key), value, seq, inswitch_hdr.freeidx
	+ CACHE_EVICT: op_hdr(optype+key),, value, seq, result, serveridx
	+ CACHE_EVICT_ACK: op_hdr(optype+key)
	+ CASE1: op_hdr(optype+key), value, seq, inswitch_hdr, result
	+ SCANREQ: op_hdr(optype+key), endkey
	+ SCANREQ_SPLIT: op_hdr(optype+key), endkey, split_hdr(cur_scanidx, max_scannum)
- Client
	+ Send GETREQ and wait for GETRES
	+ Range query
		* Send SCANREQ <optype, key, endkey>
		* Wait for all SCANRES_SPLIT <optype, key, endkey, cur_scanidx, max_scannum, pairnum, key-value pairs>
		* NOTE: cur_scanidx are different (e.g., 1/2/3) under max_scannum (e.g., 3)
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
			- Partition for normal requests
				- hash_for_partition_tbl (optype, key -> hashval_for_partition)
					+ optype: GETREQ, PUTREQ, DELREQ, CACHE_POP_INSWITCH
				- range_partition_tbl: (optype, key range, ingress port -> udp.dstPort, egress port, is_wrong_pipeline)
					+ optype: GETREQ, PUTREQ, DELREQ, CACHE_POP_INSWITCH
					+ NOTE: we treat the most significant 4B of key (keyhihi) as int32_t for range matching -> need conversion between little-endian and small-endian for each 4B of a key (note that we deploy key by ptf)
					+ TODO: Check why we need the param of 0 for MAT with range matching
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
			- Partition for normal requests
				- hash_partition_tbl: (optype, hashval_for_partition range, ingress port -> udp.dstPort, egress port, is_wrong_pipeline)
					+ optype: GETREQ, PUTREQ, DELREQ
					+ NOTE: PARTITION_COUNT (for consistent hashing) is independent with KV_BUCKET_COUNT
					+ TODO: Check why we need the param of 0 for MAT with range matching
				- range_partition_for_scan_tbl: (optype, startkey range, endkey range -> udp.dstPort, egress port, max_scannum)
					+ optype: SCANREQ
					+ start_key range -> udpport and eport (indicate first server); startkey range + endkey range -> max_scannum
					+ TODO: Check why we need the param of 0 for MAT with range matching
				- NOTE: we use different udp.dstPort under the same egress port to simulate different egress ports
		* Stage 3 (need_recirculate == 0)
			- ig_port_forward_tbl (op_hdr.optype -> op_hdr.optype)
				+ optype: GETREQ, PUTREQ, DELREQ, GETRES_LATEST_SEQ, GETRES_DELETED_SEQ
					* Add inswitch_hdr to FarReach packet
				+ optype: SCANREQ
					* Update SCANREQ as SCANREQ_SPLIT
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
			- process_scanreq_split_tbl (optype, udphdr.dstPort -> egress port, sid)
				+ optype: SCANREQ_SPLIT
				+ udphdr.dstPort -> serveridx -> egress port and sid
				+ meta.remain_scannum = split_hdr.max_scannum - split_hdr.cur_scanidx
				+ NOTE: if max_scannum=3, cur_scanidx = 0 here -> remain_scannum = 3
			- process_cloned_scanreq_split_tbl (optype, udphdr.dstPort -> dstPort, egress port, sid)
				+ optype: cloned SCANREQ_SPLIT
				+ udphdr.dstPort = dstPort + 1
				+ udphdr.dstPort+1 -> serveridx -> egress port and sid
				+ meta.remain_scannum = split_hdr.max_scannum - split_hdr.cur_scanidx
				+ NOTE: if max_scannum=3, cur_scanidx = 1/2 here -> remain_scannum = 2/1
		* Stage 1 (3 ALU)
			- is_last_scansplit_tbl (optype, remain_scannum -> is_last_scansplit)
				+ optype: SCANREQ_SPLIT
				+ If remain_scannum=1, set is_last_scansplit=1; otherwise (default action), set is_last_scansplit=0
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
			- eg_port_forward_tbl (optype, is_cached, is_hot, is_valid, is_latest, is_deleted, is_wrong_pipeline, is_last_scansplit -> optype)
				+ optype: GETREQ_INSWITCH, PUTREQ_INSIWTCH, DELREQ_INSWITCH, GETRES_LATEST_SEQ_INSWITCH, GETRES_LATEST_SEQ
				+ TODO: IMPORTANT: output to optype2 to break dependency if necessary
					* GUESS: if w/ read-write or write-write contention in action, we need two stages; if w/ read in match and write in action, we can place into the same stage (match will read the value before write in action)
				+ TODO: If TCAM of single stage is not enough, we can assume the value of each matched key field is initialized
			- scan_forward_tbl (optype, is_last_scansplit -> cur_scanidx)
				+ optype: SCANREQ_SPLIT
					* If is_last_scansplit=1, cur_scanidx+=1, forward SCANREQ_SPLIT
					* Otherwise (default action), cur_scanidx+=1, forward SCANREQ_SPLIT, and clone_e2e(sid)
					* NOTE: if max_scannum=3, cur_scanidx = 1/2/3 here
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
	+ Snapshot
		* Make server-side snapshot if necessary
			- optype: PUTREQ_SEQ_CASE3, DELREQ_SEQ_CASE3, CACHE_EVICT_CASE2, and SNAPSHOT_SERVERSIDE from controller.snapshotclient
		* server.consnapshotserver receives snapshot data -> distribute data among servers with RCU mechanism for range query
			* TODO: deduplicate during range query
			* TODO: Treat DELREQ as a special write request -> do not ignore and free deleted atomic value, and remove deleted set (extended_xindex_dynamic_seq_del) -> range query needs to get value+deleted+seq for seq comparison with inswitch_results
			* TODO: Provide getseq API for seq comparison
	+ Range query
		* Pre-calculate min_startkey and max_endkey for each server
		* For each SCANREQ_SPLIT
			- Get verified key range
			- Get results of in-memory snapshot and in-switch snapshot for verified key range
			- Merge sort w/ seq comparison -> sendback SCANRES_SPLIT
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
				+ TODO: controller enforces other spine switches echo each PUTREQ/DELREQ by the same spine switch A
				+ TODO: Think about whether we need to report case2 for GETRES_LATEST/DELETED_SEQ?
					* If so controller needs to enforce all leaf switches echo each GETRES_LATEST/DELETED_SEQ to the same spine switch A
					* Maybe acceptable, as we target on write-intensive workloads + GETRES_LATSET/DELETED_SEQ is only triggered by the first packets of newly populated records + limited time (3 RTTs: enable echo -> set snapshot_flags -> disable echo)
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
		* Intermediate stages: reset_and_get vallen = 0 (save bandwidth), reset_and_get value
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
- Check APIs of register access; check parameter 0 of MAT configuration w/ range matched fields 

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
- Compile in C
	+ Implement concurrent map and fix volatile issue
- Compile in Tofino
	* Fix PHV allocation issue (using inswitch_hdr.sid in MAU instead of ALU)
		- If clone_e2e(inswitch_hdr.sid), inswitch_hdr.sid is used in the same ALU of eg_intr_md.mirror_id, which must be placed in the same PHV group; as inswitch_hdr.sid must be placed in the same PHV container with other inswitch_hdr fields, all of them cannot be placed in the same PHV group of eg_intr_md
		- Instead, we match inswitch_hdr.sid in MAT reads (i.e., used by MAU) and pass it into the action as an action paramter, as it is just used by MAU instead of ALU, it does not need to be placed in the same PHV of eg_intr_md
			+ NOTE: MAU can match different PHV containers from different PHV groups
	* Fix parser issue: parse_vallen cannot use switch expression due to the limited chain length of the same packet header
		- GUESS1: maybe due to limited # of selects -> reduce selects of ethernet and ipv4 -> FAIL
		- GUESS2: maybe due to limited # of branches -> reduce branches of vallen by byte alignment and those of optype by careful coding -> FAIL
			+ update optype in configure.py and packet_format.h
		- GUESS3: maybe due to limited length of chain of the same packet field/header
			+ Original parser: optype -> vallen -> optype -> optype -> optype (length > 4)
			+ (1) Not select vallen, directly return parse_val_len16 -> length = 4 -> SUCCESS
				* NODO: pad (not align) small value (vallen <= 128B) to 128 bytes in client side and use vallen to identify the real bytes in server side
			+ (2) Introduce op_hdr.optype2 -> limited length of chain of the same packet header -> FAIL
			+ (3) Introduce shadowtype_hdr.shadowtype (only except SCANREQ/SCANREQ_SPLIT/SCANRES_SPLIT and CACHE_POP/CACHE_EVICT and ophdr-only packets) -> SUCCESS
				* Introduce shadowtype in packet_format.h
				* Keep shadowtype the same as optype in p4
	* Fix placement issue of cache_lookup_tbl and hash_for_seq_tbl -> check visulatization -> update p4src/ingress.p4 and configure.py
	* Fix signedness issue (Tofino uses unsigned PHV/reg by default)
		- Consider
			+ C: key, val, prepare, packet_format, localtest, ycsb/parser, ycsb_remote_client, ycsb_server, server_impl, controller, switchos, common_impl, reflector_impl, snapshot_record, special_case
			+ P4: remove_cache_lookup, add_cache_lookup, configure, load_snapshot_data, get_evictdata, set_valid0
			+ XIndex: xindex, xindex_buffer. xindex_root, xindex_group, xindex_util
		- Use uint8_t for optype (packet_format, ycsb/parser, ycsb_remote_client, ycsb_server, server_impl, controller, switchos)
		- Use uint32_t for key (key, prepare, ycsb/parser, recb_remote_client, server_impl, common_impl, remove_cache_lookup, add_cache_lookup, configure)
		- Use uint32_t for vallen (val, ycsb/parser, server_impl, controller, switchos, load_snapshot_data, get_evictdata)
		- Use uint32_t for seq (packet_format, localtest, server_impl, controller, switchos, load_snapshot_data, get_evictdata, snapshot_record, special_case, xindex, xindex_buffer, xindex_root, xindex_group, xindex_util)
		- Use uint16_t for inswitch_hdr.idx (packet_format, switchos, remove_cache_lookup, add_cache_lookup, get_evictdata, set_valid0)
		- stat (bool): not need to change
		- Use uint16_t for split_hdr.cur_scanidx/max_scannum (packet_format, ycsb_remote_client)
		- Use uint16_t for serveridx/thread_id/worker_id (packet_format, localtest, ycsb_remote_client, ycsb_server, server_impl, controller, switchos)
		- NOTE: Use 0 instead of -1 to initialize (ycsb_remote_client, server_impl, controller, switchos, common_impl)
	* Fix syntax issue in ptf
	* Fix signedness issue in ptf.Thrift
	* Fix field length limitation in P4
		- Use 16-bit keyhihilo and 116-bit keyhihihi for 32-bit keyhihi (ingress_mat.p4, header.p4, remove_cache_lookup.py, add_cache_lookup.py, configure.py, key, server_impl.h)
		- Use uint16_t instead of uint32_t for vallen (egress_mat.p4 (udp.hdrlen), header.p4, netbufferv4.p4 (one comment), configure.py (udp.hdrlen), load_snapshot_data.py, get_evictdata.py, val, switchos.c, controller.c, iniparser_wrapper.c, packet_format_impl.h (size))
- Test corrcetness of normal requests
	* Fix software bugs when launching controller, switchos, and server successfully
		- Use new to allocate large buf in heap instead of stack to fix segmentation error
		- Invoke parse_control_ini in each main file
		- Fix configuration issue of controller_ip_for_server (should not be dpdk NIC)
		- Fix EAGAIN issue of tcpsocket after accept()
	* Pass correctness test for normal requests w/o cache hit (client -> server -> client)
		- Pass case1 of single GETREQ
			- Introduce debug_hdr for debugging (TODO: remove it due to direct counter for debugging)
				+ Move meta.is_hot/is_lastclone_for_pktloss into debug_hdr (*.p4, configure.py)
				+ Add debug_hdr at the end of each packet (only except SCANREQ/SCANREQ_SPLIT and CACHE_POP/CACHE_EVICT) (header.p4, parser.p4, egress.p4, configure.py, packet_format, switchos.c, controller.c)
			- Fix unmatched action (GETREQ_INSWITCH -> GETREQ) issue due to w/o add_header(shadowtype_hdr)
		- Pass case2 and case3 of PUTREQ (comment cache population temporarily)
			- Fix width limitation of range matched field (<= 20bits)
			- Fix incorrect seq issue due to unchanged udphdr.hdrLen
		- Pass case4 and case5 of DELREQ
	* Use DPDK flow director instead of custom receiver
- Test correctness of cache population/eviction
	* Update reflector_port in P4
	* Fix udp.dstport issue for cache hit in P4
	* Issue: server cannot receive CACHE_POP_INSWITCH_ACK
		+ Update destination macaddr for cache hit, cache pop ack, and case1
		+ Introduce direct counter for debugging
	- Issue: server cannot receive CACHE_POP_INSWITCH_ACK
		+ Update destination macaddr for cache hit, cache pop ack, and case1
		+ Introduce direct counter for debugging
		+ Update configuration to enable egress-direction mirroring
	- Issue: meta.clonenum_for_pktloss in clone field list is ignored by Tofino due to unclear reason
		+ Distinguish client_sid, server_sid, and switchos_sid
			* Use inswitch_hdr.client_sid for response of cache hit; use client_sid as action parameter in eg_port_forward_tbl (header.p4, netbufferv4.p4, ingress.p4, egress.p4, configure.py)
			* Use meta.server_sid for scanreq_split; use server_sid as action parameter in process_scanreq_tbl, process_cloned_scanreq_split, and scan_forward_tbl (egress.p4, configure.py)
			* Use switchos_sid as action parameter in eg_port_forward_tbl
		+ Introduce clone_hdr including clonenum_for_pktloss and client_port (header.p4, parser.p4, egress.p4, configure.py, packet_format)
	- Issue: i40e driver still returns multiple packets under a receive burst even if nb_pkts=1 after setting RTE_I40E_DESCS_PER_LOOP=1 (due to vector instruction?)
		+ Use RX_BURST_SIZE instead of 1 for server, reflector, and SCANRES, where we may receive multiple packets once a time
	- Fix python.struct pack/unpack issue (only the first character of format string can be used as byte order)
	- Issue: incorrect val15 and val16 due to optype changed by eg_port_forward_tbl
		+ Update meta.access_val_mode if we need to access vallen; use access_val_mode w/o optype to access val_reg; introduce drop_tbl to place drop() in the last stage -> run cachepop.case1 and check val16_reg, and then PUTREQ w/ cache hit and check val16_reg
	- Pass case1, case2, case3
		+ Fix cache eviction issue
- Test correctness of other features
	* Test conservative read
		- Pass case1, case2
	* Test cache hit
		- Pass case1, case2, case3, case4
		- Add add_and_remove_value_header_tbl in stage 11 for correct deparse -> run cachehit.case2 and check pktsize
	* Test crash-consistent snapshot
		- case3, case2, case1
			+ Fix concurrent map issue: concurrent_val::read does not return in while loop
	* Remove debug_hdr
- Reduce delay of cache population/eviction and snapshot
	* Check whether we can maintain two persistent ptf sessions
		- Implement ptf.popserver by persistent ptf session to reduce session create/destroy overhead
		- Implement ptf.snapshotserver by persistent ptf session to reduce session create/destroy overhead
	* Re-test cache population/eviction and snapshot
	* Send END to ptf.popserver and ptf.snapshotserver
* Test multi-clients and multi-servers
* Test latency
	- Try to fix latency issue (may be caused by dpdk issue or transmission issue)
		+ Try rte_rdtsc -> result not change
		+ Change dpdk settings
			+ Disable promisc mode -> result not change
			+ Allocate exclusive mempool for RX quque -> result not change
			+ Allocate more mbuf structures for TX_RING and RX_RING -> result not change
		+ Test Tofino latency by dpdktest-tofino -> 0xa84b - 0xa750 < 256ns
		+ Implement socktest_client, socktest_server, socktest-tofino to check latency difference between w/o hit and w/ hit
			* DPDK w/ key-value pair: client-server of 28us; client-switch of 23us
			* socket w/ payloadvalue: client-server of 42us; client-switch of 14us
			* DPDK w/ payloadvalue: client-server of 28us; client-switch of 23us
		+ Check DPDK configuration -> FAIL
			* Allocate huge page in exclusive memory for each numa node -> no change
			* Enable jumbo frame offloading -> no change
			* Set rx_thresh to setup rx_queue -> no change
* Pass P4 compilation w/ maximum # of record entries
* Support general KVS
	- Our main contribution is to avoid server-side overhead for PUT request, instead of reducing transmission latency for GET request; and we should target general KVS
		+ Try to test RTT latency for GET/PUT/DEL request in small-scale in-memory KVS
			* same key w/o cache hit
				- GET: 28us; PUT: 29us; DEL: 28us
			* same key w/ cache hit
				- GET: 23us; PUT: 23us, DEL: 23us
			* same key in practice (cache population delay)
				- GET: 23us; PUT: 22us; DEL: 22us
		+ Deprecated: Try large-scale in-memory KVS and test latency
			* Enable swap space to avoid being killed by OOM killer
			* Use localtest to get larger server-side latency first
				- w/o using swap space: ~2us
				- w/ using swap space: 10-500us
			* NOTE: we should not use xindex
				- xindex trades memory for query time -> memory-consuming (such as model parameters and concurrency control metadata)
					+ Original xindex (16B key + 8B value + w/o snapshot): 50M records (~1G data) -> 10G memory cost
					+ Extened xindex (16B key + 128B value + w/ snapshot): 50M records (~1G data) -> 60G memory cost
				- If we introduce swap space to increase server-side overhead, xindex cannot support large # of records -> we cannot argue that we target large-scale in-memory KVS
				- We only require that server-side KVS can support snapshot and range query
					+ LSM tree, e.g., rocksdb, can support both snapshot and range query, which should be our server-side KVS
* Use rocksdb instead of xindex as our server-side KVS
	* Implement netreach-v4-lsm based on netreach-v4-xindex
		- Implement key::from/to_string_for_rocksdb to match default rocksdb::ByteWiseComparator (aka Slice::compare)
		- Implement val::to_string_for_rocksdb to concatenate val and seq
		- Implement RocksdbWrapper to support get/put/delete/scan with seq
		- Update localtest
			+ 10M items: server-side overhead of GET/PUT/DEL = ~50us
		- Update split_workload and ycsb_loader for loading phase
		- Update ycsb_server, server_impl, controller for transaction phase
	* Test latency
		- Optimize loading phase
			+ Use mmap+strchr to getline from memory in ycsb/parser to reduce disk overload of loading workload file -> implement next_batch
			+ Commit a batch of key-value pairs in one transaction in force_multiput
			+ Use multi-thread to invoke force_put for parallel loading
			+ Update split_workload and ycsb_loader: workloadname-load-servernum/serveridx-loaderidx.out
		- Same key
			- server stores 1M/10M records, server-side latency of GET/PUT is always ~10us
				+ RTT w/o cache hit: 32us; RTT w/ cache hit: 23us	
			- Reason: GET on the same key always hit in block cache; PUT on the same key always hit in memtable
		- Fix latency issue under YCSB workload
			+ Check skewness -> 1% key frequency > 100; sum frequency = ~35%
		- Write-intensive skewed workload
			- NOTE: reset database before each experiment
			- server stores 1M records
				+ RTT w/o inswitch cache: us (server-side: 43us); RTT w/ inswitch cache: 52us (server-side: 45us);
				+ 15% cache hit -> expected latency: 51us
- Replace DPDK with UDP socket
	* Add udphdr.checksum into P4 and compile
	* Try to hide server.srcport in switch and compile
		- Ingress
			+ udphdr: change dstport for all REQ
			+ RES: change eport for dstip for forwarding
		- Egress
			+ udphdr: change dstport for SCANREQ_SPLIT; change srcport and dstport for cache hit; change dstport for switch notification
			+ RES: change udp.hdrlen; change macaddr
		- change update_macaddr_tbl into update_ipmac_srcport_tbl to hide server-side partition details
			+ NOTE: for dstport, REQ is update by partition; RES is update by server or switch (swapped with srcport); notification is update by switch (action parameter)
			+ Although srcport of RES under cahce hit has been set by switch in eg_port_Forward_tbl, we still update srcport for RES here for those issued by server
		- NOTE: udpserver only cares about dstmac (NIC) and dstport; we also update srcport to hide server-side partition details
			+ TODO: If compilation fails due to PHV limitation, we can remove srcmac and srcip
	* Implement client-side timeout and retry
	* Test correctness of all testcases 
		- Issue: client cannot send out packet
			+ Solution: manually configure arp such that udp socket can set *dstmac* (NOTE: not srcmac) for dstip
		- Issue: server udpsocket.recvfrom cannot return, yet tcpdump shows that the packet arrives at server
			+ If dstmac is wrong, it is dropped by either NIC or kernel network stack 
				* Solution: take client as example, we should configure arp with serverip and servermac (not clientmac)
			+ If iphdr.hdrlen or udphdr.hdrlen is wrong, kernel will drop the packet
				* udphdr.hdrlen: we retrieve checksum in udphdr, yet not update udphdr.hdrlen accordingly -> modify update_udplen_tbl
				* iphdr.hdrlen: we replace dpdk with udp socket, yet not update iphdr.hdrlen accordingly -> replace update_udplen_tbl with update_pktlen_tbl
			+ setsockopt can only disable udp/tcp checksum but not ipv4 checksum; while kernel will drop packet with wrong ipv4 checksum
				* Solution: update ipv4 checksum in switch by calculated_field
		- Test normal requests
		- Test cache population/eviction
		- Test conservative read
		- Test cache hit
+ Implement server-side snapshot
	* Make snapshot for rocksdb atomically and persistently
		- Create snapshot by checkpoint, and open it for range query
	* Erase out-of-date items from deleted set to keep memory efficiency
		- Triggered by DeletedSet::add if size exceeds a predefined threshold (constant factor * max thpt * rtt)
		- Erase item with the smallest seq number (aka oldest item) before adding new deleted record
	* Make snapshot for deleted set atomically and persistently
		- Copy deleted set as a read-only snapshot only for range query
			+ Implement clear and operator= for deleted set
		- Save deleted set into disk for persistence
			+ Implement load and store in deleted set
	- Remove old-enough snapshot database and snapshot deleted set to save storage
		* For example, for new snapshot 2, hold snapshot 1 yet remove others, as we do not know whether other servers have created current snapshots successfully
	* For ycsb_loader, create database -> make snapshot to store snapshotid, snapshotdb, and snapshotdeletedset -> close to store db and deletedset
	* For ycsb_server, open database -> load deletedset -> load snapshot including snapshotid, snapshotdb, and snapshotdeletedset
+ Test snapshot and fix issue of long wait time in switch os
+ Support switch failure recovery (provide reliability/weak-durability of consistency guarantee)
	- Controller stores latest snapshotid and in-switch snapshot data (including key, val, seq, and deleted) after sending snapshot data to servers 
		+ Just an engineering trick; in essence, we can get latest snapshotid and in-switch snapshot data from servers
	- Manually stop controller, and run controller_get_latest_snapshotid.c to get latest snapshotid
	- Manually stop each server to close rocksdb, and run server_recover.c with the lastet snapshotid
		* Remove larger snapshot data if any (e.g., snapshot 2)
		* Remove runtime data
		* Copy corresponding snapshot data as runtime data
	- Manually start P4 program in tofino
	- Manually copy latest snapshotid and inswitch snapshot data from controller to switchos
	- Run switchos with recover mode to retrieve inswitch cache from inswitch snapshot data
		* Invoke recover_switch/table_configure.py to set stateful regs with inswitch snapshot data
			- TODO: store and load seq_reg, and latest_reg
	- Manually start controller, each server (now runtime data = latest snapshot data), and configure programmable switch
+ NOTE: factors on RTT latency
	* Larger cache hit rate (aka larger # of queries under a fixed hot threshold) + larger server-side overhead in NoCache (aka larger # of records) / smaller server-side overhead in FarReach -> larger latency mitigation
	* Larger clean period means more cached keys and hence larger cache hit rate, yet more uniform server-side workload (aka less server-cache friendly) and hence larger server-side overhead in FarReach -> need a trade-off for best latency mitigation
		- NOTE: We should tune clean period such that all truly hot keys are cached into switch
+ Code change
	+ Implement parser interface -> ycsb_parser, synthetic_parser
		* Change synthetic_parser to set keylolo, keylohi, and keyhilo randomly (avoid from being stored in the same SST)
		* Move ycsb_server -> server, ycsb_remote_client -> remote_client, ycsb_loader -> loader
	+ Implement server.popclient thread to reduce worker latency
	+ Periodically clean up cm regs
+ Fix bug of CM
	* Calculate 4 hashes for CM
		- Issue: clone_hdr.client_udpport (save_client_udpport_tbl) cannot be placed into the 2nd stage of egress pipeline
		- Possible reason: it conflicts with inswitch_hdr.idx (access_latest_tbl) at egress stage 2?
			- Possible reason 1: compiler places inswitch_hdr.idx and clone_hdr.client_udpport into the same 32-bit container
				+ Assign 32 bits for inswitch_hdr.idx -> FAIL
			- Possible reason 2: limited 32-bit PHV containers after introducing inswitch_hdr.hashval_for_cm1/2/3/4
				+ Assign 16 bits for those fields (64K entries only needs 16 bits) -> FAIL
		- Final reason: calculating hash is MAU consuming -> ingress stage 2 consumes too many MAUs -> no MAU for save_client_udpport_tbl in egress stage 2, as # of MAUs in one physical stage is limited
			- Solution: place hash_for_cm1/2/3/4 into different stages of ingress piepline
	* Hot key does not update CM regs
		- Reason: we sample packet by key instead of random value, which is the reason of our low cache hit rate
		- Solution: sample packet by random value through modify_field_rng_uniform w/ hot threshold = 50
	* Try continuous cache evict
		- Longer delay of cache evict than that of cache population -> message queue overflow after inswitch cache is full
	* Try cache population after cleaning CM regs
		- Under synthetic workload, frequency of hot key may not exceed hot threshold during clean period
+ Deprecated result: results are not good as we begin with an empty cache, which is not the common case of system -> we should start with a pre-populated cache under static/dynamic workloads
	+ Test latency again (10M records, 1M queries, 50 hot threshold, 0.5 sampling ratio, 30s clean period, single thread)
		- Same key
			+ client-server RTT: 52us (dpdk: 32us)
				* Reason: requests on same key hit block cache, which do not access rocksdb database
			+ client-switch RTT: 13.7us (dpdk: 23us)
		- YCSB workload
			+ runtime RTT w/o cache: 75.9us (server-side latency: ~49us)
			+ runtime RTT w/ cache (w/ popclient thread; w/o cleaner): 70.2us (server-side latency: ~60.8us) -> 7.9% reduction
				* 400 hot keys, 600K total keys, 64K CM -> 1450 cached keys due to hash collision
				* Larger server-side overhead
					- If w/o cache, server receives skewed workload and can use cache to reduce server-side overhead
					- If w/ cache, server receives uniform workload and cannot use cache to reduce server-side overhead
				+ 26% hot requests -> 23% cache hit rate -> expected RTT w/ cache = 86.9*0.77 + 13.7*0.23 = 70.2us
			+ runtime RTT w/ cache (w/ popclient thread; w/ cleaner): 67us (server-side latency: ~55us) -> 11.8% reduction
				* 400 hot keys, 600K total keys, 64K CM -> 300 cached keys due to period clean
				* Better than w/o cleaner w/ smaller server-side overhead
					- No unnecessary cache population for cold keys due to false positive in CM
					- Although 2% packets do not hit in switch, they may make server-side workload more cache friendly?
				+ 26% hot requests -> 21% cache hit rate -> expected RTT w/ cache = 81.9*0.79 + 13.7*0.21 = 67.58us
			+ NOTE: To avoid server-side cache issue, we could use cache hit rate + runtime RTT w/ cache + client-switch RTT to calculate runtime RTT w/o cache
				* x*0.77 + 13.7*0.23 = 70.2 -> x=87 -> 19.5% reduction
				* x*0.79 + 13.7*0.21 = 67 -> x=81.2 -> 20.9% reduction
		- synthetic workload
			+ Implement parser interface -> ycsb_parser, synthetic_parser
				* Move ycsb_server -> server, ycsb_remote_client -> remote_client, ycsb_loader -> loader
			+ runtime RTT w/o cache: 60.3us (server-side latency: ~36.7us)
			+ runtime RTT w/ cache (w/ popclient thread; w/o cleaner): 57.1us (server-side latency: ~52.7us) -> 5% mitigation
				* 580 hot keys, 250K total keys, 64K CM -> 1655 cached keys due to hash collision
				* 39% hot requests -> 34% cache hit rate -> expected RTT w/ cache: 76*0.66 + 13.7*0.34 = 54.8us
			+ runtime RTT w/ cache (w/ popclient thread; w/ cleaner): 48.9us (server-side latency: ~45.3us) -> 18.33% mitigation
				* 580 hot keys, 250K total keys, 64K CM -> 647 cached keys due to period clean
				* 39% hot requests -> 33% cache hit rate -> expected RTT w/ cache: 69*0.67 + 13.7*0.33 = 50.75us
			+ NOTE: To avoid server-side cache issue, we could use cache hit rate + runtime RTT w/ cache + client-switch RTT to calculate runtime RTT w/o cache
				* x*0.66 + 13.7*0.34 = 57.1 -> x=79.5 -> 28% reduction
				* x*0.67 + 13.7*0.33 = 48.9 -> x=66.2 -> 26% reduction
	+ Test latency again (10M records, 10M queries, 50 hot threshold, 0.5 sampling ratio, 30s period, single thread)
		- synthetic workload
			+ runtime RTT w/o cache: 64.4us (server-side latency: 41us)
			+ runtime RTT w/ cache (w/ cleaner; 30s clean period): 54.1us (server-side latency: 51.7us) -> 16% reduction
				* 6K hot keys, 1.6M total keys, 64K CM -> 792 cached keys due to too small clean period
				* 53% hot requests -> 37% cache hit rate -> expected RTT w/ cache: 74.4*0.63 + 13.7*0.37 = 51.9us
				* x*0.63 + 13.7*0.33 = 54.1 -> x=77.8 -> 30% reduction
			+ runtime RTT w/ cache (w/ cleaner; 60s clean period): 48.1us (server-side latency: 51.7us) -> 25.3% reduction
				* 6K hot keys, 1.6M total keys, 64K CM -> 2406 cached keys due to still small clean period
				* 53% hot requests -> 42% cache hit rate -> expected RTT w/ cache:  74.4*0.58 + 13.7*0.37 = 48.2us
				* x*0.58 + 13.7*0.42 = 48.1 -> x=73 -> 34% reduction
	* Try ycsb workload (32 clients, 32 servers, 100M records, 32M queries, 50 hot threshold, 0.5 sampling ratio, 60s period)
		* runtime RTT w/ cache: 380us (server-side latency not including packet receiving overhead: 80us)
			- Reason: in our simulation testbed, multiple client/server threads are waiting for one physical NIC, which introduces resource contention and hence larger overhead of receiving packets; while in real scenario, one thread should be a physical machine with an individual NIC, i.e., no contention on hardware resource -> we should use one client thread <-> one server thread to test latency
		* No thpt improvement (<10% cache hit rate -> most significant hot keys are not cached in switch)
			- Reason: ycsb workload is not highly skewed -> very small cache hit rate under multi-thread scenario -> server is still the bottleneck
			- TODO: retest w/ warmup phase
			- TODO: we can tune Zipf parameter to generate synthetic workload by ycsb
	* Try Zipf synthetic workload (32 clients, 32 servers, 100M records, 32M queries, 50 hot threshold, 0.5 sampling ratio, 15s period)
		* 30% packets belonging to 600 hot keys -> 15.6% cache hit rate w/ 6000 cached keys -> only 54% throughput improvement
			- Reason of low cache hit rate
				+ Larger socket overhead due to one NIC listened by many threads -> longer cache population delay (us-level) 
				+ Many false positives due to large clean period -> longer cache population delay (ms-level)
				+ Many false negatives due to large hot threshold -> miss truly hot keys
				+ Cache warmup overhead -> lose many opportunities of cache hits for hot keys
			+ Consider smaller hot threshold and smaller clean period + NetCache parameters (64M records) -> FAIL
				* hot_threshold = 10, sampling ratio = 0.5, clean period = 1s -> 22% cache hit rate -> still imbalance
				* hot_threshold = 10, sampling ratio = 0.5, clean period = 5s -> 13% cache hit rate -> still imbalance
				* hot_threshold = 10, sampling ratio = 0.5, first period = 1s, clean period = 5s -> 31% cache hit rate -> better but still limited improvement
+ cache a key -> clean CM -> test cache hit: SUCCESS
	+ Issue: controller cannot receive CACHE_POP after server sends
		* Possible reason 1: after opening too many file descriptors, controller and server cannot trasnmit packets -> FAIL
			+ Solution: after launching server, we gracefully close servers to close fd instead of brute-force ctrl-c
				* Issue: server cannot exit after receiving SIGKILL due to blocking socket
				* Solution: change blocking socket (including recv() and accept()) to non-blocking manner
		- Possible reason 2: tcp cannot receive CACHE_POPs due to too many connections and subthreads on a single tcp port -> SUCCESS
			- Solution: use multiple tcp ports for different controller.popservers instead of single one with subthreads
	+ Issue: large client-server RTT latency and larger cache population delay
		* Reason: after many threads listen on the same NIC, UDP sockets for normal requests cannot send/receive packet immediately (extra tens / hundreds of microseconds)
		* RTT latency: use single thread to test latency
		* Cache population delay: socket overhead is not dominant, which should be false positive
+ Cached key set increases slower after cleaning CM regs
	+ Reason: packets of cached keys will not update CM -> fewer false positives after cleaning CM regs -> that's what we want
+ Test normalized throughput and calculate absolute throughput
	* Implement calculate_thpt.py
		* Suppose one physical packet = x virtual packets
		* Fix # of records stored in server, and run client within a fixed period (deprecated) -> we use normalized thpt directly
			- For switch, # of cache hits * x <= switch max throughput (not critical)
				+ Recirculation affects switch max throughput
			- For each server, # of cache misses * x <= server max throughput
				+ We should test server max throughput with multiple threads for rocksdb
			- Absoluate throughput = # of packets per second * x
		* NOTE: for NetCache + write-back policy under dynamic workload
			- For YCSB workload, some packets arrive server during cache population, baseline is blocking while we do not block
			- Within a fixed time period, we can process more packets; for example
				+ Baseline: 1 pkt w/ 1ms, and 80 pkts w/ 20us
				+ FarReach: 20 pkts w/ 50us, and 80 pkts w/ 20us
				+ A little smaller avg latency, yet extremely smaller max/tail latency
				+ Maybe similar normalized thpt under sufficient large input traffic -> we cannot use this baseline?
+ Implement warmup phase for static workload
	* Use PUTREQ instead of GETREQ to implement WARMUPREQ
		- Issue: server cannot receive packet due to wrong hdrlen
			+ Reason: default function of add_or_remove_header_tbl is remove_all
			+ Solution: change ptf configure script to add value header for WARMUPREQ
		- Issue: duplicate cache population reported by same server or two different servers
			+ After checking controller and server, they do not report the anomaly -> switchos issue
			+ Reason: tcp socket uses shallow copy for send buffer; even set NODELAY, tcp cannot send packet immediately;
				- For pkt i w/ value A, if tcp shallow-copies the userspace buf w/o sending out immediately, the next pkt i+1 w/ value B will overwrite the userspace buf, which trigger duplicate content
			+ Solution: we use UDP + timeout-and-retry to implement reliable and low-latency channel for cache population/eviction -> SUCCESS
			+ TODO: Replace TCP w/ UDP + timeout-and-retry for snapshot
	* Use MAT to configure hot threshold instead of repeat re-compilation
	* Issue: loader cannot write keys into correct databases -> GETREQ_POP may not trigger cache population due to stat=false
		- Multiple servers cause this issue
			- 10M records + 1 server -> no such issue
			- 100M records + 32 servers -> w/ issue
			- 10M records + 32 servers -> w/ issue
			- 10K records + 2 servers -> w/ issue
			- Reason: we uniformly split loading workload for different servers yet not based on partition strategy
				+ E.g., key 1 is stored in worker0 in loading phase, while another worker answers its query in transaction phase
		- Try software crc32 to see whether it is equivalent with Tofino -> SUCCESS
		- Deprecated: Update split_workload to split workload based on crc32 result -> hash each key to split workload by a single thread of split_workload is too slow
		- Solution: in loader, each thread hashes keys to find target dbs, then store them into corresponding db
			- 10M records + 32 servers -> SUCCESS
+ Test latency and throughput on static workload
	* Latency
		- 1 server thread (w/ 1 client thread): 66.16us -> 40.08us; 40% reduction
		- 128 server threads (w/ 32 client threads): 344.5us -> 172.5us; 49.94% reduction
			+ Reason: more server threads incurs more simulation overhead including context switching and NIC resource contention
	* Throughput
		* Calculate normalized throughput (100M records, 32M queries, 32 client threads)
			- 8 server threads: 2.48X
			- 32 server threads: 4.6X
			- 128 server threads: 13.8X
+ Implement hot-in dynamic workload (preliminary version)
	- Generate dynamic rule on key popularity change for every 10 secs
	- TODO: Switchos resets query statistics every 1 sec, including CMs and frequency counters
	- Client changes key popularity every 10 secs
		+ For hot-in, client chooses 200 uncached cold keys to replace 200 hotest keys
	- server.main collects # of pkts of all servers per second after receiving the first packet (globally)
	- NOTE: client can start to apply the dynamic rule at any sec in every 10 secs, yet not affect server-side statistic module
	- Imlement dynamic_calculate_thpt.py
	- Test 20-sec latency and thpt on dynamic workload
		+ Issue: server-side statistics do not change every 10 seconds
		+ Reason: client and server do not have synchronous/consistent timing
		+ Solution: for precise normalized thpt, use client.main<->server.main to count server-side statistics per second
	- For random: limited effect on cache hit rate
	- For hot-out: no effect on cache hit rate
+ Implement range query
	- Change range_parition_for_scan_tbl into range_partition_for_scan_endkey_tbl to reduce MAT entries
+ Compile range query
	- Reduce MATs related with range query
		+ Merge process_cloned_scanreq_split_tbl into process_scanreq_split_tbl
		+ Merge scan_forward_tbl into eg_port_forward_tbl
	- Issue: VLIW limitation
		+ Reason 1: we cannot access eg_intr_md_from_parser_aux.clone_src due to PHV/VLIW limitation
		+ Solution 1: add is_clone in split_hdr for SCANREQ
		+ Reason 2: too many MATs
		+ Solution 2: merge is_last_scansplit_tbl and is_last_clone_tbl into lastclone_lastscansplit_tbl
+ Test range query
	+ Fix ptf syntax issues
	+ Check range partition of normal request
	+ Check repeat packets of CACHE_POP_INSWITCH_ACK and XXX_CASE1
	+ Update pktlen for SCANREQ_SPLIT
	* Check multiple packets of SCANREQ_SPLIT
		- Cannot read latest put -> correct as we perform range query based on snapshot
		- Incorrect # of SCANREQ_SPLIT
			+ Reason: clone_hdr.clonenum_for_pktloss = 1 (compiler reuse same PHV for clonenum_for_pksloss and cur_scanidx???)
			+ Solution: in process_scansplit_tbl, we set clone_hdr.clonenum_for_pksloss = 0 for SCANREQ_SPLIT, set meta.remain_scannum = 0 for other optypes in default action -> compiler should not reuse same PHV now as both fields appear in the same action
			+ Test CACHE_POP_INSWITCH_ACK, XXX_CASE1, and SCANREQ_SPLIT in debug mode
			+ Test CACHE_POP_INSWITCH_ACK, XXX_CASE1, and SCANREQ_SPLIT in non-debug mode
+ Prepare LOADREQ/LOADACK for loading phase to avoid in-switch state change (simple fowarding is enough)
+ Add nodeidx_foreval in stat_hdr for GET/PUT/DELRES and after split_hdr for SCANRES_SPLIT
	* NOTE: SCANRES_SPLIT is not processed by switch (only used by end-hosts)
	* Check nodeidx_foreval for cache hit/miss
		- Fix hdrlen issue for GETRES/PUTRES/DELRES/XXX_CASE1 due to adding nodeidx_foreval in stat_hdr
	* Change remote_client.c to measure normalized throuhgput in client side -> then we can drop dynamic control process between client.main and server.main
	* Test normalized thpt for static/dynamic workload in client-side (prepare for ycsb)
+ Use server-side timeout-and-retry to guarantee that the cache population must arrive at switchos
	* Replace controller.popclient with controller.popserver.popclient
	* NOTE: not wait ptf framework to reduce latency and leverage switchos max thpt
+ Implement udpsend/recvlarge_udp/ipfrag to send/recv snapshot data and ScanResponseSplit (providing frag_hdrsize)
	* Implement udprecvlarge_multisrc_udp/ipfrag for ScanResponseSplit
	* Introduce SNAPSHOT_ACK and use switchos-side timeout-and-retry to guarantee that snapshot data can arrive at server
	* Test empty snapshot data, small snapshot data, and special case1/case2
	* Test large snapshot data
		- Fix timeout issue caused by udp packet loss due to insufficient udp receive buffer (many threads listen on localhost interface -> some thread cannot receive udp packets immediately from short udp receive buffer)
			+ Set udp recv buffer max size of linux by configure_client/server/switchos.sh
			+ Set udp recv buffer size of socket in runtime for large data by setsockopt
		- Fix duplicate control packet in switch/server.snapshotserver
	* Test empty scan data, small scan data
	* Test large scan response from single/multiple src(s)
* Re-implement crash-consistent snapshot such that controller guides switchos to perform in-switch snapshot
	- Implement perserver snapshotserver and snapshotdataserver
	- Embed snapshotid into SNAPSHOT_SENDDATA, which must = server.snapshotid + 1 (store latest inswitch snapshot -> atomically update inswitch snapshot and server-side snapshot -> increase snapshot id and drop old snapshot data)
	- Encapsulate in-switch snapshot into RocksdbWrapper (rocsdb + deletedset for all methods; inswitch data for FarReach)
	- Embed serveridx into perserver scan response -> client-side retry if inconsistent
	- Update expected_threads in controller, server, and switchos
	- Test time of snapshot
		- Entire time: ~3.5s
		- Time of stopping cache population/eviction: ~50ms
		- Time of enforcing single path: ~10ms
		- Time of loading data by ptf: ~3.3s
+ Reliability
	+ NOTE: we focus on weak-form durability (aka data loss w/ consistency guarantee after failure) and eventual availability
		* We do not target service full uptime / zero downtime / fault tolerance / fast recovery
	+ Support controller failure recovery (stateless aka no key-value data -> no concern on availability/durability)
		* For cache population, server-side timeout-and-retry guarantees that the populated data can be eventually sent to switchos
		* For cache eviction, switchos-side timeout-and-retry guarantees that the evicted data can be eventually sent to server
		* For snapshot, controller.cleanup phase can clean stale snapshot states in switchos/client after launching a new controller
		* We only need to launch a new controller, and restart a new snapshot
			+ TODO: Check all kinds of corner cases
			+ TODO: If send or recv fails (if using tcp), wait for connecting with new controller (no need now as w/o tcp sockets)
	+ For server crash, we can utilize persistence of rocksdb to recover data
		* NOTE: if we treat both switch and server as storage node, we have already trade availability for failure consistency based on snapshot
		* TODO: We can use sync write for recoverable failure -> undermine deployability/generality
		* TODO: For unrecoverable failure, we can use server-side replication -> undermine thpt
+ Deprecated: Re-implement client-side throughput calculation
	* For each client thread: total pktcnt / sum latency
	* For system thpt: sum over throughputs of all clients
	* Deprecation reason: we should use (total opcnt / system running time) to get system average throughput as in YCSB client.java
		- It is ok to sum over per-client average throughput to get the system throughput, but be careful about calculating way
		- Correct way of calculating per-client thpt
			+ (1) per-client opcnt / system running time
			+ (2) average value of per-client per-sec opcnt based on entire system running time
		- Wrong way of calculating per-client thpt
			+ WRONG: per-client opcnt / per-client time of finishing those operations to reduce client overhead
			+ Reason: if one client thread finishes before system running time, the calculated per-client thpt is not averaged based on the entire system running time, and system throughput will infinitely increase as # of client threads increases
			+ Example: system thpt of 100; per-client max thpt of 50
				* 2 client threads: 50 + 50 = 100
				* 4 client threads (2 finish in the 1st second, and 2 finish in the 2nd second): 50 + 50 + 25 + 25 = 150
					- NOTE: we do NOT consider 0 op/s thpt of the first 2 client threads in the 2nd second
				* 6 client threads (2 finish in 1st sec, 2 finish in 2nd sec, 2 finish in 3rd sec): 50*2 + 25*2 + 16*2 = 182
+ Reduce memory overhead in end-hosts by dynamic array
	+ Change udprecvlarge_multisrc to assign dynamic memory structures in runtime instead of max server num
	+ Implement dynamic array to reduce memory usage of MAX_LARGE_BUFSIZE
		+ Implement DynamicArray::dyanmic_memcpy: increase memory each time until achieving MAX_LARGE_BUFSIZE
		+ Implement DynamicArray::clear: decrease memory to MAX_BUFSIZE and reset metadata
		+ Implement DynamicArray::dynamic_memset: increase memory each time until achieving MAX_LARGE_BUFSIZE
		+ Implement ScanResponseSplit::dynamic_serialize
		+ Implement Key::dynamic_serialize
		+ Implement Val::dynamic_serialize
	+ Deploy dynamic array for scan response
		+ Server: use dynamic array to serialize scan response in server.worker
		+ Client: use dynamic array to receive scan response in socket_helper.udprecvlarge_multisrc
	+ Deploy dynamic array for snapshot data
		+ switchos: use dynamic memory to receive snapshot from ptf in socket_helper.udprecvlarge, and send to controller by SNAPSHOT_GETDATA_ACK
		+ controller: use dynamic memory to receive snapshot from switchos, split snapshot, and send to each server by SNAPSHOT_SENDDATA
		+ server: use dynamic memory to receive large snapshot data from SNAPSHOT_SENDDATA
	+ Test large scan response from single/multiple src
	+ Test large snapshot
		+ Issue: switchos cannot receive all snapshot data from ptf
		+ Reason: dynamic array is slightly slower than raw buf, so switchos cannot receive all fragments immediately
		+ Solution: run configure_switchos.sh to enlarge max udp rcvbuf size; set udp rcvbuf size of switchos_snapshotserver_for_ptf_udpsock to avoid pktloss of snapshot data from ptf
+ Test static workload under range partition
	* Use settings after fixing bottleneck issues (see docs/bottleneck-find-process-20220618.md for details)
	* FarReach w/o inswitch cache
		- server 1 + client 64: 0.09 MOPS
		- server 8 + client 512: 0.17 MOPS
		- server 16
			+ client 1024: 0.19 MOPS
			+ client 2048: 0.18 MOPS
			+ client 1024 w/ disable WAL: 0.257 MOPS
		- server 32 + client 2048: 0.196 MOPS
	* FarReach w/ inswitch cache
		- server 1 + client 64: 0.1776 MOPS (1.97X runtime; 1.97X normalized)
		- server 8 + client 512: 1.08 MOPS (6.35X runtime; 6.75X normalized)
		- server 16 
			+ client 1024: 1.67 MOPS (8.8X runtime; 11.3X normalized)
			+ client 2048: 1.4~1.5 MOPS
			+ client 1024 w/ disable WAL: 1.72 MOPS (6.7X runtime; 11.3X normalized)
				* Reason of limited improvement: no disk overhead; yet input traffic is not sufficient
			+ client 2048 w/ disable WAL: 1.73 MOPS
				* Reason of limited improvement: client-side input traffic is insufficient
				* NOTE: 2048 client threads makes no sense vs. 1024 client threads, as wait latency increases from ~400us to ~800us
		- server 32 
			+ client 1024: 1.68 MOPS (8.5X runtime; 18.4X normalized)
				* Reason: FarReach does not improve thpt due to disk overhead; FarReachNoCache improves thpt due to CPU bottleneck
			+ client 2048: 1.67 MOPS
				* Reason of no improvement: disk overhead
			+ client 2048 w/ 8M wal_bytes_per_sync: 1.4 MOPS
				+ More write stalls and slowdowns
			+ client 2048 w/ 32M wal_bytes_per_sync: 1.7 MOPS
				+ Still w/ write stall/slowdown
			+ client 2048 w/o wal_bytes_per_sync: 1.73 MOPS
				+ Still w/ write stall/slowdown
			+ client 2048 w/ disable WAL: 1.74 MOPS
				+ Issue: no disk overhead, yet server-side CPU is not fully utilized
				+ Observation: server-side throughput is relatively stable; even if for the second with smaller system throughput, the process latency including rocksdb process latency is still small, but the wait latency is large -> input traffic is not large enough
				+ Observation: client-side wait latency (from receiving response to sending next request) is relatively large due to CPU contention (~800us)
			+ client 3096 w/ disable WAL: 1.68 MOPS
				+ Reason of no improvement: no sufficient input traffic due to client-side CPU contention

## Run

- Hardware configure
	+ DPDK
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
		+ `./server`
		+ NOTE: to close server, use `sudo kill -15` to send SIGKILL
	- Launch clients in end host
		- Warmup phase: `./warmup_client`
		- Transaction phase: `./remote_client`

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
- Test cases of cache population/eviction: see directory of "testcases/population"
	+ Case 1: read(k1,v1)-read(k1,v1) (GETREQ-GETREQ_POP arrive at server, which sends CACHE_POP_INSWITCH and waits for CACHE_POP_INSWITCH_ACK)
		* Key k1 in cache_lookup_tbl, cm=2, v1 in vallen and val, valid=1, {cache_frequency, seq, savedseq, latest, deleted, case1}=0
	+ Case 2: put(k1,v1)-put(k1,v2) (PUTREQ-PUTREQ_POP arrive at server, which sends CACHE_POP_INSWITCH and waits for CACHE_POP_INSWITCH_ACK)
		* Key k1 in cache_lookup_tbl, cm=2, v2 in vallen and val, valid=1, seq=2, savedseq=2, {cache_frequency, latest, deleted, case1}=0
	+ Case 3: read(k1,v1)-read(k1,v1)-put(k2,v2) (GETREQ-GETREQ_POP-PUTREQ_POP, where GETREQ_POP triggers cache population, while PUTREQ_POP triggers cache eviction)
		* Key k2 in cache_lookup_tbl, cm=3, v2 in vallen and val, valid=1, seq=1, savedseq=1, {cache_frequency, latest, deleted, case1}=0
- Test cases of conservative read
	+ Case 1
		* Step 1: case 1/2/3 of cache population
		* Step 2: read(k1,v1) (GETREQ_NLATEST arrive at server, which sends GETRES_LATEST_SEQ and hence GETRES)
			- Key k1 in cache_lookup_tbl, cm=2, v1 in vallen and val, valid=1, cache_frequency=1, latest=1, {seq, savedseq, deleted, case1}=0
	+ Case 2
		* Step 1: case 1/2/3 of cache population + DELREQ (arrive before finishing cache population)
			- Key k1 in cache_lookup_tbl, cm=2, v1 in vallen and val, valid=1, seq=1, {cache_frequency, latest, deleted, savedseq, case1}=0
		* Step 2: read(k1,deleted) (GETREQ_NLATEST arrive at server, which sends GETRES_DELETED_SEQ and hence GETRES)
			- Key k1 in cache_lookup_tbl, cm=2, vallen=0, valid=1, cache_frequency=1, latest=1, deleted=1, seq=1, savedseq=1, {case1}=0
- Test cases of cache hit
	+ Case 1
		* Step 1: case 1 of conservative read
		* Step 2: read(k1,v1) (GETREQ arrive at switch, which sends GETRES to client)
			- Key k1 in cache_lookup_tbl, cm=2, v1 in vallen and val, valid=1, cache_frequency=2, latest=1, {seq, savedseq, deleted, case1}=0
	+ Case 2
		* Step 1: case 2 of conservative read
		* Step 2: read(k1, deleted) (GETREQ arrive at switch, which sends GETRES w/ stat=0 to client)
			- Key k1 in cache_lookup_tbl, cm=2, vallen=0, valid=1, cache_frequency=2, latest=1, deleted=1, seq=1, savedseq=1, {case1}=0
	+ Case 3
		* Step 1: case 2 of conservative read
		* Step 2: put(k1, v2) (PUTREQ arrive at switch, which sends PUTRES w/ stat=1 to client)
			- Key k1 in cache_lookup_tbl, cm=2, v2 in vallen and val, valid=1, cache_frequency=2, latest=1, deleted=0, seq=2, savedseq=2, {case1}=0
	+ Case 4
		* Step 1: case 1/2/3 of cache population
		* Step 2: del(k1, v2) (PUTREQ arrive at switch, which sends PUTRES w/ stat=1 to client)
			- Key k1 in cache_lookup_tbl, cm=2, vallen=0, valid=1, cache_frequency=0, latest=1, deleted=1, seq=1, savedseq=1, {case1}=0
- Test cases of latency
	+ Case 1: latency between client and server
		* 1000 GET/PUT/DEL w/o cache hit
	+ Case 2: latency between client and switch
		* Step 1: case 1 of conservative read
		* Step 2: 1000 GET/PUT/DEL w/ cache hit
	+ Case 3: latency with cache population delay
		* Same as case 1 but not disable cache population
- Test crash-consistent snapshot
	+ Case 3: controller sends SNAPSHOT_START -> switchos sets snapshot flag -> put(k1,v1) (PUTREQ_SEQ_CASE3) -> put(k1,v2) (PUTREQ_POP_SEQ_CASE3 -> cache population) -> del(k3) (DELREQ_SEQ_CASE3) -> swichos loads snapshot and finishes
		* No case 2 as cache is not full and hence no eviction
		* Snapshot data: no cached record in switch
	+ Case 2: controller sends SNAPSHOT_START -> read(k1,v1) (GETREQ) -> read(k1,v1) (GETREQ_POP) -> switchos sets snapshot flag -> put(k2,v2) (PUTREQ_POP_SEQ_CASE3 -> cache eviction w/ CACHE_EVICT_CASE2) -> put(k3,v3) (PUTREQ_POP_SEQ_CASE3 -> cache eviction w/ CACHE_EVICT_CASE2) -> swichos loads snapshot and finishes
		* Snapshot data: <k1, v3> (before rollback) -> <k1, v1> (after rollback)
	+ Case 1-1: controller sends SNAPSHOT_START -> read(k1,v1) (GETREQ) -> read(k1,v1) (GETREQ_POP) -> switchos sets snapshot flag -> del(k1) (DELREQ_SEQ_INSWITCH_CASE1 and DELRES) -> swichos loads snapshot and finishes
		* Snapshot data: <k1, deleted> (before rollback) -> <k1, v1> (after rollback)
	+ Case 1-2: controller sends SNAPSHOT_START -> read(k1,v1) (GETREQ) -> read(k1,v1) (GETREQ_POP) -> delete(k1) in switch -> switchos sets snapshot flag -> put(k1,v2) (PUTREQ_SEQ_INSWITCH_CASE1 and PUTRES) -> swichos loads snapshot and finishes
		* Snapshot data: <k1, v2> (before rollback) -> <k1, deleted> (after rollback)
	+ Case 1-3: controller sends SNAPSHOT_START -> read(k1,v1) (GETREQ) -> read(k1,v1) (GETREQ_POP) -> put(k1,v2) in server -> switchos sets snapshot flag -> read(k1,v2) (GETRES_LATEST_SEQ_INSWITCH_CASE1 and GETRES) -> swichos loads snapshot and finishes
		* Snapshot data: <k1, v2> (before rollback) -> <k1, v1> (after rollback)
	+ Case 1-4: controller sends SNAPSHOT_START -> read(k1,v1) (GETREQ) -> read(k1,v1) (GETREQ_POP) -> del(k1) in server -> switchos sets snapshot flag -> read(k1,deleted) (GETRES_DELETED_SEQ_INSWITCH_CASE1 and GETRES) -> swichos loads snapshot and finishes
		* Snapshot data: <k1, deleted> (before rollback) -> <k1, v1> (after rollback)

## Fixed issues

- Process of fixing latency issue
	+ Survey
		+ Read source code of l3fwd
			* l3fwd maintains an application-level buffer of 32 packets over DPDK; if buffer exceeds 32 packets or time exceeds BURST_TX_DRAIN_US, it invokes rte_eth_tx_burst to send out packets immediately
		+ Read source code of vhost
			* BURST_TX_DRAIN_US is an application-level parameter, which is used to drain tx_mbuf->mbuf_table periodically
			* BURST_RX_WAIT_US is an application-level parameter, which periodically waits for available entries of vhost such that it can place the packets from guest.rte_eth_rx_burst into vhost
		+ Read source code of lib/librte_ethdev/rte_ethdev.h::rte_eth_rx_burst (i.e., dev->rx_pkt_burst) -> drivers/net/i40e/i40e_rxtx_vec_neon.c::_recv_raw_pkts_vec -> drivers/net/i40e/i40e_rxtx.c::i40e_recv_pkts
			* i40e_recv_pkts
			* RTE_I40E_DESCS_PER_LOOP
		+ TX latency
			* NOTE: pkt -> tx_ring (hardware ring mapped with tx_queue)
			* As user must know # of packets to be sent out, DPDK does not maintain internal buffer for rte_eth_tx_burst
				- If nb_pkts > # of packets in rte_mbufs, it may incur buffer overflow
				- We can set nb_pkts=1 in rte_eth_tx_burst for low TX latency
			* If for high thpt, we can maintain an application-level buffer as in l3fwd or vhost or use rte_eth_tx_buffer
		+ RX latency
			* NOTE: pkt -> rx_ring (hardware ring mapped with rx_queue) -> sw_ring (software ring) -> RSS
			* Try to set nv_pkts=1 in  rte_eth_rx_burst -> FAIL
				- Even if nb_pkts > 1, i40e_recv_pkts should return if encountering a desc w/ DD bit = 0
	+ Failed trials
		- set RTE_I40E_DESCS_PER_LOOP=1 in drivers/net/i40e/i40e_rxtx.h in dpdk-18.11 to support rx_burst=1 -> FAIL
				+ avg latency: 30us for rx_burst=32; 28us for rx_burst=4; 28us for rx_burst=1
			- Implement break-down latency test in both client (req + rsp + wait (including receiver)) and server (process + receiver)
				- client: 0.4us to process request; 0.3us to process response; 26.4us for polling (0.4us for message queue)
				- server: 0.4us for message queue; 3us for in-memory KVS
				- -> 26.4-3.4=23us for DPDK overhead (dominant) + transmission latency
		- Use a simple dpdk-based echo program (dpdktest_client-to-dpdktest_server w/o receiver) -> message queue overhead -> FAIL
			- client-side wait_latency is still around 28us
			- One-to-one manner also implies that it is not due to cross-NUMA overhead
		- Use a simple P4 forwarding program as switch -> tofino overhead -> FAIL
			- client-side wait_latency is still around 27us
		- Mount hugelbfs to enable 2MB hugepage -> TLB overhead -> FAIL
			- client-side wait_latency is still around 27us
		- Check dpdk-related part in NetCache source code
			- only provide a simple simulator based on udp socket and bmv2 w/o dpdk module and tofino module -> FAIL

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
