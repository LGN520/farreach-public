#ifndef REFLECTOR_H
#define REFLECTOR_H

// switchos.popworker -> (udp channel) -> one reflector.udpserver -> data plane
// data plane -> receiver -> (message) -> reflector.dpdkserver -> switchos.popworker
int volatile reflector_udpsock = -1;

void prepare_reflector();
void *run_reflector(void *param);
void close_reflector();

void prepare_reflector() {
	// Set paramserver socket
	reflector_udpsock = socket(AF_INET, SOCK_DGRAM, 0);
	if (reflector_udpsock == -1) {
		printf("[reflector] Fail to create udp socket of paramserver: errno: %d!\n", errno);
		exit(-1);
	}
	// reuse the occupied port for the last created socket instead of being crashed
	const int trueFlag = 1;
	if (setsockopt(reflector_udpsock, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
		printf("[reflector] Fail to setsockopt of paramserver: errno: %d!\n", errno);
		exit(-1);
	}
	// Disable udp/tcp check
	int disable = 1;
	if (setsockopt(reflector_udpsock, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		COUT_N_EXIT("[reflector] Disable udp checksum failed");
	}
	// Set timeout for recvfrom/accept of udp/tcp
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec =  0;
	int res = setsockopt(reflector_udpsock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	INVARIANT(res >= 0);
	// Set listen address
	//sockaddr_in listen_addr;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(reflector_port);
	if ((bind(reflector_udpsock, (struct sockaddr*)&listen_addr, sizeof(listen_addr))) != 0) {
		printf("[reflector] Fail to bind socket on port %hu for udpserver, errno: %d!\n", reflector_port, errno);
		exit(-1);
	}
}

void *run_reflector(void *param) {
	// DPDL
	uint16_t burst_size = 256;
	struct rte_mbuf *sent_pkts[burst_size];
	uint16_t sent_pkt_idx = 0;
	int res = rte_pktmbuf_alloc_bulk(mbuf_pool, sent_pkts, burst_size);
	INVARIANT(res == 0);

	char buf[MAX_BUFSIZE];
	while (running) {
		struct sockaddr_in switchos_popworker_addr;
		unsigned int switchos_popworker_addr_len = sizeof(struct sockaddr);

		recv_size = recvfrom(reflector_udpsock, buf, MAX_BUFSIZE, 0, (struct sockaddr *)&switchos_popworker_addr, &switchos_popworker_addr_len);
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

		// send CACHE_POP_INSWITCH to data plane
		//cache_pop_inswitch_t tmp_cache_pop_inswitch_pkt(buf, recv_size); // TODO: check whether buf is CACHE_POP_INSWITCH
		encode_mbuf(sent_pkts[sent_pkt_idx], server_macaddr, client_macaddr, server_ip, client_ip, server_port_start, client_port_start, buf, recv_size);
		res = rte_eth_tx_burst(0, server_num, &sent_pkt, 1); // through dpdk queue for reflector
		sent_pkt_idx++;

		// wait for CACHE_POP_INSWITCH_ACK from data plane
		// TODO: use a new dpdkserver to send ACKs to switchos.popworker
		// TODO: receiver insert each ACK into a message queue
		// TODO: reflector gets all ACKs from message queue; use while(running) to avoid deadlock
		// TODO: reflector sends ACK to switchos.popworker, and clear the message

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

	close(reflector_udpsock);
	pthread_exit(nullptr);
}

#endif
