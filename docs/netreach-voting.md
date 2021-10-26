# Tofino-based NetREACH (voting-based) + DPDK-based XIndex with persistence + variable-length key-value pair (netreach)

## In-switch eviction mechanism

- For GETREQ/PUTREQ
	+ hash -> match keys -> get valid/dirty bit and vallen -> update votes -> get/put values -> try response directly -> 
	calculate vote diff -> access lock -> cache update decision
- For GETRES_S (for GETREQ_S which is cache update decision for GETREQ)
	+ hash -> replace keys -> set valid bit as 1, dirty bit as 0, and set corresponding vallen -> reset votes as 0 -> put values ->
	reset lock as 0 -> clone a packet as GETRES [update GETRES_S as PUTREQ_S to server for eviction if necessary]
	+ TODO

## Other notes

- For PUT 
	+ PUTREQ_N (cached key): converted from PUTREQ_U without eviction
	+ PUTREQ_GS (evicted kv); converted from GETRES_S with eviction
	+ PUTREQ_PS (evicted kv + cached key): converted from PUTREQ_U with eviction
		* The ideal way is to seperate PUTREQ_N from PUTREQ_PS
	+ PUTRES: cloned from PUTREQ_U
- For UDP ports
	- We first save the initial src-dst port of incoming packet at Stage 0 (client port is dynamic)
	- At the end of ingress pipeline
		- For normal REQ, GETREQ_S, DELREQ_S, and PUTREQ_N, we use client(dynamic)-server(1111) as src-dst port
			+ meta.tmp_sport = client port, meta.tmp_dport = server_port
			+ We just set dst port based on meta.hashidx for hash partition
		- For PUTREQ_N, PUTREQ_GS and PUTREQ_PS, we use client(dynamic)-server(1111) as src-dst port
			+ PUTREQ_GS comes from GETRES_S, meta.tmp_sport = server_port, meta.tmp_dport = client port
				* We set src port as meta.tmp_dport, and set dst port based on meta.origin_hashidx for hash partition
			+ PUTREQ_PS comes from PUTREQ_U, meta.tmp_sport = client port, meta.tmp_dport = server port
				* We just set dst port based on meta.origin_hashidx for hash partition
	- At the end of egress pipeline
		- For cloned RES (GETRES and DELRES), we use server(1111)-client(dynamic) port as src-dst port
			+ For GETRES cloned from PUTREQ_GS from GETRES_S, set src port = meta.tmp_sport = server port, dst port = meta.tmp_dport = client port
			+ For DELRES cloned from DELREQ_S from DELREQ, set src port = meta.tmp_dport = server port, dst port = meta.tmp_sport = client port
			+ For PUTRES cloned from PUTREQ_PS/PUTREQ_N from PUTREQ_U from PUTREQ, set src port = meta.tmp_dport = server port, dst port = meta.tmp_sport = client port

## Implementation log

- Copy netreach to netreach-voting
- Support voting-based decision
	+ Match key for all requests -> is_match (key.p4, basic.p4, and configure/table_configure.py)
	+ Add pos/neg vote for get/put (vote.p4, basic.p4, and configure/table_configure.py)
		* If valid = 1 and key matches, increase corresponding positive vote
		* Otherwise, increase corresponding negative vote
	+ Add dirty bit (dirty.p4, basic.p4, and configure/table_configure.py)
	+ Add voting-based decision
		* Add vote diff calculate (ingerss_mat.p4, basic.p4, and configure/table_configure.py)
		* Add two thresholds (basic.p4, ingress_mat.p4, and configure/table_configure.py (TODO 1))
		* Only if key does not match: compare vote diff and corresponding threshold to update lock bit; also get original lock bit
		* Key matches -> response
			- Only if it is valid and key matches, get/put value register (basic.p4, val.p4, and configure/table_configure.py)
			- Only if it is valid and key matches, sendback responses direcctly or by cloning (basic.p4, ingress_mat.p4, and configure/table_configure.py)
				+ For GETREQ: sendback GETRES directly
				+ For PUTREQ: sendback PUTRES directly
				+ For DELREQ: update transferred packet as DELREQ_S and sendback DELRES by cloning (TODO 2: delete cached keys in server)
		* Key does not match, and original lock bit = 0 && diff >= threshold -> trigger cache update
			- For GETREQ (response-based update): update transferred packet as GETREQ_S (basic.p4, ingress_mat.p4, and configure/table_configure.py)
				+ Server receives GETREQ_S and gives GETRES_S to switch (ycsb_server.c, packet_format.h, packet_format_impl.h)
				+ Switch processes GETRES_S, updates it as PUTREQ_GS towards server, and clones a packet as GETRES to client (basic.p4, ingress_mat.p4, egress_mat.p4, and configure/table_configure.py)
			- TODO: For PUTREQ (recirculation-based update): update packet as PUTREQ_U and then recirculate
				+ TODO: For PUTREQ_U, we convert it as PUTREQ_PS or PUTREQ_N in ingress pipeline and clone a PUTRES to client
				+ TODO: Server receives PUTREQ_N to update cached keys; Server receives PUTREQ_PS to update cached keys and key-value store
				+ TODO: For PUTRES,set udp port correspondingly
				* TODO: We should set MAC addr according to optype
				+ TODO 4: local sequence number
		* TODO: Key does not match, and original lock bit = 0 && diff < threshold -> forward
		* TODO: Key does not match, and original lock bit = 1 -> also recirculate
- TODO: For put req
	+ If the entry is empty, we need to update the cache directly and notify the server (do not need to drop put_req, which becomes put_req_n; need to clone for put_res)
	+ If the entry is not empty but key matches, we need to update value (need to drop original put req; need to clone for put_res)
	+ If the entry is not emtpy and key does not match
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
