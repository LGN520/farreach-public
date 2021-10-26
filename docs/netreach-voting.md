# Tofino-based NetREACH (voting-based) + DPDK-based XIndex with persistence + variable-length key-value pair (netreach)

## In-switch eviction mechanism

- For GETREQ/PUTREQ
	+ hash -> match keys -> get valid/dirty bit and vallen -> update votes -> get/put values -> try response directly -> 
	calculate vote diff -> access lock -> cache update decision
- For GETRES_S (for GETREQ_S which is cache update decision for GETREQ)
	+ hash -> replace keys -> set valid bit as 1, dirty bit as 0, and set corresponding vallen -> reset votes as 0 -> put values ->
	reset lock as 0 -> clone a packet as GETRES [update GETRES_S as PUTREQ_S to server for eviction if necessary]
	+ TODO
		* update port_forward_tbl configuration
		* update sendback_getres_tbl (no need to swap ports)
		* update sendback_delres_tbl (need to swap ports)

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
			- TODO 3: add cached keys in server-side
		* Key does not match, and original lock bit = 0 && diff >= threshold -> trigger cache update
			- For GETREQ: update transferred packet as GETREQ_S (basic.p4, ingress_mat.p4, and configure/table_configure.py)
				+ Server receives GETREQ_S and gives GETRES_S to switch (ycsb_server.c, packet_format.h, packet_format_impl.h)
				+ Switch processes GETRES_S, updates it as PUTREQ_S towards server, and clones a packet as GETRES to client (configure/table_configure.py)
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
