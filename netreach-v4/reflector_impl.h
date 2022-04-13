#ifndef REFLECTOR_H
#define REFLECTOR_H

// shared by reflector.popserver and reflector.dpdkserver, but no contention
bool volatile reflector_with_switchos_popworker_addr = false;
struct sockaddr_in volatile reflector_switchos_popworker_addr;
unsigned int volatile reflector_switchos_popworker_addr_len = sizeof(struct sockaddr);

// switchos.popworker -> (udp channel) -> one reflector.popserver -> data plane
int volatile reflector_popserver_udpsock = -1;

// data plane -> receiver -> (message) -> reflector.dpdkserver -> switchos.popworker
struct rte_mbuf** volatile reflector_pkts_for_popack; // pkts from receiver to reflector.dpdkserver
volatile uint32_t reflector_head_for_popack;
volatile uint32_t reflector_tail_for_popack;
int volatile reflector_dpdkserver_popclient_udpsock = -1;

void prepare_reflector();
void *run_reflector_popserver(void *param);
void *run_reflector_dpdkserver(void *param);
void close_reflector();

void prepare_reflector() {
	// prepare popserver socket
	prepare_udpserver(reflector_popserver_udpsock, true, reflector_popserver_port, "reflector.popserver");

	// From receiver to reflector.dpdkserver
	reflector_pkts_for_popack = new struct rte_mbuf*[MQ_SIZE];
	for (size_t j = 0; j < MQ_SIZE; j++) {
		reflector_pkts_for_popack[i][j] = nullptr;
	}
	reflector_head_for_popack = 0;
	reflector_tail_for_popack = 0;
	create_udpsock(reflector_dpdkserver_popclient_udpsock, "reflector.dpdkserver.popclient");
}

void *run_reflector_popserver(void *param) {
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
		int recvsize = 0;
		if (!reflector_with_switchos_popworker_addr) {
			udprecvfrom(reflector_popserver_udpsock, buf, MAX_BUFSIZE, 0, (struct sockaddr *)&tmp_switchos_popworker_addr, &tmp_switchos_popworker_addr_len, recvsize, "reflector.popserver");
		}
		else {
			udprecvfrom(reflector_popserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "reflector.popserver");
		}

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

	close(reflector_popserver_udpsock);
	pthread_exit(nullptr);
}

void *run_reflector_dpdkserver() {
	char buf[MAX_BUFSIZE];
	uint32_t recvsize = 0;

	while (!running) {
	}

	while(running) {
		if (reflector_tail_for_popack != reflector_head_for_popack) {
			INVARIANT(reflector_with_switchos_popworker_addr);
			recvsize = get_payload(reflector_pkts_for_popack[reflector_tail_for_popack], buf);
			// send CACHE_POP_INSWITCH_ACK to switchos.popworker
			int sendsize = 0;
			udpsendto(reflector_dpdkserver_popclient_udpsock, buf, recvsize, 0, (struct sockaddr *)&reflector_switchos_popworker_addr, reflector_switchos_popworker_addr_len, sendsize, "reflector.dpdkserver.popclient");

			rte_pktmbuf_free(reflector_pkts_for_popack[reflector_tail_for_popack]);
			reflector_pkts_for_popack[reflector_tail_for_popack] = nullptr;
			reflector_tail_for_popack = (reflector_tail_for_popack + 1) % MQ_SIZE;
		}
	}

	close(reflector_dpdkserver_popclient_udpsock);
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
