#ifndef SEQ_CACHE_H
#define SEQ_CACHE_H

#include <map>
#include <stdint.h>

#include "key.h"
#include "helper.h"

// total seq cache size = # of server thread * per-server seq cache size
// For example, under 128 server threads, we need at least 128 * 1M * (16B+4B) = 5GB
#define SEQ_CACHE_ENTRIES 1 * 1000 * 1000

struct SeqCacheEntry {
	netreach_key_t key;
	uint32_t seq;
}PACKED;

typedef SeqCacheEntry seq_cache_entry_t;

class SeqCache {
	public:
		SeqCache();
		~SeqCache();

		bool get(netreach_key_t key, uint32_t &seq);
		void put(netreach_key_t key, uint32_t seq);
		void remove(netreach_key_t key);
	private:
		seq_cache_entry_t *_cachetable = NULL;
};

typedef SeqCache seq_cache_t;

#endif
