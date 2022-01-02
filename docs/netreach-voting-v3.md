# Tofino-based NetREACH (voting-based) + DPDK-based XIndex with persistence + variable-length key-value pair (netreach-voting-v3)

- Client calculates op_hdr.hashidx as slot index to reduce 1 stage (similar as OmniMon)
	+ We calculate meta.hashidx for hash partition in switch
- Offload some functions to egress pipeline to reduce stages
- Using 4B for each unit of key instead of 2B to reduce 1 stage

## Overview

- Design features
	+ Parameter-free decision
		* Existing: large parameter -> miss hot keys; small parameter -> too many hot keys -> insufficient cache capacity and switch OS bottleneck
		* Challenge: Slow warmup with incast populations -> data-plane-based cache population
			- NOTE: Mirroring GETRES_POP_EVICT/PUTREQ_POP_EVICT does not incur switch OS bottleneck since we do not need send old key-value pair
			to server if the entry is invalid during warmup phase
	+ Data-plane-based cache update
		* Data-plane-based value update
		* Data-plane-based cache population
			- GETREQ: response-based cache population
			- PUTREQ: recirculation-based cache population
			- Atomicity: 
				+ Use lock bit + recirculation-based blocking
				+ Use seq to cope with packet reordering of PUT
				+ Controller periodically checks lock bit to cope with rare packet loss
			- NOTE: recirculation is acceptable
				+ We do not block normal packets
				+ NetReach packets must be hashed into the same slot with different keys of currently cached one
				+ Population time is limited: us-level for GET and ns-level for PUT
				+ # of blocked packets is limited by # of clients (e.g., one thread in a host) due to one-by-one pattern of NetReach client library
			- NOTE: recirculation-based blocking guarantees that version of switch >= server
	+ Crash-consistent backup
		* Case 1: change on in-switch cache value;
		* Case 2: change on in-switch cache key-value pair;
		* TODO: Case 3: change on server KVS for range query;
	+ TODO: Others
		* CBF-based fast path
		* Range query support
		* Switch-driven consistent hashing
		* Distributed extension
		* Variable-length key-value
- Term routine
	+ Match: just compare without changing packet header field
	+ Get: load to change packet header field (or metadata field)
	+ Set: store new value without changing packet header field
	+ Set_and_get: store new value and change packet header field with old value
	+ Reset: reset value as 0
	+ Init: initialize value as 1
- Baselines
- NOTES

## Details 

- Packet format
	+ op_hdr: 1B optype, 2B hashidx, 16B key
	+ val_hdr: 1B vallen, 128B value
- In-switch processing
	+ Overview
		* Stage 0: keylolo, keylohi, keyhilo, keyhihi, load_backup_tbl
			- For key, we provide two operations: match, set_and_get
		* Stage 1: valid, vote, seq (assign only if key matches for PUT/DELREQ), update_iskeymatch_tbl
		* Stage 2: savedseq, lock, 
		* Stage 3: vallen, vallo1, valhi1, case12
			- For vallen and val, we provide two operations: get, set_and_get
		* Stage 4-10: from val2 to val15
		* Stage 11: vallo16, valhi16, case3, port_forward_tbl
			- For case2, if backup=1 and iscase12=0
				+ Update GETRES_POP with old key-value as GETRES_POP_EVICT_CASE2 to server, clone_i2e for GETRES with new key-value
				+ Update PUTREQ_POP with old key-value as PUTREQ_POP_EVICT_CASE2 to server, clone_i2e for PUTRES with new key
			- For case3, if backup=1
				+ Embed other_hdr (valid, case3) at the end of PUT/DELREQ
					+ Update PUTREQ with new key-value as PUTREQ_CASEV
					+ Update DELREQ without key-value as DELREQ_CASENV
		* Egress pipeline
			- For PUTREQ_CASEV/DELREQ_CASENV
				+ If case3=0, update it as PUT/DELREQ_CASE3 to server
				+ Otherwise, update it as PUT/DELREQ to server
			- For cloned packet
				+ If PUTREQ/DELREQ/PUTREQ_RECIR/DELREQ_RECIR, update as PUT/DELRES/PUTRES to client
				+ If GETRES_POP (with new key-value), update it as GETRES to client
				+ If PUTREQ_POP (with new key-value), update it as PUTRES to client
			- Hash parition for normal REQ pacekts
	+ GETREQ
		* Stage 0: match key 
		* Stage 1
			- Get valid
			- Update vote: if key matches, increase vote; otherwise, decrease vote
			- Update iskeymatch
		* Stage 2
			- Access lock: if valid=0 or zerovote=2, try_lock; otherwise, read_lock
		* Stage 3-11: get vallen and value
		* Stage 11: port_forward
			- If (valid=0 or zerovote=2) and lock=0, update GETREQ to GETREQ_POP -> hash_partition_tbl
			- If valid=1 and iskeymatch=1, update GETREQ to GETRES
			- If (valid=0 or iskeymatch=0) and lock=1, recirculate GETREQ
			- Otherwise, forward GETREQ to server -> hash_partition_tbl
	+ GETRES_POP
		* Stage 0: set_and_get key
		* Stage 1: set valid=1, vote=1
		* Stage 2: set savedseq=0, lock=0
		* Stage 3-11:
			- Set_and_get vallen and value
			- Try_case12 isbackup=1; otherwise, read_case12
		* Stage 11: port_forward
			- NOTE: current GETRES_POP has old key-value pair instead of new one, we must send original packet to egress pipeline
			- TODO: If isbackup=1 and iscase12=0, update GETRES_POP as GETRES_POP_EVICT_CASE2 to server, clone_i2e for GETRES to client
				+ GETRES_POP_EVICT_CASE2 -> hash_partition_reverse_tbl, update_macaddr_c2s
			- Otherwise
				+ If valid=0, drop original packet, clone_i2e for GETRES to client
				+ If valid=1, update GETRES_POP as GETRES_POP_EVICT to server, clone_i2e for GETRES to client
				+ GETRES_POP_EVICT -> hash_partition_reverse_tbl, update_macaddr_c2s
	+ GETRES_NPOP
		* Stage 2: set lock=0
		* Stage 11: port_forward -> update GETRES_NPOP as GETRES to client
	+ PUTREQ
		* Stage 0: match key
		* Stage 1
			- Get valid
			- Update vote: if key matches, increase vote; otherwise, decrease vote
			- Assign seq for each slot (no matter key matches or not)
			- Update iskeymatch
		* Stage 2
			- If valid=1 and iskeymatch=1, try to update savedseq to update meta.canput
			- Access lock: if valid=0 or zerovote=2, try_lock; otherwise, read_lock
		* Stage 3-11: 
			- Set_and_get vallen and value if valid=1 and canput=2 (valid=1, iskeymatch=1, and seq>savedseq)
			- Try_case12 if valid=1 and iskeymatch=1 and canput=2 and isbackup=1; otherwise, read_case12
		* Stage 11: port_forward
			- If (valid=0 or zerovote=2) and lock=0, update PUTREQ to PUTREQ_POP and recirculate
			- If valid=1 and iskeymatch=1
				+ If canput=2 and isbackup=1 and iscase12=0, update PUTREQ as PUTREQ_CASE1 to server, clone_i2e for PUTRES to client
				+ Otherwise, update PUTREQ to PUTRES
				- NOTE: current PUTREQ has old key-value pair instead of new one, we must send original packet to egress pipeline
			- If (valid=0 or iskeymatch=0) and lock=1, update PUTREQ as PUTREQ_RECIR (with seq_hdr, not need to assign seq again) and recirculate
			- Otherwise, forward PUTREQ to server -> hash_partition_tbl
	+ PUTREQ_POP
		* Stage 0: set_and_get key
		* Stage 1: set valid=1, vote=1
		* Stage 2: set savedseq=0, lock=0
		* Stage 3-11:
			- Set_and_get vallen and value
			- Try_case12 isbackup=1; otherwise, read_case12
		* Stage 11: port_forward
			- NOTE: current PUTREQ_POP has old key-value pair instead of new one, we must send original packet to egress pipeline
			- TODO: If isbackup=1 and iscase12=0, update PUTREQ_POP as PUTREQ_POP_EVICT_CASE2 to server, clone_i2e for PUTRES to client
				+ PUTREQ_POP_EVICT_CASE2 -> hash_partition_tbl
			- If (isbackup=0 or iscase12=1) and valid=0, drop original packet, clone_i2e for PUTRES to client
			- If (isbackup=0 or iscase12=1) and valid=1, update PUTREQ_POP as PUTREQ_POP_EVICT to server, clone_i2e for PUTRES to client
				+ PUTREQ_POP_EVICT -> hash_partition_tbl
	+ PUTREQ_RECIR (carry seq_hdr already)
		* Stage 0: match key
		* Stage 1
			- Get valid
			- Update vote: if key matches, increase vote; otherwise, decrease vote
			- NOTE: do not assign seq 
			- Update iskeymatch
		* Stage 2
			- If valid=1 and iskeymatch=1, try to update savedseq to update meta.canput
			- Access lock: if valid=0 or zerovote=2, try_lock; otherwise, read_lock
		* Stage 3-11: 
			- Set_and_get vallen and value if valid=1 and canput=2 (valid=1, iskeymatch=1, and seq>savedseq)
			- Try_case12 if valid=1 and iskeymatch=1 and canput=2 and isbackup=1; otherwise, read_case12
		* Stage 11: port_forward
			- If (valid=0 or zerovote=2) and lock=0, update PUTREQ_RECIR to PUTREQ_POP and recirculate
			- If valid=1 and iskeymatch=1
				+ If canput=2 and isbackup=1 and iscase12=0, update PUTREQ as PUTREQ_CASE1 to server, clone_i2e for PUTRES to client
				+ Otherwise, update PUTREQ to PUTRES
				- NOTE: current PUTREQ_RECIR has old key-value pair instead of new one, we must send original packet to egress pipeline
			- If (valid=0 or iskeymatch=0) and lock=1, recirculate PUTREQ_RECIR
			- Otherwise, update PUTREQ_RECIR as PUTREQ to server -> hash_partition_tbl
	+ DELREQ (as a speical PUTREQ)
		* Stage 0: match key
		* Stage 1
			- Get valid (treat DELREQ as a special PUTREQ which does not need to reset valid bit)
			- Not update vote now (TODO: maybe set vote=0 is better if popularity in the slot changes after DEL)
			- Assign seq for each slot (no matter key matches or not)
			- Update iskeymatch
		* Stage 2
			- If valid=1 and iskeymatch=1, try to update savedseq to update meta.canput
			- Not access lock (DEL will not trigger eviction)
		* Stage 3-11:
			- Reset_and_get vallen, and get value if valid=1 and canput=2 (valid=1, iskeymatch=1, and seq>savedseq)
			- Try_case12 if valid=1 and iskeymatch=1 and canput=2 and isbackup=1; otherwise, read_case12
		* Stage 11: port_forward
			- If valid=1 and iskeymatch=1
				+ If canput=2 and isbackup=1 and iscase12=0, update DELREQ as DELREQ_CASE1 to server, clone_i2e for DELRES to client
				+ Otherwise, update DELREQ to DELRES
				- NOTE: current DELREQ has old key-value pair instead of new one, we must send original packet to egress pipeline
			- If (valid=0 or iskeymatch=0) and lock=1, update DELREQ as DELREQ_RECIR (with seq_hdr, not need to assign seq again) and recirculate
			- Otherwise, forward DELREQ to server -> hash_partition_tbl
	+ DELREQ_RECIR
		* Stage 0: match key
		* Stage 1
			- Get valid (treat DELREQ_RECIR as a special PUTREQ which does not need to reset valid bit)
			- Not update vote now (TODO: maybe set vote=0 is better if popularity in the slot changes after DEL)
			- NOTE: do not assign seq 
			- Update iskeymatch
		* Stage 2
			- If valid=1 and iskeymatch=1, try to update savedseq to update meta.canput
			- Not access lock (DEL will not trigger eviction)
		* Stage 3-11:
			- Reset_and_get vallen, and get value if valid=1 and canput=2 (valid=1, iskeymatch=1, and seq>savedseq)
			- Try_case12 if valid=1 and iskeymatch=1 and canput=2 and isbackup=1; otherwise, read_case12
		* Stage 11: port_forward
			- If valid=1 and iskeymatch=1
				+ If canput=2 and isbackup=1 and iscase12=0, update DELREQ as DELREQ_CASE1 to server, clone_i2e for DELRES to client
				+ Otherwise, update DELREQ to DELRES
				- NOTE: current DELREQ_RECIR has old key-value pair instead of new one, we must send original packet to egress pipeline
			- If (valid=0 or iskeymatch=0) and lock=1, recirculate DELREQ_RECIR
			- Otherwise, update DELREQ_RECIR as DELREQ to server -> hash_partition_tbl
	+ TODO: Cope with packet loss
		* For GETRES_POP_EVICT/PUTREQ_POP_EVICT: send to server as well as mirroringing to switch OS for timeout and retransmission
		* For case1/case2: send to server as well as mirroringing to switch OS for local rollback 
		* For case3: wait for ACK from client to set iscase3=0
- Server-side processsing
	+ GETREQ: sendback GETRES
	+ GETREQ_POP:
		* If key exists in KVS, sendback GETRES_POP
		* Otherwise, sendback GETRES_NPOP
	+ GETRES_POP_EVICT:
		* If vallen > 0, put evicted key-value pair into KVS without response
		* Otherwise, delete evicted key from KVS without response
	+ PUTREQ_POP_EVICT:
		* If vallen > 0, put evicted key-value pair into KVS without response
		* Otherwise, delete evicted key from KVS without response
	+ PUTREQ_CASE1/DELREQ_CASE1:
		* Add into special case
	+ GETRES_POP_EVICT_CASE2/PUTREQ_POP_EVICT_CASE2
		* If vallen > 0, put evicted key-value pair into KVS without response, add into special case (valid)
		* Otherwise, delete evicted key from KVS without response, add into special case (invalid)
	+ TODO: snapshot for range query
		* Make a snapshot when init or open
		* Ensure that in each period of backup, kv snapshot can only be performance once
			* Use std::atomic_flag: test_and_set & clear
		* Steps of processing backup and making snapshot
			* Set isbackup as true to disable server threads from touching per-thread special cases
			* If is_kvsnapshot is false, mark it as true and make kv snapshot by RCU
			* Use TCP to receive new backup data
			* Rollback per-thread special cases (case1/case2) from new backup data
				* If vallen = 0 delete the entry from backup data; otherwise, overwrite the entry in backup data
			* Replace old backup data with new backup data
			* RCU barrier (no other threads touching per-thread special cases and old backup data)
			* Free old backup data and emptize per-thread special cases, set isbackup as false and is_kvsnapshot as false
- Controller-side processsing
	+ Crash-consistent backup (two-phase backup)
		* Phase 1
			* Reset registers: case12_reg, case3_reg
			* Set flag
		* Phase 2
			* Read registers
			* Reset flag -> no special optype from now on
			* Send backup data by TCP
			* Optional: reset registers: case1_reg, case2_reg, case3_reg


## Implementation log

- Copy netreach-voting to netreach-voting-v3
- Embed hashidx into packet op_hdr at client side (ycsb_remote_client.c, config.ini, packet_format.h, packet_format_impl,h, ycsb_server.c)
- Packet support
	+ Switch-side: tofino/\*.p4
	+ Server-side: packet_format.h, packet_format_impl,h, ycsb_server.c
	+ Support GETREQ, GETREQ_POP, GETRES_POP, GETRES_NPOP, GETRES_POP_EVICT
	+ Support PUTREQ, PUTREQ_POP, PUTREQ_RECIR, PUTREQ_POP_EVICT
	+ Support DELREQ, DELREQ_RECIR
	+ Support PUTREQ_CASE1, DELREQ_CASE1
	+ Support GETRES_POP_CASE2, PUTREQ_POP_CASE2
- TODO: crash-consistent backup (value read pattern)
- TODO: PUTREQ_CASE3, DELREQ_CASE3, range query
- TODO: debug and test, try as long value as we can
- TODO: extend xindex with variable-length value and snapshot
- TODO: replace rocksdb with xindex
- TODO: switch-driven consistent hashing
- TODO: distributed extension

## How to run

- Prepare for YCSB
	- Prepare workload for loading or transaction phase
		+ For example:
		+ `./bin/ycsb.sh load basic -P workloads/workloada -P netbuffer.dat > workloada-load.out`
		+ `./bin/ycsb.sh run basic -P workloads/workloada -P netbuffer.dat > workloada-run.out`
		+ `./split_workload load`
		+ `./split_workload run`
- Server
	- `./ycsb_local_client` for loading phase
	- `./ycsb_server` for server-side in transaction phase
- Client
	- `./ycsb_remote_client` for client-side in transaction phase
- Switch
	- Run `cd tofino`
	+ Run `su` to enter root account
	+ Run `bash compile.sh` to compile p4 into binary code
	+ Run `bash start_switch.sh` to launch Tofino
	+ Create a new terminal and run `bash configure.sh` to configure data plane
	+ `bash controller.sh setup`
	+ END: `bash controller.sh cleanup`
- Directory structure
	+ Raw workload file: workloada-load.out, workloada-run.out
	+ Split workload file: e.g., workloada-load-5/2.out
	+ Database directory: e.g., /tmp/netbuffer/workloada/group0.db, /tmp/netbuffer/workloada/buffer0-0.db
	+ RMI model at root node when init key-value store: workloada-root.out

## Fixed issues
