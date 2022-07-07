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

#include "helper.h"
#include "key.h"
#include "val.h"
#include "iniparser/iniparser_wrapper.h"
#include "crc32.h"
#include "latency_helper.h"
#include "socket_helper.h"

#ifdef USE_YCSB
#include "workloadparser/ycsb_parser.h"
#elif defined USE_SYNTHETIC
#include "workloadparser/synthetic_parser.h"
#endif

#include "common_impl.h"

//#define DUMP_BUF

void run_warmuper();

void * run_fg(void *param); // sender

int main(int argc, char **argv) {
	parse_ini("config.ini");

	run_warmuper();

	exit(0);
}

void run_warmuper() {
	int res = 0;

	// workload parser
	ParserIterator *iter = NULL;
#ifdef USE_YCSB
	iter = new YcsbParserIterator(raw_warmup_workload_filename);
#elif defined USE_SYNTHETIC
	iter = new SyntheticParserIterator(raw_warmup_workload_filename);
#endif
	INVARIANT(iter != NULL);

	netreach_key_t tmpkey;
	val_t tmpval;

	// for network communication
	char buf[MAX_BUFSIZE];
	int req_size = 0;
	int recv_size = 0;
	int clientsock = -1;
	create_udpsock(clientsock, false, "warmup_client");
	struct sockaddr_in server_addr;
	set_sockaddr(server_addr, inet_addr(server_ips[0]), server_worker_port_start);
	socklen_t server_addrlen = sizeof(struct sockaddr_in);

	while (true) {
		if (!iter->next()) {
			break;
		}

		tmpkey = iter->key();
		if (iter->type() == uint8_t(packet_type_t::PUTREQ)) { // update or insert
			tmpval = iter->val();
			INVARIANT(tmpval.val_length <= val_t::SWITCH_MAX_VALLEN);

			warmup_request_t req(tmpkey, tmpval);
			req_size = req.serialize(buf, MAX_BUFSIZE);
#ifdef DUMP_BUF
			dump_buf(buf, req_size);
#endif

			udpsendto(clientsock, buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");

			udprecvfrom(clientsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
			INVARIANT(recv_size > 0);
#ifdef DUMP_BUF
			dump_buf(buf, recv_size);
#endif

			warmup_ack_t rsp(buf, recv_size);
			UNUSED(rsp);
		}
		else {
			printf("Invalid request type: %u\n", uint32_t(iter->type()));
			exit(-1);
		}
	}

	iter->closeiter();
	delete iter;
	iter = NULL;

	close(clientsock);
#if !defined(NDEBUGGING_LOG)
	ofs.close();
#endif
	return;
}
