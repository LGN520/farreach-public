# Simple test results

## Localtest of original XIndex w/o log

- Original XIndex: 16B key + 8B value w/o snapshot (read-only)
	+ 1 thread: 4.9 Mops
	+ 1 thread (0.5 range query): 3 Mops
	+ 32 threads: 125 Mops
	+ 32 threads (0.5 range query): 77 Mops
	+ 32 threads (0.5 read and 0.5 update): 50 Mops
	+ 32 threads (0.3 read and 0.1 insert and 0.1 delete and 0.5 range): 4.9 Mops
- Extended XIndex: 16B key + max-memory-based varlen value w/ multi-versioning-based snapshot fixing compact issue (read-only)
	+ 1 thread: 2.6 Mops
	+ 1 thread (0.5 range query): 1.9 Mops
	+ 32 threads: 58 Mops
	+ 32 threads (0.5 range query): 43 Mops
	+ 32 threads (0.5 read and 0.5 update): 21 Mops
	+ 32 threads (0.3 read and 0.1 insert and 0.1 delete and 0.5 range): 8.2 Mops
	+ 32 threads w/ 1 thread for making snapshot: 34.5 Mops
	+ 32 threads w/ 1 thread for making snapshot (0.5 range query): 29.4 Mops
- Extended XIndex: 16B key + dynamic-memory-based varlen value w/ multi-versioning-based snapshot fixing compact issue (read-only)
	+ 1 thread: 2.6 Mops
	+ 1 thread (0.5 range query): 1.9 Mops
	+ 32 threads: 51 Mops
	+ 32 threads (0.5 range query): 41 Mops
	+ 32 threads (0.5 read and 0.5 update): 21 Mops
	+ 32 threads (0.3 read and 0.1 insert and 0.1 delete and 0.5 range): 7.3 Mops
	+ 32 threads w/ 1 thread for making snapshot: 33.5 Mops
	+ 32 threads w/ 1 thread for making snapshot (0.5 range query): 27.7 Mops
