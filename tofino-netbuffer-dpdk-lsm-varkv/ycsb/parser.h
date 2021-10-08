#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "../helper.h"
#include "../key.h"
#include "../val.h"
#include "../packet_format.h"

class ParserIterator {
	public:
		ParserIterator(const char* filename);
		ParserIterator(const ParserIterator& other);
		~ParserIterator();

		Key key();
		Val val();
		uint8_t type();
		bool next();
		void close();
	private:
		Key _key;
		Val _val;
		uint8_t _type;
		FILE *fp = nullptr;

		bool parsekv(const char* line);
		bool parsekey(const char* line);
};

class Parser {
	public:
		Parser(const char* filename);
		ParserIterator begin();
	private:
		char filename[256];
};

#endif
