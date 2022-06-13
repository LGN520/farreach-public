#ifndef DYNAMIC_ARRAY
#define DYNAMIC_ARRAY

// We use global rebuilding to allocate space for array dynamically
// Compared with allocating a large array statically, dynamic array is more memory efficient
// Besides, the amortized time cost is still O(1) and space cost is still O(n) based on charging argument or potential method

#include <stdint.h>
#include <memory>

#include "helper.h"

class DynamicArray {
	public:
		DynamicArray();
		DynamicArray(int mincapability, int maxcapability);
		~DynamicArray();

		void init(int mincapability, int maxcapability);
		char& operator[](int idx);
		// copy len bytes from srcarray to _array + off
		void dynamic_memcpy(int off, char *srcarray, int len);
		void dynamic_memset(int off, int value, int len);
		void clear();

		int size() const;
		char *array() const;
	private:
		void dynamic_reserve(int off, int len);

		int _cursize = 0;
		int _curcapability = 0;
		char *_array = NULL;

		// only set in constructor or init
		int _mincapability = 0;
		int _maxcapability = 0;
};

typedef DynamicArray dynamic_array_t;

#endif
