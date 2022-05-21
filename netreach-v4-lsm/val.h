#ifndef VAL_H
#define VAL_H

#include <array>
#include <limits>
#include <string>
//#include "rocksdb/slice.h"
#include "helper.h"

class Val {

 public:

  static uint16_t MAX_VALLEN; // max # of bytes
  static uint16_t SWITCH_MAX_VALLEN; // max # of bytes in switch
  static uint16_t get_padding_size(uint16_t vallen);

  Val();
  ~Val();
  Val(const char* buf, uint16_t length);
  Val(const Val &other);
  Val(const volatile Val &other);
  Val &operator=(const Val &other);
  //Val &operator=(const volatile Val &other);
  // Qualifiers of member functions are used to restrict "this"
  void operator=(const Val &other) volatile;
  //void operator=(const volatile Val &other) volatile;
  bool operator==(const Val& other);

  /*rocksdb::Slice to_slice() const; // NOTE: slice is shallow copy
  void from_slice(rocksdb::Slice& slice);*/
  std::string to_string_for_rocksdb(uint32_t seq);
  uint32_t from_string_for_rocksdb(std::string valstr); // return seq

  //void from_string(std::string& str);

  // operation on packet buf (1B vallength + valdata)
  uint32_t deserialize(const char *buf, uint32_t buflen);
  uint32_t deserialize(const char *buf, uint32_t buflen) volatile;
  uint32_t deserialize_vallen(const char *buf, uint32_t buflen);
  uint32_t deserialize_val(const char *buf, uint32_t buflen);
  uint32_t serialize(char *buf, uint32_t buflen);

  uint16_t get_bytesnum() const;

  std::string to_string_for_print() const; // For print

  //uint32_t val_length; // val_length (# of bytes)
  uint16_t val_length; // val_length (# of bytes)
  char *val_data;
} PACKED;

typedef Val val_t;

#endif
