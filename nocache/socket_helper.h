#ifndef SOCKET_HELPER_H
#define SOCKET_HELPER_H

#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in_in
#include <errno.h> // errno
#include <net/if.h> // struct ifreq; ifname -> ifidx
//#include <sys/ioctl.h> // ioctl
#include <netpacket/packet.h> // sockaddr_ll

#include "helper.h"
#include "dynamic_array.h"
#include "key.h"
#include "packet_format_impl.h"
#include "pkt_ring_buffer.h"

#define SOCKET_TIMEOUT 5 // 5s
// for limited effect on system thpt of normal request timeout
//#define CLIENT_SOCKET_TIMEOUT_SECS 1 // 1s (for static workload under server rotation)
#define CLIENT_SOCKET_TIMEOUT_SECS 5 // 5s (for dynamic workload due to server-side disk contention of simulation overhead)
#define CLIENT_SCAN_SOCKET_TIMEOUT_SECS 5 // 5s
// for low snapshot latency
#define SWITCHOS_POPCLIENT_FOR_REFLECTOR_TIMEOUT_USECS 100000 // 0.1s
#define SWITCHOS_SNAPSHOTCLIENT_FOR_REFLECTOR_TIMEOUT_USECS 100000 // 0.1s
#define SWITCHOS_SPECIALCASESERVER_TIMEOUT_USECS 1000 // 1ms

// payload only used by end-hosts -> linux kernel performs ip-level fragmentation
// max payload size to avoid udp fragmentation (manual udp fragmentation): 65535(ipmax) - 20(iphdr) - 8(udphdr)
#define UDP_FRAGMENT_MAXSIZE 65507
// programmable switch needs to process udp_hdr and op_hdr in payload -> we must perform ip-level fragmentation manually
// max ethernet pkt 1518 - ethernet header 18 - ipv4 header 20 - udp header 8
// max payload size to avoid ipv4 fragmentation (manual ipv4 fragmentation)
//#define IP_FRAGMENT_MAXSIZE 1472
// NOTE: reserve 64B for PUTREQ_LARGEVALUE_SEQ_INSWITCH
#define IP_FRAGMENT_MAXSIZE 1408

#define IP_FRAGTYPE 1
#define UDP_FRAGTYPE 2

// udp client: create_udpsock -> set_sockaddr -> udpsendto
// udp server: prepare_udpserver -> udprecvfrom
// tcp client: create_tcpsock -> tcpconnect -> tcpsend
// tcp server: prepare_tcpserver -> tcpaccept -> tcprecv

// NOTE: we use uint32_t for ipaddr due to htonl(INADDR_ANY)
void set_sockaddr(sockaddr_in &addr, uint32_t bigendian_saddr, short littleendian_port);
void set_recvtimeout(int sockfd, int timeout_sec = SOCKET_TIMEOUT, int timeout_usec = 0);

// udp
void create_udpsock(int &sockfd, bool need_timeout, const char* role = "sockethelper.udpsock", int timeout_sec = SOCKET_TIMEOUT, int timeout_usec = 0, int udp_rcvbufsize = UDP_DEFAULT_RCVBUFSIZE);
void udpsendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role = "sockethelper.udpsendto");
void prepare_udpserver(int &sockfd, bool need_timeout, short server_port, const char* role = "sockethelper.udpserver", int timeout_sec = SOCKET_TIMEOUT, int timeout_usec = 0, int udp_rcvbufsize = UDP_DEFAULT_RCVBUFSIZE);
bool udprecvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role = "sockethelper.udprecvfrom");

void udpsendlarge_udpfrag(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role);
void udpsendlarge_ipfrag(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role, size_t frag_hdrsize);
void udpsendlarge(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen, const char* role, size_t frag_hdrsize, size_t frag_maxsize);
//bool udprecvlarge_udpfrag(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role);
//bool udprecvlarge_ipfrag(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role, size_t frag_hdrsize);
//bool udprecvlarge(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, int &recvsize, const char* role, size_t frag_hdrsize, size_t frag_maxsize);
//bool udprecvlarge_multisrc_udpfrag(int sockfd, void *bufs, size_t bufnum, size_t len, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, int *recvsizes, int& recvnum, const char* role, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion);
//bool udprecvlarge_multisrc_ipfrag(int sockfd, void *bufs, size_t bufnum, size_t len, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, int *recvsizes, int& recvnum, const char* role, size_t frag_hdrsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion);
//bool udprecvlarge_multisrc(int sockfd, void *bufs, size_t bufnum, size_t len, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, int *recvsizes, int& recvnum, const char* role, size_t frag_hdrsize, size_t frag_maxsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion);
bool udprecvlarge_udpfrag(int sockfd, dynamic_array_t &buf, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, const char* role);
bool udprecvlarge_ipfrag(int sockfd, dynamic_array_t &buf, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, const char* role, pkt_ring_buffer_t *pkt_ring_buffer_ptr = NULL);
bool udprecvlarge(int sockfd, dynamic_array_t &buf, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen, const char* role, size_t frag_maxsize, int fragtype, pkt_ring_buffer_t *pkt_ring_buffer_ptr);
bool udprecvlarge_multisrc_udpfrag(int sockfd, dynamic_array_t **bufs_ptr, size_t &bufnum, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, const char* role, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion, bool isfilter=false, optype_t optype=0, netreach_key_t targetkey=netreach_key_t());
bool udprecvlarge_multisrc_ipfrag(int sockfd, dynamic_array_t **bufs_ptr, size_t &bufnum, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, const char* role, size_t frag_hdrsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion, bool isfilter=false, optype_t optype=0, netreach_key_t targetkey=netreach_key_t());
bool udprecvlarge_multisrc(int sockfd, dynamic_array_t **bufs_ptr, size_t &bufnum, int flags, struct sockaddr_in *src_addrs, socklen_t *addrlens, const char* role, size_t frag_hdrsize, size_t frag_maxsize, size_t srcnum_off, size_t srcnum_len, bool srcnum_conversion, size_t srcid_off, size_t srcid_len, bool srcid_conversion, bool isfilter=false, optype_t optype=0, netreach_key_t targetkey=netreach_key_t());

// tcp
void create_tcpsock(int &sockfd, bool need_timeout, const char* role = "sockethelper.tcpsock", int timeout_sec = SOCKET_TIMEOUT, int timeout_usec = 0);
void tcpconnect(int sockfd, const char* ip, short port, const char *srcrole = "sockhelper.tcpclient", const char* dstrole = "sockethelper.tcpserver");
void tcpsend(int sockfd, char *buf, int size, const char *role = "sockethelper.tcpsend");
void prepare_tcpserver(int &sockfd, bool need_timeout, short server_port, int max_pending_num, const char *role = "sockethelper.tcpserver", int timeout_sec = SOCKET_TIMEOUT, int timeout_usec = 0);
bool tcpaccept(int sockfd, struct sockaddr_in *addr, socklen_t *addrrlen, int &connfd, const char* role = "sockethelper.tcpaccept");
bool tcprecv(int sockfd, void *buf, size_t len, int flags, int &recvsize, const char* role = "sockethelper.tcprecv");

#endif
