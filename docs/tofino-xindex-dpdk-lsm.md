# Tofino-based forwarding + DPDK-based XIndex with persistency (tofino-xindex-dpdk-lsm)

## Implementation log

- Install RocksDB 6.22.1
- Fix compilation bugs
- Fix bugs of opening database (not link snappy in librocksdb.a)
- Create directory of rocksdb-6.22.1-model
- Add Version* in VersionStorageInfo (db/version_set.h, db/version_set.cc)
- Add level_models_ in each ColumnFamilyData (db/column_family.h)
	+ It is a vector with maps of io_options_.num_levels
	+ In each map, key is uint64_t (i.e., FileMetaData::FileDescriptor::file_number as file ID?) and value is LinearModelWrapper*
	+ Each LinearModelWrapper contains linear models of model_n for each sstable file
- Add level_locks_ in each ColumnFamilyData (db/column_family.h)
	+ Each lock is a r-w lock for each map in level_linear_models_
	+ NOTE: GET/SCAN are readers, OPEN/FLUSH/COMPACTION are writers
		- For Open, since no readers exist, it is ok to get the lock
		- For FLUSH/COMPACTION, since their frequency are relatively low and we only get lock after preparing everything including new linear models, it is also acceptable
- TODO:
- Implement LinearModelWrapper
- Train linear models in VersionBuilder::Rep::MaybeAddFile(vstorage, level, f) (db/version_builder.cc)
	+ New and prepare LinearModelWrapper for the new file
	+ Try lock
	+ If lock successfully, add the pair of file number and LinearModelWrapper into level_models
	+ Unlock
- Drop linear models in VersionBuilder::Rep::MaybeAddFile(vstorage, level, f) (db/version_builder.cc)
	+ Try lock
	+ If lock successfully, remove the pair of file number and LinearModelWrapper into level_models, and free the LinearModelWrapper
	+ Unlock

## How to run

- Prepare randomly-generated keys
	+ `make all`
	+ `./prepare`
- Run `cd tofino`
	+ Run `su` to enter root account
	+ Run `bash compile.sh` to compile p4 into binary code
	+ Run `bash start_switch.sh` to launch Tofino
	+ Create a new terminal and run `bash configure.sh` to configure data plane
- Run `bash start_server.sh` in server host
- Run `bash start_client.sh` in client host

## Fixed issues
