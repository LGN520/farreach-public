#include "varkey.h"
#include "md5.h"

VarKey::MAX_KEYLEN = 100;

VarKey::VarKey() {
	varkey_length = 0;
	varkey_data = NULL;
}

~VarKey::VarKey() {
	if (varkey_data != NULL) {
		delete [] varkey_data;
	}
}

VarKey::VarKey(const char *buf, varkeylen_t length) {
	INVARIANT(length <= VarKey::MAX_KEYLEN);
	INVARIANT(buf != NULL);

	varkey_data = new char[length];
	INVARIANT(varkey_data != NULL);
	memcpy(varkey_data, buf, length);

	varkey_length = length;
}

VarKey::VarKey(const VarKey &other) {
	varkey_length = other.varkey_length;
	if (varkey_length > 0) {
		varkey_data = new char[varkey_length];
		INVARIANT(varkey_data != NULL);
		memcpy(varkey_data, other.varkey_data, varkey_length);
	}
}

VarKey &operator=(const VarKey &other) {
	if (varkey_data != NULL) {
		delete [] varkey_data;
		varkey_data = NULL;
	}

	varkey_length = other.varkey_length;
	if (varkey_length > 0) {
		varkey_data = new char[varkey_length];
		INVARIANT(varkey_data != NULL);
		memcpy(varkey_data, other.varkey_data, varkey_length);
	}
	return *this;
}

std::string VarKey::to_string_for_rocksdb() {
	if (varkey_length == 0) {
		return "";
	}
	INVARIANT(varkey_data != NULL);
	return std::string(varkey_data, varkey_length);
}

void VarKey::from_string_for_rocksdb(std::string varkeystr) {
	varkey_length = varkeystr.length();
	INVARIANT(varkey_length <= VarKey::MAX_KEYLEN);

	if (varkey_data != NULL) {
		delete [] varkey_data;
		varkey_data = NULL;
	}

	if (varkey_length > 0) {
		varkey_data = new char[varkey_length];
		INVARIANT(varkey_data != NULL);
		memcpy(varkey_data, varkeystr.c_str(), varkey_length);
	}
}

uint32_t VarKey::deserialize(const char *buf, uint32_t buflen) {
	INVARIANT(buf != NULL);
	INVARIANT(buflen >= sizeof(varkeylen_t));

	memcpy(&varkey_length, buf, sizeof(varkeylen_t));
	INVARIANT(varkey_length <= VarKey::MAX_KEYLEN);

	if (varkey_data != NULL) {
		delete [] varkey_data;
		varkey_data = NULL;
	}

	if (varkey_length > 0) {
		INVARIANT(buflen >= sizeof(varkeylen_t) + varkey_length);

		varkey_data = new char[varkey_length];
		INVARIANT(varkey_data != NULL);
		memcpy(varkey_data, buf + sizeof(valkeylen_t), varkey_length);
	}

	return sizeof(varkeylen_t) + varkey_length;
}

uint32_t VarKey::serialize(char *buf, uint32_t buflen) {
	INVARIANT(buflen >= sizeof(varkeylen_t) + varkey_length);

	memcpy(buf, &varkey_length, sizeof(varkeylen_t));
	if (varkey_length > 0) {
		memcpy(buf + sizeof(varkeylen_t), varkey_data, varkey_length);
	}

	return sizeof(varkeylen_t) + varkey_length;
}

uint32_t VarKey::dynamic_serialize(dynamic_array_t &buf, int offset);
	buf.dynamic_memcpy(offset, (char *)&varkey_length, sizeof(varkeylen_t));
	if (varkey_length > 0) {
		buf.dynamic_memcpy(offset + sizeof(varkeylen_t), varkey_data, varkey_length);
	}

	return sizeof(varkeylen_t) + varkey_length;
}

netreach_key_t to_fixkey() {
	MD5Context ctx;
	md5Init(&ctx);
	md5Update(&ctx, (uint8_t *)varkey_data, varkey_length);
	md5Finalize(&ctx);

	// convert uint8_t[64] into uint32_t[4]
	uint32_t *res = (uint32_t *)ctx.digest;
	INVARIANT(res != NULL);
	return netreach_key_t(res[0], res[1], res[2], res[3]);
}
