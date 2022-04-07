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

// Parameters
short switchos_popserver_port = 0;

// Cache population

// controller.popclient <-> switchos.popserver
int volatile switchos_popserver_tcpsock = -1;
// message queue between switchos.popserver and switchos.popworker
cache_pop_t ** volatile switchos_cache_pop_ptrs = NULL;
uint32_t switchos_head_for_pop;
uint32_t switchos_tail_for_pop;

inline void parse_ini(const char *config_file);
void prepare_switchos();
void *run_switchos_popserver(void *param);
void *run_switchos_popworker(void *param);
void close_switchis();

inline void parse_ini(const char* config_file) {
	IniparserWrapper ini;
	ini.load(config_file);

	switchos_popserver_port = ini.get_switchos_popserver_port();
	
	COUT_VAR(switchos_popserver_port);
}

void prepare_switchos() {
	// Set popserver socket
	switchos_popserver_tcpsock = socket(AF_INET, SOCK_STREAM, 0);
	if (switchos_popserver_tcpsock == -1) {
		printf("[Switch os] Fail to create tcp socket of popserver: errno: %d!\n", errno);
		exit(-1);
	// reuse the occupied port for the last created socket instead of being crashed
	const int trueFlag = 1;
	if (setsockopt(switchos_popserver_tcpsock, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
		printf("[Switch os] Fail to setsockopt of popserver: errno: %d!\n", errno);
		exit(-1);
	}
	// Disable udp/tcp check
	int disable = 1;
	if (setsockopt(switchos_popserver_tcpsock, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		COUT_N_EXIT("[Switch os] Disable tcp checksum failed");
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
		printf("[Switch os] Fail to bind socket on port %hu for popserver, errno: %d!\n", switchos_popserver_port, errno);
		exit(-1);
	}
	if ((listen(switchos_popserver_tcpsock, 1)) != 0) { // MAX_PENDING_CONNECTION = 1
		printf("[Switch os] Fail to listen on port %hu for popserver, errno: %d!\n", switchos_popserver_port, errno);
		exit(-1);
	}

	switchos_cache_pop_ptrs = new cache_pop_t*[MQ_SIZE];
	switchos_head_for_pop = 0;
	switchos_tail_for_pop = 0;
}

// TODO
void *run_switchos_popserver(void *param);
void *run_switchos_popworker(void *param);

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
}
