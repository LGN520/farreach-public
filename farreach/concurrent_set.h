#ifndef CONCURRENT_SET_H
#define CONCURRENT_SET_H

#include <set>
#include <mutex>

template<class key_t>
class ConcurrentSet {
	public:
		ConcurrentSet();

		bool is_exist(key_t key);
		void insert(key_t key);
		void erase(key_t key);
	private:
		std::mutex _mutex;
		std::set<key_t> _set;
};

#endif
