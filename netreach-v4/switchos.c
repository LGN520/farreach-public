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
#include <mutex>

#include "helper.h"
#include "key.h"
#include "val.h"
//#include "common_impl.h"
//
#include "socket_helper.h"
#include "special_case.h"
#include "iniparser/iniparser_wrapper.h"
#include "message_queue_impl.h"
#include "packet_format_impl.h"
#include "concurrent_map_impl.h"

// ptf scripts used by switchos
// cache eviction: get_evictdata_setvalid3.sh, remove_cache_lookup.sh
// cache population: setvalid0.sh, add_cache_lookup_setvalid1.sh
// snapshot: set_snapshot_flag.sh, load_snapshot_data.sh

typedef Key index_key_t;
typedef Val val_t;
typedef SpecialCase special_case_t;
typedef GetResponseLatestSeqInswitchCase1<index_key_t, val_t> getres_latest_seq_inswitch_case1_t;
typedef GetResponseDeletedSeqInswitchCase1<index_key_t, val_t> getres_deleted_seq_inswitch_case1_t;
typedef PutRequestSeqInswitchCase1<index_key_t, val_t> putreq_seq_inswitch_case1_t;
typedef DelRequestSeqInswitchCase1<index_key_t, val_t> delreq_seq_inswitch_case1_t;
typedef CachePop<index_key_t, val_t> cache_pop_t;
typedef CachePopInswitch<index_key_t, val_t> cache_pop_inswitch_t;
typedef CachePopInswitchAck<index_key_t> cache_pop_inswitch_ack_t;
typedef CacheEvict<index_key_t, val_t> cache_evict_t;
typedef CacheEvictCase2<index_key_t, val_t> cache_evict_case2_t;
typedef ConcurrentMap<uint16_t, special_case_t> concurrent_specicalcase_map_t;

bool volatile switchos_running = false;
std::atomic<size_t> switchos_ready_threads(0);
const size_t switchos_expected_ready_threads = 6;
bool volatile switchos_popserver_finish = false;

// Parameters
size_t server_num = 1;
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
short switchos_snapshotdataserver_port = -1;

// Packet types used by switchos.paramserver and ptf framework
int SWITCHOS_GET_FREEIDX = -1; // ptf get freeidx from paramserver
int SWITCHOS_GET_KEY_FREEIDX = -1; // ptf get key and freeidx from paramserver
int SWITCHOS_SET_EVICTDATA = -1; // ptf set evictidx, evictvallen, evictval, evictstat, and evictseq to paramserver
int SWITCHOS_GET_EVICTKEY = -1; // ptf get evictkey
//const int SWITCHOS_GET_EVICTKEY_EVICTIDX = 4; // ptf get evictkey and evictidx
int SWITCHOS_GET_CACHEDEMPTYINDEX = -1; // ptf get cached_empty_index_backup

// Packet types used by switchos/controller/server for snapshot
int SNAPSHOT_START = -1;
int SNAPSHOT_SERVERSIDE = -1;
int SNAPSHOT_SERVERSIDE_ACK = -1;
int SNAPSHOT_DATA = -1;

// Cache population

// controller.popclient <-> switchos.popserver
int switchos_popserver_tcpsock = -1;
int switchos_paramserver_udpsock = -1;
//std::set<index_key_t> switchos_cached_keyset; // TODO: Comment it after checking server.cached_keyset_list
// message queue between switchos.popserver and switchos.popworker
MessagePtrQueue<cache_pop_t> switchos_cache_pop_ptr_queue(MQ_SIZE);
/*cache_pop_t * volatile * switchos_cache_pop_ptrs = NULL;
uint32_t volatile switchos_head_for_pop;
uint32_t volatile switchos_tail_for_pop;*/

// switchos.popworker
index_key_t volatile * switchos_cached_keyarray = NULL; // idx (of inswitch KV) -> key (TODO: different switches for distributed case)
uint16_t volatile * switchos_cached_serveridxarray = NULL; // idx (of inswitch KV) -> serveridx of the key
uint32_t volatile switchos_cached_empty_index = 0; // [empty index, kv_bucket_num-1] is empty
// std::map<index_key_t, uint16_t> volatile switchos_cached_key_idx_map; // key -> idx (of inswitch KV)
int switchos_popworker_popclient_udpsock = -1;

// Used by switchos.paramserer <-> ptf framework
uint16_t volatile switchos_freeidx = 0; // switchos.popworker write -> launch ptf.population -> switchos.paramserver read (no contention)
index_key_t volatile switchos_newkey = index_key_t();

// Cache eviction

bool volatile is_switchos_popworker_evictclient_connected = false;
int switchos_popworker_evictclient_tcpsock = -1;

// Used by switchos.paramserer <-> ptf framework
// launch ptf.eviction -> ptf sets evicted information -> switchos.paramserver read
bool volatile switchos_with_evictdata = false;
uint16_t volatile switchos_evictidx = 0;
//uint32_t volatile switchos_evictvallen = 0;
//uint16_t volatile switchos_evictvallen = 0;
//char * volatile switchos_evictvalbytes = NULL;
val_t volatile switchos_evictvalue = val_t();
uint32_t volatile switchos_evictseq = 0;
bool volatile switchos_evictstat = false;

// Snapshot

// snapshotserver socket to lead snapshot workflow
int switchos_snapshotserver_tcpsock = -1;

// prepare to backup cache metadata with atomicity
bool volatile is_snapshot_prepare = false;
bool volatile popworker_know_snapshot_prepare = false; // ensure current cache population/eviction is finished before metadata backup
// NOTE: not need specialcaseserver_know_snapshot_prepare as switchos_specialcases has been cleared in the previous snapshot period

// backuped cache metadata (set/reset by snapshotserver, read by popoworker/specialcaseserver)
index_key_t * volatile switchos_cached_keyarray_backup = NULL; // idx (of inswitch KV) -> key (TODO: different switches for distributed case)
uint16_t * volatile switchos_cached_serveridxarray_backup = NULL; // idx (of inswitch KV) -> serveridx of the key
uint32_t volatile switchos_cached_empty_index_backup = 0; // [empty index, kv_bucket_num-1] is empty (not used)
// std::map<index_key_t, uint16_t> volatile switchos_cached_key_idx_map_backup; // key -> idx (of inswitch KV)

// collect special cases during snapshot
bool volatile is_snapshot = false;
// specialcaseserver socket
int switchos_specialcaseserver_udpsock = -1;
// special cases updated by specialcaseserver and popworker
std::mutex switchos_mutex_for_specialcases;
//std::map<uint16_t, special_case_t> volatile switchos_specialcases;
concurrent_specicalcase_map_t * volatile switchos_specialcases_ptr;

// snapshotdataserver socket to load snapshot data from ptf
int switchos_snapshotdataserver_tcpsock = -1;

// loaded snapshot data
bool volatile switchos_with_snapshotdata = false;
val_t * volatile switchos_snapshot_values = NULL;
uint32_t * volatile switchos_snapshot_seqs = NULL;
bool * volatile switchos_snapshot_stats = NULL;

// rollback after collecting all special cases
bool volatile is_snapshot_end = false;
bool volatile popworker_know_snapshot_end = false; // ensure current case2 is reported before rollback
bool volatile specialcaseserver_know_snapshot_end = false; // ensure current case1s are reported before rollback

inline void parse_ini(const char *config_file);
inline void parse_control_ini(const char *config_file);
void prepare_switchos();
void *run_switchos_popserver(void *param);
void *run_switchos_paramserver(void *param); // simple message transfer between switchos and ptf by udp
void *run_switchos_snapshotdataserver(void *param); // snapshot data transfer between switchos and ptf and tcp
void *run_switchos_popworker(void *param);
void *run_switchos_snapshotserver(void *param);
void *run_switchos_specialcaseserver(void *param);
void process_specialcase(const uint16_t &tmpidx, const index_key_t &tmpkey, const val_t &tmpval, const uint32_t &tmpseq, const bool &tmpstat);
void close_switchos();

int main(int argc, char **argv) {
	parse_ini("config.ini");
	parse_control_ini("control_type.ini");

	prepare_switchos();

	pthread_t popserver_thread;
	int ret = pthread_create(&popserver_thread, nullptr, run_switchos_popserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t paramserver_thread;
	ret = pthread_create(&paramserver_thread, nullptr, run_switchos_paramserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t snapshotdataserver_thread;
	ret = pthread_create(&snapshotdataserver_thread, nullptr, run_switchos_snapshotdataserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t popworker_thread;
	ret = pthread_create(&popworker_thread, nullptr, run_switchos_popworker, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t snapshotserver_thread;
	ret = pthread_create(&snapshotserver_thread, nullptr, run_switchos_snapshotserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	pthread_t specialcaseserver_thread;
	ret = pthread_create(&specialcaseserver_thread, nullptr, run_switchos_specialcaseserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error: " << ret);
	}

	while (switchos_ready_threads < switchos_expected_ready_threads) sleep(1);
	printf("[switchos] all threads ready\n");

	switchos_running = true;

	// connection from controller
	while (!switchos_popserver_finish) {}

	switchos_running = false;

	void *status;
	int rc = pthread_join(popserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(paramserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(snapshotdataserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(popworker_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(snapshotserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}
	rc = pthread_join(specialcaseserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error:unable to join," << rc);
	}

	printf("[switchos] all threads end\n");
	close_switchos();
}

// TODO: try common_impl.h
inline void parse_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	server_num = ini.get_server_num();
	switchos_popserver_port = ini.get_switchos_popserver_port();
	switchos_paramserver_port = ini.get_switchos_paramserver_port();
	switch_kv_bucket_num = ini.get_switch_kv_bucket_num();
	//switchos_sample_cnt = ini.get_switchos_sample_cnt();
	val_t::SWITCH_MAX_VALLEN = ini.get_switch_max_vallen();
	val_t::MAX_VALLEN = ini.get_max_vallen();
	reflector_ip_for_switchos = ini.get_reflector_ip_for_switchos();
	reflector_popserver_port = ini.get_reflector_popserver_port();
	controller_ip_for_switchos = ini.get_controller_ip_for_switchos();
	controller_evictserver_port = ini.get_controller_evictserver_port();
	switchos_snapshotserver_port = ini.get_switchos_snapshotserver_port();
	switchos_specialcaseserver_port = ini.get_switchos_specialcaseserver_port();
	
	COUT_VAR(server_num);
	COUT_VAR(switchos_popserver_port);
	COUT_VAR(switchos_paramserver_port);
	COUT_VAR(switch_kv_bucket_num);
	//COUT_VAR(switchos_sample_cnt);
	COUT_VAR(val_t::SWITCH_MAX_VALLEN);
	COUT_VAR(val_t::MAX_VALLEN);
	printf("reflector ip for switchos: %s\n", reflector_ip_for_switchos);
	COUT_VAR(reflector_popserver_port);
	printf("controller ip for switchos: %s\n", controller_ip_for_switchos);
	COUT_VAR(controller_evictserver_port);
	COUT_VAR(switchos_snapshotserver_port);
	COUT_VAR(switchos_specialcaseserver_port);
}

inline void parse_control_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	SWITCHOS_GET_FREEIDX = ini.get_switchos_get_freeidx();
	SWITCHOS_GET_KEY_FREEIDX = ini.get_switchos_get_key_freeidx();
	SWITCHOS_SET_EVICTDATA = ini.get_switchos_set_evictdata();
	SWITCHOS_GET_EVICTKEY = ini.get_switchos_get_evictkey();
	SWITCHOS_GET_CACHEDEMPTYINDEX = ini.get_switchos_get_cachedemptyindex();
	SNAPSHOT_START = ini.get_snapshot_start();
	SNAPSHOT_SERVERSIDE = ini.get_snapshot_serverside();
	SNAPSHOT_SERVERSIDE_ACK = ini.get_snapshot_serverside_ack();
	SNAPSHOT_DATA = ini.get_snapshot_data();
	COUT_VAR(SWITCHOS_GET_FREEIDX);
	COUT_VAR(SWITCHOS_GET_KEY_FREEIDX);
	COUT_VAR(SWITCHOS_SET_EVICTDATA);
	COUT_VAR(SWITCHOS_GET_EVICTKEY);
	COUT_VAR(SWITCHOS_GET_CACHEDEMPTYINDEX);
	COUT_VAR(SNAPSHOT_START);
	COUT_VAR(SNAPSHOT_SERVERSIDE);
	COUT_VAR(SNAPSHOT_SERVERSIDE_ACK);
	COUT_VAR(SNAPSHOT_DATA);
}

void prepare_switchos() {
	printf("[switchos] prepare start\n");

	// prepare popserver socket
	prepare_tcpserver(switchos_popserver_tcpsock, false, switchos_popserver_port, 1, "switchos.popserver"); // MAX_PENDING_CONNECTION = 1

	// prepare paramserver socket
	prepare_udpserver(switchos_paramserver_udpsock, true, switchos_paramserver_port, "switchos.paramserver");

	//switchos_cached_keyset.clear();
	//switchos_cache_pop_ptrs = new cache_pop_t*[MQ_SIZE];
	//switchos_head_for_pop = 0;
	//switchos_tail_for_pop = 0;

	create_udpsock(switchos_popworker_popclient_udpsock, "switchos.popworker");
	switchos_cached_keyarray = new index_key_t[switch_kv_bucket_num]();
	switchos_cached_serveridxarray = new uint16_t[switch_kv_bucket_num];
	for (size_t i = 0; i < switch_kv_bucket_num; i++) {
		switchos_cached_serveridxarray[i] = -1;
	}
	switchos_cached_empty_index = 0;
	//switchos_cached_key_idx_map.clear();

	create_tcpsock(switchos_popworker_evictclient_tcpsock, "switchos.popworker.evictclient");
	//switchos_evictvalbytes = new char[val_t::MAX_VALLEN];
	//INVARIANT(switchos_evictvalbytes != NULL);
	//memset(switchos_evictvalbytes, 0, val_t::MAX_VALLEN);

	// prepare snapshotserver socket
	prepare_tcpserver(switchos_snapshotserver_tcpsock, false, switchos_snapshotserver_port, 1, "switchos.snapshotserver"); // MAX_PENDING_CONNECTION = 1

	// prepare specialcaseserver socket
	prepare_udpserver(switchos_specialcaseserver_udpsock, true, switchos_specialcaseserver_port, "switchos.specialcaseserver");

	//switchos_specialcases->clear();

	// prepare snapshotdataserver socket
	prepare_tcpserver(switchos_snapshotdataserver_tcpsock, true, switchos_snapshotdataserver_port, 1, "switchos.snapshotdataserver");

	memory_fence();

	printf("[switchos] prepare end\n");
}

void *run_switchos_popserver(void *param) {
	// Not used
	//struct sockaddr_in controller_addr;
	//unsigned int controller_addr_len = sizeof(struct sockaddr);
	printf("[switchos.popserver] ready\n");
	switchos_ready_threads++;

	while (!switchos_running) {}

	// accept connection from controller.popclient
	int connfd = -1;
	tcpaccept(switchos_popserver_tcpsock, NULL, NULL, connfd, "switchos.popserver");

	// Process CACHE_POP packet <optype, key, vallen, value, seq, serveridx, debug_hdr>
	char buf[MAX_BUFSIZE];
	int cur_recv_bytes = 0;
	uint8_t optype = 0;
	bool with_optype = false;
	//uint32_t vallen = 0;
	uint16_t vallen = 0;
	bool with_vallen = false;
	bool is_cached_before = false; // TODO: remove
	//index_key_t tmpkey(0, 0, 0, 0);
	const int arrive_optype_bytes = sizeof(uint8_t);
	//const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(index_key_t) + sizeof(uint32_t);
	const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(index_key_t) + sizeof(uint16_t);
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
		if (!with_optype && cur_recv_bytes >= arrive_optype_bytes) {
			optype = *((uint8_t *)buf);
			INVARIANT(packet_type_t(optype) == packet_type_t::CACHE_POP);
			with_optype = true;
		}

		// Get vallen
		if (with_optype && !with_vallen && cur_recv_bytes >= arrive_vallen_bytes) {
			////tmpkey.deserialize(buf + arrive_optype_bytes, cur_recv_bytes - arrive_optype_bytes);
			//vallen = *((uint32_t *)(buf + arrive_vallen_bytes - sizeof(uint32_t)));
			//vallen = ntohl(vallen);
			vallen = *((uint16_t *)(buf + arrive_vallen_bytes - sizeof(uint16_t)));
			vallen = ntohs(vallen);
			INVARIANT(vallen >= 0);
			int padding_size = int(val_t::get_padding_size(vallen)); // padding for value <= 128B
			arrive_serveridx_bytes = arrive_vallen_bytes + vallen + padding_size + sizeof(uint32_t) + sizeof(uint16_t);
			with_vallen = true;
		}

		// Get one complete CACHE_POP
		if (with_optype && with_vallen && cur_recv_bytes >= arrive_serveridx_bytes) {
			printf("[switchos.popserver] cur_recv_bytes: %d, arrive_serveridx_bytes: %d\n", cur_recv_bytes, arrive_serveridx_bytes); // TMPDEBUG
			dump_buf(buf, cur_recv_bytes);
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
				cur_recv_bytes = cur_recv_bytes - arrive_serveridx_bytes;
			}
			else {
				cur_recv_bytes = 0;
			}
			optype = 0;
			with_optype = false;
			vallen = 0;
			with_vallen = false;
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
	printf("[switchos.paramserver] ready\n");
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

		int control_type = *((int *)buf);
		if (control_type == SWITCHOS_GET_FREEIDX) {
			memset(buf, 0, MAX_BUFSIZE); // use buf as sendbuf

			// ptf get freeidx -> popworker send switchos_freeidx to ptf framework
			//memcpy(buf, (void *)&switchos_freeidx, sizeof(uint16_t));
			*((uint16_t *)buf) = switchos_freeidx;
			udpsendto(switchos_paramserver_udpsock, buf, sizeof(uint16_t), 0, (struct sockaddr *)&ptf_addr, ptf_addr_len, "switchos.paramserver");
		}
		else if (control_type == SWITCHOS_GET_KEY_FREEIDX) {
			memset(buf, 0, MAX_BUFSIZE); // use buf as sendbuf

			// ptf get key and freeidx -> popworker send key and switchos_freeidx to ptf framework
			uint32_t tmp_keysize = switchos_newkey.serialize(buf, MAX_BUFSIZE);
			//memcpy(buf + tmp_keysize, (void *)&switchos_freeidx, sizeof(uint16_t));
			*((uint16_t *)(buf + tmp_keysize)) = switchos_freeidx;
			udpsendto(switchos_paramserver_udpsock, buf, tmp_keysize + sizeof(uint16_t), 0, (struct sockaddr *)&ptf_addr, ptf_addr_len, "switchos.paramserver");
		}
		else if (control_type == SWITCHOS_SET_EVICTDATA) {
			char *curptr = buf; // continue to use buf as recvbuf
			curptr += sizeof(int); // skip control_type

			// ptf set evict data -> popworker get evictdata from ptf framework
			switchos_evictidx = *((uint16_t *)curptr);
			curptr += sizeof(uint16_t);
			/*//switchos_evictvallen = *((uint32_t *)curptr);
			switchos_evictvallen = *((uint16_t *)curptr);
			curptr += sizeof(uint32_t);
			uint32_t tmp_valbytesnum = switchos_evictvallen + val_t::get_padding_size(switchos_evictvallen);
			memcpy(switchos_evictvalbytes, curptr, tmp_valbytesnum);
			curptr += tmp_valbytesnum;*/
			uint32_t tmp_valsize = switchos_evictvalue.deserialize(curptr, recv_size - sizeof(int) - sizeof(uint16_t));
			curptr += tmp_valsize;
			switchos_evictseq = *((uint32_t *)curptr);
			curptr += sizeof(uint32_t);
			switchos_evictstat = *((bool *)curptr);

			switchos_with_evictdata = true;
		}
		else if (control_type == SWITCHOS_GET_EVICTKEY) {
			memset(buf, 0, MAX_BUFSIZE); // use buf as sendbuf

			// ptf get evictkey to remove evicted data from cache_lookup_tbl -> popworker send evictkey to ptf framework
			// NOTE: we reset cached_keyarray[evictidx] after removing evicted data from cache_lookup_tbl, so here the evicted key is correct
			uint32_t tmp_keysize = switchos_cached_keyarray[switchos_evictidx].serialize(buf, MAX_BUFSIZE);
			udpsendto(switchos_paramserver_udpsock, buf, tmp_keysize, 0, (struct sockaddr *)&ptf_addr, ptf_addr_len, "switchos.paramserver");
			//memcpy(buf + tmp_keysize, (void *)&switchos_evictidx, sizeof(uint16_t));
			//udpsendto(switchos_paramserver_udpsock, buf, tmp_keysize + sizeof(uint16_t), 0, (struct sockaddr *)ptf_addr, ptf_addr_len, "switchos.paramserver");
		}
		else if (control_type == SWITCHOS_GET_CACHEDEMPTYINDEX) {
			memset(buf, 0, MAX_BUFSIZE); // use buf as sendbuf

			// ptf get cached_empty_index_backup -> popworker send cached_empty_index_backup to ptf framework for snapshot
			memcpy(buf, (void *)&switchos_cached_empty_index_backup, sizeof(uint32_t));
			udpsendto(switchos_paramserver_udpsock, buf, sizeof(uint32_t), 0, (struct sockaddr *)&ptf_addr, ptf_addr_len, "switchos.paramserver");
		}
		else {
			printf("[switch os] invalid requset type from ptf to paramserver: %d\n", control_type);
			exit(-1);
		}

	}

	close(switchos_paramserver_udpsock);
	pthread_exit(nullptr);
}

void *run_switchos_snapshotdataserver(void *param) {
	char *buf = new char[MAX_LARGE_BUFSIZE];
	INVARIANT(buf != NULL);
	memset(buf, 0, MAX_LARGE_BUFSIZE);
	printf("[switchos.snapshotdataserver] ready\n");
	switchos_ready_threads++;

	while (!switchos_running) {}

	while (switchos_running) {
		// Not used
		//struct sockaddr_in ptf_addr;
		//unsigned int ptf_addr_len = sizeof(struct sockaddr);

		// accept connection from switchos.ptf
		int connfd = -1;
		bool is_timeout = tcpaccept(switchos_snapshotdataserver_tcpsock, NULL, NULL, connfd, "switchos.snapshotdataserver");
		if (is_timeout) {
			continue; // timeout or interrupted system call
		}
		INVARIANT(connfd != -1);

		// disable timeout for snapshotdataserver.connfd
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		int res = setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		INVARIANT(res >= 0);

		// Receive snapshot data from switchos.ptf
		int cur_recv_bytes = 0;
		int total_bytesnum = -1;
		while (true) {
			int recvsize = 0;
			bool is_broken = tcprecv(connfd, buf + cur_recv_bytes, MAX_LARGE_BUFSIZE - cur_recv_bytes, 0, recvsize, "controller.snapshotdataserver");
			if (is_broken) {
				break;
			}

			cur_recv_bytes += recvsize;
			if (cur_recv_bytes >= MAX_LARGE_BUFSIZE) {
				printf("[controller.snapshotdataserver] Overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_LARGE_BUFSIZE);
				exit(-1);
			}

			// Get total_bytesum
			if (total_bytesnum == -1 && cur_recv_bytes >= int(sizeof(int32_t))) {
				total_bytesnum = *((int32_t *)buf);
				INVARIANT(total_bytesnum <= MAX_LARGE_BUFSIZE);
			}

			// Receive all snapshot data
			if (total_bytesnum != -1 && cur_recv_bytes >= total_bytesnum) {
				// NOTE: freed by snapshotserver
				switchos_snapshot_values = new val_t[switchos_cached_empty_index_backup];
				switchos_snapshot_seqs = new uint32_t[switchos_cached_empty_index_backup];
				switchos_snapshot_stats = new bool[switchos_cached_empty_index_backup];

        		// save snapshot data: <total_bytesnum, records>
				// for each record: <vallen (big-endian), valbytes (same order), seq, result>
				uint32_t tmp_offset = sizeof(int32_t);
				for (uint32_t tmp_recordidx = 0; tmp_recordidx < switchos_cached_empty_index_backup; tmp_recordidx++) {
					uint32_t tmp_valsize = switchos_snapshot_values[tmp_recordidx].deserialize(buf + tmp_offset, cur_recv_bytes - tmp_offset);
					tmp_offset += tmp_valsize;
					switchos_snapshot_seqs[tmp_recordidx] = *((uint32_t *)(buf + tmp_offset));
					tmp_offset += sizeof(uint32_t);
					switchos_snapshot_stats[tmp_recordidx] = *((bool *)(buf + tmp_offset));
					tmp_offset += sizeof(bool);
					INVARIANT(int32_t(tmp_offset) <= total_bytesnum);
				}
				memory_fence();

				// notify switchos.snapshotserver
				switchos_with_snapshotdata = true;

				// Move remaining bytes and reset metadata
				if (cur_recv_bytes > total_bytesnum) {
					memcpy(buf, buf + total_bytesnum, cur_recv_bytes - total_bytesnum);
					cur_recv_bytes = cur_recv_bytes - total_bytesnum;
				}
				else {
					cur_recv_bytes = 0;
				}
				total_bytesnum = -1;
			}
		}
	}

	delete [] buf;
	buf = NULL;
	close(switchos_snapshotdataserver_port);
	pthread_exit(nullptr);
}

void *run_switchos_popworker(void *param) {
	// used by udp socket for cache population
	sockaddr_in reflector_popserver_addr;
	set_sockaddr(reflector_popserver_addr, inet_addr(reflector_ip_for_switchos), reflector_popserver_port);
	int reflector_popserver_addr_len = sizeof(struct sockaddr);

	printf("[switchos.popworker] ready\n");
	switchos_ready_threads++;

	while (!switchos_running) {}

	while (switchos_running) {
		char pktbuf[MAX_BUFSIZE];
		uint32_t pktsize = 0;
		char evictclient_buf[MAX_BUFSIZE];
		int evictclient_cur_recv_bytes = 0;
		const int evictclient_arrive_key_bytes = sizeof(uint8_t) + sizeof(index_key_t);
		if (is_snapshot_prepare && !popworker_know_snapshot_prepare) {
			popworker_know_snapshot_prepare = true;
		}
		else if (!is_snapshot_prepare) {
		//if (switchos_tail_for_pop != switchos_head_for_pop) {
			cache_pop_t *tmp_cache_pop_ptr = switchos_cache_pop_ptr_queue.read();
			if (tmp_cache_pop_ptr != NULL) {
				if (!is_switchos_popworker_evictclient_connected) {
					// used by tcp socket for cache eviction
					tcpconnect(switchos_popworker_evictclient_tcpsock, controller_ip_for_switchos, controller_evictserver_port, "switchos.popworker.evictclient", "controller.evictserver");
					is_switchos_popworker_evictclient_connected = true;
				}

				//cache_pop_t *tmp_cache_pop_ptr = switchos_cache_pop_ptrs[switchos_tail_for_pop];
				//INVARIANT(tmp_cache_pop_ptr != NULL);
				switchos_newkey = tmp_cache_pop_ptr->key();
				memory_fence();

				// assign switchos_freeidx for new record 
				if (switchos_cached_empty_index < switch_kv_bucket_num) { // With free idx
					switchos_freeidx = switchos_cached_empty_index;
					switchos_cached_empty_index += 1;
					// NOTE: as freeidx of new record must > switchos_cached_empty_index_backup, no case2 for cache population
				}
				else { // Without free idx
					bool is_case2 = is_snapshot || (is_snapshot_end && !popworker_know_snapshot_end);

					// get evictdata from ptf framework 
					system("bash tofino/get_evictdata_setvalid3.sh");

					// wait for paramserver to get evictdata
					while (!switchos_with_evictdata) {}

					// switchos.popworker.evictclient sends CACHE_EVICT to controller.evictserver
					INVARIANT(switchos_evictidx >= 0 && switchos_evictidx < switch_kv_bucket_num);
					index_key_t cur_evictkey = switchos_cached_keyarray[switchos_evictidx];
					if (!is_case2) {
						cache_evict_t tmp_cache_evict(cur_evictkey, \
								//val_t(switchos_evictvalbytes, switchos_evictvallen),
								switchos_evictvalue, \
								switchos_evictseq, switchos_evictstat, \
								switchos_cached_serveridxarray[switchos_evictidx]);
						pktsize = tmp_cache_evict.serialize(pktbuf, MAX_BUFSIZE);
					}
					else { // send CACHE_EVICT_CASE2 for server-side snapshot
						cache_evict_case2_t tmp_cache_evict_case2(cur_evictkey, \
								//val_t(switchos_evictvalbytes, switchos_evictvallen),
								switchos_evictvalue, \
								switchos_evictseq, switchos_evictstat, \
								switchos_cached_serveridxarray[switchos_evictidx]);
						pktsize = tmp_cache_evict_case2.serialize(pktbuf, MAX_BUFSIZE);
					}
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
							uint8_t evictclient_optype = *((uint8_t *)evictclient_buf);
							INVARIANT(packet_type_t(evictclient_optype) == packet_type_t::CACHE_EVICT_ACK);

							// move remaining bytes and reset metadata
							if (evictclient_cur_recv_bytes > evictclient_arrive_key_bytes) {
								memcpy(evictclient_buf, evictclient_buf + evictclient_arrive_key_bytes, evictclient_cur_recv_bytes - evictclient_arrive_key_bytes);
								evictclient_cur_recv_bytes = evictclient_cur_recv_bytes - evictclient_arrive_key_bytes;
							}
							else {
								evictclient_cur_recv_bytes = 0;
							}
						}
					}

					// store case2
					if (is_case2) {
						process_specialcase(uint16_t(switchos_evictidx), cur_evictkey, \
								//val_t(switchos_evictvalbytes, switchos_evictvallen), 
								val_t(switchos_evictvalue), \
								uint32_t(switchos_evictseq), bool(switchos_evictstat));
					}

					// remove evicted data from cache_lookup_tbl
					system("bash tofino/remove_cache_lookup.sh");
					//switchos_cached_key_idx_map.erase(cur_evictkey);

					// set freeidx as evictidx for cache popluation later
					switchos_freeidx = switchos_evictidx;

					// reset keyarray and serveridxarray at evictidx
					switchos_cached_keyarray[switchos_evictidx] = index_key_t();
					switchos_cached_serveridxarray[switchos_evictidx] = -1;
				}

				/* cache population for new record */

				INVARIANT(switchos_freeidx >= 0 && switchos_freeidx < switch_kv_bucket_num);
				printf("[switchos.popworker] switchos_cached_empty_index: %d, switchos_freeidx: %d\n", int(switchos_cached_empty_index), int(switchos_freeidx)); // TMPDEBUG

				// set valid=0 for atomicity
				system("bash tofino/setvalid0.sh");
				// send CACHE_POP_INSWITCH to reflector (TODO: try internal pcie port)
				cache_pop_inswitch_t tmp_cache_pop_inswitch(tmp_cache_pop_ptr->key(), tmp_cache_pop_ptr->val(), tmp_cache_pop_ptr->seq(), switchos_freeidx);
				pktsize = tmp_cache_pop_inswitch.serialize(pktbuf, MAX_BUFSIZE);
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
				system("bash tofino/add_cache_lookup_setvalid1.sh");

				switchos_cached_keyarray[switchos_freeidx] = tmp_cache_pop_ptr->key();
				switchos_cached_serveridxarray[switchos_freeidx] = tmp_cache_pop_ptr->serveridx();
				//switchos_cached_key_idx_map.insert(std::pair<index_key_t, uint16_t>(tmp_cache_pop_ptr->key(), switchos_freeidx));

				// free CACHE_POP
				delete tmp_cache_pop_ptr;
				tmp_cache_pop_ptr = NULL;
				//switchos_cache_pop_ptrs[switchos_tail_for_pop] = NULL;
				//switchos_tail_for_pop = (switchos_tail_for_pop + 1) % MQ_SIZE;

				// reset intermediate data for paramserver
				// for cache population
				switchos_freeidx = 0;
				switchos_newkey = index_key_t();
				// for cache eviction
				switchos_with_evictdata = false;
				switchos_evictidx = 0;
				//switchos_evictvallen = 0;
				//memset(switchos_evictvalbytes, 0, val_t::MAX_VALLEN);
				switchos_evictvalue = val_t();
				switchos_evictseq = 0;
				switchos_evictstat = false;
			} // tmp_cache_pop_ptr != NULL
		} // !is_snapshot_prepare
		if (is_snapshot_end && !popworker_know_snapshot_end) {
			popworker_know_snapshot_end = true;
		}
	}

	close(switchos_popworker_popclient_udpsock);
	close(switchos_popworker_evictclient_tcpsock);
	pthread_exit(nullptr);
}

void *run_switchos_snapshotserver(void *param) {
	// Not used
	//struct sockaddr_in controller_addr;
	//unsigned int controller_addr_len = sizeof(struct sockaddr);
	char *tmp_sendbuf_list[server_num];
	for (uint16_t i = 0; i < server_num; i++) {
		tmp_sendbuf_list[i] = new char[MAX_LARGE_BUFSIZE];
		INVARIANT(tmp_sendbuf_list[i] != NULL);
		memset(tmp_sendbuf_list[i], 0, MAX_LARGE_BUFSIZE);
	}
	char *sendbuf = new char[MAX_LARGE_BUFSIZE];
	INVARIANT(sendbuf != NULL);
	memset(sendbuf, 0, MAX_LARGE_BUFSIZE);
	printf("[switchos.snapshotserver] ready");
	switchos_ready_threads++;

	while (!switchos_running) {}

	// accept connection from controller.snapshotclient
	int connfd = -1;
	tcpaccept(switchos_snapshotserver_tcpsock, NULL, NULL, connfd, "switchos.snapshotserver");

	char recvbuf[MAX_BUFSIZE];
	int cur_recv_bytes = 0;
	int phase = 0; // 0: wait for SNAPSHOT_START; 1: wait for SNAPSHOT_SERVERSIDE_ACK;
	int control_type_phase0 = -1;
	int control_type_phase1 = -1;
	// TODO: a large sendbuf to controller
	int tmp_send_bytes[server_num];
	memset((void *)tmp_send_bytes, 0, server_num*sizeof(int));
	int tmp_record_cnts[server_num];
	memset((void *)tmp_record_cnts, 0, server_num*sizeof(int));
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

		// wait for SNAPSHOT_START from controller.snapshotclient
		if (phase == 0) {
			if (control_type_phase0 == -1 && cur_recv_bytes >= int(sizeof(int))) {
				control_type_phase0 = *((int *)recvbuf);
				INVARIANT(control_type_phase0 == SNAPSHOT_START);

				// NOTE: popserver/specialcaseserver will not touch speicalcases_ptr now, as both is_snapshot/is_snapshot_end are false
				INVARIANT(switchos_specialcases_ptr == NULL);
				switchos_specialcases_ptr = new concurrent_specicalcase_map_t();

				// stop cache population/eviction
				is_snapshot_prepare = true;

				// wait until popworker_know_snapshot_prepare = true
				while (!popworker_know_snapshot_prepare) {}
				// by now, cache population/eviction is temporarily stopped -> snapshotserver can backup cache metadata atomically

				// ptf sets snapshot flag as true atomically
				system("bash tofino/set_snapshot_flag.sh");

				// backup cache metadata (freed later)
				memory_fence();
				switchos_cached_keyarray_backup = new index_key_t[switch_kv_bucket_num];
				INVARIANT(switchos_cached_keyarray_backup != NULL);
				memcpy(switchos_cached_keyarray_backup, (void *)switchos_cached_keyarray, switch_kv_bucket_num * sizeof(index_key_t));
				switchos_cached_serveridxarray_backup = new uint16_t[switch_kv_bucket_num];
				INVARIANT(switchos_cached_serveridxarray_backup != NULL);
				memcpy(switchos_cached_serveridxarray_backup, (void *)switchos_cached_serveridxarray, switch_kv_bucket_num * sizeof(uint16_t));
				switchos_cached_empty_index_backup = switchos_cached_empty_index;
				//switchos_cached_key_idx_map_backup = switchos_cached_key_idx_map; // by copy assignment
				memory_fence();

				// resume cache population/eviction
				is_snapshot = true; // notify popworker and specialcaseserver to collect special cases
				is_snapshot_prepare = false; // resume cache population/eviction

				// load vallen, value, deleted, and savedseq in [0, switchos_cached_empty_index_backup-1]
				if (switchos_cached_empty_index_backup > 0) {
					system("bash tofino/load_snapshot_data.sh"); // load snapshot (maybe inconsistent -> need rollback later)
				}

				// wait for switchos.snapshotdataserver
				while (!switchos_with_snapshotdata) {}

				// send SNAPSHOT_SERVERSIDE to controller to notify servers for server-side snapshot
				tcpsend(connfd, (char *)&SNAPSHOT_SERVERSIDE, sizeof(int), "switchos.snapshotserver");

				phase = 1; // wait for SNAPSHOT_SERVERSIDE_ACK
			} // receive a SNAPSHOT_START
		} // phase == 0
		else if (phase == 1) { // wait for SNAPSHOT_SERVERSIDE_ACK
			// NOTE: skip sizeof(int) for SNAPSHOT_START
			if (control_type_phase1 == -1 && cur_recv_bytes >= int(sizeof(int) + sizeof(int))) {
				control_type_phase1 = *((int *)recvbuf);
				INVARIANT(control_type_phase1 == SNAPSHOT_SERVERSIDE_ACK);

				// finish snapshot
				system("bash tofino/reset_snapshot_flag_and_reg");
				is_snapshot = false;
				is_snapshot_end = true;

				// wait for case2 from popworker and case1 from specialcaseserver
				while (!popworker_know_snapshot_end || !specialcaseserver_know_snapshot_end) {}

				// perform rollback (now both popserver/specicalserver will not touch specialcases; snapshotdataserver will not touch snapshot data)
				/*for (std::map<uint16_t, special_case_t>::iterator iter = switchos_specialcases.begin(); iter != switchos_specialcases.end(); iter++) {
					INVARIANT(switchos_cached_keyarray_backup[iter->first] == iter->second._key);
					switchos_snapshot_values[iter->first] = iter->second._val;
					switchos_snapshot_seqs[iter->first] = iter->second._seq;
					switchos_snapshot_stats[iter->first] = iter->second._valid;
				}*/
				INVARIANT(switchos_specialcases_ptr != NULL);
				concurrent_specicalcase_map_t::DataSource source(0, (concurrent_specicalcase_map_t *)switchos_specialcases_ptr);
				source.advance_to_next_valid();
				while (source.has_next) {
					special_case_t tmpcase = source.get_val();
					INVARIANT(switchos_cached_keyarray_backup[source.get_key()] == tmpcase._key);
					switchos_snapshot_values[source.get_key()] = tmpcase._val;
					switchos_snapshot_seqs[source.get_key()] = tmpcase._seq;
					switchos_snapshot_stats[source.get_key()] = tmpcase._valid;
					source.advance_to_next_valid();
				}

				// snapshot data: <int SNAPSHOT_DATA, int32_t total_bytes, per-server data>
				// per-server data: <int32_t perserver_bytes, uint16_t serveridx, int32_t recordcnt, per-record data>
				//// per-record data: <16B key, uint32_t vallen, value (w/ padding), uint32_t seq, bool stat>
				// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>
				for (uint32_t tmpidx = 0; tmpidx < switchos_cached_empty_index_backup; tmpidx++) { // prepare per-server per-record data
					uint16_t tmp_serveridx = switchos_cached_serveridxarray_backup[tmpidx];
					char * tmpptr = tmp_sendbuf_list[tmp_serveridx];
					uint32_t tmp_keysize = switchos_cached_keyarray_backup[tmpidx].serialize(tmpptr + tmp_send_bytes[tmp_serveridx], MAX_LARGE_BUFSIZE - tmp_send_bytes[tmp_serveridx]);
					tmp_send_bytes[tmp_serveridx] += tmp_keysize;
					uint32_t tmp_valsize = switchos_snapshot_values[tmpidx].serialize(tmpptr + tmp_send_bytes[tmp_serveridx], MAX_LARGE_BUFSIZE - tmp_send_bytes[tmp_serveridx]);
					tmp_send_bytes[tmp_serveridx] += tmp_valsize;
					memcpy(tmpptr + tmp_send_bytes[tmp_serveridx], (void *)&switchos_snapshot_seqs[tmpidx], sizeof(uint32_t));
					tmp_send_bytes[tmp_serveridx] += sizeof(uint32_t);
					memcpy(tmpptr + tmp_send_bytes[tmp_serveridx], (void *)&switchos_snapshot_stats[tmpidx], sizeof(bool));
					tmp_send_bytes[tmp_serveridx] += sizeof(bool);
					tmp_record_cnts[tmp_serveridx] += 1;
				}
				int total_bytes = sizeof(int) + sizeof(int32_t); // leave 4B for SNAPSHOT_DATA and total_bytes
				for (uint16_t tmp_serveridx = 0; tmp_serveridx < server_num; tmp_serveridx++) {
					if (tmp_record_cnts[tmp_serveridx] > 0) {
						int32_t tmp_perserver_bytes = tmp_send_bytes[tmp_serveridx] + sizeof(int32_t) + sizeof(uint16_t) + sizeof(int);
						memcpy(sendbuf + total_bytes, (void *)&tmp_perserver_bytes, sizeof(int32_t));
						total_bytes += sizeof(int32_t);
						memcpy(sendbuf + total_bytes, (void *)&tmp_serveridx, sizeof(uint16_t));
						total_bytes += sizeof(uint16_t);
						memcpy(sendbuf + total_bytes, (void *)&tmp_record_cnts[tmp_serveridx], sizeof(int));
						total_bytes += sizeof(int);
						memcpy(sendbuf + total_bytes, tmp_sendbuf_list[tmp_serveridx], tmp_send_bytes[tmp_serveridx]);
						total_bytes += tmp_send_bytes[tmp_serveridx];
					}
				}
				memcpy(sendbuf, (void *)&SNAPSHOT_DATA, sizeof(int)); // set 1st 4B as SNAPSHOT_DATA
				memcpy(sendbuf + sizeof(int), (void *)&total_bytes, sizeof(int32_t)); // set 2nd 4B as total_bytes
				INVARIANT(total_bytes <= MAX_LARGE_BUFSIZE);

				// send rollbacked snapshot data to controller.snapshotclient
				tcpsend(connfd, sendbuf, total_bytes, "switchos.snapshotserver");

				// reset metadata for next snapshot
				is_snapshot_end = false;
				popworker_know_snapshot_end = false;
				specialcaseserver_know_snapshot_end = false;
				is_snapshot_prepare = false;
				popworker_know_snapshot_prepare = false;
				is_snapshot = false;
				delete [] switchos_cached_keyarray_backup;
				switchos_cached_keyarray_backup = NULL;
				delete [] switchos_cached_serveridxarray_backup;
				switchos_cached_serveridxarray_backup = NULL;
				switchos_cached_empty_index_backup = 0;
				// //clear switchos_cached_key_idx_map_backup
				//switchos_specialcases.clear();
				delete switchos_specialcases_ptr;
				switchos_specialcases_ptr = NULL;
				switchos_with_snapshotdata = false;
				delete [] switchos_snapshot_values;
				switchos_snapshot_values = NULL;
				delete [] switchos_snapshot_seqs;
				switchos_snapshot_seqs = NULL;
				delete [] switchos_snapshot_stats;
				switchos_snapshot_stats = NULL;
				
				// Move remaining bytes and reset metadata for snapshotserver.tcpsock
				if (cur_recv_bytes > int(2*sizeof(int))) { // SNAPSHOT_START + SNAPSHOT_SERVERSIDE_ACK
					memcpy(recvbuf, recvbuf + 2*sizeof(int), cur_recv_bytes - 2*sizeof(int));
					cur_recv_bytes = cur_recv_bytes - 2*sizeof(int);
				}
				else {
					cur_recv_bytes = 0;
				}
				phase = 0;
				control_type_phase0 = -1;
				control_type_phase1 = -1;
				memset((void *)tmp_send_bytes, 0, server_num*sizeof(int));
				memset((void *)tmp_record_cnts, 0, server_num*sizeof(int));
			} // receive a SNAPSHOT_SERVERSIDE_ACK

		} // phase == 1
	} // while (switchos_running)

	for (uint16_t i = 0; i < server_num; i++) {
		delete [] tmp_sendbuf_list[i];
		tmp_sendbuf_list[i] = NULL;
	}
	delete [] sendbuf;
	sendbuf = NULL;
	close(switchos_snapshotserver_tcpsock);
	pthread_exit(nullptr);
}

void *run_switchos_specialcaseserver(void *param) {
	// reflector addr (not used) 
	//struct sockaddr_in reflector_addr;
	//unsigned int reflector_addr_len = sizeof(struct sockaddr);

	char buf[MAX_BUFSIZE];
	printf("[switchos.specialcaseserver] ready\n");
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
				case packet_type_t::GETRES_LATEST_SEQ_INSWITCH_CASE1:
					{
						getres_latest_seq_inswitch_case1_t req(buf, recv_size);
						process_specialcase(req.idx(), req.key(), req.val(), req.seq(), req.stat());
						break;
					}
				case packet_type_t::GETRES_DELETED_SEQ_INSWITCH_CASE1:
					{
						getres_deleted_seq_inswitch_case1_t req(buf, recv_size);
						process_specialcase(req.idx(), req.key(), req.val(), req.seq(), req.stat());
						break;
					}
				case packet_type_t::PUTREQ_SEQ_INSWITCH_CASE1:
					{
						putreq_seq_inswitch_case1_t req(buf, recv_size);
						process_specialcase(req.idx(), req.key(), req.val(), req.seq(), req.stat());
						break;
					}
				case packet_type_t::DELREQ_SEQ_INSWITCH_CASE1:
					{
						delreq_seq_inswitch_case1_t req(buf, recv_size);
						process_specialcase(req.idx(), req.key(), req.val(), req.seq(), req.stat());
						break;
					}
				default:
					{
						printf("[switchos.specialcaseserver] Invalid packet type from reflector.dpdkserver.specialcaseclient!\n");
						exit(-1);
					}
			} // end of switch
		} // is_snapshot || (is_snapshot_end && !specialcaseserver_know_snapshot_end)
	}

	close(switchos_specialcaseserver_udpsock);
	pthread_exit(nullptr);
}

void process_specialcase(const uint16_t &tmpidx, const index_key_t &tmpkey, const val_t &tmpval, const uint32_t &tmpseq, const bool &tmpstat) {
	// verify key (NOTE: use switchos_cached_key_idx_map_backup) 
	/*std::map<index_key_t, uint16_t>::iterator key_idx_map_iter = switchos_cached_key_idx_map_backup.find(req.key()); 
	if (key_idx_map_iter == switchos_cached_key_idx_map_backup.end()) { // unmatched key 
		break; 
	} 
	uint16_t tmpidx = key_idx_map_iter->second;*/

	INVARIANT(tmpidx >= 0 && tmpidx < switch_kv_bucket_num);
	if (switchos_cached_keyarray_backup[tmpidx] == tmpkey) { 
		//switchos_mutex_for_specialcases.lock(); 
		//std::map<uint16_t, special_case_t>::iterator specialcase_iter = switchos_specialcases.find(tmpidx); 
		special_case_t tmpcase;
		bool res = switchos_specialcases_ptr->get(tmpidx, tmpcase);
		//if (specialcase_iter == switchos_specialcases.end()) { // no special case for the idx/key
		if (!res) { // no special case for the idx/key
			//special_case_t tmpcase; 
			tmpcase._key = tmpkey;
			tmpcase._val = tmpval;
			tmpcase._seq = tmpseq;
			tmpcase._valid = tmpstat; // stat=1 means not deleted
			//switchos_specialcases.insert(std::pair<uint16_t, special_case_t>(tmpidx, tmpcase)); 
			switchos_specialcases_ptr->insert(tmpidx, tmpcase);
		} 
		//else if (req.seq() < specialcase_iter->second.seq()) { // current special case is more close to snapshot timepoint
		else if (tmpseq < tmpcase._seq) { // current special case is more close to snapshot timepoint
			//INVARIANT(specialcase_iter->second._key == tmpkey); 
			//specialcase_iter->second._val = tmpval;
			//specialcase_iter->second._seq = tmpseq; 
			//specialcase_iter->second._valid = tmpstat; 
			INVARIANT(tmpcase._key == tmpkey);
			tmpcase._val = tmpval;
			tmpcase._seq = tmpseq;
			tmpcase._valid = tmpstat; // stat=1 means not deleted
			res = switchos_specialcases_ptr->update(tmpidx, tmpcase);
			INVARIANT(res == true);
		} 
		//switchos_mutex_for_specialcases.unlock(); 
	} 
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
	/*if (switchos_evictvalbytes != NULL) {
		delete [] switchos_evictvalbytes;
		switchos_evictvalbytes = NULL;
	}*/
	if (switchos_cached_keyarray_backup != NULL) {
		delete [] switchos_cached_keyarray_backup;
		switchos_cached_keyarray_backup = NULL;
	}
	if (switchos_cached_serveridxarray_backup != NULL) {
		delete [] switchos_cached_serveridxarray_backup;
		switchos_cached_serveridxarray_backup = NULL;
	}
	if (switchos_snapshot_values != NULL) {
		delete [] switchos_snapshot_values;
		switchos_snapshot_values = NULL;
	}
	if (switchos_snapshot_seqs != NULL) {
		delete [] switchos_snapshot_seqs;
		switchos_snapshot_seqs = NULL;
	}
	if (switchos_snapshot_stats != NULL) {
		delete [] switchos_snapshot_stats;
		switchos_snapshot_stats = NULL;
	}
	if (switchos_specialcases_ptr != NULL) {
		delete switchos_specialcases_ptr;
		switchos_specialcases_ptr = NULL;
	}
}
