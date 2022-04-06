#ifndef CONTROLLER_IMPL_H
#define CONTROLLER_IMPL_H

// Per-server popclient <-> one popserver in controller
int * volatile controller_popserver_tcpsock_list = NULL;
std::set<index_key_t> * volatile controller_cached_keyset_list = NULL;

// Per-server evictserver <-> one evictclient in controller
int * volatile controller_evictclient_tcpsock_list = NULL;

void prepare_controller();
void *run_controller_popserver(void *param);
void close_controller();

void prepare_controller() {
	// Prepare for cache population
	controller_popserver_tcpsock_list = new int[fg_n];
	controller_cached_keyset_list = new std::set<index_key_t>[fg_n];
	for (size_t i = 0; i < fg_n; i++) {
		// Set popserver socket
		controller_popserver_tcpsock_list[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (controller_popserver_tcpsock_list[i] == -1) {
			printf("[Controller] Fail to create tcp socket of popserver %ld: errno: %d!\n", i, errno);
			exit(-1);
		// reuse the occupied port for the last created socket instead of being crashed
		const int trueFlag = 1;
		if (setsockopt(controller_popserver_tcpsock_list[i], SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
			printf("[Controller] Fail to setsockopt of of popserver %ld: errno: %d!\n", i, errno);
			exit(-1);
		}
		// Disable udp/tcp check
		int disable = 1;
		if (setsockopt(controller_popserver_tcpsock_list[i], SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
			COUT_N_EXIT("[Controller] Disable tcp checksum failed");
		}
		// Set timeout for recvfrom/accept of udp/tcp
		/*struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec =  0;
		int res = setsockopt(controller_popserver_tcpsock_list[i], SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		INVARIANT(res >= 0);*/
		// Set listen address
		sockaddr_in listen_addr;
		memset(&listen_addr, 0, sizeof(listen_addr));
		listen_addr.sin_family = AF_INET;
		listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		listen_addr.sin_port = htons(controller_popserver_port_start+i);
		if ((bind(controller_popserver_tcpsock_list[i], (struct sockaddr*)&listen_addr, sizeof(listen_addr))) != 0) {
			printf("[Controller] Fail to bind socket on %s:%hu of popserver %ld, errno: %d!\n", controller_ip, controller_popserver_port_start+i, i, errno);
			exit(-1);
		}
		if ((listen(controller_popserver_tcpsock_list[i], fg_n)) != 0) { // MAX_PENDING_CONNECTION = server num
			printf("[Controller] Fail to listen on %s:%hu of popserver %ld, errno: %d!\n", controller_ip, controller_popserver_port_start+i, i, errno);
			exit(-1);
		}

		controller_cached_keyset_list[i].clear();
	}
}

void *run_controller_popserver(void *param) {
	tcpserver_param_t param = *((tcpserver_param_t *)param);
	uint8_t thread_id = param.thread_id;

	// Not used
	struct sockaddr_in server_addr;
	unsigned int server_addr_len;

	int connfd = accept(controller_popserver_tcpsock_list[thread_id], NULL, NULL);
	if (connfd == -1) {
		if (errno == EWOULDBLOCK || errno == EINTR) {
			continue; // timeout or interrupted system call
		}
		else {
			COUT_N_EXIT("[Controller] Error of accept: errno = " << std::string(strerror(errno)));
		}
	}

	// Process CACHE_POP packet <optype, key, vallen, value, stat, seq>
	char buf[MAX_BUFSIZE];
	int cur_recv_bytes = -1;
	int8_t optype = -1;
	int32_t vallen = -1;
	int arrive_seq_bytes = -1;
	bool is_cached_before = false;
	index_key_t tmpkey(0, 0, 0, 0);
	const int arrive_optype_bytes = sizeof(int8_t);
	const int arrive_vallen_bytes = arrive_optype_bytes + sizeof(key_t) + sizeof(int32_t);
	while (true) {
		int tmpsize = recv(connfd, buf + cur_recv_bytes, MAX_BUFSIZE - cur_recv_bytes, 0);
		if (tmpsize < 0) {
			// Errno 32 means broken pipe, i.e., server.popclient is closed
			if (errno == 32) {
				printf("[Controller] remote server.popclient %ld is closed", thread_id);
				break;
			}
			else {
				COUT_N_EXIT("[Controller] popserver recv fails: " << tmpsize << " errno = " << std::string(strerror(errno)));
			}
		}
		cur_recv_bytes += tmpsize;
		if (cur_recv_bytes >= MAX_BUFSIZE) {
			printf("[Controller] Overflow: cur received bytes (%d), maxbufsize (%d)\n", cur_recv_bytes, MAX_BUFSIZE);
            exit(-1);
		}

		// Get optype
		if (optype == -1 && cur_recv_bytes >= arrive_optype_bytes) {
			optype = *((int8_t *)buf);
			INVARIANT(packet_type_t(optype) == packet_type_t::CACHE_POP);
		}

		// Get vallen
		if (optype != -1 && vallen == -1 && cur_recv_bytes >= arrive_vallen_bytes) {
			// Add key into cached keyset
			tmpkey.deserialize(buf + arrive_optype_bytes, cur_recv_bytes - arrive_optype_bytes);
			is_cached_before = (controller_cached_keyset_list[thread_id].find(tmpkey) != controller_cached_keyset_list[thread_id].end());
			if (!is_cached_before) {
				controller_cached_keyset_list[thread_id].insert(tmpkey);
			}

			vallen = *((int32_t *)(buf + arrive_vallen_bytes - sizeof(int32_t)));
			INVARIANT(vallen >= 0);
			int padding_size = int(val_t::get_padding_size(vallen)); // padding for value <= 128B
			arrive_seq_bytes = arrive_vallen_bytes + vallen + padding_size + sizeof(bool) + sizeof(int32_t);
		}

		// Get one complete CACHE_POP
		if (optype != -1 && vallen != -1 && cur_recv_bytes >= arrive_seq_bytes) {
			if (!is_cached_before) {
				// TODO: Send bytes of CACHE_POP to switch OS
			}

			// Move remaining bytes and reset metadata
			if (cur_recv_bytes > arrive_seq_bytes) {
				memcpy(buf, buf + arrive_seq_bytes, cur_recv_bytes - arrive_seq_bytes);
				cur_recv_bytes = cur_recv_bytes - arrive_seq_byets;
				optype = -1;
				vallen = -1;
				arrive_seq_bytes = -1;
				is_cached_before = false;
			}
			else {
				cur_recv_bytes = 0;
				optype = -1;
				vallen = -1;
				arrive_seq_bytes = -1;
				is_cached_before = false;
			}
		}
	}

	close(connfd);
	close(controller_popserver_tcpsock_list[thread_id]);
	pthread_exit(nullptr);
}

void close_controller() {
	if (controller_popserver_tcpsock_list != NULL) {
		delete [] controller_popserver_tcpsock_list;
		controller_popserver_tcpsock_list = NULL;
	}
	if (controller_cached_keyset_list != NULL) {
		delete [] controller_cached_keyset_list;
		controller_cached_keyset_list = NULL;
	}
	if (controller_evictclient_tcpsock_list != NULL) {
		delete [] controller_evictclient_tcpsock_list;
		controller_evictclient_tcpsock_list = NULL;
	}
}

#endif
