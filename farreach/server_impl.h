#ifndef SERVER_IMPL_H
#define SERVER_IMPL_H

// Transaction phase for ycsb_server

#include <vector>
#include "helper.h"
#include "key.h"
#include "val.h"

#include "snapshot_record.h"
#include "concurrent_map_impl.h"
#include "concurrent_set_impl.h"
#include "message_queue_impl.h"
#include "rocksdb_wrapper.h"
#include "dynamic_array.h"
#include "pkt_ring_buffer.h"

//#define DUMP_BUF

typedef DeletedSet<netreach_key_t, uint32_t> deleted_set_t;
typedef ConcurrentSet<netreach_key_t> concurrent_set_t;

struct alignas(CACHELINE_SIZE) ServerWorkerParam {
  uint16_t local_server_logical_idx;
#ifdef DEBUG_SERVER
  std::vector<double> process_latency_list;
  std::vector<double> wait_latency_list;
  std::vector<double> wait_beforerecv_latency_list;
  std::vector<double> udprecv_latency_list;
  std::vector<double> rocksdb_latency_list;
#endif
};
typedef ServerWorkerParam server_worker_param_t;

RocksdbWrapper *db_wrappers = NULL;
int * server_worker_udpsock_list = NULL;
int * server_worker_lwpid_list = NULL;
pkt_ring_buffer_t * server_worker_pkt_ring_buffer_list = NULL;

// Per-server popclient <-> one popserver in controller
int * server_popclient_udpsock_list = NULL;
concurrent_set_t * server_cached_keyset_list = NULL;

// per-server worker <-> per-server popclient
//MessagePtrQueue<cache_pop_t> *server_cache_pop_ptr_queue_list;

// server.evictservers <-> controller.evictserver.evictclients
//int * server_evictserver_tcpsock_list = NULL;
// server.evictserver <-> controller.evictserver.evictclient
int *server_evictserver_udpsock_list = NULL;

// snapshot
int *server_snapshotserver_udpsock_list = NULL;
int *server_snapshotdataserver_udpsock_list = NULL;
std::atomic<bool> *server_issnapshot_list = NULL; // TODO: be atomic

void prepare_server();
// server.workers for processing pkts
void *run_server_worker(void *param);
//void *run_server_popclient(void *param);
void send_cachepop(const int &sockfd, const char *buf, const int &buflen, const struct sockaddr_in &dstaddr, const socklen_t &dstaddrlen, char *recvbuf, const int& maxrecvbuflen, int &recvsize);
void *run_server_evictserver(void *param);
void *run_server_snapshotserver(void *param);
void *run_server_snapshotdataserver(void *param);
void close_server();

void prepare_server() {
	printf("[server] prepare start\n");

	RocksdbWrapper::prepare_rocksdb();

	uint32_t current_server_logical_num = server_logical_idxes_list[server_physical_idx].size();

	db_wrappers = new RocksdbWrapper[current_server_logical_num];
	INVARIANT(db_wrappers != NULL);

	server_worker_udpsock_list = new int[current_server_logical_num];
	for (size_t tmp_local_server_logical_idx = 0; tmp_local_server_logical_idx < current_server_logical_num; tmp_local_server_logical_idx++) {
		uint16_t tmp_global_server_logical_idx = server_logical_idxes_list[server_physical_idx][tmp_local_server_logical_idx];
		//short tmp_server_worker_port = server_worker_port_start + tmp_global_server_logical_idx;
#ifndef SERVER_ROTATION
		short tmp_server_worker_port = server_worker_port_start + tmp_local_server_logical_idx;
#else
		short tmp_server_worker_port = 0;
		if (tmp_global_server_logical_idx == bottleneck_serveridx_for_rotation) {
			INVARIANT(tmp_local_server_logical_idx == 0);
			tmp_server_worker_port = server_worker_port_start;
		}
		else {
			tmp_server_worker_port = server_worker_port_start + tmp_global_server_logical_idx;
			if (tmp_global_server_logical_idx > bottleneck_serveridx_for_rotation) {
				tmp_server_worker_port -= 1;
			}
		}
#endif
		//prepare_udpserver(server_worker_udpsock_list[tmp_local_server_logical_idx], true, tmp_server_worker_port, "server.worker", SOCKET_TIMEOUT, 0, UDP_LARGE_RCVBUFSIZE);
		prepare_udpserver(server_worker_udpsock_list[tmp_local_server_logical_idx], true, tmp_server_worker_port, "server.worker", 0, SERVER_SOCKET_TIMEOUT_USECS, UDP_LARGE_RCVBUFSIZE);
		printf("prepare udp socket for server.worker %d-%d on port %d\n", tmp_local_server_logical_idx, tmp_global_server_logical_idx, server_worker_port_start + tmp_local_server_logical_idx);
	}
	server_worker_lwpid_list = new int[current_server_logical_num];
	memset(server_worker_lwpid_list, 0, current_server_logical_num);

	// for large value
	server_worker_pkt_ring_buffer_list = new pkt_ring_buffer_t[current_server_logical_num];
	for (size_t tmp_local_server_logical_idx = 0; tmp_local_server_logical_idx < current_server_logical_num; tmp_local_server_logical_idx++) {
		server_worker_pkt_ring_buffer_list[tmp_local_server_logical_idx].init(PKT_RING_BUFFER_SIZE);
	}

	// Prepare for cache population
	server_popclient_udpsock_list = new int[current_server_logical_num];
	server_cached_keyset_list = new concurrent_set_t[current_server_logical_num];
	for (size_t i = 0; i < current_server_logical_num; i++) {
		create_udpsock(server_popclient_udpsock_list[i], true, "server.popclient");
	}

	// Prepare message queue between per-server worker and per-server popclient
	/*server_cache_pop_ptr_queue_list = new MessagePtrQueue<cache_pop_t>[server_num];
	for (size_t i = 0; i < server_num; i++) {
		server_cache_pop_ptr_queue_list[i].init(MQ_SIZE);
	}*/

	// prepare for cache eviction
	/*server_evictserver_tcpksock_list = new int[server_num];
	for (size_t i = 0; i < server_num; i++) {
		prepare_tcpserver(server_evictserver_tcpksock_list[i], false, server_evictserver_port_start+i, 1, "server.evictserver"); // MAX_PENDING_NUM = 1
	}*/
	server_evictserver_udpsock_list = new int[current_server_logical_num];
	for (size_t i = 0; i < current_server_logical_num; i++) {
		uint16_t tmp_global_server_logical_idx = server_logical_idxes_list[server_physical_idx][i];
		prepare_udpserver(server_evictserver_udpsock_list[i], true, server_evictserver_port_start + tmp_global_server_logical_idx, "server.evictserver");
	}

	// prepare for snapshotserver
	server_snapshotserver_udpsock_list = new int[current_server_logical_num];
	server_snapshotdataserver_udpsock_list = new int[current_server_logical_num];
	for (size_t i = 0; i < current_server_logical_num; i++) {
		uint16_t tmp_global_server_logical_idx = server_logical_idxes_list[server_physical_idx][i];
		prepare_udpserver(server_snapshotserver_udpsock_list[i], true, server_snapshotserver_port_start + tmp_global_server_logical_idx, "server.snapshotserver");
		prepare_udpserver(server_snapshotdataserver_udpsock_list[i], true, server_snapshotdataserver_port_start + tmp_global_server_logical_idx, "server.snapshotdataserver", SOCKET_TIMEOUT, 0, UDP_LARGE_RCVBUFSIZE);
	}

	server_issnapshot_list = new std::atomic<bool>[current_server_logical_num];

	memory_fence();

	printf("[server] prepare end\n");
}

void close_server() {

	if (db_wrappers != NULL) {
		printf("Close rocksdb databases\n");
		delete [] db_wrappers;
		db_wrappers = NULL;
	}

	if (server_worker_udpsock_list != NULL) {
		delete [] server_worker_udpsock_list;
		server_worker_udpsock_list = NULL;
	}
	if (server_worker_lwpid_list != NULL) {
		delete [] server_worker_lwpid_list;
		server_worker_lwpid_list = NULL;
	}
	if (server_worker_pkt_ring_buffer_list != NULL) {
		delete [] server_worker_pkt_ring_buffer_list;
		server_worker_pkt_ring_buffer_list = NULL;
	}
	if (server_popclient_udpsock_list != NULL) {
		delete [] server_popclient_udpsock_list;
		server_popclient_udpsock_list = NULL;
	}
	/*if (server_cache_pop_ptr_queue_list != NULL) {
		delete [] server_cache_pop_ptr_queue_list;
		server_cache_pop_ptr_queue_list = NULL;
	}*/
	if (server_cached_keyset_list != NULL) {
		delete [] server_cached_keyset_list;
		server_cached_keyset_list = NULL;
	}
	if (server_evictserver_udpsock_list != NULL) {
		delete [] server_evictserver_udpsock_list;
		server_evictserver_udpsock_list = NULL;
	}
	if (server_snapshotserver_udpsock_list != NULL) {
		delete [] server_snapshotserver_udpsock_list;
		server_snapshotserver_udpsock_list = NULL;
	}
	if (server_snapshotdataserver_udpsock_list != NULL) {
		delete [] server_snapshotdataserver_udpsock_list;
		server_snapshotdataserver_udpsock_list = NULL;
	}
	if (server_issnapshot_list != NULL) {
		delete [] server_issnapshot_list;
		server_issnapshot_list = NULL;
	}
	/*if (server_evictserver_tcpsock_list != NULL) {
		delete [] server_evictserver_tcpsock_list;
		server_evictserver_tcpsock_list = NULL;
	}*/
}

/*void *run_server_popclient(void *param) {
  // Parse param
  uint16_t serveridx = *((uint16_t *)param); // [0, server_num-1]

  // NOTE: controller and switchos should have been launched before servers
  struct sockaddr_in controller_popserver_addr;
//  if (strcmp(controller_ip_for_server, server_ip_for_controller_list[server_physical_idx]) == 0) {
//	  set_sockaddr(controller_popserver_addr, inet_addr("127.0.0.1"), controller_popserver_port_start + global_server_logical_idx);
//  }
//  else {
//	  set_sockaddr(controller_popserver_addr, inet_addr(controller_ip_for_server), controller_popserver_port_start + global_server_logical_idx);
//  }
  set_sockaddr(controller_popserver_addr, inet_addr(controller_ip_for_server), controller_popserver_port_start + global_server_logical_idx);
  socklen_t controller_popserver_addrlen = sizeof(struct sockaddr_in);
  
  printf("[server.popclient%d] ready\n", int(serveridx));

  transaction_ready_threads++;

  while (!transaction_running) {}

  char buf[MAX_BUFSIZE];
  char recvbuf[MAX_BUFSIZE];
  int recvsize = 0;
  while (transaction_running) {
  	cache_pop_t *tmp_cache_pop_ptr = server_cache_pop_ptr_queue_list[serveridx].read();
  	if (tmp_cache_pop_ptr != NULL) {
		uint32_t popsize = tmp_cache_pop_ptr->serialize(buf, MAX_BUFSIZE);
		while (true) {
			send_cachepop(server_popclient_udpsock_list[serveridx], buf, popsize, controller_popserver_addr, controller_popserver_addrlen, recvbuf, MAX_BUFSIZE, recvsize);
			cache_pop_ack_t rsp(recvbuf, recvsize);
			INVARIANT(rsp.key() == tmp_cache_pop_ptr->key());
		}

		delete tmp_cache_pop_ptr;
		tmp_cache_pop_ptr = NULL;
	}
  }

  close(server_popclient_udpsock_list[serveridx]);
  pthread_exit(nullptr);
  return 0;
}*/

void send_cachepop(const int &sockfd, const char *buf, const int &buflen, const struct sockaddr_in &dstaddr, const socklen_t &dstaddrlen, char *recvbuf, const int& maxrecvbuflen, int &recvsize) {
	INVARIANT(buf != recvbuf && &buflen != &recvsize);
	while (true) {
		//printf("send CACHE_POP to controller\n");
		//dump_buf(buf, popsize);
		udpsendto(sockfd, buf, buflen, 0, &dstaddr, dstaddrlen, "server.popclient");

		// wait for CACHE_POP_ACK
		// NOTE: we do not wait for CACHE_POP_INSWITCH_ACK, as it needs to wait for finishing entire cache population workflow, and cannot utilize max thpt of switchos<->ptf
		bool is_timeout = udprecvfrom(sockfd, recvbuf, maxrecvbuflen, 0, NULL, NULL, recvsize, "server.popclient");
		if (unlikely(is_timeout)) {
			printf("Cache population timeout!\n");
			continue;
		}
		else {
			return;
		}
	}
}

/*
 * Worker for server-side processing 
 */

void *run_server_worker(void * param) {
  // Parse param
  server_worker_param_t &thread_param = *(server_worker_param_t *)param;
  uint16_t local_server_logical_idx = thread_param.local_server_logical_idx; // [0, current_server_logical_num - 1]
  uint16_t global_server_logical_idx = server_logical_idxes_list[server_physical_idx][local_server_logical_idx];

  // NOTE: pthread id != LWP id (linux thread id)
  server_worker_lwpid_list[local_server_logical_idx] = CUR_LWPID();
  
  pkt_ring_buffer_t &cur_worker_pkt_ring_buffer = server_worker_pkt_ring_buffer_list[local_server_logical_idx];

  // open rocksdb
  bool is_existing = db_wrappers[local_server_logical_idx].open(global_server_logical_idx);
  if (!is_existing) {
	  printf("[server.worker %d-%d] you need to run loader before server\n", local_server_logical_idx, global_server_logical_idx);
	  //exit(-1);
  }

  // packet headers (only needed by dpdk / raw socket)
  //uint8_t srcmac[6];
  //uint8_t dstmac[6];
  //char srcip[16];
  //char dstip[16];
  //uint16_t srcport;
  //uint16_t unused_dstport; // we use server_port_start instead of received dstport to hide server-side partition for client
  
  // client address (only needed by udp socket)
  // NOTE: udp socket uses binded port as server.srcport; if we use raw socket, we need to judge client.dstport in every worker, which is time-consumine; -> we resort switch to hide server.srcport
  struct sockaddr_in client_addr;
  socklen_t client_addrlen = sizeof(struct sockaddr_in);

  // NOTE: controller and switchos should have been launched before servers
  struct sockaddr_in controller_popserver_addr;
  /*if (strcmp(controller_ip_for_server, server_ip_for_controller_list[server_physical_idx]) == 0) {
	  set_sockaddr(controller_popserver_addr, inet_addr("127.0.0.1"), controller_popserver_port_start + global_server_logical_idx);
  }
  else {
	  set_sockaddr(controller_popserver_addr, inet_addr(controller_ip_for_server), controller_popserver_port_start + global_server_logical_idx);
  }*/
  set_sockaddr(controller_popserver_addr, inet_addr(controller_ip_for_server), controller_popserver_port_start + global_server_logical_idx);
  socklen_t controller_popserver_addrlen = sizeof(struct sockaddr_in);

  // scan.startkey <= max_startkey; scan.endkey >= min_startkey
  // use size_t to avoid int overflow
  uint64_t min_startkeyhihihi = global_server_logical_idx * perserver_keyrange;
  uint64_t max_endkeyhihihi = min_startkeyhihihi - 1 + perserver_keyrange;
  INVARIANT(min_startkeyhihihi >= std::numeric_limits<uint16_t>::min() && min_startkeyhihihi <= std::numeric_limits<uint16_t>::max());
  INVARIANT(max_endkeyhihihi >= std::numeric_limits<uint16_t>::min() && max_endkeyhihihi <= std::numeric_limits<uint16_t>::max());
  INVARIANT(max_endkeyhihihi >= min_startkeyhihihi);
  uint32_t min_startkeyhihi = min_startkeyhihihi << 16;
  uint32_t max_endkeyhihi = (max_endkeyhihihi << 16) | 0xFFFF;
  netreach_key_t min_startkey(0, 0, 0, min_startkeyhihi);
  netreach_key_t max_endkey(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), max_endkeyhihi);

  char buf[MAX_BUFSIZE];
  dynamic_array_t dynamicbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
  int recv_size = 0;
  int rsp_size = 0;
  char recvbuf[MAX_BUFSIZE];

  printf("[server.worker %d-%d] ready\n", local_server_logical_idx, global_server_logical_idx);
  transaction_ready_threads++;

  while (!transaction_running) {
  }

#ifdef DEBUG_SERVER
  struct timespec process_t1, process_t2, process_t3;
  struct timespec wait_t1, wait_t2, wait_t3, wait_beforerecv_t2, wait_beforerecv_t3;
  struct timespec udprecv_t1, udprecv_t2, udprecv_t3;
  struct timespec rocksdb_t1, rocksdb_t2, rocksdb_t3;
  struct timespec statistic_t1, statistic_t2, statistic_t3;
#endif
  bool is_first_pkt = true;
  bool is_timeout = false;
  while (transaction_running) {

#ifdef DEBUG_SERVER
	if (!is_first_pkt && !is_timeout) {
		CUR_TIME(udprecv_t1);

		CUR_TIME(wait_beforerecv_t2);
		DELTA_TIME(wait_beforerecv_t2, wait_t1, wait_beforerecv_t3);
		thread_param.wait_beforerecv_latency_list.push_back(GET_MICROSECOND(wait_beforerecv_t3));
	}
#endif

	//bool is_timeout = udprecvfrom(server_worker_udpsock_list[local_server_logical_idx], buf, MAX_BUFSIZE, 0, &client_addr, &client_addrlen, recv_size, "server.worker");
	dynamicbuf.clear();
	is_timeout = udprecvlarge_ipfrag(server_worker_udpsock_list[local_server_logical_idx], dynamicbuf, 0, &client_addr, &client_addrlen, "server.worker", &cur_worker_pkt_ring_buffer);
	recv_size = dynamicbuf.size();
	if (is_timeout) {
		/*if (!is_first_pkt) {
			printf("timeout\n");
		}*/
		continue; // continue to check transaction_running
	}

#ifdef DEBUG_SERVER
	if (!is_first_pkt) {
		CUR_TIME(wait_t2);
		DELTA_TIME(wait_t2, wait_t1, wait_t3);
		thread_param.wait_latency_list.push_back(GET_MICROSECOND(wait_t3));

		CUR_TIME(udprecv_t2);
		DELTA_TIME(udprecv_t2, udprecv_t1, udprecv_t3);
		thread_param.udprecv_latency_list.push_back(GET_MICROSECOND(udprecv_t3));
	}
	CUR_TIME(process_t1);
#endif

	//packet_type_t pkt_type = get_packet_type(buf, recv_size);
	packet_type_t pkt_type = get_packet_type(dynamicbuf.array(), recv_size);
	switch (pkt_type) {
		case packet_type_t::GETREQ: 
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif
				get_request_t req(dynamicbuf.array(), recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string())
				val_t tmp_val;
				uint32_t tmp_seq = 0;
				bool tmp_stat = db_wrappers[local_server_logical_idx].get(req.key(), tmp_val, tmp_seq);
				//COUT_THIS("[server] val = " << tmp_val.to_string())

				if (tmp_val.val_length <= val_t::SWITCH_MAX_VALLEN) {
					get_response_t rsp(req.key(), tmp_val, tmp_stat, global_server_logical_idx);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
					dump_buf(buf, rsp_size);
#endif
				}
				else {
					get_response_largevalue_t rsp(req.key(), tmp_val, tmp_stat, global_server_logical_idx);
					dynamicbuf.clear()
					rsp_size = rsp.serialize(dynamicbuf);
					udpsendlarge_ipfrag(server_worker_udpsock_list[local_server_logical_idx], dynamicbuf.array(), rsp_size, 0, &client_addr, client_addrlen, "server.worker", get_response_largevalue_t::get_frag_hdrsize());
#ifdef DUMP_BUF
					dump_buf(dynamicbuf.array(), rsp_size);
#endif
				}
				break;
			}
		case packet_type_t::GETREQ_NLATEST:
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif
				get_request_nlatest_t req(dynamicbuf.array(), recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string())
				val_t tmp_val;
				uint32_t tmp_seq = 0;
				bool tmp_stat = db_wrappers[local_server_logical_idx].get(req.key(), tmp_val, tmp_seq);
				//COUT_THIS("[server] val = " << tmp_val.to_string())

				if (tmp_val.val_length <= val_t::SWITCH_MAX_VALLEN) {
					if (tmp_stat) { // key exists
						get_response_latest_seq_t rsp(req.key(), tmp_val, tmp_seq, global_server_logical_idx);
						rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					}
					else { // key not exist
						get_response_deleted_seq_t rsp(req.key(), tmp_val, tmp_seq, global_server_logical_idx);
						rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					}
					udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
					dump_buf(buf, rsp_size);
#endif
				}
				else {
					get_response_largevalue_t rsp(req.key(), tmp_val, tmp_stat, global_server_logical_idx);
					dynamicbuf.clear()
					rsp_size = rsp.serialize(dynamicbuf);
					udpsendlarge_ipfrag(server_worker_udpsock_list[local_server_logical_idx], dynamicbuf.array(), rsp_size, 0, &client_addr, client_addrlen, "server.worker", get_response_largevalue_t::get_frag_hdrsize());
#ifdef DUMP_BUF
					dump_buf(dynamicbuf.array(), rsp_size);
#endif
				}
				break;
			}
		case packet_type_t::PUTREQ_SEQ:
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif

#ifdef DEBUG_SERVER
				CUR_TIME(rocksdb_t1);
#endif

				put_request_seq_t req(dynamicbuf.array(), recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
				bool tmp_stat = db_wrappers[local_server_logical_idx].put(req.key(), req.val(), req.seq());
				UNUSED(tmp_stat);
				//COUT_THIS("[server] stat = " << tmp_stat)
				
#ifdef DEBUG_SERVER
				CUR_TIME(rocksdb_t2);
#endif
				
				put_response_t rsp(req.key(), true, global_server_logical_idx);
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::PUTREQ_LARGEVALUE_SEQ:
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif

#ifdef DEBUG_SERVER
				CUR_TIME(rocksdb_t1);
#endif

				put_request_largevalue_seq_t req(dynamicbuf.array(), recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
				bool tmp_stat = db_wrappers[local_server_logical_idx].put(req.key(), req.val(), req.seq());
				UNUSED(tmp_stat);
				//COUT_THIS("[server] stat = " << tmp_stat)
				
#ifdef DEBUG_SERVER
				CUR_TIME(rocksdb_t2);
#endif
				
				put_response_t rsp(req.key(), true, global_server_logical_idx);
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::DELREQ_SEQ:
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif
				del_request_seq_t req(dynamicbuf.array(), recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string())
				bool tmp_stat = db_wrappers[local_server_logical_idx].remove(req.key(), req.seq());
				UNUSED(tmp_stat);
				//COUT_THIS("[server] stat = " << tmp_stat)
				del_response_t rsp(req.key(), true, global_server_logical_idx);
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::SCANREQ_SPLIT:
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif
				scan_request_split_t req(dynamicbuf.array(), recv_size);
				
				// get verified key range
				INVARIANT(req.key() <= max_endkey);
				INVARIANT(req.endkey() >= min_startkey);
				netreach_key_t cur_startkey = req.key();
				netreach_key_t cur_endkey = req.endkey();
				if (cur_startkey < min_startkey) {
					cur_startkey = min_startkey;
				}
				if (cur_endkey > max_endkey) {
					cur_endkey = max_endkey;
				}

				std::vector<std::pair<netreach_key_t, snapshot_record_t>> results;
				db_wrappers[local_server_logical_idx].range_scan(cur_startkey, cur_endkey, results);

				//COUT_THIS("results size: " << results.size());

				scan_response_split_t rsp(req.key(), req.endkey(), req.cur_scanidx(), req.max_scannum(), global_server_logical_idx, db_wrappers[local_server_logical_idx].get_snapshotid(), results.size(), results);
				//rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				//udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
				dynamicbuf.clear();
				rsp_size = rsp.dynamic_serialize(dynamicbuf);
				udpsendlarge_ipfrag(server_worker_udpsock_list[local_server_logical_idx], dynamicbuf.array(), rsp_size, 0, &client_addr, client_addrlen, "server.worker", scan_response_split_t::get_frag_hdrsize());
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), rsp_size);
#endif
				break;
			}
		case packet_type_t::GETREQ_POP: 
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif
				get_request_pop_t req(dynamicbuf.array(), recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string())
				val_t tmp_val;
				uint32_t tmp_seq = 0;
				bool tmp_stat = db_wrappers[local_server_logical_idx].get(req.key(), tmp_val, tmp_seq);
				//COUT_THIS("[server] val = " << tmp_val.to_string())
				
				if (tmp_val.val_length <= val_t::SWITCH_MAX_VALLEN) {
					get_response_t rsp(req.key(), tmp_val, tmp_stat, global_server_logical_idx);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
					udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
					dump_buf(buf, rsp_size);
#endif

					// Trigger cache population if necessary (key exist and not being cached)
					if (workload_mode != 0) {
						bool is_cached_before = server_cached_keyset_list[local_server_logical_idx].is_exist(req.key());
						if (!is_cached_before) {
							server_cached_keyset_list[local_server_logical_idx].insert(req.key());
							// Send CACHE_POP to server.popclient
							//cache_pop_t cache_pop_req_ptr = new cache_pop_t(req.key(), tmp_val, tmp_seq, tmp_stat, global_server_logical_idx); // freed by server.popclient
							//server_cache_pop_ptr_queue_list[serveridx].write(cache_pop_req_ptr);
							
							cache_pop_t cache_pop_req(req.key(), tmp_val, tmp_seq, tmp_stat, global_server_logical_idx);
							rsp_size = cache_pop_req.serialize(buf, MAX_BUFSIZE);
							send_cachepop(server_popclient_udpsock_list[local_server_logical_idx], buf, rsp_size, controller_popserver_addr, controller_popserver_addrlen, recvbuf, MAX_BUFSIZE, recv_size);
							cache_pop_ack_t cache_pop_rsp(recvbuf, recv_size);
							INVARIANT(cache_pop_rsp.key() == cache_pop_req.key());
						}
					}
				}
				else {
					get_response_largevalue_t rsp(req.key(), tmp_val, tmp_stat, global_server_logical_idx);
					dynamicbuf.clear()
					rsp_size = rsp.serialize(dynamicbuf);
					udpsendlarge_ipfrag(server_worker_udpsock_list[local_server_logical_idx], dynamicbuf.array(), rsp_size, 0, &client_addr, client_addrlen, "server.worker", get_response_largevalue_t::get_frag_hdrsize());
#ifdef DUMP_BUF
					dump_buf(dynamicbuf.array(), rsp_size);
#endif
				}
				break;
			}
		case packet_type_t::PUTREQ_POP_SEQ: 
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif

#ifdef DEBUG_SERVER
				CUR_TIME(rocksdb_t1);
#endif

				put_request_pop_seq_t req(dynamicbuf.array(), recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string())
				bool tmp_stat = db_wrappers[local_server_logical_idx].put(req.key(), req.val(), req.seq());
				//COUT_THIS("[server] val = " << req.val().to_string())
			
#ifdef DEBUG_SERVER
				CUR_TIME(rocksdb_t2);
#endif
				
				put_response_t rsp(req.key(), true, global_server_logical_idx);
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif

				// Trigger cache population if necessary (key exist and not being cached)
				if (workload_mode != 0) { // successful put
					bool is_cached_before = server_cached_keyset_list[local_server_logical_idx].is_exist(req.key());
					if (!is_cached_before) {
						server_cached_keyset_list[local_server_logical_idx].insert(req.key());
						// Send CACHE_POP to server.popclient
						//cache_pop_t *cache_pop_req_ptr = new cache_pop_t(req.key(), req.val(), req.seq(), tmp_stat, global_server_logical_idx); // freed by server.popclient
						//server_cache_pop_ptr_queue_list[local_server_logical_idx].write(cache_pop_req_ptr);
						
						cache_pop_t cache_pop_req(req.key(), req.val(), req.seq(), tmp_stat, global_server_logical_idx);
						rsp_size = cache_pop_req.serialize(buf, MAX_BUFSIZE);
						send_cachepop(server_popclient_udpsock_list[local_server_logical_idx], buf, rsp_size, controller_popserver_addr, controller_popserver_addrlen, recvbuf, MAX_BUFSIZE, recv_size);
						cache_pop_ack_t cache_pop_rsp(recvbuf, recv_size);
						INVARIANT(cache_pop_rsp.key() == cache_pop_req.key());
					}
				}
				break;
			}
		case packet_type_t::PUTREQ_SEQ_CASE3:
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif
				//COUT_THIS("PUTREQ_SEQ_CASE3")
				put_request_seq_case3_t req(dynamicbuf.array(), recv_size);

				if (!server_issnapshot_list[local_server_logical_idx]) {
					db_wrappers[local_server_logical_idx].make_snapshot();
				}

				bool tmp_stat = db_wrappers[local_server_logical_idx].put(req.key(), req.val(), req.seq());
				UNUSED(tmp_stat);
				//put_response_case3_t rsp(req.hashidx(), req.key(), serveridx, tmp_stat); // no case3_reg in switch
				put_response_t rsp(req.key(), true, global_server_logical_idx);
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::PUTREQ_POP_SEQ_CASE3: 
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif
				//COUT_THIS("PUTREQ_POP_SEQ_CASE3")
				put_request_pop_seq_case3_t req(dynamicbuf.array(), recv_size);

				if (!server_issnapshot_list[local_server_logical_idx]) {
					db_wrappers[local_server_logical_idx].make_snapshot();
				}

				//COUT_THIS("[server] key = " << req.key().to_string())
				bool tmp_stat = db_wrappers[local_server_logical_idx].put(req.key(), req.val(), req.seq());
				//COUT_THIS("[server] val = " << tmp_val.to_string())
				put_response_t rsp(req.key(), true, global_server_logical_idx);
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif

				// Trigger cache population if necessary (key exist and not being cached)
				if (workload_mode != 0) { // successful put
					bool is_cached_before = server_cached_keyset_list[local_server_logical_idx].is_exist(req.key());
					if (!is_cached_before) {
						server_cached_keyset_list[local_server_logical_idx].insert(req.key());
						// Send CACHE_POP to server.popclient
						//cache_pop_t *cache_pop_req_ptr = new cache_pop_t(req.key(), req.val(), req.seq(), tmp_stat, global_server_logical_idx); // freed by server.popclient
						//server_cache_pop_ptr_queue_list[local_server_logical_idx].write(cache_pop_req_ptr);
						
						cache_pop_t cache_pop_req(req.key(), req.val(), req.seq(), tmp_stat, global_server_logical_idx);
						rsp_size = cache_pop_req.serialize(buf, MAX_BUFSIZE);
						send_cachepop(server_popclient_udpsock_list[local_server_logical_idx], buf, rsp_size, controller_popserver_addr, controller_popserver_addrlen, recvbuf, MAX_BUFSIZE, recv_size);
						cache_pop_ack_t cache_pop_rsp(recvbuf, recv_size);
						INVARIANT(cache_pop_rsp.key() == cache_pop_req.key());
					}
				}
				break;
			}
		case packet_type_t::DELREQ_SEQ_CASE3:
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif
				//COUT_THIS("DELREQ_SEQ_CASE3")
				del_request_seq_case3_t req(dynamicbuf.array(), recv_size);

				if (!server_issnapshot_list[local_server_logical_idx]) {
					db_wrappers[local_server_logical_idx].make_snapshot();
				}

				bool tmp_stat = db_wrappers[local_server_logical_idx].remove(req.key(), req.seq());
				UNUSED(tmp_stat);
				//del_response_case3_t rsp(req.hashidx(), req.key(), serveridx, tmp_stat); // no case3_reg in switch
				del_response_t rsp(req.key(), true, global_server_logical_idx);
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::WARMUPREQ:
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif
				warmup_request_t req(dynamicbuf.array(), recv_size);

				//uint32_t tmp_seq = 0;
				// NOTE: we do not need to write the key-value pair for WARMUPREQ except cache population
				//bool tmp_stat = db_wrappers[serveridx].force_put(req.key(), req.val());
				//INVARIANT(tmp_stat == true);
				
				val_t tmp_val;
				uint32_t tmp_seq = 0;
				bool tmp_stat = db_wrappers[local_server_logical_idx].get(req.key(), tmp_val, tmp_seq);
				
				warmup_ack_t rsp(req.key());
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif

				// Trigger cache population if necessary (key exist and not being cached)
				bool is_cached_before = server_cached_keyset_list[local_server_logical_idx].is_exist(req.key());
				INVARIANT(!is_cached_before);
				server_cached_keyset_list[local_server_logical_idx].insert(req.key());
				// Send CACHE_POP to server.popclient
				//cache_pop_t *cache_pop_req_ptr = new cache_pop_t(req.key(), tmp_val, tmp_seq, tmp_stat, serveridx); // freed by server.popclient
				//server_cache_pop_ptr_queue_list[serveridx].write(cache_pop_req_ptr);
						
				cache_pop_t cache_pop_req(req.key(), tmp_val, tmp_seq, tmp_stat, global_server_logical_idx);
				rsp_size = cache_pop_req.serialize(buf, MAX_BUFSIZE);
				send_cachepop(server_popclient_udpsock_list[local_server_logical_idx], buf, rsp_size, controller_popserver_addr, controller_popserver_addrlen, recvbuf, MAX_BUFSIZE, recv_size);
				cache_pop_ack_t cache_pop_rsp(recvbuf, recv_size);
				INVARIANT(cache_pop_rsp.key() == cache_pop_req.key());
				break;
			}
		case packet_type_t::LOADREQ:
			{
#ifdef DUMP_BUF
				dump_buf(dynamicbuf.array(), recv_size);
#endif
				load_request_t req(dynamicbuf.array(), recv_size);

				//char val_buf[Val::SWITCH_MAX_VALLEN];
				//memset(val_buf, 0x11, Val::SWITCH_MAX_VALLEN);
				//val_t tmp_val(val_buf, Val::SWITCH_MAX_VALLEN);
				bool tmp_stat = db_wrappers[local_server_logical_idx].force_put(req.key(), req.val());
				UNUSED(tmp_stat);
				
				load_ack_t rsp(req.key());
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		default:
			{
				COUT_THIS("[server.worker] Invalid packet type: " << int(pkt_type))
				std::cout << std::flush;
				exit(-1);
			}
	}

	if (likely(pkt_type != packet_type_t::WARMUPREQ && pkt_type != packet_type_t::LOADREQ)) {
#ifdef DEBUG_SERVER
		CUR_TIME(process_t2);
		DELTA_TIME(process_t2, process_t1, process_t3);
		thread_param.process_latency_list.push_back(GET_MICROSECOND(process_t3));

		DELTA_TIME(rocksdb_t2, rocksdb_t1, rocksdb_t3);
		thread_param.rocksdb_latency_list.push_back(GET_MICROSECOND(rocksdb_t3));

		CUR_TIME(wait_t1);
#endif
		is_first_pkt = false;
	}

  } // end of while(transaction_running)

  close(server_worker_udpsock_list[local_server_logical_idx]);
  printf("[server.worker %d-%d] exits", local_server_logical_idx, global_server_logical_idx);
  pthread_exit(nullptr);
}

void *run_server_evictserver(void *param) {
	uint16_t local_server_logical_idx = *((uint16_t *)param);
	uint16_t global_server_logical_idx = server_logical_idxes_list[server_physical_idx][local_server_logical_idx];

	struct sockaddr_in controller_evictclient_addr;
	unsigned int controller_evictclient_addrlen = sizeof(struct sockaddr_in);
	//bool with_controller_evictclient_addr = false;
	
	printf("[server.evictserver %d-%d] ready\n", local_server_logical_idx, global_server_logical_idx);
	transaction_ready_threads++;

	while (!transaction_running) {}

	// process CACHE_EVICT/_CASE2 packet <optype, key, vallen, value, result, seq, serveridx>
	char recvbuf[MAX_BUFSIZE];
	int recvsize = 0;
	bool is_timeout = false;
	char sendbuf[MAX_BUFSIZE]; // used to send CACHE_EVICT_ACK to controller
	while (transaction_running) {
		is_timeout = udprecvfrom(server_evictserver_udpsock_list[local_server_logical_idx], recvbuf, MAX_BUFSIZE, 0, &controller_evictclient_addr, &controller_evictclient_addrlen, recvsize, "server.evictserver");
		if (is_timeout) {
			memset(&controller_evictclient_addr, 0, sizeof(struct sockaddr_in));
			controller_evictclient_addrlen = sizeof(struct sockaddr_in);
			continue; // continue to check transaction_running
		}

		//printf("receive CACHE_EVICT from controller\n");
		//dump_buf(recvbuf, recvsize);
		cache_evict_t *tmp_cache_evict_ptr;
		packet_type_t optype = get_packet_type(recvbuf, recvsize);
		if (optype == packet_type_t::CACHE_EVICT) {
			tmp_cache_evict_ptr = new cache_evict_t(recvbuf, recvsize);
		}
		else if (optype == packet_type_t::CACHE_EVICT_CASE2) {
			tmp_cache_evict_ptr = new cache_evict_case2_t(recvbuf, recvsize);
		}
		else {
			printf("[server.evictserver] error: invalid optype: %d\n", int(optype));
			exit(-1);
		}

		uint16_t tmp_serveridx = tmp_cache_evict_ptr->serveridx();
		INVARIANT(tmp_serveridx == global_server_logical_idx);
		//INVARIANT(server_cached_keyset_list[local_server_logical_idx].is_exist(tmp_cache_evict_ptr->key()));
		if (!server_cached_keyset_list[local_server_logical_idx].is_exist(tmp_cache_evict_ptr->key())) {
			printf("[ERROR] server %d-%d does not cache key %x whose expected server is %d\n",\
					local_server_logical_idx, global_server_logical_idx, tmp_cache_evict_ptr->key().keyhihi,\
					tmp_cache_evict_ptr->key().get_hashpartition_idx(switch_partition_count, max_server_total_logical_num));
			exit(-1);
		}

		// make server-side snapshot for CACHE_EVICT_CASE2
		if (packet_type_t(optype) == packet_type_t::CACHE_EVICT_CASE2) {
			//printf("CACHE_EVICT_CASE2!\n");
			if (!server_issnapshot_list[local_server_logical_idx]) {
				db_wrappers[local_server_logical_idx].make_snapshot();
			}
		}

		// remove from cached keyset
		server_cached_keyset_list[local_server_logical_idx].erase(tmp_cache_evict_ptr->key()); // NOTE: no contention

		// update in-memory KVS if necessary
		// NOTE: we need to check seq to avoid from overwriting normal data
		if (tmp_cache_evict_ptr->stat()) { // put
			db_wrappers[local_server_logical_idx].put(tmp_cache_evict_ptr->key(), tmp_cache_evict_ptr->val(), tmp_cache_evict_ptr->seq(), true);
		}
		else { // del
			db_wrappers[local_server_logical_idx].remove(tmp_cache_evict_ptr->key(), tmp_cache_evict_ptr->seq(), true);
		}

		// send CACHE_EVICT_ACK to controller.evictserver.evictclient
		cache_evict_ack_t tmp_cache_evict_ack(tmp_cache_evict_ptr->key());
		int sendsize = tmp_cache_evict_ack.serialize(sendbuf, MAX_BUFSIZE);
		//printf("send CACHE_EVICT_ACK to controller\n");
		//dump_buf(sendbuf, sendsize);
		udpsendto(server_evictserver_udpsock_list[local_server_logical_idx], sendbuf, sendsize, 0, &controller_evictclient_addr, controller_evictclient_addrlen, "server.evictserver");

		// free CACHE_EVIT
		delete tmp_cache_evict_ptr;
		tmp_cache_evict_ptr = NULL;
	}
	close(server_evictserver_udpsock_list[local_server_logical_idx]);
	pthread_exit(nullptr);
}

void *run_server_snapshotserver(void *param) {
	uint16_t local_server_logical_idx = *((uint16_t *)param);
	uint16_t global_server_logical_idx = server_logical_idxes_list[server_physical_idx][local_server_logical_idx];

	struct sockaddr_in controller_snapshotclient_addr;
	socklen_t controller_snapshotclient_addrlen = sizeof(struct sockaddr_in);
	//bool with_controller_snapshotclient_addr = false;

	printf("[server.snapshotserver %d-%d] ready\n", local_server_logical_idx, global_server_logical_idx);
	transaction_ready_threads += 1;

	while (!transaction_running) {}

	char recvbuf[MAX_BUFSIZE];
	int recvsize = 0;
	bool is_timeout = false;
	int control_type = -1;
	int snapshotid = -1;
	while (transaction_running) {
		/*if (!with_controller_snapshotclient_addr) {
			is_timeout = udprecvfrom(server_snapshotserver_udpsock_list[serveridx], recvbuf, MAX_BUFSIZE, 0, &controller_snapshotclient_addr, &controller_snapshotclient_addrlen, recvsize, "server.snapshotserver");
			if (!is_timeout) {
				with_controller_snapshotclient_addr = true;
			}
		}
		else {
			is_timeout = udprecvfrom(server_snapshotserver_udpsock_list[serveridx], recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "server.snapshotserver");
		}*/
		is_timeout = udprecvfrom(server_snapshotserver_udpsock_list[local_server_logical_idx], recvbuf, MAX_BUFSIZE, 0, &controller_snapshotclient_addr, &controller_snapshotclient_addrlen, recvsize, "server.snapshotserver");

		if (is_timeout) {
			memset(&controller_snapshotclient_addr, 0, sizeof(struct sockaddr_in));
			controller_snapshotclient_addrlen = sizeof(struct sockaddr_in);
			continue;
		}

		// Fix duplicate packet
		if (control_type == *((int *)recvbuf) && snapshotid == *((int *)(recvbuf + sizeof(int)))) {
			printf("[server.snapshotserver] receive duplicate control type %d for snapshot id %d\n", control_type, snapshotid); // TMPDEBUG
			continue;
		}
		else {
			control_type = *((int *)recvbuf);
			snapshotid = *((int *)(recvbuf + sizeof(int)));
		}

		if (control_type == SNAPSHOT_CLEANUP) {
			// cleanup stale snapshot states
			db_wrappers[local_server_logical_idx].clean_snapshot(snapshotid);

			// enable making server-side snapshot for new snapshot period
			server_issnapshot_list[local_server_logical_idx] = false;
			db_wrappers[local_server_logical_idx].stop_snapshot();
			
			// sendback SNAPSHOT_CLEANUP_ACK to controller
			udpsendto(server_snapshotserver_udpsock_list[local_server_logical_idx], &SNAPSHOT_CLEANUP_ACK, sizeof(int), 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "server.snapshotserver");
		}
		else if (control_type == SNAPSHOT_START) {
			INVARIANT(!server_issnapshot_list[local_server_logical_idx]);
			db_wrappers[local_server_logical_idx].make_snapshot(snapshotid);
			
			// sendback SNAPSHOT_START_ACK to controller
			udpsendto(server_snapshotserver_udpsock_list[local_server_logical_idx], &SNAPSHOT_START_ACK, sizeof(int), 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "server.snapshotserver");
		}
		else {
			printf("[server.snapshotserver] invalid control type: %d\n", control_type);
			exit(-1);
		}
	}

	close(server_snapshotserver_udpsock_list[local_server_logical_idx]);
	pthread_exit(nullptr);
}

void *run_server_snapshotdataserver(void *param) {
	uint16_t local_server_logical_idx = *((uint16_t *)param);
	uint16_t global_server_logical_idx = server_logical_idxes_list[server_physical_idx][local_server_logical_idx];

	struct sockaddr_in controller_snapshotclient_addr;
	socklen_t controller_snapshotclient_addrlen;
	//bool with_controller_snapshotclient_addr = false;

	// receive per-server snapshot data from controller
	dynamic_array_t recvbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);

	printf("[server.snapshotdataserver %d-%d] ready\n", local_server_logical_idx, global_server_logical_idx);
	transaction_ready_threads += 1;

	while (!transaction_running) {}

	bool is_timeout = false;
	int control_type = -1;
	int snapshotid = -1;
	while (transaction_running) {
		/*if (!with_controller_snapshotclient_addr) {
			is_timeout = udprecvlarge_udpfrag(server_snapshotdataserver_udpsock_list[serveridx], recvbuf, 0, &controller_snapshotclient_addr, &controller_snapshotclient_addrlen, "server.snapshotdataserver");
			if (!is_timeout) {
				with_controller_snapshotclient_addr = true;
			}
		}
		else {
			is_timeout = udprecvlarge_udpfrag(server_snapshotdataserver_udpsock_list[serveridx], recvbuf, 0, NULL, NULL, "server.snapshotdataserver");
		}*/

		recvbuf.clear();
		is_timeout = udprecvlarge_udpfrag(server_snapshotdataserver_udpsock_list[local_server_logical_idx], recvbuf, 0, &controller_snapshotclient_addr, &controller_snapshotclient_addrlen, "server.snapshotdataserver");
		if (is_timeout) {
			memset(&controller_snapshotclient_addr, 0, sizeof(struct sockaddr_in));
			controller_snapshotclient_addrlen = sizeof(struct sockaddr_in);
			continue;
		}

		// Fix duplicate packet
		if (control_type == *((int *)recvbuf.array()) && snapshotid == *((int *)(recvbuf.array() + sizeof(int)))) {
			printf("[server.snapshotdataserver] receive duplicate control type %d for snapshot id %d\n", control_type, snapshotid); // TMPDEBUG
			continue;
		}
		else {
			control_type = *((int *)recvbuf.array());
			snapshotid = *((int *)(recvbuf.array() + sizeof(int)));
		}

		if (control_type == SNAPSHOT_SENDDATA) {
			// sendback SNAPSHOT_SENDDATA_ACK to controller
			udpsendto(server_snapshotdataserver_udpsock_list[local_server_logical_idx], &SNAPSHOT_SENDDATA_ACK, sizeof(int), 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "server.snapshotdataserver");

			// per-server snapshot data: <int SNAPSHOT_SENDDATA, int snapshotid, int32_t perserver_bytes, uint16_t serveridx, int32_t record_cnt, per-record data>
			// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>
			server_issnapshot_list[local_server_logical_idx] = true;

			// parse in-switch snapshot data
			int32_t tmp_serverbytes = *((int32_t *)(recvbuf.array() + sizeof(int) + sizeof(int)));
			INVARIANT(recvbuf.size() == tmp_serverbytes);
			uint16_t tmp_serveridx = *((uint16_t *)(recvbuf.array() + sizeof(int) + sizeof(int) + sizeof(int32_t)));
			INVARIANT(tmp_serveridx == global_server_logical_idx);
			int32_t tmp_recordcnt = *((int32_t *)(recvbuf.array() + sizeof(int) + sizeof(int) + sizeof(int32_t) + sizeof(uint16_t)));

			std::map<netreach_key_t, snapshot_record_t> tmp_inswitch_snapshot;
			int tmp_offset = sizeof(int) + sizeof(int) + sizeof(int32_t) + sizeof(uint16_t) + sizeof(int32_t);
			for (int32_t tmp_recordidx = 0; tmp_recordidx < tmp_recordcnt; tmp_recordidx++) {
				netreach_key_t tmp_key;
				snapshot_record_t tmp_record;
				uint32_t tmp_keysize = tmp_key.deserialize(recvbuf.array() + tmp_offset, recvbuf.size() - tmp_offset);
				tmp_offset += tmp_keysize;
				uint32_t tmp_valsize = tmp_record.val.deserialize(recvbuf.array() + tmp_offset, recvbuf.size() - tmp_offset);
				tmp_offset += tmp_valsize;
				tmp_record.seq = *((uint32_t *)(recvbuf.array() + tmp_offset));
				tmp_offset += sizeof(uint32_t);
				tmp_record.stat = *((bool *)(recvbuf.array() + tmp_offset));
				tmp_offset += sizeof(bool);
				tmp_inswitch_snapshot.insert(std::pair<netreach_key_t, snapshot_record_t>(tmp_key, tmp_record));
			}

#ifdef DEBUG_SNAPSHOT
			// TMPDEBUG
			printf("[server.snapshotdataserver %d-%d] receive snapshot data of size %d from controller\n", local_server_logical_idx, global_server_logical_idx, tmp_inswitch_snapshot.size());
			/*int debugi = 0;
			for (std::map<netreach_key_t, snapshot_record_t>::iterator iter = tmp_inswitch_snapshot.begin();
					iter != tmp_inswitch_snapshot.end(); iter++) {
				char debugbuf[MAX_BUFSIZE];
				uint32_t debugkeysize = iter->first.serialize(debugbuf, MAX_BUFSIZE);
				uint32_t debugvalsize = iter->second.val.serialize(debugbuf + debugkeysize, MAX_BUFSIZE - debugkeysize);
				printf("serialized key-value %d:\n", debugi);
				dump_buf(debugbuf, debugkeysize+debugvalsize);
				printf("seq: %d, stat %d\n", iter->second.seq, iter->second.stat?1:0);
			}*/
#endif

			// update in-switch and server-side snapshot
			db_wrappers[local_server_logical_idx].update_snapshot(tmp_inswitch_snapshot, snapshotid);
			
			// stop current snapshot to enable making next server-side snapshot
			db_wrappers[local_server_logical_idx].stop_snapshot();
			server_issnapshot_list[local_server_logical_idx] = false;
		}
		else {
			printf("[server.snapshotdataserver] invalid control type: %d\n", control_type);
			exit(-1);
		}
	}

	close(server_snapshotdataserver_udpsock_list[local_server_logical_idx]);
	pthread_exit(nullptr);
}

#endif
