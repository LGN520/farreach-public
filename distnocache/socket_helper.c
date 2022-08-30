#include "socket_helper.h"

#include <arpa/inet.h> // inetaddr conversion; endianess conversion
#include <netinet/tcp.h> // TCP_NODELAY

//#define SERVER_UNMATCHEDCNT_THRESHOLD 1000

//struct timespec process_t1, process_t2, process_t3; // TMPDEBUG

// utils for raw socket

uint16_t checksum (uint16_t *addr, int len) {
	int count = len;
	register uint32_t sum = 0;
	uint16_t answer = 0;

	// Sum up 2-byte values until none or only one byte left.
	while (count > 1) {
		sum += *(addr++);
		count -= 2;
	}

	// Add left-over byte, if any.
	if (count > 0) {
		sum += *(uint8_t *) addr;
	}

	// Fold 32-bit sum into 16 bits; we lose information by doing this,
	// increasing the chances of a collision.
	// sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
	while (sum >> 16) {
		sum = (sum & 0xffff) + (sum >> 16);
	}

	// Checksum is one's compliment of sum.
	answer = ~sum;

	return (answer);
}

uint16_t udp_checksum (struct iphdr* iph, struct udphdr* udph, const void *payload, int payloadlen) {
	char buf[1024];
	char *ptr;
	int chksumlen = 0;
	int i;

	ptr = &buf[0];  // ptr points to beginning of buffer buf

	// Copy source IP address into buf (32 bits)
	memcpy (ptr, &iph->saddr, sizeof (struct in_addr));
	ptr += sizeof (struct in_addr);
	chksumlen += sizeof (struct in_addr);

	// Copy destination IP address into buf (32 bits)
	memcpy (ptr, &iph->daddr, sizeof (struct in_addr));
	ptr += sizeof (struct in_addr);
	chksumlen += sizeof (struct in_addr);

	// Copy zero field to buf (8 bits)
	*ptr = 0; 
	ptr++;
	chksumlen += 1;

	// Copy transport layer protocol to buf (8 bits)
	memcpy (ptr, &iph->protocol, sizeof (iph->protocol));
	ptr += sizeof (iph->protocol);
	chksumlen += sizeof (iph->protocol);

	// Copy UDP length to buf (16 bits)
	memcpy (ptr, &udph->len, sizeof (udph->len));
	ptr += sizeof (udph->len);
	chksumlen += sizeof (udph->len);

	// Copy UDP source port to buf (16 bits)
	memcpy (ptr, &udph->source, sizeof (udph->source));
	ptr += sizeof (udph->source);
	chksumlen += sizeof (udph->source);

	// Copy UDP destination port to buf (16 bits)
	memcpy (ptr, &udph->dest, sizeof (udph->dest));
	ptr += sizeof (udph->dest);
	chksumlen += sizeof (udph->dest);

	// Copy UDP length again to buf (16 bits)
	memcpy (ptr, &udph->len, sizeof (udph->len));
	ptr += sizeof (udph->len);
	chksumlen += sizeof (udph->len);

	// Copy UDP checksum to buf (16 bits)
	// Zero, since we don't know it yet
	*ptr = 0; ptr++;
	*ptr = 0; ptr++;
	chksumlen += 2;

	// Copy payload to buf
	memcpy (ptr, payload, payloadlen);
	ptr += payloadlen;
	chksumlen += payloadlen;

	// Pad to the next 16-bit boundary
	for (i=0; i<payloadlen%2; i++, ptr++) {
		*ptr = 0;
		ptr++;
		chksumlen++;
	}

	return checksum ((uint16_t *) buf, chksumlen);
}

int lookup_if(int sockfd, std::string ifname, uint8_t *src_macaddr)
{
	struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ-1);
	// lookup ifidx and macaddr based on ifname
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) { 
		perror("SIOCGIFINDEX");
	}
	int ifidx = ifr.ifr_ifindex;
	if (src_macaddr != NULL) {
		memcpy(src_macaddr, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));
	}
	return ifidx;
}

// AF_PACKET raw socket
#ifdef AF_PACKET_RAW
size_t init_buf(char *buf, uint32_t maxsize, uint8_t *src_macaddr, uint8_t *dst_macaddr, const char *src_ipaddr, const char *dst_ipaddr, short src_port, short dst_port, const void *payload, uint32_t payload_size) {
#else
// AF_INET raw socket
size_t init_buf(char *buf, uint32_t maxsize, const char* src_ipaddr, const char* dst_ipaddr, short src_port, short dst_port, const void *payload, uint32_t payload_size) {
#endif
	assert(buf != NULL);
	memset(buf, 0, maxsize);
	size_t tx_len = 0;

	// Ethernet header
#ifdef AF_PACKET_RAW
	struct ether_header *eh = (struct ether_header *) buf;
    eh->ether_shost[0] = src_macaddr[0];
    eh->ether_shost[1] = src_macaddr[1];
    eh->ether_shost[2] = src_macaddr[2];
    eh->ether_shost[3] = src_macaddr[3];
    eh->ether_shost[4] = src_macaddr[4];
    eh->ether_shost[5] = src_macaddr[5];
    eh->ether_dhost[0] = dst_macaddr[0];
    eh->ether_dhost[1] = dst_macaddr[1];
    eh->ether_dhost[2] = dst_macaddr[2];
    eh->ether_dhost[3] = dst_macaddr[3];
    eh->ether_dhost[4] = dst_macaddr[4];
    eh->ether_dhost[5] = dst_macaddr[5];
    eh->ether_type = htons(ETH_P_IP);
    tx_len += sizeof(struct ether_header);
#endif

	// IP Header
    struct iphdr *iph = (struct iphdr *) (buf + sizeof(struct ether_header));
    iph->ihl = 5; // 5 * 32-bit words in IP header
    iph->version = 4;
    iph->tos = 0; // Low delay
    iph->id = htons(1); // the first IP packet for UDP flow
    iph->frag_off = 0x00;
    //iph->ttl = 0xff; // hops
    iph->ttl = 0x40; // hops
    //iph->protocol = IPPROTO_TCP;
    iph->protocol = IPPROTO_UDP; // UDP packet
    /* Source IP address */
    iph->saddr = inet_addr(src_ipaddr);
    /* Destination IP address */
    iph->daddr = inet_addr(dst_ipaddr);
    tx_len += sizeof(struct iphdr);

	// UDP header
    struct udphdr *udph = (struct udphdr *) (buf + tx_len);
    udph->source = htons(src_port);
    udph->dest = htons(dst_port);
    udph->check = 0; // skip
    tx_len += sizeof(struct udphdr);

	// Payload
	memcpy(buf + tx_len, payload, payload_size);
	tx_len += payload_size;

	// Set sizes
	udph->len = htons(sizeof(struct udphdr) + payload_size);
#ifdef AF_PACKET_RAW
	iph->tot_len = htons(tx_len - sizeof(struct ether_header)); // AF_PACKET socket
#else
	iph->tot_len = htons(tx_len); // AF_INET socket
#endif

	// Set IP checksum
	iph->check = 0;
	iph->check = checksum((uint16_t *)iph, sizeof(iphdr));
	udph->check = udp_checksum(iph, udph, payload, payload_size);

	//dump_buf(buf, tx_len);
	return tx_len;
}

// raw socket

void set_rawsockaddr(int sockfd, sockaddr_ll &addr, const char *ifname) {
	INVARIANT(ifname != NULL);

	// lookup interface to get ifidx and mac
	uint8_t macaddr[6];
	int ifidx = lookup_if(sockfd, ifname, macaddr);

	memset(&addr, 0, sizeof(struct sockaddr_ll));
#ifdef AF_PACKET_RAW
	addr.sll_family = AF_PACKET;
	addr.sll_protocol = htons(ETH_P_ALL);
#else
	addr.sll_family = AF_INET;
	addr.sll_protocol = htons(IPPROTO_RAW);
#endif
	addr.sll_ifindex = ifidx; 
    addr.sll_halen = ETH_ALEN; // 48-bit address
    addr.sll_addr[0] = macaddr[0];
    addr.sll_addr[1] = macaddr[1];
    addr.sll_addr[2] = macaddr[2];
    addr.sll_addr[3] = macaddr[3];
    addr.sll_addr[4] = macaddr[4];
    addr.sll_addr[5] = macaddr[5];
}

void create_rawsock(int &sockfd, bool need_timeout, const char* role, int timeout_sec, int timeout_usec, int raw_rcvbufsize) {
#ifdef AF_PACKET_RAW
	sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
#else
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
#endif
	if (sockfd == -1) {
		printf("[%s] fail to create raw socket, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Set timeout for recvfrom/accept of udp/tcp
	if (need_timeout) {
		set_recvtimeout(sockfd, timeout_sec, timeout_usec);
	}
	// set rawsock receive buffer size
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &raw_rcvbufsize, sizeof(int)) == -1) {
		printf("[%s] fail to set rawsock receive bufsize as %d, errno: %d\n", role, raw_rcvbufsize, errno);
		exit(-1);
	}
}

#ifdef AF_PACKET_RAW
void rawsendto(int sockfd, const void *buf, size_t len, int flags, uint8_t *src_mac, uint8_t *dst_mac, const char *src_ip, const char *dst_ip, short src_port, short dst_port, const struct sockaddr_ll *dest_addr, socklen_t addrlen, const char* role) {
#else
void rawsendto(int sockfd, const void *buf, size_t len, int flags, const char *src_ip, const char *dst_ip, short src_port, short dst_port, const struct sockaddr_ll *dest_addr, socklen_t addrlen, const char* role) {
#endif
	// add ethernet, ip, and udp header before buf
	char finalbuf[MAX_BUFSIZE];
#ifdef AF_PACKET_RAW
	size_t finallen = init_buf(finalbuf, MAX_BUFSIZE, src_mac, dst_mac, src_ip, dst_ip, src_port, dst_port, buf, len);
#else
	size_t finallen = init_buf(finalbuf, MAX_BUFSIZE, src_ip, dst_ip, src_port, dst_port, buf, len);
#endif

	int res = sendto(sockfd, finalbuf, finallen, flags, (struct sockaddr *)dest_addr, addrlen);
	if (res < 0) {
		printf("[%s] sendto of udp socket fails, errno: %d!\n", role, errno);
		exit(-1);
	}
}

bool rawrecvfrom(int sockfd, void *buf, size_t len, int flags, char **src_ip, const char *dst_ip, short &src_port, const short &dst_port, struct sockaddr_ll *src_addr, socklen_t *addrlen, int &recvsize, const char* role) {
// TMPDEBUG
//bool rawrecvfrom(int sockfd, void *buf, size_t len, int flags, char **src_ip, const char *dst_ip, short &src_port, const short &dst_port, struct sockaddr_ll *src_addr, socklen_t *addrlen, int &recvsize, const char* role, size_t &unmatched_cnt) {
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
	char totalbuf[MAX_BUFSIZE];
	int totalsize = 0;
	//unmatched_cnt = 0; // TMPDEBUG
	while (true) {
		totalsize = recvfrom(sockfd, totalbuf, MAX_BUFSIZE, flags, (struct sockaddr *)src_addr, addrlen);
		if (totalsize < 0) {
			if (need_timeout && (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)) {
				recvsize = 0;
				is_timeout = true;
				break;
			}
			else {
				printf("[%s] error of recvfrom, errno: %d!\n", role, errno);
				exit(-1);
			}
		}

		// extract ip and port 
		struct ether_header *eh = (struct ether_header *) totalbuf;
		if (eh->ether_type == htons(ETH_P_IP)) {
			struct iphdr *iph = (struct iphdr *) (totalbuf + sizeof(struct ether_header));
			if (iph->protocol == IPPROTO_UDP && iph->daddr == inet_addr(dst_ip)) {
				struct udphdr *udph = (struct udphdr *) (totalbuf + sizeof(struct ether_header) + sizeof(struct iphdr));
				if (ntohs(udph->dest) == dst_port) {
					if (src_ip != NULL) {
						struct in_addr tmp_sin_addr;
						tmp_sin_addr.s_addr = iph->saddr;
						*src_ip = inet_ntoa(tmp_sin_addr);
					}
					src_port = ntohs(udph->source);
					//COUT_VAR(ntohs(udph->dest));

					int headersize = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr);
					recvsize = totalsize - headersize;
					INVARIANT(recvsize <= len);
					memcpy(buf, totalbuf + headersize, recvsize);
					//dump_buf((char *)buf, recvsize);
					break;
				}
				//printf("unmatched srcport %d dstport %d\n", ntohs(udph->source), ntohs(udph->dest));
			}
			//struct in_addr tmp_sin_addr;
			//tmp_sin_addr.s_addr = iph->daddr;
			//printf("unmatched protocol: %x dstip: %s\n", iph->protocol, inet_ntoa(tmp_sin_addr));
		}
		//printf("unmatched ether type: %x\n", eh->ether_type);
		//fflush(stdout);
		//unmatched_cnt += 1; // TMPDEBUG

		//continue;
	}

	return is_timeout;
}

void prepare_rawserver(int &sockfd, bool need_timeout, const char * ifname, const char* role, int timeout_sec, int timeout_usec, int raw_rcvbufsize) {
	INVARIANT(ifname != NULL && role != NULL);

	// create socket
	create_rawsock(sockfd, need_timeout, role, timeout_sec, timeout_usec, raw_rcvbufsize);
	// Set listen interface
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifname, strlen(ifname)) != 0) {
		printf("[%s] fail to bind raw socket on interface %s, errno: %d!\n", role, ifname, errno);
		exit(-1);
	}
	// filter for specific udp port
	uint32_t udpport = 5018;
	struct sock_filter tmp_bpf_code[bpf_codelen];
	memcpy(tmp_bpf_code, bpf_code, bpf_codelen * sizeof(struct sock_filter));
	tmp_bpf_code[5].k = udpport; // change to specific udp port
	struct sock_fprog bpf_filter;
	bpf_filter.len = bpf_codelen;
	bpf_filter.filter = tmp_bpf_code;
	if (setsockopt(sockfd, SOL_SOCKET, SO_ATTACH_FILTER, &bpf_filter, sizeof(bpf_filter)) < 0) {
		printf("[%s] fail to attach bpf filter for udp port %d, errno: %d\n", role, udpport, errno);
		exit(-1);
	}
}

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
	//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d, total_bodysize: %d, fragnum: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize, total_bodysize, fragnum);

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
		//// NOTE: UDP fragmentation is processed by end-hosts instead of switch -> no need for endianess conversion
		//memcpy(fragbuf + frag_hdrsize, &cur_fragidx, sizeof(uint16_t));
		//memcpy(fragbuf + frag_hdrsize + sizeof(uint16_t), &max_fragnum, sizeof(uint16_t));
		// NOTE: to support large value, switch needs to parse fragidx and fragnum -> NEED endianess conversion now
		uint16_t bigendian_cur_fragidx = htons(cur_fragidx); // littleendian -> bigendian for large value
		memcpy(fragbuf + frag_hdrsize, &bigendian_cur_fragidx, sizeof(uint16_t));
		uint16_t bigendian_max_fragnum = htons(max_fragnum); // littleendian -> bigendian for large value
		memcpy(fragbuf + frag_hdrsize + sizeof(uint16_t), &bigendian_max_fragnum, sizeof(uint16_t));

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
	return udprecvlarge(sockfd, buf, flags, src_addr, addrlen, role, UDP_FRAGMENT_MAXSIZE, UDP_FRAGTYPE, NULL);
}

bool udprecvlarge_ipfrag(int sockfd, dynamic_array_t &buf, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, const char* role, pkt_ring_buffer_t *pkt_ring_buffer_ptr) {
	return udprecvlarge(sockfd, buf, flags, src_addr, addrlen, role, IP_FRAGMENT_MAXSIZE, IP_FRAGTYPE, pkt_ring_buffer_ptr);
}

// NOTE: receive large packet from one source
bool udprecvlarge(int sockfd, dynamic_array_t &buf, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, const char* role, size_t frag_maxsize, int fragtype, pkt_ring_buffer_t *pkt_ring_buffer_ptr) {

	bool is_timeout = false;
	struct sockaddr_in tmp_src_addr;
	socklen_t tmp_addrlen = sizeof(struct sockaddr_in);

	size_t frag_hdrsize = 0;
	size_t final_frag_hdrsize = 0;
	size_t frag_bodysize = 0;

	char fragbuf[frag_maxsize];
	int frag_recvsize = 0;
	bool is_first = true;
	uint16_t max_fragnum = 0;
	uint16_t cur_fragnum = 0;

	// ONLY for udprecvlarge_ipfrag
	bool is_used_by_server = (pkt_ring_buffer_ptr != NULL);
	netreach_key_t largepkt_key;
	packet_type_t largepkt_optype;
	uint16_t largepkt_clientlogicalidx; // ONLY for server
	uint32_t largepkt_fragseq; // ONLY for server
	//uint32_t server_unmatchedcnt = 0; // ONLY for server
	struct sockaddr_in largepkt_clientaddr;
	socklen_t largepkt_clientaddrlen;

	// pop pkt from pkt_ring_buffer if any ONLY for IP_FRAGTYPE by server
	if (fragtype == IP_FRAGTYPE && is_used_by_server) {
		bool has_pkt = pkt_ring_buffer_ptr->pop(largepkt_optype, largepkt_key, buf, cur_fragnum, max_fragnum, tmp_src_addr, tmp_addrlen, largepkt_clientlogicalidx, largepkt_fragseq);
		if (has_pkt) { // if w/ pkt in pkt_ring_buffer
			// Copy src address of the first packet for both large and not-large packet
			if (src_addr != NULL) {
				*src_addr = tmp_src_addr;
			}
			if (addrlen != NULL) {
				*addrlen = tmp_addrlen;
			}

			if (max_fragnum == 0) { // small packet
				return false;
			}
			else { // large packet
				if (cur_fragnum >= max_fragnum) { // w/ all fragments
					/*printf("pop a complete large packet %x of key %x from client %d\n", int(largepkt_optype), largepkt_key.keyhihi, largepkt_clientlogicalidx);
					if (largepkt_clientlogicalidx == 16) {
						CUR_TIME(process_t2);
						DELTA_TIME(process_t2, process_t1, process_t3);
						double process_time = GET_MICROSECOND(process_t3);
						printf("process time for client 16: %f\n", process_time);
						fflush(stdout);
					}*/
					return false;
				}
				else { // need to receive remaining fragments
					is_first = false; // we do NOT need to process the first packet
					INVARIANT(is_packet_with_largevalue(largepkt_optype) == true);
					frag_hdrsize = get_frag_hdrsize(largepkt_optype);
					final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
					frag_bodysize = frag_maxsize - final_frag_hdrsize;
					largepkt_clientaddr = tmp_src_addr;
					largepkt_clientaddrlen = tmp_addrlen;
					//printf("pop incomplete large packet %x of key %x from client %d w/ cur_fragnum %d max_fragnum %d\n", int(largepkt_optype), largepkt_key.keyhihi, largepkt_clientlogicalidx, cur_fragnum, max_fragnum);
				}
			}
		}
	}

	while (true) {
		if (is_first) {
			is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, &tmp_src_addr, &tmp_addrlen, frag_recvsize, role);
			if (is_timeout) {
				// NOTE: we do NOT need to push packet back into PktRingBuffer as even the first packet is NOT received
				break;
			}

			// Copy src address of the first packet for both large and not-large packet
			if (src_addr != NULL) {
				*src_addr = tmp_src_addr;
			}
			if (addrlen != NULL) {
				*addrlen = tmp_addrlen;
			}

			// Judge whether the current packet is large or not
			bool is_largepkt = false;
			if (fragtype == IP_FRAGTYPE) { // NOTE: server and client (ONLY for GETREQ) do NOT know whether the packet will be large or not before udprecvlarge_ipfrag
				packet_type_t tmp_optype = get_packet_type(fragbuf, frag_recvsize);
				is_largepkt = is_packet_with_largevalue(tmp_optype);
				if (is_largepkt) {
					frag_hdrsize = get_frag_hdrsize(tmp_optype);
				}
			}
			else if (fragtype == UDP_FRAGTYPE) {
				frag_hdrsize = 0;
				is_largepkt = true; // SNAPSHOT_GETDATA_ACK or SNAPSHOT_SENDDATA MUST be large packet
			}
			else {
				printf("[ERROR] invalid fragtype in udprecvlarge: %d\n", fragtype);
				exit(-1);
			}

			if (!is_largepkt) {
				// NOT large packet -> jump out of the while loop -> behave similar as udprecvfrom to receive a single small packet
				buf.dynamic_memcpy(0, fragbuf, frag_recvsize);
				INVARIANT(!is_timeout);
				break; // <=> return;
			}

			// NOTE: now the current packet MUST be large

			// Save optype and key [and client_logical_idx] ONLY for udprecvlarge_ipfrag to filter unnecessary packet fro client [and server]
			if (fragtype == IP_FRAGTYPE) {
				largepkt_key = get_packet_key(fragbuf, frag_recvsize);
				largepkt_optype = get_packet_type(fragbuf, frag_recvsize);
				if (is_used_by_server) {
					largepkt_clientlogicalidx = get_packet_clientlogicalidx(fragbuf, frag_recvsize);
					largepkt_fragseq = get_packet_fragseq(fragbuf, frag_recvsize);
					largepkt_clientaddr = tmp_src_addr;
					largepkt_clientaddrlen = tmp_addrlen;
					/*if (largepkt_clientlogicalidx == 16) {
						CUR_TIME(process_t1);
					}*/
				}
			}

			// Calculate fraghdrsize, final fraghdrsize, and fragbodysize based on optype
			final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
			frag_bodysize = frag_maxsize - final_frag_hdrsize;
			INVARIANT(final_frag_hdrsize != 0 && frag_bodysize != 0);
			INVARIANT(size_t(frag_recvsize) >= final_frag_hdrsize && size_t(frag_recvsize) <= frag_maxsize);
			//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize);

			// Copy fraghdr and get max fragnum
			buf.dynamic_memcpy(0, fragbuf, frag_hdrsize);
			memcpy(&max_fragnum, fragbuf + frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));
			max_fragnum = ntohs(max_fragnum); // bigendian -> littleendian for large value
			is_first = false; // NOTE: we MUST set is_first = false after timeout checking
		}
		else { // NOTE: access the following code block ONLY if we are pursuiting a large packet
			/*if (server_unmatchedcnt >= SERVER_UNMATCHEDCNT_THRESHOLD) {
				printf("server.worker timeouts w/ server_unmatchedcnt %d vs. SERVER_UNMATCHEDCNT_THRESHOLD %d\n", server_unmatchedcnt, SERVER_UNMATCHEDCNT_THRESHOLD);
				is_timeout = true;
				break;
			}*/

			is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, &tmp_src_addr, &tmp_addrlen, frag_recvsize, role);
			if (is_timeout) {
				break;
			}
	
			// Filter unexpected packets ONLY for udprecvlarge_ipfrag
			if (fragtype == IP_FRAGTYPE) {
				netreach_key_t tmp_nonfirstpkt_key = get_packet_key(fragbuf, frag_recvsize);
				packet_type_t tmp_nonfirstpkt_optype = get_packet_type(fragbuf, frag_recvsize);
				if (!is_used_by_server) { // used by client
					if (tmp_nonfirstpkt_key != largepkt_key || tmp_nonfirstpkt_optype != largepkt_optype) { // unmatched packet
						continue; // skip current unmatched packet, go to receive next one
					}
					// else {} // matched packet
				}
				else { // used by server
					bool tmp_stat = false;
					if (!is_packet_with_largevalue(tmp_nonfirstpkt_optype)) { // not-large packet
						// Push small request into PktRingBuffer
						//printf("push a small request %x into pkt ring buffer\n", int(tmp_nonfirstpkt_optype));
						tmp_stat = pkt_ring_buffer_ptr->push(tmp_nonfirstpkt_optype, tmp_nonfirstpkt_key, fragbuf, frag_recvsize, tmp_src_addr, tmp_addrlen);
						if (!tmp_stat) {
							printf("[ERROR] overflow of pkt_ring_buffer when push optype %x\n", optype_t(tmp_nonfirstpkt_optype));
							exit(-1);
						}
						//server_unmatchedcnt += 1;
						continue;
					}
					else { // large packet
						if (is_packet_with_clientlogicalidx(tmp_nonfirstpkt_optype)) { // large packet for server
							uint16_t tmp_nonfirstpkt_clientlogicalidx = get_packet_clientlogicalidx(fragbuf, frag_recvsize);
							uint32_t tmp_nonfirstpkt_fragseq = get_packet_fragseq(fragbuf, frag_recvsize);
							if (tmp_nonfirstpkt_clientlogicalidx != largepkt_clientlogicalidx) { // from different client
								// calculate fraghdrsize and fragbodysize for the large packet from different client
								uint32_t tmp_nonfirstpkt_frag_hdrsize = get_frag_hdrsize(tmp_nonfirstpkt_optype);
								uint32_t tmp_nonfirstpkt_final_frag_hdrsize = tmp_nonfirstpkt_frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
								uint32_t tmp_nonfirstpkt_frag_bodysize = frag_maxsize - tmp_nonfirstpkt_final_frag_hdrsize;
								INVARIANT(tmp_nonfirstpkt_final_frag_hdrsize != 0 && tmp_nonfirstpkt_frag_bodysize != 0);
								INVARIANT(size_t(frag_recvsize) >= tmp_nonfirstpkt_final_frag_hdrsize && size_t(frag_recvsize) <= frag_maxsize);

								// calculate maxfragnum and curfragidx for the large packet from different client
								uint16_t tmp_nonfirstpkt_max_fragnum = 0, tmp_nonfirstpkt_cur_fragidx = 0;
								memcpy(&tmp_nonfirstpkt_max_fragnum, fragbuf + tmp_nonfirstpkt_frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));
								tmp_nonfirstpkt_max_fragnum = ntohs(tmp_nonfirstpkt_max_fragnum); // bigendian -> littleendian for large value
								memcpy(&tmp_nonfirstpkt_cur_fragidx, fragbuf + tmp_nonfirstpkt_frag_hdrsize, sizeof(uint16_t));
								tmp_nonfirstpkt_cur_fragidx = ntohs(tmp_nonfirstpkt_cur_fragidx); // bigendian -> littleendian for large value

								// judge whether it is a new large packet or existing large packet
								bool is_clientlogicalidx_exist = pkt_ring_buffer_ptr->is_clientlogicalidx_exist(tmp_nonfirstpkt_clientlogicalidx);
								if (is_clientlogicalidx_exist) { // update large packet received before
									// Update existing large packet in PktRingBuffer
									INVARIANT(fragtype == IP_FRAGTYPE);
									//printf("update a large packet %x of key %x from client %d into pkt ring buffer\n", int(tmp_nonfirstpkt_optype), tmp_nonfirstpkt_key.keyhihi, tmp_nonfirstpkt_clientlogicalidx);
									if (tmp_nonfirstpkt_cur_fragidx != 0) {
										pkt_ring_buffer_ptr->update_large(tmp_nonfirstpkt_optype, tmp_nonfirstpkt_key, fragbuf, tmp_nonfirstpkt_frag_hdrsize, tmp_nonfirstpkt_frag_hdrsize + tmp_nonfirstpkt_cur_fragidx * tmp_nonfirstpkt_frag_bodysize, fragbuf + tmp_nonfirstpkt_final_frag_hdrsize, frag_recvsize - tmp_nonfirstpkt_final_frag_hdrsize, tmp_src_addr, tmp_addrlen, tmp_nonfirstpkt_clientlogicalidx, tmp_nonfirstpkt_fragseq, false);
									}
									else { // memcpy fraghdr again for fragment 0 to ensure correct seq for farreach/distfarreach/netcache/distcache
										pkt_ring_buffer_ptr->update_large(tmp_nonfirstpkt_optype, tmp_nonfirstpkt_key, fragbuf, tmp_nonfirstpkt_frag_hdrsize, tmp_nonfirstpkt_frag_hdrsize + tmp_nonfirstpkt_cur_fragidx * tmp_nonfirstpkt_frag_bodysize, fragbuf + tmp_nonfirstpkt_final_frag_hdrsize, frag_recvsize - tmp_nonfirstpkt_final_frag_hdrsize, tmp_src_addr, tmp_addrlen, tmp_nonfirstpkt_clientlogicalidx, tmp_nonfirstpkt_fragseq, true);
									}
								}
								else { // add new large packet
									/*if (tmp_nonfirstpkt_clientlogicalidx == 16) {
										CUR_TIME(process_t1);
									}*/
									// Push large packet into PktRingBuffer
									//printf("push a large packet %x of key %x from client %d into pkt ring buffer\n", int(tmp_nonfirstpkt_optype), tmp_nonfirstpkt_key.keyhihi, tmp_nonfirstpkt_clientlogicalidx);
									tmp_stat = pkt_ring_buffer_ptr->push_large(tmp_nonfirstpkt_optype, tmp_nonfirstpkt_key, fragbuf, tmp_nonfirstpkt_frag_hdrsize, tmp_nonfirstpkt_frag_hdrsize + tmp_nonfirstpkt_cur_fragidx * tmp_nonfirstpkt_frag_bodysize, fragbuf + tmp_nonfirstpkt_final_frag_hdrsize, frag_recvsize - tmp_nonfirstpkt_final_frag_hdrsize, 1, tmp_nonfirstpkt_max_fragnum, tmp_src_addr, tmp_addrlen, tmp_nonfirstpkt_clientlogicalidx, tmp_nonfirstpkt_fragseq);
									if (!tmp_stat) {
										printf("[ERROR] overflow of pkt_ring_buffer when push_large optype %x clientlogicalidx %d\n", optype_t(tmp_nonfirstpkt_optype), tmp_nonfirstpkt_clientlogicalidx);
										exit(-1);
									}
								}
								//server_unmatchedcnt += 1;
								continue;
							}
							else { // from the same client
								if (tmp_nonfirstpkt_key != largepkt_key || !is_same_optype(tmp_nonfirstpkt_optype, largepkt_optype) || tmp_nonfirstpkt_fragseq < largepkt_fragseq) { // unmatched packet
									printf("[WARNING] unmatched large packet %x (expect: %x) from client %d of key %x (expect: %x) with fragseq %d (expect %d)\n", \
											int(tmp_nonfirstpkt_optype), int(largepkt_optype), tmp_nonfirstpkt_clientlogicalidx, tmp_nonfirstpkt_key.keyhihi, largepkt_key.keyhihi, tmp_nonfirstpkt_fragseq, largepkt_fragseq);
									continue; // skip current unmatched packet, go to receive next one
								}
								else { // matched packet w/ fragseq >= largepkt_fragseq
									if (tmp_nonfirstpkt_fragseq > largepkt_fragseq) {
										printf("[WARNING] socket helper receives larger fragseq %d > %d of key %x from client %d due to client-side timeout-and-retry\n", tmp_nonfirstpkt_fragseq, largepkt_fragseq, largepkt_key.keyhihi, largepkt_clientlogicalidx);
										largepkt_fragseq = tmp_nonfirstpkt_fragseq;
										cur_fragnum = 0; // increased by 1 later; treat the current fragment w/ larger fragseq as the first fragment
									}
									//else {} // fragseq == largepkt_fragseq
									//server_unmatchedcnt = 0;
								}
							}
						}
						else { // packet NOT for server
							printf("[ERROR] invalid packet type received by server: %x\n", optype_t(tmp_nonfirstpkt_optype));
							exit(-1);
						} // large packet to server w/ clientlogicalidx
					} // large/small to server
				} // to client/server
			} // IP_FRAGTYPE

			// NOTE: ONLY matched packet to client OR matched large packet from the same client to server can arrive here
			INVARIANT(size_t(frag_recvsize) >= final_frag_hdrsize);
		}

		// NOTE: ONLY large packet can access the following code

		uint16_t cur_fragidx = 0;
		memcpy(&cur_fragidx, fragbuf + frag_hdrsize, sizeof(uint16_t));
		cur_fragidx = ntohs(cur_fragidx); // bigendian -> littleendian for large value
		INVARIANT(cur_fragidx < max_fragnum);
		//printf("cur_fragidx: %d, max_fragnum: %d, frag_recvsize: %d, buf_offset: %d, copy_size: %d\n", cur_fragidx, max_fragnum, frag_recvsize, cur_fragidx * frag_bodysize, frag_recvsize - final_frag_hdrsize);
		
		//printf("cur_fragidx %d cur_fragnum %d max_fragnum %d key %x client %d\n", cur_fragidx, cur_fragnum, max_fragnum, largepkt_key.keyhihi, largepkt_clientlogicalidx);
		//fflush(stdout);

		buf.dynamic_memcpy(0 + frag_hdrsize + cur_fragidx * frag_bodysize, fragbuf + final_frag_hdrsize, frag_recvsize - final_frag_hdrsize);

		// memcpy fraghdr again for fragment 0 to ensure correct seq for farreach/distfarreach/netcache/distcache
		if (fragtype == IP_FRAGTYPE && cur_fragidx == 0) {
			buf.dynamic_memcpy(0, fragbuf, frag_hdrsize);
		}

		cur_fragnum += 1;
		if (cur_fragnum >= max_fragnum) {
			/*if (largepkt_clientlogicalidx == 16) {
				CUR_TIME(process_t2);
				DELTA_TIME(process_t2, process_t1, process_t3);
				double process_time = GET_MICROSECOND(process_t3);
				printf("process time for client 16: %f\n", process_time);
				fflush(stdout);
			}*/
			break;
		}
	}

	if (is_timeout) {
		if (fragtype == IP_FRAGTYPE && is_used_by_server) {
			if (!is_first) { // we are waiting for remaining fragments of a large packet
				INVARIANT(pkt_ring_buffer_ptr->is_clientlogicalidx_exist(largepkt_clientlogicalidx) == false);
				INVARIANT(cur_fragnum >= 1);
				// Push back current large packet into PktRingBuffer
				//printf("[WARNING] push back current large packet %x of key %x from client %d into pkt ring buffer\n", int(largepkt_optype), largepkt_key.keyhihi, largepkt_clientlogicalidx);
				bool tmp_stat = pkt_ring_buffer_ptr->push_large(largepkt_optype, largepkt_key, buf.array(), frag_hdrsize, frag_hdrsize, buf.array() + frag_hdrsize, buf.size() - frag_hdrsize, cur_fragnum, max_fragnum, largepkt_clientaddr, largepkt_clientaddrlen, largepkt_clientlogicalidx, largepkt_fragseq);
				if (!tmp_stat) {
					printf("[ERROR] overflow of pkt_ring_buffer when push_large optype %x clientlogicalidx %d\n", optype_t(largepkt_optype), largepkt_clientlogicalidx);
					exit(-1);
				}
			}
		}
		buf.clear();
	}

	return is_timeout;
}


bool udprecvlarge_multisrc_udpfrag(int sockfd, std::vector<std::vector<dynamic_array_t>> &perswitch_perserver_bufs, int flags, std::vector<std::vector<struct sockaddr_in>> &perswitch_perserver_addrs, std::vector<std::vector<socklen_t>> &perswitch_perserver_addrlens, const char* role, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion, size_t srcswitchnum_off, size_t srcswitchnum_len, bool srcswitchnum_conversion, size_t srcswitchid_off, size_t srcswitchid_len, bool srcswitchid_conversion, bool isfilter, optype_t optype, netreach_key_t targetkey) {
	return udprecvlarge_multisrc(sockfd, perswitch_perserver_bufs, flags, perswitch_perserver_addrs, perswitch_perserver_addrlens, role, 0, UDP_FRAGMENT_MAXSIZE, srcnum_off, srcnum_len, srcnum_conversion, srcid_off, srcid_len, srcid_conversion, srcswitchnum_off, srcswitchnum_len, srcswitchnum_conversion, srcswitchid_off, srcswitchid_len, srcswitchid_conversion, isfilter, optype, targetkey);
}

bool udprecvlarge_multisrc_ipfrag(int sockfd, std::vector<std::vector<dynamic_array_t>> &perswitch_perserver_bufs, int flags, std::vector<std::vector<struct sockaddr_in>> &perswitch_perserver_addrs, std::vector<std::vector<socklen_t>> &perswitch_perserver_addrlens, const char* role, size_t frag_hdrsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion, size_t srcswitchnum_off, size_t srcswitchnum_len, bool srcswitchnum_conversion, size_t srcswitchid_off, size_t srcswitchid_len, bool srcswitchid_conversion, bool isfilter, optype_t optype, netreach_key_t targetkey) {
	return udprecvlarge_multisrc(sockfd, perswitch_perserver_bufs, flags, perswitch_perserver_addrs, perswitch_perserver_addrlens, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE, srcnum_off, srcnum_len, srcnum_conversion, srcid_off, srcid_len, srcid_conversion, srcswitchnum_off, srcswitchnum_len, srcswitchnum_conversion, srcswitchid_off, srcswitchid_len, srcswitchid_conversion, isfilter, optype, targetkey);
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
			//printf("received optype: %x, expected optype: %x\n", int(get_packet_type(fragbuf, frag_recvsize)), int(optype));
			if (optype_t(get_packet_type(fragbuf, frag_recvsize)) != optype) {
				continue; // filter the unmatched packet
			}
			tmpkey.deserialize(fragbuf + sizeof(optype_t) + sizeof(switchidx_t), frag_recvsize - sizeof(optype_t) - sizeof(switchidx_t));
			//printf("received key: %x, expected key: %x\n", tmpkey.keyhihi, targetkey.keyhihi);
			if (tmpkey != targetkey) {
				continue;
			}
		}

		if (global_isfirst) { // first packet in global -> get switchnum
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

			//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d, max_srcswitchnum: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize, max_srcswitchnum);
		}

		// get switchidx
		uint16_t tmpsrcswitchid = 0;
		memcpy(&tmpsrcswitchid, fragbuf + srcswitchid_off, srcswitchid_len);
		if (srcswitchid_conversion && srcswitchid_len == 2) tmpsrcswitchid = size_t(ntohs(uint16_t(tmpsrcswitchid)));
		else if (srcswitchid_conversion && srcswitchid_len == 4) tmpsrcswitchid = size_t(ntohl(uint32_t(tmpsrcswitchid)));
		INVARIANT(tmpsrcswitchid > 0 && tmpsrcswitchid <= perswitch_perserver_bufs.size());
		int tmp_switchidx = tmpsrcswitchid - 1; // [1, max_srcswitchnum] -> [0, max_srcswitchnum-1]

		//printf("tmpsrcswitchid: %d, tmp_switchidx: %d\n", tmpsrcswitchid, tmp_switchidx);

		if (perswitch_perserver_bufs[tmp_switchidx].size() == 0) { // first packet from the leaf switch -> get servernum for the switch
			size_t max_srcnum = 0;
			memcpy(&max_srcnum, fragbuf + srcnum_off, srcnum_len);
			if (srcnum_conversion && srcnum_len == 2) max_srcnum = size_t(ntohs(uint16_t(max_srcnum)));
			else if (srcnum_conversion && srcnum_len == 4) max_srcnum = size_t(ntohl(uint32_t(max_srcnum)));
			INVARIANT(max_srcnum > 0);

			// initialize
			perswitch_perserver_bufs[tmp_switchidx].resize(max_srcnum);
			for (size_t i = 0; i < max_srcnum; i++) {
				perswitch_perserver_bufs[tmp_switchidx][i].init(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
			}
			perswitch_perserver_addrs[tmp_switchidx].resize(max_srcnum);
			perswitch_perserver_addrlens[tmp_switchidx].resize(max_srcnum, sizeof(struct sockaddr_in));
			perswitch_perserver_max_fragnums[tmp_switchidx].resize(max_srcnum, 0);
			perswitch_perserver_cur_fragnums[tmp_switchidx].resize(max_srcnum, 0);

			//printf("max_srcnum: %d\n", max_srcnum);
		}

		// get serveridx
		uint16_t tmpsrcid = 0;
		memcpy(&tmpsrcid, fragbuf + srcid_off, srcid_len);
		if (srcid_conversion && srcid_len == 2) tmpsrcid = size_t(ntohs(uint16_t(tmpsrcid)));
		else if (srcid_conversion && srcid_len == 4) tmpsrcid = size_t(ntohl(uint32_t(tmpsrcid)));
		//printf("tmpsrcid: %d, max_srcnum: %d, bufnum: %d\n", tmpsrcid, max_srcnum, bufnum);
		INVARIANT(tmpsrcid > 0 && tmpsrcid <= perswitch_perserver_bufs[tmp_switchidx].size());
		int tmp_bufidx = tmpsrcid - 1; // [1, max_srcnum] -> [0, max_srcnum-1]

		//printf("tmpsrcid: %d, tmp_bufidx: %d\n", tmpsrcid, tmp_bufidx);

		// get dynamic array for the switch and the server
		dynamic_array_t &tmpbuf = perswitch_perserver_bufs[tmp_switchidx][tmp_bufidx];

		if (perswitch_perserver_max_fragnums[tmp_switchidx][tmp_bufidx] == 0) { // first packet from the leaf switch and the server
			uint16_t max_fragnum = 0;
			memcpy(&max_fragnum, fragbuf + frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));
			max_fragnum = ntohs(max_fragnum); // bigendian -> littleendian for large value
			INVARIANT(max_fragnum > 0);

			perswitch_perserver_addrs[tmp_switchidx][tmp_bufidx] = tmp_srcaddr;
			perswitch_perserver_addrlens[tmp_switchidx][tmp_bufidx] = tmp_addrlen;
			perswitch_perserver_max_fragnums[tmp_switchidx][tmp_bufidx] = max_fragnum;

			tmpbuf.dynamic_memcpy(0, fragbuf, frag_hdrsize);

			//printf("max_fragnum: %d\n", max_fragnum);
		}

		uint16_t cur_fragidx = 0;
		memcpy(&cur_fragidx, fragbuf + frag_hdrsize, sizeof(uint16_t));
		cur_fragidx = ntohs(cur_fragidx); // bigendian -> littleendian for large value
		INVARIANT(cur_fragidx < perswitch_perserver_max_fragnums[tmp_switchidx][tmp_bufidx]);
		//printf("cur_fragidx: %d, max_fragnum: %d, frag_recvsize: %d, buf_offset: %d, copy_size: %d\n", cur_fragidx, perswitch_perserver_max_fragnums[tmp_switchidx][tmp_bufidx], frag_recvsize, cur_fragidx * frag_bodysize, frag_recvsize - final_frag_hdrsize);

		tmpbuf.dynamic_memcpy(frag_hdrsize + cur_fragidx * frag_bodysize, fragbuf + final_frag_hdrsize, frag_recvsize - final_frag_hdrsize);

		//printf("cur_fragidx: %d\n", cur_fragidx);

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
