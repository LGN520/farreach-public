#ifndef CACHED_VAL_H
#define CACHED_VAL_H

#include "helper.h"
#include "val.h"

class CachedVal {
	public:
		CachedVal();
		CachedVal(bool deleted, val_t val, uint32_t seq);

		bool _deleted;
		val_t _val;
		uint32_t _seq;
} PACKED;

#endif
