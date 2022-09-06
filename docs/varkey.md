# Variable-length key for all six methods (nocache/netcache/farreach/distcache/distnocache/distfarreach)

- [IMPORTANT] NOTEs of variable-length key
	+ We only consider large value > ip fragment maxsize; yet NOT consider large key > ip fragment maxsize
		* TODO: Check max keysize and max valsize of Twitter Trace
		* TODO: For each request, client limits keysize <= 100B and valsize <= 10KB
	+ ONLY client-issued packets and cache population need to embed original key for server-side operations
		* All of switchos/server-issued packets including responses do NOT need to embed original key
	+ NOTE: we perform range partition for the most significant 16b of fixed-length key due to Tofino limitation -> we use the same strategy in all methods for fair comparison
		* For range partition of variable-length key, it belongs to client-level/application-level engineering work; for example:
			- We can add a range-partition-aware converstion layer betwen variable-length key and fixed-length key, e.g., place the original 16 bits required by range partition into the most significant 16b of fixed-length key, and hash remaining keybytes into other bits
			- We can also use client-side partition or server-side partition, as long as all method use the same strategy for fair comparison
		* NOTE: we assume that clients use bytewisecomparator to compare/sort keys -> we directly store original keybytes into rocksdb to use the default bytewisecomparator in rocksdb
			- For other key comparison methods, it is just a client-level/application-level difference -> we can change VarKey::from/to_string_for_rocksdb to support them

## Implementation of part 1

- TODO: Part 1 of variable-length key
	+ For nocache/netcache/distnocache/distcache/farreach/distfarreach
		* Hash variable-length original key into 16B fixed-length hashed key (files: varkey.*, md5.*, Makefile)
		* TODO: Embed variable-length original key into payload for all requests and their extended packets (files: varkey.*, packet_format.*, remote_client.c)
			- Implement get_ophdrsize, serialize_ophdr, dynamic_serialize_ophdr, and deserialize_ophdr in Packet and use them in all packets -> SYNC to ALL (files: packet_format.*)
			- Use val_t::SWITCH_MAX_VALLEN for small value; use val_t::MAX_VALLEN for large value (files: packet_format.*)
			- Involved packets (adding varkey into payload, use standard APIs for ophdr, distinguish val_t::SWITCH_MAX_VALLEN/MAX_VALLEN for small/large value)
				+ TODO: === END HERE === before PUTREQ_SEQ_CASE3 for all six methods (DELREQ_SEQ_INSWITCH_CASE1 also NOT updated for varkey) -> NOTE: we save updated files for varkey in NetBuffer/varkey-files/
				+ Including varkey (aka original variable-length key): GETREQ (-> GETREQ_POP, GETREQ_NLATEST), PUTREQ, DELREQ, SCANREQ, PUTREQ_SEQ (-> PUTREQ_POP_SEQ), PUT/DELREQ_SEQ_INSWITCH_CASE1 (NOT need to deserialize the varkey inherited from original PUT/DELREQ)
				+ Others: GETRES (-> GETRES_SERVER, GETRES_LATEST_SEQ), PUTRES (-> PUTRES_SERVER), DELRES (-> DELRES_SERVER), SCANRES_SPLIT (NOTE: w/o varkey of start/endkey; w/ varkeys of payload.pairs) (-> SCANRES_SPLIT_SERVER), GETRES_LATEST_SEQ (-> GETRES_LATEST_SEQ_SERVER, GETRES_DELETED_SEQ, GETRES_DELETED_SEQ_SERVER, GETRES_DELETED_SEQ_INSWITCH_CASE1)
					* TODO: NOTE: for SCANRES_SPLIT, use varkey instead of fixkey in payload.pairs (files: server_impl.h)
			- NOTE: all responses do NOT need to embed original key into payload
		* TODO: Server extract original key and val from each request, and stores <original key, original val> into rocksdb (files: server_impl.h)
		* For cache population/eviction
			- TODO: switchos needs to maintain cached_keyarray and cached_varkeyarray
				+ NOTE: CAHCE_POPs from different servers may have different varkeys yet with the same fixkey if NOT partitioned based on the fixkey
					* In farreach/distfarreach
						- TODO: switchos WARNs original key w/ hash collision and remembers serveridx (if != the server in cached_serveridxarray) into collided_serveridxsetarray instead of exit
							+ TODO: NOTE: as we use in-switch paritition based on fixkey, different varkeys hashed into the same fixkey MUST be partitioned into the same server -> server will NOT report cache populations w/ hash collision -> our collided_serveridxsetarray should always be empty!
						- TODO: server maintains cached keyset for fixkeys instead of varkeys as usual
						- TODO: controller use fixkey-based sampline instead of random sampling to assign hot key to a specific switch
						- TODO: switchos notifies evicted fixkey to not only the server in cached_serveridxarray but also the servers in collided_serveridxsetarray during cache eviction
						- TODO: NOTE: controller needs to send varkeys within snapshot data
					* In netcache/distcache
						- TODO: switchos just WARNs original key w/ hash collision instead of exit
							+ NOTE: data plane could report different varkeys mapping into the same fixkey due to cache collision, but switchos only caches the fixkey for the first varkey yet not for other varkeys -> netcache does NOT fetch value of other varkeys from server; distcache does NOT perform phase 1 of cache coherence for other varkeys by server -> switchos does NOT need to remember collided_serveridxsetarray to notify other servers during cache eviction
						- TODO: server maintains cached keyset for varkeys (original keys)
						- TODO: switchos fetches the value of varkey in netcache or perform phase 1 for varkey in distcache instead of fixkey
						- TODO: switchos notifies cache eviction to server for varkey instead of fixkey
			- TODO: [IMPORTANT] when dumping hot varkeys for warmup phase, pre-convert original varkeys into fixkeys to check whether we have 10,000 hot records -> if w/ hash collision, continue to dump until we have 10,000 hot records
		* TODO: Padding small value (vallen <= 128B) to 128 bytes (NOTE: keep original vallen; just change bytes) (files: val.*)
			- Example: 2B vallen + 128B valbytes, where vallen can be any value <= 128B (e.g., 0)
			+ NOTE: for large value (vallen > 128B) in PUTREQ/GETRES_LARGEVALUE, we still use 4B vallen + vallenB valbytes as usual
			+ [IMPORTANT] NOTE: such padding is acceptable in implementation (just an implementation trick for Tofino) -> reasons:
				* We do NOT use 8B-padding for small value due to Tofino itself limitation for pktlen update instead of due to our design
					- It could be easily fixed by future hardware architecture
				* Our problem is to use inswitch cache to improve performance by cache access/management and provide reliability by snapshot, instead of minimizing data plane BW usage -> we do NOT care about how we send a small value, e.g., no padding or padding to 128B
					- As padding to 128B does NOT trigger IP fragmentation, and our bottleneck is server-side load imbalance, CPU, and disk instead of switch or NIC or network BW -> 0B or 128B for small value within a packet does NOT affect latency and throughput
					- Even if we need to count data plane BW usage, it is still OK as it is equivalent to simulating variable-length small value by 2B vallen
			- TODO: Change update_pktlen_tbl -> NOT need to match vallen now (files: tofino*/p4src/egress_mat.p4, tofino*/configure/table_configure.py)
			- TODOTODO: If we still use 8B-aligned valbytes based on vallen as usual
				+ For GETRES_LATEST_SEQ_CASE1 and PUTREQ_SEQ_CASE1
					* NOTE: for GETRES_DELETED_SEQ_CASE1 and DELREQ_SEQ_CASE1, the original vallen MUST be 0 -> we can only range match new vallen as usual
					* We need an extra MAT to range match original vallen to get 8B-aligned value
					* We need an extra MAT to range match new vallen to get 8B-aligned value
					* We need to calculate vallen delta based on the two 8B-aligned values in eg_port_forward_tbl
					* We need to add the vallen delta into udp/iplen as well as len of extra packet header fields in update_pktlen_tbl
					* For response converted from XXX_CASE1, we need to range match vallen to subtract udp/iplen in update_pktlen_tbl
				+ NOTE: due to Tofino power budget and other limitations, we CANNOT add the above extra functions
		* TODO: Compile and test nocache/distnocache
			- TODO: SYNC varkey and md5 to ALL after test
	+ For netcache/farreach/distcache/distfarreach
		* TODO: Update pktlen of optypes related with requests by add/subtract_pktlen instead of update_pktlen (files: tofino*/p4src/egress_mat.p4, tofino*/configure/table_configure.py)
			- TODO: Use add/subtract/update_pktlen in update_pktlen_tbl
			- TODO: For response of cache hit (also belong to server-issued packets)
				+ NOTE: response from server does NOT have payload of original key, yet respones of cache hit has the payload!
				+ Trial: directly update_pktlen as usual -> TODO: check whether client can receive the response of cache hit w/ correct ip/udp pktlen yet w/ incorrect bytes due to payload of original key inherited from request
				+ TODO: If NOT receive response w/ incorrect bytes, use GET/PUT/DELRES_CACHEHIT for response of cache hit
					* TODO: Use add/subtract_pktlen for GET/PUT/DELRES_CACHEHIT
					* TODO: Use update_pktlen for GET/PUT/DELRES (for special response in farreach/distfarreach)
						- TODOTODO: If response w/ original key, we do NOT change pktlen of GET/PUT/DELRES; for special response in farreach/distfarreach, we must subtract ip/udp pktlen in ig/eg_port_forward_tbl
			- For switchos/server-issued packets
				+ NOTE: all of CACHE_POP_INSWITCH/_ACK, CACHE_EVICT_LOADFREQ_INSWITCH/_ACK, CACHE_EVICT_LOADDATA_INSWITCH/_ACK, SETVALID_INSWITCH/_ACK, SNAPSHOT_LOADDATA_INSWITCH/_ACK, NETCAHCE_VALUEUPDATE/_ACK, DISTCACHE_INVALIDATE/_ACK, DISTCACHE_VALUEUPDATE_INSWITCH/_ACK do NOT need to embed original key into payload -> they can directly use update_pktlen
			- For extended packets from client-issued requests
				+ TODO: Use add_pktlen for PUT/DELREQ_SEQ
				+ TODO: Use add_pktlen for the first packet of NETCACHE_WARMUPREQ_INSWITCH_POP, NETCACHE_GETREQ_POP, PUT/DELREQ_SEQ_INSWITCH_CASE1, GETRES_LATEST/DELETED_SEQ_CASE1
					* TODO: Use subtract_pktlen for the last packet of above optypes
					* NOTE: due to limited stages and power budget in Tofino, we CANNOT calculate vallen delta for XXX_CASE1 -> in implementation, we use a trick by always padding small value to 128 bytes for PUTREQ and GETRES/_SERVER/_LATEST/_DELETED

## Implementation of other parts

- TODO: Part 2 of variable-length key
	+ For netcache/distcache -> TODO: SYNC to farreach/distfarreach
		* TODO: Server embed original key into original val before cache population -> ONLY for netcache/distcache (files: varkey.*, server_impl.h)
		* TODO: Add optypes of NORMALREQ/NORMALREQ_SPINE/NORMALREQ_SEQ/NORMALREQ_SEQ_CASE3/NORMALRES_SERVER/NORMALRES including implementation (files: tofino*/main.p4, tofino*/common.py, packet_format.*, common_impl.h)
			- NOTE: NORMALXXX means that the hashed key MUST NOT be cached into programmable switch
			- TODO NOTE: all of them belong to large packets -> we should update optype_for_udprecvlarge_ipfrag_num/list and util funcs; implement get_frag_hdrsize and dynamic_serialize (files: packet_format.*)
				+ TODO: Check largepkt_totalsize for NORMALREQ_SEQ/NORMALREQ_SEQ_CASE3 in server.worker
			- NOTE: all of them ONLY w/ op_hdr + payload (payload_optype, originalkey_hdr w/ keylen and keybytes, originalval_hdr w/ vallen and valbytes)
				+ TODO: NOTE: for NORMALRES_SERVER/NORMALRES, we do NOT need originalkey -> originalkey.keylen MUST be 0
		* TODO: Check original key of GETRES w/ nodeidx_foreval = 0xff; if not matched, send NORMALREQ by udpsendlarge_ipfrag and wait for NORMALRES by udprecvlarge_ipfrag -> ONLY for netcache/distcache (files: remote_client.c)
		* TODO: Server receives NORMALREQ as a large packet, processes it based on payload.optype, and sends back NORMALRES by udpsendlarge_ipfrag (files: server_impl.h)
- TODO: Part 3 of variable-length key -> ONLY for farreach/distfarreach
	+ TODO: client-side metadata cache w/ strong consistency
- TODO: (Trial) Part 4 of variable-length key -> ONLY for farreach/distfarreach
	+ TODO: Copy directories of farreach/distfarreach as farreach-versioncheck/distfarreach-versioncheck for trial
	+ TODO: eventual consistency w/ version check to reduce controller overhead

## Deprecated staff

- TODO: Implement variable-length key
	+ [IMPORTANT] NOTE: we only consider large value > ip fragment maxsize; yet NOT consider large key > ip fragment maxsize
		* NOTE: if key > ip fragment maxsize, we can simply forward each fragment of NORMALREQ_LARGEKEY, yet we MUST generate multiple SCANREQ_LARGEKEY_SPLITs to different servers for each fragment of SCANREQ_LARGEKEY -> server needs udprecvlarge_ipfrag for SCANREQ_LARGEKEY_SPLIT w/ extra overhead
		* Therefore, we assume that key <= ip fragment maxsize (e.g., at most 125B in Twitter trae) -> we do NOT need to consider udprecvlarge_ipfrag for most XXX_LARGEKEY
			- TODO: NOTE: we still need to use udpsend/recvlarge_ipfrag for NORMALREQ_LARGEKEY due to possible large value
			- TODO: -> thus we also need to add NORMALREQ_LARGEKEY and NORMALRES_LARGEYKEY into optype_for_udprecvlarge_ipfrag_list
	+ NORMALREQ_LARGEKEY and NORMALREQ_LARGEKEY_SPINE: op_hdr w/ the most significant 16B key for partition + client_logical_idx + payload (specific optype, varkey w/ netreach_key_t and keylen and key, vallen, and value)
		* NOTE: NO cache hit; NO invalidation / valueupdate; yet with CASE3 (NEED NORMALREQ_LARGEKEY_INSWITCH and NORMALREQ_LARGEKEY_SPINE_INSWITCH for is_snapshot)
	+ SCANREQ_LARGEKEY and SCANREQ_LARGEKEY_SPLIT: op_hdr w/ the most significant 16B key for partition + scan_hdr w/ the most significant 16B endkey for partition + [split_hdr] + payload (varkey w/ keylen and key + varendkey)
		* SCANRES_LARGEKEY/_SPLIT: op_hdr w/ the most significant 16B key for partition + scan_hdr w/ the most significant 16B endkey for partition + [split_hdr] + payload (varkey w/ keylen and key + varendkey + pairnum + keyvalue pairs w/ any size)
+ TODO
	+ TODO: For variable-length key
		* NOTE: we CANNOT hash original key into 16B fixed key and embed original key into value, as Tofino cannot extract original key for write-back policy (cache_lookup_tbl is in ingress_pipeline due to stage limitation, while value is saved in egress_pipeline)
		* TODO: Padding <16B key to 16B, forward pkts w/ >16B key to server
			- NOTE: NetCache can hash original key into 16B fixed key to utilize in-switch cache, but it can increase value length -> if vallen > 128B, it cannot hit inswitch caceh and undermine read performance
			- NOTE: FarReach forward pkts w/ >16B key to server, which also undermines read performance
			- TODO: For >16B key, we can use a speical packet type (e.g., PUTREQ_LARGEKEY) to avoid inswitch processing; but we still need to place the last 16 bytes of the key into op_hdr for in-switch hash/range partition
				+ NOTE: we do NOT need to parse/deparse vallen/val_hdr for XXX_LARGEKEY
			- TODO: Using the same strategy to process variable-length key in application layer for fair comparison???
		* TODO: For pkts w/ >16B key, we CANNOT use in-switch partition; instead we use server-based partition
