# Implementation log of YCSB

## TODO list

- TODO
	* TODO: Add YCSB-E in exp2 on different workloads after running all experiments
		- TODO: Test ScanResponseSplit of range query
	* Client-side upstream backup (before exp9 recovery time)
		* Debug and test client-side upstream backup colleborating with periodic snapshot
			- Check whether the seq of GET/PUT/DELRES_SEQ and GETRES_LARGEVALUE_SEQ > 0
		* TODO: Re-run exp8 on w/ or w/o snapshot for FarReach -> replace 1.02 imbalance ratio
			- NOTE: Use synthetic-nosnapshot as the workloadname for exp8 w/o snapshot
		* TODO: Re-run exp9 on control plane bandwidth cost vs. snapshot interrupt for FarReach
		* TODO: Finish exp10 on recovery time vs. cache size for FarReach
	* Others
		* TODO: Fix issue of not overwriting existing statistics in single rotation mode (maybe due to using wrong value of -sr)
	* TODO: Try in-memory KVS after we have got all results of RocksDB

- 11.10
	+ Siyuan
		* Proofread related work in paper
		* TODO: Update background in paper
		* TODO: Update design in paper
		* TODO: Fix figure issue in paper
		* TODO: Read FAST'20 Twitter traces, OSDI'20 Cachelib
		* TODO: Add discussion about distributed extension in paper
		* TODO: Update exp8 bandwidth cost of evaluation in paper
		* TODO: Update exp2 workoad E and Twitter traces, and exp10 recovery time of evaluation in paper
		* TODO: Use student-T distribution to calculate the error bars of each experiment

- 11.9
	+ Siyuan
		* Read NSDI'13 Memcache and ATC'17 Memshare, which are general key-value caching for storage of web applications instead of key-value store
		* Update evaluation plan to merge exp8 and exp9
		* Update exp8 thpt of evaluation in paper
		* [IMPORTANT] Background of paper: do NOT mention write allocate and write around policy
			- Write allocate: trigger cache admission/eviction to allocate space for cache misses instead of accessing storage servers
			- Write around: NOT trigger cache admission/eviction for cache misses and only access storage servers
			- FarReach: trigger cache admission/eviction to allocate space for cache misses by controller, and access storage servers to process the cache misses
			- NOTE: FarReach is between write around and write allocate
		* Proofread related work
		* Read arxiv PKache and VLDB read-after-write consistency
	+ Huancheng
		* TODO: Finish twitter trace evaluation
			- TODO: Fix issue of zero cache hit rate
			- TODO: Fix issue of missing iterations + largevalue timeout
			- TODO: Fix issue of incorrect results under automatic script
			- TODO: Fix issue of dynamic array set for upstream backups
			- TODO: Fix issue of missing PUTREQ_LARGEVALUE in server1 of farreach
			- TODO: Fix issue of controller.evictserver of netcache
				+ TODO: Fix issue of failed 8th iteration of netcache
			- TODO: Place switch launching and configuration into internal scripts
			- TODO: Update benchmark.md to guide users to download and store Twitter traces into the specific path with required filename
		* TODO: Exp4: Change generate_dynamic_rules.py to generate <workloadname>-staticrules, such that all rule files are the same as that for the first 10 seconds (no key popularity changes)
		* TODO: Use workloadname=synthetic, dynamic_ruleprefix=static to get the results of static pattern w/ only 2 servers during 70 seconds
		* TODO: Finish exp8 on control plane bandwidth cost vs. different snapshot interrupts for FarReach
			- Check generated files
				- Check tmp_switchos.out, tmp_controller.out, and tmp_controller_bwcost.out
				- Check tmp_controller_bwcost.out: totalcost should be larger as server rotation iteration proceeds, as more in-switch records are marked as latest (vallen from 0 to 128B)
					+ Check tmp_serverrotation_part<1/2>_controller.out, which dumps # of in-switch entries with non-zero vallen
				- Check tmp_controller_bwcost.out: for each rotation, two localcosts of bottleneck partition and rotate partition should be non-zero
					+ Check tmp_switchos.out, which dumps # of speical cases during snapshot
			- TODO: Update YCSB client to disable snapshot if snapshot period = 0
			- TODO: Get bwcost and thpt for snapshot period = 0 under static and dynamic patterns
			- TODO: Do we need to add results of snapshot period = 2.5/7.5?
		* Other evaluation (after finishing the above exps)
			* TODO: Finish exp9 on recovery time
				- TODO: Fix issue of dynamic rulemap server in server 1
				- [IMPORTANT] TODO: add the following two notes to benchmark.md for exp10
					+ NOTE: you have to execute `ssh` from bf1 to all clients/servers, and from each server to all clients such that bf1/server has the host key of clients/servers
						* Otherwise, you will have an error message of `host key verification failed` for scp in farreach/localscripts/fetch*.sh
					+ NOTE: you have to use the correct ownership for /tmp/farreach in bf1 and servers
						* Otherwise, you will have an error message of `Permission denied` for scp in farreach/localscripts/fetch*.sh 
				- TODO: Write down how to calculate average recovery time into benchmark.md
				- TODO: To speed up evaluation, for each cache size, we only need to run server rotation only once -> under the given cache size, we can run test_recovery_time.sh duplicately to get multi-round recovery time
				- TODO: In warmup_client.c, we need to admit the top K hottest keys for the given cache size K
		* TODO: Start to re-run experiments for multiple rounds
			- NOTE: if thpt is affected after fixing write stalls, the previous results cannot be used as the results of the 1st round
			- TODO: Update benchmark.md to hint user to create SSH key for switch and change private key path in common.sh if necessary

- 11.8
	+ Siyuan
		* Update exp10.sh to support roundidx, and backup generated files for the given round
		* Update exp4 dynamic, exp6 skewness of evaluation in paper
		* Dump more bwcost information in controller
		* Update calculate_bwcost_helper.py to calculate average globalbwcost and per-server average localbwcost for exp9 on bandwidth cost, and then sums up avg localbwcost of all logical servers and avg globalbwcost
		* Add design details of upstream backup in paper

- 11.7
	+ Siyuan
		* Update exp1 latency, exp2 LOAD of evaluation in paper
		* Update part of exp4 dynmiac of evaluation in paper
	+ HuanCheng
		* Evaluation
			* [IMPORTANT] Fix write stalls
				+ Sleep between stop and kill in scripts to wait for flushing and compaction in loading/transaction phase -> WORK
					* FUTURE: uncomment close_server in farreach/server.c::transaction_main() to close rocksdb instances asap
					* FUTURE: uncomment sleep in farreach/server_impl.h::close_server() if RocksDB does not wait for completing flush/compaction before close
					* Re-run exp4 farreach + hotin to see if we can avoid write stalls -> still with performance fluctuation
						- [IMPORTANT] NOTE: it has effect on average thpt -> we have to re-run all experiments!
					* Re-run two numbers of exp1 latency to make a double-check -> still with large latency (due to RocksDB disk operations?)
					* Find the reason why test_server_rotation_p1.sh can sleep 15s after retrieving loading phase files, while test_server_rotation_p2.sh needs 120s and test_dynamic.sh needs 90s -> ALL sleep 120s now!
			* Add Twitter Traces for exp2 on different workloads
				- Dump twitter traces through keydump.
				- Fix a typo of libcommon to deserialize PUTREQ_LARGEVALUE_SEQ correctly
				- Fix memory overflow issue for cluster 40
				- NOTE: double-check the Twitter Traces of the choosen clusters before experiments
			* Add the results of static pattern (w/o server rotations; w/ only two storage servers) for exp4 on dynamic workloads
			* Run multiple times of each experiment (as many times as we can)
				- [IMPORTANT] keep all raw data of each time for each experiment
				- NOTE: if you use an individual script to run a given experiment multiple times, you can give an iteration number of the current running time, and backup the statistics files to the directory related with the iteration number in the script before next time of running
				- NOTE: to login Tofino as root in the script such that you can launch and configure switch automatically, you can use `ssh -i <private-key-for-switch> root@bf1 "<command>"`

- 11.6
	+ Siyuan
		* Debug and test replay-based recovery for exp10 on recovery time
			+ Pass compilation client, server, and switchos
			+ Try exp 10: in-switch and server-side recovery time vs. cache size
				* Script change
					- Dynamically set cache size in config.ini for FarReach
					- Create scripts/exps/exp10.sh, which can be invoked by external scripts for multiple rounds if necessary
						+ Backup required files in exp10.sh before test_recovery_time.sh
						+ Echo information in exp10.sh to hint users to get statistics from tmp.out
				* Run server rotation for static pattern of FarReach under a given cache size
					- NOTE: each time of running server rotation will replace the following files, we backup them into benchmark/output/exp10/cachesize-<cachesize> in exp10.sh
						+ Client-side upstream backups in dl11 and dl15: under benchmark/output/upstreambackups/, static*client<0/1>.out and dynamic-client<0/1>.out
						+ Controller-side in-switch snapshot in dl16: /tmp/farreach/controller.snapshot*
						+ Server-side maxseq files in dl16 and dl13: /tmp/farreach/*maxseq*
				* Run test_recovery_time.sh under the same cache size
					- NOTE: you can directly run test_recovery_time.sh w/ 5 times to get variance on <=5 rounds, respectively
				* Fix issues
					- Fix typo of controller.c for strid of bwcost.out
					- Add `source /root/.zshrc` into all method/tofino/*.sh to retrieve switch env for non-interative terminal
					- Use `su user -c` to switch normal user and copy recovery information to switch
					- Filter filename in outband_packet_format.c to avoid deserializing "." and ".."

- 11.5
	+ Siyuan
		* Implement server part for server-side replay-based recovery
		* Fix maxseq issue under recovery mode
		* Take a look at TPC-C benchmark especially for the following concerns
			- Official website only provides specification yet no implementation
			- Most open-sourced TPC-C benchmarks are only for OLTP relational databases, e.g., MySQL
			- Some TPC-C benchmarks can test RocksDB, as RocksDB has implemented an OLTP interface called MyRocks similar as MySQL on top the server-side storage server
				+ However, as in-switch cache cannot provide an OLTP interface, we cannot directly use MyRocksd; while it is too heavy to re-implement MyRocksdb in the client-side
		* Update implementation
	+ HuanCheng
		* Evaluation
			* Add LOAD in exp2 on different worklads
				- Add doInsert for PregeneratedWorklad
				- Still retrieve loading phase files, yet use `./bin/ycsb load` instead of `./bin/ycsb run`
		* Coding
			* Code changes
				* Fix keydump issue of memory overflow for key-frequency map
				* Fix GlobalConfig issue of not judging methodid for client_upstreambackupreleaser_port
				* Print ratio of write requests with >128B values in keydump
			* Finish TraceReplay workload
				- Get correpsonding trace file based on workloadName
				- Limit the maximum number of parsed requests, and the maximum value size based on its paper
				- Comment request filtering under static pattern in TraceReplayWorkload -> resort to KeydumpClient and PregeneratedWorkload
				- Twitter key -> 16B keybytes by md5 -> inswitchcache.core.Key by fromBytes
					+ How to implement it more reasonable? -> possible solution:
						* Change toString() to dump key as two 8B longs, which is "<8B long> <8B long>"
						* Change fromString() to extract key from "<8B long> <8B long>"
						* Add a static function Key::convertYcsbString() to convert "user<8B long>" as "<8B long> 0", before invoking fromString()
						* Add fromBytes() to parse 16B keybytes for Twitter traces
						* Re-run keydump to generate hot keys, dynamic rules, and pre-generated workloads
						* NOTE: check affected modules -> ycsb::PregeneratedWorkload, ycsb::DynamicRulemap, common/workloadparser

- 11.4
	+ Siyuan
		* Review code for large write_delay_time
			- (1) WriteOptions.low_pri is false by default -> not due to priority
			- (2) WriteController.NeedsDelay() is true which limits the rate of all writes -> maybe due to flushing/compaction?
			- Reason: stop writes due to too many level-0 files
		* Implement switchos part for server-side replay-based recovery
		* Fix issues in related work

- 11.3
	+ Siyuan
		* Fix compilations errors of upstream backup
		* Simply test correcness of upstream backup
		* Merge into main branch of NetBuffer/ and benchmark/
		* Survey write-back cache (not related with switch) and update related work
			- Search some papers first, (e.g., FAST'13, ATC'14, OSDI, and Concordia) for a double-check, and then update them into related work
	+ HuanCheng
		* Find an issue of timeout caused by write stopping due to too many level-0 SST files

- 11.2
	+ Siyuan
		* Implement controller part of upstream backup
	+ HuanCheng
		* Finish exp4 on dynamic pattern

- 11.1
	+ Siyuan
		* Fix issue exp9 on bandwidth cost; update scripts to support changes of snapshot period
		* Implement client part of upstream backup
	+ Huancheng
		* Finish latency statistics for exp1 on overall analysis
		* Update benchmark.md for each exp including code change and configuration change
			* Basic order: prepare phase (keydump + loading) -> exp1 -> exp2 -> exp3 ...
			* For exp1, give details of static server rotation as a module and latency evaluation as another module
				- For exp2, exp3, exp5, ..., refer to exp1 for the same static server rotation, but give details of code/configuration changes
			* For exp4, give details of dynamic pattern

- 10.31 (using new exp order in slides; add exp1 overall analysis for latency statistics)
	+ Siyuan
		* Update to-do list and evaluation plan slides
		* Implement switch and server parts of upstream backup
	+ Huancheng
		* Others
			- Rename benchmark/results/exp\*.md
			- Miss one rotation in farreach skewness 0.95 -> update exp5.md
		* Evaluation
			* Double-check write stall in exp4 on dynamic pattern
				- Try to disable flushing&compaction in RocksDB by modifying common/rocksdb_wrapper.h -> FAIL!
					+ Try to set GLOBAL_MAX_FLUSH_THREAD_NUM and GLOBAL_MAX_COMPACTION_THREAD_NUM as zero or 80 -> NOT work
					+ Increase MAX_MEMTABLE_IMMUTABLE_NUM and MIN_IMMUTABLE_FLUSH_NUM to 400 and 160 -> NOT work
					+ Use DEBUG_ROCKSDB to locate issue -> write_wal has large time (>0.5s)
					+ Set DISABLE_WAL as true only alleviates write stalls yet cannot avoid, not to speak of achieving the same results as no loading phase
				- NOTE: test exp4 still with loading phase -> argue that throughput variation is due to runtime variance of RocksDB, mainly use normalized thpt and cache hit rate to present dynamic results

- 10.30 (using new exp order in paper; add exp3 as latency statistics)
	+ Siyuan
		* Add imbalance ratio and update part of evaluation
		* Update imbalance ratio results of exp5.md
		* Debug and test read blocking for rare case of PUTREQ_LARGEVALUE (before exp1 Twitter traces) -> comment TMPDEBUG in server_impl.h
		* Update evaluation
		* Survey for upstream backup
			- Reference 31 in SIGCOMM (ACM Computing Survey'02 of checkpoint/log-based methods)

- 10.29
	+ Siyuan
		* Support different snapshot interrupts during server rotation
			- Provide calculate_bwcost.sh for bandwidth calculation
		* Run experiment 11 by compiling nocache, netcache, and farreach in bf3
		* Update paper including methodology and finished exps
			- Split YCSB and Twitter traces
			- Use Tucana to verity our resullts; use RocksDB for server-side persistence
			- Update evaluation, including methodology and all finished exps
	+ HuanCheng
		* Evaluation
			* Finish experiment 6 on value size 16/32/64
			* Finish experiment 7 on w/o snapshot for FarReach -> update imbalance ratio (use 1.02 as a placeholder; to be re-tested after implementing upstream backup)
		* Parallel with evaluation
			* Update normalized thpt for all exps

- 10.28
	+ Siyuan
		* Survey and think about upstream backup (snapshot for consistency and backup for durability)
			- ICDE'05 High-Availability Algorithms for Distributed Stream Processing
			- SIGCOMM'15 Rollback-Recovery for Middleboxes
	+ HuanCheng
		- Implement evaluation for static latency
			- Scripts: remote/test_server_rotation_latency.sh and remote/calculate_latency_statistics.sh for static latency
				+ NOTE: test_server_rotation_latency.sh uses per-rotation target (i.e., OPS per physical client) to start clients
				+ NOTE: calculate_latency_statistics.sh still invokes local/calculate_statistics_helper.py yet with benchmark/output/<workloadname>-statistics/<methodname>-static<scale>-latency-client<0/1>.out, which is different from the thpt statistics
			- YCSB: limit target (# of operations of all client threads per second) for static latency of corresponding method
			- YCSB: save statistics file with different name from that of static thpt
		* Evaluation
			* Check if latency is reasonable with methodology of fixing aggregate throughput
				- Try farreach + A (target: 1MOPS): avglat = 113us
				- Try nocache + A (target: 0.8MOPS): avglat = 315us
			* Fix exp5 issue of unchanged request distribution -> finish exp5 on skewness 0.9 and 0.95

- 10.27
	+ Siyuan
		* Implement read blocking for PUTREQ_LARGEVALUE pktloss / pkt-reordering
		* Scripts issues
			* Fix issue of calculate_statistics_helper.py in dynamic pattern
				- Add persec normalized thpt and agg thpt
			* Fix totalthpt_list and target in calculate_target_helper.py
	+ Huancheng
		- Implement part of static latency evaluation
		* Evaluation
			* Finish experiment 2 on 32/64/128 servers
			* Finish experiment 4 on 0.25/0.75/1.0 write ratio
			* Finish experiment 5 on skewness 0.9 and 0.95

- 10.26
	+ Siyuan
		- Update JNI for range query
			- Invoke _udprecvlarge_multisrc_ipfrag and _udprecvlarge_multisrc_ipfrag_dist of libcommon in JNI-based socket
				+ If under server rotation, directly return after receiving all SCANRES_SPLITs of one src (one server / one server + one switch) (change libcommon)
				+ Pass one Java dyanmic array as a parameter to store the encoded result
					* In JNI, encode all C dynamic arrays as one dynamic array, copy it to the Java dynamic array
					* In JAVA, decode the single Java dynamic array into multiple dynamic arrays
			- Update DbUdpNative to invoke the native function to receive SCANRES_SPLIT
				+ Invoke DbUdpNative::scanNative in farreach/nocache/netcache-client
		- Provide part of remote/test_server_rotation_latency.sh and local/calculate_target_helper.py
			- NOTE: calculate_target_helper.py uses thpt results of server rotation to calculate the expected target for the current method for the fixed 1MOPS aggregate thpt
		- Think about exponential moving average of CM and access frequency
			- For CM, we cannot use exponential moving average
				+ As data plane cannot support moving average calculation, we must perform it in control plane
				+ However, as CM is accessed by data plane to trigger cache admission, which cannot wait for the long delay of control plane within each period to update the history CM
				+ We also do not have sufficient stateful ALUs to maintain another CM sketch
			- For per-cached-record access frequency, we have the opportunity to use exponential moving average
				+ As access frequency is used by the controller to make cache eviction decision instead of the data plane, we can allow the control plane to update the history access frequency periodically
				+ Two concers: (1) correctly compile into Tofino due to limited hardware resources; (2) periodically load-calculate-update within limited time in control plane
	+ Huancheng
		* Evaluation
			* Finish experiment 1
			* Run most experiment 2
			* Finish experiment 3

- 10.25
	+ Siyuan
		- Dump more latency info and update latency calculation script to consider server rotation simulation effect
		- [IMPORTANT] latency statistics are NOT stable under server rotation
			+ -> Use normalized thpt for multiple storage servers under server rotation to evaluate load balance ratio
			+ -> Present latency statistics under 2 storage servers under dynamic patterns
	+ HuanCheng
		* Evaluation
			* Run most experiment 1

- 10.24
	+ Siyuan
		* For consistent loading phase
			- Update inswitchcache-java-lib and benchmark.md
			- Update prepare_load.sh and load_and_backup.sh
		* Calculate normalized thpt in calculate_statistics_helper.py
		* Prepare for quiz (Cypher)
		* Support range query
			- Get frag_hdrsize, srcnum_off, and other scan-related metadata based on packet type and methodid dynamically instead of from function parameters in common module
				+ Update the usage of udprecvlarge_multisrc\[_dist\] for scan response in C code
			- Add SCANRES_SPLIT (use custom Pair)
	+ HuanCheng
		* Update scripts: test_server_rotation_p1.sh, test_server_rotation_p2.sh, test_dynamic.sh
		* Evaluation
			* Re-run experiment 1 for reasonable medium latency -> FAIL due to unstable latency under server rotation

- 10.23 (Sunday)
	+ Siyuan
		* Use new scripts to update latency statistics of exp 1 and exp 2
		* Update implementation and exp1 of evaluation in paper
	+ Huancheng
		* Evaluation
			* Get expected thpt results of exp1 and exp2
		* Others
			* Prepare for assignment 2

- 10.22 (Saturday)
	+ Siyuan
		* Fix a bug in TommyDS usage
		* Add workload mode judgement in common_impl.h
		* Test in-memory KVS under PKU's testbed
			+ TODO: Wait for XMU's testbed to install 8.9.1 compiler
				+ Test correctness of farreach P4 under 1M records
					* TODO: Try warmup phase to check if the hot keys are cached
					* TODO: Send some requests to see if cache hit rate is reasonable and all responses of the requests can be received
			+ Test dynamic workload performnace of FarReach under 100M records (NOTE: re-run keydump if not)
				* NOTE: as our cache hit latency 30~40 us is lower than NetCache 5 us due to testbed difference, we need more client threads to saturate the system
				* NOTE: NetCache may implement a concurrent KVS based on TommyDS
				* Disk bottleneck? (still 2 logical servers)
					- Thpt under 128*2 logical clients: 0.5 MQPS (avg latency is ~200 us)
					- Thpt under 1024*2 logical clients: 0.5 MQPS
				* Network bottleneck? (with 32 logical servers)
					- Thpt under 1024*2 logical clients: 1.5 MQPS (NOT saturate server)
					- Thpt under 4096*2 logical clients: 1.5 MQPS (YCSB cannot launch too many subthreads -> still NOT saturate server)
				* Therefore, the reason of the difference (around 100X) between our absolute result and NetCache is disk based on NetCache paper
					- Single-server RocksDB w/o cache: <0.1 MQPS; single-server TommyDS (maybe modified as concurrent) w/o cache: <10 MQPS
		* Test dynamic pattern of RocksDB under different # of clients
			+ 128*2 logical clients: still 0.24 MOPS and server has been saturated -> using 64*2 logical clients is ok
		* Update statistics calculation scripts to remove runtime variance
		* Update part of exp1 in evaluation
	+ HuanCheng
		* Evaluation
			* Re-run all experiment 1 after fixing runtime variance
				- NOTE: run farreach/netcache/nocache on workload C first
				- Move original benchmark/results/exp1.md as benchmark/results/exp1_old.md; create a new exp1.md
				- Use new scripts to update thpt/latency numbers in benchmark/results/exp1.md, and also keep necessary data files in benchmark/results/exp1/
					+ NOTE: we need to keep Json files but not track them in git as they are too large
			* Run farreach/netcache for experiment 2 with 128/32/64 servers

- 10.21 (Friday)
	+ Siyuan
		* Update statistics module to fix runtime variance under server rotation especially for large scale
		* Update methodology of evaluation in paper
	+ HuanCheng
		* Evaluation
			* Enable server-side snapshot successfully
				* For farreach, test preparefinish_client at withinBenchmark() to see if java can trigger snapshot successfully
					- Check tmp_controller.out. tmp_switchos.out, and tmp_controller_bwcost.out

- 10.20 (Thursday)
	+ Siyuan
		+ Fix a bug in withinBenchmark: we only ./preparefinish_client in client 0 for farreach
		+ Test correctness of farreach P4 under 1M records
			* Port 3/0 is always DWN (other five ports are UP) -> FIX
			* Server's NIC RX errors due to compiler 9.2.0 bug
				- NOTE: NOT due to ipv4 checksum which is correct under csum16
					+ Observation: the standard P4 program chksum works with csum16
				- Reason: ethernet.srcAddr loses one byte due to Tofino hardware bug
					+ (NOT work) Solution: add pa_no_overlay for ethernet.dstAddr/srcAddr/etherType
				- Reason: miss output width for ipv4_hdr.checksum under USE_BFSDE920
					+ (NOT work) Solution: move `width: 16` out of the #endif of USE_BFSDE920
				- (NOT work) Trial: remove ipv4 checksum calculation from p4 code
				- (NOT work) Trial: remove update_ipmac_srcport_tbl from ptf -> from p4 code
					+ Observation: miss first byte of srcAddr before removing update_ipmac_srcport_tbl; miss first byte of dstAddr after that
				- Debub by commenting P4 code
					+ Remove all except l2l3_forward_tbl -> OK
					+ Retrieve all ingress MATs -> OK
					+ Retrieve all egress MATs except values -> lost byte!
						* Retrieve all egress MATs except values and MATs in stage 11 -> lost byte!
						* Retrieve all egress MATs except values and MATs in stage 10 & 11 -> lost byte!
						* Retrieve all egress MATs except values and MATs in stage 9 & 10 & 11 -> lost byte!
						* Retrieve all egress MATs except values and MATs in stage 8 & 9 & 10 & 11 -> OK!
						* Retrieve all egress MATs except values and MATs in stage 8 & 10 & 11 -> lost byte!
					+ Retrieve all egress MATs yet remove MATs of four ingress stages -> lost byte!
				- [IMPORTANT] compiler 9.2.0 cannot correctly compile such a complex P4 program of our current implementation into Tofino, unless we remove sufficient MATs; however we have optimized our P4 code carefully for compiler 8.9.1
	+ HuanCheng
		* Evaluation
			* Run nocache/farreach/netcache for experiment 2 with 32 servers
				- Update benchmark/results
			* Re-run some numbers of farreach and netcache

- 10.19
	+ Siyuan
		+ Fix a script issue of killing clients for server rotation
		+ Configure XMU's testbed to make them communicatable in a single LAN
		+ Deploy FarReach into XMU's testbed
			* Pass compilation of client, switchos, and server
				- NOTE: mvn 3.6.0 only support openjdk-8/11 instead of openjdk-17
			* Pass compilation of P4 under bf_sde_9.2.0 by moving cache_lookup_tbl from stage 2 ingress to stage 3 ingress to save resources for access_latest_tbl in stage 2 egress
			* Update hardware configuration
				- Provide scripts/local/configure\<client/server/switchos\> to reduce redundant scripts
				- Update macaddr in farreach/configs/config.ini.inmemory accordingly
				- Update scripts-inmemory/local/configure\<client/server/switchos\> accordingly
			* Update scripts-inmemory/local/changeto_inmemory_testbed.sh
	+ HuanCheng
		* Make evaluation of experiment 1 on netcache after fixing client killing issues

- 10.18
	+ Siyuan
		* Prepare for in-memory KVS in TangLu's testbed
			- Define USE_TOMMYDS_KVS in helper.h (commented by default)
			- If USE_TOMMYDS_KVS, replace rocksdb with in-memory KVS
			- Store vallen instead of valdata for TommyDS with limited running time to save memory
			- Prepare configs/config.ini.inmemory for farreach
			- Provide scripts/local/change_testbed_inmemory.sh to replace config.ini as configs/config.ini.inmemory for farreach, and update common/helper.h, farreach/Makefile, and scripts/remote/sync.sh accordingly
		* Get XMU machines topology information and login successfully
	+ HuanCheng
		* Evaluation
			* Make evaluation of experiment 1 on nocache
			* Encounter a failure of killing client1 under timeouts

- 10.17
	+ Siyuan
		* Fix timeout issue for correct execution time
			- Dump static statistics in TerminatorThread instead of Client
			- Add a boolean isStop in InswitchCacheClient: set it as true in TerminalThread and judge it in DbUdpNative
			- Sleep 1s before stopping client1 to wait for client1.TerminatorThread to dump static statistics in test_server_rotation\*.sh
		* Update scripts
			- Add 90P and 95P latency result
			- Support singlerotation mode in YCSB client
				+ Add -sr <1/0> in command line parameters and save it into GlobalConfig
				+ If GlobalConfig.singlerotation = true
					* NOTE: the statistics file must exist; otherwise, you should re-run the entire server rotation
					* For the first rotation, StatisticsHelper does NOT delete the statistics file; instead, we load exsting statistics
					* For the first and each subsequent rotation
						- If the statistics of the rotation exists in statistics file, overwrite it
						- If the statistics of the rotation does not exist in statistics file, insert it into the correct position
				+ Add -sr when launching clients in test_server_rotation_p1/p2.sh
			- In calculate_statistics_helper.py, judge whether strid is the same in aggregate()
		* Adapt to YCSB range query
			- Update client code to set endkey = startkey + scan recordcnt (from 1 to 100 in YCSB core workload E) (files: Key.java)
			- Update server code to calculate scan recordcnt based on endkey - startkey (files: rocksdb_wrapper.c, deleted_set_impl.h)
		* Tiny changes in Java
			- For InswitchCacheClient, always enable withinBenchmark() for FarReach/DistFarReach
			- Test withinBenchmark() to see whether Java can invoke shell command successfully
		* Others in C
			- Add bandwidth usage of reporting original values during snapshot (files: switchos.c, controller.c)
				+ NOTE: bandwidth cost of switch os (i.e., local control plane) also belongs to control plane bandwidth usage
				+ In controller, bandwidth cost refers to total control plane bandwidth usage including the local control plane bandwidth cost (i.e., bandwidth cost for special cases)
		* Update benchmark.md for synthetic workload path issue (make an IMPORTANT NOTE)
	+ Huancheng
		+ Evaluation
			* Make evaluation of experiment 1 on nocache

- 10.16
	+ Siyuan
		* Fix cached keyset resume issue of netcache/distcache under static pattern
		* Use keydump_and_sync.sh for workload analysis and pregeneration
		* Use test_dynamic.sh for dynamic pattern
		* Launch servers and clients in test_server_rotation.sh, test_dynamic.sh, and load_and_backup.sh
		* Dump more information for server rotation
			- Dump strid into each JSONObject of JSONArray in StatisticsHelper
			- Calculate cache hit rate and normalized throughput in calculate_statistics_helper.py
		* Test scripts
			- Test sync.sh and keydump_and_sync.sh to check whether they will overwrite benchmark/output/*-statistics
	+ Huancheng
		* Make evaluation of experiment 1 on farreaceh

- 10.15 ([IMPORTANT] start evaluation)
	+ Siyuan
		* Add new scripts (NOTE: always invoke scripts under root directory)
			- For loading phase
				+ Remove recordload/ -> directly use nocache/config.ini for both YCSB client, switch, and server to avoid consistency issue
				+ Use load_and_backup.sh to run recordload, backup the loaded database files, and sync required database files between two clients if the loading phase is for static server rotation
					+ NOTE: place backuped files into /tmp/backupedrocksdb instead of /tmp
					+ NOTE: Change permission to all users for rocksdb files after loading
				+ Parse config.ini in shell and update load_and_backup.sh to avoid parameter consistency issue
				+ Test common.sh, sync.sh, makeremotefiles.sh, stopservertestbed.sh, and load_and_backup.sh
			- For workload analysis
				- After running keydump, use synckeydump.sh to copy pregenereated workload to secondary client
			- Reduce unnecessary remotescripts and localscripts in each method/
			- For server rotation
				- Use prepare_server_rotation.sh to generate config file before launching testbed
				- Remove the line number in test_server_rotation.sh
				- Retrieve config.ini after test/stop_server_rotation.sh
				- For each method, provide a template config and use a script to generate the config files for the given workloadname and server rotation parameters
			- Update benchmark.md
		* Test scripts for server rotation

- 10.14
	+ Siyuan
		* Update static workload script for YCSB client
			- Use localstop/kill.sh and remove stop/kill_server/client/controller.sh -> update all related scripts
			- Add /home/${USER} before CLIENT/SWITCH/SERVER_ROOTPATH in common.sh -> update all other scripts to remove ~/ and /home/${USER} before XXX_ROOTPATH
			- Add make allclient/allserver/allswitch to all methods
		* Others in java
			- Use dynamic test time + 5s in Client.java instead of 75
			- Fix dbNative nullpointer issue and invalid methodid issue of recordload, and multiple insert values issue of CoreWorkload
			- Fix val toString issue with valdata = null
			- FIx serializeOphdr issue in Packet.java (Key.java returns false size in dynamicSerialize)
			- NOTE: LOAD triggers force_put instead of put
		* Check whether nocache saves seq=0 for LOADREQ and read seq=0 for READREQ with 128B value (val size = 128 and valstr size = 132)
			- NOTE: 50M per physical client may change key distribution, while 100M per physical client loads 200M records -> we limit the number of physical index as 1 for recordload as in keydump
		* Others in C
			- Add control plane bandwidth usage calculation (files: controller.c in farreach/distfarreach)
		* Use loading phase to pre-load 100M records into 16 storage servers (NOTE: recordload client + nocache switch/server)
			- Backuped path: /home/rocksdbbackups/16/ in both dl16 and dl13
		* Test static workload script w/ 16 servers
			- Changes
				+ Use variables defined in scripts/common.sh
				+ Use backuped files to retrieve consistent state of server-side rocksdb
				+ Overwrite all related fields in corresponding config templates
				+ Launch YCSB client instead of remote_client
			- Fix path issue, max server num issue in script
			- Fix wrong throughput statistics caused by PregeneratedWorkload
				+ Alive client threads may NOT send packets due to doTransaction returns false
				+ initThread() is counted into execution time
			- Update test_server_rotation.sh and stop_server_rotation.sh -> SYNC to all methods after testing correctness
	+ Huancheng
		* Implement YCSB trace replay for Twitter trace
			- Implement TraceReplayWorkload for Twitter traces


- 10.11 - 10.13
	+ Siyuan
		* Dump timeout information
		* Prepare for static workload
			- Update benchmark.md (not need -pi for keydump and workloadpregenerate)
			- Pre-generate per-client workloads for server rotation by Keydump
			- Load pre-generated workload by PregeneratedWorkload, keep requests for running servers under server rotation, and assign requests to different logical clients
				+ NOTE: keydump and recordload never uses PregeneratedWorkload (for transaction phase), which only uses CoreWorkload or TwitterWorkload for preparation and loading phase
				+ NOTE: each method uses CoreWorkload/TwitterWorkload if workload mode = 1, or PregeneratedWorkload otherwise during transaction phase
			- Test Keydump and PregeneratedWorkload
				+ Directly write InmemoryReq to file in Keydump instead of maintaining as a list in InswitchCacheClient (NOTE: NO reqnum at the beginning of the file now)
				+ Test time of different operation counts (1M: 1s, 10M: 8s, and 100M: <2min)
				+ Optimize PregeneratedWorkload by mmap I/O (MappedByteBuffer) and concurrent loading (initThread)
		* Implement CustomHistogramMeasurement to collect us-level avg/medium/99P latency
			+ NOTE: as we need YCSB's statistics including ClientThreads (local clients) for throughput and Measurements (global singleton) for latency, we cannot use it in inswitchcache-java-lib
			+ For static workload, collect and flush throughput and latency statistics after postBenchmark in Client.java
			+ For dynamic workload, collect per-second throughput and latency statistics in computeStats of StatusThread.java
				* Flush per-second statistics at the end of StatusThread
		* Implement StatisticsHelper
			- Support to collect and dump latency histogram, system thpt, and per-server thpt
			- For dynamic workload, given the file per physical client, rm existing file and dump per-second statistics
				+ NOTE: each JSONObject corresponds to the statistics of each second
			- For static workload, given the file per physical client, rm existing file for the first rotation; merge and dump statistics for each subsequent rotation
				+ NOTE: each JSONObject corresponds to the statistics of each rotation
		* Use scripts to aggregate per-physical-client statistics
			- NOTE: dynamic workload statistics may not have the same number of JSONObjects due to execution time variance
			- NOTE: sum per-physical-client thpt; sum per-physical-client latency histogram to calculate avg/medium/99P latency
	+ Huancheng
		* Reproduce preliminary results of farreach on 4 cases
			- Without cache (1 case): client w/ workload mode = 1 + server w/ workload mode = 0 + NO warmup phase
			- With cache (3 cases): client and server w/ workload mode = 1 + warmup phase on hotin/hotout/random dynamic workloads
		* Test farreach/nocache/netcache under hotin/hotout/random pattern with 128B-value write-only 0.99-skewed workload (use 100B value to get preliminary results) -> get reasonable results!
		* Test loading phase by LOADREQ + GETREQ
		* Run farreach with hotin write-only workload to get aggregated statistics, and check whether it is consistent with our preliminary result -> consistent after fixing python script issues

- 10.10
	+ Siyuan
		* Code review
			- Review code related with ByteBuffer -> update equals() and hashCode() in inswitchcache.core.Key
		* Overwrite workload based on config (map workload name into workload property), cancel required param for workload property
			- Remove -P and -df from command-line parameters
		* Prepare for static workload
			- Find bottleneck server for 16/32/64/128 scale in KeydumpClient
			- Update calculation of cache hit rate and normalized throughput

- 10.9
	+ Siyuan
		* Support large value
			- Add GETRES_LARGEVALUE and PUTREQ_LARGEVALUE w/ dynamic_serialize 
			- Use udprecvlarge_ipfrag for GETRES_LARGEVALUE in read
			- Use udpsendlarge_ipfrag for PUTREQ_LARGEVALUE in update/insert
		* Encapsulate an individual class for GET/PUT/DEL/SCAN (general for each method) (NOT need InetAddress of ip and svraddr of udprecvfrom)
		* Implement a RecordloadClient for loading phase
			- Use udpsendlarge_ipfrag for LOADREQ
			- NOTE: YCSB uses recordcnt for loading phase, where each request (MUST be INSERT) has a different key
		* Implement NoCacheClient and NetCacheClient
			- Add preparefinish_client in prebenchmark of farreach to trigger snapshot
	+ HuanCheng
		* Fix data race of the shared map for dynamic rulemap statistics
		* Fix the bug of Key comparison in dynamic rulemap

- 10.8 - 10.9
	+ Siyuan
		* Update jnisrc to use libcommon (add methodid into inswitchcache-java-lib::SocketHelper)
		* Fix path errors of Tofino scripts
			- Use CLIENT/SWITCH/SERVER_ROOTPATH in scripts
		* Update inswitchcache.core.PacketFormat as libcommon (introduce methodid yet NOT affect UDP packet content)
			- Add LOADREQ and LOADACK; add dynamic_serialize in Val

- 10.7
	+ Siyuan
		* Check correctness of JNI on UDP checksum for cache hits/misses
		* Re-organize ycsb/ as benchmark/, including ycsb/, inswitchcache-java-lib/, inswitchcache-c-lib/, output/, results/
			- Use inswitchcache.core.Key/Value/SocketHelper in farreach
			- Disaggregate JAVA-based lib for YCSB
		* Update scripts and benchmark.md (make rocksdb/common and then make each method in scripts/local/make\*.sh)
	+ Huancheng
		* Fix inconsistent hash_partition_idx issue of key
		* Double-check packet format of GET/PUT/DEL/request and response

- 10.5 - 10.6
	+ Siyuan: Disaggregate C-based lib for remote_client.c and server for ALL methods
		* [IMPORTANT] NOTE: the first commit for libcommon is b18b0a3e10f0b3fe76e7006fdfc1d0f38925507d with message of "add common module for benchmarks and servers"
		* Place the same modules into common/ (including crc32, key, value, dynamic_array, dynamic_rulemap, helper, latency_helper, pkt_ring_buffer, snapshot_record, special_case, workloadparser/\*, iniparser/\*)
			- Merge ALL packet_format\.* as a single file with methodid as a parameter, and place it into common/
			- Introduce methodid into socket_helper, rocksdb_wrapper, io_helper
				+ NOTE: pkt_ring_buffer and dynamic_rulemap ONLY need packet_type_t -> NOT introduce methodid as parameter
		* Compile common/ as libcommon.a for benchmark/ and method/
			- Update usage of packet_format, socket_helper, rocksdb_wrapper, io_helper (files: loader.c, localtest.c, remote_client.c, server_impl.h, switchos.c, controller.c, recover/\*) -> SYNC to ALL
			- Update Makefile and includes by running replace.sh -> SYNC to ALL

## History log

- 10.3 - 10.4
	+ Siyuan
		+ Code for dynamic workload
			* Update helper.h, warmup_client.c, and ycsb_parser.c for new filepath and new file format -> SYNC to ALL
			* Add large value in JNI -> update header file after fixing java compilation errors
			* Implement rulemap generation for hot-in, random, and hot-out workload
		+ Fix tiny issues
			* Remove -cf and -threads from command-line parameters
			* Place all command-line parameters related with InswitchCache into GlobalConfig -> only use GlobalConfig in subthreads
		+ Set seed = threadid in random generator for each logical client in YCSB -> test keydump results
		+ Try individual jar
	+ Huancheng
		* Re-implement sendpkt client/server and rulemap client/server
		* Implement rulemapclient in TerminatorThread to notify rulemap switching during each period

- 9.27
	+ Decouple inswitch-cache-related lib as an individual module in YCSB

- 9.24
	+ Re-organize YCSB client code

- 9.23
	+ Implement DyanmicRulemap for dynamic workload

- 9.22
	+ Implement socket helper based on JNI
		* Remove redundancy of RemoteDB; use short for UDP port; 
		* Move Key/Value from farreach into core module
		* Calculate cache hit rate of keydump file

- 9.21
	+ Implement Keydump and rename netbuffer as farreach
		* Extract RemoteDB static members as InswitchCacheClient
		* Process per-keydump-client keyfrequencymap in main client, and dump hotest keys
		* Convert keystring into 16B key instead of changing workload file
	+ Implement dynamic array and part of socket helper

## FUTURE

* FUTURE: SYNC read blocking for PUTREQ_LARGEVALUE pktloss to DistFarreach
	+ Switch: largevalueseq_reg and is_largevalueblock
	+ Server: per-server blockinfomap and mutex for read blocking
* FUTURE: SYNC client-side upstream backup and server-side replay-based recovery for durability to DistFarreach

## DEPRECATED

* DEPRECATED: Reduce redundant switch-related scripts in method/localscripts/
* DEPRECATED: Implement DistfarreachClient, DistnocacheClient, and DistcacheClient (send pkt for power-of-two-choices for sampled GETRES) in YCSB (just with different methodids)
	- [IMPORTANT] current distributed extension is a single discussion instead of a critical design -> NOT need to evaluate
* DEPRECATED: Encapsulate GET/PUT/DEL/SCAN in inswitchcache-c-lib/ for remote_client.c
	- [IMPORTANT] NOT need to provide c-lib for db_bench

## Important NOTEs

- [IMPORTANT] explanation of our results compared with NetCache paper -> reason: difference KVS
	- Per FarReach KVS: 0.075 MOPS
		+ Dynamic workload: 0.075*2*1.6=0.25MOPS
		+ Static worklaod: 0.075*128*1.6=15.36MOPS
			* NOTE: expected thpt of nocache is 1.5MOPS
	- Per NetCache KVS: 10 MOPS
		+ Dynamic workload: 10*2*1.6=32MOPS
			* NOTE: NOT sure why NetCache has 40MOPS, maybe due to imprecise evaluation
			* Based on its own arguments, the dynamic thpt should be 1/64 of static = 2*1000/64=31.3MOPS
		+ Static workload: 10*128*1.6=2.04GOPS
- [IMPORTANT] for synthetic workload, use different workloadname for write ratio, skewness, value size, and w/o snapshot
	- Different workloadname can avoid overwriting workload files generated by keydump and statistics files generated during server rotation
- [IMPORTANT] to get reasonable latency statistics under server rotation, we should fix the same aggregate throughput for all methods for fair comparions
	- Give an individual experiment to reflect the non-trivial trade-off between thpt and latency
	- In each other experiment, mainly focus on throughput statistics

## Important design changes

* Introduce read blocking for PUTREQ_LARGEVALUE pktloss / pkt-reordering
	- Introduce ENABLE_LARGEVALUEBLOCK (files: farreach/tofino/main.p4 and farreach/tofino/common.py)
	- Introduce largevalueseq_reg (files: farreach/tofino/p4src/regs/largevalueseq.p4, farreach/tofino/main.p4, farreach/tofino/p4src/header.p4, farreach/tofino/p4src/egress_mat.p4, farreach/tofino/configure/table_configure.py)
		- For largevalueseq_reg
			- If cached=1 and valid=1, GETREQ_INSWITCH reads largevalueseq
			- If cached=1 and valid=1, PUTREQ/DELREQ_INSWITCH and GETRES_LATEST/DELETED_SEQ_INSWITCH sets latest=1 and largevalueseq=0 and is_largevalueblock=0
			- CACHE_POP_INSWITCH sets latest=0 and largevalueseq=0 and is_largevalueblock=0
			- If cached=1 and valid=1, PUTREQ_LARGEVALUE_INSWITCH sets latest=0 and largevalueseq=assignedseq and is_largevalueblock=0
		- For is_largevalueblock_tbl (keyless), set meta.is_largevalueblock = 1 only if meta.largevalueseq != 0
		- For another_eg_port_forward_tbl
			+ If cached=1 and valid=1 and latest=0 and largevalueblock=1, convert GETREQ_INSWITCH into GETREQ_LARGEVALUEBLOCK_SEQ, set seq_hdr as meta.largevalueseq, and keep shadowtype_hdr and add seq_hdr
		- For other egress processing
			+ Update pktlen as DELREQ_SEQ and ipmacport as client2server for GETREQ_LARGEVALUEBLOCK_SEQ (including shadowtype and seq)
	- Pass P4 compilation, and pass configure.sh
		+ Move meta.is_largevalueblock field and GETREQ-related actions of eg_port_forward_tbl to another_eg_port_forward_tbl (files: farreach/p4src/egress_mat.p4, farreach/configure/table_configure.py)
	- Maintain per-server largevalue blockinfo list
		- For PUTREQ_LARGEVALUE_SEQ
			+ If the key does not exist in largevalue blockinfo list, add the key with isblocked = false
			+ For the blockinfo, mark isblocked = false, update seq, and clear read blockings list if any
		- For GETREQ_LARGEVALUEBLOCK_SEQ
			+ If the key does not exist in largevalue blockinfo list, add the key, mark isblocked = true, and block
			+ If the key exists and req.seq = blockinfo.seq and isblocked = true, block
			+ If the key exists and req.seq > blockinfo.seq, mark blocked = true and block
			+ Otherwise, process as usual without blocking
		- XXX_BEINGEVICTED and CAHCE_EVICT can also clear the block list of PUTREQ_LARGEVALUE
			+ Maintain per-server mutex for blockinfomap for the rare case of PUTREQ_LARGEVALUE
	- Introduce GETREQ_LARGEVALUEBLOCK_SEQ (files: farreach/tofino/main.p4, farreach/tofino/common.py, common/packet_format.h, farreeach/common_impl.h, common/packet_format_impl.h)

* Implement client-side record preservations
	- Add seq into GET/PUT/DELRES for FarReach such that snapshot can release part of client-side backup
		+ [IMPORTANT] Save assignedseq for PUT/DELREQ_INSWITCH before being overwritten by access_savedseq_tbl if cached=1 and valid=1
			* NOTE: we must save seq_hdr.seq into clone_hdr.assignedseq_for_farreach between seq_reg and savedseq_reg, yet cannot perform it in other MATs of stage 2 due to the confliction on seq_hdr.seq
			* Change access_largevalueseq_tbl into access_largevalueseq_and_save_assignedseq_tbl (files: farreach/tofino/main.p4, farreach/tofino/p4src/header.p4, farreach/tofino/p4src/regs/largevalueseq.p4, farreach/tofino/configure/table_configure.py, common/packet_format_impl.h)
				- Save seq_hdr.seq into clone_hdr.assignedseq_for_farreach when reset_largevalueseq for PUT/DELREQ_INSWITCH if cached=1 and valid=1
				- Change clone_hdr_bytes from 4 as 8 for FarReach in libcommon
			* Set seq_hdr.seq=clone_hdr.assignedseq_for_farreach when converting PUT/DELREQ_INSWITCH and PUT/DELREQ_SEQ_INSWITCH_CASE1 into PUT/DELRES_SEQ (files: farreach/tofino/p4src/egress_mat.p4)
		+ Add GETRES_SEQ, GETRES_LARGEVALUE_SEQ, PUTRES_SEQ, DELRES_SEQ including the following files
			* Server: common/packet_format\*, farreach/common_impl.h, farreach/server_impl.h
			* Client: new files in inswitchcache-java-lib/core/packets/, inswitchcache-java-lib/core/db/UdpNative.java
			* Switch: farreach/tofino/common.py, farreach/tofino/p4src/egress_mat.p4, farreach/tofino/configure/table_configure.py
				- GETRES_SEQ, GETREQ_INSWITCH, GETRES_LATEST/DELETED_SEQ -> GETRES_SEQ (including ipv4_forward_tbl, access_savedseq_tbl, another_eg_port_forward_tbl, update_pktlen_tbl, update_ipmac_srcport_tbl, add_and_remove_value_header_hdr)
					+ NOTE: GETREQ_INSWITCH needs to read savedseq
				- GETRES_LARGEVALUE_SEQ -> GETRES_LARGEVALUE_SEQ (including ipv4_forward_tbl, update_pktlen_tbl, update_ipmac_srcport_tbl)
				- PUTRES_SEQ, PUTREQ_INSWITCH, PUTREQ_SEQ_INSWITCH_CASE1 -> PUTRES_SEQ (including ipv4_forward_tbl, eg_port_forward_tbl, update_pktlen_tbl, update_ipmac_srcport_tbl)
					+ NOTE: PUTREQ_INSWITCH needs to update clone_hdr.assignedseq_for_farreach
				- DELRES_SEQ, DELREQ_INSWITCH, DELREQ_SEQ_INSWITCH_CASE1 -> GETRES_SEQ (including ipv4_forward_tbl, eg_port_forward_tbl, update_pktlen_tbl, update_ipmac_srcport_tbl)
					+ NOTE: DELREQ_INSWITCH needs to update clone_hdr.assignedseq_for_farreach
		+ Fix server-side compilation errors
	- Maintain client-side record preservations in a concurrent map for each logical client
		+ Update the record preservation of (key, value, seq) into the map for each GET/PUT/DELRES of a cache hit
	- Each client releases record preservations whose seq < snapshotseq after receiving upstream backup notification (keys and seqs) from controller
		+ Add outband_packet_format.h and outband_packet_format.c for switchos/controller/server
			+ Implement snapshot-related packets including snapshot signal, snapshot getdata ack, and snapshot senddata
			+ Implement upstream-backup-related packets
		+ Controller sends upstream backup notification to each physical client
			* Count the notification of releasing snapshotted data from upstream backup into bandwidth cost
		+ For speical case bwcost
			* Switchos maintains and embeds per-server speical case bandwidth cost into snapshot data
			* Controller dumps localbwcost as well as serveridx
	- Dump client-side backup in TerminatorThread
	- Debug and test
		+ Fix compilation errors of client, switch, and server&controller
		+ Pass p4 compilation under range partition
		+ Pass p4 compilation under hash partition
		+ Send PUT + READ + DEL + READ w/o and w/ warmup to check if client-side seq is correct
			* Fix issue of not read seq from deleted set
			* Fix issue of sending value with null value data
			* Fix issue of invalid type of SNAPSHOT_SENDDATA
			* Fix issue of upstream backup notification

* Implement replay-based recovery
	- Replay cache admissions for in-switch cache based on in-switch snapshot (files: scripts/common.sh scripts/remote/test_recovery_time.sh, farreach/localscripts/launchswitchtestbed.sh, farreach/switchos.c, farreach/localscripts/fetchsnapshot_controller2switch.sh)
		+ Login bf1 as root -> launch switch data plane -> copy in-switch snapshot data -> configure switch data plane and launch switchos w/ recovery mode
		+ switchos sends CACHE_POP_INSWITCH to admit snapshot keys w/ values w/o waiting for ACKs
			* NOTE: we do NOT need recover_switch.sh to admit snapshot records
			* NOTE: reflector/ptf needs to update switchos addr each time
			* [IMPORTANT] NOTE: switchos simply poses a warning for the cache admission of a cached key instead of exiting, as now we do not retrieve server-side cached keyset during recovery
	- Replay record updates for server-side KVS based on in-switch snapshot and client-side record preservations (files: farreach/localscripts/fetchbackup_client2server.sh, farreach/server.c, farreach/server_impl.h)
		+ Copy corresponding client-side backups to server
			* Update upstream backup client-side filepath
			* Dump stat for upstream backups
		+ Aggregate per-client backups into a single map
			* For dynamic pattern, directly aggregate two upstream backups
			+ For static pattern, aggreagate per-rotation upstream backups of two clients
		+ Each server worker uses the corresponding aggregated backup map for recovery
	- Fix maxseq issue (NOTE: no P4 modification) (files: common/rocksdb_wrapper.c, common/io_helper.c, farreach/localscripts/fetchsnapshot_controller2switch.sh -> fetchall_all2switch.sh, farreach/switchos.c, farreach/tofino/ptf_popserver/table_configure.py, farreach/common_impl.h, common/iniparser/iniparser_wrapper.c, farreach/tofino/common.py)
		+ Server updates maxseq for each cache miss of put/del write request
			* Server saves maxseq of cache misses for each snapshot, and also saves latest maxseq after finishing transaction phase
			* Server loads latest/snapshotted maxseq if any when opening, and choose the larger one as final maxseq
		+ Switchos copies maxseq files from server0/1 to switch
			* Switchos gets maxseq of cache hits based on in-switch snapshot and client-side upstream backups
			* Switchos gets maxseq of cache misses based on per-server latest/snapshotted maxseq
			* Switchos gets final maxseq based on maxseq of cache hits/misses, and write all seq_reg conservatively by ptf.popserver
