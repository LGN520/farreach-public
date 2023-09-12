#ifndef DPDK_HELPER_H
#define DPDK_HELPER_H

#include <stdint.h>
#include <string>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_flow.h>
#include <arpa/inet.h>

#include "../common/key.h"
#include "checksum_helper.h"

#define MAX_LCORE_NUM 72

#define NUM_MBUFS 4096 * 64
//#define NUM_MBUFS 4096
#define MEMPOOL_CACHE_SIZE 256
#define CORE_MAX_BUF_SIZE 1536
#define MCORE_MAX_BUF_SIZE (CORE_MAX_BUF_SIZE + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM)

#define RX_RING_DESCS 1024
#define TX_RING_DESCS 1024

#define RX_BURST_SIZE 32

#define MAX_PATTERN_NUM 4
#define MAX_ACTION_NUM 2

// Internal var
// Designated initializer is not suported by g++ 7.5.0
/*static const struct rte_eth_conf port_conf_default = {
	.rxmode = {
		.max_rx_pkt_len = RTE_ETHER_MAX_LEN,
	},
};*/
//static struct rte_eth_conf port_conf_default;

// Internal funcs
//static inline int port_init(uint16_t port, struct rte_mempool *mbuf_pool, uint16_t n_txring);

void dpdk_eal_init(int *argc, char ***argv);
void dpdk_port_init(uint16_t portid, uint16_t n_txring, uint16_t n_rxring);
void dpdk_queue_setup(uint16_t portid, uint16_t queueid, struct rte_mempool ** tx_mbufpool_ptr);
void dpdk_port_start(uint16_t portid);
void dpdk_free();

void encode_mbuf(struct rte_mbuf *mbuf, uint8_t *srcmac, uint8_t *dstmac, const char *srcip, const char *dstip, uint16_t srcport, uint16_t dstport, char *payload, uint32_t payload_size);
int decode_mbuf(struct rte_mbuf * volatile mbuf, uint8_t *srcmac, uint8_t *dstmac, char *srcip, char *dstip, uint16_t *srcport, uint16_t *dstport, char *payload);
// Used by receiver in client side (mbuf is shared by receiver and workers)
int get_srcport(struct rte_mbuf * volatile mbuf);
int get_dstport(struct rte_mbuf * volatile mbuf);
int get_payload(struct rte_mbuf * volatile mbuf, char *payload);
int8_t get_optype(struct rte_mbuf * volatile mbuf);
bool get_scan_keys(struct rte_mbuf * volatile mbuf, Key *startkey, Key *endkey, uint32_t *num);
void set_scan_keys(struct rte_mbuf * volatile mbuf, Key *startkey, Key *endkey, uint32_t *num);

void generate_udp_fdir_rule(uint16_t port_id, uint16_t rx_queue_id, uint16_t dst_port);
uint16_t receive_pkts(uint16_t port_id, uint16_t rx_queue_id, struct rte_mbuf ** rx_pkts, uint16_t nb_pkts, uint16_t expected_udp_dstport);

#endif
