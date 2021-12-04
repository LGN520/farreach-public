#include "cached_val.h"

CachedVal::CachedVal() : _deleted(false), _val(0), _seq(0) {}

CachedVal::CachedVal(bool deleted, val_t val, uint32_t seq) {
	_deleted = deleted;
	_val = val;
	_seq = seq;
}
