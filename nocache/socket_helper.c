#include "socket_helper.h"

#include <arpa/inet.h> // inetaddr conversion; endianess conversion
#include <netinet/tcp.h> // TCP_NODELAY

// udp/tcp sockaddr

void set_sockaddr(sockaddr_in &addr, uint32_t bigendian_saddr, short littleendian_port) {
	memset((void *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = bigendian_saddr;
	addr.sin_port = htons(littleendian_port);
}

void set_recvtimeout(int sockfd, int timeout_sec, int timeout_usec) {
	struct timeval tv;
	tv.tv_sec = timeout_sec;
	tv.tv_usec =  timeout_usec;
	int res = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	INVARIANT(res >= 0);
}

// udp

void create_udpsock(int &sockfd, bool need_timeout, const char* role, int timeout_sec, int timeout_usec, int udp_rcvbufsize) {
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		printf("[%s] fail to create udp socket, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Disable udp/tcp check
	int disable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		printf("[%s] disable checksum failed, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Set timeout for recvfrom/accept of udp/tcp
	if (need_timeout) {
		set_recvtimeout(sockfd, timeout_sec, timeout_usec);
	}
	// set udp receive buffer size
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &udp_rcvbufsize, sizeof(int)) == -1) {
		printf("[%s] fail to set udp receive bufsize as %d, errno: %d\n", role, udp_rcvbufsize, errno);
		exit(-1);
	}
}

void udpsendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role) {
	int res = sendto(sockfd, buf, len, flags, (struct sockaddr *)dest_addr, addrlen);
	if (res < 0) {
		printf("[%s] sendto of udp socket fails, errno: %d!\n", role, errno);
		exit(-1);
	}
}

bool udprecvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role) {
	bool need_timeout = false;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec =  0;
	socklen_t tvsz = sizeof(tv);
	int res = getsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, &tvsz);
	UNUSED(res);
	if (tv.tv_sec != 0 || tv.tv_usec != 0) {
		need_timeout = true;
	}

	bool is_timeout = false;
	recvsize = recvfrom(sockfd, buf, len, flags, (struct sockaddr *)src_addr, addrlen);
	if (recvsize < 0) {
		if (need_timeout && (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)) {
			recvsize = 0;
			is_timeout = true;
		}
		else {
			printf("[%s] error of recvfrom, errno: %d!\n", role, errno);
			exit(-1);
		}
	}
	return is_timeout;
}

void prepare_udpserver(int &sockfd, bool need_timeout, short server_port, const char* role, int timeout_sec, int timeout_usec, int udp_rcvbufsize) {
	INVARIANT(role != NULL);

	// create socket
	create_udpsock(sockfd, need_timeout, role, timeout_sec, timeout_usec, udp_rcvbufsize);
	// reuse the occupied port for the last created socket instead of being crashed
	const int trueFlag = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
		printf("[%s] fail to setsockopt, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Set listen address
	sockaddr_in listen_addr;
	set_sockaddr(listen_addr, htonl(INADDR_ANY), server_port);
	if ((bind(sockfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr))) != 0) {
		printf("[%s] fail to bind socket on port %hu, errno: %d!\n", role, server_port, errno);
		exit(-1);
	}
}

void udpsendlarge_udpfrag(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role) {
	udpsendlarge(sockfd, buf, len, flags, dest_addr, addrlen, role, 0, UDP_FRAGMENT_MAXSIZE);
}

void udpsendlarge_ipfrag(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role, size_t frag_hdrsize) {
	udpsendlarge(sockfd, buf, len, flags, dest_addr, addrlen, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE);
}

void udpsendlarge(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role, size_t frag_hdrsize, size_t frag_maxsize) {
	// (1) buf[0:frag_hdrsize] + <cur_fragidx, max_fragnum> as final header of each fragment payload
	// (2) frag_maxsize is the max size of fragment payload (final fragment header + fragment body), yet not including ethernet/ipv4/udp header
	INVARIANT(len >= frag_hdrsize);
	size_t final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
	size_t frag_bodysize = frag_maxsize - final_frag_hdrsize;
	size_t total_bodysize = len - frag_hdrsize;
	size_t fragnum = 1;
	if (likely(total_bodysize > 0)) {
		fragnum = (total_bodysize + frag_bodysize - 1) / frag_bodysize;
	}
	INVARIANT(fragnum > 0);
	//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d, total_bodysize: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize, total_bodysize);

	// <frag_hdrsize, cur_fragidx, max_fragnum, frag_bodysize>
	char fragbuf[frag_maxsize];
	memset(fragbuf, 0, frag_maxsize);
	memcpy(fragbuf, buf, frag_hdrsize); // fixed fragment header
	uint16_t cur_fragidx = 0;
	uint16_t max_fragnum = uint16_t(fragnum);
	size_t buf_sentsize = frag_hdrsize;
	for (; cur_fragidx < max_fragnum; cur_fragidx++) {
		memset(fragbuf + frag_hdrsize, 0, frag_maxsize - frag_hdrsize);

		// prepare for final fragment header
		//// NOTE: UDP fragmentation is processed by end-hosts instead of switch -> no need for endianess conversion
		//memcpy(fragbuf + frag_hdrsize, &cur_fragidx, sizeof(uint16_t));
		//memcpy(fragbuf + frag_hdrsize + sizeof(uint16_t), &max_fragnum, sizeof(uint16_t));
		// NOTE: to support large value, switch needs to parse fragidx and fragnum -> NEED endianess conversion now
		uint16_t bigendian_cur_fragidx = htons(cur_fragidx); // littleendian -> bigendian for large value
		memcpy(fragbuf + frag_hdrsize, &bigendian_cur_fragidx, sizeof(uint16_t));
		uint16_t bigendian_max_fragnum = htons(max_fragnum); // littleendian -> bigendian for large value
		memcpy(fragbuf + frag_hdrsize + sizeof(uint16_t), &bigendian_max_fragnum, sizeof(uint16_t));

		// prepare for fragment body
		int cur_frag_bodysize = frag_bodysize;
		if (cur_fragidx == max_fragnum - 1) {
			cur_frag_bodysize = total_bodysize - frag_bodysize * cur_fragidx;
		}
		memcpy(fragbuf + final_frag_hdrsize, buf + buf_sentsize, cur_frag_bodysize);
		buf_sentsize += cur_frag_bodysize;

		//printf("cur_fragidx: %d, max_fragnum: %d, cur_fragsize: %d\n", cur_fragidx, max_fragnum, final_frag_hdrsize + cur_frag_bodysize);
		udpsendto(sockfd, fragbuf, final_frag_hdrsize + cur_frag_bodysize, flags, dest_addr, addrlen, role);
	}
}

bool udprecvlarge_udpfrag(int sockfd, dynamic_array_t &buf, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, const char* role) {
	return udprecvlarge(sockfd, buf, flags, src_addr, addrlen, role, UDP_FRAGMENT_MAXSIZE, UDP_FRAGTYPE, NULL);
}

bool udprecvlarge_ipfrag(int sockfd, dynamic_array_t &buf, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, const char* role, pkt_ring_buffer_t *pkt_ring_buffer_ptr) {
	return udprecvlarge(sockfd, buf, flags, src_addr, addrlen, role, IP_FRAGMENT_MAXSIZE, IP_FRAGTYPE, pkt_ring_buffer_ptr);
}

// NOTE: receive large packet from one source
bool udprecvlarge(int sockfd, dynamic_array_t &buf, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, const char* role, size_t frag_maxsize, int fragtype, pkt_ring_buffer_t *pkt_ring_buffer_ptr) {

	bool is_timeout = false;
	struct sockaddr_in tmp_src_addr;
	socklen_t tmp_addrlen = sizeof(struct sockaddr_in);

	size_t frag_hdrsize = 0;
	size_t final_frag_hdrsize = 0;
	size_t frag_bodysize = 0;

	char fragbuf[frag_maxsize];
	int frag_recvsize = 0;
	bool is_first = true;
	uint16_t max_fragnum = 0;
	uint16_t cur_fragnum = 0;

	// ONLY for udprecvlarge_ipfrag
	bool is_used_by_server = (pkt_ring_buffer_ptr != NULL);
	netreach_key_t largepkt_key;
	packet_type_t largepkt_optype;
	uint16_t largepkt_client_logical_idx; // ONLY for server

	// pop pkt from pkt_ring_buffer if any ONLY for IP_FRAGTYPE by server
	if (fragtype == IP_FRAGTYPE && is_used_by_server) {
		bool has_pkt = pkt_ring_buffer_ptr->pop(largepkt_optype, largepkt_key, buf, cur_fragnum, max_fragnum, tmp_src_addr, tmp_addrlen, largepkt_client_logical_idx);
		if (has_pkt) { // if w/ pkt in pkt_ring_buffer
			// Copy src address of the first packet for both large and not-large packet
			if (src_addr != NULL) {
				*src_addr = tmp_src_addr;
			}
			if (src_addrlen != NULL) {
				*addrlen = tmp_addrlen;
			}

			if (max_fragnum == 0) { // small packet
				return true;
			}
			else { // large packet
				if (cur_fragnum >= max_fragnum) { // w/ all fragments
					return true;
				}
				else { // need to receive remaining fragments
					is_first = false; // we do NOT need to process the first packet
					INVARIANT(is_packet_with_largevalue(largepkt_optype)) == true;
					frag_hdrsize = get_frag_hdrsize(largepkt_optype);
					final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
					frag_bodysize = frag_maxsize - final_frag_hdrsize;
				}
			}
		}
	}

	while (true) {
		if (is_first) {
			is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, &tmp_src_addr, &tmp_addrlen, frag_recvsize, role);
			if (is_timeout) {
				break;
			}

			// Copy src address of the first packet for both large and not-large packet
			if (src_addr != NULL) {
				*src_addr = tmp_src_addr;
			}
			if (src_addrlen != NULL) {
				*addrlen = tmp_addrlen;
			}

			// Judge whether the current packet is large or not
			bool is_largepkt = false;
			if (fragtype == IP_FRAGTYPE) { // NOTE: server and client (ONLY for GETREQ) do NOT know whether the packet will be large or not before udprecvlarge_ipfrag
				packet_type_t tmp_optype = get_packet_type(fragbuf, frag_recvsize);
				is_largepkt = is_packet_with_largevalue(tmp_optype);
				if (is_largepkt) {
					frag_hdrsize = get_frag_hdrsize(tmp_optype);
				}
			}
			else if (fragtype == UDP_FRAGTYPE) {
				frag_hdrsize = 0;
				is_largepkt = true; // SNAPSHOT_GETDATA_ACK or SNAPSHOT_SENDDATA MUST be large packet
			}
			else {
				printf("[ERROR] invalid fragtype in udprecvlarge: %d\n", fragtype);
				exit(-1);
			}

			if (!is_largepkt) {
				// NOT large packet -> jump out of the while loop -> behave similar as udprecvfrom to receive a single small packet
				buf.dynamic_memcpy(0, fragbuf, frag_recvsize);
				break; // <=> return;
			}

			// NOTE: now the current packet MUST be large

			// Save optype and key [and client_logical_idx] ONLY for udprecvlarge_ipfrag to filter unnecessary packet fro client [and server]
			if (fragtype == IP_FRAGTYPE) {
				largepkt_key = get_packet_key(fragbuf, frag_recvsize);
				largepkt_optype = get_packet_type(fragbuf, frag_recvsize);
				if (is_used_by_server) {
					largepkt_client_logical_idx = get_packet_logicalidx(fragbuf, frag_recvsize);
				}
			}

			// Calculate fraghdrsize, final fraghdrsize, and fragbodysize based on optype
			final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
			frag_bodysize = frag_maxsize - final_frag_hdrsize;
			INVARIANT(final_frag_hdrsize != 0 && frag_bodysize != 0);
			INVARIANT(size_t(frag_recvsize) >= final_frag_hdrsize && size_t(frag_recvsize) <= frag_maxsize);
			//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize);

			// Copy fraghdr and get max fragnum
			buf.dynamic_memcpy(0, fragbuf, frag_hdrsize);
			memcpy(&max_fragnum, fragbuf + frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));
			max_fragnum = ntohs(max_fragnum); // bigendian -> littleendian for large value
			is_first = false;
		}
		else { // NOTE: access the following code block ONLY if we are pursuiting a large packet
			is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, &tmp_src_addr, &tmp_addrlen, frag_recvsize, role);
			if (is_timeout) {
				break;
			}
	
			// Filter unexpected packets ONLY for udprecvlarge_ipfrag
			if (fragtype == IP_FRAGTYPE) {
				netreach_key_t tmp_nonfirstpkt_key = get_packet_key(fragbuf, frag_recvsize);
				packet_type_t tmp_nonfirstpkt_optype = get_packet_type(fragbuf, frag_recvsize);
				if (!is_used_by_server) { // used by client
					if (tmp_nonfirstpkt_key != largepkt_key || tmp_nonfirstpkt_optype != largepkt_optype) { // unmatched packet
						continue; // skip current unmatched packet, go to receive next one
					}
					// else {} // matched packet
				}
				else { // used by server
					bool tmp_stat = false;
					if (!is_packet_with_largevalue(tmp_nonfirstpkt_optype)) { // not-large packet
						// Push small request into PktRingBuffer
						tmp_stat = pkt_ring_buffer_ptr->push(tmp_nonfirstpkt_optype, tmp_nonfirstpkt_key, fragbuf, frag_recvsize, tmp_src_addr, tmp_addrlen);
						if (!tmp_stat) {
							printf("[ERROR] overflow of pkt_ring_buffer when push optype %x\n", optype_t(tmp_nonfirstpkt_optype));
							exit(-1);
						}
						continue;
					}
					else { // large packet
						if (is_packet_with_clientlogicalidx(tmp_nonfirstpkt_optype)) { // large packet for server
							uint16_t tmp_nonfirstpkt_clientlogicalidx = get_packet_clientlogicalidx(fragbuf, frag_recvsize);
							if (tmp_nonfirstpkt_clientlogicalidx != largepkt_clientlogicalidx) { // from different client
								// calculate fraghdrsize and fragbodysize for the large packet from different client
								uint32_t tmp_nonfirstpkt_frag_hdrsize = get_frag_hdrsize(tmp_nonfirstpkt_optype);
								uint32_t tmp_nonfirstpkt_final_frag_hdrsize = tmp_nonfirstpkt_frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
								uint32_t tmp_nonfirstpkt_frag_bodysize = frag_maxsize - tmp_nonfirstpkt_final_frag_hdrsize;
								INVARIANT(tmp_nonfirstpkt_final_frag_hdrsize != 0 && tmp_nonfirstpkt_frag_bodysize != 0);
								INVARIANT(size_t(frag_recvsize) >= tmp_nonfirstpkt_final_frag_hdrsize && size_t(frag_recvsize) <= frag_maxsize);

								// calculate maxfragnum and curfragidx for the large packet from different client
								uint16_t tmp_nonfirstpkt_max_fragnum = 0, tmp_nonfirstpkt_cur_fragidx = 0;
								memcpy(&tmp_nonfirstpkt_max_fragnum, fragbuf + tmp_nonfirstpkt_frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));
								tmp_nonfirstpkt_max_fragnum = ntohs(tmp_nonfirstpkt_max_fragnum); // bigendian -> littleendian for large value
								memcpy(&tmp_nonfirstpkt_cur_fragidx, fragbuf + tmp_nonfirstpkt_frag_hdrsize, sizeof(uint16_t));
								tmp_nonfirstpkt_cur_fragidx = ntohs(tmp_nonfirstpkt_cur_fragidx); // bigendian -> littleendian for large value

								// judge whether it is a new large packet or existing large packet
								bool is_clientlogicalidx_exist = pkt_ring_buffer_ptr->is_clientlogicalidx_exist(tmp_nonfirstpkt_clientlogicalidx);
								if (is_client_logicalidx_exist) { // update large packet received before
									// Update existing large packet in PktRingBuffer
									tmp_stat = pkt_ring_buffer_ptr->update_large(tmp_nonfirstpkt_optype, tmp_nonfirstpkt_key, tmp_nonfirstpkt_frag_hdrsize + tmp_nonfirstpkt_cur_fragidx * tmp_nonfirstpkt_frag_bodysize, fragbuf + tmp_nonfirstpkt_final_frag_hdrsize, frag_recvsize - tmp_nonfirstpkt_final_frag_hdrsize, tmp_src_addr, tmp_addrlen, tmp_nonfirstpkt_clientlogicalidx);
									if (!tmp_stat) {
										printf("[ERROR] overflow of pkt_ring_buffer when push_large optype %x clientlogicalidx %d\n", optype_t(tmp_nonfirstpkt_optype), tmp_nonfirstpkt_clientlogicalidx);
										exit(-1);
									}
								}
								else { // add new large packet
									// Push large packet into PktRingBuffer
									tmp_stat = pkt_ring_buffer_ptr->push_large(tmp_nonfirstpkt_optype, tmp_nonfirstpkt_key, fragbuf, tmp_nonfirstpkt_frag_hdrsize, tmp_nonfirstpkt_frag_hdrsize + tmp_nonfirstpkt_cur_fragidx * tmp_nonfirstpkt_frag_bodysize, fragbuf + tmp_nonfirstpkt_final_frag_hdrsize, frag_recvsize - tmp_nonfirstpkt_final_frag_hdrsize, tmp_nonfirstpkt_max_fragnum, tmp_src_addr, tmp_addrlen, tmp_nonfirstpkt_clientlogicalidx);
									if (!tmp_stat) {
										printf("[ERROR] overflow of pkt_ring_buffer when push_large optype %x clientlogicalidx %d\n", optype_t(tmp_nonfirstpkt_optype), tmp_nonfirstpkt_clientlogicalidx);
										exit(-1);
									}
								}
								continue;
							}
							else { // from the same client
								if (tmp_nonfirstpkt_key != largepkt_key || tmp_nonfirstpkt_optype != largepkt_optype) { // unmatched packet
									continue; // skip current unmatched packet, go to receive next one
								}
								// else {} // matched packet
							}
						}
						else { // packet NOT for server
							printf("[ERROR] invalid packet type received by server: %x\n", optype_t(tmp_nonfirstpkt_optype));
							exit(-1);
						} // large packet to server w/ clientlogicalidx
					} // large/small to server
				} // to client/server
			} // IP_FRAGTYPE

			// NOTE: ONLY matched packet to client OR matched large packet from the same client to server can arrive here
			INVARIANT(size_t(frag_recvsize) >= final_frag_hdrsize);
		}

		// NOTE: ONLY large packet can access the following code

		uint16_t cur_fragidx = 0;
		memcpy(&cur_fragidx, fragbuf + frag_hdrsize, sizeof(uint16_t));
		cur_fragidx = ntohs(cur_fragidx); // bigendian -> littleendian for large value
		INVARIANT(cur_fragidx < max_fragnum);
		//printf("cur_fragidx: %d, max_fragnum: %d, frag_recvsize: %d, buf_offset: %d, copy_size: %d\n", cur_fragidx, max_fragnum, frag_recvsize, cur_fragidx * frag_bodysize, frag_recvsize - final_frag_hdrsize);

		buf.dynamic_memcpy(0 + frag_hdrsize + cur_fragidx * frag_bodysize, fragbuf + final_frag_hdrsize, frag_recvsize - final_frag_hdrsize);

		cur_fragnum += 1;
		if (cur_fragnum >= max_fragnum) {
			break;
		}
	}

	if (is_timeout) {
		buf.clear();
	}

	return is_timeout;
}


bool udprecvlarge_multisrc_udpfrag(int sockfd, dynamic_array_t **bufs_ptr, size_t &bufnum, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, const char* role, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion, bool isfilter, optype_t optype, netreach_key_t targetkey) {
	return udprecvlarge_multisrc(sockfd, bufs_ptr, bufnum, flags, src_addrs, addrlens, role, 0, UDP_FRAGMENT_MAXSIZE, srcnum_off, srcnum_len, srcnum_conversion, srcid_off, srcid_len, srcid_conversion, isfilter, optype, targetkey);
}

bool udprecvlarge_multisrc_ipfrag(int sockfd, dynamic_array_t **bufs_ptr, size_t &bufnum, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, const char* role, size_t frag_hdrsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion, bool isfilter, optype_t optype, netreach_key_t targetkey) {
	return udprecvlarge_multisrc(sockfd, bufs_ptr, bufnum, flags, src_addrs, addrlens, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE, srcnum_off, srcnum_len, srcnum_conversion, srcid_off, srcid_len, srcid_conversion, isfilter, optype, targetkey);
}

// NOTE: receive large packet from multiple sources; used for SCANRES_SPLIT (IMPORTANT: srcid > 0 && srcid <= srcnum <= bufnum)
// *bufs_ptr: max_srcnum dynamic arrays; if is_timeout = true, *bufs_ptr = NULL;
// bufnum: max_srcnum; if is_timeout = true, bufnum = 0;
// src_addrs: NULL or bufnum; addrlens: NULL or bufnum
// For example, srcnum for SCANRES_SPLIT is split_hdr.max_scannum; srcid for SCANRES_SPLIT is split_hdr.cur_scanidx
bool udprecvlarge_multisrc(int sockfd, dynamic_array_t **bufs_ptr, size_t &bufnum, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, const char* role, size_t frag_hdrsize, size_t frag_maxsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion, bool isfilter, optype_t optype, netreach_key_t targetkey) {
	INVARIANT(srcnum_len == 1 || srcnum_len == 2 || srcnum_len == 4);
	INVARIANT(srcid_len == 1 || srcid_len == 2 || srcid_len == 4);
	INVARIANT(srcnum_off <= frag_hdrsize && srcid_off <= frag_hdrsize);

	bool is_timeout = false;
	size_t final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
	size_t frag_bodysize = frag_maxsize - final_frag_hdrsize;
	struct sockaddr_in tmp_srcaddr;
	socklen_t tmp_addrlen;

	// receive current fragment
	char fragbuf[frag_maxsize];
	int frag_recvsize = 0;
	// set by the global first fragment
	bool global_isfirst = true;
	size_t max_srcnum = 0;
	size_t cur_srcnum = 0;
	// set by the first fragment of each srcid
	bool *local_isfirsts = NULL;
	uint16_t *max_fragnums = NULL;
	uint16_t *cur_fragnums = NULL;
	netreach_key_t tmpkey;
	while (true) {
		is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, &tmp_srcaddr, &tmp_addrlen, frag_recvsize, role);
		if (is_timeout) {
			break;
		}
		INVARIANT(size_t(frag_recvsize) >= final_frag_hdrsize);

		if (isfilter) {
			if (optype_t(get_packet_type(fragbuf, frag_recvsize)) != optype) {
				continue; // filter the unmatched packet
			}
			tmpkey.deserialize(fragbuf + sizeof(optype_t), frag_recvsize - sizeof(optype_t));
			if (tmpkey != targetkey) {
				continue;
			}
		}

		// set max_srcnum for the global first packet
		if (global_isfirst) {
			//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize);
			memcpy(&max_srcnum, fragbuf + srcnum_off, srcnum_len);
			if (srcnum_conversion && srcnum_len == 2) max_srcnum = size_t(ntohs(uint16_t(max_srcnum)));
			else if (srcnum_conversion && srcnum_len == 4) max_srcnum = size_t(ntohl(uint32_t(max_srcnum)));

			// initialize
			bufnum = max_srcnum;
			*bufs_ptr = new dynamic_array_t[max_srcnum];
			local_isfirsts = new bool[max_srcnum];
			max_fragnums = new uint16_t[max_srcnum];
			cur_fragnums = new uint16_t[max_srcnum];
			for (size_t i = 0; i < max_srcnum; i++) {
				(*bufs_ptr)[i].init(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
				local_isfirsts[i] = true;
				max_fragnums[i] = 0;
				cur_fragnums[i] = 0;
			}

			global_isfirst = false;
		}

		uint16_t tmpsrcid = 0;
		memcpy(&tmpsrcid, fragbuf + srcid_off, srcid_len);
		if (srcid_conversion && srcid_len == 2) tmpsrcid = size_t(ntohs(uint16_t(tmpsrcid)));
		else if (srcid_conversion && srcid_len == 4) tmpsrcid = size_t(ntohl(uint32_t(tmpsrcid)));
		//printf("tmpsrcid: %d, max_srcnum: %d, bufnum: %d\n", tmpsrcid, max_srcnum, bufnum);
		INVARIANT(tmpsrcid > 0 && tmpsrcid <= max_srcnum);

		int tmp_bufidx = tmpsrcid - 1; // [1, max_srcnum] -> [0, max_srcnum-1]
		dynamic_array_t &tmpbuf = (*bufs_ptr)[tmp_bufidx];

		if (local_isfirsts[tmp_bufidx]) {
			if (src_addrs != NULL) {
				src_addrs[tmp_bufidx] = tmp_srcaddr;
				addrlens[tmp_bufidx] = tmp_addrlen;
			}

			tmpbuf.dynamic_memcpy(0, fragbuf, frag_hdrsize);
			memcpy(&max_fragnums[tmp_bufidx], fragbuf + frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));
			max_fragnums[tmp_bufidx] = ntohs(max_fragnums[tmp_bufidx]); // bigendian -> littleendian for large value

			local_isfirsts[tmp_bufidx] = false;
		}

		uint16_t cur_fragidx = 0;
		memcpy(&cur_fragidx, fragbuf + frag_hdrsize, sizeof(uint16_t));
		cur_fragidx = ntohs(cur_fragidx); // bigendian -> littleendian for large value
		INVARIANT(cur_fragidx < max_fragnums[tmp_bufidx]);
		//printf("cur_fragidx: %d, max_fragnum: %d, frag_recvsize: %d, buf_offset: %d, copy_size: %d\n", cur_fragidx, max_fragnums[tmp_bufidx], frag_recvsize, cur_fragidx * frag_bodysize, frag_recvsize - final_frag_hdrsize);

		tmpbuf.dynamic_memcpy(frag_hdrsize + cur_fragidx * frag_bodysize, fragbuf + final_frag_hdrsize, frag_recvsize - final_frag_hdrsize);

		cur_fragnums[tmp_bufidx] += 1;
		INVARIANT(cur_fragnums[tmp_bufidx] <= max_fragnums[tmp_bufidx]);
		if (cur_fragnums[tmp_bufidx] == max_fragnums[tmp_bufidx]) {
			cur_srcnum += 1;
			if (cur_srcnum >= max_srcnum) {
				break;
			}
		}
	}

	INVARIANT(cur_srcnum == bufnum && bufnum == max_srcnum);
	if (local_isfirsts != NULL) {
		delete [] local_isfirsts;
		local_isfirsts = NULL;
	}
	if (max_fragnums != NULL) {
		delete [] max_fragnums;
		max_fragnums = NULL;
	}
	if (cur_fragnums != NULL) {
		delete [] cur_fragnums;
		cur_fragnums = NULL;
	}

	if (is_timeout) {
		if ((*bufs_ptr) != NULL) {
			delete [] (*bufs_ptr);
			*bufs_ptr = NULL;
		}
		bufnum = 0;
	}

	INVARIANT(bufnum == 0 || (bufnum > 0 && (*bufs_ptr) != NULL));

	return is_timeout;
}

// tcp

void create_tcpsock(int &sockfd, bool need_timeout, const char* role, int timeout_sec, int timeout_usec) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("[%s] fail to create tcp socket, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Disable udp/tcp check
	int disable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_NO_CHECK, (void*)&disable, sizeof(disable)) < 0) {
		printf("[%s] disable tcp checksum failed, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Send immediately
	int flag = 1; 
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
	// Set timeout for recvfrom/accept of udp/tcp
	if (need_timeout) {
		set_recvtimeout(sockfd, timeout_sec, timeout_usec);
	}
}

void tcpconnect(int sockfd, const char* ip, short port, const char *srcrole, const char* dstrole) {
	INVARIANT(ip != NULL);
	sockaddr_in addr;
	set_sockaddr(addr, inet_addr(ip), port);
	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		// TODO broken
		printf("[%s] fail to connect %s at %s:%hu, errno: %d!\n", srcrole, dstrole, ip, port, errno);
		exit(-1);
	}

}

void tcpsend(int sockfd, char *buf, int size, const char *role) {
	INVARIANT(buf != NULL && size >= 0);

	int send_size = size;
	const char *ptr = buf;
	while (send_size > 0) {
		int tmpsize = send(sockfd, ptr, send_size, 0);
		if (tmpsize < 0) {
			// TODO: Errno 32 means broken pipe, i.e., remote TCP connection is closed
			printf("[%s] TCP send returns %d, errno: %d\n", role, tmpsize, errno);
			exit(-1);
		}
		ptr += tmpsize;
		send_size -= tmpsize;
	}
}

void prepare_tcpserver(int &sockfd, bool need_timeout, short server_port, int max_pending_num, const char *role, int timeout_sec, int timeout_usec) {
	INVARIANT(role != NULL);

	// create socket
	create_tcpsock(sockfd, need_timeout, role, timeout_sec, timeout_usec);
	// reuse the occupied port for the last created socket instead of being crashed
	const int trueFlag = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0) {
		printf("[%s] fail to setsockopt, errno: %d!\n", role, errno);
		exit(-1);
	}
	// Set listen address
	sockaddr_in listen_addr;
	set_sockaddr(listen_addr, htonl(INADDR_ANY), server_port);
	if ((bind(sockfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr))) != 0) {
		printf("[%s] fail to bind socket on port %hu, errno: %d!\n", role, server_port, errno);
		exit(-1);
	}
	if ((listen(sockfd, max_pending_num)) != 0) {
		printf("[%s] fail to listen on port %hu, errno: %d!\n", role, server_port, errno);
		exit(-1);
	}
}

bool tcpaccept(int sockfd, struct sockaddr_in *addr, socklen_t *addrlen, int &connfd, const char* role) {
	bool need_timeout = false;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec =  0;
	socklen_t tvsz = sizeof(tv);
	int res = getsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, &tvsz);
	UNUSED(res);
	if (tv.tv_sec != 0 || tv.tv_usec != 0) {
		need_timeout = true;
	}

	bool is_timeout = false;

	connfd = accept(sockfd, (struct sockaddr *)addr, addrlen);
	if (connfd == -1) {
		if (need_timeout && (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)) {
			is_timeout = true;
		}
		else {
			printf("[%s] error of accept, errno: %d!\n", role, errno);
			exit(-1);
		}
	}
	return is_timeout;
}

bool tcprecv(int sockfd, void *buf, size_t len, int flags, int &recvsize, const char* role) {
	bool need_timeout = false;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec =  0;
	socklen_t tvsz = sizeof(tv);
	int res = getsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, &tvsz);
	UNUSED(res);
	if (tv.tv_sec != 0 || tv.tv_usec != 0) {
		need_timeout = true;
	}

	//bool is_broken = false;
	bool is_timeout = false;
	recvsize = recv(sockfd, buf, len, flags);
	if (recvsize < 0) {
		// Errno 32 means broken pipe, i.e., server.client is closed
		/*if (errno == 32) {
			printf("[%s] remote client is closed\n", role);
			recvsize = 0;
			is_broken = true;
		}*/
		if (need_timeout && (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)) {
			is_timeout = true;
		}
		else {
			printf("[%s] recv fails: %d, errno: %d!\n", role, recvsize, errno);
			exit(-1);
		}
	}
	//return is_broken;
	return is_timeout;
}





// Deprecated

//bool udprecvlarge_udpfrag(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role) {
//	return udprecvlarge(sockfd, buf, len, flags, src_addr, addrlen, recvsize, role, 0, UDP_FRAGMENT_MAXSIZE);
//}

//bool udprecvlarge_ipfrag(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role, size_t frag_hdrsize) {
//	return udprecvlarge(sockfd, buf, len, flags, src_addr, addrlen, recvsize, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE);
//}

// NOTE: receive large packet from one source
/*bool udprecvlarge(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role, size_t frag_hdrsize, size_t frag_maxsize) {
	bool is_timeout = false;
	size_t final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
	size_t frag_bodysize = frag_maxsize - final_frag_hdrsize;
	recvsize = 0;

	char fragbuf[frag_maxsize];
	int frag_recvsize = 0;
	bool is_first = true;
	uint16_t max_fragnum = 0;
	uint16_t cur_fragnum = 0;
	while (true) {
		if (is_first) {
			is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, src_addr, addrlen, frag_recvsize, role);
			if (is_timeout) {
				break;
			}
			INVARIANT(frag_recvsize >= final_frag_hdrsize && frag_recvsize <= frag_maxsize);
			//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize);

			INVARIANT(len >= frag_hdrsize);
			memcpy(buf, fragbuf, frag_hdrsize);
			recvsize += frag_hdrsize;
			memcpy(&max_fragnum, fragbuf + frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));
			is_first = false;
		}
		else {
			is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, NULL, NULL, frag_recvsize, role);
			if (is_timeout) {
				break;
			}
			INVARIANT(frag_recvsize >= final_frag_hdrsize);
		}

		uint16_t cur_fragidx = 0;
		memcpy(&cur_fragidx, fragbuf + frag_hdrsize, sizeof(uint16_t));
		INVARIANT(cur_fragidx < max_fragnum);
		//printf("cur_fragidx: %d, max_fragnum: %d, frag_recvsize: %d, len: %d, buf_offset: %d, copy_size: %d\n", cur_fragidx, max_fragnum, frag_recvsize, len, cur_fragidx * frag_bodysize, frag_recvsize - final_frag_hdrsize);

		INVARIANT(len >= (cur_fragidx * frag_bodysize + frag_recvsize - final_frag_hdrsize));
		memcpy(buf + frag_hdrsize + cur_fragidx * frag_bodysize, fragbuf + final_frag_hdrsize, frag_recvsize - final_frag_hdrsize);
		recvsize += (frag_recvsize - final_frag_hdrsize);

		cur_fragnum += 1;
		if (cur_fragnum >= max_fragnum) {
			break;
		}
	}

	return is_timeout;
}*/

//bool udprecvlarge_udpfrag(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role) {
//	return udprecvlarge(sockfd, buf, len, flags, src_addr, addrlen, recvsize, role, 0, UDP_FRAGMENT_MAXSIZE);
//}

//bool udprecvlarge_ipfrag(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role, size_t frag_hdrsize) {
//	return udprecvlarge(sockfd, buf, len, flags, src_addr, addrlen, recvsize, role, frag_hdrsize, IP_FRAGMENT_MAXSIZE);
//}

// NOTE: receive large packet from multiple sources (IMPORTANT: srcid > 0 && srcid <= srcnum <= bufnum)
// bufs: bufnum * len; src_addrs: NULL or bufnum; addrlens: NULL or bufnum; recvsizes: bufnum
// For example, srcnum for SCANRES_SPLIT is split_hdr.max_scannum; srcid for SCANRES_SPLIT is split_hdr.cur_scanidx
/*bool udprecvlarge_multisrc(int sockfd, void *bufs, size_t bufnum, size_t len, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, int *recvsizes, int& recvnum, const char* role, size_t frag_hdrsize, size_t frag_maxsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion) {
	INVARIANT(srcnum_len == 1 || srcnum_len == 2 || srcnum_len == 4);
	INVARIANT(srcid_len == 1 || srcid_len == 2 || srcid_len == 4);
	INVARIANT(srcnum_off <= frag_hdrsize && srcid_off <= frag_hdrsize);

	bool is_timeout = false;
	size_t final_frag_hdrsize = frag_hdrsize + sizeof(uint16_t) + sizeof(uint16_t);
	size_t frag_bodysize = frag_maxsize - final_frag_hdrsize;
	memset(recvsizes, 0, sizeof(int) * bufnum);
	struct sockaddr_in tmp_srcaddr;
	socklen_t tmp_addrlen;

	// receive current fragment
	char fragbuf[frag_maxsize];
	int frag_recvsize = 0;
	// set by the global first fragment
	bool global_isfirst = true;
	size_t max_srcnum = 0;
	size_t cur_srcnum = 0;
	// set by the first fragment of each srcid
	bool local_isfirsts[bufnum];
	uint16_t max_fragnums[bufnum];
	uint16_t cur_fragnums[bufnum];
	// initialize
	for (size_t tmpbufidx = 0; tmpbufidx < bufnum; tmpbufidx++) {
		local_isfirsts[tmpbufidx] = true;
		max_fragnums[tmpbufidx] = 0;
		cur_fragnums[tmpbufidx] = 0;

		recvsizes[tmpbufidx] = 0;
	}
	while (true) {
		is_timeout = udprecvfrom(sockfd, fragbuf, frag_maxsize, flags, &tmp_srcaddr, &tmp_addrlen, frag_recvsize, role);
		if (is_timeout) {
			break;
		}
		INVARIANT(frag_recvsize >= final_frag_hdrsize);

		// set max_srcnum for the global first packet
		if (global_isfirst) {
			//printf("frag_hdrsize: %d, final_frag_hdrsize: %d, frag_maxsize: %d, frag_bodysize: %d\n", frag_hdrsize, final_frag_hdrsize, frag_maxsize, frag_bodysize);
			memcpy(&max_srcnum, fragbuf + srcnum_off, srcnum_len);
			if (srcnum_conversion && srcnum_len == 2) max_srcnum = size_t(ntohs(uint16_t(max_srcnum)));
			else if (srcnum_conversion && srcnum_len == 4) max_srcnum = size_t(ntohl(uint32_t(max_srcnum)));
			INVARIANT(max_srcnum <= bufnum);
			global_isfirst = false;
		}

		uint16_t tmpsrcid = 0;
		memcpy(&tmpsrcid, fragbuf + srcid_off, srcid_len);
		if (srcid_conversion && srcid_len == 2) tmpsrcid = size_t(ntohs(uint16_t(tmpsrcid)));
		else if (srcid_conversion && srcid_len == 4) tmpsrcid = size_t(ntohl(uint32_t(tmpsrcid)));
		//printf("tmpsrcid: %d, max_srcnum: %d, bufnum: %d\n", tmpsrcid, max_srcnum, bufnum);
		INVARIANT(tmpsrcid > 0 && tmpsrcid <= max_srcnum);

		int tmp_bufidx = tmpsrcid - 1; // [1, max_srcnum] -> [0, max_srcnum-1]
		void *tmpbuf = bufs + tmp_bufidx * len;

		if (local_isfirsts[tmp_bufidx]) {
			if (src_addrs != NULL) {
				src_addrs[tmp_bufidx] = tmp_srcaddr;
				addrlens[tmp_bufidx] = tmp_addrlen;
			}

			INVARIANT(len >= frag_hdrsize);
			memcpy(tmpbuf, fragbuf, frag_hdrsize);
			recvsizes[tmp_bufidx] += frag_hdrsize;
			memcpy(&max_fragnums[tmp_bufidx], fragbuf + frag_hdrsize + sizeof(uint16_t), sizeof(uint16_t));

			local_isfirsts[tmp_bufidx] = false;
		}

		uint16_t cur_fragidx = 0;
		memcpy(&cur_fragidx, fragbuf + frag_hdrsize, sizeof(uint16_t));
		INVARIANT(cur_fragidx < max_fragnums[tmp_bufidx]);
		//printf("cur_fragidx: %d, max_fragnum: %d, frag_recvsize: %d, len: %d, buf_offset: %d, copy_size: %d\n", cur_fragidx, max_fragnums[tmp_bufidx], frag_recvsize, len, cur_fragidx * frag_bodysize, frag_recvsize - final_frag_hdrsize);

		INVARIANT(len >= (cur_fragidx * frag_bodysize + frag_recvsize - final_frag_hdrsize));
		memcpy(tmpbuf + frag_hdrsize + cur_fragidx * frag_bodysize, fragbuf + final_frag_hdrsize, frag_recvsize - final_frag_hdrsize);
		recvsizes[tmp_bufidx] += (frag_recvsize - final_frag_hdrsize);

		cur_fragnums[tmp_bufidx] += 1;
		INVARIANT(cur_fragnums[tmp_bufidx] <= max_fragnums[tmp_bufidx]);
		if (cur_fragnums[tmp_bufidx] == max_fragnums[tmp_bufidx]) {
			cur_srcnum += 1;
			if (cur_srcnum >= max_srcnum) {
				break;
			}
		}
	}

	INVARIANT(cur_srcnum <= bufnum);
	recvnum = cur_srcnum;
	return is_timeout;
}*/
