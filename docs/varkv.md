# Variable-length key-value for all six methods (nocache/netcache/farreach/distcache/distnocache/distfarreach)

## Implementation of large value part 1 (forwarding rule + ip fragmentation)

- Implement >128B variable-length value (part 1)
	+ NOTE: ONLY PUTREQ_LARGEVALUE and GETRES_LARGEVALUE require frag_hdr (at the end of each packet)
	+ Under DistNoCache -> SYNC to ALL (8.22 - 8.26)
		* Add PUTREQ_LARGEVALUE, DISTNOCACHE_PUTREQ_LARGEVALUE_SPINE, GETRES_LARGEVALUE_SERVER, and GETRES_LARGEVALUE w/ op_hdr; PUTREQ_LARGEVALUE_SEQ w/ op_hdr, shadowtype_hdr, and seq_hdr; PUTREQ_LARGEVALUE_SEQ_INSWITCH w/ op_hdr, shadowtype_hdr, seq_hdr, and inswitch_hdr; PUTREQ_LARGEVALUE_INSWITCH w/ op_hdr, shadowtype_hdr, and inswitch_hdr (files: tofino-*/main.p4, tofino-*/common.py, packet_format.*, common_impl.h)
			- [IMPORTANT] NOTE: add uint16_t client_logical_idx at the end of PUTREQ_LARGEVALUE, PUTREQ_LARGEVALUE_SEQ, and LOADREQ for server.udprecvlarge_ipfrag (files: packet_format.*)
				+ TODOTODO: NOTE: we can also add client_logical_idx into op_hdr, such that server can double-check when adding pkt into PktRingBuffer
			- NOTE: we need to serialize/deserialize vallen and value yet NOT parsed by switch for all the above packet types
			- NOTE: we need to serialize/deserialize stat_hdr (stat MUST be true) yet NOT parsed by switch for GETRES_LARGEVALUE/_SERVER
			- NOTE: switch will parse frag_hdr (fragidx + fragnum) for put requests w/ large value after op_hdr/seq_hdr/inswitch_hdr
			- Implement static function get_frag_hdrsize() for PUTREQ_LARGEVALUE, PUTREQ_LARGEVALUE_SEQ, GETRES_LARGEVALUE_SERVER, and GETRES_LARGEVALUE (packet_format.*)
			- Reduce ipfrag maxfragsize by 64B (34B reserved for shadowtype_hdr, seq_hdr, and inswitch_hdr) (files: socket_helper.h)
			- Implement dynamic_serialize() for PUTREQ_LARGEVALUE and GETRES_LARGEVALUE_SERVER (aka GETRES_LARGEVALUE) (files: packet_format.*)
		* Use udpsendlarge_ipfrag in server ONLY for GETRES_LARGEVALUE_SERVER (while process PUTREQ_LARGEVALUE_SEQ as PUTREQ_SEQ), and client ONLY for PUTREQ_LARGEVALUE and LOADREQ (files: server_impl.h, remote_client.c)
			- Use ntohs and htons for fragidx and fragnum in socket helper for udpsend/recvlarge and udprecvlrage_multisrc (files: socket_helper.c)
			- ONLY parse op_hdr for LOADREQ in switch (LOADREQ: op_hdr + vallen_hdr&val_hdr)
				+ NOTE: We do NOT need to parse/deparse vallen/value for LOADREQ in switch
					* Change optype of LOADREQ and LOADREQ_SPINE (files: tofino-*/main.p4, tofino-*/common.py, packet_format.h)
					* NOT access add_and_remove_value_header_tbl for LOADREQ and LOADREQ_SPINE (files: tofino-*/configure/table_configure.py)
					* Remove shadowtype_hdr from LOADREQ (files: packet_format.*)
				+ NOTE: for key >16B, we should use NORMALREQ_LARGEKEY and NORMALRES_LARGEKEY w/ op_hdr + client_logical_idx + payload (specific optype + varkey + vallen + val)
					* NOTE: netreach_key_t is 16B for in-switch processing; varkey_t (netreach_key_t for the most significant 16B key; and keylen + keybytes for original key > 16B) is variable-length key for original key >16B
				+ Implement static function get_frag_hdrsize() and dynamic_serialize() for LOADREQ (files: packet_format.*)
		* client/client-leaf.PUTREQ_LARGEVALUE -> spine.DISTNOCACHE_PUTREQ_LARGEVALUE_SPINE -> server-leaf/server.PUTREQ_LARGEVALUE
			- client-leaf: hash_for_spineselect_tbl and spineselect_tbl for PUTREQ_LARGEVAULE (files: tofino-leaf/configure/table_configure.py)
			- spine: hash_for_partition_tbl, hash/range_partition_tbl and ig_port_forward_tbl for PUTREQ_LARGEVALUE -> DISTNOCACHE_PUTREQ_LARGEVALUE_SPINE (files: tofino-spine/p4src/ingress_mat.p4, tofino-spine/configure/table_configure.py)
			- server-leaf: hash/range_partition_tbl and ig_port_forward_tbl for DISTNOCACHE_PUTREQ_LARGEVALUE_SPINE -> PUTREQ_LARGEVALUE -> access update_ipmac_srcport_tbl as client2server (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
			- single switch: hash/range_partition_tbl and update_ipmac_srcport_tbl as client2server for PUTREQ_LARGEVALUE from client to server (files: tofino/configure/table_configure.py)
		* server/server-leaf.GETRES_LARGEVALUE_SERVER -> spine.GETRES_LARGEVALUE -> client-leaf/client.GEGRES_LARGEVALUE
			- NOTE: Use GETRES_LARGEVALUE/_SERVER to avoid in-switch parser/deparser (NO shadowtype; >128B vallen) and add_and_remove_value_header_tbl (>128B vallen -> default remove_all() action) -> only parse/deparse op_hdr for ipv4_forward_tbl is OK
			- server-leaf: ipv4_forward_tbl and ig_port_forward_tbl for GETRES_LARGEVALUE_SERVER -> GETRES_LARGEVALUE (files: tofino-leaf/p4src/ingress_mat.p4, tofino-leaf/configure/table_configure.py)
			- spine: ipv4_forward_tbl for GETRES_LARGEVALUE (files: tofino-spine/configure/table_configure.py)
			- client-leaf: ipv4_forward_tbl and update_ipmac_srcport_tbl as server2client for GETRES_LARGEVALUE (files: tofino-leaf/configure/table_configure.py)
			- single switch: ipv4_forward_tbl and update_ipmac_srcport_tbl as server2client for GETRES_LARGEVALUE (files: tofino/configure/table_configure.py)
				+ NOTE: server directly sends back GETRES_LARGEVALUE instead of GETRES_LARGEVALUE_SERVER in single switch (files: server_impl.h)
		* In udprecvlarge of socket helper, parse optype after the first time of udpsock.recvform
			- Main optype_for_udprecvlarge_ipfrag_list for udprecvlarge_ipfrag (files: packet_format.h)
				+ NOTE: for SNAPSHOT_GETDATA_ACK and SNAPSHOT_SENDDATA using udprecvlarge_udpfrag, controller/server already know that they will receive a large packet -> NOT need to jump out of udprecvlarge based on packet type
					* TODOTODO: if needed in the future, we can add SNAPSHOT_GETDATA_ACK and SNAPSHOT_SENDDATA into packet_format.* and optype_for_udprecvlarge_list
				+ NOTE: for SCANRES_SPLITs using udprecvlarge_ipfrag_multisrc, client already knows that it will receive multiple large packets -> NOT need to jump out of udprecvlarge_multisrc based on packet type
				+ [IMPORTANT] NOTE: for PUTREQ_LARGEVALUE, PUTREQ_LARGEVALUE_SEQ, GETRES_LARGEVALUE, and LOADREQ using udprecvlarge_ipfrag, server/client does NOT know whether the received packet is large or not -> NEED to jump out of udprecvlarge based on packet type
			- If optype is in optype_for_udprecvlarge_ipfrag_list, perform udprecvlarge_ipfrag as usual (files: socket_helper.*)
				+ For optype in optype_for_udprecvlarge_ipfrag_list, decide fraghdrsize based on optype (files: socket_helper.*, packet_format.*)
				+ NOTE: for UDP_FRAGTYPE, fraghdrsize MUST be 0
			- Otherwise, directly return as in udprecvfrom due to NO frag_hdr (files: socket_helper.*)
			- NOTE: server.worker NEEDs to use dynamic array as recvbuf + udprecvlarge_ipfrag for all optypes to cope with potential large value; while client ONLY NEEDs dynamic array + udprecvlarge_ipfrag for GETREQ (files: server_impl.h, remote_client.c)
		* As multiple clients can send requests to the same server simultaneously, the server could receive other requests when receiving a request with large value
			- Implement PktRingBuffer (for single thread or sequential), each entry has valid flag, optype, key, dynamic array, cur_fragnum, max_fragnum, client.addr&addrlen, and clientlogicalidx (to erase large packet from clientlogicalidx_bufidx_map) -> SYNC to ALL after compilation success
				+ Maintain a clientlogicalidx_bufidx_map<key: uint16_t cilent_logical_idx, value: uint32_t buffer_idx> to locate large packet in PktRingtBuffer
				+ NOTE: we pop pkt from PktRingBuffer in FIFO order instead of map order
				+ NOTE: ONLY used by server for PUTREQ_LARGEVALUE, PUTREQ_LARGEVALUE_SEQ, and LOADREQ; client does NOT need PktRingBuffer as it receives at most one response from server(s) once a time
			- Update packet_format util APIs for udprecvlarge_ipfrag (files: packet_format.*)
				+ Add get_packet_key(buf, bufsize) to get op_hdr.key
				+ Maintain optype_with_clientlogicalidx_list; add get_packet_clientlogicalidx(buf, bufsize) to get client_logical_idx before fraghdr(fragidx&fragnum) for PUTREQ_LARGEVALUE/_SEQ and LOADREQ
				+ Add is_packet_with_largevalue(optype) and is_packet_with_clientlogicalidx(optype) (files: packet_format.*)
			- In udprecvlarge_ipfrag -> NOTE: ONLY for IP_FRAGTYPE in udprecvlarge 
				+ If w/o PktRingBuffer (aka client)
					* If isfirst, directly copy tmpaddr&tmpaddrlen to client.addr&addrlen if NOT null (for both large and non-large packet) -> save key and optype for large packet
					* Otherwise (the first pkt MUST belong to a large packet), filter out all pkts not matching key and optype
				+ If w/ PktRingBuffer (aka server)
					* If PktRingBuffer is NOT empty
						- If the first pkt is a small pkt, pop it from PktRingBuffer and return
						- If the first pkt is a large pkt, pop it from PktRingBuffer, simulate process of is_first=true, and receive the remaining fragments
					* Otherwise (empty PktRingBuffer)
						- If isfirst, directly copy tmpaddr&tmpaddrlen to client.addr&addrlen if NOT null (for both large and non-large packet) -> save key, optype, and client_logical_idx for large packet
						- Otherwise (the first pkt MUST belong to a large packet)
							- If current packet is a not-large packet/request, directly add it into PktRingBuffer
							- Otherwise (current pkt belongs to a large packet), get key, optype, and client_logical_idx
								+ If w/ different client_logical_idx, update PktRingBuffer
									- If client_logical_idx already exists, the entry MUST have valid=true (aka valid entry/pkt), max_fragnum>0 (aka large pkt)
										+ If optype and key matches, update body of dynamicbuf and cur_fragnum in PktRingBuffer
										+ Otherwise, ignore the current packet
									- Otherwise (no such client_logical_idx before), use a new entry for the current packet, set valid = true, update cur_fragnum and max_fragnum, update fraghdr and body of dynamicbuf in PktRingBuffer
								+ Otherwise (w/ the same client_logical_idx), filter out all pkts not matching key and optype
			- Use PktRingBuffer in server.worker (files: server_impl.h)
		* Use uint32_t as vallen for PUTREQ_LARGEVALUE/_SEQ, LOADREQ, GETRES_LARGEVALUE, SCANRES_SPLIT (files: val.*, packet_format_impl.h)
			- NOTE: Use Val::serialize/deserialize_large for the above packets; use Val::serialize/deserialize for other packets
			- NOTE: In Val::serialize/deserialize, we MUST convert uint32_t vallen as uint16_t vallen for in-switch processing
		* Compile DistNoCache under range partition, and update visualization files
		* Test normal request, and putreq / getres / range query with large value (e.g., 2KB and 200KB)
		* Test PUTREQ_LARGEVALUEs from multiple clients to the same server
			- Issue: timeout issue of large write requests from multiple clients
				+ Use blocking socket -> in the first 7 seconds, client-side thpt is 0 MOPS due to delayed response from server
				+ Dump server-side detailed info -> one client dominates server.worker.udpsock as other clients timeout
				+ Dump filtered packets in socket helper to see whether we drop some correct packets which should not be dropped -> NO unmatched packets
				+ Reason: udprecvlarge return true (is_timeout) for complete packets popped from PktRingBuffer -> server.worker does NOT send response as is_timeout = true
				+ Solution: udprecvlarge should return false (is_timeout) for complete packets popped from PktRingBuffer
			- [IMPORTANT] Code change: op_hdr + [shadowtype_hdr + seq_hdr + inswitch_hdr] + in-switch frag_hdr (clientlogicalidx + fragseq + fragidx&fragnum) + payload -> NOTE: server-side frag_hdr includes clientlogicalidx and fragseq yet W/O fragidx&fragnum -> SYNC to ALL
				+ Implement fraghdr memcpy for IP_FRAGTYPE and fragidx = 0 (files: socket_helper.c, pkt_ring_buffer.*)
				+ Add fragseq between clientlogicalidx and fragidx&fragnum for udprecvlarge_ipfrag to avoid duplicate large packet caused by packet loss (files: remote_client.c, packet_format.*, socket_helper.c, pkt_ring_buffer.*)
					* NOTE: ONLY for packets w/ clientlogicalidx (aka client to server), as NO duplicate responses for server to client
					* NOTE: clientlogicalidx and fragseq are NOT accessed by switch (placed in T-PHV)
				+ Process real packet loss in server.worker (files: socket_helper.*, server_impl.h)
					* TODOTODO: (1) if unmatchedcnt >= SERVER_UNMATCHEDCNT_THRESHOLD, push current large packet back into pktringbuffer, and return true (is_timeout) -> NOTE: NOT use now
						- NOTE: it ONLY works with sufficient clients to provide sufficient unmatched packets -> we need condition (2)
					* (2) set a small timeout threshold for server.worker.udpsock; if server.worker.udpsock timeouts, push current large packet back into pktringbuffer, and return true (is_timeout)
					* NOTE: if timeout = true, server.worker will re-call udprecvlarge_ipfrag(), which will pop another packet from pktringbuffer -> the large packet w/ pktloss will be resent by client w/ a larger fragseq, which will reset curfragnum in pktringbuffer as 1
		* Test PUTREQ_LARGEVALUEs and PUTREQs from multiple clients to the same server
			- Issue: timeout issue of large write request from multiple clients
				+ Reason: default clientlogicalidx in pktringbuffer is 0, which is not correctly set by small packet -> each time we pop a small packet, we will remove clientlogicalidx from pktringbuffer::clientlogicalidx_bufidx_map -> if we have already pushed a large packet of client 0 into pktringbuffer, then the next fragment will invoke pktringbuffer::push_large to set pktringbuffer::curfragnum as 1 instead of increase it by pktringbuffer:update_large -> server waits for non-existing fragments from client 0, and hence all clients timeout
				+ Solution: we erase clientlogicalidx from pktringbuffer::clientlogicalidx_bufidx_map ONLY if the popped packet is a large packet (i.e., maxfragnum > 0)
			- Issue: timeout issue of large write request pushed back into pktringbuffer
				+ Reason: we should set correct cur_fragnum instead of 0
				+ Solution: pass correct cur_fragnum for pushed-back large write request
			- Issue: timeout issue under 1024 clients and 8 servers
				+ Reason 1: server-leaf does NOT set dstip correctly
				+ Solution 1: update ptf configuration to set ip/mac correctly for PUTREQ_LARGEVALUE and GETRES_LARGEVALUE
				+ Reason 2: two physical clients use the same local client logicalidx in PUTREQ_LARGEVALUE -> unmatched key in PktRingBuffer::update_large
				+ Solution 2: use global client logicalidx in PUTREQ_LARGEVALUE and LOADREQ
		* Test PUTREQs from multiple clients to the same server
		* Compile NoCache under range partition, test normal requests and putreq/getres/scan w/ large value, test PUTREQs&PUTREQ_LARGEVALUEs under dynamic workload, and update visualization files
		* Re-compile DistNoCache and NoCache under hash partition, test normal requests and putreq/getres w/ large value, test PUTREQs&PUTREQ_LARGEVALUEs under dynamic workload, and update visualization files

## Implementatino of large value part 2 (in-switch invalidation)

## Deprecated staff for large value

* DEPRECATED: for variable-length value, PUTREQ_LARGEVALUE invalidates in-switch value, and clones duplicate in-switch value to server for pktloss
	- NOTE: invalidate-and-clone is DEPRECATED now
		+ Reason 1: we may NOT have sufficient power budget to maintain such an invalidate-and-clone in-switch mechanism
		+ Reason 2: clone duplicate packets to send in-switch value to server significantly overloads storage server, which could make our write performance even worse than baselines
		+ Reason 3: pktloss of PUTREQ_LARGE between switch and server is just a rare case
	- TODO: NOTE: we only parse/deparse op_hdr for PUTREQ_LARGE instead of vallen/val_hdr
	- In ingress pipeline
		+ TODO: If is_cached=0, directly forward PUTREQ_LARGE to egress pipeline;
		+ TODO: Otherwise, convert original PUTREQ_LARGE as PUTREQ_LARGE_INSWITCH to egress pipeline (deprecated: clone_i2e to generate PUTREQ_LARGE to egress pipeline)
	- In egress pipeline
		+ TODO: PUTREQ_LARGE does NOT access any MATs
		+ TODO: For PUTREQ_LARGE_INSWITCH
			* TODO: Invalidate inswitch value by setting latest_reg = 0 (deprecated: setting with_largevalue_reg)
			* TODO: seq_tbl assigns seq for PUTREQ_LARGE_INSWITCH
			* TODO: eg_port_forward_tbl forwards and clones PUTREQ_LARGE_INSWITCH as PUTREQ_LARGE_SEQ_INSWITCH (including seq_hdr, inswitch_hdr, and clone_hdr) to server
				- NOTE: the first PUTREQ_LARGE_SEQ_INSWITCH carries larger seq assigned by seq_tbl
		+ TODO: For PUTREQ_LARGE_SEQ_INSWITCH (cloned by PUTREQ_LARGE_INSWITCH)
			- TODO: Get savedseq, deleted, vallen, and value from in-switch cache
			- TODO: eg_port_forward converts it as PUTREQ_LARGE_EVICTION (including val_hdr, seq_hdr, inswitch_hdr, stat_hdr, and clone_hdr), forward and clone to server by duplicate packets
			- TODO: If snapshot_flag=1, we need to use PUTREQ_LARGE_SEQ_INSWITCH_CASE3 and PUTREQ_LARGE_EVICTION_CASE3 to server
			- TODO: PUTREQ_LARGE_EVICTION/_CASE3 needs to check seq to avoid from overwritting large value carried by PUTREQ_LARGE_SEQ (only parse op_hdr + seq_hdr w/o val_hdr; large value is in payload)
				* TODO: In server, we can maintain an in-memory map for recently received SEQ of PUTREQ_LARGE_SEQ to speed up
		+ TODO: In prepare_for_putreq_large_tbl
			* TODO: For PUTREQ_LARGE_INSWITCH, we save server_sid and server_udpport into clone_hdr as prepare_for_cachepop_tbl in NetCache -> NOT reset clone_hdr.server_sid in process_scanreq_split_tbl; be careful about pkts using clone_hdr.server_sid
			* TODO: For PUTREQ_LRAGE_SEQ_INSWITCH and PUTREQ_LARGE_EVICTION, explicitly invoke nop()
		+ TODO: Change for other packet types
			* TODO: Convert GETREQ as GETRES only if is_cached=1, validvalue=1, is_latest=1 and with_largevalue=0
			* TODO: PUT/DELREQ directly sets is_latest=1 and with_largevalue=0
