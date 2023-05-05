#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
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

#include "../common/helper.h"
#include "../common/io_helper.h"
#include "../common/key.h"
#include "../common/val.h"

#include "../common/socket_helper.h"
#include "../common/special_case.h"
#include "../common/iniparser/iniparser_wrapper.h"
#include "message_queue_impl.h"
#include "../common/packet_format_impl.h"
#include "concurrent_map_impl.h"
#include "../common/dynamic_array.h"

#include "common_impl.h"

// ptf scripts used by switchos
// cache eviction: get_evictdata_setvalid3.sh, remove_cache_lookup.sh
// cache population: setvalid0.sh, add_cache_lookup_setvalid1.sh
// snapshot: set_snapshot_flag.sh, load_snapshot_data.sh, reset_snapshot_flag_and_reg.sh

typedef Key netreach_key_t;
typedef Val val_t;
typedef SpecialCase special_case_t;
typedef GetResponseLatestSeqInswitchCase1<netreach_key_t, val_t> getres_latest_seq_inswitch_case1_t;
typedef GetResponseDeletedSeqInswitchCase1<netreach_key_t, val_t> getres_deleted_seq_inswitch_case1_t;
typedef PutRequestSeqInswitchCase1<netreach_key_t, val_t> putreq_seq_inswitch_case1_t;
typedef DelRequestSeqInswitchCase1<netreach_key_t, val_t> delreq_seq_inswitch_case1_t;
typedef CachePop<netreach_key_t, val_t> cache_pop_t;
typedef CachePopInswitch<netreach_key_t, val_t> cache_pop_inswitch_t;
typedef CachePopInswitchAck<netreach_key_t> cache_pop_inswitch_ack_t;
typedef CacheEvict<netreach_key_t, val_t> cache_evict_t;
typedef CacheEvictAck<netreach_key_t> cache_evict_ack_t;
typedef CacheEvictCase2<netreach_key_t, val_t> cache_evict_case2_t;
typedef ConcurrentMap<uint16_t, special_case_t> concurrent_specialcase_map_t;
typedef CachePopAck<netreach_key_t> cache_pop_ack_t;
typedef CacheEvictLoadfreqInswitch<netreach_key_t> cache_evict_loadfreq_inswitch_t;
typedef CacheEvictLoadfreqInswitchAck<netreach_key_t> cache_evict_loadfreq_inswitch_ack_t;
typedef CacheEvictLoaddataInswitch<netreach_key_t> cache_evict_loaddata_inswitch_t;
typedef CacheEvictLoaddataInswitchAck<netreach_key_t, val_t> cache_evict_loaddata_inswitch_ack_t;

bool recover_mode = false;

bool volatile switchos_running = false;
std::atomic<size_t> switchos_ready_threads(0);
const size_t switchos_expected_ready_threads = 4;
bool volatile switchos_popserver_finish = false;

// Cache population

// controller.popclient <-> switchos.popserver
int switchos_popserver_udpsock = -1;
// message queue between switchos.popserver and switchos.popworker
MessagePtrQueue<cache_pop_t> switchos_cache_pop_ptr_queue(MQ_SIZE);
/*cache_pop_t * volatile * switchos_cache_pop_ptrs = NULL;
uint32_t volatile switchos_head_for_pop;
uint32_t volatile switchos_tail_for_pop;*/

// switchos.popworker

// switchos.popworker <-> ptf.popserver
int switchos_popworker_popclient_for_ptf_udpsock = -1; 
// switchos.popworker <-> reflector.cp2dpserver
int switchos_popworker_popclient_for_reflector_udpsock = -1;
// switchos.popworker <-> controller.evictserver for cache eviction
int switchos_popworker_evictclient_for_controller_udpsock = -1;
// cached metadata
std::map<netreach_key_t, uint32_t> switchos_cached_keyidx_map; // TODO: Comment it after checking server.cached_keyset_list
netreach_key_t volatile ** switchos_perpipeline_cached_keyarray = NULL; // idx (of inswitch KV) -> key (TODO: different switches for distributed case)
uint16_t volatile ** switchos_perpipeline_cached_serveridxarray = NULL; // idx (of inswitch KV) -> serveridx of the key
uint32_t volatile * switchos_perpipeline_cached_empty_index = 0; // [empty index, kv_bucket_num-1] is empty

// Snapshot

// snapshotserver socket to receive control command from controller.snapshotclient
int switchos_snapshotserver_udpsock = -1;
// switchos.snapshotserver <-> ptf.snapshotserver
int switchos_snapshotserver_snapshotclient_for_ptf_udpsock = -1; 
// switchos.snapshotserver <-> reflector.cp2dpserver
int switchos_snapshotserver_snapshotclient_for_reflector_udpsock = -1;

// prepare to backup cache metadata for snapshot case2 with atomicity
//bool volatile is_stop_cachepop = false;
std::atomic<bool> is_stop_cachepop(false);
//bool volatile popworker_know_stop_cachepop = false; // ensure current cache population/eviction is finished before metadata backup
std::atomic<bool> popworker_know_stop_cachepop(false); // ensure current cache population/eviction is finished before metadata backup
// NOTE: not need specialcaseserver_know_stop_cachepop as switchos_specialcases has been cleared in the previous snapshot period

// backuped cache metadata (set/reset by snapshotserver, read by popoworker/specialcaseserver)
netreach_key_t ** volatile switchos_perpipeline_cached_keyarray_backup = NULL; // idx (of inswitch KV) -> key (TODO: different switches for distributed case)
uint16_t ** volatile switchos_perpipeline_cached_serveridxarray_backup = NULL; // idx (of inswitch KV) -> serveridx of the key
uint32_t * volatile switchos_perpipeline_cached_empty_index_backup = 0; // [empty index, kv_bucket_num-1] is empty (not used)

// collect special cases during snapshot
//bool volatile is_snapshot = false;
std::atomic<bool> is_snapshot(false);
// specialcaseserver socket
int switchos_specialcaseserver_udpsock = -1;
// special cases updated by specialcaseserver and popworker
std::mutex switchos_mutex_for_specialcases;
//std::map<uint16_t, special_case_t> volatile switchos_specialcases;
concurrent_specialcase_map_t ** volatile switchos_perpipeline_specialcases_ptr = NULL;

// rollback after collecting all special cases
//bool volatile is_snapshot_end = false;
std::atomic<bool> is_snapshot_end(false);
//bool volatile popworker_know_snapshot_end = false; // ensure current case2 is reported before rollback
std::atomic<bool> popworker_know_snapshot_end(false); // ensure current case2 is reported before rollback
//bool volatile specialcaseserver_know_snapshot_end = false; // ensure current case1s are reported before rollback
std::atomic<bool> specialcaseserver_know_snapshot_end(false); // ensure current case1s are reported before rollback

// local control plane bandwidth usage
std::mutex mutex_for_specialcasebwcost;
uint64_t *switchos_perserver_specialcasebwcosts = NULL; // in unit of byte (cleared per snapshot period)

void prepare_switchos();
void recover();
void *run_switchos_popserver(void *param);
void *run_switchos_popworker(void *param);
void *run_switchos_snapshotserver(void *param);
void *run_switchos_specialcaseserver(void *param);
void process_specialcase(const uint16_t &tmpidx, const netreach_key_t &tmpkey, const val_t &tmpval, const uint32_t &tmpseq, const bool &tmpstat);
void close_switchos();

// switchos <-> ptf.popserver
//inline uint32_t serialize_setvalid0(char *buf, uint16_t freeidx, uint32_t pipeidx);
//inline uint32_t serialize_add_cache_lookup_setvalid1(char *buf, netreach_key_t key, uint16_t freeidx, uint32_t pipeidx);
inline uint32_t serialize_add_cache_lookup(char *buf, netreach_key_t key, uint16_t freeidx);
inline uint32_t serialize_writeallseq(char *buf, uint32_t maxseq);
//inline uint32_t serialize_setvalid3(char *buf, uint16_t evictidx, uint32_t pipeidx);
// NOTE: now we load evicted data directly from data plane instead of via ptf channel
//inline uint32_t serialize_get_evictdata_setvalid3(char *buf, uint32_t pipeidx);
//inline void parse_evictdata(char *buf, int recvsize, uint16_t &switchos_evictidx, val_t &switchos_evictvalue, uint32_t &switchos_evictseq, bool &switchos_evictstat);
inline uint32_t serialize_remove_cache_lookup(char *buf, netreach_key_t key);
// NOTE: now we load snapshot data directly from data plane instead of via ptf channel
//inline uint32_t serialize_load_snapshot_data(char *buf, uint32_t emptyidx, uint32_t pipeidx);
//void parse_snapshotdata_fromptf(char *buf, uint32_t buflen, val_t *values, uint32_t *seqs, bool *stats, uint32_t record_cnt);

int main(int argc, char **argv) {
	if ((argc == 2) && (strcmp(argv[1], "recover") == 0)) {
		recover_mode = true;
	}

	parse_ini("config.ini");
	parse_control_ini("control_type.ini");

	prepare_switchos();

	pthread_t popserver_thread;
	int ret = pthread_create(&popserver_thread, nullptr, run_switchos_popserver, nullptr);
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

	free_common();
	close_switchos();
	printf("[switchos] all threads end\n");
}

void prepare_switchos() {
	printf("[switchos] prepare start\n");

	srand(0); // set random seed as 0 for cache eviction

	// prepare popserver socket
	prepare_udpserver(switchos_popserver_udpsock, false, switchos_popserver_port, "switchos.popserver");

	//switchos_cache_pop_ptrs = new cache_pop_t*[MQ_SIZE];
	//switchos_head_for_pop = 0;
	//switchos_tail_for_pop = 0;

	// prepare for popworker
	
	// popworker <-> ptf.popserver
	create_udpsock(switchos_popworker_popclient_for_ptf_udpsock, false, "switchos.popworker.popclient_for_ptf");

	// popworker <-> controller.evictserver
	create_udpsock(switchos_popworker_evictclient_for_controller_udpsock, true, "switchos.popworker.evictclient");
	//switchos_evictvalbytes = new char[val_t::MAX_VALLEN];
	//INVARIANT(switchos_evictvalbytes != NULL);
	//memset(switchos_evictvalbytes, 0, val_t::MAX_VALLEN);
	
	// popworker <-> reflector.cp2dpserver
	create_udpsock(switchos_popworker_popclient_for_reflector_udpsock, true, "switchos.popworker.popclient_for_reflector", 0, SWITCHOS_POPCLIENT_FOR_REFLECTOR_TIMEOUT_USECS); // to reduce snapshot latency

	// cached metadata
	switchos_cached_keyidx_map.clear();
	switchos_perpipeline_cached_keyarray = (netreach_key_t volatile **)(new netreach_key_t*[switch_pipeline_num]);
	switchos_perpipeline_cached_serveridxarray = (uint16_t volatile **)new uint16_t*[switch_pipeline_num];
	switchos_perpipeline_cached_empty_index = new uint32_t[switch_pipeline_num];
	switchos_perpipeline_cached_keyarray_backup = new netreach_key_t*[switch_pipeline_num];
	switchos_perpipeline_cached_serveridxarray_backup = new uint16_t*[switch_pipeline_num];
	switchos_perpipeline_cached_empty_index_backup = new uint32_t[switch_pipeline_num];
	for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
		switchos_perpipeline_cached_keyarray[tmp_pipeidx] = new netreach_key_t[switch_kv_bucket_num]();
		switchos_perpipeline_cached_serveridxarray[tmp_pipeidx] = new uint16_t[switch_kv_bucket_num];
		memset((void *)switchos_perpipeline_cached_serveridxarray[tmp_pipeidx], 0, sizeof(uint16_t) * switch_kv_bucket_num);
		switchos_perpipeline_cached_empty_index[tmp_pipeidx] = 0;

		switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx] = NULL;
		switchos_perpipeline_cached_serveridxarray_backup[tmp_pipeidx] = NULL;
		switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx] = 0;
	}
	
	// prepare for snapshotserver

	// prepare snapshotserver socket
	prepare_udpserver(switchos_snapshotserver_udpsock, false, switchos_snapshotserver_port, "switchos.snapshotserver");

	// snapshotserver <-> ptf.snapshotserver
	create_udpsock(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, false, "switchos.snapshotserver.snapshotclient_for_ptf", SOCKET_TIMEOUT, 0, UDP_LARGE_RCVBUFSIZE);
	
	// snapshotserver <-> reflector.cp2dpserver
	create_udpsock(switchos_snapshotserver_snapshotclient_for_reflector_udpsock, true, "switchos.snapshotserver.snapshotclient_for_reflector", 0, SWITCHOS_SNAPSHOTCLIENT_FOR_REFLECTOR_TIMEOUT_USECS, 2 * UDP_LARGE_RCVBUFSIZE); // 0.5s for low snapshot latency

	// prepare specialcaseserver socket
	prepare_udpserver(switchos_specialcaseserver_udpsock, true, switchos_specialcaseserver_port, "switchos.specialcaseserver", 0, SWITCHOS_SPECIALCASESERVER_TIMEOUT_USECS, 2*UDP_LARGE_RCVBUFSIZE); // timeout interval: 1000us to avoid long wait time when making snapshot
	//switchos_specialcases->clear();
	
	switchos_perpipeline_specialcases_ptr = new concurrent_specialcase_map_t *[switch_pipeline_num];
	for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
		switchos_perpipeline_specialcases_ptr[tmp_pipeidx] = NULL;
	}

	// prepare for per-server specialcase bwcost
	switchos_perserver_specialcasebwcosts = new uint64_t[max_server_total_logical_num];
	memset(switchos_perserver_specialcasebwcosts, 0, sizeof(uint64_t) * max_server_total_logical_num);

	if (recover_mode) {
		recover();
	}

	memory_fence();

	printf("[switchos] prepare end\n");
}

void recover() {
	struct timespec recover_t1, recover_t2, recover_t3;
	CUR_TIME(recover_t1);

	// (1) copy in-switch snapshot data, client-side maxseq, and server-side maxseq from controller/client/server to switch (put into test_recovery_time.sh)
	//system("bash localscripts/fetchall_all2switch.sh");

	uint32_t maxseq = 0;

	// (2) extract in-switch snapshot data

	char snapshotid_path[256];
	get_controller_snapshotid_path(CURMETHOD_ID, snapshotid_path, 256);
	if (!isexist(snapshotid_path)) {
		//printf("You need to copy latest snapshotid from controller to switchos before running with recover mode\n");
		printf("No such file: %s", snapshotid_path);
		fflush(stdout);
		exit(-1);
	}

	int controller_snapshotid = 0;
	load_snapshotid(controller_snapshotid, snapshotid_path);
	char snapshotdata_path[256];
	get_controller_snapshotdata_path(CURMETHOD_ID, snapshotdata_path, 256, controller_snapshotid);
	if (!isexist(snapshotdata_path)) {
		//printf("You need to copy inswitch snapshot data from controller to switchos before running with recover mode\n");
		printf("No such file: %s", snapshotdata_path);
		fflush(stdout);
		exit(-1);
	}

	uint32_t filesize = get_filesize(snapshotdata_path);
	INVARIANT(filesize > 0);
	char *content = readonly_mmap(snapshotdata_path, 0, filesize);
	INVARIANT(content != NULL);

	// NOTE: switchos is launched by root, so we can directly run recover_switch.sh to set inswitch SRAM with inswitch snapshot data
	// DEPRECATED: we can directly perform cache admissions by switchos
	//system("cd tofino; bash recover_switch.sh >../tmp_recoverswitch.sh 2>&1; cd ..");
	
	int total_bytes = 0;
	std::vector<int> perserver_bytes;
	std::vector<uint16_t> perserver_serveridx;
	std::vector<int> perserver_recordcnt;
	std::vector<uint64_t> perserver_specialcase_bwcost;
	std::vector<std::vector<netreach_key_t>> perserver_keyarray;
	std::vector<std::vector<val_t>> perserver_valarray;
	std::vector<std::vector<uint32_t>> perserver_seqarray;
	std::vector<std::vector<bool>> perserver_statarray;

	deserialize_snapshot_getdata_ack(content, filesize, SNAPSHOT_GETDATA_ACK, total_bytes, perserver_bytes, perserver_serveridx, perserver_recordcnt, perserver_specialcase_bwcost, perserver_keyarray, perserver_valarray, perserver_seqarray, perserver_statarray);

	munmap(content, filesize);

	// (3) Prepare for cache admissions
	
	// used by udp socket for cache population
	sockaddr_in reflector_cp2dpserver_addr;
	set_sockaddr(reflector_cp2dpserver_addr, inet_addr(reflector_ip_for_switchos), reflector_cp2dpserver_port);
	int reflector_cp2dpserver_addr_len = sizeof(struct sockaddr);
	int tmpudpsock_for_reflector = -1;
	create_udpsock(tmpudpsock_for_reflector, true, "switchos.recover.udpsock_for_reflector", 0, SWITCHOS_POPCLIENT_FOR_REFLECTOR_TIMEOUT_USECS); // to reduce snapshot latency

	// used by udpsocket to communicate with ptf.popserver
	sockaddr_in ptf_popserver_addr;
	set_sockaddr(ptf_popserver_addr, inet_addr("127.0.0.1"), switchos_ptf_popserver_port);
	int ptf_popserver_addr_len = sizeof(struct sockaddr);
	int tmpudpsock_for_ptf = -1;
	create_udpsock(tmpudpsock_for_ptf, false, "switchos.recover.udpsock_for_ptf");

	// communicate with reflector.cp2dpserer
	char pktbuf[MAX_BUFSIZE];
	uint32_t pktsize = 0;
	char ackbuf[MAX_BUFSIZE];
	int ack_recvsize = 0;
	// communicate with ptf.popserver
	char ptfbuf[MAX_BUFSIZE];
	uint32_t ptf_sendsize = 0;
	int ptf_recvsize = 0;
	
	// (4) Perform cache admissions
	for (int i = 0; i < perserver_serveridx.size(); i++) {
		uint16_t tmp_serveridx = perserver_serveridx[i];

		// find corresponding pipeline idx
		int tmp_server_physical_idx = -1;
		for (int i = 0; i < server_physical_num; i++) {
			for (int j = 0; j < server_logical_idxes_list[i].size(); j++) {
				if (server_logical_idxes_list[i][j] == tmp_serveridx) {
					tmp_server_physical_idx = i;
					break;
				}
			}
		}
		INVARIANT(tmp_server_physical_idx != -1);
		uint32_t tmp_pipeidx = server_pipeidxes[tmp_server_physical_idx];

		for (int j = 0; j < perserver_keyarray[i].size(); j++) {
			uint16_t tmp_freeidx = switchos_perpipeline_cached_empty_index[tmp_pipeidx];

			netreach_key_t tmp_key = perserver_keyarray[i][j];
			val_t tmp_val = perserver_valarray[i][j];
			uint32_t tmp_seq = perserver_seqarray[i][j];
			bool tmp_stat = perserver_statarray[i][j];

			// Update maxseq for in-switch snapshot
			if (tmp_seq > maxseq) {
				maxseq = tmp_seq;
			}

			if (tmp_freeidx >= switch_kv_bucket_num) {
				printf("[WARN] total number of snapshot records > %d; perserver_keyarray[%d].size() is %d\n", switch_kv_bucket_num, i, perserver_keyarray[i].size());
				fflush(stdout);
				break;
			}

			// Set valid = 0 (not necessary under recovery mode due to no background traffic)
			//setvalid_inswitch_t tmp_setvalid_req(CURMETHOD_ID, tmp_key, tmp_freeidx, 0);
			//pktsize = tmp_setvalid_req.serialize(pktbuf, MAX_BUFSIZE);
			//udpsendto(tmpudpsock_for_reflector, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.recover.udpsock_for_reflector");
			////is_timeout = udprecvfrom(tmpudpsock_for_reflector, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.recover.udpsock_for_reflector");
			////setvalid_inswitch_ack_t tmp_setvalid_rsp(CURMETHOD_ID, ackbuf, ack_recvsize);
			////INVARIANT(tmp_setvalid_rsp.key() == tmp_key);

			// send CACHE_POP_INSWITCH to reflector (DEPRECATED: try internal pcie port)
			cache_pop_inswitch_t tmp_cache_pop_inswitch(CURMETHOD_ID, tmp_key, tmp_val, tmp_seq, tmp_freeidx, tmp_stat);
			pktsize = tmp_cache_pop_inswitch.serialize(pktbuf, MAX_BUFSIZE);
			udpsendto(tmpudpsock_for_reflector, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.recover.udpsock_for_reflector");
			////is_timeout = udprecvfrom(tmpudpsock_for_reflector, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.recover.udpsock_for_reflector");
			////cache_pop_inswitch_ack_t tmp_cache_pop_inswitch_ack(CURMETHOD_ID, ackbuf, ack_recvsize);
			////INVARIANT(tmp_cache_pop_inswitch_ack.key() == tmp_key);

			// add new <key, value> pair into cache_lookup_tbl
			ptf_sendsize = serialize_add_cache_lookup(ptfbuf, tmp_key, tmp_freeidx);
			udpsendto(tmpudpsock_for_ptf, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.recover.udpsock_for_ptf");
			////udprecvfrom(tmpudpsock_for_ptf, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.recover.udpsock_for_ptf");
			////INVARIANT(*((int *)ptfbuf) == SWITCHOS_ADD_CACHE_LOOKUP_ACK); // wait for SWITCHOS_ADD_CACHE_LOOKUP_ACK
				
			// set valid=1 to enable the entry through reflector
			// NOTE: newkey will be partitioned into the same pipeline as evictkey if any
			setvalid_inswitch_t tmp_setvalid_req(CURMETHOD_ID, tmp_key, tmp_freeidx, 1);
			pktsize = tmp_setvalid_req.serialize(pktbuf, MAX_BUFSIZE);
			udpsendto(tmpudpsock_for_reflector, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.recover.udpsock_for_reflector");
			////is_timeout = udprecvfrom(tmpudpsock_for_reflector, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.recover.udpsock_for_reflector");
			////setvalid_inswitch_ack_t tmp_setvalid_rsp(CURMETHOD_ID, ackbuf, ack_recvsize);
			////INVARIANT(tmp_setvalid_rsp.key() == tmp_key);

			// Update switchos inswitch cache metadata
			switchos_cached_keyidx_map.insert(std::pair<netreach_key_t, uint32_t>(tmp_key, tmp_freeidx));
			switchos_perpipeline_cached_keyarray[tmp_pipeidx][tmp_freeidx] = tmp_key;
			switchos_perpipeline_cached_serveridxarray[tmp_pipeidx][tmp_freeidx] = tmp_serveridx;
			// NOTE: we follow the order of in-switch snapshot data to restore in-switch cache in data plane, so we can directly use and increase empty index as kvidx in control plane
			switchos_perpipeline_cached_empty_index[tmp_pipeidx] += 1;
		}
	}

	// (5) get final maxseq
	
	// update maxseq for client-side backup files (not necessary, as we recover servers before in-switch cache)
	/*std::vector<std::vector<netreach_key_t>> perclient_keyarray;
	std::vector<std::vector<val_t>> perclient_valarray;
	std::vector<std::vector<uint32_t>> perclient_seqarray;
	std::vector<std::vector<bool>> perclient_statarray;
	char dirname[256];
	memset(dirname, '\0', 256);
	sprintf(dirname, "../benchmark/output/upstreambackups/");
	deserialize_perclient_upstream_backup_files(dirname, perclient_keyarray, perclient_valarray, perclient_seqarray, perclient_statarray);
	for (int i = 0; i < perclient_seqarray.size(); i++) {
		for (int j = 0; j < perclient_seqarray[i].size(); j++) {
			if (perclient_seqarray[i][j] > maxseq) {
				maxseq = perclient_seqarray[i][j];
			}
		}
	}*/

	// update maxseq for server-side maxseq files
	std::vector<uint32_t> perserver_maxseq;
	deserialize_perserver_maxseq_files("/tmp/farreach/", perserver_maxseq);
	for (int i = 0; i < perserver_maxseq.size(); i++) {
		if (perserver_maxseq[i] > maxseq) {
			maxseq = perserver_maxseq[i];
		}
	}

	// (6) write all seq_reg as maxseq (no ack)
	printf("Write maxseq %d to all seq_reg\n", maxseq);
	fflush(stdout);
	ptf_sendsize = serialize_writeallseq(ptfbuf, maxseq);
	udpsendto(tmpudpsock_for_ptf, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.recover.udpsock_for_ptf");

	close(tmpudpsock_for_reflector);
	close(tmpudpsock_for_ptf);

	CUR_TIME(recover_t2);
	DELTA_TIME(recover_t2, recover_t1, recover_t3);
	printf("[Statistics] Replay time of switch&switchos: %f s w/ cache size %d\n", GET_MICROSECOND(recover_t3) / 1000.0 / 1000.0, switch_kv_bucket_num);
	fflush(stdout);
}

void *run_switchos_popserver(void *param) {
	// NOTE: controller.popclient address continues to change
	struct sockaddr_in controller_popclient_addr;
	socklen_t controller_popclient_addrlen = sizeof(struct sockaddr_in);
	
	printf("[switchos.popserver] ready\n");
	switchos_ready_threads++;

	while (!switchos_running) {}

	// Process CACHE_POP packet <optype, key, vallen, value, seq, serveridx>
	char buf[MAX_BUFSIZE];
	int recvsize = 0;
	while (switchos_running) {
		udprecvfrom(switchos_popserver_udpsock, buf, MAX_BUFSIZE, 0, &controller_popclient_addr, &controller_popclient_addrlen, recvsize, "switchos.popserver");

		//printf("receive CACHE_POP from controller\n");
		//dump_buf(buf, recvsize);
		cache_pop_t *tmp_cache_pop_ptr = new cache_pop_t(CURMETHOD_ID, buf, recvsize); // freed by switchos.popworker

		// send CACHE_POP_ACK to controller.popclient immediately to avoid timeout
		cache_pop_ack_t tmp_cache_pop_ack(CURMETHOD_ID, tmp_cache_pop_ptr->key());
		uint32_t acksize = tmp_cache_pop_ack.serialize(buf, MAX_BUFSIZE);
		udpsendto(switchos_popserver_udpsock, buf, acksize, 0, &controller_popclient_addr, controller_popclient_addrlen, "switchos.popserver");

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

	switchos_popserver_finish = true;
	close(switchos_popserver_udpsock);
	pthread_exit(nullptr);
}

void *run_switchos_popworker(void *param) {
	// used by udp socket for cache population
	sockaddr_in reflector_cp2dpserver_addr;
	set_sockaddr(reflector_cp2dpserver_addr, inet_addr(reflector_ip_for_switchos), reflector_cp2dpserver_port);
	int reflector_cp2dpserver_addr_len = sizeof(struct sockaddr);

	// used by udpsocket to communicate with ptf.popserver
	sockaddr_in ptf_popserver_addr;
	set_sockaddr(ptf_popserver_addr, inet_addr("127.0.0.1"), switchos_ptf_popserver_port);
	int ptf_popserver_addr_len = sizeof(struct sockaddr);

	// used by popworker.evictclient
	sockaddr_in controller_evictserver_addr;
	set_sockaddr(controller_evictserver_addr, inet_addr(controller_ip_for_switchos), controller_evictserver_port);
	socklen_t controller_evictserver_addrlen = sizeof(struct sockaddr_in);

	// get valid server logical idxes (TMPDEBUG)
	std::vector<uint16_t> valid_global_server_logical_idxes;
	for (int i = 0; i < server_physical_num; i++) {
		for (int j = 0; j < server_logical_idxes_list[i].size(); j++) {
			uint16_t tmp_global_server_logical_idx = server_logical_idxes_list[i][j];
			valid_global_server_logical_idxes.push_back(tmp_global_server_logical_idx);
		}
	}

	printf("[switchos.popworker] ready\n");
	switchos_ready_threads++;

	while (!switchos_running) {}

	// (1) communicate with controller.evictserver or reflector.cp2dpserer
	// send CACHE_POP_INSWITCH and CACHE_EVICT to controller
	char pktbuf[MAX_BUFSIZE];
	uint32_t pktsize = 0;
	// recv CACHE_POP_INSWITCH_ACK and CACHE_EVICT_ACK from controlelr
	char ackbuf[MAX_BUFSIZE];
	int ack_recvsize = 0;
	// (2) communicate with ptf.popserver
	char ptfbuf[MAX_BUFSIZE];
	uint32_t ptf_sendsize = 0;
	int ptf_recvsize = 0;
	uint16_t switchos_freeidx = 0;
	// load evictdata from ptf
	uint16_t switchos_evictidx = 0;
	val_t switchos_evictvalue = val_t();
	uint32_t switchos_evictseq = 0;
	bool switchos_evictstat = false;

	// TMPDEBUG
	//std::vector<double> evict_load_time_list, evict_sendrecv_time_list, evict_remove_time_list, evict_total_time_list;
	//struct timespec evict_load_t1, evict_load_t2, evict_load_t3, evict_sendrecv_t1, evict_sendrecv_t2, evict_sendrecv_t3, evict_remove_t1, evict_remove_t2, evict_remove_t3, evict_total_t1, evict_total_t2, evict_total_t3;

	while (switchos_running) {
		if (is_stop_cachepop && !popworker_know_stop_cachepop) {
			popworker_know_stop_cachepop = true;
		}
		else if (!is_stop_cachepop) {
		//if (switchos_tail_for_pop != switchos_head_for_pop) {
			cache_pop_t *tmp_cache_pop_ptr = switchos_cache_pop_ptr_queue.read();
			if (tmp_cache_pop_ptr != NULL) {
				//cache_pop_t *tmp_cache_pop_ptr = switchos_cache_pop_ptrs[switchos_tail_for_pop];
				//INVARIANT(tmp_cache_pop_ptr != NULL);
				
				// TMPDEBUG
				bool is_valid = false;
				for (int i = 0; i < valid_global_server_logical_idxes.size(); i++) {
					if (tmp_cache_pop_ptr->serveridx() == valid_global_server_logical_idxes[i]) {
						is_valid = true;
						break;
					}
				}
				INVARIANT(is_valid == true);
				
				if (switchos_cached_keyidx_map.find(tmp_cache_pop_ptr->key()) != switchos_cached_keyidx_map.end()) {
					//printf("Error: populating a key %x cached at kvidx %u from server %d\n", tmp_cache_pop_ptr->key().keyhihi, switchos_cached_keyidx_map[tmp_cache_pop_ptr->key()], int(tmp_cache_pop_ptr->serveridx()));
					//exit(-1);
					printf("[WARNING] populating a key %x cached at kvidx %u from server %d\n", tmp_cache_pop_ptr->key().keyhihi, switchos_cached_keyidx_map[tmp_cache_pop_ptr->key()], int(tmp_cache_pop_ptr->serveridx()));
					fflush(stdout);
				}

				// find corresponding pipeline idx
				int tmp_server_physical_idx = -1;
				for (int i = 0; i < server_physical_num; i++) {
					for (int j = 0; j < server_logical_idxes_list[i].size(); j++) {
						if (server_logical_idxes_list[i][j] == tmp_cache_pop_ptr->serveridx()) {
							tmp_server_physical_idx = i;
							break;
						}
					}
				}
				INVARIANT(tmp_server_physical_idx != -1);
				uint32_t tmp_pipeidx = server_pipeidxes[tmp_server_physical_idx];

				// assign switchos_freeidx for new record 
				if (switchos_perpipeline_cached_empty_index[tmp_pipeidx] < switch_kv_bucket_num) { // With free idx
					switchos_freeidx = switchos_perpipeline_cached_empty_index[tmp_pipeidx];
					switchos_perpipeline_cached_empty_index[tmp_pipeidx] += 1;
					// NOTE: as freeidx of new record must > switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx], no case2 for cache population

					// set valid=0 for atomicity
					////system("bash tofino/setvalid0.sh");
					//ptf_sendsize = serialize_setvalid0(ptfbuf, switchos_freeidx, tmp_pipeidx);
					//udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
					//udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
					//INVARIANT(*((int *)ptfbuf) == SWITCHOS_SETVALID0_ACK); // wait for SWITCHOS_SETVALID0_ACK
					
					// set valid = 0 through reflector
					// NOTE: newkey will be partitioned into the same pipeline as evictkey if any
					// NOTE: we MUST set correct key and switchidx to enter the corresponding egress pipeline
					while (true) {
						setvalid_inswitch_t tmp_setvalid_req(CURMETHOD_ID, tmp_cache_pop_ptr->key(), switchos_freeidx, 0);
						pktsize = tmp_setvalid_req.serialize(pktbuf, MAX_BUFSIZE);
						udpsendto(switchos_popworker_popclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.popworker.popclient_for_reflector");

						bool is_timeout = false;
						is_timeout = udprecvfrom(switchos_popworker_popclient_for_reflector_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.evictclient");
						if (unlikely(is_timeout)) {
							continue;
						}

						setvalid_inswitch_ack_t tmp_setvalid_rsp(CURMETHOD_ID, ackbuf, ack_recvsize);
						//INVARIANT(tmp_setvalid_rsp.key() == tmp_cache_pop_ptr->key());
						if (unlikely(!tmp_setvalid_rsp.is_valid_ || (tmp_setvalid_rsp.key() != tmp_cache_pop_ptr->key()))) {
							printf("invalid key of SETVALID_INSWITCH_ACK %x which should be %x\n", tmp_setvalid_rsp.key().keyhihi, tmp_cache_pop_ptr->key().keyhihi);
							continue;
						}
						break;
					}
				}
				else { // Without free idx
					//CUR_TIME(evict_total_t1);
					bool is_case2 = is_snapshot || (is_snapshot_end && !popworker_know_snapshot_end);

					//CUR_TIME(evict_load_t1);
					
					// get evictdata from ptf framework 
					////system("bash tofino/get_evictdata_setvalid3.sh");
					/*ptf_sendsize = serialize_get_evictdata_setvalid3(ptfbuf, tmp_pipeidx);
					udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
					udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
					INVARIANT(*((int *)ptfbuf) == SWITCHOS_GET_EVICTDATA_SETVALID3_ACK); // wait for SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK
					parse_evictdata(ptfbuf, ptf_recvsize, switchos_evictidx, switchos_evictvalue, switchos_evictseq, switchos_evictstat);*/

					// generate sampled indexes
					uint32_t sampled_idxes[switchos_sample_cnt];
					memset(sampled_idxes, 0, sizeof(uint32_t) * switchos_sample_cnt);
					for (size_t i = 0; i < switchos_sample_cnt; i++) {
						sampled_idxes[i] = rand() % switch_kv_bucket_num; // [0, switch_kv_bucket_num - 1]
					}

					// load frequency counters from data plane
					uint32_t frequency_counters[switchos_sample_cnt];
					memset(frequency_counters, 0, sizeof(uint32_t) * switchos_sample_cnt);
					while (true) {
						//printf("send %d CACHE_EVICT_LOADFREQ_INSWITCHs to reflector\n", switchos_sample_cnt);
						for (size_t i = 0; i < switchos_sample_cnt; i++) {
							cache_evict_loadfreq_inswitch_t tmp_cache_evict_loadfreq_inswitch_req(CURMETHOD_ID, switchos_perpipeline_cached_keyarray[tmp_pipeidx][sampled_idxes[i]], sampled_idxes[i]);
							pktsize = tmp_cache_evict_loadfreq_inswitch_req.serialize(pktbuf, MAX_BUFSIZE);
							udpsendto(switchos_popworker_popclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.popworker.popclient_for_reflector");
						}

						// loop until receiving corresponding ACK (ignore unmatched ACKs which are duplicate ACKs of previous cache population)
						bool is_timeout = false;
						int tmp_acknum = 0;
						while (true) {
							is_timeout = udprecvfrom(switchos_popworker_popclient_for_reflector_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.popclient_for_reflector");
							if (unlikely(is_timeout)) {
								break;
							}

							cache_evict_loadfreq_inswitch_ack_t tmp_cache_evict_loadfreq_inswitch_ack(CURMETHOD_ID, ackbuf, ack_recvsize);
							for (size_t i = 0; i < switchos_sample_cnt; i++) {
								if (static_cast<netreach_key_t>(switchos_perpipeline_cached_keyarray[tmp_pipeidx][sampled_idxes[i]]) == tmp_cache_evict_loadfreq_inswitch_ack.key()) {
									frequency_counters[i] = tmp_cache_evict_loadfreq_inswitch_ack.frequency();
									tmp_acknum += 1;
									break;
								}
								else if (i == switchos_sample_cnt - 1) {
									printf("Receive CACHE_EVICT_LOADFREQ_ACK of key %x, not match any sampled key from %d CACHE_EVICT_LOADFREQ_INSWITCH_ACKs!\n", tmp_cache_evict_loadfreq_inswitch_ack.key().keyhihi, switchos_sample_cnt);
									exit(-1);
								}
							}

							if (tmp_acknum >= switchos_sample_cnt) {
								break;
							}
						}

						if (unlikely(is_timeout)) {
							continue;
						}
						else {
							break;
						}
					}

					// choose the idx with minimum frequency counter as victim
					uint32_t min_frequency_counter = 0;
					uint32_t switchos_evictidx = 0;
					for (size_t i = 0; i < switchos_sample_cnt; i++) {
						if ((i == 0) || (min_frequency_counter > frequency_counters[i])) {
							min_frequency_counter = frequency_counters[i];
							switchos_evictidx = sampled_idxes[i];
						}
					}

					// validate switchos_evictidx and cur_evictkey
					INVARIANT(switchos_evictidx >= 0 && switchos_evictidx < switch_kv_bucket_num);
					netreach_key_t cur_evictkey = switchos_perpipeline_cached_keyarray[tmp_pipeidx][switchos_evictidx];
					if (switchos_cached_keyidx_map.find(cur_evictkey) == switchos_cached_keyidx_map.end()) {
						printf("Evicted key %x at kvidx %d is not cached\n", switchos_perpipeline_cached_keyarray[tmp_pipeidx][switchos_evictidx].keyhihi, switchos_evictidx);
						exit(-1);
					}

					// TMPDEBUG
					//printf("Evict key %ld for new hot key %ld\n", ((uint64_t)cur_evictkey.keyhihi)<<32 | ((uint64_t)cur_evictkey.keyhilo), ((uint64_t)tmp_cache_pop_ptr->key().keyhihi)<<32 | ((uint64_t)tmp_cache_pop_ptr->key().keyhilo));
					//fflush(stdout);

					// set valid = 3 by ptf channel (cannot perform it in data plane due to stateful ALU API limitation)
					//ptf_sendsize = serialize_setvalid3(ptfbuf, switchos_evictidx, tmp_pipeidx);
					//udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
					//udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
					//INVARIANT(*((int *)ptfbuf) == SWITCHOS_SETVALID3_ACK); // wait for SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK
					
					// set valid = 3 through reflector
					while (true) {
						setvalid_inswitch_t tmp_setvalid_req(CURMETHOD_ID, cur_evictkey, switchos_evictidx, 3);
						pktsize = tmp_setvalid_req.serialize(pktbuf, MAX_BUFSIZE);
						udpsendto(switchos_popworker_popclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.popworker.popclient_for_reflector");

						bool is_timeout = false;
						is_timeout = udprecvfrom(switchos_popworker_popclient_for_reflector_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.evictclient");
						if (unlikely(is_timeout)) {
							continue;
						}

						setvalid_inswitch_ack_t tmp_setvalid_rsp(CURMETHOD_ID, ackbuf, ack_recvsize);
						//INVARIANT(tmp_setvalid_rsp.key() == cur_evictkey);
						if (unlikely(!tmp_setvalid_rsp.is_valid_ || (tmp_setvalid_rsp.key() != cur_evictkey))) {
							printf("invalid key of SETVALID_INSWITCH_ACK %x which should be %x\n", tmp_setvalid_rsp.key().keyhihi, cur_evictkey.keyhihi);
							continue;
						}
						break;
					}
					
					// load evicted data of victim from data plane and set valid=3 at the same time for availability of latest value
					while (true) {
						//printf("send CACHE_EVICT_LOADDATA_INSWITCH to reflector\n");
						cache_evict_loaddata_inswitch_t tmp_cache_evict_loaddata_inswitch_req(CURMETHOD_ID, cur_evictkey, switchos_evictidx);
						pktsize = tmp_cache_evict_loaddata_inswitch_req.serialize(pktbuf, MAX_BUFSIZE);
						udpsendto(switchos_popworker_popclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.popworker.popclient_for_reflector");

						bool is_timeout = false;
						is_timeout = udprecvfrom(switchos_popworker_popclient_for_reflector_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.evictclient");
						if (unlikely(is_timeout)) {
							continue;
						}

						cache_evict_loaddata_inswitch_ack_t tmp_cache_evict_loaddata_inswitch_ack(CURMETHOD_ID, ackbuf, ack_recvsize);
						INVARIANT(tmp_cache_evict_loaddata_inswitch_ack.key() == cur_evictkey);
						switchos_evictvalue = tmp_cache_evict_loaddata_inswitch_ack.val();
						switchos_evictseq = tmp_cache_evict_loaddata_inswitch_ack.seq();
						switchos_evictstat = tmp_cache_evict_loaddata_inswitch_ack.stat();
						break;
					}

					//CUR_TIME(evict_load_t2);

					// switchos.popworker.evictclient sends CACHE_EVICT to controller.evictserver
					
					//CUR_TIME(evict_sendrecv_t1);
					if (!is_case2) {
						cache_evict_t tmp_cache_evict(CURMETHOD_ID, cur_evictkey, \
								//val_t(switchos_evictvalbytes, switchos_evictvallen),
								switchos_evictvalue, \
								switchos_evictseq, switchos_evictstat, \
								switchos_perpipeline_cached_serveridxarray[tmp_pipeidx][switchos_evictidx]);
						pktsize = tmp_cache_evict.serialize(pktbuf, MAX_BUFSIZE);
					}
					else { // send CACHE_EVICT_CASE2 for server-side snapshot
						cache_evict_case2_t tmp_cache_evict_case2(CURMETHOD_ID, cur_evictkey, \
								//val_t(switchos_evictvalbytes, switchos_evictvallen),
								switchos_evictvalue, \
								switchos_evictseq, switchos_evictstat, \
								switchos_perpipeline_cached_serveridxarray[tmp_pipeidx][switchos_evictidx]);
						pktsize = tmp_cache_evict_case2.serialize(pktbuf, MAX_BUFSIZE);
					}

					while (true) {
						//printf("send CACHE_EVICT to controller\n");
						//dump_buf(pktbuf, pktsize);
						udpsendto(switchos_popworker_evictclient_for_controller_udpsock, pktbuf, pktsize, 0, &controller_evictserver_addr, controller_evictserver_addrlen, "switchos.popworker.evictclient_for_controller");

						// wait for CACHE_EVICT_ACK from controller.evictserver
						// NOTE: no concurrent CACHE_EVICTs -> use request-and-reply manner to wait for entire eviction workflow
						bool is_timeout = udprecvfrom(switchos_popworker_evictclient_for_controller_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.evictclient_for_controller");
						if (unlikely(is_timeout)) {
							continue;
						}
						else {
							cache_evict_ack_t tmp_cache_evict_ack(CURMETHOD_ID, ackbuf, ack_recvsize);
							INVARIANT(tmp_cache_evict_ack.key() == cur_evictkey);
							break;
						}
					}
					//CUR_TIME(evict_sendrecv_t2);

					// store case2
					if (is_case2) {
						process_specialcase(uint16_t(switchos_evictidx), cur_evictkey, \
								//val_t(switchos_evictvalbytes, switchos_evictvallen), 
								val_t(switchos_evictvalue), \
								uint32_t(switchos_evictseq), bool(switchos_evictstat));
					}

					// NOTE: by now, server MUST have a value at least with the same freshness as that in switch
				
					// NOTE: we should set valid = 0 through reflector before removing victim key from cache_lookup_tbl
					// NOTE: we MUST set correct key and switchidx to enter the corresponding egress pipeline
					while (true) {
						setvalid_inswitch_t tmp_setvalid_req(CURMETHOD_ID, cur_evictkey, switchos_evictidx, 0);
						pktsize = tmp_setvalid_req.serialize(pktbuf, MAX_BUFSIZE);
						udpsendto(switchos_popworker_popclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.popworker.popclient_for_reflector");

						bool is_timeout = false;
						is_timeout = udprecvfrom(switchos_popworker_popclient_for_reflector_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.evictclient");
						if (unlikely(is_timeout)) {
							continue;
						}

						setvalid_inswitch_ack_t tmp_setvalid_rsp(CURMETHOD_ID, ackbuf, ack_recvsize);
						//INVARIANT(tmp_setvalid_rsp.key() == cur_evictkey);
						if (unlikely(!tmp_setvalid_rsp.is_valid_ || (tmp_setvalid_rsp.key() != cur_evictkey))) {
							printf("invalid key of SETVALID_INSWITCH_ACK %x which should be %x\n", tmp_setvalid_rsp.key().keyhihi, cur_evictkey.keyhihi);
							continue;
						}
						break;
					}

					//CUR_TIME(evict_remove_t1);
					// remove evicted data from cache_lookup_tbl
					//system("bash tofino/remove_cache_lookup.sh");
					ptf_sendsize = serialize_remove_cache_lookup(ptfbuf, cur_evictkey);
					udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
					udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
					INVARIANT(*((int *)ptfbuf) == SWITCHOS_REMOVE_CACHE_LOOKUP_ACK); // wait for SWITCHOS_REMOVE_CACHE_LOOKUP_ACK
					//CUR_TIME(evict_remove_t2);

					//printf("Evict %x to %x\n", cur_evictkey.keyhihi, tmp_cache_pop_ptr->key().keyhihi);

					// set freeidx as evictidx for cache popluation later
					switchos_freeidx = switchos_evictidx;

					// reset keyarray and serveridxarray at evictidx
					switchos_cached_keyidx_map.erase(cur_evictkey);
					switchos_perpipeline_cached_keyarray[tmp_pipeidx][switchos_evictidx] = netreach_key_t();
					switchos_perpipeline_cached_serveridxarray[tmp_pipeidx][switchos_evictidx] = -1;

					//CUR_TIME(evict_total_t2);

					/*DELTA_TIME(evict_load_t2, evict_load_t1, evict_load_t3);
					DELTA_TIME(evict_sendrecv_t2, evict_sendrecv_t1, evict_sendrecv_t3);
					DELTA_TIME(evict_remove_t2, evict_remove_t1, evict_remove_t3);
					DELTA_TIME(evict_total_t2, evict_total_t1, evict_total_t3);
					evict_load_time_list.push_back(GET_MICROSECOND(evict_load_t3));
					evict_sendrecv_time_list.push_back(GET_MICROSECOND(evict_sendrecv_t3));
					evict_remove_time_list.push_back(GET_MICROSECOND(evict_remove_t3));
					evict_total_time_list.push_back(GET_MICROSECOND(evict_total_t3));

					if ((evict_total_time_list.size() + 1) % 100 == 0) {
						double load_time = 0, sendrecv_time = 0, remove_time = 0, total_time = 0;
						for (size_t i = 0; i < evict_total_time_list.size(); i++) {
							load_time += evict_load_time_list[i];
							sendrecv_time += evict_sendrecv_time_list[i];
							remove_time += evict_remove_time_list[i];
							total_time += evict_total_time_list[i];
						}
						int tmpsize = evict_total_time_list.size();
						printf("average load time: %f, sendrecv time: %f, remove time: %f, total time: %f\n", load_time/tmpsize, sendrecv_time/tmpsize, remove_time/tmpsize, total_time/tmpsize);
					}*/
				}

				/* cache population for new record */

				INVARIANT(switchos_freeidx >= 0 && switchos_freeidx < switch_kv_bucket_num);
				//printf("[switchos.popworker] switchos_perpipeline_cached_empty_index[%d]: %d, switchos_freeidx: %d\n", tmp_pipeidx, int(switchos_perpipeline_cached_empty_index[tmp_pipeidx]), int(switchos_freeidx)); // TMPDEBUG

				// send CACHE_POP_INSWITCH to reflector (TODO: try internal pcie port)
				cache_pop_inswitch_t tmp_cache_pop_inswitch(CURMETHOD_ID, tmp_cache_pop_ptr->key(), tmp_cache_pop_ptr->val(), tmp_cache_pop_ptr->seq(), switchos_freeidx, tmp_cache_pop_ptr->stat());
				pktsize = tmp_cache_pop_inswitch.serialize(pktbuf, MAX_BUFSIZE);

				while (true) {
					udpsendto(switchos_popworker_popclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.popworker.popclient");

					// loop until receiving corresponding ACK (ignore unmatched ACKs which are duplicate ACKs of previous cache population)
					bool is_timeout = false;
					bool with_correctack = false;
					while (true) {
						is_timeout = udprecvfrom(switchos_popworker_popclient_for_reflector_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.popclient");
						if (unlikely(is_timeout)) {
							break;
						}

						cache_pop_inswitch_ack_t tmp_cache_pop_inswitch_ack(CURMETHOD_ID, ackbuf, ack_recvsize);
						if (tmp_cache_pop_inswitch_ack.is_valid_ && tmp_cache_pop_inswitch_ack.key() == tmp_cache_pop_ptr->key()) {
							with_correctack = true;
							break;
						}
					}

					if (unlikely(is_timeout)) {
						continue;
					}
					if (with_correctack) {
						break;
					}
				}

				// (1) add new <key, value> pair into cache_lookup_tbl; DEPRECATED: (2) and set valid=1 to enable the entry
				////system("bash tofino/add_cache_lookup_setvalid1.sh");
				//ptf_sendsize = serialize_add_cache_lookup_setvalid1(ptfbuf, tmp_cache_pop_ptr->key(), switchos_freeidx, tmp_pipeidx);
				ptf_sendsize = serialize_add_cache_lookup(ptfbuf, tmp_cache_pop_ptr->key(), switchos_freeidx);
				udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
				udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
				//INVARIANT(*((int *)ptfbuf) == SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK); // wait for SWITCHOS_ADD_CACHE_LOOKUP_SETVALID1_ACK
				INVARIANT(*((int *)ptfbuf) == SWITCHOS_ADD_CACHE_LOOKUP_ACK); // wait for SWITCHOS_ADD_CACHE_LOOKUP_ACK
				
				// set valid = 1 through reflector
				// NOTE: newkey will be partitioned into the same pipeline as evictkey if any
				while (true) {
					setvalid_inswitch_t tmp_setvalid_req(CURMETHOD_ID, tmp_cache_pop_ptr->key(), switchos_freeidx, 1);
					pktsize = tmp_setvalid_req.serialize(pktbuf, MAX_BUFSIZE);
					udpsendto(switchos_popworker_popclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.popworker.popclient_for_reflector");

					bool is_timeout = false;
					is_timeout = udprecvfrom(switchos_popworker_popclient_for_reflector_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.evictclient");
					if (unlikely(is_timeout)) {
						continue;
					}

					setvalid_inswitch_ack_t tmp_setvalid_rsp(CURMETHOD_ID, ackbuf, ack_recvsize);
					//INVARIANT(tmp_setvalid_rsp.key() == tmp_cache_pop_ptr->key());
					if (unlikely(!tmp_setvalid_rsp.is_valid_ || (tmp_setvalid_rsp.key() != tmp_cache_pop_ptr->key()))) {
						printf("invalid key of SETVALID_INSWITCH_ACK %x which should be %x\n", tmp_setvalid_rsp.key().keyhihi, tmp_cache_pop_ptr->key().keyhihi);
						continue;
					}
					break;
				}

				switchos_cached_keyidx_map.insert(std::pair<netreach_key_t, uint32_t>(tmp_cache_pop_ptr->key(), switchos_freeidx));
				switchos_perpipeline_cached_keyarray[tmp_pipeidx][switchos_freeidx] = tmp_cache_pop_ptr->key();
				switchos_perpipeline_cached_serveridxarray[tmp_pipeidx][switchos_freeidx] = tmp_cache_pop_ptr->serveridx();

				// free CACHE_POP
				delete tmp_cache_pop_ptr;
				tmp_cache_pop_ptr = NULL;
				//switchos_cache_pop_ptrs[switchos_tail_for_pop] = NULL;
				//switchos_tail_for_pop = (switchos_tail_for_pop + 1) % MQ_SIZE;

				// reset metadata for popclient_for_ptf
				ptf_sendsize = 0;
				ptf_recvsize = 0;
				switchos_freeidx = 0;
				switchos_evictidx = 0;
				switchos_evictvalue = val_t();
				switchos_evictseq = 0;
				switchos_evictstat = false;
			} // tmp_cache_pop_ptr != NULL
		} // !is_stop_cachepop
		if (is_snapshot_end && !popworker_know_snapshot_end) {
			popworker_know_snapshot_end = true;
		}
	}

	// send SWITCHOS_PTF_POPSERVER_END to ptf.popserver
	memcpy(ptfbuf, &SWITCHOS_PTF_POPSERVER_END, sizeof(int));
	udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");

	close(switchos_popworker_popclient_for_reflector_udpsock);
	close(switchos_popworker_evictclient_for_controller_udpsock);
	close(switchos_popworker_popclient_for_ptf_udpsock);
	pthread_exit(nullptr);
}

void *run_switchos_snapshotserver(void *param) {
	// for SNAPSHOT_START/_ACK
	struct sockaddr_in controller_snapshotclient_addr;
	socklen_t controller_snapshotclient_addrlen = sizeof(struct sockaddr);
	//bool with_controller_snapshotclient_addr = false;

	struct sockaddr_in ptf_addr;
	set_sockaddr(ptf_addr, inet_addr("127.0.0.1"), switchos_ptf_snapshotserver_port);
	socklen_t ptf_addrlen = sizeof(struct sockaddr_in);
	
	// for loading snapshot data
	sockaddr_in reflector_cp2dpserver_addr;
	set_sockaddr(reflector_cp2dpserver_addr, inet_addr(reflector_ip_for_switchos), reflector_cp2dpserver_port);
	int reflector_cp2dpserver_addr_len = sizeof(struct sockaddr);

	// for snapshot data from data plane instead of ptf
	//dynamic_array_t ptf_largebuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
	val_t ** switchos_perpipeline_snapshot_values = new val_t*[switch_pipeline_num];
	uint32_t ** switchos_perpipeline_snapshot_seqs = new uint32_t*[switch_pipeline_num];
	bool ** switchos_perpipeline_snapshot_stats = new bool*[switch_pipeline_num];
	for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
		switchos_perpipeline_snapshot_values[tmp_pipeidx] = NULL;
		switchos_perpipeline_snapshot_seqs[tmp_pipeidx] = NULL;
		switchos_perpipeline_snapshot_stats[tmp_pipeidx] = NULL;
	}
	
	// for consistent snapshot data to controller
	dynamic_array_t dynamicbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);

	printf("[switchos.snapshotserver] ready\n");
	switchos_ready_threads++;

	while (!switchos_running) {}

	bool is_first_snapshot = true;

	// communicate with controller.snapshotclient
	char recvbuf[MAX_BUFSIZE];
	int recvsize = 0;
	char sendbuf[MAX_BUFSIZE];
	int sendsize = 0;
	int control_type = -1;
	int snapshotid = -1;
	// communicate with ptf.snapshotserver
	char ptfbuf[MAX_BUFSIZE];
	int ptf_recvsize = 0;
	// communicate with reflector.cp2dpserver
	char pktbuf[MAX_BUFSIZE];
	int pktsize = 0;
	int pkt_recvsize = 0;
	struct timespec stop_cachepop_t1, stop_cachepop_t2, stop_cachepop_t3, enable_singlepath_t1, enable_singlepath_t2, enable_singlepath_t3, load_snapshotdata_t1, load_snapshotdata_t2, load_snapshotdata_t3;
	while (switchos_running) {
		// wait for control instruction from controller.snapshotclient
		/*if (!with_controller_snapshotclient_addr) {
			udprecvfrom(switchos_snapshotserver_udpsock, recvbuf, MAX_BUFSIZE, 0, &controller_snapshotclient_addr, &controller_snapshotclient_addrlen, recvsize, "switchos.snapshotserver");
			with_controller_snapshotclient_addr = true;
		}
		else {
			udprecvfrom(switchos_snapshotserver_udpsock, recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "switchos.snapshotserver");
		}*/
		udprecvfrom(switchos_snapshotserver_udpsock, recvbuf, MAX_BUFSIZE, 0, &controller_snapshotclient_addr, &controller_snapshotclient_addrlen, recvsize, "switchos.snapshotserver");

		snapshot_signal_t cur_signal(recvbuf, recvsize);

		// Fix duplicate packet
		if (control_type != SNAPSHOT_CLEANUP && control_type == cur_signal.control_type() && snapshotid == cur_signal.snapshotid()) {
			printf("[switchos.snapshotserver] receive duplicate control type %d for snapshot id %d\n", control_type, snapshotid); // TMPDEBUG
			fflush(stdout);
			continue;
		}
		else {
			control_type = cur_signal.control_type();
			snapshotid = cur_signal.snapshotid();
			printf("[switchos.snapshotserver] receive control type %d for snapshot id %d\n", control_type, snapshotid); // TMPDEBUG
			fflush(stdout);
		}

		if (control_type == SNAPSHOT_CLEANUP) {
			// (1) cleanup stale snapshot
			
			// cleanup dataplane: disable single path, reset flag and reg
			memcpy(ptfbuf, &SWITCHOS_CLEANUP, sizeof(int));
			udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
			udprecvfrom(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.snapshotserver.snapshotclient_for_ptf");
			INVARIANT(ptf_recvsize == sizeof(int) && *((int *)ptfbuf) == SWITCHOS_CLEANUP_ACK);
			
			// enable cache popuation/eviction
			is_stop_cachepop = false;
			popworker_know_stop_cachepop = false;

			if (likely(is_first_snapshot == false)) { // not the first snapshot
				for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
					INVARIANT(switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx] != NULL && switchos_perpipeline_cached_serveridxarray_backup[tmp_pipeidx] != NULL);
				}

				// clear stale in-switch state
				
				// disable single path
				memcpy(ptfbuf, &SWITCHOS_DISABLE_SINGLEPATH, sizeof(int));
				udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
				udprecvfrom(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.snapshotserver.snapshotclient_for_ptf");
				INVARIANT(ptf_recvsize == sizeof(int) && *((int *)ptfbuf) == SWITCHOS_DISABLE_SINGLEPATH_ACK);

				// reset snapshot flag as false in data plane
				memcpy(ptfbuf, &SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG, sizeof(int));
				udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
				udprecvfrom(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.snapshotserver.snapshotclient_for_ptf");
				INVARIANT(ptf_recvsize == sizeof(int) && *((int *)ptfbuf) == SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG_ACK);

				// notify to end stale snapshot
				memory_fence();
				is_snapshot_end = true;
				is_snapshot = false;
				memory_fence();
				popworker_know_snapshot_end = false;
				specialcaseserver_know_snapshot_end = false;
				memory_fence();

				// wait for case2 from popworker and case1 from specialcaseserver (finish using switchos_perpipeline_specialcases_ptr)
				while (!popworker_know_snapshot_end || !specialcaseserver_know_snapshot_end) {}

				// end stale snapshot (by now popworker/specialcaseserver will not access switchos_perpipeline_specialcases_ptr and backuped metadata)
				memory_fence();
				is_snapshot_end = false;
				memory_fence();
				popworker_know_snapshot_end = false;
				specialcaseserver_know_snapshot_end = false;
				memory_fence();

				//INVARIANT(switchos_perpipeline_snapshot_values != NULL && switchos_perpipeline_snapshot_seqs != NULL && switchos_perpipeline_snapshot_stats != NULL);
				for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
					// free stale special cases
					// NOTE: popserver/specialcaseserver will not touch speicalcases_ptr now, as both is_snapshot/is_snapshot_end are false
					if (switchos_perpipeline_specialcases_ptr[tmp_pipeidx] != NULL) {
						delete switchos_perpipeline_specialcases_ptr[tmp_pipeidx];
						switchos_perpipeline_specialcases_ptr[tmp_pipeidx] = NULL;
					}

					// free backuped metadata
					// NOTE: popserver/specialcaseserver will not touch backuped metadata now, as both is_snapshot/is_snapshot_end are false
					if (switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx] != NULL) {
						delete [] switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx];
						switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx] = NULL;
					}
					if (switchos_perpipeline_cached_serveridxarray_backup[tmp_pipeidx] != NULL) {
						delete [] switchos_perpipeline_cached_serveridxarray_backup[tmp_pipeidx];
						switchos_perpipeline_cached_serveridxarray_backup[tmp_pipeidx] = NULL;
					}
					switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx] = 0;

					// free snapshot data
					if (switchos_perpipeline_snapshot_values[tmp_pipeidx] != NULL) {
						delete [] switchos_perpipeline_snapshot_values[tmp_pipeidx];
						switchos_perpipeline_snapshot_values[tmp_pipeidx] = NULL;
					}
					if (switchos_perpipeline_snapshot_seqs[tmp_pipeidx] != NULL) {
						delete [] switchos_perpipeline_snapshot_seqs[tmp_pipeidx];
						switchos_perpipeline_snapshot_seqs[tmp_pipeidx] = NULL;
					}
					if (switchos_perpipeline_snapshot_stats[tmp_pipeidx] != NULL) {
						delete [] switchos_perpipeline_snapshot_stats[tmp_pipeidx];
						switchos_perpipeline_snapshot_stats[tmp_pipeidx] = NULL;
					}
				}
			}

			// verification
			INVARIANT(switchos_perpipeline_snapshot_values != NULL && switchos_perpipeline_snapshot_seqs != NULL && switchos_perpipeline_snapshot_stats != NULL);
			for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
				INVARIANT(switchos_perpipeline_specialcases_ptr[tmp_pipeidx] == NULL);
				INVARIANT(switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx] == NULL && switchos_perpipeline_cached_serveridxarray_backup[tmp_pipeidx] == NULL);
				INVARIANT(switchos_perpipeline_snapshot_values[tmp_pipeidx] == NULL && switchos_perpipeline_snapshot_seqs[tmp_pipeidx] == NULL && switchos_perpipeline_snapshot_stats[tmp_pipeidx] == NULL);
			}
			//INVARIANT(is_stop_cachepop == false && popworker_know_stop_cachepop == false);
			//INVARIANT(is_snapshot == false && is_snapshot_end == false && popworker_know_snapshot_end == false && specialcaseserver_know_snapshot_end == false);*/
			if(unlikely(!(is_stop_cachepop == false && popworker_know_stop_cachepop == false))) {
				is_stop_cachepop = false;
				popworker_know_stop_cachepop = false;
			}
			if(unlikely(!(is_snapshot == false && is_snapshot_end == false && popworker_know_snapshot_end == false && specialcaseserver_know_snapshot_end == false))) {
				is_snapshot = false;
				is_snapshot_end = false;
				popworker_know_snapshot_end = false;
				specialcaseserver_know_snapshot_end = false;
			}

			// (2) init for new snapshot
			
			is_first_snapshot = false;
			
			// create new speical cases (freed by next SNAPSHOT_CLEANUP)
			for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
				switchos_perpipeline_specialcases_ptr[tmp_pipeidx] = new concurrent_specialcase_map_t();
			}

			CUR_TIME(stop_cachepop_t1);
			// stop cache population/eviction (stop until we can ensure that snapshot has started aka SNAPSHOT_START)
			is_stop_cachepop = true;
			memory_fence();

			// wait until popworker_know_stop_cachepop = true
			while (!popworker_know_stop_cachepop) {}
			// by now, cache population/eviction is temporarily stopped -> snapshotserver can backup cache metadata atomically

			// backup cache metadata (freed by next SNAPSHOT_CLEANUP)
			// NOTE: is_snapshot/_end = false -> we can write backuped metadata
			// NOTE: is_stop_cache/popworker_know_stop_cachepop = true -> we can read cache metadata
			memory_fence();
			for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
				switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx] = new netreach_key_t[switch_kv_bucket_num];
				INVARIANT(switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx] != NULL);
				memcpy(switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx], (void *)switchos_perpipeline_cached_keyarray[tmp_pipeidx], switch_kv_bucket_num * sizeof(netreach_key_t));
				switchos_perpipeline_cached_serveridxarray_backup[tmp_pipeidx] = new uint16_t[switch_kv_bucket_num];
				INVARIANT(switchos_perpipeline_cached_serveridxarray_backup[tmp_pipeidx] != NULL);
				memcpy(switchos_perpipeline_cached_serveridxarray_backup[tmp_pipeidx], (void *)switchos_perpipeline_cached_serveridxarray[tmp_pipeidx], switch_kv_bucket_num * sizeof(uint16_t));
				switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx] = switchos_perpipeline_cached_empty_index[tmp_pipeidx];
			}
			memory_fence();

			// reset local control plane bandwidth cost
			mutex_for_specialcasebwcost.lock();
			memset(switchos_perserver_specialcasebwcosts, 0, sizeof(uint64_t) * max_server_total_logical_num);
			mutex_for_specialcasebwcost.unlock();
			
			// sendback SNAPSHOT_CLEANUP_ACK
			snapshot_signal_t snapshot_cleanupack_signal(SNAPSHOT_CLEANUP_ACK, snapshotid);
			sendsize = snapshot_cleanupack_signal.serialize(sendbuf, MAX_BUFSIZE);
			udpsendto(switchos_snapshotserver_udpsock, sendbuf, sendsize, 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "switchos.snapshotserver");
		}
		else if (control_type == SNAPSHOT_PREPARE) {
			CUR_TIME(enable_singlepath_t1);
			// enable a single path to prepare for setting snapshot with atomicity
			memcpy(ptfbuf, &SWITCHOS_ENABLE_SINGLEPATH, sizeof(int));
			udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
			udprecvfrom(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.snapshotserver.snapshotclient_for_ptf");
			INVARIANT(ptf_recvsize == sizeof(int) && *((int *)ptfbuf) == SWITCHOS_ENABLE_SINGLEPATH_ACK);

			// sendback SNAPSHOT_PREPARE_ACK
			snapshot_signal_t snapshot_prepareack_signal(SNAPSHOT_PREPARE_ACK, snapshotid);
			sendsize = snapshot_prepareack_signal.serialize(sendbuf, MAX_BUFSIZE);
			udpsendto(switchos_snapshotserver_udpsock, sendbuf, sendsize, 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "switchos.snapshotserver");
		}
		else if (control_type == SNAPSHOT_SETFLAG) {
			// ptf sets snapshot flag as true atomically
			//system("bash tofino/set_snapshot_flag.sh");
			memcpy(ptfbuf, &SWITCHOS_SET_SNAPSHOT_FLAG, sizeof(int));
			memcpy(ptfbuf + sizeof(int), &snapshotid, sizeof(int)); // for seq_hdr.snapshot_token
			udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, 2*sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
			udprecvfrom(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.snapshotserver.snapshotclient_for_ptf");
			INVARIANT(ptf_recvsize == sizeof(int) && *((int *)ptfbuf) == SWITCHOS_SET_SNAPSHOT_FLAG_ACK);

			// NOTE: by now data plane starts to report case1 to controller, although switchos wlll not process case1 after SNAPSHOT_START, the special cases will be stored in UDP socket receive buffer

			// sendback SNAPSHOT_SETFLAG_ACK
			snapshot_signal_t snapshot_setflagack_signal(SNAPSHOT_SETFLAG_ACK, snapshotid);
			sendsize = snapshot_setflagack_signal.serialize(sendbuf, MAX_BUFSIZE);
			udpsendto(switchos_snapshotserver_udpsock, sendbuf, sendsize, 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "switchos.snapshotserver");
		}
		else if (control_type == SNAPSHOT_START) {
			// disable single path
			memcpy(ptfbuf, &SWITCHOS_DISABLE_SINGLEPATH, sizeof(int));
			udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
			udprecvfrom(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.snapshotserver.snapshotclient_for_ptf");
			INVARIANT(ptf_recvsize == sizeof(int) && *((int *)ptfbuf) == SWITCHOS_DISABLE_SINGLEPATH_ACK);
			CUR_TIME(enable_singlepath_t2);

			// enable special case processing and resume cache population/eviction
			//INVARIANT(is_snapshot == false && is_stop_cachepop == true && popworker_know_stop_cachepop == true);
			if (unlikely(!(is_snapshot == false && is_stop_cachepop == true && popworker_know_stop_cachepop == true))) {
				COUT_VAR(is_snapshot);
				COUT_VAR(is_stop_cachepop);
				COUT_VAR(popworker_know_stop_cachepop);
				exit(-1);
			}
			is_snapshot = true; // notify popworker and specialcaseserver to collect special cases
			is_stop_cachepop = false; // resume cache population/eviction
			memory_fence();
			popworker_know_stop_cachepop = false;
			CUR_TIME(stop_cachepop_t2);

			DELTA_TIME(stop_cachepop_t2, stop_cachepop_t1, stop_cachepop_t3);
			DELTA_TIME(enable_singlepath_t2, enable_singlepath_t1, enable_singlepath_t3);
			printf("Time of stopping cache population: %f ms\n", GET_MICROSECOND(stop_cachepop_t3) / 1000.0);
			printf("Time of enabling single path: %f ms\n", GET_MICROSECOND(enable_singlepath_t3) / 1000.0);
			fflush(stdout);
			

#ifdef DEBUG_SNAPSHOT
			// TMPDEBUG (NOTE: temporarily dislabe timeout-and-retry of SNAPSHOT_START in controller to debug snapshot here)
			printf("Type to load snapshot data...\n");
			getchar();
#endif

			// for each pipeline, we load vallen, value, deleted, and savedseq in [0, switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]-1]
			CUR_TIME(load_snapshotdata_t1);
			for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
				if (switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx] > 0) {
					// NOTE: freed by next SNAPSHOT_CLEANUP
					switchos_perpipeline_snapshot_values[tmp_pipeidx] = new val_t[switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]];
					switchos_perpipeline_snapshot_seqs[tmp_pipeidx] = new uint32_t[switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]];
					switchos_perpipeline_snapshot_stats[tmp_pipeidx] = new bool[switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]];

					// load snapshot data from data plane
					////system("bash tofino/load_snapshot_data.sh"); // load snapshot (maybe inconsistent -> need rollback later)
					//uint32_t ptf_sendsize = serialize_load_snapshot_data(ptfbuf, switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx], tmp_pipeidx);
					//udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
					//ptf_largebuf.clear();
					//udprecvlarge_udpfrag(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptf_largebuf, 0, NULL, NULL, "switchos.snapshotserver.snapshotclient_for_ptf");
					//parse_snapshotdata_fromptf(ptf_largebuf.array(), ptf_largebuf.size(), 
					//		switchos_perpipeline_snapshot_values[tmp_pipeidx], switchos_perpipeline_snapshot_seqs[tmp_pipeidx], switchos_perpipeline_snapshot_stats[tmp_pipeidx], switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]);

					// NOTE: now we directly send all LOADSNAPSHOTDATA_INSWITCH reqs once a time
					// TODO: we can send LOADSNAPSHOTDATA_INSWITCH reqs batch by batch (e.g., 1000 pkts each time) if necessary
					// NOTE: each ingress pipeline of data plane can inject the packet into the corresponding egress pipeline by key-based partition
					for (uint32_t tmp_loadsnapshotdata_idx = 0; tmp_loadsnapshotdata_idx < switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]; tmp_loadsnapshotdata_idx++) {
						loadsnapshotdata_inswitch_t tmp_loadsnapshotdata_inswitch_req(CURMETHOD_ID, switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx][tmp_loadsnapshotdata_idx], tmp_loadsnapshotdata_idx);
						pktsize = tmp_loadsnapshotdata_inswitch_req.serialize(pktbuf, MAX_BUFSIZE);
						udpsendto(switchos_snapshotserver_snapshotclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.snapshotserver.snapshotclient_for_reflector");
					}

					// switchos-driven timeout-and-retry mechanism
					bool received_bitmap[switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]];
					memset(received_bitmap, 0, sizeof(bool) * switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]);
					int received_num = 0;
					while (true) {
						bool is_timeout = false;
						is_timeout = udprecvfrom(switchos_snapshotserver_snapshotclient_for_reflector_udpsock, pktbuf, MAX_BUFSIZE, 0, NULL, NULL, pkt_recvsize, "switchos.snapshotserver.snapshotclient_for_reflector");
						if (unlikely(is_timeout)) {
							// send all unreceived packets once again
							for (uint32_t unreceived_idx = 0; unreceived_idx < switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]; unreceived_idx++) {
								if (received_bitmap[unreceived_idx] == false) {
									loadsnapshotdata_inswitch_t tmp_loadsnapshotdata_inswitch_req(CURMETHOD_ID, switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx][unreceived_idx], unreceived_idx);
									pktsize = tmp_loadsnapshotdata_inswitch_req.serialize(pktbuf, MAX_BUFSIZE);
									udpsendto(switchos_snapshotserver_snapshotclient_for_reflector_udpsock, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.snapshotserver.snapshotclient_for_reflector");
								}
							}
						}
						else {
							loadsnapshotdata_inswitch_ack_t tmp_loadsnapshotdata_inswitch_ack(CURMETHOD_ID, pktbuf, pkt_recvsize);
							INVARIANT(tmp_loadsnapshotdata_inswitch_ack.key() == switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx][tmp_loadsnapshotdata_inswitch_ack.idx()]);
							if (received_bitmap[tmp_loadsnapshotdata_inswitch_ack.idx()] == false) {
								switchos_perpipeline_snapshot_values[tmp_pipeidx][tmp_loadsnapshotdata_inswitch_ack.idx()] = tmp_loadsnapshotdata_inswitch_ack.val();
								switchos_perpipeline_snapshot_seqs[tmp_pipeidx][tmp_loadsnapshotdata_inswitch_ack.idx()] = tmp_loadsnapshotdata_inswitch_ack.seq();
								switchos_perpipeline_snapshot_stats[tmp_pipeidx][tmp_loadsnapshotdata_inswitch_ack.idx()] = tmp_loadsnapshotdata_inswitch_ack.stat();
								received_num += 1;
								received_bitmap[tmp_loadsnapshotdata_inswitch_ack.idx()] = true;
							}
							if (received_num >= switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]) {
								break;
							}
						}
					}
				}
			}
			CUR_TIME(load_snapshotdata_t2);
			DELTA_TIME(load_snapshotdata_t2, load_snapshotdata_t1, load_snapshotdata_t3);
			//printf("Time of loading snapshot data by ptf: %f s\n", GET_MICROSECOND(load_snapshotdata_t3) / 1000.0 / 1000.0);
			printf("Time of loading snapshot data by reflector: %f s\n", GET_MICROSECOND(load_snapshotdata_t3) / 1000.0 / 1000.0);
			fflush(stdout);

			// sendback SNAPSHOT_START_ACK
			snapshot_signal_t snapshot_startack_signal(SNAPSHOT_START_ACK, snapshotid);
			sendsize = snapshot_startack_signal.serialize(sendbuf, MAX_BUFSIZE);
			udpsendto(switchos_snapshotserver_udpsock, sendbuf, sendsize, 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "switchos.snapshotserver");
		}
		else if (control_type == SNAPSHOT_GETDATA) {
			// reset snapshot flag as false in data plane
			//system("bash tofino/reset_snapshot_flag_and_reg.sh");
			memcpy(ptfbuf, &SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG, sizeof(int));
			udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");
			udprecvfrom(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.snapshotserver.snapshotclient_for_ptf");
			INVARIANT(ptf_recvsize == sizeof(int) && *((int *)ptfbuf) == SWITCHOS_RESET_SNAPSHOT_FLAG_AND_REG_ACK);

			// finish snapshot
			//INVARIANT(is_snapshot == true && is_snapshot_end == false && popworker_know_snapshot_end == false && specialcaseserver_know_snapshot_end == false);
			if (unlikely(!(is_snapshot == true && is_snapshot_end == false && popworker_know_snapshot_end == false && specialcaseserver_know_snapshot_end == false))) {
				COUT_VAR(is_snapshot);
				COUT_VAR(is_snapshot_end);
				COUT_VAR(popworker_know_snapshot_end);
				COUT_VAR(specialcaseserver_know_snapshot_end);
				exit(-1);
			}
			is_snapshot_end = true;
			is_snapshot = false;
			memory_fence();

			// wait for case2 from popworker and case1 from specialcaseserver
			while (!popworker_know_snapshot_end || !specialcaseserver_know_snapshot_end) {}

			// end snapshot (by now popworker/specialcaseserver will not access switchos_perpipeline_specialcases_ptr and backuped metadata)
			is_snapshot_end = false;
			memory_fence();
			popworker_know_snapshot_end = false;
			specialcaseserver_know_snapshot_end = false;

			// rollback
			for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
#ifdef DEBUG_SNAPSHOT
				// TMPDEBUG
				printf("[before rollback] snapshot size of pipeline %d: %d\n", tmp_pipeidx, switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]);
				fflush(stdout);
				/*for (size_t debugi = 0; debugi < switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]; debugi++) {
					char debugbuf[MAX_BUFSIZE];
					uint32_t debugkeysize = switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx][debugi].serialize(debugbuf, MAX_BUFSIZE);
					uint32_t debugvalsize = switchos_perpipeline_snapshot_values[tmp_pipeidx][debugi].serialize(debugbuf+debugkeysize, MAX_BUFSIZE-debugkeysize);
					printf("serialized debug key-value[%d]:\n", int(debugi));
					dump_buf(debugbuf, debugkeysize+debugvalsize);
					printf("seq: %d, stat %d\n", switchos_perpipeline_snapshot_seqs[tmp_pipeidx][debugi], switchos_perpipeline_snapshot_stats[tmp_pipeidx][debugi]?1:0);
				}*/
#endif

				// perform rollback (now both popserver/specicalserver will not touch specialcases)
				/*for (std::map<uint16_t, special_case_t>::iterator iter = switchos_specialcases.begin(); iter != switchos_specialcases.end(); iter++) {
					INVARIANT(switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx][iter->first] == iter->second._key);
					switchos_perpipeline_snapshot_values[tmp_pipeidx][iter->first] = iter->second._val;
					switchos_perpipeline_snapshot_seqs[tmp_pipeidx][iter->first] = iter->second._seq;
					switchos_perpipeline_snapshot_stats[tmp_pipeidx][iter->first] = iter->second._valid;
				}*/
				INVARIANT(switchos_perpipeline_specialcases_ptr[tmp_pipeidx] != NULL);
				concurrent_specialcase_map_t::DataSource source(0, (concurrent_specialcase_map_t *)switchos_perpipeline_specialcases_ptr[tmp_pipeidx]);
				source.advance_to_next_valid();
				int specialcase_cnt = 0;
				while (source.has_next) {
					special_case_t tmpcase = source.get_val();
					INVARIANT(switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx][source.get_key()] == tmpcase._key);
					switchos_perpipeline_snapshot_values[tmp_pipeidx][source.get_key()] = tmpcase._val;
					switchos_perpipeline_snapshot_seqs[tmp_pipeidx][source.get_key()] = tmpcase._seq;
					switchos_perpipeline_snapshot_stats[tmp_pipeidx][source.get_key()] = tmpcase._valid;
					source.advance_to_next_valid();
					specialcase_cnt += 1;
				}
				COUT_VAR(specialcase_cnt);

#ifdef DEBUG_SNAPSHOT
				// TMPDEBUG
				printf("[after rollback] snapshot size of pipeline %d: %d\n", tmp_pipeidx, switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]);
				fflush(stdout);
				/*for (size_t debugi = 0; debugi < switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx]; debugi++) {
					char debugbuf[MAX_BUFSIZE];
					uint32_t debugkeysize = switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx][debugi].serialize(debugbuf, MAX_BUFSIZE);
					uint32_t debugkeysize = switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx][debugi].serialize(debugbuf, MAX_BUFSIZE);
					printf("serialized debug key-value[%d]:\n", int(debugi));
					dump_buf(debugbuf, debugkeysize+debugvalsize);
					printf("seq: %d, stat %d\n", switchos_perpipeline_snapshot_seqs[tmp_pipeidx][debugi], switchos_perpipeline_snapshot_stats[tmp_pipeidx][debugi]?1:0);
				}*/
#endif
			}

			uint64_t cur_perserver_specialcasebwcosts[max_server_total_logical_num];
			mutex_for_specialcasebwcost.lock();
			memcpy(cur_perserver_specialcasebwcosts, switchos_perserver_specialcasebwcosts, sizeof(uint64_t) * max_server_total_logical_num);
			mutex_for_specialcasebwcost.unlock();

			// snapshot data: <int SNAPSHOT_GETDATA_ACK, int32_t total_bytes, uint64_t cur_specialcase_bwcost, per-server data>
			// per-server data: <int32_t perserver_bytes, uint16_t serveridx, int32_t recordcnt, per-record data>
			// per-record data: <16B key, uint16_t vallen, value (w/ padding), uint32_t seq, bool stat>
			dynamicbuf.clear();
			int total_bytes = dynamic_serialize_snapshot_getdata_ack(dynamicbuf, SNAPSHOT_GETDATA_ACK, switch_pipeline_num, max_server_total_logical_num, switchos_perpipeline_cached_empty_index_backup, switchos_perpipeline_cached_serveridxarray_backup, switchos_perpipeline_cached_keyarray_backup, switchos_perpipeline_snapshot_values, switchos_perpipeline_snapshot_seqs, switchos_perpipeline_snapshot_stats, cur_perserver_specialcasebwcosts);

			// send rollbacked snapshot data to controller.snapshotserver
			printf("[switchos.snapshotserver] send snapshot data to controller\n"); // TMPDEBUG
			// printf("total_bytes: %dB, cur_specialcase_bwcost: %dB\n", total_bytes, cur_specialcase_bwcost); // TMPDEBUG
			fflush(stdout);
			udpsendlarge_udpfrag(switchos_snapshotserver_udpsock, dynamicbuf.array(), total_bytes, 0, &controller_snapshotclient_addr, controller_snapshotclient_addrlen, "switchos.snapshotserver");
		}
		else {
			printf("[switchos.snapshotserver] invalid control type: %d\n", control_type);
			exit(-1);
		}
	} // while (switchos_running)

	// send SWITCHOS_PTF_SNAPSHOTSERVER_END to ptf.snapshotserver
	memcpy(ptfbuf, &SWITCHOS_PTF_SNAPSHOTSERVER_END, sizeof(int));
	udpsendto(switchos_snapshotserver_snapshotclient_for_ptf_udpsock, ptfbuf, sizeof(int), 0, &ptf_addr, ptf_addrlen, "switchos.snapshotserver.snapshotclient_for_ptf");

	if (switchos_perpipeline_snapshot_values != NULL) {
		for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
			if (switchos_perpipeline_snapshot_values[tmp_pipeidx] != NULL) {
				delete [] switchos_perpipeline_snapshot_values[tmp_pipeidx];
				switchos_perpipeline_snapshot_values[tmp_pipeidx] = NULL;
			}
			if (switchos_perpipeline_snapshot_seqs[tmp_pipeidx] != NULL) {
				delete [] switchos_perpipeline_snapshot_seqs[tmp_pipeidx];
				switchos_perpipeline_snapshot_seqs[tmp_pipeidx] = NULL;
			}
			if (switchos_perpipeline_snapshot_stats[tmp_pipeidx] != NULL) {
				delete [] switchos_perpipeline_snapshot_stats[tmp_pipeidx];
				switchos_perpipeline_snapshot_stats[tmp_pipeidx] = NULL;
			}
		}
		delete [] switchos_perpipeline_snapshot_values;
		switchos_perpipeline_snapshot_values = NULL;
		delete [] switchos_perpipeline_snapshot_seqs;
		switchos_perpipeline_snapshot_seqs = NULL;
		delete [] switchos_perpipeline_snapshot_stats;
		switchos_perpipeline_snapshot_stats = NULL;
	}

	close(switchos_snapshotserver_udpsock);
	close(switchos_snapshotserver_snapshotclient_for_reflector_udpsock);
	close(switchos_snapshotserver_snapshotclient_for_ptf_udpsock);
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
			netreach_key_t tmpkey;
			uint32_t tmpbwcost;
			switch (pkt_type) {
				case packet_type_t::GETRES_LATEST_SEQ_INSWITCH_CASE1:
					{
						getres_latest_seq_inswitch_case1_t req(CURMETHOD_ID, buf, recv_size);
						process_specialcase(req.idx(), req.key(), req.val(), req.seq(), req.stat());
						tmpkey = req.key();
						tmpbwcost = req.bwcost();
						break;
					}
				case packet_type_t::GETRES_DELETED_SEQ_INSWITCH_CASE1:
					{
						getres_deleted_seq_inswitch_case1_t req(CURMETHOD_ID, buf, recv_size);
						process_specialcase(req.idx(), req.key(), req.val(), req.seq(), req.stat());
						tmpkey = req.key();
						tmpbwcost = req.bwcost();
						break;
					}
				case packet_type_t::PUTREQ_SEQ_INSWITCH_CASE1:
					{
						putreq_seq_inswitch_case1_t req(CURMETHOD_ID, buf, recv_size);
						process_specialcase(req.idx(), req.key(), req.val(), req.seq(), req.stat());
						tmpkey = req.key();
						tmpbwcost = req.bwcost();
						break;
					}
				case packet_type_t::DELREQ_SEQ_INSWITCH_CASE1:
					{
						delreq_seq_inswitch_case1_t req(CURMETHOD_ID, buf, recv_size);
						process_specialcase(req.idx(), req.key(), req.val(), req.seq(), req.stat());
						tmpkey = req.key();
						tmpbwcost = req.bwcost();
						break;
					}
				default:
					{
						printf("[switchos.specialcaseserver] Invalid packet type from reflector.dp2cpserver.specialcaseclient!\n");
						exit(-1);
					}
			} // end of switch

#ifdef USE_HASH
			uint32_t expected_serveridx = tmpkey.get_hashpartition_idx(switch_partition_count, max_server_total_logical_num);
#elif defined(USE_RANGE)
			uint32_t expected_serveridx = tmpkey.get_rangepartition_idx(max_server_total_logical_num);
#endif
			mutex_for_specialcasebwcost.lock();
			switchos_perserver_specialcasebwcosts[expected_serveridx] += tmpbwcost;
			mutex_for_specialcasebwcost.unlock();
		} // is_snapshot || (is_snapshot_end && !specialcaseserver_know_snapshot_end)
	}

	close(switchos_specialcaseserver_udpsock);
	pthread_exit(nullptr);
}

void process_specialcase(const uint16_t &tmpidx, const netreach_key_t &tmpkey, const val_t &tmpval, const uint32_t &tmpseq, const bool &tmpstat) {
	INVARIANT(tmpidx >= 0 && tmpidx < switch_kv_bucket_num);

	// find correpsonding pipeline idx
	// NOTE: for each given key, it can only be stored into cached metadata of at most one pipeline
	int tmp_pipeidx = -1;
	for (int i = 0; i < switch_pipeline_num; i++) {
		INVARIANT(switchos_perpipeline_specialcases_ptr[i] != NULL && switchos_perpipeline_cached_keyarray_backup[i] != NULL);
		if (switchos_perpipeline_cached_keyarray_backup[i][tmpidx] == tmpkey) {
			tmp_pipeidx = i;
			break;
		}
	}
	if (tmp_pipeidx == -1) { // no matched pipeidx means the key was not cached at the snapshot timepoint
		return;
	}

	INVARIANT(switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx] >= 0 && switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx] <= switch_kv_bucket_num);
	if (tmpidx < switchos_perpipeline_cached_empty_index_backup[tmp_pipeidx] && switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx][tmpidx] == tmpkey) { 
		//switchos_mutex_for_specialcases.lock(); 
		//std::map<uint16_t, special_case_t>::iterator specialcase_iter = switchos_specialcases.find(tmpidx); 
		special_case_t tmpcase;
		bool res = switchos_perpipeline_specialcases_ptr[tmp_pipeidx]->get(tmpidx, tmpcase);
		//if (specialcase_iter == switchos_specialcases.end()) { // no special case for the idx/key
		if (!res) { // no special case for the idx/key
			//special_case_t tmpcase; 
			tmpcase._key = tmpkey;
			tmpcase._val = tmpval;
			tmpcase._seq = tmpseq;
			tmpcase._valid = tmpstat; // stat=1 means not deleted
			//switchos_specialcases.insert(std::pair<uint16_t, special_case_t>(tmpidx, tmpcase)); 
			switchos_perpipeline_specialcases_ptr[tmp_pipeidx]->insert(tmpidx, tmpcase);
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
			res = switchos_perpipeline_specialcases_ptr[tmp_pipeidx]->update(tmpidx, tmpcase);
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
	if (switchos_perpipeline_cached_keyarray != NULL) {
		for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
			if (switchos_perpipeline_cached_keyarray[tmp_pipeidx] != NULL) {
				delete [] switchos_perpipeline_cached_keyarray[tmp_pipeidx];
				switchos_perpipeline_cached_keyarray[tmp_pipeidx] = NULL;
			}
		}
		delete [] switchos_perpipeline_cached_keyarray;
		switchos_perpipeline_cached_keyarray = NULL;
	}
	if (switchos_perpipeline_cached_serveridxarray != NULL) {
		for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
			if (switchos_perpipeline_cached_serveridxarray[tmp_pipeidx] != NULL) {
				delete [] switchos_perpipeline_cached_serveridxarray[tmp_pipeidx];
				switchos_perpipeline_cached_serveridxarray[tmp_pipeidx] = NULL;
			}
		}
		delete [] switchos_perpipeline_cached_serveridxarray;
		switchos_perpipeline_cached_serveridxarray = NULL;
	}
	if (switchos_perpipeline_cached_keyarray_backup != NULL) {
		for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
			if (switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx] != NULL) {
				delete [] switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx];
				switchos_perpipeline_cached_keyarray_backup[tmp_pipeidx] = NULL;
			}
		}
		delete [] switchos_perpipeline_cached_keyarray_backup;
		switchos_perpipeline_cached_keyarray_backup = NULL;
	}
	if (switchos_perpipeline_cached_serveridxarray_backup != NULL) {
		for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
			if (switchos_perpipeline_cached_serveridxarray_backup[tmp_pipeidx] != NULL) {
				delete [] switchos_perpipeline_cached_serveridxarray_backup[tmp_pipeidx];
				switchos_perpipeline_cached_serveridxarray_backup[tmp_pipeidx] = NULL;
			}
		}
		delete [] switchos_perpipeline_cached_serveridxarray_backup;
		switchos_perpipeline_cached_serveridxarray_backup = NULL;
	}
	if (switchos_perpipeline_specialcases_ptr != NULL) {
		for (uint32_t tmp_pipeidx = 0; tmp_pipeidx < switch_pipeline_num; tmp_pipeidx++) {
			if (switchos_perpipeline_specialcases_ptr[tmp_pipeidx] != NULL) {
				delete switchos_perpipeline_specialcases_ptr[tmp_pipeidx];
				switchos_perpipeline_specialcases_ptr[tmp_pipeidx] = NULL;
			}
		}
		delete [] switchos_perpipeline_specialcases_ptr;
		switchos_perpipeline_specialcases_ptr = NULL;
	}
	if (switchos_perserver_specialcasebwcosts != NULL) {
		delete [] switchos_perserver_specialcasebwcosts;
		switchos_perserver_specialcasebwcosts = NULL;
	}
}

// switchos <-> ptf.popserver

/*inline uint32_t serialize_setvalid0(char *buf, uint16_t freeidx, uint32_t pipeidx) {
	memcpy(buf, &SWITCHOS_SETVALID0, sizeof(int));
	memcpy(buf + sizeof(int), &freeidx, sizeof(uint16_t));
	memcpy(buf + sizeof(int) + sizeof(uint16_t), &pipeidx, sizeof(uint32_t));
	return sizeof(int) + sizeof(uint16_t) + sizeof(uint32_t);
}*/

//inline uint32_t serialize_add_cache_lookup_setvalid1(char *buf, netreach_key_t key, uint16_t freeidx, uint32_t pipeidx) {
inline uint32_t serialize_add_cache_lookup(char *buf, netreach_key_t key, uint16_t freeidx) {
	memcpy(buf, &SWITCHOS_ADD_CACHE_LOOKUP, sizeof(int));
	uint32_t tmp_keysize = key.serialize(buf + sizeof(int), MAX_BUFSIZE - sizeof(int));
	memcpy(buf + sizeof(int) + tmp_keysize, &freeidx, sizeof(uint16_t));
	return sizeof(int) + tmp_keysize + sizeof(uint16_t);
	//memcpy(buf + sizeof(int) + tmp_keysize + sizeof(uint16_t), &pipeidx, sizeof(uint32_t));
	//return sizeof(int) + tmp_keysize + sizeof(uint16_t) + sizeof(uint32_t);
}

inline uint32_t serialize_writeallseq(char *buf, uint32_t maxseq) {
	memcpy(buf, &SWITCHOS_WRITEALLSEQ, sizeof(int));
	memcpy(buf + sizeof(int), &maxseq, sizeof(uint32_t));
	return sizeof(int) + sizeof(uint32_t);
}

/*inline uint32_t serialize_setvalid3(char *buf, uint16_t evictidx, uint32_t pipeidx) {
	memcpy(buf, &SWITCHOS_SETVALID3, sizeof(int));
	memcpy(buf + sizeof(int), &evictidx, sizeof(uint16_t));
	memcpy(buf + sizeof(int) + sizeof(uint16_t), &pipeidx, sizeof(uint32_t));
	return sizeof(int) + sizeof(uint16_t) + sizeof(uint32_t);
}*/

/*inline uint32_t serialize_get_evictdata_setvalid3(char *buf, uin32_t pipeidx) {
	memcpy(buf, &SWITCHOS_GET_EVICTDATA_SETVALID3, sizeof(int));
	memcpy(buf + sizeof(int), &pipeidx, sizeof(uint32_t));
	return sizeof(int) + sizeof(uint32_t);
}

inline void parse_evictdata(char *buf, int recvsize, uint16_t &switchos_evictidx, val_t &switchos_evictvalue, uint32_t &switchos_evictseq, bool &switchos_evictstat) {
	char * curptr = buf + sizeof(int);
	switchos_evictidx = *((uint16_t *)curptr);
	curptr += sizeof(uint16_t);
	uint32_t tmp_valsize = switchos_evictvalue.deserialize(curptr, recvsize - sizeof(int) - sizeof(uint16_t));
	curptr += tmp_valsize;
	switchos_evictseq = *((uint32_t *)curptr);
	curptr += sizeof(uint32_t);
	switchos_evictstat = *((bool *)curptr);
}*/

inline uint32_t serialize_remove_cache_lookup(char *buf, netreach_key_t key) {
	memcpy(buf, &SWITCHOS_REMOVE_CACHE_LOOKUP, sizeof(int));
	uint32_t tmp_keysize = key.serialize(buf + sizeof(int), MAX_BUFSIZE - sizeof(int));
	return sizeof(int) + tmp_keysize;
}

/*inline uint32_t serialize_load_snapshot_data(char *buf, uint32_t emptyidx, uint32_t pipeidx) {
	memcpy(buf, &SWITCHOS_LOAD_SNAPSHOT_DATA, sizeof(int));
	memcpy(buf + sizeof(int), &emptyidx, sizeof(uint32_t));
	memcpy(buf + sizeof(int) + sizeof(uint32_t), &pipeidx, sizeof(uint32_t));
	return sizeof(int) + sizeof(uint32_t) + sizeof(uint32_t);
}

void parse_snapshotdata_fromptf(char *buf, uint32_t buflen, \
		val_t *values, uint32_t *seqs, bool *stats, uint32_t record_cnt) {
	int control_type = *((int *)buf);
	INVARIANT(control_type == SWITCHOS_LOAD_SNAPSHOT_DATA_ACK);
	int total_bytesnum = *((int32_t *)(buf + sizeof(int)));
	INVARIANT(total_bytesnum <= int(buflen));

	// save snapshot data: <SWITCHOS_LOAD_SNAPSHOT_DATA_ACK, total_bytesnum, records>
	// for each record: <vallen (big-endian), valbytes (same order), seq, result>
	uint32_t tmp_offset = sizeof(int) + sizeof(int32_t);
	for (uint32_t tmp_recordidx = 0; tmp_recordidx < record_cnt; tmp_recordidx++) {
		uint32_t tmp_valsize = values[tmp_recordidx].deserialize(buf + tmp_offset, buflen - tmp_offset);
		tmp_offset += tmp_valsize;
		seqs[tmp_recordidx] = *((uint32_t *)(buf + tmp_offset));
		tmp_offset += sizeof(uint32_t);
		stats[tmp_recordidx] = *((bool *)(buf + tmp_offset));
		tmp_offset += sizeof(bool);
		INVARIANT(int32_t(tmp_offset) <= total_bytesnum);
	}
	memory_fence();
	return;
}*/
