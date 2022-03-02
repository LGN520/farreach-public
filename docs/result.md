# Simple test results

## Localtest of original XIndex w/o log

- Original XIndex: 16B key + 8B value w/o snapshot (read-only)
	+ 1 thread: 4.9 Mops
	+ 32 threads: 129 Mops
- Extended XIndex: 16B key + variable-length value w/ snapshot (read-only)
	+ 1 thread: 2.7 Mops
	+ 32 threads: 59.7 Mops
	+ 32 threads w/ 1 thread for making snapshot: 42.5 Mops
	+ 32 threads w/ 1 thread for making snapshot (0.5 range query): 3.9 Mops (due to large contention)
	+ 32 threads (0.5 range query): 58.8 Mops
