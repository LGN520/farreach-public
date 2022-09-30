#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <fstream>

#include "helper.h"

class Key;
typedef Key index_key_t;

inline void parse_args(int, char **);
inline void prepare();
void save();

// parameters
double insert_ratio = 0;
size_t table_size = 1000000;

std::vector<index_key_t> exist_keys;
std::vector<index_key_t> non_exist_keys;

class Key {
  typedef std::array<double, 1> model_key_t;

 public:
  static constexpr size_t model_key_size() { return 1; }
  static Key max() {
    static Key max_key(std::numeric_limits<uint64_t>::max());
    return max_key;
  }
  static Key min() {
    static Key min_key(std::numeric_limits<uint64_t>::min());
    return min_key;
  }

  Key() : key(0) {}
  Key(uint64_t key) : key(key) {}
  Key(const Key &other) { key = other.key; }
  Key &operator=(const Key &other) {
    key = other.key;
    return *this;
  }

  model_key_t to_model_key() const {
    model_key_t model_key;
    model_key[0] = key;
    return model_key;
  }

  friend bool operator<(const Key &l, const Key &r) { return l.key < r.key; }
  friend bool operator>(const Key &l, const Key &r) { return l.key > r.key; }
  friend bool operator>=(const Key &l, const Key &r) { return l.key >= r.key; }
  friend bool operator<=(const Key &l, const Key &r) { return l.key <= r.key; }
  friend bool operator==(const Key &l, const Key &r) { return l.key == r.key; }
  friend bool operator!=(const Key &l, const Key &r) { return l.key != r.key; }

  uint64_t key;
} PACKED;

int main(int argc, char **argv) {
  parse_args(argc, argv);
  prepare();
  save();
}

inline void parse_args(int argc, char **argv) {
  struct option long_options[] = {
      {"insert", required_argument, 0, 'b'},
      {"table-size", required_argument, 0, 'f'},
      {0, 0, 0, 0}};
  std::string ops = "b:f:";
  int option_index = 0;

  while (1) {
    int c = getopt_long(argc, argv, ops.c_str(), long_options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 0:
        if (long_options[option_index].flag != 0) break;
        abort();
        break;
      case 'b':
        insert_ratio = strtod(optarg, NULL);
        INVARIANT(insert_ratio >= 0 && insert_ratio <= 1);
        break;
      case 'f':
        table_size = strtoul(optarg, NULL, 10);
        INVARIANT(table_size > 0);
        break;
      default:
        abort();
    }
  }

  COUT_THIS("[prepare] Table size:Insert ratio = " << table_size << ":" << insert_ratio)
}

inline void prepare() {
  // prepare data
  //std::random_device rd;
  //std::mt19937 gen(rd());
  std::mt19937 gen(0);
  std::uniform_int_distribution<int64_t> rand_int64(
      0, std::numeric_limits<int64_t>::max());

  exist_keys.reserve(table_size);
  for (size_t i = 0; i < table_size; ++i) {
    exist_keys.push_back(index_key_t(rand_int64(gen)));
  }

  if (insert_ratio > 0) {
    non_exist_keys.reserve(table_size);
    for (size_t i = 0; i < table_size; ++i) {
      non_exist_keys.push_back(index_key_t(rand_int64(gen)));
    }
  }

  std::sort(exist_keys.begin(), exist_keys.end());

  COUT_VAR(exist_keys.size());
  COUT_VAR(exist_keys[0].key);
  COUT_VAR(non_exist_keys.size());
  if (insert_ratio > 0) {
	COUT_VAR(non_exist_keys[0].key);
  }
}

void save() {
	std::ofstream fd("exist_keys.out", std::ios::out | std::ios::trunc | std::ios::binary);
	INVARIANT(fd);
	for (size_t exist_i = 0; exist_i < exist_keys.size(); exist_i++) {
		fd.write((char *)&exist_keys[exist_i], sizeof(index_key_t));
	}
	fd.close();
	COUT_THIS("[prepare] Save exist keys into exist_keys.out")

	fd.open("nonexist_keys.out", std::ios::out | std::ios::trunc | std::ios::binary);
	INVARIANT(fd);
	for (size_t nonexist_i = 0; nonexist_i < non_exist_keys.size(); nonexist_i++) {
		fd.write((char *)&non_exist_keys[nonexist_i], sizeof(index_key_t));
	}
	fd.close();
	COUT_THIS("[prepare] Save nonexist keys into nonexist_keys.out")
}
