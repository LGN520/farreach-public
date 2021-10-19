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
