# Simple test results

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
