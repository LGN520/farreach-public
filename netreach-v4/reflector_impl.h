#ifndef REFLECTOR_H
#define REFLECTOR_H

// shared by reflector.udpserver and reflector.dpdkserver, but no contention
bool volatile reflector_with_switchos_popworker_addr = false;
struct sockaddr_in volatile reflector_switchos_popworker_addr;
unsigned int volatile reflector_switchos_popworker_addr_len = sizeof(struct sockaddr);

// switchos.popworker -> (udp channel) -> one reflector.udpserver -> data plane
int volatile reflector_udpserver_udpsock = -1;

// data plane -> receiver -> (message) -> reflector.dpdkserver -> switchos.popworker
struct rte_mbuf** volatile reflector_pkts_for_popack; // pkts from receiver to reflector.dpdkserver
volatile uint32_t reflector_head_for_popack;
volatile uint32_t reflector_tail_for_popack;
int volatile reflector_dpdkserver_udpsock = -1;

void prepare_reflector();
void *run_reflector_udpserver(void *param);
void *run_reflector_dpdkserver(void *param);
void close_reflector();

void prepare_reflector() {
	// Set paramserver socket
	reflector_udpserver_udpsock = socket(AF_INET, SOCK_DGRAM, 0);
	if (reflector_udpserver_udpsock == -1) {
		printf("[reflector] Fail to create udp socket of paramserver: errno: %d!\n", errno);
		exit(-1);
	}
	// reuse the occupied port for the last created socket instead of being crashed
	const int trueFlag = 1;
	if (setsockopt(reflector_udpserver_udpsock, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
		printf("[reflector] Fail to setsockopt of paramserver: errno: %d!\n", errno);
		exit(-1);
	}
	// Disable udp/tcp check
	int disable = 1;
	if (setsockopt(reflector_udpserver_udpsock, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		COUT_N_EXIT("[reflector] Disable udp checksum failed");
	}
	// Set timeout for recvfrom/accept of udp/tcp
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec =  0;
	int res = setsockopt(reflector_udpserver_udpsock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	INVARIANT(res >= 0);
	// Set listen address
	//sockaddr_in listen_addr;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(reflector_port);
	if ((bind(reflector_udpserver_udpsock, (struct sockaddr*)&listen_addr, sizeof(listen_addr))) != 0) {
		printf("[reflector] Fail to bind socket on port %hu for udpserver, errno: %d!\n", reflector_port, errno);
		exit(-1);
	}

	// From receiver to reflector.dpdkserver
	reflector_pkts_for_popack = new struct rte_mbuf*[MQ_SIZE];
	for (size_t j = 0; j < MQ_SIZE; j++) {
		reflector_pkts_for_popack[i][j] = nullptr;
	}
	reflector_head_for_popack = 0;
	reflector_tail_for_popack = 0;
	reflector_dpdkserver_udpsock = socket(AF_INET, SOCK_DGRAM, 0);
}

void *run_reflector_udpserver(void *param) {
	// DPDK
	uint16_t burst_size = 256;
	struct rte_mbuf *sent_pkts[burst_size];
	uint16_t sent_pkt_idx = 0;
	int res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
	INVARIANT(res == 0);

	char buf[MAX_BUFSIZE];

	while (!running) {
	}

	while (running) {
		struct sockaddr_in tmp_switchos_popworker_addr;
		unsigned int tmp_switchos_popworker_addr_len = sizeof(struct sockaddr);
		if (!reflector_with_switchos_popworker_addr) {
			recv_size = recvfrom(reflector_udpserver_udpsock, buf, MAX_BUFSIZE, 0, (struct sockaddr *)&tmp_switchos_popworker_addr, &tmp_switchos_popworker_addr_len);
		}
		else {
			recv_size = recvfrom(reflector_udpserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL);
		}

		if (recv_size == -1) {
			if (errno == EWOULDBLOCK || errno == EINTR) {
				continue; // timeout or interrupted system call
			}
			else {
				COUT_THIS("[reflector] Error of recvfrom: errno = " << errno)
				exit(-1);
			}
		}
		INVARIANT(recv_size > 0);

		if (!reflector_with_switchos_popworker_addr) {
			reflector_switchos_popworker_addr = tmp_switchos_popworker_addr;
			reflector_switchos_popworker_addr_len = tmp_switchos_popworker_addr_len;
			reflector_with_switchos_popworker_addr = true;
		}

		// send CACHE_POP_INSWITCH to data plane
		//cache_pop_inswitch_t tmp_cache_pop_inswitch_pkt(buf, recv_size); // TODO: check whether buf is CACHE_POP_INSWITCH
		encode_mbuf(sent_pkts[sent_pkt_idx], server_macaddr, client_macaddr, server_ip, client_ip, server_port_start, client_port_start, buf, recv_size);
		res = rte_eth_tx_burst(0, server_num, &sent_pkt, 1); // through dpdk queue for reflector
		sent_pkt_idx++;

		if (sent_pkt_idx >= burst_size) {
			sent_pkt_idx = 0;
			res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
			INVARIANT(res == 0);
		}
	}

	if (sent_pkt_idx < burst_size) {
		for (size_t i = sent_pkt_idx; i != burst_size; i++) {
			rte_pktmbuf_free(sent_pkts[i]);
		}
	}

	close(reflector_udpserver_udpsock);
	pthread_exit(nullptr);
}

void *run_reflector_dpdkserver() {
	char buf[MAX_BUFSIZE];
	uint32_t recv_size = 0;

	while (!running) {
	}

	while(running) {
		if (reflector_tail_for_popack != reflector_head_for_popack) {
			INVARIANT(reflector_with_switchos_popworker_addr);
			recv_size = get_payload(reflector_pkts_for_popack[reflector_tail_for_popack], buf);
			// send CACHE_POP_INSWITCH_ACK to switchos.popworker
			int tmpsize = sendto(reflector_dpdkserver_udpsock, buf, recv_size, 0, (struct sockaddr *)&reflector_switchos_popworker_addr, reflector_switchos_popworker_addr_len);
			if (tmpsize < 0) {
				printf("[reflector] fail to send to switchos.popworker, errno: %d\n", errno);
				exit(-1);
			}

			rte_pktmbuf_free(reflector_pkts_for_popack[reflector_tail_for_popack]);
			reflector_pkts_for_popack[reflector_tail_for_popack] = nullptr;
			reflector_tail_for_popack = (reflector_tail_for_popack + 1) % MQ_SIZE;
		}
	}
	pthread_exit(nullptr);
}

void close_reflector() {
	for (size_t i = 0; i < MQ_SIZE; i++) {
		if (reflector_pkts_for_popack[i] != nullptr) {
			rte_pktmbuf_free(reflector_pkts_for_popack[i]);
			reflector_pkts_for_popack[i] = nullptr;
		}
	}
}

#endif
