#include "rocksdb_wrapper.h"

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
	db_ptr = NULL;
}

RocksdbWrapper::~RocksdbWrapper() {
	// close runtime database
	if (db_ptr != NULL) {
		delete db_ptr;
		db_ptr = NULL;
	}
}

/*RocksdbWrapper::RocksdbWrapper(uint16_t tmpworkerid) {
	open(tmpworkerid);
}*/

bool RocksdbWrapper::open(uint16_t tmpworkerid) {
	bool is_runtime_existing = false;
	workerid = tmpworkerid;

	// open database
	std::string db_path;
	get_server_db_path(db_path, tmpworkerid);
	is_runtime_existing = isexist(db_path);
	//rocksdb::Status s = rocksdb::TransactionDB::Open(rocksdb_options, rocksdb::TransactionDBOptions(), db_path, &db_ptr);
	rocksdb::Status s = rocksdb::DB::Open(rocksdb_options, db_path, &db_ptr);
	if (!s.ok()) {
		printf("Please create directory for %s\n", db_path.c_str());
		exit(-1);
	}
	INVARIANT(db_ptr != NULL);

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

bool RocksdbWrapper::get(netreach_key_t key, val_t &val, uint32_t &seq) {
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
	seq = 0;
	if (valstr != "") {
		seq = val.from_string_for_rocksdb(valstr);
		INVARIANT(s.ok());
		stat = true;
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

	rocksdb::Status s;
	std::string valstr = val.to_string_for_rocksdb(seq);
	rocksdb::WriteOptions write_options;
	write_options.sync = SYNC_WRITE; // Write through for persistency
	write_options.disableWAL = DISABLE_WAL;
	//rocksdb::Transaction* txn = db_ptr->BeginTransaction(write_options, rocksdb::TransactionOptions());
	std::string keystr = key.to_string_for_rocksdb();

	// check seq of undeleted keys if necessary
	if (checkseq) {
		uint32_t tmp_seq = 0;
		std::string tmp_valstr;
		//s = txn->Get(rocksdb::ReadOptions(), keystr, &tmp_valstr);
		s = db_ptr->Get(rocksdb::ReadOptions(), keystr, &tmp_valstr);
		if (tmp_valstr != "") {
			val_t tmp_val;
			tmp_seq = tmp_val.from_string_for_rocksdb(tmp_valstr);
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
	}

#ifdef DEBUG_ROCKSDB
	struct timespec debug_t1, debug_t2, debug_t3;
	rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
	rocksdb::get_perf_context()->Reset();
	rocksdb::get_iostats_context()->Reset();
	CUR_TIME(debug_t1);
#endif
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

	rocksdb::Status s;
	rocksdb::WriteOptions write_options;
	write_options.sync = SYNC_WRITE; // Write through for persistency
	write_options.disableWAL = DISABLE_WAL;
	//rocksdb::Transaction* txn = db_ptr->BeginTransaction(write_options, rocksdb::TransactionOptions());
	std::string keystr = key.to_string_for_rocksdb();

	// check seq of undeleted keys if necessary
	if (checkseq) {
		uint32_t tmp_seq = 0;
		std::string tmp_valstr;
		//s = txn->Get(rocksdb::ReadOptions(), keystr, &tmp_valstr);
		s = db_ptr->Get(rocksdb::ReadOptions(), keystr, &tmp_valstr);
		if (tmp_valstr != "") {
			val_t tmp_val;
			tmp_seq = tmp_val.from_string_for_rocksdb(tmp_valstr);
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
	}

	// update seq pf undeleted keys
	//s = txn->Delete(keystr);
	//s = txn->Commit();
	s = db_ptr->Delete(write_options, keystr);
	INVARIANT(s.ok());
	//delete txn;
	//txn = NULL;

	//mutexlock.unlock();
	rwlock.unlock();
	return true;
}

size_t RocksdbWrapper::range_scan(netreach_key_t startkey, netreach_key_t endkey, std::vector<std::pair<netreach_key_t, snapshot_record_t>> &results) {

	rocksdb::Status s;
	//rocksdb::Transaction* txn = NULL;
	rocksdb::Iterator* iter = NULL;
	rocksdb::ReadOptions read_options;
	iter = db_ptr->NewIterator(read_options);
	INVARIANT(iter != NULL);

	// get results from rocksdb database
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
	
	results = db_results;
	return results.size();
}
