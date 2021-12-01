# Tofino-based NetREACH (voting-based) + DPDK-based XIndex with persistence + variable-length key-value pair (netreach-voting-v2)

## Important changes

- Use control-plane-assisted evition to reduce hardware resource usage
- Packet format
	+ GETREQ: optype, hashidx, key
	+ PUTREQ: optype, hashidx, key, vallen, value
	+ DELREQ: optype, hashidx, key
- For eviction of invalid entry
	+ TODO: server sends an eviction packet (k, v, hashidx) to controller
	+ TODO: server adds an entry in cache_lookup_tbl (check if key is cached)
- In-switch processing
	+ Overview	
		* Stage 0: calculate_hashidx_tbl, cache_lookup_tbl
		* Stage 1: valid, vote, being_evicted (for inconsistency of evicted data)
		* Stage 2: lock, vallen, case (case=1 -> case1; case=2 -> case3; case=3 -> case1&3), latest (for inconsistency of populated data)
			- Case 1: PUT/DELREQ on switch for backup
			- Case 3: First PUTREQ on server for backup
			- Case 2: First eviction on switch for backup (since each eviction is forwarded to server now, do not need case 2)
		* Stage 3-10: val1-val16
		* Stage 11: port_forward_tbl
	+ GETREQ: 
		* Stage 0: calculate hashidx (save into other_hdr.hashidx); access cache_lookup_tbl (get iscached and being_evicted)
		* Stage 1: access valid (get_valid) and vote (increase_vote if iscached=1 and being_evicted=0; decrease_vote if iscached=0 and being_evicted=0)
		* Stage 2: access lock (if isvalid=0 and being_evicted=0, or iszerovote=2 and being_evcited=0, try_lock; otherwise, read_lock); 
		access_latest (read_latest)
		* Stage 2-10: Read vallen and values
		* Stage 11: access port_forward_tbl (if iscached=1 and isvalid=1 and islatest=1 and being_evicted=0 -> return GETRES; if isvalid=0 and islock=0, or 
		iszerovote=2 and islock=0 and being_evicted=0 -> trigger eviction; otherwise, forward to server)
			- NOTE: if being_evicted=1, islock may be 0 (set at phase 2); if iszerovote=2, iscached must be 0
- Server-side processing
	+ TODO: GETREQ: sendback GETRES
	+ TODO: GETREQ_S: (1) parse optype in receiver; (2) sendback GETRES; (3) trigger cache update to controller
- Controller-side processing
	+ TODO: two-phase eviction
		* Phase 1
			* Set MATh with being_evicted as 1 (cannot use valid bit directly: if read valid=1 then set valid=0, there might be DELREQ between read and set)
				- If being_evicted = 1, cannot modify any field in switch
			* Load <k1, v1', valid bit>
		* Phase 2
			* Set register value (val, vallen, valid=1, vote=1, lock=0, latest=0)
			* Then set MAT with being_evicted of 1

## Implementation log

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
