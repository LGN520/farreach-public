#include "snapshot_record.h"

SnapshotRecord::SnapshotRecord() {
	val = val_t();
	seq = 0;
	stat = false;
}

SnapshotRecord::SnapshotRecord(const val_t &tmpval, const uint32_t &tmpseq, const bool &tmpstat) {
	val = tmpval;
	seq = tmpseq;
	stat = tmpstat;
}

SnapshotRecord& SnapshotRecord::operator=(const SnapshotRecord &other) {
	val = other.val;
	seq = other.seq;
	stat = other.stat;
	return *this;
}
