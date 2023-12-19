#ifndef CHECKSUM_HELPER_H
#define CHECKSUM_HELPER_H

//#define DPDK_CHECKSUM

#include <stdint.h>
#include <string.h> // memcpy

#ifdef DPDK_CHECKSUM
#include <rte_ip.h> // ipv4_hdr
#include <rte_udp.h> // udp_hdr
#include <rte_ethdev.h> // ether_hdr
#else
#include <netinet/in.h> // in_addr
#include <netinet/ether.h> // ether_header
#include <linux/ip.h> // iphdr
#include <linux/udp.h> // udphdr
#endif

inline uint16_t checksum (uint16_t *addr, int len);
#ifdef DPDK_CHECKSUM
inline uint16_t udp4_checksum (struct ipv4_hdr* iph, struct udp_hdr* udph, char *payload, int payloadlen);
#else
inline uint16_t udp4_checksum (struct iphdr* iph, struct udphdr* udph, char *payload, int payloadlen);
#endif

#endif
