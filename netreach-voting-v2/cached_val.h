#ifndef CACHED_VAL_H
#define CACHED_VAL_H

#include "helper.h"
#include "val.h"

class CachedVal {
	public:
		CachedVal();
		CachedVal(uint8_t _status, val_t val, uint32_t seq);

		uint8_t _status; // 0: not latest; 1: latest; 2: latest yet deleted
		val_t _val;
		uint32_t _seq;
} PACKED;

#endif
