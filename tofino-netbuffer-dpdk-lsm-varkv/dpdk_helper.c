#include "dpdk_helper.h" 
#include "helper.h"
static struct rte_eth_conf port_conf_default;

static inline void dump_buf(char *buf, uint32_t bufsize)
{
	for (uint32_t byteidx = 0; byteidx < bufsize; byteidx++) {
		printf("0x%02x ", uint8_t(buf[byteidx]));
	}
	printf("\n");
}

static inline uint16_t checksum (uint16_t *addr, int len) {
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

static inline uint16_t udp4_checksum (struct ipv4_hdr* iph, struct udp_hdr* udph, char *payload, int payloadlen) {
	char buf[1024];
	char *ptr;
	int chksumlen = 0;
	int i;

	ptr = &buf[0];  // ptr points to beginning of buffer buf

	// Copy source IP address into buf (32 bits)
	memcpy (ptr, &iph->src_addr, sizeof (rte_be32_t));
	ptr += sizeof (rte_be32_t);
	chksumlen += sizeof (rte_be32_t);

	// Copy destination IP address into buf (32 bits)
	memcpy (ptr, &iph->dst_addr, sizeof (rte_be32_t));
	ptr += sizeof (rte_be32_t);
	chksumlen += sizeof (rte_be32_t);

	// Copy zero field to buf (8 bits)
	*ptr = 0; 
	ptr++;
	chksumlen += 1;

	// Copy transport layer protocol to buf (8 bits)
	memcpy (ptr, &iph->next_proto_id, sizeof (iph->next_proto_id));
	ptr += sizeof (iph->next_proto_id);
	chksumlen += sizeof (iph->next_proto_id);

	// Copy UDP length to buf (16 bits)
	memcpy (ptr, &udph->dgram_len, sizeof (udph->dgram_len));
	ptr += sizeof (udph->dgram_len);
	chksumlen += sizeof (udph->dgram_len);

	// Copy UDP source port to buf (16 bits)
	memcpy (ptr, &udph->src_port, sizeof (udph->src_port));
	ptr += sizeof (udph->src_port);
	chksumlen += sizeof (udph->src_port);

	// Copy UDP destination port to buf (16 bits)
	memcpy (ptr, &udph->dst_port, sizeof (udph->dst_port));
	ptr += sizeof (udph->dst_port);
	chksumlen += sizeof (udph->dst_port);

	// Copy UDP length again to buf (16 bits)
	memcpy (ptr, &udph->dgram_len, sizeof (udph->dgram_len));
	ptr += sizeof (udph->dgram_len);
	chksumlen += sizeof (udph->dgram_len);

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

static inline 
int port_init(uint16_t port, struct rte_mempool *mbuf_pool, uint16_t n_txring, uint16_t n_rxring) {
	struct rte_eth_conf port_conf = port_conf_default;
	const uint16_t rx_rings = n_rxring, tx_rings = n_txring;
	uint16_t nb_rxd = RX_RING_SIZE;
	uint16_t nb_txd = TX_RING_SIZE;
	int retval;
	uint16_t q;
	struct rte_eth_dev_info dev_info;
	struct rte_eth_rxconf rxconf;
	struct rte_eth_txconf txconf;

	if (!rte_eth_dev_is_valid_port(port))
		return -1;

	/*retval = rte_eth_dev_info_get(port, &dev_info);
	if (retval != 0) {
		printf("Error during getting device (port %u) info: %s\n",
				port, strerror(-retval));
		return retval;
	}*/

	rte_eth_dev_info_get(port, &dev_info);
	if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
		port_conf.txmode.offloads |=
			DEV_TX_OFFLOAD_MBUF_FAST_FREE;

	/* Configure the Ethernet device. */
	printf("Initialize port %u with %u TX rings and %u RX rings\n", port, n_txring, n_rxring);
	retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
	if (retval != 0)
		return retval;

	retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
	if (retval != 0)
		return retval;

	rxconf = dev_info.default_rxconf;
	rxconf.offloads = port_conf.rxmode.offloads;
	/* Allocate and set up 1 RX queue per Ethernet port. */
	for (q = 0; q < rx_rings; q++) {
		retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
				rte_eth_dev_socket_id(port), &rxconf, mbuf_pool);
		if (retval < 0)
			return retval;
	}

	txconf = dev_info.default_txconf;
	txconf.offloads = port_conf.txmode.offloads;
	/* Allocate and set up 1 TX queue per Ethernet port. */
	for (q = 0; q < tx_rings; q++) {
		retval = rte_eth_tx_queue_setup(port, q, nb_txd,
				rte_eth_dev_socket_id(port), &txconf);
		if (retval < 0)
			return retval;
	}

	/* Display the port MAC address. */
	struct ether_addr addr;
	rte_eth_macaddr_get(port, &addr);
	printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
			   " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
			port,
			addr.addr_bytes[0], addr.addr_bytes[1],
			addr.addr_bytes[2], addr.addr_bytes[3],
			addr.addr_bytes[4], addr.addr_bytes[5]);

	/* Enable RX in promiscuous mode for the Ethernet device. */
	rte_eth_promiscuous_enable(port);

	/* Start the Ethernet port. */
	retval = rte_eth_dev_start(port);
	if (retval < 0)
		return retval;

	// Wait until link up
	struct rte_eth_link link;
	memset(&link, 0, sizeof(struct rte_eth_link));
	uint32_t max_repeat_times = 1000;
	uint32_t check_interval_ms = 10;
	for (uint32_t i = 0; i <= max_repeat_times; i++) {
		rte_eth_link_get(port, &link);
		if (link.link_status == ETH_LINK_UP)
			break;
		rte_delay_ms(check_interval_ms);
	}
	if (link.link_status == ETH_LINK_DOWN) {
		rte_exit(EXIT_FAILURE, "Link is down for port %u\n", port);
	}
	printf("Initialize port %u done!\n", port);

	return 0;
}

void rte_eal_init_helper(int *argc, char ***argv) {
	/* Initialize the Environment Abstraction Layer (EAL). */
	int ret = rte_eal_init(*argc, *argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	//*argc -= ret;
	//*argv += ret;

	port_conf_default.link_speeds = ETH_LINK_SPEED_40G;
	struct rte_eth_rxmode tmp_rxmode;
	tmp_rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
	tmp_rxmode.split_hdr_size = 0;
	port_conf_default.rxmode = tmp_rxmode;
	/*struct rte_eth_txmode tmp_txmode;
	tmp_txmode.offloads = DEV_TX_OFFLOAD_VLAN_INSERT |
				DEV_TX_OFFLOAD_IPV4_CKSUM  |
				DEV_TX_OFFLOAD_UDP_CKSUM   |
				DEV_TX_OFFLOAD_TCP_CKSUM   |
				DEV_TX_OFFLOAD_SCTP_CKSUM  |
				DEV_TX_OFFLOAD_TCP_TSO;
	port_conf_default.txmode = tmp_txmode;*/
}

void dpdk_init(struct rte_mempool **mbuf_pool_ptr, uint16_t n_txring, uint16_t n_rxring) {
	unsigned nb_ports;
	unsigned lcore_count;
	uint16_t portid = 0;

	/* Check that there is an even number of ports to send/receive on. */
	nb_ports = rte_eth_dev_count_avail();
	if (nb_ports == 0)
		rte_exit(EXIT_FAILURE, "No available DPDK port\n");
	printf("Available number of ports: %u, while we only use port 0\n", nb_ports);

	/* Creates a new mempool in memory to hold the mbufs. */
	printf("mbuf num: %d\n", int(NUM_MBUFS * nb_ports));
	*mbuf_pool_ptr = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
		MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

	if (*mbuf_pool_ptr == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

	/* Initialize all ports. */
	if (port_init(portid, *mbuf_pool_ptr, n_txring, n_rxring) != 0)
		rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu16 "\n",
				portid);

	lcore_count = rte_lcore_count();
	printf("Number of logical cores: %u\n", lcore_count);
	/*if (rte_lcore_count() > 1)
		printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");*/
}

void dpdk_free() {
	unsigned portid = 0;
	/* closing and releasing resources */
	struct rte_flow_error error;
	rte_flow_flush(portid, &error);
	rte_eth_dev_stop(portid);
	rte_eth_dev_close(portid);
}

void encode_mbuf(struct rte_mbuf *mbuf, uint8_t *srcmac, uint8_t *dstmac, const char *srcip, const char *dstip, uint16_t srcport, uint16_t dstport, char *payload, uint32_t payload_size) {
	//struct ether_hdr *ethhdr;
	//struct ipv4_hdr *iphdr;
	//struct udp_hdr *udphdr;
	char *data;
	uint32_t pktsize = 0;
	char *payload_begin;

	data = rte_pktmbuf_mtod(mbuf, char *);

	/*ethhdr = (struct ether_hdr *)data;
	rte_memcpy(ethhdr->d_addr.addr_bytes, dstmac, 6);
	rte_memcpy(ethhdr->s_addr.addr_bytes, srcmac, 6);
	ethhdr->ether_type = 0x0008;
	pktsize += sizeof(ether_hdr);

	iphdr = (struct ipv4_hdr *)(data + pktsize);
	iphdr->version_ihl = (0x04 << 4 | 0x05);
	iphdr->type_of_service = 0;
	//iphdr->total_length = 0;
	iphdr->packet_id = 0x0100;
	iphdr->fragment_offset = 0x00;
	iphdr->time_to_live = 0x40;
	iphdr->next_proto_id = 0x11;
	iphdr->hdr_checksum = 0;
	iphdr->src_addr = inet_addr(srcip);
	iphdr->dst_addr = inet_addr(dstip);
	pktsize += sizeof(struct ipv4_hdr);

	udphdr = (struct udp_hdr *)(data + pktsize);
	udphdr->src_port = htons(srcport);
	udphdr->dst_port = htons(dstport);
	udphdr->dgram_len = 0;
	udphdr->dgram_cksum = 0;
	pktsize += sizeof(udp_hdr);*/

	/*UNUSED(srcmac);
	UNUSED(dstmac);
	UNUSED(srcip);
	UNUSED(dstip);*/
	*(uint16_t*)data = htons(srcport);
	*(uint16_t*)(data+2) = htons(dstport);
	*(uint16_t*)(data+4) = htons(payload_size+6);
	pktsize += 6;

	payload_begin = data + pktsize;
	rte_memcpy(payload_begin, payload, payload_size);
	pktsize += payload_size;

	//iphdr->total_length = htons(sizeof(struct ipv4_hdr) + sizeof(struct udp_hdr) + payload_size);
	//udphdr->dgram_len = htons(sizeof(struct udp_hdr) + payload_size);

	//iphdr->hdr_checksum = checksum((uint16_t *)iphdr, sizeof(struct ipv4_hdr));
	//udphdr->dgram_cksum = udp4_checksum(iphdr, udphdr, payload, payload_size);

	printf("pktsize: %d\n", pktsize);
	dump_buf(data, pktsize);

	mbuf->data_len = pktsize;
	mbuf->pkt_len = pktsize;
}

int decode_mbuf(volatile struct rte_mbuf *mbuf, uint8_t *srcmac, uint8_t *dstmac, char *srcip, char *dstip, uint16_t *srcport, uint16_t *dstport, char *payload) {
	//struct ether_hdr *ethhdr;
	//struct ipv4_hdr *iphdr;
	//struct udp_hdr *udphdr;
	char *data;
	//struct in_addr tmp_ipaddr;
	//char * tmp_ipstr;
	uint32_t payload_size;
	char *payload_begin;

	data = rte_pktmbuf_mtod(mbuf, char *);

	printf("pktsize: %d\n", mbuf->pkt_len);
	dump_buf(data, mbuf->pkt_len);

	/*ethhdr = (struct ether_hdr *)data;
	if (ethhdr->ether_type != 0x0008) {
		return -1;
	}
	rte_memcpy(dstmac, ethhdr->d_addr.addr_bytes, 6);
	rte_memcpy(srcmac, ethhdr->s_addr.addr_bytes, 6);

	iphdr = (struct ipv4_hdr *)(data + sizeof(ether_hdr));
	if (iphdr->next_proto_id != 0x11) {
		return -1;
	}
	tmp_ipaddr.s_addr = iphdr->src_addr;
	tmp_ipstr = inet_ntoa(tmp_ipaddr);
	rte_memcpy(srcip, tmp_ipstr, strlen(tmp_ipstr));
	tmp_ipaddr.s_addr = iphdr->dst_addr;
	tmp_ipstr = inet_ntoa(tmp_ipaddr);
	rte_memcpy(dstip, tmp_ipstr, strlen(tmp_ipstr));

	udphdr = (struct udp_hdr *)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr));
	*srcport = ntohs(udphdr->src_port);
	*dstport = ntohs(udphdr->dst_port);
	payload_size = ntohs(udphdr->dgram_len) - sizeof(udp_hdr);*/

	*srcport = ntohs(*(uint16_t*)data);
	*dstport = ntohs(*(uint16_t*)(data+2));
	payload_size = ntohs(*(uint16_t*)(data+4)) - 6;

	//payload_begin = data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + sizeof(udp_hdr);
	payload_begin = data + 6;
	rte_memcpy(payload, payload_begin, payload_size);
	return payload_size;
}

int get_dstport(volatile struct rte_mbuf *mbuf) {
	//struct ether_hdr *ethhdr;
	//struct ipv4_hdr *iphdr;
	//struct udp_hdr *udphdr;
	char *data;

	data = rte_pktmbuf_mtod(mbuf, char *);

	/*ethhdr = (struct ether_hdr *)data;
	if (ethhdr->ether_type != 0x0008) {
		return -1;
	}

	iphdr = (struct ipv4_hdr *)(data + sizeof(ether_hdr));
	if (iphdr->next_proto_id != 0x11) {
		return -1;
	}

	udphdr = (struct udp_hdr *)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr));
	return ntohs(udphdr->dst_port);*/

	return ntohs(*(uint16_t*)(data+2));
}

int get_srcport(volatile struct rte_mbuf *mbuf) {
	//struct ether_hdr *ethhdr;
	//struct ipv4_hdr *iphdr;
	//struct udp_hdr *udphdr;
	char *data;

	data = rte_pktmbuf_mtod(mbuf, char *);

	/*ethhdr = (struct ether_hdr *)data;
	if (ethhdr->ether_type != 0x0008) {
		return -1;
	}

	iphdr = (struct ipv4_hdr *)(data + sizeof(ether_hdr));
	if (iphdr->next_proto_id != 0x11) {
		return -1;
	}

	udphdr = (struct udp_hdr *)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr));
	return ntohs(udphdr->src_port);*/

	return ntohs(*(uint16_t*)data);
}

int get_payload(volatile struct rte_mbuf *mbuf, char *payload) {
	//struct ether_hdr *ethhdr;
	//struct ipv4_hdr *iphdr;
	//struct udp_hdr *udphdr;
	char *data;
	uint32_t payload_size;
	char *payload_begin;

	data = rte_pktmbuf_mtod(mbuf, char *);

	printf("pktsize: %d\n", mbuf->pkt_len);
	dump_buf(data, mbuf->pkt_len);

	/*ethhdr = (struct ether_hdr *)data;
	if (ethhdr->ether_type != 0x0008) {
		return -1;
	}

	iphdr = (struct ipv4_hdr *)(data + sizeof(ether_hdr));
	if (iphdr->next_proto_id != 0x11) {
		return -1;
	}

	udphdr = (struct udp_hdr *)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr));
	payload_size = ntohs(udphdr->dgram_len) - sizeof(udp_hdr);*/

	//payload_begin = data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + sizeof(udp_hdr);
	payload_begin = data + 6;
	payload_size = ntohs(*(uint16_t*)(data+4)) - 6;
	rte_memcpy(payload, payload_begin, payload_size);
	return payload_size;
}
