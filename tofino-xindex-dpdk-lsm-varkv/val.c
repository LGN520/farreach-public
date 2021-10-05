#include "val.h"
#include <sstream>

Val::Val() {
	val_length = 0;
	val_data = nullptr;
}

Val::~Val() {
	if (val_data != nullptr) {
		delete val_data;
		val_data = nullptr;
	}
}

Val::Val(const uint64_t* buf, uint8_t length) {
	INVARIANT(buf != nullptr);
	INVARIANT(length != 0 && length <= MAX_VAL_LENGTH);

	// Deep copy
	val_data = new uint64_t[length];
	INVARIANT(val_data != nullptr);
	memcpy((char *)val_data, buf, sizeof(uint64_t) * length);

	val_length = length;
}

Val::Val(const Val &other) {
	if (other.val_data != nullptr && other.val_length != 0) {
		// Deep copy
		val_data = new uint64_t[other.val_length];
		INVARIANT(val_data != nullptr);
		memcpy((char *)val_data, other.val_data, sizeof(uint64_t) * other.val_length);

		val_length = other.val_length;
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
}

Val& Val::operator=(const Val &other) {
	if (other.val_data != nullptr && other.val_length != 0) {
		// Deep copy
		val_data = new uint64_t[other.val_length];
		INVARIANT(val_data != nullptr);
		memcpy((char *)val_data, other.val_data, sizeof(uint64_t) * other.val_length);

		val_length = other.val_length;
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
	return *this;
}

rocksdb::Slice Val::to_slice() const {
	return rocksdb::Slice((const char*)val_data, sizeof(uint64_t) * val_length); // NOTE: slice is shallow copy
}

void Val::from_slice(rocksdb::Slice& slice) {
	INVARIANT(slice.size_ % sizeof(uint64_t) == 0);
	INVARIANT(slice.size_ <= MAX_VAL_LENGTH * sizeof(uint64_t));
	if (slice.data_ != nullptr && slice.size_ != 0) {
		// Deep copy
		val_data = new uint64_t[slice.size_/8];
		INVARIANT(val_data != nullptr);
		memcpy((char *)val_data, slice.data_, slice.size_);

		val_length = uint8_t(slice.size_/8);
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
}

void Val::from_string(std::string& str) {
	INVARIANT(str.length() % sizeof(uint64_t) == 0);
	INVARIANT(str.length() <= MAX_VAL_LENGTH * sizeof(uint64_t));
	if (str.data() != nullptr && str.length() != 0) {
		// Deep copy
		val_data = new uint64_t[str.length()/8];
		INVARIANT(val_data != nullptr);
		memcpy((char *)val_data, str.data(), str.length());

		val_length = uint8_t(str.length()/8);
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
}

std::string Val::to_string() const { // For print
	std::stringstream ss;
	for (uint8_t i = 0; i < val_length; i++) {
		ss << val_data[i];
		if (i != val_length-1) {
			ss << ", ";
		}
	}
	return ss.str();
}

uint32_t Val::get_bytesnum() const {
	return val_length * sizeof(uint64_t);
}

uint32_t Val::deserialize(const char *buf) {
	INVARIANT(buf != nullptr);

	val_length = *(const uint8_t*)buf;
	buf += sizeof(uint8_t);
	INVARIANT(val_length <= MAX_VAL_LENGTH);

	if (val_data != nullptr) {
		delete val_data;
	}

	// Deep copy
	val_data = new uint64_t[val_length];
	INVARIANT(val_data != nullptr);
	memcpy((char *)val_data, buf, sizeof(uint64_t) * val_length);
	return sizeof(uint8_t) + sizeof(uint64_t) * val_length;
}

uint32_t Val::serialize(char *buf) {
	memcpy(buf, (char *)&val_length, sizeof(uint8_t));
	memcpy(buf + sizeof(uint8_t), (char *)val_data, sizeof(uint64_t) * val_length);
	return sizeof(uint8_t) + sizeof(uint64_t) * val_length;
}

