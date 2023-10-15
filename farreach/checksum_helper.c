#include "checksum_helper.h"

inline uint16_t checksum (uint16_t *addr, int len) {
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

#ifdef DPDK_CHECKSUM
inline uint16_t udp4_checksum (struct ipv4_hdr* iph, struct udp_hdr* udph, char *payload, int payloadlen) {
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
#else
inline uint16_t udp4_checksum (struct iphdr* iph, struct udphdr* udph, char *payload, int payloadlen) {
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
#endif
