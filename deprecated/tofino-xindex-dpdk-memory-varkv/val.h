#ifndef VAL_H
#define VAL_H

#include <array>
#include <limits>
#include <string>
//#include "rocksdb/slice.h"
#include "helper.h"

class Val {

 public:

  static uint32_t MAX_VAL_LENGTH;

  // MAX_VAL_LENGTH * 8B
  static uint32_t max_bytesnum() {
	return MAX_VAL_LENGTH * sizeof(uint64_t);
  }

  Val();
  ~Val();
  Val(const uint64_t* buf, uint8_t length);
  Val(const Val &other);
  Val(const volatile Val &other);
  Val &operator=(const Val &other);
  Val &operator=(const volatile Val &other);
  // Qualifiers of member functions are used to restrict "this"
  volatile Val &operator=(const Val &other) volatile;
  volatile Val &operator=(const volatile Val &other) volatile;
  bool operator==(const Val& other);

  /*rocksdb::Slice to_slice() const; // NOTE: slice is shallow copy
  void from_slice(rocksdb::Slice& slice);*/

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
