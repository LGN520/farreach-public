#ifndef SERVER_IMPL_H
#define SERVER_IMPL_H

// Transaction phase for ycsb_server

#include <map>
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

//#define DUMP_BUF

//volatile bool first_cachepop = true;

typedef ConcurrentSet<netreach_key_t> concurrent_set_t;

struct alignas(CACHELINE_SIZE) ServerWorkerParam {
  uint16_t local_server_logical_idx;
#ifdef DEBUG_SERVER
  std::vector<double> process_latency_list;
  std::vector<double> wait_latency_list;
  std::vector<double> wait_beforerecv_latency_list;
  std::vector<double> udprecv_latency_list;
  std::vector<double> rocksdb_latency_list;
  std::vector<double> beingcached_latency_list;
  std::vector<double> beingupdated_latency_list;
#endif
};
typedef ServerWorkerParam server_worker_param_t;

RocksdbWrapper *db_wrappers = NULL;
int * server_worker_udpsock_list = NULL;
int * server_worker_invalidateserver_udpsock_list = NULL;
int * server_worker_lwpid_list = NULL;

// Per-server popserver <-> controller.popserver.popclient
int * server_popserver_udpsock_list = NULL;
// access all of beingcached/cached/beingupdated keysets should be atomic
std::mutex *server_mutex_for_keyset_list = NULL;
std::map<netreach_key_t, uint16_t> *server_cached_keyidxmap_list = NULL;
std::set<netreach_key_t> *server_beingupdated_keyset_list = NULL;

// server.evictservers <-> controller.evictserver.evictclients
//int * server_evictserver_tcpsock_list = NULL;
// server.evictserver <-> controller.evictserver.evictclient
int *server_evictserver_udpsock_list = NULL;

// data plane <-> per-server.valueupdateserver
//MessagePtrQueue<netcache_valueupdate_t> *server_netcache_valueupdate_ptr_queue_list = NULL;
MessagePtrQueue<distcache_spine_valueupdate_inswitch_t> *server_distcache_spine_valueupdate_inswitch_ptr_queue_list = NULL;
int *server_valueupdateserver_udpsock_list = NULL;

void prepare_server();
// server.workers for processing pkts
void *run_server_worker(void *param);
void *run_server_popserver(void *param);
void *run_server_evictserver(void *param);
void *run_server_valueupdateserver(void *param);
void close_server();

void prepare_server() {
	printf("[server] prepare start\n");

	RocksdbWrapper::prepare_rocksdb();

	uint32_t current_server_logical_num = server_logical_idxes_list[server_physical_idx].size();

	db_wrappers = new RocksdbWrapper[current_server_logical_num];
	INVARIANT(db_wrappers != NULL);

	server_worker_udpsock_list = new int[current_server_logical_num];
	server_worker_invalidateserver_udpsock_list = new int[current_server_logical_num];
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
		prepare_udpserver(server_worker_udpsock_list[tmp_local_server_logical_idx], true, tmp_server_worker_port, "server.worker", SOCKET_TIMEOUT, 0, UDP_LARGE_RCVBUFSIZE);
		printf("prepare udp socket for server.worker %d-%d on port %d\n", tmp_local_server_logical_idx, tmp_global_server_logical_idx, server_worker_port_start + tmp_local_server_logical_idx);

		prepare_udpserver(server_worker_invalidateserver_udpsock_list[tmp_local_server_logical_idx], true, server_invalidateserver_port_start + tmp_global_server_logical_idx, "server.worker.invalidateserver");
	}
	server_worker_lwpid_list = new int[current_server_logical_num];
	memset(server_worker_lwpid_list, 0, current_server_logical_num);

	// Prepare for cache population
	server_popserver_udpsock_list = new int[current_server_logical_num];
	server_mutex_for_keyset_list = new std::mutex[current_server_logical_num];
	server_cached_keyidxmap_list = new std::map<netreach_key_t, uint16_t>[current_server_logical_num];
	server_beingupdated_keyset_list = new std::set<netreach_key_t>[current_server_logical_num];
	for (size_t i = 0; i < current_server_logical_num; i++) {
		prepare_udpserver(server_popserver_udpsock_list[i], true, server_popserver_port_start + server_logical_idxes_list[server_physical_idx][i], "server.popserver");
		server_cached_keyidxmap_list[i].clear();
		server_beingupdated_keyset_list[i].clear();
	}

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

	// prepare for inswitch value update
	//server_netcache_valueupdate_ptr_queue_list = new MessagePtrQueue<netcache_valueupdate_t>[current_server_logical_num];
	server_distcache_spine_valueupdate_inswitch_ptr_queue_list = new MessagePtrQueue<distcache_spine_valueupdate_inswitch_t>[current_server_logical_num];
	server_valueupdateserver_udpsock_list = new int[current_server_logical_num];
	for (size_t i = 0; i < current_server_logical_num; i++) {
		//server_netcache_valueupdate_ptr_queue_list[i].init(MQ_SIZE);
		server_distcache_spine_valueupdate_inswitch_ptr_queue_list[i].init(MQ_SIZE);
		uint16_t tmp_global_server_logical_idx = server_logical_idxes_list[server_physical_idx][i];
		prepare_udpserver(server_valueupdateserver_udpsock_list[i], true, server_valueupdateserver_port_start + tmp_global_server_logical_idx, "server.valueupdateserver");
	}

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
	if (server_popserver_udpsock_list != NULL) {
		delete [] server_popserver_udpsock_list;
		server_popserver_udpsock_list = NULL;
	}
	if (server_mutex_for_keyset_list != NULL) {
		delete [] server_mutex_for_keyset_list;
		server_mutex_for_keyset_list = NULL;
	}
	if (server_cached_keyidxmap_list != NULL) {
		delete [] server_cached_keyidxmap_list;
		server_cached_keyidxmap_list = NULL;
	}
	if (server_beingupdated_keyset_list != NULL) {
		delete [] server_beingupdated_keyset_list;
		server_beingupdated_keyset_list = NULL;
	}
	if (server_evictserver_udpsock_list != NULL) {
		delete [] server_evictserver_udpsock_list;
		server_evictserver_udpsock_list = NULL;
	}
	/*if (server_evictserver_tcpsock_list != NULL) {
		delete [] server_evictserver_tcpsock_list;
		server_evictserver_tcpsock_list = NULL;
	}*/
	/*if (server_netcache_valueupdate_ptr_queue_list != NULL) {
		delete [] server_netcache_valueupdate_ptr_queue_list;
		server_netcache_valueupdate_ptr_queue_list = NULL;
	}*/
	if (server_distcache_spine_valueupdate_inswitch_ptr_queue_list != NULL) {
		delete [] server_distcache_spine_valueupdate_inswitch_ptr_queue_list;
		server_distcache_spine_valueupdate_inswitch_ptr_queue_list = NULL;
	}
	if (server_valueupdateserver_udpsock_list != NULL) {
		delete [] server_valueupdateserver_udpsock_list;
		server_valueupdateserver_udpsock_list = NULL;
	}
}

void *run_server_popserver(void *param) {
  // Parse param
  uint16_t local_server_logical_idx = *((uint16_t *)param);
  uint16_t global_server_logical_idx = server_logical_idxes_list[server_physical_idx][local_server_logical_idx];

  // NOTE: controller and switchos should have been launched before servers
  struct sockaddr_in controller_popserver_popclient_addr;
  socklen_t controller_popserver_popclient_addrlen = sizeof(struct sockaddr_in);
  
  printf("[server.popserver%d] ready\n", int(local_server_logical_idx));

  transaction_ready_threads++;

  while (!transaction_running) {}

  char buf[MAX_BUFSIZE];
  char recvbuf[MAX_BUFSIZE];
  int recvsize = 0;
  while (transaction_running) {
	bool is_timeout = udprecvfrom(server_popserver_udpsock_list[local_server_logical_idx], recvbuf, MAX_BUFSIZE, 0, &controller_popserver_popclient_addr, &controller_popserver_popclient_addrlen, recvsize, "server.popserver");
	if (is_timeout) {
		controller_popserver_popclient_addrlen = sizeof(struct sockaddr_in);
		continue;
	}

	packet_type_t tmp_optype = get_packet_type(recvbuf, recvsize);

	if (tmp_optype == packet_type_t::NETCACHE_CACHE_POP_FINISH) {
		// receive NETCACHE_CACHE_POP_FINISH from controller
		netcache_cache_pop_finish_t tmp_netcache_cache_pop_finish(recvbuf, recvsize);
		INVARIANT(tmp_netcache_cache_pop_finish.serveridx() == global_server_logical_idx);

		// send NETCACHE_CACHE_POP_FINISH_ACK to controller immediately to avoid timeout and retry
		netcache_cache_pop_finish_ack_t tmp_netcache_cache_pop_finish_ack(tmp_netcache_cache_pop_finish.key(), global_server_logical_idx);
		uint32_t pktsize = tmp_netcache_cache_pop_finish_ack.serialize(buf, MAX_BUFSIZE);
		udpsendto(server_popserver_udpsock_list[local_server_logical_idx], buf, pktsize, 0, &controller_popserver_popclient_addr, controller_popserver_popclient_addrlen, "server.popserver");

		// keep atomicity
		server_mutex_for_keyset_list[local_server_logical_idx].lock();
		// NETCACHE_CACHE_POP_FINISH's key must NOT in beingupdated keyset if NOT in cached keyset (consider duplicate NETCACHE_CACHE_POP_FINISHs)
		bool is_cached = (server_cached_keyidxmap_list[local_server_logical_idx].find(tmp_netcache_cache_pop_finish.key()) != server_cached_keyidxmap_list[local_server_logical_idx].end());
		bool is_being_updated = (server_beingupdated_keyset_list[local_server_logical_idx].find(tmp_netcache_cache_pop_finish.key()) != server_beingupdated_keyset_list[local_server_logical_idx].end());
		printf("kvidx: %d, newkey: %x, iscached: %d\n", tmp_netcache_cache_pop_finish.kvidx(), tmp_netcache_cache_pop_finish.key().keyhihi, is_cached?0:1);
		fflush(stdout);
		if (!is_cached) { // not in cached keyset
			// add key into cached keyset
			server_cached_keyidxmap_list[local_server_logical_idx].insert(std::pair<netreach_key_t, uint16_t>(tmp_netcache_cache_pop_finish.key(), tmp_netcache_cache_pop_finish.kvidx()));

			// must NOT in beingupdated keyset
			INVARIANT(!is_being_updated);
			server_beingupdated_keyset_list[local_server_logical_idx].insert(tmp_netcache_cache_pop_finish.key()); // mark it as being updated

			// get value from storage server KVS
			val_t tmp_val;
			uint32_t tmp_seq = 0;
			bool tmp_stat = db_wrappers[local_server_logical_idx].get(tmp_netcache_cache_pop_finish.key(), tmp_val, tmp_seq);
			uint16_t tmp_spineswitchidx = uint16_t(tmp_netcache_cache_pop_finish.key().get_spineswitch_idx(switch_partition_count, spineswitch_total_logical_num));
			uint16_t tmp_leafswitchidx = uint16_t(tmp_netcache_cache_pop_finish.key().get_leafswitch_idx(switch_partition_count, max_server_total_logical_num, leafswitch_total_logical_num, spineswitch_total_logical_num));

			// Phase 2 of cache coherence: notify server.valueupdateserver to update inswitch value in background
			//netcache_valueupdate_t *tmp_netcache_valueupdate_ptr = NULL; // freed by server.valueupdateserver
			//tmp_netcache_valueupdate_ptr = new netcache_valueupdate_t(tmp_spineswitchidx, tmp_leafswitchidx, tmp_netcache_cache_pop_finish.key(), tmp_val, tmp_seq, tmp_stat);
			//bool res = server_netcache_valueupdate_ptr_queue_list[local_server_logical_idx].write(tmp_netcache_valueupdate_ptr);
			distcache_spine_valueupdate_inswitch_t *tmp_distcache_spine_valueupdate_inswitch_ptr = NULL; // freed by server.valueupdateserver
			tmp_distcache_spine_valueupdate_inswitch_ptr = new distcache_spine_valueupdate_inswitch_t(tmp_spineswitchidx, tmp_leafswitchidx, tmp_netcache_cache_pop_finish.key(), tmp_val, tmp_seq, tmp_stat, tmp_netcache_cache_pop_finish.kvidx());
			bool res = server_distcache_spine_valueupdate_inswitch_ptr_queue_list[local_server_logical_idx].write(tmp_distcache_spine_valueupdate_inswitch_ptr);
			if (!res) {
				printf("[server.popserver %d-%d] message queue overflow of NETCACHE_VALUEUPDATE\n", local_server_logical_idx, global_server_logical_idx);
				fflush(stdout);
			}
		}
		server_mutex_for_keyset_list[local_server_logical_idx].unlock();
	}
	else {
		printf("[server.popserver] invalid optype: %x\n", optype_t(tmp_optype));
		fflush(stdout);
		exit(-1);
	}
  }

  close(server_popserver_udpsock_list[local_server_logical_idx]);
  pthread_exit(nullptr);
  return 0;
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

  // open rocksdb
  bool is_existing = db_wrappers[local_server_logical_idx].open(global_server_logical_idx);
  if (!is_existing) {
	  printf("[server.worker %d-%d] you need to run loader before server\n", local_server_logical_idx, global_server_logical_idx);
	  fflush(stdout);
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
  dynamic_array_t scanbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
  int recv_size = 0;
  int rsp_size = 0;
  char recvbuf[MAX_BUFSIZE];

  struct timespec polling_interrupt_for_blocking;
  polling_interrupt_for_blocking.tv_sec = 0;
  polling_interrupt_for_blocking.tv_nsec = 1000; // 1ms = 1000ns

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
  struct timespec beingcached_t1, beingcached_t2, beingcached_t3;
  struct timespec beingupdated_t1, beingupdated_t2, beingupdated_t3;
  double beingupdated_time = 0.0;
#endif
  bool is_first_pkt = true;
  while (transaction_running) {

#ifdef DEBUG_SERVER
	if (!is_first_pkt) {
		CUR_TIME(udprecv_t1);

		CUR_TIME(wait_beforerecv_t2);
		DELTA_TIME(wait_beforerecv_t2, wait_t1, wait_beforerecv_t3);
		thread_param.wait_beforerecv_latency_list.push_back(GET_MICROSECOND(wait_beforerecv_t3));
	}
#endif

	bool is_timeout = udprecvfrom(server_worker_udpsock_list[local_server_logical_idx], buf, MAX_BUFSIZE, 0, &client_addr, &client_addrlen, recv_size, "server.worker");
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

	packet_type_t pkt_type = get_packet_type(buf, recv_size);
	switch (pkt_type) {
		case packet_type_t::GETREQ: 
			{
				get_request_t req(buf, recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string())
				val_t tmp_val;
				uint32_t tmp_seq = 0;
				bool tmp_stat = db_wrappers[local_server_logical_idx].get(req.key(), tmp_val, tmp_seq);
				//COUT_THIS("[server] val = " << tmp_val.to_string())
				//uint16_t tmp_spineswitchidx = uint16_t(req.key().get_spineswitch_idx(switch_partition_count, spineswitch_total_logical_num));
				//uint16_t tmp_leafswitchidx = uint16_t(req.key().get_leafswitch_idx(switch_partition_count, max_server_total_logical_num, leafswitch_total_logical_num, spineswitch_total_logical_num));
				//get_response_server_t rsp(tmp_spineswitchidx, tmp_leafswitchidx, req.key(), tmp_val, tmp_stat, global_server_logical_idx);
				// NOTE: we use req.spine/leafswitchidx such that GETRES_SERVER can update spine/leafload_forclient in client-leaf
				get_response_server_t rsp(req.spineswitchidx(), req.leafswitchidx(), req.key(), tmp_val, tmp_stat, global_server_logical_idx, req.spineload(), req.leafload());
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::PUTREQ_SEQ:
		case packet_type_t::DELREQ_SEQ:
			{
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif

#ifdef DEBUG_SERVER
				CUR_TIME(rocksdb_t1);
#endif

				netreach_key_t tmp_key;
				val_t tmp_val;
				uint32_t tmp_seq;
				if (pkt_type == packet_type_t::PUTREQ_SEQ) {
					put_request_seq_t req(buf, recv_size);
					tmp_key = req.key();
					tmp_val = req.val();
					tmp_seq = req.seq();
				}
				else if (pkt_type == packet_type_t::DELREQ_SEQ) {
					del_request_seq_t req(buf, recv_size);
					tmp_key = req.key();
					tmp_seq = req.seq();
				}
				else {
					printf("[server.worker] invalid pkttype: %x which should be PUTREQ_SEQ/DELREQ_SEQ\n", optype_t(pkt_type));
					fflush(stdout);
					exit(-1);
				}

				//COUT_THIS("[server] key = " << tmp_key.to_string())
				
#ifdef DEBUG_SERVER
				beingupdated_time = 0.0;
#endif
				
				bool tmp_stat = false;
				server_mutex_for_keyset_list[local_server_logical_idx].lock();
				bool is_cached = (server_cached_keyidxmap_list[local_server_logical_idx].find(tmp_key) != server_cached_keyidxmap_list[local_server_logical_idx].end());
				bool is_being_updated = (server_beingupdated_keyset_list[local_server_logical_idx].find(tmp_key) != server_beingupdated_keyset_list[local_server_logical_idx].end());
				if (likely(!is_cached)) { // uncached
					if (pkt_type == packet_type_t::PUTREQ_SEQ) {
						tmp_stat = db_wrappers[local_server_logical_idx].put(tmp_key, tmp_val, tmp_seq); // perform PUT operation
					}
					else {
						tmp_stat = db_wrappers[local_server_logical_idx].remove(tmp_key, tmp_seq); // perform DEL operation
					}
				}

				if (unlikely(is_cached)) { // already cached
					// Phase 1 of cache coherence: invalidate in-switch value
					uint16_t tmp_spineswitchidx = uint16_t(tmp_key.get_spineswitch_idx(switch_partition_count, spineswitch_total_logical_num));
					uint16_t tmp_leafswitchidx = uint16_t(tmp_key.get_leafswitch_idx(switch_partition_count, max_server_total_logical_num, leafswitch_total_logical_num, spineswitch_total_logical_num));
					distcache_invalidate_t tmp_distcache_invalidate(tmp_spineswitchidx, tmp_leafswitchidx, tmp_key);
					while (true) {
						int tmp_distcache_invalidate_size = tmp_distcache_invalidate.serialize(buf, MAX_BUFSIZE);
						udpsendto(server_worker_invalidateserver_udpsock_list[local_server_logical_idx], buf, tmp_distcache_invalidate_size, 0, &client_addr, client_addrlen, "server.worker.invalidateserver");

						int tmp_distcache_invalidate_ack_size = 0;
						bool is_timeout = udprecvfrom(server_worker_invalidateserver_udpsock_list[local_server_logical_idx], buf, MAX_BUFSIZE, 0, NULL, NULL, tmp_distcache_invalidate_ack_size, "server.worker.invalidateserver");
						if (unlikely(is_timeout)) {
							continue;
						}
						else {
							distcache_invalidate_ack_t tmp_distcache_invalidate_ack(buf, tmp_distcache_invalidate_ack_size);
							INVARIANT(tmp_distcache_invalidate_ack.key() == tmp_distcache_invalidate.key());
							break;
						}
					}

					// Phase 2 of cache coherence: update in-switch value in background
#ifdef DEBUG_SERVER
					CUR_TIME(beingupdated_t1);
#endif
					while (is_being_updated) { // being updated
						server_mutex_for_keyset_list[local_server_logical_idx].unlock();
						//usleep(1); // wait for inswitch value update finish
						nanosleep(&polling_interrupt_for_blocking, NULL); // wait for cache population finish
						server_mutex_for_keyset_list[local_server_logical_idx].lock();
						is_being_updated = (server_beingupdated_keyset_list[local_server_logical_idx].find(tmp_key) != server_beingupdated_keyset_list[local_server_logical_idx].end());
					}
					INVARIANT(!is_being_updated);
#ifdef DEBUG_SERVER
					CUR_TIME(beingupdated_t2);
					DELTA_TIME(beingupdated_t2, beingupdated_t1, beingupdated_t3);
					beingupdated_time = GET_MICROSECOND(beingupdated_t3);
#endif

					// Double-check due to potential cache eviction
					is_cached = (server_cached_keyidxmap_list[local_server_logical_idx].find(tmp_key) != server_cached_keyidxmap_list[local_server_logical_idx].end());
					if (is_cached) { // key is removed from beingupdated keyset by server.valueupdateserver
						server_beingupdated_keyset_list[local_server_logical_idx].insert(tmp_key); // mark it as being updated

						// notify server.valueupdateserver to update inswitch value in background
						/*netcache_valueupdate_t *tmp_netcache_valueupdate_ptr = NULL; // freed by server.valueupdateserver
						if (pkt_type == packet_type_t::PUTREQ_SEQ) {
							tmp_netcache_valueupdate_ptr = new netcache_valueupdate_t(tmp_spineswitchidx, tmp_leafswitchidx, tmp_key, tmp_val, tmp_seq, true);
						}
						else { // NOTE: for DEL, tmp_val = val_t() whose length is 0
							tmp_netcache_valueupdate_ptr = new netcache_valueupdate_t(tmp_spineswitchidx, tmp_leafswitchidx, tmp_key, tmp_val, tmp_seq, false);
						}
						bool res = server_netcache_valueupdate_ptr_queue_list[local_server_logical_idx].write(tmp_netcache_valueupdate_ptr);*/
						distcache_spine_valueupdate_inswitch_t *tmp_distcache_spine_valueupdate_inswitch_ptr = NULL; // freed by server.valueupdateserver
						if (pkt_type == packet_type_t::PUTREQ_SEQ) {
							tmp_distcache_spine_valueupdate_inswitch_ptr = new distcache_spine_valueupdate_inswitch_t(tmp_spineswitchidx, tmp_leafswitchidx, tmp_key, tmp_val, tmp_seq, true, server_cached_keyidxmap_list[local_server_logical_idx][tmp_key]);
						}
						else { // NOTE: for DEL, tmp_val = val_t() whose length is 0
							tmp_distcache_spine_valueupdate_inswitch_ptr = new distcache_spine_valueupdate_inswitch_t(tmp_spineswitchidx, tmp_leafswitchidx, tmp_key, tmp_val, tmp_seq, false, server_cached_keyidxmap_list[local_server_logical_idx][tmp_key]);
						}
						bool res = server_distcache_spine_valueupdate_inswitch_ptr_queue_list[local_server_logical_idx].write(tmp_distcache_spine_valueupdate_inswitch_ptr);
						if (!res) {
							printf("[server.worker %d-%d] message queue overflow of NETCACHE_VALUEUPDATE\n", local_server_logical_idx, global_server_logical_idx);
						}
					}
					// else: do nothing as key is removed from beingupdated keyset by server.evictserver

					if (pkt_type == packet_type_t::PUTREQ_SEQ) {
						tmp_stat = db_wrappers[local_server_logical_idx].put(tmp_key, tmp_val, tmp_seq); // perform PUT operation
					}
					else {
						tmp_stat = db_wrappers[local_server_logical_idx].remove(tmp_key, tmp_seq); // perform DEL operation
					}
				}
				server_mutex_for_keyset_list[local_server_logical_idx].unlock();
				UNUSED(tmp_stat);
				//COUT_THIS("[server] stat = " << tmp_stat)
				
#ifdef DEBUG_SERVER
				CUR_TIME(rocksdb_t2);
#endif
				
				if (pkt_type == packet_type_t::PUTREQ_SEQ) {
					put_response_server_t rsp(tmp_key, true, global_server_logical_idx);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				}
				else {
					del_response_server_t rsp(tmp_key, true, global_server_logical_idx);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				}
				udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::NETCACHE_PUTREQ_SEQ_CACHED:
		case packet_type_t::NETCACHE_DELREQ_SEQ_CACHED:
			{
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif

#ifdef DEBUG_SERVER
				CUR_TIME(rocksdb_t1);
#endif

				netreach_key_t tmp_key;
				val_t tmp_val;
				uint32_t tmp_seq;
				if (pkt_type == packet_type_t::NETCACHE_PUTREQ_SEQ_CACHED) {
					netcache_put_request_seq_cached_t req(buf, recv_size);
					tmp_key = req.key();
					tmp_val = req.val();
					tmp_seq = req.seq();
				}
				else if (pkt_type == packet_type_t::NETCACHE_DELREQ_SEQ_CACHED) {
					netcache_del_request_seq_cached_t req(buf, recv_size);
					tmp_key = req.key();
					tmp_seq = req.seq();
				}
				else {
					printf("[server.worker] invalid pkttype: %x which should be NETCACHED_PUTREQ_SEQ_CACHED/NETCACHE_DELREQ_SEQ_CACHED\n", optype_t(pkt_type));
					fflush(stdout);
					exit(-1);
				}

				//COUT_THIS("[server] key = " << tmp_key.to_string() << " val = " << req.val().to_string())
				
#ifdef DEBUG_SERVER
				beingupdated_time = 0.0;
#endif

				bool tmp_stat = false;
				server_mutex_for_keyset_list[local_server_logical_idx].lock();
				bool is_cached = (server_cached_keyidxmap_list[local_server_logical_idx].find(tmp_key) != server_cached_keyidxmap_list[local_server_logical_idx].end());
				bool is_being_updated = (server_beingupdated_keyset_list[local_server_logical_idx].find(tmp_key) != server_beingupdated_keyset_list[local_server_logical_idx].end());
				if (unlikely(!is_cached)) { // uncached
					if (pkt_type == packet_type_t::NETCACHE_PUTREQ_SEQ_CACHED) {
						tmp_stat = db_wrappers[local_server_logical_idx].put(tmp_key, tmp_val, tmp_seq); // perform PUT operation
					}
					else {
						tmp_stat = db_wrappers[local_server_logical_idx].remove(tmp_key, tmp_seq); // perform DEL operation
					}
				}

				if (likely(is_cached)) { // already cached
					// Phase 1 of cache coherence: invalidate in-switch value
					uint16_t tmp_spineswitchidx = uint16_t(tmp_key.get_spineswitch_idx(switch_partition_count, spineswitch_total_logical_num));
					uint16_t tmp_leafswitchidx = uint16_t(tmp_key.get_leafswitch_idx(switch_partition_count, max_server_total_logical_num, leafswitch_total_logical_num, spineswitch_total_logical_num));
					distcache_invalidate_t tmp_distcache_invalidate(tmp_spineswitchidx, tmp_leafswitchidx, tmp_key);
					while (true) {
						int tmp_distcache_invalidate_size = tmp_distcache_invalidate.serialize(buf, MAX_BUFSIZE);
						udpsendto(server_worker_invalidateserver_udpsock_list[local_server_logical_idx], buf, tmp_distcache_invalidate_size, 0, &client_addr, client_addrlen, "server.worker.invalidateserver");

						int tmp_distcache_invalidate_ack_size = 0;
						bool is_timeout = udprecvfrom(server_worker_invalidateserver_udpsock_list[local_server_logical_idx], buf, MAX_BUFSIZE, 0, NULL, NULL, tmp_distcache_invalidate_ack_size, "server.worker.invalidateserver");
						if (unlikely(is_timeout)) {
							continue;
						}
						else {
							distcache_invalidate_ack_t tmp_distcache_invalidate_ack(buf, tmp_distcache_invalidate_ack_size);
							INVARIANT(tmp_distcache_invalidate_ack.key() == tmp_distcache_invalidate.key());
							break;
						}
					}

					// Phase 2 of cache coherence: update in-switch value in background
#ifdef DEBUG_SERVER
					CUR_TIME(beingupdated_t1);
#endif
					while (is_being_updated) { // being updated
						server_mutex_for_keyset_list[local_server_logical_idx].unlock();
						//usleep(1); // wait for inswitch value update finish
						nanosleep(&polling_interrupt_for_blocking, NULL); // wait for cache population finish
						server_mutex_for_keyset_list[local_server_logical_idx].lock();
						is_being_updated = (server_beingupdated_keyset_list[local_server_logical_idx].find(tmp_key) != server_beingupdated_keyset_list[local_server_logical_idx].end());
					}
					INVARIANT(!is_being_updated);
#ifdef DEBUG_SERVER
					CUR_TIME(beingupdated_t2);
					DELTA_TIME(beingupdated_t2, beingupdated_t1, beingupdated_t3);
					beingupdated_time = GET_MICROSECOND(beingupdated_t3);
#endif

					// Double-check due to potential cache eviction
					is_cached = (server_cached_keyidxmap_list[local_server_logical_idx].find(tmp_key) != server_cached_keyidxmap_list[local_server_logical_idx].end());
					if (is_cached) { // key is removed from beingupdated keyset by server.valueupdateserver
						server_beingupdated_keyset_list[local_server_logical_idx].insert(tmp_key); // mark it as being updated

						// notify server.valueupdateserver to update inswitch value in background
						/*netcache_valueupdate_t *tmp_netcache_valueupdate_ptr = NULL; // freed by server.valueupdateserver
						if (pkt_type == packet_type_t::NETCACHE_PUTREQ_SEQ_CACHED) {
							tmp_netcache_valueupdate_ptr = new netcache_valueupdate_t(tmp_spineswitchidx, tmp_leafswitchidx, tmp_key, tmp_val, tmp_seq, true);
						}
						else { // NOTE: for DEL, tmp_val = val_t() whose length is 0
							tmp_netcache_valueupdate_ptr = new netcache_valueupdate_t(tmp_spineswitchidx, tmp_leafswitchidx, tmp_key, tmp_val, tmp_seq, false);
						}
						bool res = server_netcache_valueupdate_ptr_queue_list[local_server_logical_idx].write(tmp_netcache_valueupdate_ptr);*/
						distcache_spine_valueupdate_inswitch_t *tmp_distcache_spine_valueupdate_inswitch_ptr = NULL; // freed by server.valueupdateserver
						if (pkt_type == packet_type_t::NETCACHE_PUTREQ_SEQ_CACHED) {
							tmp_distcache_spine_valueupdate_inswitch_ptr = new distcache_spine_valueupdate_inswitch_t(tmp_spineswitchidx, tmp_leafswitchidx, tmp_key, tmp_val, tmp_seq, true, server_cached_keyidxmap_list[local_server_logical_idx][tmp_key]);
						}
						else { // NOTE: for DEL, tmp_val = val_t() whose length is 0
							tmp_distcache_spine_valueupdate_inswitch_ptr = new distcache_spine_valueupdate_inswitch_t(tmp_spineswitchidx, tmp_leafswitchidx, tmp_key, tmp_val, tmp_seq, false, server_cached_keyidxmap_list[local_server_logical_idx][tmp_key]);
						}
						bool res = server_distcache_spine_valueupdate_inswitch_ptr_queue_list[local_server_logical_idx].write(tmp_distcache_spine_valueupdate_inswitch_ptr);
						if (!res) {
							printf("[server.worker %d-%d] message queue overflow of NETCACHE_VALUEUPDATE\n", local_server_logical_idx, global_server_logical_idx);
						}
					}
					// else: do nothing as key is removed from beingupdated keyset by server.evictserver

					if (pkt_type == packet_type_t::NETCACHE_PUTREQ_SEQ_CACHED) {
						tmp_stat = db_wrappers[local_server_logical_idx].put(tmp_key, tmp_val, tmp_seq); // perform PUT operation
					}
					else {
						tmp_stat = db_wrappers[local_server_logical_idx].remove(tmp_key, tmp_seq); // perform DEL operation
					}
				}
				server_mutex_for_keyset_list[local_server_logical_idx].unlock();
				UNUSED(tmp_stat);
				//COUT_THIS("[server] stat = " << tmp_stat)
				
#ifdef DEBUG_SERVER
				CUR_TIME(rocksdb_t2);
#endif
				
				if (pkt_type == packet_type_t::NETCACHE_PUTREQ_SEQ_CACHED) {
					put_response_t rsp(tmp_key, true, global_server_logical_idx);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				}
				else {
					del_response_t rsp(tmp_key, true, global_server_logical_idx);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				}
				udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::SCANREQ_SPLIT:
			{
				scan_request_split_t req(buf, recv_size);
				
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

				scan_response_split_server_t rsp(req.key(), req.endkey(), req.cur_scanidx(), req.max_scannum(), req.cur_scanswitchidx(), req.max_scanswitchnum(), global_server_logical_idx, 0, results.size(), results);
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				//rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				//udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
				scanbuf.clear();
				rsp_size = rsp.dynamic_serialize(scanbuf);
				udpsendlarge_ipfrag(server_worker_udpsock_list[local_server_logical_idx], scanbuf.array(), rsp_size, 0, &client_addr, client_addrlen, "server.worker", scan_response_split_t::get_frag_hdrsize());
#ifdef DUMP_BUF
				dump_buf(scanbuf.array(), rsp_size);
#endif
				break;
			}
		case packet_type_t::LOADREQ:
			{
				load_request_t req(buf, recv_size);
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif

				//char val_buf[Val::SWITCH_MAX_VALLEN];
				//memset(val_buf, 0x11, Val::SWITCH_MAX_VALLEN);
				//val_t tmp_val(val_buf, Val::SWITCH_MAX_VALLEN);
				bool tmp_stat = db_wrappers[local_server_logical_idx].force_put(req.key(), req.val());
				UNUSED(tmp_stat);
				
				load_ack_server_t rsp(req.key());
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
				//exit(-1);
			}
	}

	if (likely(pkt_type != packet_type_t::LOADREQ)) {
#ifdef DEBUG_SERVER
		CUR_TIME(process_t2);
		DELTA_TIME(process_t2, process_t1, process_t3);
		thread_param.process_latency_list.push_back(GET_MICROSECOND(process_t3));

		DELTA_TIME(rocksdb_t2, rocksdb_t1, rocksdb_t3);
		thread_param.rocksdb_latency_list.push_back(GET_MICROSECOND(rocksdb_t3));

		thread_param.beingupdated_latency_list.push_back(beingupdated_time);

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

	// process NETCACHE_CACHE_EVICT packet <optype, key, serveridx>
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

		//printf("receive NETCACHE_CACHE_EVICT from controller\n");
		//dump_buf(recvbuf, recvsize);
		
		netcache_cache_evict_t tmp_netcache_cache_evict(recvbuf, recvsize);

		uint16_t tmp_serveridx = tmp_netcache_cache_evict.serveridx();
		INVARIANT(tmp_serveridx == global_server_logical_idx);

		// keep atomicity
		server_mutex_for_keyset_list[local_server_logical_idx].lock();
		// NETCACHE_CACHE_EVICT's key must in cached keyset
		// consider duplicate NETCACHE_CACHE_EVICTs -> key may already be removed from cached/beingudpated keyset
		printf("evcitidx: %d, evictkey %x\n", server_cached_keyidxmap_list[local_server_logical_idx][tmp_netcache_cache_evict.key()], tmp_netcache_cache_evict.key().keyhihi);
		fflush(stdout);
		if (!(server_cached_keyidxmap_list[local_server_logical_idx].find(tmp_netcache_cache_evict.key()) != server_cached_keyidxmap_list[local_server_logical_idx].end())) {
			printf("[server.evictserver %d-%d WARNING] evicted key %x is not in cached keyset (size: %d)\n", local_server_logical_idx, global_server_logical_idx, tmp_netcache_cache_evict.key().keyhihi, server_cached_keyidxmap_list[local_server_logical_idx].size());
			fflush(stdout);
		}
		// remove key from cached/beingupdated keyset
		server_cached_keyidxmap_list[local_server_logical_idx].erase(tmp_netcache_cache_evict.key());
		server_beingupdated_keyset_list[local_server_logical_idx].erase(tmp_netcache_cache_evict.key());
		server_mutex_for_keyset_list[local_server_logical_idx].unlock();

		// send NETCACHE_CACHE_EVICT_ACK to controller.evictserver.evictclient
		netcache_cache_evict_ack_t tmp_netcache_cache_evict_ack(tmp_netcache_cache_evict.key(), global_server_logical_idx);
		int sendsize = tmp_netcache_cache_evict_ack.serialize(sendbuf, MAX_BUFSIZE);
		//printf("send NETCACHE_CACHE_EVICT_ACK to controller\n");
		//dump_buf(sendbuf, sendsize);
		udpsendto(server_evictserver_udpsock_list[local_server_logical_idx], sendbuf, sendsize, 0, &controller_evictclient_addr, controller_evictclient_addrlen, "server.evictserver");
	}

	close(server_evictserver_udpsock_list[local_server_logical_idx]);
	pthread_exit(nullptr);
}

void *run_server_valueupdateserver(void *param) {
	uint16_t local_server_logical_idx = *((uint16_t *)param);
	uint16_t global_server_logical_idx = server_logical_idxes_list[server_physical_idx][local_server_logical_idx];

	// client address (switch will not hide NETCACHE_VALUEUPDATE from clients)
	struct sockaddr_in client_addr;
	set_sockaddr(client_addr, inet_addr(client_ips[0]), 123); // client ip and client port are not important
	socklen_t client_addrlen = sizeof(struct sockaddr_in);
	
	printf("[server.valueupdateserver %d-%d] ready\n", local_server_logical_idx, global_server_logical_idx);
	transaction_ready_threads++;

	while (!transaction_running) {}

	//char sendbuf[MAX_BUFSIZE];
	char sendbuf_forspine[MAX_BUFSIZE];
	char sendbuf_forleaf[MAX_BUFSIZE];
	char recvbuf[MAX_BUFSIZE];
	//int sendsize = 0;
	int sendsize_forspine = 0;
	int sendsize_forleaf = 0 ;
	int recvsize = 0;
	while (transaction_running) {
		//netcache_valueupdate_t *tmp_netcache_valueupdate_ptr = server_netcache_valueupdate_ptr_queue_list[local_server_logical_idx].read();
		distcache_spine_valueupdate_inswitch_t *tmp_distcache_spine_valueupdate_inswitch_ptr = server_distcache_spine_valueupdate_inswitch_ptr_queue_list[local_server_logical_idx].read();
		//if (tmp_netcache_valueupdate_ptr != NULL) {
		if (tmp_distcache_spine_valueupdate_inswitch_ptr != NULL) {
			/*if (first_cachepop) {
				first_cachepop = false;
				// sleep 0.5s for warmup phase to avoid not-yet-effective newly-populated keys in cache_lookup_tbl
				// NOTE: it does NOT affect server-rotation/normal transaction phase
				usleep(500 * 1000); 
			}*/

			//sendsize = tmp_netcache_valueupdate_ptr->serialize(sendbuf, MAX_BUFSIZE);

			distcache_leaf_valueupdate_inswitch_t tmp_distcache_leaf_valueupdate_inswitch(tmp_distcache_spine_valueupdate_inswitch_ptr->spineswitchidx(), tmp_distcache_spine_valueupdate_inswitch_ptr->leafswitchidx(), tmp_distcache_spine_valueupdate_inswitch_ptr->key(), tmp_distcache_spine_valueupdate_inswitch_ptr->val(), tmp_distcache_spine_valueupdate_inswitch_ptr->seq(), tmp_distcache_spine_valueupdate_inswitch_ptr->stat(), tmp_distcache_spine_valueupdate_inswitch_ptr->kvidx());
			sendsize_forspine = tmp_distcache_spine_valueupdate_inswitch_ptr->serialize(sendbuf_forspine, MAX_BUFSIZE);
			sendsize_forleaf = tmp_distcache_leaf_valueupdate_inswitch.serialize(sendbuf_forleaf, MAX_BUFSIZE);
			
			/*while (true) {
				udpsendto(server_valueupdateserver_udpsock_list[local_server_logical_idx], sendbuf, sendsize, 0, &client_addr, client_addrlen, "server.valueupdateserver");

				bool is_timeout = udprecvfrom(server_valueupdateserver_udpsock_list[local_server_logical_idx], recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "server.valueupdateserver");
				if (is_timeout) {
					continue;
				}
				else {
					netcache_valueupdate_ack_t tmp_netcache_valueupdate_ack(recvbuf, recvsize);
					if (tmp_netcache_valueupdate_ack.key() != tmp_netcache_valueupdate_ptr->key()) {
						continue;
					}
					else {
						break;
					}
				}
			}*/

			// send to spine/server-leaf simultaneously to reduce waiting time
			udpsendto(server_valueupdateserver_udpsock_list[local_server_logical_idx], sendbuf_forspine, sendsize_forspine, 0, &client_addr, client_addrlen, "server.valueupdateserver");
			udpsendto(server_valueupdateserver_udpsock_list[local_server_logical_idx], sendbuf_forleaf, sendsize_forleaf, 0, &client_addr, client_addrlen, "server.valueupdateserver");

			// NOTE: valueupdate overhead is too large under write-intensive workload, which is caused by NetCache/DistCache design (write-through policy + server-issued value update) -> we skip timeout-and-retry mechanism here
			/*bool with_spineack = false;
			bool with_leafack = false;
			while (true) {
				bool is_timeout = udprecvfrom(server_valueupdateserver_udpsock_list[local_server_logical_idx], recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "server.valueupdateserver");
				if (is_timeout) {
					if (!with_spineack) {
						udpsendto(server_valueupdateserver_udpsock_list[local_server_logical_idx], sendbuf_forspine, sendsize_forspine, 0, &client_addr, client_addrlen, "server.valueupdateserver");
					}
					if (!with_leafack) {
						udpsendto(server_valueupdateserver_udpsock_list[local_server_logical_idx], sendbuf_forleaf, sendsize_forleaf, 0, &client_addr, client_addrlen, "server.valueupdateserver");
					}
					continue;
				}
				else {
					packet_type_t tmp_optype = get_packet_type(recvbuf, recvsize);
					if (tmp_optype == packet_type_t::DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK) {
						distcache_spine_valueupdate_inswitch_ack_t tmp_distcache_spine_valueupdate_inswitch_ack(recvbuf, recvsize);
						if (tmp_distcache_spine_valueupdate_inswitch_ack.key() == tmp_distcache_spine_valueupdate_inswitch_ptr->key()) {
							with_spineack = true;
						}
					}
					else if (tmp_optype == packet_type_t::DISTCACHE_LEAF_VALUEUPDATE_INSWITCH_ACK) {
						distcache_leaf_valueupdate_inswitch_ack_t tmp_distcache_leaf_valueupdate_inswitch_ack(recvbuf, recvsize);
						if (tmp_distcache_leaf_valueupdate_inswitch_ack.key() == tmp_distcache_leaf_valueupdate_inswitch.key()) {
							with_leafack = true;
						}
					}

					if (with_spineack && with_leafack) {
						break;
					}
				}
			}*/

			// remove key from beingupdated keyset atomically
			server_mutex_for_keyset_list[local_server_logical_idx].lock();
			//bool is_being_updated = (server_beingupdated_keyset_list[local_server_logical_idx].find(tmp_netcache_valueupdate_ptr->key()) != server_beingupdated_keyset_list[local_server_logical_idx].end());
			bool is_being_updated = (server_beingupdated_keyset_list[local_server_logical_idx].find(tmp_distcache_spine_valueupdate_inswitch_ptr->key()) != server_beingupdated_keyset_list[local_server_logical_idx].end());
			if (likely(is_being_updated)) {
				//server_beingupdated_keyset_list[local_server_logical_idx].erase(tmp_netcache_valueupdate_ptr->key());
				server_beingupdated_keyset_list[local_server_logical_idx].erase(tmp_distcache_spine_valueupdate_inswitch_ptr->key());
			}
			else { // due to cache eviciton
				//bool is_cached = (server_cached_keyidxmap_list[local_server_logical_idx].find(tmp_netcache_valueupdate_ptr->key()) != server_cached_keyidxmap_list[local_server_logical_idx].end());
				bool is_cached = (server_cached_keyidxmap_list[local_server_logical_idx].find(tmp_distcache_spine_valueupdate_inswitch_ptr->key()) != server_cached_keyidxmap_list[local_server_logical_idx].end());
				INVARIANT(!is_cached); // key must NOT in beingcached/cached keyset
			}
			server_mutex_for_keyset_list[local_server_logical_idx].unlock();

			//delete tmp_netcache_valueupdate_ptr;
			//tmp_netcache_valueupdate_ptr = NULL;
			delete tmp_distcache_spine_valueupdate_inswitch_ptr;
			tmp_distcache_spine_valueupdate_inswitch_ptr = NULL;
		}
	}

	close(server_valueupdateserver_udpsock_list[local_server_logical_idx]);
	pthread_exit(nullptr);
}

#endif
