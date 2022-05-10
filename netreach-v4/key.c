#include "key.h"
#include <sstream>
#include <arpa/inet.h> // endian conversion

Key Key::max() {
#ifdef LARGE_KEY
    static Key max_key(std::numeric_limits<uint32_t>::max(),
			std::numeric_limits<uint32_t>::max(),
			std::numeric_limits<uint32_t>::max(),
			std::numeric_limits<uint32_t>::max());
#else
    static Key max_key(std::numeric_limits<uint32_t>::max(),
			std::numeric_limits<uint32_t>::max());
#endif
    return max_key;
}

Key Key::min() {
#ifdef LARGE_KEY
    static Key min_key(std::numeric_limits<uint32_t>::min(),
			std::numeric_limits<uint32_t>::min(),
			std::numeric_limits<uint32_t>::min(),
			std::numeric_limits<uint32_t>::min());
#else
    static Key min_key(std::numeric_limits<uint32_t>::min(),
			std::numeric_limits<uint32_t>::min());
#endif
    return min_key;
}

Key::Key() {
#ifdef LARGE_KEY
	keylolo = 0;
	keylohi = 0;
	keyhilo = 0;
	keyhihi = 0;
#else
	keylo = 0;
	keyhi = 0;
#endif
}

#ifdef LARGE_KEY
Key::Key(uint32_t keylolo, uint32_t keylohi, uint32_t keyhilo, uint32_t keyhihi) {
	this->keylolo = keylolo;
	this->keylohi = keylohi;
	this->keyhilo = keyhilo;
	this->keyhihi = keyhihi;
}
#else
Key::Key(uint32_t keylo, uint32_t keyhi) {
	this->keylo = keylo;
	this->keyhi = keyhi;
}
#endif

Key::Key(const Key &other) {
#ifdef LARGE_KEY
	this->keylolo = other.keylolo;
	this->keylohi = other.keylohi;
	this->keyhilo = other.keyhilo;
	this->keyhihi = other.keyhihi;
#else
	keylo = other.keylo;
	keyhi = other.keyhi;
#endif
}

Key::Key(const Key volatile &other) {
	memory_fence();
#ifdef LARGE_KEY
	this->keylolo = other.keylolo;
	this->keylohi = other.keylohi;
	this->keyhilo = other.keyhilo;
	this->keyhihi = other.keyhihi;
#else
	keylo = other.keylo;
	keyhi = other.keyhi;
#endif
}

Key& Key::operator=(const Key &other) {
#ifdef LARGE_KEY
	this->keylolo = other.keylolo;
	this->keylohi = other.keylohi;
	this->keyhilo = other.keyhilo;
	this->keyhihi = other.keyhihi;
#else
	keylo = other.keylo;
	keyhi = other.keyhi;
#endif
	return *this;
}

Key& Key::operator=(const Key volatile &other) {
	memory_fence();
#ifdef LARGE_KEY
	this->keylolo = other.keylolo;
	this->keylohi = other.keylohi;
	this->keyhilo = other.keyhilo;
	this->keyhihi = other.keyhihi;
#else
	keylo = other.keylo;
	keyhi = other.keyhi;
#endif
	return *this;
}

void Key::operator=(const Key &other) volatile {
#ifdef LARGE_KEY
	this->keylolo = other.keylolo;
	this->keylohi = other.keylohi;
	this->keyhilo = other.keyhilo;
	this->keyhihi = other.keyhihi;
#else
	keylo = other.keylo;
	keyhi = other.keyhi;
#endif
	memory_fence();
}

/*
rocksdb::Slice Key::to_slice() const {
#ifdef LARGE_KEY
	// NOTE: keylo-keyhi, in rocksdb/include/slice.h, we compare two slices from end to beginning
	// NOTE: slice is a shallow copy, where we cannot give slice a local varialbe!!!
	// NOTE: addr of this = addr of keylo due to packed
	rocksdb::Slice result((char *)this, 16); 
#else
	rocksdb::Slice result((char *)(&key), 8); // convert int64_t to char[8]
#endif
	return result;
}

void Key::from_slice(rocksdb::Slice& slice) {
#ifdef LARGE_KEY
	keylolo = *(uint32_t*)slice.data_;
	keylohi = *(uint32_t*)(slice.data_+4);
	keyhilo = *(uint32_t*)(slice.data_+8);
	keyhihi = *(uint32_t*)(slice.data_+12);
#else
	keylo = *(uint32_t*)slice.data_;
	keyhi = *(uint32_t*)(slice.data_+4);
#endif
}
*/

Key::model_key_t Key::to_model_key() const {
    Key::model_key_t model_key;
#ifdef LARGE_KEY
	model_key[0] = keylolo;
	model_key[1] = keylohi;
	model_key[2] = keyhilo;
	model_key[3] = keyhihi; // highest priority
#else
    model_key[0] = keylo;
	model_key[1] = keyhi; // higher priority	
#endif
    return model_key;
}

int64_t Key::to_int() const {
#ifdef LARGE_KEY
	return ((int64_t(keylohi)<<32) | int64_t(keylolo)) ^ ((int64_t(keyhihi)<<32) | int64_t(keyhilo));
#else
	return (int64_t(keyhi)<<32) | int64_t(keylo);
#endif
}

std::string Key::to_string() const {
	std::stringstream ss;
#ifdef LARGE_KEY
	ss << keyhihi << "," << keyhilo << "," << keylohi << "," << keylolo;
#else
	ss << keyhi << "," << keylo;
#endif
	return ss.str();
}

bool operator<(const Key &l, const Key &r) { 
#ifdef LARGE_KEY
	return (l.keyhihi < r.keyhihi) || ((l.keyhihi == r.keyhihi) && (l.keyhilo < r.keyhilo)) || 
		((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi < r.keylohi)) ||
		((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi == r.keylohi) && (l.keylolo < r.keylolo));
#else
	return (l.keyhi < r.keyhi) || ((l.keyhi == r.keyhi) && (l.keylo < r.keylo));
#endif
}

bool operator>(const Key &l, const Key &r) {
#ifdef LARGE_KEY
	return (l.keyhihi > r.keyhihi) || ((l.keyhihi == r.keyhihi) && (l.keyhilo > r.keyhilo)) || 
		((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi > r.keylohi)) ||
		((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi == r.keylohi) && (l.keylolo > r.keylolo));
#else
	return (l.keyhi > r.keyhi) || ((l.keyhi == r.keyhi) && (l.keylo > r.keylo));
#endif
}

bool operator>=(const Key &l, const Key &r) {
#ifdef LARGE_KEY
	return (l.keyhihi > r.keyhihi) || ((l.keyhihi == r.keyhihi) && (l.keyhilo > r.keyhilo)) || 
		((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi > r.keylohi)) ||
		((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi == r.keylohi) && (l.keylolo >= r.keylolo));
#else
	return (l.keyhi > r.keyhi) || ((l.keyhi == r.keyhi) && (l.keylo >= r.keylo));
#endif
}

bool operator<=(const Key &l, const Key &r) {
#ifdef LARGE_KEY
	return (l.keyhihi < r.keyhihi) || ((l.keyhihi == r.keyhihi) && (l.keyhilo < r.keyhilo)) || 
		((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi < r.keylohi)) ||
		((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi == r.keylohi) && (l.keylolo <= r.keylolo));
#else
	return (l.keyhi < r.keyhi) || ((l.keyhi == r.keyhi) && (l.keylo <= r.keylo));
#endif
}

bool operator==(const Key &l, const Key &r) {
#ifdef LARGE_KEY
	return (l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi == r.keylohi) && (l.keylolo == r.keylolo);
#else
	return (l.keyhi == r.keyhi) && (l.keylo == r.keylo);
#endif
}

bool operator!=(const Key &l, const Key &r) { 
#ifdef LARGE_KEY
	return (l.keyhihi != r.keyhihi) || (l.keyhilo != r.keyhilo) || (l.keylohi != r.keylohi) || (l.keylolo != r.keylolo);
#else
	return (l.keyhi != r.keyhi) || (l.keylo != r.keylo);
#endif
}

uint32_t Key::deserialize(const char* buf, uint32_t buflen) {
#ifdef LARGE_KEY
	INVARIANT(buf != nullptr && buflen >= 16);
	memcpy((char *)&keylolo, buf, sizeof(uint32_t));
	memcpy((char *)&keylohi, buf+4, sizeof(uint32_t));
	memcpy((char *)&keyhilo, buf+8, sizeof(uint32_t));
	memcpy((char *)&keyhihi, buf+12, sizeof(uint32_t));
	// Big-endian to little-endian
	keylolo = ntohl(keylolo);
	keylohi = ntohl(keylohi);
	keyhilo = ntohl(keyhilo);
	keyhihi = ntohl(keyhihi);
	return 16;
#else
	INVARIANT(buf != nullptr && buflen >= 8);
	memcpy((char *)&keylo, buf, sizeof(uint32_t));
	memcpy((char *)&keyhi, buf+4, sizeof(uint32_t));
	// Big-endian to little-endian
	keylo = ntohl(keylo);
	keyhi = ntohl(keyhi);
	return 8;
#endif
}

uint32_t Key::serialize(char* buf, uint32_t buflen) {
#ifdef LARGE_KEY
	INVARIANT(buf != nullptr && buflen >= 16);
	// Little-endian to big-endian
	uint32_t bigendian_keylolo = htonl(keylolo);
	uint32_t bigendian_keylohi = htonl(keylohi);
	uint32_t bigendian_keyhilo = htonl(keyhilo);
	uint32_t bigendian_keyhihi = htonl(keyhihi);
	memcpy(buf, (char *)&bigendian_keylolo, sizeof(uint32_t));
	memcpy(buf+4, (char *)&bigendian_keylohi, sizeof(uint32_t));
	memcpy(buf+8, (char *)&bigendian_keyhilo, sizeof(uint32_t));
	memcpy(buf+12, (char *)&bigendian_keyhihi, sizeof(uint32_t));
	return 16;
#else
	INVARIANT(buf != nullptr && buflen >= 8);
	// Little-endian to big-endian
	uint32_t bigendian_keylo = htonl(keylo);
	uint32_t bigendian_keyhi = htonl(keyhi);
	memcpy(buf, (char *)&bigendian_keylo, sizeof(uint32_t));
	memcpy(buf+4, (char *)&bigendian_keyhi, sizeof(uint32_t));
	return 8;
#endif
}

uint32_t Key::serialize(char* buf, uint32_t buflen) volatile {
	memory_fence();
#ifdef LARGE_KEY
	INVARIANT(buf != nullptr && buflen >= 16);
	// Little-endian to big-endian
	uint32_t bigendian_keylolo = htonl(keylolo);
	uint32_t bigendian_keylohi = htonl(keylohi);
	uint32_t bigendian_keyhilo = htonl(keyhilo);
	uint32_t bigendian_keyhihi = htonl(keyhihi);
	memcpy(buf, (char *)&bigendian_keylolo, sizeof(uint32_t));
	memcpy(buf+4, (char *)&bigendian_keylohi, sizeof(uint32_t));
	memcpy(buf+8, (char *)&bigendian_keyhilo, sizeof(uint32_t));
	memcpy(buf+12, (char *)&bigendian_keyhihi, sizeof(uint32_t));
	return 16;
#else
	INVARIANT(buf != nullptr && buflen >= 8);
	// Little-endian to big-endian
	uint32_t bigendian_keylo = htonl(keylo);
	uint32_t bigendian_keyhi = htonl(keyhi);
	memcpy(buf, (char *)&bigendian_keylo, sizeof(uint32_t));
	memcpy(buf+4, (char *)&bigendian_keyhi, sizeof(uint32_t));
	return 8;
#endif
}

/*uint32_t Key::size() const {
#ifdef LARGE_KEY
	return 16;
#else
	return 8;
}*/
