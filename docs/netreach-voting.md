# Tofino-based NetREACH (voting-based) + DPDK-based XIndex with persistence + variable-length key-value pair (netreach)

## Important changes

- Change threshold voting into major voting
	+ Remove load_gthreshold_tbl and load_pthreshold_tbl
	+ Remove gposvote, gnegvote, pposvote, and pnegvote; add vote
	+ Remove try_glock and try_plock; add try_lock
	+ Remove vote diff calculation between pos vote and neg vote
	+ Remove update_getreq_tbl and update_putreq_tbl; add trigger_cache_update_tbl
- We use backup-based range query
	+ Remove PUTREQ_N (change update_putreq_to_n_and_clone into sendback_putres)
	+ Remove evicted key and its header
	+ Remove notified key in PUTREQ_PS
	+ Remove cached keys in server
	+ TODO: remove listener for triggerred backup in server

## In-switch eviction mechanism

- For GETREQ/PUTREQ
	+ get seq for PUTREQ and hash -> match keys -> get valid/dirty bit, and access savedseq -> update vallen and get/put values 
	(meta.canput = 2 for PUTREQ with matched key) -> update votes -> try response directly (still send PUTRES for PUTREQ 
	with matched key even if canput = 0) -> calculate vote diff -> access lock -> cache update decision -> port forward and 
	hash partition
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
		* Add two thresholds (basic.p4, ingress_mat.p4, and configure/table_configure.py (TODO 1))
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
	+ Support scan
		- TODO: change scan from key+num to start_key+end_key
		- TODO: get the key range of each server thread in loading phase
		- TODO: simulate multiple packets in server-side by split the request to multiple server threads by range partition
		- TODO: support SCAN with as much latest data as possible or SCAN with a guarantee of some point-in-time?
	+ TODO: If req does not need to access backup data for SCAN, then we do not need RCU for backup data
	+ TODO: For carsh-consistent backup, we only need to remember the evicted data from PUTREQ_GS and PUTREQ_PS instead of PUTREQ_N in server
	+ TODO: If we do not use recirculation-based range query, we do not need key coherence. For PUTREQ_PS, we only keep evicted data
	instead of the new key; For PUTREQ_N, we drop the original packet by drop_put_tbl
	+ TODO: set size of each table accordingly
- TODO: For put req
	+ If key matches, we need to update value (need to drop original put req; need to clone for put_res)
	+ If key does not match
		* If with cache update, we need to change pkt to put_req_u and update cache by recirculation
			- If the original entry is not dirty, we still need to notify the server by changing pkt to put_req_n (do not need to drop put_req_u; need to clone for put_res)
			- If it is dirty, we need to change pkt to put_req_s (a new key and evicted key-value pair) (do not need to drop put_req_s; need to clone for put_res)
		* If without cache update, we need to forward put_req (do not need to clone pkt for put_res)
	+ NOTE: in design, put_req_n and put_req_s must be two packets since the two servers may be different

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
