#include "val.h"
#include <sstream>
#include <arpa/inet.h>

int32_t Val::MAX_VALLEN = 128;
int32_t Val::SWITCH_MAX_VALLEN = 256;

int32_t Val::get_padding_size(int32_t vallen) {
	int32_t padding_size = 0;
	if (vallen <= SWITCH_MAX_VALLEN) {
		if (vallen % 8 != 0) {
			padding_size = 8 - vallen % 8;
		}
	}
	INVARIANT(padding_size >= 0 && padding_size < 8);
	return padding_size;
}

Val::Val() {
	val_length = 0;
	val_data = nullptr;
}

Val::~Val() {
	if (val_data != nullptr) {
		delete [] val_data;
		val_data = nullptr;
	}
}

Val::Val(const char* buf, int32_t length) {
	INVARIANT(buf != nullptr);
	INVARIANT(length >= 0 && length <= MAX_VALLEN);

	// Deep copy
	val_data = new char[length];
	INVARIANT(val_data != nullptr);
	memcpy(val_data, buf, length); // length is # of bytes in buf

	val_length = length;
}

Val::Val(const Val &other) {
	if (other.val_data != nullptr && other.val_length != 0) {
		// Deep copy
		val_data = new char[other.val_length];
		INVARIANT(val_data != nullptr);
		memcpy(val_data, other.val_data, other.val_length); // val_length is # of bytes

		val_length = other.val_length;
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
}

Val::Val(const volatile Val &other) {
	memory_fence();
	if (other.val_data != nullptr && other.val_length != 0) {
		// Deep copy
		val_data = new char[other.val_length];
		INVARIANT(val_data != nullptr);
		memcpy(val_data, other.val_data, other.val_length); // val_length is # of bytes

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
		val_data = new char[other.val_length];
		INVARIANT(val_data != nullptr);
		memcpy(val_data, other.val_data, other.val_length); // val_length is # of bytes

		val_length = other.val_length;
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
	return *this;
}

/*Val& Val::operator=(const volatile Val &other) {
	memory_fence();
	if (other.val_data != nullptr && other.val_length != 0) {
		// Deep copy
		val_data = new char[other.val_length];
		INVARIANT(val_data != nullptr);
		memcpy(val_data, other.val_data, other.val_length); // val_length is # of bytes

		val_length = other.val_length;
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
	return *this;
}*/

void Val::operator=(const Val &other) volatile {
	if (other.val_data != nullptr && other.val_length != 0) {
		// Deep copy
		val_data = new char[other.val_length];
		INVARIANT(val_data != nullptr);
		memcpy(val_data, other.val_data, other.val_length); // val_length is # of bytes

		val_length = other.val_length;
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
	memory_fence();
}

/*void Val::operator=(const volatile Val &other) volatile {
	memory_fence();
	if (other.val_data != nullptr && other.val_length != 0) {
		// Deep copy
		val_data = new char[other.val_length];
		INVARIANT(val_data != nullptr);
		memcpy(val_data, other.val_data, other.val_length); // val_length is # of bytes

		val_length = other.val_length;
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
	memory_fence();
}*/

bool Val::operator==(const Val &other) {
	if (val_length == 0 && val_data == nullptr && \
			other.val_length == 0 && other.val_data == nullptr) {
		return true;
	}
	if (val_length == other.val_length && val_data != nullptr && other.val_data != nullptr) {
		for (int32_t i = 0; i < val_length; i++) {
			if (val_data[i] != other.val_data[i]) {
				return false;
			}
		}
		return true;
	}
	return false;
}

/*
rocksdb::Slice Val::to_slice() const {
	return rocksdb::Slice((const char*)val_data, val_length); // NOTE: slice is shallow copy
}

void Val::from_slice(rocksdb::Slice& slice) {
	INVARIANT(slice.size_ <= MAX_VALLEN);
	if (slice.data_ != nullptr && slice.size_ != 0) {
		// Deep copy
		val_data = new char[slice.size_];
		INVARIANT(val_data != nullptr);
		memcpy((char *)val_data, slice.data_, slice.size_);

		val_length = int32_t(slice.size_);
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
}
*/

void Val::from_string(std::string& str) {
	INVARIANT(int32_t(str.length()) <= MAX_VALLEN);
	if (str.data() != nullptr && str.length() != 0) {
		// Deep copy
		val_data = new char[str.length()];
		INVARIANT(val_data != nullptr);
		memcpy((char *)val_data, str.data(), str.length());

		val_length = int32_t(str.length());
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
}

std::string Val::to_string() const { // For print
	std::stringstream ss;
	for (int32_t i = 0; i < val_length; i++) {
		ss << uint8_t(val_data[i]);
		if (i != val_length-1) {
			ss << ":";
		}
	}
	return ss.str();
}

uint32_t Val::get_bytesnum() const {
	return uint32_t(val_length);
}

uint32_t Val::deserialize(const char *buf, uint32_t buflen) {
	INVARIANT(buf != nullptr);

	uint32_t vallen_bytes = deserialize_vallen(buf, buflen); // sizeof(vallen)
	buf += vallen_bytes;
	buflen -= vallen_bytes;
	uint32_t val_deserialize_size = deserialize_val(buf, buflen); // vallen + [padding size]

	return vallen_bytes + val_deserialize_size; // sizeof(vallen) + vallen + [padding size]
}

uint32_t Val::deserialize(const char *buf, uint32_t buflen) volatile {
	INVARIANT(buf != nullptr);

	uint32_t vallen_bytes = ((Val *)this)->deserialize_vallen(buf, buflen); // sizeof(vallen)
	memory_fence();
	buf += vallen_bytes;
	buflen -= vallen_bytes;
	uint32_t val_deserialize_size = ((Val *)this)->deserialize_val(buf, buflen); // vallen + [padding size]
	memory_fence();

	return vallen_bytes + val_deserialize_size; // sizeof(vallen) + vallen + [padding size]
}

uint32_t Val::deserialize_vallen(const char *buf, uint32_t buflen) {
	INVARIANT(buf != nullptr);

	INVARIANT(buflen >= sizeof(int32_t));
	val_length = *(const int32_t*)buf; // # of bytes
	val_length = int32_t(ntohl(uint32_t(val_length))); // Big-endian to little-endian
	INVARIANT(val_length >= 0 && val_length <= MAX_VALLEN);
	return sizeof(int32_t); // sizeof(vallen)
}

uint32_t Val::deserialize_val(const char *buf, uint32_t buflen) {
	INVARIANT(buf != nullptr);

	int32_t padding_size = get_padding_size(val_length); // padding for value <= 128B 
	int32_t deserialize_size = val_length + padding_size;
	INVARIANT(deserialize_size >= 0 && buflen >= uint32_t(deserialize_size));

	if (val_data != nullptr) {
		delete [] val_data;
	}

	// Deep copy
	val_data = new char[val_length];
	INVARIANT(val_data != nullptr);
	memcpy(val_data, buf, val_length);
	return deserialize_size; // vallen + [padding size]
}

uint32_t Val::serialize(char *buf, uint32_t buflen) {
	INVARIANT(val_data != nullptr);

	int32_t padding_size = get_padding_size(val_length); // padding for value <= 128B 
	int32_t serialize_size = sizeof(int32_t) + val_length + padding_size;
	INVARIANT(serialize_size >= 0 && buflen >= uint32_t(serialize_size));

	int32_t bigendian_vallen = int32_t(htonl(uint32_t(val_length))); // Little-endian to big-endian
	memcpy(buf, (char *)&bigendian_vallen, sizeof(int32_t)); // Switch needs to use vallen
	memcpy(buf + sizeof(int32_t), val_data, val_length);
	if (padding_size > 0) { // vallen <= 128B and vallen % 8 != 0
		memset(buf + sizeof(int32_t) + val_length, 0, padding_size);
	}
	return serialize_size; // sizeof(vallen) + vallen + [padding size]
}

