#ifndef SERVER_IMPL_H
#define SERVER_IMPL_H

// Per-server popclient <-> one popserver in controller
int * volatile server_popclient_tcpsock_list = NULL;
std::set<index_key_t> * volatile server_cached_keyset_list = NULL;

// Per-server evictserver <-> one evictclient in controller
int * volatile server_evictserver_tcpsock_list = NULL;
cache_evict_t ** volatile cache_evicts_list = NULL;
uint32_t volatile heads_for_evict;
uint32_t volatile tails_for_evict;

void prepare_server();
void close_server();

void prepare_server() {
	// Prepare for cache population
	server_popclient_tcpsock_list = new int[fg_n];
	server_cached_keyset_list = new std::set<index_key_t>[fg_n];
	for (size_t i = 0; i < fg_n; i++) {
		server_popclient_tcpsock_list[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (server_popclient_tcpsock_list[i] == -1) {
			printf("Fail to create tcp socket for server.popclient %ld: errno: %d!\n", i, errno);
			exit(-1);
		}

		server_cached_keyset_list[i].clear();
	}
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
}

#endif
