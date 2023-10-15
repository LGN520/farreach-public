#ifndef BF_IMPL_H
#define BF_IMPL_H

#include "bf.h"

template<class key_t>
BF<key_t>::BF(uint32_t bytes_num, uint32_t hash_num) {
	INVARIANT(hash_num <= 6);
	this->width = bytes_num / sizeof(bucket_t);
	this->hash_num = hash_num;

	sketch.resize(width, false);
}

template<class key_t>
void BF<key_t>::update(const key_t &key) {
	for (uint32_t i = 0; i < hash_num; i++) {
		uint32_t hashidx = hash(key, i);
		sketch[hashidx] = true;
	}
}

template<class key_t>
bool BF<key_t>::query(const key_t &key) {
	for (uint32_t i = 0; i < hash_num; i++) {
		uint32_t hashidx = hash(key, i);
		if (sketch[hashidx] == false) {
			return false;
		}
	}
	return true;
}

template<class key_t>
uint32_t BF<key_t>::hash(const key_t &key, uint32_t seed) const {
	uint64_t result = ((a_ * uint64_t(key.to_int()) + b_) % p_list_[seed]) % width;
	return uint32_t(result);
}

#endif
