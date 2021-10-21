# Tofino-based NetBuffer + DPDK-based XIndex with persistency + variable-length key-value pair (tofino-netbuffer-dpdk-lsm-varkv)

## Implementation log

- Copy tofino-netbuffer-dpdk-lsm-varkv to netreach
- Set val length as 8B for fast debug (val.h, tofino/*.p4, tofino/*.py)
- Add config module (config.ini, iniparser/\**)
	- Read config.ini in ptf test files (configure/table_configure.py)
	- Read config.ini in \*.c files (server.c, client.c, localtest.c, split_workload,c, ycsb_local_client.c, ycsb_remote_client.c)
	- Update Makefile
- Key-based routing
	- Directly use reserved dst port 1111 instead of dst port start (client.c)
		+ For changing dst port to simulate different servers, we deploy it in switch
		+ In server, we use multiple threads to simulate servers, and a concurrent key-value store to simulate distributed key-value store
	- Add key-based routing in switch (hash_partition_tbl in basic.p4)
		+ Add range matching rules (configure/table_configure.py)
	- Debug and test
		+ Fix a bug in ycsb/parser.c: do not use macro of VALLEN at parsekv
		+ Fix two bugs in tofino/basic.p4: (1) use modify_field instead of add_to_field in hash_partition_tbl; (2) we should change
		two macros about maximum value length accordingly in basic.p4
- Support YCSB
	- Copy client.c to yscb_remote_client.c and integrate YCSB parser into transaction phase (ycsb_remote_client.c)
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

- Microbenchmark
	- Prepare randomly-generated keys
		+ NOTE: we direclty use makefile to enable DPDK (to detet ports) without cmake
		+ `make all`
		+ `./prepare`
- YCSB
	- Prepare workload for loading or transaction phase
		+ For example:
		+ `./bin/ycsb.sh load basic -P workloads/workloada -P netbuffer.dat > workloada-load.out`
		+ `./bin/ycsb.sh run basic -P workloads/workloada -P netbuffer.dat > workloada-run.out`
		+ `./split_workload load`
		+ `./split_workload run`
	- `./ycsb_local_client` for loading phase
	- Directory structure
		+ Raw workload file: workloada-load.out, workloada-run.out
		+ Split workload file: e.g., workloada-load-5/2.out
		+ Database directory: e.g., /tmp/netbuffer/workloada/group0.db, /tmp/netbuffer/workloada/buffer0-0.db
		+ RMI model at root node when init key-value store: workloada-root.out
- Run `cd tofino`
	+ Run `su` to enter root account
	+ Run `bash compile.sh` to compile p4 into binary code
	+ Run `bash start_switch.sh` to launch Tofino
	+ Create a new terminal and run `bash configure.sh` to configure data plane
- Run `bash start_server.sh` in server host
- Run `bash start_client.sh` in client host
- Switch
	+ `cd tofino`
	+ `bash controller.sh setup`
	+ END: `bash controller.sh cleanup`

## Fixed issues
