#ifndef SPECIAL_CASE_H
#define SPECIAL_CASE_H

#include <map>
#include <stdint.h>

#include "helper.h"
#include "key.h"
#include "val.h"

class SpecialCase {
	public:
		SpecialCase();

		Key _key;
		Val _val;
		int32_t _seq;
		bool _valid; // If valid = true, overwrite backup with this kv; otherwise, delete backup item if key exists
};

#endif
