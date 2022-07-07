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
#define MEMTABLE_SIZE 128 * 1024 * 1024
// heavy flush
//#define MAX_MEMTABLE_IMMUTABLE_NUM 5
//#define MIN_IMMUTABLE_FLUSH_NUM 2
// less flush
//#define MAX_MEMTABLE_IMMUTABLE_NUM 10
//#define MIN_IMMUTABLE_FLUSH_NUM 4
// no flush
#define MAX_MEMTABLE_IMMUTABLE_NUM 20
#define MIN_IMMUTABLE_FLUSH_NUM 8
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

// NOTE: for NoCache, no cache eviction -> not need seq mechanism -> not need DeletedSet; also not need snapshot mechanism
class RocksdbWrapper {

	public:
		static void prepare_rocksdb();
		static rocksdb::Options rocksdb_options;

		RocksdbWrapper();
		//RocksdbWrapper(uint16_t tmpworkerid);
		~RocksdbWrapper();

		bool open(uint16_t tmpworkerid);

		// loading phase (per-server loaders only touch rocksdb instead of deleted set; not need mutex for atomicity)
		bool force_put(netreach_key_t key, val_t val);
		bool force_multiput(netreach_key_t *keys, val_t *vals, int maxidx);
		
		// transaction phase (per-server worker and evictserver touch both rocksdb and deleted set; need mutex for atomicity)
		bool get(netreach_key_t key, val_t &val);
		bool put(netreach_key_t key, val_t val);
		bool remove(netreach_key_t key);

		size_t range_scan(netreach_key_t startkey, netreach_key_t endkey, std::vector<std::pair<netreach_key_t, snapshot_record_t>> &results);

	private:
		uint16_t workerid;

		// NOTE: we provide thread safety between per-server worker and evictserver; and there is no contention across workers of different servers
		// NOTE: lock overhead is ignorable compared with that of rocksdb
		
		/* server-side KVS */

		boost::shared_mutex rwlock; // protect db_ptr and deleted_set in get/put/remove/make_snapshot
		//std::mutex mutexlock; // protect db_ptr and deleted_set in get/put/remove/make_snapshot
		//rocksdb::TransactionDB *db_ptr = NULL;
		rocksdb::DB *db_ptr = NULL;
};

typedef RocksdbWrapper rocksdb_wrapper_t;

#endif
