#include "key.h"
#include <sstream>
#include <arpa/inet.h> // endian conversion

/*#ifdef LARGE_KEY
uint32_t Key::KEYLEN = 16;
#else
uint32_t Key::KEYLEN = 8;
#endif*/

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

/*rocksdb::Slice Key::to_slice() const {
#ifdef LARGE_KEY
	// NOTE: slice is a shallow copy, where we cannot give slice a local variable!!! (yet UserKey -> InternalKey in rocksdb is a deep copy)
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
}*/

// NOTE: to match default ByteWiseComparator (aka Slice::compare in rocksdb/include/slice.h), we convert key into slice from the most significant byte to the least significant byte
// NOTE: ByteWiseComparator uses memcmp which does not care about per-byte signedness (treat each byte as uint8_t)
std::string Key::to_string_for_rocksdb() const {
#ifdef LARGE_KEY
	uint16_t keyhihilo = uint16_t(keyhihi & 0xFFFF);
	uint16_t keyhihihi = uint16_t((keyhihi >> 16) & 0xFFFF);
	uint8_t data[16];
	data[0] = uint8_t((keyhihihi >> 8) & 0xFF);
	data[1] = uint8_t(keyhihihi & 0xFF);
	data[2] = uint8_t((keyhihilo >> 8) & 0xFF);
	data[3] = uint8_t(keyhihilo & 0xFF);
	data[4] = uint8_t((keyhilo >> 24) & 0xFF);
	data[5] = uint8_t((keyhilo >> 16) & 0xFF);
	data[6] = uint8_t((keyhilo >> 8) & 0xFF);
	data[7] = uint8_t(keyhilo & 0xFF);
	data[8] = uint8_t((keylohi >> 24) & 0xFF);
	data[9] = uint8_t((keylohi >> 16) & 0xFF);
	data[10] = uint8_t((keylohi >> 8) & 0xFF);
	data[11] = uint8_t(keylohi & 0xFF);
	data[12] = uint8_t((keylolo >> 24) & 0xFF);
	data[13] = uint8_t((keylolo >> 16) & 0xFF);
	data[14] = uint8_t((keylolo >> 8) & 0xFF);
	data[15] = uint8_t(keylolo & 0xFF);
	std::string result((char *)data, 16);
#else
	uint16_t keyhilo = uint16_t(keyhi & 0xFFFF);
	uint16_t keyhihi = uint16_t((keyhi >> 16) & 0xFFFF);
	uint8_t data[8];
	data[0] = uint8_t((keyhihi >> 8) & 0xFF);
	data[1] = uint8_t(keyhihi & 0xFF);
	data[2] = uint8_t((keyhilo >> 8) & 0xFF);
	data[3] = uint8_t(keyhilo & 0xFF);
	data[4] = uint8_t((keylo >> 24) & 0xFF);
	data[5] = uint8_t((keylo >> 16) & 0xFF);
	data[6] = uint8_t((keylo >> 8) & 0xFF);
	data[7] = uint8_t(keylo & 0xFF);
	std::string result((char *)data, 8);
#endif
	return result;
}

void Key::from_string_for_rocksdb(std::string& keystr) {
#ifdef LARGE_KEY
	uint16_t keyhihihi = (uint16_t(uint8_t(keystr[0])) << 8) | (uint16_t(uint8_t(keystr[1])));
	uint16_t keyhihilo = (uint16_t(uint8_t(keystr[2])) << 8) | (uint16_t(uint8_t(keystr[3])));
	keyhihi = (uint32_t(keyhihihi) << 16) | uint32_t(keyhihilo);
	keyhilo = (uint32_t(uint8_t(keystr[4])) << 24) | (uint32_t(uint8_t(keystr[5])) << 16) |\
			  (uint32_t(uint8_t(keystr[6])) << 8) | uint32_t(uint8_t(keystr[7]));
	keylohi = (uint32_t(uint8_t(keystr[8])) << 24) | (uint32_t(uint8_t(keystr[9])) << 16) |\
			  (uint32_t(uint8_t(keystr[10])) << 8) | uint32_t(uint8_t(keystr[11]));
	keylolo = (uint32_t(uint8_t(keystr[12])) << 24) | (uint32_t(uint8_t(keystr[13])) << 16) |\
			  (uint32_t(uint8_t(keystr[14])) << 8) | uint32_t(uint8_t(keystr[15]));
#else
	uint16_t keyhihi = (uint16_t(uint8_t(keystr[0])) << 8) | (uint16_t(uint8_t(keystr[1])));
	uint16_t keyhilo = (uint16_t(uint8_t(keystr[2])) << 8) | (uint16_t(uint8_t(keystr[3])));
	keyhi = (uint32_t(keyhihi) << 16) | uint32_t(keyhilo);
	keylo = (uint32_t(uint8_t(keystr[4])) << 24) | (uint32_t(uint8_t(keystr[5])) << 16) |\
			  (uint32_t(uint8_t(keystr[6])) << 8) | uint32_t(uint8_t(keystr[7]));
#endif
}

/*Key::model_key_t Key::to_model_key_for_train() const {
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
}*/

int64_t Key::to_int_for_hash() const {
#ifdef LARGE_KEY
	return ((int64_t(keylohi)<<32) | int64_t(keylolo)) ^ ((int64_t(keyhihi)<<32) | int64_t(keyhilo));
#else
	return (int64_t(keyhi)<<32) | int64_t(keylo);
#endif
}

std::string Key::to_string_for_print() const {
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
	//memcpy((char *)&keyhihi, buf+12, sizeof(uint32_t));
	uint16_t bigendian_keyhihilo = 0;
	uint16_t bigendian_keyhihihi = 0;
	memcpy((char *)&bigendian_keyhihilo, buf+12, sizeof(uint16_t));
	memcpy((char *)&bigendian_keyhihihi, buf+14, sizeof(uint16_t));
	// Big-endian to little-endian
	keylolo = ntohl(keylolo);
	keylohi = ntohl(keylohi);
	keyhilo = ntohl(keyhilo);
	//keyhihi = ntohl(keyhihi);
	uint16_t keyhihilo = ntohs(bigendian_keyhihilo);
	uint16_t keyhihihi = ntohs(bigendian_keyhihihi);
	keyhihi = (uint32_t(keyhihihi) << 16) | uint32_t(keyhihilo);
	return 16;
#else
	INVARIANT(buf != nullptr && buflen >= 8);
	memcpy((char *)&keylo, buf, sizeof(uint32_t));
	//memcpy((char *)&keyhi, buf+4, sizeof(uint32_t));
	uint16_t bigendian_keyhilo = 0;
	uint16_t bigendian_keyhihi = 0;
	memcpy((char *)&bigendian_keyhilo, buf+4, sizeof(uint16_t));
	memcpy((char *)&bigendian_keyhihi, buf+6, sizeof(uint16_t));
	// Big-endian to little-endian
	keylo = ntohl(keylo);
	//keyhi = ntohl(keyhi);
	uint16_t keyhilo = ntohs(bigendian_keyhilo);
	uint16_t keyhihi = ntohs(bigendian_keyhihi);
	keyhi = (uint32_t(keyhihi) << 16) | uint32_t(keyhilo);
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
	//uint32_t bigendian_keyhihi = htonl(keyhihi);
	uint16_t keyhihilo = uint16_t(keyhihi & 0xFFFF);
	uint16_t keyhihihi = uint16_t((keyhihi >> 16) & 0xFFFF);
	uint16_t bigendian_keyhihilo = htons(keyhihilo);
	uint16_t bigendian_keyhihihi = htons(keyhihihi);
	memcpy(buf, (char *)&bigendian_keylolo, sizeof(uint32_t));
	memcpy(buf+4, (char *)&bigendian_keylohi, sizeof(uint32_t));
	memcpy(buf+8, (char *)&bigendian_keyhilo, sizeof(uint32_t));
	//memcpy(buf+12, (char *)&bigendian_keyhihi, sizeof(uint32_t));
	memcpy(buf+12, (char *)&bigendian_keyhihilo, sizeof(uint16_t));
	memcpy(buf+14, (char *)&bigendian_keyhihihi, sizeof(uint16_t)); // the highest 2 bytes will be used for range matching
	return 16;
#else
	INVARIANT(buf != nullptr && buflen >= 8);
	// Little-endian to big-endian
	uint32_t bigendian_keylo = htonl(keylo);
	//uint32_t bigendian_keyhi = htonl(keyhi);
	uint16_t keyhilo = uint16_t(keyhi & 0xFFFF);
	uint16_t keyhihi = uint16_t((keyhi >> 16) & 0xFFFF);
	uint16_t bigendian_keyhilo = htons(keyhilo);
	uint16_t bigendian_keyhihi = htons(keyhihi);
	memcpy(buf, (char *)&bigendian_keylo, sizeof(uint32_t));
	//memcpy(buf+4, (char *)&bigendian_keyhi, sizeof(uint32_t));
	memcpy(buf+4, (char *)&bigendian_keyhilo, sizeof(uint16_t));
	memcpy(buf+6, (char *)&bigendian_keyhihi, sizeof(uint16_t)); // the highest 2 bytes will be used for range matching
	return 8;
#endif
}

uint32_t Key::serialize(char* buf, uint32_t buflen) const {
#ifdef LARGE_KEY
	INVARIANT(buf != nullptr && buflen >= 16);
	// Little-endian to big-endian
	uint32_t bigendian_keylolo = htonl(keylolo);
	uint32_t bigendian_keylohi = htonl(keylohi);
	uint32_t bigendian_keyhilo = htonl(keyhilo);
	//uint32_t bigendian_keyhihi = htonl(keyhihi);
	uint16_t keyhihilo = uint16_t(keyhihi & 0xFFFF);
	uint16_t keyhihihi = uint16_t((keyhihi >> 16) & 0xFFFF);
	uint16_t bigendian_keyhihilo = htons(keyhihilo);
	uint16_t bigendian_keyhihihi = htons(keyhihihi);
	memcpy(buf, (char *)&bigendian_keylolo, sizeof(uint32_t));
	memcpy(buf+4, (char *)&bigendian_keylohi, sizeof(uint32_t));
	memcpy(buf+8, (char *)&bigendian_keyhilo, sizeof(uint32_t));
	//memcpy(buf+12, (char *)&bigendian_keyhihi, sizeof(uint32_t));
	memcpy(buf+12, (char *)&bigendian_keyhihilo, sizeof(uint16_t));
	memcpy(buf+14, (char *)&bigendian_keyhihihi, sizeof(uint16_t)); // the highest 2 bytes will be used for range matching
	return 16;
#else
	INVARIANT(buf != nullptr && buflen >= 8);
	// Little-endian to big-endian
	uint32_t bigendian_keylo = htonl(keylo);
	//uint32_t bigendian_keyhi = htonl(keyhi);
	uint16_t keyhilo = uint16_t(keyhi & 0xFFFF);
	uint16_t keyhihi = uint16_t((keyhi >> 16) & 0xFFFF);
	uint16_t bigendian_keyhilo = htons(keyhilo);
	uint16_t bigendian_keyhihi = htons(keyhihi);
	memcpy(buf, (char *)&bigendian_keylo, sizeof(uint32_t));
	//memcpy(buf+4, (char *)&bigendian_keyhi, sizeof(uint32_t));
	memcpy(buf+4, (char *)&bigendian_keyhilo, sizeof(uint16_t));
	memcpy(buf+6, (char *)&bigendian_keyhihi, sizeof(uint16_t)); // the highest 2 bytes will be used for range matching
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
	//uint32_t bigendian_keyhihi = htonl(keyhihi);
	uint16_t keyhihilo = uint16_t(keyhihi & 0xFFFF);
	uint16_t keyhihihi = uint16_t((keyhihi >> 16) & 0xFFFF);
	uint16_t bigendian_keyhihilo = htons(keyhihilo);
	uint16_t bigendian_keyhihihi = htons(keyhihihi);
	memcpy(buf, (char *)&bigendian_keylolo, sizeof(uint32_t));
	memcpy(buf+4, (char *)&bigendian_keylohi, sizeof(uint32_t));
	memcpy(buf+8, (char *)&bigendian_keyhilo, sizeof(uint32_t));
	//memcpy(buf+12, (char *)&bigendian_keyhihi, sizeof(uint32_t));
	memcpy(buf+12, (char *)&bigendian_keyhihilo, sizeof(uint16_t));
	memcpy(buf+14, (char *)&bigendian_keyhihihi, sizeof(uint16_t)); // the highest 2 bytes will be used for range matching
	return 16;
#else
	INVARIANT(buf != nullptr && buflen >= 8);
	// Little-endian to big-endian
	uint32_t bigendian_keylo = htonl(keylo);
	//uint32_t bigendian_keyhi = htonl(keyhi);
	uint16_t keyhilo = uint16_t(keyhi & 0xFFFF);
	uint16_t keyhihi = uint16_t((keyhi >> 16) & 0xFFFF);
	uint16_t bigendian_keyhilo = htons(keyhilo);
	uint16_t bigendian_keyhihi = htons(keyhihi);
	memcpy(buf, (char *)&bigendian_keylo, sizeof(uint32_t));
	//memcpy(buf+4, (char *)&bigendian_keyhi, sizeof(uint32_t));
	memcpy(buf+4, (char *)&bigendian_keyhilo, sizeof(uint16_t));
	memcpy(buf+6, (char *)&bigendian_keyhihi, sizeof(uint16_t)); // the highest 2 bytes will be used for range matching
	return 8;
#endif
}
