/*
 * Wrapper of Redis with key-value operation and snapshot.
 */

#include "hiredis.h"
#include "helper.h"

template <class key_t, class val_t>
class RedisWrapper {

 public:
  RedisWrapper(const char *hostname, int port, size_t worker_num);
  RedisWrapper(const char *hostname, int port, size_t worker_num, const std::vector<key_t> &keys, const std::vector<val_t> &vals);
  ~RedisWrapper();

  inline bool get(const key_t &key, val_t &val);
  inline bool put(const key_t &key, const val_t &val);
  inline bool remove(const key_t &key);
  inline size_t scan(const key_t &begin, const size_t n,
                     std::vector<std::pair<key_t, val_t>> &result);
  size_t range_scan(const key_t &begin, const key_t &end,
                    std::vector<std::pair<key_t, val_t>> &result);

  void make_snapshot();
  void stop_snapshot();

 private:
  // NOTE: redisContext for each TCP connection is not thread-safe
  // As Redis only supports multi-thread network IO, while memory IO is still single-thread (main thread), we should use multiple redisContext for server-side threads
  redisContext **redis_conns = NULL;
  size_t worker_num;
  size_t max_command_strlen;
  std::atomic_flag is_snapshot = ATOMIC_FLAG_INIT;

};
