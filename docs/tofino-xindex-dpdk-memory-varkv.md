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
- Deprecated
	- Add variable length value into XIndex (replace original uint64_t with Val (pointing to variable-length value data) in vector)
		+ Merge variable length Val into AtomicVal (val.c, xindex_util.h, xindex_group_impl.h)
		+ Compile localtest.c, ycsb_server.c, and ycsb_remote_client.c
			* Error: use of deleted function -> need to define some function explicitly
			* Reason: C++ will not define fuction of a class implicitly if the internal member has explicit function
		+ Test xindex with varkv -> localtest: 127561 op/s with 1 thread
	- Support snapshot operation in XIndex based on copy-on-write like Redis
		* Use Val\* instead of Val for value in AtomicVal (xindex_util.h, xindex_group_impl.h)
			* By doing this, we can keep copy constructor of both Val and AtomicVal as deep copy, while using pointer copy to achieve copy-on-write of snapshot
			* Otherwise, if we directly maintain Val in AtomicVal, Val must support shallow copy which is not transparent to snapshot
		* For each AtomicVal, make snapshot
			- Copy status as status' (status' != 0 means being snapshoted), copy val as val' (shallow copy, point to the same data)
		* If delete AtomicVal, touch status instead of status'
		* If modify AtomicVal, change val instead of val' (check address of data to decide whether to free the space of old data)
			- If old data address of val = data address of val' -> not free; otherwise, free
		* If during compact, copy status, status', val, and val' in ReplacePointer (now val and val' must point to different memory)
		* NOTE: we do not need the peace period in RCU (copy-on-write + peace period)
- Variable length value and snapshot (based on the deprecated version)
	+ NOTE: RCU-based method supports pointer-type dynamic variable, while version-based (copy-for-read) method only support static variable
		* If using pointer-type dyanmic variable under version-based method, before assigning the pointer to the memory of new value, the pointer may still 
		point to the memory of old value which has been deleted, copying the deleted memory will incur segmentation fault!!!
	- Add variable length value into XIndex (replace original uint64_t with uint64_t[16])
		+ Support val_length + val_data (max length: 16*8B) in AtomicVal (xindex_util.h, xindex_group_impl.h)
		+ Compile and test
			* localtest w/ 1 thread under read-only workload: 200K op/s
		+ NOTE: if Val.val_length == 0, Val.val_data == nullptr, which cannot be used in memcpy -> segnmentation fault otherwise!!!
	- Support snapshot based on copy-o-write like Redis
		+ Prepare val' (val_length' + val_data') and status' for snapshot
		* For each AtomicVal, make snapshot
			- Copy status as status' (status' != 0 means being snapshoted), copy val as val' (shallow copy, point to the same data)
		* If delete AtomicVal, touch status instead of status'
		* If modify AtomicVal, change val instead of val' (check address of data to decide whether to free the space of old data)
			- If old data address of val = data address of val' -> not free; otherwise, free
		* If during compact, copy status, status', val, and val' in ReplacePointer (now val and val' must point to different memory)

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
