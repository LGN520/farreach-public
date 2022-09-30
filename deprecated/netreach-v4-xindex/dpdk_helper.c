#include "dpdk_helper.h" 
#include "helper.h"
#include "packet_format.h"

static struct rte_eth_conf port_conf_default;

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

void dpdk_eal_init(int *argc, char ***argv) {
	/* Initialize the Environment Abstraction Layer (EAL). */
	int ret = rte_eal_init(*argc, *argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	//*argc -= ret;
	//*argv += ret;

	port_conf_default.link_speeds = ETH_LINK_SPEED_40G;
	struct rte_eth_rxmode tmp_rxmode;
	tmp_rxmode.mq_mode = ETH_MQ_RX_NONE;
	//tmp_rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
	tmp_rxmode.max_rx_pkt_len = CORE_MAX_BUF_SIZE;
	tmp_rxmode.split_hdr_size = 0;
	tmp_rxmode.offloads = DEV_RX_OFFLOAD_JUMBO_FRAME;
	port_conf_default.rxmode = tmp_rxmode;

	struct rte_eth_txmode tmp_txmode;
	tmp_txmode.mq_mode = ETH_MQ_TX_NONE;
	tmp_txmode.offloads = DEV_TX_OFFLOAD_VLAN_INSERT |
				DEV_TX_OFFLOAD_IPV4_CKSUM |
				DEV_TX_OFFLOAD_MBUF_FAST_FREE;
	/*tmp_txmode.offloads = DEV_TX_OFFLOAD_VLAN_INSERT |
				DEV_TX_OFFLOAD_IPV4_CKSUM  |
				DEV_TX_OFFLOAD_UDP_CKSUM   |
				DEV_TX_OFFLOAD_TCP_CKSUM   |
				DEV_TX_OFFLOAD_SCTP_CKSUM  |
				DEV_TX_OFFLOAD_TCP_TSO;*/
	port_conf_default.txmode = tmp_txmode;
}

void dpdk_port_init(uint16_t portid, uint16_t n_txring, uint16_t n_rxring) {
	// (1) check port status

	/* Check that there is an even number of ports to send/receive on. */
	unsigned nb_ports = rte_eth_dev_count_avail();
	if (nb_ports == 0)
		rte_exit(EXIT_FAILURE, "No available DPDK port\n");
	printf("Available number of ports: %u, while we only use port %d\n", nb_ports, int(portid));

	if (!rte_eth_dev_is_valid_port(portid))
		rte_exit(EXIT_FAILURE, "Invalid port\n");

	/* Display the port MAC address. */
	struct ether_addr addr;
	rte_eth_macaddr_get(portid, &addr);
	printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
			   " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
			portid,
			addr.addr_bytes[0], addr.addr_bytes[1],
			addr.addr_bytes[2], addr.addr_bytes[3],
			addr.addr_bytes[4], addr.addr_bytes[5]);

	unsigned lcore_count = rte_lcore_count();
	printf("Number of logical cores: %u\n", lcore_count);
	/*if (rte_lcore_count() > 1)
		printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");*/

	// (2) configure port

	struct rte_eth_conf port_conf = port_conf_default;
	int retval;

	struct rte_eth_dev_info dev_info;
	rte_eth_dev_info_get(portid, &dev_info);
	port_conf.txmode.offloads &= dev_info.tx_offload_capa;

	uint16_t nb_rxd = RX_RING_DESCS;
	uint16_t nb_txd = TX_RING_DESCS;
	retval = rte_eth_dev_adjust_nb_rx_tx_desc(portid, &nb_rxd, &nb_txd);
	if (retval != 0)
		rte_exit(EXIT_FAILURE, "Cannot adjust desc number\n");

	/* Configure the Ethernet device. */
	printf("Initialize port %u with %u TX rings and %u RX rings\n", portid, n_txring, n_rxring);
	retval = rte_eth_dev_configure(portid, n_rxring, n_txring, &port_conf);
	if (retval != 0)
		rte_exit(EXIT_FAILURE, "Cannot configure port\n");
}

void dpdk_queue_setup(uint16_t portid, uint16_t queueid, struct rte_mempool ** tx_mbufpool_ptr) {
	struct rte_eth_conf port_conf = port_conf_default;
	int retval;

	struct rte_eth_dev_info dev_info;
	rte_eth_dev_info_get(portid, &dev_info);
	port_conf.txmode.offloads &= dev_info.tx_offload_capa;

	/* Creates a new mempool in memory to hold the mbufs. */
	char txname[256];
	memset(txname, '\0', 256);
	sprintf(txname, "MBUF_POOL_TX_QUEUE_%d", int(queueid));
	//*tx_mbufpool_ptr = rte_pktmbuf_pool_create(txname, NUM_MBUFS,
	//	MEMPOOL_CACHE_SIZE, RTE_MBUF_PRIV_ALIGN, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	*tx_mbufpool_ptr = rte_pktmbuf_pool_create(txname, NUM_MBUFS,
		MEMPOOL_CACHE_SIZE, RTE_MBUF_PRIV_ALIGN, MCORE_MAX_BUF_SIZE, rte_socket_id());
	if (*tx_mbufpool_ptr == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool for TX queue\n");

	struct rte_eth_txconf txconf;
	txconf = dev_info.default_txconf;
	txconf.offloads = port_conf.txmode.offloads;
	/* Allocate and set up 1 TX queue per Ethernet port. */
	retval = rte_eth_tx_queue_setup(portid, queueid, TX_RING_DESCS,
			rte_eth_dev_socket_id(portid), &txconf);
	if (retval < 0)
		rte_exit(EXIT_FAILURE, "Cannot setup TX queue\n");

	/* Creates a new mempool in memory to hold the mbufs. */
	char rxname[256];
	memset(rxname, '\0', 256);
	sprintf(rxname, "MBUF_POOL_RX_QUEUE_%d", int(queueid));
	//struct rte_mempool *rx_mbufpool = rte_pktmbuf_pool_create(rxname, NUM_MBUFS,
	//	MEMPOOL_CACHE_SIZE, RTE_MBUF_PRIV_ALIGN, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	struct rte_mempool *rx_mbufpool = rte_pktmbuf_pool_create(rxname, NUM_MBUFS,
		MEMPOOL_CACHE_SIZE, RTE_MBUF_PRIV_ALIGN, MCORE_MAX_BUF_SIZE, rte_socket_id());
	if (rx_mbufpool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool for RX queue\n");

	struct rte_eth_rxconf rxconf;
	rxconf = dev_info.default_rxconf;
	rxconf.offloads = port_conf.rxmode.offloads;
	rxconf.rx_thresh.pthresh = 16;
	rxconf.rx_thresh.hthresh = 8;
	rxconf.rx_thresh.wthresh = 0;
	/* Allocate and set up 1 RX queue per Ethernet port. */
	retval = rte_eth_rx_queue_setup(portid, queueid, RX_RING_DESCS,
			rte_eth_dev_socket_id(portid), &rxconf, rx_mbufpool);
	if (retval < 0)
		rte_exit(EXIT_FAILURE, "Cannot setup RX queue\n");
}

void dpdk_port_start(uint16_t portid) {
	int retval;

	/* Enable RX in promiscuous mode for the Ethernet device. */
	rte_eth_promiscuous_enable(portid);

	/* Start the Ethernet port. */
	retval = rte_eth_dev_start(portid);
	if (retval < 0)
		rte_exit(EXIT_FAILURE, "Cannot start port\n");

	// Wait until link up
	struct rte_eth_link link;
	memset(&link, 0, sizeof(struct rte_eth_link));
	uint32_t max_repeat_times = 1000;
	uint32_t check_interval_ms = 10;
	for (uint32_t i = 0; i <= max_repeat_times; i++) {
		rte_eth_link_get(portid, &link);
		if (link.link_status == ETH_LINK_UP)
			break;
		rte_delay_ms(check_interval_ms);
	}
	if (link.link_status == ETH_LINK_DOWN) {
		rte_exit(EXIT_FAILURE, "Link is down for port %u\n", portid);
	}
	printf("Initialize port %u done!\n", portid);
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
	struct ether_hdr *ethhdr;
	struct ipv4_hdr *iphdr;
	//struct udp_hdr *udphdr;
	char *data;
	uint32_t pktsize = 0;
	char *payload_begin;

	data = rte_pktmbuf_mtod(mbuf, char *);

	ethhdr = (struct ether_hdr *)data;
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

	/*udphdr = (struct udp_hdr *)(data + pktsize);
	udphdr->src_port = htons(srcport);
	udphdr->dst_port = htons(dstport);
	udphdr->dgram_len = 0;
	udphdr->dgram_cksum = 0;
	pktsize += sizeof(udp_hdr);*/

	*(uint16_t*)(data+pktsize) = htons(srcport);
	*(uint16_t*)(data+pktsize+2) = htons(dstport);
	*(uint16_t*)(data+pktsize+4) = htons(payload_size+6);
	pktsize += 6;

	payload_begin = data + pktsize;
	rte_memcpy(payload_begin, payload, payload_size);
	pktsize += payload_size;

	//iphdr->total_length = htons(sizeof(struct ipv4_hdr) + sizeof(struct udp_hdr) + payload_size);
	iphdr->total_length = htons(sizeof(struct ipv4_hdr) + 6 + payload_size);
	//udphdr->dgram_len = htons(sizeof(struct udp_hdr) + payload_size);

	iphdr->hdr_checksum = checksum((uint16_t *)iphdr, sizeof(struct ipv4_hdr));
	//udphdr->dgram_cksum = udp4_checksum(iphdr, udphdr, payload, payload_size);

	//printf("pktsize: %d\n", pktsize);
	//dump_buf(data, pktsize);

	mbuf->data_len = pktsize;
	mbuf->pkt_len = pktsize;
}

int decode_mbuf(struct rte_mbuf *volatile mbuf, uint8_t *srcmac, uint8_t *dstmac, char *srcip, char *dstip, uint16_t *srcport, uint16_t *dstport, char *payload) {
	struct ether_hdr *ethhdr;
	struct ipv4_hdr *iphdr;
	//struct udp_hdr *udphdr;
	char *data;
	struct in_addr tmp_ipaddr;
	char * tmp_ipstr;
	uint32_t payload_size;
	char *payload_begin;

	data = rte_pktmbuf_mtod(mbuf, char *);

	//printf("pktsize: %d\n", mbuf->pkt_len);
	//dump_buf(data, mbuf->pkt_len);

	ethhdr = (struct ether_hdr *)data;
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

	/*udphdr = (struct udp_hdr *)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr));
	*srcport = ntohs(udphdr->src_port);
	*dstport = ntohs(udphdr->dst_port);
	payload_size = ntohs(udphdr->dgram_len) - sizeof(udp_hdr);*/

	*srcport = ntohs(*(uint16_t*)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr)));
	*dstport = ntohs(*(uint16_t*)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 2));
	payload_size = ntohs(*(uint16_t*)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 4)) - 6;
	//*srcport = ntohs(*(uint16_t*)data);
	//*dstport = ntohs(*(uint16_t*)(data+2));
	//payload_size = ntohs(*(uint16_t*)(data+4)) - 6;

	//payload_begin = data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + sizeof(udp_hdr);
	payload_begin = data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 6;
	//payload_begin = data + 6;
	rte_memcpy(payload, payload_begin, payload_size);
	return payload_size;
}

int get_dstport(struct rte_mbuf *volatile mbuf) {
	//struct ether_hdr *ethhdr;
	//struct ipv4_hdr *iphdr;
	//struct udp_hdr *udphdr;
	char *data;

	data = rte_pktmbuf_mtod(mbuf, char *);

	//printf("pktsize: %d\n", mbuf->pkt_len);
	//dump_buf(data, mbuf->pkt_len);

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

	//return ntohs(*(uint16_t*)(data+2));
	return ntohs(*(uint16_t*)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 2));
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

	//return ntohs(*(uint16_t*)data);
	return ntohs(*(uint16_t*)(data+sizeof(ether_hdr) + sizeof(ipv4_hdr)));
}

int get_payload(struct rte_mbuf *volatile mbuf, char *payload) {
	struct ether_hdr *ethhdr;
	struct ipv4_hdr *iphdr;
	//struct udp_hdr *udphdr;
	char *data;
	uint32_t payload_size;
	char *payload_begin;

	data = rte_pktmbuf_mtod(mbuf, char *);

	//printf("pktsize: %d\n", mbuf->pkt_len);
	//dump_buf(data, mbuf->pkt_len);

	ethhdr = (struct ether_hdr *)data;
	if (ethhdr->ether_type != 0x0008) {
		return -1;
	}

	iphdr = (struct ipv4_hdr *)(data + sizeof(ether_hdr));
	if (iphdr->next_proto_id != 0x11) {
		return -1;
	}

	/*udphdr = (struct udp_hdr *)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr));
	payload_size = ntohs(udphdr->dgram_len) - sizeof(udp_hdr);*/

	//payload_begin = data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + sizeof(udp_hdr);
	payload_begin = data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 6;
	payload_size = ntohs(*(uint16_t*)(data+sizeof(ether_hdr) + sizeof(ipv4_hdr)+4)) - 6;
	//payload_begin = data + 6;
	//payload_size = ntohs(*(uint16_t*)(data+4)) - 6;
	rte_memcpy(payload, payload_begin, payload_size);
	return payload_size;
}

int8_t get_optype(struct rte_mbuf * volatile mbuf) {
	char *data;
	data = rte_pktmbuf_mtod(mbuf, char *);
	int8_t optype = *(int8_t*)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 6);
	return optype;
}

/*//bool get_scan_keys(struct rte_mbuf * volatile mbuf, Key *startkey, Key *endkey, int32_t *num) {
bool get_scan_keys(struct rte_mbuf * volatile mbuf, Key *startkey, Key *endkey) {
	char *data;

	data = rte_pktmbuf_mtod(mbuf, char *);

	//printf("pktsize: %d\n", mbuf->pkt_len);
	//dump_buf(data, mbuf->pkt_len);
	packet_type_t optype = packet_type_t(*(int8_t*)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 6));
	if (optype == packet_type_t::SCANREQ || optype == packet_type_t::SCANREQ_SPLIT) {
		uint32_t tmp_keysize = startkey->deserialize(data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 7, ???);
		uint32_t tmp_endkeysize = startkey->deserialize(data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 7 + tmp_keysize, ???);
		UNUSED(tmp_endkeysize)
		// *num = *(int32_t*)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 40);
		return true;
	}
	return false;
}*/

/*void set_scan_keys(struct rte_mbuf * volatile mbuf, Key *startkey, Key *endkey, uint32_t *num) {
	char *data;

	data = rte_pktmbuf_mtod(mbuf, char *);

	//printf("pktsize: %d\n", mbuf->pkt_len);
	//dump_buf(data, mbuf->pkt_len);
	
	uint32_t tmp_keysize = startkey->serialize(data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 7, ???);
	uint32_t tmp_endkeysize = startkey->serialize(data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 7 + tmp_keysize, ???);
	UNUSED(tmp_endkeysize);
	// *(int32_t*)(data + sizeof(ether_hdr) + sizeof(ipv4_hdr) + 40) = *num;
}*/

void generate_udp_fdir_rule(uint16_t port_id, uint16_t rx_queue_id, uint16_t dst_port) {
	struct rte_flow_attr attr;
    struct rte_flow_item pattern[MAX_PATTERN_NUM];
    struct rte_flow_action action[MAX_ACTION_NUM];
    struct rte_flow_action_queue queue;
	queue.index = rx_queue_id;
    struct rte_flow_item_udp udp_spec;
    struct rte_flow_item_udp udp_mask;

	// only check ingress packet
    memset(&attr, 0, sizeof(struct rte_flow_attr));
    attr.ingress = 1;

	// place into the specific queue
	INVARIANT(MAX_ACTION_NUM >= 2);
	action[0].type = RTE_FLOW_ACTION_TYPE_QUEUE;
    action[0].conf = &queue;
    action[1].type = RTE_FLOW_ACTION_TYPE_END;

	INVARIANT(MAX_PATTERN_NUM >= 4);
	// first-level pattern: allow all ethernet header
	pattern[0].type = RTE_FLOW_ITEM_TYPE_ETH;
	// second-level pattern: allow all ipv4 header
	pattern[1].type = RTE_FLOW_ITEM_TYPE_IPV4;
	// third-level pattern: match udp.dstport
	pattern[2].type = RTE_FLOW_ITEM_TYPE_UDP;
	memset(&udp_spec, 0, sizeof(struct rte_flow_item_udp));
	memset(&udp_mask, 0, sizeof(struct rte_flow_item_udp));
	udp_spec.hdr.src_port = 0;
	udp_mask.hdr.src_port = 0; // allow all src ports
	udp_spec.hdr.dst_port = htons(dst_port);
	udp_mask.hdr.dst_port = 0xFFFF; // only allow specific destination port
	pattern[2].spec = &udp_spec;
	pattern[2].mask = &udp_mask;
	pattern[2].last = 0; // disable range match
	// last-level pattern: end of pattern list
	pattern[3].type = RTE_FLOW_ITEM_TYPE_END;

	struct rte_flow_error error;
	int res = rte_flow_validate(port_id, &attr, pattern, action, &error);
	if (res != 0) {
		printf("[dpdk helper] flow validate error: type %d, message %s\n", error.type, error.message ? error.message : "(no stated reason)");
    	rte_exit(EXIT_FAILURE, "flow validate error");
	}

	struct rte_flow *flow = NULL;
	flow = rte_flow_create(port_id, &attr, pattern, action, &error);
	if (flow == NULL) {
		printf("[dpdk helper] flow create error: type %d, message %s\n", error.type, error.message ? error.message : "(no stated reason)");
    	rte_exit(EXIT_FAILURE, "flow create error");
	}
}

// NOTE: for i40e driver, due to vetor instruction, even if we set rx_burst_size=1 after changing RTE_I40E_DESCS_PER_LOOP, it can still return more than 1 packet
// NOTE: as driver returns immediately as long as it encounters a DD_BIT=1, rx_burst_size does not affect latency obviously
uint16_t receive_pkts(uint16_t port_id, uint16_t rx_queue_id, struct rte_mbuf ** rx_pkts, uint16_t nb_pkts, uint16_t expected_udp_dstport) {
	//while (true) {
	uint16_t n_rx = rte_eth_rx_burst(port_id, rx_queue_id, rx_pkts, nb_pkts);
	//if (n_rx == 0) {
	//	continue;
	//}
#ifdef DEBUG_ASSERT
	INVARIANT(n_rx <= nb_pkts);
	for (uint16_t i = 0; i < n_rx; i++) {
		INVARIANT(rx_pkts[i] != NULL);
		INVARIANT(get_dstport(rx_pkts[i]) == expected_udp_dstport);
	}
#endif
	return n_rx;
	//}
	//return 0; // never arrive
}
