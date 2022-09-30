#ifndef CBF_H
#define CBF_H

#include "helper.h"
#include <vector>

typedef uint8_t bucket_t;
#define MAX_BUCKET_VALUE 255

template<class key_t>
class CBF {
	public:
		CBF(uint32_t bytes_num, uint32_t hash_num = 3);
		void update(const key_t &key);
		void remove(const key_t &key);
		uint32_t query(const key_t &key) const;
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
