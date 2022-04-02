# Tofino-based NetREACH + DPDK-based XIndex (in-memory KVS) + variable-length key-value pair w/ snapshot + control-plane-based cache update (netreach-v4)

- Copy from netreach-voting-v3-memory, refer to netreach-voting-v2

## Overview

- Packet format
	+ Ethernet + IP + UDP
	+ Operation header: 1B optype, 16B key
	+ Value header: 4B vallen, variable length value (8B padding if vallen <= 128B)
	+ Result header: 1B result
		* NOTE: success flag for PUTRES/DELRES; deleted flag for GETRES; SCANRES does not need it
	+ Inswitch header: 1b is_cached, 1b is_sampled, 6b padding, 2B hashidx, 2B idx
- Client
- Switch
	+ Ingress pipeline
		* Stage 0
			- hash_for_partition_tbl (optype, key -> hashval)
				+ Hashval: partition across servers
			- hash_for_CM_tbl (optype, key -> hashidx)
				+ hashidx: index for CM
			- lookup_tbl (optype, key -> idx, is_cached)
				+ idx: index for in-switch cache (assigned by controller)
			- sample_tbl (optype, key -> is_sampled)
			- TODO: In distributed extension, spine/leaf switch uses idx to access the bucket in the corresponding virtual switch (controller has considered partition across virtual spine/leaf switches when assigning idx)
				+ TODO: Only leaf switch needs to perform hash_for_partition_tbl for partition across servers
				+ TODO; Only spine switch needs to assign seq
				+ NOTE: we enforce spine-leaf path for each key to ensure serializability and availability, which can be statically configured into switches (no hash operation in client); however, DistCache uses dynamic mechanism (power-of-two-choices) for load balance, client needs to perform hash operation
			- NOTE: SCANREQ does not access all above tables
		* Stage 1: partition_tbl
			- Hash partition: (optype, hashval range -> udp.dstPort, egress_port)
			- TODO: range partition: (optype, key range -> udp.dstPort, egress_port)
				+ TODO: We treat the most significant 4B of key as int32_t for range matching -> need conversion between little-endian and small-endian for the 4B of each key
			- NOTE: we use udp.dstPort to simulate different egress ports
	+ Egress pipeline
		* Stage 0
			- CM: 4 register arrays of 16b (optype, is_sampled, is_cached, hashidx -> is_hot)
		* Stage 1:
			- cache_frequency_tbl (optype, is_sampled, is_cached, idx -> none)
			- status_tbl (optype, is_cached, idx -> status)
				+ 8b status (from highest to lowest): 5b padding, 1b deleted, 1b latest, 1b valid
				+ Masks: valid (0x1); latest (0x2); deleted (0x4)
				+ TODO: PUTREQ_LARGE should set status.valid=0 and status.latest=1 (status of large value in server is latest)
		* Stage 1: 
			- vallen, vallo1, valhi1, vallo2 (optype, is_cached, idx -> vallen_hdr and val_hdr)
		* Stage 2-8:
			- From valhi2 to vallo16 (optype, is_cached, idx -> vallen_hdr and val_hdr)
		* Stage 9:
			- valhi16 (optype, is_cached, idx -> vallen_hdr and val_hdr)
		* NOTE: # of buckets in CM is dependent with other register arrays (# of cache buckets)
- Server
- Controller
- SwitchOS

## Details 

- GETREQ
	+ TODO: Client sends GETREQ
	+ TODO: Ingress: GETREQ -> GETREQ_INSWITCH (is_sampled, is_cached, hashidx, idx)
	+ Egress
		* TODO: If inswitch_hdr.is_sampled=1 and inswitch_hdr.is_cached=0, update CM and forward GETREQ to server
			- TODO: If any counter in CM >= HH_THRESHOLD, forward GETREQ_POP to server (processed as below)
			- NOTE: we resort to server to notify controller to avoid in-switch BF; NetCache directly reports to server yet with BF
		- TOOD: If inswitch_hdr.is_sampled=1 and inswitch_hdr.is_cached=1, update per-record frequency counter
		* TODO: If inswitch_hdr.is_cached=1
			- TOOD: If status.valid=0, forward GETREQ to server
			- If status.valid=1
				+ If status.latest=1 and status.deleted=1, set result_hdr.result=0 as deleted and send back GETRES
				+ If status.latest=1 and status.deleted=0, read value and send back GETRES
				+ If status.latest=0, forward GETREQ_NLATEST to server (processed as below)
	+ Server
		* TODO: GETREQ -> GETRES
		* GETREQ_POP
			- TODO: If key not exist or vallen > 128B, send back GETRES and ignore cache population
			- TODO: Otherwise, send back GETRES and notify controller for cache population
		* GETREQ_NLATEST
			- TODO: If key exists and vallen > 128B, send back GETRES
			- TODO: If key exists and vallen <= 128B, send back GETRES_LATEST
			- TODO: If key not exist, send back GETRES_DELETED
- GETRES_LATEST
	+ TODO: Server sends GETRES_LATEST
	+ TODO: Ingress: GETRES_LATEST -> GETRES_LATEST_INSWITCH (is_cached, idx)
	+ Egress
		* TODO: If inswitch_hdr.is_cached=1 and inswitch_hdr.status.valid=1 and status.latest=0
			+ NOTE: valid=1 means no large value in server; latest=0 means no PUTREQ/PUTREQ_LARGE/DELREQ arrive at switch
			+ Set status.latest=1, status.delete=0, update vallen and value, forward GETRES to client
- GETRES_DELETED
	+ TODO: Server sends GETRES_DELETED
	+ TODO: Ingress: GETRES_DELETED -> GETRES_DELETED_INSWITCH (is_cached, idx)
	+ Egress
		* TODO: If inswitch_hdr.is_cached=1 and inswitch_hdr.status.valid=1 and status.latest=0
			+ NOTE: valid=1 means no large value in server; latest=0 means no PUTREQ/PUTREQ_LARGE/DELREQ arrive at switch
			+ Set status.latest=1, status.deleted=1, update vallen and value, forward GETRES to client

## Implementation log

- Copy netreach-voting-v3-memory to netreach-v4

## Simple test

## Fixed issues
