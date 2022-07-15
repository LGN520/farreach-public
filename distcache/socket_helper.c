#include "socket_helper.h"

#include <arpa/inet.h> // inetaddr conversion; endianess conversion
#include <netinet/tcp.h> // TCP_NODELAY

// udp/tcp sockaddr

void set_sockaddr(sockaddr_in &addr, uint32_t bigendian_saddr, short littleendian_port) {
	memset((void *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = bigendian_saddr;
	addr.sin_port = htons(littleendian_port);
}

void set_recvtimeout(int sockfd, int timeout_sec, int timeout_usec) {
	struct timeval tv;
	tv.tv_sec = timeout_sec;
	tv.tv_usec =  timeout_usec;
	int res = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	INVARIANT(res >= 0);
}

// udp

void create_udpsock(int &sockfd, bool need_timeout, const char* role, int timeout_sec, int timeout_usec, int udp_rcvbufsize) {
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		printf("[%s] fail to create udp socket, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Disable udp/tcp check
	int disable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		printf("[%s] disable checksum failed, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Set timeout for recvfrom/accept of udp/tcp
	if (need_timeout) {
		set_recvtimeout(sockfd, timeout_sec, timeout_usec);
	}
	// set udp receive buffer size
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &udp_rcvbufsize, sizeof(int)) == -1) {
		printf("[%s] fail to set udp receive bufsize as %d, errno: %d\n", role, udp_rcvbufsize, errno);
		exit(-1);
	}
}

void udpsendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role) {
	int res = sendto(sockfd, buf, len, flags, (struct sockaddr *)dest_addr, addrlen);
	if (res < 0) {
		printf("[%s] sendto of udp socket fails, errno: %d!\n", role, errno);
		exit(-1);
	}
}

bool udprecvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role) {
	bool need_timeout = false;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec =  0;
	socklen_t tvsz = sizeof(tv);
	int res = getsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, &tvsz);
	UNUSED(res);
	if (tv.tv_sec != 0 || tv.tv_usec != 0) {
		need_timeout = true;
	}

	bool is_timeout = false;
	recvsize = recvfrom(sockfd, buf, len, flags, (struct sockaddr *)src_addr, addrlen);
	if (recvsize < 0) {
		if (need_timeout && (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)) {
			recvsize = 0;
			is_timeout = true;
		}
		else {
			printf("[%s] error of recvfrom, errno: %d!\n", role, errno);
			exit(-1);
		}
	}
	return is_timeout;
}

void prepare_udpserver(int &sockfd, bool need_timeout, short server_port, const char* role, int timeout_sec, int timeout_usec, int udp_rcvbufsize) {
	INVARIANT(role != NULL);

	// create socket
	create_udpsock(sockfd, need_timeout, role, timeout_sec, timeout_usec, udp_rcvbufsize);
	// reuse the occupied port for the last created socket instead of being crashed
	const int trueFlag = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
		printf("[%s] fail to setsockopt, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Set listen address
	sockaddr_in listen_addr;
	set_sockaddr(listen_addr, htonl(INADDR_ANY), server_port);
	if ((bind(sockfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr))) != 0) {
		printf("[%s] fail to bind socket on port %hu, errno: %d!\n", role, server_port, errno);
		exit(-1);
	}
}

void udpsendlarge_udpfrag(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role) {
	udpsendlarge(sockfd, buf, len, flags, dest_addr, addrlen, role, 0, UDP_FRAGMENT_MAXSIZE);
}

void udpsendlarge_ipfrag(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role, size_t frag_hdrsize) {
	udpsendlarge(sockfd, buf, len, flags, dest_addr, addrlen, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE);
}

void udpsendlarge(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role, size_t frag_hdrsize, size_t frag_maxsize) {
	// (1) buf[0:frag_hdrsize] + <cur_fragidx, max_fragnum> as final header of each fragment payload
	// (2) frag_maxsize is the max size of fragment payload (final fragment header + fragment body), yet not including ethernet/ipv4/udp header
	INVARIANT(len >= frag_hdrsize);
	size_t final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
	size_t frag_bodysize = frag_maxsize - final_frag_hdrsize;
	size_t total_bodysize = len - frag_hdrsize;
	size_t fragnum = 1;
	if (likely(total_bodysize > 0)) {
		fragnum = (total_bodysize + frag_bodysize - 1) / frag_bodysize;
	}
	INVARIANT(fragnum > 0);
	//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d, total_bodysize: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize, total_bodysize);

	// <frag_hdrsize, cur_fragidx, max_fragnum, frag_bodysize>
	char fragbuf[frag_maxsize];
	memset(fragbuf, 0, frag_maxsize);
	memcpy(fragbuf, buf, frag_hdrsize); // fixed fragment header
	uint16_t cur_fragidx = 0;
	uint16_t max_fragnum = uint16_t(fragnum);
	size_t buf_sentsize = frag_hdrsize;
	for (; cur_fragidx < max_fragnum; cur_fragidx++) {
		memset(fragbuf + frag_hdrsize, 0, frag_maxsize - frag_hdrsize);

		// prepare for final fragment header
		// NOTE: UDP fragmentation is processed by end-hosts instead of switch -> no need for endianess conversion
		memcpy(fragbuf + frag_hdrsize, &cur_fragidx, sizeof(uint16_t));
		memcpy(fragbuf + frag_hdrsize + sizeof(uint16_t), &max_fragnum, sizeof(uint16_t));

		// prepare for fragment body
		int cur_frag_bodysize = frag_bodysize;
		if (cur_fragidx == max_fragnum - 1) {
			cur_frag_bodysize = total_bodysize - frag_bodysize * cur_fragidx;
		}
		memcpy(fragbuf + final_frag_hdrsize, buf + buf_sentsize, cur_frag_bodysize);
		buf_sentsize += cur_frag_bodysize;

		//printf("cur_fragidx: %d, max_fragnum: %d, cur_fragsize: %d\n", cur_fragidx, max_fragnum, final_frag_hdrsize + cur_frag_bodysize);
		udpsendto(sockfd, fragbuf, final_frag_hdrsize + cur_frag_bodysize, flags, dest_addr, addrlen, role);
	}
}

bool udprecvlarge_udpfrag(int sockfd, dynamic_array_t &buf, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, const char* role) {
	return udprecvlarge(sockfd, buf, flags, src_addr, addrlen, role, 0, UDP_FRAGMENT_MAXSIZE);
}

bool udprecvlarge_ipfrag(int sockfd, dynamic_array_t &buf, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, const char* role, size_t frag_hdrsize) {
	return udprecvlarge(sockfd, buf, flags, src_addr, addrlen, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE);
}

// NOTE: receive large packet from one source
bool udprecvlarge(int sockfd, dynamic_array_t &buf, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, const char* role, size_t frag_hdrsize, size_t frag_maxsize) {
	bool is_timeout = false;
	size_t final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
	size_t frag_bodysize = frag_maxsize - final_frag_hdrsize;

	char fragbuf[frag_maxsize];
	int frag_recvsize = 0;
	bool is_first = true;
	uint16_t max_fragnum = 0;
	uint16_t cur_fragnum = 0;
	while (true) {
		if (is_first) {
			is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, src_addr, addrlen, frag_recvsize, role);
			if (is_timeout) {
				break;
			}
			INVARIANT(size_t(frag_recvsize) >= final_frag_hdrsize && size_t(frag_recvsize) <= frag_maxsize);
			//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize);

			buf.dynamic_memcpy(0, fragbuf, frag_hdrsize);
			memcpy(&max_fragnum, fragbuf + frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));
			is_first = false;
		}
		else {
			is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, NULL, NULL, frag_recvsize, role);
			if (is_timeout) {
				break;
			}
			INVARIANT(size_t(frag_recvsize) >= final_frag_hdrsize);
		}

		uint16_t cur_fragidx = 0;
		memcpy(&cur_fragidx, fragbuf + frag_hdrsize, sizeof(uint16_t));
		INVARIANT(cur_fragidx < max_fragnum);
		//printf("cur_fragidx: %d, max_fragnum: %d, frag_recvsize: %d, buf_offset: %d, copy_size: %d\n", cur_fragidx, max_fragnum, frag_recvsize, cur_fragidx * frag_bodysize, frag_recvsize - final_frag_hdrsize);

		buf.dynamic_memcpy(0 + frag_hdrsize + cur_fragidx * frag_bodysize, fragbuf + final_frag_hdrsize, frag_recvsize - final_frag_hdrsize);

		cur_fragnum += 1;
		if (cur_fragnum >= max_fragnum) {
			break;
		}
	}

	if (is_timeout) {
		buf.clear();
	}

	return is_timeout;
}


bool udprecvlarge_multisrc_udpfrag(int sockfd, dynamic_array_t **bufs_ptr, size_t &bufnum, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, const char* role, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion, size_t srcswitchnum_off, size_t srcswitchnum_len, bool srcswitchnum_conversion, size_t srcswitchid_off, size_t srcswitchid_len, bool srcswitchid_conversion, bool isfilter, optype_t optype, netreach_key_t targetkey) {
	return udprecvlarge_multisrc(sockfd, bufs_ptr, bufnum, flags, src_addrs, addrlens, role, 0, UDP_FRAGMENT_MAXSIZE, srcnum_off, srcnum_len, srcnum_conversion, srcid_off, srcid_len, srcid_conversion, srcswitchnum_off, srcswitchnum_len, srcswitchnum_conversion, srcswitchid_off, srcswitchid_len, srcswitchid_conversion, isfilter, optype, targetkey);
}

bool udprecvlarge_multisrc_ipfrag(int sockfd, dynamic_array_t **bufs_ptr, size_t &bufnum, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, const char* role, size_t frag_hdrsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion, size_t srcswitchnum_off, size_t srcswitchnum_len, bool srcswitchnum_conversion, size_t srcswitchid_off, size_t srcswitchid_len, bool srcswitchid_conversion, bool isfilter, optype_t optype, netreach_key_t targetkey) {
	return udprecvlarge_multisrc(sockfd, bufs_ptr, bufnum, flags, src_addrs, addrlens, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE, srcnum_off, srcnum_len, srcnum_conversion, srcid_off, srcid_len, srcid_conversion, srcswitchnum_off, srcswitchnum_len, srcswitchnum_conversion, srcswitchid_off, srcswitchid_len, srcswitchid_conversion, isfilter, optype, targetkey);
}

// NOTE: receive large packet from multiple sources; used for SCANRES_SPLIT
// IMPORTANT: srcswitchid > 0 && srcswitchid <= srcswitchnum; srcid > 0 && srcid <= srcnum; each srcswitchid can have different srcnums
// perswitch_perserver_bufs: srcswitchnum * srcnum dynamic arrays; if is_timeout = true, size = 0
// perswitch_perserver_addrs/addrlens have the same shape as perswitch_perserver_bufs
// For example, for SCANRES_SPLIT, srcswitchnum is split_hdr.max_scanswitchnum; srcswitchid is split_hdr.cur_scanswitchidx; srcnum is split_hdr.max_scannum; srcid is split_hdr.cur_scanidx
bool udprecvlarge_multisrc(int sockfd, std::vector<std::vector<dynamic_array_t>> &perswitch_perserver_bufs, int flags, std::vector<std::vector<struct sockaddr_in>> &perswitch_perserver_addrs, std::vector<std::vector<socklen_t>> &perswitch_perserver_addrlens, const char* role, size_t frag_hdrsize, size_t frag_maxsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion, size_t srcswitchnum_off, size_t srcswitchnum_len, bool srcswitchnum_conversion, size_t srcswitchid_off, size_t srcswitchid_len, bool srcswitchid_conversion, bool isfilter, optype_t optype, netreach_key_t targetkey) {
	INVARIANT(srcnum_len == 1 || srcnum_len == 2 || srcnum_len == 4);
	INVARIANT(srcid_len == 1 || srcid_len == 2 || srcid_len == 4);
	INVARIANT(srcnum_off <= frag_hdrsize && srcid_off <= frag_hdrsize);
	INVARIANT(srcswitchnum_len == 1 || srcswitchnum_len == 2 || srcswitchnum_len == 4);
	INVARIANT(srcswitchid_len == 1 || srcswitchid_len == 2 || srcswitchid_len == 4);
	INVARIANT(srcswitchnum_off <= frag_hdrsize && srcswitchid_off <= frag_hdrsize);

	bool is_timeout = false;
	size_t final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
	size_t frag_bodysize = frag_maxsize - final_frag_hdrsize;
	struct sockaddr_in tmp_srcaddr;
	socklen_t tmp_addrlen;

	// receive current fragment
	char fragbuf[frag_maxsize];
	int frag_recvsize = 0;
	// set by the global first fragment
	bool global_isfirst = true;
	size_t cur_srcswitchnum = 0; // already received switchnum
	std::vector<size_t> perswitch_cur_srcnums; // per-switch already received servernum
	// set by the first fragment of each srcswitchid and each srcid
	std::vector<std::vector<uint16_t>> perswitch_perserver_max_fragnums;
	std::vector<std::vector<uint16_t>> perswitch_perserver_cur_fragnums; // per-switch per-server already received fragnum
	netreach_key_t tmpkey;
	while (true) {
		is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, &tmp_srcaddr, &tmp_addrlen, frag_recvsize, role);
		if (is_timeout) {
			break;
		}
		INVARIANT(size_t(frag_recvsize) >= final_frag_hdrsize);

		if (isfilter) {
			if (optype_t(get_packet_type(fragbuf, frag_recvsize)) != optype) {
				continue; // filter the unmatched packet
			}
			tmpkey.deserialize(fragbuf + sizeof(optype_t), frag_recvsize - sizeof(optype_t));
			if (tmpkey != targetkey) {
				continue;
			}
		}

		if (global_isfirst) { // first packet in global -> get switchnum
			//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize);

			size_t max_srcswitchnum = 0;
			memcpy(&max_srcswitchnum, fragbuf + srcswitchnum_off, srcswitchnum_len);
			if (srcswitchnum_conversion && srcswitchnum_len == 2) max_srcswitchnum = size_t(ntohs(uint16_t(max_srcswitchnum)));
			else if (srcswitchnum_conversion && srcswitchnum_len == 4) max_srcswitchnum = size_t(ntohl(uint32_t(max_srcswitchnum)));
			INVARIANT(max_srcswitchnum > 0);

			// initialize
			perswitch_perserver_bufs.resize(max_srcswitchnum);
			perswitch_perserver_addrs.resize(max_srcswitchnum);
			perswitch_perserver_addrlens.resize(max_srcswitchnum);
			perswitch_cur_srcnums.resize(max_srcswitchnum, 0);
			perswitch_perserver_max_fragnums.resize(max_srcswitchnum);
			perswitch_perserver_cur_fragnums.resize(max_srcswitchnum);

			global_isfirst = false;
		}

		// get switchidx
		uint16_t tmpsrcswitchid = 0;
		memcpy(&tmpsrcswitchid, fragbuf + srcswitchid_off, srcswitchid_len);
		if (srcswitchid_conversion && srcswitchid_len == 2) tmpsrcswitchid = size_t(ntohs(uint16_t(tmpsrcswitchid)));
		else if (srcswitchid_conversion && srcswitchid_len == 4) tmpsrcswitchid = size_t(ntohl(uint32_t(tmpsrcswitchid)));
		INVARIANT(tmpsrcswitchid > 0 && tmpsrcswitchid <= perswitch_perserver_bufs.size());
		int tmp_switchidx = tmpsrcswitchid - 1; // [1, max_srcswitchnum] -> [0, max_srcswitchnum-1]

		if (perswitch_perserver_bufs[tmp_switchidx].size() == 0) { // first packet from the leaf switch -> get servernum for the switch
			size_t max_srcnum = 0;
			memcpy(&max_srcnum, fragbuf + srcnum_off, srcnum_len);
			if (srcnum_conversion && srcnum_len == 2) max_srcnum = size_t(ntohs(uint16_t(max_srcnum)));
			else if (srcnum_conversion && srcnum_len == 4) max_srcnum = size_t(ntohl(uint32_t(max_srcnum)));
			INVARIANT(max_srcnum > 0);

			// initialize
			perswitch_perserver_bufs[tmp_switchidx].resize(max_srcnum);
			perswitch_perserver_addrs[tmp_switchidx].resize(max_srcnum);
			perswitch_perserver_addrlens[tmp_switchidx].resize(max_srcnum, sizeof(struct sockaddr_in));
			perswitch_perserver_max_fragnums[tmp_switchidx].resize(max_srcnum, 0);
			perswitch_perserver_cur_fragnums[tmp_switchidx].resize(max_srcnum, 0);
		}

		// get serveridx
		uint16_t tmpsrcid = 0;
		memcpy(&tmpsrcid, fragbuf + srcid_off, srcid_len);
		if (srcid_conversion && srcid_len == 2) tmpsrcid = size_t(ntohs(uint16_t(tmpsrcid)));
		else if (srcid_conversion && srcid_len == 4) tmpsrcid = size_t(ntohl(uint32_t(tmpsrcid)));
		//printf("tmpsrcid: %d, max_srcnum: %d, bufnum: %d\n", tmpsrcid, max_srcnum, bufnum);
		INVARIANT(tmpsrcid > 0 && tmpsrcid <= perswitch_perserver_bufs[tmp_switchidx].size());
		int tmp_bufidx = tmpsrcid - 1; // [1, max_srcnum] -> [0, max_srcnum-1]

		// get dynamic array for the switch and the server
		dynamic_array_t &tmpbuf = perswitch_perserver_bufs[tmp_switchidx][tmp_bufidx];

		if (perswitch_perserver_max_fragnums[tmp_switchidx][tmp_bufidx] == 0) { // first packet from the leaf switch and the server
			uint16_t max_fragnum = 0;
			memcpy(&max_fragnum, fragbuf + frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));
			INVARIANT(max_fragnum > 0);

			perswitch_perserver_addrs[tmp_switchidx][tmp_bufidx] = tmp_srcaddr;
			perswitch_perserver_addrlens[tmp_switchidx][tmp_bufidx] = tmp_addrlen;
			perswitch_perserver_max_fragnums[tmp_switchidx][tmp_bufidx] = max_fragnum;

			tmpbuf.dynamic_memcpy(0, fragbuf, frag_hdrsize);
		}

		uint16_t cur_fragidx = 0;
		memcpy(&cur_fragidx, fragbuf + frag_hdrsize, sizeof(uint16_t));
		INVARIANT(cur_fragidx < perswitch_perserver_max_fragnums[tmp_switchidx][tmp_bufidx]);
		//printf("cur_fragidx: %d, max_fragnum: %d, frag_recvsize: %d, buf_offset: %d, copy_size: %d\n", cur_fragidx, max_fragnums[tmp_bufidx], frag_recvsize, cur_fragidx * frag_bodysize, frag_recvsize - final_frag_hdrsize);

		tmpbuf.dynamic_memcpy(frag_hdrsize + cur_fragidx * frag_bodysize, fragbuf + final_frag_hdrsize, frag_recvsize - final_frag_hdrsize);

		perswitch_perserver_cur_fragnums[tmp_switchidx][tmp_bufidx] += 1;
		INVARIANT(perswitch_perserver_cur_fragnums[tmp_switchidx][tmp_bufidx] <= perswitch_perserver_max_fragnums[tmp_switchidx][tmp_bufidx]);
		if (perswitch_perserver_cur_fragnums[tmp_switchidx][tmp_bufidx] == perswitch_perserver_max_fragnums[tmp_switchidx][tmp_bufidx]) {
			perswitch_cur_srcnums[tmp_switchidx] += 1;
			if (perswitch_cur_srcnums[tmp_switchidx] >= perswitch_perserver_bufs[tmp_switchidx].size()) {
				cur_srcswitchnum += 1;
				if (cur_srcswitchnum >= perswitch_perserver_bufs.size()) {
					break;
				}
			}
		}
	}

	if (is_timeout) {
		perswitch_perserver_bufs.clear();
		perswitch_perserver_addrs.clear();
		perswitch_perserver_addrlens.clear();
	}

	return is_timeout;
}

// tcp

void create_tcpsock(int &sockfd, bool need_timeout, const char* role, int timeout_sec, int timeout_usec) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("[%s] fail to create tcp socket, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Disable udp/tcp check
	int disable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		printf("[%s] disable tcp checksum failed, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Send immediately
	int flag = 1; 
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
	// Set timeout for recvfrom/accept of udp/tcp
	if (need_timeout) {
		set_recvtimeout(sockfd, timeout_sec, timeout_usec);
	}
}

void tcpconnect(int sockfd, const char* ip, short port, const char *srcrole, const char* dstrole) {
	INVARIANT(ip != NULL);
	sockaddr_in addr;
	set_sockaddr(addr, inet_addr(ip), port);
	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		// TODO broken
		printf("[%s] fail to connect %s at %s:%hu, errno: %d!\n", srcrole, dstrole, ip, port, errno);
		exit(-1);
	}

}

void tcpsend(int sockfd, char *buf, int size, const char *role) {
	INVARIANT(buf != NULL && size >= 0);

	int send_size = size;
	const char *ptr = buf;
	while (send_size > 0) {
		int tmpsize = send(sockfd, ptr, send_size, 0);
		if (tmpsize < 0) {
			// TODO: Errno 32 means broken pipe, i.e., remote TCP connection is closed
			printf("[%s] TCP send returns %d, errno: %d\n", role, tmpsize, errno);
			exit(-1);
		}
		ptr += tmpsize;
		send_size -= tmpsize;
	}
}

void prepare_tcpserver(int &sockfd, bool need_timeout, short server_port, int max_pending_num, const char *role, int timeout_sec, int timeout_usec) {
	INVARIANT(role != NULL);

	// create socket
	create_tcpsock(sockfd, need_timeout, role, timeout_sec, timeout_usec);
	// reuse the occupied port for the last created socket instead of being crashed
	const int trueFlag = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
		printf("[%s] fail to setsockopt, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Set listen address
	sockaddr_in listen_addr;
	set_sockaddr(listen_addr, htonl(INADDR_ANY), server_port);
	if ((bind(sockfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr))) != 0) {
		printf("[%s] fail to bind socket on port %hu, errno: %d!\n", role, server_port, errno);
		exit(-1);
	}
	if ((listen(sockfd, max_pending_num)) != 0) {
		printf("[%s] fail to listen on port %hu, errno: %d!\n", role, server_port, errno);
		exit(-1);
	}
}

bool tcpaccept(int sockfd, struct sockaddr_in *addr, socklen_t *addrlen, int &connfd, const char* role) {
	bool need_timeout = false;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec =  0;
	socklen_t tvsz = sizeof(tv);
	int res = getsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, &tvsz);
	UNUSED(res);
	if (tv.tv_sec != 0 || tv.tv_usec != 0) {
		need_timeout = true;
	}

	bool is_timeout = false;

	connfd = accept(sockfd, (struct sockaddr *)addr, addrlen);
	if (connfd == -1) {
		if (need_timeout && (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)) {
			is_timeout = true;
		}
		else {
			printf("[%s] error of accept, errno: %d!\n", role, errno);
			exit(-1);
		}
	}
	return is_timeout;
}

bool tcprecv(int sockfd, void *buf, size_t len, int flags, int &recvsize, const char* role) {
	bool need_timeout = false;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec =  0;
	socklen_t tvsz = sizeof(tv);
	int res = getsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, &tvsz);
	UNUSED(res);
	if (tv.tv_sec != 0 || tv.tv_usec != 0) {
		need_timeout = true;
	}

	//bool is_broken = false;
	bool is_timeout = false;
	recvsize = recv(sockfd, buf, len, flags);
	if (recvsize < 0) {
		// Errno 32 means broken pipe, i.e., server.client is closed
		/*if (errno == 32) {
			printf("[%s] remote client is closed\n", role);
			recvsize = 0;
			is_broken = true;
		}*/
		if (need_timeout && (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)) {
			is_timeout = true;
		}
		else {
			printf("[%s] recv fails: %d, errno: %d!\n", role, recvsize, errno);
			exit(-1);
		}
	}
	//return is_broken;
	return is_timeout;
}





// Deprecated

//bool udprecvlarge_udpfrag(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role) {
//	return udprecvlarge(sockfd, buf, len, flags, src_addr, addrlen, recvsize, role, 0, UDP_FRAGMENT_MAXSIZE);
//}

//bool udprecvlarge_ipfrag(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role, size_t frag_hdrsize) {
//	return udprecvlarge(sockfd, buf, len, flags, src_addr, addrlen, recvsize, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE);
//}

// NOTE: receive large packet from one source
/*bool udprecvlarge(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role, size_t frag_hdrsize, size_t frag_maxsize) {
	bool is_timeout = false;
	size_t final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
	size_t frag_bodysize = frag_maxsize - final_frag_hdrsize;
	recvsize = 0;

	char fragbuf[frag_maxsize];
	int frag_recvsize = 0;
	bool is_first = true;
	uint16_t max_fragnum = 0;
	uint16_t cur_fragnum = 0;
	while (true) {
		if (is_first) {
			is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, src_addr, addrlen, frag_recvsize, role);
			if (is_timeout) {
				break;
			}
			INVARIANT(frag_recvsize >= final_frag_hdrsize && frag_recvsize <= frag_maxsize);
			//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize);

			INVARIANT(len >= frag_hdrsize);
			memcpy(buf, fragbuf, frag_hdrsize);
			recvsize += frag_hdrsize;
			memcpy(&max_fragnum, fragbuf + frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));
			is_first = false;
		}
		else {
			is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, NULL, NULL, frag_recvsize, role);
			if (is_timeout) {
				break;
			}
			INVARIANT(frag_recvsize >= final_frag_hdrsize);
		}

		uint16_t cur_fragidx = 0;
		memcpy(&cur_fragidx, fragbuf + frag_hdrsize, sizeof(uint16_t));
		INVARIANT(cur_fragidx < max_fragnum);
		//printf("cur_fragidx: %d, max_fragnum: %d, frag_recvsize: %d, len: %d, buf_offset: %d, copy_size: %d\n", cur_fragidx, max_fragnum, frag_recvsize, len, cur_fragidx * frag_bodysize, frag_recvsize - final_frag_hdrsize);

		INVARIANT(len >= (cur_fragidx * frag_bodysize + frag_recvsize - final_frag_hdrsize));
		memcpy(buf + frag_hdrsize + cur_fragidx * frag_bodysize, fragbuf + final_frag_hdrsize, frag_recvsize - final_frag_hdrsize);
		recvsize += (frag_recvsize - final_frag_hdrsize);

		cur_fragnum += 1;
		if (cur_fragnum >= max_fragnum) {
			break;
		}
	}

	return is_timeout;
}*/

//bool udprecvlarge_udpfrag(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role) {
//	return udprecvlarge(sockfd, buf, len, flags, src_addr, addrlen, recvsize, role, 0, UDP_FRAGMENT_MAXSIZE);
//}

//bool udprecvlarge_ipfrag(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role, size_t frag_hdrsize) {
//	return udprecvlarge(sockfd, buf, len, flags, src_addr, addrlen, recvsize, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE);
//}

// NOTE: receive large packet from multiple sources (IMPORTANT: srcid > 0 && srcid <= srcnum <= bufnum)
// bufs: bufnum * len; src_addrs: NULL or bufnum; addrlens: NULL or bufnum; recvsizes: bufnum
// For example, srcnum for SCANRES_SPLIT is split_hdr.max_scannum; srcid for SCANRES_SPLIT is split_hdr.cur_scanidx
/*bool udprecvlarge_multisrc(int sockfd, void *bufs, size_t bufnum, size_t len, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, int *recvsizes, int& recvnum, const char* role, size_t frag_hdrsize, size_t frag_maxsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion) {
	INVARIANT(srcnum_len == 1 || srcnum_len == 2 || srcnum_len == 4);
	INVARIANT(srcid_len == 1 || srcid_len == 2 || srcid_len == 4);
	INVARIANT(srcnum_off <= frag_hdrsize && srcid_off <= frag_hdrsize);

	bool is_timeout = false;
	size_t final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
	size_t frag_bodysize = frag_maxsize - final_frag_hdrsize;
	memset(recvsizes, 0, sizeof(int) * bufnum);
	struct sockaddr_in tmp_srcaddr;
	socklen_t tmp_addrlen;

	// receive current fragment
	char fragbuf[frag_maxsize];
	int frag_recvsize = 0;
	// set by the global first fragment
	bool global_isfirst = true;
	size_t max_srcnum = 0;
	size_t cur_srcnum = 0;
	// set by the first fragment of each srcid
	bool local_isfirsts[bufnum];
	uint16_t max_fragnums[bufnum];
	uint16_t cur_fragnums[bufnum];
	// initialize
	for (size_t tmpbufidx = 0; tmpbufidx < bufnum; tmpbufidx++) {
		local_isfirsts[tmpbufidx] = true;
		max_fragnums[tmpbufidx] = 0;
		cur_fragnums[tmpbufidx] = 0;

		recvsizes[tmpbufidx] = 0;
	}
	while (true) {
		is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, &tmp_srcaddr, &tmp_addrlen, frag_recvsize, role);
		if (is_timeout) {
			break;
		}
		INVARIANT(frag_recvsize >= final_frag_hdrsize);

		// set max_srcnum for the global first packet
		if (global_isfirst) {
			//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize);
			memcpy(&max_srcnum, fragbuf + srcnum_off, srcnum_len);
			if (srcnum_conversion && srcnum_len == 2) max_srcnum = size_t(ntohs(uint16_t(max_srcnum)));
			else if (srcnum_conversion && srcnum_len == 4) max_srcnum = size_t(ntohl(uint32_t(max_srcnum)));
			INVARIANT(max_srcnum <= bufnum);
			global_isfirst = false;
		}

		uint16_t tmpsrcid = 0;
		memcpy(&tmpsrcid, fragbuf + srcid_off, srcid_len);
		if (srcid_conversion && srcid_len == 2) tmpsrcid = size_t(ntohs(uint16_t(tmpsrcid)));
		else if (srcid_conversion && srcid_len == 4) tmpsrcid = size_t(ntohl(uint32_t(tmpsrcid)));
		//printf("tmpsrcid: %d, max_srcnum: %d, bufnum: %d\n", tmpsrcid, max_srcnum, bufnum);
		INVARIANT(tmpsrcid > 0 && tmpsrcid <= max_srcnum);

		int tmp_bufidx = tmpsrcid - 1; // [1, max_srcnum] -> [0, max_srcnum-1]
		void *tmpbuf = bufs + tmp_bufidx * len;

		if (local_isfirsts[tmp_bufidx]) {
			if (src_addrs != NULL) {
				src_addrs[tmp_bufidx] = tmp_srcaddr;
				addrlens[tmp_bufidx] = tmp_addrlen;
			}

			INVARIANT(len >= frag_hdrsize);
			memcpy(tmpbuf, fragbuf, frag_hdrsize);
			recvsizes[tmp_bufidx] += frag_hdrsize;
			memcpy(&max_fragnums[tmp_bufidx], fragbuf + frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));

			local_isfirsts[tmp_bufidx] = false;
		}

		uint16_t cur_fragidx = 0;
		memcpy(&cur_fragidx, fragbuf + frag_hdrsize, sizeof(uint16_t));
		INVARIANT(cur_fragidx < max_fragnums[tmp_bufidx]);
		//printf("cur_fragidx: %d, max_fragnum: %d, frag_recvsize: %d, len: %d, buf_offset: %d, copy_size: %d\n", cur_fragidx, max_fragnums[tmp_bufidx], frag_recvsize, len, cur_fragidx * frag_bodysize, frag_recvsize - final_frag_hdrsize);

		INVARIANT(len >= (cur_fragidx * frag_bodysize + frag_recvsize - final_frag_hdrsize));
		memcpy(tmpbuf + frag_hdrsize + cur_fragidx * frag_bodysize, fragbuf + final_frag_hdrsize, frag_recvsize - final_frag_hdrsize);
		recvsizes[tmp_bufidx] += (frag_recvsize - final_frag_hdrsize);

		cur_fragnums[tmp_bufidx] += 1;
		INVARIANT(cur_fragnums[tmp_bufidx] <= max_fragnums[tmp_bufidx]);
		if (cur_fragnums[tmp_bufidx] == max_fragnums[tmp_bufidx]) {
			cur_srcnum += 1;
			if (cur_srcnum >= max_srcnum) {
				break;
			}
		}
	}

	INVARIANT(cur_srcnum <= bufnum);
	recvnum = cur_srcnum;
	return is_timeout;
}*/
