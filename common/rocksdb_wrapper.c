#include "rocksdb_wrapper.h"

/***** for all ******/

rocksdb::Options RocksdbWrapper::rocksdb_options = rocksdb::Options();

void RocksdbWrapper::prepare_rocksdb() {

  rocksdb_options.create_if_missing = true; // create database if not exist
  rocksdb_options.enable_blob_files = false; // disable key-value separation
  
  // (1) parallism options
  // NOTE: all rocksdb instances of the process share the same thread pool
  //rocksdb_options.IncreaseParallelism(WRITE_PARALLISM);
  rocksdb_options.max_background_flushes = GLOBAL_MAX_FLUSH_THREAD_NUM; // flush thread number 
  rocksdb_options.env->SetBackgroundThreads(GLOBAL_MAX_FLUSH_THREAD_NUM, rocksdb::Env::Priority::HIGH);
  rocksdb_options.max_background_compactions = GLOBAL_MAX_COMPACTION_THREAD_NUM; // compaction thread number 
  rocksdb_options.env->SetBackgroundThreads(GLOBAL_MAX_COMPACTION_THREAD_NUM, rocksdb::Env::Priority::LOW);
  
  // (2) general options
  // NOTE: all rocksdb instances of the process share the same block cache
  rocksdb::BlockBasedTableOptions table_options;
  table_options.filter_policy = std::shared_ptr<const rocksdb::FilterPolicy>(rocksdb::NewBloomFilterPolicy(BLOOMFILTER_BITS_PER_KEY));
  table_options.block_cache = rocksdb::NewLRUCache(BLOCKCACHE_SIZE, BLOCKCACHE_SHARDBITS); // Block cache with uncompressed blocks: 1GB with 2^4 shards
  table_options.block_size = BLOCK_SIZE;
  rocksdb_options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));
  //rocksdb_options.allow_os_buffer = false; // Disable OS-cache (Not provided in current version of rocksdb now)
  rocksdb_options.max_open_files = -1; // always keep all files open
  rocksdb_options.table_cache_numshardbits = TABLECACHE_SHARDBITS;

  // (3) flushing options
  rocksdb_options.write_buffer_size = MEMTABLE_SIZE; // single memtable size
  // max # of memtable and immutable: if # = max-1, limit write speed; if # = max, forbid write operations
  rocksdb_options.max_write_buffer_number = MAX_MEMTABLE_IMMUTABLE_NUM;
  // min # of immutable to flush into disk: if # < min, no flush; if # >= min, flush
  rocksdb_options.min_write_buffer_number_to_merge = MIN_IMMUTABLE_FLUSH_NUM;

  // (4) level-style compaction
  // NOTE: read amplification = number_of_level0_files + number_of_non_empty_levels
  rocksdb_options.level0_file_num_compaction_trigger = LEVEL0_SST_NUM;
  rocksdb_options.target_file_size_base = LEVEL1_SST_SIZE; // single sst size
  rocksdb_options.max_bytes_for_level_base = LEVEL1_SST_NUM * LEVEL1_SST_SIZE;
  rocksdb_options.max_bytes_for_level_multiplier = LEVEL_TOTALSIZE_MULTIPLIER;
  rocksdb_options.target_file_size_multiplier = LEVEL_SSTSIZE_MULTIPLIER;
  // NOTE: we keep default rocksdb_options.compression_per_level
  rocksdb_options.num_levels = LEVEL_NUM; // level number

  // (5) other options
  rocksdb_options.compaction_style = rocksdb::kCompactionStyleLevel; // leveled compaction
  rocksdb_options.wal_bytes_per_sync = WAL_BYTES_PER_SYNC;
  ////rocksdb_options.max_successive_merges = 1000;
  //rocksdb_options.enable_write_thread_adaptive_yield = true;
  //rocksdb_options.allow_concurrent_memtable_write = true;

#ifdef DEBUG_ROCKSDB
  //rocksdb_options.statistics = rocksdb::CreateDBStatistics();
  rocksdb_options.stats_dump_period_sec = 1;
#endif
}

RocksdbWrapper::RocksdbWrapper() {
	methodid = INVALID_ID;

	db_ptr = NULL;

	// for farreach/distfarreach
	snapshotid = -1;
	sp_ptr = NULL;
	snapshotdb_ptr = NULL;
	latest_sp_ptr = NULL;
}

RocksdbWrapper::RocksdbWrapper(method_t curmethodid) {
	init(curmethodid);
}

RocksdbWrapper::~RocksdbWrapper() {
	// close runtime database
	if (db_ptr != NULL) {
		delete db_ptr;
		db_ptr = NULL;
	}

	// store runtime delete set into disk
	if (method_needsnapshot()) {
		std::string deletedset_path;
		get_server_deletedset_path(methodid, deletedset_path, workerid);
		rmfiles(deletedset_path.c_str());
		deleted_set.store(deletedset_path);

		// close snapshot database
		if (snapshotdb_ptr != NULL) {
			delete snapshotdb_ptr;
			snapshotdb_ptr = NULL;
			std::string snapshotdb_path;
			get_server_snapshotdb_path(methodid, snapshotdb_path, workerid);
			rmfiles(snapshotdb_path.c_str());
		}
	}
}

void RocksdbWrapper::init(method_t curmethodid) {
	methodid = curmethodid;
	INVARIANT(methodid != INVALID_ID);

	db_ptr = NULL;

	// for farreach/distfarreach
	snapshotid = -1;
	sp_ptr = NULL;
	snapshotdb_ptr = NULL;
	latest_sp_ptr = NULL;
}

/*RocksdbWrapper::RocksdbWrapper(uint16_t tmpworkerid) {
	open(tmpworkerid);
}*/

bool RocksdbWrapper::open(uint16_t tmpworkerid) {
	bool is_runtime_existing = false;
	bool is_snapshot_existing = false;
	workerid = tmpworkerid;

	// open database
	std::string db_path;
	get_server_db_path(methodid, db_path, tmpworkerid);
	is_runtime_existing = isexist(db_path);
	//rocksdb::Status s = rocksdb::TransactionDB::Open(rocksdb_options, rocksdb::TransactionDBOptions(), db_path, &db_ptr);
	rocksdb::Status s = rocksdb::DB::Open(rocksdb_options, db_path, &db_ptr);
	if (!s.ok()) {
		printf("Please create directory for %s, %s\n", db_path.c_str(), s.ToString().c_str());
		exit(-1);
	}
	INVARIANT(db_ptr != NULL);

	if (method_needsnapshot()) {
		// load deleted set
		std::string deletedset_path;
		get_server_deletedset_path(methodid, deletedset_path, tmpworkerid);
		if (is_runtime_existing) {
			if (isexist(deletedset_path)) {
				deleted_set.load(deletedset_path);
			}
			else {
				printf("[WARNING] no deleted set for non-empty server %d\n", tmpworkerid);
				deleted_set.clear();
			}
		}
		else {
			if (isexist(deletedset_path)) {
				printf("[WARNING] with deleted set for empty server %d\n", tmpworkerid);
			}
			deleted_set.clear();
		}

		// load snapshot id
		std::string snapshotid_path;
		get_server_snapshotid_path(methodid, snapshotid_path, tmpworkerid);
		is_snapshot_existing = isexist(snapshotid_path);
		if (is_snapshot_existing) {
			load_snapshotid(snapshotid, snapshotid_path);
			INVARIANT(snapshotid >= 0);
		}
		else {
			snapshotid = -1;
		}

		// load snapshot data from disk if any
		if (is_snapshot_existing) {
			// set snapshotdb_ptr, snapshot_deleted_set, and inswitch_snapshot; reset sp_ptr
			load_snapshot_files(snapshotid);

			latest_sp_ptr = NULL;
		}
		else {
			snapshotdb_ptr = NULL;
			snapshot_deleted_set.clear();
			inswitch_snapshot.clear();
			sp_ptr = NULL;

			latest_sp_ptr = NULL;
			latest_snapshot_deleted_set.clear();
		}
	}

	return is_runtime_existing;
}

// loading phase

bool RocksdbWrapper::force_multiput(netreach_key_t *keys, val_t *vals, int maxidx) {
	rocksdb::Status s;
	rocksdb::WriteOptions write_options;
	write_options.sync = SYNC_WRITE; // Write through for persistency
	write_options.disableWAL = DISABLE_WAL;
	//rocksdb::Transaction* txn = db_ptr->BeginTransaction(write_options, rocksdb::TransactionOptions());

	for (int i = 0; i < maxidx; i++) {
		std::string keystr = keys[i].to_string_for_rocksdb();
		std::string valstr = vals[i].to_string_for_rocksdb(0);
		//s = txn->Put(keystr, valstr);
		s = db_ptr->Put(write_options, keystr, valstr);
	}
	//s = txn->Commit();
	INVARIANT(s.ok());

	//delete txn;
	//txn = NULL;
	return true;
}

bool RocksdbWrapper::force_put(netreach_key_t key, val_t val) {
	rocksdb::Status s;
	std::string valstr = val.to_string_for_rocksdb(0);
	rocksdb::WriteOptions write_options;
	write_options.sync = SYNC_WRITE; // Write through for persistency
	write_options.disableWAL = DISABLE_WAL;
	//rocksdb::Transaction* txn = db_ptr->BeginTransaction(write_options, rocksdb::TransactionOptions());

	std::string keystr = key.to_string_for_rocksdb();
	//s = txn->Put(keystr, valstr);
	//s = txn->Commit();
	s = db_ptr->Put(write_options, keystr, valstr);
	INVARIANT(s.ok());

	//delete txn;
	//txn = NULL;
	return true;
}

// transaction phase

bool RocksdbWrapper::get(netreach_key_t key, val_t &val, uint32_t *seqptr) {
	//mutexlock.lock();
	while (true) {
		if (rwlock.try_lock_shared()) break;
	}

	rocksdb::Status s;
	std::string valstr;
	//rocksdb::Transaction* txn = db_ptr->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
	//INVARIANT(txn != nullptr);

	//read_options.fill_cache = false; // Bypass OS-cache (page cache), use block cache only
	//s = txn->Get(rocksdb::ReadOptions(), key.to_string_for_rocksdb(), &valstr);
	//s = txn->Commit();
	//delete txn;
	//txn = NULL;
	s = db_ptr->Get(rocksdb::ReadOptions(), key.to_string_for_rocksdb(), &valstr);

	bool stat = false;
	if (method_needseq()) {
		INVARIANT(seqptr != NULL);
		*seqptr = 0;
		if (valstr != "") {
			*seqptr = val.from_string_for_rocksdb(valstr);
			INVARIANT(s.ok());
			stat = true;
		}
	}
	else {
		if (valstr != "") {
			val.from_string_for_rocksdb_noseq(valstr);
			INVARIANT(s.ok());
			stat = true;
		}
	}

	//mutexlock.unlock();
	rwlock.unlock_shared();
	return stat;
}

bool RocksdbWrapper::put(netreach_key_t key, val_t val, uint32_t seq, bool checkseq) {
	//mutexlock.lock();
	while (true) {
		if (rwlock.try_lock()) break;
	}

	// check and update seq of deleted keys
	if (method_needsnapshot()) {
		uint32_t deleted_seq = 0;
		bool is_deleted = deleted_set.check_and_remove(key, seq, &deleted_seq);
		if (is_deleted) {
			INVARIANT(deleted_seq > seq);
			//mutexlock.unlock();
			rwlock.unlock();
			return false;
		}
	}

	rocksdb::Status s;
	std::string valstr = val.to_string_for_rocksdb(seq);
	rocksdb::WriteOptions write_options;
	write_options.sync = SYNC_WRITE; // Write through for persistency
	write_options.disableWAL = DISABLE_WAL;
	//rocksdb::Transaction* txn = db_ptr->BeginTransaction(write_options, rocksdb::TransactionOptions());
	std::string keystr = key.to_string_for_rocksdb();

	// check seq of undeleted keys if necessary
	if (method_needseq() && checkseq) {
		uint32_t tmp_seq = 0;
		std::string tmp_valstr;
		//s = txn->Get(rocksdb::ReadOptions(), keystr, &tmp_valstr);
		s = db_ptr->Get(rocksdb::ReadOptions(), keystr, &tmp_valstr);
		if (tmp_valstr != "") {
			val_t tmp_val;
			tmp_seq = tmp_val.from_string_for_rocksdb(tmp_valstr);
		}

		if (method_needsnapshot()) {
			uint32_t deleted_seq = 0;
			bool is_deleted = deleted_set.check_and_remove(key, tmp_seq, &deleted_seq);
			if (is_deleted && deleted_seq > tmp_seq) {
				//INVARIANT(deleted_seq > seq);
				tmp_seq = deleted_seq;
			}
		}

		if (tmp_seq > seq) {
			//s = txn->Commit();
			INVARIANT(s.ok());
			//delete txn;
			//txn = NULL;

			//mutexlock.unlock();
			rwlock.unlock();
			return true;
		}
	}

#ifdef DEBUG_ROCKSDB
	struct timespec debug_t1, debug_t2, debug_t3;
	rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
	rocksdb::get_perf_context()->Reset();
	rocksdb::get_iostats_context()->Reset();
	CUR_TIME(debug_t1);
#endif
	// update seq of undeleted keys
	//s = txn->Put(keystr, valstr);
	//s = txn->Commit();
	s = db_ptr->Put(write_options, keystr, valstr);
	INVARIANT(s.ok());
	//delete txn;
	//txn = NULL;
#ifdef DEBUG_ROCKSDB
	CUR_TIME(debug_t2);
	rocksdb::SetPerfLevel(rocksdb::PerfLevel::kDisable);
	DELTA_TIME(debug_t2, debug_t1, debug_t3);
	int abnormal_usecs = 0.5 * 1000 * 1000; // 0.5s
	if (GET_MICROSECOND(debug_t3) > abnormal_usecs) {
		COUT_THIS(rocksdb::get_perf_context()->ToString());
		COUT_THIS(rocksdb::get_iostats_context()->ToString());
		printf("\n");
	}
#endif

	//mutexlock.unlock();
	rwlock.unlock();
	return true;
}

bool RocksdbWrapper::remove(netreach_key_t key, uint32_t seq, bool checkseq) {
	//mutexlock.lock();
	while (true) {
		if (rwlock.try_lock()) break;
	}

	// check and update seq of deleted keys
	if (method_needsnapshot()) {
		uint32_t deleted_seq = 0;
		bool is_deleted = deleted_set.check_and_remove(key, seq, &deleted_seq);
		if (is_deleted) {
			INVARIANT(deleted_seq > seq);
			//mutexlock.unlock();
			rwlock.unlock();
			return false;
		}
	}

	rocksdb::Status s;
	rocksdb::WriteOptions write_options;
	write_options.sync = SYNC_WRITE; // Write through for persistency
	write_options.disableWAL = DISABLE_WAL;
	//rocksdb::Transaction* txn = db_ptr->BeginTransaction(write_options, rocksdb::TransactionOptions());
	std::string keystr = key.to_string_for_rocksdb();

	// check seq of undeleted keys if necessary
	if (method_needseq() && checkseq) {
		uint32_t tmp_seq = 0;
		std::string tmp_valstr;
		//s = txn->Get(rocksdb::ReadOptions(), keystr, &tmp_valstr);
		s = db_ptr->Get(rocksdb::ReadOptions(), keystr, &tmp_valstr);
		if (tmp_valstr != "") {
			val_t tmp_val;
			tmp_seq = tmp_val.from_string_for_rocksdb(tmp_valstr);
		}

		if (method_needsnapshot()) {
			uint32_t deleted_seq = 0;
			bool is_deleted = deleted_set.check_and_remove(key, tmp_seq, &deleted_seq);
			if (is_deleted && deleted_seq > tmp_seq) {
				//INVARIANT(deleted_seq > seq);
				tmp_seq = deleted_seq;
			}
		}

		if (tmp_seq > seq) {
			//s = txn->Commit();
			INVARIANT(s.ok());
			//delete txn;
			//txn = NULL;

			//mutexlock.unlock();
			rwlock.unlock();
			return true;
		}
	}

	// update seq pf undeleted keys
	//s = txn->Delete(keystr);
	//s = txn->Commit();
	s = db_ptr->Delete(write_options, keystr);
	INVARIANT(s.ok());
	//delete txn;
	//txn = NULL;

	if (method_needsnapshot()) {
		deleted_set.add(key, seq);
	}

	//mutexlock.unlock();
	rwlock.unlock();
	return true;
}

size_t RocksdbWrapper::range_scan(netreach_key_t startkey, netreach_key_t endkey, std::vector<std::pair<netreach_key_t, snapshot_record_t>> &results) {
	//mutexlock.lock();
	if (method_needsnapshot()) {
		while (true) {
			if (rwlock_for_snapshot.try_lock_shared()) break;
		}
		INVARIANT(sp_ptr != NULL || snapshotdb_ptr != NULL);
	}

	rocksdb::Status s;
	//rocksdb::Transaction* txn = NULL;
	rocksdb::Iterator* iter = NULL;
	rocksdb::ReadOptions read_options;
	if (method_needsnapshot()) {
		if (sp_ptr != NULL) {
			//txn = db_ptr->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
			read_options.snapshot = sp_ptr;
			//iter = txn->GetIterator(read_options);
			iter = db_ptr->NewIterator(read_options);
		}
		else if (snapshotdb_ptr != NULL) {
			//txn = snapshotdb_ptr->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
			//iter = txn->GetIterator(read_options);
			iter = snapshotdb_ptr->NewIterator(read_options);
		}
	}
	else {
		iter = db_ptr->NewIterator(read_options);
	}
	INVARIANT(iter != NULL);

	// get results from snapshot database
	std::vector<std::pair<netreach_key_t, snapshot_record_t>> db_results;
	std::string startkeystr = startkey.to_string_for_rocksdb();
	std::string endkeystr = endkey.to_string_for_rocksdb();
	iter->Seek(startkeystr);
	while (iter->Valid()) {
		std::string tmpkeystr = iter->key().ToString();
		netreach_key_t tmpkey;
		tmpkey.from_string_for_rocksdb(tmpkeystr);
		if (tmpkey > endkey) {
			break;
		}
		std::string tmpvalstr = iter->value().ToString();
		INVARIANT(tmpvalstr != "");

		val_t tmpval;
		uint32_t tmpseq = 0;
		bool tmpstat = false;
	
		//if (tmpvalstr != "") {
		tmpseq = tmpval.from_string_for_rocksdb(tmpvalstr);
		tmpstat = true;
		//}

		snapshot_record_t tmprecord;
		tmprecord.val = tmpval;
		tmprecord.seq = tmpseq;
		tmprecord.stat = tmpstat;

		db_results.push_back(std::pair<netreach_key_t, snapshot_record_t>(tmpkey, tmprecord));
		iter->Next();
	}
	//s = txn->Commit();
	//INVARIANT(s.ok());
	//delete txn;
	//txn = NULL;

	if (method_needsnapshot()) {
		// get results from snapshot deleted set
		std::vector<std::pair<netreach_key_t, snapshot_record_t>> deleted_results;
		snapshot_deleted_set.range_scan(startkey, endkey, deleted_results);

		// get results from inswitch snapshot
		std::vector<std::pair<netreach_key_t, snapshot_record_t>> inswitch_results;
		std::map<netreach_key_t, snapshot_record_t>::iterator inswitch_snapshot_iter = inswitch_snapshot.lower_bound(startkey);
		for (; inswitch_snapshot_iter != inswitch_snapshot.end() && inswitch_snapshot_iter->first <= endkey; inswitch_snapshot_iter++) {
			inswitch_results.push_back(*inswitch_snapshot_iter);
		}

		//mutexlock.unlock();
		rwlock_for_snapshot.unlock_shared();

		// merge sort between snapshotdb and snapshot deleted set
		std::vector<std::pair<netreach_key_t, snapshot_record_t>> serverside_results;
		merge_sort(db_results, deleted_results, serverside_results, false); // hold records with stat = true/false

		// merge sort between serverside and inswitch snapshot
		results.clear();
		merge_sort(serverside_results, inswitch_results, results, true); // only hold records with stat = true
	}
	else {
		results = db_results;
	}

	return results.size();
}

bool RocksdbWrapper::method_needseq() const {
	INVARIANT(methodid != INVALID_ID);
	return methodid == NETCACHE_ID || methodid == DISTCACHE_ID || methodid == FARREACH_ID || methodid == DISTFARREACH_ID;
}

bool RocksdbWrapper::method_needsnapshot() const {
	INVARIANT(methodid != INVALID_ID);
	return methodid == FARREACH_ID || methodid == DISTFARREACH_ID;
}

/***** for farreach/distfarreaech *****/

int RocksdbWrapper::get_snapshotid() const {
	INVARIANT(method_needsnapshot());
	return snapshotid;
}

// snapshot

// invoked by WARMUPREQ after loading phase (only the first WARMUPREQ could success; create server-side snapshot of snapshot id 0)
void RocksdbWrapper::init_snapshot() {
	INVARIANT(method_needsnapshot());
	if (snapshotid != -1) {
		return;
	}

	if (!is_snapshot.test_and_set(std::memory_order_acquire)) {
		if (snapshotid != -1) { // check one more time
			is_snapshot.clear(std::memory_order_release);
			return;
		}

		// make snapshot of database
		sp_ptr = db_ptr->GetSnapshot();
		uint64_t snapshotdbseq = sp_ptr->GetSequenceNumber();
		INVARIANT(sp_ptr != NULL);
		INVARIANT(snapshotdb_ptr == NULL); // checkpointdb must be null after loading phase

		// snapshot of deleted set must be empty after loading phase
		//INVARIANT(deleted_set.size() == 0);
		INVARIANT(snapshot_deleted_set.size() == 0);

		// in-switch snapshot must be empty after loading phase
		INVARIANT(inswitch_snapshot.size() == 0);

		// latest server-side snapshot must be empty after loading phase
		INVARIANT(latest_sp_ptr == NULL);
		INVARIANT(latest_snapshot_deleted_set.size() == 0);

		// increase snapshot id from -1 to 0
		snapshotid += 1;
		INVARIANT(snapshotid == 0);

		// store snapshot database seq (sp_ptr->dbseq) into disk
		std::string snapshotdbseq_path;
		get_server_snapshotdbseq_path(methodid, snapshotdbseq_path, workerid, snapshotid);
		store_snapshotdbseq(snapshotdbseq, snapshotdbseq_path);

		// store snapshot deleted set into disk (NOTE: empty now)
		std::string snapshotdeletedset_path;
		get_server_snapshotdeletedset_path(methodid, snapshotdeletedset_path, workerid, snapshotid);
		snapshot_deleted_set.store(snapshotdeletedset_path);
		
		// store inswitch snapshot (NOTE: empty now)
		store_inswitch_snapshot(snapshotid);

		// flush all memtables into disk after loading phase, which also clears WAL buffer (delete old WAL files)
		//db_ptr->Flush(rocksdb::FlushOptions());

		is_snapshot.clear(std::memory_order_release);
		return;
	}
}

// invoked by SNAPSHOT_CLEANUP
void RocksdbWrapper::clean_snapshot(int tmpsnapshotid) {
	INVARIANT(method_needsnapshot());
	if (snapshotid == -1) {
		printf("[WARNING] you should run loadfinish_client after loading phase to create an initial server-side snapshot!\n");
		init_snapshot();
	}

	// common case: tmpsnapshotid == snapshotid + 1; rare case tmpsnapshotid == snapshotid due to controller failure
	INVARIANT(tmpsnapshotid == snapshotid || tmpsnapshotid == snapshotid + 1);

	// remove snapshot files of tmpsnapshotid
	int latest_snapshotid = tmpsnapshotid;
	remove_snapshot_files(latest_snapshotid);

	// remove latest server-side snapshot memory
	if (latest_sp_ptr != NULL) {
		db_ptr->ReleaseSnapshot(latest_sp_ptr);
		latest_sp_ptr = NULL;
	}
	latest_snapshot_deleted_set.clear();

	if (unlikely(tmpsnapshotid == snapshotid)) {
		while (true) {
			if (rwlock_for_snapshot.try_lock()) break;
		}

		// close snapshot database and clear snapshot deleted set
		if (sp_ptr != NULL) {
			db_ptr->ReleaseSnapshot(sp_ptr); // TODO: need protect db_ptr here?
			sp_ptr = NULL;
		}
		snapshot_deleted_set.clear();
		inswitch_snapshot.clear();

		// load snapshot of snapshotid-1
		int previous_snapshotid = snapshotid - 1;
		load_snapshot_files(previous_snapshotid);

		snapshotid -= 1;

		// NOTE: we can unlock rwlock_for_snapshot after updating sp_ptr, snapshotdb_ptr, and snapshot_deleted_set -> limited effect on range_scan
		rwlock_for_snapshot.unlock();
	}
}

// create a new server-side snapshot yet not used by range query (as in-switch snapshot is stale)
// invoked by worker due to case3 (tmpsnapshotid == 0); invoked by SNAPSHOT_START (tmpsnapshotid == snapshotid/snapshotid+1)
void RocksdbWrapper::make_snapshot(int tmpsnapshotid) {
	INVARIANT(method_needsnapshot());
	INVARIANT(tmpsnapshotid >= 0);
	
	// NOTE: if tmpsnapshotid == snapshotid, it has been solevd by SNAPSHOT_CLEANUP
	INVARIANT(tmpsnapshotid == 0 || tmpsnapshotid == snapshotid + 1)

	if (!is_snapshot.test_and_set(std::memory_order_acquire)) {

		//struct timespec create_t1, create_t2, create_t3, store_t1, store_t2, store_t3;
		//CUR_TIME(create_t1);

		// create new snapshot database and snapshot deleted set
		while (true) {
			if (rwlock.try_lock_shared()) break;
		}
		// make snapshot of database
		latest_sp_ptr = db_ptr->GetSnapshot();
		uint64_t latest_snapshotdbseq = latest_sp_ptr->GetSequenceNumber();
		INVARIANT(latest_sp_ptr != NULL);
		// make snapshot of deleted set
		latest_snapshot_deleted_set = deleted_set;

		// NOTE: we can unlock rwlock after accessing db_ptr and deleted_set -> limited effect on put/delete
		rwlock.unlock_shared();

		//CUR_TIME(create_t2);
		//CUR_TIME(store_t1);

		// TODO: (1) we should store a checkpoint database of latest_snapshotdbseq with latest_snapshotid? but how to keep time efficiency instead of bottlenecked by disk?
		// TODO: (2) can we use stored seq to create a snapshot when opening? (3) we need to check whether records before/after seq will be merged?

		// store snapshot database seq (latest_sp_ptr->dbseq) into disk
		std::string latest_snapshotdbseq_path;
		get_server_snapshotdbseq_path(methodid, latest_snapshotdbseq_path, workerid, snapshotid+1);
		store_snapshotdbseq(latest_snapshotdbseq, latest_snapshotdbseq_path);

		// store snapshot deleted set into disk
		std::string latest_snapshotdeletedset_path;
		get_server_snapshotdeletedset_path(methodid, latest_snapshotdeletedset_path, workerid, snapshotid+1);
		snapshot_deleted_set.store(latest_snapshotdeletedset_path);

		//CUR_TIME(store_t2);
		//DELTA_TIME(create_t2, create_t1, create_t3);
		//DELTA_TIME(store_t2, store_t1, store_t3);
		//COUT_VAR(GET_MICROSECOND(create_t3));
		//COUT_VAR(GET_MICROSECOND(store_t3));
	}

	return;
}

// use latest server-side and in-switch snapshot of snapshotid+1 for range query after receiving latest in-switch snapshot
// invoked by SNAPSHOT_SENDDATA; (deprecated: or invoked by make_snapshot under rare case due to controller failure)
void RocksdbWrapper::update_snapshot(std::map<netreach_key_t, snapshot_record_t> &tmp_inswitch_snapshot, int tmpsnapshotid) {
	INVARIANT(method_needsnapshot());
	INVARIANT(tmpsnapshotid == snapshotid + 1);

	while (true) {
		if (rwlock_for_snapshot.try_lock()) break;
	}

	// close snapshot database and clear snapshot deleted set
	if (sp_ptr != NULL) {
		db_ptr->ReleaseSnapshot(sp_ptr); // TODO: need protect db_ptr here?
		sp_ptr = NULL;
	}
	// NOTE: we resort deconstructor to close and remove snapshotdb to save time in make_snapshot()
	/*if (snapshotdb_ptr != NULL) {
		delete snapshotdb_ptr;
		snapshotdb_ptr = NULL;
		std::string snapshotdb_path;
		get_server_snapshotdb_path(methodid, snapshotdb_path, workerid);
		rmfiles(snapshotdb_path.c_str());
	}*/
	snapshot_deleted_set.clear();
	inswitch_snapshot.clear();

	// update snapshot database and snapshot deleted set
	if (likely(latest_sp_ptr != NULL)) {
		sp_ptr = latest_sp_ptr;
		snapshot_deleted_set = latest_snapshot_deleted_set;
	}
	else { // both latest_sp_ptr and sp_ptr are NULL here (rare)
		// load snapshot of snapshotid+1
		load_serverside_snapshot_files(snapshotid + 1);
	}
	latest_sp_ptr = NULL;
	latest_snapshot_deleted_set.clear();

	// update inswitch snapshot
	inswitch_snapshot = tmp_inswitch_snapshot;

	// update snapshotid
	snapshotid += 1;

	// NOTE: we can unlock rwlock_for_snapshot after updating sp_ptr, snapshotdb_ptr, and snapshot_deleted_set -> limited effect on range_scan
	rwlock_for_snapshot.unlock();

	// store inswitch snapshot
	store_inswitch_snapshot(snapshotid);

	// store snapshotid at last (snapshotid for range query)
	std::string snapshotid_path;
	get_server_snapshotid_path(methodid, snapshotid_path, workerid);
	store_snapshotid(snapshotid, snapshotid_path);
		
	// remove old-enough snapshot data w/ snapshotid-2 (holding snapshots of snapshotid and snapshotid-1)
	if (snapshotid >= 2) {
		int old_snapshotid = snapshotid - 2;
		remove_snapshot_files(old_snapshotid);
	}
}

// invoked by SNAPSHOT_SENDDATA
void RocksdbWrapper::stop_snapshot() {
	INVARIANT(method_needsnapshot());
	is_snapshot.clear(std::memory_order_release);
	return;
}

// snapshot utils

// recover snapshot database on the dbseq by checkpoint (set snapshotdb_ptr)
// invoked by constructor; or invoked by update_snapshot/clean_snapshot (rare case)
void RocksdbWrapper::create_snapshotdb_checkpoint(uint64_t snapshotdbseq) {
	INVARIANT(method_needsnapshot());
	INVARIANT(db_ptr != NULL);
	rocksdb::Checkpoint *checkpoint_ptr = NULL;
	rocksdb::Status s = rocksdb::Checkpoint::Create(db_ptr, &checkpoint_ptr);
	INVARIANT(s.ok());
	INVARIANT(checkpoint_ptr != NULL);

	std::string snapshotdb_path;
	get_server_snapshotdb_path(methodid, snapshotdb_path, workerid);
	if (snapshotdb_ptr != NULL) {
		delete snapshotdb_ptr;
		snapshotdb_ptr = NULL;
	}
	rmfiles(snapshotdb_path.c_str());

	// log_size_for_flush = 0; sequence_number_ptr = &snapshotdbseq
	s = checkpoint_ptr->CreateCheckpoint(snapshotdb_path, 0, &snapshotdbseq); // snapshot database has been stored into disk by hardlink
	INVARIANT(s.ok());
	//s = rocksdb::TransactionDB::Open(rocksdb_options, rocksdb::TransactionDBOptions(), snapshotdb_path, &snapshotdb_ptr);
	s = rocksdb::DB::Open(rocksdb_options, snapshotdb_path, &snapshotdb_ptr);
	INVARIANT(s.ok());
	INVARIANT(snapshotdb_ptr != NULL);
	return;
}

void RocksdbWrapper::load_serverside_snapshot_files(int tmpsnapshotid) {
	INVARIANT(method_needsnapshot());
	std::string snapshotdbseq_path;
	get_server_snapshotdbseq_path(methodid, snapshotdbseq_path, workerid, tmpsnapshotid);
	INVARIANT(isexist(snapshotdbseq_path));
	uint64_t snapshotdbseq = 0;
	load_snapshotdbseq(snapshotdbseq, snapshotdbseq_path);
	create_snapshotdb_checkpoint(snapshotdbseq); // set snapshotdb_ptr
	if (sp_ptr != NULL) {
		db_ptr->ReleaseSnapshot(sp_ptr); // TODO: need protect db_ptr here?
		sp_ptr = NULL;
	}

	std::string snapshotdeletedset_path;
	get_server_snapshotdeletedset_path(methodid, snapshotdeletedset_path, workerid, tmpsnapshotid);
	INVARIANT(isexist(snapshotdeletedset_path));
	snapshot_deleted_set.load(snapshotdeletedset_path);
	return;
}

void RocksdbWrapper::load_inswitch_snapshot(int tmpsnapshotid) {
	INVARIANT(method_needsnapshot());
	std::string inswitchsnapshot_path;
	get_server_inswitchsnapshot_path(methodid, inswitchsnapshot_path, workerid, tmpsnapshotid);
	if (unlikely(!isexist(inswitchsnapshot_path))) {
		printf("no inswitch snapshot: %s\n", inswitchsnapshot_path.c_str());
		return;
	}

	// <int recordcnt, key1, value1, seq1, stat1, ..., keyn, valuen, seqn, statn>
	uint32_t tmpfilesize = get_filesize(inswitchsnapshot_path);
	char *tmpbuf = readonly_mmap(inswitchsnapshot_path, 0, tmpfilesize);
	int tmpbuflen = 0;

	int tmprecordcnt = *((int *)tmpbuf);
	tmpbuflen += sizeof(int);

	inswitch_snapshot.clear();
	for (int tmprecordidx = 0; tmprecordidx < tmprecordcnt; tmprecordidx++) {
		netreach_key_t tmpkey;
		snapshot_record_t tmprecord;

		uint32_t tmpkeysize = tmpkey.deserialize(tmpbuf + tmpbuflen, tmpfilesize - tmpbuflen);
		tmpbuflen += tmpkeysize;
		uint32_t tmpvalsize = tmprecord.val.deserialize(tmpbuf + tmpbuflen, tmpfilesize - tmpbuflen);
		tmpbuflen += tmpvalsize;
		tmprecord.seq = *((uint32_t *)(tmpbuf + tmpbuflen));
		tmpbuflen += sizeof(uint32_t);
		tmprecord.stat = *((bool *)(tmpbuf + tmpbuflen));
		tmpbuflen += sizeof(bool);

		inswitch_snapshot.insert(std::pair<netreach_key_t, snapshot_record_t>(tmpkey, tmprecord));
	}

	munmap(tmpbuf, tmpfilesize);
	return;
}

void RocksdbWrapper::load_snapshot_files(int tmpsnapshotid) {
	INVARIANT(method_needsnapshot());
	load_serverside_snapshot_files(tmpsnapshotid);
	load_inswitch_snapshot(tmpsnapshotid);
	return;
}

void RocksdbWrapper::store_inswitch_snapshot(int tmpsnapshotid) {
	INVARIANT(method_needsnapshot());
	std::string inswitchsnapshot_path;
	get_server_inswitchsnapshot_path(methodid, inswitchsnapshot_path, workerid, tmpsnapshotid);

	// <int recordcnt, key1, value1, seq1, stat1, ..., keyn, valuen, seqn, statn>
	char *tmpbuf = new char[MAX_LARGE_BUFSIZE];
	int tmpbuflen = 0;

	int tmprecordcnt = inswitch_snapshot.size();
	memcpy(tmpbuf, &tmprecordcnt, sizeof(int));
	tmpbuflen += sizeof(int);

	for (std::map<netreach_key_t, snapshot_record_t>::iterator iter = inswitch_snapshot.begin();
			iter != inswitch_snapshot.end(); iter++) {
		uint32_t tmpkeysize = iter->first.serialize(tmpbuf + tmpbuflen, MAX_LARGE_BUFSIZE - tmpbuflen);
		tmpbuflen += tmpkeysize;
		uint32_t tmpvalsize = iter->second.val.serialize(tmpbuf + tmpbuflen, MAX_LARGE_BUFSIZE - tmpbuflen);
		tmpbuflen += tmpvalsize;
		memcpy(tmpbuf + tmpbuflen, &iter->second.seq, sizeof(uint32_t));
		tmpbuflen += sizeof(uint32_t);
		memcpy(tmpbuf + tmpbuflen, &iter->second.stat, sizeof(bool));
		tmpbuflen += sizeof(bool);
	}

	store_buf(tmpbuf, tmpbuflen, inswitchsnapshot_path);
	delete [] tmpbuf;
	tmpbuf = NULL;
	return;
}

void RocksdbWrapper::remove_snapshot_files(int tmpsnapshotid) {
	INVARIANT(method_needsnapshot());
	std::string snapshotdbseq_path;
	get_server_snapshotdbseq_path(methodid, snapshotdbseq_path, workerid, tmpsnapshotid);
	rmfiles(snapshotdbseq_path.c_str());

	std::string snapshotdeletedset_path;
	get_server_snapshotdeletedset_path(methodid, snapshotdeletedset_path, workerid, tmpsnapshotid);
	rmfiles(snapshotdeletedset_path.c_str());

	std::string inswitchsnapshot_path;
	get_server_inswitchsnapshot_path(methodid, inswitchsnapshot_path, workerid, tmpsnapshotid);
	rmfiles(inswitchsnapshot_path.c_str());
	return;
}

// merge sort between snapshot db and snapshot deleted set: need_exist = false
// merge sort between server-side snapshot and in-switch snapshot: need_exist = true
void RocksdbWrapper::merge_sort(const std::vector<std::pair<netreach_key_t, snapshot_record_t>> &veca, const std::vector<std::pair<netreach_key_t, snapshot_record_t>> &vecb, std::vector<std::pair<netreach_key_t, snapshot_record_t>> &results, bool need_exist) {
	INVARIANT(method_needsnapshot());
	if (veca.size() == 0) {
		if (!need_exist) {
			results = vecb;
		}
		else {
			for (size_t i = 0; i < vecb.size(); i++) {
				if (vecb[i].second.stat) {
					results.push_back(vecb[i]);
				}
			}
		}
	}
	else if (vecb.size() == 0) {
		if (!need_exist) {
			results = veca;
		}
		else {
			for (size_t i = 0; i < veca.size(); i++) {
				if (veca[i].second.stat) {
					results.push_back(veca[i]);
				}
			}
		}
	}
	else {
		uint32_t veca_idx = 0;
		uint32_t vecb_idx = 0;
		bool remain_veca = false;
		bool remain_vecb = false;
		while (true) {
			const netreach_key_t &tmp_veca_key = veca[veca_idx].first;
			const snapshot_record_t &tmp_veca_record = veca[veca_idx].second;
			const netreach_key_t &tmp_vecb_key = vecb[vecb_idx].first;
			const snapshot_record_t &tmp_vecb_record = vecb[vecb_idx].second;
			if (tmp_veca_key < tmp_vecb_key) {
				if (!need_exist || tmp_veca_record.stat) {
					results.push_back(veca[veca_idx]);
				}
				veca_idx += 1;
			}
			else if (tmp_veca_key > tmp_vecb_key) {
				if (!need_exist || tmp_vecb_record.stat) {
					results.push_back(vecb[vecb_idx]);
				}
				vecb_idx += 1;
			}
			else {
				//printf("[RocksdbWrapper] deleted set and database cannot have the same key!\n");
				//exit(-1);
				INVARIANT(need_exist == true); // between serverside and inswitch snapshot

				if (tmp_veca_record.seq >= tmp_vecb_record.seq) {
					if (!need_exist || tmp_veca_record.stat) {
						results.push_back(veca[veca_idx]);
					}
				}
				else {
					if (!need_exist || tmp_vecb_record.stat) {
						results.push_back(vecb[vecb_idx]);
					}
				}
				veca_idx += 1;
				vecb_idx += 1;
			}
			if (veca_idx >= veca.size()) {
				remain_vecb = true;
				break;
			}
			if (vecb_idx >= vecb.size()) {
				remain_veca = true;
				break;
			}
		}
		if (remain_veca) {
			for (; veca_idx < veca.size(); veca_idx++) {
				if (!need_exist || veca[veca_idx].second.stat) {
					results.push_back(veca[veca_idx]);
				}
			}
		}
		else if (remain_vecb) {
			for (; vecb_idx < vecb.size(); vecb_idx++) {
				if (!need_exist || vecb[vecb_idx].second.stat) {
					results.push_back(vecb[vecb_idx]);
				}
			}
		}
	}
}
