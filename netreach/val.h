#ifndef VAL_H
#define VAL_H

#include <array>
#include <limits>
#include <string>
#include "rocksdb/slice.h"
#include "helper.h"

// MAX_VAL_LENGTH * 8B
#define MAX_VAL_LENGTH 1

class Val {

 public:

  static uint32_t max_bytesnum() {
	return MAX_VAL_LENGTH * sizeof(uint64_t);
  }

  Val();
  ~Val();
  Val(const uint64_t* buf, uint8_t length);
  Val(const Val &other);
  Val &operator=(const Val &other);

  rocksdb::Slice to_slice() const; // NOTE: slice is shallow copy

  void from_slice(rocksdb::Slice& slice);
  void from_string(std::string& str);

  // operation on packet buf (1B vallength + valdata)
  uint32_t deserialize(const char *buf);
  uint32_t serialize(char *buf);

  uint32_t get_bytesnum() const;

  std::string to_string() const; // For print

  uint8_t val_length; // val_length * 8B
  uint64_t *val_data;
} PACKED;

#endif
