#include "../common/rocksdb_wrapper.h"

rocksdb::Options RocksdbWrapper::rocksdb_options = rocksdb::Options();

void RocksdbWrapper::prepare_rocksdb() {
  std::shared_ptr<rocksdb::Cache> cache = rocksdb::NewLRUCache(BLOCKCACHE_SIZE, BLOCKCACHE_SHARDBITS); // 4 MB with 2^4 shards
  rocksdb::BlockBasedTableOptions table_options;
  table_options.block_cache = cache;

  rocksdb_options.create_if_missing = true; // create database if not exist
  rocksdb_options.enable_blob_files = false; // disable key-value separation
  //rocksdb_options.allow_os_buffer = false; // Disable OS-cache (Not provided in current version of rocksdb now)
  rocksdb_options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options)); // Block cache with uncompressed blocks
  rocksdb_options.compaction_style = rocksdb::kCompactionStyleLevel; // leveled compaction
  rocksdb_options.write_buffer_size = MEMTABLE_SIZE; // single memtable size
  rocksdb_options.max_write_buffer_number = MAX_MEMTABLE_NUM; // memtable number
  rocksdb_options.target_file_size_base = SST_SIZE; // single sst size
  rocksdb_options.max_background_compactions = COMPACTION_THREAD_NUM; // compaction thread number
  rocksdb_options.level0_file_num_compaction_trigger = LEVEL0_SST_NUM; // sst number in level0
  rocksdb_options.num_levels = LEVEL_NUM; // level number
  rocksdb_options.max_bytes_for_level_base = LEVEL1_TOTAL_SIZE; // byte number in level1
  rocksdb_options.max_bytes_for_level_multiplier = LEVEL_MULTIPLIER;
  
  rocksdb_options.enable_write_thread_adaptive_yield = true;
  rocksdb_options.allow_concurrent_memtable_write = true;
}

RocksdbWrapper::RocksdbWrapper() {
	db_ptr = NULL;
	snapshotid = -1;
	sp_ptr = NULL;
	snapshotdb_ptr = NULL;
}

RocksdbWrapper::~RocksdbWrapper() {
	// close runtime database
	if (db_ptr != NULL) {
		delete db_ptr;
		db_ptr = NULL;
	}

	// store runtime delete set into disk
	std::string deletedset_path;
	get_server_deletedset_path(deletedset_path, workerid);
	rmfiles(deletedset_path.c_str());
	deleted_set.store(deletedset_path);

	// close snapshot database
	if (snapshotdb_ptr != NULL) {
		delete snapshotdb_ptr;
		snapshotdb_ptr = NULL;
		std::string snapshotdb_path;
		get_server_snapshotdb_path(snapshotdb_path, workerid);
		rmfiles(snapshotdb_path.c_str());
	}
}

/*RocksdbWrapper::RocksdbWrapper(uint16_t tmpworkerid) {
	open(tmpworkerid);
}*/

bool RocksdbWrapper::open(uint16_t tmpworkerid) {
	bool is_existing = false;
	workerid = tmpworkerid;

	// open database
	std::string db_path;
	get_server_db_path(db_path, tmpworkerid);
	is_existing = isexist(db_path);
	rocksdb::Status s = rocksdb::TransactionDB::Open(rocksdb_options, rocksdb::TransactionDBOptions(), db_path, &db_ptr);
	INVARIANT(s.ok());
	INVARIANT(db_ptr != NULL);

	// load deleted set
	std::string deletedset_path;
	get_server_deletedset_path(deletedset_path, tmpworkerid);
	if (is_existing) {
		INVARIANT(isexist(deletedset_path));
		deleted_set.load(deletedset_path);
	}
	else {
		INVARIANT(!isexist(deletedset_path));
		deleted_set.clear();
	}

	// load snapshot id
	std::string snapshotid_path;
	get_server_snapshotid_path(snapshotid_path, tmpworkerid);
	if (is_existing) {
		INVARIANT(isexist(snapshotid_path));
		load_snapshotid(snapshotid, snapshotid_path);
	}
	else {
		INVARIANT(!isexist(snapshotid_path));
		snapshotid = -1;
	}

	// load snapshot dbseq
	std::string snapshotdbseq_path;
	get_server_snapshotdbseq_path(snapshotdbseq_path, tmpworkerid, snapshotid);
	if (is_existing) {
		INVARIANT(isexist(snapshotdbseq_path));
		uint64_t snapshotdbseq = 0;
		load_snapshotdbseq(snapshotdbseq, snapshotdbseq_path);

		sp_ptr = NULL;

		// recover snapshot database on the dbseq by checkpoint
		rocksdb::Checkpoint *checkpoint_ptr = NULL;
		rocksdb::Status s = rocksdb::Checkpoint::Create(db_ptr, &checkpoint_ptr);
		INVARIANT(s.ok());
		INVARIANT(checkpoint_ptr != NULL);
		std::string snapshotdb_path;
		get_server_snapshotdb_path(snapshotdb_path, tmpworkerid);
		rmfiles(snapshotdb_path.c_str());
		// log_size_for_flush = 0; sequence_number_ptr = &snapshotdbseq
		s = checkpoint_ptr->CreateCheckpoint(snapshotdb_path, 0, &snapshotdbseq); // snapshot database has been stored into disk by hardlink
		INVARIANT(s.ok());
		s = rocksdb::TransactionDB::Open(rocksdb_options, rocksdb::TransactionDBOptions(), snapshotdb_path, &snapshotdb_ptr);
		INVARIANT(s.ok());
		INVARIANT(snapshotdb_ptr != NULL);
	}
	else {
		INVARIANT(!isexist(snapshotdbseq_path));
		sp_ptr = NULL;
		snapshotdb_ptr = NULL;
	}

	// load deleted set snapshot
	std::string snapshotdeletedset_path;
	get_server_snapshotdeletedset_path(snapshotdeletedset_path, tmpworkerid, snapshotid);
	if (is_existing) {
		INVARIANT(isexist(snapshotdeletedset_path));
		snapshot_deleted_set.load(snapshotdeletedset_path);
	}
	else {
		INVARIANT(!isexist(snapshotdeletedset_path));
		snapshot_deleted_set.clear();
	}

	return is_existing;
}

// loading phase

bool RocksdbWrapper::force_multiput(netreach_key_t *keys, val_t *vals, int maxidx) {
	rocksdb::Status s;
	rocksdb::WriteOptions write_options;
	write_options.sync = SYNC_WRITE; // Write through for persistency
	rocksdb::Transaction* txn = db_ptr->BeginTransaction(write_options, rocksdb::TransactionOptions());

	for (int i = 0; i < maxidx; i++) {
		std::string keystr = keys[i].to_string_for_rocksdb();
		std::string valstr = vals[i].to_string_for_rocksdb(0);
		s = txn->Put(keystr, valstr);
	}
	s = txn->Commit();
	INVARIANT(s.ok());

	delete txn;
	txn = NULL;
	return true;
}

bool RocksdbWrapper::force_put(netreach_key_t key, val_t val) {
	rocksdb::Status s;
	std::string valstr = val.to_string_for_rocksdb(0);
	rocksdb::WriteOptions write_options;
	write_options.sync = SYNC_WRITE; // Write through for persistency
	rocksdb::Transaction* txn = db_ptr->BeginTransaction(write_options, rocksdb::TransactionOptions());

	std::string keystr = key.to_string_for_rocksdb();
	s = txn->Put(keystr, valstr);
	s = txn->Commit();
	INVARIANT(s.ok());

	delete txn;
	txn = NULL;
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
	rocksdb::Transaction* txn = db_ptr->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
	INVARIANT(txn != nullptr);

	//read_options.fill_cache = false; // Bypass OS-cache (page cache), use block cache only
	s = txn->Get(rocksdb::ReadOptions(), key.to_string_for_rocksdb(), &valstr);
	s = txn->Commit();
	delete txn;
	txn = NULL;

	bool stat = false;
	if (valstr != "") {
		seq = val.from_string_for_rocksdb(valstr);
		INVARIANT(s.ok());
		stat = true;
	}

	uint32_t deleted_seq = 0;
	bool is_deleted = deleted_set.check_and_remove(key, seq, &deleted_seq);
	if (is_deleted) {
		INVARIANT(deleted_seq > seq);
		seq = deleted_seq;
		stat = false;
	}

	//mutexlock.unlock();
	rwlock.unlock_shared();
	return stat;
}

bool RocksdbWrapper::put(netreach_key_t key, val_t val, uint32_t seq) {
	//mutexlock.lock();
	while (true) {
		if (rwlock.try_lock()) break;
	}

	uint32_t deleted_seq = 0;
	bool is_deleted = deleted_set.check_and_remove(key, seq, &deleted_seq);
	if (is_deleted) {
		INVARIANT(deleted_seq > seq);
		//mutexlock.unlock();
		rwlock.unlock();
		return false;
	}

	rocksdb::Status s;
	std::string valstr = val.to_string_for_rocksdb(seq);
	rocksdb::WriteOptions write_options;
	write_options.sync = SYNC_WRITE; // Write through for persistency
	rocksdb::Transaction* txn = db_ptr->BeginTransaction(write_options, rocksdb::TransactionOptions());

	std::string keystr = key.to_string_for_rocksdb();
	std::string tmp_valstr;
	s = txn->Get(rocksdb::ReadOptions(), keystr, &tmp_valstr);
	uint32_t tmp_seq = 0;
	if (tmp_valstr != "") {
		val_t tmp_val;
		tmp_seq = tmp_val.from_string_for_rocksdb(tmp_valstr);
		if (tmp_seq > seq) {
			s = txn->Commit();
			INVARIANT(s.ok());
			delete txn;
			txn = NULL;

			//mutexlock.unlock();
			rwlock.unlock();
			return true;
		}
	}

	s = txn->Put(keystr, valstr);
	s = txn->Commit();
	INVARIANT(s.ok());
	delete txn;
	txn = NULL;

	//mutexlock.unlock();
	rwlock.unlock();
	return true;
}

bool RocksdbWrapper::remove(netreach_key_t key, uint32_t seq) {
	//mutexlock.lock();
	while (true) {
		if (rwlock.try_lock()) break;
	}

	uint32_t deleted_seq = 0;
	bool is_deleted = deleted_set.check_and_remove(key, seq, &deleted_seq);
	if (is_deleted) {
		INVARIANT(deleted_seq > seq);
		//mutexlock.unlock();
		rwlock.unlock();
		return false;
	}

	rocksdb::Status s;
	rocksdb::WriteOptions write_options;
	write_options.sync = SYNC_WRITE; // Write through for persistency
	rocksdb::Transaction* txn = db_ptr->BeginTransaction(write_options, rocksdb::TransactionOptions());

	std::string keystr = key.to_string_for_rocksdb();
	std::string tmp_valstr;
	s = txn->Get(rocksdb::ReadOptions(), keystr, &tmp_valstr);
	uint32_t tmp_seq = 0;
	if (tmp_valstr != "") {
		val_t tmp_val;
		tmp_seq = tmp_val.from_string_for_rocksdb(tmp_valstr);
		if (tmp_seq > seq) {
			s = txn->Commit();
			INVARIANT(s.ok());
			delete txn;
			txn = NULL;

			//mutexlock.unlock();
			rwlock.unlock();
			return true;
		}
	}

	s = txn->Delete(keystr);
	s = txn->Commit();
	INVARIANT(s.ok());
	delete txn;
	txn = NULL;

	deleted_set.add(key, seq);

	//mutexlock.unlock();
	rwlock.unlock();
	return true;
}

size_t RocksdbWrapper::range_scan(netreach_key_t startkey, netreach_key_t endkey, std::vector<std::pair<netreach_key_t, snapshot_record_t>> &results) {
	//mutexlock.lock();
	while (true) {
		if (rwlock_for_snapshot.try_lock_shared()) break;
	}

	INVARIANT(sp_ptr != NULL || snapshotdb_ptr != NULL);

	rocksdb::Status s;
	rocksdb::Transaction* txn = NULL;
	rocksdb::Iterator* iter = NULL;
	rocksdb::ReadOptions read_options;
	if (sp_ptr != NULL) {
		txn = db_ptr->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
		read_options.snapshot = sp_ptr;
		iter = txn->GetIterator(read_options);
	}
	else if (snapshotdb_ptr != NULL) {
		txn = snapshotdb_ptr->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
		iter = txn->GetIterator(read_options);
	}
	INVARIANT(iter != NULL);
	results.clear();

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
	s = txn->Commit();
	INVARIANT(s.ok());
	delete txn;
	txn = NULL;

	std::vector<std::pair<netreach_key_t, snapshot_record_t>> deleted_results;
	snapshot_deleted_set.range_scan(startkey, endkey, deleted_results);

	// merge sort
	if (db_results.size() == 0) {
		results = deleted_results;
	}
	else if (deleted_results.size() == 0) {
		results = db_results;
	}
	else {
		uint32_t db_idx = 0;
		uint32_t deleted_idx = 0;
		bool remain_db = false;
		bool remain_deleted = false;
		while (true) {
			netreach_key_t &tmp_db_key = db_results[db_idx].first;
			snapshot_record_t &tmp_db_record = db_results[db_idx].second;
			netreach_key_t &tmp_deleted_key = deleted_results[deleted_idx].first;
			snapshot_record_t &tmp_deleted_record = deleted_results[deleted_idx].second;
			if (tmp_db_key < tmp_deleted_key) {
				results.push_back(db_results[db_idx]);
				db_idx += 1;
			}
			else if (tmp_db_key > tmp_deleted_key) {
				results.push_back(deleted_results[deleted_idx]);
				deleted_idx += 1;
			}
			else {
				printf("[RocksdbWrapper] deleted set and database cannot have the same key!\n");
				exit(-1);
			}
			if (db_idx >= db_results.size()) {
				remain_deleted = true;
				break;
			}
			if (deleted_idx >= deleted_results.size()) {
				remain_db = true;
				break;
			}
		}
		if (remain_db) {
			for (; db_idx < db_results.size(); db_idx++) {
				results.push_back(db_results[db_idx]);
			}
		}
		else if (remain_deleted) {
			for (; deleted_idx < deleted_results.size(); deleted_idx++) {
				results.push_back(deleted_results[deleted_idx]);
			}
		}
	}

	//mutexlock.unlock();
	rwlock_for_snapshot.unlock_shared();
	return results.size();
}

void RocksdbWrapper::make_snapshot() {
	if (!is_snapshot.test_and_set(std::memory_order_acquire)) {

		struct timespec create_t1, create_t2, create_t3, store_t1, store_t2, store_t3;
		CUR_TIME(create_t1);

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
			get_server_snapshotdb_path(snapshotdb_path, workerid);
			rmfiles(snapshotdb_path.c_str());
		}*/
		snapshot_deleted_set.clear();

		snapshotid += 1;

		// create new snapshot database and snapshot deleted set
		while (true) {
			if (rwlock.try_lock_shared()) break;
		}
		// make snapshot of database
		sp_ptr = db_ptr->GetSnapshot();
		uint64_t snapshotdbseq = sp_ptr->GetSequenceNumber();
		INVARIANT(sp_ptr != NULL);
		// make snapshot of deleted set
		snapshot_deleted_set = deleted_set;

		// NOTE: we can unlock rwlock after accessing db_ptr and deleted_set -> limited effect on put/delete
		rwlock.unlock_shared();

		// NOTE: we can unlock rwlock_for_snapshot after updating sp_ptr, snapshotdb_ptr, and snapshot_deleted_set -> limited effect on range_scan
		rwlock_for_snapshot.unlock();

		CUR_TIME(create_t2);
		CUR_TIME(store_t1);

		while (true) {
			if (rwlock_for_snapshot.try_lock_shared()) break;
		}

		// store snapshot database seq (sp_ptr->dbseq) into disk
		std::string snapshotdbseq_path;
		get_server_snapshotdbseq_path(snapshotdbseq_path, workerid, snapshotid);
		store_snapshotdbseq(snapshotdbseq, snapshotdbseq_path);

		// store snapshot deleted set into disk
		std::string snapshotdeletedset_path;
		get_server_snapshotdeletedset_path(snapshotdeletedset_path, workerid, snapshotid);
		snapshot_deleted_set.store(snapshotdeletedset_path);

		// store snapshotid at last
		std::string snapshotid_path;
		get_server_snapshotid_path(snapshotid_path, workerid);
		store_snapshotid(snapshotid, snapshotid_path);
		
		// remove old-enough snapshot data w/ snapshotid-2
		if (snapshotid >= 2) {
			int old_snapshotid = snapshotid - 2;

			std::string old_snapshotdbseq_path;
			get_server_snapshotdbseq_path(old_snapshotdbseq_path, workerid, old_snapshotid);
			rmfiles(old_snapshotdbseq_path.c_str());

			std::string old_snapshotdeletedset_path;
			get_server_snapshotdeletedset_path(old_snapshotdeletedset_path, workerid, old_snapshotid);
			rmfiles(old_snapshotdeletedset_path.c_str());
		}

		// NOTE: get shared lock of rwlock_for_snapshot does not affect range_scan
		rwlock_for_snapshot.unlock_shared();

		CUR_TIME(store_t2);
		DELTA_TIME(create_t2, create_t1, create_t3);
		DELTA_TIME(store_t2, store_t1, store_t3);
		//COUT_VAR(GET_MICROSECOND(create_t3));
		//COUT_VAR(GET_MICROSECOND(store_t3));
	}

	return;
}

void RocksdbWrapper::stop_snapshot() {
	is_snapshot.clear(std::memory_order_release);
	return;
}
