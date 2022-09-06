#ifndef VARKEY_H
#define VARKEY_H

#include "key.h"
#include "helper.h"
#include "dynamic_array.h"

typedef uint32_t varkeylen_t;

// NOTE: VarKey will be placed into payload instead of being parsed/deparsed by switch
class VarKey {
	public:
		static varkeylen_t MAX_KEYLEN; // max # of bytes

		VarKey();
		~VarKey();
		VarKey(const char *buf, varkeylen_t length);
  		VarKey(const VarKey &other);

		VarKey &operator=(const VarKey &other);

		// NOTE: seq is embedded with val instead of key/varkey
		// NOTE: we assume that clients use the same bytewisecomparator as rocksdb to compare/sort/scan keys -> we can change from/to_string_for_rocksdb to support other methods, while it is just an applicate-related engineering work, which is unrelated with our core design
		std::string to_string_for_rocksdb();
		void from_string_for_rocksdb(std::string varkeystr);

		// <varkeylen_t keylen, keybytes>
		uint32_t deserialize(const char *buf, uint32_t buflen);
		uint32_t serialize(char *buf, uint32_t buflen);
		uint32_t dynamic_serialize(dynamic_array_t &buf, int offset);

		// convert into fixed-length key
		// NOTE: we use the most significant 16b of fixed-length key for range partition due to Tofino limitation, and hold it in all methods for fair comparison; for range partition of variable-length, we can use range-partition-aware converstion between varkey and fixkey, or use client/server-based partition as long as with fair comparison, which is just a client-level engineering work, which is unrelated with our core design
		netreach_key_t to_fixkey();

		varkeylen_t varkey_length;
		char *varkey_data;
};

typedef VarKey varkey_t;

#endif
