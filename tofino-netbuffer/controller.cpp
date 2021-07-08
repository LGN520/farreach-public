#include <stdint.h>
#include <stdlib.h>
#include "helper.h"
#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // inetaddr conversion
#include <signal.h> // for signal and raise

void listen_server();
void sendto_ptf(const char *buf, int size);

uint32_t controller_port = 2222;
std::string ptf_addr = "127.0.0.1";
uint32_t ptf_port = 3333;

std::string cmd = "bash tofino/update.sh &";

bool killed = false;
void kill(int signum);

int main(int argc, char **argv) {
	// register signal handler
	signal(SIGTERM, kill);

	listen_server();
}

void listen_server() {
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	INVARIANT(sockfd >= 0);
	struct sockaddr_in controller_sockaddr;
	memset(&controller_sockaddr, 0, sizeof(struct sockaddr_in));
	controller_sockaddr.sin_family = AF_INET;
	controller_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	controller_sockaddr.sin_port = htons(controller_port);
	int res = bind(sockfd, (struct sockaddr *)&controller_sockaddr, sizeof(struct sockaddr));
	INVARIANT(res != -1);

	char buf[MAX_BUFSIZE];
	int recv_size = 0;
	struct sockaddr_in server_sockaddr;
	memset(&server_sockaddr, 0, sizeof(struct sockaddr_in));
	uint32_t sockaddr_len = sizeof(struct sockaddr);
	while (!killed) {
		recv_size = recvfrom(sockfd, buf, MAX_BUFSIZE, 0, (struct sockaddr *)&server_sockaddr, &sockaddr_len);
		INVARIANT(recv_size > 0);

		system(cmd.c_str());
		sendto_ptf(buf, recv_size);
	}
}

void sendto_ptf(const char *buf, int size) {
	int ptf_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	INVARIANT(ptf_sockfd >= 0);
	struct sockaddr_in ptf_sockaddr;
	memset(&ptf_sockaddr, 0, sizeof(struct sockaddr_in));
	ptf_sockaddr.sin_family = AF_INET;
	INVARIANT(inet_pton(AF_INET, ptf_addr.c_str(), &ptf_sockaddr.sin_addr));
	ptf_sockaddr.sin_port = htons(ptf_port);
	int res = sendto(ptf_sockfd, buf, size, 0, (struct sockaddr *)&ptf_sockaddr, sizeof(struct sockaddr));
	INVARIANT(res != -1);
}

void kill(int signum) {
	COUT_THIS("[controller] Receive SIGKILL!")
	killed = true;
}
