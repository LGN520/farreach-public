#ifndef PKT_RING_BUFFER_H
#define PKT_RING_BUFFER_H

#include <vector>
#include <map>

#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // htons htonl

#include "helper.h"
#include "key.h"
#include "packet_format.h"
#include "dynamic_array.h"

// at most one packet from each logical client ideally
#define PKT_RING_BUFFER_SIZE 2048

class PktRingBuffer {
	public:
		PktRingBuffer();
		PktRingBuffer(uint32_t tmpcapacity);

		void init(uint32_t tmpcapacity);

		// functions

		bool push(const packet_type_t &optype, const netreach_key_t &key, char *buf, uint32_t bufsize, const struct sockaddr_in &clientaddr, const socklen_t &clientaddrlen);

		bool is_clientlogicalidx_exist(uint16_t clientlogicalidx);
		bool push_large(const packet_type_t &optype, const netreach_key_t &key, char *fraghdr_buf, uint32_t fraghdr_bufsize, uint32_t fragbody_off, char *fragbody_buf, uint32_t fragbody_bufsize, uint16_t curfragnum, uint16_t maxfragnum, const struct sockaddr_in &clientaddr, const socklen_t &clientaddrlen, uint16_t clientlogicalidx, uint32_t fragseq);
		void update_large(const packet_type_t &optype, const netreach_key_t &key, char *fraghudr_buf, uint32_t fraghdr_bufsize, uint32_t fragbody_off, char *fragbody_buf, uint32_t fragbody_bufsize, const struct sockaddr_in &clientaddr, const socklen_t &clientaddrlen, uint16_t clientlogicalidx, uint32_t fragseq, bool is_frag0);

		// NOTE: FIFO order
		bool pop(packet_type_t &optype, netreach_key_t &key, dynamic_array_t &dynamicbuf, uint16_t &curfragnum, uint16_t &maxfragnum, struct sockaddr_in &clientaddr, socklen_t &clientaddrlen, uint16_t &clientlogicalidx, uint32_t &fragseq);

		// variables

		std::vector<bool> valid_list;	
		std::vector<packet_type_t> optype_list;
		std::vector<netreach_key_t> key_list;
		std::vector<dynamic_array_t> dynamicbuf_list;
		std::vector<uint16_t> curfragnum_list;
		std::vector<uint16_t> maxfragnum_list;
		std::vector<struct sockaddr_in> clientaddr_list;
		std::vector<socklen_t> clientaddrlen_list;
		std::vector<uint16_t> clientlogicalidx_list;
		std::vector<uint32_t> fragseq_list;

		std::map<uint16_t, uint32_t> clientlogicalidx_bufidx_map;

		uint32_t head;
		uint32_t tail;
		uint32_t capacity;
};

typedef PktRingBuffer pkt_ring_buffer_t;

#endif
