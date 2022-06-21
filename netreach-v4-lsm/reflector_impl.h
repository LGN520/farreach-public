#ifndef REFLECTOR_IMPL_H
#define REFLECTOR_IMPL_H

int reflector_worker_udpsock = -1;

// shared by reflector.popserver and reflector.worker, but no contention
bool volatile reflector_with_switchos_popworker_addr = false;
struct sockaddr_in reflector_switchos_popworker_addr;
unsigned int reflector_switchos_popworker_addr_len = sizeof(struct sockaddr);

// switchos.popworker -> (udp channel) -> one reflector.popserver -> data plane
int reflector_popserver_udpsock = -1;

// data plane -> receiver -> (message) -> reflector.worker -> switchos.popworker
// for cache population ack and special cases of snapshot
/*struct rte_mbuf* volatile * reflector_pkts_for_popack_snapshot; // pkts from receiver to reflector.worker
uint32_t volatile reflector_head_for_popack_snapshot;
uint32_t volatile reflector_tail_for_popack_snapshot;*/
int reflector_worker_popclient_udpsock = -1;

// reflector.worker -> switchos.specialcaseserver
int reflector_worker_specialcaseclient_udpsock = -1;

void prepare_reflector();
void *run_reflector_popserver(void *param);
void *run_reflector_worker(void *param);
void close_reflector();

void prepare_reflector() {
	printf("[reflector] prepare start\n");

	// prepare worker socket
	prepare_udpserver(reflector_worker_udpsock, true, reflector_port, "reflector.worker");

	// prepare popserver socket
	prepare_udpserver(reflector_popserver_udpsock, true, reflector_popserver_port, "reflector.popserver");

	// From receiver to reflector.worker
	/*reflector_pkts_for_popack_snapshot = new struct rte_mbuf*[MQ_SIZE];
	for (size_t i = 0; i < MQ_SIZE; i++) {
		reflector_pkts_for_popack_snapshot[i] = nullptr;
	}
	reflector_head_for_popack_snapshot = 0;
	reflector_tail_for_popack_snapshot = 0;*/
	create_udpsock(reflector_worker_popclient_udpsock, false, "reflector.worker.popclient");

	create_udpsock(reflector_worker_specialcaseclient_udpsock, false, "reflector.worker.specialcaseclient");

	memory_fence();

	printf("[reflector] prepare end\n");
}

// forward pkt from switchos to switch
void *run_reflector_popserver(void *param) {
	// client address (switch will not hide CACHE_POP_INSWITCH from clients)
	struct sockaddr_in client_addr;
	set_sockaddr(client_addr, inet_addr(client_ip), client_port_start);
	socklen_t client_addrlen = sizeof(struct sockaddr_in);

	char buf[MAX_BUFSIZE];
	printf("[reflector.popserver] ready\n");
	transaction_ready_threads++;

	while (!transaction_running) {}

	while (transaction_running) {
		int recvsize = -1;
		bool is_timeout = true;
		if (!reflector_with_switchos_popworker_addr) {
			is_timeout = udprecvfrom(reflector_popserver_udpsock, buf, MAX_BUFSIZE, 0, &reflector_switchos_popworker_addr, (socklen_t *)&reflector_switchos_popworker_addr_len, recvsize, "reflector.popserver");
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

		// send CACHE_POP_INSWITCH or CACHE_EVICT_LOADFREQ_INSWITCH to data plane
		udpsendto(reflector_popserver_udpsock, buf, recvsize, 0, &client_addr, client_addrlen, "reflector.popserver");
	}

	close(reflector_popserver_udpsock);
	pthread_exit(nullptr);
}

// forward pkt from switch to switchos
void *run_reflector_worker(void *param) {
	char buf[MAX_BUFSIZE];
	int recvsize = 0;

	struct sockaddr_in reflector_switchos_specialcaseserver_addr;
	set_sockaddr(reflector_switchos_specialcaseserver_addr, inet_addr(switchos_ip), switchos_specialcaseserver_port);
	unsigned int reflector_switchos_specialcaseserver_addr_len = sizeof(struct sockaddr);

	printf("[reflector.worker] ready\n");
	transaction_ready_threads++;

	while (!transaction_running) {}

	while (transaction_running) {

		bool is_timeout = udprecvfrom(reflector_worker_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "reflector.worker");
		if (is_timeout) {
			continue;
		}
		INVARIANT(recvsize > 0);

		packet_type_t pkt_type = get_packet_type(buf, recvsize);
		switch (pkt_type) {
			case packet_type_t::CACHE_POP_INSWITCH_ACK:
			case packet_type_t::CACHE_EVICT_LOADFREQ_INSWITCH_ACK:
			case packet_type_t::CACHE_EVICT_LOADDATA_INSWITCH_ACK:
				{
					INVARIANT(reflector_with_switchos_popworker_addr == true);
					// send CACHE_POP_INSWITCH_ACK to switchos.popworker
					// NOTE: not use popserver.popclient due to duplicate packets for packet loss issued by switch
					udpsendto(reflector_worker_popclient_udpsock, buf, recvsize, 0, &reflector_switchos_popworker_addr, reflector_switchos_popworker_addr_len, "reflector.worker.popclient");
					break;
				}
			case packet_type_t::GETRES_LATEST_SEQ_INSWITCH_CASE1:
			case packet_type_t::GETRES_DELETED_SEQ_INSWITCH_CASE1:
			case packet_type_t::PUTREQ_SEQ_INSWITCH_CASE1:
			case packet_type_t::DELREQ_SEQ_INSWITCH_CASE1:
				{
					// send CASE1 to switchos.specialcaseserver
					udpsendto(reflector_worker_specialcaseclient_udpsock, buf, recvsize, 0, &reflector_switchos_specialcaseserver_addr, reflector_switchos_specialcaseserver_addr_len, "reflector.worker.specialcaseclient");
					break;
				}
			default:
				{
					printf("[reflector.worker] invalid packet type: %d\n", int(pkt_type));
					break;
				}
		}
	}

	close(reflector_worker_udpsock);
	close(reflector_worker_popclient_udpsock);
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
