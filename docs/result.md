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

## Preliminary Exp of Microbench (local test with persistency)

- GET latency
	+ Per-sstable: 4MB; Key-value number: 10MB
		* XIndex with rocksdb: 100809 op/s = 9.92us
		* XIndex with rocksdb-model: 103611 op/s = 9.65us
	+ Per-sstable: 4MB; Key-value number: 100MB
		* XIndex with rocksdb: 103302 op/s
		* XIndex with rocksdb-model: 104692 op/s
	+ Per-sstable: 16MB; Key-value number: 100MB
		* XIndex with rocksdb: 93607 op/s
		* XIndex with rocksdb-model: 97598 op/s
- SYNC PUT latency
	+ Per-sstable: 4MB; Key-value number: 100MB
		* XIndex with rocksdb: 1748 op/s
		* XIndex with rocksdb-model: 1792 op/s
- ASYNC PUT latency
	+ Per-sstable: 4MB; Key-value number: 100MB
		* XIndex with rocksdb: 50655 - 55602 op/s
		* XIndex with rocksdb-model: 49698 - 54514 op/s

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
