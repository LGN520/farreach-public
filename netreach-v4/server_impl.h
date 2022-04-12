#ifndef SERVER_IMPL_H
#define SERVER_IMPL_H

// Per-server popclient <-> one popserver in controller
int * volatile server_popclient_tcpsock_list = NULL;
std::set<index_key_t> * volatile server_cached_keyset_list = NULL;

// Per-server evictserver <-> one evictclient in controller
int * volatile server_evictserver_tcpsock_list = NULL;
cache_evict_t ** volatile cache_evicts_list = NULL;
uint32_t volatile heads_for_evict;
uint32_t volatile tails_for_evict;

void prepare_server();
void close_server();

// server.workers for processing pkts
static int run_sfg(void *param);
//void *run_sfg(void *param);

void prepare_server() {
	// Prepare for cache population
	server_popclient_tcpsock_list = new int[fg_n];
	server_cached_keyset_list = new std::set<index_key_t>[fg_n];
	for (size_t i = 0; i < fg_n; i++) {
		create_tcpsock(server_popclient_tcpsock_list[i], "server.popclient");

		server_cached_keyset_list[i].clear();
	}
}

void close_server() {
	if (server_popclient_tcpsock_list != NULL) {
		delete [] server_popclient_tcpsock_list;
		server_popclient_tcpsock_list = NULL;
	}
	if (server_cached_keyset_list != NULL) {
		delete [] server_cached_keyset_list;
		server_cached_keyset_list = NULL;
	}
}

/*
 * Worker for server-side processing 
 */

static int run_sfg(void * param) {
//void *run_sfg(void * param) {
  // Parse param
  sfg_param_t &thread_param = *(sfg_param_t *)param;
  uint8_t thread_id = thread_param.thread_id;
  xindex_t *table = thread_param.table;

  tcpconnect(server_popclient_tcpsock_list[thread_id], controller_ip, controller_popserver_port, "server.popclient", "controller.popserver"); // enforce the packet to go through NIC 
  //tcpconnect(server_popclient_tcpsock_list[thread_id], controller_ip, controller_popserver_port_start + thread_id, "server.popclient", "controller.popserver"); // enforce the packet to go through NIC 

  int res = 0;

  // DPDK
  //struct rte_mbuf *sent_pkt = rte_pktmbuf_alloc(mbuf_pool); // Send to DPDK port
  //struct rte_mbuf *sent_pkt_wrapper[1] = {sent_pkt};
  // Optimize mbuf allocation
  uint16_t burst_size = 256;
  struct rte_mbuf *sent_pkts[burst_size];
  uint16_t sent_pkt_idx = 0;
  struct rte_mbuf *sent_pkt = NULL;
  res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
  INVARIANT(res == 0);

  // prepare socket
  /*int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  INVARIANT(sockfd >= 0);
  int disable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
	perror("Disable udp checksum failed");
  }
  short dst_port = server_port_start + thread_id;
  struct sockaddr_in server_sockaddr;
  memset(&server_sockaddr, 0, sizeof(struct sockaddr_in));
  server_sockaddr.sin_family = AF_INET;
  server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_sockaddr.sin_port = htons(dst_port);
  res = bind(sockfd, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr));
  INVARIANT(res != -1);
  uint32_t sockaddr_len = sizeof(struct sockaddr);

  // Set timeout
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec =  0;
  res = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  INVARIANT(res >= 0);*/

  char buf[MAX_BUFSIZE];
  int recv_size = 0;
  int rsp_size = 0;

  // DPDK
  uint8_t srcmac[6];
  uint8_t dstmac[6];
  char srcip[16];
  char dstip[16];
  uint16_t srcport;
  uint16_t unused_dstport; // we use server_port_start instead of received dstport to hide server-side partition for client

  ready_threads++;

  while (!running) {
  }

  while (running) {
	/*recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, (struct sockaddr *)&server_sockaddr, &sockaddr_len);
	if (recv_size == -1) {
		if (errno == EWOULDBLOCK || errno == EINTR) {
			continue; // timeout or interrupted system call
		}
		else {
			COUT_THIS("[server] Error of recvfrom: errno = " << errno)
			exit(-1);
		}
	}*/

#ifdef TEST_AGG_THPT
	struct timespec t1, t2, t3;
	CUR_TIME(t1);
#endif
	//if (stats[thread_id]) {
	if (heads[thread_id] != tails[thread_id]) {
		//COUT_THIS("[server] Receive packet!")

		// DPDK
		//stats[thread_id] = false;
		//recv_size = decode_mbuf(pkts[thread_id], srcmac, dstmac, srcip, dstip, &srcport, &dstport, buf);
		//rte_pktmbuf_free((struct rte_mbuf*)pkts[thread_id]);
		sent_pkt = sent_pkts[sent_pkt_idx];

		INVARIANT(pkts_list[thread_id][tails[thread_id]] != nullptr);
		memset(srcip, '\0', 16);
		memset(dstip, '\0', 16);
		memset(srcmac, 0, 6);
		memset(dstmac, 0, 6);
		recv_size = decode_mbuf(pkts_list[thread_id][tails[thread_id]], srcmac, dstmac, srcip, dstip, &srcport, &unused_dstport, buf);
		rte_pktmbuf_free((struct rte_mbuf*)pkts_list[thread_id][tails[thread_id]]);
		tails[thread_id] = (tails[thread_id] + 1) % MQ_SIZE;

		/*if ((debug_idx + 1) % 10001 == 0) {
			COUT_VAR((t0 - prevt0) / 10000.0);
			prevt0 = t0;
		}*/

		packet_type_t pkt_type = get_packet_type(buf, recv_size);
		switch (pkt_type) {
			case packet_type_t::GETREQ: 
				{
					get_request_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					val_t tmp_val;
					int32_t tmp_seq = 0;
					bool tmp_stat = table->get(req.key(), tmp_val, thread_id, tmp_seq);
					//COUT_THIS("[server] val = " << tmp_val.to_string())
					get_response_t rsp(req.key(), tmp_val, tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::GETREQ_NLATEST:
				{
					get_request_nlatest_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					val_t tmp_val;
					int32_t tmp_seq = 0;
					bool tmp_stat = table->get(req.key(), tmp_val, thread_id, tmp_seq);
					//COUT_THIS("[server] val = " << tmp_val.to_string())
					if (tmp_stat) { // key exists
						get_response_latest_seq_t rsp(req.key(), tmp_val, tmp_seq);
						rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					}
					else { // key not exist
						get_response_deleted_seq_t rsp(req.key(), tmp_val, tmp_seq);
						rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					}
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::PUT_REQ_SEQ:
				{
					put_request_seq_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool tmp_stat = table->put(req.key(), req.val(), thread_id, req.seq());
					//COUT_THIS("[server] stat = " << tmp_stat)
					put_response_t rsp(req.hashidx(), req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::DEL_REQ_SEQ:
				{
					del_request_seq_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					bool tmp_stat = table->remove(req.key(), thread_id, req.seq());
					//COUT_THIS("[server] stat = " << tmp_stat)
					del_response_t rsp(req.hashidx(), req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;

					// NOTE: no matter tmp_stat is true (key is deleted) or false (no such key or key has been deleted before), we should always treat the key does not exist (i.e., being deleted), so a reordered eviction will never overwrite this result for linearizability -> we should always update the corresponding deleted set
					deleted_sets[thread_id].add(req.key(), req.seq());
					break;
				}
			case packet_type_t::SCAN_REQ:
				{
					scan_request_t req(buf, recv_size);
					//COUT_THIS("[server] startkey = " << req.key().to_string() << 
					//		<< "endkey = " << req.endkey().to_string() << " num = " << req.num())
					std::vector<std::pair<index_key_t, val_t>> results;
					//size_t tmp_num = table->scan(req.key(), req.num(), results, thread_id);
					size_t tmp_num = table->range_scan(req.key(), req.endkey(), results, thread_id);

					// Add kv pairs of backup data into results
					std::map<index_key_t, val_t> *kvdata = &backup_data->_kvmap;
					if (kvdata != nullptr) {
						std::map<index_key_t, val_t>::iterator kviter = kvdata->lower_bound(req.key());
						for (; kviter != kvdata->end() && kviter->first < req.endkey(); kviter++) {
							results.push_back(*kviter);
						}
					}
					//COUT_THIS("SCAN results size: " << results.size())

					scan_response_t rsp(req.hashidx(), req.key(), req.endkey(), results.size(), results);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::GETREQ_POP: 
				{
					get_request_pop_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					val_t tmp_val;
					int32_t tmp_seq;
					bool tmp_stat = table->get(req.key(), tmp_val, thread_id, tmp_seq);
					//COUT_THIS("[server] val = " << tmp_val.to_string())
					
					get_response_t rsp(req.key(), tmp_val, tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;

					// Trigger cache population if necessary (key exist and not being cached)
					if (tmp_stat) {
						bool is_cached_before = (server_cached_keyset_list[thread_id].find(req.key()) != server_cached_keyset_list.end());
						if (!is_cached_before) {
							server_cached_keyset_list[thread_id].insert(req.key());
							// Send CACHE_POP to controller.popserver
							cache_pop_t cache_pop_req(req.key(), tmp_val, tmp_seq, int16_t(thread_id));
							uint32_t popsize = cache_pop_req.serialize(buf, MAX_BUFSIZE);
							tcpsend(server_popclient_tcpsock_list[thread_id], buf, popsize, "server.popclient");
						}
					}
					break;
				}
			case packet_type_t::GET_RES_POP_EVICT:
				{
					// Put evicted data into key-value store
					get_response_pop_evict_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_POP_EVICT:
				{
					// Put evicted data into key-value store
					put_request_pop_evict_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_SEQ:
				{
					put_request_large_seq_t req(buf, recv_size);
					INVARIANT(req.val().val_length > val_t::SWITCH_MAX_VALLEN);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool tmp_stat = table->put(req.key(), req.val(), thread_id, req.seq());
					//COUT_THIS("[server] stat = " << tmp_stat)
					put_response_t rsp(req.hashidx(), req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_EVICT:
				{
					// Put evicted data into key-value store
					put_request_large_evict_t req(buf, recv_size);
					INVARIANT(req.val().val_length <= val_t::SWITCH_MAX_VALLEN);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::GET_RES_POP_EVICT_CASE2:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					get_response_pop_evict_case2_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_POP_EVICT_CASE2:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					put_request_pop_evict_case2_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_EVICT_CASE2:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					put_request_large_evict_case2_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					INVARIANT(req.val().val_length <= val_t.SWITCH_MAX_VALLEN);
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_CASE3:
				{
					COUT_THIS("PUT_REQ_CASE3")
					put_request_case3_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool tmp_stat = table->put(req.key(), req.val(), thread_id, req.seq());
					put_response_case3_t rsp(req.hashidx(), req.key(), thread_id, tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::DEL_REQ_CASE3:
				{
					COUT_THIS("DEL_REQ_CASE3")
					del_request_case3_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool tmp_stat = table->remove(req.key(), thread_id, req.seq());
					del_response_case3_t rsp(req.hashidx(), req.key(), thread_id, tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;

					// NOTE: no matter tmp_stat is true (key is deleted) or false (no such key or key has been deleted before), we should always treat the key does not exist (i.e., being deleted), so a reordered eviction will never overwrite this result for linearizability -> we should always update the corresponding deleted set
					deleted_sets[thread_id].add(req.key(), req.seq());
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_CASE3:
				{
					COUT_THIS("PUT_REQ_LARGE_CASE3")
					put_request_large_case3_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool tmp_stat = table->put(req.key(), req.val(), thread_id, req.seq());
					put_response_case3_t rsp(req.hashidx(), req.key(), thread_id, tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, thread_id, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			default:
				{
					COUT_THIS("[server] Invalid packet type from receiver: " << int(pkt_type))
					std::cout << std::flush;
					exit(-1);
				}
		}
#ifdef TEST_AGG_THPT
		CUR_TIME(t2);
		DELTA_TIME(t2, t1, t3);
		thread_param.sum_latency += GET_MICROSECOND(t3);
#endif
		backup_rcu[thread_id]++;
		thread_param.throughput++;

		if (sent_pkt_idx >= burst_size) {
			sent_pkt_idx = 0;
			res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
			INVARIANT(res == 0);
		}
	} // End for mbuf from receiver to server
	if (heads_for_pktloss[thread_id] != tails_for_pktloss[thread_id]) {
		INVARIANT(pkts_list_for_pktloss[thread_id][tails_for_pktloss[thread_id]] != nullptr);
		recv_size = get_payload(pkts_list_for_pktloss[thread_id][tails_for_pktloss[thread_id]], buf);
		rte_pktmbuf_free((struct rte_mbuf*)pkts_list_for_pktloss[thread_id][tails_for_pktloss[thread_id]]);
		tails_for_pktloss[thread_id] = (tails_for_pktloss[thread_id] + 1) % MQ_SIZE;

		packet_type_t pkt_type = get_packet_type(buf, recv_size);
		switch (pkt_type) {
			// NOTE: we use seq mechanism to avoid incorrect overwrite of packet reordering
			case packet_type_t::GET_RES_POP_EVICT_SWITCH:
				{
					// Put evicted data into key-value store
					get_response_pop_evict_switch_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_POP_EVICT_SWITCH:
				{
					// Put evicted data into key-value store
					put_request_pop_evict_switch_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_EVICT_SWITCH:
				{
					// Put evicted data into key-value store
					put_request_large_evict_switch_t req(buf, recv_size);
					INVARIANT(req.val().val_length <= val_t::SWITCH_MAX_VALLEN);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::GET_RES_POP_EVICT_CASE2_SWITCH:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					get_response_pop_evict_case2_switch_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_POP_EVICT_CASE2_SWITCH:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					put_request_pop_evict_case2_switch_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_EVICT_CASE2_SWITCH:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					put_request_large_evict_case2_switch_t req(buf, recv_size);

					if (!isbackup) {
						try_kvsnapshot(table);
					}

					INVARIANT(req.val().val_length <= val_t.SWITCH_MAX_VALLEN);
					bool isdeleted = deleted_sets[thread_id].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), thread_id, req.seq());
						}
						else {
							table->remove(req.key(), thread_id, req.seq());
						}
					}
					break;
				}
			default:
				{
					COUT_THIS("[server] Invalid packet type from tcp channel: " << int(pkt_type))
					std::cout << std::flush;
					exit(-1);
				}
		}
	} // End for mbuf from switch os (and tcp server) to server 
  }
  
  // DPDK
  //rte_pktmbuf_free((struct rte_mbuf*)sent_pkt);
  if (sent_pkt_idx < burst_size) {
	for (uint16_t free_idx = sent_pkt_idx; free_idx != burst_size; free_idx++) {
		rte_pktmbuf_free(sent_pkts[free_idx]);
	}
  }

  //close(sockfd);
  close(server_popclient_tcpsock_list[thread_id]);
  COUT_THIS("[thread" << uint32_t(thread_id) << "] exits")
  //pthread_exit(nullptr);
  return 0;
}

#endif
