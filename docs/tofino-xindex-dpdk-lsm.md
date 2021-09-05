# Tofino-based forwarding + DPDK-based XIndex with persistency (tofino-xindex-dpdk-lsm)

## Implementation log

- Install RocksDB 6.22.1
- Fix compilation bugs
- Fix bugs of opening database (not link snappy in librocksdb.a)
- Create directory of rocksdb-6.22.1-model
- Add Version* version_ in VersionStorageInfo (db/version_set.h, db/version_set.cc)
- Add level_models_ in each ColumnFamilyData (db/column_family.h)
	+ It is a vector with maps of io_options_.num_levels
	+ In each map, key is uint64_t (i.e., FileMetaData::FileDescriptor::file_number as file ID?) and value is LinearModelWrapper*
	+ Each LinearModelWrapper contains linear models of model_n for each sstable file
- Add level_locks_ in each ColumnFamilyData (db/column_family.h)
	+ Each lock is a r-w lock for each map in level_linear_models_
	+ NOTE: GET/SCAN are readers, OPEN/FLUSH/COMPACTION are writers
		- For Open, since no readers exist, it is ok to get the lock
		- For FLUSH/COMPACTION, since their frequency are relatively low and we only get lock after preparing everything including new linear models, it is also acceptable
- Create directory of rocksdb-6.22.1-model/model
- Change xindex_model.h to make it support keys of variable length (model/xindex_model.h, model/xindex_model_impl.h)
- For Slice, implement to_model_key to convert Slice into double array (include/rocksdb/slice.h)
- Change to_string to directly conert the byte array of key as Slice (client.c, server.c)
- Implement LinearModelWrapper (model/linear_model_wrapper.h, model/linear_model_wrapper.c)
- For VersionBuilder::Rep::SaveTo(vstorage) -> VersionBuilder::Rep::MaybeAddFile(vstorage, level, f) (db/version_builder.cc)
	+ Train linear models in VersionStorageInfo::AddFile(level, f) (db/version_set.cc)
		+ New and prepare LinearModelWrapper for the new file
		+ Try lock
		+ If lock successfully, add the pair of file number and LinearModelWrapper into level_models
		+ Unlock
	+ Drop linear models in VersionStorageInfo::RemoveCurrentStats(file_meta) (db/version_builder.cc)
		+ NOTE: Add a parameter of level in RemoveCurrentStats(file_meta, level) (db/version_set.cc, db/version_set.h, db/version_builder.cc)
		+ Try lock
		+ If lock successfully, free the LinearModelWrapper, and remove the pair of file number and LinearModelWrapper from level_models
		+ Unlock
- TODO: 
- Judge if (unlikely) table_reader exists in VersionStorageInfo::AddFile, otherwise use VersionStorageInfo::version->table_cache to get the table_reader
- Add set_cfd() in TableReader, then BlockBasedTable will inherit this function
- Add cfd into table_reader by set_cfd() at TableCache::GetTableReader after table_factory->NewTableReader
- Add cfd into BlockIter for IndexBlockIter and DataBlockIter
- Lookup model in GET/SCAN
	+ For GET, change IndexBlockIter::Seek (table/block_based/block.cc)
		+ Add BlockIter::ModelSeek referring to BlockIter::BinarySeek (table/block_based/block.cc)
	+ For GET, change DataBlockIter::Seek (table/block_based/block.cc)

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
