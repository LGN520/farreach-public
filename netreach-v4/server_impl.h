#ifndef SERVER_IMPL_H
#define SERVER_IMPL_H

struct alignas(CACHELINE_SIZE) ServerWorkerParam {
  xindex_t *table;
  int16_t serveridx;
  size_t throughput;
#ifdef TEST_AGG_THPT
  double sum_latency;
#endif
};
typedef ServerWorkerParam server_worker_param_t;

struct SnapshotRecord {
	val_t val;
	int32_t seq;
	bool stat;
};
typedef SnapshotRecord snapshot_record_t;

// Per-server popclient <-> one popserver in controller
int * volatile server_popclient_tcpsock_list = NULL;
std::set<index_key_t> * volatile server_cached_keyset_list = NULL;

// server.evictservers <-> controller.evictserver.evictclients
//int * volatile server_evictserver_tcpsock_list = NULL;
// server.evictserver <-> controller.evictserver.evictclient
int volatile server_evictserver_tcpsock = -1;
// single-message queues is sufficient between server.evictclients and server.workers
message_ptr_queue_t<cache_evict_t> * volatile server_cache_evict_or_case2_ptr_queue_list = NULL;
message_ptr_queue_t<cache_evict_ack_t> * volatile server_cache_evict_ack_ptr_queue_list = NULL;
/*cache_evict_ack_t *** volatile cache_evict_ack_ptrs_list = NULL;
uint32_t *volatile server_heads_for_evict_ack = NULL;
uint32_t *volatile tails_for_evict_ack = NULL;*/

// snapshot
int volatile server_consnapshotserver_tcpsock = -1;
// TODO: END HERE
std::map<index_key_t, snapshot_record_t> * volatile snapshot_data_ptr = NULL;
uint32_t* volatile snapshot_rcu;
bool volatile issnapshot = false; // TODO: it should be atomic

void prepare_server();
void close_server();

// server.workers for processing pkts
static int run_server_worker(void *param);
//void *run_server_worker(void *param);
void *run_server_evictserver(void *param);
void *run_server_consnapshotserver(void *param);

void prepare_server() {
	// Prepare for cache population
	server_popclient_tcpsock_list = new int[server_num];
	server_cached_keyset_list = new std::set<index_key_t>[server_num];
	for (size_t i = 0; i < server_num; i++) {
		create_tcpsock(server_popclient_tcpsock_list[i], "server.popclient");

		server_cached_keyset_list[i].clear();
	}

	// prepare for cache eviction
	/*server_evictserver_tcpksock_list = new int[server_num];
	for (size_t i = 0; i < server_num; i++) {
		prepare_tcpserver(server_evictserver_tcpksock_list[i], false, server_evictserver_port_start+i, 1, "server.evictserver"); // MAX_PENDING_NUM = 1
	}*/
	prepare_tcpserver(server_evictserver_tcpsock, false, server_evictserver_port_start, 1, "server.evictserver"); // MAX_PENDING_NUM = 1
	server_cache_evict_or_case2_ptr_queue_list = new message_ptr_queue_t<cache_evict_t>[server_num](SINGLE_MQ_SIZE);
	server_cache_evict_ack_ptr_queue_list = new message_ptr_queue_t<cache_evict_ack_t>[server_num](SINGLE_MQ_SIZE);

	// prepare for crash-consistent snapshot
	prepare_tcpserver(server_consnapshotserver_tcpsock, false, server_consnapshotserver_port, 1, "server.consnapshotserver"); // MAX_PENDING_NUM = 1
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
	/*if (server_evictserver_tcpsock_list != NULL) {
		delete [] server_evictserver_tcpsock_list;
		server_evictserver_tcpsock_list = NULL;
	}*/
	if (server_cache_evict_or_case2_ptr_queue_list != NULL) {
		delete [] server_cache_evict_or_case2_ptr_queue_list;
		server_cache_evict_or_case2_ptr_queue_list = NULL;
	}
	if (server_cache_evict_ack_ptr_queue_list != NULL) {
		delete [] server_cache_evict_ack_ptr_queue_list;
		server_cache_evict_ack_ptr_queue_list = NULL;
	}
}

/*
 * Worker for server-side processing 
 */

static int run_server_worker(void * param) {
//void *run_perserver_worker(void * param) {
  // Parse param
  sfg_param_t &thread_param = *(sfg_param_t *)param;
  uint8_t serveridx = thread_param.serveridx;
  xindex_t *table = thread_param.table;

  tcpconnect(server_popclient_tcpsock_list[serveridx], controller_ip_for_server, controller_popserver_port, "server.popclient", "controller.popserver"); // enforce the packet to go through NIC 
  //tcpconnect(server_popclient_tcpsock_list[serveridx], controller_ip_for_server, controller_popserver_port_start + serveridx, "server.popclient", "controller.popserver"); // enforce the packet to go through NIC 

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
  short dst_port = server_port_start + serveridx;
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

  while (!server_running) {
  }

  while (server_running) {
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
	//if (stats[serveridx]) {
	if (heads[serveridx] != tails[serveridx]) {
		//COUT_THIS("[server] Receive packet!")

		// DPDK
		//stats[serveridx] = false;
		//recv_size = decode_mbuf(pkts[serveridx], srcmac, dstmac, srcip, dstip, &srcport, &dstport, buf);
		//rte_pktmbuf_free((struct rte_mbuf*)pkts[serveridx]);
		sent_pkt = sent_pkts[sent_pkt_idx];

		INVARIANT(pkts_list[serveridx][tails[serveridx]] != nullptr);
		memset(srcip, '\0', 16);
		memset(dstip, '\0', 16);
		memset(srcmac, 0, 6);
		memset(dstmac, 0, 6);
		recv_size = decode_mbuf(pkts_list[serveridx][tails[serveridx]], srcmac, dstmac, srcip, dstip, &srcport, &unused_dstport, buf);
		rte_pktmbuf_free((struct rte_mbuf*)pkts_list[serveridx][tails[serveridx]]);
		tails[serveridx] = (tails[serveridx] + 1) % MQ_SIZE;

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
					bool tmp_stat = table->get(req.key(), tmp_val, serveridx, tmp_seq);
					//COUT_THIS("[server] val = " << tmp_val.to_string())
					get_response_t rsp(req.key(), tmp_val, tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, serveridx, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::GETREQ_NLATEST:
				{
					get_request_nlatest_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					val_t tmp_val;
					int32_t tmp_seq = 0;
					bool tmp_stat = table->get(req.key(), tmp_val, serveridx, tmp_seq);
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
					res = rte_eth_tx_burst(0, serveridx, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::PUTREQ_SEQ:
				{
					put_request_seq_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool tmp_stat = table->put(req.key(), req.val(), serveridx, req.seq());
					//COUT_THIS("[server] stat = " << tmp_stat)
					put_response_t rsp(req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, serveridx, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::DELREQ_SEQ:
				{
					del_request_seq_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					bool tmp_stat = table->remove(req.key(), serveridx, req.seq());
					//COUT_THIS("[server] stat = " << tmp_stat)
					del_response_t rsp(req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, serveridx, &sent_pkt, 1);
					sent_pkt_idx++;

					// NOTE: no matter tmp_stat is true (key is deleted) or false (no such key or key has been deleted before), we should always treat the key does not exist (i.e., being deleted), so a reordered eviction will never overwrite this result for linearizability -> we should always update the corresponding deleted set
					deleted_sets[serveridx].add(req.key(), req.seq());
					break;
				}
			case packet_type_t::SCAN_REQ:
				{
					scan_request_t req(buf, recv_size);
					//COUT_THIS("[server] startkey = " << req.key().to_string() << 
					//		<< "endkey = " << req.endkey().to_string() << " num = " << req.num())
					std::vector<std::pair<index_key_t, val_t>> results;
					//size_t tmp_num = table->scan(req.key(), req.num(), results, serveridx);
					size_t tmp_num = table->range_scan(req.key(), req.endkey(), results, serveridx);

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
					res = rte_eth_tx_burst(0, serveridx, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::GETREQ_POP: 
				{
					get_request_pop_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					val_t tmp_val;
					int32_t tmp_seq;
					bool tmp_stat = table->get(req.key(), tmp_val, serveridx, tmp_seq);
					//COUT_THIS("[server] val = " << tmp_val.to_string())
					
					get_response_t rsp(req.key(), tmp_val, tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, serveridx, &sent_pkt, 1);
					sent_pkt_idx++;

					// Trigger cache population if necessary (key exist and not being cached)
					if (tmp_stat) {
						bool is_cached_before = (server_cached_keyset_list[serveridx].find(req.key()) != server_cached_keyset_list.end());
						if (!is_cached_before) {
							server_cached_keyset_list[serveridx].insert(req.key());
							// Send CACHE_POP to controller.popserver
							cache_pop_t cache_pop_req(req.key(), tmp_val, tmp_seq, int16_t(serveridx));
							uint32_t popsize = cache_pop_req.serialize(buf, MAX_BUFSIZE);
							tcpsend(server_popclient_tcpsock_list[serveridx], buf, popsize, "server.popclient");
						}
					}
					break;
				}
			case packet_type_t::PUTREQ_POP_SEQ: 
				{
					put_request_pop_seq_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string())
					bool tmp_stat = table->put(req.key(), req.val(), serveridx, req.seq());
					//COUT_THIS("[server] val = " << tmp_val.to_string())
					
					put_response_t rsp(req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, serveridx, &sent_pkt, 1);
					sent_pkt_idx++;

					// Trigger cache population if necessary (key exist and not being cached)
					if (tmp_stat) { // successful put
						bool is_cached_before = (server_cached_keyset_list[serveridx].find(req.key()) != server_cached_keyset_list.end());
						if (!is_cached_before) {
							server_cached_keyset_list[serveridx].insert(req.key());
							// Send CACHE_POP to controller.popserver
							cache_pop_t cache_pop_req(req.key(), req.val(), req.seq(), int16_t(serveridx));
							uint32_t popsize = cache_pop_req.serialize(buf, MAX_BUFSIZE);
							tcpsend(server_popclient_tcpsock_list[serveridx], buf, popsize, "server.popclient");
						}
					}
					break;
				}
			case packet_type_t::PUTREQ_SEQ_CASE3:
				{
					COUT_THIS("PUTREQ_SEQ_CASE3")
					put_request_seq_case3_t req(buf, recv_size);

					if (!issnapshot) {
						try_kvsnapshot(table);
					}

					bool tmp_stat = table->put(req.key(), req.val(), serveridx, req.seq());
					//put_response_case3_t rsp(req.hashidx(), req.key(), serveridx, tmp_stat); // no case3_reg in switch
					put_response_t rsp(req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, serveridx, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::PUTREQ_POP_SEQ_CASE3: 
				{
					COUT_THIS("PUTREQ_POP_SEQ_CASE3")
					put_request_pop_seq_case3_t req(buf, recv_size);

					if (!issnapshot) {
						try_kvsnapshot(table);
					}

					//COUT_THIS("[server] key = " << req.key().to_string())
					bool tmp_stat = table->put(req.key(), req.val(), serveridx, req.seq());
					//COUT_THIS("[server] val = " << tmp_val.to_string())
					put_response_t rsp(req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, serveridx, &sent_pkt, 1);
					sent_pkt_idx++;

					// Trigger cache population if necessary (key exist and not being cached)
					if (tmp_stat) { // successful put
						bool is_cached_before = (server_cached_keyset_list[serveridx].find(req.key()) != server_cached_keyset_list.end());
						if (!is_cached_before) {
							server_cached_keyset_list[serveridx].insert(req.key());
							// Send CACHE_POP to controller.popserver
							cache_pop_t cache_pop_req(req.key(), req.val(), req.seq(), int16_t(serveridx));
							uint32_t popsize = cache_pop_req.serialize(buf, MAX_BUFSIZE);
							tcpsend(server_popclient_tcpsock_list[serveridx], buf, popsize, "server.popclient");
						}
					}
					break;
				}
			case packet_type_t::DELREQ_SEQ_CASE3:
				{
					COUT_THIS("DELREQ_SEQ_CASE3")
					del_request_seq_case3_t req(buf, recv_size);

					if (!issnapshot) {
						try_kvsnapshot(table);
					}

					bool tmp_stat = table->remove(req.key(), serveridx, req.seq());
					//del_response_case3_t rsp(req.hashidx(), req.key(), serveridx, tmp_stat); // no case3_reg in switch
					del_response_t rsp(req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, serveridx, &sent_pkt, 1);
					sent_pkt_idx++;

					// NOTE: no matter tmp_stat is true (key is deleted) or false (no such key or key has been deleted before), we should always treat the key does not exist (i.e., being deleted), so a reordered eviction will never overwrite this result for linearizability -> we should always update the corresponding deleted set
					deleted_sets[serveridx].add(req.key(), req.seq());
					break;
				}













			case packet_type_t::GET_RES_POP_EVICT:
				{
					// Put evicted data into key-value store
					get_response_pop_evict_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[serveridx].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), serveridx, req.seq());
						}
						else {
							table->remove(req.key(), serveridx, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_POP_EVICT:
				{
					// Put evicted data into key-value store
					put_request_pop_evict_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[serveridx].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), serveridx, req.seq());
						}
						else {
							table->remove(req.key(), serveridx, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_SEQ:
				{
					put_request_large_seq_t req(buf, recv_size);
					INVARIANT(req.val().val_length > val_t::SWITCH_MAX_VALLEN);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool tmp_stat = table->put(req.key(), req.val(), serveridx, req.seq());
					//COUT_THIS("[server] stat = " << tmp_stat)
					put_response_t rsp(req.hashidx(), req.key(), tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, serveridx, &sent_pkt, 1);
					sent_pkt_idx++;
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_EVICT:
				{
					// Put evicted data into key-value store
					put_request_large_evict_t req(buf, recv_size);
					INVARIANT(req.val().val_length <= val_t::SWITCH_MAX_VALLEN);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[serveridx].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), serveridx, req.seq());
						}
						else {
							table->remove(req.key(), serveridx, req.seq());
						}
					}
					break;
				}
			case packet_type_t::GET_RES_POP_EVICT_CASE2:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					get_response_pop_evict_case2_t req(buf, recv_size);

					if (!issnapshot) {
						try_kvsnapshot(table);
					}

					bool isdeleted = deleted_sets[serveridx].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), serveridx, req.seq());
						}
						else {
							table->remove(req.key(), serveridx, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_POP_EVICT_CASE2:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					put_request_pop_evict_case2_t req(buf, recv_size);

					if (!issnapshot) {
						try_kvsnapshot(table);
					}

					bool isdeleted = deleted_sets[serveridx].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), serveridx, req.seq());
						}
						else {
							table->remove(req.key(), serveridx, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_EVICT_CASE2:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					put_request_large_evict_case2_t req(buf, recv_size);

					if (!issnapshot) {
						try_kvsnapshot(table);
					}

					INVARIANT(req.val().val_length <= val_t.SWITCH_MAX_VALLEN);
					bool isdeleted = deleted_sets[serveridx].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), serveridx, req.seq());
						}
						else {
							table->remove(req.key(), serveridx, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_CASE3:
				{
					COUT_THIS("PUT_REQ_LARGE_CASE3")
					put_request_large_case3_t req(buf, recv_size);

					if (!issnapshot) {
						try_kvsnapshot(table);
					}

					bool tmp_stat = table->put(req.key(), req.val(), serveridx, req.seq());
					put_response_case3_t rsp(req.hashidx(), req.key(), serveridx, tmp_stat);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					
					// DPDK
					encode_mbuf(sent_pkt, dstmac, srcmac, dstip, srcip, server_port_start, srcport, buf, rsp_size);
					res = rte_eth_tx_burst(0, serveridx, &sent_pkt, 1);
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
		backup_rcu[serveridx]++;
		thread_param.throughput++;

		if (sent_pkt_idx >= burst_size) {
			sent_pkt_idx = 0;
			res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
			INVARIANT(res == 0);
		}
	} // End for mbuf from receiver to server
	if (server_heads_for_pktloss[serveridx] != tails_for_pktloss[serveridx]) {
		INVARIANT(pkts_list_for_pktloss[serveridx][tails_for_pktloss[serveridx]] != nullptr);
		recv_size = get_payload(pkts_list_for_pktloss[serveridx][tails_for_pktloss[serveridx]], buf);
		rte_pktmbuf_free((struct rte_mbuf*)pkts_list_for_pktloss[serveridx][tails_for_pktloss[serveridx]]);
		tails_for_pktloss[serveridx] = (tails_for_pktloss[serveridx] + 1) % MQ_SIZE;

		packet_type_t pkt_type = get_packet_type(buf, recv_size);
		switch (pkt_type) {
			// NOTE: we use seq mechanism to avoid incorrect overwrite of packet reordering
			case packet_type_t::GET_RES_POP_EVICT_SWITCH:
				{
					// Put evicted data into key-value store
					get_response_pop_evict_switch_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[serveridx].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), serveridx, req.seq());
						}
						else {
							table->remove(req.key(), serveridx, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_POP_EVICT_SWITCH:
				{
					// Put evicted data into key-value store
					put_request_pop_evict_switch_t req(buf, recv_size);
					//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
					bool isdeleted = deleted_sets[serveridx].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), serveridx, req.seq());
						}
						else {
							table->remove(req.key(), serveridx, req.seq());
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
					bool isdeleted = deleted_sets[serveridx].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), serveridx, req.seq());
						}
						else {
							table->remove(req.key(), serveridx, req.seq());
						}
					}
					break;
				}
			case packet_type_t::GET_RES_POP_EVICT_CASE2_SWITCH:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					get_response_pop_evict_case2_switch_t req(buf, recv_size);

					if (!issnapshot) {
						try_kvsnapshot(table);
					}

					bool isdeleted = deleted_sets[serveridx].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), serveridx, req.seq());
						}
						else {
							table->remove(req.key(), serveridx, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_POP_EVICT_CASE2_SWITCH:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					put_request_pop_evict_case2_switch_t req(buf, recv_size);

					if (!issnapshot) {
						try_kvsnapshot(table);
					}

					bool isdeleted = deleted_sets[serveridx].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), serveridx, req.seq());
						}
						else {
							table->remove(req.key(), serveridx, req.seq());
						}
					}
					break;
				}
			case packet_type_t::PUT_REQ_LARGE_EVICT_CASE2_SWITCH:
				{
					// Switch OS will add special case for rollback; here we only perform the pkt if necessary
					put_request_large_evict_case2_switch_t req(buf, recv_size);

					if (!issnapshot) {
						try_kvsnapshot(table);
					}

					INVARIANT(req.val().val_length <= val_t.SWITCH_MAX_VALLEN);
					bool isdeleted = deleted_sets[serveridx].check_and_remove(req.key(), req.seq());
					if (!isdeleted) { // Do not overwrite if delete.seq > evict.eq
						if (req.val().val_length > 0) {
							table->put(req.key(), req.val(), serveridx, req.seq());
						}
						else {
							table->remove(req.key(), serveridx, req.seq());
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













	cache_evict_t *tmp_cache_evict_ptr = server_cache_evict_or_case2_ptr_queue_list[serveridx].read();
	if (tmp_cache_evict_ptr != NULL) {
		INVARIANT(tmp_cache_evict_ptr->serveridx() == serveridx);
		INVARIANT(server_cached_keyset_list[serveridx].find(tmp_cache_evict_ptr->key()) != server_cached_keyset_list.end());

		// make server-side snapshot for CACHE_EVICT_CASE2
		if (tmp_cache_evict_ptr->_type == packet_type_t::CACHE_EVICT_CASE2) {
			printf("CACHE_EVICT_CASE2!\n");
			if (!issnapshot) {
				try_kvsnapshot(table);
			}
		}

		// remove from cached keyset
		server_cached_keysetlist[serveridx].erase(tmp_cache_evict_ptr->key()); // NOTE: no contention

		// update in-memory KVS if necessary
		if (tmp_cache_evict_ptr->result()) { // put
			table->put(tmp_cache_evict_ptr->key(), tmp_cache_evict_ptr->val(), serveridx, tmp_cache_evict_ptr->seq());
		}
		else { // del
			table->remove(tmp_cache_evict_ptr->key(), serveridx, tmp_cache_evict_ptr->seq());
		}

		// send CACHE_EVICT_ACK to server.evictserver
		cachr_evict_ack_t *tmp_cache_evict_ack_ptr = new cache_evict_ack(tmp_cache_evict_ptr->key()); // freed by server.evictserver
		bool res = server_cache_evict_ack_ptr_queue_list[serveridx].write(tmp_cache_evict_ack_ptr);
		if (!res) {
			printf("[server.worker] error: more than one CACHE_EVICT_ACK!\n");
			exit(-1);
		}

		// free CACHE_EVIT
		delete tmp_cache_evict_ptr;
		tmp_cache_evict_ptr = NULL;
	}
  }
  
  // DPDK
  //rte_pktmbuf_free((struct rte_mbuf*)sent_pkt);
  if (sent_pkt_idx < burst_size) {
	for (uint16_t free_idx = sent_pkt_idx; free_idx != burst_size; free_idx++) {
		rte_pktmbuf_free(sent_pkts[free_idx]);
	}
  }

  //close(sockfd);
  close(server_popclient_tcpsock_list[serveridx]);
  COUT_THIS("[thread" << uint32_t(serveridx) << "] exits")
  //pthread_exit(nullptr);
  return 0;
}

void run_server_evictserver(void *param) {
	//int16_t serveridx = *((int16_t *)param);

	// Not used
	//struct sockaddr_in controller_addr;
	//unsigned int controller_addr_len = sizeof(struct sockaddr);

	while (!server_running) {}

	int connfd = -1;
	//tcpaccept(server_evictserver_tcpsock_list[serveridx], NULL, NULL, connfd, "server.evictserver");
	tcpaccept(server_evictserver_tcpsock, NULL, NULL, connfd, "server.evictserver");

	// process CACHE_EVICT/_CASE2 packet <optype, key, vallen, value, result, seq, serveridx>
	char recvbuf[MAX_BUFSIZE];
	int cur_recv_bytes = 0;
	int8_t optype = -1;
	index_key_t tmpkey = index_key_t();
	int32_t vallen = -1;
	bool is_waitack = false;
	const int arrive_optype_bytes = sizeof(int8_t);
	const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(index_key_t) + sizeof(int32_t);
	int arrive_serveridx_bytes = -1;
	int tmp_serveridx = -1;
	while (server_running) {
		int recvsize = 0;
		bool is_broken = tcprecv(connfd, recvbuf + cur_recv_bytes, MAX_BUFSIZE - cur_recv_bytes, 0, recvsize, "server.evictserver");
		if (is_broken) {
			break;
		}

		cur_recv_bytes += recvsize;
		if (cur_recv_bytes >= MAX_BUFSIZE) {
			printf("[server.evictserver] Overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_BUFSIZE);
            exit(-1);
		}

		// Get optype
		if (optype == -1 && cur_recv_bytes >= arrive_optype_bytes) {
			optype = *((int8_t *)recvbuf);
			INVARIANT(packet_type_t(optype) == packet_type_t::CACHE_EVICT || packet_type_t(optype) == packet_type_t::CACHE_EVICT_CASE2);
		}

		// Get key and vallen
		if (optype != -1 && vallen == -1 && cur_recv_bytes >= arrive_vallen_bytes) {
			tmpkey.deserialize(recvbuf + arrive_optype_bytes, cur_recv_bytes - arrive_optype_bytes);
			vallen = *((int32_t *)(recvbuf + arrive_vallen_bytes - sizeof(int32_t)));
			vallen = int32_t(ntohl(uint32_t(vallen)));
			INVARIANT(vallen >= 0);
			int padding_size = int(val_t::get_padding_size(vallen)); // padding for value <= 128B
			arrive_serveridx_bytes = arrive_vallen_bytes + vallen + padding_size + sizeof(int32_t) + sizeof(bool) + sizeof(int16_t);
		}

		// Get one complete CACHE_EVICT/_CASE2 (only need serveridx here)
		if (optype != -1 && vallen != -1 && cur_recv_bytes >= arrive_serveridx_bytes && !is_waitack) {
			bool res = false;
			// send CACHE_EVICT to server.worker 
			if (packet_type_t(optype) == packet_type_t::CACHE_EVICT) {
				cache_evict_t *tmp_cache_evict_ptr = new cache_evict_t(recvbuf, arrive_serveridx_bytes); // freed by server.worker
				//INVARIANT(tmp_cache_evict_ptr->serveridx() == serveridx);
				tmp_serveridx = tmp_cache_evict_ptr->serveridx();
				res = server_cache_evict_or_case2_ptr_queue_list[tmp_serveridx].write(tmp_cache_evict_ptr);
			}
			else if (packet_type_t(optype) == packet_type_t::CACHE_EVICT_CASE2) {
				cache_evict_case2_t *tmp_cache_evict_case2_ptr = new cache_evict_case2_t(recvbuf, arrive_serveridx_bytes); // freed by server.worker
				//INVARIANT(tmp_cache_evict_case2_ptr->serveridx() == serveridx);
				tmp_serveridx = tmp_cache_evict_case2_ptr->serveridx();
				res = server_cache_evict_or_case2_ptr_queue_list[tmp_serveridx].write((cache_evict_t *)tmp_cache_evict_case2_ptr);
			}
			else {
				printf("[server.evictserver] error: invalid optype: %d\n", int(optype));
				exit(-1);
			}
			
			if (!res) {
				printf("[server.evictserver] error: more than one CACHE_EVICT/_CASE2!\n");
				exit(-1);
			}

			is_waitack = true;
		}

		// wait for CACHE_EVICT_ACK from server.worker
		if (is_waitack) {
			cache_evict_ack_t *tmp_cache_evict_ack_ptr = server_cache_evict_ack_ptr_queue_list[tmp_serveridx].read();
			if (tmp_cache_evict_ack_ptr != NULL) {
				INVARIANT(tmp_cache_evict_ack_ptr->key() == tmpkey);

				// send CACHE_EVICT_ACK to controller.evictserver.evictclient
				char sendbuf[MAX_BUFSIZE];
				int sendsize = tmp_cache_evict_ack_ptr.serialize(sendbuf, MAX_BUF_SIZE);
				tcpsend(connfd, sendbuf, sendsize, "server.evictserver");

				// move remaining bytes and reset metadata
				if (cur_recv_bytes > arrive_serveridx_bytes) {
					memcpy(buf, buf + arrive_serveridx_bytes, cur_recv_bytes - arrive_serveridx_bytes);
					cur_recv_bytes = cur_recv_bytes - arrive_serveridx_byets;
				}
				else {
					cur_recv_bytes = 0;
				}
				optype = -1;
				tmpkey = index_key_t();
				vallen = -1;
				is_waitack = false;
				arrive_serveridx_bytes = -1;
				tmp_serveridx = -1;

				// free CACHE_EVIT_ACK
				delete tmp_cache_evict_ack_ptr;
				tmp_cache_evict_ack_ptr = NULL;
			}
		}
	}
}

void *run_server_consnapshotserver(void *param) {
	xindex_t *table = (xindex_t *)param;
	INVARIANT(table != NULL);

	struct sockaddr_in switchos_addr;
	unsigned int switchos_addr_len = sizeof(struct sockaddr);

	while (!server_running) {}

	int connfd = -1;
	tcpaccept(server_consnapshotserver_tcpsock, NULL, NULL, connfd, "server.consnapshotserver");

	char recvbuf[MAX_LARGE_BUFSIZE];
	int cur_recv_bytes = 0;
	int phase = 0; // 0: wait for SNAPSHOT_SERVERSIDE; 1: wait for crash-consistent snapshot data
	int control_type_phase0 = -1;
	while (server_running) {
		int recvsize = 0;
		bool is_broken = tcprecv(connfd, recvbuf + cur_recv_bytes, MAX_LARGE_BUFSIZE - cur_recv_bytes, 0, recvsize, "server.consnapshotserver");
		if (is_broken) {
			break;
		}

		cur_recv_bytes += recvsize;
		if (cur_recv_bytes >= MAX_LARGE_BUFSIZE) {
			printf("[server.consnapshotserver] Overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_LARGE_BUFSIZE);
            exit(-1);
		}

		if (phase == 0) {
			// process SNAPSHOT_SERVERSIDE for server-side snapshot
			if (control_type_phase0 == -1 && cur_recv_bytes >= sizeof(int)) {
				control_type_phase0 = *((int *)recvbuf);
				INVARIANT(control_type_phase0 == SNAPSHOT_SERVERSIDE);
			}

			// make server-side snapshot (simulate distributed in-memory KVS by concurrent one)
			if (!issnapshot) {
				try_kvsnapshot(table);
			}

			// send SNAPSHOT_SERVERSIDE_ACK to controller
			tcpsend(connfd, (char *)&SNAPSHOT_SERVERSIDE_ACK, sizeof(int), "server.consnapshotserver");

			phase = 1; // wait for crash-consistent snapshot data
		}
		else if (phase == 1) {
			// NOTE: skip sizeof(int) for SNAPSHOT_SERVERSIDE
			// TODO: process crash-consistent snapshot
			
			// TODO: move and reset
		}
	}
}

#endif
