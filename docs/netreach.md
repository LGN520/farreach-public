# Tofino-based NetBuffer + DPDK-based XIndex with persistency + variable-length key-value pair (tofino-netbuffer-dpdk-lsm-varkv)

## Implementation log

- Copy tofino-netbuffer-dpdk-lsm-varkv to netreach
- Directly use reserved dst port 1111 instead of dst port start (client.c)
	+ For changing dst port to simulate different servers, we deploy it in switch
	+ In server, we use multiple thresholds to simulate servers, and a concurrent key-value store to simulate distributed key-value store
- Set val length as 8B for fast debug (val.h, tofino/*.p4, tofino/*.py)
- Add key-based routing in switch (hash_partition_tbl and select_server_tbl in basic.p4)
- Read config.ini in ptf test files (config.ini)
- TODO: read config.ini in *.c files
- TODO: For put req
	+ If the entry is empty, we need to update the cache directly and notify the server (do not need to drop put_req, which becomes put_req_n; need to clone for put_res)
	+ If the entry is not empty but key matches, we need to update value (need to drop original put req; need to clone for put_res)
	+ If the entry is not emtpy and key does not match
		* If with cache update, we need to change pkt to put_req_u and update cache by recirculation
			- If the original entry is not dirty, we still need to notify the server by changing pkt to put_req_n (do not need to drop put_req_u; need to clone for put_res)
			- If it is dirty, we need to change pkt to put_req_s (a new key and evicted key-value pair) (do not need to drop put_req_s; need to clone for put_res)
		* If without cache update, we need to forward put_req (do not need to clone pkt for put_res)

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
		+ `./split_workload load workloada threadnum`
		+ `./split_workload run workloada threadnum`
	- `./ycsb_local_client -h threadnum -p workloada`
	- Directory structure
		+ Raw workload file: workloada-load.out, workloada-run.out
		+ Split workload file: e.g., workloada-load-5/2.out
		+ Database directory: e.g., /tmp/netbuffer/workloada/group0.db, /tmp/netbuffer/workloada/buffer0-0.db
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
