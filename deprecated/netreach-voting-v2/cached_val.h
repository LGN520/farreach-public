#ifndef CACHED_VAL_H
#define CACHED_VAL_H

#include "helper.h"
#include "val.h"

typedef Val val_t;

class CachedVal {
	public:
		CachedVal();
		CachedVal(uint8_t _status, val_t val, int seq);

		uint8_t _status; // 0: not latest; 1: latest; 2: latest yet deleted
		val_t _val;
		int _seq;
} PACKED;

#endif
