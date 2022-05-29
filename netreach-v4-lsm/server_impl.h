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

//#define DUMP_BUF

typedef DeletedSet<netreach_key_t, uint32_t> deleted_set_t;
typedef ConcurrentMap<netreach_key_t, snapshot_record_t> concurrent_snapshot_map_t;
typedef ConcurrentSet<netreach_key_t> concurrent_set_t;

struct alignas(CACHELINE_SIZE) ServerWorkerParam {
  uint16_t serveridx;
  size_t throughput;
  std::vector<double> latency_list;
#ifdef TEST_AGG_THPT
  double sum_latency;
#endif
};
typedef ServerWorkerParam server_worker_param_t;

RocksdbWrapper *db_wrappers = NULL;
int * server_worker_udpsock_list = NULL;

// Per-server popclient <-> one popserver in controller
int * server_popclient_tcpsock_list = NULL;
concurrent_set_t * server_cached_keyset_list = NULL;

// per-server worker <-> per-server popclient
MessagePtrQueue<cache_pop_t> *server_cache_pop_ptr_queue_list;

// server.evictservers <-> controller.evictserver.evictclients
//int * server_evictserver_tcpsock_list = NULL;
// server.evictserver <-> controller.evictserver.evictclient
int server_evictserver_tcpsock = -1;

// snapshot
int server_consnapshotserver_tcpsock = -1;
// per-server snapshot_map and snapshot_rcu
//std::map<netreach_key_t, snapshot_record_t> * volatile server_snapshot_maps = NULL;
concurrent_snapshot_map_t * volatile server_snapshot_maps = NULL;
uint32_t volatile * server_snapshot_rcus = NULL;
bool volatile server_issnapshot = false; // TODO: it should be atomic

void prepare_server();
// server.workers for processing pkts
void *run_server_worker(void *param);
void *run_server_popclient(void *param);
void *run_server_evictserver(void *param);
void *run_server_consnapshotserver(void *param);
void close_server();

void prepare_server() {
	printf("[server] prepare start\n");

	RocksdbWrapper::prepare_rocksdb();

	db_wrappers = new RocksdbWrapper[server_num];
	INVARIANT(db_wrappers != NULL);

	server_worker_udpsock_list = new int[server_num];
	for (size_t i = 0; i < server_num; i++) {
		prepare_udpserver(server_worker_udpsock_list[i], true, server_port_start + i, "server.worker");
	}

	// Prepare for cache population
	server_popclient_tcpsock_list = new int[server_num];
	server_cached_keyset_list = new concurrent_set_t[server_num];
	for (size_t i = 0; i < server_num; i++) {
		create_tcpsock(server_popclient_tcpsock_list[i], false, "server.popclient");
	}

	// Prepare message queue between per-server worker and per-server popclient
	server_cache_pop_ptr_queue_list = new MessagePtrQueue<cache_pop_t>[server_num];
	for (size_t i = 0; i < server_num; i++) {
		server_cache_pop_ptr_queue_list[i].init(MQ_SIZE);
	}

	// prepare for cache eviction
	/*server_evictserver_tcpksock_list = new int[server_num];
	for (size_t i = 0; i < server_num; i++) {
		prepare_tcpserver(server_evictserver_tcpksock_list[i], false, server_evictserver_port_start+i, 1, "server.evictserver"); // MAX_PENDING_NUM = 1
	}*/
	prepare_tcpserver(server_evictserver_tcpsock, true, server_evictserver_port_start, 1, "server.evictserver"); // MAX_PENDING_NUM = 1

	// prepare for crash-consistent snapshot
	prepare_tcpserver(server_consnapshotserver_tcpsock, true, server_consnapshotserver_port, 1, "server.consnapshotserver"); // MAX_PENDING_NUM = 1

	// prepare for snapshot
	server_snapshot_rcus = new uint32_t[server_num];
	for (size_t i = 0; i < server_num; i++) {
		server_snapshot_rcus[i] = 0;
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
	if (server_popclient_tcpsock_list != NULL) {
		delete [] server_popclient_tcpsock_list;
		server_popclient_tcpsock_list = NULL;
	}
	if (server_cache_pop_ptr_queue_list != NULL) {
		delete [] server_cache_pop_ptr_queue_list;
		server_cache_pop_ptr_queue_list = NULL;
	}
	if (server_cached_keyset_list != NULL) {
		delete [] server_cached_keyset_list;
		server_cached_keyset_list = NULL;
	}
	/*if (server_evictserver_tcpsock_list != NULL) {
		delete [] server_evictserver_tcpsock_list;
		server_evictserver_tcpsock_list = NULL;
	}*/
	if (server_snapshot_rcus != NULL) {
		delete [] server_snapshot_rcus;
		server_snapshot_rcus = NULL;
	}
}

void *run_server_popclient(void *param) {
  // Parse param
  uint16_t serveridx = *((uint16_t *)param); // [0, server_num-1]

  // NOTE: controller and switchos should have been launched before servers
  tcpconnect(server_popclient_tcpsock_list[serveridx], controller_ip_for_server, controller_popserver_port, "server.popclient", "controller.popserver"); // enforce the packet to go through NIC 
  //tcpconnect(server_popclient_tcpsock_list[serveridx], controller_ip_for_server, controller_popserver_port_start + serveridx, "server.popclient", "controller.popserver"); // enforce the packet to go through NIC 
  
  printf("[server.popclient%d] ready\n", int(serveridx));

  transaction_ready_threads++;

  while (!transaction_running) {}

  while (transaction_running) {
	char buf[MAX_BUFSIZE];
  	cache_pop_t *tmp_cache_pop_ptr = server_cache_pop_ptr_queue_list[serveridx].read();
  	if (tmp_cache_pop_ptr != NULL) {
		uint32_t popsize = tmp_cache_pop_ptr->serialize(buf, MAX_BUFSIZE);
		//printf("send CACHE_POP to controller\n");
		//dump_buf(buf, popsize);
		tcpsend(server_popclient_tcpsock_list[serveridx], buf, popsize, "server.popclient");

		delete tmp_cache_pop_ptr;
		tmp_cache_pop_ptr = NULL;
	}
  }

  close(server_popclient_tcpsock_list[serveridx]);
  pthread_exit(nullptr);
  return 0;
}

/*
 * Worker for server-side processing 
 */

void *run_server_worker(void * param) {
  // Parse param
  server_worker_param_t &thread_param = *(server_worker_param_t *)param;
  uint16_t serveridx = thread_param.serveridx; // [0, server_num-1]

  bool is_existing = db_wrappers[serveridx].open(serveridx);
  if (!is_existing) {
	  printf("You need to run ycsb_loader before ycsb_server\n");
	  exit(-1);
  }

  // scan.startkey <= max_startkey; scan.endkey >= min_startkey
  // use size_t to avoid int overflow
  uint64_t min_startkeyhihi = serveridx * perserver_keyrange;
  uint64_t max_endkeyhihi = min_startkeyhihi - 1 + perserver_keyrange;
  INVARIANT(min_startkeyhihi >= std::numeric_limits<uint32_t>::min() && min_startkeyhihi <= std::numeric_limits<uint32_t>::max());
  INVARIANT(max_endkeyhihi >= std::numeric_limits<uint32_t>::min() && max_endkeyhihi <= std::numeric_limits<uint32_t>::max());
  INVARIANT(max_endkeyhihi >= min_startkeyhihi);
  netreach_key_t min_startkey(0, 0, 0, min_startkeyhihi);
  netreach_key_t max_endkey(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), max_endkeyhihi);

  char buf[MAX_BUFSIZE];
  int recv_size = 0;
  int rsp_size = 0;

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

  printf("[server.worker%d] ready\n", int(serveridx));
  transaction_ready_threads++;

  while (!transaction_running) {
  }

  while (transaction_running) {

	bool is_timeout = udprecvfrom(server_worker_udpsock_list[serveridx], buf, MAX_BUFSIZE, 0, (struct sockaddr *)&client_addr, &client_addrlen, recv_size, "server.worker");
	if (is_timeout) {
		continue; // continue to check transaction_running
	}

	struct timespec t1, t2, t3;
	CUR_TIME(t1);

	packet_type_t pkt_type = get_packet_type(buf, recv_size);
	switch (pkt_type) {
		case packet_type_t::GETREQ: 
			{
				get_request_t req(buf, recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string())
				val_t tmp_val;
				uint32_t tmp_seq = 0;
				bool tmp_stat = db_wrappers[serveridx].get(req.key(), tmp_val, tmp_seq);
				//COUT_THIS("[server] val = " << tmp_val.to_string())
				get_response_t rsp(req.key(), tmp_val, tmp_stat);
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[serveridx], buf, rsp_size, 0, (struct sockaddr *)&client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::GETREQ_NLATEST:
			{
				get_request_nlatest_t req(buf, recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string())
				val_t tmp_val;
				uint32_t tmp_seq = 0;
				bool tmp_stat = db_wrappers[serveridx].get(req.key(), tmp_val, tmp_seq);
				//COUT_THIS("[server] val = " << tmp_val.to_string())
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				if (tmp_stat) { // key exists
					get_response_latest_seq_t rsp(req.key(), tmp_val, tmp_seq);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				}
				else { // key not exist
					get_response_deleted_seq_t rsp(req.key(), tmp_val, tmp_seq);
					rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				}
				udpsendto(server_worker_udpsock_list[serveridx], buf, rsp_size, 0, (struct sockaddr *)&client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::PUTREQ_SEQ:
			{
				put_request_seq_t req(buf, recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
				bool tmp_stat = db_wrappers[serveridx].put(req.key(), req.val(), req.seq());
				//COUT_THIS("[server] stat = " << tmp_stat)
				put_response_t rsp(req.key(), true);
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[serveridx], buf, rsp_size, 0, (struct sockaddr *)&client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::DELREQ_SEQ:
			{
				del_request_seq_t req(buf, recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string())
				bool tmp_stat = db_wrappers[serveridx].remove(req.key(), req.seq());
				//COUT_THIS("[server] stat = " << tmp_stat)
				del_response_t rsp(req.key(), true);
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[serveridx], buf, rsp_size, 0, (struct sockaddr *)&client_addr, client_addrlen, "server.worker");
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

				// get results from in-memory snapshot in [cur_startkey, cur_endkey]
				//COUT_THIS("[server.worker] startkey: " << cur_startkey << "endkey: " << cur_endkey().to_string()
				//		<< "min_startkey: " << min_startkey << "max_endkey: " << max_endkey; // << " num = " << req.num())
				std::vector<std::pair<netreach_key_t, snapshot_record_t>> inmemory_results;
				size_t inmemory_num = db_wrappers[serveridx].range_scan(cur_startkey, cur_endkey, inmemory_results);
				UNUSED(inmemory_num);

				// get results from in-switch snapshot in [cur_startkey, cur_endkey]
				//std::map<netreach_key_t, snapshot_record_t> *tmp_server_snapshot_maps = server_snapshot_maps;
				concurrent_snapshot_map_t *tmp_server_snapshot_maps = server_snapshot_maps;
				std::vector<std::pair<netreach_key_t, snapshot_record_t>> inswitch_results;
				if (tmp_server_snapshot_maps != NULL) {
					/*std::map<netreach_key_t, snapshot_record_t>::iterator iter = tmp_server_snapshot_maps[serveridx].lower_bound(cur_startkey);
					for (; iter != tmp_server_snapshot_maps[serveridx].end() && iter->first <= cur_endkey; iter++) {
						//inmemory_results.push_back(*iter);
						inswitch_results.push_back(*iter);
					}*/
					tmp_server_snapshot_maps[serveridx].range_scan(cur_startkey, cur_endkey, inswitch_results);
				}

				// merge sort w/ seq comparison
				std::vector<std::pair<netreach_key_t, val_t>> results;
				if (inmemory_results.size() == 0) {
					for (uint32_t inswitch_idx = 0; inswitch_idx < inswitch_results.size(); inswitch_idx++) {
						netreach_key_t &tmpkey = inswitch_results[inswitch_idx].first;
						snapshot_record_t &tmprecord = inswitch_results[inswitch_idx].second;
						if (tmprecord.stat) {
							results.push_back(std::pair<netreach_key_t, val_t>(tmpkey, tmprecord.val));
						}
					}
				}
				else if (inswitch_results.size() == 0) {
					for (uint32_t inmemory_idx = 0; inmemory_idx < inmemory_results.size(); inmemory_idx++) {
						netreach_key_t &tmpkey = inmemory_results[inmemory_idx].first;
						snapshot_record_t &tmprecord = inmemory_results[inmemory_idx].second;
						if (tmprecord.stat) {
							results.push_back(std::pair<netreach_key_t, val_t>(tmpkey, tmprecord.val));
						}
					}
				}
				else {
					uint32_t inmemory_idx = 0;
					uint32_t inswitch_idx = 0;
					bool remain_inmemory = false;
					bool remain_inswitch = false;
					while (true) {
						netreach_key_t &tmp_inmemory_key = inmemory_results[inmemory_idx].first;
						snapshot_record_t &tmp_inmemory_record = inmemory_results[inmemory_idx].second;
						netreach_key_t &tmp_inswitch_key = inswitch_results[inswitch_idx].first;
						snapshot_record_t &tmp_inswitch_record = inswitch_results[inswitch_idx].second;
						if (tmp_inmemory_key < tmp_inswitch_key) {
							if (tmp_inmemory_record.stat) {
								results.push_back(std::pair<netreach_key_t, val_t>(tmp_inmemory_key, tmp_inmemory_record.val));
							}
							inmemory_idx += 1;
						}
						else if (tmp_inswitch_key < tmp_inmemory_key) {
							if (tmp_inswitch_record.stat) {
								results.push_back(std::pair<netreach_key_t, val_t>(tmp_inswitch_key, tmp_inswitch_record.val));
							}
							inswitch_idx += 1;
						}
						else if (tmp_inmemory_key == tmp_inswitch_key) {
							/*if (tmp_inmemory_record.stat && tmp_inswitch_record.stat) {
								if (tmp_inmemory_record.seq >= tmp_inswitch_record.seq) {
									results.push_back(std::pair<netreach_key_t, val_t>(tmp_inmemory_key, tmp_inmemory_record.val));
									inmemory_idx += 1;
								}
								else {
									results.push_back(std::pair<netreach_key_t, val_t>(tmp_inswitch_key, tmp_inswitch_record.val));
									inswitch_idx += 1;
								}
							}
							else if (tmp_inmemory_record.stat) {
								results.push_back(std::pair<netreach_key_t, val_t>(tmp_inmemory_key, tmp_inmemory_record.val));
								inmemory_idx += 1;
							}
							else if (tmp_inswitch_record.stat) {
								results.push_back(std::pair<netreach_key_t, val_t>(tmp_inswitch_key, tmp_inswitch_record.val));
								inswitch_idx += 1;
							}*/
							if (tmp_inmemory_record.seq >= tmp_inswitch_record.seq) {
								if (tmp_inmemory_record.stat) {
									results.push_back(std::pair<netreach_key_t, val_t>(tmp_inmemory_key, tmp_inmemory_record.val));
								}
							}
							else {
								if (tmp_inswitch_record.stat) {
									results.push_back(std::pair<netreach_key_t, val_t>(tmp_inswitch_key, tmp_inswitch_record.val));
								}
							}
							inmemory_idx += 1;
							inswitch_idx += 1;
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
							netreach_key_t &tmpkey = inswitch_results[inswitch_idx].first;
							snapshot_record_t &tmprecord = inswitch_results[inswitch_idx].second;
							if (tmprecord.stat) {
								results.push_back(std::pair<netreach_key_t, val_t>(tmpkey, tmprecord.val));
							}
						}
					}
					else if (remain_inmemory) {
						for (; inmemory_idx < inmemory_results.size(); inmemory_idx++) {
							netreach_key_t &tmpkey = inmemory_results[inmemory_idx].first;
							snapshot_record_t &tmprecord = inmemory_results[inmemory_idx].second;
							if (tmprecord.stat) {
								results.push_back(std::pair<netreach_key_t, val_t>(tmpkey, tmprecord.val));
							}
						}
					}
				} // both inmemory_results and inswitch_results are not empty
				//COUT_THIS("results size: " << results.size());

				scan_response_split_t rsp(req.key(), req.endkey(), req.cur_scanidx(), req.max_scannum(), results.size(), results);
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[serveridx], buf, rsp_size, 0, (struct sockaddr *)&client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::GETREQ_POP: 
			{
				get_request_pop_t req(buf, recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string())
				val_t tmp_val;
				uint32_t tmp_seq = 0;
				bool tmp_stat = db_wrappers[serveridx].get(req.key(), tmp_val, tmp_seq);
				//COUT_THIS("[server] val = " << tmp_val.to_string())
				
				get_response_t rsp(req.key(), tmp_val, tmp_stat);
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[serveridx], buf, rsp_size, 0, (struct sockaddr *)&client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif

				// Trigger cache population if necessary (key exist and not being cached)
				if (tmp_stat) {
				//if (false) {
					bool is_cached_before = server_cached_keyset_list[serveridx].is_exist(req.key());
					if (!is_cached_before) {
						server_cached_keyset_list[serveridx].insert(req.key());
						// Send CACHE_POP to server.popclient
						cache_pop_t *cache_pop_req_ptr = new cache_pop_t(req.key(), tmp_val, tmp_seq, serveridx); // freed by server.popclient
						server_cache_pop_ptr_queue_list[serveridx].write(cache_pop_req_ptr);
					}
				}
				break;
			}
		case packet_type_t::PUTREQ_POP_SEQ: 
			{
				put_request_pop_seq_t req(buf, recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string())
				bool tmp_stat = db_wrappers[serveridx].put(req.key(), req.val(), req.seq());
				//COUT_THIS("[server] val = " << tmp_val.to_string())
				
				put_response_t rsp(req.key(), true);
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[serveridx], buf, rsp_size, 0, (struct sockaddr *)&client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif

				// Trigger cache population if necessary (key exist and not being cached)
				if (tmp_stat) { // successful put
				//if (false) {
					bool is_cached_before = server_cached_keyset_list[serveridx].is_exist(req.key());
					if (!is_cached_before) {
						server_cached_keyset_list[serveridx].insert(req.key());
						// Send CACHE_POP to server.popclient
						cache_pop_t *cache_pop_req_ptr = new cache_pop_t(req.key(), req.val(), req.seq(), serveridx); // freed by server.popclient
						server_cache_pop_ptr_queue_list[serveridx].write(cache_pop_req_ptr);
					}
				}
				break;
			}
		case packet_type_t::PUTREQ_SEQ_CASE3:
			{
				COUT_THIS("PUTREQ_SEQ_CASE3")
				put_request_seq_case3_t req(buf, recv_size);

				if (!server_issnapshot) {
					db_wrappers[serveridx].make_snapshot();
				}

				bool tmp_stat = db_wrappers[serveridx].put(req.key(), req.val(), req.seq());
				//put_response_case3_t rsp(req.hashidx(), req.key(), serveridx, tmp_stat); // no case3_reg in switch
				put_response_t rsp(req.key(), true);
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[serveridx], buf, rsp_size, 0, (struct sockaddr *)&client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif
				break;
			}
		case packet_type_t::PUTREQ_POP_SEQ_CASE3: 
			{
				COUT_THIS("PUTREQ_POP_SEQ_CASE3")
				put_request_pop_seq_case3_t req(buf, recv_size);

				if (!server_issnapshot) {
					db_wrappers[serveridx].make_snapshot();
				}

				//COUT_THIS("[server] key = " << req.key().to_string())
				bool tmp_stat = db_wrappers[serveridx].put(req.key(), req.val(), req.seq());
				//COUT_THIS("[server] val = " << tmp_val.to_string())
				put_response_t rsp(req.key(), true);
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[serveridx], buf, rsp_size, 0, (struct sockaddr *)&client_addr, client_addrlen, "server.worker");
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
#endif

				// Trigger cache population if necessary (key exist and not being cached)
				if (tmp_stat) { // successful put
				//if (false) {
					bool is_cached_before = server_cached_keyset_list[serveridx].is_exist(req.key());
					if (!is_cached_before) {
						server_cached_keyset_list[serveridx].insert(req.key());
						// Send CACHE_POP to server.popclient
						cache_pop_t *cache_pop_req_ptr = new cache_pop_t(req.key(), req.val(), req.seq(), serveridx); // freed by server.popclient
						server_cache_pop_ptr_queue_list[serveridx].write(cache_pop_req_ptr);
					}
				}
				break;
			}
		case packet_type_t::DELREQ_SEQ_CASE3:
			{
				COUT_THIS("DELREQ_SEQ_CASE3")
				del_request_seq_case3_t req(buf, recv_size);

				if (!server_issnapshot) {
					db_wrappers[serveridx].make_snapshot();
				}

				bool tmp_stat = db_wrappers[serveridx].remove(req.key(), req.seq());
				//del_response_case3_t rsp(req.hashidx(), req.key(), serveridx, tmp_stat); // no case3_reg in switch
				del_response_t rsp(req.key(), true);
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				udpsendto(server_worker_udpsock_list[serveridx], buf, rsp_size, 0, (struct sockaddr *)&client_addr, client_addrlen, "server.worker");
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
	CUR_TIME(t2);
	DELTA_TIME(t2, t1, t3);
	thread_param.latency_list.push_back(GET_MICROSECOND(t3));
#ifdef TEST_AGG_THPT
	thread_param.sum_latency += GET_MICROSECOND(t3);
#endif
	//backup_rcu[serveridx]++;
	server_snapshot_rcus[serveridx]++;
	thread_param.throughput++;

  } // end of while(transaction_running)

  close(server_worker_udpsock_list[serveridx]);
  COUT_THIS("[server.worker " << uint32_t(serveridx) << "] exits")
  pthread_exit(nullptr);
}

void *run_server_evictserver(void *param) {
	//uint16_t serveridx = *((uint16_t *)param);

	// Not used
	//struct sockaddr_in controller_addr;
	//unsigned int controller_addr_len = sizeof(struct sockaddr);
	
	printf("[server.evictserver] ready\n");
	transaction_ready_threads++;

	while (!transaction_running) {}

	int connfd = -1;
	while (transaction_running) {
		//tcpaccept(server_evictserver_tcpsock_list[serveridx], NULL, NULL, connfd, "server.evictserver");
		bool is_timeout = tcpaccept(server_evictserver_tcpsock, NULL, NULL, connfd, "server.evictserver");
		if (is_timeout) {
			continue;
		}
		else {
			break;
		}
	}

	if (!transaction_running) {
		if (connfd != -1) {
			close(connfd);
		}
		close(server_evictserver_tcpsock);
		pthread_exit(nullptr);
		return 0;
	}

	INVARIANT(connfd != -1);
	set_recvtimeout(connfd);

	// process CACHE_EVICT/_CASE2 packet <optype, key, vallen, value, result, seq, serveridx>
	char recvbuf[MAX_BUFSIZE];
	int cur_recv_bytes = 0;
	bool direct_parse = false;
	//bool is_broken = false;
	bool is_timeout = false;
	uint8_t optype = 0;
	bool with_optype = false;
	uint32_t vallen = 0;
	bool with_vallen = false;
	const int arrive_optype_bytes = sizeof(uint8_t);
	//const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(netreach_key_t) + sizeof(uint32_t);
	const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(netreach_key_t) + sizeof(uint16_t);
	int arrive_serveridx_bytes = -1;
	char sendbuf[MAX_BUFSIZE]; // used to send CACHE_EVICT_ACK to control;er
	while (transaction_running) {
		if (!direct_parse) {
			int recvsize = 0;
			/*is_broken = tcprecv(connfd, recvbuf + cur_recv_bytes, MAX_BUFSIZE - cur_recv_bytes, 0, recvsize, "server.evictserver");
			if (is_broken) {
				break;
			}*/
			is_timeout = tcprecv(connfd, recvbuf + cur_recv_bytes, MAX_BUFSIZE - cur_recv_bytes, 0, recvsize, "server.evictserver");
			if (is_timeout) {
				continue; // continue to check transaction_running
			}

			cur_recv_bytes += recvsize;
			if (cur_recv_bytes >= MAX_BUFSIZE) {
				printf("[server.evictserver] Overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_BUFSIZE);
				exit(-1);
			}
		}

		// Get optype
		if (!with_optype && cur_recv_bytes >= arrive_optype_bytes) {
			optype = *((uint8_t *)recvbuf);
			INVARIANT(packet_type_t(optype) == packet_type_t::CACHE_EVICT || packet_type_t(optype) == packet_type_t::CACHE_EVICT_CASE2);
			direct_parse = false;
			with_optype = true;
		}

		// Skip key and get vallen
		if (with_optype && !with_vallen && cur_recv_bytes >= arrive_vallen_bytes) {
			//vallen = *((uint32_t *)(recvbuf + arrive_vallen_bytes - sizeof(uint32_t)));
			//vallen = ntohl(vallen);
			vallen = *((uint16_t *)(recvbuf + arrive_vallen_bytes - sizeof(uint16_t)));
			vallen = ntohs(vallen);
			INVARIANT(vallen >= 0);
			int padding_size = int(val_t::get_padding_size(vallen)); // padding for value <= 128B
			arrive_serveridx_bytes = arrive_vallen_bytes + vallen + padding_size + sizeof(uint32_t) + sizeof(bool) + sizeof(uint16_t);
			with_vallen = true;
		}

		// Get one complete CACHE_EVICT/_CASE2 (only need serveridx here)
		if (with_optype && with_vallen && cur_recv_bytes >= arrive_serveridx_bytes) {
			//printf("receive CACHE_EVICT from controller\n");
			//dump_buf(recvbuf, arrive_serveridx_bytes);
			
			cache_evict_t *tmp_cache_evict_ptr;
			if (packet_type_t(optype) == packet_type_t::CACHE_EVICT) {
				tmp_cache_evict_ptr = new cache_evict_t(recvbuf, arrive_serveridx_bytes);
			}
			else if (packet_type_t(optype) == packet_type_t::CACHE_EVICT_CASE2) {
				tmp_cache_evict_ptr = new cache_evict_case2_t(recvbuf, arrive_serveridx_bytes);
			}
			else {
				printf("[server.evictserver] error: invalid optype: %d\n", int(optype));
				exit(-1);
			}

			uint16_t tmp_serveridx = tmp_cache_evict_ptr->serveridx();
			//INVARIANT(tmp_serveridx == serveridx);
			INVARIANT(server_cached_keyset_list[tmp_serveridx].is_exist(tmp_cache_evict_ptr->key()));

			// make server-side snapshot for CACHE_EVICT_CASE2
			if (packet_type_t(optype) == packet_type_t::CACHE_EVICT_CASE2) {
				printf("CACHE_EVICT_CASE2!\n");
				if (!server_issnapshot) {
					db_wrappers[tmp_serveridx].make_snapshot();
				}
			}

			// remove from cached keyset
			server_cached_keyset_list[tmp_serveridx].erase(tmp_cache_evict_ptr->key()); // NOTE: no contention

			// update in-memory KVS if necessary
			if (tmp_cache_evict_ptr->stat()) { // put
				db_wrappers[tmp_serveridx].put(tmp_cache_evict_ptr->key(), tmp_cache_evict_ptr->val(), tmp_cache_evict_ptr->seq());
			}
			else { // del
				db_wrappers[tmp_serveridx].remove(tmp_cache_evict_ptr->key(), tmp_cache_evict_ptr->seq());
			}

			// send CACHE_EVICT_ACK to controller.evictserver.evictclient
			cache_evict_ack_t tmp_cache_evict_ack(tmp_cache_evict_ptr->key());
			int sendsize = tmp_cache_evict_ack.serialize(sendbuf, MAX_BUFSIZE);
			//printf("send CACHE_EVICT_ACK to controller\n");
			//dump_buf(sendbuf, sendsize);
			tcpsend(connfd, sendbuf, sendsize, "server.evictserver");

			// free CACHE_EVIT
			delete tmp_cache_evict_ptr;
			tmp_cache_evict_ptr = NULL;

			// move remaining bytes and reset metadata
			if (cur_recv_bytes > arrive_serveridx_bytes) {
				memcpy(recvbuf, recvbuf + arrive_serveridx_bytes, cur_recv_bytes - arrive_serveridx_bytes);
				cur_recv_bytes = cur_recv_bytes - arrive_serveridx_bytes;
			}
			else {
				cur_recv_bytes = 0;
			}
			if (cur_recv_bytes >= arrive_optype_bytes) {
				direct_parse = true;
			}
			else {
				direct_parse = false;
			}
			//is_broken = false;
			optype = 0;
			with_optype = false;
			vallen = 0;
			with_vallen = false;
			arrive_serveridx_bytes = -1;
		}
	}
	close(server_evictserver_tcpsock);
	pthread_exit(nullptr);
}

void *run_server_consnapshotserver(void *param) {

	// Not used
	//struct sockaddr_in controller_addr;
	//unsigned int controller_addr_len = sizeof(struct sockaddr);
	
	char *recvbuf = new char[MAX_LARGE_BUFSIZE];
	INVARIANT(recvbuf != NULL);
	memset(recvbuf, 0, MAX_LARGE_BUFSIZE);
	printf("[server.consnapshotserver] ready\n");
	transaction_ready_threads++;

	while (!transaction_running) {}

	int connfd = -1;
	while (transaction_running) {
		bool is_timeout = tcpaccept(server_consnapshotserver_tcpsock, NULL, NULL, connfd, "server.consnapshotserver");
		if (is_timeout) {
			continue;
		}
		else {
			break;
		}
	}

	if (!transaction_running) {
		if (connfd != -1) {
			close(connfd);
		}
		delete [] recvbuf;
		recvbuf = NULL;
		close(server_consnapshotserver_tcpsock);
		pthread_exit(nullptr);
		return 0;
	}

	INVARIANT(connfd != -1);
	set_recvtimeout(connfd);

	int cur_recv_bytes = 0;
	bool direct_parse = false;
	//bool is_broken = false;
	bool is_timeout = false;
	int phase = 0; // 0: wait for SNAPSHOT_SERVERSIDE; 1: wait for crash-consistent snapshot data
	int control_type_phase0 = -1;
	int control_type_phase1 = -1;
	int total_bytes = -1;
	while (transaction_running) {
		if (!direct_parse) {
			int recvsize = 0;
			/*is_broken = tcprecv(connfd, recvbuf + cur_recv_bytes, MAX_LARGE_BUFSIZE - cur_recv_bytes, 0, recvsize, "server.consnapshotserver");
			if (is_broken) {
				break;
			}*/
			is_timeout = tcprecv(connfd, recvbuf + cur_recv_bytes, MAX_LARGE_BUFSIZE - cur_recv_bytes, 0, recvsize, "server.consnapshotserver");
			if (is_timeout) {
				continue; // continue to check transaction_running
			}

			cur_recv_bytes += recvsize;
			if (cur_recv_bytes >= MAX_LARGE_BUFSIZE) {
				printf("[server.consnapshotserver] Overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_LARGE_BUFSIZE);
				exit(-1);
			}
		}

		if (phase == 0) {
			// process SNAPSHOT_SERVERSIDE for server-side snapshot
			if (control_type_phase0 == -1 && cur_recv_bytes >= int(sizeof(int))) {
				printf("[server.snapshotserver] receive SNAPSHOT_SERVERISDE from controller\n"); // TMPDEBUG
				control_type_phase0 = *((int *)recvbuf);
				INVARIANT(control_type_phase0 == SNAPSHOT_SERVERSIDE);

				// make server-side snapshot (simulate distributed in-memory KVS by concurrent one)
				if (!server_issnapshot) {
					for (uint16_t i = 0; i < server_num; i++) {
						db_wrappers[i].make_snapshot();
					}
				}

				// send SNAPSHOT_SERVERSIDE_ACK to controller
				printf("[server.snapshotserver] send SNAPSHOT_SERVERSIDE_ACK to controller\n"); // TMPDEBUG
				tcpsend(connfd, (char *)&SNAPSHOT_SERVERSIDE_ACK, sizeof(int), "server.consnapshotserver");

				direct_parse = false;
				phase = 1; // wait for crash-consistent snapshot data
			}
		}
		
		if (phase == 1) {
			// NOTE: skip sizeof(int) for SNAPSHOT_SERVERSIDE
			if (control_type_phase1 == -1 && cur_recv_bytes >= int(sizeof(int) + sizeof(int) + sizeof(int32_t))) { // SNAPSHOT_SERVERSIDE + SNAPSHOT_DATA + total_bytes
				control_type_phase1 = *((int *)(recvbuf + sizeof(int)));
				INVARIANT(control_type_phase1 == SNAPSHOT_DATA);
				total_bytes = *((int32_t *)(recvbuf + sizeof(int) + sizeof(int)));
				server_issnapshot = true;
			}

			// snapshot data: <int SNAPSHOT_DATA, int32_t total_bytes, per-server data>
			// per-server data: <int32_t perserver_bytes, uint16_t serveridx, int32_t recordcnt, per-record data>
			// per-record data: <16B key, uint32_t vallen, value (w/ padding), uint32_t seq, bool stat>
			if (control_type_phase1 != -1 && cur_recv_bytes >= int(sizeof(int) + total_bytes)) { // SNAPSHOT_SERVERSIDE + snapshot data of total_bytes
				// NOTE: per-server_bytes is used for processing snapshot data of each server (not used now)
				
				// save snapshot data of servers
				int tmp_offset = sizeof(int) + sizeof(int) + sizeof(int32_t); // SNAPSHOT_SERVERSIDE + SNAPSHOT_DATA + total_bytes
				//std::map<netreach_key_t, snapshot_record_t> *new_server_snapshot_maps = new std::map<netreach_key_t, snapshot_record_t>[server_num];
				concurrent_snapshot_map_t *new_server_snapshot_maps = new concurrent_snapshot_map_t[server_num];
				const int tmp_maxbytes = sizeof(int) + total_bytes;
				while (true) {
					tmp_offset += sizeof(int32_t); // skip perserver_bytes
					uint16_t tmp_serveridx = *((uint16_t *)(recvbuf + tmp_offset));
					tmp_offset += sizeof(uint16_t);
					int32_t tmp_recordcnt = *((int32_t *)(recvbuf + tmp_offset));
					tmp_offset += sizeof(int32_t);
					for (int32_t tmp_recordidx = 0; tmp_recordidx < tmp_recordcnt; tmp_recordidx++) {
						netreach_key_t tmp_key;
						snapshot_record_t tmp_record;
						uint32_t tmp_keysize = tmp_key.deserialize(recvbuf + tmp_offset, tmp_maxbytes - tmp_offset);
						tmp_offset += tmp_keysize;
						uint32_t tmp_valsize = tmp_record.val.deserialize(recvbuf + tmp_offset, tmp_maxbytes - tmp_offset);
						tmp_offset += tmp_valsize;
						tmp_record.seq = *((uint32_t *)(recvbuf + tmp_offset));
						tmp_offset += sizeof(uint32_t);
						tmp_record.stat = *((bool *)(recvbuf + tmp_offset));
						tmp_offset += sizeof(bool);
						//new_server_snapshot_maps[tmp_serveridx].insert(std::pair<netreach_key_t, snapshot_record_t>(tmp_key, tmp_record));
						new_server_snapshot_maps[tmp_serveridx].insert(tmp_key, tmp_record);
					}
					if (tmp_offset >= tmp_maxbytes) {
						break;
					}
				}

				// TMPDEBUG
				printf("[server.snapshotserver] receive snapshot data from controller\n");
				for (size_t debugi = 0; debugi < server_num; debugi++) {
					printf("snapshot of serveridx: %d\n", int(debugi));
					std::vector<std::pair<netreach_key_t, snapshot_record_t>> debugvec;
					new_server_snapshot_maps[debugi].range_scan(netreach_key_t::min(), netreach_key_t::max(), debugvec);
					printf("debugvec size: %d\n", debugvec.size());
					for (size_t veci = 0; veci < debugvec.size(); veci++) {
						char debugbuf[MAX_BUFSIZE];
						uint32_t debugkeysize = debugvec[veci].first.serialize(debugbuf, MAX_BUFSIZE);
						uint32_t debugvalsize = debugvec[veci].second.val.serialize(debugbuf+debugkeysize, MAX_BUFSIZE-debugkeysize);
						printf("serialized debugvec[%d]:\n", veci);
						dump_buf(debugbuf, debugkeysize+debugvalsize);
						printf("seq: %d, stat %d\n", debugvec[veci].second.seq, debugvec[veci].second.stat?1:0);
					}
				}

				// replace old snapshot data based on RCU
				//std::map<netreach_key_t, snapshot_record_t> *old_server_snapshot_maps = server_snapshot_maps;
				concurrent_snapshot_map_t *old_server_snapshot_maps = server_snapshot_maps;
				server_snapshot_maps = new_server_snapshot_maps;
				if (old_server_snapshot_maps != NULL) {
					uint32_t prev_server_snapshot_rcus[server_num];
					for (uint32_t i = 0; i < server_num; i++) {
						prev_server_snapshot_rcus[i] = server_snapshot_rcus[i];
					}
					for (uint32_t i = 0; i < server_num; i++) {
						while (transaction_running && server_snapshot_rcus[i] == prev_server_snapshot_rcus[i]) {}
					}
					delete [] old_server_snapshot_maps;
					old_server_snapshot_maps = NULL;
				}

				// Move remaining bytes and reset metadata
				if (cur_recv_bytes > int(sizeof(int) + total_bytes)) {
					memcpy(recvbuf, recvbuf + sizeof(int) + total_bytes, cur_recv_bytes - sizeof(int) - total_bytes);
					cur_recv_bytes = cur_recv_bytes - sizeof(int) - total_bytes;
				}
				else {
					cur_recv_bytes = 0;
				}
				if (cur_recv_bytes >= int(sizeof(int))) {
					direct_parse = true;
				}
				else {
					direct_parse = false;
				}
				//is_broken = false;
				phase = 0;
				control_type_phase0 = -1;
				control_type_phase1 = -1;
				total_bytes = -1;

				for (uint16_t i = 0; i < server_num; i++) {
					db_wrappers[i].stop_snapshot();
				}
				server_issnapshot = false;
			}
		}
	}
	delete [] recvbuf;
	recvbuf = NULL;
	close(server_consnapshotserver_tcpsock);
	pthread_exit(nullptr);
}

#endif
