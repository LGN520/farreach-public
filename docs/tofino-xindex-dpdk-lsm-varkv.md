# Tofino + DPDK-based XIndex with persistency + variable-length key-value pair (tofino-netbuffer-dpdk-lsm-varkv)

## Implementation log

- Copy from tofino-xindex-dpdk-lsm to tofino-xindex-dpdk-lsm-varkv
- Support 16B key
	+ Modify client.c and server.c
- Apply config module, key-based routing and YCSB integration (see docs/netreach.md)
- Change scan from key+num to start_key+end_key (applied from netreach-voting)
	+ Optional: use num to restrict # of per-subrequest kv pairs
	+ Change volatile rte_mbuf * to rte_mbuf * volatile (client.c, server.c, ycsb_remote_client.c, ycsb_server.c)
	+ Add endkey into ScanRequest and ScanResponse (packet_format.h, packet_format_impl.h, client.c, server.c, ycsb_remote_cliet.c, ycsb_server.c)
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

## Fixed issues
