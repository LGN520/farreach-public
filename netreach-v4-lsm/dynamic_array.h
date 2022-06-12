#ifndef DYNAMIC_ARRAY
#define DYNAMIC_ARRAY

// We use global rebuilding to allocate space for array dynamically
// Compared with allocating a large array statically, dynamic array is more memory efficient
// Besides, the amortized time cost is still O(1) and space cost is still O(n) based on charging argument or potential method

#include <stdint.h>

#include "helper.h"

class DynamicArray {
	public:
		DynamicArray();
		DynamicArray(int mincapability, int maxcapability);
		~DynamicArray();

		void init(int mincapability, int maxcapability);
		char& operator[](int idx);
		// copy len bytes from srcarray to _array + off
		void memcpy(int off, char *srcarray, int len);

		int size() const;
		char *array() const;
	private:
		int _cursize = 0;
		int _curcapability = 0;
		int _maxcapability = 0;
		char *_array = NULL;
};

typedef DynamicArray dynamic_array_t;

#endif
