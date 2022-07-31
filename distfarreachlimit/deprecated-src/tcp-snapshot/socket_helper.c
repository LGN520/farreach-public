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

void udpsendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen, const char* role) {
	int res = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
	if (res < 0) {
		printf("[%s] sendto of udp socket fails, errno: %d!\n", role, errno);
		exit(-1);
	}
}

bool udprecvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen, int &recvsize, const char* role) {
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
	recvsize = recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
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
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
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

bool tcpaccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int &connfd, const char* role) {
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

	connfd = accept(sockfd, addr, addrlen);
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
