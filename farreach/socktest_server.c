#include <getopt.h>
#include <stdlib.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <fstream>
#include <errno.h>
#include <set>
#include <signal.h> // for signal and raise
#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // inetaddr conversion
#include <sys/time.h> // struct timeval
#include <string.h>
#include <map>

#include "../common/helper.h"

#include "common_impl.h"
#include "../common/latency_helper.h"

size_t pktcnt = 10000;
std::vector<double> wait_latency_list;
std::vector<double> process_latency_list;

void run_server();

int main(int argc, char **argv) {
  parse_ini("config.ini");

  run_server();

  dump_latency(wait_latency_list, "wait_latency_list");
  dump_latency(process_latency_list, "process_latency_list");

  exit(0);
}

void run_server() {
  int sockfd = -1;
  prepare_udpserver(sockfd, false, server_port_start, "socktest.server");
  struct sockaddr_in client_addr;
  socklen_t client_addrlen = sizeof(struct sockaddr_in);

  char buf[MAX_BUFSIZE];
  int recvsize = 0;
  int pktidx = 0;
  while (true) {
	struct timespec t1, t2, t3, wait_t1, wait_t2, wait_t3;

	CUR_TIME(wait_t1);
	udprecvfrom(sockfd, buf, MAX_BUFSIZE, 0, (struct sockaddr*)&client_addr, &client_addrlen, recvsize, "socktest.server");
	CUR_TIME(wait_t2);

	CUR_TIME(t1);
	udpsendto(sockfd, buf, recvsize, 0, (struct sockaddr *)&client_addr, client_addrlen, "socktest.server");
	CUR_TIME(t2);

	DELTA_TIME(wait_t2, wait_t1, wait_t3);
	DELTA_TIME(t2, t1, t3);
	wait_latency_list.push_back(GET_MICROSECOND(wait_t3));
	process_latency_list.push_back(GET_MICROSECOND(t3));

	pktidx += 1;
	if (pktidx >= pktcnt) {
		break;
	}
  }
}
