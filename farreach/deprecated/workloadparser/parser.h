#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <string>

#include "../../common/helper.h"
#include "../../common/key.h"
#include "../../common/val.h"
#include "../packet_format.h"

#define MINIMUM_WORKLOAD_FILESIZE

#define MAX_LINE_SIZE 256

class ParserIterator {
	public:
		virtual ~ParserIterator() = 0;

		static int load_batch_size;

		virtual Key key() = 0;
		virtual Val val() = 0;
		virtual optype_t type() = 0;
		virtual std::string line() = 0;
		virtual bool next() = 0;
		virtual void reset() = 0;

		virtual Key *keys() = 0;
		virtual Val *vals() = 0;
		virtual optype_t *types() = 0;
		virtual int maxidx() = 0;
		virtual bool next_batch() = 0;

		virtual void closeiter() = 0;
};

netreach_key_t extract_key(const char *buf, int buflen);

#endif
