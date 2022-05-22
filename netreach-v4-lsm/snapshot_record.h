#ifndef SNAPSHOT_RECORD_H
#define SNAPSHOT_RECORD_H

#include "helper.h"
#include "val.h"

class SnapshotRecord {
	public:
		SnapshotRecord();
		SnapshotRecord(const val_t &tmpval, const uint32_t &tmpseq, const bool &tmpstat);

		SnapshotRecord &operator=(const SnapshotRecord &other);

		val_t val;
		uint32_t seq;
		bool stat;
};
typedef SnapshotRecord snapshot_record_t;

#endif
