#ifndef BLOCKINFO_H
#define BLOCKINFO_H

#include <mutex>
#include <vector>
#include <netinet/in.h> // struct sockaddr_in

#include "../common/key.h"
#include "../common/val.h"
#include "../common/packet_format.h"

typedef GetRequestBeingevicted<netreach_key_t> get_request_beingevicted_t;

// For read-blocking under rare case of cache eviciton or PUTREQ_LARGEVALUE
typedef struct BlockInfo {
	// NOTE: ONLY used for read blocking of cache eviction
	// For read blocking of PUTREQ_LARGEVALUE, only worker accesses BlockInfo -> NOT need mutex
	std::mutex _mutex;

	uint32_t _largevalueseq; // ONLY used for read blocking of PUTERQ_LARGEVALUE

	netreach_key_t _blockedkey;
	bool _isblocked;
	std::vector<get_request_beingevicted_t> _blockedreq_list;
	std::vector<struct sockaddr_in> _blockedaddr_list;
	std::vector<socklen_t> _blockedaddrlen_list;
} blockinfo_t;

#endif
