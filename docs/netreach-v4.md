# Tofino-based NetREACH + DPDK-based XIndex (in-memory KVS) + variable-length key-value pair w/ snapshot + control-plane-based cache update (netreach-v4)

- Copy from netreach-voting-v3-memory, refer to netreach-voting-v2
- Ptf will configure MAT in all pipelines
	+ To configure a specific pipeline, inject packet into data plane
- Althought eviction or snapshot focus on latest=1, which may come from response of GETREQ_NLATEST
	+ As savedseq of such a record must <= seq in server, no unnecessary overwrite for eviction and no incorrect result for range query
	+ For the results with latest=1 from PUTREQ/DELREQ, savedseq can be either larger or smaller than seq in server -> seq comparison
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
- For crash-consistent snapshot
	+ NOTE: we do not need case3 as it does not save bandwidth, while explict notification for server-side snapshot is acceptable as snapshot is not frequent and dominated by register loading
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

## Overview

- Packet format
	+ Ethernet + IP + UDP
	+ Operation header: 1B optype, 16B key
	+ Value header: 4B vallen, variable length value (8B padding if vallen <= 128B)
	+ Result header: 1B result
		* NOTE: success flag for PUTRES/DELRES; deleted flag for GETRES; SCANRES does not need it
	+ Inswitch header: 1b is_cached, 1b is_sampled, 6b padding, 2B hashval, 2B serveridx (for case3), 2B idx
- Client
- Switch
	+ Ingress pipeline
		* Stage 0
			- cache_lookup_tbl (optype, key -> idx, is_cached)
				+ idx: index for in-switch cache (assigned by controller)
			- hash_tbl (optype, key -> hashval)
				- hash_for_partition_tbl (optype, key -> hash_partitionidx)
					+ Hash_partitionidx: partition across servers
				- hash_for_CM_tbl (optype, key -> hash_cmidx)
					+ hash_cmidx: index for CM
				- In implementation, we use hashval = hash_partitionidx = hash_cmidx to save PHV
			- sample_tbl (optype, key -> is_sampled)
			- TODO: In distributed extension, spine/leaf switch uses idx to access the bucket in the corresponding virtual switch (controller has considered partition across virtual spine/leaf switches when assigning idx)
				+ TODO: Only leaf switch needs to perform hash_for_partition_tbl for partition across servers
				+ TODO; Only spine switch needs to assign seq

				+ NOTE: we enforce spine-leaf path for each key to ensure serializability and availability, which can be statically configured into switches (no hash operation in client); however, DistCache uses dynamic mechanism (power-of-two-choices) for load balance, client needs to perform hash-based sampling
					* TODO: In DistCache, client calculates a hash value to introduce sampling overhead
					* TODO: But not need to embed the sampling result; instead, we can use in-switch sample to choose spine or leaf;
			- NOTE: SCANREQ does not access all above tables
		* Stage 1: hash_partition_tbl or range_partition_tbl
			- Hash partition: (optype, hashval range -> udp.dstPort, egress_port)
				+ TODO: Check why we need the param of 0 for MAT with range matching
			- TODO: range partition: (optype, key range -> udp.dstPort, egress_port)
				+ TODO: We treat the most significant 4B of key as int32_t for range matching -> need conversion between little-endian and small-endian for the 4B of each key
			- NOTE: we use udp.dstPort to simulate different egress ports
		* Stage 2: ig_port_forward_tbl (op_hdr.optype -> op_hdr.optype)
			- Add inswitch_hdr to FarReach packet
	+ Egress pipeline
		* Stage 0 (4 ALU)
			- CM: 4 register arrays of 16b (optype, is_sampled, is_cached, hashval -> cm_predicates)
				+ NOTE: CM_BUCKET_COUNT is independent with KV_BUCKET_COUNT
				+ NOTE: we resort to server to notify controller to avoid in-switch BF; NetCache directly reports to server yet with BF
			- TODO: is_snapshot_tbl (optype -> is_snapshot)
		* Stage 1 (4 ALU)
			- is_hot_tbl (cm_predicates -> meta.is_hot)
				+ Reduce 4 cm_predicates into 1 meta.is_hot to reduce TCAM usage
			- cache_frequency_tbl (optype, is_sampled, is_cached, idx -> none)
			- valid_tbl (optype, is_cached, idx -> is_valid)
				+ NOTE: eviction triggerred by PUTREQ_LARGE belongs to case 1 (w/o key change, w/ value change to an inalid status)
				+ NOTE: valid=1 means no large vaule in server
				+ TODO: PUTREQ_LARGE_EVICT_CASE1 does not need to embed status_hdr.valid as valid must be 1 (w/o eviction if invalid)
				+ TODO: PUTREQ_CASE1/DELREQ_CASE1 need to embed status_hdr.valid as valid count be 0
			- TODO: seq (optype -> seq)
				+ NOTE: SEQ_BUCKET_COUNT is independent with KV_BUCKET_COUNT
				+ NOTE: we need seq mechaism to avoid unnecessary overwrite caused by eviction of PUTREQ_LARGE or cache update
			- TODO: case1 (optype, is_cached, idx, is_snapshot -> is_case1)
		* Stage 2 (4 ALU)
			- latest_tbl, deleted_tbl (optype, is_cached, idx -> status)
				+ NOTE: one register can provide at most 3 stateful APIs -> cannot aggregate valid and latest/deleted as a whole
					* As latest/deleted relies on valid for GETRES_LATEST/DELETED, we place valid before latest/deleted
				+ NOTE: latest=0 means no PUTREQ/DELREQ or response of GETREQ_NLATEST arrive at switch
				+ NOTE: even with seq mechanism, we still need latest_tbl for response of GETREQ_NLATEST (and PUTREQ_LARGE_EVICT)
			- TODO: savedseq (optype, is_cached, idx, seq -> savedseq)
				+ NOTE: we assume that there is no reordering between spine and leaf switch due to a single FIFO channel
			- vallen (optype, is_cached, idx -> vallen_hdr and val_hdr)
				+ TODO: vallen and value can be placed from stage 1 to save # of stages if necessary
		* Stage 3-9 (4 ALU)
			- From vallo1 to valhi14 (optype, is_cached, idx -> vallen_hdr and val_hdr)
		* Stage 10 (4 ALU)
			- vallo15, valhi15, vallo16, valhi16 (optype, is_cached, idx -> vallen_hdr and val_hdr)
			- TODO: eg_port_forward_tbl (optype, is_cached, is_hot, is_valid, is_latest, is_deleted)
- Server
- Controller
- SwitchOS

## Details 

- GETREQ
	+ Client sends GETREQ
	+ Ingress: GETREQ -> GETREQ_INSWITCH (is_sampled, is_cached, hashval, idx)
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
					+ If latest=1 and deleted=1, set result_hdr.result=0 as deleted and send back GETRES
					+ If latest=1 and deleted=0, set result_hdr.result=1, and send back GETRES
					+ If latest=0, forward GETREQ_NLATEST to server
	+ Server
		* GETREQ -> GETRES
		* GETREQ_POP
			- TODO: If key not exist or vallen > 128B, send back GETRES and ignore cache population
			- TODO: Otherwise, send back GETRES and notify controller for cache population
		* GETREQ_NLATEST
			- TODO: If key exists and vallen > 128B, send back GETRES
			- TODO: If key exists and vallen <= 128B, send back GETRES_LATEST
			- TODO: If key not exist, send back GETRES_DELETED
- GETRES_LATEST_SEQ
	+ TODO: Server sends GETRES_LATEST_SEQ
	+ TODO: Ingress: GETRES_LATEST_SEQ -> GETRES_LATEST_SEQ_INSWITCH (is_cached, idx)
	+ Egress
		* TODO: If inswitch_hdr.is_cached=1 and inswitch_hdr.valid=1 and latest=0
			+ Set latest=1, delete=0, update savedseq, update vallen and value, forward GETRES to client
- GETRES_DELETED_SEQ
	+ TODO: Server sends GETRES_DELETED_SEQ
	+ TODO: Ingress: GETRES_DELETED_SEQ -> GETRES_DELETED_SEQ_INSWITCH (is_cached, idx)
	+ Egress
		* TODO: If inswitch_hdr.is_cached=1 and inswitch_hdr.valid=1 and latest=0
			+ Set latest=1, deleted=1, update savedseq, update vallen and value, forward GETRES to client
- TODO: Cache population also updates savedseq
- TODO: If cache crashes, server should reset all seq as zero after recovery such that switch-assigned seq (>=1) must be larger

## Implementation log

- Copy netreach-voting-v3-memory to netreach-v4
- Remove unnecessary files
	+ \*.bak and \*.custom files in tofino/
- Use int32_t instead of uint64_t for key, as switch needs key for cache lookup, and compare key for range partition (key.\*, parser.\*)
	+ NOTE: we convert each 4B from little-endian into big-endian when serializing, and convert back when deserializing; so when switch OS receives key from controller, ptf will automatically perform endian conversion without extra processing
	+ TODO: test to_model_key in xindex_model_impl.h
+ Use int8_t instead of uint8_t for optype (packet_format.\*, ycsb/parser.\*)
- Remove host-side hashidx (packet_format.\*)
- Implement GET
	+ Support GETREQ in client, switch, and server (packet_format.\*, ycsb_remote_client.c, \*.p4)
	+ TODO: Support GETREQ_NLATEST
	+ TODO: Support GETRES_LATEST_SEQ
	+ TODO: Support GETRES_DELETED_SEQ

## Simple test

## Fixed issues
