#include "pkt_ring_buffer.h"

PktRingBuffer::PktRingBuffer() {
}

PktRingBuffer::PktRingBuffer(uint32_t tmpcapacity) {
	init(tmpcapacity);
}

void PktRingBuffer::init(uint32_t tmpcapacity) {
	valid_list.resize(tmpcapacity, false);
	optype_list.resize(tmpcapacity, packet_type_t(0));
	key_list.resize(tmpcapacity, netreach_key_t());
	dynamicbuf_list.resize(tmpcapacity);
	for (uint32_t i = 0; i < tmpcapacity; i++) {
		dynamicbuf_list[i].init(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
	}
	curfragnum_list.resize(tmpcapacity, 0);
	maxfragnum_list.resize(tmpcapacity, 0);
	clientaddr_list.resize(tmpcapacity);
	clientaddrlen_list.resize(tmpcapacity, sizeof(struct sockaddr_in));
	clientlogicalidx_list.resize(tmpcapacity, 0);
	fragseq_list.resize(tmpcapacity, 0);

	head = 0;
	tail = 0;
	capacity = tmpcapacity;
}

bool PktRingBuffer::push(const packet_type_t &optype, const netreach_key_t &key, char *buf, uint32_t bufsize, const struct sockaddr_in &clientaddr, const socklen_t &clientaddrlen) {
	INVARIANT(buf != NULL);

	if (((head + 1) % capacity) == tail) {
		return false;
	}

	// push into ring buffer
	valid_list[head] = true;
	optype_list[head] = optype;
	key_list[head] = key;
	dynamicbuf_list[head].clear();
	dynamicbuf_list[head].dynamic_memcpy(0, buf, bufsize);
	curfragnum_list[head] = 0;
	maxfragnum_list[head] = 0;
	clientaddr_list[head] = clientaddr;
	clientaddrlen_list[head] = clientaddrlen;
	clientlogicalidx_list[head] = 0;
	fragseq_list[head] = 0;

	// TMPDEBUG
	//printf("push key %x into bufidx %d\n", key.keyhihi, head);
	//fflush(stdout);

	// NOTE: small packet does NOT need to update clientlogicalidx_bufidx_map
	
	// update head
	head = (head + 1) % capacity;
	return true;
}

bool PktRingBuffer::is_clientlogicalidx_exist(uint16_t clientlogicalidx) {
	std::map<uint16_t, uint32_t>::iterator iter = clientlogicalidx_bufidx_map.find(clientlogicalidx);
	if (iter == clientlogicalidx_bufidx_map.end()) {
		return false;
	}
	else {
		uint32_t bufidx = iter->second;
		INVARIANT(valid_list[bufidx] == true);
		INVARIANT(maxfragnum_list[bufidx] > 0); // aka a large packet/request
		return true;
	}
}

bool PktRingBuffer::push_large(const packet_type_t &optype, const netreach_key_t &key, char *fraghdr_buf, uint32_t fraghdr_bufsize, uint32_t fragbody_off, char *fragbody_buf, uint32_t fragbody_bufsize, uint16_t curfragnum, uint16_t maxfragnum, const struct sockaddr_in &clientaddr, const socklen_t &clientaddrlen, uint16_t clientlogicalidx, uint32_t fragseq) {
	INVARIANT(fraghdr_buf != NULL);
	INVARIANT(fragbody_buf != NULL);
	INVARIANT(maxfragnum > 0);

	if (((head + 1) % capacity) == tail) {
		return false;
	}

	// push into ring buffer
	valid_list[head] = true;
	optype_list[head] = optype;
	key_list[head] = key;
	dynamicbuf_list[head].clear();
	dynamicbuf_list[head].dynamic_memcpy(0, fraghdr_buf, fraghdr_bufsize);
	dynamicbuf_list[head].dynamic_memcpy(fragbody_off, fragbody_buf, fragbody_bufsize);
	curfragnum_list[head] = curfragnum; // NOTE: for new large packet, curfragnum = 1; for pushed-back large packet, curfragnum < maxfragnum
	maxfragnum_list[head] = maxfragnum;
	clientaddr_list[head] = clientaddr;
	clientaddrlen_list[head] = clientaddrlen;
	clientlogicalidx_list[head] = clientlogicalidx;
	fragseq_list[head] = fragseq;

	// TMPDEBUG
	//printf("push large key %x into bufidx %d\n", key.keyhihi, head);
	//fflush(stdout);

	// NOTE: large packet NEEDs to update clientlogicalidx_bufidx_map
	INVARIANT(clientlogicalidx_bufidx_map.find(clientlogicalidx) == clientlogicalidx_bufidx_map.end());
	clientlogicalidx_bufidx_map.insert(std::pair<uint16_t, uint32_t>(clientlogicalidx, head));
	
	// update head
	head = (head + 1) % capacity;
	return true;
}
		
void PktRingBuffer::update_large(const packet_type_t &optype, const netreach_key_t &key, char *fraghdr_buf, uint32_t fraghdr_bufsize, uint32_t fragbody_off, char *fragbody_buf, uint32_t fragbody_bufsize, const struct sockaddr_in &clientaddr, const socklen_t &clientaddrlen, uint16_t clientlogicalidx, uint32_t fragseq, bool is_frag0) {
	INVARIANT(fraghdr_buf != NULL);
	INVARIANT(fragbody_buf != NULL);

	std::map<uint16_t, uint32_t>::iterator iter = clientlogicalidx_bufidx_map.find(clientlogicalidx);
	INVARIANT(iter != clientlogicalidx_bufidx_map.end());

	uint32_t bufidx = iter->second;

	// TMPDEBUG
	//printf("update large key %x into bufidx %d\n", key.keyhihi, bufidx);
	//fflush(stdout);

	INVARIANT(valid_list[bufidx] == true);
	INVARIANT(is_same_optype(optype_list[bufidx], optype));

	/*if (key_list[bufidx] != key) { // TMPDEBUG
		printf("[ERROR] key_list[%d] should be %x, while deparsed key is %x\n", bufidx, key_list[bufidx].keyhihi, key.keyhihi);
		fflush(stdout);
	}*/
	INVARIANT(key_list[bufidx] == key);

	dynamicbuf_list[bufidx].dynamic_memcpy(fragbody_off, fragbody_buf, fragbody_bufsize);
	curfragnum_list[bufidx] += 1;
	INVARIANT(maxfragnum_list[bufidx] > 0);
	//INVARIANT(clientaddr_list[bufidx] == clientaddr);
	INVARIANT(clientaddrlen_list[bufidx] == clientaddrlen);
	INVARIANT(clientlogicalidx_list[bufidx] == clientlogicalidx);
	INVARIANT(fragseq >= fragseq_list[bufidx]);

	if (fragseq > fragseq_list[bufidx]) {
		printf("[WARNING] pkt_ring_buffer receives larger fragseq %d > %d of key %x from client %d due to client-side timeout-and-retry\n", fragseq, fragseq_list[bufidx], key_list[bufidx].keyhihi, clientlogicalidx_list[bufidx]);
		fragseq_list[bufidx] = fragseq;
		curfragnum_list[bufidx] = 1; // treat the current fragment w/ larger fragseq as the first fragment
	}
	if (curfragnum_list[bufidx] == 1 || is_frag0 == true) {
		// memcpy fraghdr again for the first fragment or fragment 0 to ensure correct seq for farreach/distfarreach/netcache/distcache
		dynamicbuf_list[bufidx].dynamic_memcpy(0, fraghdr_buf, fraghdr_bufsize);
	}
}

bool PktRingBuffer::pop(packet_type_t &optype, netreach_key_t &key, dynamic_array_t &dynamicbuf, uint16_t &curfragnum, uint16_t &maxfragnum, struct sockaddr_in &clientaddr, socklen_t &clientaddrlen, uint16_t &clientlogicalidx, uint32_t &fragseq) {
	if (tail == head) {
		return false;
	}

	INVARIANT(valid_list[tail] == true);
	valid_list[tail] = false;

	optype = optype_list[tail];
	optype_list[tail] = packet_type_t(0);

	key = key_list[tail];
	key_list[tail] = netreach_key_t();

	// TMPDEBUG
	//printf("pop key %x from bufidx %d\n", key.keyhihi, tail);
	//fflush(stdout);

	dynamicbuf.clear();
	dynamicbuf.dynamic_memcpy(0, dynamicbuf_list[tail].array(), dynamicbuf_list[tail].size());
	dynamicbuf_list[tail].clear();

	curfragnum = curfragnum_list[tail];
	curfragnum_list[tail] = 0;

	maxfragnum = maxfragnum_list[tail];
	maxfragnum_list[tail] = 0;

	clientaddr = clientaddr_list[tail];
	clientaddrlen = clientaddrlen_list[tail];
	clientaddrlen_list[tail] = 0;

	clientlogicalidx = clientlogicalidx_list[tail];
	clientlogicalidx_list[tail] = 0;

	fragseq = fragseq_list[tail];
	fragseq_list[tail] = 0;

	if (maxfragnum > 0) { // popping a large packet now
		if (clientlogicalidx_bufidx_map.find(clientlogicalidx) != clientlogicalidx_bufidx_map.end()) {
			clientlogicalidx_bufidx_map.erase(clientlogicalidx);
		}
	}

	// update tail
	tail = (tail + 1) % capacity;
	return true;
}
