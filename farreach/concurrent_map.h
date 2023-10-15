#ifndef CONCURRENT_MAP_H
#define CONCURRENT_MAP_H

#include "concurrent_val.h"

// concurrent map w/ atomicity and cache consistency between memory and CPU registers
// only used for snapshot by switchos (speical cases) and server (snapshot data)
// val_t can be Val, SpeicalCase, SnapshotRecord, etc.

const uint8_t alt_buf_fanout = 16;
const uint8_t node_capacity = alt_buf_fanout - 1;

template <class key_t, class val_t>
class ConcurrentMap {
  class Node;
  class Internal;
  class Leaf;

  typedef ConcurrentVal<val_t> concurrent_val_t;
  typedef Node node_t;
  typedef Internal internal_t;
  typedef Leaf leaf_t;

  class Node {
   public:
    void lock();
    void unlock();
    bool is_full() { return key_n == node_capacity; };
    int find_first_larger_than(const key_t &key);
    int find_first_larger_than_or_equal_to(const key_t &key);
    void move_keys_backward(int begin, int end, int off);
    void move_keys_backward(int begin, int off);
    void copy_keys(int begin, int end, key_t *dst);
    void copy_keys(int begin, key_t *dst);

   public:
    bool is_leaf;
    volatile uint8_t locked = 0;
    uint8_t key_n;
    volatile uint64_t version = 0;
    key_t keys[node_capacity];
    Internal *volatile parent = nullptr;
  };

  class Internal : public Node {
   public:
    using Node::is_leaf;
    using Node::key_n;
    using Node::keys;
    using Node::locked;
    using Node::parent;
    using Node::version;

   public:
    Node *find_child(const key_t &key);
    void move_children_backward(int begin, int end, int off);
    void move_children_backward(int begin, int off);
    void copy_children(int begin, int end, Node **dst);
    void copy_children(int begin, Node **dst);

   public:
    Node *children[alt_buf_fanout];
  };

  class Leaf : public Node {
   public:
    using Node::is_leaf;
    using Node::key_n;
    using Node::keys;
    using Node::locked;
    using Node::parent;
    using Node::version;

   public:
    void move_vals_backward(int begin, int end, int off);
    void move_vals_backward(int begin, int off);
    void copy_vals(int begin, int end, concurrent_val_t *dst);
    void copy_vals(int begin, concurrent_val_t *dst);

   public:
    concurrent_val_t vals[node_capacity];
    Leaf *next;
  };

 public:

  // Only used for range query
  struct DataSource {
    DataSource(key_t begin, ConcurrentMap *buffer);
    void advance_to_next_valid();
    const key_t &get_key();
    const val_t &get_val();

    leaf_t *next = nullptr;
    bool has_next = false;
    int pos = 0, n = 0;
    key_t keys[node_capacity];
    val_t vals[node_capacity];
  };

  ConcurrentMap();
  ~ConcurrentMap();

  inline bool get(const key_t &key, val_t &val);
  inline bool update(const key_t &key, const val_t &val);
  inline void insert(const key_t &key, const val_t &val);
  inline bool remove(const key_t &key);
  inline size_t scan(const key_t &key_begin, const size_t n,
                     std::vector<std::pair<key_t, val_t>> &result);
  inline void range_scan(const key_t &key_begin, const key_t &key_end,
                         std::vector<std::pair<key_t, val_t>> &result);

  inline uint32_t size();

 private:
  leaf_t *locate_leaf(key_t key, uint64_t &version);
  leaf_t *locate_leaf_locked(key_t key);

  void insert_leaf(const key_t &key, const val_t &val, leaf_t *target);
  void split_n_insert_leaf(const key_t &key, const val_t &val, int slot,
                           leaf_t *target);

  inline void allocate_new_block();
  inline uint8_t *allocate_node();
  inline internal_t *allocate_internal();
  inline leaf_t *allocate_leaf();
  inline size_t available_node_index();

  node_t *root = nullptr;
  leaf_t *begin = nullptr;
  std::atomic<uint32_t> size_est;
  std::mutex alloc_mut;
  std::vector<uint8_t *> allocated_blocks;
  size_t next_node_i = 0;
  static const size_t node_n_per_block = alt_buf_fanout + 1;
  static const size_t node_size = (sizeof(leaf_t) >= sizeof(internal_t)) ? sizeof(leaf_t) : sizeof(internal_t); // for compilation in switchos
  //static const size_t node_size = std::max(sizeof(leaf_t), sizeof(internal_t));
};

#endif
