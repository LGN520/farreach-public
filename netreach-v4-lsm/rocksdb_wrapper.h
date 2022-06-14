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
#include "rocksdb/utilities/transaction.h"
#include "rocksdb/utilities/transaction_db.h"
#include "rocksdb/utilities/checkpoint.h"

#include "helper.h"
#include "key.h"
#include "val.h"
#include "deleted_set_impl.h"
#include "snapshot_record.h"
#include "io_helper.h"

// Default configuration of rocksdb
#define MEMTABLE_SIZE 64 * 1024 * 1024 // x
#define MIN_IMMUTABLE_FLUSH_NUM 4
#define MAX_MEMTABLE_IMMUTABLE_NUM 5
#define SST_SIZE 64 * 1024 * 1024 // x
#define COMPACTION_THREAD_NUM 2
#define LEVEL0_SST_NUM 4
#define LEVEL_NUM 7
#define LEVEL1_TOTAL_SIZE 256 * 1024 * 1024 // 4x
#define LEVEL_MULTIPLIER 10
#define BLOCKCACHE_SIZE 64 * 1024 * 1024 // x
#define BLOCKCACHE_SHARDBITS 4
#define SYNC_WRITE false

class RocksdbWrapper {

	typedef DeletedSet<netreach_key_t, uint32_t> deleted_set_t;

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
		bool get(netreach_key_t key, val_t &val, uint32_t &seq);
		bool put(netreach_key_t key, val_t val, uint32_t seq);
		bool remove(netreach_key_t key, uint32_t seq);

		// transaction phase (per-server worker, evictserver, and consnapshotserver touch both rocksdb and deleted set; need mutex for atomicity)
		void clean_snapshot(int tmpsnapshotid);
		void make_snapshot(int tmpsnapshotid = 0);
		void update_snapshot(std::map<netreach_key_t, snapshot_record_t> &tmp_inswitch_snapshot, int tmpsnapshotid);
		void stop_snapshot();

		size_t range_scan(netreach_key_t startkey, netreach_key_t endkey, std::vector<std::pair<netreach_key_t, snapshot_record_t>> &results);
		int get_snapshotid() const;

	private:
		uint16_t workerid;

		// NOTE: we provide thread safety between per-server worker and evictserver; and there is no contention across workers of different servers
		// NOTE: lock overhead is ignorable compared with that of rocksdb
		
		/* server-side KVS */

		boost::shared_mutex rwlock; // protect db_ptr and deleted_set in get/put/remove/make_snapshot
		//std::mutex mutexlock; // protect db_ptr and deleted_set in get/put/remove/make_snapshot
		rocksdb::TransactionDB *db_ptr = NULL;
		deleted_set_t deleted_set;

		int snapshotid = -1; // to locate snapshot files

		/* snapshot data for range query */

		boost::shared_mutex rwlock_for_snapshot; // protect snapshotdata used by range query (including sp_ptr, snapshotdb_ptr, snapshot_deleted_set, and inswitch_snapshot) in range_scan/make_snapshot/update_snapshot
		// server-side snapshot
		const rocksdb::Snapshot *sp_ptr = NULL; // normal database snapshot 
		rocksdb::TransactionDB *snapshotdb_ptr = NULL; // database checkpoint to recover database snapshot from server crash
		deleted_set_t snapshot_deleted_set; // read-only, only used for range query
		// in-switch snapshot
		std::map<netreach_key_t, snapshot_record_t> inswitch_snapshot;

		/* latest server-side snapshot data */

  		std::atomic_flag is_snapshot = ATOMIC_FLAG_INIT; // protect is_snapshot and latest server-side snapshot in make_snapshot/stop_snapshot
		// save latest snapshot temporarily
		const rocksdb::Snapshot *latest_sp_ptr = NULL;
		deleted_set_t latest_snapshot_deleted_set;

		/* utils */

		void merge_sort(const std::vector<std::pair<netreach_key_t, snapshot_record_t>> &veca, const std::vector<std::pair<netreach_key_t, snapshot_record_t>> &vecb, std::vector<std::pair<netreach_key_t, snapshot_record_t>> &results, bool need_exist = false);
		void create_snapshotdb_checkpoint(uint64_t snapshotdbseq);
		void load_inswitch_snapshot(std::string inswitchsnapshot_path);
		void store_inswitch_snapshot(std::string inswitchsnapshot_path);
		void load_serverside_snapshot_files(int tmpsnapshotid);
		void load_snapshot_files(int tmpsnapshotid);
		void remove_snapshot_files(int tmpsnapshotid);
};

typedef RocksdbWrapper rocksdb_wrapper_t;

#endif
