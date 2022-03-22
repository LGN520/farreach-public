#include "val.h"
#include <sstream>

uint32_t Val::MAX_VALLEN = 128;
uint32_t Val::SWITCH_MAX_VALLEN = 256;

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

Val::Val(const char* buf, uint32_t length) {
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

Val& Val::operator=(const volatile Val &other) {
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

volatile Val& Val::operator=(const Val &other) volatile {
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

volatile Val& Val::operator=(const volatile Val &other) volatile {
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

bool Val::operator==(const Val &other) {
	if (val_length == 0 && val_data == nullptr && \
			other.val_length == 0 && other.val_data == nullptr) {
		return true;
	}
	if (val_length == other.val_length && val_data != nullptr && other.val_data != nullptr) {
		for (size_t i = 0; i < val_length; i++) {
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

		val_length = uint32_t(slice.size_);
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
}
*/

void Val::from_string(std::string& str) {
	INVARIANT(str.length() <= MAX_VALLEN);
	if (str.data() != nullptr && str.length() != 0) {
		// Deep copy
		val_data = new char[str.length()];
		INVARIANT(val_data != nullptr);
		memcpy((char *)val_data, str.data(), str.length());

		val_length = uint32_t(str.length());
	}
	else {
		val_length = 0;
		val_data = nullptr;
	}
}

std::string Val::to_string() const { // For print
	std::stringstream ss;
	for (uint32_t i = 0; i < val_length; i++) {
		ss << uint8_t(val_data[i]);
		if (i != val_length-1) {
			ss << ":";
		}
	}
	return ss.str();
}

uint32_t Val::get_bytesnum() const {
	return val_length;
}

uint32_t Val::deserialize(const char *buf, uint32_t buflen) {
	INVARIANT(buf != nullptr);

	uint32_t vallen_bytes = deserialize_vallen(buf, buflen); // sizeof(vallen)
	buf += vallen_bytes;
	buflen -= vallen_bytes;
	uint32_t val_deserialize_size = deserialize_val(buf, buflen); // vallen + [padding size]

	return vallen_bytes + val_deserialize_size; // sizeof(vallen) + vallen + [padding size]
}

uint32_t Val::deserialize_vallen(const char *buf, uint32_t buflen) {
	INVARIANT(buf != nullptr);

	INVARIANT(buflen >= sizeof(uint32_t));
	val_length = *(const uint32_t*)buf; // # of bytes
	val_length = ntohl(val_length); // Big-endian to little-endian
	INVARIANT(val_length <= MAX_VALLEN);
	return sizeof(uint32_t); // sizeof(vallen)
}

uint32_t Val::deserialize_val(const char *buf, uint32_t buflen) {
	INVARIANT(buf != nullptr);

	uint32_t padding_size = 0;
	uint32_t deserialize_size = val_length;
	if (val_length <= SWITCH_MAX_VALLEN) {
		if (val_length % 8 != 0) {
			padding_size = 8 - val_length % 8;
		}
		deserialize_size += padding_size; // padding for value <= 128B 
	}
	INVARIANT(buflen >= deserialize_size);

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

	uint32_t padding_size = 0;
	uint32_t serialize_size = sizeof(uint32_t) + val_length;
	if (val_length <= SWITCH_MAX_VALLEN) {
		if (val_length % 8 != 0) {
			padding_size = 8 - val_length % 8;
		}
		serialize_size += padding_size; // padding for value <= 128B 
	}
	INVARIANT(buflen >= serialize_size);

	uint32_t bigendian_vallen = htonl(val_length); // Little-endian to big-endian
	memcpy(buf, (char *)&bigendian_vallen, sizeof(uint32_t)); // Switch needs to use vallen
	memcpy(buf + sizeof(uint32_t), val_data, val_length);
	if (padding_size > 0) { // vallen <= 128B and vallen % 8 != 0
		memset(buf + sizeof(uint32_t) + val_length, 0, padding_size);
	}
	return serialize_size; // sizeof(vallen) + vallen + [padding size]
}

