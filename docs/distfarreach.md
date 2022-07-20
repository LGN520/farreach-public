## Deprecated staff

+ DEPRECATED design for cache population/eviction in DistFarReach
	* NOTE: DistFarReach triggers cache population by servers -> key of CACHE_POP must NOT be cached -> key MUST be cached at most ONCE and hence in at most ONE switch
	* NOTE: to keep stateless controller, controller sends CACHE_POP_ACK to server only if receive CACHE_POP_ACK from spine/leaf, which means that some switchos.popworker has performed cache population/eviction for the key
		- TODO: server sends CACNE_POP to controller by server.popclient
			+ NOTE: wait time for CACHE_POP_ACK in distributed scenario becomes larger than single-switch -> using server.popclient can avoid from affecting server.worker performance for normal requests
			+ TODO: Now we still use server.worker.popclient instead of server.popclient as cache eviction is rare compared with normal requests
	* controller.popserver receives CACHE_POP from server.popclient
		- controller.popserver calculates logical spineswitchidx based on the key to find physical spine switch
			+ NOTE: as we only have one physical spine switch, we validate the logical spineswitchidx
		- TODO: controller.popserver.popclient_for_spine forwards CACHE_POP to switchos.popserver of corresponding spine switch
		- TODO: controller.popserver.popclient_for_spine waits for CACHE_POP_ACK / CACHE_POP_FULL_ACK
		- TODO: If receive CACHE_POP_ACK from spine switch, controller.popserver forwards CACHE_POP_ACK to server.popclient, and waits for next CACHE_POP from server.popclient
		- If receive CACHE_POP_FULL_ACK from spine switch
			+ controller.popserver calculates logical leafswitchidx based on the key to find physical server-leaf
				* NOTE: as we only have one physical leaf switch, we validate the logical leafswitchidx
			+ TODO: controller.popserver.popclient sends CACHE_POP to switchos.popserver of corresponding phyiscal leaf
			+ TODO: controller.popserver.popclient waits for CACHE_POP_ACK / CACHE_POP_FULL_ACK
			+ TODO: If receive CACHE_POP_ACK from leaf switch, controller.popserver forwards CACHE_POP_ACK to server.popclient, and waits for next CACHE_POP from server.popclient
			+ If receive CACHE_POP_FULL_ACK from leaf switch
				* NOTE: we do NOT need to find global victim by CACHE_POP_GETVICTIMs; instead, we still use sampling to reduce cache pop latency
				* TODO: controller.popserver.popclient samples one switch from spine/leaf and sends CACHE_POP_FULL w/ new key to sampled switch
				* TODO: controller receives CACHE_POP_ACK from sampled switch, and forwards it to server.popclient
	* For each physical spine/leaf switchos, after receiving CACHE_POP from controller.popserver.popclient
		- TODO: switchos.popserver checks inswitch cache size first
			+ TODO: Maintain a mutex lock for switchos_cached_empty_index
			+ TODO: If the current switch is not full, switchos.popserver sends CACHE_POP_ACK to controller.popserver.popclient, and pass CACHE_POP w/ new key to switchos.popworker for cache population
			+ TODO: If the current switch is full, switchos.popserver sends CACHE_POP_FULL_ACK to controller.popserver.popclient, yet NOT pass CACHE_POP w/ new key to switchos.popworker for cache population
	* For each physical spine/leaf switchos, after receiving CACHE_POP_FULL from controller.popserver.popclient
		- TODO: switchos.popserver directly sends CACHE_POP_ACK w/ new key to controller.popserver.popclient, and pass CACHE_POP_FULL w/ new key to switchos.popworker for cache eviction
		- TODO: NOTE: CACHE_POP_FULL is inherited from CACHE_POP
		- TODO: NOTE: switchos_cached_empty_index must be full
+ DEPRECATED implementation of cache population/eviction in DistFarReach
	+ TODO: controller.popsever.popclient_for_spine forwards CACHE_POP to spine.switchos.popserver and waits for CACHE_POP/_FULL_ACK (files: controller.c)
		* NOTE: controller needs to communiate with both spine/leaf switchos -> need two sets of global variables; while reflector/switchos can update exclusive global variables based on its role (spine or leaf)
		* TODO: Add packet type of CACHE_POP_FULL_ACK (files: tofino-*/main.p4, tofino-*/common,py, tofino-*/p4src/parser.p4, packet_format.*, common_impl.h)
	+ spine/leafswitchos.popserver processes CACHE_POP from controller.popserver.popclient_for_spine/leaf
		* TODO: Add mutex lock for inswitch cache metadata (writer: switchos.popworker; reader: switchos.popsever/snapshotserver) (files: switchos.c)
		* TODO: switchos.popserver gets pipeidx based on role (spineswitchos -> leafswitch.pipeidx; leafswitchos -> server.pipeidx)
		* TODO: If spine/leaf inswitch cache of the pipeidx is NOT full, switchos.popserver sends CACHE_POP_ACK to controller.popserver.popclient_for_spine/leaf, and passes CACHE_POP to switchos.popworker
		* TODO: If spine/leaf inswitch cache of the pipeidx is full, switchos.popserver sends CACHE_POP_FULL_ACK to controller.popserver.popclient_for_spine/leaf
