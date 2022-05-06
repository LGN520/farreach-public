#ifndef CONCURRENT_VAL_H
#define CONCURRENT_VAL_H

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <mutex>

template <class val_t>
struct ConcurrentVal {

  // 60 bits for version
  static const uint64_t version_mask = 0x0fffffffffffffff;
  static const uint64_t lock_mask = 0x1000000000000000;

  // lock - removed - is_ptr
  val_t val;
  volatile uint64_t status;

  ConcurrentVal() : status(0) {}
  ~ConcurrentVal() {
  }
  ConcurrentVal(val_t val) : status(0) {
	  this->val = val;
  }
  ConcurrentVal(const ConcurrentVal& other) {
	  *this = other;
  }
  ConcurrentVal & operator=(const ConcurrentVal& other) {
	  this->val = other.val;
	  status = other.status;
	  return *this;
  }

  bool locked(uint64_t status) { return status & lock_mask; }
  uint64_t get_version(uint64_t status) { return status & version_mask; }

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

  void dump() {
	printf("val: %s\n", this->val.to_string().c_str());
	printf("locked: %d\n", int(this->locked));
	printf("version: %d\n", int(this->version));
  }

  // semantics: atomically read the value and the `removed` flag
  bool read(val_t &val) {
    while (true) {
      uint64_t status = this->status;
      memory_fence();
	  val_t tmpval = this->val;
	  memory_fence();

	  uint64_t current_status = this->status; // ensure consistent tmpval
	  memory_fence();

	  if (unlikely(locked(current_status))) {  // check lock
		continue;
	  }

	  if (likely(get_version(status) ==
				 get_version(current_status))) {  // check version
		val = tmpval; 
	  }
    }
	return true;
  }
  bool update(const val_t &val) {
    lock();
	this->val = val;
    memory_fence();
    incr_version();
    memory_fence();
    unlock();
    return true;
  }
};

#endif
