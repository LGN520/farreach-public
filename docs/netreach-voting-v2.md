# Tofino-based NetREACH (voting-based) + DPDK-based XIndex with persistence + variable-length key-value pair (netreach-voting-v2)

## Workflow (NOT Completed)

- client: GETREQ -> switch (if invalid or zerovote): change GETREQ to GETREQ_POP -> server: sendback GETRES/GETRES_NPOP, and trigger popultation to controller
- client: GETREQ -> switch (valid yet not latest): change GETREQ to GETREQ_NLATEST -> server: sendback: GETRES_LATEST/GETRES_NEXIST
- client: GETREQ -> switch (valid or deleted): change GETREQ to GETRES and sendback to client
- client: PUTREQ -> switch (if invalid or zerovote): change PUTREQ to PUTREQ_POP -> server: sendback PUTRES, and trigger population to controller
- client: PUTREQ -> switch (valid): put value, set latest=1, change PUTREQ to PUTRES and sendback to client
- client: DELREQ -> switch (valid): set latest=2 (deleted), change DELREQ to DELREQ and sendback to client
- other packets or being evicted: forward to client
- Design features
	+ Parameter-free decision
	+ Data-plane-based value update
	+ Control-plane-based non-blocking cache population (conservative query and version-aware query for read-after-write consistency)
	+ Crash-consistent backup
	+ Others: switch-driven consistent hashing, CBF-based fast path, range query support, distributed extension
- NOTES
	+ We can use seq=0 to mark not latest in server, why not we use seq=0 to mark not latest in switch?
		* If we use seq reg to mark islatest, use deleted reg to mark isdeleted, then they have data dependency for GETRES_NEXIST, which 
		can only mark the slot is deleted if not latest
		* Therefore, we use latest reg to identify both islatest and isdeleted to reduce stage cost
		* NOTE: we use status=0/1/2 in CKVS of server for consistent semantics

## Important changes

- Use control-plane-assisted evition to reduce hardware resource usage
- Packet format
	+ GETREQ: optype, hashidx, key
	+ PUTREQ: optype, hashidx, key, vallen, value
	+ DELREQ: optype, hashidx, key
- In-switch processing
	+ Overview	
		* Stage 0: valid, cache_lookup_tbl (get iscached), being_evicted (for inconsistency of evicted data), load_backup_tbl
		* Stage 1: vote, latest (for inconsistency of populated/deleted data), seq (for version-aware/conservative query), case 1, case 2 
		* Stage 2: lock, vallen
			- Case 1: PUT/DELREQ on switch for backup
			- Case 3: First PUTREQ on server for backup
			- Case 2: First eviction on switch for backup (since eviction is based on control plane now, controller can handle it)
		* Stage 3-10: val1-val16
		* Stage 11: port_forward_tbl
		* Logical meaning
			- valid=1: valid value of cached key (up-to-date or out-of-date); valid=0: invalid value (default value of register)
				+ Deleted value (i.e., NONE) can be either up-to-date or out-of-date
			- latest=0: in-switch value may not be up-to-date (serve PUT, conservative GET)
				+ latest=1: in-switch value must be up-to-date (serve GET/PUT); 
				+ latest=2: value has been deleted (also up-to-date) (serve GET/PUT, GET directly returns NONE value)
			- being_evicted=1: in-switch value may not be up-to-date (serve nothing, all GET/PUT are forwarded to server)
				+ Data plane cannot modify any field of in-switch cache
	+ GETREQ: 
		* Stage 0
			- Access cache_lookup_tbl (get iscached)
			- Access valid (get_valid)
			- Access being_evicted (get being_evicted)
		* Stage 1
			- Access vote
				+ Increase_vote if iscached=1 and isvalid = 1 and being_evicted=0
				+ Decrease_vote if iscached=0 and isvalid = 1 and being_evicted=0
			- Read latest 
			- Read seq
		* Stage 2:
			- Access lock
				+ If isvalid=0 and being_evicted=0, or iszerovote=2 and being_evcited=0, try_lock
				+ Otherwise, read_lock
		* Stage 2-10: Read vallen and values
		* Stage 11: access port_forward_tbl
			- If iscached=1 and isvalid=1 and islatest=1 and being_evicted=0 -> return GETRES; 
			- If iscached=1 and isvalid=1 and islatest=2 and being_evicted=0 -> return GETRES (vallen=0 and not value headers);
			- If isvalid=0 and islock=0 and being_evicted=0, or iszerovote=2 and islock=0 and being_evicted=0 -> trigger population (GETREQ_POP); 
			- If iscached=1 and isvalid=1 and being_evicted=0 and islatest=0 -> forward GETREQ_NLATEST for conservative query (not latest, first GETs after population); 
			- If iscached=1 and isvalid=1 and being_evicted=1 -> forward GETREQ_BE for version-aware query;
			- Otherwise, forward GETREQ to server
			- NOTE: if being_evicted=1, islock may be 0 (set as 0 at phase 2); if iszerovote=2, iscached must be 0
	+ GETRES
		* Sendback to client
	+ GETRES_NPOP (no population)
		* If being_evicted=0, set lock=0; sendback to client as GETRES
		* NOTE: lock must be 1 and being_evicted must be 0
	+ GETRES_LATEST (exist for GETREQ_NLATEST)
		* If iscached=1 and isvalid = 1 and latest=0 and being_evicted=0, set latest=1, vallen, and value; sendback to client as GETRES
	+ GETRES_NEXIST (not exist for GETREQ_NLATEST)
		* If iscached=1 and isvalid = 1 and latest=0 and being_evicted=0, set latest=2 (being deleted); sendback to client as GETRES
	+ TODO: PUTREQ:
		* NOTE: other stages are the same as GETREQ
		* Stage 1
			- Access vote
				+ Increase_vote if iscached=1 and isvalid = 1 and being_evicted=0
				+ Decrease_vote if iscached=0 and isvalid = 1 and being_evicted=0
			- Set latest = 1 if iscached=1 and isvalid=1 and being_evicted=0
		* Stage 2-10
			- Set vallen and values if iscached=1 and isvalid=1 and being_evicted=0
		* Stage 11: access port_forward_tbl
			- If isvalid=0 and islock=0 and being_evicted=0, or iszerovote=2 and islock=0 and being_evicted=0 -> trigger population (PUTREQ_POP)
			- If iscached=1 and isvalid=1 and being_evicted=0, increase seq, set latest=1, update vallen and value, sendback PUTRES to client
			- TODO: If iscached = 1 and isvalid = 1 and being_evicted = 1, embed seq, send PUTREQ_BE to server (being evicted)
	+ PUTRES
		* Sendback to client
- Client-side processsing
	+ Use client-side consistent hashing for hashidx (baseline also needs it for routing)
- Server-side processing
	+ Receiver for normal packets; backuper for periodic backup; workers for server simulation; TODO: populator for cache population
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
	+ GETREQ_BE (being evicted)
		* If seq (embedded in pkt) >= seq' (stored in CKVS), use val carried by request packet
		* Otherwise, sendback GETRES based on status in CKVS
			- If status == 1 or status == 2, use val in CKVS to sendback GETRES
			- Otherwise, use val in KVS to sendback GETRES, update CKVS
	+ GETREQ: sendback GETRES
	+ PUTREQ_POP (population)
		* Add kv in KVS, sendback PUTRES, add key in CKVS with status = 0 and seq = 1, trigger population (k, v, hashidx) 
		* NOTE: key cannot exist in CKVS (key is cached so vote cannot be zero; DEL cannot invalidate the entry, which only sets a special status of value)
	+ TODO: PUTREQ_BE (being evicted)
		* TODO: If exist in CKVS, set val, status=1, and seq'=seq+1 (seq is that number embedded in the request packet), sendback PUTRES
		* TODO: Otherwise, put into KVS, sendback PUTRES
	+ PUTREQ: sendback PUTRES
- Controller-side processing
	+ controller/periodic_backup.py for periodic backup; controller/cache_update.py for cache population
	+ TODO: if cache is full, eviction is required; performing eviction in data plane for atomicity is resouce-consuming (proportional to value length) 
	-> control-plane-based atomic population
		* Set being_evicted bit as 1
		* Load valid bit
		* If valid = 0
			- Reset registers (vote=1, latest=0, case1=0, case2=0, lock=0, seq=0), set vallen and val, add cached key, set valid=1
			- NOTE: Before being_evicted bit is set as 0, all GETs and PUTs of the populated key are forwarded to server -> out-of-date value in switch
			- NOTE: Forward the first GETs to server if latest = 0 (for read-after-write consistency of populated data)
		* If valid = 1
			- Load seq, latest, valen and val, send evicted key, value, and seq to server, wait for ACK (for read-after-write consistency of evicted data)
				+ If latest=2, tell server to delete it if in-switch seq >= server-side seq' 
			- Remove cached key, reset registers (vote=1, latest=0, case1=0, case2=0, lock=0), set vallen and val, add cached key, set valid=1
		* Set being_evcited bit as 0
	+ TODO: periodically check lock bit, if it is 1 for the same key across two adjacent periods, unlock it (for packet loss of GETRES_NS)

## Implementation log

- Client side: ycsb_remote_client.c
	- Support GETREQ, GETRES, PUTREQ, PUTRES
- Packet type support
	- Switch side: tofino/\*.p4; Server side: ycsb_server.c; Utils: packet_format.h, packet_format_impl,h, cache_val.h, cache_val.c
	- Support GETREQ, GETREQ_POP, GETRES, GETRES_NPOP for get-triggerred eviction
	- Support GETREQ_NLATEST, GETRES_LATEST, GETRES_NEXIST for conservative query
	- Support PUTREQ, PUTREQ_POP, PUTRES
	- Support GETREQ_BE for version-aware query
	- Support CacheVal including val, deleted bool, and key
- TODO: PUTREQ_BE
- TODO: DEL, DELREQ_BE
- TODO: Support cache update in controller (TODO: non-blocking population)
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
