#ifndef KEY_H
#define KEY_H

#include <array>
#include <limits>
#include <string>
//#include "rocksdb/slice.h"
#include "helper.h"
#include "crc32.h"

// Comment it for 8B small key
#define LARGE_KEY

class Key {
/*#ifdef LARGE_KEY
  typedef std::array<double, 4> model_key_t;
#else
  typedef std::array<double, 2> model_key_t;
#endif*/

 public:
  /*static constexpr size_t model_key_size() { // # of 4B doulbes
#ifdef LARGE_KEY
	return 4;
#else
	return 2; 
#endif
  }*/
  static Key max();
  static Key min();

  Key();
#ifdef LARGE_KEY
  Key(uint32_t keylolo, uint32_t keylohi, uint32_t keyhilo, uint32_t keyhihi);
#else
  Key(uint32_t keylo, uint32_t keyhi);
#endif
  Key(const Key &other);
  Key(const Key volatile &other);
  Key &operator=(const Key &other);
  Key &operator=(const Key volatile &other);
  void operator=(const Key &other) volatile; // suppress warning of implicit deference of volatile type

  // not use Slice due to shallow copy
  //rocksdb::Slice to_slice() const;
  //void from_slice(rocksdb::Slice& slice);
  std::string to_string_for_rocksdb() const;
  void from_string_for_rocksdb(std::string& keystr);

  //model_key_t to_model_key_for_train() const; // for xindex

  int64_t to_int_for_hash() const; // for hash

  std::string to_string_for_print() const; // for print

  friend bool operator<(const Key &l, const Key &r);
  friend bool operator>(const Key &l, const Key &r);
  friend bool operator>=(const Key &l, const Key &r);
  friend bool operator<=(const Key &l, const Key &r);
  friend bool operator==(const Key &l, const Key &r);
  friend bool operator!=(const Key &l, const Key &r);

  // operation on packet buf (16B key)
  uint32_t deserialize(const char *buf, uint32_t buflen);
  uint32_t serialize(char *buf, uint32_t buflen);
  uint32_t serialize(char *buf, uint32_t buflen) const;
  uint32_t serialize(char *buf, uint32_t buflen) volatile;

  uint32_t get_hashpartition_idx(uint32_t partitionnum, uint32_t servernum);

#ifdef LARGE_KEY
  uint32_t keylolo;
  uint32_t keylohi;
  uint32_t keyhilo;
  uint32_t keyhihi;
#else
  uint32_t keylo;
  uint32_t keyhi;
#endif
} PACKED;

typedef Key netreach_key_t;

#endif
