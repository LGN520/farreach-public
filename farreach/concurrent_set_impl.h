#ifndef CONCURRENT_SET_IMPL_H
#define CONCURRENT_SET_IMPL_H

#include "concurrent_set.h"

template<class key_t>
ConcurrentSet<key_t>::ConcurrentSet() {
	_set.clear();
}

template<class key_t>
bool ConcurrentSet<key_t>::is_exist(key_t key) {
	_mutex.lock();
	bool result = (_set.find(key) != _set.end());
	_mutex.unlock();
	return result;
}

template<class key_t>
void ConcurrentSet<key_t>::insert(key_t key) {
	_mutex.lock();
	_set.insert(key);
	_mutex.unlock();
	return;
}

template<class key_t>
void ConcurrentSet<key_t>::erase(key_t key) {
	_mutex.lock();
	_set.erase(key);
	_mutex.unlock();
	return;
}

#endif
