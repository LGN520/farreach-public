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
class BlockInfo {
	public:
		BlockInfo() {
			_largevalueseq = 0;
			_blockedkey = netreach_key_t::min();
			_isblocked = false;
			_blockedreq_list.resize(0);
			_blockedaddr_list.resize(0);
			_blockedaddrlen_list.resize(0);
		}

		BlockInfo(const BlockInfo& other) {
			_largevalueseq = other._largevalueseq;
			_blockedkey = other._blockedkey;
			_isblocked = other._isblocked;
			_blockedreq_list = other._blockedreq_list;
			_blockedaddr_list = other._blockedaddr_list;
			_blockedaddrlen_list = other._blockedaddrlen_list;
		}

		// NOTE: ONLY used for read blocking of cache eviction
		// For read blocking of PUTREQ_LARGEVALUE, only worker accesses BlockInfo -> NOT need mutex
		std::mutex _mutex;

		uint32_t _largevalueseq; // ONLY used for read blocking of PUTERQ_LARGEVALUE

		netreach_key_t _blockedkey;
		bool _isblocked;
		std::vector<get_request_beingevicted_t> _blockedreq_list;
		std::vector<struct sockaddr_in> _blockedaddr_list;
		std::vector<socklen_t> _blockedaddrlen_list;
};

typedef BlockInfo blockinfo_t;

#endif
