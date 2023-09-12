#ifndef BF_H
#define BF_H

#include "helper.h"
#include <vector>

template<class key_t>
class BF {
	typedef bool bucket_t;

	public:
		BF(uint32_t bytes_num, uint32_t hash_num = 1);
		void update(const key_t &key);
		bool query(const key_t &key);
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
