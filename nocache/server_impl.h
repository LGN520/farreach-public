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

//#define DUMP_BUF

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

void prepare_server();
void *run_server_worker(void *param); // server.workers for processing pkts
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
		//prepare_udpserver(server_worker_udpsock_list[tmp_local_server_logical_idx], true, server_worker_port_start + tmp_global_server_logical_idx, "server.worker", SOCKET_TIMEOUT, 0, UDP_LARGE_RCVBUFSIZE);
		prepare_udpserver(server_worker_udpsock_list[tmp_local_server_logical_idx], true, server_worker_port_start + tmp_local_server_logical_idx, "server.worker", SOCKET_TIMEOUT, 0, UDP_LARGE_RCVBUFSIZE);
		printf("prepare udp socket for server.worker %d-%d on port %d\n", tmp_local_server_logical_idx, tmp_global_server_logical_idx, server_worker_port_start + tmp_local_server_logical_idx);
	}
	server_worker_lwpid_list = new int[current_server_logical_num];
	memset(server_worker_lwpid_list, 0, current_server_logical_num);

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
				bool tmp_stat = db_wrappers[local_server_logical_idx].get(req.key(), tmp_val);
				//COUT_THIS("[server] val = " << tmp_val.to_string())
				get_response_t rsp(req.key(), tmp_val, tmp_stat, global_server_logical_idx);
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
		case packet_type_t::PUTREQ:
			{
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif

#ifdef DEBUG_SERVER
				CUR_TIME(rocksdb_t1);
#endif

				put_request_t req(buf, recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string() << " val = " << req.val().to_string())
				bool tmp_stat = db_wrappers[local_server_logical_idx].put(req.key(), req.val());
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
		case packet_type_t::DELREQ:
			{
				del_request_t req(buf, recv_size);
				//COUT_THIS("[server] key = " << req.key().to_string())
				bool tmp_stat = db_wrappers[local_server_logical_idx].remove(req.key());
				UNUSED(tmp_stat);
				//COUT_THIS("[server] stat = " << tmp_stat)
				del_response_t rsp(req.key(), true, global_server_logical_idx);
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

				scan_response_split_t rsp(req.key(), req.endkey(), req.cur_scanidx(), req.max_scannum(), global_server_logical_idx, 0, results.size(), results);
#ifdef DUMP_BUF
				dump_buf(buf, recv_size);
#endif
				//rsp_size = rsp.serialize(buf, MAX_BUFSIZE);
				//udpsendto(server_worker_udpsock_list[local_server_logical_idx], buf, rsp_size, 0, &client_addr, client_addrlen, "server.worker");
				scanbuf.clear();
				rsp_size = rsp.dynamic_serialize(scanbuf);
				udpsendlarge_ipfrag(server_worker_udpsock_list[local_server_logical_idx], scanbuf.array(), rsp_size, 0, &client_addr, client_addrlen, "server.worker", scan_response_split_t::get_frag_hdrsize());
#ifdef DUMP_BUF
				dump_buf(buf, rsp_size);
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

	if (likely(pkt_type != packet_type_t::LOADREQ)) {
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

#endif
