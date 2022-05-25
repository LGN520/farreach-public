#include <unistd.h>
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
}

RocksdbWrapper::~RocksdbWrapper() {
	if (db_ptr != NULL) {
		delete db_ptr;
		db_ptr = NULL;
	}
}

/*RocksdbWrapper::RocksdbWrapper(uint16_t workerid) {
	open(workerid);
}*/

bool RocksdbWrapper::open(uint16_t workerid) {
	bool is_existing = false;

	std::string db_path;
	GET_STRING(db_path, "/tmp/netreach-v4-lsm/worker" << workerid << ".db");
	if (access(db_path.c_str(), F_OK) == 0) {
		is_existing = true;
	}

	rocksdb::Status s = rocksdb::TransactionDB::Open(rocksdb_options, rocksdb::TransactionDBOptions(), db_path, &db_ptr);
	INVARIANT(s.ok());
	INVARIANT(db_ptr != NULL);

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
	mutexlock.lock();

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

	mutexlock.unlock();
	return stat;
}

bool RocksdbWrapper::put(netreach_key_t key, val_t val, uint32_t seq) {
	mutexlock.lock();

	uint32_t deleted_seq = 0;
	bool is_deleted = deleted_set.check_and_remove(key, seq, &deleted_seq);
	if (is_deleted) {
		INVARIANT(deleted_seq > seq);
		mutexlock.unlock();
		return true;
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

			mutexlock.unlock();
			return true;
		}
	}

	s = txn->Put(keystr, valstr);
	s = txn->Commit();
	INVARIANT(s.ok());
	delete txn;
	txn = NULL;

	mutexlock.unlock();
	return true;
}

bool RocksdbWrapper::remove(netreach_key_t key, uint32_t seq) {
	mutexlock.lock();

	uint32_t deleted_seq = 0;
	bool is_deleted = deleted_set.check_and_remove(key, seq, &deleted_seq);
	if (is_deleted) {
		INVARIANT(deleted_seq > seq);
		mutexlock.unlock();
		return true;
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

			mutexlock.unlock();
			return true;
		}
	}

	s = txn->Delete(keystr);
	s = txn->Commit();
	INVARIANT(s.ok());
	delete txn;
	txn = NULL;

	deleted_set.add(key, seq);

	mutexlock.unlock();
	return true;
}

size_t RocksdbWrapper::range_scan(netreach_key_t startkey, netreach_key_t endkey, std::vector<std::pair<netreach_key_t, snapshot_record_t>> &results) {
	mutexlock.lock();

	rocksdb::Status s;
	rocksdb::Transaction* txn = db_ptr->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
	// TODO: use snapshot
	rocksdb::Iterator* iter = txn->GetIterator(rocksdb::ReadOptions());
	INVARIANT(iter!=nullptr);
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
	deleted_set.range_scan(startkey, endkey, deleted_results);

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

	mutexlock.unlock();
	return results.size();
}

void RocksdbWrapper::make_snapshot() {
	printf("TODO: implement make_snapshot()\n");
	return;
}

void RocksdbWrapper::stop_snapshot() {
	printf("TODO: implement stop_snapshot()\n");
	return;
}
