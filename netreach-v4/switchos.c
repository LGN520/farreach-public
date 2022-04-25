#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <fstream>
#include <set>
#include <signal.h> // for signal and raise
#include <arpa/inet.h> // inetaddr conversion
#include <sys/time.h> // struct timeval
#include <string.h>
#include <map>

#include "helper.h"
#include "key.h"
#include "val.h"
#include "socket_helper.h"
#include "special_case.h"
#include "message_queue_impl.h"

// ptf scripts used by switchos
// cache eviction: get_evictdata_setvalid3.sh, remove_cache_lookup.sh
// cache population: setvalid0.sh, add_cache_lookup_setvalid1.sh
// snapshot: set_snapshot_flag.sh

typedef Key index_key_t;
typedef Val val_t;
typedef SpecialCase special_case_t;

bool volatile switchos_running = false;
std::atomic<size_t> switchos_ready_threads(0);
const size_t switchos_expected_ready_threads = 5;
bool volatile switchos_popserver_finish = false;

// Parameters
short switchos_popserver_port = 0;
short switchos_paramserver_port = 0;
uint32_t switch_kv_bucket_num = 0;
//uint32_t switchos_sample_cnt = 0; // sample_cnt is only used by ptf (transparent for switchos)
const char *reflector_ip_for_switchos = nullptr;
short reflector_popserver_port = -1;
const char *controller_ip_for_switchos = nullptr;
short controller_evictserver_port = -1;
short switchos_snapshotserver_port = -1;
short switchos_specialcaseserver_port = -1;

// Cache population

// controller.popclient <-> switchos.popserver
int volatile switchos_popserver_tcpsock = -1;
int volatile switchos_paramserver_udpsock = -1;
//std::set<index_key_t> switchos_cached_keyset; // TODO: Comment it after checking server.cached_keyset_list
// message queue between switchos.popserver and switchos.popworker
message_ptr_queue_t<cache_pop_t> switchos_cache_pop_ptr_queue(MQ_SIZE);
/*cache_pop_t ** volatile switchos_cache_pop_ptrs = NULL;
uint32_t switchos_head_for_pop;
uint32_t switchos_tail_for_pop;*/

// switchos.popworker
index_key_t * volatile switchos_cached_keyarray = NULL; // idx (of inswitch KV) -> key (TODO: different switches for distributed case)
int16_t * volatile switchos_cached_serveridxarray = NULL; // idx (of inswitch KV) -> serveridx of the key
uint32_t volatile switchos_cached_keyarray_empty_index = 0; // [empty index, kv_bucket_num-1] is empty
std::map<index_key_t, int16_t> volatile switchos_key_idx_map; // key -> idx (of inswitch KV)
int volatile switchos_popworker_popclient_udpsock = -1;

// Used by switchos.paramserer <-> ptf framework
int16_t volatile switchos_freeidx = -1; // switchos.popworker write -> launch ptf.population -> switchos.paramserver read (no contention)
index_key_t volatile switchos_newkey = index_key_t();

// Cache eviction

bool volatile is_switchos_popworker_evictclient_connected = false;
int volatile switchos_popworker_evictclient_tcpsock = -1;

// Used by switchos.paramserer <-> ptf framework
// launch ptf.eviction -> ptf sets evicted information -> switchos.paramserver read
bool volatile switchos_with_evictdata = false;
int16_t volatile switchos_evictidx = -1;
int32_t volatile switchos_evictvallen = 0;
char * volatile switchos_evictvalbytes = NULL;
bool volatile switchos_evictstat = false;
int32_t volatile switchos_evictseq = -1;

// Packet types used by switchos.paramserver and ptf framework
const int SWITCHOS_GET_FREEIDX = 1; // ptf get freeidx from paramserver
const int SWITCHOS_GET_KEY_FREEIDX = 2; // ptf get key and freeidx from paramserver
const int SWITCHOS_SET_EVICTDATA = 3; // ptf set evictidx, evictvallen, evictval, evictstat, and evictseq to paramserver
const int SWITCHOS_GET_EVICTKEY = 4; // ptf get evictkey
//const int SWITCHOS_GET_EVICTKEY_EVICTIDX = 4; // ptf get evictkey and evictidx

// Snapshot

// snapshotserver socket
int volatile switchos_snapshotserver_tcpsock = -1;

// back cache metadata with atomicity
bool volatile is_snapshot_prepare = false;
bool volatile popworker_know_snapshot_prepare = false; // ensure current cache population/eviction is finished before metadata backup
// NOTE: not need specialcaseserver_know_snapshot_prepare as switchos_specialcases has been cleared in the previous snapshot period
bool volatile popworker_know_snapshot_end = false; // ensure current case2 is reported before rollback
bool volatile specialcaseserver_know_snapshot_end = false; // ensure current case1s are reported before rollback

// specialcaseserver socket
int volatile switchos_specialcaseserver_udpsock = -1;
std::map<int16_t, special_case_t> volatile switchos_specialcases;

inline void parse_ini(const char *config_file);
void prepare_switchos();
void *run_switchos_popserver(void *param);
void *run_switchos_paramserver(void *param);
void *run_switchos_popworker(void *param);
void *run_switchos_snapshotserver(void *param);
void *run_switchos_specialcaseserver(void *param);
void close_switchis();

int main(int argc, char **argv) {
	parse_ini("config.ini");

	prepare_switchos();

	pthread_t popserver_thread;
	int ret = pthread_create(&popserver_thread, nullptr, run_switchos_popserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t paramserver_thread;
	int ret = pthread_create(&paramserver_thread, nullptr, run_switchos_paramserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t popworker_thread;
	int ret = pthread_create(&popworker_thread, nullptr, run_switchos_popworker, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	while (switchos_ready_threads < switchos_expected_ready_threads) sleep(1);

	switchos_running = true;

	while (!switchos_popserver_finish) {}

	switchos_running = false;

	int rc = pthread_join(popsever_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(paramserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(popworker_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}

	close_switchos();
}

// TODO: try common_impl.h
inline void parse_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	switchos_popserver_port = ini.get_switchos_popserver_port();
	switchos_paramserver_port = ini.get_switchos_paramserver_port();
	switch_kv_bucket_num = ini.get_switch_kv_bucket_num();
	switchos_sample_cnt = ini.get_switchos_sample_cnt();
	val_t::SWITCH_MAX_VALLEN = ini.get_switch_max_vallen();
	val_t::MAX_VALLEN = ini.get_max_vallen();
	reflector_ip_for_switchos = ini.get_reflector_ip_for_switchos();
	reflector_popserver_port = ini.get_reflector_popserver_port();
	controller_ip_for_switchos = ini.get_controller_ip_for_switchos();
	controller_evictserver_port = ini.get_controller_evictserver_port();
	switchos_snapshotserver_port = ini.get_switchos_snapshotserver_port();
	switchos_specialcasesever_port = ini.get_switchos_specialcaseserver_port();
	
	COUT_VAR(switchos_popserver_port);
	COUT_VAR(switchos_paramserver_port);
	COUT_VAR(switch_kv_bucket_num);
	COUT_VAR(switchos_sample_cnt);
	COUT_VAR(val_t::SWITCH_MAX_VALLEN);
	COUT_VAR(val_t::MAX_VALLEN);
	printf("reflector ip for switchos: %s\n", reflector_ip_for_switchos);
	COUT_VAR(reflector_popserver_port);
	printf("controller ip for switchos: %s\n", controller_ip_for_switchos);
	COUT_VAR(controller_evictserver_port);
	COUT_VAR(switchos_snapshotserver_port);
	COUT_VAR(switchos_specialcaseserver_port);
}

void prepare_switchos() {
	// prepare popserver socket
	prepare_tcpserver(switchos_popserver_tcpsock, false, switchos_popserver_port, 1, "switchos.popserver"); // MAX_PENDING_CONNECTION = 1

	// prepare paramserver socket
	prepare_udpserver(switchos_paramserver_udpsock, true, switchos_paramserver_port, "switchos.paramserver");

	//switchos_cached_keyset.clear();
	/*switchos_cache_pop_ptrs = new cache_pop_t*[MQ_SIZE];
	switchos_head_for_pop = 0;
	switchos_tail_for_pop = 0;*/

	create_udpsock(switchos_popworker_popclient_udpsock, "switchos.popworker");
	switchos_cached_keyarray = new index_key_t[switch_kv_bucket_num]();
	switchos_cached_serveridxarray = new int16_t[switch_kv_bucket_num](-1);
	switchos_cached_keyarray_empty_index = 0;
	switchos_key_idx_map.clear();

	create_tcpsock(switchos_popworker_evictclient_tcpsock, "switchos.popworker.evictclient");
	switchos_evictvalbytes = new char[val_t::MAX_VALLEN];
	INVARIANT(switchos_evictvalbytes != NULL);
	memset(switchos_evictvalbytes, 0, val_t::MAX_VALLEN);

	// prepare snapshotserver socket
	prepare_tcpserver(switchos_snapshotserver_tcpsock, false, switchos_snapshotserver_port, 1, "switchos.snapshotserver"); // MAX_PENDING_CONNECTION = 1

	// prepare specialcaseserver socket
	prepare_udpserver(switchos_specialcaseserver_udpsock, true, switchos_specialcaseserver_port, "switchos.specialcaseserver");

	switchos_specialcases.clear();
}

void *run_switchos_popserver(void *param) {
	// Not used
	struct sockaddr_in controller_addr;
	unsigned int controller_addr_len = sizeof(struct sockaddr);
	switchos_ready_threads++;

	while (!switchos_running) {}

	// accept connection from controller.popclient
	int connfd = -1;
	tcpaccept(switchos_popserver_tcpsock, NULL, NULL, connfd, "switchos.popserver");

	// Process CACHE_POP packet <optype, key, vallen, value, seq, serveridx>
	char buf[MAX_BUFSIZE];
	int cur_recv_bytes = 0;
	int8_t optype = -1;
	int32_t vallen = -1;
	bool is_cached_before = false; // TODO: remove
	//index_key_t tmpkey(0, 0, 0, 0);
	const int arrive_optype_bytes = sizeof(int8_t);
	const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(key_t) + sizeof(int32_t);
	int arrive_serveridx_bytes = -1;
	while (switchos_running) {
		int recvsize = 0;
		bool is_broken = tcprecv(connfd, buf + cur_recv_bytes, MAX_BUFSIZE - cur_recv_bytes, 0, recvsize, "switchos.popserver");
		if (is_broken) {
			break;
		}

		cur_recv_bytes += recvsize;
		if (cur_recv_bytes >= MAX_BUFSIZE) {
			printf("[switch os] Overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_BUFSIZE);
            exit(-1);
		}

		// Get optype
		if (optype == -1 && cur_recv_bytes >= arrive_optype_bytes) {
			optype = *((int8_t *)buf);
			INVARIANT(packet_type_t(optype) == packet_type_t::CACHE_POP);
		}

		// Get vallen
		if (optype != -1 && vallen == -1 && cur_recv_bytes >= arrive_vallen_bytes) {
			//tmpkey.deserialize(buf + arrive_optype_bytes, cur_recv_bytes - arrive_optype_bytes);
			vallen = *((int32_t *)(buf + arrive_vallen_bytes - sizeof(int32_t)));
			valen = int32_t(ntohl(uint32_t(vallen)));
			INVARIANT(vallen >= 0);
			int padding_size = int(val_t::get_padding_size(vallen)); // padding for value <= 128B
			arrive_serveridx_bytes = arrive_vallen_bytes + vallen + padding_size + sizeof(int32_t) + sizeof(int16_t);
		}

		// Get one complete CACHE_POP
		if (optype != -1 && vallen != -1 && cur_recv_bytes >= arrive_serveridx_bytes) {
			cache_pop_t *tmp_cache_pop_ptr = new cache_pop_t(buf, arrive_serveridx_bytes); // freed by switchos.popworker

			//is_cached_before = (switchos_cached_keyset.find(tmp_cache_pop_ptr->key()) != switchos_cached_keyset.end());
			if (!is_cached_before) {
				// Add key into cached keyset
				//switchos_cached_keyset.insert(tmp_cache_pop_ptr->key());

				bool res = switchos_cache_pop_ptr_queue.write(tmp_cache_pop_ptr);
				if (!res) {
					printf("[switch os] message queue overflow of switchos.switchos_cache_pop_ptr_queue!");
				}
				/*if ((switchos_head_for_pop+1)%MQ_SIZE != switchos_tail_for_pop) {
					switchos_cache_pop_ptrs[switchos_head_for_pop] = tmp_cache_pop_ptr;
					switchos_head_for_pop = (switchos_head_for_pop + 1) % MQ_SIZE;
				}
				else {
					printf("[switch os] message queue overflow of switchos.switchos_cache_pop_ptrs!");
				}*/
			}

			// Move remaining bytes and reset metadata
			if (cur_recv_bytes > arrive_serveridx_bytes) {
				memcpy(buf, buf + arrive_serveridx_bytes, cur_recv_bytes - arrive_serveridx_bytes);
				cur_recv_bytes = cur_recv_bytes - arrive_serveridx_byets;
			}
			else {
				cur_recv_bytes = 0;
			}
			optype = -1;
			vallen = -1;
			arrive_serveridx_bytes = -1;
			is_cached_before = false;
		}
	}

	switchos_popserver_finish = true;
	close(connfd);
	close(switchos_popserver_tcpsock);
	pthread_exit(nullptr);
}

void *run_switchos_paramserver(void *param) {
	char buf[MAX_BUFSIZE];
	switchos_ready_threads++;
	while (!switchos_running) {}

	while (switchos_running) {
		memset(buf, 0, MAX_BUFSIZE);

		// ptf framework is in the same device of switch os
		struct sockaddr_in ptf_addr;
		unsigned int ptf_addr_len = sizeof(struct sockaddr);

		int recv_size = 0;
		bool is_timeout = udprecvfrom(switchos_paramserver_udpsock, buf, MAX_BUFSIZE, 0, (struct sockaddr *)&ptf_addr, &ptf_addr_len, recv_size, "switchos.paramserver");
		if (is_timeout) {
			continue;
		}

		int tmp_type = *((int *)buf);
		if (tmp_type == SWITCHOS_GET_FREEIDX) {
			memset(buf, 0, MAX_BUFSIZE); // use buf as sendbuf

			// ptf get freeidx -> popworker send switchos_freeidx to ptf framework
			memcpy(buf, (void *)&switchos_freeidx, sizeof(int16_t));
			udpsendto(switchos_paramserver_udpsock, buf, sizeof(int16_t), 0, (struct sockaddr *)ptf_addr, ptf_addr_len, "switchos.paramserver");
		}
		else if (tmp_type == SWITCHOS_GET_KEY_FREEIDX) {
			memset(buf, 0, MAX_BUFSIZE); // use buf as sendbuf

			// ptf get key and freeidx -> popworker send key and switchos_freeidx to ptf framework
			uint32_t tmp_keysize = switchos_newkey.serialize(buf, MAX_BUFSIZE);
			memcpy(buf + tmp_keysize, (void *)&switchos_freeidx, sizeof(int16_t));
			udpsendto(switchos_paramserver_udpsock, buf, tmp_keysize + sizeof(int16_t), 0, (struct sockaddr *)ptf_addr, ptf_addr_len, "switchos.paramserver");
		}
		else if (tmp_type == SWITCHOS_SET_EVICTDATA) {
			char *curptr = buf; // continue to use buf as recvbuf
			curptr += sizeof(int); // skip tmp_type

			// ptf set evict data -> popworker get evictdata from ptf framework
			switchos_evictidx = *((int16_t *)curptr);
			curptr += sizeof(int16_t);
			switchos_evictvallen = *((int32_t *)curptr);
			curptr += sizeof(int32_t);
			int32_t tmp_valbytesnum = switchos_evictvallen + val_t::get_padding_size(switchos_evictvallen);
			memcpy(switchos_evictvalbytes, curptr, tmp_valbytesnum);
			curptr += tmp_valbytesnum;
			switchos_evictstat = *((bool *)curptr);
			curptr += sizeof(bool);
			switchos_evictseq = *((int32_t *)curptr);

			switchos_with_evictdata = true;
		}
		else if (tmp_type == SWITCHOS_GET_EVICTKEY) {
			memset(buf, 0, MAX_BUFSIZE); // use buf as sendbuf

			// ptf get evictkey to remove evicted data from cache_lookup_tbl -> popworker send evictkey to ptf framework
			// NOTE: we reset cached_keyarray[evictidx] after removing evicted data from cache_lookup_tbl, so here the evicted key is correct
			uint32_t tmp_keysize = switchos_cached_keyarray[switchos_evictidx].serialize(buf, MAX_BUFSIZE);
			udpsendto(switchos_paramserver_udpsock, buf, tmp_keysize, 0, (struct sockaddr *)ptf_addr, ptf_addr_len, "switchos.paramserver");
			//memcpy(buf + tmp_keysize, (void *)&switchos_evictidx, sizeof(int16_t));
			//udpsendto(switchos_paramserver_udpsock, buf, tmp_keysize + sizeof(int16_t), 0, (struct sockaddr *)ptf_addr, ptf_addr_len, "switchos.paramserver");
		}
		else {
			printf("[switch os] invalid requset type from ptf to paramserver: %d\n", tmp_type);
			exit(-1);
		}

	}

	close(switchos_paramserver_udpsock);
	pthread_exit(nullptr);
}

void *run_switchos_popworker(void *param) {
	// used by udp socket for cache population
	sockaddr_in reflector_popserver_addr;
	set_sockaddr(reflector_popserver_addr, inet_addr(reflector_ip_for_switchos), reflector_popserver_port);
	int reflector_popserver_addr_len = sizeof(struct sockaddr);

	switchos_ready_threads++;

	while (!switchos_running) {}

	while (switchos_running) {
		char pktbuf[MAX_BUFSIZE];
		uint32_t pktsize = 0;
		char evictclient_buf[MAX_BUFSIZE];
		int evictclient_cur_recv_bytes = 0;
		const int evictclient_arrive_key_bytes = sizeof(int8_t) + sizeof(key_t);
		cache_pop_t *tmp_cache_pop_ptr = switchos_cache_pop_ptr_queue.read();
		if (is_snapshot_prepare && !popworker_know_snapshot_prepare) {
			popworker_know_snapshot_prepare = true;
		}
		else if (!is_snapshot_prepare && tmp_cache_pop_ptr != NULL) {
		//if (switchos_tail_for_pop != switchos_head_for_pop) {
			if (!is_switchos_popworker_evictclient_connected) {
				// used by tcp socket for cache eviction
				tcpconnect(switchos_popworker_evictclient_tcpsock, controller_ip_for_switchos, controller_evictserver_port, "switchos.popworker.evictclient", "controller.evictserver");
				is_switchos_popworker_evictclient_connected = true;
			}

			//cache_pop_t *tmp_cache_pop_ptr = switchos_cache_pop_ptrs[switchos_tail_for_pop];
			//INVARIANT(tmp_cache_pop_ptr != NULL);
			switchos_newkey = tmp_cache_pop_ptr->key();

			// assign switchos_freeidx for new record 
			if (switchos_cached_keyarray_empty_index < switch_kv_bucket_num) { // With free idx
				switchos_freeidx = int16_t(switchos_cached_keyarray_empty_index);
				switchos_cached_keyarray_empty_index += 1;
			}
			else { // Without free idx
				// get evictdata from ptf framework 
				system("bash tofino/get_evictdata_setvalid3.sh");

				// wait for paramserver to get evictdata
				while (!switchos_with_evictdata) {}

				// switchos.popworker.evictclient sends CACHE_EVICT to controller.evictserver
				INVARIANT(switchos_evictidx >= 0 && switchos_evictidx < switchos_kv_bucket_num);
				cache_evict_t tmp_cache_evict(switchos_cached_keyarray(switchos_evictidx), \
						val_t(switchos_evictvalbytes, switchos_evictvallen), \
						switchos_evictseq, switchos_evictstat, \
						switchos_cached_serveridxarray(switchos_evictidx))
				pktsize = tmp_cache_evict.serialize(pktbuf, MAX_BUFSIZE);
				tcpsend(switchos_popworker_evictclient_tcpsock, pktbuf, pktsize, "switchos.popworker.evictclient");

				// wait for CACHE_EVICT_ACK from controller.evictserver
				while (true) {
					int evictclient_recvsize = 0;
					bool evictclient_is_broken = tcprecv(switchos_popworker_evictclient_tcpsock, evictclient_buf + evictclient_cur_recv_bytes, MAX_BUFSIZE - evictclient_cur_recv_bytes, 0, evictclient_recvsize, "switchos.popworker.evictclient");
					if (evictclient_is_broken) {
						break;
					}

					evictclient_cur_recv_bytes += evictclient_recvsize;
					if (evictclient_cur_recv_bytes >= MAX_BUFSIZE) {
						printf("[switchos.popworker.evictclient] Overflow: cur received bytes (%d), maxbufsize (%d)\n", evictclient_cur_recv_bytes, MAX_BUFSIZE);
						exit(-1);
					}

					// get CACHE_EVICT_ACK from controller.evictserver
					if (evictclient_cur_recv_bytes >= evictclient_arrive_key_bytes) {
						int8_t evictclient_optype = *((int8_t *)evictclient_buf);
						INVARIANT(packet_type_t(evictclient_optype) == packet_type_t::CACHE_EVICT_ACK);

						// move remaining bytes and reset metadata
						if (evictclient_cur_recv_bytes > evictclient_arrive_key_bytes) {
							memcpy(evictclient_buf, evictclient_buf + evictclient_arrive_key_bytes, evictclient_cur_recv_bytes - evictclient_arrive_key_bytes);
							evictclient_cur_recv_bytes = evictclient_cur_recv_bytes - evictclient_arrive_key_byets;
						}
						else {
							evictclient_cur_recv_bytes = 0;
						}
					}
				}

				// remove evicted data from cache_lookup_tbl
				system("bash tofino/remove_cache_lookup.sh")

				// reset keyarray and serveridxarray at evictidx
				switchos_cached_keyarray[switchos_evictidx] = index_key_t();
				switchos_cached_serveridxarray[switchos_evictidx] = -1;
				switchos_key_idx_map.erase(tmp_cache_evict.key());

				// set freeidx as evictidx for cache popluation later
				switchos_freeidx = switchos_evictidx;
			}

			/* cache population for new record */

			INVARIANT(switchos_freeidx >= 0 && switchos_freeidx < switch_kv_bucket_num);

			// set valid=0 for atomicity
			system("bash tofino/setvalid0.sh");
			// send CACHE_POP_INSWITCH to reflector (TODO: try internal pcie port)
			cache_pop_inswitch_t tmp_cache_pop_inswitch(tmp_cache_pop_ptr->key(), tmp_cache_pop_ptr->val(), tmp_cache_pop_ptr->seq(), switchos_freeidx);
			pktsize = tmp_cache_pop_inswitch.serialize(pktbuf, MAX_BUF_SIZE);
			udpsendto(switchos_popworker_popclient_udpsock, pktbuf, pktsize, 0, (struct sockaddr *)&reflector_popserver_addr, reflector_popserver_addr_len, "switchos.popworker.udpsock");
			// loop until receiving corresponding ACK (ignore unmatched ACKs which are duplicate ACKs of previous cache population)
			while (true) {
				int recv_size = 0;
				udprecvfrom(switchos_popworker_popclient_udpsock, pktbuf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "switchos.popworker.popclient");
				cache_pop_inswitch_ack_t tmp_cache_pop_inswitch_ack(pktbuf, recv_size);
				if (tmp_cache_pop_inswitch_ack.key() == tmp_cache_pop_ptr->key()) {
					break;
				}
			}
			// (1) add new <key, value> pair into cache_lookup_tbl; (2) and set valid=1 to enable the entry
			system("bash add_cache_lookup_setvalid1.sh");

			// free CACHE_POP
			delete tmp_cache_pop_ptr;
			tmp_cache_pop_ptr = NULL;
			//switchos_cache_pop_ptrs[switchos_tail_for_pop] = NULL;
			//switchos_tail_for_pop = (switchos_tail_for_pop + 1) % MQ_SIZE;

			switchos_cached_keyarray[switchos_freeidx] = tmp_cache_pop_ptr->key();
			switchos_cached_serveridxarray[switchos_freeidx] = tmp_cache_pop_ptr->serveridx();
			switchos_key_idx_map.insert(std::pair<index_key_t, int16_t>(tmp_cache_pop_ptr->key(), switchos_freeidx));

			// reset intermediate data for paramserver
			// for cache population
			switchos_freeidx = -1;
			switchos_newkey = index_key_t();
			// for cache eviction
			switchos_with_evictdata = false;
			switchos_evictidx = -1;
			switchos_evictvallen = 0;
			memset(switchos_evictvalbytes, 0, val_t::MAX_VALLEN);
			switchos_evictstat = false;
			switchos_evictseq = -1;
		}
	}

	close(switchos_popworker_popclient_udpsock);
	close(switchos_popworker_evictclient_tcpsock);
	pthread_exit(nullptr);
}

void *run_switchos_snapshotserver(void *param) {
	// Not used
	struct sockaddr_in controller_addr;
	unsigned int controller_addr_len = sizeof(struct sockaddr);
	switchos_ready_threads++;

	while (!switchos_running) {}

	// accept connection from controller.snapshotclient
	int connfd = -1;
	tcpaccept(switchos_snapshotserver_tcpsock, NULL, NULL, connfd, "switchos.snapshotserver");

	char recvbuf[MAX_BUFSIZE];
	int cur_recv_bytes = 0;
	int phase = 0;
	int tmptype = -1;
	// TODO: a large sendbuf
	while (switchos_running) {
		int recvsize = 0;
		bool is_broken = tcprecv(connfd, recvbuf + cur_recv_bytes, MAX_BUFSIZE - cur_recv_bytes, 0, recvsize, "switchos.snapshotserver");
		if (is_broken) {
			break;
		}

		cur_recv_bytes += recvsize;
		if (cur_recv_bytes >= MAX_BUFSIZE) {
			printf("[switchos.snapshotserver] overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_BUFSIZE);
            exit(-1);
		}

		if (phase == 0) { // wait for SNAPSHOT_START
			// Get packet type related with snapshot
			if (tmptype == -1 && cur_recv_bytes >= sizeof(int)) {
				tmptype = *((int *)buf);
				INVARIANT(tmptype == SNAPSHOT_START);
			}

			// stop cache population/eviction
			is_snapshot_prepare = true;

			// wait until popworker_know_snapshot_prepare = true
			while (!popworker_know_snapshot_prepare) {}
			// by now, cache population/eviction is temporarily stopped -> snapshotserver can backup cache metadata atomically

			// ptf sets snapshot flag as true atomically
			system("bash tofino/set_snapshot_flag.sh");

			// TODO: reset metadata for next phase
		}
	}

	close(switchos_snapshotserver_tcpsock);
	pthread_exit(nullptr);
}

void *run_switchos_specialcaseserver(void *param) {
	// reflector addr (not used) 
	//struct sockaddr_in reflector_addr;
	//unsigned int reflector_addr_len = sizeof(struct sockaddr);

	char buf[MAX_BUFSIZE];
	switchos_ready_threads++;
	while (!switchos_running) {}

	while (switchos_running) {
		memset(buf, 0, MAX_BUFSIZE);

		if (is_snapshot || (is_snapshot_end && !specialcaseserver_know_snapshot_end)) {
			int recv_size = 0;
			bool is_timeout = udprecvfrom(switchos_specialcaseserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "switchos.specialcaseserver");
			if (is_timeout) {
				if (is_snapshot_end && !specialcaseserver_know_snapshot_end) {
					specialcaseserver_know_snapshot_end = true;
				}
				continue;
			}

			packet_type_t pkt_type = get_packet_type(buf, recv_size);
			switch (pkt_type) {
				case packet_type_t::GETRES_LATEST_SEQ_CASE1:
					{
						getres_latest_seq_case1_t req(buf, recv_size);

						// verify key (TODO: use switchos_key_idx_map_backup)
						std::map<index_key_t, int16_t>::iterator key_idx_map_iter = switchos_key_idx_map.find(req.key());
						if (key_idx_map_iter == switchos_key_idx_map.end()) { // unmatched key
							break;
						}
						int16_t tmpidx = key_idx_map_iter->second;

						if (switchos_specialcases.find(tmpidx) == switchos_specialcases.end()) { // no special case for the idx/key
							special_case_t tmpcase;
							tmpcase._key = req.key();
							tmpcase._val = req.val();
							tmpcase._valid = req.stat(); // stat=1 means not deleted
							switchos_specialcases.insert(std::pair<int16_t, special_case_t>(tmpidx, tmpcase));
						}

						break;
					}
			} // end of switch
		} // is_snapshot || (is_snapshot_end && !specialcaseserver_know_snapshot_end)
	}

	close(switchos_specialcaseserver_udpsock);
	pthread_exit(nullptr);
}

void close_switchos() {
	/*if (switchos_cache_pop_ptrs != NULL) {
		for (size_t i = 0; i < MQ_SIZE; i++) {
			if (switchos_cache_pop_ptrs[i] != NULL) {
				delete switchos_cache_pop_ptrs[i];
				switchos_cache_pop_ptrs[i] = NULL;
			}
		}
		delete [] switchos_cache_pop_ptrs;
		switchos_cache_pop_ptrs = NULL;
	}*/
	if (switchos_cached_keyarray != NULL) {
		delete [] switchos_cached_keyarray;
		switchos_cached_keyarray = NULL;
	}
	if (switchos_cached_serveridxarray != NULL) {
		delete [] switchos_cached_serveridxarray;
		switchos_cached_serveridxarray = NULL;
	}
	if (switchos_evictvalbytes != NULL) {
		delete [] switchos_evictvalbytes;
		switchos_evictvalbytes = NULL;
	}
}
