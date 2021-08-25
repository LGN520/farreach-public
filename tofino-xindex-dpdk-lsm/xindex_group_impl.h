/*
 * The code is part of the XIndex project.
 *
 *    Copyright (C) 2020 Institute of Parallel and Distributed Systems (IPADS),
 * Shanghai Jiao Tong University. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more about XIndex, visit:
 *     https://ppopp20.sigplan.org/details/PPoPP-2020-papers/13/XIndex-A-Scalable-Learned-Index-for-Multicore-Data-Storage
 */

#include "xindex_group.h"

#if !defined(XINDEX_GROUP_IMPL_H)
#define XINDEX_GROUP_IMPL_H

namespace xindex {

template <class key_t, class val_t, bool seq, size_t max_model_n>
Group<key_t, val_t, seq, max_model_n>::Group() {}

template <class key_t, class val_t, bool seq, size_t max_model_n>
Group<key_t, val_t, seq, max_model_n>::~Group() {}

template <class key_t, class val_t, bool seq, size_t max_model_n>
void Group<key_t, val_t, seq, max_model_n>::init(
    const typename std::vector<key_t>::const_iterator &keys_begin,
    const typename std::vector<val_t>::const_iterator &vals_begin,
    uint32_t array_size, uint32_t group_idx) {
  init(keys_begin, vals_begin, 1, array_size, group_idx);
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
void Group<key_t, val_t, seq, max_model_n>::init(
    const typename std::vector<key_t>::const_iterator &keys_begin,
    const typename std::vector<val_t>::const_iterator &vals_begin,
    uint32_t model_n, uint32_t array_size, uint32_t group_idx) {
  assert(array_size > 0);
  this->pivot = *keys_begin;
  this->array_size = array_size;
  //this->capacity = array_size * seq_insert_reserve_factor;
  this->model_n = model_n; // # of models per sstable

  // Create original data
  std::string data_path;
  GET_STRING(data_path, "/tmp/netbuffer/group"<<group_idx<<".db");
  rocksdb::Status s = rocksdb::TransactionDB::Open(data_options, rocksdb::TransactionDBOptions(), data_path, &data);
  assert(s.ok());
  
  // Create delta index
  std::string buffer_path;
  GET_STRING(buffer_path, "/tmp/netbuffer/buffer"<<group_idx<<".db");
  s = rocksdb::TransactionDB::Open(buffer_options, rocksdb::TransactionDBOptions(), buffer_path, &buffer);
  assert(s.ok());

  // Write original data
  rocksdb::WriteBatch batch;
  for (size_t rec_i = 0; rec_i < array_size; rec_i++) {
	std::string valstr;
	GET_STRING(valstr, *(vals_begin + rec_i));
	batch.Put((*(keys_begin + rec_i)).to_string(), valstr);
  }
  s = data->Write(rocksdb::WriteOptions(), &batch);
  assert(s.ok());

  // RocksDB will train model_n linear models for each new sstable 
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
const key_t &Group<key_t, val_t, seq, max_model_n>::get_pivot() {
  return pivot;
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline result_t Group<key_t, val_t, seq, max_model_n>::get(const key_t &key,
                                                           val_t &val) {
  if (get_from_lsm(key, val, data)) {
    return result_t::ok;
  }
  if (get_from_lsm(key, val, buffer)) {
    return result_t::ok;
  }
  if (buffer_temp && get_from_lsm(key, val, buffer_temp)) {
    return result_t::ok;
  }
  return result_t::failed;
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline result_t Group<key_t, val_t, seq, max_model_n>::put(
    const key_t &key, const val_t &val) {
  result_t res;

  val_t old_val;
  if (get_from_lsm(key, old_val, data)) {
	res = update_to_lsm(key, val, data);
	assert(res == result_t::ok);
	return res;
  }

  if (likely(buffer_temp == nullptr)) {
    if (buf_frozen) {
      return result_t::retry;
    }
    res = update_to_lsm(key, val, buffer);
	assert(res == result_t::ok);
	buffer_size++;
    return res;
  } else {
	if (get_from_lsm(key, old_val, buffer)) {
		res = update_to_lsm(key, val, buffer);
		assert(res == result_t::ok);
		return res;
	}
    update_to_lsm(key, val, buffer_temp);
    return result_t::ok;
  }
  COUT_N_EXIT("put should not fail!");
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline result_t Group<key_t, val_t, seq, max_model_n>::remove(
    const key_t &key) {
  if (remove_from_lsm(key, data)) {
    return result_t::ok;
  }
  if (remove_from_lsm(key, buffer)) {
    return result_t::ok;
  }
  if (buffer_temp && remove_from_lsm(key, buffer_temp)) {
    return result_t::ok;
  }
  return result_t::failed;
}

/*template <class key_t, class val_t, bool seq, size_t max_model_n>
inline size_t Group<key_t, val_t, seq, max_model_n>::scan(
    const key_t &begin, const size_t n,
    std::vector<std::pair<key_t, val_t>> &result) {
  return buffer_temp ? scan_3_way(begin, n, key_t::max(), result)
                     : scan_2_way(begin, n, key_t::max(), result);
}*/

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
  if (seq) {  // disable seq seq
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
  if (seq) {  // mark capacity as negative to let seq insert not insert to buf
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

template <class key_t, class val_t, bool seq, size_t max_model_n>
void Group<key_t, val_t, seq, max_model_n>::free_data() {
  delete data;
}
template <class key_t, class val_t, bool seq, size_t max_model_n>
void Group<key_t, val_t, seq, max_model_n>::free_buffer() {
  delete buffer;
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
	s = txn->Get(rocksdb::ReadOptions(), key.to_string(), &valstr);
	s = txn->Commit();
	val = std::stoi(valstr);
	delete txn;
	return s.ok();
}

template <class key_t, class val_t, bool seq, size_t max_model_n>
inline result_t Group<key_t, val_t, seq, max_model_n>::update_to_lsm(
    const key_t &key, const val_t &val, rocksdb::TransactionDB *txn_db) {
	std::string valstr;
	GET_STRING(valstr, val);
	rocksdb::Status s;
	rocksdb::Transaction* txn = txn_db->BeginTransaction(rocksdb::WriteOptions(), rocksdb::TransactionOptions());
	s = txn->Put(key.to_string(), valstr);
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
	s = txn->Delete(key.to_string());
	s = txn->Commit();
	delete txn;
	return s.ok();
}

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

    assert(base_key != buf_key);  // since update are inplaced

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

    assert(base_key != buf_key);      // since update are inplaced
    assert(base_key != tmp_buf_key);  // and removed values are skipped

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

    assert(base_key != buf_key);  // since update are inplaced

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

    assert(buf_key != tmp_buf_key);  // and removed values are skipped

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

    assert(base_key != tmp_buf_key);  // and removed values are skipped

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

}  // namespace xindex

#endif  // XINDEX_GROUP_IMPL_H
