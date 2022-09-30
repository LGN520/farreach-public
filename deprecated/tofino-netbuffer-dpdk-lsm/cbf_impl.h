#include "cbf.h"

template<class key_t>
CBF<key_t>::CBF(uint32_t bytes_num, uint32_t hash_num) {
	INVARIANT(hash_num <= 6);
	this->width = bytes_num / sizeof(bucket_t);
	this->hash_num = hash_num;

	sketch.resize(width, 0);
}

template<class key_t>
void CBF<key_t>::update(const key_t &key) {
	for (uint32_t i = 0; i < hash_num; i++) {
		uint32_t hashidx = hash(key, i);
		if (unlikely(sketch[hashidx] == MAX_BUCKET_VALUE)) {
			COUT_N_EXIT("Integer overflow in CBF!");
		}
		sketch[hashidx] += 1;
	}
}

template<class key_t>
void CBF<key_t>::remove(const key_t &key) {
	for (uint32_t i = 0; i < hash_num; i++) {
		uint32_t hashidx = hash(key, i);
		if (unlikely(sketch[hashidx] == 0)) {
			COUT_N_EXIT("Integer underflow in CBF!");
		}
		sketch[hashidx] -= 1;
	}
}

template<class key_t>
uint32_t CBF<key_t>::query(const key_t &key) const {
	uint32_t min = 0;
	for (uint32_t i = 0; i < hash_num; i++) {
		uint32_t hashidx = hash(key, i);
		if (i == 0 || sketch[hashidx] < min) {
			min = sketch[hashidx];
		}
	}
	return min;
}

template<class key_t>
uint32_t CBF<key_t>::hash(const key_t &key, uint32_t seed) const {
	uint64_t result = ((a_ * uint64_t(key.to_int()) + b_) % p_list_[seed]) % width;
	return uint32_t(result);
}
