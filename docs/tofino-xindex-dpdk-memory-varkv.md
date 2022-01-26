# Tofino + DPDK-based XIndex with in-memory KVS + variable-length key-value pair + YCSB (tofino-xindex-dpdk-memory-varkv)

## Implementation log

- Copy from tofino-xindex-dpdk-lsm-varkv to tofino-xindex-dpdk-memory-varkv
- Replace rocksdb with XIndex
	+ Merge ycsb_local_client.c into ycsb_server.c (cannot split loading phase and running phase)
		* Loading phase
			- Load split_n-1 slice as keys and values to initialize xindex
			- Load 0 ~ split_n-2 slices to put data into xindex
		* Runing phase
			- Launch fg_n threads to handle requests sent by ycsb_remote_client.c
	+ Remove rocksdb (xindex_util.h)
	+ Remove workload_name, open() and data_put(), and retrieve RMI training fro group_n during initialization (xindex.h, xindex_impl.h)
	+ Remove workload_name, open(), data_put(), serialize_root(), and deserialize_root(), retrieve background model/group merge/split (xindex_root.h, xindex_root_impl.h)
	+ Remove rocksdb, open(), and data_put() (xindex_group.h, xindex_group_impl.h)
- Add variable length value into XIndex
	+ Merge variable length Val into AtomicVal (val.c, xindex_util.h, xindex_group_impl,h)
	+ Compile localtest.c, ycsb_server.c, and ycsb_remote_client.c
		* Error: use of deleted function -> need to define some function explicitly
		* Reason: C++ will not define fuction of a class implicitly if the internal member has explicit function
	+ Test xindex with varkv -> localtest: 127561 op/s with 1 thread
- TODO: add snapshot into xindex
- TODO: test xindex with snapshot

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
