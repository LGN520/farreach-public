#include <stdint.h>
#include <stdlib.h>
#include "helper.h"
#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // inetaddr conversion

std::string controller_addr = "172.16.252.9";
uint32_t controller_port = 2222;

int main(int argc, char **argv) {
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	INVARIANT(sockfd >= 0);
	struct sockaddr_in test_sockaddr;
	memset(&test_sockaddr, 0, sizeof(struct sockaddr_in));
	test_sockaddr.sin_family = AF_INET;
	INVARIANT(inet_pton(AF_INET, controller_addr.c_str(), &test_sockaddr.sin_addr));
	test_sockaddr.sin_port = htons(controller_port);

	char buf[1024];
	memset(buf, 0, 1024);
	uint32_t cnt = 1;
	uint32_t idx = 10;
	uint32_t size = 0;
	memcpy(buf, &cnt, sizeof(uint32_t));
	size += sizeof(uint32_t);
	memcpy(buf+sizeof(uint32_t), &idx, sizeof(uint32_t));
	size += sizeof(uint32_t);

	int res = sendto(sockfd, buf, size, 0, (struct sockaddr *)&test_sockaddr, sizeof(struct sockaddr));
	INVARIANT(res != -1);
}
