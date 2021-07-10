#include <unistd.h>
#include <linux/if_packet.h>
#include <netinet/ether.hi> // ether_header
#include <linux/ip.h> // iphdr
#include <linux/udp.h> // udphdr

unsigned short csum16(unsigned short *buf, int nwords);
struct ifreq get_ifidx(std::string ifname);
struct sockaddr_ll get_raw_sockaddr(struct ifreq if_idx, char *macaddr);
size_t init_buf(char *buf, uint32_t maxsize, char *src_macaddr, char *dst_macaddr, 
		std::string src_ipaddr, std::string dst_ipaddr, short src_port, 
		short dst_port, char *payload, uint32_t payload_size);
void init_msghdr(struct msghdr *msg, struct sockaddr_ll *socket_address);
