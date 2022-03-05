#include "cbf.h"

Bucket::Bucket() {
	counter = 0;
}

template<class key_t>
CBF<key_t>::CBF(uint32_t bytes_num, uint32_t hash_num) {
	INVARIANT(hash_num <= 6);
	this->width = bytes_num / sizeof(bucket_t);
	this->hash_num = hash_num;

	sketch.resize(width, Bucket());
}

template<class key_t>
void CBF<key_t>::update_nolock(const key_t &key) {
	for (uint32_t i = 0; i < hash_num; i++) {
		uint32_t hashidx = hash(key, i);
		if (unlikely(sketch[hashidx].counter == MAX_COUNTER_VALUE)) {
			COUT_N_EXIT("Integer overflow in CBF!");
		}
		sketch[hashidx].counter += 1;
	}
}

template<class key_t>
void CBF<key_t>::remove(const key_t &key) {
	for (uint32_t i = 0; i < hash_num; i++) {
		uint32_t hashidx = hash(key, i);
		if (unlikely(sketch[hashidx].counter == 0)) {
			COUT_N_EXIT("Integer underflow in CBF!");
		}
		while (true) {
			if (sketch[hashidx].rwlock.try_lock()) break;
		}
		sketch[hashidx].counter -= 1;
		sketch[hashidx].rwlock->unlock();
	}
}

template<class key_t>
uint32_t CBF<key_t>::query(const key_t &key) const {
	uint32_t min = 0;
	for (uint32_t i = 0; i < hash_num; i++) {
		uint32_t hashidx = hash(key, i);
		while (true) {
			if (sketch[hashidx].rwlock.try_lock_shared()) break;
		}
		if (i == 0 || sketch[hashidx].counter < min) {
			min = sketch[hashidx].counter;
		}
		sketch[hashidx].rwlock.unlock_shared();
	}
	return min;
}

template<class key_t>
uint32_t CBF<key_t>::hash(const key_t &key, uint32_t seed) const {
	uint64_t result = ((a_ * uint64_t(key.to_int()) + b_) % p_list_[seed]) % width;
	return uint32_t(result);
}
