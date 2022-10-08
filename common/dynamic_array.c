#include "dynamic_array.h"

DynamicArray::DynamicArray() {
	_cursize = 0;
	_curcapability = 0;
	_array = NULL;

	_mincapability = 0;
	_maxcapability = 0;
}

DynamicArray::DynamicArray(int mincapability, int maxcapability) {
	_cursize = 0;
	init(mincapability, maxcapability);
}

DynamicArray::~DynamicArray() {
	if (_array != NULL) {
		delete [] _array;
		_array = NULL;
	}
}

void DynamicArray::init(int mincapability, int maxcapability) {
	if (maxcapability <= mincapability) {
		printf("[ERROR] DynamicArray::init() maxcapability %d should > mincapability %d\n", maxcapability, mincapability);
		exit(-1);
	}

	_mincapability = mincapability;
	_maxcapability = maxcapability;

	_curcapability = mincapability;
	_array = new char[_curcapability];
	memset(_array, 0, _curcapability);
}

char& DynamicArray::operator[](int idx) {
	if (!(idx >= 0 && idx < _cursize)) {
		printf("[ERROR] DynamicArray::operator[]() invalid idx %d should should in [0, %d)\n", idx, _cursize);
		exit(-1);
	}
	return _array[idx];
}

void DynamicArray::dynamic_memcpy(int off, char *srcarray, int len) {
	dynamic_reserve(off, len);

	memcpy(_array + off, srcarray, len);
	if (off + len > _cursize) {
		_cursize = off + len;
	}
}

void DynamicArray::dynamic_memset(int off, int value, int len) {
	dynamic_reserve(off, len);

	memset(_array + off, value, len);
	if (off + len > _cursize) {
		_cursize = off + len;
	}
}

void DynamicArray::clear() {
	if (_curcapability != _mincapability) {
		if (_array == NULL) {
			printf("[ERROR] DynamicArray::clear() _array is NULL!\n");
			exit(-1);
		}
		delete [] _array;
		_array = new char[_mincapability];
		_curcapability = _mincapability;
	}
	memset(_array, 0, _mincapability);
	_cursize = 0;
}

int DynamicArray::size() const {
	return _cursize;
}

char *DynamicArray::array() const {
	return _array;
}

void DynamicArray::dynamic_reserve(int off, int len) {
	if (off + len > _maxcapability) {
		printf("[DynamicArray] off %d + len %d exceeds maxcapability %d!\n", off, len, _maxcapability);
		exit(-1);
	}

	// [off, off+len)
	if (off + len > _curcapability) {
		// NOTE: _curcapability < off + len <= _maxcapability
		if (_curcapability >= _maxcapability) {
			printf("[ERROR] DynamicArray::dynamic_reserve() _curcapability %d should < _maxcapability %d\n", _curcapability, _maxcapability);
			exit(-1);
		}

		INVARIANT(_curcapability > 0 && _maxcapcbility > 0);
		int newcapability = 2*_curcapability;
		while (newcapability < off + len) {
			newcapability = 2*newcapability;
		}
		if (newcapability > _maxcapability) {
			newcapability = _maxcapability;
		}

		char *newarray = new char[newcapability];
		memset(newarray, 0, newcapability);
		memcpy(newarray, _array, _curcapability);
		delete [] _array;
		_array = newarray;
		newarray = NULL;
		_curcapability = newcapability;
	}
}
