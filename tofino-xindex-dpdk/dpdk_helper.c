#include "dpdk_helper.h"

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

static inline uint16_t udp4_checksum (struct iphdr* iph, struct udphdr* udph, char *payload, int payloadlen) {
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

static inline
void rte_eal_init_helper(int *argc, char ***argv) {
	/* Initialize the Environment Abstraction Layer (EAL). */
	int ret = rte_eal_init(*argc, *argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	*argc -= ret;
	*argv += ret;
}

static inline 
int port_init(uint16_t port, struct rte_mempool *mbuf_pool, uint16_t n_txring) {
	struct rte_eth_conf port_conf = port_conf_default;
	const uint16_t rx_rings = 1, tx_rings = n_txring;
	uint16_t nb_rxd = RX_RING_SIZE;
	uint16_t nb_txd = TX_RING_SIZE;
	int retval;
	uint16_t q;
	struct rte_eth_dev_info dev_info;
	struct rte_eth_txconf txconf;

	if (!rte_eth_dev_is_valid_port(port))
		return -1;

	retval = rte_eth_dev_info_get(port, &dev_info);
	if (retval != 0) {
		printf("Error during getting device (port %u) info: %s\n",
				port, strerror(-retval));
		return retval;
	}

	if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
		port_conf.txmode.offloads |=
			DEV_TX_OFFLOAD_MBUF_FAST_FREE;

	/* Configure the Ethernet device. */
	retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
	if (retval != 0)
		return retval;

	retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
	if (retval != 0)
		return retval;

	/* Allocate and set up 1 RX queue per Ethernet port. */
	for (q = 0; q < rx_rings; q++) {
		retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
				rte_eth_dev_socket_id(port), NULL, mbuf_pool);
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

	/* Start the Ethernet port. */
	retval = rte_eth_dev_start(port);
	if (retval < 0)
		return retval;

	/* Display the port MAC address. */
	struct rte_ether_addr addr;
	retval = rte_eth_macaddr_get(port, &addr);
	if (retval != 0)
		return retval;

	printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
			   " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
			port,
			addr.addr_bytes[0], addr.addr_bytes[1],
			addr.addr_bytes[2], addr.addr_bytes[3],
			addr.addr_bytes[4], addr.addr_bytes[5]);

	/* Enable RX in promiscuous mode for the Ethernet device. */
	retval = rte_eth_promiscuous_enable(port);
	if (retval != 0)
		return retval;

	return 0;
}

static inline 
void dpdk_init(struct rte_mempool **mbuf_pool_ptr, uint16_t n_txring) {
	unsigned nb_ports;
	unsigned lcore_count;

	/* Check that there is an even number of ports to send/receive on. */
	nb_ports = rte_eth_dev_count_avail();
	/*if (nb_ports < 2 || (nb_ports & 1))
		rte_exit(EXIT_FAILURE, "Error: number of ports must be even\n");*/
	printf("Available number of ports: %u\n", nb_ports);

	/* Creates a new mempool in memory to hold the mbufs. */
	*mbuf_pool_ptr = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
		MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

	/* Initialize all ports. */
	RTE_ETH_FOREACH_DEV(portid) {
		if (port_init(portid, *mbuf_pool_ptr, n_txring) != 0)
			rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu16 "\n",
					portid);
	}

	lcore_count = rte_lcore_count();
	printf("Number of logical cores: %u\n", lcore_count);
	/*if (rte_lcore_count() > 1)
		printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");*/
}

static inline 
void encode_mbuf(struct rte_mbuf *mbuf, uint8_t *srcmac, uint8_t *dstmac, std::string srcip, std::string dstip, uint16_t srcport, uint16_t dstport, char *payload, uint32_t payload_size) {
	struct rte_ether_hdr *eth_hdr;
	struct rte_ipv4_hdr *ipv4_hdr;
	struct rte_udp_hdr *udp_hdr;
	char *data;
	uint32_t pktsize = 0;

	data = rte_pktmbuf_mtod(mbuf, char *);

	eth_hdr = (struct rte_ether_hdr *)data;
	rte_memcpy(eth_hdr->d_addr.addr_bytes, dstmac, 6);
	rte_memcpy(eth_hdr->s_addr.addr_bytes, srcmac, 6);
	eth_hdr->ether_type = 0x0800;
	pktsize += sizeof(rte_ether_hdr);

	ipv4_hdr = (struct rte_ipv4_hdr *)(data + pktsize);
	ipv4_hdr->version_ihl = (0x04 << 4 | 0x05);
	ipv4_hdr->type_of_service = 0;
	//ipv4_hdr->total_length = 0;
	ipv4_hdr->packet_id = 0x0100;
	ipv4_hdr->fragment_offset = 0x00;
	ipv4_hdr->time_to_live = 0x40;
	ipv4_hdr->next_proto_id = 0x11;
	ipv4_hdr->hdr_checksum = 0;
	ipv4_hdr->src_addr = inet_addr(srcip.c_str());
	ipv4_hdr->dst_addr = inet_addr(dstip.c_str());
	pktsize += sizeof(rte_ipv4_hdr);

	udp_hdr = (struct rte_udp_hdr *)(data + pktsize);
	udp_hdr->src_port = htons(srcport);
	udp_hdr->dst_port = htons(dstport);
	udp_hdr->dgram_len = 0;
	udp_hdr->dgram_cksum = 0;
	pktsize += sizeof(rte_udp_hdr);

	payload_begin = data + pktsize;
	rte_memcpy(payload_begin, payload, payload_size);
	pktsize += payload_size;

	ipv4_hdr->total_length = sizeof(rte_ipv4_hdr) + sizeof(rte_udp_hdr) + payload_size;
	udp_hdr->dgram_len = sizeof(rte_udp_hdr) + payload_size;

	ipv4_hdr->hdr_checksum = checksum((uint16_t *)ipv4_hdr, sizeof(rte_ipv4_hdr));
	udp_hdr->dgram_cksum = udp4_checksum(ipv4_hdr, udp_hdr, payload, payload_size);

	mbuf->data_len = pktsize;
	mbuf->pkt_len = pktsize;
}

static inline 
int decode_mbuf(struct rte_mbuf *mbuf, uint8_t *srcmac, uint8_t *dstmac, char *srcip, char *dstip, uint16_t *srcport, uint16_t *dstport, char *payload) {
	struct rte_ether_hdr *eth_hdr;
	struct rte_ipv4_hdr *ipv4_hdr;
	struct rte_udp_hdr *udp_hdr;
	char *data;
	struct in_addr tmp_ipaddr;
	char *tmp_ipstr;
	uint32_t payload_size;

	data = rte_pktmbuf_mtod(mbuf, char *);

	eth_hdr = (struct rte_ether_hdr *)data;
	if (eth_hdr->ether_type != 0x0800) {
		return -1;
	}
	rte_memcpy(dstmac, eth_hdr->d_addr.addr_bytes, 6);
	rte_memcpy(srcmac, eth_hdr->s_addr.addr_bytes, 6);

	ipv4_hdr = (struct rte_ipv4_hdr *)(data + sizeof(rte_ether_hdr));
	if (ipv4_hdr->next_proto_id != 0x11) {
		return -1;
	}
	tmp_ipaddr.addr = ipv4_hdr->src_addr;
	tmp_ipstr = inet_ntoa(tmp_ipaddr);
	rte_memcpy(srcip, tmp_ipstr, 4);
	tmp_ipaddr.addr = ipv4_hdr->dst_addr;
	tmp_ipstr = inet_ntoa(tmp_ipaddr);
	rte_memcpy(dstip, tmp_ipstr, 4);

	udp_hdr = (struct rte_udp_hdr *)(data + sizeof(rte_ether_hdr) + sizeof(rte_ipv4_hdr));
	*srcport = ntohs(udp_hdr->src_port);
	*dstport = ntohs(udp_hdr->dst_port);
	payload_size = udp_hdr->dgram_len - sizeof(rte_udp_hdr);

	payload_begin = data + sizeof(rte_ether_hdr) + sizeof(rte_ipv4_hdr) + sizeof(rte_udp_hdr);
	rte_memcpy(payload, payload_begin, payload_size);
	return payload_size;
}

static inline
int get_dstport(struct rte_mbuf *mbuf) {
	struct rte_ether_hdr *eth_hdr;
	struct rte_ipv4_hdr *ipv4_hdr;
	struct rte_udp_hdr *udp_hdr;
	char *data;

	data = rte_pktmbuf_mtod(mbuf, char *);

	eth_hdr = (struct rte_ether_hdr *)data;
	if (eth_hdr->ether_type != 0x0800) {
		return -1;
	}

	ipv4_hdr = (struct rte_ipv4_hdr *)(data + sizeof(rte_ether_hdr));
	if (ipv4_hdr->next_proto_id != 0x11) {
		return -1;
	}

	udp_hdr = (struct rte_udp_hdr *)(data + sizeof(rte_ether_hdr) + sizeof(rte_ipv4_hdr));
	return ntohs(udp_hdr->dst_port);
}

static inline 
int decode_mbuf(struct rte_mbuf *mbuf, char *payload) {
	struct rte_ether_hdr *eth_hdr;
	struct rte_ipv4_hdr *ipv4_hdr;
	struct rte_udp_hdr *udp_hdr;
	char *data;
	uint32_t payload_size;

	data = rte_pktmbuf_mtod(mbuf, char *);

	eth_hdr = (struct rte_ether_hdr *)data;
	if (eth_hdr->ether_type != 0x0800) {
		return -1;
	}

	ipv4_hdr = (struct rte_ipv4_hdr *)(data + sizeof(rte_ether_hdr));
	if (ipv4_hdr->next_proto_id != 0x11) {
		return -1;
	}

	udp_hdr = (struct rte_udp_hdr *)(data + sizeof(rte_ether_hdr) + sizeof(rte_ipv4_hdr));
	payload_size = udp_hdr->dgram_len - sizeof(rte_udp_hdr);

	payload_begin = data + sizeof(rte_ether_hdr) + sizeof(rte_ipv4_hdr) + sizeof(rte_udp_hdr);
	rte_memcpy(payload, payload_begin, payload_size);
	return payload_size;
}
