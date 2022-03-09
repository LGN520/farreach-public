# Simple test results

## Localtest of original XIndex w/o log

- Original XIndex: 16B key + 8B value w/o snapshot (read-only)
	+ 1 thread: 4.9 Mops
	+ 32 threads: 129 Mops
	+ 32 threads (0.5 range query): 15 Mops
	+ 1 thread (0.5 range query): 1.3 Mops
- Extended XIndex: 16B key + variable-length value w/ snapshot (read-only)
	+ 1 thread: 2.7 Mops
	+ 32 threads: 59.7 Mops
	+ 32 threads w/ 1 thread for making snapshot: 42.5 Mops
	+ 32 threads w/ 1 thread for making snapshot (0.5 range query): 3.9 Mops (due to large contention)
	+ ~~32 threads (0.5 range query): 58.8 Mops (wrong)~~
- Newly extended XIndex: 16B key + variable-length value w/ multi-versioning-based snapshot fixing compact issue (read-only)
	+ 1 thread: 2.7 Mops
	+ 32 threads: 62 Mops
	+ 32 threads w/ 1 thread for making snapshot: 38.2 Mops
	+ 32 threads w/ 1 thread for making snapshot (0.5 range query): 4.2 Mops (due to large contention)
	+ 32 threads (0.5 range query): 4.3 Mops (reasonable)
	+ 32 threads (0.5 read and 0.5 write): 19.1 Mops
	+ 1 thread (0.5 range query): 0.45 Mops
- Improve newly extended xindex w/ at most one memcpy per-op (same setting as the previous one)
	+ 1 thread: 2.7 Mops
	+ 32 threads: 58 Mops
	+ 32 threads w/ 1 thread for making snapshot: 38 Mops
	+ 32 threads w/ 1 thread for making snapshot (0.5 range query): 4.2 Mops
	+ 32 threads (0.5 range query): 4.7 Mops (reasonable)
	+ 32 threads (0.5 read and 0.5 write): 20 Mops
	+ 1 thread (0.5 range query): 0.47 Mops
_ Introduce dynamic memory management
	+ 1 thread: 2.5 Mops (due to rwlock overhead)
	+ 32 threads: 55 Mops (due to rwlock overhead)
	+ 32 threads (0.5 range query): 4.3 Mops (reasonable)
	+ 32 threads (0.5 read and 0.5 write): 21.7 Mops
	+ 1 thread (0.5 range query): 0.48 Mops
