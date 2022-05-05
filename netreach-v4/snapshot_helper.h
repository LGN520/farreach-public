#ifndef SNAPSHOT_HELPER_H
#define SNAPSHOT_HELPER_H

#include "helper.h"

#ifdef ORIGINAL_XINDEX
typedef uint64_t val_t;
#else
#include "val.h"
typedef Val val_t;
#endif

struct SnapshotRecord {
	val_t val;
	int32_t seq;
	bool stat;
};
typedef SnapshotRecord snapshot_record_t;

#endif
