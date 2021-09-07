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
- Add Get/SetColumnFamilyData and cfd_ in TableReader, then BlockBasedTable will inherit them (table/table_reader.h)
- Set cfd of table_reader by SetColumnFamilyData() at TableCache::GetTableReader after table_factory->NewTableReader (db/table_cache.cc)
- Pass cfd to TableCache::GetTableReader (called only when no corresponding hanlde in table_cache) -> TableCache::FindTable (db/table_cache.h, db/table_cache.cc)
	+ VersionBuilder::Rep::LoadTableHandlers (db/version_builder.h, db/version_builder.cc)
		* VersionSet::ProcessManifestWrites (db/version_set.cc)
		* VersionEditHandler::LoadTables (db/version_edit_handler.cc)
	+ TableCache::NewIterator (db/table_cache.cc, db/table_cache.h)
		* CompactionJob::Run (db/compaction/compaction_job.cc)
		* BuildTable (db/builder.cc, db/builder.h)
			- Repairer::ConvertLogToTable (db/repair.cc)
			- DBImpl::WriteLevel0TableForRecovery (db/db_impl/db_impl_open.cc)
			- FlushJob::WriteLevel0Table (db/flush_job.cc)
		* Repairer::ScanTable (db/repair.cc)
		* LevelIterator::NewFileIterator (db/version_set.cc)
			- LevelIterator::LevelIterator (db/version_set.cc)
			- Version::AddIteratVorsForLevel (db/version_set.cc)
			- Version::OverlapWithLevelIterator (db/version_set.cc)
			- VersionSet::MakeInputIterator (db/version_set.cc)
		* Version::AddIteratVorsForLevel (db/version_set.cc)
		* Version::OverlapWithLevelIterator (db/version_set.cc)
		* VersionSet::MakeInputIterator (db/version_set.cc)
		* ForwardLevelIterator::Reset (db/forward_iterator.cc)
		* ForwardIterator::RebuildIterators (db/forward_iterator.cc)
		* ForwardIterator::RenewIterators (db/forward_iterator.cc)
		* ForwardIterator::ResetIncompleteIterators (db/forward_iterator.cc)
	+ TableCache::GetRangeTombStoneIterator (db/table_cache.cc, db/table_cache.h)
		* Version::TablesRangeTombstoneSummary (db/version_set.cc)
	+ TableCache::Get/MultiGet (db/table_cache.cc, db/table_cache.h)
		* Version::Get/MultiGet (db/version_set.cc)
	+ TableCache::GetTableProperties (db/table_cache.cc, db/table_cache.h)
		* Repairer::ScanTable (db/repair.cc)
		* Version::GetTableProperties (db/version_set.cc)
	+ TableCache:: (db/table_cache.cc, db/table_cache.h)
		* Version::GetMemoryUsageByTableReaders (db/version_set.cc)
	+ TableCache::ApproximateOffsetOf (db/table_cache.cc, db/table_cache.h)
		* VersionSet::ApproximateOffsetOf (db/version_set.cc)
	+ TableCache::ApproximateSize (db/table_cache.cc, db/table_cache.h)
		+ VersionSet::ApproximateSize (db/version_set.cc)
- Add filenum_ in BlockBasedTable when calling BlockBasedTable::Open (table/block_based/block_based_table_reader.h, table/block_based/block_based_table_reader.cc)
- Pass cfd/level/filenumber into IndexBlockIter::Seek and DataBlockIter::Seek (= BlockIter::Seek) (table/block_based/block.h)
	+ BlockIter::Seek (table/block_based/block.h)
		* BlockBasedTable::PrefixMayMatch (table/block_based/block_based_table_reader.cc)
		* BlockBasedTable::Get (table/block_based/block_based_table_reader.cc)
		* BlockBasedTable::MultiGet (table/block_based/block_based_table_reader.cc)
		* BlockBasedTable::Prefetch (table/block_based/block_based_table_reader.cc)
		* BlockBasedTable::TEST_KeyInCache (table/block_based/block_based_table_reader.cc)
		* BlockBasedTable::ArpproximateOffsetOf (table/block_based/block_based_table_reader.cc)
		* BlockBasedTable::ArpproximateSize (table/block_based/block_based_table_reader.cc)
		* PartitionedIndexIterator::SeekImpl (table/block_based/partitioned_index_iterator.cc)
		* BlockBasedTableIterator::SeekImpl (table/block_based/block_based_table_iterator.cc)
		* BlockBasedTableIterator::SeekForPrev (table/block_based/block_based_table_iterator.cc)
	+ BlockIter::SeekImpl (table/block_based/block.h)
	+ IndexBlockIter::SeekImpl (table/block_based/block.h, table/block_based/block.c)
	+ DataBlockIter::SeekImpl (table/block_based/block.h, table/block_based/block.c)
	+ DataBlockIter::SeekForGet (table/block_based/block.h)
		* BlockBasedTable::Get (table/block_based/block_based_table_reader.cc)
		* BlockBasedTable::MultiGet (table/block_based/block_based_table_reader.cc)
	+ DataBlockIter::SeekForGetImpl (table/block_based/block.h, table/block_based/block.c)
- Lookup model in GET/SCAN
	+ For GET, change IndexBlockIter::SeekImpl (table/block_based/block.cc)
		+ Add BlockIter::ModelSeek referring to BlockIter::BinarySeek (table/block_based/block.h, table/block_based/block.cc)
- TODO: 
	+ For GET, change DataBlockIter::SeekImpl/SeekForGetImpl (table/block_based/block.cc)
- Add cfd in BlockBasedTable::Open
- TODO: Judge if (unlikely) table_reader exists in VersionStorageInfo::AddFile, otherwise use VersionStorageInfo::version->table_cache to get the table_reader (db/version_set.cc)

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
