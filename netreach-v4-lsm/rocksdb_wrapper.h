#ifndef ROCKSDB_WRAPPER_H
#define ROCKSDB_WRAPPER_H

//#include <mutex>
#include <atomic>
#include <boost/thread/shared_mutex.hpp>

#include "rocksdb/db.h"
#include "rocksdb/cache.h"
#include "rocksdb/table.h"
#include "rocksdb/utilities/transaction.h"
#include "rocksdb/utilities/transaction_db.h"
#include "rocksdb/utilities/checkpoint.h"

#include "helper.h"
#include "key.h"
#include "val.h"
#include "deleted_set_impl.h"
#include "snapshot_record.h"

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
		//RocksdbWrapper(uint16_t tmpworkerid);
		~RocksdbWrapper();

		bool open(uint16_t tmpworkerid);

		// loading phase (per-server loaders only touch rocksdb instead of deleted set; not need mutex for atomicity)
		bool force_put(netreach_key_t key, val_t val);
		bool force_multiput(netreach_key_t *keys, val_t *vals, int maxidx);
		
		// transaction phase (per-server worker and evictserver touch both rocksdb and deleted set; need mutex for atomicity)
		bool get(netreach_key_t key, val_t &val, uint32_t &seq);
		bool put(netreach_key_t key, val_t val, uint32_t seq);
		bool remove(netreach_key_t key, uint32_t seq);

		// transaction phase (per-server worker, evictserver, and consnapshotserver touch both rocksdb and deleted set; need mutex for atomicity)
		void make_snapshot();
		void stop_snapshot();
		// TODO: based on snapshot
		size_t range_scan(netreach_key_t startkey, netreach_key_t endkey, std::vector<std::pair<netreach_key_t, snapshot_record_t>> &results);

	private:
		static rocksdb::Options rocksdb_options;
		uint16_t workerid;

		// NOTE: we provide thread safety between per-server worker and evictserver; and there is no contention across workers of different servers
		// NOTE: lock overhead is ignorable compared with that of rocksdb
		
		boost::shared_mutex rwlock; // protect db_ptr and deleted_set in get/put/remove/make_snapshot
		//std::mutex mutexlock; // protect db_ptr and deleted_set in get/put/remove/make_snapshot
		rocksdb::TransactionDB *db_ptr = NULL;
		deleted_set_t deleted_set;

  		std::atomic_flag is_snapshot = ATOMIC_FLAG_INIT; // protect is_snapshot and snapshotid in make_snapshot/stop_snapshot
		int snapshotid = -1; // to locate snapshot files

		boost::shared_mutex rwlock_for_snapshot; // protect snapshotdata (including sp_ptr, snapshotdb_ptr, snapshot_deleted_set) in range_scan/make_snapshot
		// normal database snapshot 
		const rocksdb::Snapshot *sp_ptr = NULL;
		// database checkpoint to recover database snapshot from server crash
		rocksdb::TransactionDB *snapshotdb_ptr = NULL;
		deleted_set_t snapshot_deleted_set; // read-only, only used for range query

		inline void get_db_path(std::string &db_path, uint16_t tmpworkerid);
		inline void get_deletedset_path(std::string &deletedset_path, uint16_t tmpworkerid);
		inline void get_snapshotid_path(std::string &snapshotid_path, uint16_t tmpworkerid);
		inline void get_snapshotdb_path(std::string &snapshotdb_path, uint16_t tmpworkerid); // at most one snapshotdb for server-side recovery
		inline void get_snapshotdbseq_path(std::string &snapshotdbseq_path, uint16_t tmpworkerid, uint32_t tmpsnapshotid);
		inline void get_snapshotdeletedset_path(std::string &snapshotdeletedset_path, uint16_t tmpworkerid, uint32_t tmpsnapshotid);
};

typedef RocksdbWrapper rocksdb_wrapper_t;

#endif
