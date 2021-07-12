#include <unistd.h> // uint32_t uint8_t
#include <assert.h> // assert
#include <string> // std::string
#include <string.h> // memset
#include <net/if.h> // struct ifreq
#include <netinet/ether.h> // ether_header
#include <linux/ip.h> // iphdr
#include <linux/udp.h> // udphdr
#include <sys/ioctl.h> // ioctl
#include <netpacket/packet.h> // sockaddr_ll
#include <arpa/inet.h> // htons htonl
#include <iostream>

unsigned short csum16(unsigned short *buf, int nwords, bool isodd = false);
int lookup_if(int sockfd, std::string ifname, uint8_t *src_macaddr);
void init_raw_sockaddr(struct sockaddr_ll *socket_address, int ifidx, uint8_t *macaddr);
size_t init_buf(char *buf, uint32_t maxsize, uint8_t *src_macaddr, uint8_t *dst_macaddr, 
		std::string src_ipaddr, std::string dst_ipaddr, short src_port, 
		short dst_port, char *payload, uint32_t payload_size);
void init_msghdr(struct msghdr *msg, struct sockaddr_ll *socket_address, char *buf, size_t bufsize);
//void recv_buf(char *buf, uint32_t maxsize, struct msghdr *msg, uint32_t recvsize);
int client_recv_payload(char *buf, char *totalbuf, uint32_t totalsize, short client_port, short server_port);
int server_recv_payload(char *buf, char *totalbuf, uint32_t totalsize, short server_port, 
		uint8_t *src_mac, char *src_ip, short *src_port);
