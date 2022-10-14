# Implementation log of YCSB

- FUTURE
	* TODO: Implement DistfarreachClient, DistnocacheClient, and DistcacheClient in YCSB (just with different methodids)
		- NOTE: Add preparefinish_client in prebenchmark of distfarreach to trigger snapshot
		- [IMPORTANT] current distributed extension is a single discussion instead of a critical design -> NOT need to evaluate
	* TODO: Encapsulate GET/PUT/DEL/SCAN in inswitchcache-c-lib/ for remote_client.c
		- [IMPORTANT] NOT need to provide c-lib for db_bench

- 10.14 ([IMPORTANT] start evaluation)
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
		* TODO: Others in C
			- TODO: Add control plane bandwidth usage calculation
			- TODO: Maintain benchmark/results/, and benchmark.md of each command and code/configuration change
		* TODO: Support range query
			- TODO: Add SCANRES_SPLIT (use Map::Entry as pair)
			- TODO: Add _udprecvlarge_multisrc_ipfrag and _udprecvlarge_multisrc_ipfrag_dist in JNI-based socket
			- TODO: Add parsebufs_multisrc_ipfrag(_dist) for udprecvlarge_multisrc_ipfrag(_dist) in Java
			- TODO: Update benchmark.md
		* TODO: Test static workload script w/ 16 servers
			- TODO: Update test_server_rotation.sh and stop_server_rotation.sh -> TODO: SYNC to all methods after testing correctness
	+ Huancheng
		* TODO: Implement YCSB trace replay for Twitter trace
			- TODO: Implement TraceReplayWorkload for Twitter traces
			- (TODO after discussion) Filter requests if workload mode = 0 (or use keydump + PregeneratedWorkload)
		* TODO: Check whether nocache saves seq=0 for LOADREQ and read seq=0 for READREQ with 128B value
			- TODO: Check valsize in server log
		* TODO: Use loading phase to pre-load 100M records into 16 storage servers (NOTE: recordload client + nocache switch/server)
			- TODO: Use script to backup the loaded database files (worker0/ - worker7/ in server 0 and worker8/ - worker15/ in server 1) into both server 0 and server 1
				+ NOTE: place backuped files into /home/backupedrocksdb instead of /tmp
				+ TODO: Change permission to all users for rocksdb files after loading
		* TODO: Test preparefinish_client at withinBenchmark() to see whether java can invoke shell command successfully
		* TODO: Start evaluation of experiment 2


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
