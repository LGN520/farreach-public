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

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <mutex>
#include "../helper.h"

//#include "rocksdb/db.h"
//#include "rocksdb/cache.h"
//#include "rocksdb/table.h"
//#include "rocksdb/utilities/transaction.h"
//#include "rocksdb/utilities/transaction_db.h"

#if !defined(XINDEX_UTIL_H)
#define XINDEX_UTIL_H

#define SYNC_WRITE false

namespace xindex {

static const size_t desired_training_key_n = 10000000;
static const size_t max_model_n = 4;
static const size_t seq_insert_reserve_factor = 2;

struct alignas(CACHELINE_SIZE) RCUStatus;
enum class Result;
struct alignas(CACHELINE_SIZE) BGInfo;
struct IndexConfig;

typedef RCUStatus rcu_status_t;
typedef Result result_t;
typedef BGInfo bg_info_t;
typedef IndexConfig index_config_t;

struct RCUStatus {
  std::atomic<int64_t> status;
  std::atomic<bool> waiting;
};
enum class Result { ok, failed, retry };
struct BGInfo {
  size_t bg_i;  // for calculation responsible range
  size_t bg_n;  // for calculation responsible range
  volatile void *root_ptr;
  volatile bool should_update_array;
  std::atomic<bool> started;
  std::atomic<bool> finished;
  volatile bool running;
};
struct IndexConfig {
  double root_error_bound = 32;
  double root_memory_constraint = 1024 * 1024;
  double group_error_bound = 32;
  double group_error_tolerance = 4;
  size_t buffer_size_bound = 256;
  double buffer_size_tolerance = 3;
  size_t buffer_compact_threshold = 256 * 1024; // 256K * (16+128)B kvpair
  size_t worker_n = 0;
  std::unique_ptr<rcu_status_t[]> rcu_status;
  volatile bool exited = false;

  // RocksDB
  /*size_t memtable_size = 4 * 1024 * 1024; // 4 MB
  size_t max_memtable_num = 2;
  size_t sst_size = 4 * 1024 * 1024; // 4 MB
  size_t compaction_thread_num = 2;
  size_t level0_num = 8; // 32 MB
  size_t level_num = 4;
  size_t level1_size = 32 * 1024 * 1024; // 32 MB
  size_t level_multiplier = 8; // level n+1 = 8 * level n*/
};

index_config_t config;
std::mutex config_mutex;

/*rocksdb::Options data_options;
rocksdb::Options buffer_options;
void inline init_options() {
  std::shared_ptr<rocksdb::Cache> cache = rocksdb::NewLRUCache(4 * 1024 * 1024, 4); // 4 MB with 16 shards
  rocksdb::BlockBasedTableOptions table_options;
  table_options.block_cache = cache;

  // options for data
  data_options.create_if_missing = true; // create database if not exist
  data_options.enable_blob_files = true; // enable key-value separation
  //data_options.allow_os_buffer = false; // Disable OS-cache (Not provided in current version of rocksdb now)
  data_options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options)); // Block cache with uncompressed blocks
  data_options.compaction_style = rocksdb::kCompactionStyleLevel; // leveled compaction
  data_options.write_buffer_size = config.memtable_size; // single memtable size
  data_options.max_write_buffer_number = config.max_memtable_num; // memtable number
  data_options.target_file_size_base = config.sst_size; // single sst size
  data_options.max_background_compactions = config.compaction_thread_num; // compaction thread number
  data_options.level0_file_num_compaction_trigger = config.level0_num; // sst number in level0
  data_options.num_levels = config.level_num; // level number
  data_options.max_bytes_for_level_base = config.level1_size; // byte number in level1
  data_options.max_bytes_for_level_multiplier = config.level_multiplier;

  // options for buffer
  buffer_options.create_if_missing = true; // create database if not exist
  buffer_options.enable_blob_files = false; // disable key-value separation
  //buffer_options.allow_os_buffer = false;
  buffer_options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options)); // Block cache with uncompressed blocks
  buffer_options.compaction_style = rocksdb::kCompactionStyleLevel; // leveled compaction
  buffer_options.write_buffer_size = config.memtable_size;
  buffer_options.max_write_buffer_number = config.max_memtable_num;
  buffer_options.target_file_size_base = config.sst_size;
  buffer_options.max_background_compactions = config.compaction_thread_num;
  buffer_options.level0_file_num_compaction_trigger = config.level0_num;
  buffer_options.num_levels = config.level_num;
  buffer_options.max_bytes_for_level_base = config.level1_size;
  buffer_options.max_bytes_for_level_multiplier = config.level_multiplier;
}*/

// TODO replace it with user space RCU (e.g., qsbr)
void rcu_init() {
  config_mutex.lock();
  if (config.rcu_status.get() == nullptr) {
    config.rcu_status = std::make_unique<rcu_status_t[]>(config.worker_n);
    for (size_t worker_i = 0; worker_i < config.worker_n; worker_i++) {
      config.rcu_status[worker_i].status = 0;
      config.rcu_status[worker_i].waiting = false;
    }
  }
  config_mutex.unlock();
}

void rcu_progress(const uint32_t worker_id) {
  config.rcu_status[worker_id].status++;
}

// wait for all workers
void rcu_barrier() {
  int64_t prev_status[config.worker_n];
  for (size_t w_i = 0; w_i < config.worker_n; w_i++) {
    prev_status[w_i] = config.rcu_status[w_i].status;
  }
  for (size_t w_i = 0; w_i < config.worker_n; w_i++) {
    while (config.rcu_status[w_i].status <= prev_status[w_i] && !config.exited)
      ;
  }
}

// wait for workers whose 'waiting' is false
void rcu_barrier(const uint32_t worker_id) {
  // set myself to waiting for barrier
  config.rcu_status[worker_id].waiting = true;

  int64_t prev_status[config.worker_n];
  for (size_t w_i = 0; w_i < config.worker_n; w_i++) {
    prev_status[w_i] = config.rcu_status[w_i].status;
  }
  for (size_t w_i = 0; w_i < config.worker_n; w_i++) {
    // skipped workers that is wating for barrier (include myself)
    while (config.rcu_status[w_i].status <= prev_status[w_i] &&
           !config.rcu_status[w_i].waiting && !config.exited)
      ;
  }
  config.rcu_status[worker_id].waiting = false;  // restore my state
}

template <class val_t>
struct AtomicVal {
  typedef val_t value_type;
  uint64_t *val_data = new uint64_t[val_t::MAX_VAL_LENGTH]; // trade space for high performance by optimistic locking
  uint8_t val_length = 0;
  AtomicVal *aval_ptr = nullptr; // Only aval_ptr makes sense for pointer-type atomic value

  // 60 bits for version
  static const uint64_t version_mask = 0x0fffffffffffffff;
  static const uint64_t lock_mask = 0x1000000000000000;
  static const uint64_t removed_mask = 0x2000000000000000;
  static const uint64_t pointer_mask = 0x4000000000000000;

  // lock - removed - is_ptr
  volatile uint64_t status;

  // For snapshot (only valid for value-type AtomicVal)
  // Maintain two versions for snapshot to support unfinished range query when making new snapshot
  int32_t create_id = -1; // -1: impossible value; 0: new entry created at epoch 0; 1: created at epoch 1
  uint8_t ss_val_length_0 = 0;
  uint64_t *ss_val_data_0 = new uint64_t[val_t::MAX_VAL_LENGTH];
  volatile uint64_t ss_status_0 = 0; // only focus on removed and version w/o lock and is_ptr
  int32_t ss_id_0 = -1; // -1: no snapshot; otherwise: snapshot of epoch k 
  uint8_t ss_val_length_1 = 0;
  uint64_t *ss_val_data_1 = new uint64_t[val_t::MAX_VAL_LENGTH];
  int32_t ss_id_1 = -1; // -1: no snapshot; otherwise: snapshot of epoch k'
  volatile uint64_t ss_status_1 = 0; // only focus on removed and version w/o lock and is_ptr

  AtomicVal() : status(0) {}
  ~AtomicVal() {
	  delete [] val_data;
	  delete [] ss_val_data_0;
	  delete [] ss_val_data_1;
  }
  AtomicVal(val_t val, int32_t snapshot_id) : status(0) {
	  val_length = val.val_length;
	  if (val_length > 0) { // NOTE: val.val_data may not long enough
		  memcpy((char *)val_data, (char *)val.val_data, val_length * sizeof(uint64_t));
	  }
	  // Empty snapshot
	  ss_id_0 = -1;
	  ss_id_1 = -1;
	  create_id = snapshot_id; // created at current epoch (help to decide if we need to snapshot the new entry)
  }
  AtomicVal(AtomicVal *ptr) : aval_ptr(ptr), status(0) { set_is_ptr(); }
  AtomicVal(const AtomicVal& other) {
	  *this = other;
  }
  AtomicVal & operator=(const AtomicVal& other) {
	  create_id = other.create_id;
	  val_length = other.val_length;
	  memcpy((char *)val_data, (char *)other.val_data, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
	  ss_val_length_0 = other.ss_val_length_0;
	  memcpy((char *)ss_val_data_0, (char *)other.ss_val_data_0, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
	  ss_status_0 = other.ss_status_0;
	  ss_id_0 = other.ss_id_0;
	  ss_val_length_1 = other.ss_val_length_1;
	  memcpy((char *)ss_val_data_1, (char *)other.ss_val_data_1, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
	  ss_status_1 = other.ss_status_1;
	  ss_id_1 = other.ss_id_1;
	  aval_ptr = other.aval_ptr;
	  status = other.status;
	  return *this;
  }

  bool is_ptr(uint64_t status) { return status & pointer_mask; }
  bool removed(uint64_t status) { return status & removed_mask; }
  bool locked(uint64_t status) { return status & lock_mask; }
  uint64_t get_version(uint64_t status) { return status & version_mask; }

  void set_is_ptr() { status |= pointer_mask; }
  void unset_is_ptr() { status &= ~pointer_mask; }
  void set_removed() { status |= removed_mask; }
  void lock() {
    while (true) {
      uint64_t old = status;
      uint64_t expected = old & ~lock_mask;  // expect to be unlocked
      uint64_t desired = old | lock_mask;    // desire to lock
      if (likely(cmpxchg((uint64_t *)&this->status, expected, desired) ==
                 expected)) {
        return;
      }
    }
  }
  void unlock() { status &= ~lock_mask; }
  void incr_version() {
    uint64_t version = get_version(status);
    UNUSED(version);
    status++;
    assert(get_version(status) == version + 1);
  }

  friend std::ostream &operator<<(std::ostream &os, const AtomicVal &leaf) {
	COUT_VAR(int(leaf.val_length));
	for (size_t i = 0; i < leaf.val_length; i++) {
		COUT_VAR(leaf.val_data[i]);
	}
	COUT_VAR(leaf.aval_ptr);
    COUT_VAR(leaf.is_ptr);
    COUT_VAR(leaf.removed);
    COUT_VAR(leaf.locked);
    COUT_VAR(leaf.verion);
    return os;
  }

  // Snapshot based on multiple versions
  // Compared with common multi-versioning which makes the snapshot for a record when the write in new epoch arrives,
  // we make the snapshot in a batch for each record when epoch switches before the write arrives to reduce write latency
  void make_snapshot(int32_t snapshot_id) {
	  lock();
	  memory_fence();
      if (unlikely(is_ptr(status))) {
          assert(!removed(status));
		  assert(aval_ptr != nullptr);
		  aval_ptr->make_snapshot(snapshot_id);
	  }
	  else if (ss_id_0 != snapshot_id && ss_id_1 != snapshot_id && create_id < snapshot_id) {
		  // (1) empty snapshot; (2) older and out-of-date snapshot
		  if ((ss_id_0 == -1) || (ss_id_0 != -1 && ss_id_1 != -1 && ss_id_0 < ss_id_1 && ss_id_0 < snapshot_id)) {
			  ss_id_0 = snapshot_id;
			  ss_val_length_0 = val_length;
			  memcpy((char *)ss_val_data_0, (char *)val_data, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
			  if (removed(status)) {
				  ss_status_0 |= removed_mask;
			  }
			  ss_status_0++; // increase version 
		  }
		  else if ((ss_id_1 == -1) || (ss_id_0 != -1 && ss_id_1 != -1 && ss_id_0 >= ss_id_1 && ss_id_1 < snapshot_id)) {
			  ss_id_1 = snapshot_id;
			  ss_val_length_1 = val_length;
			  memcpy((char *)ss_val_data_1, (char *)val_data, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
			  if (removed(status)) {
				  ss_status_1 |= removed_mask;
			  }
			  ss_status_1++; // increase version 
		  }
	  }
	  memory_fence();
	  unlock();
  }
  bool read_snapshot(val_t &val, int32_t snapshot_id) {
    while (true) {
	  uint64_t prev_status = this->status;
      uint64_t prev_ss_status_0 = this->ss_status_0; // focus on version and remove
      uint64_t prev_ss_status_1 = this->ss_status_1; // focus on version and remove
      memory_fence();
	  int32_t tmp_createid = this->create_id;
	  val_t tmpssval_0(this->ss_val_data_0, this->ss_val_length_0);
	  int32_t tmpssid_0 = this->ss_id_0;
	  val_t tmpssval_1(this->ss_val_data_1, this->ss_val_length_1);
	  int32_t tmpssid_1 = this->ss_id_1;
	  AtomicVal *tmpptr = this->aval_ptr; // AtomicVal pointed by tmpptr will not be freed before finishing this read_snapshot() due to RCU during compact
	  val_t tmpval(this->val_data, this->val_length);
      memory_fence();

	  uint64_t current_status = this->status; // focus on lock, is_ptr, and consistency of val
      memory_fence();
      uint64_t current_ss_status_0 = this->ss_status_0; // ensure consistent ss_val
      memory_fence();
      uint64_t current_ss_status_1 = this->ss_status_1; // ensure consistent ss_val
      memory_fence();

      if (unlikely(locked(current_status))) {  // check lock
        continue;
      }

      if (likely(get_version(prev_ss_status_0) == get_version(current_ss_status_0) && 
				  get_version(prev_ss_status_1 == get_version(current_ss_status_1)))) {  // check version
        if (unlikely(is_ptr(current_status))) { // check is_ptr
          assert(!removed(current_status)); // check removed of pointer-type atomic value
		  assert(tmpptr != nullptr);
          return tmpptr->read_snapshot(val, snapshot_id);
        } else {
			if (tmpssid_0 == snapshot_id) {
			  val = tmpssval_0; 
			  return !removed(current_ss_status_0); // check removed of snapshot value
			}
			else if (tmpssid_1 == snapshot_id) {
			  val = tmpssval_1; 
			  return !removed(current_ss_status_1); // check removed of snapshot value
			}
			// (1) current atomic value has not been snapshotted and without PUT/DEL; (2) new entry without snapshot but created at the previous epoch
			else if (tmp_createid < snapshot_id) {
				if (likely(get_version(prev_status) == get_version(current_status))) { // ensure the consistency of val
					// We leave snapshot of current atomic value in other threads (make_snapshot/PUT/DEL)
					val = tmpval;
					return !removed(current_status);
				}
			}
			// new entry without snapshot and created at the current epoch
			else {
				return false;
			}
        }
      }
    }
  }
  bool read_snapshot_0(val_t &val, int32_t &ss_id) {
    while (true) {
	  uint64_t prev_status = this->status;
      uint64_t prev_ss_status_0 = this->ss_status_0; // focus on version and remove
      memory_fence();
	  val_t tmpssval_0(this->ss_val_data_0, this->ss_val_length_0);
	  int32_t tmpssid_0 = this->ss_id_0;
	  AtomicVal *tmpptr = this->aval_ptr; // AtomicVal pointed by tmpptr will not be freed before finishing this read_snapshot() due to RCU during compact
      memory_fence();

	  uint64_t current_status = this->status; // focus on lock, is_ptr, and consistency of val
      memory_fence();
      uint64_t current_ss_status_0 = this->ss_status_0; // ensure consistent ss_val
      memory_fence();

      if (unlikely(locked(current_status))) {  // check lock
        continue;
      }

      if (likely(get_version(prev_ss_status_0) == get_version(current_ss_status_0))) {  // check version
        if (unlikely(is_ptr(current_status))) { // check is_ptr
          assert(!removed(current_status)); // check removed of pointer-type atomic value
		  assert(tmpptr != nullptr);
          return tmpptr->read_snapshot_0(val, ss_id);
        } else {
			val = tmpssval_0;
			ss_id = tmpssid_0;
			return !removed(current_ss_status_0); // check removed of snapshot value
        }
      }
    }
  }
  bool read_snapshot_1(val_t &val, int32_t &ss_id) {
    while (true) {
	  uint64_t prev_status = this->status;
      uint64_t prev_ss_status_1 = this->ss_status_1; // focus on version and remove
      memory_fence();
	  val_t tmpssval_1(this->ss_val_data_1, this->ss_val_length_1);
	  int32_t tmpssid_1 = this->ss_id_1;
	  AtomicVal *tmpptr = this->aval_ptr; // AtomicVal pointed by tmpptr will not be freed before finishing this read_snapshot() due to RCU during compact
      memory_fence();

	  uint64_t current_status = this->status; // focus on lock, is_ptr, and consistency of val
      memory_fence();
      uint64_t current_ss_status_1 = this->ss_status_1; // ensure consistent ss_val
      memory_fence();

      if (unlikely(locked(current_status))) {  // check lock
        continue;
      }

      if (likely(get_version(prev_ss_status_1) == get_version(current_ss_status_1))) {  // check version
        if (unlikely(is_ptr(current_status))) { // check is_ptr
          assert(!removed(current_status)); // check removed of pointer-type atomic value
		  assert(tmpptr != nullptr);
          return tmpptr->read_snapshot_1(val, ss_id);
        } else {
			val = tmpssval_1;
			ss_id = tmpssid_1;
			return !removed(current_ss_status_1); // check removed of snapshot value
        }
      }
    }
  }
  // AtomicVal in buffer and buffer_temp cannot be ptr-type
  bool read_snapshot_ignoring_ptr(val_t &val, int32_t snapshot_id) {
    while (true) {
	  uint64_t prev_status = this->status;
      uint64_t prev_ss_status_0 = this->ss_status_0; // focus on version and remove
      uint64_t prev_ss_status_1 = this->ss_status_1; // focus on version and remove
      memory_fence();
	  int32_t tmp_createid = this->create_id;
	  val_t tmpssval_0(this->ss_val_data_0, this->ss_val_length_0);
	  int32_t tmpssid_0 = this->ss_id_0;
	  val_t tmpssval_1(this->ss_val_data_1, this->ss_val_length_1);
	  int32_t tmpssid_1 = this->ss_id_1;
	  AtomicVal *tmpptr = this->aval_ptr; // AtomicVal pointed by tmpptr will not be freed before finishing this read_snapshot() due to RCU during compact
	  val_t tmpval(this->val_data, this->val_length);
      memory_fence();

	  uint64_t current_status = this->status; // focus on lock and is_ptr
      memory_fence();
      uint64_t current_ss_status_0 = this->ss_status_0; // ensure consistent ss_val
      memory_fence();
      uint64_t current_ss_status_1 = this->ss_status_1; // ensure consistent ss_val
      memory_fence();

      if (unlikely(locked(current_status))) {  // check lock
        continue;
      }

      if (likely(get_version(prev_ss_status_0) == get_version(current_ss_status_0) && 
				  get_version(prev_ss_status_1 == get_version(current_ss_status_1)))) {  // check version
		if (tmpssid_0 == snapshot_id) {
		  val = tmpssval_0; 
		  return !removed(current_ss_status_0); // check removed of snapshot value
		}
		else if (tmpssid_1 == snapshot_id) {
		  val = tmpssval_1; 
		  return !removed(current_ss_status_1); // check removed of snapshot value
		}
		// (1) current atomic value has not been snapshotted and without PUT/DEL; (2) new entry without snapshot but created at the previous epoch
		else if (tmp_createid < snapshot_id) {
			if (likely(get_version(prev_status) == get_version(current_status))) { // ensure the consistency of val
				// We leave snapshot of current atomic value in other threads (make_snapshot/PUT/DEL)
				val = tmpval;
				return !removed(current_status);
			}
		}
		// new entry without snapshot and created at the current epoch
		else {
			return false;
		}
      }
    }
  }

  // semantics: atomically read the value and the `removed` flag
  bool read(val_t &val) {
    while (true) {
      uint64_t status = this->status;
      memory_fence();
	  val_t tmpval(this->val_data, this->val_length);
	  AtomicVal *tmpptr = this->aval_ptr; // AtomicVal pointed by tmpptr will not be freed before finishing this read() due to RCU during compact
	  memory_fence();

	  uint64_t current_status = this->status; // ensure consistent tmpval
	  memory_fence();

	  if (unlikely(locked(current_status))) {  // check lock
		continue;
	  }

	  if (likely(get_version(status) ==
				 get_version(current_status))) {  // check version
		if (unlikely(is_ptr(status))) {
		  assert(!removed(status));
		  assert(tmpptr != nullptr);
		  return tmpptr->read(val);
		} else {
		  val = tmpval; 
		  return !removed(status);
		}
	  }
    }
  }
  bool update(const val_t &val, int32_t snapshot_id) {
    lock();
    uint64_t status = this->status;
    bool res;
    if (unlikely(is_ptr(status))) {
      assert(!removed(status));
	  assert(aval_ptr != nullptr);
      res = this->aval_ptr->update(val, snapshot_id);
    } else if (!removed(status)) {
	  if (this->ss_id_0 != snapshot_id && this->ss_id_1 != snapshot_id && this->create_id < snapshot_id) {
		  // (1) enpty snapshot; (2) older snapshot
		  if ((ss_id_0 == -1) || (ss_id_0 != -1 && ss_id_1 != -1 && ss_id_0 < ss_id_1 && ss_id_0 < snapshot_id)) {
			  ss_id_0 = snapshot_id;
			  ss_val_length_0 = val_length;
			  memcpy((char *)ss_val_data_0, (char *)val_data, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
			  if (removed(status)) {
				  ss_status_0 |= removed_mask;
			  }
			  ss_status_0++; // increase version 
		  }
		  else if ((ss_id_1 == -1) || (ss_id_0 != -1 && ss_id_1 != -1 && ss_id_0 >= ss_id_1 && ss_id_1 < snapshot_id)) {
			  ss_id_1 = snapshot_id;
			  ss_val_length_1 = val_length;
			  memcpy((char *)ss_val_data_1, (char *)val_data, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
			  if (removed(status)) {
				  ss_status_1 |= removed_mask;
			  }
			  ss_status_1++; // increase version 
		  }
	  }
	  this->val_length = val.val_length;
	  if (this->val_length > 0) {
		  memcpy((char *)this->val_data, (char *)val.val_data, this->val_length * sizeof(uint64_t));
	  }
      res = true;
    } else {
      res = false;
    }
    memory_fence();
    incr_version();
    memory_fence();
    unlock();
    return res;
  }
  bool remove(int32_t snapshot_id) {
    lock();
    uint64_t status = this->status;
    bool res;
    if (unlikely(is_ptr(status))) {
      assert(!removed(status));
	  assert(aval_ptr != nullptr);
      res = this->aval_ptr->remove(snapshot_id);
    } else if (!removed(status)) {
	  if (this->ss_id_0 != snapshot_id && this->ss_id_1 != snapshot_id && this->create_id < snapshot_id) {
		  // (1) enpty snapshot; (2) older snapshot
		  if ((ss_id_0 == -1) || (ss_id_0 != -1 && ss_id_1 != -1 && ss_id_0 < ss_id_1 && ss_id_0 < snapshot_id)) {
			  ss_id_0 = snapshot_id;
			  ss_val_length_0 = val_length;
			  memcpy((char *)ss_val_data_0, (char *)val_data, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
			  if (removed(status)) {
				  ss_status_0 |= removed_mask;
			  }
			  ss_status_0++; // increase version 
		  }
		  else if ((ss_id_1 == -1) || (ss_id_0 != -1 && ss_id_1 != -1 && ss_id_0 >= ss_id_1 && ss_id_1 < snapshot_id)) {
			  ss_id_1 = snapshot_id;
			  ss_val_length_1 = val_length;
			  memcpy((char *)ss_val_data_1, (char *)val_data, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
			  if (removed(status)) {
				  ss_status_1 |= removed_mask;
			  }
			  ss_status_1++; // increase version 
		  }
	  }
      set_removed();
      res = true;
    } else {
      res = false;
    }
    memory_fence();
    incr_version();
    memory_fence();
    unlock();
    return res;
  }
  void replace_pointer() {
    lock();
    uint64_t status = this->status;
    UNUSED(status);
    assert(is_ptr(status));
    assert(!removed(status));
	this->create_id = aval_ptr->create_id;
	// Get value
	val_t tmpval;
	assert(aval_ptr != nullptr);
    if (!aval_ptr->read(tmpval)) {
      set_removed();
    }
	this->val_length = tmpval.val_length;
	if (tmpval.val_length > 0) {
		memcpy((char *)this->val_data, (char *)tmpval.val_data, tmpval.val_length * sizeof(uint64_t));
	}
	// Get snapshot value 0
	val_t tmpssval_0;
	int32_t tmpssid_0;
	if (!aval_ptr->read_snapshot_0(tmpssval_0, tmpssid_0)) {
		ss_status_0 != removed_mask;
	}
	ss_id_0 = tmpssid_0;
	this->ss_val_length_0 = tmpssval_0.val_length;
	// NOTE: if val_t.val_length == 0, then val_t.val_data is null
	if (tmpssval_0.val_length > 0) {
		memcpy((char *)this->ss_val_data_0, (char *)tmpssval_0.val_data, tmpssval_0.val_length * sizeof(uint64_t));
	}
	// Get snapshot value 1
	val_t tmpssval_1;
	int32_t tmpssid_1;
	if (!aval_ptr->read_snapshot_1(tmpssval_1, tmpssid_1)) {
		ss_status_1 != removed_mask;
	}
	ss_id_1 = tmpssid_1;
	this->ss_val_length_1 = tmpssval_1.val_length;
	// NOTE: if val_t.val_length == 0, then val_t.val_data is null
	if (tmpssval_1.val_length > 0) {
		memcpy((char *)this->ss_val_data_1, (char *)tmpssval_1.val_data, tmpssval_1.val_length * sizeof(uint64_t));
	}
    unset_is_ptr();
    memory_fence();
    incr_version();
    memory_fence();
    unlock();
  }
  bool read_ignoring_ptr(val_t &val) {
    while (true) {
      uint64_t status = this->status;
      memory_fence();
	  val_t tmpval(this->val_data, this->val_length);
	  AtomicVal *tmpptr = this->aval_ptr;
      memory_fence();
      if (unlikely(locked(status))) {
        continue;
      }
      memory_fence();

      uint64_t current_status = this->status;
      if (likely(get_version(status) == get_version(current_status))) {
        val = tmpval;
        return !removed(status);
      }
    }
  }
  bool update_ignoring_ptr(const val_t &val, int32_t snapshot_id) {
    lock();
    uint64_t status = this->status;
    bool res;
    if (!removed(status)) {
	  if (this->ss_id_0 != snapshot_id && this->ss_id_1 != snapshot_id && this->create_id < snapshot_id) {
		  // (1) enpty snapshot; (2) older snapshot
		  if ((ss_id_0 == -1) || (ss_id_0 != -1 && ss_id_1 != -1 && ss_id_0 < ss_id_1 && ss_id_0 < snapshot_id)) {
			  ss_id_0 = snapshot_id;
			  ss_val_length_0 = val_length;
			  memcpy((char *)ss_val_data_0, (char *)val_data, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
			  if (removed(status)) {
				  ss_status_0 |= removed_mask;
			  }
			  ss_status_0++; // increase version 
		  }
		  else if ((ss_id_1 == -1) || (ss_id_0 != -1 && ss_id_1 != -1 && ss_id_0 >= ss_id_1 && ss_id_1 < snapshot_id)) {
			  ss_id_1 = snapshot_id;
			  ss_val_length_1 = val_length;
			  memcpy((char *)ss_val_data_1, (char *)val_data, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
			  if (removed(status)) {
				  ss_status_1 |= removed_mask;
			  }
			  ss_status_1++; // increase version 
		  }
	  }
	  this->val_length = val.val_length;
	  if (this->val_length > 0) {
		  memcpy((char *)this->val_data, (char *)val.val_data, this->val_length * sizeof(uint64_t));
	  }
      res = true;
    } else {
      res = false;
    }
    memory_fence();
    incr_version();
    memory_fence();
    unlock();
    return res;
  }
  bool remove_ignoring_ptr(int32_t snapshot_id) {
    lock();
    uint64_t status = this->status;
    bool res;
    if (!removed(status)) {
	  if (this->ss_id_0 != snapshot_id && this->ss_id_1 != snapshot_id && this->create_id < snapshot_id) {
		  // (1) enpty snapshot; (2) older snapshot
		  if ((ss_id_0 == -1) || (ss_id_0 != -1 && ss_id_1 != -1 && ss_id_0 < ss_id_1 && ss_id_0 < snapshot_id)) {
			  ss_id_0 = snapshot_id;
			  ss_val_length_0 = val_length;
			  memcpy((char *)ss_val_data_0, (char *)val_data, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
			  if (removed(status)) {
				  ss_status_0 |= removed_mask;
			  }
			  ss_status_0++; // increase version 
		  }
		  else if ((ss_id_1 == -1) || (ss_id_0 != -1 && ss_id_1 != -1 && ss_id_0 >= ss_id_1 && ss_id_1 < snapshot_id)) {
			  ss_id_1 = snapshot_id;
			  ss_val_length_1 = val_length;
			  memcpy((char *)ss_val_data_1, (char *)val_data, val_t::MAX_VAL_LENGTH * sizeof(uint64_t));
			  if (removed(status)) {
				  ss_status_1 |= removed_mask;
			  }
			  ss_status_1++; // increase version 
		  }
	  }
      set_removed();
      res = true;
    } else {
      res = false;
    }
    memory_fence();
    incr_version();
    memory_fence();
    unlock();
    return res;
  }
};

}  // namespace xindex

#endif  // XINDEX_UTIL_H
