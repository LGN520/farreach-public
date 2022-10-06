# Implementation log of YCSB

- TODO
	+ TODO: Use loading phase to pre-load 100M records into stoarge server (NOTE: without in-switch cache)
	+ TODO: Test farreach/nocache/netcache under hotin pattern for six workloads
	+ Debug keydump module to get keydump results
		* TODO: Overwrite workload based on config (map workload name into workload property), cancel required param for workload property
			- TODO: Remove -df from command-line parameters
		* TODO: Send back SendPktRcvAck from sendpktserver to sendpktclient
		* TODO: Use custom pair for SCANRES_SPLIT

- 10.7
	+ Siyuan
		* TODO: Re-organize ycsb/ as benchmark/, including ycsb/, inswitchcache-java-lib/, inswitchcache-c-lib/, output/, results/
			- TODO: Disaggregate JAVA-based lib for YCSB
		* TODO: Update jnisrc to use libcommon
		* TODO: Add udprecvlarge_ipfrag_dist in JNI-based socket
		* TODO: Add udprecvlarge_ipfrag_multisrc and udprecvlarge_ipfrag_multisrc_dist in JNI-based socket
		* TODO: Implement NoCacheClient, NetCacheClient, DistfarreachClient, DistnocacheClient, and DistcacheClient in YCSB
		* TODO: Encapsulate GET/PUT/DEL/SCAN in inswitchcache-c-lib/ for remote_client.c
	+ Huancheng
		* TODO: Use inswitchcache.core.Key/Value in farreach and keydump
		* TODO: Update inswitchcache.core.PacketFormat as common/ (introduce methodid yet NOT affect UDP packet content)
		* TODO: Double check remote_client of farreach (e.g., add preparefinish_client in prebenchmark of farreach)
		* TODO: Try hotout and random workloads
		* TODO: Encapsulate an individual class for GET/PUT/DEL/SCAN of FarreachClient (NOT need InetAddress of ip and svraddr of udprecvfrom)

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
	+ Huancheng
		* TODO: Check correctness of JNI on UDP checksum for cache hits/misses
		* TODO: Reproduce preliminary results on hotin workload
		* TODO: Check correctness of loading phase by LOADREQ
		* TODO: Update run.md (make rocksdb and common -> make each method)

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
