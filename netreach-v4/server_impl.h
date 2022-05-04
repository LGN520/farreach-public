#ifndef SERVER_IMPL_H
#define SERVER_IMPL_H

// Transaction phase for ycsb_server

#include "deleted_set_impl.h"

typedef DeletedSet<index_key_t, int32_t> deleted_set_t;
typedef xindex::XIndex<index_key_t, val_t> xindex_t;

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

/*struct alignas(CACHELINE_SIZE) ReceiverParam {
	size_t overall_thpt;
};*/
//typedef ReceiverParam receiver_param_t;

bool volatile server_running = false;
std::atomic<size_t> server_ready_threads(0);
size_t server_expected_ready_threads = 0;

// server.receiver in transaction phase
struct rte_mempool *mbuf_pool = NULL;
//volatile struct rte_mbuf **pkts;
//volatile bool *stats;
struct rte_mbuf*** volatile server_worker_pkts_list; // pkts from receiver to each worker
uint32_t* volatile server_worker_heads;
uint32_t* volatile server_worker_tails;

// Per-server popclient <-> one popserver in controller
int * volatile server_popclient_tcpsock_list = NULL;
std::set<index_key_t> * volatile server_cached_keyset_list = NULL;

// server.evictservers <-> controller.evictserver.evictclients
//int * volatile server_evictserver_tcpsock_list = NULL;
// server.evictserver <-> controller.evictserver.evictclient
int volatile server_evictserver_tcpsock = -1;
// single-message queues is sufficient between server.evictclients and server.workers
MessagePtrQueue<cache_evict_t> * volatile server_cache_evict_or_case2_ptr_queue_list = NULL;
MessagePtrQueue<cache_evict_ack_t> * volatile server_cache_evict_ack_ptr_queue_list = NULL;
/*cache_evict_ack_t *** volatile cache_evict_ack_ptrs_list = NULL;
uint32_t *volatile server_server_worker_heads_for_evict_ack = NULL;
uint32_t *volatile server_worker_tails_for_evict_ack = NULL;*/

// snapshot
int volatile server_consnapshotserver_tcpsock = -1;
// per-server snapshot_map and snapshot_rcu
std::map<index_key_t, snapshot_record_t> * volatile server_snapshot_maps = NULL;
uint32_t* volatile server_snapshot_rcus = NULL;
bool volatile server_issnapshot = false; // TODO: it should be atomic

// DELREQ
deleted_set_t *server_deleted_sets = NULL;

bool volatile killed = false;
void kill(int signum) {
	COUT_THIS("[server] receive SIGKILL!")
	killed = true;
}

void prepare_server();
void run_server(xindex_t *table); // transaction phase
void close_server();

static int run_receiver(__attribute__((unused)) void *param);
// server.workers for processing pkts
static int run_server_worker(void *param);
//void *run_server_worker(void *param);
void *run_server_evictserver(void *param);
void *run_server_consnapshotserver(void *param);

void prepare_server() {
	// server_num workers + receiver + evictserver + consnapshotserver
	server_expected_ready_threads = server_num + 3;

	// Prepare pkts and stats for receiver (based on ring buffer)
	// From receiver to each server
	//pkts = new volatile struct rte_mbuf*[server_num];
	//stats = new volatile bool[server_num];
	//memset((void *)pkts, 0, sizeof(struct rte_mbuf *)*server_num);
	//memset((void *)stats, 0, sizeof(bool)*server_num);
	//for (size_t i = 0; i < server_num; i++) {
	//  pkts[i] = rte_pktmbuf_alloc(mbuf_pool);
	//}
	server_worker_pkts_list = new struct rte_mbuf**[server_num];
	server_worker_heads = new uint32_t[server_num];
	server_worker_tails = new uint32_t[server_num];
	memset((void*)server_worker_heads, 0, sizeof(uint32_t)*server_num);
	memset((void*)server_worker_tails, 0, sizeof(uint32_t)*server_num);
	//int res = 0;
	for (size_t i = 0; i < server_num; i++) {
	  server_worker_pkts_list[i] = new struct rte_mbuf*[MQ_SIZE];
	  for (size_t j = 0; j < MQ_SIZE; j++) {
		  server_worker_pkts_list[i][j] = nullptr;
	  }
	  //res = rte_pktmbuf_alloc_bulk(mbuf_pool, server_worker_pkts_list[i], MQ_SIZE);
	}

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
	server_cache_evict_or_case2_ptr_queue_list = new MessagePtrQueue<cache_evict_t>[server_num];
	server_cache_evict_ack_ptr_queue_list = new MessagePtrQueue<cache_evict_ack_t>[server_num];
	for (size_t i = 0; i < server_num; i++) {
		server_cache_evict_or_case2_ptr_queue_list[i].init(SINGLE_MQ_SIZE);
		server_cache_evict_ack_ptr_queue_list[i].init(SINGLE_MQ_SIZE);
	}

	// prepare for crash-consistent snapshot
	prepare_tcpserver(server_consnapshotserver_tcpsock, false, server_consnapshotserver_port, 1, "server.consnapshotserver"); // MAX_PENDING_NUM = 1

	// prepare for snapshot
	server_snapshot_rcus = new uint32_t[server_num];
	memset((void *)server_snapshot_rcus, 0, server_num*sizeof(uint32_t));

	// DELREQ
	server_deleted_sets = new deleted_set_t[server_num];
}

void run_server(xindex_t *table) {
	int ret = 0;
	unsigned lcoreid = 1; // 0: master lcore; 1: slave lcore for receiver

	server_running = false;

	// launch receiver
	//receiver_param_t receiver_param;
	//receiver_param.overall_thpt = 0;
	//ret = rte_eal_remote_launch(run_receiver, (void*)&receiver_param, lcoreid);
	ret = rte_eal_remote_launch(run_receiver, NULL, lcoreid);
	if (ret) {
		COUT_N_EXIT("Error of launching receiver:" << ret);
	}
	COUT_THIS("[server] Launch receiver with ret code " << ret);
	lcoreid++;

	// launch workers (processing normal packets)
	//pthread_t threads[server_num];
	server_worker_param_t server_worker_params[server_num];
	// check if parameters are cacheline aligned
	for (size_t i = 0; i < server_num; i++) {
		if ((uint64_t)(&(server_worker_params[i])) % CACHELINE_SIZE != 0) {
			COUT_N_EXIT("wrong parameter address: " << &(server_worker_params[i]));
		}
	}
	for (int16_t worker_i = 0; worker_i < int16_t(server_num); worker_i++) {
		server_worker_params[worker_i].table = table;
		server_worker_params[worker_i].serveridx = worker_i;
		server_worker_params[worker_i].throughput = 0;
#ifdef TEST_AGG_THPT
		server_worker_params[worker_i].sum_latency = 0.0;
#endif
		//int ret = pthread_create(&threads[worker_i], nullptr, run_sfg, (void *)&server_worker_params[worker_i]);
		ret = rte_eal_remote_launch(run_server_worker, (void *)&server_worker_params[worker_i], lcoreid);
		if (ret) {
		  COUT_N_EXIT("Error of launching some worker:" << ret);
		}
		lcoreid++;
		if (lcoreid >= MAX_LCORE_NUM) {
			lcoreid = 1;
		}
	}
	COUT_THIS("[server] prepare server worker threads...")

	// launch evictserver
	pthread_t evictserver_thread;
	ret = pthread_create(&evictserver_thread, nullptr, run_server_evictserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error of launching evictserver: " << ret);
	}

	// launch consnapshotserver
	pthread_t consnapshotserver_thread;
	ret = pthread_create(&consnapshotserver_thread, nullptr, run_server_consnapshotserver, (void *)table);
	if (ret) {
		COUT_N_EXIT("Error of launching consnapshotserver: " << ret);
	}

	while (server_ready_threads < server_expected_ready_threads) sleep(1);

	server_running = true;
	COUT_THIS("[server] start running...")

	signal(SIGTERM, kill); // Set for main thread (kill -15)

	while (!killed) {
		sleep(1);
	}

	/* Processing Statistics */
	//COUT_THIS("Server-side aggregate throughput: " << receiver_param.overall_thpt);
	size_t overall_thpt = 0;
	std::vector<double> load_balance_ratio_list;
	for (size_t i = 0; i < server_num; i++) {
		overall_thpt += server_worker_params[i].throughput;
	}
	COUT_THIS("Server-side overall throughput: " << overall_thpt);
	double avg_per_server_thpt = double(overall_thpt) / double(server_num);
	for (size_t i = 0; i < server_num; i++) {
		load_balance_ratio_list.push_back(double(server_worker_params[i].throughput) / avg_per_server_thpt);
	}
	for (size_t i = 0; i < load_balance_ratio_list.size(); i++) {
		COUT_THIS("Load balance ratio of server " << i << ": " << load_balance_ratio_list[i]);
	}
#ifdef TEST_AGG_THPT
	double max_agg_thpt = 0.0;
	for (size_t i = 0; i < server_num; i++) {
		max_agg_thpt += (double(server_worker_params[i].throughput) / server_worker_params[i].sum_latency * 1000 * 1000);
	}
	max_agg_thpt /= double(1024 * 1024);
	COUT_THIS("Max server-side aggregate throughput: " << max_agg_thpt << " MQPS");
#endif

	server_running = false;
	/*void *status;
	for (size_t i = 0; i < server_num; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
		  COUT_N_EXIT("Error:unable to join," << rc);
		}
	}*/
	rte_eal_mp_wait_lcore();
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
	if (server_snapshot_rcus != NULL) {
		delete [] server_snapshot_rcus;
		server_snapshot_rcus = NULL;
	}
	if (server_deleted_sets != NULL) {
		delete [] server_deleted_sets;
		server_deleted_sets = NULL;
	}

	// Free DPDK mbufs
	for (size_t i = 0; i < server_num; i++) {
	//rte_pktmbuf_free((struct rte_mbuf *)pkts[i]);
	  while (server_worker_heads[i] != server_worker_tails[i]) {
		  rte_pktmbuf_free((struct rte_mbuf*)server_worker_pkts_list[i][server_worker_tails[i]]);
		  server_worker_tails[i] += 1;
	  }
	}
}

/*
 * Receiver for normal packets
 */

static int run_receiver(void *param) {
	//receiver_param_t &receiver_param = *((receiver_param_t *)param);
	server_ready_threads++;

	while (!server_running)
		;

	struct rte_mbuf *received_pkts[32];
	memset((void *)received_pkts, 0, 32*sizeof(struct rte_mbuf *));
	index_key_t startkey, endkey;
	uint32_t num;
	while (server_running) {
		uint16_t n_rx;
		n_rx = rte_eth_rx_burst(0, 0, received_pkts, 32);
		//n_rx = rte_eth_rx_burst(0, 0, received_pkts, 1);
		//struct rte_eth_stats ethstats;
		//rte_eth_stats_get(0, &ethstats);

		if (n_rx == 0) continue;
		for (size_t i = 0; i < n_rx; i++) {
			int received_port = get_dstport(received_pkts[i]);
			if (received_port == -1) {
				continue;
			}
			else {
				packet_type_t optype = packet_type_t(get_optype(received_pkts[i]));
				if (optype == packet_type_t::CACHE_POP_INSWITCH_ACK
						|| optype == packet_type_t::GETRES_LATEST_SEQ_INSWITCH_CASE1
						|| optype == packet_type_t::GETRES_DELETED_SEQ_INSWITCH_CASE1
						|| optype == packet_type_t::PUTREQ_SEQ_INSWITCH_CASE1
						|| optype == packet_type_t::DELREQ_SEQ_INSWITCH_CASE1) {
					if (((reflector_head_for_popack_snapshot + 1) % MQ_SIZE) != reflector_tail_for_popack_snapshot) {
						reflector_pkts_for_popack_snapshot[reflector_head_for_popack_snapshot] = received_pkts[i];
						reflector_head_for_popack_snapshot = (reflector_head_for_popack_snapshot + 1) % MQ_SIZE;
					}
				}
				else { // normal packets
					int idx = received_port - server_port_start;
					if (idx < 0 || unsigned(idx) >= server_num) {
						COUT_THIS("Invalid dst port received by server: " << received_port)
						continue;
					}
					else {
						/*if (stats[idx]) {
							COUT_THIS("Invalid stas[" << idx << "] which is true!")
							continue;
						}
						else {
							pkts[idx] = received_pkts[i];
							stats[idx] = true;
						}*/
						if (((server_worker_heads[idx] + 1) % MQ_SIZE) != server_worker_tails[idx]) {
							//receiver_param.overall_thpt++;
							server_worker_pkts_list[idx][server_worker_heads[idx]] = received_pkts[i];
							server_worker_heads[idx] = (server_worker_heads[idx] + 1) % MQ_SIZE;
						}
						else {
							COUT_THIS("Drop pkt since server_worker_pkts_list["<<idx<<"] is full!")
							rte_pktmbuf_free(received_pkts[i]);
						}
					}
				} // give to reflector/worker
			} // judge received_port
			received_pkts[i] = NULL;
		} // for each i
	}
	return 0;
}

/*
 * Worker for server-side processing 
 */

static int run_server_worker(void * param) {
//void *run_perserver_worker(void * param) {
  // Parse param
  server_worker_param_t &thread_param = *(server_worker_param_t *)param;
  uint8_t serveridx = thread_param.serveridx; // [0, server_num-1]
  xindex_t *table = thread_param.table;

  // scan.startkey <= max_startkey; scan.endkey >= min_startkey
  // use size_t to avoid int overflow
  int64_t min_startkeyhihi = serveridx * perserver_keyrange;
  int64_t max_endkeyhihi = min_startkeyhihi - 1 + perserver_keyrange;
  INVARIANT(min_startkeyhihi >= std::numeric_limits<int32_t>::min() && min_startkeyhihi <= std::numeric_limits<int32_t>::max());
  INVARIANT(max_endkeyhihi >= std::numeric_limits<int32_t>::min() && max_endkeyhihi <= std::numeric_limits<int32_t>::max());
  INVARIANT(max_endkeyhihi >= min_startkeyhihi);
  index_key_t min_startkey(0, 0, 0, min_startkeyhihi);
  index_key_t max_endkey(std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max(), max_endkeyhihi);

  // NOTE: controller and switchos should have been launched before servers
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

  // packet headers
  uint8_t srcmac[6];
  uint8_t dstmac[6];
  char srcip[16];
  char dstip[16];
  uint16_t srcport;
  uint16_t unused_dstport; // we use server_port_start instead of received dstport to hide server-side partition for client

  server_ready_threads++;

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
	if (server_worker_heads[serveridx] != server_worker_tails[serveridx]) {
		//COUT_THIS("[server] Receive packet!")

		// DPDK
		//stats[serveridx] = false;
		//recv_size = decode_mbuf(pkts[serveridx], srcmac, dstmac, srcip, dstip, &srcport, &dstport, buf);
		//rte_pktmbuf_free((struct rte_mbuf*)pkts[serveridx]);
		sent_pkt = sent_pkts[sent_pkt_idx];

		INVARIANT(server_worker_pkts_list[serveridx][server_worker_tails[serveridx]] != nullptr);
		memset(srcip, '\0', 16);
		memset(dstip, '\0', 16);
		memset(srcmac, 0, 6);
		memset(dstmac, 0, 6);
		recv_size = decode_mbuf(server_worker_pkts_list[serveridx][server_worker_tails[serveridx]], srcmac, dstmac, srcip, dstip, &srcport, &unused_dstport, buf);
		rte_pktmbuf_free((struct rte_mbuf*)server_worker_pkts_list[serveridx][server_worker_tails[serveridx]]);
		server_worker_tails[serveridx] = (server_worker_tails[serveridx] + 1) % MQ_SIZE;

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
					server_deleted_sets[serveridx].add(req.key(), req.seq());
					break;
				}
			case packet_type_t::SCANREQ_SPLIT:
				{
					scan_request_split_t req(buf, recv_size);
					
					// get verified key range
					INVARIANT(req.key() <= max_endkey);
					INVARIANT(req.endkey() >= min_startkey);
					index_key_t cur_startkey = req.key();
					index_key_t cur_endkey = req.endkey();
					if (cur_startkey < min_startkey) {
						cur_startkey = min_startkey;
					}
					if (cur_endkey > max_endkey) {
						cur_endkey = max_endkey;
					}

					// get results from in-memory snapshot in [cur_startkey, cur_endkey]
					//COUT_THIS("[server.worker] startkey: " << cur_startkey << "endkey: " << cur_endkey().to_string()
					//		<< "min_startkey: " << min_startkey << "max_endkey: " << max_endkey; // << " num = " << req.num())
					std::vector<std::pair<index_key_t, val_t>> inmemory_results; // TODO: val_t -> snapshot_record_t
					//size_t tmp_num = table->scan(req.key(), req.num(), results, serveridx);
					size_t inmemory_num = table->range_scan(cur_startkey, cur_endkey, inmemory_results, serveridx);

					// get results from in-switch snapshot in [cur_startkey, cur_endkey]
					std::map<index_key_t, snapshot_record_t> *tmp_server_snapshot_maps = server_snapshot_maps;
					std::vector<std::pair<index_key_t, snapshot_record_t>> inswitch_results;
					if (tmp_server_snapshot_maps != NULL) {
						std::map<index_key_t, snapshot_record_t>::iterator iter = tmp_server_snapshot_maps[serveridx].lower_bound(cur_startkey);
						for (; iter != tmp_server_snapshot_maps[serveridx].end() && iter->first <= cur_endkey; iter++) {
							//inmemory_results.push_back(*iter);
							inswitch_results.push_back(*iter);
						}
					}

					// merge sort w/ seq comparison
					// TODO: change val_t to snapshot_record_t for results in xindex.range_scan
					std::vector<std::pair<index_key_t, val_t>> results;
					if (inmemory_results.size() == 0) {
						for (uint32_t inswitch_idx = 0; inswitch_idx < inswitch_results.size(); inswitch_idx++) {
							index_key_t &tmpkey = inswitch_results[inswitch_idx].first;
							snapshot_record_t &tmprecord = inswitch_results[inswitch_idx].second;
							if (tmprecord.stat) {
								results.push_back(std::pair<index_key_t, val_t>(tmpkey, tmprecord.val));
							}
						}
					}
					else if (inswitch_results.size() == 0) {
						for (uint32_t inmemory_idx = 0; inmemory_idx < inmemory_results.size(); inmemory_idx++) {
							index_key_t &tmpkey = inmemory_results[inmemory_idx].first;
							snapshot_record_t &tmprecord = inmemory_results[inmemory_idx].second;
							if (tmprecord.stat) {
								results.push_back(std::pair<index_key_t, val_t>(tmpkey, tmprecord.val));
							}
						}
					}
					else {
						uint32_t inmemory_idx = 0;
						uint32_t inswitch_idx = 0;
						bool remain_inmemory = false;
						bool remain_inswitch = false;
						while (true) {
							index_key_t &tmp_inmemory_key = inmemory_results[inmemory_idx].first;
							snapshot_record_t &tmp_inmemory_record = inmemory_results[inmemory_idx].second;
							index_key_t &tmp_inswitch_key = inswitch_results[inswitch_idx].first;
							snapshot_record_t &tmp_inswitch_record = inswitch_results[inswitch_idx].second;
							if ((tmp_inmemory_key < tmp_inswitch_key) && tmp_inmemory_record.stat) {
								results.push_back(std::pair<index_key_t, val_t>(tmp_inmemory_key, tmp_inmemory_record.val));
								inmemory_idx += 1;
							}
							else if ((tmp_inswitch_key < tmp_inmemory_key) && tmp_inswitch_record.stat) {
								results.push_back(std::pair<index_key_t, val_t>(tmp_inswitch_key, tmp_inswitch_record.val));
								inswitch_idx += 1;
							}
							else if (tmp_inmemory_key == tmp_inswitch_key) {
								if (tmp_inmemory_record.stat && tmp_inswitch_record.stat) {
									if (tmp_inmemory_record.seq >= tmp_inswitch_record.seq) {
										results.push_back(std::pair<index_key_t, val_t>(tmp_inmemory_key, tmp_inmemory_record.val));
										inmemory_idx += 1;
									}
									else {
										results.push_back(std::pair<index_key_t, val_t>(tmp_inswitch_key, tmp_inswitch_record.val));
										inswitch_idx += 1;
									}
								}
								else if (tmp_inmemory_record.stat) {
									results.push_back(std::pair<index_key_t, val_t>(tmp_inmemory_key, tmp_inmemory_record.val));
									inmemory_idx += 1;
								}
								else if (tmp_inswitch_record.stat) {
									results.push_back(std::pair<index_key_t, val_t>(tmp_inswitch_key, tmp_inswitch_record.val));
									inswitch_idx += 1;
								}
							}

							if (inmemory_idx >= inmemory_results.size()) {
								remain_inswitch = true;
								break;
							}
							else if (inswitch_idx >= inswitch_results.size()) {
								remain_inmemory = true;
								break;
							}
						} // while (true)
						if (remain_inswitch) {
							for (; inswitch_idx < inswitch_results.size(); inswitch_idx++) {
								index_key_t &tmpkey = inswitch_results[inswitch_idx].first;
								snapshot_record_t &tmprecord = inswitch_results[inswitch_idx].second;
								if (tmprecord.stat) {
									results.push_back(std::pair<index_key_t, val_t>(tmpkey, tmprecord.val));
								}
							}
						}
						else if (remain_inmemory) {
							for (; inmemory_idx < inmemory_results.size(); inmemory_idx++) {
								index_key_t &tmpkey = inmemory_results[inmemory_idx].first;
								snapshot_record_t &tmprecord = inmemory_results[inmemory_idx].second;
								if (tmprecord.stat) {
									results.push_back(std::pair<index_key_t, val_t>(tmpkey, tmprecord.val));
								}
							}
						}
					} // both inmemory_results and inswitch_results are not empty
					//COUT_THIS("results size: " << results.size());

					scan_response_split_t rsp(req.key(), req.endkey(), req.cur_scanidx(), req.max_scannum(), results.size(), results);
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

					if (!server_issnapshot) {
						table->make_snapshot();
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

					if (!server_issnapshot) {
						table->make_snapshot();
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

					if (!server_issnapshot) {
						table->make_snapshot();
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
					server_deleted_sets[serveridx].add(req.key(), req.seq());
					break;
				}
			default:
				{
					COUT_THIS("[server.worker] Invalid packet type from receiver: " << int(pkt_type))
					std::cout << std::flush;
					exit(-1);
				}
		}
#ifdef TEST_AGG_THPT
		CUR_TIME(t2);
		DELTA_TIME(t2, t1, t3);
		thread_param.sum_latency += GET_MICROSECOND(t3);
#endif
		//backup_rcu[serveridx]++;
		server_snapshot_rcus[serveridx]++;
		thread_param.throughput++;

		if (sent_pkt_idx >= burst_size) {
			sent_pkt_idx = 0;
			res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
			INVARIANT(res == 0);
		}
	} // End for mbuf from receiver to server

	cache_evict_t *tmp_cache_evict_ptr = server_cache_evict_or_case2_ptr_queue_list[serveridx].read();
	if (tmp_cache_evict_ptr != NULL) {
		INVARIANT(tmp_cache_evict_ptr->serveridx() == serveridx);
		INVARIANT(server_cached_keyset_list[serveridx].find(tmp_cache_evict_ptr->key()) != server_cached_keyset_list.end());

		// make server-side snapshot for CACHE_EVICT_CASE2
		if (tmp_cache_evict_ptr->_type == packet_type_t::CACHE_EVICT_CASE2) {
			printf("CACHE_EVICT_CASE2!\n");
			if (!server_issnapshot) {
				table->make_snapshot();
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
	
	server_ready_threads++;

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

	// Not used
	//struct sockaddr_in controller_addr;
	//unsigned int controller_addr_len = sizeof(struct sockaddr);
	
	server_ready_threads++;

	while (!server_running) {}

	int connfd = -1;
	tcpaccept(server_consnapshotserver_tcpsock, NULL, NULL, connfd, "server.consnapshotserver");

	char recvbuf[MAX_LARGE_BUFSIZE];
	int cur_recv_bytes = 0;
	int phase = 0; // 0: wait for SNAPSHOT_SERVERSIDE; 1: wait for crash-consistent snapshot data
	int control_type_phase0 = -1;
	int control_type_phase1 = -1;
	int total_bytes = -1;
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
			if (!server_issnapshot) {
				table->make_snapshot();
			}

			// send SNAPSHOT_SERVERSIDE_ACK to controller
			tcpsend(connfd, (char *)&SNAPSHOT_SERVERSIDE_ACK, sizeof(int), "server.consnapshotserver");

			phase = 1; // wait for crash-consistent snapshot data
		}
		else if (phase == 1) {
			// NOTE: skip sizeof(int) for SNAPSHOT_SERVERSIDE
			if (control_type_phase1 == -1 && cur_recv_bytes >= sizeof(int) + sizeof(int) + sizeof(int32_t)) { // SNAPSHOT_SERVERSIDE + SNAPSHOT_DATA + total_bytes
				control_type_phase1 = *((int *)(recvbuf + sizeof(int)));
				INVARIANT(control_type_phase1 == SNAPSHOT_DATA);
				total_bytes = *((int32_t *)(recvbuf + sizeof(int) + sizeof(int)));
			}

			// snapshot data: <int SNAPSHOT_DATA, int32_t total_bytes, per-server data>
			// per-server data: <int32_t perserver_bytes, int16_t serveridx, int32_t recordcnt, per-record data>
			// per-record data: <16B key, int32_t vallen, value (w/ padding), int32_t seq, bool stat>
			if (control_type_phase1 != -1 && cur_recv_bytes >= sizeof(int) + total_bytes) { // SNAPSHOT_SERVERSIDE + snapshot data of total_bytes
				// NOTE: per-server_bytes is used for processing snapshot data of each server (not used now)
				
				// save snapshot data of servers
				int tmp_offset = sizeof(int) + sizeof(int) + sizeof(int32_t); // SNAPSHOT_SERVERSIDE + SNAPSHOT_DATA + total_bytes
				std::map<index_key_t, snapshot_record_t> *new_server_snapshot_maps = new std::map<index_key_t, snapshot_record_t>[server_num];
				const int tmp_maxbytes = sizeof(int) + total_bytes;
				while (true) {
					tmp_offset += sizeof(int32_t); // skip perserver_bytes
					int16_t tmp_serveridx = *((int16_t *)(recvbuf + tmp_offset));
					tmp_offset += sizeof(int16_t);
					int32_t tmp_recordcnt = *((int32_t *)(recvbuf + tmp_offset));
					tmp_offset += sizeof(int32_t);
					for (int32_t tmp_recordidx = 0; tmp_recordidx < tmp_recordcnt; tmp_recordidx++) {
						index_key_t tmp_key;
						snapshot_record_t tmp_record;
						uint32_t tmp_keysize = tmp_key.deserialize(recvbuf + tmp_offset, tmp_maxbytes - tmp_offset);
						tmp_offset += tmp_keysize;
						uint32_t tmp_valsize = tmp_record.val.deserialize(recvbuf + tmp_offset, tmp_maxbytes - tmp_offset);
						tmp_offset += tmp_valsize;
						tmp_record.seq = *((int32_t *)(recvbuf + tmp_offset));
						tmp_offset += sizeof(int32_t);
						tmp_record.stat = *((bool *)(recvbuf + tmp_offset));
						tmp_offset += sizeof(bool);
						new_server_snapshot_maps[tmp_serveridx].insert(std::pair<index_key_t, snapshot_record_t>(tmp_key, tmp_record));
					}
					if (tmp_offset >= tmp_maxbytes) {
						break;
					}
				}

				// replace old snapshot data based on RCU
				std::map<index_key_t, snapshot_record_t> *old_server_snapshot_maps = server_snapshot_maps;
				server_snapshot_maps = new_server_snapshot_maps;
				if (old_server_snapshot_maps != NULL) {
					uint32_t prev_snapshot_rcus[server_num];
					for (uint32_t i = 0; i < server_num; i++) {
						prev_snapshot_rcus[i] = snapshot_rcus[i];
					}
					for (uint32_t i = 0; i < server_num; i++) {
						while (server_running && snapshot_rcus[i] == prev_snapshot_rcus[i]) {}
					}
					delete [] old_server_snapshot_maps;
					old_server_snapshot_maps = NULL;
				}

				// Move remaining bytes and reset metadata
				if (cur_recv_bytes > sizeof(int) + total_bytes) {
					memcpy(recvbuf, recvbuf + sizeof(int) + total_bytes, cur_recv_bytes - sizeof(int) - total_bytes);
					cur_recv_bytes = cur_recv_bytes - sizeof(int) - total_bytes;
				}
				else {
					cur_recv_bytes = 0;
				}
				phase = 0;
				control_type_phase0 = -1;
				control_type_phase1 = -1;
				total_bytes = -1;
			}
		}
	}
}

#endif
