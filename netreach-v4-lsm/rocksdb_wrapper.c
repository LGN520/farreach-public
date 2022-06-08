#include "rocksdb_wrapper.h"

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
	latest_sp_ptr = NULL;
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
		create_snapshotdb_checkpoint(snapshotdbseq); // set snapshotdb_ptr

		sp_ptr = NULL;
		latest_sp_ptr = NULL;
	}
	else {
		INVARIANT(!isexist(snapshotdbseq_path));
		snapshotdb_ptr = NULL;
		sp_ptr = NULL;
		latest_sp_ptr = NULL;
	}

	// load deleted set snapshot
	std::string snapshotdeletedset_path;
	get_server_snapshotdeletedset_path(snapshotdeletedset_path, tmpworkerid, snapshotid);
	if (is_existing) {
		INVARIANT(isexist(snapshotdeletedset_path));
		snapshot_deleted_set.load(snapshotdeletedset_path);
		latest_snapshot_deleted_set.clear();
	}
	else {
		INVARIANT(!isexist(snapshotdeletedset_path));
		snapshot_deleted_set.clear();
		latest_snapshot_deleted_set.clear();
	}

	// TODO: load inswitch snapshot

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
	s = txn->Commit();
	INVARIANT(s.ok());
	delete txn;
	txn = NULL;

	// get results from snapshot deleted set
	std::vector<std::pair<netreach_key_t, snapshot_record_t>> deleted_results;
	snapshot_deleted_set.range_scan(startkey, endkey, deleted_results);

	// merge sort
	results.clear();
	merge_sort(db_results, deleted_results, results, false);

	//mutexlock.unlock();
	rwlock_for_snapshot.unlock_shared();
	return results.size();
}

// merge sort between snapshot db and snapshot deleted set: need_exist = false
// merge sort between server-side snapshot and in-switch snapshot: need_exist = true
void RocksdbWrapper::merge_sort(const std::vector<std::pair<netreach_key_t, snapshot_record_t>> &veca, const std::vector<std::pair<netreach_key_t, snapshot_record_t>> &vecb, std::vecotr<std::pair<netreach_key_t, snapshot_record_t>> &result, bool need_exist) {
	if (veca.size() == 0) {
		results = vecb;
	}
	else if (vecb.size() == 0) {
		results = veca;
	}
	else {
		uint32_t veca_idx = 0;
		uint32_t vecb_idx = 0;
		bool remain_veca = false;
		bool remain_vecb = false;
		while (true) {
			netreach_key_t &tmp_veca_key = veca[veca_idx].first;
			snapshot_record_t &tmp_veca_record = veca[veca_idx].second;
			netreach_key_t &tmp_vecb_key = vecb[vecb_idx].first;
			snapshot_record_t &tmp_vecb_record = vecb[vecb_idx].second;
			if (tmp_veca_key < tmp_vecb_key) {
				results.push_back(veca[veca_idx]);
				veca_idx += 1;
			}
			else if (tmp_veca_key > tmp_vecb_key) {
				results.push_back(vecb[vecb_idx]);
				vecb_idx += 1;
			}
			else {
				//printf("[RocksdbWrapper] deleted set and database cannot have the same key!\n");
				//exit(-1);
				INVARIANT(need_exist == true);
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
				results.push_back(veca[veca_idx]);
			}
		}
		else if (remain_vecb) {
			for (; vecb_idx < vecb.size(); vecb_idx++) {
				results.push_back(vecb[vecb_idx]);
			}
		}
	}
}

// snapshot

// recover snapshot database on the dbseq by checkpoint (set snapshotdb_ptr)
// invoked by constructor; or invoked by update_snapshot
void RocksdbWrapper::create_snapshotdb_checkpoint(uint64_t snapshotdbseq) {
	INVARIANT(db_ptr != NULL);
	rocksdb::Checkpoint *checkpoint_ptr = NULL;
	rocksdb::Status s = rocksdb::Checkpoint::Create(db_ptr, &checkpoint_ptr);
	INVARIANT(s.ok());
	INVARIANT(checkpoint_ptr != NULL);
	std::string snapshotdb_path;
	get_server_snapshotdb_path(snapshotdb_path, workerid);
	rmfiles(snapshotdb_path.c_str());
	// log_size_for_flush = 0; sequence_number_ptr = &snapshotdbseq
	s = checkpoint_ptr->CreateCheckpoint(snapshotdb_path, 0, &snapshotdbseq); // snapshot database has been stored into disk by hardlink
	INVARIANT(s.ok());
	s = rocksdb::TransactionDB::Open(rocksdb_options, rocksdb::TransactionDBOptions(), snapshotdb_path, &snapshotdb_ptr);
	INVARIANT(s.ok());
	INVARIANT(snapshotdb_ptr != NULL);
}

// invoked by SNAPSHOT_CLEANUP
void RocksdbWrapper::clean_snapshot(int tmpsnapshotid) {
	INVARIANT(tmpsnapshotid == snapshotid || tmpsnapshotid == snapshotid + 1);

	// remove latest snapshot of tmpsnapshotid
	int latest_snapshotid = tmpsnapshotid;
	std::string latest_snapshotdbseq_path;
	get_server_snapshotdbseq_path(latest_snapshotdbseq_path, workerid, latest_snapshotid);
	rmfiles(latest_snapshotdbseq_path.c_str());
	std::string latest_snapshotdeletedset_path;
	get_server_snapshotdeletedset_path(latest_snapshotdeletedset_path, workerid, latest_snapshotid);
	rmfiles(latest_snapshotdeletedset_path.c_str());

	if (latest_sp_ptr != NULL) {
		db_ptr->ReleaseSnapshot(latest_sp_ptr);
		latest_sp_ptr = NULL;
	}
	latest_snapshot_deleted_set.clear();

	if (tmpsnapshotid == snapshotid) {
		while (true) {
			if (rwlock_for_snapshot.try_lock()) break;
		}

		// close snapshot database and clear snapshot deleted set
		if (sp_ptr != NULL) {
			db_ptr->ReleaseSnapshot(sp_ptr); // TODO: need protect db_ptr here?
			sp_ptr = NULL;
		}
		snapshot_deleted_set.clear();

		// load snapshot of snapshotid-1
		int previous_snapshotid = snapshotid - 1;
		std::string previous_snapshotdbseq_path;
		get_server_snapshotdbseq_path(previous_snapshotdbseq_path, workerid, previous_snapshotid);
		INVARIANT(isexist(previous_snapshotdbseq_path));
		uint64_t previous_snapshotdbseq = 0;
		load_snapshotdbseq(previous_snapshotdbseq, previous_snapshotdbseq_path);
		create_snapshotdb_checkpoint(previous_snapshotdbseq); // set snapshotdb_ptr

		std::string previous_snapshotdeletedset_path;
		get_server_snapshotdeletedset_path(previous_snapshotdeletedset_path, workerid, previous_snapshotid);
		INVARIANT(isexist(previous_snapshotdeletedset_path));
		snapshot_deleted_set.load(previous_snapshotdeletedset_path);

		snapshotid -= 1;

		// NOTE: we can unlock rwlock_for_snapshot after updating sp_ptr, snapshotdb_ptr, and snapshot_deleted_set -> limited effect on range_scan
		rwlock_for_snapshot.unlock();
	}
}

// create a new server-side snapshot yet not used by range query (as in-switch snapshot is stale)
// invoked by worker due to case3 (tmpsnapshotid == 0); invoked by SNAPSHOT_START (tmpsnapshotid == snapshotid/snapshotid+1)
void RocksdbWrapper::make_snapshot(int tmpsnapshotid) {
	INVARIANT(tmpsnapshotid >= 0);

	//// common case: +1; rare case due to controller failure: +2
	//INVARIANT(tmpsnapshotid == snapshotid + 1 || tmpsnapshotid == snapshotid + 2);
	
	// NOTE: if tmpsnapshotid == snapshotid, it has been solevd by SNAPSHOT_CLEANUP
	INVARIANT(tmpsnapshotid == 0 || tmpsnapshotid == snapshotid + 1)

	if (!is_snapshot.test_and_set(std::memory_order_acquire)) {

		//if (tmpsnapshotid == snapshotid + 2) {
		//	update_snapshot(); // snapshotid <- snapshotid + 1
		//}
		//INVARIANT(tmpsnapshotid == snapshotid + 1);

		struct timespec create_t1, create_t2, create_t3, store_t1, store_t2, store_t3;
		CUR_TIME(create_t1);

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

		CUR_TIME(create_t2);
		CUR_TIME(store_t1);

		//while (true) {
		//	if (rwlock_for_snapshot.try_lock_shared()) break;
		//}

		// store snapshot database seq (sp_ptr->dbseq) into disk
		std::string latest_snapshotdbseq_path;
		get_server_snapshotdbseq_path(latest_snapshotdbseq_path, workerid, snapshotid+1);
		store_snapshotdbseq(latest_snapshotdbseq, latest_snapshotdbseq_path);

		// store snapshot deleted set into disk
		std::string latest_snapshotdeletedset_path;
		get_server_snapshotdeletedset_path(latest_snapshotdeletedset_path, workerid, snapshotid+1);
		snapshot_deleted_set.store(latest_snapshotdeletedset_path);

		// NOTE: get shared lock of rwlock_for_snapshot does not affect range_scan
		//rwlock_for_snapshot.unlock_shared();

		CUR_TIME(store_t2);
		DELTA_TIME(create_t2, create_t1, create_t3);
		DELTA_TIME(store_t2, store_t1, store_t3);
		//COUT_VAR(GET_MICROSECOND(create_t3));
		//COUT_VAR(GET_MICROSECOND(store_t3));
	}

	return;
}

// use latest server-side snapshot of snapshotid+1 for range query after receiving latest in-switch snapshot
// invoked by SNAPSHOT_SENDDATA; (deprecated: or invoked by make_snapshot under rare case due to controller failure)
void RocksdbWrapper::update_snapshot() {
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

	// update snapshot database and snapshot deleted set
	if (likely(latest_sp_ptr != NULL)) {
		sp_ptr = latest_sp_ptr;
		latest_sp_ptr = NULL;
		snapshot_deleted_set = latest_snapshot_deleted_set;
		latest_snapshot_deleted_set.clear();
	}
	else { // both latest_sp_ptr and sp_ptr are NULL here
		// load snapshot of snapshotid+1
		std::string latest_snapshotdbseq_path;
		get_server_snapshotdbseq_path(latest_snapshotdbseq_path, workerid, snapshotid+1);
		INVARIANT(isexist(latest_snapshotdbseq_path));
		uint64_t latest_snapshotdbseq = 0;
		load_snapshotdbseq(latest_snapshotdbseq, latest_snapshotdbseq_path);
		create_snapshotdb_checkpoint(latest_snapshotdbseq); // set snapshotdb_ptr

		std::string latest_snapshotdeletedset_path;
		get_server_snapshotdeletedset_path(latest_snapshotdeletedset_path, workerid, snapshotid+1);
		INVARIANT(isexist(latest_snapshotdeletedset_path));
		snapshot_deleted_set.load(latest_snapshotdeletedset_path);
	}

	// update snapshotid
	snapshotid += 1;

	// NOTE: we can unlock rwlock_for_snapshot after updating sp_ptr, snapshotdb_ptr, and snapshot_deleted_set -> limited effect on range_scan
	rwlock_for_snapshot.unlock();

	// store snapshotid at last (snapshotid for range query)
	std::string snapshotid_path;
	get_server_snapshotid_path(snapshotid_path, workerid);
	store_snapshotid(snapshotid, snapshotid_path);
		
	// remove old-enough snapshot data w/ snapshotid-2 (holding snapshots of snapshotid and snapshotid-1)
	if (snapshotid >= 2) {
		int old_snapshotid = snapshotid - 2;

		std::string old_snapshotdbseq_path;
		get_server_snapshotdbseq_path(old_snapshotdbseq_path, workerid, old_snapshotid);
		rmfiles(old_snapshotdbseq_path.c_str());

		std::string old_snapshotdeletedset_path;
		get_server_snapshotdeletedset_path(old_snapshotdeletedset_path, workerid, old_snapshotid);
		rmfiles(old_snapshotdeletedset_path.c_str());
	}
}

// invoked by SNAPSHOT_SENDDATA
void RocksdbWrapper::stop_snapshot() {
	is_snapshot.clear(std::memory_order_release);
	return;
}
