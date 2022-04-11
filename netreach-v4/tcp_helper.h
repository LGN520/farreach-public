#ifndef TCP_HELPER_H
#define TCP_HELPER_H

#include <sys/socket.h> // socket API
#include <netinet/in.h> // struct sockaddr_in

#include "helper.h"

void tcpsend(char *buf, int size);

#endif
