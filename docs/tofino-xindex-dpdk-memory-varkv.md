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
	- Add variable length value into XIndex (replace original uint64_t with uint64_t[16])
		+ Support val_length + val_data (max length: 16*8B) in AtomicVal (xindex_util.h, xindex_group_impl.h)
		+ Compile and test
			* localtest w/ 1 thread under read-only workload: 200K op/s
		+ Debug of segmengtation fault (pointer, array boundary)
			+ NOTE: RCU-based method supports pointer-type dynamic variable, while version-based (copy-for-read) method only support static variable
				* If using pointer-type dyanmic variable under version-based method, before assigning the pointer to the memory of new value, the pointer may still 
				point to the memory of old value which has been deleted, copying the deleted memory will incur segmentation fault!!!
			+ NOTE: if Val.val_length == 0, Val.val_data == nullptr, which cannot be used in memcpy -> segnmentation fault otherwise!!!
	- Support snapshot based on copy-on-write like Redis
		+ Update AtomicVal (xindex_util.h)
			+ Prepare val' (val_length' + val_data') and status' for snapshot
			* For each AtomicVal, make snapshot
				- Copy status as status' (status' != 0 means being snapshoted), copy val as val' (shallow copy, point to the same data)
			* If delete AtomicVal, touch status instead of status'
			* If modify AtomicVal, change val instead of val' (check address of data to decide whether to free the space of old data)
				- If old data address of val = data address of val' -> not free; otherwise, free
			* If during compact, copy status, status', val, and val' in ReplacePointer (now val and val' must point to different memory)
		+ Add make_snapshot() (xindex_impl.h, xindex_root_impl.h, xindex_group_impl.h)
		+ Change DataSource and ArrayDataSource to read_snapshot() (xindex_util.h, xindex_group_impl.h, xindex_buffer_impl.h)
- Implement extended xindex 
	+ Add macro of ORIGINAL_XINDEX to allow the switching between original xindex and extended version
	+ Place is_snapshot atomic flag and snapshot_id into xindex (xindex.h, xindex_impl.h)
		* Monotically increase snapshot_id to idenfity the current epoch ID for snapshot
	+ Deprecated
		+ Save ss_id and create_id for each atomic value; we maintain two versions for snapshot to support unfinished range query during making snapshot (xindex_util.h)
			* NOTE: we assume that time of range query cannot exceed snapshot period (snapshot is an infrequent event)
			* ss_id = -1: empty snapshot value; ss_id = k: existing entry with snapshot value of epoch k
			* create_id = k: the entry is created at epoch k
			* We snapshot the value only if
				- Both ss_id_0 and ss_id_1 != snapshot_id: the snapshot of current epoch has not been made
				- And create_id < snapshot_id: the entry is created at a previous epoch
				- And one of ss_id_0 and ss_id_1 < snapshot_id: replace the out-of-date snapshot
		+ Pass snapshot_id to each operation (init/SCAN/PUT/DEL) for consistent snapshot of atomic values (xindex.h, xindex_root.h, xindex_group.h, xindex_buffer.h, and xindex\*\_impl.h)
			* NOTE: compact creates pointer-type atomic values, which do not need snapshot_id to initialize create_id
			* When inserting a new entry, set ss_id = -1 and create_id = snapshot_id
			* When updating an existing entry, we try to snapshot the old value if necessary
			* When reading a snapshot
				- Read the snapshot value only if either ss_id_0 or ss_id_1 == snapshot_id
				- Read the latest value only if no matched snapshot (both != snapshot_id) and create_id < snapshot_id
				- Return false (i.e., treat it as non-existing or deleted for the snapshot due to a new entry) otherwise 
	+ Standard multi-versioning algorithm
		+ We maintain one latest version and two snapshot versions to support unfinished range query during making snapshot (xindex_util.h)
			* NOTE: we assume that time of range query cannot exceed snapshot period (snapshot is an infrequent event)
			* Latest version has value, latest_id (snapshot id of latest PUT/DEL), and status (islock, isptr, isremoved, version check)
				- NOTE: latest_id is always larger than the two ss_id, and latest_id >= 0
			* Each snapshot version of 2 has ss_value, ss_id (id of the snapshot), and ss_status (isremoved, version check)
				- NOTE: ss_id = -1: empty snapshot value; ss_id = k: valid entry with snapshot value at the beginning of epoch k
				- NOTE: snapshot version may be in removed status
					+ In normal case, if latest version is removed, xindex will insert new record instead of updating removed one and making snapshot
					+ However, during compact, removed value of data may be merged into atomic value of buffer as a snaoshot!
			* We increase the snapshot id by 1 when making snapshot
				- NOTE: snapshot id can be local to each server in a practical distributed KVS
		+ Pass snapshot_id to each operation (init/SCAN/PUT/DEL) for consistent snapshot of atomic values (xindex.h, xindex_root.h, xindex_group.h, xindex_buffer.h, and xindex\*\_impl.h)
			* When inserting a new record
				- Set latest_id = current snapshot_id and update the latest vlaue
			* When updating an existing record 
				- If latest_id = current snapshot_id, update the lastest value
				- Otherwise, replace older snapshot version with the latest version, and update lastest value as well as latest_id
			* When reading a snapshot
				- If latest_id < snapshot_id, return latest_value
				- Otherwise, find the max ss_id != -1 and < snapshot_id, return corresponding value
				- Otherwise, return false
		+ Fix snapshot issues when compact and group merge/split without range query
			* NOTE: we only answer range query based on the latest snapshot
			* NOTE: all of data, buffer, and buffer_temp may have snapshot of the same key
				- Case 1: data is not removed (buffer and buffer_temp cannot have the key)
				- Case 2: data is removed or not exist, buffer is not removed (buffer_temp cannot have the key)
					+ If data is removed, buffer.min_snapshot_id >= data.max_snapshot_id
				- Case 3: data is removed or not exist, buffer is removed or not exist, buffer_temp is not removed
					+ If data is removed and buffer is removed, buffer.min_snapshot_id >= data.max_snapshot_id
					+ If buffer is removed, buffer_temp.min_snapshot_id >= buffer.max_snapshot_id
				- Case 4: data is removed or not exist, buffer is removed or not exist, buffer_temp is removed or not exist
					+ If data is removed and buffer is removed, buffer.min_snapshot_id >= data.max_snapshot_id
					+ If buffer is removed and buffer_temp is removed, buffer_temp.min_snapshot_id >= buffer.max_snapshot_id
			* We do not need to consider the records for range query if read_snapshot() returns false 
				- Case 1: directly try to get snapshot from data
				- Case 2
					+ If snapshot_id <= data.max_snapshot_id: data.read_snapshot() may return true while buffer must return false
					+ If snapshot_id > data.max_snapshot_id: data must return false due to removed while buffer may return true
					+ NOTE: data and buffer cannot return true for the same key -> do not need to fix two valid snapshots for range query
				- Case 3 and 4: similar as case 2, at most one of data/buffer/buffer_temp.read_snapshot() can return true
			- We only need to consider the removed records for compact between (a value in) data and (a value in) buffer
				- If they have records with the same key
					+ Case 1: data is all removed and buffer is all removed -> ignore both data and buffer
   					+ Case 2: data is all removed and buffer is not all removed -> ignore data and keep buffer
						* We do not need to merge snapshots from data into buffer even if buffer has empty snapshot entries
						* Reason: buffer.min_snapshot_id >= data.max_snapshot_id and data is all removed
					+ Case 3: data is not all removed and buffer is all removed -> try to merge data into buffer and keep buffer
					+ Case 4: data is not all removed and buffer is not all removed -> try to merge data into buffer and keep buffer
				+ Implement all_removed() and all_removed_ignoring_ptr() (xindex_util.h)
					* Return true only if the latest version is removed and both snapshots are removed or not existing 
					* Invoke them in ArrayRefSource and merge_refs_internal (xindex_group_impl.h)
				+ Implement merge_snapshot() (xindex_util.h)
					+ If buf_val has base_val.latest_id, try to merge two snapshot versions of data into buffer if with empty entries
					+ Otherwise, try to merge the latest version and a newer snapshot version of data into buffer if with empty entries
					* Invoke it in merge_refs_internal before adding buf_val into new_data (xindex_group_impl.h)
				- NOTE: we do not need to change group split/merge
					+ Group split: compact current group to get a compacted data, and split the data into two groups
					+ Group merge: compact each of two groups to get a compacted data, and concatenate two data for the new group
					+ Since different groups must have disjoint key ranges, we do not need to consider merging multiple snapshots for the same key
			* NOTE: only PUT/DEL in data/buffer/buffer_temp can ignore record with removed latest version
				- Reason: they only perform in-place PUT/DEL and possible snapshot for unremoved record
		* Improve extended xindex by avoiding unnecessary memcpy during execution phase of optimistic locking
		* Implement dynamic-memory-based variable-length value for space efficiency
	+ All of TommyDS (NetCache), Redis (DistCache), Memcached do not support range query for key-value store (i.e., sorted map)
		* We cannot use levelDB (TurboKV) as it is not in-memory (not focus on small objects)
		* We cannot self-implemented in-memory KVS (Pegasus) as it does not mention range query and Pegasus already has an in-switch design
		* We can use xindex with simple extension as it is state-of-the-art in-memory KVS with range query
- Fix segmentation faults
	+ Use gdb to locate segmentation fault by backtrace
	+ Common errors
		* memcpy_avx_unaligned: access null address or unaligned address (new should return 16-byte-aligned memory address)
			- If new returns not-null pointer yet still raise the error, it means the memory address is not aligned and your program has some other bug for memory management
		* je_extent_heap_remove: incorrect size passed to delete
- Fix degraded range perf
	+ Use scan_i instead of query_i in localtest.c
	+ The fluctutaion of perf is due to the design of original xindex

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
