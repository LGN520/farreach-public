# Tofino-based NetREACH (voting-based) + DPDK-based XIndex with persistence + variable-length key-value pair (netreach-voting-v2)

## Workflow (NOT Completed)

- client: GETREQ -> switch (if zerovote): change GETREQ to GETREQ_POP -> server: sendback GETRES/GETRES_NPOP, and trigger popultation to controller
- client: GETREQ -> switch (cached yet not latest): change GETREQ to GETREQ_NLATEST -> server: sendback: GETRES_LATEST/GETRES_NEXIST
- client: GETREQ -> switch (cached or deleted): change GETREQ to GETRES and sendback to client
- client: PUTREQ -> switch (if zerovote): change PUTREQ to PUTREQ_POP -> server: sendback PUTRES, and trigger population to controller
- client: PUTREQ -> switch (cached): put value, set latest=1, change PUTREQ to PUTRES and sendback to client
- client: DELREQ -> switch (cached): set latest=2 (deleted), change DELREQ to DELREQ and sendback to client
- other packets or being evicted: forward to client
- Design features
	+ Parameter-free decision
	+ Data-plane-based value update
	+ Control-plane-based non-blocking cache population (conservative query and version-aware query for read-after-write consistency)
	+ TODO: Crash-consistent backup
		* Case 1: change on in-switch cache value; case 2: change on in-switch cache key-value pair; case 3: change on server KVS/CKVS
	+ Others
		* TODO: switch-driven consistent hashing
		* CBF-based fast path
		* TODO: Range query support
		* TODO: Variable-length key-value
		* TODO: Distributed extension
- NOTES
	+ We can use seq=0 to mark not latest in server, why not we use seq=0 to mark not latest in switch?
		* If we use seq reg to mark islatest, use deleted reg to mark isdeleted, then they have data dependency for GETRES_NEXIST, which 
		can only mark the slot is deleted if not latest
		* Therefore, we use latest reg to identify both islatest and isdeleted to reduce stage cost
		* NOTE: we use status=0/1/2 in CKVS of server for consistent semantics
	+ Ptf/cli will make conversion between big-endian and small-endian automatically as signed int
		* For vote, we write small-endian value in ptf, then data plane will save the big-endian format for calculation
		* For optype and vallen, since they only have 1 byte, they do not care about endianess
		* For value, we want data plane directly stores small-endian format
			- When reading value (phase2), since ptf falsely converts the value from small-endian into big-endian, we need to convert it back again
			- When writing value (cache_update), since ptf make conversion automatically, we write the big-endian format such that reg will store small-endian
		* TODO: For key, not sure whether ptf converts for MAT

## Important changes

- Use control-plane-assisted evition to reduce hardware resource usage
- Packet format
	+ GETREQ: optype, hashidx, key
	+ PUTREQ: optype, hashidx, key, vallen, value
	+ DELREQ: optype, hashidx, key
- In-switch processing
	+ Overview	
		* Stage 0: cache_lookup_tbl (get iscached), being_evicted (for inconsistency of evicted data), load_backup_tbl
			- NOTE: we do not need valid bit, switch OS maitains a global KVS to record hashidx-cachedkey mapping, which can be used
			to identify either cache population or eviction, and get previously cached key
		* Stage 1: vote, latest (for inconsistency of populated/deleted data), seq (for version-aware/conservative query), case 1, case 3 
			- NOTE: seq is created by switch (big-endian), so we need to deserialize seq into little-endian in server for GET/PUTREQ_BE
			- NOTE: case 1 represent both case 1 (value update of cached key) and case 2 (cache population)
		* Stage 2: lock, vallen
			- Case 1: PUT/DELREQ on switch for backup
			- Case 3: First PUTREQ on server for backup
			- Case 2: First eviction on switch for backup (since eviction is based on control plane now, controller can handle it)
		* Stage 3-10: val1-val16
		* Stage 11: port_forward_tbl
		* Logical meaning
			- latest=0: in-switch value may not be up-to-date (serve PUT, conservative GET)
				+ latest=1: in-switch value must be up-to-date (serve GET/PUT); 
				+ latest=2: value has been deleted (also up-to-date) (serve GET/PUT, GET directly returns NONE value)
			- being_evicted=1: in-switch value may not be up-to-date (serve nothing, all GET/PUT are forwarded to server)
				+ Data plane cannot modify any field of in-switch cache
	+ GETREQ: 
		* Stage 0
			- Access cache_lookup_tbl (get iscached)
			- Access being_evicted (get being_evicted)
		* Stage 1
			- Access vote
				+ Increase_vote if iscached=1 and being_evicted=0
				+ Decrease_vote if iscached=0 and and being_evicted=0
			- Read latest 
			- Read seq
		* Stage 2:
			- Access lock
				+ If iszerovote=2 and being_evcited=0, try_lock
				+ Otherwise, read_lock
		* Stage 2-10: Read vallen and values
		* Stage 11: access port_forward_tbl
			- If iscached=1 and islatest=1 and being_evicted=0 -> return GETRES; 
			- If iscached=1 and islatest=2 and being_evicted=0 -> return GETRES (vallen=0 and not value headers);
			- If iszerovote=2 and islock=0 and being_evicted=0 -> trigger population (GETREQ_POP); 
			- If iscached=1 and being_evicted=0 and islatest=0 -> forward GETREQ_NLATEST for conservative query (not latest, first GETs after population); 
			- If iscached=1 and being_evicted=1 -> forward GETREQ_BE for version-aware query;
			- Otherwise, forward GETREQ to server
			- NOTE: if being_evicted=1, islock may be 0 (set as 0 at phase 2); if iszerovote=2, iscached must be 0
	+ GETRES
		* Sendback to client
	+ GETRES_NPOP (no population)
		* If being_evicted=0, set lock=0; sendback to client as GETRES
		* NOTE: lock must be 1 and being_evicted must be 0
	+ GETRES_LATEST (exist for GETREQ_NLATEST)
		* If iscached=1 and latest=0 and being_evicted=0, set latest=1, vallen, and value; sendback to client as GETRES
	+ GETRES_NEXIST (not exist for GETREQ_NLATEST)
		* If iscached=1 and latest=0 and being_evicted=0, set latest=2 (being deleted); sendback to client as GETRES
	+ PUTREQ:
		* NOTE: other stages are the same as GETREQ
		* Stage 1
			- Access vote
				+ Increase_vote if iscached=1 and being_evicted=0
				+ Decrease_vote if iscached=0 and being_evicted=0
			- Set latest = 1 if iscached=1 and being_evicted=0
			- Access seq
				+ Increase seq if iscached=1 and being_evicted=0
				+ Read seq otherwise 
			- Access case1
				+ If iscached=1 and being_evicted=0 and isbackup=1, try_case1
		* Stage 2-10
			- Set vallen and values if iscached=1 and being_evicted=0
		* Stage 11: access port_forward_tbl
			- If iszerovote=2 and islock=0 and being_evicted=0 -> trigger population (PUTREQ_POP)
			- If iscached=1 and being_evicted=0, increase seq, set latest=1, update vallen and value, sendback PUTRES to client
				+ If iscached=1 and being_evicted=0 and isbackup=1 and iscase1=0, update PUTREQ as PUTREQ_CASE1 and sendback PUTRES
			- If iscached=1 and being_evicted=1, embed seq, send PUTREQ_BE to server (being evicted)
	+ DELREQ:
		* NOTE: other stages are the same as PUTREQ
		* Stage 1
			- Not touch vote
			- Set latest = 2 if iscached=1 and being_evicted=0
			- Access seq
				+ Increase seq if iscached=1 and being_evicted=0
				+ Read seq otherwise 
			- Access case1
				+ If iscached=1 and being_evicted=0 and isbackup=1, try_case1
		* Stage 2-10
			- Not touch vallen and value 
		* Stage 11: access port_forward_tbl
			- If iscached=1 and being_evicted=0, increase seq, set latest=2, sendback DELRES to client
				+ If iscached=1 and being_evicted=0 and isbackup=1 and iscase1=0, update DELREQ as DELREQ_CASE1 and sendback DELRES
			- If iscached=1 and being_evicted=1, embed seq, send DELREQ_BE to server (being evicted)
	+ PUTRES
		* Sendback to client
	+ Crash-consistent backup
		* NOTE: iscase1 represent both case 1 and case 2 (only the earliest one can trigger rollback once)
		* Case 1: PUTREQ / DELREQ
			- If iscached=1 and being_evicted=0 and isbackup=1 and iscase1=0, set iscase1=1, notify switch OS (server) with old value by PUTREQ_CASE1 / DELREQ_CASE1
			- NOTE: if being_evicted=1, we do not allow any change of the slot in data plane -> value update will not occur to change
			in-switch cache, which is forwarded to server
		+ TODO: If backup and cache population run in parallel (case 2, not yet now)
			* TODO: During cache population, if isbackup=1 and iscase1=0, save old value / empty value in redis with specific prefix,
			and set iscase1=1 before setting being_evicted=0
- Client-side processsing
	+ Use client-side consistent hashing for hashidx (baseline also needs it for routing)
- Server-side processing
	+ Receiver for normal packets; backuper for periodic backup; workers for server simulation; populator for cache population
	+ GETREQ_POP (population): 
		* If value exists in KVS, sendback GETRES, add key in CKVS with status = 0 and seq = 1, and [TODO] trigger population (k, v, hashidx) to controller
		* Otherwise, sendback GETRES_NPOP (no population) to unlock the entry
		* NOTE: key cannot exist in CKVS (key is cached so vote cannot be zero; DEL cannot invalidate the entry, which only sets a special status of value)
	+ GETREQ_NLATEST: 
		* If status == 0 in CKVS -> not latest 
			+ If found in KVS, update CKVS, and sendback GETRES_LATEST
			+ If not found in KVS, update CKVS, and sendback GETRES_NEXIST
		* If status == 1 in CKVS -> latest
			+ Sendback GETRES_LATEST
		* If status == 2 in CKVS -> latest yet deleted
			- Sendback GETRES_NEXIST
		* NOTE: key must exist in CKVS due to being_evicted = 0 for GETREQ_NLATEST
	+ GETREQ_BE (being evicted)
		* If key in CKVS
			* If seq (embedded in pkt) >= seq' (stored in CKVS), use val carried by request packet
			* Otherwise, sendback GETRES based on status in CKVS
				- If status == 1 or status == 2, use val in CKVS to sendback GETRES
				- Otherwise, use val in KVS to sendback GETRES, update CKVS
		* Otherwise
			* Get value from KVS and sendback GETRES
		* NOTE: key may not exist in CKVS since controller may remove the key before GETREQ_BE
	+ GETREQ: sendback GETRES
	+ PUTREQ_POP (population)
		* Add kv in KVS, sendback PUTRES, add key in CKVS with status = 0 and seq = 1, trigger population (k, v, hashidx) 
		* NOTE: key cannot exist in CKVS (key is cached so vote cannot be zero; DEL cannot invalidate the entry, which only sets a special status of value)
	+ PUTREQ_BE (being evicted)
		* If exist in CKVS, set val, status=1, and seq'=seq+1 (seq is that number embedded in the request packet), sendback PUTRES
		* Otherwise, put into KVS, sendback PUTRES
		* NOTE: key may not exist in CKVS since controller may remove the key before PUTREQ_BE
	+ PUTREQ: sendback PUTRES
	+ DELREQ: sendback DELRES
	+ DELREQ_BE
		* If exist in CKVS, set status=2, and seq'=seq+1 (seq is that number embedded in the request packet), sendback DELRES
		* Otherwise, delete from KVS, sendback DELRES
		* NOTE: key may not exist in CKVS since controller may remove the key before DELREQ_BE
	+ Cache population
		* Send <k, v, hashidx, threadid> to controller for GETREQ_POP and PUTREQ_POP
		* Populator: listen on a port to wait for controller's notification
			- Load latest, threadid, and evicted key with prefix of "hashidx:"
			- If latest=0
				+ Put entry from CKVS into KVS if status = 1, or delete it in KVS if status = 2, or nothing if status = 0
			- If latest=1
				+ If seq>=seq', put entry from notification into KVS
				+ If seq<seq', put entry from CKVS into KVS if status = 1, or delete it in KVS if status = 2, or nothing if status = 0
			- If latest=2
				+ If seq>=seq', delete in KVS 
				+ If seq<seq', put entry from CKVS into KVS if status = 1, or delete it in KVS if status = 2, or nothing if status = 0
			+ Remove it from CKVS with prefix of "hashidx:"
			+ Send ACK to controller
	+ Crash-consistent backup
		* TODO: Save PUTREQ_CASE1(_DELETED) and DELREQ_CASE1(_DELETED) as special cases
		* TODO: Rollback special cases (happened in switch OS in design as case 2)
- Controller-side processing
	+ controller/periodic_backup.py for periodic backup; controller/cache_update.py for cache population; controller/controller.py for combination of the two functionalities
	+ Cache population (controller/cache_update.py)
		+ NOTE: if cache is full, eviction is required; performing eviction in data plane for atomicity is resouce-consuming (PHV is proportional to value length) 
		+ Control-plane-based non-blocking population
			* Wait for population request from server (k, v, hashidx)
			* Set being_evicted bit at hashidx slot as 1
			* Check redis to see if there exists cached key in the slot
			* If not exist
				- Reset registers (vote=1, latest=0, lock=0, seq=0), set vallen and val, add cached key in redis and MAT
				- NOTE: Before being_evicted bit is set as 0, if new key is not added, all GETs and PUTs of the populated key are forwarded to server;
				if new key is added, all GETs/PUTs touch CKVS in server as GET_BE/PUT_BE -> out-of-date value in switch;
				- NOTE: Forward the first GETs to server if latest = 0 (for read-after-write consistency of populated data)
			* If exist
				- Load evicted key and thread id from redis, latest
					+ If latest=0
						- Send evicted key
					+ If latest=1
						- Load seq, vallen, and val
						- Send evicted key, seq, vallen, and val
						- Server removes it from CKVS, updates KVS with the lastest version (seq/seq'), and ACKs
					+ If latest=2
						- Load seq
						- Send evicted key, seq
						- Server removes it from CKVS, deletes it from KVS if in-switch seq >= server-side seq' or updates KVS as CKVS, and ACKs
				- Wait for ACK (for read-after-write consistency of evicted data)
				- Remove evicted key from redis
				- Reset registers (vote=1, latest=0, lock=0, seq=0), set vallen and val, add cached key in redis and MAT
			* Set being_evcited bit as 0
		+ TODO: check big-endian and small-endian
	+ Crash-consistent backup
		* Phase 1
			+ Clean up case1 and case3
			+ TODO: If backup and cache population run in parallel (case2, not yet now)
				* TODO: Clean up old values / empty values in redis with specific prefix
			+ Mark isbackup = 1
		* Phase 2
			+ Load all key-value pairs
			+ Reset isbackup = 0
			+ Filter key-value pairs with latest=1/2
			+ TODO: If backup and cache population run in parallel (case2, not yet now)
				* TODO: Load old values / emtpy values from redis with specific prefixes to rollback backup data
			+ Send backup data to server
	+ TODO: periodically check lock bit, if it is 1 for the same key across two adjacent periods, unlock it (for packet loss of GETRES_NS)

## Implementation log

- Client side: ycsb_remote_client.c
	- Support GETREQ, GETRES, PUTREQ, PUTRES
- Packet type support
	- Switch side: tofino/\*.p4; Server side: ycsb_server.c; Utils: packet_format.h, packet_format_impl,h, cache_val.h, cache_val.c
	- Support GETREQ, PUTREQ, and DELREQ for normal packets
	- Support GETREQ_POP, GETRES, GETRES_NPOP for get-triggerred eviction
	- Support GETREQ_NLATEST, GETRES_LATEST, GETRES_NEXIST for conservative query
	- Support PUTREQ_POP, PUTRES for put-triggerred eviction
	- Support GETREQ_BE and PUTREQ_BE and DELREQ_BE for version-aware query
	- Support CacheVal including val, deleted bool, and key
- Support non-blocking cache population
	- Controller part (controller/cache_update.py, cache_update/register_update.py)
	- Server part (ycsb_server.c)
- Support crash-consistent backup
	- Controller part (controller/periodic_backup.py, phase2/read_register.py)
	- Switch part (port_forward_tbl in ingress.p4)
	- Server part (ycsb_server.c)
	- Support case 1
	- TODO: Support case 2 and case 3
- TODO: apply to baseline
	+ TODO: replace thread_id with hashidx (packet_format.h, packet_foramt_impl.h, ycsb_remove_client.c)

## TODO: Simple test

- Test case 1: 
	+ GETREQ k1 -> GETREQ_S k1 -> eviction k1-v1 (latest=0, vote=1)
	+ GETREQ k1 -> GETREQ k1 (vote=2) -> GETRES k1-v1 (latest=1)
	+ GETREQ k1 -> GETRES k1 (vote=3)
	+ GETREQ k2 -> GETREQ k2 (vote=2)
	+ GETREQ k2 -> GETREQ k2 (vote=1)
	+ GETREQ k2 -> GETREQ k2 (vote=0) -> eviction k2-v2 (latest=0, vote1)

## How to run

- Microbenchmark (TBD)
	- Prepare randomly-generated keys
		+ NOTE: we direclty use makefile to enable DPDK (to detet ports) without cmake
		+ `make all`
		+ `./prepare`
	- Run `bash start_server.sh` in server host
	- Run `bash start_client.sh` in client host
- YCSB
	- Prepare workload for loading or transaction phase
		+ For example:
		+ `./bin/ycsb.sh load basic -P workloads/workloada -P netbuffer.dat > workloada-load.out`
		+ `./bin/ycsb.sh run basic -P workloads/workloada -P netbuffer.dat > workloada-run.out`
		+ `./split_workload load`
		+ `./split_workload run`
	- `./ycsb_local_client` for loading phase
	- `./ycsb_server` for server-side in transaction phase
	- `./ycsb_remote_client` for client-side in transaction phase
	- Directory structure
		+ Raw workload file: workloada-load.out, workloada-run.out
		+ Split workload file: e.g., workloada-load-5/2.out
		+ Database directory: e.g., /tmp/netbuffer/workloada/group0.db, /tmp/netbuffer/workloada/buffer0-0.db
		+ RMI model at root node when init key-value store: workloada-root.out
- Switch
	- Run `cd tofino`
	+ Run `su` to enter root account
	+ Run `bash compile.sh` to compile p4 into binary code
	+ Run `bash start_switch.sh` to launch Tofino
	+ Create a new terminal and run `bash configure.sh` to configure data plane
	+ `bash controller.sh setup`
	+ END: `bash controller.sh cleanup`

## Fixed issues

- Error: Not support branching from parser to egress pipeline
	+ Solution: do not use a same MAT in both ingress and egress pipelines
- NOTE: if MAT entries are too many, the MAT will be placed into multiple stages
