#include "raw_socket.h"

unsigned short csum16(unsigned short *buf, int nwords, bool isodd)
{
    unsigned long sum;
    for(sum=0; nwords>0; nwords--)
        sum += *buf++;
	if (isodd) {
		unsigned short tail = 0;
		tail = tail | (*(uint8_t *)buf);
		sum += tail;
	}

    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);

    return (unsigned short)(~sum);
}

void init_ifidx(struct ifreq * ifidx, std::string ifname)
{
    memset(ifidx, 0, sizeof(struct ifreq));
    strncpy(ifidx->ifr_name, ifname.c_str(), IFNAMSIZ-1);
}

void init_raw_sockaddr(struct sockaddr_ll *socket_address, struct ifreq& if_idx, uint8_t *macaddr)
{
	socket_address->sll_ifindex = if_idx.ifr_ifindex;
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
    iph->ihl = 7;
    iph->version = 4;
    iph->tos = 0; // Low delay
    iph->id = htons(11111);
    iph->frag_off = 0x00;
    iph->ttl = 0xff; // hops
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
	udph->len = sizeof(struct udphdr) + payload_size;
	iph->tot_len = htons(tx_len - sizeof(struct ether_header));

	// Set IP checksum
	iph->check = csum16((unsigned short *)(buf + sizeof(ether_header)), sizeof(iphdr)/2);
	size_t remain_bytes = tx_len - sizeof(ether_header) - sizeof(iphdr);
	std::cout << "udp:" << udph->len << " ip:" << ntohs(iph->tot_len) << " pkt:" << tx_len << " remain:" << remain_bytes << std::endl;
	bool isodd = (remain_bytes % 2) == 1;
	udph->check = csum16((unsigned short *)(buf + sizeof(ether_header) + sizeof(iphdr)), remain_bytes/2, isodd);

	return tx_len;
}

void init_msghdr(struct msghdr *msg, struct sockaddr_ll *socket_address, char *buf, size_t bufsize)
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
}
