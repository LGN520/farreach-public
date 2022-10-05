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
		* TODO: Re-implement sendpkt client/server and rulemap client/server
		* TODO: Implement rulemapclient in TerminatorThread to notify rulemap switching during each period
	+ Try individual jar

- 10.5
	+ Huancheng
		* TODO: Disaggregate JAVA-based lib for YCSB
		* TODO: Use inswitchcachecore.Key/Value in farreach and keydump
		* TODO: Encapsulate an individual class for GET/PUT/DEL/SCAN of FarreachClient (NOT need InetAddress of ip and svraddr of udprecvfrom)
	+ TODO: Add preparefinish_client in prebenchmark of farreach
	+ Debug keydump module to get keydump results
		* Use JNI-based socket for farreach -> TODO: test and evaluate overhead
		* TODO: Overwrite workload based on config (map workload name into workload property), cancel required param for workload property
			- TODO: Remove -df from command-line parameters
		* TODO: Send back SendPktRcvAck from sendpktserver to sendpktclient
		* TODO: Use custom pair for SCANRES_SPLIT

- TODO
	+ TODO: Add udprecvlarge_ipfrag_dist in JNI-based socket
