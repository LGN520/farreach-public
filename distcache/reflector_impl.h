#ifndef REFLECTOR_IMPL_H
#define REFLECTOR_IMPL_H

// shared by reflector.cp2dpserver and reflector.dp2cpserver, but no contention
bool volatile reflector_with_switchos_popworker_popclient_for_reflector_addr = false;
struct sockaddr_in reflector_switchos_popworker_popclient_for_reflector_addr;
unsigned int reflector_switchos_popworker_popclient_for_reflector_addr_len = sizeof(struct sockaddr);

// switchos.popworker -> (udp channel) -> one reflector.cp2dpserver -> data plane
int reflector_cp2dpserver_udpsock = -1;

// data plane -> reflector.dp2cpserver
int reflector_dp2cpserver_udpsock = -1;

// reflector.dp2cpserver -> switchos.popworker
int reflector_dp2cpserver_popclient_udpsock = -1;

// TMPDEBUG
/*struct timespec cp2dp_t1, dp2cp_t2, t3;
std::vector<double> latency_list;
std::vector<size_t> unmatched_cnt_list;*/

void prepare_reflector();
void *run_reflector_cp2dpserver(void *param);
void *run_reflector_dp2cpserver(void *param);
void close_reflector();

void prepare_reflector() {
	printf("[reflector] prepare start\n");

	// prepare worker socket
	if (strcmp(reflector_role, "spine") == 0) {
		prepare_rawserver(reflector_dp2cpserver_udpsock, true, "enp129s0f0", "reflector.dp2cpserver", SOCKET_TIMEOUT, 0, 2*UDP_LARGE_RCVBUFSIZE);
	}
	else {
		prepare_udpserver(reflector_dp2cpserver_udpsock, true, reflector_dp2cpserver_port, "reflector.dp2cpserver", SOCKET_TIMEOUT, 0, 2*UDP_LARGE_RCVBUFSIZE);
	}

	// prepare popserver socket
	prepare_udpserver(reflector_cp2dpserver_udpsock, true, reflector_cp2dpserver_port, "reflector.cp2dpserver", SOCKET_TIMEOUT, 0, UDP_LARGE_RCVBUFSIZE);

	create_udpsock(reflector_dp2cpserver_popclient_udpsock, false, "reflector.dp2cpserver.popclient");

	memory_fence();

	printf("[reflector] prepare end\n");
}

// forward pkt from switchos to switch
void *run_reflector_cp2dpserver(void *param) {
	// client address (switch will not hide CACHE_POP_INSWITCH from clients)
	struct sockaddr_in client_addr;
	set_sockaddr(client_addr, inet_addr(reflector_cp2dp_dstip), 123); // client port is not important, yet we NEED client_ip to choose correct NIC
	socklen_t client_addrlen = sizeof(struct sockaddr_in);

	struct sockaddr_in tmp_addr;
	socklen_t tmp_addrlen = sizeof(struct sockaddr_in);

	char buf[MAX_BUFSIZE];
	printf("[reflector.cp2dpserver] ready\n");
	reflector_ready_threads++;

	while (!reflector_running) {}

	while (reflector_running) {
		int recvsize = -1;
		bool is_timeout = true;
		is_timeout = udprecvfrom(reflector_cp2dpserver_udpsock, buf, MAX_BUFSIZE, 0, &tmp_addr, &tmp_addrlen, recvsize, "reflector.cp2dpserver");

		if (is_timeout) {
			tmp_addrlen = sizeof(struct sockaddr_in);
			continue;
		}

		INVARIANT(recvsize >= 0);

		//CUR_TIME(cp2dp_t1); // TMPDEBUG

		packet_type_t tmp_optype = packet_type_t(get_packet_type(buf, recvsize));
		switch (tmp_optype) {
			case packet_type_t::CACHE_POP_INSWITCH:
			case packet_type_t::CACHE_EVICT_LOADFREQ_INSWITCH:
				{
					if (!reflector_with_switchos_popworker_popclient_for_reflector_addr) {
						memcpy(&reflector_switchos_popworker_popclient_for_reflector_addr, &tmp_addr, sizeof(struct sockaddr_in));
						reflector_switchos_popworker_popclient_for_reflector_addr_len = tmp_addrlen;
						reflector_with_switchos_popworker_popclient_for_reflector_addr = true;
					}
					break;
				}
			default:
				{
					printf("[reflector.cp2dpserver] invalid optype %d\n", int(tmp_optype));
					exit(-1);
				}
		}

		udpsendto(reflector_cp2dpserver_udpsock, buf, recvsize, 0, &client_addr, client_addrlen, "reflector.cp2dpserver");
	}

	close(reflector_cp2dpserver_udpsock);
	pthread_exit(nullptr);
}

// forward pkt from switch to switchos
void *run_reflector_dp2cpserver(void *param) {
	char buf[MAX_BUFSIZE];
	int recvsize = 0;

	struct sockaddr_in switchos_dppopserver_addr;
	set_sockaddr(switchos_dppopserver_addr, inet_addr(switchos_ip), switchos_dppopserver_port);
	socklen_t switchos_dppopserver_addrlen = sizeof(struct sockaddr_in);

	printf("[reflector.dp2cpserver] ready\n");
	reflector_ready_threads++;

	while (!reflector_running) {}

	bool is_timeout = false;
	char *srcip = NULL;
	short srcport = 0;
	//size_t total_unmatched_cnt = 0; // TMPDEBUG
	//size_t tmp_unmatched_cnt = 0; // TMPDEBUG
	while (reflector_running) {

		if (strcmp(reflector_role, "spine") == 0) {
			// TMPDEBUG
			/*tmp_unmatched_cnt = 0;
			is_timeout = rawrecvfrom(reflector_dp2cpserver_udpsock, buf, MAX_BUFSIZE, 0, &srcip, "10.0.2.11", srcport, reflector_dp2cpserver_port, NULL, NULL, recvsize, "reflector.dp2cpserver", tmp_unmatched_cnt);
			total_unmatched_cnt += tmp_unmatched_cnt;*/
			
			is_timeout = rawrecvfrom(reflector_dp2cpserver_udpsock, buf, MAX_BUFSIZE, 0, &srcip, "10.0.2.11", srcport, reflector_dp2cpserver_port, NULL, NULL, recvsize, "reflector.dp2cpserver");
		}
		else {
			is_timeout = udprecvfrom(reflector_dp2cpserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "reflector.dp2cpserver");
		}
		if (is_timeout) {
			continue;
		}
		INVARIANT(recvsize > 0);

		// TMPDEBUG
		/*CUR_TIME(dp2cp_t2);
		DELTA_TIME(dp2cp_t2, cp2dp_t1, t3);
		double tmp_latency = GET_MICROSECOND(t3);
		if (tmp_latency > 0) {
			latency_list.push_back(tmp_latency);
			unmatched_cnt_list.push_back(tmp_unmatched_cnt);
		}*/

		packet_type_t pkt_type = get_packet_type(buf, recvsize);
		switch (pkt_type) {
			case packet_type_t::CACHE_POP_INSWITCH_ACK:
			case packet_type_t::CACHE_EVICT_LOADFREQ_INSWITCH_ACK:
				{
					INVARIANT(reflector_with_switchos_popworker_popclient_for_reflector_addr == true);
					// NOTE: not use popserver.popclient due to duplicate packets for packet loss issued by switch
					udpsendto(reflector_dp2cpserver_popclient_udpsock, buf, recvsize, 0, &reflector_switchos_popworker_popclient_for_reflector_addr, reflector_switchos_popworker_popclient_for_reflector_addr_len, "reflector.dp2cpserver.popclient");
					break;
				}
			case packet_type_t::NETCACHE_GETREQ_POP:
			case packet_type_t::NETCACHE_WARMUPREQ_INSWITCH_POP:
				{
					udpsendto(reflector_dp2cpserver_popclient_udpsock, buf, recvsize, 0, &switchos_dppopserver_addr, switchos_dppopserver_addrlen, "reflector.dp2cpserver.popclient");
					break;
				}
			default:
				{
					printf("[reflector.dp2cpserver] invalid packet type: %d\n", int(pkt_type));
					break;
				}
		}
	}

	close(reflector_dp2cpserver_udpsock);
	close(reflector_dp2cpserver_popclient_udpsock);
	pthread_exit(nullptr);
}

void close_reflector() {
	return;
}

#endif
