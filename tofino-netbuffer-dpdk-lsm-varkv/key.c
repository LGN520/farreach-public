#include "key.h"

static constexpr size_t Key::model_key_size() { 
	return 1; 
}

static Key Key::max() {
#ifdef LARGE_KEY
    static Key max_key(std::numeric_limits<uint64_t>::max(),
			std::numeric_limits<uint64_t>::max());
#else
    static Key max_key(std::numeric_limits<uint64_t>::max());
#endif
    return max_key;
}

static Key Key::min() {
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
	char buf[16];
	// NOTE: keylo-keyhi, in rocksdb/include/slice.h, we compare two slices from end to beginning
	memcpy(buf, (const char *)&keylo, 8);
	memcpy(buf+8, (const char *)&keyhi, 8);
	rocksdb::Slice result(buf, 16);
#else
	rocksdb::Slice result((char *)(&key), 8); // convert uint64_t to char[8]
#endif
	return result;
}

void Key::from_slice() {
#ifdef LARGE_KEY
	keylo = *(uint64_t*)slice.data_;
	keyhi = *(uint64_t*)(slice.data_+8);
#else
	key = *(uint64_t*)slice.data_;
#endif
}

model_key_t Key::to_model_key() const {
    model_key_t model_key;
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
	return (l.keyhi == r.key) && (l.keylo == r.keylo);
#else
	return l.key == r.key;
#endif
}

bool operator!=(const Key &l, const Key &r) { 
#ifdef LARGE_KEY
	return (l.keyhi != r.key) || (l.keylo != r.keylo);
#else
	return l.key != r.key;
#endif
}
