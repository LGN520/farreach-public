/*
 * Wrapper of Redis with key-value operation and snapshot.
 */

#include "redis_wrapper.h"

// max uint64_t: 18446744073709551615
#define MAX_UINT64_STRLEN = 20
// SET key value
// GET key
// DEL key
// NOTE: Redis does not support sorted map
#define MAX_COMMAND_STRLEN = 16

template <class key_t, class val_t>
RedisWrapper<key_t, val_t>::RedisWrapper(const char *hostname, int port, size_t worker_num) {
	redis_conns = new redisConext*[worker_num];
	for (size_t i = 0; i < worker_num; i++) {
		redis_conns[i] = redisConnect(hostname, port);
		if (redis_conns[i] == NULL || redis_conns[i]->err) {
			if (redis_conns[i]) {
				printf("Error: %s\n", redis_conns[i]->errstr);
				for (size_t j = 0; j <= i; j++) {
					redisFree(redis_conns[j]);
				}
				exit(-1);
			}
			else {
				printf("Error: cannot connect redis for worker %lld in %s:%d\n", worker_num, hostname, port);
				exit(-1);
			}
		}
	}
	this->worker_num = worker_num;
	// value + key + command
	max_command_strlen = val_t::MAX_VAL_LENGTH * MAX_UINT64_STRLEN + 2 * MAX_UINT64_STRLEN + MAX_COMMAND_STRLEN;
}

template <class key_t, class val_t>
RedisWrapper<key_t, val_t>::RedisWrapper(const char *hostname, int port, size_t worker_num, const std::vector<key_t> &keys, const std::vector<val_t> &vals) {
	this->RedisWrapper(hostname, port, worker_num);

	// Loading phase
	void *reply = NULL;
	char tmp_command[max_command_strlen];
	for (size_t i = 0; i< keys.size(); i++) {
		sscanf("PUT %s %s", keys[i].to_string().c_str(); vals[i].to_string().c_str());
		reply = redisCommand(redis_conns[0], tmp_command);
		// END HERE
	}
}
