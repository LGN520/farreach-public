#include "parser.h"

int ParserIterator::load_batch_size = 1 * 1000 * 1000; // 1M records per batch

ParserIterator::~ParserIterator() {}

#ifdef USE_YCSB
netreach_key_t extract_key(const char *buf, int buflen) {
	std::string keystr(buf, buflen);
	uint64_t tmpkey = std::stoull(keystr);
	uint32_t keyhilo = 0;
	uint32_t keyhihi = 0;
	memcpy((void *)&keyhilo, (void *)&tmpkey, sizeof(uint32_t)); // lowest 4B -> keyhilo
	memcpy((void *)&keyhihi, ((char *)&tmpkey)+4, sizeof(uint32_t)); // highest 4B -> keyhihi
	return netreach_key_t(0, 0, keyhilo, keyhihi);
}
#elif defined USE_SYNTHETIC
netreach_key_t extract_key(const char *buf, int buflen) {
	std::string keystr(buf, buflen);
	uint32_t tmpkey = std::stoul(keystr);
	uint16_t keyhihilo = 0;
	uint16_t keyhihihi = 0;
	memcpy((void *)&keyhihihi, (void *)&tmpkey, sizeof(uint16_t)); // lowest 2B -> keyhihihi
	memcpy((void *)&keyhihilo, ((char *)&tmpkey)+2, sizeof(uint16_t)); // highest 2B -> keyhihilo
	uint32_t keyhihi = (keyhihihi << 16) | keyhihilo;
	//uint32_t keylolo = (2*keyhihi + 3) & 0xFFFFFFFF;
	//uint32_t keylohi = (5*keyhihi + 7) & 0xFFFFFFFF;
	//uint32_t keyhilo = (11*keyhihi + 13) & 0xFFFFFFFF;
	uint32_t keylolo = 0;
	uint32_t keylohi = 0;
	uint32_t keyhilo = 0;
	return netreach_key_t(keylolo, keylohi, keyhilo, keyhihi);
}
#endif
