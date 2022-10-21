# Implementation log of YCSB

- FUTURE
	* TODO: For synthetic workload, add write ratio, skewness, and value size into StaticStatisticsFilepath -> introduce too many CLI parameters into scripts/remote/calculate_statistics.sh and change too many lines of code in YCSB
	* TODO: Reduce redundant switch-related scripts in method/localscripts/
	* TODO: Implement DistfarreachClient, DistnocacheClient, and DistcacheClient in YCSB (just with different methodids)
		- NOTE: Add preparefinish_client in prebenchmark of distfarreach to trigger snapshot
		- [IMPORTANT] current distributed extension is a single discussion instead of a critical design -> NOT need to evaluate
	* TODO: Encapsulate GET/PUT/DEL/SCAN in inswitchcache-c-lib/ for remote_client.c
		- [IMPORTANT] NOT need to provide c-lib for db_bench

- TODO
	* Support range query
		- TODO: Add SCANRES_SPLIT (maybe use Map::Entry as pair)
		- (Discuss first before implementation) update JNI for range query
			- TODO: Invoke _udprecvlarge_multisrc_ipfrag and _udprecvlarge_multisrc_ipfrag_dist of libcommon in JNI-based socket
				+ TODO: If under server rotation, directly return after receiving all SCANRES_SPLITs of one src (one server / one server + one switch) (change libcommon by Siyuan)
				+ TODO: Pass one Java dyanmic array as a parameter to store the encoded result
					* TODO: In JNI, encode all C dynamic arrays as one dynamic array, copy it to the Java dynamic array
					* TODO: In JAVA, decode the single Java dynamic array into multiple dynamic arrays
			- TODO: Add parsebufs_multisrc_ipfrag(_dist) for udprecvlarge_multisrc_ipfrag(_dist) in Java
			- TODO: Update DbUdpNative to invoke the native function to receive SCANRES_SPLIT

- 10.23 (Sunday)
	+ Huancheng
		* Evaluation
			* TODO: Launch nocache for experiment 2 with 128 servers under YCSB A
			* TODO: Run experiment 3
				- TODO: Use loading phase to pre-load 100M records into 2 storage servers (w/ workload_mode=1)
			* TODO: Run experiment 1 on Twitter Traces
				- NOTE: double-check the Twitter Traces of the choosen clusters before experiments (maybe we can have a discussion)
			* TODO: Start experiment 4

- 10.22 (Saturday)
	+ Siyuan
		* TODO: Update evaluation writing
	+ HuanCheng
		* Evaluation
			* TODO: Run nocache/farreach/netcache for experiment 2 with 32/64 servers
				- TODO: If not load before, use loading phase to pre-load 100M records into 32/64 storage servers
			* TODO: Launch netcache for experiment 2 with 128 servers under YCSB A
		* Coding
			* TODO: Finish TraceReplay workload
				- TODO: Get correpsonding trace file based on workloadName
				- TODO: Limit the maximum number of parsed requests, and the maximum value size based on its paper
				- TODO: Comment request filtering under static pattern in TraceReplayWorkload -> resort to KeydumpClient and PregeneratedWorkload
				- TODO: Twitter key -> keystring by md5 -> inswitchcache.core.Key by fromString
			* TODO: Fix retrieving issue of deleting /tmp/rocksdbbackups/16
			* TODO: Fix issue of not overwriting existing statistics in single rotation mode (maybe due to using wrong value of -sr)

- 10.21 (Friday)
	+ Siyuan
		* Update statistics module to fix runtime variance under server rotation especially for large scale
		* Test in-memory KVS under PKU's testbed
			+ TODO: Wait for XMU's testbed to install 8.9.1 compiler
				+ Test correctness of farreach P4 under 1M records
					* TODO: Try warmup phase to check if the hot keys are cached
					* TODO: Send some requests to see if cache hit rate is reasonable and all responses of the requests can be received
			+ TODO: Test dynamic workload performnace of FarReach under 100M records (TODO: re-run keydump)
				* NOTE: as our cache hit latency 30~40 us is lower than NetCache 5 us due to testbed difference, we need more client threads to saturate the system
				* TODO: Disk bottleneck (still 2 logical servers): <20 MQPS with in-memory KVS instead of RocksDB
					- Thpt under 256*2 logical clients: TODO MQPS
					- Thpt under 512*2 logical clients: TODO MQPS
				* TODO: Network bottleneck (with 64 logical servers): <40 MOPS under 1024*2 logical clients
					- Thpt under 512*2 logical clients: TODO MQPS
					- Thpt under 1024*2 logical clients: TODO MQPS
				* Therefore, the reason of the difference (around 100X) between our absolute result and that in NetCache paper is disk
					- Single-server RocksDB w/o cache: <0.1 MQPS; single-server TommyDS w/o cache: <10 MQPS
					- Our static thpt based on RocksDB under 64 servers w/ cache: ~20 MOPS (TODO); that of NetCache: 2 GQPS
	+ HuanCheng
		* Evaluation
			* Enable server-side snapshot successfully
				* For farreach, test preparefinish_client at withinBenchmark() to see if java can trigger snapshot successfully
					- Check tmp_controller.out. tmp_switchos.out, and tmp_controller_bwcost.out
			* TODO: Re-run all experiment 1 after fixing runtime variance
				- NOTE: run farreach/netcache/nocache on workload C first
				- TODO: Move original benchmark/results/exp1.md as benchmark/results/exp1_old.md; create a new exp1.md
				- TODO: Update thpt/latency numbers in benchmark/results/exp1.md, and also keep necessary data files in benchmark/results/exp1/
					+ NOTE: we need to keep Json files but not track them in git as they are too large
			* TODO: Launch FarReach + 128 servers for experiment 2 under YCSB A
				* TODO: Use loading phase to pre-load 100M records into 128 storage servers

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
