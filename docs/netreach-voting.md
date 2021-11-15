# Tofino-based NetREACH (voting-based) + DPDK-based XIndex with persistence + variable-length key-value pair (netreach)

## Important changes

- Change threshold voting into major voting
	+ Remove load_gthreshold_tbl and load_pthreshold_tbl
	+ Remove gposvote, gnegvote, pposvote, and pnegvote; add vote
	+ Remove try_glock and try_plock; add try_lock
	+ Remove vote diff calculation between pos vote and neg vote
	+ Remove update_getreq_tbl and update_putreq_tbl; add trigger_cache_update_tbl
- We use backup-based range query (do not need key coherence for recirculation-based range query now)
	+ Remove PUTREQ_N (change update_putreq_ru_to_n_and_clone into sendback_putres)
	+ Remove evicted key and its header
	+ Remove notified key in PUTREQ_PS
	+ Remove cached keys in server
	+ Remove listener for triggerred backup in server
- DEPRECATED: Update operations related with recirculate
	+ NOTE: resubmit will use the original packet before ingress pipeline even if we change some packet header 
	fields in ingress pipeline
	+ Remove PUTREQ_RU, instead we use PUTREQ + meta.is_putreq_ru of 1 to distinguish it (port_forward_tbl)
	+ Update MATs for PUTREQ_RU (val.p4 for value and vallen, key.p4, valid.p4, dirty.p4, seq.p4 for seq and savedseq, vote.p4, lock.p4)
	+ Use meta fields to store seq and is_assigned (thus we can carry the information with recirculated pkt) instead of in packet header
- Use recirculate instead of resubmit (resubmitted packet cannot be resubmitted again)
	+ NOTE: recirculate will use the packet after ingress deparser?
	+ Use PUTREQ_RU instead of meta.is_putreq_ru
	+ Re-update MATs for PUTREQ_RU
	+ Use packet header fields to store seq and is_assigned

## In-switch eviction mechanism

- For GETREQ/PUTREQ
	+ get seq for PUTREQ and hash -> match keys -> get valid/dirty bit, and access savedseq -> update vallen and get/put values 
	(meta.canput = 2 for PUTREQ with matched key) -> update votes -> try response directly (still send PUTRES for PUTREQ 
	with matched key even if canput = 0) -> calculate vote diff -> access lock -> cache update decision -> port forward and 
	hash partition
	+ NOTE: it is ok to set savedseq, vallen, and val without meta.isvalid; Even if the entry is invalid, set those data does not
	affect the correctness of responses and cache update, as long as we check meta.isvalid for try_res_tbl and cache update
- For GETRES_S (for GETREQ_S which is cache update decision for GETREQ)
	+ hash -> replace keys -> set valid bit as 1, dirty bit as 0, and reset savedseq -> set corresponding vallen and put values -> 
	-> reset all votes as 0 -> reset lock as 0 -> clone/convert a packet as GETRES [update GETRES_S as PUTREQ_GS to server for eviction 
	if necessary]
- For GETRES_NS (for GETREQ_S which is cache update decision for GETREQ but without data in server)
	+ hash -> keep original keys -> keep original valid, dirty, and savedseq -> keep original vallen and values -> 
	-> reset neg votes as 0 -> reset lock as 0 -> convert a packet as GETRES 
- For PUTREQ_RU (for PUTREQ_U which is recirculation-based cache update for PUTREQ)
	+ hash -> replace keys -> set valid bit as 1, dirty bit as 1, and reset savedseq -> reset corresponding vallen and put values ->
	-> reset all votes as 0 -> reeset lock as 0 -> clone a packet as PUTRES, and update PUTREQ_RU as PUTREQ_PS or PUTREQ_N to server for 
	eviction or notification

## Other notes

- For PUT 
	+ PUTREQ_N (cached key): converted from PUTREQ_RU without eviction
	+ PUTREQ_GS (evicted kv); converted from GETRES_S with eviction
	+ PUTREQ_PS (evicted kv + cached key): converted from PUTREQ_RU with eviction
		* The ideal way is to seperate PUTREQ_N from PUTREQ_PS
	+ PUTRES: cloned from PUTREQ_RU
- For UDP ports
	- We first save the initial src-dst port of incoming packet at Stage 0 (client port is dynamic)
	- At the end of ingress pipeline
		- For normal REQ, GETREQ_S, DELREQ_S, and PUTREQ_N, we use client(dynamic)-server(1111) as src-dst port
			+ meta.tmp_sport = client port, meta.tmp_dport = server_port
			+ We just set dst port based on meta.hashidx for hash partition
		- For PUTREQ_N, PUTREQ_GS and PUTREQ_PS, we use client(dynamic)-server(1111) as src-dst port
			+ PUTREQ_GS comes from GETRES_S, meta.tmp_sport = server_port, meta.tmp_dport = client port
				* We set src port as meta.tmp_dport, and set dst port based on meta.origin_hashidx for hash partition
			+ PUTREQ_PS comes from PUTREQ_RU, meta.tmp_sport = client port, meta.tmp_dport = server port
				* We just set dst port based on meta.origin_hashidx for hash partition
	- At the end of egress pipeline
		- For cloned RES (GETRES and DELRES), we use server(1111)-client(dynamic) port as src-dst port
			+ For GETRES cloned from PUTREQ_GS from GETRES_S, set src port = meta.tmp_sport = server port, dst port = meta.tmp_dport = client port
			+ For DELRES cloned from DELREQ_S from DELREQ, set src port = meta.tmp_dport = server port, dst port = meta.tmp_sport = client port
			+ For PUTRES cloned from PUTREQ_PS/PUTREQ_N from PUTREQ_RU from PUTREQ, set src port = meta.tmp_dport = server port, dst port = meta.tmp_sport = client port

## Implementation log

- Copy netreach to netreach-voting
- Support voting-based decision
	+ Match key for all requests -> is_match (key.p4, basic.p4, and configure/table_configure.py)
	+ Add pos/neg vote for get/put (vote.p4, basic.p4, and configure/table_configure.py)
		* If valid = 1 and key matches, increase corresponding positive vote
		* Otherwise, increase corresponding negative vote
	+ Add dirty bit (dirty.p4, basic.p4, and configure/table_configure.py)
	+ Add in-switch eviction mechanism
		* Add vote diff calculate (ingerss_mat.p4, basic.p4, and configure/table_configure.py)
		* DEPRECATED: Add two thresholds (basic.p4, ingress_mat.p4, and configure/table_configure.py)
		* Only if key does not match: compare vote diff and corresponding threshold to update lock bit; also get original lock bit
		* Key matches -> response
			- Only if it is valid and key matches, get/put value register (basic.p4, val.p4, and configure/table_configure.py)
			- Only if it is valid and key matches, sendback responses direcctly or by cloning (basic.p4, ingress_mat.p4, and configure/table_configure.py)
				+ For GETREQ: sendback GETRES directly
				+ For PUTREQ: sendback PUTRES directly
					* If PUTREQ and key matches, we need to set dirty as 1 immediately
						- We need to send a PUTREQ_N to update the cached keys in server
					* NOTE: even if valid is 0, set dirty as 1 does not affect correctness
						- In basic.p4, we need to use g/pposvote to calculate diff based on isdirty. However, if valid is 0, posvote
						must be 0. So using gposvote or pposvote does not matter.
						- In port_forward_tbl, we use dirty to decide whether we need to evict keys for GETRES_S. However, we do it only
						if dirty is 1 and valid is 1. If valid is 0, we will never evicte keys for GETRES_S.
						- Also, for PUTREQ_RU, we should convert PUTREQ_RU to PUTREQ_PS only if valid is 1 and dirty is 1
				+ For DELREQ: update transferred packet as DELREQ_S and sendback DELRES by cloning (delete cached keys in server)
		* Key does not match, and original lock bit = 0 && diff >= threshold -> trigger cache update
			- For GETREQ (response-based update): update transferred packet as GETREQ_S (basic.p4, ingress_mat.p4, and configure/table_configure.py)
				+ Server receives GETREQ_S and gives GETRES_S to switch (ycsb_server.c, packet_format.h, packet_format_impl.h)
				+ Switch processes GETRES_S, updates it as PUTREQ_GS towards server, and clones a packet as GETRES to client (basic.p4, ingress_mat.p4, egress_mat.p4, and configure/table_configure.py)
				+ Server receives PUTREQ_GS to update key-value store and remove cached keys
			- For PUTREQ (recirculation-based update): update packet as PUTREQ_U for port_forward_tbl, and 
			then update it as PUTREQ_RU and recirculate (ingress_mat.p4, and configure/table_configure.py)
				+ For PUTREQ_RU, we need to update regs including keys, vals, votes, lock, valid, dirty, vallen, etc. (configure/table_configure.py)
				+ For PUTREQ_RU, we convert it as PUTREQ_PS or PUTREQ_N in ingress pipeline and clone a PUTRES to client (basic.p4, ingress_mat.p4, egress_mat.p4, and configure.table_configure,py)
				+ Server receives PUTREQ_N to update cached keys; Server receives PUTREQ_PS to update cached keys and key-value store (packet_format.h, packet_format_impl.h, ycsb_server.c)
				+ For PUTRES,set udp port and udp hdr length correspondingly (egress_mat.p4 and configure/table_configure.py)
				* We should set MAC addr according to optype
		* Key does not match (GETREQ/PUTREQ/DELREQ), and original lock bit = 0 && diff < threshold -> forward
		* Key does not match (GETREQ/PUTREQ/DELREQ), and original lock bit = 1 -> also recirculate
		* We maintain a set of cached keys for each server thread
	+ Add in-switch local sequence number
		* Change packet format (header.p4, parser.p4, packet_format.h, packet_format_impl.h)
			- PUTREQ: <op_hdr, vallen, val, seq, is_assigned>
			- PUTREQ_GS: <op_hdr, vallen, val, seq, is_assigned> (seq is useless)
			- PUTREQ_N: <op_hdr, vallen, val, seq, is_assigned> (seq is useless)
			- PUTREQ_PS: <op_hdr, evicted_key, vallen, val, seq, is_assigned> (seq is useless)
			- GETRES: <op_hdr, vallen, val, seq, is_assigned> (seq is useless)
			- Only in switch-side
				- GETRES_S: <op_hdr, vallen, val, seq, is_assigned> (seq is useless)
				- PUTREQ_U: <op_hdr, vallen, val, seq, is_assigned> (seq is useless)
				- PUTREQ_RU: <op_hdr, vallen, val, seq, is_assigned> (seq is useless)
		* Assign a local sequence number for PUTREQ whose is_assigned = 0 at Stage 0, and set is_assigned as 1
		* Compare seq number before accessing val for PUTREQ with matched key and valid = 1 -> set meta.canput as 2 (predicate) 
		if seq > saved_seq or saved_seq - seq == 0xFFFFFFFF
			- If optype is GETRES_S or PUTREQ_RU, we reset savedseq
		* For PUTREQ, only if key matches and meta.canput = 2 (we do not need to see whether it is valid, valid is only important for
		sending back responses), we update the value
			* NOTE: we drectly sendback PUTRES in try_res_tbl as long as key matches and valid = 1 even if meta.canput = 0
	+ TODO: Consider how to optimize the extra latency introduced by responsed-based cache update
		* We can use a register array to save the key to be cached, only if the key does not match the cached key and the key to be 
		cached, we need to recirculate it (extra latency for these requests)
		* Add future_valid and future_keys (valid.p4, key.p4)
		* TODO: update metadata
		* NOTE: do it after compile -> get current resource usage
	+ If server does not have the key-value pair for GETREQ_S, generate a GETRES_NS to set the valid as 0 
	(configure/table_configure.py, packet_format.h, packet_format_impl.h, ycsb_server.c)
		* NOTE: we keep original keys, valid, dirty, savedseq, vallen, values, and pos votes. We only reset neg votes and lock. We
		do not generate PUTREQ_PS, instead we directly convert it as GETRES.
	+ Set size of each table accordingly
- Support crash-consistent backup
	+ Switch: add flag to mark three cases (load_backup_flag_tbl)
		* NOTE: for invalid or valid but non-dirty, we set is_assigned as 0; otherwise, we set it as 1 (need to remember in server's backup)
		* Case 1: first PUT/DEL in switch for each bucket -> evict old value
			- Use case1_reg to mark whether case 1 has been triggered in access_case1_tbl (keymatches, valid = 1, and backup = 1 for PUTREQ/DELREQ)
			- In try_res_tbl, if PUTREQ/DELREQ, valid = 1, key matches, isbackup = 1, iscase1 = 0, convert it to PUTREQ_CASE1/DELREQ_CASE1 and set value in packet
			header as old value (set hashidx in seq, and set dirty bit in is_assigned (valid must be 1 in this case))
			- In port_forward_tbl, if PUTREQ_CASE1/DELREQ_CASE1, forward it to server and clone PUTRES/DELRES to client
			- Use hash_partition_tbl directly
		* Case 2: first eviction for each bucket -> evicted data
			- Use case2_reg to mark whether case 2 has been triggered in access_case2_tbl (backup = 1 for GETRES_S and PUTREQ_RU)
			- In try_res_tbl, if GETRES_S/PUTREQ_RU, isbackup = 1, iscase2 = 0, convert it to GETRES_S_CASE2 and PUTREQ_RU_CASE2,
			set key and value in header as evicted data, set hashidx in seq, and set is_assigned (as 1 only if valid = 1 and dirty = 1)
			- In port_forward_tbl, if GETRES_S_CASE2/PUTREQ_RU_CASE2, convert it to PUTREQ_GS_CASE2/PUTREQ_PS_CASE2 to server, and
			clone GETRES/PUTRES to client
			- Use the same hash parition table as PUTREQ_GS/PUTREQ_PS
		* Case 3: first PUT/DEL to server
			- Use case3_reg to mark whether case 3 has been trigger in trigger_cache_update_tbl (for entire switch to trigger snapshot of server)
				+ For PUTREQ: isbackup = 1, key does not match or entry is invalid, lock = 0, isevict = 1 -> try_case3
				+ For DELREQ: isbackup = 1, key does not match or entry is invalid, lock = 0 -> try_case3
			- In port_forward_tbl, if isbackup = 1, islock = 0, isevcit = 1 only for PUTREQ, iscase3 = 0, convert PUTREQ/DELREQ to PUTREQ_CASE3/DELREQ_CASE3, and forward to server
	+ Controller
		* Phase 1
			* Reset registers: case1_reg, case2_reg, case3_reg
			* Set flag
		* Phase 2
			* Read registers
			* Reset flag -> no special optype from now on
			* Send backup data by TCP
			* Optional: reset registers: case1_reg, case2_reg, case3_reg
	+ Server
		* Make a snapshot when init or open
		* Ensure that in each period of backup, kv snapshot can only be performance once
			* Use std::atomic_flag: test_and_set & clear
		* Steps
			* Set isbackup as true to disable server threads from touching per-thread special cases
			* If is_kvsnapshot is false, mark it as true and make kv snapshot by RCU
			* Use TCP to receive new backup data
			* Rollback per-thread special cases from new backup data
			* Replace old backup data with new backup data
			* RCU barrier (no other threads touching per-thread special cases and old backup data)
			* Free old backup data and emptize per-thread special cases, set isbackup as false and is_kvsnapshot as false
		* If not isbackup, process PUTREQ_CASE1, DELREQ_CASE1, PUTREQ_GS_CASE2, PUTREQ_PS_CASE2, PUTREQ_CASE3, DELREQ_CASE3 to remember speical cases
			* Add all packet formats
			* PUTREQ_CASE1: add it into special cases if hashidx does not exist
			* DELREQ_CASE1: delete kv-store, add it into special cases if hashidx does not exist
			* PUTREQ_GS/PS_CASE2: insert into kv-store, add it into special cases if hashidx does not exist
			* PUT/DELREQ_CASE3: insert/delete in kv-store, sendback response
			* ALL: if not is_kvsnapshot, mark is_kvsnapshot as true (data plane will only report at most one 
			such packet for each server) -> make snapshot of kv-store for each group by RCU
- Try default action with hashidx by removing condition for hash calculation
	+ Fail! Even if the hash calculation must be performed.
+ Support scan (range query)
	- Change scan from key+num to start_key+end_key (TODO: apply all the followings to baseline)
		+ Optional: use num to restrict # of per-subrequest kv pairs
		+ Change volatile rte_mbuf * to rte_mbuf * volatile (client.c, server.c, ycsb_remote_client.c, ycsb_server.c)
		+ Add endkey into ScanRequest and ScanResponse (packet_format.h, packet_format_impl.h, client.c, server.,c ycsb_remote_cliet.c, ycsb_server.c)
		+ Change switch side accordingly
			+ Disable hash parition for SCAN request (basic.p4)
		+ Change client side accordingly (ycsb_remote_client.c)
			+ Use message queue for each client thread
			+ Get number of sub-requests
		+ Change server side accordingly (ycsb_server.c)
			+ Get optype and scan keys in receiver (dpdk_helper.h, dpdk_helper.c)
			+ Receiver split the SCAN request into multiple sub-requests
			+ Process sub-requests by different server threads (thpt: count in client side since all requests are handled by server; latency: count in 
			the granularity of sub-requests, split latency should not be counted which is happened in switch)
			+ Implement range scan of kv-store (xindex_root_impl.h, xindex_group.h, xindex_group_impl.h)
			+ Perform range scan on both kv-store and backup -> reply the final results
	- Support SCAN with a guarantee of some point-in-time: we still need RCU for backup data in server (only for NetReach)
- Debug
	+ Use thread_id of server thread to perform operation in key-value store instead of req.thread_id() (ycsb_server.c)
	+ If condition_lo is true, the predicate is 2; (NOTE) if condition_lo is false, the predicate is 1 instead of 0!!!
		* Solution: add initialize_tbl to initialize predicate fields in meta as 1 to reduce MAT entries
	+ Try official example of resubmit -> success
		* NOTE: clone/recirculate/resubmit will only reprocess the original packet before ingress pipeline even if you modify the packet
		field in ingress pipeline (making no sense for recirculated pkt)
	+ Fix bugs of recirculation and seq
	+ Fix a bug of packet format (incorrect size for PUTREQ and GETRES)
	+ Fix a bug of udp hdrlen (remove seq and is_assigned from PKT_VALLEN and PKT_VALLEN_MINUS_ONE)
	+ Fix a bug of incorrect MAC addr of PUTREQ_GS (converted from GETRES_S)
	+ Fix a bug of processing GETREQ_S for non-existing key (judge by vallen instead of status in server)
	+ Find a bug of resubmit: resubmitted packet cannot be resubmitted again; (recirculate is performed as setting egress port)
		* Solution: use recirculation (we must enable and add port for recir/cpu/pktgen in ptf before usage)
	+ Fix a bug of # of MAT entries; fix a bug of making snapshot when init/open
- TODO: Optimize for stage (at most 32B -> 48B; optional after we finish all implementation)
	+ TODO: Combine access_lock_tbl into try_res_tbl

## Simple test

- NOTE: update bucket_num in config.ini as 1 before test
- Test cases of normal operations: See directory of "testcases/normal" (with only 1 bucket in sketch)
	+ Case 1: single read (GET evicts invalid)
		* Read the value of a given key
		* It should read the value from the server and also store it in switch
		* In-switch result: non-zero key, vallen, and val, seq = 0, savedseq = 0, lock = 0, valid = 1, dirty = 0, vote = 1
	+ Case 2: single write (PUT evicts invalid)
		* Write new value for a given key
		* It should write the value into switch by recirculation and sendback PUTRES (no PUTREQ_PS)
		* In-switch result: non-zero key, vallen, and val, seq = 1, savedseq = 0, lock = 0, valid = 1, dirty = 1, vote = 1
	+ Case 3: read-after-write
		* Write value of k1 and then read k1
		* It should write the value in switch and read the value from switch (not touch server)
		* In-switch result: non-zero key, vallen, and val of k1, seq = 1, savedseq = 0, lock = 0, valid = 1, dirty = 1, vote = 2
	+ Case 4: read-after-two-writes
		* Write value of k1 twice, and then read k1
		* It should write the value in switch and read the value from switch (not touch server)
		* In-switch result: non-zero key, vallen, and val of k1, seq = 2, savedseq = 2, lock = 0, valid = 1, dirty = 1, vote = 3
	+ Case 5: write-after-read1
		* Read value of k1 and then write k1
		* It reads the value of k1 from server and store it in switch, PUT increases vote, updates vallen & val, and does not touch server
		* In-switch result: non-zero key, vallen, and val of k1, seq = 1, savedseq = 1, lock = 0, valid = 1, dirty = 1, vote = 2
	+ Case 6: write-after-read2
		* Read value of k1 and then write k2
		* It should read the value of k1 from server and store it in switch, k2 will decrease vote and be forwarded to server (no cache update)
		* In-switch result: non-zero key, vallen, and val of k1, seq = 1, savedseq = 0, lock = 0, valid = 1, dirty = 0, vote = 0
	+ Case 7: two-writes-after-read (PUT evicts GET)
		* Read value of k1 and then write k2 twice
		* It should read the value of k1 from server and store it in switch, k2 will replace k1 finally (PUTs touch server only once)
		* In-switch result: non-zero key, vallen, and val of 2nd k2, seq = 2, savedseq = 0, lock = 0, valid = 1, dirty = 1, vote = 1
	+ Case 8: read-after-two-writes-after-write (PUT evicts PUT)
		* Write value of k1, write k2 twice, and then read k2
		* PUT of k1 writes the value in switch and sendback PUTRES, 1st PUT of k2 is forwarded to server, 2nd PUT of k2 evicts k1, GET
		is directly processed by switch
		* In-switch result: non-zero key, vallen, and val of 2nd k2, seq = 3, savedseq = 0, lock = 0, valid = 1, dirty = 1, vote = 2
	+ Case 9: two-reads-after-write (GET evicts PUT)
		* Write value of k1, read k2 twice
		* It writes value of k1 in switch, GETs of k2 evicted k1 (the evicted data touches server)
		* In-switch result: non-zero key, vallen, and val of k2, seq = 1, savedseq = 0, lock = 0, valid = 1, dirty = 0, vote = 1
	+ Case 10: two-reads-after-read (GET evicts GET)
		* Read value of k1, read k2 twice
		* It first gets value of k1 from server and stores it in switch, the 2nd GET of k2 replaces k1 in switch
		* In-switch result: non-zero key, vallen, and val of k2, seq = 0, savedseq = 0, lock = 0, valid = 1, dirty = 0, vote = 1
	+ Case 11: read-delete-read
		* Read value of k1, delete k1, and then read k1 again
		* It first gets value of k1 from server and stores it in switch, then it deletes k1 and sends DELREQ_S to server, the 2nd GET
		of k1 does not have value and triggers a GETRES_NS
		* In-switch result: non-zero key, vallen, and val of k1, seq = 0, savedseq = 0, lock = 0, valid = 0, dirty = 0, vote = 0
- Test cases of crash-consistent backup: See "testcases/backup" (with only 1 bucket in sketch)
	+ Phase1: reset regs and set flag as 1
	+ Case 1-1: undirty + PUT case1
		* Get <k1, v1> -> Run phase1 -> PUT <k1, v2> -> Run phase2
		* Result: receive PUTREQ_CASE1 with <k1, v1> (undirty), receive backup with <k1, v2>, final backup after rollback without k1
	+ Case 1-2: dirty + PUT case1
		* PUT <k1, v1> -> Run phase1 -> PUT <k1, v2> -> Run phase2
		* Result: receive PUTREQ_CASE1 with <k1, v1> (dirty), receive backup with <k1, v2>, final backup after rollback with <k1, v1>
	+ Case 1-3: undirty + DEL case1
		* Get <k1, v1> -> Run phase1 -> DEL k1 -> Run phase2
		* Result: receive DELREQ_CASE1 with <k1, v1> (undirty), receive backup without k1, final backup after rollback without k1
	+ TODO: Case 1-4: dirty + DEL case1
		* PUT <k1, v1> -> Run phase1 -> DEL k1 -> Run phase2
		* Result: receive DELREQ_CASE1 with <k1, v1> (dirty), receive backup without k1, final backup after rollback with <k1, v1>
	+ TODO: Case 2-1: invalid + PUTGS case2
		* Run phase1 -> GET <k1, v1> -> Run phase2
		* Result: receive PUTREQ_GS_CASE2 with <0, 0>, receive backup with <k1, v1> (undirty), final backup after rollback without k1
	+ TODO: Case 2-2: undirty + PUTGS case2
		* GET <k1, v1> -> Run phase1 -> GET <k2, v2> -> Get <k3, v3> -> Run phase2
		* Result: receive PUTREQ_GS_CASE2 with <k1, v1> (undirty), receive backup with <k3, v3> (undirty), final backup after 
		rollback without k1
	+ TODO: Case 2-3: dirty + PUTGS case2
		* PUT <k1, v1> -> Run phase1 -> GET <k2, v2> -> GET <k3, v3> -> Run phase2
		* Result: receive PUTREQ_GS_CASE2 with <k1, v1> (dirty), receive backup with <k3, v3> (undirty), final backup after 
		rollback with <k1, v1>
	+ Case 2-4: invalid + PUTPS case2
		* Run phase1 -> PUT <k1, v1> -> Run phase2
		* Result: receive PUTREQ_PS_CASE2 with <0, 0>, receive backup with <k1, v1>, final backup after rollback without k1
	+ TODO: Case 2-5: undirty + PUTPS case2 + PUTREQ case3
		* GET <k1, v1> -> Run phase1 -> PUT <k2, v2> -> PUT <k3, v3> -> Run phase2
		* Result: receive PUTREQ_CASE3 with <k2, v2> and PUTREQ_PS_CASE2 with <k1, v1> (undirty), receive backup with <k3, v3>, final 
		backup after rollback without k1
	+ TODO: Case 2-6: dirty + PUTPS case2 + PUTREQ case3
		* PUT <k1, v1> -> Run phase1 -> PUT <k2, v2> -> PUT <k3, v3> -> Run phase2
		* Result: receive PUTREQ_CASE3 with <k2, v2> and PUTREQ_PS_CASE2 with <k1, v1> (dirty), receive backup with <k3, v3>, final 
		backup after rollback with <k1, v1>
	+ TODO: Case 3-1: DELREQ case3
		* PUT <k1, v1> -> Run phase1 -> DEL <k2, v2> -> RUN phase2
		* Result: receive DELREQ_CASE3 with k2, receive backup with <k1, v1>, final backup after rollback with <k1, v1>

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
