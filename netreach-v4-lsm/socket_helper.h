#ifndef SOCKET_HELPER_H
#define SOCKET_HELPER_H

#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in
#include <errno.h> // errno
#include <net/if.h> // struct ifreq; ifname -> ifidx
//#include <sys/ioctl.h> // ioctl
#include <netpacket/packet.h> // sockaddr_ll

#include "helper.h"

#define SOCKET_TIMEOUT 5

// udp client: create_udpsock -> set_sockaddr -> udpsendto
// udp server: prepare_udpserver -> udprecvfrom
// tcp client: create_tcpsock -> tcpconnect -> tcpsend
// tcp server: prepare_tcpserver -> tcpaccept -> tcprecv

// NOTE: we use uint32_t for ipaddr due to htonl(INADDR_ANY)
void set_sockaddr(sockaddr_in &addr, uint32_t bigendian_saddr, short littleendian_port);
void set_recvtimeout(int sockfd, int timeout_sec = SOCKET_TIMEOUT, int timeout_usec = 0);

// udp
void create_udpsock(int &sockfd, bool need_timeout, const char* role = "sockethelper.udpsock", int timeout_sec = SOCKET_TIMEOUT, int timeout_usec = 0);
void udpsendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen, const char* role = "sockethelper.udpsendto");
void prepare_udpserver(int &sockfd, bool need_timeout, short server_port, const char* role = "sockethelper.udpserver", int timeout_sec = SOCKET_TIMEOUT, int timeout_usec = 0);
bool udprecvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen, int &recvsize, const char* role = "sockethelper.udprecvfrom");

// tcp
void create_tcpsock(int &sockfd, bool need_timeout, const char* role = "sockethelper.tcpsock", int timeout_sec = SOCKET_TIMEOUT, int timeout_usec = 0);
void tcpconnect(int sockfd, const char* ip, short port, const char *srcrole = "sockhelper.tcpclient", const char* dstrole = "sockethelper.tcpserver");
void tcpsend(int sockfd, char *buf, int size, const char *role = "sockethelper.tcpsend");
void prepare_tcpserver(int &sockfd, bool need_timeout, short server_port, int max_pending_num, const char *role = "sockethelper.tcpserver", int timeout_sec = SOCKET_TIMEOUT, int timeout_usec = 0);
bool tcpaccept(int sockfd, struct sockaddr *addr, socklen_t *addrrlen, int &connfd, const char* role = "sockethelper.tcpaccept");
bool tcprecv(int sockfd, void *buf, size_t len, int flags, int &recvsize, const char* role = "sockethelper.tcprecv");

#endif
