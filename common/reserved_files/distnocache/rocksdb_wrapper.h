#ifndef ROCKSDB_WRAPPER_H
#define ROCKSDB_WRAPPER_H

//#include <mutex>
#include <atomic>
#include <vector>
#include <map>
#include <boost/thread/shared_mutex.hpp>

#include "rocksdb/db.h"
#include "rocksdb/cache.h"
#include "rocksdb/table.h"
#include "rocksdb/filter_policy.h"
//#include "rocksdb/utilities/transaction.h"
//#include "rocksdb/utilities/transaction_db.h"
#include "rocksdb/utilities/checkpoint.h"

#include "helper.h"
#include "key.h"
#include "val.h"
#include "snapshot_record.h"
#include "io_helper.h"

#ifdef DEBUG_ROCKSDB
#include "rocksdb/iostats_context.h"
#include "rocksdb/perf_context.h"
#endif

// Default configuration as in rocksdb tuning guide
// (1) parallism options
//#define WRITE_PARALLISM 16
#define GLOBAL_MAX_FLUSH_THREAD_NUM 8
#define GLOBAL_MAX_COMPACTION_THREAD_NUM 8
// (2) general options
#define BLOOMFILTER_BITS_PER_KEY 10 // larger bits per key -> fewer false positives to lookup key yet larger space cost
#define BLOCKCACHE_SIZE 1 * 1024 * 1024 * 1024 // larger blockcache size -> better read performance yet larger space cost
#define BLOCKCACHE_SHARDBITS 4 // increase if bottlenecked at blockcache's mutex
#define TABLECACHE_SHARDBITS 6 // increase if bottlenecked at tablecache's mutex
#define BLOCK_SIZE 4 * 1024 // larger block size -> smaller memory cost for block metadata, yet worse read performance
// (3) flushing options
#define MEMTABLE_SIZE 64 * 1024 * 1024
// heavy flush
//#define MAX_MEMTABLE_IMMUTABLE_NUM 10
//#define MIN_IMMUTABLE_FLUSH_NUM 4
// less flush
//#define MAX_MEMTABLE_IMMUTABLE_NUM 20
//#define MIN_IMMUTABLE_FLUSH_NUM 8
// no flush
#define MAX_MEMTABLE_IMMUTABLE_NUM 40
#define MIN_IMMUTABLE_FLUSH_NUM 16
// (4) level-style compaction (NOTE: level1 size should be similar as level0 size)
#define LEVEL0_SST_NUM 10 // level0 size: MEMTABLE_SIZE * MIN_IMMUTABLE_FLUSH_NUM * LEVEL0_SST_NUM
#define LEVEL1_SST_SIZE 512 * 1024 * 1024 // sst file size in level1
#define LEVEL1_SST_NUM 10 // level1 size: SST_SIZE * LEVEL1_SST_NUM
#define LEVEL_TOTALSIZE_MULTIPLIER 10
#define LEVEL_SSTSIZE_MULTIPLIER 1
#define LEVEL_NUM 7
// (5) other options
#define WAL_BYTES_PER_SYNC 2 * 1024 * 1024 // used to alleviate write slowdown due to OS flushing WAL cache
// (6) write options
#define SYNC_WRITE false // flush WAL instead of memtable for each operation
#define DISABLE_WAL false // disable WAL flush

class RocksdbWrapper {

	public:
		static void prepare_rocksdb();
		static rocksdb::Options rocksdb_options;

		RocksdbWrapper();
		//RocksdbWrapper(uint16_t tmpworkerid);
		~RocksdbWrapper();

		bool open(uint16_t tmpworkerid);

		// loading phase (per-server loaders only touch rocksdb; not need mutex for atomicity)
		bool force_put(netreach_key_t key, val_t val);
		bool force_multiput(netreach_key_t *keys, val_t *vals, int maxidx);
		
		// transaction phase
		bool get(netreach_key_t key, val_t &val);
		// NOTE: only normal pkt and evicted pkt can udpate per-key seq
		// (1) for two normal pkts, we use the order of arriving server as the final order while not comparing the seqs assigned by switch, which does not undermine serializability (besides, packet reordering seems impossible between switch and server due to a single path and FIFO manner of inswitch queue)-> not need to check seq
		// (2) for two evicted pkts, controller guarantees that they cannot arrive at the same time (one-by-one manner)
		// (3) for normal pkt and evicted pkt
		// (3-1) normal pkt with smaller seq cannot overwrite evicted pkt: (a) if normal pkt arrives at switch between cache population and cache eviction, it must be processed by switch instead arriving at server; (b) if normal pkt arrives at switch after cache eviction, the seq must be larger than that of evicted pkt; (c) if normal pkt arrives at switch before cache population, it is impossible to arrive at server after evicted pkt, as the time of cache population and cache eviction has passed, which is much longer than normal RTT!
		// (3-2) evicted pkt with smaller seq could overwrite normal pkt as we use non-blocking cache population/eviction -> to solve it, we only need to check seq for evicted pkt
		bool put(netreach_key_t key, val_t val);
		bool remove(netreach_key_t key);

		size_t range_scan(netreach_key_t startkey, netreach_key_t endkey, std::vector<std::pair<netreach_key_t, snapshot_record_t>> &results);

	private:
		uint16_t workerid;

		// NOTE: we provide thread safety between per-server worker and evictserver; and there is no contention across workers of different servers
		// NOTE: lock overhead is ignorable compared with that of rocksdb
		
		/* server-side KVS */

		boost::shared_mutex rwlock; // protect db_ptr in get/put/remove
		//std::mutex mutexlock; // protect db_ptr in get/put/remove
		//rocksdb::TransactionDB *db_ptr = NULL;
		rocksdb::DB *db_ptr = NULL;

		/* utils */

		void merge_sort(const std::vector<std::pair<netreach_key_t, snapshot_record_t>> &veca, const std::vector<std::pair<netreach_key_t, snapshot_record_t>> &vecb, std::vector<std::pair<netreach_key_t, snapshot_record_t>> &results, bool need_exist = false);
};

typedef RocksdbWrapper rocksdb_wrapper_t;

#endif
