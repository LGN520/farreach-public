#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <atomic>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <fstream>
#include <fcntl.h>
#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // inetaddr conversion
#include <unistd.h>
//#include <sys/time.h> // struct timeval

#include "../common/key.h"
#include "../common/val.h"

#include "common_impl.h"
#include "../common/latency_helper.h"
#include "../common/socket_helper.h"

size_t pktcnt = 10000;
std::vector<double> req_latency_list;
std::vector<double> rsp_latency_list;
std::vector<double> wait_latency_list;

void run_benchmark();

int main(int argc, char **argv) {
	parse_ini("config.ini");

	run_benchmark();

	dump_latency(req_latency_list, "req_latency_list");
	dump_latency(rsp_latency_list, "rsp_latency_list");
	dump_latency(wait_latency_list, "wait_latency_list");

	exit(0);
}

void run_benchmark() {
	int nohit_payload = 0;
	int hit_payload = 1;
	
	//int payloadvalue = nohit_payload;
	int payloadvalue = hit_payload;

	int sockfd = -1;
	create_udpsock(sockfd, false, "socktest.client");
	struct sockaddr_in server_addr;
	set_sockaddr(server_addr, inet_addr(server_ip), server_port_start);
	socklen_t server_addrlen = sizeof(struct sockaddr_in);

	char buf[MAX_BUFSIZE];
	int recvsize = 0;
	for (size_t i = 0; i < pktcnt; i++) {
		struct timespec req_t1, req_t2, req_t3, rsp_t1, rsp_t2, rsp_t3, final_t3;
		struct timespec wait_t1, wait_t2, wait_t3;

		CUR_TIME(req_t1);
		int bigendian_payloadvalue = int(ntohl(uint32_t(payloadvalue)));
		udpsendto(sockfd, (char *)&bigendian_payloadvalue, sizeof(int), 0, (struct sockaddr *)&server_addr, server_addrlen, "socktest.client");
		CUR_TIME(req_t2);

		CUR_TIME(wait_t1);
		udprecvfrom(sockfd, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "socktest.client");
		CUR_TIME(wait_t2);

		CUR_TIME(rsp_t1);
		INVARIANT(recvsize == sizeof(int));
		INVARIANT(*((int *)buf) == bigendian_payloadvalue);
		CUR_TIME(rsp_t2);

		DELTA_TIME(req_t2, req_t1, req_t3);
		DELTA_TIME(rsp_t2, rsp_t1, rsp_t3);
		DELTA_TIME(wait_t2, wait_t1, wait_t3);
		double req_time = GET_MICROSECOND(req_t3);
		double rsp_time = GET_MICROSECOND(rsp_t3);
		double wait_time = GET_MICROSECOND(wait_t3);
		req_latency_list.push_back(req_time);
		rsp_latency_list.push_back(rsp_time);
		wait_latency_list.push_back(wait_time);
	}
}
