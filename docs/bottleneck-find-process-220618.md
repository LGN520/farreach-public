# Process of finding the bottleneck under range partition (netreach-v4-lsm; 2022.06.18)

+ Treat system as blackbox (test in client side)
	* Test FarReach runtime thpt w/ 1 server thread
		- Result: normalized thpt is 2.3X, yet runtime thpt is 1.43X
		- Reason: rocksdb is more friendly to skewed workload
	* Running time factor: test client-side system throughput for every 5 seconds
		- NOTE: NoCache has longer running time of FarReach
		- Result with server 1 + client 32: thpt of both FarReach (~0.04 MOPS) and NoCache (~0.028 MOPS) are relatively stable against running time
		- Observation: running time does not affect runtime throughput
	* Workload factor: use different workloads (server 1 + client 32, 64MB memtable, 4 flushnum, 5 maxnum, w/ WAL flush)
		- Result
			- NoCache with Zipf workload: 0.0336 MOPS (1.72 us wait + 26.72 us process)
			- NoCache with uniform workload: 0.024 MOPS (1.7us wait + 38.07 us process)
		- Observation: both memory access and disk flush of rocksdb are more friendly to skewed workload than uniform workload
+ Network overhead
	* Test latency between sending response and receiving next request -> idle time of server is similar
		- Result
			+ server 1 + client 1: NoCache 27 us; FarReach 54.33 us
			+ server 1 + client 8: NoCache 1.72 us; FarReach 1.82 us
			+ server 1 + client 32: NoCache 1.76 us; FarReach 1.79 us
		- Observation: FarReach has longer idle time under client 1 as some packets are processed by in-switch cache, which make server to wait for a longer time
		- Observation: # of client threads is sufficient to saturate server
+ Treat server as blackbox (test in server side)
	* Test per-server thread average latency of processing normal request -> processing capability of server
		- Result
			+ server 1 + client 1: NoCache 41 us; FarReach 64.95 us
			+ server 1 + client 8: NoCache 38.9 us; FarReach 50.97 us
			+ server 1 + client 32: NoCache 36.07 us; FarReach 51.57 us
		- Observation: FarReach has larger processing time in server than NoCache as workload becomes uniform w/ in-switch cache
+ Disk overhead
	* Some NOTEs about rocksdb
		* Rocksdb writes WAL for a fixed # of operations (e.g., every 4KB) -> sync_write enforces WAL flush instead of memtable flush for each operation
		* Rocksdb flushes memtable for a fixed # of operations (e.g., every 4MB), even if most are related with the same key -> it may write a small sst file (e.g., one key-value pair), yet WAL overhead is stable -> Rocksdb is more friendly for skewed workload instead of uniform workload
	* Test thpt w/o memtable flush
		- Some NOTEs
			+ Expected normalized thpt improvement: 2.3X
			+ flushnum = min_write_buffer_number_to_merge; maxnum = max_write_buffer_number
			+ memtable_size = sstable_size = block_cache_size; level1_total_size = 8 * sstable_size
		- Result of runtime thpt under server 1 + client 32
			+ 4MB memtable size + 1 flushnum + 2 maxnum -> 1.43X
				* NoCache 0.028 MOPS (1.76 us wait + 36.07 us process)
				* FarReach 0.0395 MOPS (1.79 us wait + 51.57 us process)
			+ 64MB memtable size + 1 flushnum + 2 maxnum -> 1.85X
				* NoCache 0.0318 MOPS (1.71 us wait + 28.35 us process)
				* FarReach 0.0587 MOPS (1.74 us wait + 35.75 us process)
			+ 64MB memtable size + 4 flushnum + 5 maxnum -> 2.08X
				* NoCache 0.0336 MOPS (1.72 us wait + 26.72 us process)
				* FarReach 0.06989 MOPS (1.76 us wait + 29.7 us process)
			+ Larger-scale trials yet not impractical for multi-thread simulation
				* 256MB memtable size + 16 flushnum + 20 maxnum -> 2.1X
					* NoCache 0.04077 MOPS (1.69 us wait + 21.7 us process)
					* FarReach 0.0856 MOPS (1.77us wait + 23.94 us process)
				* 2GB memtable size + 4 flushnum + 5 maxnum -> 2.06X
					* NoCache 0.0405 MOPS (1.85 us wait + 21.75 us process)
					* FarReach 0.0834 MOPS (1.69 us wait + 24.71 us process)
				* Observation: limited runtime thpt improvement compared with 2.08X
					* Reason: memory access perfers skewed workload to uniform workload
		- Observation
			+ Fewer disk operations -> larger runtime throughput: disk is one of bottleneck
			+ Fewer disk operations -> smaller gap between runtime thpt improvement and normalized thpt improvement: rocksdb's disk operations prefers skewed workload to uniform workload
	* Test thpt w/o WAL flush (64 MB memtable size + 4 flushnum + 5 maxnum)
		+ w/ WAL flush -> 2.08X
			* NoCache 0.0336 MOPS (1.72 us wait + 26.72 us process)
			* FarReach 0.06989 MOPS (1.76 us wait + 29.7 us process)
		+ w/o WAL flush -> 1.95X
			* NoCache 0.045 MOPS (1.54 us wait + 19.62 us process)
			* FarReach 0.0879 MOPS (1.4 us wait + 23.64 us process)
		+ Observation: WAL does not affect runtime thpt improvement as it does not perfer to skewed workload
+ Test different # of server threads with setting v2 under range partition
	* Old setting (v1): 4MB memtable size + 1 flushnum + 2 maxnum + w/ WAL flush + 5s timeout + checkseq for each write
	* Setting v2: 64MB memtable size + 4 flushnum + 5 maxnum + w/ WAL flush + 5s timeout + checkseq for each write
	* NoCache
		- server 1
			+ client 32 w/ disk GET seq: 0.0336 MOPS (1.72 us wait + 26.72 us process)
			+ client 32 w/o disk GET seq: 0.0476 MOPS (1.57 us wait + 18.54us process)
			+ client 32 w/ checkseq flag (only for evicted data): 0.0532 MOPS (1.65us wait + 16.35us process)
		- server 4
			+ client 32: 0.059 MOPS (1.7/95/121/138 us wait + 22/35/35/37 us process)
			+ client 64: 0.0606 MOPS (1.6/91/115/133 us wait + 21/35/37/37 us process)
			+ Reason of longer wait time of server 1/2/3: server 0 is overloaded, so other servers cannot run in full speed
			+ Reason of longer process time of server 1/2/3: workload of server 0 is more skewed than other servers
		- server 8
			+ client 64: 0.0694 MOPS (1.64 us min wait + 21 us min process)
		- server 16
			+ client 64: 0.08 MOPS (1.87 us min wait + 20 us min process)
		- server 32
			+ client 64: 0.06607 MOPS (3.7~3.9 us send latency; 2.7 us min wait + 27 us min process)
		- server 128
			+ client 512 w/ 1ms client-side timeout: 0.0279 MOPS (~4.2us send latency; 5.9us min wait + 56us min process)
			+ client 512 w/ 10ms client-side timeout: 0.026 MOPS (~4.2us send latency; 7.7us min wait + 46us min process)
			+ client 512 w/ 5s client-side timeout: 0.02497 MOPS
			+ Reason of long process time: context switching overhead (and other resource contention) under too many server threads
			+ Observation: client-side timeout has little effect on NoCache runtime thpt
	* FarReach
		- server 1 (1.43X -> 2.08X (w/ GET) -> 2.35 (w/o GET) -> 1.36X (w/ seq cache) -> 2.31X (w/ checkseq flag) runtime thpt; 2.3X normalized thpt)
			+ client 32 w/ disk GET seq: 0.06989 MOPS (1.76 us wait + 29.7 us process)
			+ client 32 w/o disk GET seq: 0.112 MOPS (1.55 us wait + 18.17 us process)
			+ IMPORTANT: we always GET seq from rocksdb before PUT, while rocksdb's GET also prefers to skewed workload due to block cache
				* Deprecated solution: we store per-key seq into rocksdb with value, yet we cache seq of recently-accessed keys in memory to reduce server-side extra overhead
				* Correct solution: we only need to checkseq for each evicted write
			+ client 32 w/ seq cache GET seq: 0.0647 MOPS (1.72us wait + 32us process)
			+ client 32 w/ checkseq flag (only for evicted data): 0.1228 MOPS (1.65us wait + 16.28us process)
		- server 4 (2.45X -> 3.83X runtime thpt; 5X normalized thpt)
			+ client 32: 0.174 MOPS (14/10/14/19 us wait + 32/33/38/40 us process)
			+ client 64: 0.2318 MOPS (4.3/2.7/5.4/9.7 us wait + 31/30/34/35 us process)
			+ client 128: 0.229 MOPS (4.7/3.2/5.8/8.4 us wait + 31/30/34/36 us process)
			+ Reason that wait time cannot achieve 1.7 us: multiple sockets contend on the same NIC
			+ Reason of larger gap between runtime and normalized thpt: resource contention on the same NIC, memory, and disk
		- server 8 (3.5X -> 5.6X runtime thpt; 8.2X normalized thpt)
			+ client 64: 0.3707 MOPS (3.6 us min wait + 33 us min process)
			+ client 128: 0.3875 MOPS (2.2 us min wait + 32 us min process)
		- server 16 (4.1X -> 8.3X runtime thpt; 13.9X normalized thpt)
			+ client 128: 0.3277 MOPS (26 us min wait + 42 us min process)
			+ client 256: 0.4965 MOPS (8.5 us min wait + 40 us min process)
			+ client 512 w/ 5s client timeout: 0.5678 MOPS (7.17 us min wait + 36.5 us min process)
			+ client 512 w/ 10ms client timeout: 0.64 MOPS (5.3~5.6us send latency; 3.3 us min wait + 35 us min process)
			+ client 512 w/ 1ms client timeout: 0.665 MOPS (5.3~5.7us send latency; 2.56 us min wait + 36 us min process)
			+ Reason of long wait time
				* Major: too long timeout threshold (5s) in client -> solution: change to 10ms
				* Minor: client suffers from large socket overhead as many threads send pkts to the same NIC
		- server 32 (5.5X -> 12.73X runtime thpt; 23X normalized thpt)
			+ client 64: 0.3277 MOPS (4.3~4.5 us send latency; 81 us min wait + 45 us min process)
			+ client 256: 0.476 MOPS (4.7~5.0 us send latency; 40 us min wait + 46 us min process)
			+ client 512 w/ 5s client timeout: 0.5307 MOPS (5.2~5.5 us send latency; 22 us min wait + 48 us min process)
			+ client 512 w/ 1s client timeout: 0.5915 MOPS (5.2~5.5 us send latency; 17 us min wait + 48 us min process)
			+ client 512 w/ 10ms client timeout: 0.721 MOPS (5.7~6.1 us send latency; 8.8 us min wait + 51 us min process)
			+ client 1024 w/ 10ms client timeout: 0.6618 MOPS (5.6~6.4us send latency; 9.5 us min wait + 64 us min process)
			+ client 512 w/ 1ms client timeout: 0.699 MOPS (5.5~6.1 us send latency; 5.6 us min wait + 60 us min process)
			+ client 1024 w/ 1ms client timeout: 0.841 MOPS (6.9~7.4 us send latency; 4.8 us min wait + 55 us min process)
			+ Reason of gap between runtime and normalized thpt
				* Major: skewed vs. uniform workload on memory access and disk flush
				* Medium: CPU contention under too many server threads
				* Minor: memory/disk/NIC contention under too many server threads
			+ Reason of longer wait latency under larger client-side timeout
				* If client sends a req to a cold server which incurs a disk flush operation, it has to wait for the response before sending following reqs to hot server under a large timeout threshold -> larger wait latency for the hot server
				* If client quickly resends a req to the cold server, the cold server will process it in memtable and sends back response, so the client can continue to send reqs to hot server -> smaller wait latency for the hot server
		- server 128 (26X runtime thpt; 62X normalized thpt)
			+ client 512 w/ 1ms timeout: 0.725 MOPS (6.2~7.3us send latency; 5.83us min wait + 110 us min process)
			+ client 1024 w/ 1ms timeout: 0.747 MOPS (6.3~7.4us send latency; 2.518us min wait + 117 us min process)
			+ NOTE: but both NoCache and FarReach have degraded performance -> FAIL
+ NOTE: disk overhead of rocksdb's GET operation to get per-key seq
+ Test different # of server threads with setting v3 under range partition
	* Setting v3: 64MB memtable size + 4 flushnum + 5 maxnum + w/ WAL flush + 5s timeout + checkseq only for each evicted write
	* FarReach w/o inswitch cache
		- server 1
			+ client 32: 0.0532 MOPS (~3.6us send latency; 1.65us wait + 16.35us process)
		- server 4
			+ client 128: 0.078 MOPS (~3.8us send; 1.7us wait + 16us process)
		- server 8
			+ client 64: 0.087 MOPS (~3.8us send latency; 1.8us min wait + 16.55us min process)
			+ client 64 w/ 100ms timeout: 0.088 MOPS (3.8us send; 1.7us min wait + 17us min process; 400 false positives)
			+ client 64 w/ 200ms timeout: 0.085 MOPS (3.8us send; 1.7us min wait + 17us min process; 300 false positives)
	* FarReach w/ inswitch cache
		- server 1 (2.3X runtime thpt; 2.3X normalized thpt)
			+ client 32: 0.1228 MOPS (~3.6us send latency; 1.65us wait + 16.28us process)
		- server 4 (4.57X runtime thpt; 5X normalized thpt)
			+ client 128: 0.2965 MOPS (~4.2us send; 4.7us min wait + 21us min process)
			+ client 128 w/ 2GB memtable size: 0.2634 MOPS (4.1us send; 5.8us min wait + 23us min process)
			+ client 256: 0.3561 MOPS (~4.2us send; 1.7us min wait + 20us min process)
				+ 2nd run of client 256: 0.3287 MOPS (4.3us send; 2us min wait + 27us min process)
			+ client 512: 0.3403 MOPS (4.3us send; 1.7us min wait + 27us min process)
			+ client 512 w/ 10s timeout: 0.29 MOPS (4.4us send; 5.7us min wait + 20us min process)
			+ client 512 w/ 1s timeuot: 0.351 MOPS (4.5us send; 2us min wait + 24us min process)
		- server 8 (6.5X runtime thpt; 8.2X normalized thpt)
			+ client 64: 0.4658 MOPS (~4.5us send; 3.96us min wait + 24us min process)
			+ client 128: 0.4251 MOPS (~4.6us send; 6.77us min wait + 24us min process)
			+ client 256: 0.5099 MOPS (~4.8us send; 2.5us min wait + 23us min process)
			+ client 512: 0.5291 MOPS (~5.1us send; 2.5us min wait + 22us min process)
			+ client 1024: 0.5490 MOPS (~5.4us send; 2.1us min wait + 21us min process)
			+ client 2048: 0.402 MOPS (~5us send; 9.6us min wait + 23us min process)
				* Reason of lower throughput: too many client threads -> more packet loss
			+ client 1024 w/ 1s timeout: 0.5079 MOPS (4.8us send; 2.4us min wait + 23us min process)
			+ client 1024 w/ 100ms timeout: 0.5655 MOPS (5.5us send; 2us min wait + 21us min process)
	* Observation
		- use 2GB memtable size does not improve thpt -> not due to disk bottleneck -> FAIL
		- server 4/8 w/o inswitch cache can achieve 1.7us wait + 16us process -> not due to context switching overhead -> FAIL
		- REASON 1: uniform vs. skewed workload in server
			+ NOTE: more servers -> more uniform -> longer process time in server (aka smaller processing capability)
			+ Although server 1 can achieve normalized thpt improvement in runtime, it is not uniform enough, and client threads are not many enough (w/o packet loss)
		- REASON 2: no sufficient client threads
			+ NOTE: when runtime thpt is insufficient, min wait time is larger than 1.7us, we need to launch more client threads
		- REASON 3: too large client-side timeout
			+ NOTE: more client threads -> more packet losses (dropped by tofino.ingress port) -> longer wait time in server
	+ Try different workload for FarReach w/o inswitch cache
		* Zipf workload
			- server 1 + client 32: 0.0532 MOPS (~3.6us send latency; 1.65us wait + 16.35us process)
			- server 4 + client 128: 0.078 MOPS (~3.8us send; 1.7us wait + 16us process)
		* Sequential uniform workload
			- server 1 + client 32: 0.0492 MOPS (~3.6us send; 1.5us wait + 18us process)
			- server 4 + client 128: 0.193 MOPS (~4.1us send; 1.5us wait + 16us process)
				+ Reason of larger thpt: no load imbalance under uniform workload
		* Random uniform workload
			- server 1 + client 32: 0.048 MOPS (1.5us wait + 18us process)
			- server 4 + client 256: 0.147 MOPS (2us wait + 28us process)
		* Observation: workload type has effect on throughput but the key reason is no sufficient client threads
+ How to set timeout for each method?
	* NOTE: larger timeout -> longer wait time -> smaller system thpt
	* NOTE: smaller timeout -> more false positives and unnecessary responses to process in client -> smaller system thpt (limited effect?)
	* Implementation: ignore/filter unmatched response in client side (remote_client.c, socket_helper.c)
	* Result of FarReach w/o inswitch cache
		- server 8 + client 64 w/ 5s timeout: 0.091 MOPS (3.7us send; 1.8us min wait + 17us min process)
		- server 8 + client 64 w/ 100ms timeout: 0.089 MOPS (3.8us send; 1.6us min wait + 20us min process; 900/10M FPs)
		- server 8 + client 64 w/ 1ms timeout: 0.0941 MOPS (3.8us send; 1.7us min wait + 16us min process; 1500/10M FPs)
	* FUTURE: small timeout incurs false positive reqs, which could affect dynamic workload due to populating some cold keys into switch -> NOTE: deprecated now as we use a large timeout to avoid false positive (and with a large server-side udp rcvbuf size to avoid pktloss)
+ CPU overhead: use cpu_set_affinity to remve context switching overhead on server.workers
	* Use sysconf(_SC_NPROCESSORS_ONLN) to get # of cores in current physical server (common_impl.h)
	* Use pthread_setaffinity_np to set CPU affinity mask of server/controller.threads such that server.workers can occupy CPU cores individually, which reduces CPU contention overhead of server.workers
+ Test on range partition w/ setting v4
	* Setting v4: 64MB memtable size + 4 flushnum + 5 maxnum + w/ WAL flush + 100ms timeout + checkseq only for each evicted write + response filtering + CPU affinity + 10M queries
	* FarReach w/o inswitch cache
		- server 1 + client 32: 0.053 MOPS (3.7us send; 1.6us min wait + 16us min process)
		- server 8 + client 64: 0.0957 MOPS (3.8us send; 1.7us min wait + 16us min process)
		- server 16
			* client 64: 0.108 MOPS (3.8us send; 1.6us min wait + 15us min process)
			* client 64 w/ 1s: 0.1035 MOPS (2us min wait + 15us min process) (no pktloss; no false positive)
	* FarReach w/ inswitch cache
		- server 1 + client 32: 0.129 MOPS (3.6us send; 1.6us min wait + 15us min process) (2.3X runtime; 2.3X normalized)
		- server 8 (7.7X runtime thpt; 8.2X normalized thpt)
			+ client 64: 0.73 MOPS (5.5us send; 1.9us min wait + 17us min process)
			+ client 128: 0.736 MOPS (5.4us send; 1.9us min wait + 17us min process)
		- server 16 (11.3X runtime thpt; 13.9X normalized thpt)
			+ client 64: 0.66 MOPS (5.2us send; 17us min wait + 19us process)
			+ Reason: too small timeout threshold, which incur false positives, unnecessary packet process and responses, and hence undermine system throughput
				* client 128: 0.495 MOPS (4.6us send; 27us min wait + 19us min process) (no pktloss)
				* client 256: 0.89 MOPS (6.7us send; 11us min wait + 17us min process) (no pktloss)
			+ client 256 w/ 1s: 1.22 MOPS (6.5us send; 3.8us min wait + 17us min process) (no pktloss)
			+ client 512 w/ 1s: 0.66 MOPS (5.8us send; 2.7us min wait + 17us min process) (200 pktloss)
				* Reason: too large timeout threshold, which incur long wait time (not the server with min wait time) and hence undermine system throughput
			+ client 512: 0.796 MOPS (7.2us send; 14us min wiat + 17us min process) (200 pktloss, 2800 false positives)
			+ client 512 w/ 1s w/ 10*10M queries: 1.06 MOPS (2.1us min wait + 17us min process) (no pktloss, 5K false positives)
			+ client 512 w/ 5s w/ 10*10M queries: 0.915 MOPS (5us min wiat + 17us min process) (no pktloss, 400 FPs)
+ Test server throughput on range partition w/ setting v5
	* Setting v5: 64MB memtable size + 4 flushnum + 5 maxnum + w/ WAL flush + 1s timeout + checkseq only for each evicted write + response filtering + CPU affinity + 10M/100M queries
	* For tofino: pktloss = # of RX - # of TX; false positive = # of TX - # of queries
	* FarReach w/o inswitch cache
		- server 32
			+ client 256 w/ 10M queries: 0.1106 MOPS (2us min wait + 16us min process) (no pktloss; 250 FPs)
	* FarReach w/ inswitch cache
		- server 32
			+ client 256 w/ 10M queries: 0.866 MOPS (12us min wait + 20us min process) (no pktloss; no FP)
			+ client 512 w/ 10M queries: 0.7255 MOPS (7us min wait + 20us min process) (100 pktloss, no FP)
			+ client 1024 w/ 10M queries: 0.666 MOPS (5.8us min wait + 20us min process) (4K pktloss; no FP)
			+ client 1024 w/ 10*10M queries: 1.22 MOPS (13us min wait + 22us min process) (24K pktloss; no FP)
				* Long wait time due to pktloss
			+ client 1024 w/ 10*10M queries w/ 500ms timeout: 0.85 MOPS (26us wait + 21us process) (90K pktloss; 14K FPs)
				* Too many false positives
			+ client 1024 w/ 10*10M queries w/ server large udp rcvbuf size: 0.87 MOPS (28us wait + 28us process) (no pktloss; 16K FPs)
+ Test server throughput on range partition w/ setting v6 (10M queries)
	* Setting v6: 64MB memtable size + 4 flushnum + 5 maxnum + w/ WAL flush + 5s timeout + checkseq only for each evicted write + response filtering + CPU affinity + large udp receive buffer size in server
	* FarReach w/o inswitch cache
		- server 16
			+ client 512: 0.099 MOPS (1.8us wait + 16us process) (no pktloss; no FP)
			+ client 512 w/ 10*10M queries: 0.101 MOPS (1.6us wait + 16us process) (no pktloss; no FP)
			+ client 1024 w/ 100M queries: 0.113 MOPS (1.7us wait + 16us process) (no pktloss; no FP)
	* FarReach w inswitch cache
		- server 16
			+ Improvements
				* 10.91X (10M) / 7.35X (10*10M) runtime thpt; 13.9X normalized thpt
			+ client 512: 1.08 MOPS (2.7us min wait + 16us min process) (no pktloss; no FP)
			+ client 512 w/ 10*10M queries: 0.666 MOPS (4.3us min wait + 17us min process) (no pktloss; no FP)
				* FAIL: More data -> more sstables and levels -> larger overhead for memtable flush
					- NOTE: 10*10M queries do not undermine thpt of NoCache in server 16 + client 512
				* Observation: for the first 5sec, thpt = 0.7MOPS intead of 1MOPS
			+ client 1024: 1.073 MOPS (3.5us wait + 16us process) (no pktloss; no FP)
			+ client 1024 w/ 10*10M queries: 0.735 MOPS (2.7us min wait + 18us min process) (no pktloss; no FP)
			+ client 1024 w/ 2*10M queries w/ persec thpt: 1.05 MOPS (7us min wiat + 16us min process) (no pktloss; no FP)
			+ client 1024 w/ 5*10M queries w/ persec thpt: 1.205 MOPS (4.3us min wait + 16us min process) (no pktloss; no FP)
				* Observation: for each 1sec, thpt = ~1 MOPS instead of 1MOPS
			+ client 1024 w/ 10*10M queries w/ persec thpt: 0.73 MOPS (6.7us min wait + 17.5us min process) (no pktloss; no FP)
				* Observation: for each 1sec, thpt always = 0.7 MOPS instead of 1MOPS
				* Observation: for some short time interupts, all CPU cores for server.workers are not working (or fully)
			+ client 1024 w/ 100M queries: 0.55 MOPS (4us min wait + 18us min process) (no pktloss; no FP)
				* Persec throughput is stable at ~0.55 MOPS
				* Observation: for some short time interupts, all CPU cores for server.workers are not working (or fully)
			* Issue: FarReach w/ inswitch cache has worse performance under 100M queries than that under 10M queries
				- Dump client-side unmatched count and average wait time; dump server-side process latency and wait latency
				- FarReach w/ inswitch cache under server 16
					+ client 256 w/ 10M queries: 0.7 MOPS
						* client: min/max/avg send: 5/6/6us; min/max/avg wait: 2/5/4us;
						* server: min/max/avg wait: 4/47/29us; min/max/avg process: 15/52/22us
					+ client 256 w/ 10M queries: 1.04 MOPS
						* client: min/max/avg send: 5/8/7us; min/max/avg wait: 3/6/5us;
						* server: min/max/avg wait: 4/25/13us; min/max/avg process: 15/34/20us
					+ client 512 w/ 10M queries: 1.15 MOPS
						* client: min/max/avg send: 6/10/8us; min/max/avg wait: 3/9/5us; 
						* server: min/max/avg wait: 4/22/11us; min/max/avg process: 15/31/20us
					+ client 1024 w/ 10M queries: 0.78 MOPS
						* client: min/max/avg send: 6/9/7us; min/max/avg wait: 3/30/8us;
						* server: min/max/avg wait: 15/41/26us; min/max/avg process: 16/37/21us
				- FarReach w/ inswitch cache under server 8
					+ client 64 w/ 10M queries: 0.8 MOPS
						* client: min/max/avg send: 5/6/6us; min/max/avg wait: 2/3/3us;
						* server: min/max/avg wait: 2/12/8us; min/max/avg process: 14/15/15us
					+ client 64 w/ 100M queries: 0.57 MOPS
						* client: min/max/avg send: 5/6/6us; min/max/avg wait: 2/3/3us;
						* server: min/max/avg wait: 5/15/11us; min/max/avg process: 15/16/16us
						* Ovservation: persec thpt decreases from 0.75 MOPS to 0.5~0.6 MOPS
				- FarReach w/ inswitch cache under server 4
					+ client 32 w/ 10M queries: 0.46 MOPS
						* client: min/max/avg send: 4/5/4.5us; min/max/avg wait: 2/2/2us;
						* server: min/max/avg wait: 2/8/5us; min/max/avg process: 14/15/15us
						* Observation: persec thpt decreases from 0.49 MOPS to 0.43~0.46 MOPS
					+ client 64 w/ 10M queries: 0.47 MOPS
						* client: min/max/avg send: 4/5/5us; min/max/avg wait: 2/2/2us;
						* server: min/max/avg wait: 2/7/4us; min/max/avg process: 14/15/15us
						* Observation: persec thpt decreases from 0.49 MOPS to 0.43~0.46 MOPS
					+ client 32 w/ 100M queries: 0.32 MOPS
						* client: min/max/avg send: 4/5/4us; min/max/avg wait: 2/2/2us;
						* server: min/max/avg wait: 6/11/8us; min/max/avg process: 14/15/15us
						* Obvservation: persec thpt decreases from 0.43 MOPS to 0.1~0.2 MOPS
					+ client 64 w/ 100M queries: 0.32 MOPS
						* client: min/max/avg send: 4/5/5us; min/max/avg wait: 2/2/2us;
						* server: min/max/avg wait: 2.6/11/7.6us; min/max/avg process: 14/19/16us
						* Obvservation: persec thpt decreases from 0.43 MOPS to 0.1~0.2 MOPS
							- NOTE: from htop, server suffers from load imbalance and even not saturated in some short time
						* client-side persec cache hit rate is stable at 49%
						* cliend-side perserver load ratio is stable at 26:27:24:22
					+ client 64 w/ 10M of 100M queries: 0.42 MOPS
						* client: min/max/avg send: 4/5/5us; min/max/avg wait: 2/2/2us;
						* server: min/max/avg wait: 2/5/3us; min/max/avg process: 14/15/15us
						* Obvservation: persec thpt decreases from 0.43 MOPS to 0.4 MOPS
						* NOTE: decreasing from 0.49 MOPS or 0.43 MOPS is just due to different worklaods
							- Cache hit rate: 56% vs. 49%
				- FarReach w/ inswitch cache under server 1
					+ client 32 w/ 10M of 100M queries: 0.117 MOPS
						* client: min/max/avg send: 3/4/4us; min/max/avg wait: 1/1.5/1us;
						* server: min/max/avg wait: 1.6/1.6/1.6us; min/max/avg process: 14/14/14us
						* persec throughput decreases from 0.12 MOPS to 0.115 MOPS
					+ client 32 w/ 100M queries: 0.103 MOPS
						* client: min/max/avg send: 3.6/3.7/3.7us; min/max/avg wait: 1.2/1.5/1.3us;
						* server: min/max/avg wait: 1.8/1.8/1.8us; min/max/avg process: 16/16/16us
						* persec throughput changes periodically 0.12 -> 0.05 -> 0.11 -> 0.05 -> 0.11 -> ...
				+ Code change
					* Dump client-side perclient persec cache hit rate, load ratio, send and wait time
					* Dump server-side perserver persec wait and process time
					* client 64 w/ 10M of 100M queries
						* First run: 0.39 MOPS
							- The last second degrades system throughput due to stopped threads, which should be 0.41~0.42 MOPS
						* Second run: 0.35 MOPS (not restart server)
							- In some seconds, client only have ~0.25 MOPS and server has large wait time
						* Third run: 0.35 MOPS (restart server; not reset database files)
							- In the first 10 seconds, client only have 0.1~0.2 MOPS and server has larger wait time
							- NOTE: client does not have long send and wait time
							- NOTE: bottleneck server thread has slightly larger wait time, but extremely larger process time -> it increases wait time of other server threads (that's why some servers are not saturated in some short time), so the average wait time becomes larger
					* IMPORTANT: we should focus on wait and process latency of bottleneck server
						* Fourth run: 0.42 MOPS (restart server; reset database files)
							- The last second is still 0.4MOPS, so the system thot is not degraded this time
						* Fifth run: 0.35 MOPS (restart server; reset database files)
							- Bottleneck server thread has large process latency in some seconds
					* Dump server-side perserver persec rocksdb latency for PUTREQ
						- NOTE: rocksdb latency is always stable at 11~13us
						* Sixth-1 run: 0.4 MOPS (restart server; reset database files)
						* Sixth-2 run: 0.4 MOPS (restart server)
						* Sixth-3 run: 0.38 MOPS (restart server)
							- STRANGE: In one second, client does not send packets and server does not process packet
						* Sixth-4 run: 0.41 MOPS (restart server)
						* Sixth-5 run: 0.35 MOPS (not restart server)
							- Server 0 (CPU 0) becomes bottleneck w/ long process latency -> longer wait latency of other threads
							- Reason: CPU 0 contention (server.worker threads with rocksdb background threads)
					* Dump server-side perserver persec rocksdbwrapper latency for PUTREQ (including locking/deletedset overhead)
						- NOTE: rocksdbwrapper latency is still stable at 11~13us -> locking/deleted_set is not the bottleneck
						* 7th-1 run: 0.41 MOPS (restart server)
						* 7th-2 run: 0.32 MOPS (not restart server)
							- Server 0 (CPU 0) becomes bottleneck w/ long process latency -> longer wait latency of other threads
					* Code change
						* Use pthread_self + pthread_setaffinity_np to set CPU affinity mask of main thread
							- sched_affinity only affect progress and sub-progress created by fork
						* Dump full rocksdbwrapper latency for all PUTs (e.g. PUTREQ and PUTREQ_POP)
						- NOTE: rocksdbwrapper latency is still stable at 11~13us -> all PUTs have stable latency for rocksdb
						* 8th-1 run: 0.415 MOPS (restart server)
						* 8th-2 run: 0.36 MOPS (not restart server)
							- Server 0 (CPU 0) becomes bottleneck w/ long process latency -> longer wait latency of other threads
						* 9th-1 run: 0.401 MOPS (restart server; now 3 strange threads running on CPU 2)
						* 9th-2 run: 0.35 MOPS (not restart server; now 3 strange threads running on CPU 2)
							- All servers have larger wait latency and slightly larger process latency
							- Not due to nonrocksdb process latency; CPU contention can also increase wait latency?
					* Observation: rocskdb launches background threads in CPU 0-3
						- Use `taskset -pc core pid` to set cpu affinity of rocksdb threads into non-worker cores
						* 10th-1 run: 0.407 MOPS (restart server)
						* 10th-2 run: 0.407 MOPS (not restart server)
						* 10th-3 run: 0.394 MOPS (not restart server)
						* 10th-4 run: 0.288 MOPS (not restart server; use taskset to set cpu affinity of rocksdb threads into worker cores)
						* 10th-4 run: 0.395 MOPS (not restart server; use taskset to set cpu affinity of rocksdb threads into non-worker cores)
				- Final reason of performance degradation
					+ Large # of queries -> background compaction of rocksdb -> contend with server.workers due to in same CPU cores -> longer process latency
						* NOTE: normal operation of rocksdb has larger priority than background compaction/flush? -> so such CPU contention does not increase rocksdb latency but entire process latency or wait latency
					+ Large # of client threads -> large wait time in server
						* Too many client threads (e.g., 512. 1024) -> large possibility that some clients are sending packets to the server which is contended with rocksdb's background threads
				+ Code change
					* For WARMUPREQ, we only need to do cache population instead of PUT/GET
					* Invoke "reset_rocksdb_affinity.sh" to avoid CPU contention
				+ FarReac w/ inswitch cache
					* server 4 + client 64 w/ 100M queries: 0.4 MOPS -> SUCCESS!!!
					* server 4 + client 1024 w/ 100 M queries: 0.4 MOPS -> SUCCESS!!!
					+ server 16 + client 1024 w/ 100 M queries: 0.8 MOPS -> FAIL
					+ server 32 + client 1024 w/ 100 M queries: 0.8 MOPS -> FAIL
					+ Reason: too few background rocksdb threads -> some servers wait for flush/compaction and incurs larger process latency -> larger wait latency of other servers
				+ Use new setting of rocksdb
					+ 256MB memtable, 48 flushnum, 64 maxnum, 12 flush threads and parallisim to reduce flush overhead
					+ 512MB sstable, 16 level0/level1 num, and 4 compactions threads to reduce compaction overhead
					+ server 4 + client 1024 w/ 100M queries w/ new setting: 0.37 MOPS -> server 2 is bottleneck -> OK
					+ server 8 + client 1024 w/ 100M queries w/ new setting: 0.63 MOPS -> server 2 is bottleneck -> OK
					+ server 16 + client 1024 w/ 100M queries w/ new setting: 0.58 MOPS -> server 12/13 is bottleneck -> FAIL
						* NOTE: based on load ratio, server 3 should be the bottleneck!!!
						* GUESS: 12 flush threads is not sufficient for 16 server threads -> FAIL
							- w/ 16 flush threads: 0.92 MOPS (more load balance)
							- 2nd time w/ 16 flush threads: 0.64 MOPS -> server 15 is the bottleneck -> FAIL
						* Reason: popclient polls message queue which uses 100% CPU; while rocksdb background threads also run on the same CPU cores -> if memtable needs to be converted to immutable, it relies on rocksdb background thread, which degrades performance
				+ Code change
					+ Worker directly send cache population instead of using popclient to reduce CPU cpntention
					+ server 16 + client 1024 w/ 100M queries: 0.97 MOPS
					+ 2nd of server 16 + client 1024 w/ 100M queries (restart not reset): 1.02 MOPS
						* In some seconds, server cannot process any packets (degraded perf before/after these seconds)
							- Reason: wait for WAL writing/switching
						* server 12 is bottleneck
					* Read rocskdb::statistics to check whether we incur flush or compaction -> no flush/compaction
					* Check server-side persec thpt (including rocksdb latency)
						* server 12 has larger rocksdb latency and hence process latency than others
					+ 3rd of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 1G memtable: 1.14 MOPS
						* With write speed slowdown but no write stall
						* server 12 is bottleneck
					+ 4th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 1G memtable w/ skipping CPU cores 13: 1.13 MOPS
						* server 4 is bottleneck now -> OK
					+ 5th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ skipping CPU core 12: 1.13 MOPS
						* Retrieve to 256M memtable: suffer from write stall again
						* server 4 is still the bottleneck now -> OK
					+ 6th of server 16 + client 1024 w/ 100M queries (restart and reset with empty): 1.04 MOPS
						* Retrieve to 256M memtable: suffer from write stall again
						* Retrieve to use CPU cores [0-15] (not skip core 12): server 12 becomes bottleneck again
							- Reason: perf of core 12 is worse than others???
							- NOTE: no other CPU-consuming threads/processes running on CPU core 12
					+ 7th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 64MB memtable: 1.05 MOPS
						* server 12 is bottleneck; and with write stall
					+ Temporarily remove rocksdb wrapper extra overhead; and dump put/commit latency
					+ 8th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/o rocksdb wrapper extra overhead w/ put/commit latency dump: 1.05 MOPS
						* server 12 is bottleneck; and with write stall -> large commit latency especially when write stall
						* Reason: TransactionDB has write speed limitation due to commit for serializability
					+ Use normal rocksdb instead of TransactionDB
					+ 9th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ normal DB: 1.48 MOPS
						* server 12 is bottleneck; more stable yet still with write stall
					+ 10th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ normal DB w/ persec DB log: 1.6 MOPS
						* server 12 is bottleneck; more stable yet still with write speed limitation
						* NOTE: every time server 8 has a large process latency -> large wait latency of other servers
					+ 11th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ normal DB w/ persec DB log w/ killing some threads on CPU core 8: 1.66 MOPS
						* server 12 is bottleneck; no write stall and write slowdown now
					+ 12th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ normal DB w/ persec DB log w/ killing some threads on CPU core 8 w/ moving some threads out from core 12: 1.6 MOPS
						* server 12 is still bottleneck; no write stall yet with write slowdown
					+ 13th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 12th w/ 256MB memtable w/ 48 flushnum w/ 64 maxnum: 1.54 MOPS
						* server 12 is still bottleneck; with a write stall at the last time
					+ 14th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ no concurrent memtable write: 1.58 MOPS
						* server 12 is still bottleneck; with write stall and write slowdown
					+ 15th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/o rocksdb wrapper (replaced by usleep): 0.3 MOPS
						* server 4 is bottleneck (around 20% CPU utilization due to usleep)
						* No write stall or write slowdown, always stable at 0.3 MOPS -> rocksdb-wrapper-related issue
					+ 16th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/o locking and deletedset overhead w/ default write options: 1.64 MOPS
						* server 4/12 is bottlenect w write slowdown
					+ 17th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/o locking and deletedset overhead: 1.53 MOPS
						* server 12 is bottleneck w/ write stall
					+ 18th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/o locking and deletedset overhead w/ default write options: 1.56 MOPS
						* server 12 is bottleneck w/ write stall/slowdown
						* Write stall happens when the second memtable is nearly full
					+ 18th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/o locking and deletedset overhead w/ 512MB memtable: 1.63 MOPS
						* server 3/12 is bottleneck
						* NOTE: w/o write stall; w/ slight slowdown
					+ 18th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 512MB memtable: 1.47 MOPS
						* NOTE: w/o write stall; w/ obvious slowdown (no larger process latency yet larger wait latency)
							- Server thread with smallest wait latency does not have large process latency, which does not affect other server threads
					+ 18th of server 16 + client 512 w/ 100M queries (restart and reset with empty) w/ 13th w/ 512MB memtable: 1.38 MOPS
						* NOTE: w/ write stall -> insufficient client threads?
						* EXPLANATION: one request has a very large latency (e.g., 2s)
							- That's why the second before/after write stall suffers from write slowdown; and if the latency is not larger than one second, we can only see write slowdown without write stall
								+ The second before write stall sees large wait latency yet small process latency
								+ The second after write stall sees small wait latency yet large process latency
							- Fewer client threads mean larger probability that all client threads are waiting for the server
					+ 18th of server 16 + client 2048 w/ 100M queries (restart and reset with empty) w/ 13th w/ 512MB memtable: VERYSMALL MOPS
						* w/ many write stalls -> there should be a very large process latency
							- Seconds before write stall see large wait latency yet small process latency -> wait for the server
							- Seconds after write stall see large wait latency yet small process latency -> still wait for the server, and the packets are sent due to client-side timeout
					+ 18th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 256MB memtable: 1.34 MOPS
						* IMPORTANT: still w/ write stall
					+ 18th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 256MB memtable w/o rocksdb put (replaced by usleep but w/ locking and deleted set): 0.36 MOPS
						* No write stall; no write slowdown
					+ IMPORTANT: Disable linux THP!!!
					+ 18th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 256MB memtable: 1.38 MOPS
						* w/o write stall yet w/ write slowdown
					+ 18th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 512MB memtable: 1.39 MOPS
						* still w/ write stall
					+ 18th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 64MB memtable: 1.5 MOPS
						* still w/ write stall
					+ 18th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 64MB memtable w/ max_successive_merges=1000: 1.41 MOPS
						* still w/ write stall
						* [IMPORTANT] Dump perf statistics: large time on write_scheduling_flushes_compactions_time (> 1s)
							- Time for switching memtable/WAL and scheduling flushes/compactions
					+ 18th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 64MB memtable w/ disable WAL: 1.7 MOPS
						* NOTE: no write stall/slowdown
					+ NoCache: 19th of server 16 + client 1024 w/o inswitch cache w/ WAL w/ 64MB memtable: 0.169 MOPS
				- Conclusion
					+ Reason: long write latency caused by disk overhead of creating new WAL log file when switching memtable
						* For FarReach w/ or w/o inswitch cache under limited server threads (e.g., 4), disk BW cost of writing WAL and switching WAL in near time is limited -> disk is not bottleneck
						* For FarReach w/o inswitch cache under sufficient server threads (e.g., 16)
							- All server threads are writing WAL, and non-overloaded server threads may switch memtable and hence WAL in near time -> disk becomes bottleneck
							- Only one server thread is overloaded due to load imbalance, longer time for other server threads to switch WAL -> longer write stall/slowdown period
						* For FarReach w/ inswitch cache under sufficient server threads (e.g., 16)
							- All server threads are writing WAL, and they switch memtable and hence WAL at near time -> disk becomes bottleneck
								+ For 512MB memtable, although only one server thread switches memtable and WAL, others are writing WAL -> disk is still bottleneck and incur write stall/slowdown
							- Smaller time for all server threads to switch WAL due to load balance -> shorter write stall/slowdown period
					+ Solution
						* NOTE: use default wal_bytes_per_sync (aka 0) is better, which leaves OS to decide which time to flush WAL -> maximum runtime throughput
							- If we set wal_bytes_per_sync, under load balance, all server threads write WAL in near time, which incurs large write_wal_time and hence write stall/slowdown
						* NOTE: Use proper memtable size to reduce switch_memtable/WAL_time and write_wal_time
							- Although larger memtable can reduce # of switching memtable/WAL, yet incur large overhead of switching memtable/WAL -> write stall/slowdown
								+ Unless it is large enough and all data hit in memory, yet it is tricky
							- Using smaller memtable can reduce overhead of switching memtable/WAL, but increase # of switching and hence # of WAL flush -> longer write_wal_time
					+ 20th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 64MB memtable w/ 16MB wal_bytes_per_sync: 1.14 MOPS
						* still w/ write stall
					+ 20th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 64MB memtable w/ 1MB wal_bytes_per_sync: 0.93 MOPS
						* still w/ write stall
					+ 20th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 16MB memtable w/ 32 flushnum + 40 maxnum: 1.2 MOPS
						* 15s write_delay_time!!!
						* server 3 receives 688MB (43 memtable) -> achieve maxnum during flush, so rocksdb delays writes 
					+ 20th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 16MB memtable w/ 64 flushnum + 80 maxnum: 1.53 MOPS
						* w/ write slowdown yet no write stall; and it is more stable
					+ 20th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 4MB memtable w/ 128 flushnum + 256 maxnum: 1.24 MOPS
						* 1s write_wal_time!!!
					+ 20th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 8MB memtable w/ 64 flushnum + 128 maxnum: 1.51 MOPS
						* w/ write slowdown and nearly write stall (0.02MOPS)
					+ 20th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 16MB memtable w/ 32 flushnum + 64 maxnum: 1.4 MOPS
						* stable in most time but still w/ write stall/slowdown
					+ 20th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 1GB memtable w/ 2 flushnum + 3 maxnum: 1.43 MOPS
						* NOTE: w/o write stall/slowdown
					+ 20th of server 16 + client 1024 w/ 100M queries (restart and reset with empty) w/ 13th w/ 16MB memtable w/ 64 flushnum + 128 maxnum: 1.57 MOPS
						* w/ limited write slowdown
+ Fix performance degradation issue due to OS flush
	* Monitor IO by rocksdb::IOStat, iotop, and iostat
	* Configure system dirty options -> FAIL
		+ sysctl -w vm.dirty_background_ratio=50
		+ sysctl -w vm.dirty_ratio=80
		+ sysctl -w vm.dirty_writeback_centisecs=500
		+ sysctl -w vm.dirty_expire_centisecs=18000
		+ NOTE: linux is still flushing OS cache into disk
		+ GUESS: rocksdb may use sync to flush WAL into disk automatically? -> NO
	* Enable kernel sync debugging -> FAIL
		+ `sudo -i`
			* `echo 1 > /sys/kernel/debug/tracing/events/ext4/ext4_sync_file_enter/enable`
		+ `sudo cat /sys/kernel/debug/tracing/trace`
			* No one invokes sync() during transaction phase, but kernel does write data into disk even if we set a large dirty ratio and dirty expire time -> write slowdown/stall
		+ NOTE: rocksdb does not use sync to flush WAL into disk
		+ GUESS: linux write data into disk due to closing some WAL logs?
	* Tune wal_bytes_per_sync to avoid linux flushing too much data once a time -> seems to WORK
		+ Force to flush WAL in prior to avoid disk latency spike
		+ Alleviate write slowdown but cannot achieve absolutely stable throughput
+ Issue under 2048 client threads
	* Client-side segmentation fault, and client socket cannot receive response from switch/server yet tcpdump can see
		* Reason 1: we create client udp socket in each thread, which may allocate the same port to different udp client threads especially under large # of client threads -> FAIL
			* Solution: create the client-side udp sockets in main thread
		* Reason 2: we assign 8MB receive buffer for each udp socket; under 2048 client threads, it costs 16GB kernel memory which may exceed kernel limitation and hence incur segmentation fault or misbehavior
	* NOTE: client thread from 1024 to 2048 in one physical machine does not make any sense
* Final conclusion
	- Disk overhead
		+ Implementation issue of GET seq from rocksdb for each normal write, where block cache prefers skewed workload
			* Fix it by only checking seq for evicted write
		+ Too small memtable size -> many flush operations, which prefer skewed workload
			* Increase memtable size and flush threshold to reduce disk overhead
	- CPU overhead
		+ Even if with only 4/8 servers (aka 16~32 threads < 48 cores), linux still schedule them among different CPU cores, which bring context switching overhead
			* Set CPU affinitiy mask to reduce CPU overhead
		+ NOTE: If server num > CPU cores, we need to introduce more physical machines
	- Timeout issue
		+ If # of client threads is large, we will have pktloss
			* Tofino forward correct # of pkts (including normal pkts + resent pkts) to server -> not dropped by tofino
			* Tofino only receive # of normal pkts from server without # of resent pkts -> server drops some normal pkts!!!
		+ Timeout tradeoff
			+ Large timeout threshold means longer wait time and hence lower throughput
				* We solve duplicate responses due to false positives in client side
			+ Small timeout threshold means more false positives, unnecessary packet processing and responses, and hence also lower throughput
		+ Solution: run configure_client/server.sh each time to enlarge udp receive buffer in client/server
			+ As pktloss is rare, we still use a large timeout threshold by default (5s)
	- Performance degradation issue under large # of queries or client threads
		+ Reason: rocksdb background threads contend with server.workers at the same CPU cores
			+ Large # of queries -> background flush/compaction of rocksdb -> contend with server.workers due to in same CPU cores -> longer process latency
				* NOTE: normal operation of rocksdb has larger priority than background compaction/flush? -> so such CPU contention does not increase rocksdb latency but entire process latency or wait latency
			+ Large # of client threads -> large wait time in server
				* Too many client threads (e.g., 512. 1024) -> large possibility that some clients are sending packets to the server which is contended with rocksdb's background threads
		+ Solution: invoke "reset_rocksdb_affinity.sh" to avoid CPU contention
	- Performance degradation issue under large # of server threads
		+ Reason: long write latency caused by disk overhead of creating new WAL log file when switching memtable
			* For FarReach w/ or w/o inswitch cache under limited server threads (e.g., 4), disk BW cost of writing WAL and switching WAL in near time is limited -> disk is not bottleneck
			* For FarReach w/o inswitch cache under sufficient server threads (e.g., 16)
				- All server threads are writing WAL, and non-overloaded server threads may switch memtable and hence WAL in near time -> disk becomes bottleneck
				- Only one server thread is overloaded due to load imbalance, longer time for other server threads to switch WAL -> longer write stall/slowdown period
			* For FarReach w/ inswitch cache under sufficient server threads (e.g., 16)
				- All server threads are writing WAL, and they switch memtable and hence WAL at near time -> disk becomes bottleneck
					+ For 512MB memtable, although only one server thread switches memtable and WAL, others are writing WAL -> disk is still bottleneck and incur write stall/slowdown
				- Smaller time for all server threads to switch WAL due to load balance -> shorter write stall/slowdown period
		+ Solution
			* NOTE: use default wal_bytes_per_sync (aka 0) is better, which leaves OS to decide which time to flush WAL -> maximum runtime throughput
				- wal_bytes_per_sync cannot be too small (e.g., sync write), which undermines system throughput due to frequent disk operation
				- wal_bytes_per_sync cannot be too large, which incurs a long time linux flush due to huge data volumn, and incurs write stall/breakdown
				- NOTE: linux OS flushes cache into disk for every dirty seconds or every dirty bytes/ratio
			* NOTE: Use proper memtable size to reduce switch_memtable/WAL_time and write_wal_time
				- Although larger memtable can reduce # of switching memtable/WAL, yet incur large overhead of switching memtable/WAL -> write stall/slowdown
					+ Unless it is large enough and all data hit in memory, yet it is tricky
				- Using smaller memtable can reduce overhead of switching memtable/WAL, but increase # of switching and hence # of WAL flush -> longer write_wal_time
		+ Summary
			* Reason: rocksdb flushes WAL into linux OS cache periodically, and linux OS flushes OS cache into disk periodically; based on monitoring tools (iotop and iostat), when linux OS flushes OS cache, disk utilization becomes 100%; if some server thread is perform a disk operation in rocksdb (e.g., write WAL or create new WAL file), it suffers from a long latency and hence undermine system throughput
			* Alleviation: set a proper wal_bytes_per_sync to flush WAL into disk for every fixed amount of bytes -> avoid linux OS flush a large number of page cache, which utilizes 100% disk for a relatively long time and hence incur write stall/slowdown
			* NOTE: it alleviates write slowdown issue but cannot avoid it, because if server OS flushes data into disk and server thread is performing some disk operation at the same time, system throughput must be affected
	- Remaining small gap between runtime and normalized thpt improvement
		+ Reason: memory access prefers skewed workload to uniform workload
