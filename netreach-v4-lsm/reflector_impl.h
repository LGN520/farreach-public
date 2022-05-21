#ifndef REFLECTOR_IMPL_H
#define REFLECTOR_IMPL_H

// shared by reflector.popserver and reflector.dpdkserver, but no contention
bool volatile reflector_with_switchos_popworker_addr = false;
struct sockaddr_in reflector_switchos_popworker_addr;
unsigned int reflector_switchos_popworker_addr_len = sizeof(struct sockaddr);

// switchos.popworker -> (udp channel) -> one reflector.popserver -> data plane
int reflector_popserver_udpsock = -1;

// data plane -> receiver -> (message) -> reflector.dpdkserver -> switchos.popworker
// for cache population ack and special cases of snapshot
/*struct rte_mbuf* volatile * reflector_pkts_for_popack_snapshot; // pkts from receiver to reflector.dpdkserver
uint32_t volatile reflector_head_for_popack_snapshot;
uint32_t volatile reflector_tail_for_popack_snapshot;*/
int reflector_dpdkserver_popclient_udpsock = -1;

// reflector.dpdkserver -> switchos.specialcaseserver
int reflector_dpdkserver_specialcaseclient_udpsock = -1;

void prepare_reflector();
void *run_reflector_popserver(void *param);
void *run_reflector_dpdkserver(void *param);
void close_reflector();

void prepare_reflector() {
	printf("[reflector] prepare start\n");

	// prepare popserver socket
	prepare_udpserver(reflector_popserver_udpsock, true, reflector_popserver_port, "reflector.popserver");

	// From receiver to reflector.dpdkserver
	/*reflector_pkts_for_popack_snapshot = new struct rte_mbuf*[MQ_SIZE];
	for (size_t i = 0; i < MQ_SIZE; i++) {
		reflector_pkts_for_popack_snapshot[i] = nullptr;
	}
	reflector_head_for_popack_snapshot = 0;
	reflector_tail_for_popack_snapshot = 0;*/
	create_udpsock(reflector_dpdkserver_popclient_udpsock, "reflector.dpdkserver.popclient");

	create_udpsock(reflector_dpdkserver_specialcaseclient_udpsock, "reflector.dpdkserver.specialcaseclient");

	memory_fence();

	printf("[reflector] prepare end\n");
}

void *run_reflector_popserver(void *param) {
	// DPDK
	struct rte_mempool *tx_mbufpool = NULL;
	dpdk_queue_setup(0, server_num, &tx_mbufpool);
	generate_udp_fdir_rule(0, server_num, reflector_port);
	INVARIANT(tx_mbufpool != NULL);
	uint16_t burst_size = 256;
	struct rte_mbuf *sent_pkts[burst_size];
	uint16_t sent_pkt_idx = 0;
	int res = rte_pktmbuf_alloc_bulk(tx_mbufpool, sent_pkts, burst_size);
	INVARIANT(res == 0);

	char buf[MAX_BUFSIZE];
	printf("[reflector.popserver] ready\n");
	transaction_ready_threads++;

	while (!transaction_running) {}

	while (transaction_running) {
		int recvsize = -1;
		bool is_timeout = true;
		if (!reflector_with_switchos_popworker_addr) {
			is_timeout = udprecvfrom(reflector_popserver_udpsock, buf, MAX_BUFSIZE, 0, (struct sockaddr *)&reflector_switchos_popworker_addr, (socklen_t *)&reflector_switchos_popworker_addr_len, recvsize, "reflector.popserver");
			memory_fence();
			if (!is_timeout) {
				reflector_with_switchos_popworker_addr = true;
			}
		}
		else {
			is_timeout = udprecvfrom(reflector_popserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "reflector.popserver");
		}

		if (is_timeout) {
			continue;
		}
		INVARIANT(recvsize >= 0);

		// send CACHE_POP_INSWITCH to data plane
		//cache_pop_inswitch_t tmp_cache_pop_inswitch_pkt(buf, recv_size); // TODO: check whether buf is CACHE_POP_INSWITCH
		encode_mbuf(sent_pkts[sent_pkt_idx], server_macaddr, client_macaddr, server_ip, client_ip, server_port_start, client_port_start, buf, recvsize);
		res = rte_eth_tx_burst(0, server_num, &sent_pkts[sent_pkt_idx], 1); // through dpdk queue for reflector
		sent_pkt_idx++;

		if (sent_pkt_idx >= burst_size) {
			sent_pkt_idx = 0;
			res = rte_pktmbuf_alloc_bulk(tx_mbufpool, sent_pkts, burst_size);
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

void *run_reflector_dpdkserver(void *param) {
	char buf[MAX_BUFSIZE];
	uint32_t recvsize = 0;
	struct rte_mbuf *rx_pkts[RX_BURST_SIZE];
	memset(rx_pkts, 0, sizeof(struct rte_mbuf *) * RX_BURST_SIZE);
	uint16_t n_rx = 0;

	struct sockaddr_in reflector_switchos_specialcaseserver_addr;
	set_sockaddr(reflector_switchos_specialcaseserver_addr, inet_addr(switchos_ip), switchos_specialcaseserver_port);
	unsigned int reflector_switchos_specialcaseserver_addr_len = sizeof(struct sockaddr);

	printf("[reflector.dpdkserver] ready\n");
	transaction_ready_threads++;

	while (!transaction_running) {}

	while (transaction_running) {
		//if (reflector_tail_for_popack_snapshot != reflector_head_for_popack_snapshot) { // from receiver
		
		// wait for at most RX_BURST_SIZE pkts instead of a single packet due to duplicate notifications issued by data plane for pkt loss
		n_rx = receive_pkts(0, server_num, rx_pkts, RX_BURST_SIZE, reflector_port);

		if (n_rx == 0) {
			continue;
		}

		for (uint16_t i = 0; i < n_rx; i++) { // n_rx != 0
			//INVARIANT(reflector_pkts_for_popack_snapshot[reflector_tail_for_popack_snapshot] != NULL);
			//recvsize = get_payload(reflector_pkts_for_popack_snapshot[reflector_tail_for_popack_snapshot], buf);
			recvsize = get_payload(rx_pkts[i], buf);

			packet_type_t pkt_type = get_packet_type(buf, recvsize);
			switch (pkt_type) {
				case packet_type_t::CACHE_POP_INSWITCH_ACK:
					{
						INVARIANT(reflector_with_switchos_popworker_addr == true);
						// send CACHE_POP_INSWITCH_ACK to switchos.popworker
						// NOTE: not use popserver.popclient due to duplicate packets for packet loss issued by switch
						udpsendto(reflector_dpdkserver_popclient_udpsock, buf, recvsize, 0, (struct sockaddr *)&reflector_switchos_popworker_addr, reflector_switchos_popworker_addr_len, "reflector.dpdkserver.popclient");
						break;
					}
				case packet_type_t::GETRES_LATEST_SEQ_INSWITCH_CASE1:
				case packet_type_t::GETRES_DELETED_SEQ_INSWITCH_CASE1:
				case packet_type_t::PUTREQ_SEQ_INSWITCH_CASE1:
				case packet_type_t::DELREQ_SEQ_INSWITCH_CASE1:
					{
						// send CASE1 to switchos.specialcaseserver
						udpsendto(reflector_dpdkserver_specialcaseclient_udpsock, buf, recvsize, 0, (struct sockaddr *)&reflector_switchos_specialcaseserver_addr, reflector_switchos_specialcaseserver_addr_len, "reflector.dpdkserver.specialcaseclient");
						break;
					}
				default:
					{
						printf("[reflector.dpdkserver] invalid packet type: %d\n", int(pkt_type));
						break;
					}
			}

			//rte_pktmbuf_free(reflector_pkts_for_popack_snapshot[reflector_tail_for_popack_snapshot]);
			//reflector_pkts_for_popack_snapshot[reflector_tail_for_popack_snapshot] = NULL;
			//reflector_tail_for_popack_snapshot = (reflector_tail_for_popack_snapshot + 1) % MQ_SIZE;
			rte_pktmbuf_free(rx_pkts[i]);
			rx_pkts[i] = NULL;
		}
	}
	
	//} // wait for receiver

	close(reflector_dpdkserver_popclient_udpsock);
	pthread_exit(nullptr);
}

void close_reflector() {
	/*for (size_t i = 0; i < MQ_SIZE; i++) {
		if (reflector_pkts_for_popack_snapshot[i] != nullptr) {
			rte_pktmbuf_free(reflector_pkts_for_popack_snapshot[i]);
			reflector_pkts_for_popack_snapshot[i] = nullptr;
		}
	}*/
}

#endif
