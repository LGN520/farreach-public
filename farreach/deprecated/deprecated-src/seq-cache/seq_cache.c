#include "seq_cache.h"

SeqCache::SeqCache() {
	_cachetable = new seq_cache_entry_t[SEQ_CACHE_ENTRIES];
	INVARIANT(_cachetable != NULL);
}

SeqCache::~SeqCache() {
	if (_cachetable != NULL) {
		delete [] _cachetable;
		_cachetable = NULL;
	}
}

bool SeqCache::get(netreach_key_t key, uint32_t &seq) {
	int64_t idx = key.to_int_for_hash() % SEQ_CACHE_ENTRIES;
	if (_cachetable[idx].key == key) {
		seq = _cachetable[idx].seq;
		return true;
	}
	else {
		return false;
	}
}

void SeqCache::put(netreach_key_t key, uint32_t seq) {
	int64_t idx = key.to_int_for_hash() % SEQ_CACHE_ENTRIES;
	_cachetable[idx].key = key;
	_cachetable[idx].seq = seq;
	// NOTE: seq cache uses write through policy for rocksdb, so we do not need to consider eviction
}

void SeqCache::remove(netreach_key_t key) {
	int64_t idx = key.to_int_for_hash() % SEQ_CACHE_ENTRIES;
	if (_cachetable[idx].key == key) {
		_cachetable[idx].key = netreach_key_t();
		_cachetable[idx].seq = 0;
	}
	// NOTE: seq cache uses write through policy for rocksdb, so we do not need to consider eviction
}
