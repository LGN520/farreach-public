#include <stdint.h>

#include "socket_helper.h"

#include "common_impl.h"

int main(int argc, char **argv) {
	parse_ini("config.ini");

	char buf[256];
	int sendval = 1;
	memcpy(buf, &sendval, sizeof(int));
	int sendsize = sizeof(int);

	int loadfinishclient_udpsock = -1;
	create_udpsock(loadfinishclient_udpsock, false, "loadfinishclient");

	struct sockaddr_in loadfinishserver_addr;
	set_sockaddr(loadfinishserver_addr, inet_addr("127.0.0.1"), transaction_loadfinishserver_port);
	socklen_t loadfinishserver_addrlen = sizeof(struct sockaddr_in);

	printf("[loadfinishclient] send notification to loadfinishserver\n");
	udpsendto(loadfinishclient_udpsock, buf, sendsize, 0, &loadfinishserver_addr, loadfinishserver_addrlen, "loadfinishclient");

	return 0;
}
