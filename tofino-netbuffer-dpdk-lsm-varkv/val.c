#include "val.h"
#include "helper.h"

Val::Val() {
	val_length = 0;
	val_data = nullptr;
}

Vla::~Val() {
	if (val_data != nullptr) {
		delete val_data;
		val_data = nullptr;
	}
}

Val::Val(const char* buf, uint8_t length) {
	INVARIANT(buf != nullptr);
	INVARIANT(length != 0 && length <= MAX_VAL_LENGTH);

	// Deep copy
	val_data = new char[length];
	INVARIANT(val_data != nullptr);
	memcpy(val_data, buf, length);

	val_length = length;
}

Val::Val(const Val &other) {
	if (other.val_data != nullptr && other.val_length != 0) {
		// Deep copy
		val_data = new char[other.val_length];
		INVARIANT(val_data != nullptr);
		memcpy(val_data, other.val_data, other.val_length);

		val_length = other.val_length;
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
}

Val& Val::operator=(const Val &other) const {
	if (other.val_data != nullptr && other.val_length != 0) {
		// Deep copy
		val_data = new char[other.val_length];
		INVARIANT(val_data != nullptr);
		memcpy(val_data, other.val_data, other.val_length);

		val_length = other.val_length;
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
	return *this;
}

rocksdb::Slice Val::to_slice() const {
	return rocksdb::Slice(val_data, val_length); // NOTE: slice is shallow copy
}

void Val::from_slice(rocksdb::Slice& slice) {
	INVARIANT(slice.size_ < MAX_VAL_LENGTH);
	if (slice.data_ != nullptr && slice.size_ != 0) {
		// Deep copy
		val_data = new char[slice.size_];
		INVARIANT(val_data != nullptr);
		memcpy(val_data, slice.data_, slice.size_);

		val_length = uint8_t(slice.size_);
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
}

std::string Val::to_string() const {
	return std::string(val_data, val_length);
}
