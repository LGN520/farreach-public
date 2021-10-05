#ifndef VAL_H
#define VAL_H

#include <array>
#include <limits>
#include <string>
#include "rocksdb/slice.h"

#define MAX_VAL_LENGTH 96

class Val {

 public:

  Val();
  ~Val();
  Val(const char* buf, uint8_t length);
  Val(const Val &other);
  Val &operator=(const Val &other);

  rocksdb::Slice to_slice() const; // NOTE: slice is shallow copy

  void from_slice(rocksdb::Slice& slice);

  std::string to_string() const; // For print

  uint8_t val_length;
  char *val_data;
} PACKED;

#endif
