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

void create_udpsock(int &sockfd, bool need_timeout, const char* role, int timeout_sec, int timeout_usec) {
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

void prepare_udpserver(int &sockfd, bool need_timeout, short server_port, const char* role, int timeout_sec, int timeout_usec) {
	INVARIANT(role != NULL);

	// create socket
	create_udpsock(sockfd, need_timeout, role, timeout_sec, timeout_usec);
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
	udpsendlarge(sockfd, buf, len, flggs, dest_addr, addrlen, role, 0, UDP_FRAGMENT_MAXSIZE);
}

void udpsendlarge_ipfrag(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role, size_t frag_hdrsize) {
	udpsendlarge(sockfd, buf, len, flggs, dest_addr, addrlen, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE);
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

		udpsendto(sockfd, fragbuf, final_frag_hdrsize + cur_frag_bodysize, flags, dest_addr, addrlen, role);
	}
}

bool udprecvlarge_udpfrag(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role) {
	return udprecvlarge(sockfd, buf, len, flags, src_addr, addrlen, recvsize, role, 0, UDP_FRAGMENT_MAXSIZE);
}

bool udprecvlarge_ipfrag(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role, size_t frag_hdrsize) {
	return udprecvlarge(sockfd, buf, len, flags, src_addr, addrlen, recvsize, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE);
}

// NOTE: receive large packet from one source
bool udprecvlarge(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role, size_t frag_hdrsize, size_t frag_maxsize) {
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

		INVARIANT(len >= (cur_fragidx * frag_bodysize + frag_recvsize - final_frag_hdrsize));
		memcpy(buf + cur_fragidx * frag_bodysize, fragbuf + final_frag_hdrsize, frag_recvsize - final_frag_hdrsize);
		recvsize += (frag_recvsize - final_frag_hdrsize);

		cur_fragnum += 1;
		if (cur_fragnum >= max_fragnum) {
			break;
		}
	}

	return is_timeout;
}


bool udprecvlarge_multisrc_udpfrag(int sockfd, void *bufs, size_t bufnum, size_t len, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, int *recvsizes, int& recvnum, const char* role, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion) {
	return udprecvlarge_multisrc(sockfd, bufs, bufnum, len, flags, src_addrs, addrlens, recvsizes, recvnum, role, 0, UDP_FRAGMENT_MAXSIZE, srcnum_off, srcnum_len, srcnum_conversion, srcid_off, srcid_len, srcid_conversion);
}

bool udprecvlarge_multisrc_ipfrag(int sockfd, void *bufs, size_t bufnum, size_t len, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, int *recvsizes, int& recvnum, const char* role, size_t frag_hdrsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion) {
	return udprecvlarge_multisrc(sockfd, bufs, bufnum, len, flags, src_addrs, addrlens, recvsizes, recvnum, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE, srcnum_off, srcnum_len, srcnum_conversion, srcid_off, srcid_len, srcid_conversion);
}

// NOTE: receive large packet from multiple sources (srcid >= 0 && srcid < srcnum <= bufnum)
// bufs: bufnum * len; src_addrs: NULL or bufnum; addrlens: NULL or bufnum; recvsizes: bufnum
// For example, srcnum for SCANRES_SPLIT is split_hdr.max_scannum; srcid for SCANRES_SPLIT is split_hdr.cur_scanidx
bool udprecvlarge_multisrc(int sockfd, void *bufs, size_t bufnum, size_t len, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, int *recvsizes, int& recvnum, const char* role, size_t frag_hdrsize, size_t frag_maxsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion) {
	INVARIANT(srcnum_len == 1 || srcnum_len == 2 || srcnum_len == 4);
	INVARIANT(srcid_len == 1 || srcid_len == 2 || srcid_len == 4);
	INVARIANT(srcnum_off <= frag_hdrsize && srcid_off <= frag_hdrsize);

	bool is_timeout = false;
	size_t final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
	size_t frag_bodysize = frag_maxsize - final_frag_hdrsize;
	recvsize = 0;
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
		local_isfirsts[tmpbufidx] = false;
		max_fragnums[tmpbufidx] = 0;
		cur_fragnums[tmpbufidx] = 0;

		recvsizes[tmpbufidx] = 0;
	}
	while (true) {
		is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, (struct sockaddr*)&tmp_srcaddr, tmp_addrlen, frag_recvsize, role);
		if (is_timeout) {
			break;
		}
		INVARIANT(frag_recvsize >= final_frag_hdrsize);

		// set max_srcnum for the global first packet
		if (global_isfirst) {
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
		INVARIANT(tmpsrcid < max_srcnum);

		if (local_isfirsts[tmpsrcid]) {
			if (src_addrs != NULL) {
				src_addrs[tmpsrcid] = tmp_srcaddr;
				addrlens[tmpsrcid] = tmp_addrlen;
			}

			INVARIANT(len >= frag_hdrsize);
			memcpy(bufs[tmpsrcid], fragbuf, frag_hdrsize);
			recvsizes[tmpsrcid] += frag_hdrsize;
			memcpy(&max_fragnums[tmpsrcid], fragbuf + frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));

			local_isfirsts[tmpsrcid] = false;
		}

		uint16_t cur_fragidx = 0;
		memcpy(&cur_fragidx, fragbuf + frag_hdrsize, sizeof(uint16_t));
		INVARIANT(cur_fragidx < max_fragnums[tmpsrcid]);

		INVARIANT(len >= (cur_fragidx * frag_bodysize + frag_recvsize - final_frag_hdrsize));
		memcpy(bufs[tmpsrcid] + cur_fragidx * frag_bodysize, fragbuf + final_frag_hdrsize, frag_recvsize - final_frag_hdrsize);
		recvsizes[tmpsrcid] += (frag_recvsize - final_frag_hdrsize);

		cur_fragnums[tmpsrcid] += 1;
		INVARIANT(cur_fragnums[tmpsrcid] <= max_fragnums[tmpsrcid]);
		if (cur_fragnums[tmpsrcid] == max_fragnums[tmpsrcid]) {
			cur_srcnum += 1;
			if (cur_srcnum >= max_srcnum) {
				break;
			}
		}
	}

	INVARIANT(cur_srcnum <= bufnum);
	recvnum = cur_srcnum;
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
