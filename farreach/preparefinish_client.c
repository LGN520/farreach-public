#include <stdint.h>

#include "../common/socket_helper.h"

#include "common_impl.h"

int main(int argc, char **argv) {
	parse_ini("config.ini");

	char buf[256];
	int sendval = 1;
	memcpy(buf, &sendval, sizeof(int));
	int sendsize = sizeof(int);

	// NOTE: preparefinish includes loadfinish for server and warmupfinish for controller
	int preparefinishclient_udpsock = -1;
	create_udpsock(preparefinishclient_udpsock, false, "preparefinishclient");

	// notify all physical servers for initial server-side snapshot if necessary
	for (size_t i = 0; i < server_physical_num; i++) {
		struct sockaddr_in loadfinishserver_addr;
		set_sockaddr(loadfinishserver_addr, inet_addr(server_ip_for_controller_list[i]), transaction_loadfinishserver_port);
		socklen_t loadfinishserver_addrlen = sizeof(struct sockaddr_in);

		printf("[preparefinishclient] send notification to loadfinishserver of physical server %d\n", i);
		udpsendto(preparefinishclient_udpsock, buf, sendsize, 0, &loadfinishserver_addr, loadfinishserver_addrlen, "preparefinishclient");
	}

	// notify controller to start periodic snapshot
	struct sockaddr_in warmupfinishserver_addr;
	set_sockaddr(warmupfinishserver_addr, inet_addr(controller_ip_for_server), controller_warmupfinishserver_port);
	socklen_t warmupfinishserver_addrlen = sizeof(struct sockaddr_in);
	printf("[preparefinishclient] send notification to controller.warmupfinishserver\n");
	udpsendto(preparefinishclient_udpsock, buf, sendsize, 0, &warmupfinishserver_addr, warmupfinishserver_addrlen, "preparefinishclient");

	return 0;
}
