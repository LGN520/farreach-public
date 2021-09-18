/*
 * The code is part of the XIndex project.
 *
 *		Copyright (C) 2020 Institute of Parallel and Distributed Systems (IPADS),
 * Shanghai Jiao Tong University. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *		 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more about XIndex, visit:
 *		 https://ppopp20.sigplan.org/details/PPoPP-2020-papers/13/XIndex-A-Scalable-Learned-Index-for-Multicore-Data-Storage
 */

#include "xindex_group.h"

#include <fstream>
#include <experimental/filesystem>

#if !defined(XINDEX_GROUP_IMPL_H)
#define XINDEX_GROUP_IMPL_H

namespace xindex {

template <class key_t, class val_t, bool seq, size_t max_model_n>
	Group<key_t, val_t, seq, max_model_n>::Group() {}

template <class key_t, class val_t, bool seq, size_t max_model_n>
Group<key_t, val_t, seq, max_model_n>::~Group() {
	if (!after_compact) {
		if (data != nullptr) delete data;
		if (buffer != nullptr) delete buffer;
		if (buffer_temp != nullptr) delete buffer_temp;
	}
}

/*template <class key_t, class val_t, bool seq, size_t max_model_n>
void Group<key_t, val_t, seq, max_model_n>::init(
		const typename std::vector<key_t>::const_iterator &keys_begin,
		const typename std::vector<val_t>::const_iterator &vals_begin,
		uint32_t array_size, uint32_t group_idx) {
	init(keys_begin, vals_begin, 1, array_size, group_idx);
}*/

template <class key_t, class val_t, bool seq, size_t max_model_n>
void Group<key_t, val_t, seq, max_model_n>::init(
		const typename std::vector<key_t>::const_iterator &keys_begin,
		const typename std::vector<val_t>::const_iterator &vals_begin,
		uint32_t array_size, uint32_t group_idx) {
		//uint32_t model_n, uint32_t array_size, uint32_t group_idx) {
	assert(array_size > 0);
	this->pivot = *keys_begin;
	//this->array_size = array_size;
	//this->capacity = array_size * seq_insert_reserve_factor;
	//this->model_n = model_n; // # of models per sstable
	this->group_idx = group_idx;

	// Create original data
	std::string data_path;
	GET_STRING(data_path, "/tmp/netbuffer/group"<<group_idx<<".db");
	rocksdb::Status s = rocksdb::TransactionDB::Open(data_options, rocksdb::TransactionDBOptions(), data_path, &data);
	assert(s.ok());
	
	// Create delta index
	init_cur_buffer_id();
	std::string buffer_path = get_buffer_path(cur_buffer_id);
	s = rocksdb::TransactionDB::Open(buffer_options, rocksdb::TransactionDBOptions(), buffer_path, &buffer);
	assert(s.ok());

	// Write original data (execute at the first time)
	/*rocksdb::WriteBatch batch;
	for (size_t rec_i = 0; rec_i < array_size; rec_i++) {
		std::string valstr;
		GET_STRING(valstr, *(vals_begin + rec_i));
		batch.Put((*(keys_begin + rec_i)).to_slice(), valstr);
	}
	s = data->Write(rocksdb::WriteOptions(), &batch);
	assert(s.ok());*/

	// RocksDB will train model_n linear models for each new sstable 
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
const key_t &Group<key_t, val_t, seq, max_model_n>::get_pivot() {
	return pivot;
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline result_t Group<key_t, val_t, seq, max_model_n>::get(const key_t &key, val_t &val) {
	result_t res = result_t::failed;
	if (get_from_lsm(key, val, data)) {
		res = result_t::ok;
	}
	if (get_from_lsm(key, val, buffer)) {
		res = result_t::ok;
	}
	if (buffer_temp && get_from_lsm(key, val, buffer_temp)) {
		res = result_t::ok;
	}
	return res;
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline result_t Group<key_t, val_t, seq, max_model_n>::put(
		const key_t &key, const val_t &val) {
	boost::shared_mutex *rwlock = nullptr;
	if (buf_frozen) {
		uint64_t lock_idx = key.to_int() % RWLOCKMAP_SIZE;	
		rwlock = &rwlockmap[lock_idx];
		while (true) {
			if (rwlock->try_lock_shared()) break;
		}
	}
	result_t res = result_t::failed;

	val_t old_val;
	if (get_from_lsm(key, old_val, data)) {
		res = update_to_lsm(key, val, data);
		assert(res == result_t::ok);
	}

	if (likely(buffer_temp == nullptr)) {
		if (buf_frozen) {
			res = result_t::retry;
		}
		res = update_to_lsm(key, val, buffer);
		assert(res == result_t::ok);
		buffer_size++;
	} else {
		if (get_from_lsm(key, old_val, buffer)) {
			res = update_to_lsm(key, val, buffer);
			assert(res == result_t::ok);
		}
		else {
			update_to_lsm(key, val, buffer_temp);
			res = result_t::ok;
		}
	}
	if (rwlock != nullptr) {
		rwlock->unlock_shared();
	}
	return res;
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline result_t Group<key_t, val_t, seq, max_model_n>::remove(
		const key_t &key) {
	boost::shared_mutex *rwlock = nullptr;
	if (buf_frozen) {
		uint64_t lock_idx = key.to_int() % RWLOCKMAP_SIZE;	
		rwlock = &rwlockmap[lock_idx];
		while (true) {
			if (rwlock->try_lock_shared()) break;
		}
	}
	val_t tmpval;
	result_t res = result_t::failed;
	if (get_from_lsm(key, tmpval, data)) {
		remove_from_lsm(key, data);
		res = result_t::ok;
	}
	if (get_from_lsm(key, tmpval, buffer)) {
		remove_from_lsm(key, buffer);
		res = result_t::ok;
	}
	if (buffer_temp && get_from_lsm(key, tmpval, buffer_temp)) {
		remove_from_lsm(key, buffer_temp);
		res = result_t::ok;
	}
	if (rwlock != nullptr) {
		rwlock->unlock_shared();
	}
	return res;
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline size_t Group<key_t, val_t, seq, max_model_n>::scan(
		const key_t &begin, const size_t n,
		std::vector<std::pair<key_t, val_t>> &result) {
	return buffer_temp ? scan_3_way(begin, n, key_t::max(), result)
										 : scan_2_way(begin, n, key_t::max(), result);
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline size_t Group<key_t, val_t, seq, max_model_n>::scan_2_way(
		const key_t &begin, const size_t n, const key_t &end, std::vector<std::pair<key_t, val_t>> &result) {
	std::vector<std::pair<key_t, val_t>> data_result;
	std::vector<std::pair<key_t, val_t>> buffer_result;
	bool res;
	res = scan_from_lsm(begin, n, end, data_result, data);
	INVARIANT(res);
	res = scan_from_lsm(begin, n, end, buffer_result, buffer);
	INVARIANT(res);
	return merge_scan_2_way(data_result, buffer_result, n, result);
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline size_t Group<key_t, val_t, seq, max_model_n>::scan_3_way(
		const key_t &begin, const size_t n, const key_t &end, std::vector<std::pair<key_t, val_t>> &result) {
	std::vector<std::pair<key_t, val_t>> data_result;
	std::vector<std::pair<key_t, val_t>> buffer_result;
	std::vector<std::pair<key_t, val_t>> buffer_temp_result;
	bool res;
	res = scan_from_lsm(begin, n, end, data_result, data);
	INVARIANT(res);
	res = scan_from_lsm(begin, n, end, buffer_result, buffer);
	INVARIANT(res);
	res = scan_from_lsm(begin, n, end, buffer_temp_result, buffer_temp);
	INVARIANT(res);
	return merge_scan_3_way(data_result, buffer_result, buffer_temp_result, n, result);
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline size_t Group<key_t, val_t, seq, max_model_n>::merge_scan_2_way(
		const std::vector<std::pair<key_t, val_t>> &v0, const std::vector<std::pair<key_t, val_t>> &v1, 
	const size_t n, std::vector<std::pair<key_t, val_t>> &result) {
	uint32_t v0_idx = 0;
	uint32_t v1_idx = 0;
	uint32_t v0_size = v0.size();
	uint32_t v1_size = v1.size();
	uint32_t cnt = 0;
	while (true) {
		if ((v0_idx == v0_size) || (v1_idx == v1_size) || (cnt == n)) {
			break;
		}

		if (v0[v0_idx].first < v1[v1_idx].first) {
			result.push_back(v0[v0_idx]);
			v0_idx++;
		}
		else if (v0[v0_idx].first > v1[v1_idx].first) {
			result.push_back(v1[v1_idx]);
			v1_idx++;
		}
		else {
			result.push_back(v0[v0_idx]);
			v0_idx++;
			v1_idx++;
		}
		cnt++;
	}

	if (cnt == n) return cnt;

	if (v0_idx < v0_size) {
		for (; v0_idx < v0_size; v0_idx++) {
			result.push_back(v0[v0_idx]);
			cnt++;
			if (cnt == n) break;
		}
	}
	else if (v1_idx < v1_size) {
		for (; v1_idx < v1_size; v1_idx++) {
			result.push_back(v1[v1_idx]);
			cnt++;
			if (cnt == n) break;
		}
	}

	return cnt;
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline size_t Group<key_t, val_t, seq, max_model_n>::merge_scan_3_way(
		const std::vector<std::pair<key_t, val_t>> &v0, const std::vector<std::pair<key_t, val_t>> &v1, 
	const std::vector<std::pair<key_t, val_t>> &v2, const size_t n, std::vector<std::pair<key_t, val_t>> &result) {
	std::vector<std::pair<key_t, val_t>> tmp;
	merge_scan_2_way(v0, v1, n, tmp);
	return merge_scan_2_way(tmp, v2, n, result);
}

/*template <class key_t, class val_t, bool seq, size_t max_model_n>
void Group<key_t, val_t, seq, max_model_n>::free_data() {
	delete data;
}*/

template <class key_t, class val_t, bool seq, size_t max_model_n>
void Group<key_t, val_t, seq, max_model_n>::free_buffer() {
	INVARIANT(after_compact == true);
	delete buffer; // Close the TransactionDB of old buffer
	std::string buffer_path = get_buffer_path(cur_buffer_id);
	std::uintmax_t n = std::experimental::filesystem::remove_all(buffer_path);
	COUT_THIS("Remove " << n << " files from " << buffer_path);
}

// semantics: atomically read the value
// only when the key exists and the record (record_t) is not logical removed,
// return true on success
template <class key_t, class val_t, bool seq, size_t max_model_n>
inline bool Group<key_t, val_t, seq, max_model_n>::get_from_lsm(
		const key_t &key, val_t &val, rocksdb::TransactionDB *txn_db) {
	std::string valstr;
	rocksdb::Status s;
	rocksdb::Transaction* txn = txn_db->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
	INVARIANT(txn != nullptr);
	rocksdb::ReadOptions read_options;
	//read_options.fill_cache = false; // Bypass OS-cache (page cache), use block cache only
	s = txn->Get(read_options, key.to_slice(), &valstr);
	s = txn->Commit();
	delete txn;
	if (valstr != "") {
		val = std::stoi(valstr);
		return s.ok();
	}
	else {
		return false;
	}
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline result_t Group<key_t, val_t, seq, max_model_n>::update_to_lsm(
		const key_t &key, const val_t &val, rocksdb::TransactionDB *txn_db) {
	/*if (unlikely(buf_frozen && txn_db == buffer)) {
		COUT_N_EXIT("Try to update buffer when buf is frozen!");
	}*/
	std::string valstr;
	GET_STRING(valstr, val);
	rocksdb::Status s;
	rocksdb::WriteOptions write_options;
	write_options.sync = true; // Write through for persistency
	rocksdb::Transaction* txn = txn_db->BeginTransaction(write_options, rocksdb::TransactionOptions());
	s = txn->Put(key.to_slice(), valstr);
	s = txn->Commit();
	delete txn;
	if (s.ok()) {
		return result_t::ok;
	}
	else {
		return result_t::failed;
	}
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline bool Group<key_t, val_t, seq, max_model_n>::remove_from_lsm(
		const key_t &key, rocksdb::TransactionDB *txn_db) {
	rocksdb::Status s;
	rocksdb::Transaction* txn = txn_db->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
	s = txn->Delete(key.to_slice());
	s = txn->Commit();
	delete txn;
	return s.ok();
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline bool Group<key_t, val_t, seq, max_model_n>::scan_from_lsm(
		const key_t &begin, const size_t n, const key_t &end,
		std::vector<std::pair<key_t, val_t>> &result,
	rocksdb::TransactionDB *txn_db) {
	rocksdb::Status s;
	rocksdb::Transaction* txn = txn_db->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
	rocksdb::Iterator* iter = txn->GetIterator(rocksdb::ReadOptions());
	INVARIANT(iter!=nullptr);
	result.clear();
	result.reserve(n);
	for (uint32_t i = 0; i < n; i++) {
		if (i == 0) {
			iter->Seek(begin.to_slice());
			if (!iter->Valid()) break;
		}
		else {
			iter->Next();
			if (!iter->Valid()) break;
		}
		rocksdb::Slice tmpkey_slice = iter->key();
		key_t tmpkey;
		tmpkey.from_slice(tmpkey_slice);
		//if (tmpkey.key >= end.key) break;
		rocksdb::Slice tmpvalue_slice = iter->value();
		val_t tmpval = std::stoi(tmpvalue_slice.ToString());
		result.push_back(std::pair<key_t, val_t>(tmpkey, tmpval));
	}
	s = txn->Commit();
	delete txn;
	return s.ok();
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline void Group<key_t, val_t, seq, max_model_n>::init_cur_buffer_id() {
	std::string buffer_path = get_buffer_path(1);
	if (access(buffer_path.c_str(), F_OK) != -1) {
		cur_buffer_id = 1;
	}
	else {
		cur_buffer_id = 0;
	}
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline std::string Group<key_t, val_t, seq, max_model_n>::get_buffer_path(uint32_t buffer_id) {
	INVARIANT(buffer_id == 0 || buffer_id == 1);
	std::string buffer_path;
	GET_STRING(buffer_path, "/tmp/netbuffer/buffer"<<group_idx<<"-"<<buffer_id<<".db");
	return buffer_path;
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
Group<key_t, val_t, seq, max_model_n>
		*Group<key_t, val_t, seq, max_model_n>::compact_phase() {
	buf_frozen = true;
	memory_fence();
	rcu_barrier();

	std::string buffer_temp_path;
	if (cur_buffer_id == 0) {
		buffer_temp_path = get_buffer_path(1);
	}
	else {
		buffer_temp_path = get_buffer_path(0);
	}
	if (access(buffer_temp_path.c_str(), F_OK) != -1) {
		COUT_N_EXIT("ERROR: buffer temp " << buffer_temp_path << " exists before compaction!");
	}
	// Set buffer_temp to process PUTs
	rocksdb::Status s;
	s = rocksdb::TransactionDB::Open(buffer_options, rocksdb::TransactionDBOptions(), buffer_temp_path, &buffer_temp);
	assert(s.ok());

	// Enumerate all keys from buffer (NOTE: buffer does not have PUTs now)
	rocksdb::Transaction* txn = buffer->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
	rocksdb::Iterator* iter = txn->GetIterator(rocksdb::ReadOptions());
	INVARIANT(iter!=nullptr);
	iter->Seek(key_t::min().to_slice());
	while (iter->Valid()) {
		// NOTE: iter->key and iter-value point to the entry of data block
		rocksdb::Slice tmpkey_slice = iter->key();
		key_t tmpkey;
		tmpkey.from_slice(tmpkey_slice);
		
		uint32_t lock_idx = tmpkey.to_int() % RWLOCKMAP_SIZE;
		boost::shared_mutex &rwlock = rwlockmap[lock_idx];
		while (true) {
			if (rwlock.try_lock()) break; // Get exclusive lock such that PUT/DEL cannot proceed
		}

		rocksdb::Slice tmpvalue_slice = iter->value();
		val_t tmpval = std::stoi(tmpvalue_slice.ToString());
		update_to_lsm(tmpkey, tmpval, data); // Put key-value pair from buffer to data

		rwlock.unlock(); // PUT/DELs for the key will hit in data
		iter->Next();
	}
	s = txn->Commit();
	assert(s.ok());
	delete txn;

	// now merge sort into a new array and train models
	Group *new_group = new Group();
	new_group->pivot = pivot;
	new_group->data = data;
	new_group->buffer = buffer_temp;
	new_group->group_idx = group_idx;
	new_group->cur_buffer_id = cur_buffer_id == 0 ? 1 : 0;

	this->after_compact = true; // It means refcnt of data and buffer_temp are 2

	return new_group;
}

/*template <class key_t, class val_t, bool seq, size_t max_model_n>
inline size_t Group<key_t, val_t, seq, max_model_n>::range_scan(
		const key_t &begin, const key_t &end,
		std::vector<std::pair<key_t, val_t>> &result) {
	size_t old_size = result.size();
	if (buffer_temp) {
		scan_3_way(begin, std::numeric_limits<size_t>::max(), end, result);
	} else {
		scan_2_way(begin, std::numeric_limits<size_t>::max(), end, result);
	}
	return result.size() - old_size;
}*/

/*template <class key_t, class val_t, bool seq, size_t max_model_n>
Group<key_t, val_t, seq, max_model_n>
		*Group<key_t, val_t, seq, max_model_n>::compact_phase_1() {
	if (seq) {	// disable seq seq
		disable_seq_insert_opt();
	}

	buf_frozen = true;
	memory_fence();
	rcu_barrier();
	buffer_temp = new buffer_t();

	// now merge sort into a new array and train models
	Group *new_group = new Group();

	new_group->pivot = pivot;
	merge_refs(new_group->data, new_group->array_size, new_group->capacity);
	if (seq) {	// mark capacity as negative to let seq insert not insert to buf
		new_group->disable_seq_insert_opt();
	}
	new_group->init_models(model_n);
	new_group->buffer = buffer_temp;
	new_group->next = next;

	return new_group;
}*/

/*template <class key_t, class val_t, bool seq, size_t max_model_n>
inline void Group<key_t, val_t, seq, max_model_n>::compact_phase_2() {
	for (size_t rec_i = 0; rec_i < array_size; ++rec_i) {
		data[rec_i].second.replace_pointer();
	}

	if (seq) {
		// enable seq seq, because now all threads only see new node
		enable_seq_insert_opt();
	}
}*/

/*template <class key_t, class val_t, bool seq, size_t max_model_n>
inline size_t Group<key_t, val_t, seq, max_model_n>::scan_2_way(
		const key_t &begin, const size_t n, const key_t &end,
		std::vector<std::pair<key_t, val_t>> &result) {
	size_t remaining = n;
	bool out_of_range = false;
	uint32_t base_i = get_pos_from_array(begin);
	ArrayDataSource array_source(data, array_size, base_i);
	typename buffer_t::DataSource buffer_source(begin, buffer);

	// first read a not-removed value from array and buffer, to avoid double read
	// during merge
	array_source.advance_to_next_valid();
	buffer_source.advance_to_next_valid();

	while (array_source.has_next && buffer_source.has_next && remaining &&
				 !out_of_range) {
		// we are sure that these key has not-removed (pre-read) value
		const key_t &base_key = array_source.get_key();
		const val_t &base_val = array_source.get_val();
		const key_t &buf_key = buffer_source.get_key();
		const val_t &buf_val = buffer_source.get_val();

		assert(base_key != buf_key);	// since update are inplaced

		if (base_key < buf_key) {
			if (base_key >= end) {
				out_of_range = true;
				break;
			}
			result.push_back(std::pair<key_t, val_t>(base_key, base_val));
			array_source.advance_to_next_valid();
		} else {
			if (buf_key >= end) {
				out_of_range = true;
				break;
			}
			result.push_back(std::pair<key_t, val_t>(buf_key, buf_val));
			buffer_source.advance_to_next_valid();
		}

		remaining--;
	}

	while (array_source.has_next && remaining && !out_of_range) {
		const key_t &base_key = array_source.get_key();
		const val_t &base_val = array_source.get_val();
		if (base_key >= end) {
			out_of_range = true;
			break;
		}
		result.push_back(std::pair<key_t, val_t>(base_key, base_val));
		array_source.advance_to_next_valid();
		remaining--;
	}

	while (buffer_source.has_next && remaining && !out_of_range) {
		const key_t &buf_key = buffer_source.get_key();
		const val_t &buf_val = buffer_source.get_val();
		if (buf_key >= end) {
			out_of_range = true;
			break;
		}
		result.push_back(std::pair<key_t, val_t>(buf_key, buf_val));
		buffer_source.advance_to_next_valid();
		remaining--;
	}

	return n - remaining;
}*/

/*template <class key_t, class val_t, bool seq, size_t max_model_n>
inline size_t Group<key_t, val_t, seq, max_model_n>::scan_3_way(
		const key_t &begin, const size_t n, const key_t &end,
		std::vector<std::pair<key_t, val_t>> &result) {
	size_t remaining = n;
	bool out_of_range = false;
	uint32_t base_i = get_pos_from_array(begin);
	ArrayDataSource array_source(data, array_size, base_i);
	typename buffer_t::DataSource buffer_source(begin, buffer);
	typename buffer_t::DataSource temp_buffer_source(begin, buffer_temp);

	// first read a not-removed value from array and buffer, to avoid double read
	// during merge
	array_source.advance_to_next_valid();
	buffer_source.advance_to_next_valid();
	temp_buffer_source.advance_to_next_valid();

	// 3-way
	while (array_source.has_next && buffer_source.has_next &&
				 temp_buffer_source.has_next && remaining && !out_of_range) {
		// we are sure that these key has not-removed (pre-read) value
		const key_t &base_key = array_source.get_key();
		const val_t &base_val = array_source.get_val();
		const key_t &buf_key = buffer_source.get_key();
		const val_t &buf_val = buffer_source.get_val();
		const key_t &tmp_buf_key = temp_buffer_source.get_key();
		const val_t &tmp_buf_val = temp_buffer_source.get_val();

		assert(base_key != buf_key);			// since update are inplaced
		assert(base_key != tmp_buf_key);	// and removed values are skipped

		if (base_key < buf_key && base_key < tmp_buf_key) {
			if (base_key >= end) {
				out_of_range = true;
				break;
			}
			result.push_back(std::pair<key_t, val_t>(base_key, base_val));
			array_source.advance_to_next_valid();
		} else if (buf_key < base_key && buf_key < tmp_buf_key) {
			if (buf_key >= end) {
				out_of_range = true;
				break;
			}
			result.push_back(std::pair<key_t, val_t>(buf_key, buf_val));
			buffer_source.advance_to_next_valid();
		} else {
			if (tmp_buf_key >= end) {
				out_of_range = true;
				break;
			}
			result.push_back(std::pair<key_t, val_t>(tmp_buf_key, tmp_buf_val));
			temp_buffer_source.advance_to_next_valid();
		}

		remaining--;
	}

	// 2-way trailings
	while (array_source.has_next && buffer_source.has_next && remaining &&
				 !out_of_range) {
		// we are sure that these key has not-removed (pre-read) value
		const key_t &base_key = array_source.get_key();
		const val_t &base_val = array_source.get_val();
		const key_t &buf_key = buffer_source.get_key();
		const val_t &buf_val = buffer_source.get_val();

		assert(base_key != buf_key);	// since update are inplaced

		if (base_key < buf_key) {
			if (base_key >= end) {
				out_of_range = true;
				break;
			}
			result.push_back(std::pair<key_t, val_t>(base_key, base_val));
			array_source.advance_to_next_valid();
		} else {
			if (buf_key >= end) {
				out_of_range = true;
				break;
			}
			result.push_back(std::pair<key_t, val_t>(buf_key, buf_val));
			buffer_source.advance_to_next_valid();
		}

		remaining--;
	}

	while (buffer_source.has_next && temp_buffer_source.has_next && remaining &&
				 !out_of_range) {
		// we are sure that these key has not-removed (pre-read) value
		const key_t &buf_key = buffer_source.get_key();
		const val_t &buf_val = buffer_source.get_val();
		const key_t &tmp_buf_key = temp_buffer_source.get_key();
		const val_t &tmp_buf_val = temp_buffer_source.get_val();

		assert(buf_key != tmp_buf_key);	// and removed values are skipped

		if (buf_key < tmp_buf_key) {
			if (buf_key >= end) {
				out_of_range = true;
				break;
			}
			result.push_back(std::pair<key_t, val_t>(buf_key, buf_val));
			buffer_source.advance_to_next_valid();
		} else {
			if (tmp_buf_key >= end) {
				out_of_range = true;
				break;
			}
			result.push_back(std::pair<key_t, val_t>(tmp_buf_key, tmp_buf_val));
			temp_buffer_source.advance_to_next_valid();
		}

		remaining--;
	}

	while (array_source.has_next && temp_buffer_source.has_next && remaining &&
				 !out_of_range) {
		// we are sure that these key has not-removed (pre-read) value
		const key_t &base_key = array_source.get_key();
		const val_t &base_val = array_source.get_val();
		const key_t &tmp_buf_key = temp_buffer_source.get_key();
		const val_t &tmp_buf_val = temp_buffer_source.get_val();

		assert(base_key != tmp_buf_key);	// and removed values are skipped

		if (base_key < tmp_buf_key) {
			if (base_key >= end) {
				out_of_range = true;
				break;
			}
			result.push_back(std::pair<key_t, val_t>(base_key, base_val));
			array_source.advance_to_next_valid();
		} else {
			if (tmp_buf_key >= end) {
				out_of_range = true;
				break;
			}
			result.push_back(std::pair<key_t, val_t>(tmp_buf_key, tmp_buf_val));
			temp_buffer_source.advance_to_next_valid();
		}

		remaining--;
	}

	// 1-way trailings
	while (array_source.has_next && remaining && !out_of_range) {
		const key_t &base_key = array_source.get_key();
		const val_t &base_val = array_source.get_val();
		if (base_key >= end) {
			out_of_range = true;
			break;
		}
		result.push_back(std::pair<key_t, val_t>(base_key, base_val));
		array_source.advance_to_next_valid();
		remaining--;
	}

	while (buffer_source.has_next && remaining && !out_of_range) {
		const key_t &buf_key = buffer_source.get_key();
		const val_t &buf_val = buffer_source.get_val();
		if (buf_key >= end) {
			out_of_range = true;
			break;
		}
		result.push_back(std::pair<key_t, val_t>(buf_key, buf_val));
		buffer_source.advance_to_next_valid();
		remaining--;
	}

	while (temp_buffer_source.has_next && remaining && !out_of_range) {
		const key_t &tmp_buf_key = temp_buffer_source.get_key();
		const val_t &tmp_buf_val = temp_buffer_source.get_val();
		if (tmp_buf_key >= end) {
			out_of_range = true;
			break;
		}
		result.push_back(std::pair<key_t, val_t>(tmp_buf_key, tmp_buf_val));
		temp_buffer_source.advance_to_next_valid();
		remaining--;
	}

	return n - remaining;
}*/

}	// namespace xindex

#endif	// XINDEX_GROUP_IMPL_H
