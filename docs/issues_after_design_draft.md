# Issues found after writing design draft

## Preliminary results

+ The raw data of each method is saved in NetBuffer/method/results/preliminary/
- Preliminary results of write-only workload
	+ Static workload (hash partition w/ 1024 clients + 128 servers)
		* farreach: 20.22 MOPS thpt, 105.94 us avg latency
		* nocache: 1.45 MOPS thpt, 1377 us avg latency
			- NOTE: much larger avg latency due to server-side processing and queuing lateny
		* netcache: 0.17 MOPS thpt, 12855 us avg latency -> 0.91 MOPS thpt, 1841.82 us avg latency
			- Reason: extremely low thpt & large latency due to waiting for server-issued in-switch value update ACKs for cache coherence
				+ NOTE: thpt of nocache is 8X of netcache in our testbed, while thpt of nocache is 2X of netcache in netcache paper
			- Solution: NOT wait for ACK in netcache/distcache now, as they may argue that pktloss is rare and it is an engineering issue
			- Result: slightly worse than nocache which matches the results in netcache paper
		* distnocache: 1.17 MOPS thpt, 1396 us avg latency
		* distcache: 1.24 MOPS thpt, 1796 us avg latency
		* distfarreach: 18.5 MOPS thps, 107 us avg latency
	+ Dynamic workload (hash partition w/ 1024 clients + 8 servers)
		* Write-only workload
			- DONE: farreach, nocache, netcache, distnocache, distcache, distfarreach
		* Read-only workload
			- DONE: farreach, nocache, netcache, distnocache, distcache, distfarreach
		* 50%-write-50%-read workload
			- DONE: farreach, nocache, netcache, distnocache, distcache, distfarreach

## During prepare preliminary results

+ Under read-only dynamic workload, farreach is worse than netcache
	* Observation: not related with snapshot; not related with waiting for CACHE_POP_ACK; not related with server-side snapshot in rocksdb
	* Reason: different rocksdb files
	* [IMPORTANT] Solution: we should keep the same rocksdb files before evaluating each method
		- NOTE: still slightly smaller than NetCache due to limited conservative reads

+ DistCache/DistFarreach cannot populate new hot keys efficiently
	* Reason: ptf_cleaner fails after removing cm4
	+ [IMPORTANT] Optimize P4 code to support 4 CM register arrays in distcache and distfarreach
		* NOTE: poor perf under dynamic workload is due to the crashed ptf_cleaner when resetting cm4 \[and bf3\] instead of decreased CM register array (actually hash_calc/hash_calc1 and hash_calc2/hash_calc3 have the same hashed result)
		* TODOTODO: If we have sth new to implement in switch. we can consider to remove cm2 and cm4 to save hardware resources -> NOTE: update ptf_cleaner accordingly!!!

## About two design issues: recirculate for single pipeline mode and read-after-write-consistency for rare case with packet loss

- Single pipeline mode
	+ Under our Tofino, recirculate does not support cross-ingress-pipeline switching
		* The latest Tofino already supports it by recirculate_odd_pipe
- Read-after-write consistency
	+ In the rare case with packet loss, client may get a stale value before an ACKed value of the key being evicted -> violate key-value store semantics
+ [IMPORTANT] Discuss with Qun about single pipeline mode and read-after-write-consistency
	* For single switch pipeline
		- From design: recirculate issue is due to the limitation of Tofino itself, which is supported by PISA or Jupiter -> not the issue of our design
		- From evaluation: as Tofino data plane can update the snapshot module after receiving the command from the switch OS within limited time, we do not observe inconsistent results in our evaluation
	* For read-after-write-consistency (or availability)
		- For request-based invalidation: violate client-side semantics
		- For response-based invalidation: introduce a new problem about how to define a latest record
		- For request-based invalidation + in-switch sequence-based packet value update: too complex for programmable switch
		- Solution: request-based invalidation + server-based limited blocking

## Implementation for the above two design issues

### Read blocking for read-after-write consistency (availability)

- [IMPORTANT] Read blocking for the rare cases of cache eviction for availability (or read-after-write consistency)
	+ Blocking subsequent read requests until receiving a write request from data plane or controller for strong availability -> SYNC to farreach/distfarreach
		* Design NOTE: reasonable reasons
			- Only blocking read requests if with packet loss
				+ For read requests after write requests without packet loss, they will be processed by server without blocking
				+ For read requests before any write request, they will be processed by the programmable switch
			- Limited blocking time: RTT-level bound
			- Not too many read requests as we evict a less frequent key
			- Not affect write performance
		* Implementation NOTEs
			- NOTE: if cached=0, the key must be evicted by controller -> server must receive the victim record from controller -> NOT need blocking -> we ONLY consider cached=1
			- NOTE: if XXX_BEINGEVICTED is sent to leaf, spine MUST have a cache hit -> spine/leaf does NOT need inswitchhdr to access the in-switch cache in the egress pipeline
		* In switch
			- Add optypes including implementation of GETREQ_BEINGEVICTED, PUTREQ_SEQ/_CASE3_BEINGEVICTED, DELREQ_SEQ/_CASE3_BEINGEVICTED, and PUTREQ_LARGEVALUE_SEQ/_CASE3_BEINGEVICTED -> SYNC optypes to ALL (files: tofino*/main.p4, tofino*/common.py, packet_format.h; packet_format_impl.h, common_impl.h)
				+ NOTE: parse_fraginfo for PUTREQ_LARGEVALUE_SEQ/_CASE3_BEINGEVICTED -> ONLY for farreach/distfarreach (files: tofino*/p4src/parser.p4)
				+ NOTE: for PUTREQ_LARGEVALUE_SEQ/_CASE3_BEINGEVICTED, update optype_for_udprecvlarge_ipfrag_list, optype_with_clientlogicalidx_list, and related util funcs -> ONLY for farreach/distfarerach (files: packet_format.*)
			- Add optypes of PUT/DELREQ_SEQ/_CASE3_BEINGEVICTED_SPINE and PUTREQ_LARGEVALUE_SEQ/_CASE3_BEINGEVICTED_SPINE -> SYNC to ALL (files: tofino*/main.p4, tofino*/common.py, packet_format.h)
				+ NOTE: parse_fraginfo for PUTREQ_LARGEVALUE_SEQ/_CASE3_BEINGEVICTED_SPINE -> ONLY for farreach/distfarreach (files: tofino*/p4src/parser.p4)
				+ NOTE: NOT need to add PUTREQ_LARGEVALUE_SEQ/_CASE3_BEINGEVICTED_SPINE into optype_for_udprecvlarge_ipfrag_list, optype_with_clientlogicalidx_list, and related util funcs, as end-hosts will NOT process them
			- For GET
				+ In distfarreach.spine
					* For GETREQ, if cached=1, valid=3 (i.e., beingevicted=1), and latest=0, convert GETREQ_INSWITCH to GETREQ_BEINGEVICTED (files: tofino-spine/p4src/egress_mat.p4, tofino-spine/configure/table_configure.py)
				+ In distfarreach.leaf
					* For GETREQ_SPINE, if cached=1, valid=3 (i.e., beingevicted=1), and latest=0, convert GETREQ_INSWITCH to GETREQ_BEINGEVICTED, and update ip/mac as client2server (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/configure/table_configure.py)
					* For GETREQ_BEINGEVICTED, forward it to storage server by partition_tbl, and update ip/mac as client2server (files: tofino-leaf/configure/table_configure.py)
						- NOTE: NOT hash_for_cm and sample; NOT update CM, frequency; NOT access valid, latest, deleted, vallen, and val
						- NOTE NOT prepare for cache hit; NOT save_client_info
				+ In farreach.switch
					*  For GETREQ, if cached=1, valid=3 (i.e., beingevicted=1), and latest=0, convert GETREQ_INSWITCH to GETREQ_BEINGEVICTED (files: tofino/p4src/egress_mat,p4, tofino/configure/table_configure.py)
			- For PUT/DEL
				+ In distfarreach.spine
					* For PUT/DELREQ, if cached=1 and valid=3, convert PUT/DELREQ_INSWITCH to PUT/DELREQ_SEQ_BEINGEVICTED, add valheader, and update pktlen (tofino-spine/p4src/egrses_mat.p4, tofino-spine/configure/table_configure.p4)
					* For PUT/DELREQ_SEQ_BEINGEVICTED from leaf during single pipeline mode, forward them to leaf by partition_tbl, convert them as PUT/DELREQ_SEQ/_CASE3_BEINGEVICTED_SPINE in ig_port_forward_tbl based on inswitchhdr.is_snapshot (NOT need to update pktlen), add valheader (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configure/table_configure.py)
						- Access need_recirculate_tbl, recirculate_tbl, snapshot_flag_tbl (files: tofino-spine/ptf_snapshot/table_configure.py, tofino-spine/configure/table_configure.py)
						- NOTE: For PUT/DELREQ_SEQ from leaf during single pipeline mode, spine MUST have a cache miss -> NOT need to consider BEINGEVICTED. directly send PUT/DELREQ_SEQ_INSWITCH to leaf as usual
				+ In distfarreach.leaf
					* For PUT/DELREQ_SEQ, if cached=1 and valid=3, convert PUT/DELREQ_SEQ_INSWITCH to PUT/DELREQ_SEQ_/CASE3_BEINGEVICTED,, add valheader, and update ip/mac as client2server (NOT need to update pktlen) (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/configure/table_configure.py)
					* For PUT/DELREQ_SEQ_BEINGEVICTED
						- Access need_recirculate_tbl, recirculate_tbl, snapshot_flag_tbl (files: tofino-leaf/ptf_snapshot/table_configure.py, tofino-leaf/configure/table_configure.py)
						- If need_recirculate=0, forward to storage server by partition_tbl, convert them into PUT/DELREQ_SEQ/_CASE3_BEINGEVICTED based on inswitchhdr.is_snapshot in ig_port_forward_tbl, add valheader, and update ip/mac as client2server (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
							+ NOTE: NOT hash_for_cm and sample; NOT update CM, frequency; NOT access valid, latest, deleted, vallen, and val, savedseq, case1
							+ NOTE NOT prepare for cache hit; NOT save_client_info
						- If need_recirculate=1, forward it to the spine switch
					* For PUT/DELREQ_SEQ_/_CASE3_BEINGEVICTED_SPINE (need_recirculate MUST = 0), forward them to storage server by partition_tbl, convert them as PUT/DELREQ_SEQ_/_CASE3_BEINGEVICTED by ig_port_forward_tbl, add valheader, and update ip/mac as client2server (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
						- NOTE: NOT hash_for_cm and sample; NOT update CM, frequency; NOT access valid, latest, deleted, vallen, and val, savedseq, case1
						- NOTE NOT prepare for cache hit; NOT save_client_info
				+ In farreach.switch
					* For PUT/DELREQ, if cached=1 and valid=3, convert PUT/DELREQ_INSWITCH to PUT/DELREQ_SEQ/_CASE3_BEINGEVICTED, update pktlen, add valheader, and update ip/mac as client2server (files: tofino/p4src/egress_mat.p4, tofino/configure/table_configure.py)
			- For PUT with large value
				+ NOTE: NOT need to add valheader for PUT with large value
				+ In distfarreach.spine
					* For PUTREQ_LARGEVALUE, if cached=1 and valid=3, convert PUTREQ_LARGEVALUE_INSWITCH to PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED, and update pktlen (NOTE: NO valheader, NOT update ip/mac) (files: tofino-spine/p4src/egress_mat.p4, tofino-spine/configure/table_configure.py)
						- NOTE: as spine does NOT send CASE3 for PUTREQ_LARGEVALUE, NOT access need_recirculate_tbl, recirculate_tbl, and snapshot_flag_tbl
					* For PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED from leaf during single pipeline mode, forward to leaf by partition_tbl, convert it as PUTREQ_LARGEVALUE_SEQ/_CASE3_BEINGEVICTED_SPINE in ig_port_forward_tbl based on inswitchhdr.is_snapshot (NO valheader, NOT need to update pktlen) (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configure/table_configure.py)
						- Access need_recirculate_tbl, recirculate_tbl, snapshot_flag_tbl (files: tofino-spine/ptf_snapshot/table_configure.py, tofino-spine/configure/table_configure.py)
						- NOTE: For PUTREQ_LARGEVALUE_SEQ from leaf during single pipeline mode, spine MUST have a cache miss or a cache hit yet NOT being evicted -> NOT need to consider BEINGEVICTED. directly send PUT/DELREQ_SEQ_INSWITCH to leaf as usual
				+ In distfarreach.leaf
					* For PUTREQ_LARGEVALUE_SEQ, if cached=1 and valid=3, convert PUTREQ_LARGEVALUE_SEQ_INSWITCH to PUTREQ_LARGEVALUE_SEQ_/_CASE3_BEINGEVICTED, and update ip/mac (NOTE: NO valheader, NOT update pktlen) (files: tofino-leaf/p4src/egress_mat.p4, tofino-leaf/configure/table_configure.py)
					* For PUTREQ_LARGEVALUE_SEQ_BEINGEVICTED (NOTE: NO valheader, NOT update pktlen)
						- Access need_recirculate_tbl, recirculate_tbl, snapshot_flag_tbl (files: tofino-leaf/ptf_snapshot/table_configure.py, tofino-leaf/configure/table_configure.py)
						- If need_recirculate=0, forward it to server by partition_tbl, convert it as PUTREQ_LARGEVALUE_SEQ/_CASE3_BEINGEVICTED based on inswitchhdr.is_snapshot in ig_port_forward_tbl, and update ip/mac as client2server (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
						- If need_recirculate=1, forward it to the spine switch (files: tofino-leaf/configure/table_configure.py)
					* For PUTREQ_LARGEVALUE_SEQ_/_CASE3_BEINGEVICTED_SPINE (need_recirculate MUST = 0), forward them to storage server by partition_tbl, convert them as PUTREQ_LARGEVALUE_SEQ/_CASE3_BEINGEVICTED by ig_port_forward_tbl, and update ip/mac as client2server (NO valheader, NOT update pktlen) (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
				+ In farreach.switch
					* For PUTREQ_LARGEVALUE, if cached=1 and valid=3, convert PUTREQ_LARGEVALUE_INSWITCH to PUTREQ_LARGEVALUE_SEQ/_CASE3_BEINGEVICTED, update pktlen, and update ip/mac as client2server (NOTE: NO valheader) (files: tofino/p4src/egress_mat.p4, tofino/configure/table_configure.py)
		* In storage server -> SYNC to farreach/distfarreach
			- Data structure (files: server_impl.h, blockinfo.h)
				+ Use per-server mutex lock to guarantee the atomicity of read blocking
				+ Maintain per-server key blockedkey + bool isblocked + a blocklist of GETREQ_BEINGEVICTEDs and clientaddrs
			- For GETREQ_BEINGEVICTED (files: server_impl.h)
				+ If req.key=blockedkey (likely)
					* If isblocked=true (unlikely), add GETREQ_BEINGEVICTED and clientaddr into the blocklist
					* Otherwise, process it as usual without blocking
				+ If req.key!=blockedkey (unlikely)
					* If the blocklist is not empty (unlikely; propose a WARNING), answer the blocklist, and resize blocklist as 0 
					* Set blockedkey=req.key, set isblocked=true, add GETREQ_BEINGEVICTED and clientaddr into the blocklist
			- For PUT/DELREQ_SEQ/_CASE3_BEINGEVICTED, PUTREQ_LARGEVALUE_SEQ/_CASE3_BEINGEVICTED, or CACHE_EVICT
				+ Process it as usual
				+ If req.key=blockedkey
					* Set isblocked=false, answer the blocklist if not empty (unlikely), and resize blocklist as 0
				+ If req.key!=blockedkey
					* If the blocklist is not empty (unlikely; propose a WARNING), answer the blocklist, and resize blocklist as 0 
					* Set blockedkey=req.key, set isblocked=false
			- To answer the GETREQ_BEINGEVICTEDs in the blocklist, server.worker sends back responses to the clients
				+ NOTE: as read blocking is rare, it is OK to resort server.worker to clear the blocklist 
				+ TODOTODO: Notify server.blockserver to send back responses for the GETREQ_BEINGEVICTEDs in the blocklist to the clients
					* NOTE: we MUST reserve UDP ports for all blockservers in P4 parser, similar as valueupdateserver in netcache/distcache
	+ Compile and debug distfarreach
		+ Test distfarreach for GET/PUT/DELREQ and PUTREQ_LARGEVALUE of the key being evicted in spine
			* Pass set valid=3 manually; pass cache eviction
		+ Test distfarreach for GET/PUT/DELREQ and PUTREQ_LARGEVALUE of the key being evicted in leaf
			* Pass set valid=3 manually; pass cache eviction
		+ Test distfarreach for GET/PUT/DELREQ and PUTREQ_LARGEVALUE of the key being evicted in spine with snapshot
		+ Test distfarreach for GET/PUT/DELREQ and PUTREQ_LARGEVALUE of the key being evicted in leaf with snapshot
			* Issue: one duplicate XXX_BEINGEVICTED due to client-side timeout
				- Reason: server's NIC returns an ICMP w/ unreachable destination, which triggers a timeout error of client's UDP socket 
				- Reason: UDP bug??? -> observation: always the first request with a timeout quickly (not achieve timeout threshold)
		+ Test distfarreach for GET/PUT/DELREQ and PUTREQ_LARGEVALUE of the key being evicted in spine with snapshot=true and single path
		+ Test distfarreach for GET/PUT/DELREQ and PUTREQ_LARGEVALUE of the key being evicted in leaf with snapshot=true and single path
		+ Test distfarreach for GET/PUT/DELREQ and PUTREQ_LARGEVALUE of the key being evicted in spine with snapshot=false and single path
		+ Test distfarreach for GET/PUT/DELREQ and PUTREQ_LARGEVALUE of the key being evicted in leaf with snapshot=false and single path
		+ Test distfarreach under dynamic mixed workload w/ 50% write
			* NOTE: as we have sufficient cache size for distfarreach, we do NOT have XXX_BEINGEVICTED and hence no read-blocking
			* Issue: very low performance
				- Reason: set /proc/sys/vm/drop_caches as 3, which cannot recover automatically
				- [DEPRECATED] Solution: set /proc/sys/vm/drop_caches back to 4 after setting it as 3 to clear caches
					- NOTE: set /proc/sys/vm/drop_caches back to 0 will trigger an IO error
				- Solution: NOT set drop_caches as 3 in scripts!
	+ Compile and debug farreach (range -> hash)
		+ Test farreach for GET/PUT/DEQREQ and PUTREQ_LARGEVALUE of the key being evicted
			* Pass set valid=3 manually; pass cache eviction
		+ Test farreach for GET/PUT/DEQREQ and PUTREQ_LARGEVALUE of the key being evicted with snapshot
		+ Test farreach for GET/PUT/DEQREQ and PUTREQ_LARGEVALUE of the key being evicted with snapshot as true and single path
		+ Test farreach for GET/PUT/DEQREQ and PUTREQ_LARGEVALUE of the key being evicted with snapshot as false and single path
		+ Test farreach under dynamic mixed workload w/ 50% write

### hardware/software-link-based recirculation for single pipeline mode

- Single pipeline mode for atomic snapshot flag setting
	+ Update configuration under farreach and distfarreach -> SYNC to ALL (files: config.ini, configs/\*)
	+ Solution: hardware link
		* Alternative solution: software link simulated by another client corresponding to the specific pipeline
		* [IMPORTANT NOTE] farreach/distfarreach match srcip to set inswitchdr.clientsid for PUT/DELREQ and match key to set eport for GETRES_LATEST/DELETED_SEQ, which does NOT rely on ingress port
			- NOTE: even if we use software link, as long as the srcip/srcport is the correct one of the client (set by raw socket), the clientsid will be set correctly by the ingress pipeline -> the ip/mac/dstport of the cloned response will be correctly set by the egress pipeline based on the eport
	+ Implement single pipeline mode in FarReach by hardware link
		* Add configuration for recirport of 2nd pipeline (files: config.ini, configs/*, remotescript/test_server_rotation.sh, tofino.common.py)
		* For all requests and special responses in 2nd pipeline, forward to the 1st pipeline and bypass egress (files: tofino/p4src/ingress_mat.p4, tofino/p4src/header.p4, tofino/ptf_snapshotserver/table_configure.py, tofino/configure/table_configure.py)
			- NOTE: we MUST bypass egress explicitly to avoid converting GETRES_LATEST/DELETED_SEQ as GETRES, and removing valheader of PUTREQ, GETRES_LATEST/DELETED_SEQ/_SERVER
			+ TODOTODO: If bypass egress NOT work, we should convert GETRES_LATEST/DELETED_SEQ as GETRES_LATEST/DELETED_SEQ_RECIR
				* NOTE: farreach.egress will NOT convert PUT/DELREQ and PUTREQ_LARGEVALUE, BUT convert GETRES_LATEST/DELETED_SEQ as GETRES
				* NOTE: spine.egress will NOT convert PUT/DELREQ, PUTREQ_LARGEVALUE, PUT/DELREQ_SEQ, PUTREQ_LARGEVALUE_SEQ, and GETRES_LATEST/DELETED_SEQ_SERVER, BUT convert GETRES_LATEST/DELETED_SEQ as GETRES
			+ TODOTODO: If bypass egress NOT work, we should add valheader for PUTREQ and GETRES_LATEST/DELETED_SEQ_RECIR
				* NOTE: update_pktlen_tbl will remove all valheader by default
	+ Compile and debug FarReach
		* Dynamic workload without snapshot
		* snapshot = true
			* Cache hit of GET/PUT/DELREQ and PUTREQ_LARGEVALUE under single pipeline mode
			* Cache hit of GET/PUT/DELREQ and PUTREQ_LARGEVALUE with being evicted under single pipeline mode
			* Cache miss of GET/PUT/DELREQ under single pipeline mode
			* GETRES_LATEST/_DELETED_SEQ under single pipeline mode
		* snapshot = false
			* Cache hit of GET/PUT/DELREQ and PUTREQ_LARGEVALUE under single pipeline mode
			* Cache hit of GET/PUT/DELREQ and PUTREQ_LARGEVALUE with being evicted under single pipeline mode
			* Cache miss of GET/PUT/DELREQ under single pipeline mode
			* GETRES_LATEST/_DELETED_SEQ under single pipeline mode
		* Dynamic workload with snapshot
	+ Add comment for single pipeline mode under distfarreach

## Implementation tricks under Tofino limitation

- TODOTODO: Remove seq/savedseq for farreach/distfarreach/netcache/distcache
- TODOTODO: Introduce serverstatus for read blocking
	+ From design, the in-switch cache can judge whether the server-side record is latest or stale
		* Update serverstatus
			- CACHEPOP_INSWITCH sets serverstatus=1 (aka latest) after cache admission
			- PUT/DELREQ sets serverstatus=0 (aka stale) after write requests if cached=1 and valid=1
		* Read serverstatus
			- LOADDATA_INSWITCH reads serverstatus for victim report and snapshot
				+ NOTE: only if serverstatus=0, controller sends the victim report
					* If no PUT/DELREQ of the victim arrives at the server, server updates server-side record with the victim report
					* NOTE: not need deleted set now
				+ NOTE: controller only sends the snapshot records with serverstatus=0 to server
					* NOTE: for special case1 and case2 of the same key, we must order them based on time instead of sequence
					* For range query, in-switch snapshot results directly overwrite server-side snapshot results for the same keys
			- GETREQ reads serverstatus if cached=1 and valid=3
		* Only if cached=1, valid=3 (aka being evicted), latest=0 (aka out-of-date), and serverstatus=0 (aka stale), GETREQ trigggers read blocking
	+ However, due to Tofino limitation, we do not have sufficient hardware resources to support serverstatus
		* TRICK
			- Only if valid=3 (aka being evicted) and latest=0 (aka out-of-date), GETREQ trigggers read blocking
			- Controller always sends victim report
				+ Compare seq to determine whether we should replace the server-side record with the victim report
			- Controller sends all snapshot records to server
				+ Compare seq to determine whether we should use in-switch snapshot results or server-side snapshot results for the same keys under range query
		* -> still limited read blocking!
			- The in-switch record is more likely latest=1 due to lightweight cache update
				+ So only if the PUTREQ sets latest=0 yet with a packet loss, then the GETREQ will be blocked
			- If the in-switch record is latest=0 -> the key must be a cold key
				+ Very limited # of GETREQs during cache eviction
			- Limited time of read blocking: stop after a write request or a victim report
		* -> still correct with limited effect of performance
			- Bandwidth cost of victim report is limited, and processing victim report is much less frequent than normal requests
			- Bandwidth cost of snapshot is limited, and comparing seq in memory for range query does not have large overhead
- Schedule: implement YCSB first, and then change the implementation if necessary

## About open issues

- Adaptiveness
	+ [DEPRECATED]: Tune CM threshold for distcache and distfarreach due to reducing one CM register array to fix Tofino resource limitation
		* NOTE: we can use CM theoretical guarantee on error bound to argue that why we increase CM threshold if decrease CM hashnum by 1
		* NOTE: we periodically reset CM such that N/w is close to one, set a proper phi for single switch scenario, and then phi' = (phi)^(4/3)*(W/N)^(3/4) for the same error probability under distributed scenario
			- NOTE: although we round phi to set CM threshold for single switch, we should use the original phi to calcualte phi' to keep the same error probability before rounding phi'
			- Example of N/W=1: for sigma=(1/[100/150/200])^4, d=4, threshold=100/150/200 -> d=3, threshold=100^(4/3)=500/800/1200
			- Example of N/W=1.25 (W=64K, N=80KOPS for requests accessing storage servers): for sigma=(1/124)^4, d=4, threshold=125 -> d=3, threshold=125^(4/3)*(1.25)^(3/4)=800 -> we set threshold = 100 and 1000 after rounding
	+ Soft reset CM (reset to a threshold instead of to zero)
		* NOTE: CM warmup after reset is not an issue under our scenario
