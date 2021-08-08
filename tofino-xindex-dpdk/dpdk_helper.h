#ifndef DPDK_HELPER_H
#define DPDK_HELPER_H

#include <stdint.h>
#include <string>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <arpa/inet.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250

// Internal var
// Designated initializer is not suported by g++ 7.5.0
/*static const struct rte_eth_conf port_conf_default = {
	.rxmode = {
		.max_rx_pkt_len = RTE_ETHER_MAX_LEN,
	},
};*/
//static struct rte_eth_conf port_conf_default;

// Internal funcs
//static inline uint16_t checksum (uint16_t *addr, int len);
//static inline uint16_t udp4_checksum (struct iphdr* iph, struct udphdr* udph, char *payload, int payloadlen);
//static inline int port_init(uint16_t port, struct rte_mempool *mbuf_pool, uint16_t n_txring);

void rte_eal_init_helper(int *argc, char ***argv);
void dpdk_init(struct rte_mempool **mbuf_pool_ptr, uint16_t n_txring);

void encode_mbuf(struct rte_mbuf *mbuf, uint8_t *srcmac, uint8_t *dstmac, std::string srcip, std::string dstip, uint16_t srcport, uint16_t dstport, char *payload, uint32_t payload_size);
int decode_mbuf(struct rte_mbuf *mbuf, uint8_t *srcmac, uint8_t *dstmac, char *srcip, char *dstip, uint16_t *srcport, uint16_t *dstport, char *payload);
// Used by receiver in client side (mbuf is shared by receiver and workers)
int get_dstport(volatile struct rte_mbuf *mbuf);
int get_payload(volatile struct rte_mbuf *mbuf, char *payload);

#endif
