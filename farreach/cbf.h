#ifndef CBF_H
#define CBF_H

#include "../common/helper.h"
#include <vector>
#include <boost/thread/shared_mutex.hpp>

#define MAX_COUNTER_VALUE 0xFFFFFFFFFFFFFFFF

class Bucket {
	public:
		uint64_t counter;
		boost::shared_mutex rwlock; // fine-grained control

		Bucket();
		Bucket(const Bucket &other);
		Bucket &operator=(const Bucket &other);
};

template<class key_t>
class CBF {
	typedef Bucket bucket_t;

	public:
		CBF(uint32_t bytes_num, uint32_t hash_num = 3);
		void update_nolock(const key_t &key);
		void remove(const key_t &key);
		uint32_t query(const key_t &key);
	private:
		uint64_t a_ = 101;
		uint64_t b_ = 1009;
		uint64_t p_list_[6] = {1073741827, 1073741831, 1073741833, 1073741839, 1073741843, 1073741857}; // the first 6 primes > 2^30
		uint32_t hash(const key_t &key, uint32_t seed) const;

		uint32_t width = 0;
		uint32_t hash_num = 0;
		std::vector<bucket_t> sketch;
};


#endif
