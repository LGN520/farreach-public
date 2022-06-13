# Simple test results

## Preliminary results of netreach-v4-lsm

+ Test runtime throughput under synthetic write-only workload (throughput calculation includes client-side overhead)
	* Different # of server threads under range partition
		- NOTE: CPU operation = memory access; disk operation = disk access / flush / sync write
		- NoCache w/ CPU+disk operation (10M queries)
			+ Server 1
				* Client 1: 0.014 MOPS; client 8: 0.028 MOPS; client 32: 0.026 MOPS
			+ Server 4
				* Client 32: 0.053 MOPS; client 64: 0.054 MOPS
			+ Server 8
				* Client 32: 0.068 MOPS; client 64: 0.067 MOPS
			+ Server 16
				* Client 64: 0.08 MOPS; Client 128: 0.08 MOPS
			+ Server 32
				* Client 64: 0.071 MOPS; Client 128: 0.072 MOPS; Client 256: 0.68 MOPS; client 512: 0.07 MOPS
			+ TODO: Server 128
			+ NoCache w/ CPU+disk has 2X thpt from server 1 to server 4 due to batch flush from memtable to sstable
				* Reason: some writes are processed by memtable in memory, while others are processed by sstable in disk
				* If server thread only accesses memory, server 4 should be 4X thpt of server 1
				* If server thread only accesses disk, server 4 should be 1X thpt of server 1(no change)
		- FarReach w/ CPU+disk operation (10M queries)
			+ Server 4 (emulated thpt: 2.45X; normalized thpt: 5X)
				* Client 32: 0.13 MOPS; client 64: 0.13 MOPS
			+ Server 8 (emulated thpt: 3.5X; normalized thpt: 8.2X)
				* Client 32: 0.194 MOPS; client 64: 0.21 MOPS; client 128: 0.24 MOPS; client 256: 0.24 MOPS
			+ Server 16 (emulated thpt: 4.1X; normalized thpt: 13.9X)
				* Client 64: 0.33 MOPS; client 128: 0.32 MOPS
			+ Server 32 (emulated thpt: 5.5X; normalized thpt: 23X)
				* Client 64: 0.27 MOPS; client 128: 0.31 MOPS; client 256: 0.395 MOPS; client 512: 0.388 MOPS
			+ TODO: Server 128
			+ FarReach w/ CPU+disk has 2.45X thpt of NoCache w/ CPU+disk under server 4
				* Reason: some writes are processed by memtable in memory, while others are processed by sstable in disk
				* 50% traffic hits in switch w/ large thpt -> 2X
				* Traffic uses CPU/memory can benefit from load balance -> >2X
				* Traffic uses disk cannot benefit from load balance
		- TODO: NoCache w/ only disk operation (1K queries) -> TODO: expected: similar/limited_difference with hash partition
		- TODO: NoCache w/ only CPU operation
	* Different # of server threads under hash partition
		- NoCache w/ CPU+disk operation (10M queries)
			+ Server 1
				* Client 8: 0.03 MOPS; clien 32: 0.03 MOPS
			+ Server 4
				* Client 32: 0.077 MOPS; client 64: 0.08 MOPS; client 128: 0.08 MOPS
			+ Server 8
				* Client 32: 0.14 MOPS; client 64: 0.13 MOPS
			+ Server 16
				* Client 64: 0.14 MOPS; client 128: 0.157 MOPS; client 256: 0.157 MOPS
			+ Server 32
				* Client 128: 0.137 MOPS; client 256: 0.17 MOPS
			+ Server 128
				* Client 256: 0.1 MOPS; client 512: 0.1 MOPS
			+ NoCache under hash > NoCache under range w/ server 4
				* Reason: hash partition is more balanced than range partition + traffic access memory can benefit from load balance
		- FarReach w/ CPU+disk operation (10M queries)
			+ Server 4 (emulated thpt: 1.6X; normalized thpt: 2.6X)
				* Client 32: 0.123 MOPS; client 64: 0.13 MOPS
			+ Server 8 (emulated thpt: 1.7X; normalized thpt: 3X)
				* Client 32: 0.185 MOPS; client 64: 0.235 MOPS
			+ Server 16 (emulated thpt: 2.1X; normalized thpt: 3.9X)
				* Client 128: 0.33 MOPS
			+ Server 32 (emulated thpt: 2.12X; normalized thpt: 5.8X)
				* Client 256: 0.36 MOPS; client 512: 0.35 MOPS
			+ Server 128 (emulated thpt: 4X; normalized thpt: 18.16X)
				* Client 256: 0.297 MOPS; client 512: 0.37 MOPS; client 1024: 0.4 MOPS
		- NoCache w/ only disk operation (1K queries)
			+ Server 1
				* Client 8: 0.065 KOPS; clien 32: 0.067 KOPS
			+ Server 4
				* Client 32: 0.087 KOPS; client 64: 0.09 KOPS
			+ Server 8
				* Client 32: 0.15 KOPS; client 64: 0.14 KOPS
			+ Server 16
				* Client 64: 0.18 KOPS; client 128 (2K queries): 0.18 KOPS
			+ Server 32
				* Client 64: 0.2 KOPS; client 128 (2K queries): 0.2 KOPS
			+ Server 128
				* Client 128 (2K queries): 0.257 KOPS (2K queries); client 256 (8K queries): 0.24 KOPS
		- TODO: NoCache w/ only CPU operation

+ DEPRECATED: Test runtime throughput under synthetic write-only workload (throughput calculation excludes client-side overhead)
	* NOTE: WRONG way of calculating system avg thpt, as we do not consider 0ops thpt if some clients finish before system running time, which could incur infinitely-increased system thpt issue
	* Different # of server threads under hash partition
		- NoCache w/ CPU+disk operation (10M queries)
			+ Server 4
				* Client 32: 0.071 MOPS; client 64: 0.068 MOPS; client 128: 0.079 MOPS; client 256: 0.085 MOPS; client 512: 0.0955 MOPS; client 1024: 0.18 MOPS
			+ Server 32
				* Client 128: 0.156 MOPS; client 256: 0.185 MOPS; client 512: TODO MOPS; client 1024: TODO MOPS
		- FarReach w/ CPU+disk operation (10M queries)
			+ Server 32 (emulated thpt: TODOX; normalized thpt: 5.8X)
				* Client 256: 0.34 MOPS; client 512: 0.44 MOPS; client 1024: 0.617 MOPS; client 2048: TODO MOPS

## Localtest of original XIndex w/o log

- Original XIndex: 16B key + 8B value w/o snapshot (read-only)
	+ 1 thread: 4.9 Mops
	+ 1 thread (0 read and 1 insert): 0.69 Mops
	+ 1 thread (0.5 range query): 1.3 Mops
	+ 1 thread (0 read and 1 range query): 0.7 Mops
	+ 32 threads: 125 Mops
	+ 32 threads (0 read and 1 insert): 33 Mops
	+ 32 threads (0.5 range query): 15.1 Mops
	+ 32 threads (0 read and 1 range query): 8.6 Mops
	+ 32 threads (0.5 read and 0.5 update): 110 Mops
	+ 32 threads (0.3 read and 0.1 insert and 0.1 delete and 0.5 range): 4 Mops
	+ Conclusion: perf is the best of all due to only 8B value
- Extended XIndex: 16B key + max-memory-based varlen value w/ multi-versioning-based snapshot fixing compact issue (read-only)
	+ 1 thread: 2.6 Mops
	+ 1 thread (0 read and 1 insert): 0.28 Mops
	+ 1 thread (0.5 range query): 0.49 Mops
	+ 1 thread (0 read and 1 range query): 0.27 Mops
	+ 32 threads: 58 Mops
	+ 32 threads (0 read and 1 insert): 13.6 Mops
	+ 32 threads (0.5 range query): 4.6 Mops
	+ 32 threads (0 read and 1 range query): 2.5 Mops
	+ 32 threads (0.5 read and 0.5 update): 67 Mops
	+ 32 threads (0.3 read and 0.1 insert and 0.1 delete and 0.5 range): 2.34 Mops
	+ 32 threads w/ 1 thread for making snapshot: 34.5 Mops
	+ 32 threads w/ 1 thread for making snapshot (0.5 range query): 4.6 Mops
	+ Conclusion: perf is slightly better than followings due to w/o dynamic memory management
- Extended XIndex: 16B key + dynamic-memory-based varlen value w/ multi-versioning-based snapshot fixing compact issue (read-only)
	+ 1 thread: 2.6 Mops
	+ 1 thread (0 read and 1 insert): 0.31 Mops
	+ 1 thread (0.5 range query): 0.49 Mops
	+ 1 thread (0 read and 1 range query): 0.26 Mops
	+ 32 threads: 51 Mops
	+ 32 threads (0 read and 1 insert): 14.1 Mops
	+ 32 threads (0.5 range query): 4.3 Mops
	+ 32 threads (0 read and 1 range query): 1.7 Mops
	+ 32 threads (0.5 read and 0.5 update): 54 Mops
	+ 32 threads (0.3 read and 0.1 insert and 0.1 delete and 0.5 range): 2.2 Mops
	+ 32 threads w/ 1 thread for making snapshot: 33.5 Mops
	+ 32 threads w/ 1 thread for making snapshot (0.5 range query): 3 Mops

## Localtest of XIndex+ based on BF w/o log
	+ 1 thread: 1.9 Mops (hashnum=3) or 2.1 Mops (hashnum=1)
	+ 1 thread (0 read and 1 insert): 0.34 Mops
	+ 1 thread (0.5 range query): 0.47 Mops
	+ 1 thread (0 read and 1 range query): 0.26 Mops
	+ 32 threads: 40.2 Mops
	+ 32 threads (0 read and 1 insert): 16.5 Mops
	+ 32 threads (0.5 range query): 4.4 Mops
	+ 32 threads (0 read and 1 range query): 2.4 Mops
	+ 32 threads (0.5 read and 0.5 update): 45 Mops
	+ 32 threads (0.3 read and 0.1 insert and 0.1 delete and 0.5 range): 2.13 Mops
	+ 32 threads w/ 1 thread for making snapshot: 24 Mops
	+ 32 threads w/ 1 thread for making snapshot (0.5 range query): 4.1 Mops
	+ Conclusions
		* Worse GET/UPDATE perf as the workloads always access elements in data (BF is an extra overhead)
		* Better INSERT perf as the workloads touch new elements in buffer (BF can save cost of access data)
		* We need to decide whether using BF optimization based on workload and evaluation results

## Deprecated: Localtest of XIndex+ based on CBF w/o log
	+ 1 thread: 0.8 Mops
	+ 1 thread (0.5 range query): 0.4 Mops
	+ 1 thread (0 read and 1 range query): 0.26 Mops
	+ 32 threads: 14.3 Mops
	+ 32 threads (0.5 range query): 2.9 Mops
	+ 32 threads (0 read and 1 range query): 2.2 Mops
	+ 32 threads (0.5 read and 0.5 update): 8.8 Mops
	+ Conclusion: don't need to test further: too much overhead on rwlock in CBF!!!
