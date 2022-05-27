#include "rocksdb/db.h"
#include "rocksdb/cache.h"
#include "rocksdb/table.h"
#include "rocksdb/utilities/transaction.h"
#include "rocksdb/utilities/transaction_db.h"
#include "rocksdb/utilities/checkpoint.h"

#include "../helper.h"
#include "../io_helper.h"
#include "../iniparser/iniparser_wrapper.h"
#include "../rocksdb_wrapper.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usgae: ./server_recover latest_snapshotid\n");
		exit(-1);
	}

	int snapshotid = atoi(argv[1]);
	COUT_VAR(snapshotid);

	IniparserWrapper ini;
	ini.load("../config.ini");
	uint32_t server_num = ini.get_server_num();
	COUT_VAR(server_num);

	RocksdbWrapper::prepare_rocksdb();

	for (uint16_t i = 0; i < server_num; i++) {
		// overwrite snapshotid
		std::string snapshotid_path;
		get_server_snapshotid_path(snapshotid_path, i);
		store_snapshotid(snapshotid, snapshotid_path);

		// remove larger snapshot database and deletedset if any
		int larger_snapshotid = snapshotid + 1;
		std::string larger_snapshotdbseq_path;
		get_server_snapshotdbseq_path(larger_snapshotdbseq_path, i, larger_snapshotid);
		rmfiles(larger_snapshotdbseq_path.c_str());
		std::string larger_snapshotdeletedset_path;
		get_server_snapshotdeletedset_path(larger_snapshotdeletedset_path, i, larger_snapshotid);
		rmfiles(larger_snapshotdeletedset_path.c_str());

		// remove snapshotdb if any
		std::string snapshotdb_path;
		get_server_snapshotdb_path(snapshotdb_path, i);
		rmfiles(snapshotdb_path.c_str());

		// open runtime database
		std::string db_path;
		get_server_db_path(db_path, i);
		rocksdb::TransactionDB *db_ptr = NULL;
		rocksdb::Status s = rocksdb::TransactionDB::Open(RocksdbWrapper::rocksdb_options, rocksdb::TransactionDBOptions(), db_path, &db_ptr);
		INVARIANT(db_ptr != NULL);

		// create latest snapshot database as snapshotdb
		std::string latest_snapshotdbseq_path;
		get_server_snapshotdb_path(latest_snapshotdbseq_path, i);
		uint64_t latest_snapshotdbseq;
		load_snapshotdbseq(latest_snapshotdbseq, latest_snapshotdbseq_path);
		rocksdb::Checkpoint *checkpoint_ptr = NULL;
		s = rocksdb::Checkpoint::Create(db_ptr, &checkpoint_ptr);
		INVARIANT(s.ok());
		INVARIANT(checkpoint_ptr != NULL);
		// log_size_for_flush = 0; sequence_number_ptr = &snapshotdbseq
		s = checkpoint_ptr->CreateCheckpoint(snapshotdb_path, 0, &latest_snapshotdbseq); // snapshot database has been stored into disk by hardlink
		INVARIANT(s.ok());

		// remove runtime database and runtime deletedset
		delete db_ptr;
		db_ptr = NULL;
		rmfiles(db_path.c_str());
		std::string deletedset_path;
		get_server_deletedset_path(deletedset_path, i);
		rmfiles(deletedset_path.c_str());

		// copy snapshotdb as runtime database
		char cmd[256];
		sprintf(cmd, "cp -r %s %s", snapshotdb_path.c_str(), db_path.c_str());
		system(cmd);
		rmfiles(snapshotdb_path.c_str());

		// copy latest snapshot deletedset as runtime deletedset
		std::string latest_snapshotdeletedset_path;
		get_server_snapshotdeletedset_path(latest_snapshotdeletedset_path, i, snapshotid);
		sprintf(cmd, "cp -r %s %s", latest_snapshotdeletedset_path.c_str(), deletedset_path.c_str());
		system(cmd);
	}
}
