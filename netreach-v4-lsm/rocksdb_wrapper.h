#ifndef ROCKSDB_WRAPPER_H
#define ROCKSDB_WRAPPER_H

#include "rocksdb/db.h"
#include "rocksdb/cache.h"
#include "rocksdb/table.h"
#include "rocksdb/utilities/transaction.h"
#include "rocksdb/utilities/transaction_db.h"

#include "helper.h"
#include "key.h"
#include "val.h"
#include "deleted_set_impl.h"

#define MEMTABLE_SIZE 4 * 1024 * 1024
#define MAX_MEMTABLE_NUM 2
#define SST_SIZE 4 * 1024 * 1024
#define COMPACTION_THREAD_NUM 2
#define LEVEL0_SST_NUM 8
#define LEVEL_NUM 4
#define LEVEL1_TOTAL_SIZE 32 * 1024 * 1024
#define LEVEL_MULTIPLIER 8
#define BLOCKCACHE_SIZE 4 * 1024 * 1024
#define BLOCKCACHE_SHARDBITS 4
#define SYNC_WRITE false

class RocksdbWrapper {

	typedef DeletedSet<netreach_key_t, uint32_t> deleted_set_t;

	public:
		static void prepare_rocksdb();

		RocksdbWrapper();
		//RocksdbWrapper(uint16_t workerid);
		
		bool open(uint16_t workerid);
		bool force_put(netreach_key_t key, val_t val);
		bool get(netreach_key_t key, val_t &val, uint32_t &seq);
		bool put(netreach_key_t key, val_t val, uint32_t seq);
		bool remove(netreach_key_t key, uint32_t seq);
		size_t range_scan(netreach_key_t startkey, netreach_key_t endkey, std::vector<std::pair<netreach_key_t, val_t>> &results);

	private:
		static rocksdb::Options rocksdb_options;

		rocksdb::TransactionDB *db_ptr;
		deleted_set_t deleted_set;

};

typedef RocksdbWrapper rocksdb_wrapper_t;

#endif
