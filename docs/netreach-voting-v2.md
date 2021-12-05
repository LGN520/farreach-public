# Tofino-based NetREACH (voting-based) + DPDK-based XIndex with persistence + variable-length key-value pair (netreach-voting-v2)

## Important changes

- Use control-plane-assisted evition to reduce hardware resource usage
- Packet format
	+ GETREQ: optype, hashidx, key
	+ PUTREQ: optype, hashidx, key, vallen, value
	+ DELREQ: optype, hashidx, key
- In-switch processing
	+ Overview	
		* Stage 0: valid, cache_lookup_tbl (get iscached), being_evicted (for inconsistency of evicted data), load_backup_tbl
		* Stage 1: vote, latest (for inconsistency of populated/deleted data), seq (for packet loss of PUT), case 1, case 2 
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
				+ latest=3: value has been deleted (also up-to-date) (serve GET/PUT, GET directly returns NONE value)
			- being_evicted=1: in-switch value may not be up-to-date (serve nothing, all GET/PUT are forwarded to server)
				+ Data plane cannot modify any field of in-switch cache
	+ GETREQ: 
		* Stage 0
			- Access cache_lookup_tbl (get iscached)
			- Access valid (get_valid)
			- Access being_evicted (get being_evicted)
		* Stage 1
			- Access vote (increase_vote if iscached=1 and being_evicted=0; decrease_vote if iscached=0 and being_evicted=0)
			- Access_latest (read_latest)
		* Stage 2:
			- Access lock (if isvalid=0 and being_evicted=0, or iszerovote=2 and being_evcited=0, try_lock; otherwise, read_lock); 
		* Stage 2-10: Read vallen and values
		* Stage 11: access port_forward_tbl (if iscached=1 and isvalid=1 and islatest=1 and being_evicted=0 -> return GETRES; 
		if isvalid=0 and islock=0 and being_evicted=0, or iszerovote=2 and islock=0 and being_evicted=0 -> trigger population (GETREQ_POP); 
		if iscached=1 and isvalid=1 and being_evicted=0 and islatest=0 -> forward GETREQ_NLATEST (not latest, first GETs after population); 
		otherwise, forward GETREQ to server)
			- NOTE: if being_evicted=1, islock may be 0 (set as 0 at phase 2); if iszerovote=2, iscached must be 0
	+ GETRES
		* Sendback to client
	+ GETRES_NPOP (no population)
		* If being_evicted=0, set lock=0; sendback to client as GETRES
		* NOTE: lock must be 1 and being_evicted must be 0
	+ TODO: GETRES_NEXIST (not exist)
		* TODO: if iscached=1 and latest=0 and being_evicted=0, set valid=0
		* NOTE: if being_evicted = 1, GETREQ carry val due to valid = 1; if server does not have the key in cached list, reply GETRES without data
	+ TODO: PUTREQ:
		* TODO: if iscached = 1 and performed in switch, increase seq
		* TODO: if iscached = 1 and forwarded to server (being_evicted), pass seq+1 along with PUTs/DELs
- Client-side processsing
	+ Use client-side consistent hashing for hashidx (baseline also needs it for routing)
- Server-side processing
	+ Receiver for normal packets; backuper for periodic backup; workers for server simulation; TODO: populator for cache population
	+ GETREQ_POP (population): 
		* If value exists in KVS, sendback GETRES, add key in CKVS with seq = 1, and [TODO] trigger population (k, v, hashidx) to controller
		* Otherwise, sendback GETRES_NPOP (no population) to unlock the entry
		* NOTE: key cannot exist in CKVS (key is cached so vote cannot be zero; DEL cannot invalidate the entry, which only sets a special status of value)
	+ TODO: GETREQ_NLATEST: sendback GETRES_LATEST (if with value in CKVS) / GETRES_NEXT (if no value in CKVS)
	+ GETREQ: sendback GETRES
	+ TODO: PUTREQ_POP (population)
		* TODO: If key already exists in CKVS, the status must be deleted. In this case, we can use PUTRES_POP to validate the entry
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

- Switch side: tofino/\*.p4
	- Support GETREQ, GETREQ_POP, GETRES, GETRES_NPOP
- Client side: ycsb_remote_client.c
	- Support GETREQ, GETRES
- Server side: ycsb_server.c, cache_val.h, cache_val.c
	- Support GETREQ, GETREQ_POP, GETRES, GETRES_NPOP
- Utils: packet_format.h, packet_format_impl,h, cache_val.h, cache_val.c
	- Support GETREQ, GETREQ_POP, GETRES, GETRES_NPOP
	- Support CacheVal including val, deleted bool, and key
- TODO: GETREQ_NLATEST, GETRES_LATEST, GETRES_NEXIST
- TODO: PUTREQ, PUTRES
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
