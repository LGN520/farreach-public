# Implementation log of YCSB

- 9.21
	+ Implement Keydump and rename netbuffer as farreach
		* Extract RemoteDB static members as InswitchCacheClient
		* Process per-keydump-client keyfrequencymap in main client, and dump hotest keys
		* Convert keystring into 16B key instead of changing workload file
	+ Implement dynamic array and part of socket helper

- 9.22
	+ Implement socket helper based on JNI
		* Remove redundancy of RemoteDB; use short for UDP port; 
		* Move Key/Value from farreach into core module
		* Calculate cache hit rate of keydump file

- 9.23
	+ Implement DyanmicRulemap for dynamic workload

- 9.24
	+ Re-organize YCSB client code

- 9.27
	+ Decouple inswitch-cache-related lib as an individual module in YCSB

- 10.3 - 10.4
	+ Code for dynamic workload
		* Update helper.h, warmup_client.c, and ycsb_parser.c for new filepath and new file format -> SYNC to ALL
		* Add large value in JNI -> update header file after fixing java compilation errors
		* Implement rulemap generation for hot-in, random, and hot-out workload
	+ Fix tiny issues
		* Remove -cf and -threads from command-line parameters
		* Place all command-line parameters related with InswitchCache into GlobalConfig -> only use GlobalConfig in subthreads
	+ Set seed = threadid in random generator for each logical client in YCSB -> test keydump results
	+ Huancheng
		* Re-implement sendpkt client/server and rulemap client/server
		* Implement rulemapclient in TerminatorThread to notify rulemap switching during each period
	+ Try individual jar

- 10.5 - 10.6
	+ Disaggregate C-based lib for remote_client.c and server for ALL methods
		* [IMPORTANT] NOTE: the first commit for libcommon is b18b0a3e10f0b3fe76e7006fdfc1d0f38925507d with message of "add common module for benchmarks and servers"
		* Place the same modules into common/ (including crc32, key, value, dynamic_array, dynamic_rulemap, helper, latency_helper, pkt_ring_buffer, snapshot_record, special_case, workloadparser/\*, iniparser/\*)
			- Merge ALL packet_format\.* as a single file with methodid as a parameter, and place it into common/
			- Introduce methodid into socket_helper, rocksdb_wrapper, io_helper
				+ NOTE: pkt_ring_buffer and dynamic_rulemap ONLY need packet_type_t -> NOT introduce methodid as parameter
		* Compile common/ as libcommon.a for benchmark/ and method/
			- Update usage of packet_format, socket_helper, rocksdb_wrapper, io_helper (files: loader.c, localtest.c, remote_client.c, server_impl.h, switchos.c, controller.c, recover/\*) -> TODO SYNC to ALL
			- Update Makefile and includes by running replace.sh -> TODO SYNC to ALL
			- TODO: Update jnisrc to use libcommon.a
		* TODO: Re-organize ycsb as benchmark
		* TODO: Encapsulate GET/PUT/DEL/SCAN in inswitchcache-c-lib/ for remote_client.c
	+ TODO: Update the changes as a README in root file
		* Make rocksdb and common -> each method
	+ Huancheng
		* TODO: Disaggregate JAVA-based lib for YCSB
		* TODO: Use inswitchcache.core.Key/Value in farreach and keydump
		* TODO: Encapsulate an individual class for GET/PUT/DEL/SCAN of FarreachClient (NOT need InetAddress of ip and svraddr of udprecvfrom)
		* TODO: Update inswitchcache.core.PacketFormat as common/ (introduce methodid yet NOT affect UDP packet content)

- TODO
	+ TODO: Add preparefinish_client in prebenchmark of farreach
	+ Debug keydump module to get keydump results
		* Use JNI-based socket for farreach -> TODO: test and evaluate overhead
		* TODO: Overwrite workload based on config (map workload name into workload property), cancel required param for workload property
			- TODO: Remove -df from command-line parameters
		* TODO: Send back SendPktRcvAck from sendpktserver to sendpktclient
		* TODO: Use custom pair for SCANRES_SPLIT

- TODO
	+ TODO: Add udprecvlarge_ipfrag_dist in JNI-based socket
	+ TODO: Add udprecvlarge_ipfrag_multisrc and udprecvlarge_ipfrag_multisrc_dist in JNI-based socket
