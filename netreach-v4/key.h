#ifndef KEY_H
#define KEY_H

#include <array>
#include <limits>
#include <string>
//#include "rocksdb/slice.h"
#include "helper.h"

// Comment it for 8B small key
#define LARGE_KEY

class Key {
#ifdef LARGE_KEY
  typedef std::array<double, 4> model_key_t;
#else
  typedef std::array<double, 2> model_key_t;
#endif

 public:
  static constexpr size_t model_key_size() { // # of 4B doulbes
#ifdef LARGE_KEY
	return 4;
#else
	return 2; 
#endif
  }
  static Key max();
  static Key min();

  Key();
#ifdef LARGE_KEY
  Key(int32_t keylolo, int32_t keylohi, int32_t keyhilo, int32_t keyhihi);
#else
  Key(int32_t keylo, int32_t keyhi);
#endif
  Key(const Key &other);
  Key(const Key volatile &other);
  Key &operator=(const Key &other);
  Key &operator=(const Key volatile &other);
  void operator=(const Key &other) volatile; // suppress warning of implicit deference of volatile type

  /*rocksdb::Slice to_slice() const;
  void from_slice(rocksdb::Slice& slice);*/

  model_key_t to_model_key() const;

  int64_t to_int() const; // For hash

  std::string to_string() const; // For print

  friend bool operator<(const Key &l, const Key &r);
  friend bool operator>(const Key &l, const Key &r);
  friend bool operator>=(const Key &l, const Key &r);
  friend bool operator<=(const Key &l, const Key &r);
  friend bool operator==(const Key &l, const Key &r);
  friend bool operator!=(const Key &l, const Key &r);

  // operation on packet buf (16B key)
  uint32_t deserialize(const char *buf, uint32_t buflen);
  uint32_t serialize(char *buf, uint32_t buflen);
  uint32_t serialize(char *buf, uint32_t buflen) volatile;
  //uint32_t size() const;

#ifdef LARGE_KEY
  int32_t keylolo;
  int32_t keylohi;
  int32_t keyhilo;
  int32_t keyhihi;
#else
  int32_t keylo;
  int32_t keyhi;
#endif
} PACKED;

#endif
