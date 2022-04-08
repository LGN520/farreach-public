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
#include <errno.h>
#include <set>
#include <signal.h> // for signal and raise
#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // inetaddr conversion
#include <sys/time.h> // struct timeval
#include <string.h>
#include <map>

#include "helper.h"
#include "key.h"
#include "val.h"

typedef Key index_key_t;

// Parameters
short switchos_popserver_port = 0;
short switchos_paramserver_port = 0;
uint32_t switch_kv_bucket_num = 0;

// Cache population

// controller.popclient <-> switchos.popserver
int volatile switchos_popserver_tcpsock = -1;
int volatile switchos_paramserver_udpsock = -1;
//std::set<index_key_t> switchos_cached_keyset; // TODO: Comment it after checking server.cached_keyset_list
// message queue between switchos.popserver and switchos.popworker
cache_pop_t ** volatile switchos_cache_pop_ptrs = NULL;
uint32_t switchos_head_for_pop;
uint32_t switchos_tail_for_pop;
index_key_t * volatile switchos_cached_keyarray = NULL; // idx (of inswitch KV) -> key (TODO: different switches for distributed case)
uint32_t switchos_cached_keyarray_empty_index = 0; // [empty index, kv_bucket_num-1] is empty

// Used by switchos.paramserer <-> ptf framework
uint32_t switchos_freeidx = 0; // switchos.popworker write -> launch ptf.population -> switchos.paramserver read (no contention)
const int switchos_poptype = 1; // ptf get switchos_freeidx from paramserver
const int switchos_evicttype = 2; // ptf get evictidx from paramserver

inline void parse_ini(const char *config_file);
void prepare_switchos();
void *run_switchos_popserver(void *param);
void *run_switchos_paramserver(void *param);
void *run_switchos_popworker(void *param);
void close_switchis();

inline void parse_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	switchos_popserver_port = ini.get_switchos_popserver_port();
	switchos_paramserver_port = ini.get_switchos_paramserver_port();
	switch_kv_bucket_num = ini.get_switch_kv_bucket_num();
	
	COUT_VAR(switchos_popserver_port);
	COUT_VAR(switchos_paramserver_port);
	COUT_VAR(switch_kv_bucket_num);
}

void prepare_switchos() {
	// Set popserver socket
	switchos_popserver_tcpsock = socket(AF_INET, SOCK_STREAM, 0);
	if (switchos_popserver_tcpsock == -1) {
		printf("[switch os] Fail to create tcp socket of popserver: errno: %d!\n", errno);
		exit(-1);
	}
	// reuse the occupied port for the last created socket instead of being crashed
	const int trueFlag = 1;
	if (setsockopt(switchos_popserver_tcpsock, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
		printf("[switch os] Fail to setsockopt of popserver: errno: %d!\n", errno);
		exit(-1);
	}
	// Disable udp/tcp check
	int disable = 1;
	if (setsockopt(switchos_popserver_tcpsock, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		COUT_N_EXIT("[switch os] Disable tcp checksum failed");
	}
	// Set timeout for recvfrom/accept of udp/tcp
	/*struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec =  0;
	int res = setsockopt(switchos_popserver_tcpsock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	INVARIANT(res >= 0);*/
	// Set listen address
	sockaddr_in listen_addr;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(switchos_popserver_port);
	if ((bind(switchos_popserver_tcpsock, (struct sockaddr*)&listen_addr, sizeof(listen_addr))) != 0) {
		printf("[switch os] Fail to bind socket on port %hu for popserver, errno: %d!\n", switchos_popserver_port, errno);
		exit(-1);
	}
	if ((listen(switchos_popserver_tcpsock, 1)) != 0) { // MAX_PENDING_CONNECTION = 1
		printf("[switch os] Fail to listen on port %hu for popserver, errno: %d!\n", switchos_popserver_port, errno);
		exit(-1);
	}

	// Set paramserver socket
	switchos_paramserver_udpsock = socket(AF_INET, SOCK_DGRAM, 0);
	if (switchos_paramserver_udpsock == -1) {
		printf("[switch os] Fail to create udp socket of paramserver: errno: %d!\n", errno);
		exit(-1);
	}
	// reuse the occupied port for the last created socket instead of being crashed
	const int trueFlag = 1;
	if (setsockopt(switchos_paramserver_udpsock, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
		printf("[switch os] Fail to setsockopt of paramserver: errno: %d!\n", errno);
		exit(-1);
	}
	// Disable udp/tcp check
	int disable = 1;
	if (setsockopt(switchos_paramserver_udpsock, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		COUT_N_EXIT("[switch os] Disable udp checksum failed");
	}
	// Set timeout for recvfrom/accept of udp/tcp
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec =  0;
	int res = setsockopt(switchos_paramserver_udpsock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	INVARIANT(res >= 0);
	// Set listen address
	//sockaddr_in listen_addr;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(switchos_paramserver_port);
	if ((bind(switchos_paramserver_udpsock, (struct sockaddr*)&listen_addr, sizeof(listen_addr))) != 0) {
		printf("[switch os] Fail to bind socket on port %hu for paramserver, errno: %d!\n", switchos_paramserver_port, errno);
		exit(-1);
	}

	//switchos_cached_keyset.clear();
	switchos_cache_pop_ptrs = new cache_pop_t*[MQ_SIZE];
	switchos_head_for_pop = 0;
	switchos_tail_for_pop = 0;
	switchos_cached_keyarray = new index_key_t[kv_bucket_num];
	switchos_cached_keyarray_empty_index = 0;
}

void *run_switchos_popserver(void *param) {
	// Not used
	struct sockaddr_in controller_addr;
	unsigned int controller_addr_len = sizeof(struct sockaddr);

	int connfd = accept(switchos_popserver_tcpsock, NULL, NULL);
	if (connfd == -1) {
		/*if (errno == EWOULDBLOCK || errno == EINTR) {
			continue; // timeout or interrupted system call
		}
		else {
			COUT_N_EXIT("[switch os] Error of accept: errno = " << std::string(strerror(errno)));
		}*/
		COUT_N_EXIT("[switch os] Error of accept: errno = " << std::string(strerror(errno)));
	}

	// Process CACHE_POP packet <optype, key, vallen, value, stat, seq, serveridx>
	char buf[MAX_BUFSIZE];
	int cur_recv_bytes = -1;
	int8_t optype = -1;
	int32_t vallen = -1;
	int arrive_serveridx_bytes = -1;
	bool is_cached_before = false;
	//index_key_t tmpkey(0, 0, 0, 0);
	const int arrive_optype_bytes = sizeof(int8_t);
	const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(key_t) + sizeof(int32_t);
	while (true) {
		int tmpsize = recv(connfd, buf + cur_recv_bytes, MAX_BUFSIZE - cur_recv_bytes, 0);
		if (tmpsize < 0) {
			// Errno 32 means broken pipe, i.e., server.popclient is closed
			if (errno == 32) {
				printf("[switch os] remote controller.popclient is closed");
				break;
			}
			else {
				COUT_N_EXIT("[switch os] popserver recv fails: " << tmpsize << " errno = " << std::string(strerror(errno)));
			}
		}
		cur_recv_bytes += tmpsize;
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
			INVARIANT(vallen >= 0);
			int padding_size = int(val_t::get_padding_size(vallen)); // padding for value <= 128B
			arrive_serveridx_bytes = arrive_vallen_bytes + vallen + padding_size + sizeof(bool) + sizeof(int32_t) + sizeof(int16_t);
		}

		// Get one complete CACHE_POP
		if (optype != -1 && vallen != -1 && cur_recv_bytes >= arrive_serveridx_bytes) {
			cache_pop_t *tmp_cache_pop_ptr = new cache_pop_t(buf, arrive_serveridx_bytes); // freed by switchos.popworker

			//is_cached_before = (switchos_cached_keyset.find(tmp_cache_pop_ptr->key()) != switchos_cached_keyset.end());
			if (!is_cached_before) {
				// Add key into cached keyset
				//switchos_cached_keyset.insert(tmp_cache_pop_ptr->key());

				if ((switchos_head_for_pop+1)%MQ_SIZE != switchos_tail_for_pop) {
					switchos_cache_pop_ptrs[switchos_head_for_pop] = tmp_cache_pop_ptr;
					switchos_head_for_pop = (switchos_head_for_pop + 1) % MQ_SIZE;
				}
				else {
					printf("[switch os] message queue overflow of switchos.switchos_cache_pop_ptrs!");
				}
			}

			// Move remaining bytes and reset metadata
			if (cur_recv_bytes > arrive_seq_bytes) {
				memcpy(buf, buf + arrive_serveridx_bytes, cur_recv_bytes - arrive_serveridx_bytes);
				cur_recv_bytes = cur_recv_bytes - arrive_serveridx_byets;
				optype = -1;
				vallen = -1;
				arrive_serveridx_bytes = -1;
				is_cached_before = false;
			}
			else {
				cur_recv_bytes = 0;
				optype = -1;
				vallen = -1;
				arrive_serveridx_bytes = -1;
				is_cached_before = false;
			}
		}
	}

	close(connfd);
	close(switchos_popserver_tcpsock);
	pthread_exit(nullptr);
}

void *run_switchos_paramserver(void *param) {
	char buf[MAX_BUFSIZE];
	while (running) {
		// ptf framework is in the same device of switch os
		struct sockaddr_in ptf_addr;
		unsigned int ptf_addr_len = sizeof(struct sockaddr);

		recv_size = recvfrom(switchos_paramserver_udpsock, buf, MAX_BUFSIZE, 0, (struct sockaddr *)&ptf_addr, &ptf_addr_len);
		if (recv_size == -1) {
			if (errno == EWOULDBLOCK || errno == EINTR) {
				continue; // timeout or interrupted system call
			}
			else {
				COUT_THIS("[switch os] Error of recvfrom: errno = " << errno)
				exit(-1);
			}
		}
		INVARIANT(recv_size > 0);

		tmp_type = *((int *)buf);
		if (tmp_type == switchos_poptype) {
			// send switchos_freeidx to ptf framework
			memset(buf, 0, MAX_BUFSIZE);
			memcpy(buf, (void *)&switchos_freeidx, sizeof(uint32_t));
			sendto(switchos_paramserver_udpsock, buf, sizeof(uint32_t), 0, (struct sockaddr *)ptf_addr, ptf_addr_len);
		}
		else if (tmp_type == switchos_evicttype) {
			// TODO: send switchos_evictidx to ptf framework
		}
		else {
			printf("[switch os] invalid requset type from ptf to paramserver: %d\n", tmp_type);
			exit(-1);
		}

	}
}

void *run_switchos_popworker(void *param) {
	while (running) {
		char buf[MAX_BUFSIZE];
		if (switchos_tail_for_pop != switchos_head_for_pop) {
			cache_pop_t *tmp_cache_pop_ptr = switchos_cache_pop_ptrs[switchos_tail_for_pop];
			INVARIANT(tmp_cache_pop_ptr != NULL);

			// assign switchos_freeidx for new record 
			if (switchos_cached_keyarray_empty_index < switch_kv_bucket_num) { // With free idx
				switchos_freeidx = switchos_cached_keyarray_empty_index;
				switchos_cached_keyarray_empty_index += 1;
			}
			else { // Without free idx
				// TODO: set switchos_freeidx after cache eviction for old record
			}

			// TODO: cache population for new record
			system("bash tofino/setvalid0.sh"); // set valid=0 for atomicity

			// free CACHE_POP
			delete tmp_cache_pop_ptr;
			tmp_cache_pop_ptr = NULL;
		}
	}
}

void close_switchos() {
	if (switchos_cache_pop_ptrs != NULL) {
		for (size_t i = 0; i < MQ_SIZE; i++) {
			if (switchos_cache_pop_ptrs[i] != NULL) {
				delete switchos_cache_pop_ptrs[i];
				switchos_cache_pop_ptrs[i] = NULL;
			}
		}
		delete [] switchos_cache_pop_ptrs;
		switchos_cache_pop_ptrs = NULL;
	}
	if (switchos_cached_keyarray != NULL) {
		delete [] switchos_cached_keyarray;
		switchos_cached_keyarray = NULL;
	}
}
