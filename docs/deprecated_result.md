# Result

## Run

- Latency
	- Server side: `./prepare; ./server`
	- Client side
		+ Read-only: `./prepare; ./client`
		+ 90% read - 10% insert: `./prepare -b 0.1; ./client -a 0.9 -b 0.1`
		+ 80% read - 10% insert - 10% remove: `./prepare -b 0.1; ./client -a 0.8 -b 0.1 -c 0.1`
		+ 70% read - 10% insert - 10% remove - 10% update: `./prepare -b 0.1; ./client -a 0.7 -b 0.1 -c 0.1 -d 0.1`
		+ 60% read - 10% insert - 10% remove - 10% update - 10% scan: `./prepare -b 0.1; ./client -a 0.6 -b 0.1 -c 0.1 -d 0.1 -e 0.1`
- NOTES
	+ All preliminary results are with logging, which should be disabled in real evaluation
	+ All preliminary results are based on single thread, so the the results (op/s) actually represent latency
	+ For throughput, we should calculate especially for NetBuffer

## Preliminary Exp of Microbench (with persistence with CBF with 16B key)

- Local test
	+ GET
		+ XIndex: 83294 op/s
		+ NetBuffer: 84076 op/s
	+ UPDATE
		+ XIndex: 54950 op/s
		+ NetBuffer: 54117 op/s
	+ INSERT
		+ XIndex: 52611 op/s
		+ NetBuffer: 58529 op/s
		+ Reason: To count buffer size, we need check whether the key is a new key in buffer. XIndex needs to get the key in LSM,
		while NetBuffer can quickly determine by CBF
	+ DEL
		+ XIndex: 52224 op/s
		+ NetBuffer: 55500 op/s
	+ SCAN
		+ XIndex: 14631 (scan after del) / 32000 (only scan) op/s
		+ NetBuffer (merge with backup data): 17205 (scan after del) / 31000 (only scan) op/s
- Remote test
	+ GET
		+ XIndex: 31042 op/s
		+ NetBuffer: 30549 op/s
	+ UPDATE
		+ XIndex: 29389 op/s
		+ NetBuffer: 41149 op/s
	+ INSERT
		+ XIndex: 28873 op/s
		+ NetBuffer: 41501 op/s
	+ DEL
		+ XIndex: 31031 op/s
		+ NetBuffer: 30876 op/s (hit in server)
	+ SCAN
		+ XIndex: 13157 (only scan) op/s
		+ NetBuffer (merge with backup data): 12932 (only scan) op/s

## We must use perf-consistency trade-off for SCAN

- XIndex: 30us
- delta = GetKV + TransmissionDelay + UpdateKV + Merge
	+ GetKV: 2.8s for 1MB data
	+ TransmissionDelay: 1MB/40Gbps = 195us
	+ UpdateKV: >= tens to hundreds of us (not be accurately measured now)
        + MergeKV: 1-2 us
- So we must put GetKV/TransmissionDelay/UpdateKV in the background, i.e., periodic update backup in server
	+ Option: If MergeKV incurs large time overhead, consider to train learned index in the background for backup data


## Preliminary Exp of Microbench (local test with persistency with CBF)

- XIndex: 100K op/s
- NetBuffer: 170K op/s (all hit CBF, i.e., buffer)

## Preliminary Exp of Microbench (remote test with persistency)

- GET latency
	+ All hit server: 31119 op/s = 32.13us (similar as remote test without persistency due to DPDK PMD implementation)
	+ All hit switch: 41506 op/s = 24.09us (eliminate the 8us-overhead in server)
- PUT latency
	+ Since all puts must hit switch, the latency is similar as GET latency
	+ NOTE: if the workload is linear or random, then server will be overloaded. However, we assume that workload is skewed, thus most
	puts will hit switch without evicting key-value pairs to server

## Preliminary Exp of Microbench (local test with persistency)

- GET latency
	+ Per-sstable: 4MB; Key-value number: 10MB
		* XIndex with rocksdb: 100809 op/s = 9.92us
		* XIndex with rocksdb-model: 103611 op/s = 9.65us
	+ Per-sstable: 4MB; Key-value number: 100MB
		* XIndex with rocksdb: 103302 op/s = 9.68us
		* XIndex with rocksdb-model: 104692 op/s = 9.5us
	+ Per-sstable: 16MB; Key-value number: 100MB
		* XIndex with rocksdb: 93607 op/s = 10.68us
		* XIndex with rocksdb-model: 97598 op/s = 10.25us
- SYNC PUT latency
	+ Per-sstable: 4MB; Key-value number: 100MB
		* XIndex with rocksdb: 1748 op/s
		* XIndex with rocksdb-model: 1792 op/s
- ASYNC PUT latency
	+ Per-sstable: 4MB; Key-value number: 100MB
		* XIndex with rocksdb: 50655 - 55602 op/s
		* XIndex with rocksdb-model: 49698 - 54514 op/s

## Preliminary Exp of Microbench (local test without persistency)

- 8B-value XIndex without log: 7M op/s = 0.14us 
- Var-value XIndex with log: 3Mop/s = 0.31us
- Var-value XIndex with log: 200K op/s = 0.47us

## Preliminary Exp of microbench (remote test without persistency)

- Latency
	* Kernel stack
		* Read-only
			- XIndex: 19852 op/s = 50.37us
			- NetBuffer: 19874 op/s = 50.32us
		* 90% read - 10% insert
			- XIndex: 19857 op/s = 50.36us
			- NetBuffer: 19833 op/s = 50.42us
		* 80% read - 10% insert - 10% remove
			- XIndex: 19867 op/s = 50.33us
			- NetBuffer: 19845 op/s = 50.39us
		* 70% read - 10% insert - 10% remove - 10% update
			- XIndex: 19841 op/s = 50.4us
			- NetBuffer: 19780 op/s = 50.56us
		* 60% read - 10% insert - 10% remove - 10% update - 10% scan
			- XIndex: 19843 op/s = 50.39us
		* Debug test
			- XIndex: 19876 op/s = 50.31us
			- NetBuffer; 20025 op/s = 49.93us
	* DPDK
		* Read-only
			- XIndex: 31135 op/s = 32.12us
