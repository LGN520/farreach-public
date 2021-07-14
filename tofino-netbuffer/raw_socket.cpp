#include "raw_socket.h"

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

uint16_t udp4_checksum (struct iphdr* iph, struct udphdr* udph, char *payload, int payloadlen) {
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

void init_raw_sockaddr(struct sockaddr_ll *socket_address, int ifidx, uint8_t *macaddr)
{
	memset(socket_address, 0, sizeof(struct sockaddr_ll));
	socket_address->sll_family = AF_PACKET;
	socket_address->sll_ifindex = ifidx; 
	socket_address->sll_protocol = htons(ETH_P_ALL);
    socket_address->sll_halen = ETH_ALEN; // 48-bit address
    socket_address->sll_addr[0] = macaddr[0];
    socket_address->sll_addr[1] = macaddr[1];
    socket_address->sll_addr[2] = macaddr[2];
    socket_address->sll_addr[3] = macaddr[3];
    socket_address->sll_addr[4] = macaddr[4];
    socket_address->sll_addr[5] = macaddr[5];
}

size_t init_buf(char *buf, uint32_t maxsize, uint8_t *src_macaddr, uint8_t *dst_macaddr, 
		std::string src_ipaddr, std::string dst_ipaddr, short src_port, 
		short dst_port, char *payload, uint32_t payload_size) {
	assert(buf != NULL);
	memset(buf, 0, maxsize);
	size_t tx_len = 0;

	// Ethernet header
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
    iph->protocol = IPPROTO_UDP;
    /* Source IP address */
    iph->saddr = inet_addr(src_ipaddr.c_str());
    /* Destination IP address */
    iph->daddr = inet_addr(dst_ipaddr.c_str());
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
	iph->tot_len = htons(tx_len - sizeof(struct ether_header));

	// Set IP checksum
	iph->check = 0;
	iph->check = checksum((uint16_t *)iph, sizeof(iphdr));
	std::cout << int(sizeof(iphdr)) << std::endl;
	size_t remain_bytes = tx_len - sizeof(ether_header) - sizeof(iphdr);
	std::cout << "udp:" << ntohs(udph->len) << " ip:" << ntohs(iph->tot_len) << " pkt:" << tx_len << " remain:" << remain_bytes << std::endl;
	udph->check = udp4_checksum(iph, udph, payload, payload_size);

	std::cout <<"ip:" << int(iph->protocol) << std::endl;
	dump_buf(buf, tx_len);
	return tx_len;
}

/*	[Usage]
 *	struct msghdr msg;
 * 	init_msghdr(&msg, &raw_socket_address, totalbuf, totalsize);
 * 	res = sendmsg(raw_sockfd, &msg, 0);
 * 	recv_size = recvmsg(raw_sockfd, &msg, 0);
 */
/*void init_msghdr(struct msghdr *msg, struct sockaddr_ll *socket_address, char *buf, size_t bufsize)
{
	memset(msg, 0, sizeof(struct msghdr));

	// Init iovec
	struct iovec *content = new struct iovec;
	content->iov_base = buf;
	content->iov_len = bufsize;

	// Init msghdr
	msg->msg_iov = content;
	msg->msg_iovlen = 1;
	msg->msg_name = socket_address;
	msg->msg_namelen = sizeof(struct sockaddr_ll);
}*/

/*	[Usage]
 * 	recv_size = recvmsg(raw_sockfd, &msg, 0);
 *	recv_buf(totalbuf, MAX_BUFSIZE, msg, recv_size);
 */
/*void recv_buf(char *buf, uint32_t maxsize, struct msghdr *msg, uint32_t recvsize)
{
	assert(recvsize <= maxsize);
	memset(buf, 0, maxsize);
	uint32_t remain_size = recvsize;
	for (uint32_t iovidx = 0; iovidx < msg->msg_iovlen; iovidx++) {
		struct iovec *cur_content = &msg->msg_iov[iovidx];
		char *cur_buf = (char *)cur_content->iov_base;
		uint32_t cur_bufsize = cur_content->iov_len;
		if (cur_bufsize < remain_size) {
			memcpy(buf + recvsize - remain_size, cur_buf, cur_bufsize);
			remain_size -= cur_bufsize;
		}
		else {
			memcpy(buf + recvsize - remain_size, cur_buf, remain_size);
		}
	}
}*/

int client_recv_payload(char *buf, char *totalbuf, uint32_t totalsize, short client_port, short server_port) 
{
	dump_buf(totalbuf, totalsize);
	uint32_t parsed_size = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr);
	std::cout << "recvsize:" << totalsize << " parsed_size:" << parsed_size << std::endl;
	if (totalsize < parsed_size) {
		return -1;
	}
	struct ether_header *eh = (struct ether_header *) totalbuf;
	std::cout << "eth:" << int(eh->ether_type) << " " << int(htons(ETH_P_IP)) << std::endl;
	if (eh->ether_type == htons(ETH_P_IP)) {
		struct iphdr *iph = (struct iphdr *) (totalbuf + sizeof(struct ether_header));
		std::cout << "ip:" << int(iph->protocol) << " " << int(IPPROTO_UDP) << std::endl;
		if (iph->protocol == IPPROTO_UDP) {
			struct udphdr *udph = (struct udphdr *) (totalbuf + sizeof(struct ether_header) + sizeof(struct iphdr));
			std::cout << "iplen:" << ntohs(iph->tot_len) << " udplen:" << ntohs(udph->len) << std::endl;
			if (udph->source == htons(server_port) && udph->dest == htons(client_port)) {
				int payload_size = totalsize - parsed_size;
				memcpy(buf, totalbuf + parsed_size, payload_size);
				return payload_size;
			}
		}
	}
	return -1;
}

int server_recv_payload(char *buf, char *totalbuf, uint32_t totalsize, short server_port, 
		uint8_t *src_mac, char *src_ip, short *src_port)
{
	uint32_t parsed_size = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr);
	if (totalsize < parsed_size) {
		return -1;
	}
	struct ether_header *eh = (struct ether_header *) totalbuf;
	if (eh->ether_type == htons(ETH_P_IP)) {
		struct iphdr *iph = (struct iphdr *) (totalbuf + sizeof(ether_header));
		if (iph->protocol == IPPROTO_UDP) {
			struct udphdr *udph = (struct udphdr *) (totalbuf + sizeof(ether_header) + sizeof(iphdr));
			if (udph->dest == htons(server_port)) {
				int payload_size = totalsize - parsed_size;
				memcpy(buf, totalbuf + parsed_size, payload_size);
				memcpy(src_mac, eh->ether_shost, 6);
				inet_ntop(AF_INET, (struct in_addr *)&iph->saddr, src_ip, INET_ADDRSTRLEN);
				*src_port = ntohs(udph->source);
				return payload_size;
			}
		}
	}
	return -1;
}

void dump_buf(char *buf, uint32_t bufsize)
{
	for (uint32_t byteidx = 0; byteidx < bufsize; byteidx++) {
		printf("0x%02x ", uint8_t(buf[byteidx]));
	}
	std::cout << std::endl;
}
