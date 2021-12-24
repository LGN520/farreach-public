#ifndef KEY_H
#define KEY_H

#include <array>
#include <limits>
#include <string>
#include "rocksdb/slice.h"
#include "helper.h"

// Comment it for 8B small key
#define LARGE_KEY

class Key {
#ifdef LARGE_KEY
  typedef std::array<double, 2> model_key_t;
#else
  typedef std::array<double, 1> model_key_t;
#endif

 public:
  static constexpr size_t model_key_size() {
#ifdef LARGE_KEY
	return 2;
#else
	return 1; 
#endif
  }
  static Key max();
  static Key min();

  Key();
#ifdef LARGE_KEY
  Key(uint64_t keylo, uint64_t keyhi);
#else
  Key(uint64_t key);
#endif
  Key(const Key &other);
  Key &operator=(const Key &other);

  rocksdb::Slice to_slice() const;

  void from_slice(rocksdb::Slice& slice);

  model_key_t to_model_key() const;

  uint64_t to_int() const; // For hash

  std::string to_string() const; // For print

  uint32_t serialize(char *buf); // Serialize for cache population
  uint32_t deserialize(const char *buf);

  friend bool operator<(const Key &l, const Key &r);
  friend bool operator>(const Key &l, const Key &r);
  friend bool operator>=(const Key &l, const Key &r);
  friend bool operator<=(const Key &l, const Key &r);
  friend bool operator==(const Key &l, const Key &r);
  friend bool operator!=(const Key &l, const Key &r);

#ifdef LARGE_KEY
  uint64_t keylo;
  uint64_t keyhi;
#else
  uint64_t key;
#endif
} PACKED;

#endif
