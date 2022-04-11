#include "tcp_helper.h"

void tcpsend(int sockfd, char *buf, int size) {
	INVARIANT(buf != NULL && size >= 0);

	int send_size = size;
	const char *ptr = buf;
	while (send_size > 0) {
		int tmpsize = send(sockfd, ptr, send_size, 0);
		if (tmpsize < 0) {
			// Errno 32 means broken pipe, i.e., remote TCP connection is closed
			printf("TCP send returns %d, errno: %d\n", tmpsize, errno);
			break;
		}
		ptr += tmpsize;
		send_size -= tmpsize;
	}
}
