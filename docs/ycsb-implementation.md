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
		* TODO: Use custom pair for SCANRES_SPLIT
		* Move Key/Value from farreach into core module
		* TODO: Try JNI-based socket in InswitchCacheClient -> test and evaluation overhead of JNI-based socket
		* Calculate cache hit rate of keydump file
		* TODO: Encapsulate an individual class for GET/PUT/DEL/SCAN of FarreachClient

- 9.23
	+ Implement DyanmicRulemap for dynamic workload

- 9.24
	+ Re-organize YCSB client code
	+ TODO: Debug keydump module to get keydump results
		* TODO: Use core.Key/Value in farreach and keydump
		* TODO: Use JNI-based socket for farreach
		* TODO: Implement rulmapclient in TerminatorThread to notify rulemap switching during each period
		* TODO: Send back SendPktRcvAck from sendpktserver to sendpktclient
		* TODO: Overwrite workload based on config (map workload name into workload property), cancel required param for workload property

- TODO
	+ TODO: Re-generate hot-in, random, and hot-out dynamic patterns
	+ TODO: Support large value in socket helper, and update packet
