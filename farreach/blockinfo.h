#ifndef BLOCKINFO_H
#define BLOCKINFO_H

#include <mutex>
#include <vector>
#include <netinet/in.h> // struct sockaddr_in

#include "key.h"
#include "val.h"
#include "packet_format.h"

typedef GetRequestBeingevicted<netreach_key_t> get_request_beingevicted_t;

// For read-blocking under rare case of cache eviciton
typedef struct BlockInfo {
	std::mutex _mutex;
	netreach_key_t _blockedkey;
	bool _isblocked;
	std::vector<get_request_beingevicted_t> _blockedreq_list;
	std::vector<struct sockaddr_in> _blockedaddr_list;
	std::vector<socklen_t> _blockedaddrlen_list;
} blockinfo_t;

#endif
