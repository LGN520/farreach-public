#include "key.h"
#include <sstream>

Key Key::max() {
#ifdef LARGE_KEY
    static Key max_key(std::numeric_limits<uint64_t>::max(),
			std::numeric_limits<uint64_t>::max());
#else
    static Key max_key(std::numeric_limits<uint64_t>::max());
#endif
    return max_key;
}

Key Key::min() {
#ifdef LARGE_KEY
    static Key min_key(std::numeric_limits<uint64_t>::min(),
			std::numeric_limits<uint64_t>::min());
#else
    static Key min_key(std::numeric_limits<uint64_t>::min());
#endif
    return min_key;
}

Key::Key() {
#ifdef LARGE_KEY
	keylo = 0;
	keyhi = 0;
#else
	key = 0;
#endif
}

#ifdef LARGE_KEY
Key::Key(uint64_t keylo, uint64_t keyhi) {
	this->keylo = keylo;
	this->keyhi = keyhi;
}
#else
Key::Key(uint64_t key) {
	this->key = key;
}
#endif

Key::Key(const Key &other) {
#ifdef LARGE_KEY
	keylo = other.keylo;
	keyhi = other.keyhi;
#else
	key = other.key;
#endif
}

Key& Key::operator=(const Key &other) {
#ifdef LARGE_KEY
	keylo = other.keylo;
	keyhi = other.keyhi;
#else
    key = other.key;
#endif
	return *this;
}


rocksdb::Slice Key::to_slice() const {
#ifdef LARGE_KEY
	// NOTE: keylo-keyhi, in rocksdb/include/slice.h, we compare two slices from end to beginning
	// NOTE: slice is a shallow copy, where we cannot give slice a local varialbe!!!
	// NOTE: addr of this = addr of keylo due to packed
	rocksdb::Slice result((char *)this, 16); 
#else
	rocksdb::Slice result((char *)(&key), 8); // convert uint64_t to char[8]
#endif
	return result;
}

void Key::from_slice(rocksdb::Slice& slice) {
#ifdef LARGE_KEY
	keylo = *(uint64_t*)slice.data_;
	keyhi = *(uint64_t*)(slice.data_+8);
#else
	key = *(uint64_t*)slice.data_;
#endif
}

Key::model_key_t Key::to_model_key() const {
    Key::model_key_t model_key;
#ifdef LARGE_KEY
	model_key[0] = keylo;
	model_key[1] = keyhi;
#else
    model_key[0] = key;
#endif
    return model_key;
}

uint64_t Key::to_int() const {
#ifdef LARGE_KEY
	return keylo ^ keyhi;
#else
	return key;
#endif
}

std::string Key::to_string() const {
	std::stringstream ss;
#ifdef LARGE_KEY
	ss << keylo << "," << keyhi;
#else
	ss << key;
#endif
	return ss.str();
}

uint32_t Key::serialize(char *buf) {
	if (buf != nullptr) {
#ifdef LARGE_KEY
		memcpy(buf, (void *)&keylo, sizeof(uint64_t));
		memcpy(buf + sizeof(uint64_t), (void *)&keyhi, sizeof(uint64_t));
		return 2*sizeof(uint64_t);
#else
		memcpy(buf, (void *)&key, sizeof(uint64_t));
		return sizeof(uint64_t);
#endif
	}
	else {
		return 0;
	}
}

uint32_t Key::deserialize(const char *buf) {
	if (buf != nullptr) {
#ifdef LARGE_KEY
		memcpy((void *)&keylo, buf, sizeof(uint64_t));
		memcpy((void *)&keyhi, buf + sizeof(uint64_t), sizeof(uint64_t));
		return 2*sizeof(uint64_t);
#else
		memcpy((void *)&key, buf, sizeof(uint64_t));
		return sizeof(uint64_t);
#endif
	}
	else {
		return 0;
	}
}

bool operator<(const Key &l, const Key &r) { 
#ifdef LARGE_KEY
	return (l.keyhi < r.keyhi) || ((l.keyhi == r.keyhi) && (l.keylo < r.keylo));
#else
	return l.key < r.key; 
#endif
}

bool operator>(const Key &l, const Key &r) {
#ifdef LARGE_KEY
	return (l.keyhi > r.keyhi) || ((l.keyhi == r.keyhi) && (l.keylo > r.keylo));
#else
	return l.key > r.key; 
#endif
}

bool operator>=(const Key &l, const Key &r) {
#ifdef LARGE_KEY
	return (l.keyhi > r.keyhi) || ((l.keyhi == r.keyhi) && (l.keylo >= r.keylo));
#else
	return l.key >= r.key; 
#endif
}

bool operator<=(const Key &l, const Key &r) {
#ifdef LARGE_KEY
	return (l.keyhi < r.keyhi) || ((l.keyhi == r.keyhi) && (l.keylo <= r.keylo));
#else
	return l.key <= r.key; 
#endif
}

bool operator==(const Key &l, const Key &r) {
#ifdef LARGE_KEY
	return (l.keyhi == r.keyhi) && (l.keylo == r.keylo);
#else
	return l.key == r.key;
#endif
}

bool operator!=(const Key &l, const Key &r) { 
#ifdef LARGE_KEY
	return (l.keyhi != r.keyhi) || (l.keylo != r.keylo);
#else
	return l.key != r.key;
#endif
}
