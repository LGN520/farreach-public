#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <sys/stat.h>
#include "../helper.h"
#include "../key.h"
#include "../val.h"
#include "../packet_format.h"

class ParserIterator {
	public:
		ParserIterator(const char* filename);
		ParserIterator(const ParserIterator& other);
		~ParserIterator();

		static int load_batch_size;

		Key key();
		Val val();
		uint8_t type();
		std::string line();
		bool next();

		Key *keys();
		Val *vals();
		uint8_t *types();
		int maxidx();
		bool next_batch();

		void closeiter();
	private:
		// only update at constructor
		int _fd = -1;
		int _filesize = 0;

		// update at next_batch
		char *_content = NULL;
		int _fileoffset = 0;
		int _maxidx = load_batch_size - 1;

		// update at next and next_batch
		int _idx = -1; // [0, _maxidx]

		Key *_keys = NULL;
		Val *_vals = NULL;
		uint8_t *_types = NULL;
		std::string *_lines = NULL;

		void unmap_content();
		void map_content();
		bool parsekv(const char* line, int linelen);
		bool parsekey(const char* line, int linelen);
};

#endif
