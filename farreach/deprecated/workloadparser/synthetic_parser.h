#ifndef SYNTHETIC_PARSER_H
#define SYNTHETIC_PARSER_H

#include <stdio.h> // fopen
#include <sys/mman.h> // mmap, munmap
#include <fcntl.h> // open
#include <errno.h> // errno
// fstat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "parser.h"
#include "../packet_format.h"

class SyntheticParserIterator : public ParserIterator {
	public:
		SyntheticParserIterator(const char* filename);
		SyntheticParserIterator(const SyntheticParserIterator& other);
		virtual ~SyntheticParserIterator();

		virtual Key key();
		virtual Val val();
		virtual optype_t type();
		virtual std::string line();
		virtual bool next();
		virtual void reset();

		virtual Key *keys();
		virtual Val *vals();
		virtual optype_t *types();
		virtual int maxidx();
		virtual bool next_batch();

		virtual void closeiter();
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
		optype_t *_types = NULL;
		std::string *_lines = NULL;

		void unmap_content();
		void map_content();
		bool parsekv(const char* line, int linelen);
		bool parsekey(const char* line, int linelen);
};

#endif
