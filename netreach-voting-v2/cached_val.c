#include "cached_val.h"

CachedVal::CachedVal() : _status(0), _val(0), _seq(0) {}

CachedVal::CachedVal(uint8_t status, val_t val, int seq) {
	_status = status;
	_val = val;
	_seq = seq;
}
