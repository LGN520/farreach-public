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
#include <boost/thread/shared_mutex.hpp>
#include "../snapshot_helper.h"

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
  // Use dynamic memory for var-len value to save space
  val_t latest_val; // val_length = 0 and val_data = NULL
  AtomicVal *aval_ptr = nullptr; // Only aval_ptr makes sense for pointer-type atomic value

  // 60 bits for version
  //static const uint64_t version_mask = 0x0fffffffffffffff;
  //static const uint64_t lock_mask = 0x1000000000000000;
  static const uint64_t removed_mask = 0x2000000000000000;
  static const uint64_t pointer_mask = 0x4000000000000000;

  // removed - is_ptr (no lock and version now)
  uint64_t status;
  boost::shared_mutex rwlock; // fine-grained control

  // For snapshot (only valid for value-type AtomicVal)
  // Maintain one latest version and two versions for snapshot to support unfinished range query when making new snapshot
  // NOTE: the two snapshot versions may be in removed status due to merge_snapshot in compact
  int32_t latest_id = -1; // -1: impossible value; k: latest write happens at epoch k 
  uint32_t latest_seqnum = 0; // For serializability under potential packet loss

  val_t ss_val_0; // val_length = 0 and val_data = NULL
  uint32_t ss_seqnum_0 = 0;
  int32_t ss_id_0 = -1; // -1: no snapshot; otherwise: snapshot of epoch k 
  uint64_t ss_status_0 = 0; // only focus on removed w/o lock, is_ptr, and version

  val_t ss_val_1; // val_length = 0 and val_data = NULL
  uint32_t ss_seqnum_1 = 0;
  int32_t ss_id_1 = -1; // -1: no snapshot; otherwise: snapshot of epoch k'
  uint64_t ss_status_1 = 0; // only focus on removed w/o lock, is_ptr, and version

  AtomicVal() : status(0) {}
  ~AtomicVal() {}
  AtomicVal(val_t val, int32_t snapshot_id, uint32_t seqnum) : status(0) {
	  this->latest_val = val; // deep copy
	  // Empty snapshot
	  ss_id_0 = -1;
	  ss_id_1 = -1;
	  latest_id = snapshot_id; 
	  latest_seqnum = seqnum;
	  memory_fence(); // store to memory
  }
  AtomicVal(AtomicVal *ptr) : aval_ptr(ptr), status(0) { set_is_ptr(); }
  AtomicVal(const AtomicVal& other) {
	  *this = other;
  }
  AtomicVal & operator=(const AtomicVal& other) {
	  latest_id = other.latest_id;
	  latest_seqnum = other.latest_seqnum;
	  latest_val = other.latest_val;
	  status = other.status;
	  ss_val_0 = other.ss_val_0;
	  ss_seqnum_0 = other.ss_seqnum_0;
	  ss_id_0 = other.ss_id_0;
	  ss_status_0 = other.ss_status_0;
	  ss_val_1 = other.ss_val_1;
	  ss_seqnum_1 = other.ss_seqnum_1;
	  ss_id_1 = other.ss_id_1;
	  ss_status_1 = other.ss_status_1;
	  aval_ptr = other.aval_ptr;
	  memory_fence(); // store to memory
	  return *this;
  }

  bool is_ptr(uint64_t status) { return status & pointer_mask; }
  bool removed(uint64_t status) { return status & removed_mask; }
  //bool locked(uint64_t status) { return status & lock_mask; }
  //uint64_t get_version(uint64_t status) { return status & version_mask; }

  void set_is_ptr() { status |= pointer_mask; }
  void unset_is_ptr() { status &= ~pointer_mask; }
  void set_removed() { status |= removed_mask; }
  /*void lock() {
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
  }*/

  friend std::ostream &operator<<(std::ostream &os, const AtomicVal &leaf) {
	COUT_VAR(leaf.latest_val.to_string());
	COUT_VAR(leaf.aval_ptr);
    COUT_VAR(leaf.is_ptr(leaf.status));
    COUT_VAR(leaf.removed(leaf.status));
    //COUT_VAR(leaf.locked);
    //COUT_VAR(leaf.version);
    return os;
  }

  // Snapshot by multi-versioning
  //bool read_snapshot(val_t &val, int32_t &ss_id, int32_t snapshot_id) {
  bool read_snapshot(val_t &val, uint32_t &seqnum, int32_t snapshot_id) {
	while (true) {
	  if (this->rwlock.try_lock_shared()) break;
	}
	memory_fence(); // re-load registers
	  
	bool res = false;
	if (unlikely(is_ptr(this->status))) {
      assert(!removed(this->status)); // check removed of pointer-type atomic value
	  assert(this->aval_ptr != nullptr);
	  // AtomicVal pointed by aval_ptr will not be freed before finishing this read_snapshot() due to RCU during compact
      res = this->aval_ptr->read_snapshot(val, seqnum, snapshot_id);
	}
	else {
	  // At most one memcpy
	  if (this->latest_id < snapshot_id) {
		  if (!removed(this->status)) {
			val = this->latest_val;
			res = true;
		  }
	  }
	  else {
		  int32_t tmpssid_0 = this->ss_id_0;
		  int32_t tmpssid_1 = this->ss_id_1;
		  if (tmpssid_0 <= tmpssid_1) {
			  if (tmpssid_1 != -1 && tmpssid_1 < snapshot_id) {
				  if (!removed(this->ss_status_1)) {
					  val = this->ss_val_1;
					  seqnum = this->ss_seqnum_1;
					  res = true;
				  }
			  }
			  else if (tmpssid_0 != -1 && tmpssid_0 < snapshot_id) {
				  if (!removed(this->ss_status_0)) {
					  val = this->ss_val_0;
					  seqnum = this->ss_seqnum_0;
					  res = true;
				  }
			  }
		  }
		  else {
			  if (tmpssid_0 != -1 && tmpssid_0 < snapshot_id) {
				  if (!removed(this->ss_status_0)) {
					  val = this->ss_val_0;
					  seqnum = this->ss_seqnum_0;
					  res = true;
				  }
			  }
			  else if (tmpssid_1 != -1 && tmpssid_1 < snapshot_id) {
				  if (!removed(this->ss_status_1)) {
					  val = this->ss_val_1;
					  seqnum = this->ss_seqnum_1;
					  res = true;
				  }
			  }
		  }
	  }
	}
	this->rwlock.unlock_shared();
	return res;
  }
  bool read_snapshot_0(val_t &val, uint32_t &seqnum, int32_t &ss_id) {
	while (true) {
	  if (this->rwlock.try_lock_shared()) break;
	}
	memory_fence(); // re-load registers
	  
	bool res = false;
	if (unlikely(is_ptr(this->status))) { // check is_ptr
	  assert(!removed(this->status)); // check removed of pointer-type atomic value
	  assert(this->aval_ptr != nullptr);
	  // AtomicVal pointed by aval_ptr will not be freed before finishing this read_snapshot() due to RCU during compact
	  res = this->aval_ptr->read_snapshot_0(val, seqnum, ss_id);
	}
	else {
	  // At most one memcpy
	  ss_id = this->ss_id_0;
	  if (this->ss_id_0 != -1 && !removed(this->ss_status_0)) {
		  val = this->ss_val_0;
		  seqnum = this->ss_seqnum_0;
		  res = true;
	  }
	}
	this->rwlock.unlock_shared();
	return res;
  }
  bool read_snapshot_1(val_t &val, uint32_t &seqnum, int32_t &ss_id) {
	while (true) {
	  if (this->rwlock.try_lock_shared()) break;
	}
	memory_fence();
	  
	bool res = false;
	if (unlikely(is_ptr(this->status))) { // check is_ptr
	  assert(!removed(this->status)); // check removed of pointer-type atomic value
	  assert(this->aval_ptr != nullptr);
	  // AtomicVal pointed by aval_ptr will not be freed before finishing this read_snapshot() due to RCU during compact
	  res = this->aval_ptr->read_snapshot_1(val, seqnum, ss_id);
	}
	else {
	  // At most one memcpy
	  ss_id = this->ss_id_1;
	  if (this->ss_id_1 != -1 && !removed(this->ss_status_1)) {
		  val = this->ss_val_1;
		  seqnum = this->ss_seqnum_1;
		  res = true;
	  }
	}
	this->rwlock.unlock_shared();
	return res;
  }
  // Snapshot by multi-versioning while ignoring ptr, as AtomicVal in buffer and buffer_temp cannot be ptr-type
  //bool read_snapshot_ignoring_ptr(val_t &val, int32_t &ss_id, int32_t snapshot_id) {
  bool read_snapshot_ignoring_ptr(val_t &val, uint32_t &seqnum, int32_t snapshot_id) {
	while (true) {
	  if (this->rwlock.try_lock_shared()) break;
	}
	memory_fence(); // re-load registers
	  
	bool res = false;
	// At most one memcpy
	if (this->latest_id < snapshot_id) {
	  if (!removed(this->status)) {
		val = this->latest_val;
		res = true;
	  }
    }
	else {
	  int32_t tmpssid_0 = this->ss_id_0;
	  int32_t tmpssid_1 = this->ss_id_1;
	  if (tmpssid_0 <= tmpssid_1) {
		  if (tmpssid_1 != -1 && tmpssid_1 < snapshot_id) {
			  if (!removed(this->ss_status_1)) {
				  val = this->ss_val_1;
				  seqnum = this->ss_seqnum_1;
				  res = true;
			  }
		  }
		  else if (tmpssid_0 != -1 && tmpssid_0 < snapshot_id) {
			  if (!removed(this->ss_status_0)) {
				  val = this->ss_val_0;
				  seqnum = this->ss_seqnum_0;
				  res = true;
			  }
		  }
	  }
	  else {
		  if (tmpssid_0 != -1 && tmpssid_0 < snapshot_id) {
			  if (!removed(this->ss_status_0)) {
				  val = this->ss_val_0;
				  seqnum = this->ss_seqnum_0;
				  res = true;
			  }
		  }
		  else if (tmpssid_1 != -1 && tmpssid_1 < snapshot_id) {
			  if (!removed(this->ss_status_1)) {
				  val = this->ss_val_1;
				  seqnum = this->ss_seqnum_1;
				  res = true;
			  }
		  }
	  }
	}
	this->rwlock.unlock_shared();
	return res;
  }
  // Try to merge snapshot versions from data.base_val into buffer.buf_val
  // TODO: we do not need this function after treating DELREQ as special PUTREQ
  bool merge_snapshot(uint32_t &data_latestseqnum, int32_t &data_latestid, val_t &data_ssval_0, uint32_t &data_ssseqnum_0, int32_t &data_ssid_0, bool data_ssremoved_0, val_t &data_ssval_1, uint32_t &data_ssseqnum_1, int32_t &data_ssid_1, bool data_ssremoved_1) {
	while (true) {
		if (this->rwlock.try_lock()) break;
	}
	memory_fence(); // re-load registers

	// NOTE: buffer.buf_val.min_snapshot_id >= data.base_val.max_snapshot_id
	if (this->ss_id_0 == -1 || this->ss_id_1 == -1) { // with at least one empty snapshot entry
		val_t *newer_ssval = NULL;
		val_t *older_ssval = NULL;
		uint32_t *newer_ssseqnum = NULL;
		uint32_t * older_ssseqnum = NULL;
		int32_t *newer_ssid = NULL;
		int32_t *older_ssid = NULL;
		bool newer_removed, older_removed;
		if (this->latest_id == data_latestid || this->ss_id_0 == data_latestid || this->ss_id_1 == data_latestid) {
			newer_ssseqnum = &data_ssseqnum_0;
			newer_ssid = &data_ssid_0;
			newer_ssval = &data_ssval_0;
			newer_removed = data_ssremoved_0;
			older_ssseqnum = &data_ssseqnum_1;
			older_ssid = &data_ssid_1;
			older_ssval = &data_ssval_1;
			older_removed = data_ssremoved_1;
			if (data_ssid_0 < data_ssid_1) {
				newer_ssseqnum = &data_ssseqnum_1;
				newer_ssid = &data_ssid_1;
				newer_ssval = &data_ssval_1;
				newer_removed = data_ssremoved_1;
				older_ssseqnum = &data_ssseqnum_0;
				older_ssid = &data_ssid_0;
				older_ssval = &data_ssval_0;
				older_removed = data_ssremoved_0;
			}
		}
		else {
			newer_ssseqnum = &data_latestseqnum;
			newer_ssid = &data_latestid; // NOTE: latest value in data.base_val must be removed
			newer_removed = true;
			if (data_ssid_0 >= data_ssid_1) { // Choose the newer snapshot
				older_ssseqnum = &data_ssseqnum_0;
				older_ssid = &data_ssid_0;
				older_ssval = &data_ssval_0;
				older_removed = data_ssremoved_0;
			}
			else {
				older_ssseqnum = &data_ssseqnum_1;
				older_ssid = &data_ssid_1;
				older_ssval = &data_ssval_1;
				older_removed = data_ssremoved_1;
			}
		}
		if (this->ss_id_0 == -1) {
			this->ss_seqnum_0 = *newer_ssseqnum;
			this->ss_id_0 = *newer_ssid; 
			this->ss_status_0 = 0; // set not removed
			if (!newer_removed) {
				this->ss_val_0 = *newer_ssval;
			}
			else {
				this->ss_status_0 |= removed_mask; // set removed
			}
			if (this->ss_id_1 == -1) {
				this->ss_seqnum_1 = *older_ssseqnum;
				this->ss_id_1 = *older_ssid; 
				this->ss_status_1 = 0; // set not removed
				if (!older_removed) {
					this->ss_val_1 = *older_ssval;
				}
				else {
					this->ss_status_1 |= removed_mask; // set removed
				}
			}
		}
		else { // only this->ss_id_1 == -1
			this->ss_seqnum_1 = *newer_ssseqnum;
			this->ss_id_1 = *newer_ssid; 
			this->ss_status_1 = 0; // set not removed
			if (!newer_removed) {
				this->ss_val_1 = *newer_ssval;
			}
			else {
				this->ss_status_1 |= removed_mask; // set removed
			}
		}
	}
	bool res = removed(this->status) && (this->ss_id_0 == -1 || removed(this->ss_status_0)) && (this->ss_id_1 == -1 || removed(this->ss_status_1));
	memory_fence(); // store to memory
    this->rwlock.unlock();
	return res;
  }

  // semantics: atomically read the value and the `removed` flag
  bool read(val_t &val, uint32_t &seqnum) {
	while (true) {
	  if (this->rwlock.try_lock_shared()) break;
	}
	memory_fence(); // re-load registers

	bool res = false;
	if (unlikely(is_ptr(this->status))) { // check is_ptr
	  assert(!removed(this->status)); // check removed of pointer-type atomic value
	  assert(this->aval_ptr != nullptr);
	  // AtomicVal pointed by aval_ptr will not be freed before finishing this read_snapshot() due to RCU during compact
	  res = this->aval_ptr->read(val, seqnum);
	}
	else {
		if (!removed(this->status)) {
			val = this->latest_val;
			seqnum = this->latest_seqnum;
			res = true;
		}
	}
	this->rwlock.unlock_shared();
	return res;
  }
  bool read_with_latestid_seqnum(val_t &val, int32_t &id, uint32_t &seqnum) {
	while (true) {
	  if (this->rwlock.try_lock_shared()) break;
	}
	memory_fence(); // re-load registers

	bool res = false;
	if (unlikely(is_ptr(this->status))) { // check is_ptr
	  assert(!removed(this->status)); // check removed of pointer-type atomic value
	  assert(this->aval_ptr != nullptr);
	  // AtomicVal pointed by aval_ptr will not be freed before finishing this read_snapshot() due to RCU during compact
	  res = this->aval_ptr->read_with_latestid_seqnum(val, id, seqnum);
	}
	else {
		id = this->latest_id;
		seqnum = this->latest_seqnum;
		if (!removed(this->status)) {
			val = this->latest_val;
			res = true;
		}
	}
	this->rwlock.unlock_shared();
	return res;
  }
  // Return true only if the latest value is removed and both snapshot versions are removed or non-existing
  bool all_removed() {
	while (true) {
	  if (this->rwlock.try_lock_shared()) break;
	}
	memory_fence(); // re-load registers

	bool res = false;
	if (unlikely(is_ptr(this->status))) { // check is_ptr
	  assert(!removed(this->status)); // check removed of pointer-type atomic value
	  assert(this->aval_ptr != nullptr);
	  // AtomicVal pointed by aval_ptr will not be freed before finishing this read_snapshot() due to RCU during compact
	  res = this->aval_ptr->all_removed();
	}
	else {
	  res = removed(this->status) && (this->ss_id_0 == -1 || removed(this->ss_status_0)) && (this->ss_id_1 == -1 || removed(this->ss_status_1));
	}
	this->rwlock.unlock_shared();
	return res;
  }
  bool force_update(const val_t &val, int32_t snapshot_id) {
	while (true) {
		if (this->rwlock.try_lock()) break;
	}
    bool res = false;
    if (unlikely(is_ptr(this->status))) {
      assert(!removed(this->status));
	  assert(this->aval_ptr != nullptr);
      res = this->aval_ptr->force_update(val, snapshot_id);
    } else if (!removed(this->status)) { // do not make snapshot for removed latest version
		// Keep original latest_seqnum (only used in loading phase)
		INVARIANT(this->latest_seqnum == 0);
		if (this->latest_id == snapshot_id) {
			this->latest_val = val;
		}
		else {
			// latest_id must > any ss_id
			if (this->ss_id_0 == -1 || this->ss_id_0 <= this->ss_id_1) {
				this->ss_seqnum_0 = this->latest_seqnum;
				this->ss_id_0 = this->latest_id;
				this->ss_val_0 = this->latest_val;
				this->ss_status_0 = 0; // set not removed (latest value must not be removed here)
			}
			else if (this->ss_id_1 == -1 || this->ss_id_1 <= this->ss_id_0) {
				this->ss_seqnum_1 = this->latest_seqnum;
				this->ss_id_1 = this->latest_id;
				this->ss_val_1 = this->latest_val;
				this->ss_status_1 = 0; // set not removed (latest value must not be removed here)
			}

			this->latest_id = snapshot_id;
			this->latest_val = val;
		}
		res = true;
    } else { // if the latest version is removed, xindex should insert the val into buffer/buffer_temp as a new record
      res = false;
    }
    this->rwlock.unlock();
    return res;
  }
  bool update(const val_t &val, int32_t snapshot_id, uint32_t seqnum) {
	while (true) {
		if (this->rwlock.try_lock()) break;
	}
	memory_fence(); // re-load registers

    bool res = false;
    if (unlikely(is_ptr(this->status))) {
      assert(!removed(this->status));
	  assert(this->aval_ptr != nullptr);
      res = this->aval_ptr->update(val, snapshot_id, seqnum);
    } else if (!removed(this->status)) { // do not make snapshot for removed latest version
		if (seqnum > this->latest_seqnum) {
			this->latest_seqnum = seqnum;
			if (this->latest_id == snapshot_id) {
				this->latest_val = val;
			}
			else {
				// latest_id must > any ss_id
				if (this->ss_id_0 == -1 || this->ss_id_0 <= this->ss_id_1) {
					this->ss_seqnum_0 = this->latest_seqnum;
					this->ss_id_0 = this->latest_id;
					this->ss_val_0 = this->latest_val;
					this->ss_status_0 = 0; // set not removed (latest value must not be removed here)
				}
				else if (this->ss_id_1 == -1 || this->ss_id_1 <= this->ss_id_0) {
					this->ss_seqnum_1 = this->latest_seqnum;
					this->ss_id_1 = this->latest_id;
					this->ss_val_1 = this->latest_val;
					this->ss_status_1 = 0; // set not removed (latest value must not be removed here)
				}

				this->latest_id = snapshot_id;
				this->latest_val = val;
			}
		}
		res = true;
    } else { // if the latest version is removed, xindex should insert the val into buffer/buffer_temp as a new record
      res = false;
    }
	memory_fence(); // store to memory
    this->rwlock.unlock();
    return res;
  }
  bool remove(int32_t snapshot_id, uint32_t seqnum) {
	while (true) {
		if (this->rwlock.try_lock()) break;
	}
	memory_fence(); // re-load registers

    bool res = false;
    if (unlikely(is_ptr(this->status))) {
      assert(!removed(this->status));
	  assert(this->aval_ptr != nullptr);
      res = this->aval_ptr->remove(snapshot_id, seqnum);
    } else if (!removed(this->status)) { // do not make snapshot for removed latest version
		if (seqnum > this->latest_seqnum) {
			this->latest_seqnum = seqnum;
			if (this->latest_id == snapshot_id) {
			  set_removed();
			  this->latest_val = val_t(); // val_length = 0 and val_data = NULL
			}
			else {
				// latest_id must > any ss_id
				if (this->ss_id_0 == -1 || this->ss_id_0 <= this->ss_id_1) {
					this->ss_seqnum_0 = this->latest_seqnum;
					this->ss_id_0 = this->latest_id;
					this->ss_val_0 = this->latest_val;
					this->ss_status_0 = 0; // set not removed (latest value must not be removed here)
				}
				else if (this->ss_id_1 == -1 || this->ss_id_1 <= this->ss_id_0) {
					this->ss_seqnum_1 = this->latest_seqnum;
					this->ss_id_1 = this->latest_id;
					this->ss_val_1 = this->latest_val;
					this->ss_status_1 = 0; // set not removed (latest value must not be removed here)
				}

				this->latest_id = snapshot_id;
				set_removed();
				this->latest_val = val_t(); // val_length = 0 and val_data = NULL
			}
		}
		res = true;
    } else {
      res = false;
    }
	memory_fence(); // store to memory
    this->rwlock.unlock();
    return res;
  }
  void replace_pointer() {
	while (true) {
		if (this->rwlock.try_lock()) break;
	}
	memory_fence(); // re-load registers

    uint64_t status = this->status;
    UNUSED(status);
    assert(is_ptr(status));
    assert(!removed(status));
	// Get value
	assert(aval_ptr != nullptr);
	val_t tmp_latestval;
	int32_t tmp_latestid;
	uint32_t tmp_seqnum;
    if (!aval_ptr->read_with_latestid_seqnum(tmp_latestval, tmp_latestid, tmp_seqnum)) {
      set_removed();
    }
	this->latest_val = tmp_latestval;
	this->latest_id = tmp_latestid;
	this->latest_seqnum = tmp_seqnum;
	// Get snapshot value 0
	val_t tmp_ssval_0;
	int32_t tmp_ssid_0;
	uint32_t tmp_ssseqnum_0;
	if (!aval_ptr->read_snapshot_0(tmp_ssval_0, tmp_ssseqnum_0, tmp_ssid_0)) {
		this->ss_status_0 |= removed_mask;
	}
	this->ss_val_0 = tmp_ssval_0;
	this->ss_seqnum_0 = tmp_ssseqnum_0;
	this->ss_id_0 = tmp_ssid_0;
	// Get snapshot value 1
	val_t tmp_ssval_1;
	uint32_t tmp_ssseqnum_1;
	int32_t tmp_ssid_1;
	if (!aval_ptr->read_snapshot_1(tmp_ssval_1, tmp_ssseqnum_1, tmp_ssid_1)) {
		this->ss_status_1 |= removed_mask;
	}
	this->ss_val_1 = tmp_ssval_1;
	this->ss_seqnum_1 = tmp_ssseqnum_1;
	this->ss_id_1 = tmp_ssid_1;
    unset_is_ptr();
    memory_fence(); // store to memory
    this->rwlock.unlock();
  }
  bool read_ignoring_ptr(val_t &val, uint32_t &seqnum) {
	while (true) {
	  if (this->rwlock.try_lock_shared()) break;
	}
	memory_fence(); // re-load registers

	bool res = false;
	if (!removed(this->status)) {
		val = this->latest_val;
		seqnum = this->latest_seqnum;
		res = true;
	}
	this->rwlock.unlock_shared();
	return res;
  }
  // Return true only if the latest value is removed and both snapshot versions are removed or non-existing
  bool all_removed_ignoring_ptr() {
	while (true) {
	  if (this->rwlock.try_lock_shared()) break;
	}
	memory_fence(); // re-load registers

	bool res = false;
	res = removed(this->status) && (this->ss_id_0 == -1 || removed(this->ss_status_0)) && (this->ss_id_1 == -1 || removed(this->ss_status_1));
	this->rwlock.unlock_shared();
	return res;
  }
  bool force_update_ignoring_ptr(const val_t &val, int32_t snapshot_id) {
	while (true) {
		if (this->rwlock.try_lock()) break;
	}
	memory_fence(); // re-load registers

    bool res = false;
    if (!removed(this->status)) { // do not make snapshot for removed latest version
		// Keep original latest_seqnum (only used in loading phase)
		INVARIANT(this->latest_seqnum == 0);
		if (this->latest_id == snapshot_id) {
		  this->latest_val = val;
		}
		else {
			// latest_id must > any ss_id
			if (this->ss_id_0 == -1 || this->ss_id_0 <= this->ss_id_1) {
				this->ss_id_0 = this->latest_id;
				this->ss_seqnum_0 = this->latest_seqnum;
				this->ss_val_0 = this->latest_val;
				this->ss_status_0 = 0; // set not removed (latest value must not be removed here)
			}
			else if (this->ss_id_1 == -1 || this->ss_id_1 <= this->ss_id_0) {
				this->ss_id_1 = this->latest_id;
				this->ss_seqnum_1 = this->latest_seqnum;
				this->ss_val_1 = this->latest_val;
				this->ss_status_1 = 0; // set not removed (latest value must not be removed here)
			}

			this->latest_id = snapshot_id;
			this->latest_val = val;
		}
		res = true;
    } else { // if the latest version is removed, xindex should insert the val into buffer/buffer_temp as a new record
      res = false;
    }

	memory_fence(); // store to memory
    this->rwlock.unlock();
    return res;
  }
  bool update_ignoring_ptr(const val_t &val, int32_t snapshot_id, uint32_t seqnum) {
	while (true) {
		if (this->rwlock.try_lock()) break;
	}
	memory_fence(); // re-load registers

    bool res = false;
    if (!removed(this->status)) { // do not make snapshot for removed latest version
		if (seqnum > this->latest_seqnum) {
			this->latest_seqnum = seqnum;
			if (this->latest_id == snapshot_id) {
			  this->latest_val = val;
			}
			else {
				// latest_id must > any ss_id
				if (this->ss_id_0 == -1 || this->ss_id_0 <= this->ss_id_1) {
					this->ss_id_0 = this->latest_id;
					this->ss_seqnum_0 = this->latest_seqnum;
					this->ss_val_0 = this->latest_val;
					this->ss_status_0 = 0; // set not removed (latest value must not be removed here)
				}
				else if (this->ss_id_1 == -1 || this->ss_id_1 <= this->ss_id_0) {
					this->ss_id_1 = this->latest_id;
					this->ss_seqnum_1 = this->latest_seqnum;
					this->ss_val_1 = this->latest_val;
					this->ss_status_1 = 0; // set not removed (latest value must not be removed here)
				}

				this->latest_id = snapshot_id;
				this->latest_val = val;
			}
		}
		res = true;
    } else { // if the latest version is removed, xindex should insert the val into buffer/buffer_temp as a new record
      res = false;
    }

	memory_fence(); // store to memory
    this->rwlock.unlock();
    return res;
  }
  bool remove_ignoring_ptr(int32_t snapshot_id, uint32_t seqnum) {
	while (true) {
		if (this->rwlock.try_lock()) break;
	}
	memory_fence(); // re-load registers

    bool res = false;
    if (!removed(this->status)) { // do not make snapshot for removed latest version
		if (seqnum > this->latest_seqnum) {
			this->latest_seqnum = seqnum;
			if (this->latest_id == snapshot_id) {
			  set_removed();
			  this->latest_val = val_t(); // val_length = 0 and val_data = NULL
			}
			else {
				// latest_id must > any ss_id
				if (this->ss_id_0 == -1 || this->ss_id_0 <= this->ss_id_1) {
					this->ss_id_0 = this->latest_id;
					this->ss_seqnum_0 = this->latest_seqnum;
					this->ss_val_0 = this->latest_val;
					this->ss_status_0 = 0; // set not removed (latest value must not be removed here)
				}
				else if (this->ss_id_1 == -1 || this->ss_id_1 <= this->ss_id_0) {
					this->ss_id_1 = this->latest_id;
					this->ss_seqnum_1 = this->latest_seqnum;
					this->ss_val_1 = this->latest_val;
					this->ss_status_1 = 0; // set not removed (latest value must not be removed here)
				}

				this->latest_id = snapshot_id;
				set_removed();
				this->latest_val = val_t(); // val_length = 0 and val_data = NULL
			}
		}
		res = true;
    } else {
      res = false;
    }

	memory_fence(); // store to memory
    this->rwlock.unlock();
    return res;
  }
};

}  // namespace xindex

#endif  // XINDEX_UTIL_H
