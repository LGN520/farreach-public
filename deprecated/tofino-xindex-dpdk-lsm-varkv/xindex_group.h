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

#if !defined(XINDEX_GROUP_H)
#define XINDEX_GROUP_H

#include <boost/thread/shared_mutex.hpp>
#include "xindex_buffer.h"
#include "xindex_model.h"
#include "xindex_util.h"
#include "helper.h"

#include "rocksdb/db.h"
#include "rocksdb/utilities/transaction.h"
#include "rocksdb/utilities/transaction_db.h"

#define RWLOCKMAP_SIZE 1024

namespace xindex {

template <class key_t, class val_t, bool seq, size_t max_model_n = 4>
class alignas(CACHELINE_SIZE) Group {

  template <class key_tt, class val_tt, bool sequential>
  friend class XIndex;
  template <class key_tt, class val_tt, bool sequential>
  friend class Root;

 public:
  Group();
  ~Group();
  void init(const typename std::vector<key_t>::const_iterator &keys_begin,
            const typename std::vector<val_t>::const_iterator &vals_begin,
            uint32_t array_size, uint32_t group_idx, std::string workload_name);
  void open(uint32_t group_idx, std::string workload_name);
  /*void init(const typename std::vector<key_t>::const_iterator &keys_begin,
            const typename std::vector<val_t>::const_iterator &vals_begin,
            uint32_t model_n, uint32_t array_size, uint32_t group_idx);*/
  const key_t &get_pivot();

  inline result_t get(const key_t &key, val_t &val);
  inline result_t put(const key_t &key, const val_t &val);
  inline result_t data_put(const key_t &key, const val_t &val);
  inline result_t remove(const key_t &key);
  inline size_t scan(const key_t &begin, const size_t n,
                     std::vector<std::pair<key_t, val_t>> &result);
  inline bool range_scan(const key_t &begin, const key_t &end,
                           std::vector<std::pair<key_t, val_t>> &result);

  Group *compact_phase();
  //Group *compact_phase_1();
  //void compact_phase_2();

  //void free_data();
  void free_buffer();

 private:
  inline bool get_from_lsm(const key_t &key, val_t &val, rocksdb::TransactionDB *txn_db);
  inline result_t update_to_lsm(const key_t &key, const val_t &val, rocksdb::TransactionDB *txn_db);
  inline bool remove_from_lsm(const key_t &key, rocksdb::TransactionDB *txn_db);
  inline bool scan_from_lsm(const key_t &begin, const size_t n, const key_t &end, 
		  std::vector<std::pair<key_t, val_t>> &result, rocksdb::TransactionDB *txn_db);
  inline bool range_scan_from_lsm(const key_t &begin, const key_t &end, 
		  std::vector<std::pair<key_t, val_t>> &result, rocksdb::TransactionDB *txn_db,
		  rocksdb::Snapshot *txn_db_sp=nullptr);

  inline size_t scan_2_way(const key_t &begin, const size_t n, const key_t &end, 
		  std::vector<std::pair<key_t, val_t>> &result);
  inline size_t scan_3_way(const key_t &begin, const size_t n, const key_t &end, 
		  std::vector<std::pair<key_t, val_t>> &result);
  inline size_t merge_scan_2_way(const std::vector<std::pair<key_t, val_t>> &v0, 
		  const std::vector<std::pair<key_t, val_t>> &v1, const size_t n, std::vector<std::pair<key_t, val_t>> &result);
  inline size_t merge_scan_3_way(const std::vector<std::pair<key_t, val_t>> &v0, const std::vector<std::pair<key_t, val_t>> &v1, 
		  const std::vector<std::pair<key_t, val_t>> &v2, const size_t n, std::vector<std::pair<key_t, val_t>> &result);

  //inline size_t scan_2_way(const key_t &begin, const size_t n, const key_t &end,
  //                         std::vector<std::pair<key_t, val_t>> &result);
  //inline size_t scan_3_way(const key_t &begin, const size_t n, const key_t &end,
  //                         std::vector<std::pair<key_t, val_t>> &result);
  
  inline void init_cur_buffer_id();
  inline std::string get_buffer_path(uint32_t buffer_id);

  key_t pivot;
  // make array_size atomic because we don't want to acquire lock during `get`.
  // it is okay to obtain a stale (smaller) array_size during `get`.
  //uint32_t array_size;
  //uint16_t model_n = 0;
  Group *next = nullptr; // unused
  bool buf_frozen = false;
  bool after_compact = false;
  rocksdb::TransactionDB *data = nullptr;
  rocksdb::TransactionDB *buffer = nullptr;
  rocksdb::TransactionDB *buffer_temp = nullptr;

  uint32_t buffer_size = 0; // The size of puts in buffer instead of the number of elements in buffer
  uint32_t buffer_size_temp = 0;
  boost::shared_mutex rwlockmap[RWLOCKMAP_SIZE];
  uint32_t group_idx = 0;
  std::string workload_name;
  uint32_t cur_buffer_id = 0;
};

}  // namespace xindex

#endif  // XINDEX_GROUP_H
