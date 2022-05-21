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
}

RocksdbWrapper::RocksdbWrapper() {
	db_ptr = NULL;
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

bool RocksdbWrapper::force_put(netreach_key_t key, val_t val) {
	rocksdb::Status s;
	std::string valstr = val.to_string_for_rocksdb(seq);
	rocksdb::WriteOptions write_options;
	write_options.sync = SYNC_WRITE; // Write through for persistency
	rocksdb::Transaction* txn = db_ptr->BeginTransaction(write_options, rocksdb::TransactionOptions());

	s = txn->Put(keystr, valstr);
	s = txn->Commit();
	INVARIANT(s.ok());

	delete txn;
	txn = NULL;
	return true;
}

bool RocksdbWrapper::get(netreach_key_t key, val_t &val, uint32_t &seq) {
	rocksdb::Status s;
	std::string valstr;
	rocksdb::Transaction* txn = db_ptr->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
	INVARIANT(txn != nullptr);

	//read_options.fill_cache = false; // Bypass OS-cache (page cache), use block cache only
	s = txn->Get(rocksdb::ReadOptions(), key.to_string_for_rocksdb(), &valstr);
	s = txn->Commit();

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

	delete txn;
	txn = NULL;
	return stat;
}

bool RocksdbWrapper::put(netreach_key_t key, val_t val, uint32_t seq) {
	uint32_t deleted_seq = 0;
	bool is_deleted = deleted_set.check_and_remove(key, seq, &deleted_seq);
	if (is_deleted) {
		INVARIANT(deleted_seq > seq);
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
	INVARIANT(s.ok());
	uint32_t tmp_seq = 0;
	if (tmp_valstr != "") {
		val_t tmp_val;
		tmp_seq = tmp_val.from_string_for_rocksdb(tmp_valstr);
		if (tmp_seq > seq) {
			s = txn->Commit();
			INVARIANT(s.ok());
			delete txn;
			txn = NULL;
			return true;
		}
	}

	s = txn->Put(keystr, valstr);
	s = txn->Commit();
	INVARIANT(s.ok());

	delete txn;
	txn = NULL;
	return true;
}

bool RocksdbWrapper::remove(netreach_key_t key, uint32_t seq) {
	uint32_t deleted_seq = 0;
	bool is_deleted = deleted_set.check_and_remove(key, seq, &deleted_seq);
	if (is_deleted) {
		INVARIANT(deleted_seq > seq);
		return true;
	}

	rocksdb::Status s;
	rocksdb::WriteOptions write_options;
	write_options.sync = SYNC_WRITE; // Write through for persistency
	rocksdb::Transaction* txn = db_ptr->BeginTransaction(write_options, rocksdb::TransactionOptions());

	std::string keystr = key.to_string_for_rocksdb();
	std::string tmp_valstr;
	s = txn->Get(rocksdb::ReadOptions(), keystr, &tmp_valstr);
	INVARIANT(s.ok());
	uint32_t tmp_seq = 0;
	if (tmp_valstr != "") {
		val_t tmp_val;
		tmp_seq = tmp_val.from_string_for_rocksdb(tmp_valstr);
		if (tmp_seq > seq) {
			s = txn->Commit();
			INVARIANT(s.ok());
			delete txn;
			txn = NULL;
			return true;
		}
	}

	s = txn->Delete(keystr);
	s = txn->Commit();
	INVARIANT(s.ok());

	deleted_set.add(key, seq);

	delete txn;
	txn = NULL;
	return true;
}

size_t RocksdbWrapper::range_scan(netreach_key_t startkey, netreach_key_t endkey, std::vector<std::pair<netreach_key_t, val_t>> &results) {
	rocksdb::Status s;
	rocksdb::Transaction* txn = db_ptr->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
	rocksdb::Iterator* iter = txn->GetIterator(rocksdb::ReadOptions());
	INVARIANT(iter!=nullptr);

	results.clear();
	std::string startkeystr = startkey.to_string_for_rocksdb();
	std::string endkeystr = endkey.to_string_for_rocksdb();
	iter->Seek(startkeystr);
	size_t num = 0;
	while (iter->Valid()) {
		std::string tmpkeystr = iter->key().ToString();
		netreach_key_t tmpkey;
		tmpkey.from_string_for_rocksdb(tmpkeystr);
		if (tmpkey > endkey) {
			break;
		}
		std::string tmpvalstr = iter->value().ToString();
		val_t tmpval;
		tmpval.from_string_for_rocksdb(tmpvalstr);

		results.push_back(std::pair<netreach_key_t, val_t>(tmpkey, tmpval));
		num += 1;
		iter->Next();
	}
	s = txn->Commit();
	delete txn;
	INVARIANT(s.ok());
	return num;
}
